#pragma once
#pragma once
#include <include.h>
#include <post_process/post_processor.h>

#include <vulkan_helpler/vk_buffer.h>
#include <vulkan_helpler/vk_hepler.h>

namespace Skhole {

	class PPExample : public PostProcessor {
	public:
		PPExample() {}
		~PPExample() {}

		void Init(const Desc& desc) override;
		void Resize(uint32_t width, uint32_t height) override;
		void Execute(vk::CommandBuffer command, const ExecuteDesc& desc) override;
		void Destroy(vk::Device device) override;


		PostProcessParameter GetParamter() override;

	private:
		struct UniformObject {
			vec4 color = vec4(0.0);
			float intensity = 1.0f;
		};

		void WriteBinding(vk::Device device, const ExecuteDesc& desc);

		const std::vector<ShrPtr<Parameter>> params = {
			MakeShr<ParamCol>("color", vec4(1.0f, 0.0f, 0.0f,0.0f)),
			MakeShr<ParamFloat>("intensity", 1.0f)
		};

		vk::UniquePipeline computePipeline;
		vk::UniqueShaderModule csModule;
		VkHelper::BindingManager bindingManager;

		vk::UniquePipelineLayout pipelineLayout;

		vk::UniqueDescriptorPool descriptorPool;

		UniformObject uniformObject;
		Buffer uniformBuffer;

		uint32_t width, height;

	};

}
