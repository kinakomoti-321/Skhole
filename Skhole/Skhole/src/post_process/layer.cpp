#include <post_process/layer.h>

namespace Skhole {
	void PPLayer::Init(const LayerDesc& desc) {
		auto& device = desc.device;
		auto& physicalDevice = desc.physicalDevice;
		auto& csShaderPath = desc.csShaderPath;
		auto& binding = desc.binding;

		csModule = vkutils::createShaderModule(device, csShaderPath);

		bindingManager.SetBindingLayout(device, binding, vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);

		vk::PipelineShaderStageCreateInfo shaderStageInfo{};
		shaderStageInfo.setStage(vk::ShaderStageFlagBits::eCompute);
		shaderStageInfo.setModule(*csModule);
		shaderStageInfo.setPName("main");

		vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
		pipelineLayoutInfo.setSetLayouts(bindingManager.descriptorSetLayout);
		pipelineLayout = device.createPipelineLayoutUnique(pipelineLayoutInfo);

		vk::ComputePipelineCreateInfo pipelineInfo{ {},shaderStageInfo,*pipelineLayout };
		auto result = device.createComputePipelineUnique({}, pipelineInfo);
		if (result.result != vk::Result::eSuccess) {
			SKHOLE_ERROR("Failed to create compute pipeline");
		}

		computePipeline = std::move(result.value);
	}

	void PPLayer::StartBinding() {
		bindingManager.StartWriting();
	}

	void PPLayer::EndBinding(vk::Device device) {
		bindingManager.EndWriting(device);
	}

	void PPLayer::Resize(uint32_t width, uint32_t height) {

	}

	void PPLayer::SetUniformBuffer(vk::Buffer& buffer, size_t size, uint32_t bindingIndex, vk::Device device) {
		bindingManager.WriteBuffer(buffer, 0, size, vk::DescriptorType::eUniformBuffer, bindingIndex, 1, device);
	}

	void PPLayer::SetImage(vk::ImageView& image, uint32_t bindingIndex, vk::Device device)
	{
		bindingManager.WriteImage(
			image, vk::ImageLayout::eGeneral, VK_NULL_HANDLE,
			vk::DescriptorType::eStorageImage, bindingIndex, 1, device
		);
	}

	void PPLayer::Execute(vk::CommandBuffer command, uint32_t dispatchW, uint32_t dispatchH) {
		command.bindPipeline(vk::PipelineBindPoint::eCompute, *computePipeline);
		command.bindDescriptorSets(vk::PipelineBindPoint::eCompute, *pipelineLayout, 0, bindingManager.descriptorSet, nullptr);
		command.dispatch(dispatchW, dispatchH, 1);
	}

	void PPLayer::Destroy(vk::Device device) {
		device.destroyPipelineLayout(*pipelineLayout);
		device.destroyPipeline(*computePipeline);
		device.destroyShaderModule(*csModule);

		bindingManager.Release(device);

		*pipelineLayout = VK_NULL_HANDLE;
		*computePipeline = VK_NULL_HANDLE;
		*csModule = VK_NULL_HANDLE;
	}
}
