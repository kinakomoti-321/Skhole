#pragma once
#include <include.h>
#include <editor/editor.h>

namespace Skhole
{
	struct ApplicationDesc 
	{
		std::string applicationName;
		int initWidth;
		int initHeight;
		
		bool useEditor;
	};

	class Application
	{
	public:
		Application();
		~Application();

		void Init(ApplicationDesc& desc);
		void Run();

	private:
		std::shared_ptr<Editor> m_editor;
		bool m_useEditor;
	};



}
