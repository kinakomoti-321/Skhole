#pragma once

#include <renderer/renderer.h>
#include <vulkan_helpler/vkutils.hpp>

namespace Skhole
{

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

	struct Vertex {
		float pos[3];
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

	class SimpleRaytracer : public Renderer
	{
	public:
		SimpleRaytracer();
		~SimpleRaytracer();

		void Init(RendererDesc& desc) override;
		void Destroy() override;

		void Resize(unsigned int width, unsigned int height) override;
		void Render() override;

		void Wait() override;

		void OffscreenRender() override;
		RendererData GetRendererData() override;

	private:

		RendererDesc m_desc;
		void InitVulkan();

		void PrepareShader();
		void AddShader(uint32_t shaderIndex, const std::string& shaderName, vk::ShaderStageFlagBits stage);
		void CreateBottomLevelAS();
		void CreateTopLevelAS();
		void CreateDescriptorPool();
		void CreateDescSetLayout();
		void CreateDescSet();
		void CreatePipeline();
		void CreateShaderBindingTable();

		void UpdateDescriptorSet(vk::ImageView imageView);
		void RecordCommandBuffer(vk::Image image);

		vk::UniqueInstance m_instance;
		vk::UniqueDebugUtilsMessengerEXT m_debugMessenger;
		vk::UniqueSurfaceKHR m_surface;

		vk::PhysicalDevice m_physicalDevice;
		vk::UniqueDevice m_device;

		vk::Queue m_queue;
		uint32_t m_graphicsQueueIndex;

		vk::SurfaceFormatKHR m_surfaceFormat;
		vk::UniqueSwapchainKHR m_swapchain;
		std::vector<vk::Image> m_swapchainImages;
		std::vector<vk::UniqueImageView> m_swapchainImageViews;

		vk::UniqueCommandPool m_commandPool;
		vk::UniqueCommandBuffer m_commandBuffer;

		std::vector<const char*> m_layer = {
			"VK_LAYER_KHRONOS_validation"
		};

		std::vector<const char* > m_extension = {
			// For swapchain
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			// For ray tracing
			VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
			VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
			VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
			VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
			VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
		};

		AccelStruct m_bottomAccel;
		AccelStruct m_topAccel;

		std::vector<vk::UniqueShaderModule> shaderModules;
		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
		std::vector<vk::RayTracingShaderGroupCreateInfoKHR> shaderGroups;

		vk::UniqueDescriptorPool descPool;
		vk::UniqueDescriptorSetLayout descSetLayout;
		vk::UniqueDescriptorSet descSet;

		vk::UniquePipeline m_pipeline;
		vk::UniquePipelineLayout m_pipelineLayout;

		Buffer sbt{};
		vk::StridedDeviceAddressRegionKHR raygenRegion{};
		vk::StridedDeviceAddressRegionKHR missRegion{};
		vk::StridedDeviceAddressRegionKHR hitRegion{};
	};
}
