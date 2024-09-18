#include <renderer/renderer.h>

namespace Skhole {

	void Renderer::Initialize(RendererDesc& desc)
	{
		VkHelper::Context::VulkanInitialzeInfo initInfo{};
		initInfo.apiVersion = VK_API_VERSION_1_2;
		initInfo.layers = GetLayer();
		initInfo.extensions = GetExtensions();
		initInfo.useWindow = desc.useWindow;
		initInfo.window = desc.window;

		m_context.InitCore(initInfo);

		m_commandPool = vkutils::createCommandPool(*m_context.device, m_context.queueIndex);
		m_commandBuffer = vkutils::createCommandBuffer(*m_context.device, *m_commandPool);

		m_imGuiRenderPass = VkHelper::CreateRenderPass(
			vk::Format::eR8G8B8A8Unorm,
			vk::ImageLayout::eColorAttachmentOptimal,
			vk::ImageLayout::ePresentSrcKHR,
			*m_context.device
		);

		VkHelper::SwapChainInfo swapchainInfo{};
		swapchainInfo.physicalDevice = m_context.physicalDevice;
		swapchainInfo.device = *m_context.device;
		swapchainInfo.surface = *m_context.surface;
		swapchainInfo.queueIndex = m_context.queueIndex;
		swapchainInfo.queue = m_context.queue;
		swapchainInfo.commandPool = *m_commandPool;
		swapchainInfo.renderPass = m_imGuiRenderPass.get();

		swapchainInfo.swapcahinImageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;

		swapchainInfo.width = desc.Width;
		swapchainInfo.height = desc.Height;

		m_screenContext.Init(swapchainInfo);

		InitializeBiniding();

		RaytracingPipeline::Desc pipelineDesc;
		pipelineDesc.device = *m_context.device;
		pipelineDesc.physicalDevice = m_context.physicalDevice;
		pipelineDesc.descriptorSetLayout = m_bindingManager.descriptorSetLayout;

		pipelineDesc.raygenShaderPath = "shader/simple_raytracer/raygen.rgen.spv";
		pipelineDesc.missShaderPath = "shader/simple_raytracer/miss.rmiss.spv";
		pipelineDesc.closestHitShaderPath = "shader/simple_raytracer/closesthit.rchit.spv";

		m_raytracingPipeline.InitPipeline(pipelineDesc);

		m_imGuiManager.Init(
			desc.window,
			*m_context.instance,
			m_context.physicalDevice,
			*m_context.device,
			m_context.queueIndex,
			m_context.queue,
			*m_imGuiRenderPass,
			2,
			m_screenContext.swapchainImages.size()
		);

		//--------------------------------------
		// PostProcessor
		//--------------------------------------
		m_postProcessor = GetPostProcessor(desc.posproType);

		PostProcessor::Desc ppDesc{};
		ppDesc.physicalDevice = m_context.physicalDevice;
		ppDesc.device = *m_context.device;
		ppDesc.queue = m_context.queue;
		ppDesc.commandPool = *m_commandPool;
		ppDesc.width = desc.Width;
		ppDesc.height = desc.Height;
		m_postProcessor->Init(ppDesc);

		InitializeCore(desc);
	}

	void Renderer::Resize(unsigned int width, unsigned int height)
	{
		m_screenContext.Release(*m_context.device);

		VkHelper::SwapChainInfo swapchainInfo{};
		swapchainInfo.physicalDevice = m_context.physicalDevice;
		swapchainInfo.device = *m_context.device;
		swapchainInfo.surface = *m_context.surface;
		swapchainInfo.queueIndex = m_context.queueIndex;
		swapchainInfo.queue = m_context.queue;
		swapchainInfo.commandPool = *m_commandPool;
		swapchainInfo.renderPass = m_imGuiRenderPass.get();
		swapchainInfo.swapcahinImageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;
		swapchainInfo.width = width;
		swapchainInfo.height = height;
		m_screenContext.Init(swapchainInfo);

		m_renderImages.Resize(width, height, *m_context.device, m_context.physicalDevice, *m_commandPool, m_context.queue);
		m_postProcessor->Resize(width, height);

		m_scene->m_rendererParameter->sample = 1;

		ResizeCore(width, height);
	}

