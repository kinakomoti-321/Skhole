#pragma once

#include <include.h>
#include <vulkan_helpler/vk_buffer.h>
#include <vulkan_helpler/vkutils.hpp>

namespace Skhole {

	// Now, Assume that Raygen, ClosestHit ,Miss is only one.
	class RaytracingPipeline {
	public:
		struct Desc {
			vk::PhysicalDevice physicalDevice;
			vk::Device device;
			vk::DescriptorSetLayout descriptorSetLayout;

			std::string raygenShaderPath;
			std::string missShaderPath;
			std::string closestHitShaderPath;
		};

	public:
		RaytracingPipeline() {};
		~RaytracingPipeline() {};

		void InitPipeline(const Desc& desc);

		vk::Pipeline GetPipeline() { return *m_pipeline; }
		vk::PipelineLayout GetPipelineLayout() { return *m_pipelineLayout; }

	private:
		void CreateShaderModule(const Desc& desc);
		void CreatePipeline(const Desc& desc);
		void CreateShaderBindingTable(const Desc& desc);

	private:
		vk::UniquePipeline m_pipeline;
		vk::UniquePipelineLayout m_pipelineLayout;

		std::vector<vk::UniqueShaderModule> shaderModules;
		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
		std::vector<vk::RayTracingShaderGroupCreateInfoKHR> shaderGroups;

		Buffer sbt{};
		vk::StridedDeviceAddressRegionKHR raygenRegion{};
		vk::StridedDeviceAddressRegionKHR missRegion{};
		vk::StridedDeviceAddressRegionKHR hitRegion{};
	};
}
