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

	struct UpdateCommand {

	};

	struct RealTimeRenderingInfo {
		uint32_t spp;
		uint32_t frame;
	};

	struct OfflineRenderingInfo {
		uint32_t spp;
		uint32_t frame;
	};	

	// Renderer Data

	class Renderer {
	public:
		Renderer() {};
		~Renderer() {};

		virtual void Init(RendererDesc& desc) = 0;

		virtual void Destroy() = 0;

		virtual void Resize(unsigned int width,unsigned int height) = 0;

		virtual void SetScene(ShrPtr<Scene> scene) = 0;
		virtual void DestroyScene() = 0;

		// Renderer Data
		virtual ShrPtr<RendererDefinisionMaterial> GetMaterial(const ShrPtr<BasicMaterial>& material) = 0;
		virtual ShrPtr<RendererDefinisionCamera> GetCamera(const ShrPtr<BasicCamera>& basicCamera) = 0;
		virtual ShrPtr<RendererParameter> GetRendererParameter() = 0;

		virtual void InitFrameGUI() = 0;

		// Update Structure(For Editor)
		virtual void UpdateScene(const UpdateCommand& command) = 0;
		
		// Rendering
		virtual void RealTimeRender(const RealTimeRenderingInfo& renderInfo) = 0;
		virtual void OfflineRender(const OfflineRenderingInfo& renderInfo) = 0;
	};
}
