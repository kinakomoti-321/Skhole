#include <Application.h>
//#include <renderer/sample_renderer.h>
#include <cxxopts.hpp>
#include <common/filepath.h>

inline void ExecuteEditor() {
	Skhole::Application app;
	Skhole::Application::EditorModeDesc desc;
	desc.applicationName = "Skhole";
	desc.initWidth = 1280;
	desc.initHeight = 720;
	desc.useEditor = true;

	app.RunEditorMode(desc);
}

inline void ExecuteRender(const Skhole::FilePath& filepath, const Skhole::FilePath& outpath) {

	std::cout << "filepath: " << filepath.GetFullPath() << std::endl;
	std::cout << "outpath: " << outpath.GetFullPath() << std::endl;

	Skhole::Application app;
	Skhole::Application::RenderModeDesc desc;
	desc.filepath = filepath;
	desc.outpath = outpath;

	app.RunRenderMode(desc);
}

int main(int argc, char* argv[]) {
	try {
		cxxopts::Options options("Skhole", "Commands");

		options.add_options()
			("h,help", "ヘルプを表示")
			("f,file", "入力ファイル", cxxopts::value<std::string>())
			("o,out", "出力先", cxxopts::value<std::string>())
			("m,mode", "モード editor, render", cxxopts::value<std::string>());

		auto result = options.parse(argc, argv);

		if (argc == 1) {
			ExecuteEditor();
		}
		else {
			if (result.count("help")) {
				std::cout << options.help() << std::endl;
				return 0;
			}

			if (result.count("mode")) {
				std::string mode = result["mode"].as<std::string>();
				if (mode == "editor") {
					std::cout << "------------Execute Editor Mode----------------" << std::endl;
					ExecuteEditor();
				}
				if (mode == "render") {
					std::cout << "------------Execute Render Mode----------------" << std::endl;

					Skhole::FilePath filepath;
					Skhole::FilePath outpath;

					if (result.count("file")) {
						//std::cout << "file: " << result["file"].as<std::string>() << std::endl;
						filepath.SetPath(result["file"].as<std::string>());
					}

					if (result.count("out")) {
						//std::cout << "out: " << result["out"].as<std::string>() << std::endl;
						outpath.SetPath(result["out"].as<std::string>());
					}

					ExecuteRender(filepath, outpath);
				}
				else {
					std::cout << "Invalid Mode" << std::endl;
					std::cout << "------------Execute Editor Mode----------------" << std::endl;
					ExecuteEditor();
				}

			}


		}
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return -1;
	}




	return 0;
}
