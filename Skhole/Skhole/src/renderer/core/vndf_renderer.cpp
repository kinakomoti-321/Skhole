#include <renderer/core/vndf_renderer.h>
namespace Skhole {

	VNDF_Renderer::VNDF_Renderer()
	{

	}

	VNDF_Renderer::~VNDF_Renderer()
	{

	}

	void VNDF_Renderer::ResizeCore(unsigned int width, unsigned int height)
	{

	}

	void VNDF_Renderer::InitializeCore(const RendererDesc& desc)
	{
		SKHOLE_LOG_SECTION("Initialze Renderer");

		m_uniformBuffer.Init(m_context.physicalDevice, *m_context.device);

		auto& uniformBufferObject = m_uniformBuffer.data;
		uniformBufferObject.frame = 0;
		uniformBufferObject.maxSPP = 100;
		uniformBufferObject.width = desc.Width;
		uniformBufferObject.height = desc.Height;
		uniformBufferObject.cameraDir = vec3(0.0f, 0.0f, -1.0f);
		uniformBufferObject.cameraPos = vec3(0.0, 30.0, 50.0);
		uniformBufferObject.cameraUp = vec3(0.0f, 1.0f, 0.0f);
		uniformBufferObject.cameraRight = vec3(1.0f, 0.0f, 0.0f);

		m_uniformBuffer.Update(*m_context.device);

		SKHOLE_LOG_SECTION("Initialze Renderer Completed");
	}

	void VNDF_Renderer::DestroyScene()
	{
		m_sceneBufferManager.Release(*m_context.device);
		m_asManager.ReleaseTLAS(*m_context.device);
		m_asManager.ReleaseBLAS(*m_context.device);

		m_materialBuffer.Release(*m_context.device);

		m_scene = nullptr;
	}

	void VNDF_Renderer::DestroyCore()
	{
		m_uniformBuffer.Release(*m_context.device);
		m_materialBuffer.Release(*m_context.device);

		m_asManager.ReleaseBLAS(*m_context.device);
		m_asManager.ReleaseTLAS(*m_context.device);
		m_sceneBufferManager.Release(*m_context.device);
	}


