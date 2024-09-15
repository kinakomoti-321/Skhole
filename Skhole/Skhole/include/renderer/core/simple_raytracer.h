#pragma once

#include <renderer/renderer.h>
#include <renderer/common/raytracing_pipeline.h>
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

		void Init(RendererDesc& desc) override;

		void Destroy() override;

		void Resize(unsigned int width, unsigned int height) override;

		void SetScene(ShrPtr<Scene> scene) override;
		void DestroyScene() override;

		//RendererData GetRendererData() override;
		ShrPtr<RendererDefinisionMaterial> GetMaterial(const ShrPtr<BasicMaterial>& material) override;
		ShrPtr<RendererDefinisionCamera> GetCamera(const ShrPtr<RendererDefinisionCamera>& basicCamera) override;
		ShrPtr<RendererParameter> GetRendererParameter() override;

		void InitFrameGUI() override;

		void UpdateScene(const UpdataInfo& command) override;

		void RealTimeRender(const RealTimeRenderingInfo& renderInfo) override;
		void OfflineRender(const OfflineRenderingInfo& renderInfo) override;

	private:
		//--------------------------------------
		// Renderer Structures
		//--------------------------------------
		struct UniformBufferObject {
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
		void InitImGui();

		// Resources
		void InitBufferManager();
		void InitAccelerationStructures();

		// Pipeline
		void CreateRaytracingPipeline();

		// Render
		void UpdateDescriptorSet(vk::ImageView imageView);

		void FrameStart(float time);
		void FrameEnd();

		void RecordCommandBuffer(vk::Image image, vk::Framebuffer frameBuffer);

		// Scene Update;
		void UpdateMaterialBuffer(uint32_t matId);
		Material ConvertMaterial(const ShrPtr<RendererDefinisionMaterial>& material);

		void SetPostprocess(PostProcessType type);
		void DestroyPostprocess();

	private:
		//--------------------------------------
		// Renderer Data
		//--------------------------------------
		std::string rendererName = "Simple RayTracer";
		RendererDesc m_desc;

		SimpleRaytracerParameter m_raytracerParameter;

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

		PostProcessParameter m_postprocessParams;


		//--------------------------------------
		// Vulkan
		//--------------------------------------
		VkHelper::Context m_context;
		VkHelper::SwapchainContext m_swapchainContext;

		vk::UniqueCommandPool m_commandPool;
		vk::UniqueCommandBuffer m_commandBuffer;

		VkHelper::VulkanImGuiManager m_imGuiManager;
		vk::UniqueRenderPass m_imGuiRenderPass;

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

		VkHelper::BindingManager m_bindingManager;
		RaytracingPipeline m_raytracingPipeline;

		UniformBufferObject uniformBufferObject;
		Buffer m_uniformBuffer;

		//--------------------------------------	
		// Buffers
		//--------------------------------------	
		SceneBufferaManager m_sceneBufferManager;
		ASManager m_asManager;

		std::vector<Material> m_materials;
		DeviceBuffer m_materaialBuffer;

		Image accumImage;
		Image renderImage;
		Image posproIamge;


		//--------------------------------------
		// Post Process
		//--------------------------------------
		ShrPtr<PostProcessor> m_postProcessor;

		//--------------------------------------
		// Scene
		//--------------------------------------
		ShrPtr<Scene> m_scene;
	};
}
