#include <renderer/common/render_image.h>
#include <loader/gltf_loader.h>

namespace Skhole {
	void RenderImages::WritePNG(vk::Device device) {
		void* data;
		data = copyBuffer.Map(device, 0, copyBuffer.GetBufferSize());
		memcpy(pixels.data(), data, copyBuffer.GetBufferSize());
		copyBuffer.Unmap(device);

		//if (!stbi_write_png("output.png", width, height, 4, pixels.data(), width * 4)) {
		//	std::cerr << "Failed to write PNG" << std::endl;
		//}
	}
}