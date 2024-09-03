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
		vk::UniquePipeline computePipeline;
		vk::UniqueShaderModule csModule;
		vk::UniqueDescriptorSetLayout descriptorSetLayout;
		vk::UniqueDescriptorSet descriptorSet;
		vk::UniquePipelineLayout pipelineLayout;

		vk::UniqueDescriptorPool descriptorPool;

		uint32_t width, height;

	};

}
