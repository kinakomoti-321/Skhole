#include <post_process/example.h>

namespace Skhole {
	void PPExample::Init(const Desc& desc) {
		SKHOLE_LOG("... Initialization PostProcessor");
		auto& physicalDevice = desc.physicalDevice;
		auto& device = desc.device;
		auto& queue = desc.queue;
		auto& commandPool = desc.commandPool;
		width = desc.width;
		height = desc.height;

		bindingManager.bindings = {
			{0,vk::DescriptorType::eStorageImage,1,vk::ShaderStageFlagBits::eCompute},
			{1,vk::DescriptorType::eStorageImage,1,vk::ShaderStageFlagBits::eCompute},
			{2,vk::DescriptorType::eUniformBuffer,1,vk::ShaderStageFlagBits::eCompute}
		};

		bindingManager.SetLayout(device);
		bindingManager.SetPool(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, device);

		csModule = vkutils::createShaderModule(device, "shader/postprocess/example/example.comp.spv");

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

		uniformBuffer.init(
			physicalDevice,
			device,
			sizeof(UniformObject),
			vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostCached | vk::MemoryPropertyFlagBits::eHostVisible
		);

		computePipeline = std::move(result.value);

		bindingManager.SetDescriptorSet(device);
		SKHOLE_LOG("... End Initialization PostProcessor");
	}

	void PPExample::Execute(vk::CommandBuffer command, const ExecuteDesc& desc) {
		auto& device = desc.device;

		// Frame
		WriteBinding(device, desc);

		// Execute
		command.bindPipeline(vk::PipelineBindPoint::eCompute, *computePipeline);
		command.bindDescriptorSets(vk::PipelineBindPoint::eCompute, *pipelineLayout, 0, bindingManager.descriptorSet, nullptr);
		command.dispatch(width, height, 1);

	}

	void PPExample::WriteBinding(vk::Device device, const ExecuteDesc& desc) {

		// Uniform Buffer	
		auto& parameter = desc.param.param;
		uniformObject.color = CastParamCol(parameter[0])->value;
		uniformObject.intensity = CastParamFloat(parameter[1])->value;

		CopyBuffer(device, uniformBuffer, &uniformObject, uniformBuffer.GetBufferSize());

		//VkHelper::BindingManager::WritingInfo info;
		//info.numImage = 2;
		//info.numBuffer = 1;
		bindingManager.StartWriting();

		bindingManager.WriteImage(
			desc.inputImage, vk::ImageLayout::eGeneral, VK_NULL_HANDLE,
			vk::DescriptorType::eStorageImage, 0, 1, device
		);

		bindingManager.WriteImage(
			desc.outputImage, vk::ImageLayout::eGeneral, VK_NULL_HANDLE,
			vk::DescriptorType::eStorageImage, 1, 1, device
		);

		bindingManager.WriteBuffer(
			uniformBuffer.buffer.get(), 0, uniformBuffer.GetBufferSize(),
			vk::DescriptorType::eUniformBuffer, 2, 1, device
		);

		bindingManager.EndWriting(device);
	}

	PostProcessParameter PPExample::GetParamter() {
		PostProcessParameter param;
		param.name = "Example Post Process";
		param.param = params;
		return param;
	}

	void PPExample::Resize(uint32_t width, uint32_t height) {
		SKHOLE_UNIMPL();
	}

	void PPExample::Destroy(vk::Device device) {
		device.destroyDescriptorPool(*descriptorPool);
		device.destroyPipelineLayout(*pipelineLayout);
		device.destroyPipeline(*computePipeline);
		device.destroyShaderModule(*csModule);

		*descriptorPool = VK_NULL_HANDLE;
		*pipelineLayout = VK_NULL_HANDLE;
		*computePipeline = VK_NULL_HANDLE;
		*csModule = VK_NULL_HANDLE;

	}
}