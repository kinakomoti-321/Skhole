#pragma once
#include <include.h>
#include <vulkan_helpler/vkutils.hpp>

namespace VkHelper {
	inline vk::UniqueRenderPass CreateRenderPass(
		vk::Format colorFormat,
		vk::ImageLayout initialLayout,
		vk::ImageLayout finalLayout,
		vk::Device device
	)
	{
		vk::AttachmentDescription colorAttachment = {};
		colorAttachment.setFormat(colorFormat);
		colorAttachment.setSamples(vk::SampleCountFlagBits::e1);
		colorAttachment.setLoadOp(vk::AttachmentLoadOp::eLoad);
		colorAttachment.setStoreOp(vk::AttachmentStoreOp::eStore);
		colorAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
		colorAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
		colorAttachment.setInitialLayout(initialLayout);
		colorAttachment.setFinalLayout(finalLayout);

		vk::AttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.setAttachment(0);
		colorAttachmentRef.setLayout(initialLayout);

		vk::SubpassDescription subpass = {};
		subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
		subpass.setColorAttachmentCount(1);
		subpass.setPColorAttachments(&colorAttachmentRef);

		vk::RenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.setAttachmentCount(1);
		renderPassInfo.setPAttachments(&colorAttachment);
		renderPassInfo.setSubpassCount(1);
		renderPassInfo.setPSubpasses(&subpass);

		return device.createRenderPassUnique(renderPassInfo);
	}

	struct BindingLayoutElement {
		uint32_t bindNunber;
		vk::DescriptorType descriptorType;
		uint32_t descriptorCount;
		vk::ShaderStageFlagBits stageFlags;
	};

	struct BindingManager {
		vk::DescriptorPool descriptorPool;
		vk::DescriptorSetLayout descriptorSetLayout;
		vk::DescriptorSet descriptorSet;

		std::vector<BindingLayoutElement> bindings;

		void SetBindingLayout(vk::Device device, const std::vector<BindingLayoutElement>& binding, vk::DescriptorPoolCreateFlagBits frag) {
			bindings = binding;
			SetLayout(device);
			SetPool(frag, device);
			SetDescriptorSet(device);
		}

		void SetLayout(vk::Device device) {
			std::vector<vk::DescriptorSetLayoutBinding> layoutBindings;
			for (auto& binding : bindings) {
				vk::DescriptorSetLayoutBinding layoutBinding = {};
				layoutBinding.setBinding(binding.bindNunber);
				layoutBinding.setDescriptorType(binding.descriptorType);
				layoutBinding.setDescriptorCount(binding.descriptorCount);
				layoutBinding.setStageFlags(binding.stageFlags);
				layoutBindings.push_back(layoutBinding);
			}

			vk::DescriptorSetLayoutCreateInfo createInfo = {};
			createInfo.setBindings(layoutBindings);
			descriptorSetLayout = device.createDescriptorSetLayout(createInfo);
		}

		void SetPool(vk::DescriptorPoolCreateFlagBits frag, vk::Device device) {
			std::vector<vk::DescriptorPoolSize> poolSizes;
			std::map<vk::DescriptorType, uint32_t> typeMap;
			for (auto& binding : bindings) {
				typeMap[binding.descriptorType] += binding.descriptorCount;
			}

			for (auto& [type, count] : typeMap) {
				poolSizes.push_back({ type, count });
			}

			vk::DescriptorPoolCreateInfo poolInfo = {};
			poolInfo.setPoolSizes(poolSizes);
			poolInfo.setMaxSets(1);
			poolInfo.setFlags(frag);

			descriptorPool = device.createDescriptorPool(poolInfo);
		}

		void SetDescriptorSet(vk::Device device) {
			vk::DescriptorSetAllocateInfo allocInfo = {};
			allocInfo.setDescriptorPool(descriptorPool);
			allocInfo.setDescriptorSetCount(1);
			allocInfo.setPSetLayouts(&descriptorSetLayout);

			descriptorSet = device.allocateDescriptorSets(allocInfo)[0];
		}


		void StartWriting() {
			writeBufferInfo.clear();
			writeImageInfo.clear();
			writeASInfo.clear();
			writeDescriptorSets.clear();
		}

