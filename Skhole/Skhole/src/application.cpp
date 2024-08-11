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
	}

	void Application::Run()
	{
		while (!glfwWindowShouldClose(m_window)) {
			glfwPollEvents();	
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
