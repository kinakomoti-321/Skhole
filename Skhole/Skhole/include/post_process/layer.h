#pragma once

#include <include.h>
#include <vulkan_helpler/vk_buffer.h>
#include <vulkan_helpler/vk_hepler.h>

namespace Skhole {
	class PPLayer {
	public:
		struct LayerDesc {
			vk::PhysicalDevice physicalDevice;
			vk::Device device;

			std::string csShaderPath;
			std::vector<VkHelper::BindingLayoutElement> binding;

			uint32_t width, height;
		};

	public:
		PPLayer() {}
		~PPLayer() {}

		void Init(const LayerDesc& desc);
		void Resize(uint32_t width, uint32_t height);
		void StartBinding();
		void SetUniformBuffer(const vk::Buffer& buffer, size_t size, uint32_t bindingIndex, vk::Device device);
		void SetImage(const vk::ImageView& image, uint32_t index, vk::Device device);
		void EndBinding(vk::Device device);
		void Execute(vk::CommandBuffer command, uint32_t dispatchW, uint32_t dispatchH);
		void Destroy(vk::Device device);

	private:
		vk::UniquePipeline computePipeline;
		vk::UniqueShaderModule csModule;
		vk::UniquePipelineLayout pipelineLayout;
		VkHelper::BindingManager bindingManager;
	};
}
