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

		void* Map(vk::Device device, uint32_t offset, uint32_t size) {
			return device.mapMemory(*memory, offset, size);
		}

		void Ummap(vk::Device device) {
			device.unmapMemory(*memory);
		}

		void Release(vk::Device device) {
			device.destroyBuffer(*buffer);
			device.freeMemory(*memory);

			*buffer = VK_NULL_HANDLE;
			*memory = VK_NULL_HANDLE;
		}
	};

	struct DeviceBuffer {
		Buffer hostBuffer;
		Buffer deviceBuffer;
		uint32_t bufferSize;

		void Init(
			vk::PhysicalDevice physicalDevice,
			vk::Device device,
			vk::DeviceSize size,
			vk::BufferUsageFlags usage,
			vk::MemoryPropertyFlags memoryProperty
		)
		{
			hostBuffer.init(physicalDevice, device, size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
			deviceBuffer.init(physicalDevice, device, size, usage | vk::BufferUsageFlagBits::eTransferDst, memoryProperty);
			bufferSize = size;
		}

		void* Map(vk::Device device, uint32_t offset, uint32_t size) {
			return hostBuffer.Map(device, offset, size);
		}

		void Unmap(vk::Device device) {
			hostBuffer.Ummap(device);
		}

		void UploadToDevice(vk::Device device, vk::CommandPool commandPool, vk::Queue queue) {
			vkutils::oneTimeSubmit(device, commandPool, queue, [&](vk::CommandBuffer commandBuffer) {
				vk::BufferCopy copyRegion{};
				copyRegion.setSize(bufferSize);
				commandBuffer.copyBuffer(*hostBuffer.buffer, *deviceBuffer.buffer, copyRegion);
				});
		}

		void UploadToDevice(vk::Device device, vk::CommandPool commandPool, vk::Queue queue, uint32_t offset, uint32_t size) {
			vkutils::oneTimeSubmit(device, commandPool, queue, [&](vk::CommandBuffer commandBuffer) {
				vk::BufferCopy copyRegion{};
				copyRegion.setSize(size);
				copyRegion.setSrcOffset(offset);
				copyRegion.setDstOffset(offset);
				commandBuffer.copyBuffer(*hostBuffer.buffer, *deviceBuffer.buffer, copyRegion);
			});
		}

		void UploadToDevice(vk::Device device, vk::CommandPool commandPool, vk::Queue queue,uint32_t srcOffset, uint32_t dstOffset, uint32_t size) {
			vkutils::oneTimeSubmit(device, commandPool, queue, [&](vk::CommandBuffer commandBuffer) {
				vk::BufferCopy copyRegion{};
				copyRegion.setSize(size);
				copyRegion.setSrcOffset(srcOffset);
				copyRegion.setDstOffset(dstOffset);
				commandBuffer.copyBuffer(*hostBuffer.buffer, *deviceBuffer.buffer, copyRegion);
			});
		}

		vk::Buffer GetDeviceBuffer() {
			return *deviceBuffer.buffer;
		}

		vk::Buffer GetHostBuffer() {
			return *hostBuffer.buffer;
		}

		uint32_t GetBufferSize() {
			return bufferSize;
		}

		vk::DeviceAddress GetDeviceAddress() {
			return deviceBuffer.address;
		}

		void Release(vk::Device device) {
			hostBuffer.Release(device);
			deviceBuffer.Release(device);
			bufferSize = 0;
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


		void Release(vk::Device device) {
			device.destroyAccelerationStructureKHR(*accel);
			buffer.Release(device);
			*accel = VK_NULL_HANDLE;
		}
	};

	class Image {

	public:
		Image(){}
		~Image(){}

		void Init(
			vk::PhysicalDevice physicalDevice,
			vk::Device device,
			uint32_t width,
			uint32_t height,
			vk::Format format,
			vk::ImageTiling tiling,
			vk::ImageUsageFlags usage,
			vk::MemoryPropertyFlags properties
		) {
			vk::ImageCreateInfo imageInfo{};
			imageInfo.setImageType(vk::ImageType::e2D);
			imageInfo.setExtent({ width, height, 1 });
			imageInfo.setMipLevels(1);
			imageInfo.setArrayLayers(1);
			imageInfo.setFormat(format);
			imageInfo.setTiling(tiling);
			imageInfo.setInitialLayout(vk::ImageLayout::eUndefined);
			imageInfo.setUsage(usage);
			imageInfo.setSharingMode(vk::SharingMode::eExclusive);
			imageInfo.setSamples(vk::SampleCountFlagBits::e1);
			imageInfo.setFlags(vk::ImageCreateFlags());

			m_image = device.createImage(imageInfo);

			vk::MemoryRequirements memRequirements = device.getImageMemoryRequirements(m_image);

			vk::MemoryAllocateInfo allocInfo{};
			allocInfo.setAllocationSize(memRequirements.size);
			allocInfo.setMemoryTypeIndex(vkutils::getMemoryType(physicalDevice, memRequirements, properties));

			m_imageMemory = device.allocateMemory(allocInfo);
			device.bindImageMemory(m_image, m_imageMemory, 0);
		
			vk::ImageViewCreateInfo viewInfo{};
			viewInfo.setImage(m_image);
			viewInfo.setViewType(vk::ImageViewType::e2D);
			viewInfo.setFormat(format);
			viewInfo.setSubresourceRange({ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });

			m_imageView = device.createImageView(viewInfo,nullptr);
		}

		vk::Image GetImage() {
			return m_image;	
		}

		vk::ImageView GetImageView() {
			return m_imageView;
		}

		void Release(vk::Device device) {
			device.destroyImageView(m_imageView);
			device.destroyImage(m_image);
			device.freeMemory(m_imageMemory);
		}
		
	private:
		vk::Image m_image;
		vk::ImageView m_imageView;
		vk::DeviceMemory m_imageMemory;
	};
}
