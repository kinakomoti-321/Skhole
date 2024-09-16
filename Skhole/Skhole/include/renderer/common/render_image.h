#pragma once

#include <include.h>
#include <vulkan_helpler/vk_buffer.h>

namespace Skhole {
	class RenderImages {
	public:
		RenderImages() {}
		~RenderImages() {}

		void Initialize(uint32_t rw, uint32_t rh, vk::Device device, vk::PhysicalDevice physicalDevice, vk::CommandPool commandPool, vk::Queue queue)
		{
			width = rw;
			height = rh;

			CreateImage(device, physicalDevice, commandPool, queue);
		}

		void Resize(uint32_t rw, uint32_t rh, vk::Device device, vk::PhysicalDevice physicalDevice, vk::CommandPool commandPool, vk::Queue queue) {
			Release(device);
			width = rw;
			height = rh;

			CreateImage(device, physicalDevice, commandPool, queue);
		}

		void Release(vk::Device device) {
			renderImage.Release(device);
			postProcessedImage.Release(device);
		}

		//TODO
		void WritePNG();

		uint32_t GetWidth() { return width; }
		uint32_t GetHeight() { return height; }

		Image& GetAccumImage() { return accumImage; }
		Image& GetRenderImage() { return renderImage; }
		Image& GetPostProcessedImage() { return postProcessedImage; }

	private:

		void CreateImage(vk::Device device, vk::PhysicalDevice physicalDevice, vk::CommandPool commandPool, vk::Queue queue)
		{
			accumImage.Init(
				physicalDevice, device,
				width, height,
				vk::Format::eR32G32B32A32Sfloat,
				vk::ImageTiling::eOptimal,
				vk::ImageUsageFlagBits::eStorage,
				vk::MemoryPropertyFlagBits::eDeviceLocal
			);

			renderImage.Init(
				physicalDevice, device,
				width, height,
				vk::Format::eR32G32B32A32Sfloat,
				vk::ImageTiling::eOptimal,
				vk::ImageUsageFlagBits::eStorage,
				vk::MemoryPropertyFlagBits::eDeviceLocal
			);

			postProcessedImage.Init(
				physicalDevice, device,
				width, height,
				vk::Format::eR8G8B8A8Unorm,
				vk::ImageTiling::eOptimal,
				vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc,
				vk::MemoryPropertyFlagBits::eDeviceLocal
			);

			vkutils::oneTimeSubmit(device, commandPool, queue,
				[&](vk::CommandBuffer commandBuffer) {
					vkutils::setImageLayout(
						commandBuffer, accumImage.GetImage(),
						vk::ImageLayout::eUndefined,
						vk::ImageLayout::eGeneral
					);
					vkutils::setImageLayout(
						commandBuffer, renderImage.GetImage(),
						vk::ImageLayout::eUndefined,
						vk::ImageLayout::eGeneral
					);
					vkutils::setImageLayout(
						commandBuffer, postProcessedImage.GetImage(),
						vk::ImageLayout::eUndefined,
						vk::ImageLayout::eGeneral
					);
				}
			);
		}

		// Image
		Image accumImage;
		Image renderImage;
		Image postProcessedImage;

		uint32_t width, height;
	};
}
