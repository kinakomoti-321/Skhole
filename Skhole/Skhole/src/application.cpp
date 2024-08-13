#include <application.h>
#include <common/log.h>

namespace Skhole
{
	Application::Application()
	{

	}

	Application::~Application()
	{
		Destroy();	
	}

	void Application::Init(ApplicationDesc& desc)
	{
		m_desc = desc;
		
		if (m_desc.useWindow) {
			glfwInit();
			glfwWindowHint(GLFW_CLIENT_API,GLFW_NO_API);
			glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

			m_window = glfwCreateWindow(m_desc.Width, m_desc.Height, m_desc.Name.c_str(), nullptr, nullptr);

			if (!m_window) {
				SKHOLE_ERROR("Failed to create window");
				glfwTerminate();
				return;
			}
		}
		else {
			SKHOLE_UNIMPL("Offscreen rendering");
		}

		RendererDesc rendererDesc;
		rendererDesc.Name = "Skhole";
		rendererDesc.Width = m_desc.Width;
		rendererDesc.Height = m_desc.Height;
		rendererDesc.useWindow = m_desc.useWindow;
		rendererDesc.window = m_window;

		m_renderer = std::make_shared<SimpleRaytracer>();
		m_renderer->Init(rendererDesc);
	}

	void Application::Run()
	{
		while (!glfwWindowShouldClose(m_window)) {
			glfwPollEvents();	
			m_renderer->Render();
		}
	}

	void Application::Destroy()
	{
		if (!m_window){
			glfwDestroyWindow(m_window);
		}
		glfwTerminate();
	}
}
