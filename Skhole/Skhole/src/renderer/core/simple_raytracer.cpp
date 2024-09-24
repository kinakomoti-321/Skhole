#include <renderer/core/simple_raytracer.h>
#include <common/log.h>
#include <vulkan_helpler/vkutils.hpp>

namespace Skhole {

	SimpleRaytracer::SimpleRaytracer()
	{

	}

	SimpleRaytracer::~SimpleRaytracer()
	{

	}

	void SimpleRaytracer::ResizeCore(unsigned int width, unsigned int height)
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

	void SimpleRaytracer::DestroyCore()
	{
		m_uniformBuffer.Release(*m_context.device);
		m_materialBuffer.Release(*m_context.device);

		m_asManager.ReleaseBLAS(*m_context.device);
		m_asManager.ReleaseTLAS(*m_context.device);
		m_sceneBufferManager.Release(*m_context.device);
	}


	void SimpleRaytracer::SetScene(ShrPtr<Scene> scene) {
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


	void SimpleRaytracer::InitFrameGUI() {
		m_imGuiManager.NewFrame();
	}

	void SimpleRaytracer::FrameStart(float time)
	{
		auto& raytracerParam = m_scene->m_rendererParameter;

		uint32_t width = m_renderImages.GetWidth();
		uint32_t height = m_renderImages.GetHeight();

		auto& uniformBufferObject = m_uniformBuffer.data;

		auto& camera = m_scene->m_camera;
		uniformBufferObject.spp = raytracerParam->maxSPP;
		uniformBufferObject.frame = raytracerParam->frame;
		uniformBufferObject.sample = raytracerParam->numSPP;
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
		raytracerParam->numSPP++;
		if (raytracerParam->numSPP >= raytracerParam->maxSPP) {
			raytracerParam->numSPP = raytracerParam->maxSPP;
		}
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

	void SimpleRaytracer::OfflineRender(const OfflineRenderingInfo& renderInfo)
	{
		SKHOLE_UNIMPL("Offline Render");
	}

}
