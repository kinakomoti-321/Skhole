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
			accumImage.Release(device);
			renderImage.Release(device);
			postProcessedImage.Release(device);
			copyBuffer.Release(device);
		}

		void ReadBack(vk::CommandBuffer commandBuffer, vk::Device device)
		{

			vkutils::setImageLayout(commandBuffer, postProcessedImage.GetImage(), vk::ImageLayout::eGeneral, vk::ImageLayout::eTransferSrcOptimal);

			vk::BufferImageCopy copyRegion;
			copyRegion.bufferOffset = 0;
			copyRegion.bufferRowLength = 0;
			copyRegion.bufferImageHeight = 0;
			copyRegion.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
			copyRegion.imageSubresource.mipLevel = 0;
			copyRegion.imageSubresource.baseArrayLayer = 0;
			copyRegion.imageSubresource.layerCount = 1;
			copyRegion.imageOffset = vk::Offset3D{ 0, 0, 0 };
			copyRegion.imageExtent = vk::Extent3D{ width, height, 1 };

			commandBuffer.copyImageToBuffer(
				postProcessedImage.GetImage(),
				vk::ImageLayout::eTransferSrcOptimal,
				copyBuffer.GetBuffer(),
				1,
				&copyRegion
			);

			vkutils::setImageLayout(commandBuffer, postProcessedImage.GetImage(), vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eGeneral);

		}

		//TODO
		void WritePNG(const std::string& filepath,const std::string& filename,vk::Device device);
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

			//uchar4
			copyBuffer.Init(
				physicalDevice, device,
				width * height * 4 * sizeof(uint8_t),
				vk::BufferUsageFlagBits::eTransferDst,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
			);

			pixels.resize(width * height * 4 * sizeof(uint8_t));
		}

		// Image
		Image accumImage;
		Image renderImage;
		Image postProcessedImage;

		uint32_t width, height;

		Buffer copyBuffer;
		std::vector<uint8_t> pixels;
	};
}
