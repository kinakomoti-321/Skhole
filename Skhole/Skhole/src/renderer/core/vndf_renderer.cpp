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

	void RealTimeRender(const RealTimeRenderingInfo& renderInfo)
	{
		//m_raytracerParameter.spp++;

		//FrameStart(renderInfo.time);

		//vk::UniqueSemaphore imageAvailableSemaphore =
		//	m_context.device->createSemaphoreUnique({});


		//uint32_t imageIndex = m_screenContext.GetFrameIndex(*m_context.device, *imageAvailableSemaphore);

		//uint32_t width = m_renderImages.GetWidth();
		//uint32_t height = m_renderImages.GetHeight();

		//UpdateDescriptorSet();

		//m_commandBuffer->begin(vk::CommandBufferBeginInfo{});

		//RecordCommandBuffer(width, height);
		//CopyRenderToScreen(*m_commandBuffer, m_renderImages.GetPostProcessedImage().GetImage(), m_screenContext.GetFrameImage(imageIndex), width, height);
		//RenderImGuiCommand(*m_commandBuffer, m_screenContext.GetFrameBuffer(imageIndex), width, height);

		//m_commandBuffer->end();

		//vk::PipelineStageFlags waitStage{ vk::PipelineStageFlagBits::eTopOfPipe };
		//vk::SubmitInfo submitInfo{};
		//submitInfo.setWaitDstStageMask(waitStage);
		//submitInfo.setCommandBuffers(*m_commandBuffer);
		//submitInfo.setWaitSemaphores(*imageAvailableSemaphore);
		//m_context.queue.submit(submitInfo);

		//m_context.queue.waitIdle();

		//vk::PresentInfoKHR presentInfo{};
		//presentInfo.setSwapchains(*m_screenContext.swapchain);
		//presentInfo.setImageIndices(imageIndex);
		//if (m_context.queue.presentKHR(presentInfo) != vk::Result::eSuccess) {
		//	std::cerr << "Failed to present.\n";
		//	std::abort();
		//}

		//FrameEnd();
	}

	void OfflineRender(const OfflineRenderingInfo& renderInfo) {
		SKHOLE_UNIMPL();
	}



}
