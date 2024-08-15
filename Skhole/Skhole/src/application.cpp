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

	void Application::Init(ApplicationDesc& desc)
	{
		m_useEditor = desc.useEditor;
		if (m_useEditor) {
			// Editor Mode
			m_editor = std::make_shared<Editor>();

			EditorInitializationDesc editorDesc;
			editorDesc.applicationName = desc.applicationName;
			editorDesc.windowWidth = desc.initWidth;
			editorDesc.windowHeight = desc.initHeight;
			m_editor->Init(editorDesc);
		}
		else {
			// Render Mode
			SKHOLE_UNIMPL("Not Imple Render Mode");
		}
	}

	void Application::Run()
	{
		if (m_useEditor) {
			m_editor->Run();
		}
		else {
			SKHOLE_UNIMPL("Not Imple Render Mode");
		}
	}
		
}
