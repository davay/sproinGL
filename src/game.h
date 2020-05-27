#ifndef GAME_H
#define GAME_H

#include "centipede.h"
#include "emu.h"
#include "game_camera.h"
#include "model.h"
#include "particle.h"
#include "physics_manager.h"
#include "player.h"
#include "spring.h"

#include "GLXtras.h"
#include "Mesh.h"
#include "Misc.h"
#include "VecMat.h"

#include <glad.h>
#include <GLFW/glfw3.h>

class Game {
public:
    Game(GLFWwindow *window, unsigned int screenWidth, int screenHeight)
        : gameCamera(vec3(0, 1, 10), (float) screenWidth / screenHeight)
        , player(&pm, vec3(0, 0, 0))
        , centipede(&pm, vec3(5, 1, 0))
        , emu(&pm, vec3(-4, 2, 4))
        , sphereModel(vec3(1.0f, 0.5f, 0.2f))
        , cubeModel(vec3(1.0f, 0.3f, 0.4f))
        , cylinderModel(vec3(1.0f, 1.0f, 1.0f))
        , monkeyModel(vec3(0.3f, 0.7f, 0.0f))
    {
        this->window = window;
        sphereModel.read("./assets/sphere.obj");
        cubeModel.read("./assets/cube.obj");
        cylinderModel.read("./assets/cylinder.obj");
        monkeyModel.read("./assets/monkey.obj");

        sceneShader = LinkProgramViaFile("./src/shaders/scene_vshader.txt", "./src/shaders/scene_fshader.txt");
        hudShader = LinkProgramViaFile("./src/shaders/hud_vshader.txt", "./src/shaders/hud_fshader.txt");
    }

    void update(double timeDelta) {
        // Update physics
        pm.update(timeDelta);

        // Update entities
        player.input(window, &pm);
        player.update(timeDelta, nullptr);
        centipede.update(timeDelta, &player);
        emu.update(timeDelta, &player);

        gameCamera.update(timeDelta, &player);
    }

    void draw() {
        // Clear screen
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);

        glUseProgram(sceneShader);
        SetUniform(sceneShader, "cameraView", gameCamera.getView());

        // Draw arena
        cubeModel.setXform(Translate(0.0f, -1.0f, 0.0f) * Scale(25, 1, 25));
        cubeModel.setColor(vec3(1.0f, 0.3f, 0.4f));
        cubeModel.drawInScene(sceneShader);

        // Draw particles
        std::vector<Particle*>* visibleParticles = pm.getVisibleParticles();
        for (int i = 0; i < visibleParticles->size(); i++) {
            Particle* particle = (*visibleParticles)[i];
            sphereModel.setXform(particle->getXform());
            sphereModel.setColor(vec3(1.0f, 0.5f, 0.2f));
            sphereModel.drawInScene(sceneShader);
        }

        // Draw springs
        std::vector<Spring*>* visibleSprings = pm.getVisibleSprings();
        for (int i = 0; i < visibleSprings->size(); i++) {
            Spring* spring = (*visibleSprings)[i];
            cylinderModel.setXform(spring->getXform());
            cylinderModel.setColor(vec3(1.0f, 1.0f, 1.0f));
            cylinderModel.drawInScene(sceneShader);
        }

        monkeyModel.setXform(player.getXform());
        monkeyModel.setColor(vec3(0.3f, 0.7f, 0.0f));
        monkeyModel.drawInScene(sceneShader);

        // Draw healthbar
        glUseProgram(hudShader);
        cubeModel.setXform(Translate(0.0f, 0.98f, 0.0f) * Scale(player.getHealth() * 0.01, 0.02, 0.2));
        cubeModel.setColor(vec3(0.3f, 0.7f, 0.0f));
        cubeModel.drawOnHUD(hudShader);
    }

    Player* getPlayer() {
        return &player;
    }

private:
    GLFWwindow *window;
    int sceneShader, hudShader;

    Model sphereModel, cubeModel, cylinderModel, monkeyModel;

    PhysicsManager pm;
    GameCamera gameCamera;
    Player player;
    Centipede centipede; //TODO: Centipede seems to be the cause of the mouse lock
    Emu emu;
};

#endif
