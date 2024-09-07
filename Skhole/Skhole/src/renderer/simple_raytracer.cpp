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

		CreateRaytracingPipeline();

		InitImGui();

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

		swapchainInfo.swapcahinImageUsage = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eColorAttachment;

		swapchainInfo.width = m_desc.Width;
		swapchainInfo.height = m_desc.Height;

		m_swapchainContext.Init(swapchainInfo);

		accumImage.Release(*m_context.device);

		accumImage.Init(
			m_context.physicalDevice, *m_context.device,
			m_desc.Width, m_desc.Height,
			vk::Format::eR32G32B32A32Sfloat,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eStorage,
			vk::MemoryPropertyFlagBits::eDeviceLocal
		);

		vkutils::oneTimeSubmit(*m_context.device, *m_commandPool, m_context.queue,
			[&](vk::CommandBuffer commandBuffer) {
				vkutils::setImageLayout(commandBuffer, accumImage.GetImage(),
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::eGeneral);
			});
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
		PrepareShader();

		CreateDescSetLayout();
		CreateDescriptorPool();
		CreateDescSet();

		CreatePipeline();
		CreateShaderBindingTable();
	}

	void SimpleRaytracer::CreateDescriptorPool()
	{
		m_bindingManager.SetPool(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, *m_context.device);
	}

	void SimpleRaytracer::CreateDescSetLayout() {

		m_bindingManager.bindings = {
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

		m_bindingManager.SetLayout(*m_context.device);
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

		m_commandBuffer->bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, *m_pipeline);

		m_commandBuffer->bindDescriptorSets(
			vk::PipelineBindPoint::eRayTracingKHR,
			*m_pipelineLayout,
			0,
			m_bindingManager.descriptorSet,
			nullptr
		);

		m_commandBuffer->traceRaysKHR(
			raygenRegion,
			missRegion,
			hitRegion,
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
		//std::vector<vk::WriteDescriptorSet> writes(2);

		VkHelper::BindingManager::WritingInfo info;
		info.numAS = 1;
		info.numImage = 2;
		info.numBuffer = 8;
		m_bindingManager.StartWriting(info);

		m_bindingManager.WriteAS(
			*m_asManager.TLAS.accel, 0, 1, *m_context.device
		);

		//m_bindingManager.WriteImage(
		//	imageView, vk::ImageLayout::eGeneral, VK_NULL_HANDLE,
		//	vk::DescriptorType::eStorageImage, 1, 1, *m_context.device
		//);

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

#define SHADER_FILE_PATH "shader/"
	void SimpleRaytracer::AddShader(
		uint32_t shaderIndex,
		const std::string& shaderName,
		vk::ShaderStageFlagBits stage
	)
	{
		shaderModules[shaderIndex] = vkutils::createShaderModule(*m_context.device, SHADER_FILE_PATH + shaderName);
		shaderStages[shaderIndex].setStage(stage);
		shaderStages[shaderIndex].setModule(*shaderModules[shaderIndex]);
		shaderStages[shaderIndex].setPName("main");
	}

	void SimpleRaytracer::PrepareShader() {
		SKHOLE_LOG("Prepare Shader");

		uint32_t raygenShader = 0;
		uint32_t missShader = 1;
		uint32_t closestHitShader = 2;

		shaderModules.resize(3);
		shaderStages.resize(3);

		AddShader(raygenShader, "simple_raytracer/raygen.rgen.spv", vk::ShaderStageFlagBits::eRaygenKHR);
		AddShader(missShader, "simple_raytracer/miss.rmiss.spv", vk::ShaderStageFlagBits::eMissKHR);
		AddShader(closestHitShader, "simple_raytracer/closesthit.rchit.spv", vk::ShaderStageFlagBits::eClosestHitKHR);

		uint32_t raygenGroup = 0;
		uint32_t missGroup = 1;
		uint32_t hitGroup = 2;

		shaderGroups.resize(3);

		// Raygen group
		shaderGroups[raygenGroup].setType(
			vk::RayTracingShaderGroupTypeKHR::eGeneral);
		shaderGroups[raygenGroup].setGeneralShader(raygenShader);
		shaderGroups[raygenGroup].setClosestHitShader(VK_SHADER_UNUSED_KHR);
		shaderGroups[raygenGroup].setAnyHitShader(VK_SHADER_UNUSED_KHR);
		shaderGroups[raygenGroup].setIntersectionShader(VK_SHADER_UNUSED_KHR);

		// Miss group
		shaderGroups[missGroup].setType(
			vk::RayTracingShaderGroupTypeKHR::eGeneral);
		shaderGroups[missGroup].setGeneralShader(missShader);
		shaderGroups[missGroup].setClosestHitShader(VK_SHADER_UNUSED_KHR);
		shaderGroups[missGroup].setAnyHitShader(VK_SHADER_UNUSED_KHR);
		shaderGroups[missGroup].setIntersectionShader(VK_SHADER_UNUSED_KHR);

		// Hit group
		shaderGroups[hitGroup].setType(
			vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup);
		shaderGroups[hitGroup].setGeneralShader(VK_SHADER_UNUSED_KHR);
		shaderGroups[hitGroup].setClosestHitShader(closestHitShader);
		shaderGroups[hitGroup].setAnyHitShader(VK_SHADER_UNUSED_KHR);
		shaderGroups[hitGroup].setIntersectionShader(VK_SHADER_UNUSED_KHR);
	}


	void SimpleRaytracer::CreateDescSet() {
		std::cout << "Create desc set\n";

		m_bindingManager.SetDescriptorSet(*m_context.device);

	}

	void SimpleRaytracer::CreatePipeline() {
		std::cout << "Create pipeline\n";

		// Create pipeline layout
		vk::PipelineLayoutCreateInfo layoutCreateInfo{};
		layoutCreateInfo.setSetLayouts(m_bindingManager.descriptorSetLayout);
		m_pipelineLayout = m_context.device->createPipelineLayoutUnique(layoutCreateInfo);

		// Create pipeline
		vk::RayTracingPipelineCreateInfoKHR pipelineCreateInfo{};
		pipelineCreateInfo.setLayout(*m_pipelineLayout);
		pipelineCreateInfo.setStages(shaderStages);
		pipelineCreateInfo.setGroups(shaderGroups);
		pipelineCreateInfo.setMaxPipelineRayRecursionDepth(1);
		auto result = m_context.device->createRayTracingPipelineKHRUnique(
			nullptr, nullptr, pipelineCreateInfo);

		if (result.result != vk::Result::eSuccess) {
			std::cerr << "Failed to create ray tracing pipeline.\n";
			std::abort();
		}

		m_pipeline = std::move(result.value);

		vk::DescriptorSetAllocateInfo descSetAllocInfo{};
		descSetAllocInfo.setDescriptorPool(m_bindingManager.descriptorPool);
		descSetAllocInfo.setSetLayouts(m_bindingManager.descriptorSetLayout);

	}

	void SimpleRaytracer::CreateShaderBindingTable() {
		vk::PhysicalDeviceRayTracingPipelinePropertiesKHR rtProperties =
			vkutils::getRayTracingProps(m_context.physicalDevice);
		uint32_t handleSize = rtProperties.shaderGroupHandleSize;
		uint32_t handleAlignment = rtProperties.shaderGroupHandleAlignment;
		uint32_t baseAlignment = rtProperties.shaderGroupBaseAlignment;
		uint32_t handleSizeAligned =
			vkutils::alignUp(handleSize, handleAlignment);

		// Set strides and sizes
		uint32_t raygenShaderCount = 1;  // raygen count must be 1
		uint32_t missShaderCount = 1;
		uint32_t hitShaderCount = 1;

		raygenRegion.setStride(
			vkutils::alignUp(handleSizeAligned, baseAlignment));
		raygenRegion.setSize(raygenRegion.stride);

		missRegion.setStride(handleSizeAligned);
		missRegion.setSize(vkutils::alignUp(missShaderCount * handleSizeAligned,
			baseAlignment));

		hitRegion.setStride(handleSizeAligned);
		hitRegion.setSize(vkutils::alignUp(hitShaderCount * handleSizeAligned,
			baseAlignment));

		// Create SBT
		vk::DeviceSize sbtSize =
			raygenRegion.size + missRegion.size + hitRegion.size;
		sbt.init(m_context.physicalDevice, *m_context.device, sbtSize,
			vk::BufferUsageFlagBits::eShaderBindingTableKHR |
			vk::BufferUsageFlagBits::eTransferSrc |
			vk::BufferUsageFlagBits::eShaderDeviceAddress,
			vk::MemoryPropertyFlagBits::eHostVisible |
			vk::MemoryPropertyFlagBits::eHostCoherent);

		// Get shader group handles
		uint32_t handleCount =
			raygenShaderCount + missShaderCount + hitShaderCount;
		uint32_t handleStorageSize = handleCount * handleSize;
		std::vector<uint8_t> handleStorage(handleStorageSize);

		auto result = m_context.device->getRayTracingShaderGroupHandlesKHR(
			*m_pipeline, 0, handleCount, handleStorageSize, handleStorage.data());
		if (result != vk::Result::eSuccess) {
			std::cerr << "Failed to get ray tracing shader group handles.\n";
			std::abort();
		}

		// Copy handles
		uint32_t handleIndex = 0;
		uint8_t* sbtHead =
			static_cast<uint8_t*>(m_context.device->mapMemory(*sbt.memory, 0, sbtSize));

		uint8_t* dstPtr = sbtHead;
		auto copyHandle = [&](uint32_t index) {
			std::memcpy(dstPtr, handleStorage.data() + handleSize * index,
				handleSize);
			};

		// Raygen
		copyHandle(handleIndex++);

		// Miss
		dstPtr = sbtHead + raygenRegion.size;
		for (uint32_t c = 0; c < missShaderCount; c++) {
			copyHandle(handleIndex++);
			dstPtr += missRegion.stride;
		}

		// Hit
		dstPtr = sbtHead + raygenRegion.size + missRegion.size;
		for (uint32_t c = 0; c < hitShaderCount; c++) {
			copyHandle(handleIndex++);
			dstPtr += hitRegion.stride;
		}

		raygenRegion.setDeviceAddress(sbt.address);
		missRegion.setDeviceAddress(sbt.address + raygenRegion.size);
		hitRegion.setDeviceAddress(sbt.address + raygenRegion.size +
			missRegion.size);
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
