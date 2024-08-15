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

	class Editor {
	public:
		Editor();
		~Editor();
			
		void Init(const EditorInitializationDesc& editorDesc);
		void Run();
		void Destroy();

	private:
		void ShowGUI();

		void SaveScene();

		GLFWwindow* m_window;
		ImGuiContext* m_imguiContext;
		std::shared_ptr<Renderer> m_renderer;

		unsigned int m_windowWidth;
		unsigned int m_windowHeight;	
		std::string m_applicationName;
	};
}

