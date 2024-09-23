#pragma once

#include <include.h>
#include <renderer/renderer.h>
#include <scene/scene.h>
#include <scene/object/geometry.h>
#include <scene/object/instance.h>
#include <scene/material/material.h>
#include <scene/material/texture.h>
#include <scene/camera/camera.h>
#include <scene/object/cameraObject.h>
#include <renderer/core/simple_raytracer.h>
#include <editor/file.h>
#include <loader/loader.h>
#include <common/math.h>
#include <editor/imgui_helper.h>

namespace Skhole {

	struct EditorInitializationDesc {
		std::string applicationName;
		unsigned int windowWidth;
		unsigned int windowHeight;
	};

	struct EditorInputManager {
		EditorInputManager() {}

		bool rightMouseButtonPressed = false;
		bool leftMouseButtonPressed = false;
		bool middleMouseButtonPressed = false;

		bool wPressed = false;
		bool aPressed = false;
		bool sPressed = false;
		bool dPressed = false;
		bool shiftPressed = false;

		vec2 preMousePosition = vec2(0.0);
		vec2 mousePosition = vec2(0.0);

		void CheckKeyInput(GLFWwindow* window) {
			wPressed = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
			aPressed = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
			sPressed = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
			dPressed = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
			shiftPressed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
		}

		void ResetState() {
			rightMouseButtonPressed = false;
			leftMouseButtonPressed = false;
			middleMouseButtonPressed = false;
			shiftPressed = false;

			wPressed = false;
			aPressed = false;
			sPressed = false;
			dPressed = false;
		}
	};

	struct CamereController {
		CamereController() {}

		void SetCamera(ShrPtr<RendererDefinisionCamera>& camera) {
			this->camera = camera;
		}

		void Rotate(vec2 offset) {
			vec2 angleT = offset * sensitivity;

			vec3 cameraDir = camera->foward;
			vec3 cameraUp = camera->up;
			vec3 cameraRight = camera->right;

			mat3 rotY = rotateY(-angleT.x);
			Quaternion cameraQuternion = QuatanionFromAxis(-angleT.y * 2.0, cameraRight);
			mat3 rotUp = RotationMatrix(cameraQuternion);

			cameraDir = rotUp * (rotY * cameraDir);
			cameraRight = rotUp * (rotY * cameraRight);
			cameraUp = rotUp * (rotY * cameraUp);

			camera->foward = normalize(cameraDir);
			camera->right = normalize(cameraRight);
			camera->up = normalize(cameraUp);
		}

		void UpPosition(float magnitude) {
			camera->position += camera->foward * cameraSpeed * magnitude;
		}
		void DownPosition(float magnitude) {
			camera->position -= camera->foward * cameraSpeed * magnitude;
		}
		void LeftPosition(float magnitude) {
			camera->position -= camera->right * cameraSpeed * magnitude;
		}
		void RightPosition(float magnitude) {
			camera->position += camera->right * cameraSpeed * magnitude;
		}

		ShrPtr<RendererDefinisionCamera> camera;

		float yow = 0;
		float pitch = 0;

		float cameraSpeed = 0.01;
		float sensitivity = 0.0015;
	};

	class Editor {
	public:
		Editor();
		~Editor();

		void Init(const EditorInitializationDesc& editorDesc);
		void Run();
		void Destroy();

		void LoadModel(const std::string& filepath);

		void Save();

	private:
		void ShowGUI();

		void ShowSceneGUI();
		void ShowRendererGUI();
		void ShowObjectGUI();
		void ShowMaterialGUI();
		void ShowGeometryGUI();

		void ControlCamera();

		void SaveScene();

		GLFWwindow* m_window;
		ImGuiContext* m_imguiContext;
		std::shared_ptr<Renderer> m_renderer;

		std::shared_ptr<Scene> m_scene;

		unsigned int m_windowWidth;
		unsigned int m_windowHeight;
		std::string m_applicationName;

		static void MouseCallback(GLFWwindow* window, double xpos, double ypos);
		static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

		static EditorInputManager m_inputManager;
		vec2 angle;

		static void MouseResizeCallback(GLFWwindow* window, int width, int height);
		struct ResizeInfo {
			bool resizeFrag = false;
			uint32_t width;
			uint32_t height;
		};
		static ResizeInfo m_resizeInfo;

		CamereController m_cameraController;
		UpdataInfo m_updateInfo;
		bool useGUI = false;

		// Animation
		int currentFrame = 0;
		int startFrame = 0;
		int endFrame = 100;
		bool Runtime = false;

		int fps = 24;
		bool useLoopMode = false;

		int cameraIndex = 0;
		bool useScreenShot = true;
	};
}

