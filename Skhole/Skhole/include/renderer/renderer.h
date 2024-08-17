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
	
	struct UpdateCommand {

	};

	struct UpdateMaterialInfo {

	};

	struct UpdateCameraInfo {

	};

	struct UpdateGeometryInfo {

	};

	struct UpdateObjectInfo {

	};

	struct RenderInfo {
		uint32_t frame;
		uint32_t spp;
	};

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
		virtual RendererData GetRendererData() = 0;
		virtual ShrPtr<RendererDefinisionMaterial> GetMaterial(const ShrPtr<BasicMaterial>& material) = 0;
		virtual ShrPtr<RendererDefinisionCamera> GetCamera(const ShrPtr<BasicCamera>& basicCamera) = 0;


		virtual void InitFrameGUI() = 0;
		// Update Structure
		virtual void UpdateScene(const UpdateCommand& command) = 0;

		virtual void Render(const RenderInfo& renderInfo) = 0;
	};
}
