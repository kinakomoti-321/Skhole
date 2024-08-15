#pragma once

#include <renderer/renderer.h>
#include <vulkan_helpler/vkutils.hpp>
#include <vulkan_helpler/vk_buffer.h>
#include <vulkan_helpler/vk_hepler.h>
#include <vulkan_helpler/vk_imgui.h>

#include <scene/scene.h>

namespace Skhole
{
	struct Vertex {
		float pos[3];
	};


	class SimpleRaytracer : public Renderer
	{
	public:
		SimpleRaytracer();
		~SimpleRaytracer();

		void Init(RendererDesc& desc) override;
		void Destroy() override;

		void Resize(unsigned int width, unsigned int height) override;
		void Render() override;

		void Update() override;

		void SetNewFrame() override;
		void Wait() override;

		void OffscreenRender() override;
		RendererData GetRendererData() override;
		ShrPtr<RendererDefinisionMaterial> GetMaterialDefinision() override;
		ShrPtr<RendererDefinisionMaterial> GetMaterial(const ShrPtr<BasicMaterial>& material) override;

	private:


		std::string rendererName = "Simple RayTracer";
		//MaterialParameterFormat materialFormat;
		RendererDesc m_desc;

		void InitVulkan();

		void PrepareShader();
		void AddShader(uint32_t shaderIndex, const std::string& shaderName, vk::ShaderStageFlagBits stage);
		void CreateBottomLevelAS();
		void CreateTopLevelAS();
		void CreateDescriptorPool();
		void CreateDescSetLayout();
		void CreateDescSet();
		void CreatePipeline();
		void CreateShaderBindingTable();

		void UpdateDescriptorSet(vk::ImageView imageView);
		void RecordCommandBuffer(vk::Image image, vk::Framebuffer frameBuffer);

		vk::UniqueInstance m_instance;
		vk::UniqueDebugUtilsMessengerEXT m_debugMessenger;
		vk::UniqueSurfaceKHR m_surface;

		vk::PhysicalDevice m_physicalDevice;
		vk::UniqueDevice m_device;

		vk::Queue m_queue;
		uint32_t m_queueIndex;

		vk::SurfaceFormatKHR m_surfaceFormat;
		vk::UniqueSwapchainKHR m_swapchain;
		std::vector<vk::Image> m_swapchainImages;
		std::vector<vk::UniqueImageView> m_swapchainImageViews;
		std::vector<vk::UniqueFramebuffer> m_frameBuffer;

		vk::UniqueCommandPool m_commandPool;
		vk::UniqueCommandBuffer m_commandBuffer;

		std::vector<const char*> m_layer = {
			"VK_LAYER_KHRONOS_validation"
		};

		std::vector<const char* > m_extension = {
			// For swapchain
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			// For ray tracing
			VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
			VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
			VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
			VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
			VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
		};

		AccelStruct m_bottomAccel;
		AccelStruct m_topAccel;

		std::vector<vk::UniqueShaderModule> shaderModules;
		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
		std::vector<vk::RayTracingShaderGroupCreateInfoKHR> shaderGroups;

		vk::UniqueDescriptorPool descPool;
		vk::UniqueDescriptorSetLayout descSetLayout;
		vk::UniqueDescriptorSet descSet;

		vk::UniquePipeline m_pipeline;
		vk::UniquePipelineLayout m_pipelineLayout;

		Buffer sbt{};
		vk::StridedDeviceAddressRegionKHR raygenRegion{};
		vk::StridedDeviceAddressRegionKHR missRegion{};
		vk::StridedDeviceAddressRegionKHR hitRegion{};

		void InitImGui();
		ImGuiContext* m_context;

		vk::UniqueDescriptorPool m_imGuiDescriptorPool;
		vk::UniqueRenderPass m_imGuiRenderPass;


		// Material
		//void DefineMaterial();

		const std::vector<ShrPtr<MaterialParameter>> m_matParams =
		{	
			MakeShr<MatParamColor>("BaseColor", vec4(0.8f)),
			MakeShr<MatParamFloat>("Roughness", 0.0f),
			MakeShr<MatParamFloat>("Metallic", 0.0f),
		};

	};
}
