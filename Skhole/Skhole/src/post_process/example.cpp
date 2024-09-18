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


		PPLayer::LayerDesc layerDesc;
		layerDesc.width = width;
		layerDesc.height = height;
		layerDesc.csShaderPath = "shader/postprocess/example/example.comp.spv";

		layerDesc.binding =
		{
			{0, vk::DescriptorType::eStorageImage, 1,vk::ShaderStageFlagBits::eCompute},
			{1, vk::DescriptorType::eStorageImage,1, vk::ShaderStageFlagBits::eCompute},
			{2, vk::DescriptorType::eUniformBuffer,1, vk::ShaderStageFlagBits::eCompute}
		};

		layerDesc.device = device;
		layerDesc.physicalDevice = physicalDevice;

		layer1.Init(layerDesc);

		uniformBuffer.Init(
			physicalDevice,
			device,
			sizeof(UniformObject),
			vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostCached | vk::MemoryPropertyFlagBits::eHostVisible
		);

		SKHOLE_LOG("... End Initialization PostProcessor");
	}

	void PPExample::Execute(vk::CommandBuffer command, const ExecuteDesc& desc) {
		auto& device = desc.device;

		// Frame
		WriteBinding(device, desc);

		// Execute
		layer1.Execute(command, width, height);
	}

	void PPExample::WriteBinding(vk::Device device, const ExecuteDesc& desc) {

		// Uniform Buffer	
		auto& parameter = desc.param.param;
		uniformObject.color = CastParamCol(parameter[0])->value;
		uniformObject.intensity = CastParamFloat(parameter[1])->value;

		CopyBuffer(device, uniformBuffer, &uniformObject, uniformBuffer.GetBufferSize());

		layer1.StartBinding();

		layer1.SetImage(desc.inputImage, 0, device);
		layer1.SetImage(desc.outputImage, 1, device);
		layer1.SetUniformBuffer(*uniformBuffer.buffer, uniformBuffer.GetBufferSize(), 2, device);

		layer1.EndBinding(device);
	}

	PostProcessParameter PPExample::GetParamter() {
		PostProcessParameter param;
		param.name = "Example Post Process";
		CopyParameter(params, param.param);

		return param;
	}

	void PPExample::Resize(uint32_t width, uint32_t height) {
		this->width = width;
		this->height = height;
	}

	void PPExample::Destroy(vk::Device device) {
		layer1.Destroy(device);
	}
}