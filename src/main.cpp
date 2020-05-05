#include "model.h"
#include "particle.h"

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
    layout (location = 1) in vec3 normal;

    out vec3 vPoint;
    out vec3 vNormal;

    uniform mat4 modelTrans;
    uniform mat4 cameraView;

    void main() {
        vPoint = (modelTrans * vec4(point, 1)).xyz;
        vNormal = (modelTrans * vec4(normal, 0)).xyz;
        gl_Position = cameraView * modelTrans * vec4(point, 1.0);
    }
)";

const char *fragmentShaderSource = R"(
    #version 410 core

    in vec3 vPoint;
    in vec3 vNormal;

    out vec4 fragColor;

    void main() {
        vec3 light = vec3(0.5f, 5.0f, 10.0f);
        vec3 N = normalize(vNormal);            // surface normal
        vec3 L = normalize(light - vPoint);     // light vector
        vec3 E = normalize(vPoint);             // eye vector
        vec3 R = reflect(L, N);                 // highlight vector
        float d = dot(N, L);                    //  diffuse
        float s = dot(R, E);                    //  specular
        //float intensity = clamp(d + pow(s, 50.0f), 0.0f, 1.0f);
        float intensity = clamp(d, 0.05f, 1.0f);
        vec3 rgb = vec3(1.0f, 0.5f, 0.2f) * intensity;
        fragColor = vec4(rgb, 1.0f);
    }
)";

unsigned int SCR_WIDTH = 640;
unsigned int SCR_HEIGHT = 400;

Camera camera(SCR_WIDTH, SCR_HEIGHT, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -40.0f), 30);
Model sphereModel;
Model cubeModel;

Particle particle(vec3(0.0f, 5.0f, 0.0f));

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
    camera.Resize(width, height);
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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Vertex Attrib", NULL, NULL);

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


    sphereModel.read("./assets/sphere.obj", "‎⁨.⁩/assets/lily.tga⁩");
    //cubeModel.read("./assets/cube.obj", "‎⁨.⁩/assets/lily.tga⁩");


    unsigned int vao, vbo, ebo;

    std::vector<vec3> points = {
        vec3(0.0f, 0.0f, 0.0f),
        vec3(1.0f, 0.0f, 0.0f),
        vec3(0.0f, 1.0f, 0.0f)
    };

    std::vector<vec3> normals = {
        vec3(0.0f, 0.0f, 1.0f),
        vec3(0.0f, 0.0f, 1.0f),
        vec3(0.0f, 0.0f, 1.0f)
    };

    std::vector<vec3> triangles = {
        vec3(0, 1, 2)
    };

    /*
    int pointsSize = points.size() * sizeof(vec3);
    int normalsSize = normals.size() * sizeof(vec3);
    int bufferSize = pointsSize + normalsSize;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, bufferSize, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, bufferSize, &points[0]);
    glBufferSubData(GL_ARRAY_BUFFER, pointsSize, normalsSize, &normals[0]);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vec3) * triangles.size(), &triangles[0], GL_STATIC_DRAW);

    glBindVertexArray(0);
    */

    //sphereModel.xform = Translate(particle.position);
    //sphereModel.xform = Translate(-2.0f, 0.0f, 0.0f);
    //cubeModel.xform = Translate(2.0f, 0.0f, 0.0f);

    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        double timeDelta = currentTime - lastTime;
        lastTime = currentTime;

        particle.update(timeDelta);
        sphereModel.xform = Translate(particle.position);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        glUseProgram(shaderProgram);
        SetUniform(shaderProgram, "cameraView", camera.fullview);

        sphereModel.draw(shaderProgram);
        //cubeModel.draw(shaderProgram);

        /*
        mat4 xform = Translate(0.0f, 0.0f, 0.0f);
        SetUniform(shaderProgram, "modelTrans", xform);

        glBindVertexArray(vao);

        // Set vertex attributes
        int pointsSize = points.size() * sizeof(vec3);
        VertexAttribPointer(shaderProgram, "point", 3, 0, (void *) 0);
        VertexAttribPointer(shaderProgram, "normal", 3, 0, (void *) pointsSize);

        // Draw triangles
        glDrawElements(GL_TRIANGLES, 3 * triangles.size(), GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
        */

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}
