#include <application.h>
#include <common/log.h>

namespace Skhole
{
	Application::Application()
	{

	}

	Application::~Application()
	{
	}

	void Application::RunEditorMode(EditorModeDesc& desc)
	{
		// Editor Mode
		m_editor = std::make_shared<Editor>();

		EditorInitializationDesc editorDesc;
		editorDesc.applicationName = desc.applicationName;
		editorDesc.windowWidth = desc.initWidth;
		editorDesc.windowHeight = desc.initHeight;
		m_editor->Init(editorDesc);
		m_editor->Run();
	}

	void Application::RunRenderMode(RenderModeDesc& desc)
	{
		// Render Mode	
		auto& filepath = desc.filepath;
		auto& outpath = desc.outpath;
		auto ext = filepath.GetExtension();
		if (filepath.GetExtension() != ".json") {
			throw std::runtime_error("Invalid file format. Please specify a json file.");
		}

		ImportSettingOutput importInfo = ImportSetting(filepath.GetPath(), filepath.GetFileName());
		if (importInfo.success == false) {
			throw std::runtime_error("Failed to import the scene." + filepath.GetPath() + filepath.GetFileName());
		}
		auto& scene = importInfo.scene;
		auto& renderInfo = importInfo.offlineRenderingInfo;
		auto& renderer = importInfo.renderer;

		renderInfo.filename = outpath.GetFileName();
		renderInfo.filepath = outpath.GetPath();

		RendererDesc rendererDesc;
		rendererDesc.Name = "OfflineRenderer";
		rendererDesc.Width = 1024;
		rendererDesc.Height = 720;
		rendererDesc.useWindow = false;

		// Render
		renderer->Initialize(rendererDesc);
		renderer->SetScene(scene);

		// Render
		renderer->OfflineRender(renderInfo);

		renderer->Destroy();

		SKHOLE_LOG("Render Completed.");
	}

}
