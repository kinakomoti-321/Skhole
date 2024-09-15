#include <renderer/simple_raytracer.h>
#include <common/log.h>
#include <vulkan_helpler/vkutils.hpp>

namespace Skhole {

	//--------------------------------------
	// Interface Method
	//--------------------------------------

	SimpleRaytracer::SimpleRaytracer()
	{

	}

	SimpleRaytracer::~SimpleRaytracer()
	{

	}

	void SimpleRaytracer::InitImGui()
	{
		m_imGuiManager.Init(
			m_desc.window,
			*m_context.instance,
			m_context.physicalDevice,
			*m_context.device,
			m_context.queueIndex,
			m_context.queue,
			*m_imGuiRenderPass,
			2,
			m_swapchainContext.swapchainImages.size()
		);
	}

	void SimpleRaytracer::Init(RendererDesc& desc)
	{
		SKHOLE_LOG_SECTION("Initialze Renderer");
		m_desc = desc;

		//--------------------------------------
		// Vulkan Initialize
		//--------------------------------------
		VkHelper::Context::VulkanInitialzeInfo initInfo{};
		initInfo.apiVersion = VK_API_VERSION_1_2;
		initInfo.layers = m_layer;
		initInfo.extensions = m_extension;
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

		//--------------------------------------
		// swapChain Initialize
		//--------------------------------------
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

		m_swapchainContext.Init(swapchainInfo);

		//--------------------------------------
		// Create Pipeline
		//--------------------------------------

		SKHOLE_LOG("... Creating Pipeline");
		CreateRaytracingPipeline();
		SKHOLE_LOG("... End Creating Pipeline");

		SKHOLE_LOG("... Initialization ImGUI");
		InitImGui();
		SKHOLE_LOG("... End Initialization ImGUI");

		//--------------------------------------
		// Create Buffer
		//--------------------------------------

		uniformBufferObject.frame = m_raytracerParameter.frame;
		uniformBufferObject.spp = m_raytracerParameter.spp;
		uniformBufferObject.width = m_desc.Width;
		uniformBufferObject.height = m_desc.Height;

		uniformBufferObject.cameraDir = vec3(0.0f, 0.0f, -1.0f);
		uniformBufferObject.cameraPos = vec3(0.0, 30.0, 50.0);
		uniformBufferObject.cameraUp = vec3(0.0f, 1.0f, 0.0f);
		uniformBufferObject.cameraRight = vec3(1.0f, 0.0f, 0.0f);

		m_uniformBuffer.init(m_context.physicalDevice, *m_context.device, sizeof(UniformBufferObject),
			vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
			&uniformBufferObject
		);

		accumImage.Init(
			m_context.physicalDevice, *m_context.device,
			m_desc.Width, m_desc.Height,
			vk::Format::eR32G32B32A32Sfloat,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eStorage,
			vk::MemoryPropertyFlagBits::eDeviceLocal
		);

		renderImage.Init(
			m_context.physicalDevice, *m_context.device,
			m_desc.Width, m_desc.Height,
			vk::Format::eR32G32B32A32Sfloat,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eStorage,
			vk::MemoryPropertyFlagBits::eDeviceLocal
		);

		posproIamge.Init(
			m_context.physicalDevice, *m_context.device,
			m_desc.Width, m_desc.Height,
			vk::Format::eR8G8B8A8Unorm,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eDeviceLocal
		);

		vkutils::oneTimeSubmit(*m_context.device, *m_commandPool, m_context.queue,
			[&](vk::CommandBuffer commandBuffer) {
				vkutils::setImageLayout(
					commandBuffer, accumImage.GetImage(),
					vk::ImageLayout::eUndefined,
					vk::ImageLayout::eGeneral
				);
				vkutils::setImageLayout(
					commandBuffer, renderImage.GetImage(),
					vk::ImageLayout::eUndefined,
					vk::ImageLayout::eGeneral
				);
				vkutils::setImageLayout(
					commandBuffer, posproIamge.GetImage(),
					vk::ImageLayout::eUndefined,
					vk::ImageLayout::eGeneral
				);
			});



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

		SKHOLE_LOG_SECTION("Initialze Renderer Completed");
	}

