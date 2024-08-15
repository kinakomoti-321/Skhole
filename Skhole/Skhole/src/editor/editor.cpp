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

		ImGui::Begin("Rendering Infomation");
		std::string rendererName = "Renderer Name :" + rendererData.rendererName;
		ImGui::Text(rendererName.c_str());

		for (auto& parameter : rendererData.materials.materialParameters) {
			if (std::holds_alternative<MaterialParameterBool>(parameter.parameter)) {
				auto& b = std::get<MaterialParameterBool>(parameter.parameter);
				ImGui::Checkbox(b.name.c_str(), &b.value);
			}
			else if (std::holds_alternative<MaterialParameterFloat>(parameter.parameter))
			{
				auto& f = std::get<MaterialParameterFloat>(parameter.parameter);
				ImGui::SliderFloat(f.name.c_str(), &f.value, 0, 1);
			}
			else if (std::holds_alternative<MaterialParameterVector>(parameter.parameter)) {
				auto& value = std::get<MaterialParameterVector>(parameter.parameter);
				ImGui::DragFloat3(value.name.c_str(), value.value.v, 0.1f, 0.0f, 1.0f);
			}
			else if (std::holds_alternative<MaterialParameterColor>(parameter.parameter)) {
				auto& value = std::get<MaterialParameterColor>(parameter.parameter);
				ImGui::ColorEdit4(value.name.c_str(), value.value.v);	
			}
			else if (std::holds_alternative<MaterialParameterTexture>(parameter.parameter)) {
				SKHOLE_UNIMPL("Not Implemented Texture");
			}
			else {
				SKHOLE_UNIMPL("Not Implemented Material Parameter");
			}

		}

		ImGui::End();

		// Rendering Information
	}
}