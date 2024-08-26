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

	enum UpdateCommandType {
		CAMERA,
		MATERIAL,
		TEXTURE,
		OBJECT,
	};

	class UpdateCommand {
	public:
		UpdateCommand() {};
		~UpdateCommand() {};

		virtual UpdateCommandType GetCommandType() = 0;
	};

	class UpdateRendererCommand : public UpdateCommand {
	public:
		UpdateRendererCommand() {}
		~UpdateRendererCommand() {};

		UpdateCommandType GetCommandType() override {
			return UpdateCommandType::CAMERA;
		}
	};

	class UpdateObjectCommand : public UpdateCommand {
	public:
		UpdateObjectCommand(uint32_t objectId, ObjectType objectType) :objectIndex(objectId), objectType(objectType) {}
		~UpdateObjectCommand() {};

		UpdateCommandType GetCommandType() override {
			return UpdateCommandType::OBJECT;
		}

		uint32_t objectIndex;
		ObjectType objectType;
	};

	class UpdateCameraCommand : public UpdateCommand {
	public:
		UpdateCameraCommand(bool moveFrag) :isMoving(moveFrag) {}
		~UpdateCameraCommand() {};

		UpdateCommandType GetCommandType() override {
			return UpdateCommandType::CAMERA;
		}

		bool isMoving;
	};

	class UpdateMaterialCommand : public UpdateCommand {
	public:
		UpdateMaterialCommand(uint32_t materialId) : materialIndex(materialId) {};
		~UpdateMaterialCommand() {}

		UpdateCommandType GetCommandType() override {
			return UpdateCommandType::MATERIAL;
		}

		uint32_t materialIndex;
	};

	class UpdateTextureCommand : public UpdateCommand {
	public:
		UpdateTextureCommand(uint32_t textureId) : textureIndex(textureId) {};
		~UpdateTextureCommand() {}

		UpdateCommandType GetCommandType() override {
			return UpdateCommandType::TEXTURE;
		}

		uint32_t textureIndex;
	};


	struct UpdataInfo {
		void ResetCommands() {
			commands.clear();
		}

		std::vector<ShrPtr<UpdateCommand>> commands;
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

		virtual void Resize(unsigned int width, unsigned int height) = 0;

		virtual void SetScene(ShrPtr<Scene> scene) = 0;
		virtual void DestroyScene() = 0;

		// Renderer Data
		virtual ShrPtr<RendererDefinisionMaterial> GetMaterial(const ShrPtr<BasicMaterial>& material) = 0;
		virtual ShrPtr<RendererDefinisionCamera> GetCamera(const ShrPtr<BasicCamera>& basicCamera) = 0;
		virtual ShrPtr<RendererParameter> GetRendererParameter() = 0;

		virtual void InitFrameGUI() = 0;

		// Update Structure(For Editor)
		virtual void UpdateScene(const UpdataInfo& command) = 0;

		// Rendering
		virtual void RealTimeRender(const RealTimeRenderingInfo& renderInfo) = 0;
		virtual void OfflineRender(const OfflineRenderingInfo& renderInfo) = 0;
	};
}
