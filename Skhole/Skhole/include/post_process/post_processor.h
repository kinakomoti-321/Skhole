#pragma once

#include <include.h>
#include <scene/parameter/parameter.h>

namespace Skhole {

	enum class PostProcessType {
		None,
		Example,
	};


	struct PostProcessParameter {
		std::string name = "default";
		std::vector<ShrPtr<Parameter>> param;
	};

	class PostProcessor {
	public:
		struct Desc {
			vk::PhysicalDevice physicalDevice;
			vk::Device device;
			vk::Queue queue;
			vk::CommandPool commandPool;

			uint32_t width, height;
		};

		struct ExecuteDesc {
			vk::Device device;
			vk::ImageView inputImage;
			vk::ImageView outputImage;

			PostProcessParameter param;
		};

		virtual void Init(const Desc& desc) = 0;
		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual void Execute(vk::CommandBuffer command, const ExecuteDesc& desc) = 0;
		virtual void Destroy(vk::Device device) = 0;

		virtual PostProcessParameter GetParamter() = 0;
	};


}
