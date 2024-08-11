#pragma once

#include <renderer/renderer.h>

namespace Skhole 
{
	class SimpleRaytracer : public Renderer
	{
	public:
		SimpleRaytracer();
		~SimpleRaytracer();

		void Init(RendererDesc& desc) override;
		void Destroy() override;

		void Resize(unsigned int width,unsigned int height) override;
		void Render() override;

		void OffscreenRender() override;
		RendererData GetRendererData() override;

	private:
	};
}
