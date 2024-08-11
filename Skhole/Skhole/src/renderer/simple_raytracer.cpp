#include <renderer/simple_raytracer.h>
#include <common/log.h>

namespace Skhole{
	SimpleRaytracer::SimpleRaytracer()
	{
	}

	SimpleRaytracer::~SimpleRaytracer()
	{
	}

	void SimpleRaytracer::Init(RendererDesc& desc)
	{

	}

	void SimpleRaytracer::Destroy()
	{

	}

	void SimpleRaytracer::Resize(unsigned int width, unsigned int height)
	{
		SKHOLE_UNIMPL("Resize");
	}

	void SimpleRaytracer::Render()
	{
		SKHOLE_UNIMPL("Render");
	}

	void SimpleRaytracer::OffscreenRender()
	{
		SKHOLE_UNIMPL("OffscreenRender");
	}

	RendererData SimpleRaytracer::GetRendererData()
	{
		return RendererData();
	}
}
