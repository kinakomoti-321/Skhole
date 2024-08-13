#pragma once
#include <include.h>
#include <common/log.h>

namespace VKHelper {

	//--------------------------
	//  Instance & Debugger
	//--------------------------

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessengerCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageTypes,
		VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
		void* pUserData) {
		std::cerr << pCallbackData->pMessage << "\n\n";
		return VK_FALSE;
	}

	//inline static VKAPI_ATTR VkBool32 VKAPI_CALL debugMessengerCallback(
	//	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	//	VkDebugUtilsMessageTypeFlagsEXT messageTypes,
	//	VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
	//	void* pUserData) {

	//	std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

	//	return VK_FALSE;
	//}

	inline bool CheckLayerSupport(const std::vector<const char*>& layer) {
		std::vector<vk::LayerProperties> availableLayers =
			vk::enumerateInstanceLayerProperties();

		for (const char* layerName : layer) {
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


	inline std::vector<const char*> GetRequiredExtensions() {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		return extensions;
	}

	inline vk::DebugUtilsMessengerCreateInfoEXT CreateDebugCreateInfo() {
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

	inline vk::UniqueInstance CreateInstance(
		uint32_t apiVersion,
		const std::vector<const char*>& layers) {
		std::cout << "Create instance\n";

		// Setup dynamic loader
		static vk::DynamicLoader dl;
		auto vkGetInstanceProcAddr =
			dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
		VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

		// Check layer support
		if (!CheckLayerSupport(layers)) {
			std::cerr << "Requested layers not available.\n";
			std::abort();
		}

		// Create instance
		vk::ApplicationInfo appInfo{};
		appInfo.setApiVersion(apiVersion);

		std::vector<const char*> extensions = GetRequiredExtensions();

		vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo =
			CreateDebugCreateInfo();

		vk::InstanceCreateInfo instanceCreateInfo{};
		instanceCreateInfo.setPApplicationInfo(&appInfo);
		instanceCreateInfo.setPEnabledLayerNames(layers);
		instanceCreateInfo.setPEnabledExtensionNames(extensions);
		instanceCreateInfo.setPNext(&debugCreateInfo);
		vk::UniqueInstance instance = vk::createInstanceUnique(instanceCreateInfo);
		VULKAN_HPP_DEFAULT_DISPATCHER.init(*instance);
		return instance;
	}
	//inline vk::DebugUtilsMessengerCreateInfoEXT CreateDebugMessengerCreateInfo() {
	//	vk::DebugUtilsMessengerCreateInfoEXT createInfo;
	//	createInfo.setMessageSeverity(
	//		vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
	//		vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
	//	);
	//	createInfo.setMessageType(
	//		vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
	//		vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
	//		vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
	//	);
	//	createInfo.setPfnUserCallback(debugMessengerCallback);
	//	return createInfo;
	//}

	//inline vk::UniqueInstance CreateInstance(uint32_t vkVersion, const std::vector<const char*>& layer) {
	//	SKHOLE_LOG("Create VK Instance");

	//	static vk::DynamicLoader dl;
	//	auto vkGetInstanceProcAddr =
	//		dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
	//	VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

	//	if (!CheckLayerSupport(layer)) {
	//		SKHOLE_ABORT("Layer not supported");
	//	}

	//	vk::ApplicationInfo appInfo;
	//	appInfo.setApiVersion(vkVersion);

	//	vk::InstanceCreateInfo createInfo;
	//	createInfo.pApplicationInfo = &appInfo;

	//	auto extensions = GetRequiredExtensions();
	//	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	//	createInfo.ppEnabledExtensionNames = extensions.data();

	//	vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo = CreateDebugMessengerCreateInfo();
	//	createInfo.pNext = &debugCreateInfo;

	//	vk::UniqueInstance instance = vk::createInstanceUnique(createInfo);

	//	if (!(instance)) {
	//		SKHOLE_ABORT("Failed to create instance");
	//	}

	//	VULKAN_HPP_DEFAULT_DISPATCHER.init(*instance);

	//	return instance;
	//}

	//inline vk::UniqueDebugUtilsMessengerEXT CreateDebugMessenger(vk::Instance instance) {
	//	SKHOLE_LOG("Create VK Debug Messanger");
	//	return instance.createDebugUtilsMessengerEXTUnique(CreateDebugMessengerCreateInfo());
	//}

	inline vk::UniqueDebugUtilsMessengerEXT CreateDebugMessenger(
		vk::Instance instance) {
		std::cout << "Create debug messenger\n";
		return instance.createDebugUtilsMessengerEXTUnique(CreateDebugCreateInfo());
	}

	//--------------------------
	//  Surface
	//--------------------------
	inline vk::UniqueSurfaceKHR CreateSurface(vk::Instance instance, GLFWwindow* window) {
		SKHOLE_LOG("Create VK Surface");

		VkSurfaceKHR surface;
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
			SKHOLE_ABORT("Failed to create window surface");
		}

		return vk::UniqueSurfaceKHR{ vk::SurfaceKHR(surface), {instance} };
	}

	//--------------------------
	//  Physical Device
	//--------------------------
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
		if (physicalDevice.getSurfaceFormatsKHR(surface).empty() ||
			physicalDevice.getSurfacePresentModesKHR(surface).empty()) {
			return false;
		}
		return true;
	}

	inline vk::PhysicalDevice PickPhysicalDevice(vk::Instance instance, vk::SurfaceKHR surface, std::vector<const char*> extensions) {
		SKHOLE_LOG("Create VK Physical Device");

		auto physicalDevices = instance.enumeratePhysicalDevices();
		for (auto& device : physicalDevices) {
			if (isDeviceSuitable(device, surface, extensions)) {
				return device;
			}
		}

		SKHOLE_ABORT("Failed to find a suitable GPU");
	}

	//--------------------------
	//  Queue Family
	//--------------------------
	inline uint32_t FindGeneralQueueFamily(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface) {
		SKHOLE_LOG("Find General Queue Family");

		auto queueFamilies = physicalDevice.getQueueFamilyProperties();

		for (int i = 0; i < queueFamilies.size(); i++) {
			vk::Bool32 presentSurpport = physicalDevice.getSurfaceSupportKHR(i, surface);
			if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics && presentSurpport) {
				return i;
			}
		}

		SKHOLE_ABORT("Failed to find a suitable queue family");
	}

	//--------------------------
	//  Logical Device
	//--------------------------
	inline vk::UniqueDevice CreateLogicalDevice(
		vk::PhysicalDevice physicalDevice,
		uint32_t queueFamilyIndex,
		const std::vector<const char*>& deviceExtensions)
	{
		SKHOLE_LOG("Create Logical Device");

		float queuePriority = 1.0f;
		vk::DeviceQueueCreateInfo queueCreateInfo{
			{}, queueFamilyIndex, 1, &queuePriority };

		vk::DeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.setQueueCreateInfos(queueCreateInfo);
		deviceCreateInfo.setPEnabledExtensionNames(deviceExtensions);

		vk::StructureChain createInfoChain{
			deviceCreateInfo,
			vk::PhysicalDeviceRayTracingPipelineFeaturesKHR{},
			vk::PhysicalDeviceAccelerationStructureFeaturesKHR{},
			vk::PhysicalDeviceBufferDeviceAddressFeaturesKHR{},
		};

		vk::UniqueDevice device = physicalDevice.createDeviceUnique(
			createInfoChain.get<vk::DeviceCreateInfo>()
		);

		return device;
	}

	//--------------------------
	//  CommandPool
	//--------------------------
	inline vk::UniqueCommandPool CreateCommandPool(
		vk::Device device,
		uint32_t queueFamilyIndex
	) {
		SKHOLE_LOG("Create Command Pool");
		vk::CommandPoolCreateInfo createInfo; 
		createInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
		createInfo.setQueueFamilyIndex(queueFamilyIndex);
		return device.createCommandPoolUnique(createInfo);
	}


	//--------------------------
	// CommandBuffer
	//--------------------------
	inline vk::UniqueCommandBuffer CreateCommandBuffer(vk::Device device, vk::CommandPool commandPool) {
		SKHOLE_LOG("Create Command Buffer");
		vk::CommandBufferAllocateInfo allocateInfo;
		allocateInfo.setCommandPool(commandPool);
		allocateInfo.setLevel(vk::CommandBufferLevel::ePrimary);
		allocateInfo.setCommandBufferCount(1);
		return std::move(device.allocateCommandBuffersUnique(allocateInfo)[0]); // return pair<commandBuffer, commandAllocator> 
	}

	//--------------------------
	//  Swapchain
	//--------------------------
	inline vk::PresentModeKHR ChoosePresentMode(
		vk::PhysicalDevice physicalDevice,
		vk::SurfaceKHR surface
	) {
		auto availablePresentModes =
			physicalDevice.getSurfacePresentModesKHR(surface);
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == vk::PresentModeKHR::eFifoRelaxed) {
				return availablePresentMode;
			}
		}

		return vk::PresentModeKHR::eFifo;
	}

	inline vk::Extent2D ChooseExtent(
		vk::SurfaceCapabilitiesKHR capabilities,
		uint32_t width,
		uint32_t height
	) {
		if (capabilities.currentExtent.width != UINT32_MAX) {
			return capabilities.currentExtent;
		}

		vk::Extent2D actualExtent = { width, height };
		actualExtent.width = std::clamp(actualExtent.width,
			capabilities.minImageExtent.width,
			capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height,
			capabilities.minImageExtent.height,
			capabilities.maxImageExtent.height);
		return actualExtent;
	}

	inline vk::SurfaceFormatKHR ChooseSurfaceFormat(
		vk::PhysicalDevice physicalDevice,
		vk::SurfaceKHR surface,
		vk::Format format
	) {
		auto availableFormats = physicalDevice.getSurfaceFormatsKHR(surface);
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == format) {
				return availableFormat;
			}
		}
		SKHOLE_ERROR("Failed to find suitable surface format");
		return availableFormats[0];
	}


	inline vk::UniqueSwapchainKHR CreateSwapchain(
		vk::PhysicalDevice physicalDevice,
		vk::Device device,
		vk::SurfaceKHR surface,
		vk::ImageUsageFlagBits usage,
		vk::SurfaceFormatKHR surfaceFormat,
		uint32_t width,
		uint32_t height,
		uint32_t queueFamilyIndex
	)
	{
		SKHOLE_LOG("Create Swapchain");
		vk::SurfaceCapabilitiesKHR capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
		vk::PresentModeKHR presentMode = ChoosePresentMode(physicalDevice, surface);
		vk::Extent2D extent = ChooseExtent(capabilities,width,height);
		
		uint32_t imageCount = capabilities.minImageCount + 1;
		if (capabilities.maxImageCount > 0 && imageCount < capabilities.maxImageCount) {
			imageCount < capabilities.maxImageCount;
		}

		vk::SwapchainCreateInfoKHR createInfo;
		createInfo.setSurface(surface);
		createInfo.setMinImageCount(imageCount);
		createInfo.setImageFormat(surfaceFormat.format);
		createInfo.setImageColorSpace(surfaceFormat.colorSpace);
		createInfo.setImageExtent(extent);
		createInfo.setImageArrayLayers(1);
		createInfo.setImageUsage(usage);
		createInfo.setPreTransform(capabilities.currentTransform);
		createInfo.setPresentMode(presentMode);
		createInfo.setClipped(VK_TRUE);
		createInfo.setQueueFamilyIndices(queueFamilyIndex);

		return device.createSwapchainKHRUnique(createInfo);
	}

	inline void OneTimeSubmit(vk::Device device,vk::CommandPool commandPool,vk::Queue queue, const std::function<void(vk::CommandBuffer)>& func)
	{
		vk::CommandBufferAllocateInfo allocateInfo;
		allocateInfo.setCommandPool(commandPool);
		allocateInfo.setLevel(vk::CommandBufferLevel::ePrimary);
		allocateInfo.setCommandBufferCount(1);
	
		auto commandBuffer = device.allocateCommandBuffersUnique(allocateInfo);

		commandBuffer[0]->begin(vk::CommandBufferBeginInfo{});
		func(commandBuffer[0].get());
		commandBuffer[0]->end();

		vk::UniqueFence fence = device.createFenceUnique(vk::FenceCreateInfo{});

		vk::SubmitInfo submitInfo{};
		submitInfo.setCommandBufferCount(1);
		submitInfo.setPCommandBuffers(&commandBuffer[0].get());
		queue.submit(submitInfo, fence.get());

		if (device.waitForFences(fence.get(), true, std::numeric_limits<uint64_t>::max()) != vk::Result::eSuccess)
		{
			SKHOLE_ABORT("Failed to wait for fence");
		}
	}

	inline void SetImageLayout(vk::CommandBuffer commandBuffer,
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

		commandBuffer.pipelineBarrier(srcStageMask, dstStageMask,
			{}, {}, {}, imageMemoryBarrier);
	}

	//--------------------------
	// Buffer
	//--------------------------
	inline uint32_t GetMemoryType(vk::PhysicalDevice physicalDevice,
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


	//--------------------------
	//  Shader
	//--------------------------
	inline std::vector<char> ReadFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);
		if (!file.is_open()) {
			std::cerr << "Failed to open file!\n";
			std::abort();
		}

		size_t fileSize = file.tellg();
		std::vector<char> buffer(fileSize);
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();
		return buffer;
	}

	inline vk::UniqueShaderModule CreateShaderModule(vk::Device device, const std::string& filepath) {
		SKHOLE_LOG("Create Shader Module : " + filepath);

		auto code = ReadFile(filepath);

		vk::ShaderModuleCreateInfo createInfo;
		createInfo.setCodeSize(code.size());
		createInfo.setPCode(reinterpret_cast<const uint32_t*>(code.data()));
		return device.createShaderModuleUnique(createInfo);
	}

	//--------------------------
	//  RayTracing
	//--------------------------
	inline auto GetRayTracingProp(vk::PhysicalDevice physicalDevice) {
		auto deviceProperties = physicalDevice.getProperties2<
			vk::PhysicalDeviceProperties2,
			vk::PhysicalDeviceRayTracingPipelinePropertiesKHR>();
		return deviceProperties.get<vk::PhysicalDeviceRayTracingPipelinePropertiesKHR>();
	}

	inline uint32_t AlignUp(uint32_t size, uint32_t alignment) {
		return (size + alignment - 1) & ~(alignment - 1);
	}
}