	ShrPtr<RendererParameter> GetRendererParameter()
	{
		ShrPtr<RendererParameter> rendererParameter = MakeShr<RendererParameter>();
		rendererParameter->rendererName = "Simple Raytracer";
		rendererParameter->frame = 0;
		rendererParameter->spp = 100;
		rendererParameter->sample = 1;
		return nullptr;
	}

	void SimpleRaytracer::DestroyScene()
	{
		m_sceneBufferManager.Release(*m_context.device);
		m_asManager.ReleaseTLAS(*m_context.device);
		m_asManager.ReleaseBLAS(*m_context.device);

		m_materials.clear();
		m_materaialBuffer.Release(*m_context.device);

		m_scene = nullptr;
	}

	void SimpleRaytracer::UpdateScene(const UpdataInfo& updateInfo) {
		if (updateInfo.commands.size() == 0) return;

		m_scene->m_rendererParameter->sample = 1;
		for (auto& command : updateInfo.commands) {
			ShrPtr<UpdateObjectCommand> objCommand;
			ShrPtr<UpdateMaterialCommand> matCommand;

			switch (command->GetCommandType()) {
			case UpdateCommandType::CAMERA:
				break;
			case UpdateCommandType::RENDERER:
				break;
			case UpdateCommandType::MATERIAL:
				matCommand = std::static_pointer_cast<UpdateMaterialCommand>(command);
				UpdateMaterialBuffer(matCommand->materialIndex);
				break;
			case UpdateCommandType::OBJECT:
				objCommand = std::static_pointer_cast<UpdateObjectCommand>(command);
				m_scene->m_objects[objCommand->objectIndex]->ResetWorldTransformMatrix();
				break;
			default:
				SKHOLE_UNIMPL("Command");
				break;
			}
		}

	}

	void SimpleRaytracer::InitFrameGUI() {
		m_imGuiManager.NewFrame();
	}

	void SimpleRaytracer::Destroy()
	{
		m_context.device->waitIdle();

		m_postProcessor->Destroy(*m_context.device);
		accumImage.Release(*m_context.device);
		renderImage.Release(*m_context.device);
		posproIamge.Release(*m_context.device);

		m_bindingManager.Release(*m_context.device);
		m_imGuiManager.Destroy(*m_context.device);
	}

	void SimpleRaytracer::Resize(unsigned int width, unsigned int height)
	{
		m_desc.Width = width;
		m_desc.Height = height;

		m_swapchainContext.Release(*m_context.device);

		VkHelper::SwapChainInfo swapchainInfo{};
		swapchainInfo.physicalDevice = m_context.physicalDevice;
		swapchainInfo.device = *m_context.device;
		swapchainInfo.surface = *m_context.surface;
		swapchainInfo.queueIndex = m_context.queueIndex;
		swapchainInfo.queue = m_context.queue;
		swapchainInfo.commandPool = *m_commandPool;
		swapchainInfo.renderPass = m_imGuiRenderPass.get();

		//swapchainInfo.swapcahinImageUsage = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eColorAttachment;
		swapchainInfo.swapcahinImageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;

		swapchainInfo.width = m_desc.Width;
		swapchainInfo.height = m_desc.Height;

		m_swapchainContext.Init(swapchainInfo);

		accumImage.Release(*m_context.device);
		renderImage.Release(*m_context.device);
		posproIamge.Release(*m_context.device);

		accumImage.Init(
			m_context.physicalDevice, *m_context.device,
			m_desc.Width, m_desc.Height,
			vk::Format::eR32G32B32A32Sfloat,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eStorage,
			vk::MemoryPropertyFlagBits::eDeviceLocal
		);

		renderImage.Init(
			m_context.physicalDevice, *m_context.device,
			m_desc.Width, m_desc.Height,
			vk::Format::eR32G32B32A32Sfloat,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eStorage,
			vk::MemoryPropertyFlagBits::eDeviceLocal
		);

		posproIamge.Init(
			m_context.physicalDevice, *m_context.device,
			m_desc.Width, m_desc.Height,
			vk::Format::eR8G8B8A8Unorm,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eDeviceLocal
		);

		vkutils::oneTimeSubmit(*m_context.device, *m_commandPool, m_context.queue,
			[&](vk::CommandBuffer commandBuffer) {
				vkutils::setImageLayout(
					commandBuffer, accumImage.GetImage(),
					vk::ImageLayout::eUndefined,
					vk::ImageLayout::eGeneral
				);
				vkutils::setImageLayout(
					commandBuffer, renderImage.GetImage(),
					vk::ImageLayout::eUndefined,
					vk::ImageLayout::eGeneral
				);
				vkutils::setImageLayout(
					commandBuffer, posproIamge.GetImage(),
					vk::ImageLayout::eUndefined,
					vk::ImageLayout::eGeneral
				);
			});

		m_postProcessor->Resize(width, height);
		m_scene->m_rendererParameter->sample = 1;
	}

