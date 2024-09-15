#include <renderer/raytracing_pipeline.h>

namespace Skhole {
	void RaytracingPipeline::InitPipeline(const Desc& desc)
	{
		CreateShaderModule(desc);
		CreateShaderBindingTable(desc);
		CreatePipeline(desc);
	}

	void RaytracingPipeline::CreateShaderModule(const Desc& desc)
	{
		auto& device = desc.device;
		auto& raygenShaderPath = desc.raygenShaderPath;
		auto& missShaderPath = desc.missShaderPath;
		auto& closestHitShaderPath = desc.closestHitShaderPath;

		uint32_t raygenShader = 0;
		uint32_t missShader = 1;
		uint32_t closestHitShader = 2;

		shaderModules.resize(3);
		shaderStages.resize(3);

		shaderModules[raygenShader] = vkutils::createShaderModule(device, raygenShaderPath);
		shaderModules[missShader] = vkutils::createShaderModule(device, missShaderPath);
		shaderModules[closestHitShader] = vkutils::createShaderModule(device, closestHitShaderPath);

		shaderStages[raygenShader].stage = vk::ShaderStageFlagBits::eRaygenKHR;
		shaderStages[raygenShader].module = *shaderModules[raygenShader];
		shaderStages[raygenShader].pName = "main";

		shaderStages[missShader].stage = vk::ShaderStageFlagBits::eMissKHR;
		shaderStages[missShader].module = *shaderModules[missShader];
		shaderStages[missShader].pName = "main";

		shaderStages[closestHitShader].stage = vk::ShaderStageFlagBits::eClosestHitKHR;
		shaderStages[closestHitShader].module = *shaderModules[closestHitShader];
		shaderStages[closestHitShader].pName = "main";

		uint32_t raygenGroup = 0;
		uint32_t missGroup = 1;
		uint32_t hitGroup = 2;

		shaderGroups.resize(3);
		shaderGroups[raygenGroup].setType(
			vk::RayTracingShaderGroupTypeKHR::eGeneral);
		shaderGroups[raygenGroup].setGeneralShader(raygenShader);
		shaderGroups[raygenGroup].setClosestHitShader(VK_SHADER_UNUSED_KHR);
		shaderGroups[raygenGroup].setAnyHitShader(VK_SHADER_UNUSED_KHR);
		shaderGroups[raygenGroup].setIntersectionShader(VK_SHADER_UNUSED_KHR);

		shaderGroups[missGroup].setType(
			vk::RayTracingShaderGroupTypeKHR::eGeneral);
		shaderGroups[missGroup].setGeneralShader(missShader);
		shaderGroups[missGroup].setClosestHitShader(VK_SHADER_UNUSED_KHR);
		shaderGroups[missGroup].setAnyHitShader(VK_SHADER_UNUSED_KHR);
		shaderGroups[missGroup].setIntersectionShader(VK_SHADER_UNUSED_KHR);

		shaderGroups[hitGroup].setType(
			vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup);
		shaderGroups[hitGroup].setGeneralShader(VK_SHADER_UNUSED_KHR);
		shaderGroups[hitGroup].setClosestHitShader(closestHitShader);
		shaderGroups[hitGroup].setAnyHitShader(VK_SHADER_UNUSED_KHR);
		shaderGroups[hitGroup].setIntersectionShader(VK_SHADER_UNUSED_KHR);


	}

	void RaytracingPipeline::CreatePipeline(const Desc& desc)
	{
		auto& device = desc.device;
		auto& descriptorSetLayout = desc.descriptorSetLayout;

		vk::PipelineLayoutCreateInfo layoutCreateInfo{};
		layoutCreateInfo.setSetLayouts(descriptorSetLayout);
		m_pipelineLayout = device.createPipelineLayoutUnique(layoutCreateInfo);

		vk::RayTracingPipelineCreateInfoKHR pipelineCreateInfo{};
		pipelineCreateInfo.setLayout(*m_pipelineLayout);
		pipelineCreateInfo.setStages(shaderStages);
		pipelineCreateInfo.setGroups(shaderGroups);
		pipelineCreateInfo.setMaxPipelineRayRecursionDepth(1);
		auto result = device.createRayTracingPipelineKHRUnique(
			nullptr, nullptr, pipelineCreateInfo);

		if (result.result != vk::Result::eSuccess) {
			std::cerr << "Failed to create ray tracing pipeline.\n";
			std::abort();
		}

		m_pipeline = std::move(result.value);

	}

