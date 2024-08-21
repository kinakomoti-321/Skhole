#include <editor/editor.h>
#include <common/math.h>

namespace Skhole
{
	EditorInputManager Editor::m_inputManager;

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
		glfwWindowHint(GLFW_CURSOR, GLFW_CURSOR_NORMAL);

		m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, m_applicationName.c_str(), nullptr, nullptr);

		if (!m_window) {
			SKHOLE_ERROR("Failed to create window");
			glfwTerminate();
			return;
		}

		glfwSetMouseButtonCallback(m_window, MouseButtonCallback);
		glfwSetCursorPosCallback(m_window, MouseCallback);

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

	void Editor::ControlCamera() {
		m_cameraController.SetCamera(m_scene->m_camera);
		// Input 
		{
			m_inputManager.CheckKeyInput(m_window);

			if (m_inputManager.leftMouseButtonPressed) {
				vec2 offset = m_inputManager.mousePosition - m_inputManager.preMousePosition;
				if (std::abs(offset.x) < 1.5) offset.x = 0;
				if (std::abs(offset.y) < 1.5) offset.y = 0;
				m_cameraController.Rotate(offset);
			}

			float magnitude = 1.0;
			if (m_inputManager.shiftPressed) magnitude = 2.0;

			if (m_inputManager.wPressed) {
				m_cameraController.UpPosition(magnitude);
			}

			if (m_inputManager.sPressed) {
				m_cameraController.DownPosition(magnitude);
			}

			if (m_inputManager.aPressed) {
				m_cameraController.LeftPosition(magnitude);
			}

			if (m_inputManager.dPressed) {
				m_cameraController.RightPosition(magnitude);
			}

		}
	}
	void Editor::Run() {

		while (!glfwWindowShouldClose(m_window)) {
			glfwPollEvents();

			ControlCamera();

			m_renderer->InitFrameGUI();

			ShowGUI();

			UpdateCommand command;
			m_renderer->UpdateScene(command);

			RealTimeRenderingInfo renderInfo;
			renderInfo.frame = 0;
			renderInfo.spp = 100;
			m_renderer->RealTimeRender(renderInfo);
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


		ImGui::Begin("Rendering Infomation");

		if (ImGui::BeginTabBar("MyTab"))
		{
			if (ImGui::BeginTabItem("Renderer"))
			{
				ImGui::Text("Renderer Information");

				auto& rendererData = m_scene->m_rendererParameter;

				ImGui::Text("Renderer Name : %s", rendererData->rendererName.c_str());
				ImGui::Text("Frame : %u", rendererData->frame);
				ImGui::Text("SPP : %u", rendererData->spp);
				ImGui::Text("sample : %u", rendererData->sample);

				for (auto& rendererParam : rendererData->rendererParameters) {
					ShrPtr<ParamFloat> floatParam;
					ShrPtr<ParamVec> vec3Param;
					ShrPtr<ParamUint> textureIDParam;

					switch (rendererParam->getParamType())
					{
					case ParameterType::FLOAT:
						floatParam = std::static_pointer_cast<ParamFloat>(rendererParam);
						ImGui::InputFloat(floatParam->getParamName().c_str(), &floatParam->value);
						break;

					case ParameterType::VECTOR:
						vec3Param = std::static_pointer_cast<ParamVec>(rendererParam);
						ImGui::InputFloat3(vec3Param->getParamName().c_str(), vec3Param->value.v);
						break;

					case ParameterType::UINT:
						textureIDParam = std::static_pointer_cast<ParamUint>(rendererParam);
						ImGui::InputScalar(textureIDParam->getParamName().c_str(), ImGuiDataType_U32, &textureIDParam->value);
						break;

					default:
						SKHOLE_UNIMPL();
						break;
					}
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Camera"))
			{
				ImGui::Text("Renderer Camera");
				auto& camera = m_scene->m_camera;

				ImGui::InputFloat3("Position", camera->basicParameter.position.v);
				ImGui::InputFloat3("Direction Vector", camera->basicParameter.cameraDir.v);
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

				ImGui::InputFloat("Speed", &m_cameraController.cameraSpeed);
				ImGui::InputFloat("Sensitivity", &m_cameraController.sensitivity);

				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}

		ImGui::End();
	}

	void Editor::ShowObjectGUI() {
		ImGui::Begin("Object Information");

		std::vector<const char*> objectNames;
		objectNames.reserve(m_scene->m_objects.size());
		for (auto& object : m_scene->m_objects) {
			objectNames.push_back(object->GetObjectName());
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
				ImGui::InputScalar(textureIDParam->getParamName().c_str(), ImGuiDataType_U32, &textureIDParam->value);
				break;

			default:
				SKHOLE_UNIMPL();
				break;
			}
		}

		ImGui::End();
	}

	void Editor::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
			m_inputManager.leftMouseButtonPressed = true;
		}
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
			m_inputManager.leftMouseButtonPressed = false;
		}

		if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
			m_inputManager.rightMouseButtonPressed = true;
		}
	}

	void Editor::MouseCallback(GLFWwindow* window, double xpos, double ypos) {
		m_inputManager.preMousePosition = m_inputManager.mousePosition;
		m_inputManager.mousePosition = vec2(xpos, ypos);
	}
}