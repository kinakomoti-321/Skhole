#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

// シェーダーソースコード
const char* vertexShaderSource = R"(
#version 460 core
layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNormal;

layout (location = 0) out vec3 normal;
void main()
{
    gl_Position = vec4(vPos, 1.0);
	normal = vNormal;
}
)";

const char* fragmentShaderSource = R"(
#version 460 core

layout(location = 0) in vec3 normal;
out vec4 FragColor;

void main()
{
	vec3 col = normal * 0.5 + 0.5;
    FragColor = vec4(col,1.0);
}
)";

// シェーダーのコンパイルを行う関数
unsigned int compileShader(unsigned int type, const char* source) {
	unsigned int shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, nullptr);
	glCompileShader(shader);

	// コンパイルエラーのチェック
	int success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(shader, 512, nullptr, infoLog);
		std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	return shader;
}

// シェーダープログラムのリンクを行う関数
unsigned int createShaderProgram() {
	unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
	unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

	unsigned int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	// リンクエラーのチェック
	int success;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
		std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	// シェーダーの削除
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

int main() {
	// GLFWの初期化
	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return -1;
	}

	// OpenGLバージョンとプロファイルの設定
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// ウィンドウの作成
	GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Plane", nullptr, nullptr);
	if (!window) {
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	// コンテキストの作成
	glfwMakeContextCurrent(window);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	// GLADの初期化
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// ビューポートの設定
	glViewport(0, 0, 800, 600);

	// シェーダープログラムの作成
	unsigned int shaderProgram = createShaderProgram();

	// 平面の頂点データ
	float planeVertices[] = {
		// positions, normal
		-0.5f, -0.5f, 0.0, 0.0, 0.0, 1.0,
		 0.5f, -0.5f, 0.0, 0.0, 0.0, 1.0,
		 0.5f, 0.5f,  0.0, 0.0, 0.0, 1.0,
		-0.5f, 0.5f,  0.0, 0.0, 0.0, 1.0
	};

	unsigned int planeIndices[] = {
		0, 1, 2,
		2, 3, 0
	};

	// VAO, VBO, EBOの生成
	unsigned int VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	// VAOのバインド
	glBindVertexArray(VAO);

	// VBOのバインドとデータの転送
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

	// EBOのバインドとデータの転送
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(planeIndices), planeIndices, GL_STATIC_DRAW);

	// 頂点属性の設定
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// バッファのバインドを解除
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// 描画ループ
	while (!glfwWindowShouldClose(window)) {
		// 入力処理
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Hello, world!");

		ImGui::Text("This is some useful text.");

		ImGui::End();

		ImGui::Render();

		// 背景色の設定
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// シェーダープログラムの使用
		glUseProgram(shaderProgram);

		// VAOのバインドと描画
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// バッファの交換とイベントのポーリング
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// リソースの解放
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteProgram(shaderProgram);

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// GLFWの終了処理
	glfwTerminate();
	return 0;
}