	void SimpleRaytracer::RealTimeRender(const RealTimeRenderingInfo& renderInfo)
	{
		m_raytracerParameter.spp++;

		FrameStart(renderInfo.time);

		vk::UniqueSemaphore imageAvailableSemaphore =
			m_context.device->createSemaphoreUnique({});

		auto& swapchain = m_swapchainContext.swapchain;
		auto& swapchainImages = m_swapchainContext.swapchainImages;
		auto& swapchainImageViews = m_swapchainContext.swapchainImageViews;
		auto& swapchainFramebuffers = m_swapchainContext.frameBuffers;

		auto result = m_context.device->acquireNextImageKHR(
			*swapchain, std::numeric_limits<uint64_t>::max(),
			*imageAvailableSemaphore);
		if (result.result != vk::Result::eSuccess) {
			std::cerr << "Failed to acquire next image.\n";
			std::abort();
		}

		uint32_t imageIndex = result.value;
		UpdateDescriptorSet(*swapchainImageViews[imageIndex]);

		RecordCommandBuffer(swapchainImages[imageIndex], *swapchainFramebuffers[imageIndex]);

		vk::PipelineStageFlags waitStage{ vk::PipelineStageFlagBits::eTopOfPipe };
		vk::SubmitInfo submitInfo{};
		submitInfo.setWaitDstStageMask(waitStage);
		submitInfo.setCommandBuffers(*m_commandBuffer);
		submitInfo.setWaitSemaphores(*imageAvailableSemaphore);
		m_context.queue.submit(submitInfo);

		m_context.queue.waitIdle();

		vk::PresentInfoKHR presentInfo{};
		presentInfo.setSwapchains(*swapchain);
		presentInfo.setImageIndices(imageIndex);
		if (m_context.queue.presentKHR(presentInfo) != vk::Result::eSuccess) {
			std::cerr << "Failed to present.\n";
			std::abort();
		}

		FrameEnd();
	}

	void SimpleRaytracer::OfflineRender(const OfflineRenderingInfo& renderInfo)
	{
		SKHOLE_UNIMPL("Offline Render");
	}

	void SimpleRaytracer::SetScene(ShrPtr<Scene> scene) {
		SKHOLE_LOG_SECTION("Set Scene");
		m_scene = scene;
		InitBufferManager();
		InitAccelerationStructures();

		// Set Material
		{
			m_materials.reserve(m_scene->m_materials.size());
			auto& materials = m_scene->m_materials;
			for (auto& materialDef : materials)
			{
				m_materials.push_back(ConvertMaterial(materialDef));
			}
		}

		m_materaialBuffer.Init(
			m_context.physicalDevice, *m_context.device,
			m_materials.size() * sizeof(Material),
			vk::BufferUsageFlagBits::eStorageBuffer,
			vk::MemoryPropertyFlagBits::eDeviceLocal
		);

		void* map = m_materaialBuffer.Map(*m_context.device, 0, m_materials.size() * sizeof(Material));
		memcpy(map, m_materials.data(), m_materials.size() * sizeof(Material));
		m_materaialBuffer.Unmap(*m_context.device);

		m_materaialBuffer.UploadToDevice(*m_context.device, *m_commandPool, m_context.queue);
		SKHOLE_LOG_SECTION("End Set Scene");
	}

	void SimpleRaytracer::InitBufferManager() {
		SKHOLE_LOG("Init Buffer Manager");

		m_sceneBufferManager.SetScene(m_scene);

		m_sceneBufferManager.InitGeometryBuffer(m_context.physicalDevice, *m_context.device, *m_commandPool, m_context.queue);
		m_sceneBufferManager.InitInstanceBuffer(m_context.physicalDevice, *m_context.device, *m_commandPool, m_context.queue);
	}


