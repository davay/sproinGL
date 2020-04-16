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

  out vec4 fragColor;
  void main()
  {
    vec2 value = sign(sin((gl_PointCoord.xy) * 12.5));
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

    // following line will not compile unless glad.h >= OpenGLv4.3
    // glDebugMessageCallback(GlslError, NULL); // So we just don't use it
    // REQUIREMENT 2) build shader program
    GLuint shaderProgram;
    if (!(shaderProgram = LinkProgramViaCode(&vertexShaderSource, &fragmentShaderSource)))
        return AppError("can't link shader program");

    glfwSetKeyCallback(window, Keyboard);

    float vertices[] = {
        -1.0f, -1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 0.0f
    };

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    while (!glfwWindowShouldClose(window)) {
        glUseProgram(shaderProgram);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}
