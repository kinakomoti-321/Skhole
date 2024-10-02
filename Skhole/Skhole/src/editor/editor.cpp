#include <editor/editor.h>
#include <common/math.h>
#include <renderer/core/vndf_renderer.h>

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
		auto windowWidth = editorDesc.windowWidth;
		auto windowHeight = editorDesc.windowHeight;
		m_applicationName = editorDesc.applicationName;

		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		glfwWindowHint(GLFW_CURSOR, GLFW_CURSOR_NORMAL);

		m_window = glfwCreateWindow(windowWidth, windowHeight, m_applicationName.c_str(), nullptr, nullptr);

		if (!m_window) {
			SKHOLE_ERROR("Failed to create window");
			glfwTerminate();
			return;
		}

		glfwSetMouseButtonCallback(m_window, MouseButtonCallback);
		glfwSetCursorPosCallback(m_window, MouseCallback);

		glfwSetFramebufferSizeCallback(m_window, MouseResizeCallback);

		// ImGui Initialization

		m_renderer = std::make_shared<VNDF_Renderer>();
		m_scene = std::make_shared<Scene>();
		m_scene->Initialize();

		RendererDesc rendererDesc;
		rendererDesc.Name = "Skhole";
		glfwGetWindowSize(m_window, &rendererDesc.Width, &rendererDesc.Height);
		rendererDesc.useWindow = true;
		rendererDesc.window = m_window;

		InitializeRendererScene(rendererDesc, m_renderer, m_scene);
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
				glfwGetWindowSize(m_window, &m_resizeInfo.width, &m_resizeInfo.height);
				if (m_resizeInfo.width == 0 || m_resizeInfo.height == 0) continue;
				m_renderer->Resize(m_resizeInfo.width, m_resizeInfo.height);
				m_resizeInfo.resizeFrag = false;
			}

			m_updateInfo.ResetCommands();

			m_renderer->InitFrameGUI();

			ShowGUI();

			if (!useGUI)
			{
				ControlCamera();
			}

			m_renderer->UpdateScene(m_updateInfo);

			RealTimeRenderingInfo renderInfo;
			renderInfo.frame = currentFrame;
			renderInfo.time = float(currentFrame) / float(fps);
			renderInfo.spp = 100;
			renderInfo.isScreenShot = useScreenShot;
			renderInfo.filename = "screenshot";
			renderInfo.filepath = "./image/";

			m_renderer->RealTimeRender(renderInfo);

			if (recreateFrag) {
				//m_renderer->Destroy();
				m_renderer->DestroyScene();

				if (sceneLoadFrag) {
					std::string path;
					std::string filename;

					SeparatePathAndFile(settingJsonPath, path, filename);

					path += "\\";

					ImportSettingOutput importSetting = ImportSetting(path, filename);
					if (!importSetting.success) {
						SKHOLE_ERROR("Failed to Load Scene");
					}

					m_scene = importSetting.scene;

					sceneLoadFrag = false;
				}

				//m_renderer = std::make_shared<VNDF_Renderer>();

				RendererDesc desc;
				desc.Name = "Skhole";
				glfwGetWindowSize(m_window, &desc.Width, &desc.Height);
				desc.useWindow = true;
				desc.window = m_window;

				//InitializeRendererScene(desc, m_renderer, m_scene);

				//m_renderer->Initialize(desc);
				m_renderer->SetScene(m_scene);

				recreateFrag = false;
			}
		}

		m_renderer->Destroy();
		Destroy();
	}

	void Editor::Destroy() {
		glfwDestroyWindow(m_window);
		glfwTerminate();
	}

	void Editor::ShowGUI() {
		ShowSettingGUI();
		ShowSceneGUI();
		ShowRendererGUI();
		useGUI = ImGui::IsAnyItemActive();
		useGUI |= ImGui::IsItemClicked();
		useGUI |= ImGui::IsMouseClicked(0);
	}

	void Editor::ShowSettingGUI() {
		ImGui::Begin("Setting");

		ImGui::Text("Animation Information");

		bool updateAnimation = false;

		updateAnimation |= ImGui::SliderInt("Frame", &currentFrame, startFrame, endFrame);
		updateAnimation |= ImGui::InputInt("Start Frame", &startFrame);
		updateAnimation |= ImGui::InputInt("End Frame", &endFrame);

		updateAnimation |= ImGui::InputInt("FPS", &fps);

		updateAnimation |= ImGui::Checkbox("Loop Mode", &useLoopMode);
		updateAnimation |= ImGui::Checkbox("Runtime", &Runtime);

		if (updateAnimation) {
			m_updateInfo.commands.push_back(std::make_shared<UpdateRendererCommand>());
		}

		ImGui::Text("Rendering Resolution");
		InputUint("Width", &offlineRenderingInfo.width);
		InputUint("Height", &offlineRenderingInfo.height);

		InputUint("SPP", &offlineRenderingInfo.spp);

		ImGui::Checkbox("Use TimeLimit", &offlineRenderingInfo.useLimitTime);
		if (offlineRenderingInfo.useLimitTime) {
			ImGui::InputFloat("Limit Time (second)", &offlineRenderingInfo.limitTime);
		}

		if (ImGui::Button("Rendering")) {
			offlineRenderingInfo.startFrame = startFrame;
			offlineRenderingInfo.endFrame = endFrame;
			offlineRenderingInfo.fps = fps;

			std::string path;
			if (File::CallFolderDialog(path)) {
				std::filesystem::path folderPath = path;
				offlineRenderingInfo.filepath = folderPath.parent_path().string() + "/";
				offlineRenderingInfo.filename = folderPath.filename().string();
				m_renderer->OfflineRender(offlineRenderingInfo);

				m_resizeInfo.resizeFrag = true;
			}
		}

		if (ImGui::Button("RendererChange")) {

			recreateFrag = true;
		}

		if (ImGui::Button("Save Setting")) {

			offlineRenderingInfo.startFrame = startFrame;
			offlineRenderingInfo.endFrame = endFrame;
			offlineRenderingInfo.fps = fps;

			offlineRenderingInfo.filepath = "./image/";
			offlineRenderingInfo.filename = "output";

			std::string path;
			if (File::CallFolderDialog(path)) {
				std::filesystem::path folderPath = path;

				ExportSettingDesc desc;
				desc.filepath = folderPath.parent_path().string() + "/";
				desc.filename = folderPath.filename().string();
				desc.scene = m_scene;
				desc.offlineRenderingInfo = offlineRenderingInfo;

				ExportSetting(desc);
			}
		}

		if (ImGui::Button("Test Load")) {
			std::string path;
			std::string file;
			if (File::CallFileDialog(path, file)) {
				ImportSettingOutput info = ImportSetting(path, file);
			}
		}

		ImGui::End();
	}

	void Editor::ShowSceneGUI() {
		ImGui::Begin("Scene Information");

		if (ImGui::BeginTabBar("MyTab"))
		{
			if (ImGui::BeginTabItem("Scene"))
			{
				ImGui::Text("Scene Information");
				ImGui::Separator();

				if (ImGui::TreeNode("Load Scene")) {
					if (ImGui::Button("Load Scene")) {
						std::string path;
						std::string filename;
						if (File::CallFileDialog(path, filename)) {
							std::cout << path << std::endl;
							path = path + filename;

							std::string extension;
							if (!GetFileExtension(path, extension)) {
								SKHOLE_ERROR("Invalid File Path");
							}

							if (extension == "json")
							{
								settingJsonPath = path;
								sceneLoadFrag = true;
								recreateFrag = true;
							}
							else {
								m_renderer->DestroyScene();
								auto newScene = Loader::LoadFile(path);

								m_scene = newScene;
								m_scene->RendererSet(m_renderer);

								m_renderer->SetScene(m_scene);

								cameraIndex = 0;
							}
						}

					}
					ImGui::TreePop();
				}

				if (ImGui::TreeNode("Save Scene")) {

					ImGui::Text("Save Scene");

					if (ImGui::Button("Save Scene")) {

					}
					ImGui::TreePop();
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Object"))
			{
				ImGui::Text("Object Information");
				ShowObjectGUI();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Material")) {
				ImGui::Text("Material Information");
				ShowMaterialGUI();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Texture"))
			{
				ImGui::Text("Texture Information");

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Geometry"))
			{
				ImGui::Text("Geometry Information");
				ShowGeometryGUI();
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		ImGui::End();

	}

	void Editor::ShowRendererGUI() {

		ImGui::Begin("Rendering Infomation");

		if (ImGui::BeginTabBar("MyTab"))
		{
			if (ImGui::BeginTabItem("Renderer"))
			{
				ImGui::Text("Renderer Information");
				ImGui::Separator();

				auto& rendererData = m_scene->m_rendererParameter;

				ImGui::Text("Renderer Name : %s", rendererData->rendererName.c_str());
				ImGui::Text("Frame : %u", rendererData->frame);
				ImGui::Text("sample : %u", rendererData->numSPP);
				ImGui::InputScalar("Max SPP", ImGuiDataType_U32, &rendererData->maxSPP);
				InputUint("Sample Per Frame", &rendererData->sppPerFrame);

				useScreenShot = ImGui::Button("ScreenShot");

				bool updateRenderer = false;
				if (ImGui::TreeNode("Renderer Parameter")) {
					updateRenderer |= ParameterGUI(rendererData->rendererParameters);

					ImGui::TreePop();
				}

				if (ImGui::TreeNode("PostProcess")) {
					auto& posPro = rendererData->posproParameters;

					StringText("Post Processer : " + posPro.name);
					updateRenderer |= ParameterGUI(posPro.param);

					ImGui::TreePop();
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

				updateCamera |= ImGui::InputFloat3("Position", camera->position.v);
				updateCamera |= ImGui::InputFloat3("Direction Vector", camera->foward.v);
				updateCamera |= ImGui::InputFloat3("Up Vector", camera->up.v);
				updateCamera |= ImGui::InputFloat("FOV", &camera->fov);


				updateCamera |= ParameterGUI(camera->extensionParameters);

				updateCamera |= ImGui::InputFloat("Speed", &m_cameraController.cameraSpeed);
				updateCamera |= ImGui::InputFloat("Sensitivity", &m_cameraController.sensitivity);

				static bool previewMode = true;
				updateCamera |= ImGui::Checkbox("PreviewMode", &previewMode);

				int preCamId = cameraIndex;
				std::vector<const char*> items = { "None" };
				for (auto& cameraIdx : m_scene->m_cameraObjectIndices) {
					items.push_back(m_scene->m_objects[cameraIdx]->GetObjectName());
				}
				if (ImGui::Combo("Camera", &cameraIndex, items.data(), items.size())) {
					if (cameraIndex == 0) {
						m_scene->m_camera->camera = nullptr;
					}
					else if (cameraIndex != 0) {
						auto& camIndices = m_scene->m_cameraObjectIndices;
						auto& object = m_scene->m_objects[camIndices[cameraIndex - 1]];
						if (object->GetObjectType() == ObjectType::CAMERA)
						{
							m_scene->m_camera->camera = std::static_pointer_cast<CameraObject>(object);
						}
						else {
							cameraIndex = preCamId;
						}
					}

					updateCamera = true;
				}

				if (updateCamera) {
					m_updateInfo.commands.push_back(std::make_shared<UpdateRendererCommand>());
				}
				ImGui::EndTabItem();
			}


			ImGui::EndTabBar();
		}

		if (Runtime) {
			currentFrame++;
			if (currentFrame > endFrame) {
				if (useLoopMode) {
					currentFrame = startFrame;
				}
				else {
					currentFrame = endFrame;
					Runtime = false;
				}
			}
		}
		if (Runtime) {
			m_updateInfo.commands.push_back(std::make_shared<UpdateRendererCommand>());
		}

		ImGui::End();
	}

	void Editor::ShowObjectGUI() {

		ImGui::Text("Objects");
		ImGui::Separator();
		ImGui::Spacing();

		std::vector<const char*> objectNames;
		objectNames.reserve(m_scene->m_objects.size());
		for (auto& object : m_scene->m_objects) {
			objectNames.push_back(object->GetObjectName());
		}

		static int selectedIndex = 0;
		ImGui::ListBox("Ojbect List", &selectedIndex, objectNames.data(), objectNames.size());
		auto& object = m_scene->m_objects[selectedIndex];

		ImGui::Spacing();

		ImGui::Text("Object Information");
		ImGui::Separator();

		if (ImGui::CollapsingHeader("Object Infomation")) {
			ImGui::Indent(20.0f);
			ImGui::Text("Object Name : %s", object->GetObjectName());
			ImGui::Text("Object Type : %d", object->GetObjectType());
			ImGui::Text("Object Index : %d", selectedIndex);
			ImGui::Unindent(20.0f);
		}

		bool objectUpdate = false;

		if (ImGui::CollapsingHeader("Object Type")) {
			ImGui::Indent(20.0f);
			ShrPtr<Instance> instance;
			ShrPtr<CameraObject> camera;
			switch (object->GetObjectType())
			{
			case ObjectType::INSTANCE:
				instance = std::static_pointer_cast<Instance>(object);
				ImGui::Text("Object Type : INSTANCE");
				ImGui::Text("Geometry Index : %d", instance->geometryIndex);
				break;
			case ObjectType::LIGHT:
				SKHOLE_UNIMPL();
				break;

			case ObjectType::VOLUME:
				SKHOLE_UNIMPL();
				break;

			case ObjectType::CAMERA:
				camera = std::static_pointer_cast<CameraObject>(object);
				ImGui::Text("Object Type : CAMERA");
				ImGui::InputFloat("Fov : ", &camera->yFov);
				break;

			default:
				SKHOLE_UNIMPL();
				break;
			}
			ImGui::Unindent(20.0f);
		}


		if (ImGui::CollapsingHeader("Parent-Child Relationship")) {
			ImGui::Indent(20.0f);
			ImGui::Text("Parent Object : %s", object->haveParent() ? object->parentObject->GetObjectName() : "None");
			ImGui::Text("Child Object : %s", object->haveChild() ? object->childObject->GetObjectName() : "None");
			ImGui::Unindent(20.0f);
		}


		if (ImGui::CollapsingHeader("Translation")) {
			ImGui::Indent(20.0f);
			ImGui::Text("Local Translation");
			objectUpdate |= ImGui::InputFloat3("Position", object->localTranslation.v);

			Quaternion lq = object->localQuaternion;
			vec4 lr = vec4(lq.x, lq.y, lq.z, lq.w);

			objectUpdate |= ImGui::InputFloat4("Rotation", lr.v);
			objectUpdate |= ImGui::InputFloat3("Scale", object->localScale.v);

			object->localQuaternion = Quaternion(lr.x, lr.y, lr.z, lr.w);

			ImGui::Separator();

			mat4 matrix = object->GetTransformMatrix(float(currentFrame) / fps);
			ImGui::Text("Transform Matrix");
			ImGui::Text("(%f, %f, %f, %f)", matrix[0][0], matrix[0][1], matrix[0][2], matrix[0][3]);
			ImGui::Text("(%f, %f, %f, %f)", matrix[1][0], matrix[1][1], matrix[1][2], matrix[1][3]);
			ImGui::Text("(%f, %f, %f, %f)", matrix[2][0], matrix[2][1], matrix[2][2], matrix[2][3]);
			ImGui::Text("(%f, %f, %f, %f)", matrix[3][0], matrix[3][1], matrix[3][2], matrix[3][3]);

			mat3 normalWorld = NormalTransformMatrix3x3(matrix);
			ImGui::Text("Normal Transform Matrix");
			ImGui::Text("(%f, %f, %f)", normalWorld[0][0], normalWorld[0][1], normalWorld[0][2]);
			ImGui::Text("(%f, %f, %f)", normalWorld[1][0], normalWorld[1][1], normalWorld[1][2]);
			ImGui::Text("(%f, %f, %f)", normalWorld[2][0], normalWorld[2][1], normalWorld[2][2]);

			mat3 InvNormal = Inverse3x3(normalWorld);
			mat3 test = InvNormal * normalWorld;
			mat3 test2 = Transpose3x3(InvNormal);


			ImGui::Unindent(20.0f);
		}

		if (ImGui::CollapsingHeader("Object Parameters")) {
			ImGui::Indent(20.0f);
			ImGui::Text("Use Animation : %s", object->useAnimation ? "True" : "False");
			ImGui::Unindent(20.0f);
		}

		if (objectUpdate) {
			ImGui::Indent(20.0f);
			m_updateInfo.commands.push_back(std::make_shared<UpdateObjectCommand>(selectedIndex, object->GetObjectType()));
			ImGui::Unindent(20.0f);
		}

	}

	void Editor::ShowMaterialGUI() {
		std::vector<const char*> materialNames;
		materialNames.reserve(m_scene->m_basicMaterials.size());
		for (auto& material : m_scene->m_materials) {
			materialNames.push_back(material->materialName.c_str());
		}

		static int selectedIndex = 0;
		ImGui::ListBox("Material List", &selectedIndex, materialNames.data(), materialNames.size());

		auto& mat = m_scene->m_materials[selectedIndex];
		bool materialUpdate = false;
		std::string materialName = "Material Name : " + mat->materialName;
		ImGui::Text(materialName.c_str());
		materialUpdate |= ParameterGUI(mat->materialParameters);

		if (materialUpdate) {
			m_updateInfo.commands.push_back(std::make_shared<UpdateMaterialCommand>(selectedIndex));
		}
	}

	void Editor::ShowGeometryGUI() {
		ImGui::Text("Geometry");
		ImGui::Separator();
		ImGui::Spacing();

		std::vector<const char*> geometryNames;
		geometryNames.reserve(m_scene->m_geometies.size());
		for (int i = 0; i < m_scene->m_geometies.size(); i++) {
			geometryNames.push_back("Geometry");
		}

		static int selectedIndex = 0;
		ImGui::ListBox("Geometry List", &selectedIndex, geometryNames.data(), geometryNames.size());
		auto& geometry = m_scene->m_geometies[selectedIndex];

		ImGui::Spacing();
		ImGui::Text("Geometry Information");
		ImGui::Separator();

		if (ImGui::CollapsingHeader("Geometry Infomation")) {
			ImGui::Indent(20.0f);
			ImGui::Text("Geometry ID : %d", selectedIndex);
			ImGui::Text("num of Vertices : %d", geometry->m_vertices.size());
			ImGui::Text("num of Indices : %d", geometry->m_indices.size());
			ImGui::Unindent(20.0f);
		}

		if (ImGui::CollapsingHeader("Animation")) {
			ImGui::Indent(20.0f);
			ImGui::Text("Use Animation : %s", geometry->useAnimation ? "True" : "False");
			ImGui::Unindent(20.0f);
		}

		if (ImGui::CollapsingHeader("Connect Prim ID")) {
			ImGui::Indent(20.0f);
			if (ImGui::Button("Create Connect Prim ID")) {
				CreateConnectPrimId(geometry);
			}
			ImGui::Text("Use Connect Prim ID : %s", geometry->useConnectIndex ? "True" : "False");

			if (ImGui::TreeNode("Connect Prim Id")) {
				ImGui::Text("Connect Prim Id");
				for (int i = 0; i < geometry->m_connectPrimId.size() / 3; i++)
				{
					int primIdx0 = geometry->m_connectPrimId[i * 3 + 0];
					int primIdx1 = geometry->m_connectPrimId[i * 3 + 1];
					int primIdx2 = geometry->m_connectPrimId[i * 3 + 2];

					ImGui::Text("Prim Id %d : %d, %d, %d", i, primIdx0, primIdx1, primIdx2);
				}
				ImGui::TreePop();
			}
			ImGui::Unindent(20.0f);
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

	void Editor::InitializeRendererScene(const RendererDesc& rendererDesc, ShrPtr<Renderer>& renderer, ShrPtr<Scene>& scene) {

		renderer->Initialize(rendererDesc);

		// Scene Initialization
		scene->RendererSet(renderer);
		renderer->SetScene(scene);
	}
}