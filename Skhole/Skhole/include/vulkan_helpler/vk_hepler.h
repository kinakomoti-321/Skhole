#pragma once


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
		std::vector<vk::WriteDescriptorSet> writeDescriptorSets;

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

		struct WritingInfo {
			uint32_t numBuffer = 0;
			uint32_t numImage = 0;
			uint32_t numAS = 0;
		};

		std::vector<vk::DescriptorBufferInfo> writeBufferInfo;
		std::vector<vk::DescriptorImageInfo> writeImageInfo;
		std::vector<vk::WriteDescriptorSetAccelerationStructureKHR> writeASInfo;

		void StartWriting(WritingInfo& info) {
			writeBufferInfo.reserve(info.numBuffer);
			writeImageInfo.reserve(info.numImage);
			writeASInfo.reserve(info.numAS);

			uint32_t sum = info.numBuffer + info.numImage + info.numAS;
			writeDescriptorSets.reserve(sum);
		}

		void WriteAS(vk::AccelerationStructureKHR& as, uint32_t bindNumber, uint32_t descriptorCount, vk::Device device) {
			vk::WriteDescriptorSetAccelerationStructureKHR asInfo = {};
			asInfo.setAccelerationStructures(as);
			writeASInfo.push_back(asInfo);

			vk::WriteDescriptorSet writeDescriptorSet = {};
			writeDescriptorSet.setDstSet(descriptorSet);
			writeDescriptorSet.setDstBinding(bindNumber);
			writeDescriptorSet.setDescriptorCount(descriptorCount);
			writeDescriptorSet.setDescriptorType(vk::DescriptorType::eAccelerationStructureKHR);
			writeDescriptorSet.setPNext(&writeASInfo[writeASInfo.size() - 1]);

			writeDescriptorSets.push_back(writeDescriptorSet);
		}

		void WriteBuffer(vk::Buffer buffer, uint32_t offset, uint32_t range,
			vk::DescriptorType type, uint32_t bindNumber, uint32_t descriptorCount, vk::Device device) {

			vk::DescriptorBufferInfo bufferInfo = {};
			bufferInfo.setBuffer(buffer);
			bufferInfo.setOffset(0);
			bufferInfo.setRange(range);

			writeBufferInfo.push_back(bufferInfo);

			vk::WriteDescriptorSet writeDescriptorSet = {};
			writeDescriptorSet.setDstSet(descriptorSet);
			writeDescriptorSet.setDstBinding(bindNumber);
			writeDescriptorSet.setDescriptorCount(descriptorCount);
			writeDescriptorSet.setDescriptorType(type);
			writeDescriptorSet.setPBufferInfo(&writeBufferInfo[writeBufferInfo.size() - 1]);

			writeDescriptorSets.push_back(writeDescriptorSet);
		}

		void WriteImage(vk::ImageView imageView, vk::ImageLayout layout, vk::Sampler sampler,
			vk::DescriptorType type, uint32_t bindNumber, uint32_t descriptorCount, vk::Device device) {

			vk::DescriptorImageInfo imageInfo = {};
			imageInfo.setImageView(imageView);
			imageInfo.setImageLayout(layout);
			if (VK_NULL_HANDLE != sampler) imageInfo.setSampler(sampler);

			writeImageInfo.push_back(imageInfo);

			vk::WriteDescriptorSet writeDescriptorSet = {};
			writeDescriptorSet.setDstSet(descriptorSet);
			writeDescriptorSet.setDstBinding(bindNumber);
			writeDescriptorSet.setDescriptorCount(descriptorCount);
			writeDescriptorSet.setDescriptorType(type);
			writeDescriptorSet.setPImageInfo(&writeImageInfo[writeImageInfo.size() - 1]);

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
			instance = vkutils::createInstance(VK_API_VERSION_1_2, info.layers);
			debugMessage = vkutils::createDebugMessenger(*instance);

			surface = vkutils::createSurface(*instance, info.window);

			physicalDevice = vkutils::pickPhysicalDevice(*instance, *surface, info.extensions);
			queueIndex = vkutils::findGeneralQueueFamily(physicalDevice, *surface);
			device = vkutils::createLogicalDevice(physicalDevice, queueIndex, info.extensions);
			queue = device->getQueue(queueIndex, 0);
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

	struct SwapchainContext {
		SwapchainContext() {}
		~SwapchainContext() {}

		void Init(const SwapChainInfo& info) {
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
		}

		void Release(vk::Device device) {
			//for (auto& frame : frameBuffers)
			//	device.destroyFramebuffer(*frame);

			//for (auto& view : swapchainImageViews)
			//	device.destroyImageView(*view);
			
			frameBuffers.clear();
			swapchainImageViews.clear();	

			device.destroySwapchainKHR(*swapchain);

			swapchainImages.clear();
			*swapchain = VK_NULL_HANDLE;

			//frameBuffers = std::vector<vk::UniqueFramebuffer>();
			//swapchainImageViews = std::vector<vk::UniqueImageView>();
			//swapchainImages = std::vector<vk::Image>();
			//frameBuffers.clear();
			//swapchainImageViews.clear();	
			//swapchainImages.clear();
		}

		vk::SurfaceFormatKHR surfaceFormat;
		vk::UniqueSwapchainKHR swapchain;
		std::vector<vk::Image> swapchainImages;
		std::vector<vk::UniqueImageView> swapchainImageViews;
		std::vector<vk::UniqueFramebuffer> frameBuffers;
	};
};
