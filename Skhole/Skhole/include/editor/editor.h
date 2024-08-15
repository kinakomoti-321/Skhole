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
	};
}

