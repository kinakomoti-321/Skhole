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
		m_scene->Initialize();

		m_scene->RendererSet(m_renderer);

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
		ShowRendererGUI();
		ShowObjectGUI();
		ShowMateralGUI();
	}

	void Editor::ShowRendererGUI() {
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

	void Editor::ShowObjectGUI() {
		ImGui::Begin("Object Information");

		//ImGui::BeginChild(ImGui::GetID((void*)0), ImVec2(250, 100), ImGuiWindowFlags_NoTitleBar);
		//// Object Information
		//ImGui::EndChild();

		std::vector<const char*> objectNames;
		objectNames.reserve(m_scene->m_objects.size());
		for (auto& object : m_scene->m_objects) {
			objectNames.push_back(object->GetObjectName().c_str());
		}

		static int selectedIndex = 0;
		ImGui::ListBox("List",&selectedIndex,objectNames.data(),objectNames.size());

		ImGui::End();
	}

	void Editor::ShowMateralGUI() {
		ImGui::Begin("Material Information");

		std::vector<const char*> materialNames;
		materialNames.reserve(m_scene->m_basicMaterials.size());
		for (auto& material : m_scene->m_materials) {
			materialNames.push_back(material->materialName.c_str());
		}	

		static int selectedIndex = 0;
		ImGui::ListBox("List", &selectedIndex, materialNames.data(), materialNames.size());

		auto& mat = m_scene->m_materials[selectedIndex];
		for (auto& matParam : mat->materialParameters) {
			//ImGui::Text(matParam->getParamName().c_str());
			ShrPtr<MatParamBool> boolParam;
			ShrPtr<MatParamFloat> floatParam;
			ShrPtr<MatParamVector> vec3Param;
			ShrPtr<MatParamColor> vec4Param;
			ShrPtr<MatParamTexID> textureIDParam;

			switch (matParam->getParamType())
			{
			case MaterialParameterType::BOOL:
				boolParam = std::static_pointer_cast<MatParamBool>(matParam);
				ImGui::Checkbox(boolParam->getParamName().c_str(), &boolParam->value);
				break;

			case MaterialParameterType::FLOAT:
				floatParam = std::static_pointer_cast<MatParamFloat>(matParam);
				ImGui::SliderFloat(floatParam->getParamName().c_str(), &floatParam->value, 0.0f, 1.0f);
				break;

			case MaterialParameterType::VECTOR:
				vec3Param = std::static_pointer_cast<MatParamVector>(matParam);
				ImGui::SliderFloat3(vec3Param->getParamName().c_str(), vec3Param->value.v, 0.0f, 1.0f);
				break;

			case MaterialParameterType::COLOR:
				vec4Param = std::static_pointer_cast<MatParamColor>(matParam);
				ImGui::ColorEdit4(vec4Param->getParamName().c_str(), vec4Param->value.v);
				break;

			case MaterialParameterType::TEXTURE_ID:
				textureIDParam = std::static_pointer_cast<MatParamTexID>(matParam);
				ImGui::Text(textureIDParam->getParamName().c_str());
				break;

			default:
				SKHOLE_UNIMPL();
				break;
			}
		}



		ImGui::End();
	}

}