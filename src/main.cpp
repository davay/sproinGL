#include "Model.h"
#include "Particle.h"
#include "Spring.h"
#include "PhysicsManager.h"
#include "Player.h"

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

void resize(GLFWwindow *window, int width, int height) {
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
    GLFWwindow *window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "SproinGL", NULL, NULL);

    glfwMakeContextCurrent(window);
    //glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // Build and compile shader program
    int shaderProgram = LinkProgramViaFile("./src/vertex_shader.txt", "./src/fragment_shader.txt");

    // callbacks
    glfwSetCursorPosCallback(window, MouseMove);
    glfwSetMouseButtonCallback(window, MouseButton);
    glfwSetWindowSizeCallback(window, resize);

    // Load models
    Model sphereModel(vec3(1.0f, 0.5f, 0.2f));
    Model cubeModel(vec3(1.0f, 0.3f, 0.4f));
    Model cylinderModel(vec3(1.0f, 1.0f, 1.0f));
    Model monkeyModel(vec3(0.3f, 0.7f, 0.0f));
    sphereModel.read("./assets/sphere.obj", "‎⁨.⁩/assets/lily.tga⁩");
    cubeModel.read("./assets/cube.obj", "‎⁨.⁩/assets/lily.tga⁩");
    cylinderModel.read("./assets/cylinder.obj", "‎⁨.⁩/assets/lily.tga⁩");
    monkeyModel.read("./assets/monkey.obj", "‎⁨.⁩/assets/lily.tga⁩");

    // Initialize game objects
    PhysicsManager pm;
    std::vector<Particle*> particles;
    std::vector<Spring*> springs;

    Player player(vec3(0, 3, 0));

    for (int i = 0; i < 10; i++) {
        vec3 position(i * 2.2, 9.0f, -3);
        vec3 velocity(0.0f, 0.0f, 0.0f);
        particles.push_back(new Particle(position, velocity, 1.0f, 0.1f, 0.9f));
    }

    for (int i = 0; i < 9; i++) {
        springs.push_back(new Spring(particles[i], particles[i + 1], 2.2, 0.1, 0.01));
    }

    srand(time(NULL));
    double lastTime = glfwGetTime();
    double particleTimer = 0;

    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        double timeDelta = currentTime - lastTime;
        lastTime = currentTime;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);

        player.keyboardInput(window, &camera);
        player.update(timeDelta);

        // Collide particles with each other
        for (int i = 0; i < particles.size(); i++) {
            Particle *p1 = particles[i];
            for (int j = i + 1; j < particles.size(); j++) {
                Particle *p2 = particles[j];
                vec3 delta = p2->getPosition() - p1->getPosition();
                if (length(delta) < p1->getRadius() + p2->getRadius()) {
                    float bounceStrength = 0.05f / sqrt(length(delta));
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
            if (i != 0 && i != 5)
                particles[i]->update(timeDelta);
        }

        // Clear screen
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        // Set shader
        glUseProgram(shaderProgram);
        SetUniform(shaderProgram, "cameraView", camera.fullview);

        // Draw arena
        cubeModel.setXform(Translate(0.0f, -10.0f, 0.0f));
        cubeModel.draw(shaderProgram);

        // Draw particles
        for (int i = 0; i < particles.size(); i++) {
            sphereModel.setXform(particles[i]->getXform());
            sphereModel.draw(shaderProgram);
        }

        // Draw springs
        for (int i = 0; i < springs.size(); i++) {
            cylinderModel.setXform(springs[i]->getXform());
            cylinderModel.draw(shaderProgram);
        }

        monkeyModel.setXform(player.getXform());
        monkeyModel.draw(shaderProgram);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}
