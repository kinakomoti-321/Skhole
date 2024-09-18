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


}
