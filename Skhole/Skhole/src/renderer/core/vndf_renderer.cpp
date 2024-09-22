#include <renderer/core/vndf_renderer.h>
namespace Skhole {
	void VNDF_Renderer::InitializeCore(RendererDesc& desc) {
		auto& device = *m_context.device;
		auto& physicalDevice = m_context.physicalDevice;

		m_uniformBuffer.Init(physicalDevice, device);
	}

	void VNDF_Renderer::ResizeCore(unsigned int width, unsigned int height)
	{

	}

	void VNDF_Renderer::DestroyCore()
	{
		auto& device = *m_context.device;
		m_uniformBuffer.Release(device);
		m_materialBuffer.Release(device);
		m_asManager.ReleaseBLAS(device);
		m_asManager.ReleaseTLAS(device);
		m_sceneBufferManager.Release(device);
	}

	void VNDF_Renderer::SetScene(ShrPtr<Scene> scene)
	{
		auto& device = *m_context.device;
		auto& physicalDevice = m_context.physicalDevice;
		auto& commandPool = *m_commandPool;
		auto& queue = m_context.queue;

		m_scene = scene;
		m_sceneBufferManager.SetScene(scene);
		m_sceneBufferManager.InitGeometryBuffer(physicalDevice, device, commandPool, queue);
		m_sceneBufferManager.InitInstanceBuffer(physicalDevice, device, commandPool, queue);

		m_asManager.BuildBLAS(m_sceneBufferManager, physicalDevice, device, commandPool, queue);

		{
			m_materialBuffer.Init(
				physicalDevice, device,
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
	}

	void VNDF_Renderer::InitFrameGUI() {
		m_imGuiManager.NewFrame();
	}

	void VNDF_Renderer::DestroyScene() {
		m_sceneBufferManager.Release(*m_context.device);
		m_asManager.ReleaseTLAS(*m_context.device);
		m_asManager.ReleaseBLAS(*m_context.device);

		m_materialBuffer.Release(*m_context.device);

		m_scene = nullptr;
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

	void VNDF_Renderer::RealTimeRender(const RealTimeRenderingInfo& renderInfo)
	{
		//SKHOLE_LOG_SECTION("Render : ");
		//FrameStart(renderInfo.time);
		auto time = renderInfo.time;
		{
			auto& raytracerParam = m_scene->m_rendererParameter;

			uint32_t width = m_renderImages.GetWidth();
			uint32_t height = m_renderImages.GetHeight();

			auto& uniformBufferObject = m_uniformBuffer.data;

			auto& camera = m_scene->m_camera;
			uniformBufferObject.spp = raytracerParam->spp;
			uniformBufferObject.frame = raytracerParam->frame;
			uniformBufferObject.sample = raytracerParam->sample;
			uniformBufferObject.mode = 0;

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

		m_asManager.ReleaseTLAS(*m_context.device);

		auto& raytracerParam = m_scene->m_rendererParameter;
		raytracerParam->sample++;
		if (raytracerParam->sample >= raytracerParam->spp) {
			raytracerParam->sample = raytracerParam->spp;
		}
		//FrameEnd();
	}

	void VNDF_Renderer::OfflineRender(const OfflineRenderingInfo& renderInfo) {
		SKHOLE_UNIMPL();
	}



}
