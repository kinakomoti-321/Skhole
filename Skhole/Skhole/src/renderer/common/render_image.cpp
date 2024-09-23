#include <renderer/common/render_image.h>
#include <loader/gltf_loader.h>

namespace Skhole {
	void RenderImages::WritePNG(vk::Device device) {
		void* data;
		data = copyBuffer.Map(device, 0, copyBuffer.GetBufferSize());
		memcpy(pixels.data(), data, copyBuffer.GetBufferSize());
		copyBuffer.Unmap(device);


		std::string path = "output.png";
		OutputPNG(path, pixels.data(), width, height, 4);
	}
}