	ShrPtr<RendererDefinisionMaterial> SimpleRaytracer::GetMaterial(const ShrPtr<BasicMaterial>& material)
	{
		ShrPtr<RendererDefinisionMaterial> materialDef = MakeShr<RendererDefinisionMaterial>();
		materialDef->materialName = material->materialName;

		materialDef->materialParameters.resize(m_matParams.size());
		for (int i = 0; i < m_matParams.size(); i++)
		{
			materialDef->materialParameters[i] = m_matParams[i]->Copy();
		}

		materialDef->materialParameters[0]->setParamValue(material->basecolor);
		materialDef->materialParameters[1]->setParamValue(material->metallic);
		materialDef->materialParameters[2]->setParamValue(material->roughness);
		materialDef->materialParameters[3]->setParamValue(material->emissionIntensity);
		materialDef->materialParameters[4]->setParamValue(material->emissionColor);

		return materialDef;
	}

	ShrPtr<RendererDefinisionCamera> SimpleRaytracer::GetCamera(const ShrPtr<RendererDefinisionCamera>& camera)
	{
		ShrPtr<RendererDefinisionCamera> cameraDef = MakeShr<RendererDefinisionCamera>();
		cameraDef->cameraName = camera->cameraName;

		cameraDef->position = camera->position;
		cameraDef->foward = camera->foward;
		cameraDef->up = camera->up;
		cameraDef->fov = camera->fov;

		cameraDef->extensionParameters = m_camExtensionParams;


		return cameraDef;
	}

	ShrPtr<RendererParameter> SimpleRaytracer::GetRendererParameter() {

		ShrPtr<RendererParameter> rendererParameter = MakeShr<RendererParameter>();
		rendererParameter->rendererName = "Simple Raytracer";
		rendererParameter->frame = 0;
		rendererParameter->spp = 100;
		rendererParameter->sample = 1;

		rendererParameter->rendererParameters = m_rendererExtensionParams;
		rendererParameter->posproParameters = m_postProcessor->GetParamter();

		return rendererParameter;
	}

	//--------------------------------------
	// Internal Method
	//--------------------------------------
	void SimpleRaytracer::CreateRaytracingPipeline() {
		//PrepareShader();

		std::vector<VkHelper::BindingLayoutElement> bindingLayout = {
			{0, vk::DescriptorType::eAccelerationStructureKHR, 1, vk::ShaderStageFlagBits::eRaygenKHR},
			{1, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eRaygenKHR},
			{2, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eRaygenKHR },
			{3, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eClosestHitKHR},
			{4, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eClosestHitKHR},
			{5, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eClosestHitKHR},
			{6, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eClosestHitKHR},
			{7, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eClosestHitKHR},
			{8, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eClosestHitKHR},
			{9, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eRaygenKHR},
		};

		m_bindingManager.SetBindingLayout(*m_context.device, bindingLayout, vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);

		RaytracingPipeline::Desc desc;
		desc.device = *m_context.device;
		desc.physicalDevice = m_context.physicalDevice;
		desc.descriptorSetLayout = m_bindingManager.descriptorSetLayout;

		desc.raygenShaderPath = "shader/simple_raytracer/raygen.rgen.spv";
		desc.missShaderPath = "shader/simple_raytracer/miss.rmiss.spv";
		desc.closestHitShaderPath = "shader/simple_raytracer/closesthit.rchit.spv";

		m_raytracingPipeline.InitPipeline(desc);
	}

