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
};
