#include <renderer/core/simple_raytracer.h>
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

	void SimpleRaytracer::InitializeCore(RendererDesc& desc)
	{
		SKHOLE_LOG_SECTION("Initialze Renderer");

		m_uniformBuffer.Init(m_context.physicalDevice, *m_context.device);

		auto& uniformBufferObject = m_uniformBuffer.data;
		uniformBufferObject.frame = 0;
		uniformBufferObject.spp = 100;
		uniformBufferObject.width = desc.Width;
		uniformBufferObject.height = desc.Height;
		uniformBufferObject.cameraDir = vec3(0.0f, 0.0f, -1.0f);
		uniformBufferObject.cameraPos = vec3(0.0, 30.0, 50.0);
		uniformBufferObject.cameraUp = vec3(0.0f, 1.0f, 0.0f);
		uniformBufferObject.cameraRight = vec3(1.0f, 0.0f, 0.0f);

		m_uniformBuffer.Update(*m_context.device);

		m_renderImages.Initialize(desc.Width, desc.Height, *m_context.device, m_context.physicalDevice, *m_commandPool, m_context.queue);
		SKHOLE_LOG_SECTION("Initialze Renderer Completed");
	}

	void SimpleRaytracer::DestroyScene()
	{
		m_sceneBufferManager.Release(*m_context.device);
		m_asManager.ReleaseTLAS(*m_context.device);
		m_asManager.ReleaseBLAS(*m_context.device);

		m_materialBuffer.Release(*m_context.device);

		m_scene = nullptr;
	}


	void SimpleRaytracer::ResizeCore(unsigned int width, unsigned int height)
	{

	}

	void SimpleRaytracer::DestroyCore()
	{
		m_uniformBuffer.Release(*m_context.device);
		m_materialBuffer.Release(*m_context.device);

		m_asManager.ReleaseBLAS(*m_context.device);
		m_asManager.ReleaseTLAS(*m_context.device);
		m_sceneBufferManager.Release(*m_context.device);
	}

	void SimpleRaytracer::RealTimeRender(const RealTimeRenderingInfo& renderInfo)
	{
		FrameStart(renderInfo.time);

		vk::UniqueSemaphore imageAvailableSemaphore =
			m_context.device->createSemaphoreUnique({});


		uint32_t imageIndex = m_screenContext.GetFrameIndex(*m_context.device, *imageAvailableSemaphore);

		uint32_t width = m_renderImages.GetWidth();
		uint32_t height = m_renderImages.GetHeight();

		UpdateDescriptorSet();

		m_commandBuffer->begin(vk::CommandBufferBeginInfo{});

		RecordCommandBuffer(width, height);
		CopyRenderToScreen(*m_commandBuffer, m_renderImages.GetPostProcessedImage().GetImage(), m_screenContext.GetFrameImage(imageIndex), width, height);
		RenderImGuiCommand(*m_commandBuffer, m_screenContext.GetFrameBuffer(imageIndex), width, height);

		m_commandBuffer->end();

		vk::PipelineStageFlags waitStage{ vk::PipelineStageFlagBits::eTopOfPipe };
		vk::SubmitInfo submitInfo{};
		submitInfo.setWaitDstStageMask(waitStage);
		submitInfo.setCommandBuffers(*m_commandBuffer);
		submitInfo.setWaitSemaphores(*imageAvailableSemaphore);
		m_context.queue.submit(submitInfo);

		m_context.queue.waitIdle();

		vk::PresentInfoKHR presentInfo{};
		presentInfo.setSwapchains(*m_screenContext.swapchain);
		presentInfo.setImageIndices(imageIndex);
		if (m_context.queue.presentKHR(presentInfo) != vk::Result::eSuccess) {
			std::cerr << "Failed to present.\n";
			std::abort();
		}

		FrameEnd();
	}

	void SimpleRaytracer::RecordCommandBuffer(uint32_t width, uint32_t height) {

		RaytracingCommand(*m_commandBuffer, width, height);

		// Post Process
		PostProcessor::ExecuteDesc desc{};
		desc.device = *m_context.device;
		desc.inputImage = m_renderImages.GetRenderImage().GetImageView();
		desc.outputImage = m_renderImages.GetPostProcessedImage().GetImageView();
		desc.param = m_scene->m_rendererParameter->posproParameters;

		m_postProcessor->Execute(*m_commandBuffer, desc);
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
			m_materialBuffer.Init(
				m_context.physicalDevice, *m_context.device,
				m_scene->m_materials.size()
			);

			auto& materials = m_scene->m_materials;
			int index = 0;
			for (auto& materialDef : materials)
			{
				auto material = ConvertMaterial(materialDef);
				m_materialBuffer.SetMaterial(material, index);
				index++;
			}

			m_materialBuffer.UpdateBuffer(*m_context.device, *m_commandPool, m_context.queue);
		}

		SKHOLE_LOG_SECTION("End Set Scene");
	}

	void SimpleRaytracer::InitBufferManager() {
		SKHOLE_LOG("Init Buffer Manager");

		m_sceneBufferManager.SetScene(m_scene);

		m_sceneBufferManager.InitGeometryBuffer(m_context.physicalDevice, *m_context.device, *m_commandPool, m_context.queue);
		m_sceneBufferManager.InitInstanceBuffer(m_context.physicalDevice, *m_context.device, *m_commandPool, m_context.queue);
	}

	void SimpleRaytracer::UpdateScene(const UpdataInfo& updateInfo) {
		if (updateInfo.commands.size() == 0) return;

		ResetSample();

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

	void SimpleRaytracer::DefineMaterial(ShrPtr<RendererDefinisionMaterial>& materialDef, const ShrPtr<BasicMaterial>& material)
	{
		CopyParameter(m_matParams, materialDef->materialParameters);

		materialDef->materialParameters[0]->setParamValue(material->basecolor);
		materialDef->materialParameters[1]->setParamValue(material->metallic);
		materialDef->materialParameters[2]->setParamValue(material->roughness);
		materialDef->materialParameters[3]->setParamValue(material->emissionIntensity);
		materialDef->materialParameters[4]->setParamValue(material->emissionColor);
	}

	void SimpleRaytracer::DefineCamera(const ShrPtr<RendererDefinisionCamera>& cameraDef)
	{
		CopyParameter(m_camExtensionParams, cameraDef->extensionParameters);
	}

	ShrPtr<RendererParameter> SimpleRaytracer::GetRendererParameter() {

		ShrPtr<RendererParameter> rendererParameter = MakeShr<RendererParameter>();
		rendererParameter->rendererName = "Simple Raytracer";
		rendererParameter->frame = 0;
		rendererParameter->spp = 100;
		rendererParameter->sample = 1;

		CopyParameter(m_rendererExtensionParams, rendererParameter->rendererParameters);
		rendererParameter->posproParameters = m_postProcessor->GetParamter();

		return rendererParameter;
	}

	void SimpleRaytracer::InitializeBiniding()
	{
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
	}

	void SimpleRaytracer::InitFrameGUI() {
		m_imGuiManager.NewFrame();
	}

	void SimpleRaytracer::FrameStart(float time) {

		auto& raytracerParam = m_scene->m_rendererParameter;

		uint32_t width = m_renderImages.GetWidth();
		uint32_t height = m_renderImages.GetHeight();

		auto& uniformBufferObject = m_uniformBuffer.data;

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
		uniformBufferObject.cameraParam.y = static_cast<float>(width) / static_cast<float>(height);

		m_uniformBuffer.Update(*m_context.device);

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




	void SimpleRaytracer::UpdateDescriptorSet() {
		auto& accumImage = m_renderImages.GetAccumImage();
		auto& renderImage = m_renderImages.GetRenderImage();
		auto& posproIamge = m_renderImages.GetPostProcessedImage();

		m_bindingManager.StartWriting();

		m_bindingManager.WriteAS(
			*m_asManager.TLAS.accel, 0, 1, *m_context.device
		);

		m_bindingManager.WriteImage(
			renderImage.GetImageView(), vk::ImageLayout::eGeneral, VK_NULL_HANDLE,
			vk::DescriptorType::eStorageImage, 1, 1, *m_context.device
		);

		m_bindingManager.WriteBuffer(
			m_uniformBuffer.GetBuffer(), 0, m_uniformBuffer.GetBufferSize(),
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
			m_materialBuffer.GetBuffer(), 0, m_materialBuffer.GetBufferSize(),
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

	void SimpleRaytracer::UpdateMaterialBuffer(uint32_t matId)
	{
		auto material = ConvertMaterial(m_scene->m_materials[matId]);
		m_materialBuffer.SetMaterial(material, matId);

		m_materialBuffer.UpdateBufferIndex(matId, *m_context.device, *m_commandPool, m_context.queue);
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

	ShrPtr<RendererParameter> GetRendererParameter()
	{
		ShrPtr<RendererParameter> rendererParameter = MakeShr<RendererParameter>();
		rendererParameter->rendererName = "Simple Raytracer";
		rendererParameter->frame = 0;
		rendererParameter->spp = 100;
		rendererParameter->sample = 1;
		return nullptr;
	}

}
