#include "model.h"

#include "Camera.h"
#include "Draw.h"
#include "GLXtras.h"
#include "Mesh.h"
#include "Misc.h"
#include "VecMat.h"

#include <glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdio.h>

// Shaders
const char *vertexShaderSource = R"(
    #version 410 core
    layout (location = 0) in vec3 point;
    uniform mat4 modelTrans;
    uniform mat4 cameraView;
    void main() {
        gl_Position = cameraView * modelTrans * vec4(point, 1.0);
    }
)";

const char *fragmentShaderSource = R"(
    #version 410 core
    in vec3 ourColor;
    out vec4 FragColor;
    void main() {
      FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
    }
)";

unsigned int SCR_WIDTH = 640;
unsigned int SCR_HEIGHT = 400;

Camera camera(SCR_WIDTH, SCR_HEIGHT, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -8.0f), 30);
Model model;

bool Shift(GLFWwindow *w) {
    return glfwGetKey(w, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
           glfwGetKey(w, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
}

void MouseButton(GLFWwindow *w, int butn, int action, int mods) {
    if (action == GLFW_PRESS) {
        double x, y;
        glfwGetCursorPos(w, &x, &y);
        camera.MouseDown((int) x, (int) y);
    }
    if (action == GLFW_RELEASE)
        camera.MouseUp();
}

void MouseMove(GLFWwindow *w, double x, double y) {
    if (glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        camera.MouseDrag((int) x, (int) y, Shift(w));
}

void MouseWheel(GLFWwindow *w, double xoffset, double yoffset) {
    camera.MouseWheel(yoffset, Shift(w));
}

void Keyboard(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        float fieldOfView = camera.GetFOV();
        bool shift = mods & GLFW_MOD_SHIFT;
        if (key == GLFW_KEY_ESCAPE)
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        fieldOfView += key == 'F'? shift? -5 : 5 : 0;
        fieldOfView = fieldOfView < 5? 5 : fieldOfView > 150? 150 : fieldOfView;
        camera.SetFOV(fieldOfView);
    }
}

void Resize(GLFWwindow *window, int width, int height) {
    camera.Resize(width/2, height);
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GL_FALSE);
#endif

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Chessboard", NULL, NULL);

    glfwMakeContextCurrent(window);
    //glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // Build and compile shader program
    int shaderProgram = LinkProgramViaCode(&vertexShaderSource, &fragmentShaderSource);

    // callbacks
    glfwSetCursorPosCallback(window, MouseMove);
    glfwSetMouseButtonCallback(window, MouseButton);
    glfwSetScrollCallback(window, MouseWheel);
    glfwSetKeyCallback(window, Keyboard);
    glfwSetWindowSizeCallback(window, Resize);

    model.read("./assets/cube.obj", "‎⁨.⁩/assets/lily.tga⁩");

    double lastTime = glfwGetTime();
    mat4 modelTrans = Translate(0.0f, 0.0f, 0.0f);

    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        double timeDelta = currentTime - lastTime;
        lastTime = currentTime;

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Define vertex transformations
        modelTrans = RotateY(currentTime * 100.0f);

        glUseProgram(shaderProgram);
        SetUniform(shaderProgram, "modelTrans", modelTrans);
        SetUniform(shaderProgram, "cameraView", camera.fullview);

        model.draw(shaderProgram);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}
