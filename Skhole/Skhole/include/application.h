#pragma once
#include <include.h>
#include <editor/editor.h>
#include <scene/scene.h>
#include <scene/scene_exporter.h>
#include <renderer/renderer.h>

namespace Skhole
{

	class Application
	{
	public:
		struct EditorModeDesc
		{
			std::string applicationName;
			int initWidth;
			int initHeight;

			bool useEditor;
		};

		struct RenderModeDesc
		{
			FilePath filepath;
			FilePath outpath;
		};

	public:
		Application();
		~Application();

		void RunEditorMode(EditorModeDesc& desc);
		void RunRenderMode(RenderModeDesc& desc);

	private:
		std::shared_ptr<Editor> m_editor;
	};


}
