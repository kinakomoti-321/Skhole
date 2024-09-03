#include <post_process/example.h>

namespace Skhole {
	void PPExample::Init(const Desc& desc) {
		auto& physicalDevice = desc.physicalDevice;
		auto& device = desc.device;
		auto& queue = desc.queue;
		auto& commandPool = desc.commandPool;


		vk::DescriptorPoolSize poolSize{ vk::DescriptorType::eStorageImage,10 };
		vk::DescriptorPoolCreateInfo descPoolInfo{};
		descPoolInfo.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
		descPoolInfo.setPoolSizeCount(1);
		descPoolInfo.setPPoolSizes(&poolSize);
		descPoolInfo.setMaxSets(1);

		descriptorPool = device.createDescriptorPoolUnique(descPoolInfo);
		csModule = vkutils::createShaderModule(device, "shader/postprocess/example/example.comp.spv");

		vk::PipelineShaderStageCreateInfo shaderStageInfo{};
		shaderStageInfo.setStage(vk::ShaderStageFlagBits::eCompute);
		shaderStageInfo.setModule(*csModule);
		shaderStageInfo.setPName("main");

		vk::DescriptorSetLayoutBinding binding{ 0, vk::DescriptorType::eStorageImage, 3, vk::ShaderStageFlagBits::eCompute };
		vk::DescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.setBindingCount(1);
		layoutInfo.setPBindings(&binding);

		descriptorSetLayout = device.createDescriptorSetLayoutUnique(layoutInfo);

		vk::PipelineLayoutCreateInfo pipelineLayoutInfo{ {},*descriptorSetLayout };
		pipelineLayout = device.createPipelineLayoutUnique(pipelineLayoutInfo);

		vk::ComputePipelineCreateInfo pipelineInfo{ {},shaderStageInfo,*pipelineLayout };
		auto result = device.createComputePipelineUnique({}, pipelineInfo);
		if (result.result != vk::Result::eSuccess) {
			SKHOLE_ERROR("Failed to create compute pipeline");
		}

		computePipeline = std::move(result.value);

		vk::DescriptorSetAllocateInfo allocInfo{};
		allocInfo.setDescriptorPool(*descriptorPool);
		allocInfo.setDescriptorSetCount(1);
		allocInfo.setPSetLayouts(&*descriptorSetLayout);

		descriptorSet = std::move(device.allocateDescriptorSetsUnique(allocInfo)[0]);
	}

	void PPExample::Execute(vk::CommandBuffer command, const ExecuteDesc& desc) {
		auto& device = desc.device;
		// Frame
		std::vector<vk::DescriptorImageInfo> descBufferInfo;
		vk::DescriptorImageInfo descImageInfo{};
		descImageInfo.setImageView(desc.inputImage);
		descImageInfo.setImageLayout(vk::ImageLayout::eGeneral);

		descBufferInfo.push_back(descImageInfo);

		vk::WriteDescriptorSet writeDescSet{};
		writeDescSet.setDstSet(*descriptorSet);
		writeDescSet.setDstBinding(0);
		writeDescSet.setDescriptorType(vk::DescriptorType::eStorageImage);
		writeDescSet.setDescriptorCount(1);
		writeDescSet.setPImageInfo(&descImageInfo);
		device.updateDescriptorSets(writeDescSet, nullptr);

		// Execute
		command.bindPipeline(vk::PipelineBindPoint::eCompute, *computePipeline);
		command.bindDescriptorSets(vk::PipelineBindPoint::eCompute, *pipelineLayout, 0, *descriptorSet, nullptr);
		command.dispatch(width, height, 1);

	}

	PostProcessParameter PPExample::GetParamter() {
		PostProcessParameter param;
		return param;
	}

	void PPExample::Resize(uint32_t width, uint32_t height) {
		SKHOLE_UNIMPL();
	}

	void PPExample::Destroy(vk::Device device) {
		device.destroyDescriptorPool(*descriptorPool);
		device.destroyDescriptorSetLayout(*descriptorSetLayout);
		device.destroyPipelineLayout(*pipelineLayout);
		device.destroyPipeline(*computePipeline);
		device.destroyShaderModule(*csModule);
	}
}