	void SimpleRaytracer::FrameStart(float time) {

		auto& raytracerParam = m_scene->m_rendererParameter;

		auto& camera = m_scene->m_camera;
		uniformBufferObject.spp = raytracerParam->spp;
		uniformBufferObject.frame = raytracerParam->frame;
		uniformBufferObject.sample = raytracerParam->sample;
		auto param = std::dynamic_pointer_cast<ParamUint>(raytracerParam->rendererParameters[0]);
		uniformBufferObject.mode = param->value;

		uniformBufferObject.cameraPos = camera->GetCameraPosition(time);
		vec3 cameraDir, cameraUp, cameraRight;
		camera->GetCameraDirections(time, cameraDir, cameraUp, cameraRight);
		uniformBufferObject.cameraDir = cameraDir;
		uniformBufferObject.cameraUp = cameraUp;
		uniformBufferObject.cameraRight = cameraRight;
		uniformBufferObject.cameraParam.x = camera->GetYFov();
		uniformBufferObject.cameraParam.y = static_cast<float>(m_desc.Width) / static_cast<float>(m_desc.Height);

		void* map = m_uniformBuffer.Map(*m_context.device, 0, sizeof(UniformBufferObject));
		memcpy(map, &uniformBufferObject, sizeof(UniformBufferObject));
		m_uniformBuffer.Ummap(*m_context.device);

		m_scene->SetTransformMatrix(time);

		m_sceneBufferManager.FrameUpdateInstance(time, *m_context.device, *m_commandPool, m_context.queue);
		m_asManager.BuildTLAS(m_sceneBufferManager, m_context.physicalDevice, *m_context.device, *m_commandPool, m_context.queue);
	}

	void SimpleRaytracer::FrameEnd()
	{
		m_asManager.ReleaseTLAS(*m_context.device);

		auto& raytracerParam = m_scene->m_rendererParameter;
		raytracerParam->sample++;
		if (raytracerParam->sample >= raytracerParam->spp) {
			raytracerParam->sample = raytracerParam->spp;
		}
	}

	void SimpleRaytracer::RecordCommandBuffer(vk::Image image, vk::Framebuffer frameBuffer) {
		m_commandBuffer->begin(vk::CommandBufferBeginInfo{});

		m_commandBuffer->bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, m_raytracingPipeline.GetPipeline());

		m_commandBuffer->bindDescriptorSets(
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
			m_desc.Width, m_desc.Height, 1
		);

		// Post Process
		PostProcessor::ExecuteDesc desc{};
		desc.device = *m_context.device;
		desc.inputImage = renderImage.GetImageView();
		desc.outputImage = posproIamge.GetImageView();
		desc.param = m_scene->m_rendererParameter->posproParameters;

		m_postProcessor->Execute(*m_commandBuffer, desc);

		// Copy RenderImage -> WindowImage
		vkutils::setImageLayout(*m_commandBuffer, posproIamge.GetImage(), vk::ImageLayout::eGeneral, vk::ImageLayout::eTransferSrcOptimal);
		vkutils::setImageLayout(*m_commandBuffer, image, vk::ImageLayout::ePresentSrcKHR, vk::ImageLayout::eTransferDstOptimal);

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
		region.extent = vk::Extent3D(m_desc.Width, m_desc.Height, 1);


		m_commandBuffer->copyImage(
			posproIamge.GetImage(), vk::ImageLayout::eTransferSrcOptimal,
			image, vk::ImageLayout::eTransferDstOptimal,
			region
		);

		vkutils::setImageLayout(*m_commandBuffer, posproIamge.GetImage(), vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eGeneral);
		vkutils::setImageLayout(*m_commandBuffer, image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eAttachmentOptimal);

		//--------------------
		// ImGUI
		//--------------------
		vk::RenderPassBeginInfo renderPassInfo{};
		renderPassInfo.setRenderPass(*m_imGuiRenderPass);
		renderPassInfo.setFramebuffer(frameBuffer);
		vk::Rect2D rect({ 0,0 }, { (uint32_t)m_desc.Width,(uint32_t)m_desc.Height });

		renderPassInfo.setRenderArea(rect);

