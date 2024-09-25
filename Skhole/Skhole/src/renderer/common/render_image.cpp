#include <renderer/common/render_image.h>
#include <loader/gltf_loader.h>

namespace Skhole {
	void RenderImages::WritePNG(const std::string& filepath, const std::string& filename, vk::Device device, vk::CommandPool commandPool, vk::Queue queue) {

		vkutils::oneTimeSubmit(device, commandPool, queue, [&](vk::CommandBuffer commandBuffer) {
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
		);

		void* data;
		data = copyBuffer.Map(device, 0, copyBuffer.GetBufferSize());
		memcpy(pixels.data(), data, copyBuffer.GetBufferSize());
		copyBuffer.Unmap(device);


		std::string path = filepath + filename + ".png";
		//std::string path = "test.png";
		OutputPNG(path, pixels.data(), width, height, 4);
	}
}