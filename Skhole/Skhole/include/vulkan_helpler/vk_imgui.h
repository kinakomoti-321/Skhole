#pragma once

#include <include.h>

namespace VkHelper {

	inline vk::DescriptorPool CreateImGuiDescriptor(vk::Device device) {
		std::vector<vk::DescriptorPoolSize> poolSize = {
			{vk::DescriptorType::eSampler,1000},
			{vk::DescriptorType::eCombinedImageSampler,1000},
			{vk::DescriptorType::eSampledImage,1000},
			{vk::DescriptorType::eStorageImage,1000},
			{vk::DescriptorType::eUniformTexelBuffer,1000},
			{vk::DescriptorType::eStorageTexelBuffer,1000},
			{vk::DescriptorType::eUniformBuffer,1000},
			{vk::DescriptorType::eStorageBuffer,1000},
			{vk::DescriptorType::eUniformBufferDynamic,1000},
			{vk::DescriptorType::eStorageBufferDynamic,1000},
			{vk::DescriptorType::eInputAttachment,1000}
		};

		vk::DescriptorPoolCreateInfo poolInfo = {};
		poolInfo.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
		poolInfo.setMaxSets(1000);
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSize.size());
		poolInfo.pPoolSizes = poolSize.data();

		return device.createDescriptorPool(poolInfo);
	}

	class VulkanImGuiManager {
	public:
		VulkanImGuiManager() {};
		~VulkanImGuiManager() {};

		void Init(
			GLFWwindow* window,
			vk::Instance intance,
			vk::PhysicalDevice physicalDevice,
			vk::Device device,
			uint32_t queueIndex,
			vk::Queue queue,
			vk::RenderPass renderPass,
			uint32_t minImageCount,
			uint32_t imageCount
		)
		{
			m_context = ImGui::CreateContext();
			ImGui::SetCurrentContext(m_context);
			ImGui::StyleColorsDark();

			ImGui_ImplGlfw_InitForVulkan(window, true);

			m_imGuiDescriptorPool = CreateImGuiDescriptor(device);

			ImGui_ImplVulkan_InitInfo init_info = {};
			init_info.Instance = intance;
			init_info.PhysicalDevice = physicalDevice;
			init_info.Device = device;
			init_info.QueueFamily = queueIndex;
			init_info.Queue = queue;
			init_info.RenderPass = renderPass;
			init_info.DescriptorPool = m_imGuiDescriptorPool;
			init_info.MinImageCount = minImageCount;
			init_info.ImageCount = imageCount;
			init_info.PipelineCache = VK_NULL_HANDLE;
			init_info.Allocator = nullptr;
			init_info.CheckVkResultFn = nullptr;

			ImGui_ImplVulkan_Init(&init_info);

			ImGui_ImplVulkan_CreateFontsTexture();
		}

		void Destroy(vk::Device device) {
			ImGui_ImplVulkan_DestroyFontsTexture();
			ImGui_ImplVulkan_Shutdown();

			device.destroyDescriptorPool(m_imGuiDescriptorPool);

			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext(m_context);
		}

	private:
		ImGuiContext* m_context;
		vk::DescriptorPool m_imGuiDescriptorPool;
	};

	//inline void InitImGuiInVulkan(
	//	vk::Instance intance,
	//	vk::PhysicalDevice physicalDevice,
	//	vk::Device device,
	//	uint32_t queueIndex,
	//	vk::Queue queue,
	//	vk::RenderPass renderPass,
	//	uint32_t minImageCount,
	//	uint32_t imageCount
	//) {
	//	vk::DescriptorPool imGuiDescriptorPool = CreateImGuiDescriptor(device);

	//	ImGui_ImplVulkan_InitInfo init_info = {};
	//	init_info.Instance = intance;
	//	init_info.PhysicalDevice = physicalDevice;
	//	init_info.Device = device;
	//	init_info.QueueFamily = queueIndex;
	//	init_info.Queue = queue;
	//	init_info.RenderPass = renderPass;
	//	init_info.DescriptorPool = imGuiDescriptorPool;
	//	init_info.MinImageCount = minImageCount;
	//	init_info.ImageCount = imageCount;
	//	init_info.PipelineCache = VK_NULL_HANDLE;
	//	init_info.Allocator = nullptr;
	//	init_info.CheckVkResultFn = nullptr;

	//	ImGui_ImplVulkan_Init(&init_info);

	//	ImGui_ImplVulkan_CreateFontsTexture();
	//}

}