		m_commandBuffer->beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *m_commandBuffer);

		m_commandBuffer->endRenderPass();
		//--------------------
		// ImGUI End
		//--------------------

		m_commandBuffer->end();

	}


	void SimpleRaytracer::UpdateDescriptorSet(vk::ImageView imageView) {
		m_bindingManager.StartWriting();

		m_bindingManager.WriteAS(
			*m_asManager.TLAS.accel, 0, 1, *m_context.device
		);

		m_bindingManager.WriteImage(
			renderImage.GetImageView(), vk::ImageLayout::eGeneral, VK_NULL_HANDLE,
			vk::DescriptorType::eStorageImage, 1, 1, *m_context.device
		);

		m_bindingManager.WriteBuffer(
			*m_uniformBuffer.buffer, 0, sizeof(UniformBufferObject),
			vk::DescriptorType::eUniformBuffer, 2, 1, *m_context.device
		);

		m_bindingManager.WriteBuffer(
			m_sceneBufferManager.vertexBuffer.GetDeviceBuffer(), 0, m_sceneBufferManager.vertexBuffer.GetBufferSize(),
			vk::DescriptorType::eStorageBuffer, 3, 1, *m_context.device
		);

		m_bindingManager.WriteBuffer(
			m_sceneBufferManager.indexBuffer.GetDeviceBuffer(), 0, m_sceneBufferManager.indexBuffer.GetBufferSize(),
			vk::DescriptorType::eStorageBuffer, 4, 1, *m_context.device
		);

		m_bindingManager.WriteBuffer(
			m_sceneBufferManager.geometryBuffer.GetDeviceBuffer(), 0, m_sceneBufferManager.geometryBuffer.GetBufferSize(),
			vk::DescriptorType::eStorageBuffer, 5, 1, *m_context.device
		);

		m_bindingManager.WriteBuffer(
			m_sceneBufferManager.instanceBuffer.GetDeviceBuffer(), 0, m_sceneBufferManager.instanceBuffer.GetBufferSize(),
			vk::DescriptorType::eStorageBuffer, 6, 1, *m_context.device
		);

		m_bindingManager.WriteBuffer(
			m_materaialBuffer.GetDeviceBuffer(), 0, m_materaialBuffer.GetBufferSize(),
			vk::DescriptorType::eStorageBuffer, 7, 1, *m_context.device
		);

		m_bindingManager.WriteBuffer(
			m_sceneBufferManager.matIndexBuffer.GetDeviceBuffer(), 0, m_sceneBufferManager.matIndexBuffer.GetBufferSize(),
			vk::DescriptorType::eStorageBuffer, 8, 1, *m_context.device
		);

		m_bindingManager.WriteImage(
			accumImage.GetImageView(), vk::ImageLayout::eGeneral, VK_NULL_HANDLE,
			vk::DescriptorType::eStorageImage, 9, 1, *m_context.device
		);

		m_bindingManager.EndWriting(*m_context.device);
	}

	void SimpleRaytracer::InitAccelerationStructures() {
		m_asManager.BuildBLAS(m_sceneBufferManager, m_context.physicalDevice, *m_context.device, *m_commandPool, m_context.queue);
	}

	void SimpleRaytracer::UpdateMaterialBuffer(uint32_t matId) {
		m_materials[matId] = ConvertMaterial(m_scene->m_materials[matId]);

		uint32_t byteOffset = matId * sizeof(Material);

		void* map = m_materaialBuffer.Map(*m_context.device, byteOffset, sizeof(Material));
		memcpy(map, m_materials.data() + matId, sizeof(Material));
		m_materaialBuffer.Unmap(*m_context.device);

		m_materaialBuffer.UploadToDevice(*m_context.device, *m_commandPool, m_context.queue, byteOffset, sizeof(Material));
	}

	SimpleRaytracer::Material SimpleRaytracer::ConvertMaterial(const ShrPtr<RendererDefinisionMaterial>& materialDef) {
		Material material;

		auto p1 = std::static_pointer_cast<ParamCol>(materialDef->materialParameters[0]);
		material.baseColor = p1->value;

		auto p2 = std::static_pointer_cast<ParamFloat>(materialDef->materialParameters[1]);
		material.metallic = p2->value;

		auto p3 = std::static_pointer_cast<ParamFloat>(materialDef->materialParameters[2]);
		material.roughness = p3->value;

		auto p4 = std::static_pointer_cast<ParamFloat>(materialDef->materialParameters[3]);
		material.emissionIntesity = p4->value;

		auto p5 = std::static_pointer_cast<ParamCol>(materialDef->materialParameters[4]);
		material.emissionColor = p5->value;

		return material;
	}

	void SimpleRaytracer::SetPostprocess(PostProcessType type) {
		m_postProcessor = GetPostProcessor(type);
		m_postprocessParams = m_postProcessor->GetParamter();
	}
	void SimpleRaytracer::DestroyPostprocess() {
		m_postProcessor->Destroy(*m_context.device);
	}
}
