#pragma once

#include <include.h>
#include <common/log.h>

namespace vkutils {
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessengerCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageTypes,
		VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
		void* pUserData) {
		std::cerr << pCallbackData->pMessage << "\n\n";
		return VK_FALSE;
	}

	inline bool checkLayerSupport(const std::vector<const char*>& layers) {
		std::vector<vk::LayerProperties> availableLayers =
			vk::enumerateInstanceLayerProperties();

		for (const char* layerName : layers) {
			bool layerFound = false;
			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}
			if (!layerFound) {
				return false;
			}
		}
		return true;
	}

	inline std::vector<const char*> getRequiredExtensions() {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions =
			glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions,
			glfwExtensions + glfwExtensionCount);
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		return extensions;
	}

	inline vk::DebugUtilsMessengerCreateInfoEXT createDebugCreateInfo() {
		vk::DebugUtilsMessengerCreateInfoEXT createInfo{};
		createInfo.setMessageSeverity(
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
		createInfo.setMessageType(
			vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
			vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
			vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
		createInfo.setPfnUserCallback(&debugUtilsMessengerCallback);
		return createInfo;
	}

	inline vk::UniqueInstance createInstance(
		uint32_t apiVersion,
		const std::vector<const char*>& layers) {
		SKHOLE_LOG("Create Vulkan Instance");

		// Setup dynamic loader
		static vk::DynamicLoader dl;
		auto vkGetInstanceProcAddr =
			dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
		VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

		// Check layer support
		if (!checkLayerSupport(layers)) {
			std::cerr << "Requested layers not available.\n";
			std::abort();
		}

		// Create instance
		vk::ApplicationInfo appInfo{};
		appInfo.setApiVersion(apiVersion);

		std::vector<const char*> extensions = getRequiredExtensions();

		vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo =
			createDebugCreateInfo();

		vk::InstanceCreateInfo instanceCreateInfo{};
		instanceCreateInfo.setPApplicationInfo(&appInfo);
		instanceCreateInfo.setPEnabledLayerNames(layers);
		instanceCreateInfo.setPEnabledExtensionNames(extensions);
		instanceCreateInfo.setPNext(&debugCreateInfo);
		vk::UniqueInstance instance = vk::createInstanceUnique(instanceCreateInfo);
		VULKAN_HPP_DEFAULT_DISPATCHER.init(*instance);
		return instance;
	}

	inline vk::UniqueDebugUtilsMessengerEXT createDebugMessenger(
		vk::Instance instance) {

		SKHOLE_LOG("Create Vulkan DebugMessenger");

		return instance.createDebugUtilsMessengerEXTUnique(createDebugCreateInfo());
	}

	inline vk::UniqueSurfaceKHR createSurface(vk::Instance instance,
		GLFWwindow* window) {
		SKHOLE_LOG("Create Vulkan Surface");
		VkSurfaceKHR _surface;
		if (glfwCreateWindowSurface(instance, window, nullptr, &_surface) !=
			VK_SUCCESS) {
			std::cerr << "Failed to create window surface.\n";
			std::abort();
		}
		return vk::UniqueSurfaceKHR{ vk::SurfaceKHR(_surface), {instance} };
	}

	inline uint32_t findGeneralQueueFamily(vk::PhysicalDevice physicalDevice,
		vk::SurfaceKHR surface = VK_NULL_HANDLE) {
		auto queueFamilies = physicalDevice.getQueueFamilyProperties();
		for (uint32_t i = 0; i < queueFamilies.size(); i++) {
			vk::Bool32 presentSupport;
			if (VK_NULL_HANDLE == surface)  presentSupport = true;
			else presentSupport = physicalDevice.getSurfaceSupportKHR(i, surface);

			if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics &&
				presentSupport) {
				return i;
			}
		}
		std::cerr << "Failed to find general queue family.\n";
		std::abort();
	}

	inline bool checkDeviceExtensionSupport(
		vk::PhysicalDevice device,
		const std::vector<const char*>& deviceExtensions) {
		std::set<std::string> requiredExtensions{ deviceExtensions.begin(),
												 deviceExtensions.end() };
		for (const auto& extension : device.enumerateDeviceExtensionProperties()) {
			requiredExtensions.erase(extension.extensionName);
		}
		return requiredExtensions.empty();
	}

	inline bool isDeviceSuitable(vk::PhysicalDevice physicalDevice,
		vk::SurfaceKHR surface,
		const std::vector<const char*>& deviceExtensions) {
		if (!checkDeviceExtensionSupport(physicalDevice, deviceExtensions)) {
			return false;
		}
		if (surface != VK_NULL_HANDLE) {
			if (physicalDevice.getSurfaceFormatsKHR(surface).empty() ||
				physicalDevice.getSurfacePresentModesKHR(surface).empty()) {
				return false;
			}
		}
		return true;
	}

	inline vk::PhysicalDevice pickPhysicalDevice(
		vk::Instance instance,
		vk::SurfaceKHR surface,
		const std::vector<const char*>& deviceExtensions) {
		// Select suitable physical device
		for (const auto& device : instance.enumeratePhysicalDevices()) {
			if (isDeviceSuitable(device, surface, deviceExtensions)) {
				return device;
			}
		}

		SKHOLE_ABORT("not found physical device");
		std::abort();
	}

	inline auto getRayTracingProps(vk::PhysicalDevice physicalDevice) {
		auto deviceProperties = physicalDevice.getProperties2<
			vk::PhysicalDeviceProperties2,
			vk::PhysicalDeviceRayTracingPipelinePropertiesKHR>();
		return deviceProperties
			.get<vk::PhysicalDeviceRayTracingPipelinePropertiesKHR>();
	}

	inline void EnableFeatures(const std::vector<const char*>& extensions, std::vector<ShrPtr<void>>& extensionChain) {
		for (auto& ex : extensions) {
			if (ex == VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME) {
				extensionChain.push_back(MakeShr<vk::PhysicalDeviceSynchronization2FeaturesKHR>(VK_TRUE));
			}
			else if (ex == VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME) {
				extensionChain.push_back(MakeShr<vk::PhysicalDeviceRayTracingPipelineFeaturesKHR>(VK_TRUE));
			}
			else if (ex == VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME) {
				extensionChain.push_back(MakeShr<vk::PhysicalDeviceAccelerationStructureFeaturesKHR>(VK_TRUE));
			}
			else if (ex == VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME) {
				extensionChain.push_back(MakeShr<vk::PhysicalDeviceBufferDeviceAddressFeatures>(VK_TRUE));
			}
			else if (ex == VK_KHR_RELAXED_BLOCK_LAYOUT_EXTENSION_NAME) {
				//extensionChain.push_back(MakeShr<vk::PhysicalDeviceRelaxedBlockLayout)
				SKHOLE_WARN("NOT IMPLE");
			}
			else if (ex == VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME) {
				extensionChain.push_back(MakeShr<vk::PhysicalDeviceScalarBlockLayoutFeatures>(VK_TRUE));
			}
		}

		void* lastPNext = nullptr;
		for (auto& ex : extensionChain)
		{
			if (lastPNext == nullptr) {
				lastPNext = ex.get();
			}
			else {
				reinterpret_cast<vk::BaseOutStructure*>(lastPNext)->pNext = reinterpret_cast<vk::BaseOutStructure*>(ex.get());
				lastPNext = ex.get();
			}
		}
	}

	inline vk::UniqueDevice createLogicalDevice(
		vk::PhysicalDevice physicalDevice,
		uint32_t queueFamilyIndex,
		const std::vector<const char*>& deviceExtensions) {

		SKHOLE_LOG("Create Vulkan LogicalDevice");

		float queuePriority = 1.0f;
		vk::DeviceQueueCreateInfo queueCreateInfo{
			{}, queueFamilyIndex, 1, &queuePriority };

		vk::DeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.setQueueCreateInfos(queueCreateInfo);
		deviceCreateInfo.setEnabledExtensionCount(deviceExtensions.size());
		deviceCreateInfo.setPEnabledExtensionNames(deviceExtensions);
		deviceCreateInfo.setFlags(vk::DeviceCreateFlagBits(0));

		SKHOLE_LOG("End Create Vulkan LogicalDevice");


		//vk::StructureChain createInfoChain{
		//	deviceCreateInfo,
		//	vk::PhysicalDeviceRayTracingPipelineFeaturesKHR{VK_TRUE},
		//	vk::PhysicalDeviceAccelerationStructureFeaturesKHR{VK_TRUE},
		//	vk::PhysicalDeviceBufferDeviceAddressFeatures{VK_TRUE},
		//	vk::PhysicalDeviceSynchronization2FeaturesKHR{VK_TRUE},
		//};

		std::vector<ShrPtr<void>> extensionChain;
		//extensionChain.push_back(MakeShr<vk::PhysicalDeviceRayTracingPipelineFeaturesKHR>(VK_TRUE));
		//extensionChain.push_back(MakeShr<vk::PhysicalDeviceAccelerationStructureFeaturesKHR>(VK_TRUE));
		//extensionChain.push_back(MakeShr<vk::PhysicalDeviceBufferDeviceAddressFeatures>(VK_TRUE));
		//extensionChain.push_back(MakeShr<vk::PhysicalDeviceSynchronization2FeaturesKHR>(VK_TRUE));

		//void* lastPNext = nullptr;
		//for (auto& ex : extensionChain)
		//{
		//	if (lastPNext == nullptr) {
		//		lastPNext = ex.get();
		//	}
		//	else {
		//		reinterpret_cast<vk::BaseOutStructure*>(lastPNext)->pNext = reinterpret_cast<vk::BaseOutStructure*>(ex.get());
		//		lastPNext = ex.get();
		//	}
		//}

		EnableFeatures(deviceExtensions, extensionChain);

		deviceCreateInfo.pNext = extensionChain[0].get();

		//deviceCreateInfo.pNext(&ex1);

		vk::UniqueDevice device = physicalDevice.createDeviceUnique(
			deviceCreateInfo);
		VULKAN_HPP_DEFAULT_DISPATCHER.init(device.get());

		SKHOLE_LOG("End Create Vulkan LogicalDevice");

		return device;
	}

	inline vk::SurfaceFormatKHR chooseSurfaceFormat(
		vk::PhysicalDevice physicalDevice,
		vk::SurfaceKHR surface) {
		auto availableFormats = physicalDevice.getSurfaceFormatsKHR(surface);
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == vk::Format::eR8G8B8A8Unorm) {
				return availableFormat;
			}
		}
		return availableFormats[0];
	}

	inline vk::PresentModeKHR choosePresentMode(vk::PhysicalDevice physicalDevice,
		vk::SurfaceKHR surface) {
		auto availablePresentModes =
			physicalDevice.getSurfacePresentModesKHR(surface);
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == vk::PresentModeKHR::eFifoRelaxed) {
				return availablePresentMode;
			}
		}

		return vk::PresentModeKHR::eFifo;
	}

	inline vk::Extent2D chooseExtent(vk::SurfaceCapabilitiesKHR capabilities,
		uint32_t width,
		uint32_t height) {
		if (capabilities.currentExtent.width != UINT32_MAX) {
			return capabilities.currentExtent;
		}

		vk::Extent2D actualExtent = { width, height };
		actualExtent.width = std::clamp(actualExtent.width,                 //
			capabilities.minImageExtent.width,  //
			capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height,                 //
			capabilities.minImageExtent.height,  //
			capabilities.maxImageExtent.height);
		return actualExtent;
	}

	inline vk::UniqueSwapchainKHR createSwapchain(
		vk::PhysicalDevice physicalDevice,
		vk::Device device,
		vk::SurfaceKHR surface,
		uint32_t queueFamilyIndex,
		vk::ImageUsageFlags usage,
		vk::SurfaceFormatKHR surfaceFormat,
		uint32_t width,
		uint32_t height) {

		SKHOLE_LOG("Create Swapchain");

		vk::SurfaceCapabilitiesKHR capabilities =
			physicalDevice.getSurfaceCapabilitiesKHR(surface);
		vk::PresentModeKHR presentMode = choosePresentMode(physicalDevice, surface);
		vk::Extent2D extent = chooseExtent(capabilities, width, height);

		uint32_t imageCount = capabilities.minImageCount + 1;
		if (capabilities.maxImageCount > 0 &&
			imageCount > capabilities.maxImageCount) {
			imageCount = capabilities.maxImageCount;
		}

		vk::SwapchainCreateInfoKHR createInfo{};

		createInfo.setSurface(surface);
		createInfo.setMinImageCount(imageCount);
		createInfo.setImageFormat(surfaceFormat.format);
		createInfo.setImageColorSpace(surfaceFormat.colorSpace);
		createInfo.setImageExtent(extent);
		createInfo.setImageArrayLayers(1);
		createInfo.setImageUsage(usage);
		createInfo.setQueueFamilyIndices(nullptr);
		createInfo.setPreTransform(capabilities.currentTransform);
		createInfo.setPresentMode(presentMode);
		createInfo.setClipped(VK_TRUE);
		createInfo.setQueueFamilyIndices(queueFamilyIndex);


		return device.createSwapchainKHRUnique(createInfo);
	}

	inline uint32_t getMemoryType(vk::PhysicalDevice physicalDevice,
		vk::MemoryRequirements memoryRequirements,
		vk::MemoryPropertyFlags memoryProperties) {
		auto physicalDeviceMemoryProperties = physicalDevice.getMemoryProperties();
		for (uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; ++i) {
			if (memoryRequirements.memoryTypeBits & (1 << i)) {
				if ((physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags &
					memoryProperties) == memoryProperties) {
					return i;
				}
			}
		}

		std::cerr << "Failed to get memory type index.\n";
		std::abort();
	}

	inline vk::UniqueCommandPool createCommandPool(vk::Device device,
		uint32_t queueFamilyIndex) {
		vk::CommandPoolCreateInfo commandPoolCreateInfo{};
		commandPoolCreateInfo.setFlags(
			vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
		commandPoolCreateInfo.setQueueFamilyIndex(queueFamilyIndex);
		return device.createCommandPoolUnique(commandPoolCreateInfo);
	}

	inline void oneTimeSubmit(vk::Device device,
		vk::CommandPool commandPool,
		vk::Queue queue,
		const std::function<void(vk::CommandBuffer)>& func) {
		// Allocate
		vk::CommandBufferAllocateInfo allocateInfo{};
		allocateInfo.setCommandPool(commandPool);
		allocateInfo.setLevel(vk::CommandBufferLevel::ePrimary);
		allocateInfo.setCommandBufferCount(1);
		auto commandBuffers = device.allocateCommandBuffersUnique(allocateInfo);

		// Record
		commandBuffers[0]->begin(vk::CommandBufferBeginInfo{});
		func(commandBuffers[0].get());
		commandBuffers[0]->end();

		// Submit
		vk::UniqueFence fence = device.createFenceUnique({});
		vk::SubmitInfo submitInfo{};
		submitInfo.setCommandBuffers(commandBuffers[0].get());
		queue.submit(submitInfo, fence.get());

		// Wait
		if (device.waitForFences(fence.get(), true,
			std::numeric_limits<uint64_t>::max()) !=
			vk::Result::eSuccess) {
			SKHOLE_ABORT("Failed to wait for fence");
			std::abort();
		}
	}

	inline vk::UniqueCommandBuffer createCommandBuffer(
		vk::Device device,
		vk::CommandPool commandPool) {
		vk::CommandBufferAllocateInfo allocateInfo{};
		allocateInfo.setCommandPool(commandPool);
		allocateInfo.setLevel(vk::CommandBufferLevel::ePrimary);
		allocateInfo.setCommandBufferCount(1);
		return std::move(device.allocateCommandBuffersUnique(allocateInfo).front());
	}

	inline std::vector<char> readFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);
		if (!file.is_open()) {
			SKHOLE_ABORT("Not Found File : " + filename);
			std::abort();
		}

		size_t fileSize = file.tellg();
		std::vector<char> buffer(fileSize);
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();
		return buffer;
	}

	inline vk::UniqueShaderModule createShaderModule(vk::Device device,
		const std::string& filename) {
		const std::vector<char> code = readFile(filename);
		vk::ShaderModuleCreateInfo createInfo{};
		createInfo.setCodeSize(code.size());
		createInfo.setPCode(reinterpret_cast<const uint32_t*>(code.data()));
		return device.createShaderModuleUnique(createInfo);
	}

	inline void setImageLayout(vk::CommandBuffer commandBuffer,
		vk::Image image,
		vk::ImageLayout oldImageLayout,
		vk::ImageLayout newImageLayout,
		vk::ImageSubresourceRange subresourceRange =
		{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 },
		vk::PipelineStageFlags srcStageMask =
		vk::PipelineStageFlagBits::eAllCommands,
		vk::PipelineStageFlags dstStageMask =
		vk::PipelineStageFlagBits::eAllCommands) {
		vk::ImageMemoryBarrier imageMemoryBarrier{};
		imageMemoryBarrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
		imageMemoryBarrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
		imageMemoryBarrier.setImage(image);
		imageMemoryBarrier.setOldLayout(oldImageLayout);
		imageMemoryBarrier.setNewLayout(newImageLayout);
		imageMemoryBarrier.setSubresourceRange(subresourceRange);

		// Source layouts (old)
		switch (oldImageLayout) {
		case vk::ImageLayout::eUndefined:
			imageMemoryBarrier.srcAccessMask = {};
			break;
		case vk::ImageLayout::ePreinitialized:
			imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eHostWrite;
			break;
		case vk::ImageLayout::eColorAttachmentOptimal:
			imageMemoryBarrier.srcAccessMask =
				vk::AccessFlagBits::eColorAttachmentWrite;
			break;
		case vk::ImageLayout::eDepthStencilAttachmentOptimal:
			imageMemoryBarrier.srcAccessMask =
				vk::AccessFlagBits::eDepthStencilAttachmentWrite;
			break;
		case vk::ImageLayout::eTransferSrcOptimal:
			imageMemoryBarrier.srcAccessMask =
				vk::AccessFlagBits::eTransferRead;
			break;
		case vk::ImageLayout::eTransferDstOptimal:
			imageMemoryBarrier.srcAccessMask =
				vk::AccessFlagBits::eTransferWrite;
			break;
		case vk::ImageLayout::eShaderReadOnlyOptimal:
			imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
			break;
		default:
			break;
		}

		// Target layouts (new)
		switch (newImageLayout) {
		case vk::ImageLayout::eTransferDstOptimal:
			imageMemoryBarrier.dstAccessMask =
				vk::AccessFlagBits::eTransferWrite;
			break;
		case vk::ImageLayout::eTransferSrcOptimal:
			imageMemoryBarrier.dstAccessMask =
				vk::AccessFlagBits::eTransferRead;
			break;
		case vk::ImageLayout::eColorAttachmentOptimal:
			imageMemoryBarrier.dstAccessMask =
				vk::AccessFlagBits::eColorAttachmentWrite;
			break;
		case vk::ImageLayout::eDepthStencilAttachmentOptimal:
			imageMemoryBarrier.dstAccessMask =
				imageMemoryBarrier.dstAccessMask |
				vk::AccessFlagBits::eDepthStencilAttachmentWrite;
			break;
		case vk::ImageLayout::eShaderReadOnlyOptimal:
			if (imageMemoryBarrier.srcAccessMask == vk::AccessFlags{}) {
				imageMemoryBarrier.srcAccessMask =
					vk::AccessFlagBits::eHostWrite |
					vk::AccessFlagBits::eTransferWrite;
			}
			imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
			break;
		default:
			break;
		}

		// コマンドバッファにバリアを積む
		commandBuffer.pipelineBarrier(srcStageMask, dstStageMask,  //
			{}, {}, {}, imageMemoryBarrier);
	}

	inline uint32_t alignUp(uint32_t size, uint32_t alignment) {
		return (size + alignment - 1) & ~(alignment - 1);
	}
}  // namespace vkutils
