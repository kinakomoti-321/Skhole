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

	void Renderer::SetResolution(uint32_t width, uint32_t height) {

	}

}