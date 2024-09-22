import glfw
from OpenGL.GL import *
import numpy as np
import imgui
from imgui.integrations.glfw import GlfwRenderer

# 頂点シェーダー
vertex_shader_code = """
#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
out vec3 fragNormal;
out vec3 fragPosition;
void main()
{
    gl_Position = vec4(position, 1.0);
    fragNormal = normal;
    fragPosition = position;
}
"""

# フラグメントシェーダー
fragment_shader_code = """
#version 330 core
in vec3 fragNormal;
in vec3 fragPosition;
uniform vec3 lightDirection;
out vec4 fragColor;
void main()
{
    // 法線の正規化
    vec3 norm = normalize(fragNormal);
    
    // ライティング計算 (拡散反射)
    float diff = max(dot(norm, -lightDirection), 0.0);
    
    // 基本的な白色の物体に光源の拡散反射を適用
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    vec3 objectColor = vec3(0.5, 0.7, 0.2);
    vec3 diffuse = diff * lightColor;

    fragColor = vec4((diffuse + objectColor) * 0.5, 1.0);  // 拡散反射 + 物体色
}
"""

# シェーダーコンパイル用の関数
def compile_shader(source, shader_type):
    shader = glCreateShader(shader_type)
    glShaderSource(shader, source)
    glCompileShader(shader)

    if glGetShaderiv(shader, GL_COMPILE_STATUS) != GL_TRUE:
        raise RuntimeError(glGetShaderInfoLog(shader))
    
    return shader

# シェーダープログラムの作成
def create_shader_program(vertex_code, fragment_code):
    vertex_shader = compile_shader(vertex_code, GL_VERTEX_SHADER)
    fragment_shader = compile_shader(fragment_code, GL_FRAGMENT_SHADER)

    program = glCreateProgram()
    glAttachShader(program, vertex_shader)
    glAttachShader(program, fragment_shader)
    glLinkProgram(program)

    if glGetProgramiv(program, GL_LINK_STATUS) != GL_TRUE:
        raise RuntimeError(glGetProgramInfoLog(program))
    
    glDeleteShader(vertex_shader)
    glDeleteShader(fragment_shader)

    return program

# GLFW初期化とウィンドウ作成
if not glfw.init():
    raise Exception("GLFW can't be initialized")

window = glfw.create_window(800, 600, "OpenGL Plane with Normals and Lighting", None, None)

if not window:
    glfw.terminate()
    raise Exception("GLFW window can't be created")

glfw.make_context_current(window)

# ImGuiの初期化
imgui.create_context()
impl = GlfwRenderer(window)

# 頂点データ (2D平面の四角形の頂点とその法線)
plane_vertices = np.array([
    # 位置               # 法線 (Z軸方向)
    [-0.5, -0.5, 0.0,    0.0, 0.0, 1.0],
    [ 0.5, -0.5, 0.0,    0.0, 0.0, 1.0],
    [ 0.5,  0.5, 0.0,    0.0, 0.0, 1.0],
    [-0.5,  0.5, 0.0,    0.0, 0.0, 1.0]
], dtype=np.float32)

# 頂点インデックス (2つの三角形で平面を描画)
plane_indices = np.array([0, 1, 2, 2, 3, 0], dtype=np.uint32)

# VBOとVAOの設定
VAO = glGenVertexArrays(1)
VBO = glGenBuffers(1)
EBO = glGenBuffers(1)

glBindVertexArray(VAO)

glBindBuffer(GL_ARRAY_BUFFER, VBO)
glBufferData(GL_ARRAY_BUFFER, plane_vertices.nbytes, plane_vertices, GL_STATIC_DRAW)

glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO)
glBufferData(GL_ELEMENT_ARRAY_BUFFER, plane_indices.nbytes, plane_indices, GL_STATIC_DRAW)

# 頂点の位置属性 (0番目のレイアウト)
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * plane_vertices.itemsize, ctypes.c_void_p(0))
glEnableVertexAttribArray(0)

# 法線の属性 (1番目のレイアウト)
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * plane_vertices.itemsize, ctypes.c_void_p(3 * plane_vertices.itemsize))
glEnableVertexAttribArray(1)

# シェーダープログラムの作成
shader_program = create_shader_program(vertex_shader_code, fragment_shader_code)

# 光源の方向
light_direction = np.array([0.0, 0.0, 1.0], dtype=np.float32)

# メイン描画ループ
while not glfw.window_should_close(window):
    glfw.poll_events()
    impl.process_inputs()

    imgui.new_frame()

    # ImGui GUI: 光源の方向を操作
    imgui.begin("Light Direction")
    changed, light_direction = imgui.slider_float3("Direction", *light_direction, -1.0, 1.0)
    imgui.end()

    # 画面をクリア
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

    # シェーダープログラムの使用
    glUseProgram(shader_program)

    # 光源の方向をシェーダーに渡す
    light_loc = glGetUniformLocation(shader_program, "lightDirection")
    glUniform3fv(light_loc, 1, light_direction)

    # 平面を描画
    glBindVertexArray(VAO)
    glDrawElements(GL_TRIANGLES, len(plane_indices), GL_UNSIGNED_INT, None)

    # ImGuiの描画
    imgui.render()
    impl.render(imgui.get_draw_data())

    glfw.swap_buffers(window)

# 終了処理
impl.shutdown()
glDeleteVertexArrays(1, VAO)
glDeleteBuffers(1, VBO)
glDeleteBuffers(1, EBO)
glfw.terminate()
