#pragma once

#include <include.h>
#include <scene/scene.h>
#include <renderer/common/render_image.h>
#include <renderer/common/raytracing_pipeline.h>
#include <vulkan_helpler/vk_hepler.h>
#include <vulkan_helpler/vk_imgui.h>
#include <post_process/post_processor.h>
#include <post_process/post_processor_interface.h>
#include <common/util.h>
#include <common/filepath.h>

namespace Skhole
{
	class Scene;

	using ParamDefine = std::vector<ShrPtr<Parameter>>;

	struct RendererDesc
	{
		std::string Name;
		int Width;
		int Height;

		bool useWindow;
		GLFWwindow* window;

		PostProcessType posproType = PostProcessType::Example;
	};

	typedef enum class UpdateCommandType {
		CAMERA,
		MATERIAL,
		TEXTURE,
		OBJECT,
		RENDERER
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
			return UpdateCommandType::RENDERER;
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
		float time;

		bool isScreenShot = false;
		std::string filename = "output";
		std::string filepath = "./image/";
	};

	struct OfflineRenderingInfo {
		uint32_t spp = 1000;
		uint32_t startFrame;
		uint32_t endFrame;
		uint32_t fps;

		uint32_t width = 1024;
		uint32_t height = 720;

		bool useLimitTime = false;
		float limitTime = 10000.0f;

		std::string filename = "output";
		std::string filepath = "./image/";
	};

	// Renderer Data

	class Renderer {
	public:
		Renderer() {};
		~Renderer() {};

		void Initialize(const RendererDesc& desc);
		void Resize(unsigned int width, unsigned int height);
		void Destroy();

		virtual void SetScene(ShrPtr<Scene> scene) = 0;
		virtual void DestroyScene() = 0;

		// Renderer Data
		ShrPtr<RendererDefinisionMaterial> GetMaterial(const ShrPtr<BasicMaterial>& material);
		ShrPtr<RendererDefinisionCamera> GetCamera(const ShrPtr<RendererDefinisionCamera>& basicCamera);

		virtual ShrPtr<RendererParameter> GetRendererParameter() = 0;

		virtual void InitFrameGUI() = 0;

		// Update Structure(For Editor)
		virtual void UpdateScene(const UpdataInfo& command) = 0;

		// Rendering
		virtual void RealTimeRender(const RealTimeRenderingInfo& renderInfo) = 0;
		virtual void OfflineRender(const OfflineRenderingInfo& renderInfo) = 0;

	protected:
		// Internal Methods
		virtual void InitializeCore(const RendererDesc& desc) = 0;
		virtual void ResizeCore(unsigned int width, unsigned int height) = 0;
		virtual void DestroyCore() = 0;

		virtual void DefineMaterial(ShrPtr<RendererDefinisionMaterial>& materialDef, const ShrPtr<BasicMaterial>& material) = 0;
		virtual void DefineCamera(const ShrPtr<RendererDefinisionCamera>& camera) = 0;


		virtual std::vector<const char*> GetLayer() = 0;
		virtual std::vector<const char*> GetExtensions() = 0;
		virtual void InitializeBiniding() = 0;

		struct ShaderPaths {
			std::optional<std::string> raygen;
			std::optional<std::string> miss;
			std::optional<std::string> closestHit;
			std::optional<std::string> anyHit;
		};
		virtual ShaderPaths GetShaderPaths() = 0;

		// Rendering Commands
		void RaytracingCommand(const vk::CommandBuffer& commandBuffer, uint32_t width, uint32_t height);
		void RecordCommandBuffer(uint32_t width, uint32_t height);
		void CopyRenderToScreen(const vk::CommandBuffer& commandBuffer, vk::Image src, vk::Image screen, uint32_t width, uint32_t height);
		void RenderImGuiCommand(const vk::CommandBuffer& commandBuffer, vk::Framebuffer frameBuffer, uint32_t width, uint32_t height);

		void ResetSample();

		ShrPtr<Scene> m_scene;
		ShrPtr<PostProcessor> m_postProcessor;

		VkHelper::Context m_context;
		VkHelper::ScreenContext m_screenContext;

		vk::UniqueCommandPool m_commandPool;
		vk::UniqueCommandBuffer m_commandBuffer;

		VkHelper::VulkanImGuiManager m_imGuiManager;
		vk::UniqueRenderPass m_imGuiRenderPass;

		RaytracingPipeline m_raytracingPipeline;
		VkHelper::BindingManager m_bindingManager;

		RenderImages m_renderImages;

		bool editorMode = false;
	};
}
