#include <Application.h>
//#include <renderer/sample_renderer.h>

int main() {
	Skhole::Application app;
	
	Skhole::ApplicationDesc desc;
	desc.applicationName = "Skhole";
	desc.initWidth = 1280;
	desc.initHeight = 720;
	desc.useEditor = true;

	app.Init(desc);

	try {
		app.Run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return -1;
	}


	//Application app;
	//app.run();
	return 0;
}
