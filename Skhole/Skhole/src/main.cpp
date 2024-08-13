#include <Application.h>
//#include <renderer/sample_renderer.h>

int main() {
	Skhole::Application app;
	
	Skhole::ApplicationDesc desc;

	desc.Name = "Skhole";
	desc.Width = 1280;
	desc.Height = 720;

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
