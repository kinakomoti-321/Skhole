#include <editor/editor.h>

namespace Skhole
{
	Editor::Editor() {

	}

	Editor::~Editor() {

	}

	void Editor::Init(const EditorInitializationDesc& editorDesc) {

		// Editor Initialization
		m_windowWidth = editorDesc.windowWidth;
		m_windowHeight = editorDesc.windowHeight;
		m_applicationName = editorDesc.applicationName;

		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, m_applicationName.c_str(), nullptr, nullptr);

		if (!m_window) {
			SKHOLE_ERROR("Failed to create window");
			glfwTerminate();
			return;
		}

		// ImGui Initialization

		RendererDesc rendererDesc;
		rendererDesc.Name = "Skhole";
		rendererDesc.Width = m_windowWidth;
		rendererDesc.Height = m_windowHeight;
		rendererDesc.useWindow = true;
		rendererDesc.window = m_window;

		m_renderer = std::make_shared<SimpleRaytracer>();
		m_renderer->Init(rendererDesc);

		// Scene Initialization
		m_scene = std::make_shared<Scene>();

	}

	void Editor::Run() {

		while (!glfwWindowShouldClose(m_window)) {
			glfwPollEvents();
			m_renderer->SetNewFrame();
			ShowGUI();
			m_renderer->Update();
			m_renderer->Render();
		}

		m_renderer->Destroy();
		Destroy();
	}

	void Editor::Destroy() {
		glfwDestroyWindow(m_window);
		glfwTerminate();
	}

	void Editor::ShowGUI() {
		RendererData rendererData = m_renderer->GetRendererData();
		

		//Renderer Imformation
		ImGui::Begin("Rendering Infomation");
		std::string rendererName = "Renderer Name :" + rendererData.rendererName;
		ImGui::Text(rendererName.c_str());

		ImGui::Text("Renderer Material");
		auto& mat = rendererData.materials;
		for (auto& matParam : mat.materialParameters) {
			ImGui::Text(matParam->getParamName().c_str());
		}



		ImGui::End();
	}

	void Editor::ShowRendererGUI() {

	}
}