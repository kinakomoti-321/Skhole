#pragma once

#include <renderer/renderer.h>

#include <renderer/common/buffer_manager.h>

#include <vulkan_helpler/vkutils.hpp>
#include <vulkan_helpler/vk_buffer.h>
#include <vulkan_helpler/vk_hepler.h>
#include <vulkan_helpler/vk_imgui.h>

#include <post_process/post_processor.h>
#include <post_process/post_processor_interface.h>

#include <common/util.h>

#include <scene/scene.h>

namespace Skhole
{
	class SimpleRaytracer : public Renderer
	{
	public:
		//--------------------------------------
		// Interface Method
		//--------------------------------------
		SimpleRaytracer();
		~SimpleRaytracer();

		void InitializeCore(RendererDesc& desc) override;
		void ResizeCore(unsigned int width, unsigned int height) override;
		void DestroyCore() override;

		void SetScene(ShrPtr<Scene> scene) override;
		void DestroyScene() override;

		ShrPtr<RendererDefinisionMaterial> GetMaterial(const ShrPtr<BasicMaterial>& material) override;
		ShrPtr<RendererDefinisionCamera> GetCamera(const ShrPtr<RendererDefinisionCamera>& basicCamera) override;
		ShrPtr<RendererParameter> GetRendererParameter() override;
		std::vector<const char*> GetLayer() override {
			return m_layer;
		}
		std::vector<const char*> GetExtensions() override {
			return m_extension;
		}

		void InitFrameGUI() override;

		void UpdateScene(const UpdataInfo& command) override;

		void RealTimeRender(const RealTimeRenderingInfo& renderInfo) override;
		void OfflineRender(const OfflineRenderingInfo& renderInfo) override;

	private:
		//--------------------------------------
		// Renderer Structures
		//--------------------------------------
		struct Uniform {
			uint32_t spp;
			uint32_t frame;
			uint32_t sample;
			uint32_t mode;

			uint32_t width;
			uint32_t height;

			uint32_t padding[2];

			vec3 cameraPos;
			vec3 cameraDir;
			vec3 cameraUp;
			vec3 cameraRight;
			vec4 cameraParam;
		};

		struct SimpleRaytracerParameter {
			uint32_t spp = 1;
			uint32_t sample = 0;
			uint32_t frame = 0;

			uint32_t checkMode = 0;
		};

		struct Material {
			vec4 baseColor;
			float roughness;
			float metallic;

			float emissionIntesity = 1.0;
			float padding = 0.0;

			vec4 emissionColor;
		};

	private:
		//--------------------------------------
		// Private Method
		//--------------------------------------
		// Resources
		void InitBufferManager();
		void InitAccelerationStructures();

		// Render
		void UpdateDescriptorSet();

		void FrameStart(float time);
		void FrameEnd();

		void RecordCommandBuffer(uint32_t width, uint32_t height);

		// Scene Update;
		void UpdateMaterialBuffer(uint32_t matId);
		Material ConvertMaterial(const ShrPtr<RendererDefinisionMaterial>& material);


		void InitializeBiniding() override;

	private:
		//--------------------------------------
		// Renderer Data
		//--------------------------------------
		std::string rendererName = "Simple RayTracer";

		SimpleRaytracerParameter m_raytracerParameter;
		PostProcessParameter m_postprocessParams;

		const std::vector<ShrPtr<Parameter>> m_matParams =
		{
			MakeShr<ParamCol>("BaseColor", vec4(0.8f)),
			MakeShr<ParamFloat>("Roughness", 0.0f),
			MakeShr<ParamFloat>("Metallic", 0.0f),

			MakeShr<ParamFloat>("EmissionIntensity", 0.0f),
			MakeShr<ParamCol>("Emission", vec4(0.0)),
		};

		const std::vector<ShrPtr<Parameter>> m_camExtensionParams =
		{
			MakeShr<ParamVec>("LookPoint",vec3(0.0f)),
		};

		const std::vector<ShrPtr<Parameter>> m_rendererExtensionParams =
		{
			MakeShr<ParamUint>("Ckeck Mode",0),
		};

		//--------------------------------------
		// Vulkan
		//--------------------------------------
		std::vector<const char*> m_layer = {
			"VK_LAYER_KHRONOS_validation"
		};

		std::vector<const char* > m_extension = {
			// For swapchain
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,

			// For ray tracing
			VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
			VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
			VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
			VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
			VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,

			// For GLSL
			VK_KHR_RELAXED_BLOCK_LAYOUT_EXTENSION_NAME,
			VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME,
		};

		//--------------------------------------	
		// Buffers
		//--------------------------------------	
		SceneBufferaManager m_sceneBufferManager;
		ASManager m_asManager;

		MaterialBuffer<Material> m_materialBuffer;
		UniformBuffer<Uniform> m_uniformBuffer;
	};
}
