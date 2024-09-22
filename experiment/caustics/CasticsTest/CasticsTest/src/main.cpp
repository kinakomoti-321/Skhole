#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

// �V�F�[�_�[�\�[�X�R�[�h
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

// �V�F�[�_�[�̃R���p�C�����s���֐�
unsigned int compileShader(unsigned int type, const char* source) {
	unsigned int shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, nullptr);
	glCompileShader(shader);

	// �R���p�C���G���[�̃`�F�b�N
	int success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(shader, 512, nullptr, infoLog);
		std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	return shader;
}

// �V�F�[�_�[�v���O�����̃����N���s���֐�
unsigned int createShaderProgram() {
	unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
	unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

	unsigned int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	// �����N�G���[�̃`�F�b�N
	int success;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
		std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	// �V�F�[�_�[�̍폜
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

int main() {
	// GLFW�̏�����
	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return -1;
	}

	// OpenGL�o�[�W�����ƃv���t�@�C���̐ݒ�
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// �E�B���h�E�̍쐬
	GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Plane", nullptr, nullptr);
	if (!window) {
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	// �R���e�L�X�g�̍쐬
	glfwMakeContextCurrent(window);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	// GLAD�̏�����
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// �r���[�|�[�g�̐ݒ�
	glViewport(0, 0, 800, 600);

	// �V�F�[�_�[�v���O�����̍쐬
	unsigned int shaderProgram = createShaderProgram();

	// ���ʂ̒��_�f�[�^
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

	// VAO, VBO, EBO�̐���
	unsigned int VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	// VAO�̃o�C���h
	glBindVertexArray(VAO);

	// VBO�̃o�C���h�ƃf�[�^�̓]��
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

	// EBO�̃o�C���h�ƃf�[�^�̓]��
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(planeIndices), planeIndices, GL_STATIC_DRAW);

	// ���_�����̐ݒ�
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// �o�b�t�@�̃o�C���h������
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// �`�惋�[�v
	while (!glfwWindowShouldClose(window)) {
		// ���͏���
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Hello, world!");

		ImGui::Text("This is some useful text.");

		ImGui::End();

		ImGui::Render();

		// �w�i�F�̐ݒ�
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// �V�F�[�_�[�v���O�����̎g�p
		glUseProgram(shaderProgram);

		// VAO�̃o�C���h�ƕ`��
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// �o�b�t�@�̌����ƃC�x���g�̃|�[�����O
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// ���\�[�X�̉��
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteProgram(shaderProgram);

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// GLFW�̏I������
	glfwTerminate();
	return 0;
}