	void Renderer::Destroy()
	{
		m_context.device->waitIdle();

		DestroyCore();

		m_bindingManager.Release(*m_context.device);
		m_postProcessor->Destroy(*m_context.device);
		m_postProcessor = nullptr;
		m_renderImages.Release(*m_context.device);
		m_screenContext.Release(*m_context.device);
		m_imGuiManager.Destroy(*m_context.device);
	}

	void Renderer::RaytracingCommand(const vk::CommandBuffer& commandBuffer, uint32_t width, uint32_t height) {
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, m_raytracingPipeline.GetPipeline());
		commandBuffer.bindDescriptorSets(
			vk::PipelineBindPoint::eRayTracingKHR,
			m_raytracingPipeline.GetPipelineLayout(),
			0,
			m_bindingManager.descriptorSet,
			nullptr
		);

		m_commandBuffer->traceRaysKHR(
			m_raytracingPipeline.GetRaygenRegion(),
			m_raytracingPipeline.GetMissRegion(),
			m_raytracingPipeline.GetHitRegion(),
			{},
			width, height, 1
		);
	}

	void Renderer::CopyRenderToScreen(const vk::CommandBuffer& commandBuffer, vk::Image src, vk::Image screen, uint32_t width, uint32_t height) {
		vkutils::setImageLayout(*m_commandBuffer, src, vk::ImageLayout::eGeneral, vk::ImageLayout::eTransferSrcOptimal);
		vkutils::setImageLayout(*m_commandBuffer, screen, vk::ImageLayout::ePresentSrcKHR, vk::ImageLayout::eTransferDstOptimal);

		vk::ImageCopy region;
		region.srcSubresource = vk::ImageSubresourceLayers()
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setMipLevel(0)
			.setBaseArrayLayer(0)
			.setLayerCount(1);
		region.srcOffset = vk::Offset3D(0, 0, 0);

		region.dstSubresource = vk::ImageSubresourceLayers()
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setMipLevel(0)
			.setBaseArrayLayer(0)
			.setLayerCount(1);
		region.dstOffset = vk::Offset3D(0, 0, 0);

		region.extent = vk::Extent3D(width, height, 1);

		m_commandBuffer->copyImage(
			src, vk::ImageLayout::eTransferSrcOptimal,
			screen, vk::ImageLayout::eTransferDstOptimal,
			region
		);

		vkutils::setImageLayout(*m_commandBuffer, src, vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eGeneral);
		vkutils::setImageLayout(*m_commandBuffer, screen, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eAttachmentOptimal);
	}

	void Renderer::RenderImGuiCommand(const vk::CommandBuffer& commandBuffer, vk::Framebuffer frameBuffer, uint32_t width, uint32_t height) {
		vk::RenderPassBeginInfo renderPassInfo{};
		renderPassInfo.setRenderPass(*m_imGuiRenderPass);
		renderPassInfo.setFramebuffer(frameBuffer);
		vk::Rect2D rect({ 0,0 }, { (uint32_t)width,(uint32_t)height });

		renderPassInfo.setRenderArea(rect);

		m_commandBuffer->beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *m_commandBuffer);

		m_commandBuffer->endRenderPass();
	}

	ShrPtr<RendererDefinisionMaterial> Renderer::GetMaterial(const ShrPtr<BasicMaterial>& material) {
		ShrPtr<RendererDefinisionMaterial> materialDef = MakeShr<RendererDefinisionMaterial>();
		materialDef->materialName = material->materialName;

		DefineMaterial(materialDef,material);

		return materialDef;
	}

	ShrPtr<RendererDefinisionCamera> Renderer::GetCamera(const ShrPtr<RendererDefinisionCamera>& basicCamera) {
		ShrPtr<RendererDefinisionCamera> cameraDef = MakeShr<RendererDefinisionCamera>();
		cameraDef->cameraName = basicCamera->cameraName;
		cameraDef->position = basicCamera->position;
		cameraDef->foward = basicCamera->foward;
		cameraDef->up = basicCamera->up;
		cameraDef->fov = basicCamera->fov;

		DefineCamera(cameraDef);

		return cameraDef;
	}

	void Renderer::ResetSample() {
		m_rendererProperty.sample = 1;
	}

}
