#pragma once

#include <include.h>
#include <scene/scene.h>


namespace Skhole
{
	class Scene;

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
		RendererDefinisionMaterial materials;
	};

	class Renderer {
	public:
		Renderer() {};
		~Renderer() {};

		virtual void Init(RendererDesc& desc) = 0;

		virtual void Destroy() = 0;

		virtual void Resize(unsigned int width,unsigned int height) = 0;

		virtual void SetNewFrame() = 0;

		virtual void Update() = 0;

		virtual void Render() = 0;

		virtual void Wait() = 0;

		virtual void OffscreenRender() = 0;

		virtual void SetScene(ShrPtr<Scene> scene) = 0;

		// Renderer Data
		virtual RendererData GetRendererData() = 0;

		virtual ShrPtr<RendererDefinisionMaterial> GetMaterial(const ShrPtr<BasicMaterial>& material) = 0;
		virtual void UpdateMaterial() = 0;

		virtual ShrPtr<RendererDefinisionCamera> GetCamera(const ShrPtr<BasicCamera>& basicCamera) = 0;
		virtual void UpdateCamera() = 0;
	};
}