		void WriteAS(vk::AccelerationStructureKHR& as, uint32_t bindNumber, uint32_t descriptorCount, vk::Device device) {
			auto asInfo = MakeShr<vk::WriteDescriptorSetAccelerationStructureKHR>();
			asInfo->setAccelerationStructures(as);
			writeASInfo.push_back(asInfo);

			vk::WriteDescriptorSet writeDescriptorSet = {};
			writeDescriptorSet.setDstSet(descriptorSet);
			writeDescriptorSet.setDstBinding(bindNumber);
			writeDescriptorSet.setDescriptorCount(descriptorCount);
			writeDescriptorSet.setDescriptorType(vk::DescriptorType::eAccelerationStructureKHR);
			writeDescriptorSet.setPNext(asInfo.get());

			writeDescriptorSets.push_back(writeDescriptorSet);
		}

		void WriteBuffer(vk::Buffer buffer, uint32_t offset, uint32_t range,
			vk::DescriptorType type, uint32_t bindNumber, uint32_t descriptorCount, vk::Device device) {

			auto bufferInfo = MakeShr<vk::DescriptorBufferInfo>();
			bufferInfo->setBuffer(buffer);
			bufferInfo->setOffset(0);
			bufferInfo->setRange(range);

			writeBufferInfo.push_back(bufferInfo);


			vk::WriteDescriptorSet writeDescriptorSet = {};
			writeDescriptorSet.setDstSet(descriptorSet);
			writeDescriptorSet.setDstBinding(bindNumber);
			writeDescriptorSet.setDescriptorCount(descriptorCount);
			writeDescriptorSet.setDescriptorType(type);
			writeDescriptorSet.setPBufferInfo(bufferInfo.get());

			writeDescriptorSets.push_back(writeDescriptorSet);
		}

		void WriteImage(vk::ImageView imageView, vk::ImageLayout layout, vk::Sampler sampler,
			vk::DescriptorType type, uint32_t bindNumber, uint32_t descriptorCount, vk::Device device) {

			auto imageInfo = MakeShr<vk::DescriptorImageInfo>();
			imageInfo->setImageView(imageView);
			imageInfo->setImageLayout(layout);
			if (VK_NULL_HANDLE != sampler) imageInfo->setSampler(sampler);

			writeImageInfo.push_back(imageInfo);

			vk::WriteDescriptorSet writeDescriptorSet = {};
			writeDescriptorSet.setDstSet(descriptorSet);
			writeDescriptorSet.setDstBinding(bindNumber);
			writeDescriptorSet.setDescriptorCount(descriptorCount);
			writeDescriptorSet.setDescriptorType(type);
			writeDescriptorSet.setPImageInfo(imageInfo.get());

			writeDescriptorSets.push_back(writeDescriptorSet);
		}

		void EndWriting(vk::Device device) {
			device.updateDescriptorSets(writeDescriptorSets, nullptr);

			writeDescriptorSets.clear();
			writeBufferInfo.clear();
			writeImageInfo.clear();
			writeASInfo.clear();
		}

		void Release(vk::Device device) {
			device.destroyDescriptorSetLayout(descriptorSetLayout);
			device.destroyDescriptorPool(descriptorPool);
		}

	private:
		std::vector<ShrPtr<vk::DescriptorBufferInfo>> writeBufferInfo;
		std::vector<ShrPtr<vk::DescriptorImageInfo>> writeImageInfo;
		std::vector<ShrPtr<vk::WriteDescriptorSetAccelerationStructureKHR>> writeASInfo;

		std::vector<vk::WriteDescriptorSet> writeDescriptorSets;
	};


	class Context {
	public:
		Context() {};
		~Context() {};

		struct VulkanInitialzeInfo {
			VulkanInitialzeInfo() {}
			uint32_t apiVersion;
			std::vector<const char*> layers;
			std::vector<const char*> extensions;

			//Window;
			bool useWindow;
			GLFWwindow* window;
		};

		void InitCore(const VulkanInitialzeInfo& info) {
			SKHOLE_LOG("... Initialization Vulkan Context");
			instance = vkutils::createInstance(VK_API_VERSION_1_2, info.layers);
			debugMessage = vkutils::createDebugMessenger(*instance);

			if (info.useWindow) {
				surface = vkutils::createSurface(*instance, info.window);
			}

			physicalDevice = vkutils::pickPhysicalDevice(*instance, *surface, info.extensions);
			queueIndex = vkutils::findGeneralQueueFamily(physicalDevice, *surface);
			device = vkutils::createLogicalDevice(physicalDevice, queueIndex, info.extensions);
			queue = device->getQueue(queueIndex, 0);
			SKHOLE_LOG("... End Initialization Vulkan Context");
		}

