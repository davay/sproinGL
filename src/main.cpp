#include "model.h"
#include "Particle.h"
#include "Spring.h"
#include "PhysicsManager.h"

#include "Camera.h"
#include "Draw.h"
#include "GLXtras.h"
#include "Mesh.h"
#include "Misc.h"
#include "VecMat.h"

#include <glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Shaders
const char *vertexShaderSource = R"(
    #version 410 core

    layout (location = 0) in vec3 point;
    layout (location = 1) in vec3 normal;

    out vec3 vPoint;
    out vec3 vNormal;
    out vec3 vColor;

    uniform mat4 modelTrans;
    uniform vec3 modelColor;
    uniform mat4 cameraView;

    void main() {
        vPoint = (modelTrans * vec4(point, 1)).xyz;
        vNormal = (modelTrans * vec4(normal, 0)).xyz;
        vColor = modelColor;
        gl_Position = cameraView * modelTrans * vec4(point, 1.0);
    }
)";

const char *fragmentShaderSource = R"(
    #version 410 core

    in vec3 vPoint;
    in vec3 vNormal;
    in vec3 vColor;

    out vec4 fragColor;

    void main() {
        vec3 light = vec3(0.0f, 6.0f, 8.0f);
        vec3 N = normalize(vNormal);            // surface normal
        vec3 L = normalize(light - vPoint);     // light vector
        vec3 E = normalize(vPoint);             // eye vector
        vec3 R = reflect(L, N);                 // highlight vector
        float d = dot(N, L);                    //  diffuse
        float s = dot(R, E);                    //  specular
        //float intensity = clamp(d + pow(s, 50.0f), 0.0f, 1.0f);
        float intensity = clamp(d, 0.05f, 1.0f);
        vec3 rgb = vColor * intensity;
        fragColor = vec4(rgb, 1.0f);
    }
)";

const unsigned int SCREEN_WIDTH = 640;
const unsigned int SCREEN_HEIGHT = 400;

Camera camera(SCREEN_WIDTH, SCREEN_HEIGHT, vec3(20.0f, -5.0f, 0.0f), vec3(0.0f, -2.0f, -40.0f), 30);

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
    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "SproinGL", NULL, NULL);

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
    //glfwSetKeyCallback(window, Keyboard);
    glfwSetWindowSizeCallback(window, Resize);

    // Load models
    Model sphereModel(vec3(1.0f, 0.5f, 0.2f));
    Model cubeModel(vec3(1.0f, 0.3f, 0.4f));
    Model cylinderModel(vec3(1.0f, 1.0f, 1.0f));
    sphereModel.read("./assets/sphere.obj", "‎⁨.⁩/assets/lily.tga⁩");
    cubeModel.read("./assets/cube.obj", "‎⁨.⁩/assets/lily.tga⁩");
    cylinderModel.read("./assets/cylinder.obj", "‎⁨.⁩/assets/lily.tga⁩");

    // Initialize game objects
    PhysicsManager pm;
    std::vector<Particle*> particles;
    std::vector<Spring*> springs;

    /*
    particles.push_back(new Particle(vec3(3.0f, 6.0f, -4.0f), vec3(0.0f, 0.0f, 0.0f)));
    particles.push_back(new Particle(vec3(3.0f, 2.0f, -4.0f), vec3(0.0f, 0.0f, 0.0f)));

    springs.push_back(new Spring(particles[0], particles[1], 4.0f, 0.04f, 0.01f));
    */

    for (int i = 0; i < 6; i++) {
        particles.push_back(new Particle(vec3(i * 3, 10.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f)));
    }

    for (int i = 0; i < 5; i++) {
        springs.push_back(new Spring(particles[i], particles[i + 1], 3, 0.1, 0.05));
    }

    Particle *player = new Particle(vec3(0, 0, 0), vec3(0, 0, 0));
    particles.push_back(player);

    srand(time(NULL));
    double lastTime = glfwGetTime();
    double particleTimer = 0;

    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        double timeDelta = currentTime - lastTime;
        lastTime = currentTime;

        //particleTimer++;
        if (particleTimer > 120 && particles.size() < 10) {
            vec3 p1Position(rand() % 20 - 10, rand() % 10, rand() % 20 - 10);
            vec3 p2Position = p1Position + vec3(0.0f, 1.0f, 0.0f);

            vec3 p1Velocity((rand() % 100 - 50) * 0.005, (rand() % 100 - 50) * 0.005, (rand() % 100 - 50) * 0.005);
            vec3 p2Velocity((rand() % 100 - 50) * 0.005, (rand() % 100 - 50) * 0.005, (rand() % 100 - 50) * 0.005);

            Particle *p1 = new Particle(p1Position, p1Velocity);
            Particle *p2 = new Particle(p2Position, p2Velocity);

            particles.push_back(p1);
            particles.push_back(p2);

            Spring *s = new Spring(p1, p2, rand() % 2 + 3, 0.04f, 0.01f);
            springs.push_back(s);

            particleTimer = 0;
        }

        // Player particle movement
        float moveSpeed = 0.025f;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            mat4 cameraAngle = camera.GetRotate();
            vec4 dir = cameraAngle * vec4(0, 0, moveSpeed, 1);
            player->applyForce(vec3(dir.x, 0, -dir.z));
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            mat4 cameraAngle = camera.GetRotate();
            vec4 dir = cameraAngle * vec4(0, 0, -moveSpeed, 1);
            player->applyForce(vec3(dir.x, 0, -dir.z));
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            mat4 cameraAngle = camera.GetRotate();
            vec4 dir = cameraAngle * vec4(-moveSpeed, 0, 0, 1);
            player->applyForce(vec3(dir.x, 0, -dir.z));
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            mat4 cameraAngle = camera.GetRotate();
            vec4 dir = cameraAngle * vec4(moveSpeed, 0, 0, 1);
            player->applyForce(vec3(dir.x, 0, -dir.z));
        }

        // Collide particles with each other
        for (int i = 0; i < particles.size(); i++) {
            Particle *p1 = particles[i];
            for (int j = i + 1; j < particles.size(); j++) {
                Particle *p2 = particles[j];
                vec3 delta = p2->getPosition() - p1->getPosition();
                if (length(delta) < p1->getRadius() + p2->getRadius()) {
                    float bounceStrength = 0.03f / sqrt(length(delta));
                    vec3 bounceForce = -delta * bounceStrength;
                    p1->applyForce(bounceForce);
                    p2->applyForce(bounceForce * -1.0f);
                }
            }
        }

        // Apply spring forces to particles
        for (int i = 0; i < springs.size(); i++) {
            springs[i]->applyForce();
        }

        // Apply gravity force to particles and move them
        for (int i = 0; i < particles.size(); i++) {
            vec3 gravityForce(0.0f, -0.01f, 0.0f);
            particles[i]->applyForce(gravityForce);
            if (i > 0)
                particles[i]->update(timeDelta);
        }

        // Clear screen
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        glUseProgram(shaderProgram);
        SetUniform(shaderProgram, "cameraView", camera.fullview);

        // Draw cube arena
        cubeModel.setXform(Translate(0.0f, -10.0f, 0.0f));
        cubeModel.draw(shaderProgram);

        // Draw particles
        for (int i = 0; i < particles.size(); i++) {
            sphereModel.setXform(Translate(particles[i]->getPosition()));
            sphereModel.draw(shaderProgram);
        }

        // Draw springs
        for (int i = 0; i < springs.size(); i++) {
            cylinderModel.setXform(springs[i]->getXform());
            cylinderModel.draw(shaderProgram);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}
