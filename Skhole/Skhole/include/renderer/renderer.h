#pragma once

#include <include.h>
#include <scene/scene.h>

namespace Skhole
{
	struct RendererDesc 
	{
		std::string Name;
		int Width;
		int Height;	

		bool useWindow;
		GLFWwindow* window;
	};

	struct RendererData 
	{
		std::string rendererName;
		std::vector<MaterialParameter> materialParameters;
	};

	class Renderer {
	public:
		Renderer() {};
		~Renderer() {};

		virtual void Init(RendererDesc& desc) = 0;
		virtual void Destroy() = 0;

		virtual void Resize(unsigned int width,unsigned int height) = 0;
		virtual void Render() = 0;

		virtual void Wait() = 0;

		virtual void OffscreenRender() = 0;
		virtual RendererData GetRendererData() = 0;
	};
}