		vk::UniqueInstance instance;
		vk::UniqueDebugUtilsMessengerEXT debugMessage;
		vk::UniqueSurfaceKHR surface;

		vk::PhysicalDevice physicalDevice;
		vk::UniqueDevice device;

		vk::Queue queue;
		uint32_t queueIndex;
	};

	struct SwapChainInfo {
		vk::PhysicalDevice physicalDevice;
		vk::Device device;
		vk::SurfaceKHR surface;
		uint32_t queueIndex;
		vk::Queue queue;
		vk::RenderPass renderPass;
		vk::CommandPool commandPool;

		vk::ImageUsageFlags swapcahinImageUsage;

		uint32_t width;
		uint32_t height;
	};

	struct ScreenContext {
		ScreenContext() {}
		~ScreenContext() {}

		void Init(const SwapChainInfo& info) {
			SKHOLE_LOG("... Initialization Vulkan Screen")
				surfaceFormat = vkutils::chooseSurfaceFormat(info.physicalDevice, info.surface);

			swapchain = vkutils::createSwapchain(
				info.physicalDevice, info.device, info.surface, info.queueIndex,
				info.swapcahinImageUsage, surfaceFormat,  //
				info.width, info.height);

			swapchainImages = info.device.getSwapchainImagesKHR(*swapchain);

			SKHOLE_LOG("Create Swapchain Image View");
			for (auto& image : swapchainImages) {

				vk::ImageViewCreateInfo createInfo{};
				createInfo.setImage(image);
				createInfo.setViewType(vk::ImageViewType::e2D);
				createInfo.setFormat(surfaceFormat.format);
				createInfo.setComponents(
					{ vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG,
						vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA });
				createInfo.setSubresourceRange(
					{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });

				swapchainImageViews.push_back(info.device.createImageViewUnique(createInfo));
			}

			vkutils::oneTimeSubmit(info.device, info.commandPool, info.queue,
				[&](vk::CommandBuffer commandBuffer) {
					for (auto& image : swapchainImages) {
						vkutils::setImageLayout(commandBuffer, image,
							vk::ImageLayout::eUndefined,
							vk::ImageLayout::ePresentSrcKHR);
					}
				});


			frameBuffers.resize(swapchainImageViews.size());
			for (int i = 0; i < swapchainImageViews.size(); i++) {
				vk::ImageView attachments[] = {
					*swapchainImageViews[i]
				};

				vk::FramebufferCreateInfo framebufferInfo{};
				framebufferInfo.setRenderPass(info.renderPass);
				framebufferInfo.setAttachmentCount(1);
				framebufferInfo.setPAttachments(attachments);
				framebufferInfo.setWidth(info.width);
				framebufferInfo.setHeight(info.height);
				framebufferInfo.setLayers(1);

				frameBuffers[i] = info.device.createFramebufferUnique(framebufferInfo);
			}

			SKHOLE_LOG("... End Initialization Vulkan Screen")
		}

		int GetFrameIndex(vk::Device device, const vk::Semaphore& semaphore) {
			auto result = device.acquireNextImageKHR(
				*swapchain, std::numeric_limits<uint64_t>::max(),
				semaphore);
			if (result.result != vk::Result::eSuccess) {
				std::cerr << "Failed to acquire next image.\n";
				std::abort();
			}

			return result.value;
		}

		vk::Image GetFrameImage(int index) {
			return swapchainImages[index];
		}

		vk::ImageView GetFrameImageView(int index) {
			return *swapchainImageViews[index];
		}

		vk::Framebuffer GetFrameBuffer(int index) {
			return *frameBuffers[index];
		}

		void Release(vk::Device device) {
			frameBuffers.clear();
			swapchainImageViews.clear();

			device.destroySwapchainKHR(*swapchain);

			swapchainImages.clear();
			*swapchain = VK_NULL_HANDLE;
		}

		vk::SurfaceFormatKHR surfaceFormat;
		vk::UniqueSwapchainKHR swapchain;
		std::vector<vk::Image> swapchainImages;
		std::vector<vk::UniqueImageView> swapchainImageViews;
		std::vector<vk::UniqueFramebuffer> frameBuffers;
	};
};
