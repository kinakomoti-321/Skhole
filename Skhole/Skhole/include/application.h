#pragma once
#include <include.h>

namespace Skhole
{
	struct ApplicationDesc 
	{
		std::string Name;
		int Width;
		int Height;

		bool useWindow;
	};

	class Application
	{
	public:
		Application();
		~Application();

		void Init(ApplicationDesc& desc);
		void Run();

	private:
		void Destroy();

	private:
		GLFWwindow* m_window;
		ApplicationDesc m_desc;

	};



}
