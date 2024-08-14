#pragma once
#include <include.h>
#include <vulkan_helpler/vkutils.hpp>

namespace Skhole {
	struct Buffer {
		vk::UniqueBuffer buffer;
		vk::UniqueDeviceMemory memory;
		vk::DeviceAddress address{};

		void init(vk::PhysicalDevice physicalDevice,
			vk::Device device,
			vk::DeviceSize size,
			vk::BufferUsageFlags usage,
			vk::MemoryPropertyFlags memoryProperty,
			const void* data = nullptr) {
			// Create buffer
			vk::BufferCreateInfo createInfo{};
			createInfo.setSize(size);
			createInfo.setUsage(usage);
			buffer = device.createBufferUnique(createInfo);

			// Allocate memory
			vk::MemoryRequirements memoryReq =
				device.getBufferMemoryRequirements(*buffer);
			vk::MemoryAllocateFlagsInfo allocateFlags{};
			if (usage & vk::BufferUsageFlagBits::eShaderDeviceAddress) {
				allocateFlags.flags = vk::MemoryAllocateFlagBits::eDeviceAddress;
			}

			uint32_t memoryType = vkutils::getMemoryType(physicalDevice,  //
				memoryReq, memoryProperty);
			vk::MemoryAllocateInfo allocateInfo{};
			allocateInfo.setAllocationSize(memoryReq.size);
			allocateInfo.setMemoryTypeIndex(memoryType);
			allocateInfo.setPNext(&allocateFlags);
			memory = device.allocateMemoryUnique(allocateInfo);

			// Bind buffer to memory
			device.bindBufferMemory(*buffer, *memory, 0);

			// Copy data
			if (data) {
				void* mappedPtr = device.mapMemory(*memory, 0, size);
				memcpy(mappedPtr, data, size);
				device.unmapMemory(*memory);
			}

			// Get address
			if (usage & vk::BufferUsageFlagBits::eShaderDeviceAddress) {
				vk::BufferDeviceAddressInfoKHR addressInfo{};
				addressInfo.setBuffer(*buffer);
				address = device.getBufferAddressKHR(&addressInfo);
			}
		}
	};

	struct AccelStruct {
		vk::UniqueAccelerationStructureKHR accel;
		Buffer buffer;

		void init(vk::PhysicalDevice physicalDevice,
			vk::Device device,
			vk::CommandPool commandPool,
			vk::Queue queue,
			vk::AccelerationStructureTypeKHR type,
			vk::AccelerationStructureGeometryKHR geometry,
			uint32_t primitiveCount) {
			// Get build info
			vk::AccelerationStructureBuildGeometryInfoKHR buildInfo{};
			buildInfo.setType(type);
			buildInfo.setMode(vk::BuildAccelerationStructureModeKHR::eBuild);
			buildInfo.setFlags(
				vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace);
			buildInfo.setGeometries(geometry);

			vk::AccelerationStructureBuildSizesInfoKHR buildSizes =
				device.getAccelerationStructureBuildSizesKHR(
					vk::AccelerationStructureBuildTypeKHR::eDevice, buildInfo,
					primitiveCount);

			// Create buffer for AS
			buffer.init(physicalDevice, device,
				buildSizes.accelerationStructureSize,
				vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR,
				vk::MemoryPropertyFlagBits::eDeviceLocal);

			// Create AS
			vk::AccelerationStructureCreateInfoKHR createInfo{};
			createInfo.setBuffer(*buffer.buffer);
			createInfo.setSize(buildSizes.accelerationStructureSize);
			createInfo.setType(type);
			accel = device.createAccelerationStructureKHRUnique(createInfo);

			// Create scratch buffer
			Buffer scratchBuffer;
			scratchBuffer.init(physicalDevice, device, buildSizes.buildScratchSize,
				vk::BufferUsageFlagBits::eStorageBuffer |
				vk::BufferUsageFlagBits::eShaderDeviceAddress,
				vk::MemoryPropertyFlagBits::eDeviceLocal);

			buildInfo.setDstAccelerationStructure(*accel);
			buildInfo.setScratchData(scratchBuffer.address);

			vk::AccelerationStructureBuildRangeInfoKHR buildRangeInfo{};
			buildRangeInfo.setPrimitiveCount(primitiveCount);
			buildRangeInfo.setPrimitiveOffset(0);
			buildRangeInfo.setFirstVertex(0);
			buildRangeInfo.setTransformOffset(0);

			// Build
			vkutils::oneTimeSubmit(          //
				device, commandPool, queue,  //
				[&](vk::CommandBuffer commandBuffer) {
					commandBuffer.buildAccelerationStructuresKHR(buildInfo,
					&buildRangeInfo);
				});

			// Get address
			vk::AccelerationStructureDeviceAddressInfoKHR addressInfo{};
			addressInfo.setAccelerationStructure(*accel);
			buffer.address = device.getAccelerationStructureAddressKHR(addressInfo);
		}
	};
}