	void RaytracingPipeline::CreateShaderBindingTable(const Desc& desc)
	{
		auto& device = desc.device;
		auto& physicalDevice = desc.physicalDevice;

		vk::PhysicalDeviceRayTracingPipelinePropertiesKHR rtProperties =
			vkutils::getRayTracingProps(physicalDevice);

		uint32_t handleSize = rtProperties.shaderGroupHandleSize;
		uint32_t handleAlignment = rtProperties.shaderGroupHandleAlignment;
		uint32_t baseAlignment = rtProperties.shaderGroupBaseAlignment;
		uint32_t handleSizeAligned =
			vkutils::alignUp(handleSize, handleAlignment);

		// Set strides and sizes
		uint32_t raygenShaderCount = 1;  // raygen count must be 1
		uint32_t missShaderCount = 1;
		uint32_t hitShaderCount = 1;

		raygenRegion.setStride(
			vkutils::alignUp(handleSizeAligned, baseAlignment));
		raygenRegion.setSize(raygenRegion.stride);

		missRegion.setStride(handleSizeAligned);
		missRegion.setSize(vkutils::alignUp(missShaderCount * handleSizeAligned,
			baseAlignment));

		hitRegion.setStride(handleSizeAligned);
		hitRegion.setSize(vkutils::alignUp(hitShaderCount * handleSizeAligned,
			baseAlignment));

		// Create SBT
		vk::DeviceSize sbtSize =
			raygenRegion.size + missRegion.size + hitRegion.size;
		sbt.init(physicalDevice, device, sbtSize,
			vk::BufferUsageFlagBits::eShaderBindingTableKHR |
			vk::BufferUsageFlagBits::eTransferSrc |
			vk::BufferUsageFlagBits::eShaderDeviceAddress,
			vk::MemoryPropertyFlagBits::eHostVisible |
			vk::MemoryPropertyFlagBits::eHostCoherent);

		// Get shader group handles
		uint32_t handleCount =
			raygenShaderCount + missShaderCount + hitShaderCount;
		uint32_t handleStorageSize = handleCount * handleSize;
		std::vector<uint8_t> handleStorage(handleStorageSize);

		auto result = device.getRayTracingShaderGroupHandlesKHR(
			*m_pipeline, 0, handleCount, handleStorageSize, handleStorage.data());
		if (result != vk::Result::eSuccess) {
			std::cerr << "Failed to get ray tracing shader group handles.\n";
			std::abort();
		}

		// Copy handles
		uint32_t handleIndex = 0;
		uint8_t* sbtHead =
			static_cast<uint8_t*>(device.mapMemory(*sbt.memory, 0, sbtSize));

		uint8_t* dstPtr = sbtHead;
		auto copyHandle = [&](uint32_t index) {
			std::memcpy(dstPtr, handleStorage.data() + handleSize * index,
				handleSize);
			};

		// Raygen
		copyHandle(handleIndex++);

		// Miss
		dstPtr = sbtHead + raygenRegion.size;
		for (uint32_t c = 0; c < missShaderCount; c++) {
			copyHandle(handleIndex++);
			dstPtr += missRegion.stride;
		}

		// Hit
		dstPtr = sbtHead + raygenRegion.size + missRegion.size;
		for (uint32_t c = 0; c < hitShaderCount; c++) {
			copyHandle(handleIndex++);
			dstPtr += hitRegion.stride;
		}

		raygenRegion.setDeviceAddress(sbt.address);
		missRegion.setDeviceAddress(sbt.address + raygenRegion.size);
		hitRegion.setDeviceAddress(sbt.address + raygenRegion.size +
			missRegion.size);
	}
};