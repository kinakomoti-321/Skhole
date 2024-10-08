#pragma once
#pragma once
#include <include.h>
#include <post_process/post_processor.h>

#include <vulkan_helpler/vk_buffer.h>
#include <vulkan_helpler/vk_hepler.h>
#include <post_process/layer.h>

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
			float exposure = 1.0f;
			int useACES = 0;

			vec4 Lift = vec4(0.0);
			vec4 Gain = vec4(0.0);
			vec4 Gamma = vec4(1.0);
			float Brightness = 0.0;
			float Contrast = 0.0;
			float Saturation = 0.0;

		};

		void WriteBinding(vk::Device device, const ExecuteDesc& desc);

		const std::vector<ShrPtr<Parameter>> params = {
			MakeShr<ParamCol>("color", vec4(1.0f, 1.0f, 1.0f,0.0f)),
			MakeShr<ParamFloat>("exposure", 1.0f,0.0,10.0f),
			MakeShr<ParamBool>("useACES Tonemap", false),
			
			// Color Grading
			MakeShr<ParamCol>("Lift", vec4(0.0f, 0.0f, 0.0f,0.0f)),
			MakeShr<ParamCol>("Gain", vec4(0.0f, 0.0f, 0.0f,0.0f)),
			MakeShr<ParamCol>("Gamma", vec4(1.0f, 1.0f, 1.0f,0.0f)),
			MakeShr<ParamFloat>("Brightness",0.0),
			MakeShr<ParamFloat>("Contrast",0.0),
			MakeShr<ParamFloat>("Saturation",0.0),
		};

		vk::UniquePipeline computePipeline;
		vk::UniqueShaderModule csModule;
		VkHelper::BindingManager bindingManager;

		vk::UniquePipelineLayout pipelineLayout;

		UniformObject uniformObject;
		Buffer uniformBuffer;

		uint32_t width, height;

		PPLayer layer1;
	};

}
