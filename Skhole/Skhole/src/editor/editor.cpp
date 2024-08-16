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

		m_renderer->SetScene(m_scene);
	}

	void Editor::Run() {

		while (!glfwWindowShouldClose(m_window)) {
			glfwPollEvents();
			m_renderer->SetNewFrame();
			ShowGUI();
			m_renderer->UpdateCamera();

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

		if (ImGui::BeginTabBar("MyTab"))
		{
			if (ImGui::BeginTabItem("Renderer"))
			{
				std::string rendererName = "Renderer Name:" + rendererData.rendererName;
				ImGui::Text(rendererName.c_str());

				ImGui::Text("Renderer");
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Camera"))
			{
				ImGui::Text("Renderer Camera");
				auto& camera = m_scene->m_camera;

				ImGui::InputFloat3("Position",camera->basicParameter.position.v);
				ImGui::InputFloat3("Direction Vector",camera->basicParameter.cameraDir.v);
				ImGui::InputFloat3("Up Vector", camera->basicParameter.cameraUp.v);
				ImGui::InputFloat("FOV", &camera->basicParameter.fov);


				for (auto& camParam : camera->extensionParameters) {
					ShrPtr<ParamFloat> floatParam;
					ShrPtr<ParamVec> vec3Param;
					ShrPtr<ParamUint> textureIDParam;

					switch (camParam->getParamType())
					{
					case ParameterType::FLOAT:
						floatParam = std::static_pointer_cast<ParamFloat>(camParam);
						ImGui::InputFloat(floatParam->getParamName().c_str(), &floatParam->value);
						break;

					case ParameterType::VECTOR:
						vec3Param = std::static_pointer_cast<ParamVec>(camParam);
						ImGui::InputFloat3(vec3Param->getParamName().c_str(), vec3Param->value.v);
						break;

					case ParameterType::UINT:
						textureIDParam = std::static_pointer_cast<ParamUint>(camParam);
						ImGui::Text(textureIDParam->getParamName().c_str());
						break;

					default:
						SKHOLE_UNIMPL();
						break;
					}
				}
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
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
		ImGui::ListBox("List", &selectedIndex, objectNames.data(), objectNames.size());

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

		bool materialUpdate = false;
		auto& mat = m_scene->m_materials[selectedIndex];
		for (auto& matParam : mat->materialParameters) {
			//ImGui::Text(matParam->getParamName().c_str());
			ShrPtr<ParamBool> boolParam;
			ShrPtr<ParamFloat> floatParam;
			ShrPtr<ParamVec> vec3Param;
			ShrPtr<ParamCol> vec4Param;
			ShrPtr<ParamUint> textureIDParam;

			switch (matParam->getParamType())
			{
			case ParameterType::BOOL:
				boolParam = std::static_pointer_cast<ParamBool>(matParam);
				ImGui::Checkbox(boolParam->getParamName().c_str(), &boolParam->value);
				break;

			case ParameterType::FLOAT:
				floatParam = std::static_pointer_cast<ParamFloat>(matParam);
				ImGui::SliderFloat(floatParam->getParamName().c_str(), &floatParam->value, 0.0f, 1.0f);
				break;

			case ParameterType::VECTOR:
				vec3Param = std::static_pointer_cast<ParamVec>(matParam);
				ImGui::SliderFloat3(vec3Param->getParamName().c_str(), vec3Param->value.v, 0.0f, 1.0f);
				break;

			case ParameterType::COLOR:
				vec4Param = std::static_pointer_cast<ParamCol>(matParam);
				ImGui::ColorEdit4(vec4Param->getParamName().c_str(), vec4Param->value.v);
				break;

			case ParameterType::UINT:
				textureIDParam = std::static_pointer_cast<ParamUint>(matParam);
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