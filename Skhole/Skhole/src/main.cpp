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

// A x^2 + B x + C = 0
// return true if the equation has real roots
inline bool QuadraticSolver(float A, float B, float C, float& r1, float& r2)
{
	if (A == 0) {
		if (B == 0) {
			return false;
		}
		else {
			r1 = -C / B;
			return true;
		}
	}
	else {
		float D = B * B - 4.0 * A * C;
		if (D < 0.0) return false;

		float sqrtD = std::sqrt(D);
		r1 = (-B + sqrtD) / (2.0 * A);
		r2 = (-B - sqrtD) / (2.0 * A);

		return true;
	}
}

inline bool IntersectTriangle(const vec3& o, const vec3& d, const vec3& v1, const vec3& v2, const vec3& v3, vec3& hitPoint) {

}

int main(int argc, char* argv[]) {
	//try {
	//	cxxopts::Options options("Skhole", "Commands");

	//	options.add_options()
	//		("h,help", "ヘルプを表示")
	//		("f,file", "入力ファイル", cxxopts::value<std::string>())
	//		("o,out", "出力先", cxxopts::value<std::string>())
	//		("m,mode", "モード editor, render", cxxopts::value<std::string>());

	//	auto result = options.parse(argc, argv);

	//	if (argc == 1) {
	//		ExecuteEditor();
	//	}
	//	else {
	//		if (result.count("help")) {
	//			std::cout << options.help() << std::endl;
	//			return 0;
	//		}

	//		if (result.count("mode")) {
	//			std::string mode = result["mode"].as<std::string>();
	//			if (mode == "editor") {
	//				std::cout << "------------Execute Editor Mode----------------" << std::endl;
	//				ExecuteEditor();
	//			}
	//			if (mode == "render") {
	//				std::cout << "------------Execute Render Mode----------------" << std::endl;

	//				Skhole::FilePath filepath;
	//				Skhole::FilePath outpath;

	//				if (result.count("file")) {
	//					std::cout << "file: " << result["file"].as<std::string>() << std::endl;
	//					filepath.SetPath(result["file"].as<std::string>());
	//				}

	//				if (result.count("out")) {
	//					std::cout << "out: " << result["out"].as<std::string>() << std::endl;
	//					outpath.SetPath(result["out"].as<std::string>());
	//				}

	//				ExecuteRender(filepath, outpath);
	//			}
	//			else {
	//				std::cout << "Invalid Mode" << std::endl;
	//				std::cout << "------------Execute Editor Mode----------------" << std::endl;
	//				ExecuteEditor();
	//			}

	//		}


	//	}
	//}
	//catch (const std::exception& e) {
	//	std::cerr << e.what() << std::endl;
	//	return -1;
	//}

	mat3 m1 = mat3(vec3(2, 3, 2), vec3(1, 4, 3), vec3(2, 1, 4));
	mat3 m2 = mat3(vec3(3, 1, 2), vec3(2, 4, 2), vec3(1, 5, 2));

	mat3 m3 = m1 * m2;

	// Execute
	vec3 L = vec3(0.0, 2.0, 0.0);
	vec3 triangle[3] = { vec3(0.0, 1.0, 1.0) ,vec3(-1.0, 1.0, -1.0), vec3(1.0, 1.0, -1.0) };
	vec3 normal[3] = { normalize(vec3(0.0, 1.0, 0.2)), normalize(vec3(-0.1, 1.0, -0.1)), normalize(vec3(0.1, 1.0, -0.1)) };
	float ior = 1.5;

	vec3 P = vec3(0.0, 0.0, 0.0);

	vec3 PL = L - P;
	vec3 V0V1 = triangle[1] - triangle[0];
	vec3 V0V2 = triangle[2] - triangle[0];
	vec3 PV0 = triangle[0] - P;

	mat3 mN = mat3(normal[1] - normal[0], normal[2] - normal[0], normal[0]);

	vec3 mQ0 = cross(PL, V0V1);
	vec3 mQ1 = cross(PL, V0V2);
	vec3 mQ2 = cross(PL, PV0);

	//mat3 mQ = transpose(mat3(cross(PL, V0V1), cross(PL, V0V2), cross(PL, PV0)));
	mat3 mQ = transpose(mat3(cross(PL, V0V1), cross(PL, V0V2), cross(PL, PV0)));

	mat3 conicM = mN * mQ;

	// A a^2 + B b^2 + C ab + D a + E b + F = 0
	float A = conicM[0][0];
	float B = conicM[1][1];
	float C = conicM[0][1] + conicM[1][0];
	float D = conicM[0][2] + conicM[2][0];
	float E = conicM[1][2] + conicM[2][1];
	float F = conicM[2][2];

	// Alpha = 0
	float solBp, solBm;
	bool solB = QuadraticSolver(B, D, F, solBp, solBm);

	// Beta = 0
	float solAp, solAm;
	bool solA = QuadraticSolver(A, C, F, solAp, solAm);

	// Alpha + Beta = 1
	float solCp, solCm;
	bool solC = QuadraticSolver(A + B - C, C + E - 2.0 * A - D, A + D + F, solCp, solCm);

	std::cout << "solB: " << solB << std::endl;
	std::cout << "solA: " << solA << std::endl;
	std::cout << "solC: " << solC << std::endl;

	return 0;
}
