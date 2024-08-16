#pragma once

#include <include.h>
#include <renderer/renderer.h>
#include <renderer/simple_raytracer.h>

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

	struct CameraContoller {
		CameraContoller() {}	

		float cameraSpeed = 0.01;	
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

		void ShowRendererGUI();
		void ShowObjectGUI();
		void ShowMateralGUI();


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
	};
}

