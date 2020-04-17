// 2-Example-ClearScreen-v2.cpp - use OpenGL shader architecture

#include <glad.h>                           // GL header file
#include <GLFW/glfw3.h>                          // GL toolkit
#include <stdio.h>                          // printf, etc.
#include "GLXtras.h"                        // convenience routines

// vertex shader: operations before the rasterizer
const char *vertexShaderSource = R"(
  #version 330 core
  layout (location = 0) in vec3 point;

  void main()
  {
      gl_Position = vec4(point, 1.0);
  }
)";

// pixel shader: operations after the rasterizer
const char *fragmentShaderSource = R"(
  #version 330 core
  #define PI 3.14159265359

  uniform vec2 u_resolution;

  out vec4 fragColor;
  void main() {
      vec2 gridSize = vec2(8.0, 8.0);
      vec2 normalCoord = gl_FragCoord.xy / 300.0;
      vec2 value = sign(sin(normalCoord * PI * gridSize));
      vec3 rgb = vec3(value.x * value.y);
      fragColor = vec4(rgb, 1.0);
  }
)";

void Keyboard(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)     // test for program exit
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void GlfwError(int id, const char *reason) {
    printf("GFLW error %i: %s\n", id, reason);
    getchar();
}

void APIENTRY GlslError(GLenum source, GLenum type, GLuint id, GLenum severity,
                        GLsizei len, const GLchar *msg, const void *data) {
    printf("GLSL Error: %s\n", msg);
    getchar();
}

int AppError(const char *msg) {
    glfwTerminate();
    printf("Error: %s\n", msg);
    getchar();
    return 1;
}

int main() {                                                // application entry
    glfwSetErrorCallback(GlfwError);                        // init GL toolkit
    if (!glfwInit()) return 1;

    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    // create named window of given size
    GLFWwindow *window = glfwCreateWindow(300, 300, "Chessboard", NULL, NULL);
    if (!window) return AppError("can't open window");

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);    // set OpenGL extensions

    GLuint shaderProgram;
    if (!(shaderProgram = LinkProgramViaCode(&vertexShaderSource, &fragmentShaderSource)))
        return AppError("can't link shader program");

    glfwSetKeyCallback(window, Keyboard);

    float vertices[] = {
        -1.0f, -1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,
	 1.0f, -1.0f, 0.0f
    };


    GLubyte indices[] = {0,1,2, // first triangle (bottom left - top left - top right)
                     0,2,3}; // second triangle (bottom left - top right - bottom right)


    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glEnableVertexAttribArray(vao);
    glBindVertexArray(vao);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    while (!glfwWindowShouldClose(window)) {
        glUseProgram(shaderProgram);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}
