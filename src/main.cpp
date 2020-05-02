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

class Mesh {
public:
    vector<vec3> points;
    vector<vec3> normals;
    vector<vec2> uvs;
    vector<int3> triangles;
    mat4 xform;
    unsigned int vbo, vao;

    Mesh() { };

    bool Read(const char *meshName, const char *textureName) {
        ReadAsciiObj(meshName, points, triangles, &normals, &uvs);
        return true;
    }

    void Buffer() {
        /*
        int sizePoints = points.size() * sizeof(vec3);

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizePoints, NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizePoints, &points[0]);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        */
    }

    void Draw() {
        /*
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
        */
    }
};

// Shaders
const char *vertexShaderSource = R"(
    #version 410 core
    layout (location = 0) in vec3 point;
    uniform mat4 model;
    uniform mat4 cameraView;
    void main() {
        gl_Position = cameraView * model * vec4(point, 1.0);
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

Camera camera(SCR_WIDTH, SCR_HEIGHT, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -10.0f), 30);

Mesh mesh;
const char *cubeObj = "./assets/cube.obj";
const char *catTex = "‎⁨.⁩/assets/lily.tga⁩";

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

    mesh.Read(cubeObj, catTex);

    std::vector<vec3> points {
        vec3(1.0f,  1.0f, 0.0f),
        vec3(1.0f, -1.0f, 0.0f),
        vec3(-1.0f, -1.0f, 0.0f)
    };

    unsigned int vbo, vao;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    int bufferSize = points.size() * sizeof(vec3);
    glBufferData(GL_ARRAY_BUFFER, bufferSize, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, bufferSize, &points[0]);

    mat4 model = Translate(0.0f, 0.0f, 0.0f);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Set shader uniforms
        SetUniform(shaderProgram, "model", model);
        SetUniform(shaderProgram, "cameraView", camera.fullview);

        glBindVertexArray(vao);
        VertexAttribPointer(shaderProgram, "point", 3, 0, (void *) 0);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);

    glfwTerminate();

    return 0;
}
