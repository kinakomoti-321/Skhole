#include <editor/editor.h>
#include <common/math.h>

namespace Skhole
{
	EditorInputManager Editor::m_inputManager;
	Editor::ResizeInfo Editor::m_resizeInfo;

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
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		glfwWindowHint(GLFW_CURSOR, GLFW_CURSOR_NORMAL);

		m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, m_applicationName.c_str(), nullptr, nullptr);

		if (!m_window) {
			SKHOLE_ERROR("Failed to create window");
			glfwTerminate();
			return;
		}

		glfwSetMouseButtonCallback(m_window, MouseButtonCallback);
		glfwSetCursorPosCallback(m_window, MouseCallback);

		glfwSetFramebufferSizeCallback(m_window, MouseResizeCallback);

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
			bool moveFrag = false;
			m_inputManager.CheckKeyInput(m_window);

			if (m_inputManager.leftMouseButtonPressed) {
				vec2 offset = m_inputManager.mousePosition - m_inputManager.preMousePosition;
				if (std::abs(offset.x) < 1.5) offset.x = 0;
				if (std::abs(offset.y) < 1.5) offset.y = 0;
				m_cameraController.Rotate(offset);
				moveFrag = true;
			}

			float magnitude = 1.0;
			if (m_inputManager.shiftPressed) magnitude = 2.0;

			if (m_inputManager.wPressed) {
				m_cameraController.UpPosition(magnitude);
				moveFrag = true;
			}

			if (m_inputManager.sPressed) {
				m_cameraController.DownPosition(magnitude);
				moveFrag = true;
			}

			if (m_inputManager.aPressed) {
				m_cameraController.LeftPosition(magnitude);
				moveFrag = true;
			}

			if (m_inputManager.dPressed) {
				m_cameraController.RightPosition(magnitude);
				moveFrag = true;
			}

			if (moveFrag) {
				m_updateInfo.commands.push_back(std::make_shared<UpdateCameraCommand>(true));
			}
		}
	}
	void Editor::Run() {

		while (!glfwWindowShouldClose(m_window)) {
			glfwPollEvents();

			if (m_resizeInfo.resizeFrag) {
				m_renderer->Resize(m_resizeInfo.width, m_resizeInfo.height);
				m_resizeInfo.resizeFrag = false;
			}

			m_updateInfo.ResetCommands();

			m_renderer->InitFrameGUI();

			if (!useGUI)
			{
				ControlCamera();
			}
			//else {
			//	ControlCamera();
			//}
			ShowGUI();

			m_renderer->UpdateScene(m_updateInfo);

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
		ShowSceneGUI();
		ShowRendererGUI();
		ShowObjectGUI();
		ShowMateralGUI();

		useGUI = ImGui::IsAnyItemActive();
	}

	void Editor::ShowSceneGUI() {
		ImGui::Begin("Scene Information");

		if (ImGui::Button("Load Scene")) {
			std::string path;
			if (File::CallFileDialog(path)) {
				std::cout << path << std::endl;
				m_renderer->DestroyScene();
				auto newScene = Loader::LoadFile(path);

				m_scene = newScene;
				m_scene->RendererSet(m_renderer);

				m_renderer->SetScene(m_scene);
			}
		}
		ImGui::Text("Scene Information");

		ImGui::End();

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
				ImGui::Text("sample : %u", rendererData->sample);
				ImGui::InputScalar("SPP", ImGuiDataType_U32, &rendererData->spp);

				bool updateRenderer = false;
				for (auto& rendererParam : rendererData->rendererParameters) {
					ShrPtr<ParamFloat> floatParam;
					ShrPtr<ParamVec> vec3Param;
					ShrPtr<ParamUint> textureIDParam;

					switch (rendererParam->getParamType())
					{
					case ParameterType::FLOAT:
						floatParam = std::static_pointer_cast<ParamFloat>(rendererParam);
						updateRenderer |= ImGui::InputFloat(floatParam->getParamName().c_str(), &floatParam->value);
						break;

					case ParameterType::VECTOR:
						vec3Param = std::static_pointer_cast<ParamVec>(rendererParam);
						updateRenderer |= ImGui::InputFloat3(vec3Param->getParamName().c_str(), vec3Param->value.v);
						break;

					case ParameterType::UINT:
						textureIDParam = std::static_pointer_cast<ParamUint>(rendererParam);
						updateRenderer |= ImGui::InputScalar(textureIDParam->getParamName().c_str(), ImGuiDataType_U32, &textureIDParam->value);
						break;

					default:
						SKHOLE_UNIMPL();
						break;
					}
				}

				if (updateRenderer)
				{
					m_updateInfo.commands.push_back(std::make_shared<UpdateRendererCommand>());
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Camera"))
			{
				bool updateCamera = false;
				ImGui::Text("Renderer Camera");
				auto& camera = m_scene->m_camera;

				updateCamera |= ImGui::InputFloat3("Position", camera->basicParameter.position.v);
				updateCamera |= ImGui::InputFloat3("Direction Vector", camera->basicParameter.cameraDir.v);
				updateCamera |= ImGui::InputFloat3("Up Vector", camera->basicParameter.cameraUp.v);
				updateCamera |= ImGui::InputFloat("FOV", &camera->basicParameter.fov);


				for (auto& camParam : camera->extensionParameters) {
					ShrPtr<ParamFloat> floatParam;
					ShrPtr<ParamVec> vec3Param;
					ShrPtr<ParamUint> textureIDParam;

					switch (camParam->getParamType())
					{
					case ParameterType::FLOAT:
						floatParam = std::static_pointer_cast<ParamFloat>(camParam);
						updateCamera |= ImGui::InputFloat(floatParam->getParamName().c_str(), &floatParam->value);
						break;

					case ParameterType::VECTOR:
						vec3Param = std::static_pointer_cast<ParamVec>(camParam);
						updateCamera |= ImGui::InputFloat3(vec3Param->getParamName().c_str(), vec3Param->value.v);
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


				updateCamera |= ImGui::InputFloat("Speed", &m_cameraController.cameraSpeed);
				updateCamera |= ImGui::InputFloat("Sensitivity", &m_cameraController.sensitivity);

				if (updateCamera) {
					m_updateInfo.commands.push_back(std::make_shared<UpdateRendererCommand>());
				}
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

		auto& mat = m_scene->m_materials[selectedIndex];
		bool materialUpdate = false;
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
				materialUpdate |= ImGui::Checkbox(boolParam->getParamName().c_str(), &boolParam->value);
				break;

			case ParameterType::FLOAT:
				floatParam = std::static_pointer_cast<ParamFloat>(matParam);
				materialUpdate |= ImGui::InputFloat(floatParam->getParamName().c_str(), &floatParam->value);
				break;

			case ParameterType::VECTOR:
				vec3Param = std::static_pointer_cast<ParamVec>(matParam);
				materialUpdate |= ImGui::SliderFloat3(vec3Param->getParamName().c_str(), vec3Param->value.v, 0.0f, 1.0f);
				break;

			case ParameterType::COLOR:
				vec4Param = std::static_pointer_cast<ParamCol>(matParam);
				materialUpdate |= ImGui::ColorEdit4(vec4Param->getParamName().c_str(), vec4Param->value.v);
				break;

			case ParameterType::UINT:
				textureIDParam = std::static_pointer_cast<ParamUint>(matParam);
				materialUpdate |= ImGui::InputScalar(textureIDParam->getParamName().c_str(), ImGuiDataType_U32, &textureIDParam->value);
				break;

			default:
				SKHOLE_UNIMPL();
				break;
			}
		}

		ImGui::End();

		if (materialUpdate) {
			m_updateInfo.commands.push_back(std::make_shared<UpdateMaterialCommand>(selectedIndex));
		}
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

	void Editor::MouseResizeCallback(GLFWwindow* window, int width, int height)
	{
		m_resizeInfo.resizeFrag = true;
		m_resizeInfo.width = width;
		m_resizeInfo.height = height;
	}
}