	void VNDF_Renderer::SetScene(ShrPtr<Scene> scene) {
		SKHOLE_LOG_SECTION("Set Scene");
		m_scene = scene;
		m_sceneBufferManager.SetScene(m_scene);
		m_sceneBufferManager.InitGeometryBuffer(m_context.physicalDevice, *m_context.device, *m_commandPool, m_context.queue);
		m_sceneBufferManager.InitInstanceBuffer(m_context.physicalDevice, *m_context.device, *m_commandPool, m_context.queue);

		m_asManager.BuildBLAS(m_sceneBufferManager, m_context.physicalDevice, *m_context.device, *m_commandPool, m_context.queue);

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

	void VNDF_Renderer::UpdateScene(const UpdataInfo& updateInfo) {
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


	void VNDF_Renderer::InitFrameGUI() {
		m_imGuiManager.NewFrame();
	}

	void VNDF_Renderer::FrameStart(float time)
	{
		m_scene->SetTransformMatrix(time);
		m_sceneBufferManager.FrameUpdateInstance(time, *m_context.device, *m_commandPool, m_context.queue);
		m_asManager.BuildTLAS(m_sceneBufferManager, m_context.physicalDevice, *m_context.device, *m_commandPool, m_context.queue);

		auto& raytracerParam = m_scene->m_rendererParameter;

		uint32_t width = m_renderImages.GetWidth();
		uint32_t height = m_renderImages.GetHeight();

		auto& uniformBufferObject = m_uniformBuffer.data;
		uniformBufferObject.maxSPP = raytracerParam->maxSPP;
		uniformBufferObject.frame = raytracerParam->frame;
		uniformBufferObject.numSPP = raytracerParam->numSPP;
		uniformBufferObject.samplePerFrame = raytracerParam->sppPerFrame;
		uniformBufferObject.mode = GetParamUintValue(raytracerParam->rendererParameters[0]);

		auto& camera = m_scene->m_camera;
		vec3 cameraDir, cameraUp, cameraRight;
		camera->GetCameraDirections(time, cameraDir, cameraUp, cameraRight);

		uniformBufferObject.cameraPos = camera->GetCameraPosition(time);
		uniformBufferObject.cameraDir = cameraDir;
		uniformBufferObject.cameraUp = cameraUp;
		uniformBufferObject.cameraRight = cameraRight;
		uniformBufferObject.cameraParam.v[0] = camera->GetYFov();
		uniformBufferObject.cameraParam.v[1] = static_cast<float>(width) / static_cast<float>(height);

		uniformBufferObject.lightPosition = GetParamVecValue(raytracerParam->rendererParameters[1]);
		uniformBufferObject.lightColor = GetParamColValue(raytracerParam->rendererParameters[2]);
		uniformBufferObject.lightIntensity = GetParamFloatValue(raytracerParam->rendererParameters[3]);

		m_uniformBuffer.Update(*m_context.device);
	}

	void VNDF_Renderer::FrameEnd()
	{
		m_asManager.ReleaseTLAS(*m_context.device);

		auto& raytracerParam = m_scene->m_rendererParameter;
		raytracerParam->numSPP += m_scene->m_rendererParameter->sppPerFrame;
		if (raytracerParam->numSPP >= raytracerParam->maxSPP) {
			raytracerParam->numSPP = raytracerParam->maxSPP;
		}
	}

	void VNDF_Renderer::RealTimeRender(const RealTimeRenderingInfo& renderInfo)
	{
		FrameStart(renderInfo.time);

		vk::UniqueSemaphore imageAvailableSemaphore =
			m_context.device->createSemaphoreUnique({});

		uint32_t imageIndex = m_screenContext.GetFrameIndex(*m_context.device, *imageAvailableSemaphore);

		uint32_t width = m_renderImages.GetWidth();
		uint32_t height = m_renderImages.GetHeight();

		UpdateDescriptorSet(m_renderImages);

		m_commandBuffer->begin(vk::CommandBufferBeginInfo{});

		RecordCommandBuffer(width, height);
		CopyRenderToScreen(*m_commandBuffer, m_renderImages.GetPostProcessedImage().GetImage(), m_screenContext.GetFrameImage(imageIndex), width, height);
		RenderImGuiCommand(*m_commandBuffer, m_screenContext.GetFrameBuffer(imageIndex), width, height);

		if (renderInfo.isScreenShot) {
			m_renderImages.ReadBack(*m_commandBuffer, *m_context.device);
		}

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

		if (renderInfo.isScreenShot) {
			std::string time = GethCurrentTimeString();
			m_renderImages.WritePNG(renderInfo.filepath, renderInfo.filename + time, *m_context.device, *m_commandPool, m_context.queue);
		}

		FrameEnd();
	}

	void VNDF_Renderer::OfflineRender(const OfflineRenderingInfo& renderInfo)
	{
		uint32_t width = renderInfo.width;
		uint32_t height = renderInfo.height;
		uint32_t numFrame = renderInfo.endFrame - renderInfo.startFrame + 1;

		auto& fps = renderInfo.fps;

		RenderImages offlineRenderImages;
		offlineRenderImages.Initialize(width, height, *m_context.device, m_context.physicalDevice, *m_commandPool, m_context.queue);

		m_postProcessor->Resize(width, height);

		std::cout << "Start Offline Rendering" << std::endl;

		//TODO: Implement limit time
		for (int i = 0; i < numFrame; i++)
		{
			std::cout << "Start Frame : " << i << std::endl;
			m_commandBuffer->reset({});

			auto postEffectBuffer = vkutils::createCommandBuffer(*m_context.device, *m_commandPool);
			auto drawSemaphore = m_context.device->createSemaphoreUnique({});

			uint32_t nowFrame = renderInfo.startFrame + i;
			float time = static_cast<float>(nowFrame) / static_cast<float>(fps);

			std::cout << "Buffer Update" << std::endl;
			{
				m_scene->SetTransformMatrix(time);
				m_sceneBufferManager.FrameUpdateInstance(time, *m_context.device, *m_commandPool, m_context.queue);
				m_asManager.BuildTLAS(m_sceneBufferManager, m_context.physicalDevice, *m_context.device, *m_commandPool, m_context.queue);

				auto& raytracerParam = m_scene->m_rendererParameter;

				uint32_t width = m_renderImages.GetWidth();
				uint32_t height = m_renderImages.GetHeight();

				auto& uniformBufferObject = m_uniformBuffer.data;
				uniformBufferObject.maxSPP = renderInfo.spp;
				uniformBufferObject.frame = nowFrame;
				uniformBufferObject.numSPP = 0;
				uniformBufferObject.samplePerFrame = renderInfo.spp;
				uniformBufferObject.resetFrag = 1;
				uniformBufferObject.mode = GetParamUintValue(raytracerParam->rendererParameters[0]);

				auto& camera = m_scene->m_camera;
				vec3 cameraDir, cameraUp, cameraRight;
				camera->GetCameraDirections(time, cameraDir, cameraUp, cameraRight);

				uniformBufferObject.cameraPos = camera->GetCameraPosition(time);
				uniformBufferObject.cameraDir = cameraDir;
				uniformBufferObject.cameraUp = cameraUp;
				uniformBufferObject.cameraRight = cameraRight;
				uniformBufferObject.cameraParam.v[0] = camera->GetYFov();
				uniformBufferObject.cameraParam.v[1] = static_cast<float>(width) / static_cast<float>(height);

				m_uniformBuffer.Update(*m_context.device);

			}

			UpdateDescriptorSet(offlineRenderImages);

			m_commandBuffer->begin(vk::CommandBufferBeginInfo{});
			RaytracingCommand(*m_commandBuffer, width, height);
			m_commandBuffer->end();

			postEffectBuffer->begin(vk::CommandBufferBeginInfo{});
			PostProcessor::ExecuteDesc desc{};
			desc.device = *m_context.device;
			desc.inputImage = offlineRenderImages.GetRenderImage().GetImageView();
			desc.outputImage = offlineRenderImages.GetPostProcessedImage().GetImageView();
			desc.param = m_scene->m_rendererParameter->posproParameters;

			m_postProcessor->Execute(*postEffectBuffer, desc);
			postEffectBuffer->end();

			vk::UniqueFence fence = m_context.device->createFenceUnique({});
			vk::SubmitInfo drawInfo{};
			drawInfo.setCommandBuffers(*m_commandBuffer);
			//drawInfo.setSignalSemaphoreCount(1);
			//drawInfo.setSignalSemaphores(*drawSemaphore);

			m_context.queue.submit(drawInfo);
			m_context.queue.waitIdle();

			vk::PipelineStageFlags waitStages = vk::PipelineStageFlagBits::eNone;
			vk::SubmitInfo postInfo{};
			postInfo.setCommandBuffers(*postEffectBuffer);
			//postInfo.setWaitSemaphores(*drawSemaphore);
			//postInfo.setWaitDstStageMask(waitStages);

			m_context.queue.submit(postInfo, fence.get());
			m_context.queue.waitIdle();

			//if (m_context.device->waitForFences(fence.get(), true,
			//	std::numeric_limits<uint64_t>::max()) != vk::Result::eSuccess) {
			//	std::cerr << "Failed to wait fence.\n";
			//	std::abort();
			//}

			std::string frameNumber = NumbertToSerial(i, 3);
			offlineRenderImages.WritePNG(renderInfo.filepath, renderInfo.filename + frameNumber, *m_context.device, *m_commandPool, m_context.queue);

			FrameEnd();
			std::cout << "End Frame" << std::endl;
		}

		std::cout << "End Offline Rendering" << std::endl;
	}

}
