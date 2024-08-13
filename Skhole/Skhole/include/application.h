#pragma once
#include <include.h>
#include <renderer/renderer.h>
#include <renderer/simple_raytracer.h>

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
		std::shared_ptr<Renderer> m_renderer;
	};



}
