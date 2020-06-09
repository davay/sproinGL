#ifndef GAME_H
#define GAME_H

#include "bullet.h"
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

#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <vector>

class Game {
public:
    Game(GLFWwindow *window, unsigned int screenWidth, int screenHeight)
        : gameCamera(vec3(0, 1, 10), (float) screenWidth / screenHeight)
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

        srand(time(NULL));

        player = new Player(&pm, vec3(0, 0, 0));
        gameObjects.push_back(player);
    }

    void update(double timeDelta) {

        timeToSpawnEnemy -= timeDelta;
        if (timeToSpawnEnemy <= 0) {
            vec3 spawnPosition = vec3(rand() % 40 - 20, 0, rand() % 40 - 20);
            while (length(spawnPosition - player->getControllerPosition()) < 5) {
                spawnPosition = vec3(rand() % 40 - 20, 0, rand() % 40 - 20);
            }

            if (rand() % 5 < 2) {
                gameObjects.push_back(new Centipede(&pm, spawnPosition));
            } else {
                gameObjects.push_back(new Emu(&pm, spawnPosition));
            }

            timeToSpawnEnemy = rand() % 5 + 5;
        }

        // Update physics
        pm.update(timeDelta);

        Bullet *bullet = player->input(window, &pm);
        if (bullet != nullptr) {
            gameObjects.push_back(bullet);
            pm.addParticle(bullet->getParticle());
        }

        // Update entities
        for (int i = 0; i < gameObjects.size(); i++) {
            gameObjects[i]->update(timeDelta, player);
        }

        gameCamera.update(timeDelta, player);
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
        cubeModel.setColor(vec3(0.9f, 0.2f, 0.3f));
        cubeModel.draw(sceneShader);

        // Draw particles
        std::vector<Particle*>* visibleParticles = pm.getVisibleParticles();
        for (int i = 0; i < visibleParticles->size(); i++) {
            Particle* particle = (*visibleParticles)[i];
            sphereModel.setXform(particle->getXform());

            if (particle->getOwner() != nullptr) {
                GameObject *owner = particle->getOwner();
                sphereModel.setColor(owner->getColor());
            } else {
                sphereModel.setColor(vec3(1.0f, 0.5f, 0.2f));
            }

            sphereModel.draw(sceneShader);

            vec3 particlePosition = particle->getPosition();

            if (particlePosition.x >= -25 && particlePosition.x <= 25) {
                if (particlePosition.z >= -25 && particlePosition.z <= 25) {
                    float radius = particle->getRadius() * 4;
                    mat4 xf = Translate(vec3(particlePosition.x, 0.001, particlePosition.z)) * Scale(vec3(radius, 0.001, radius)) * RotateX(90);
                    cylinderModel.setXform(xf);
                    cylinderModel.setColor(vec3(0.3f, 0.0f, 0.0f));
                    cylinderModel.draw(sceneShader);
                }
            }
        }

        // Draw springs
        std::vector<Spring*>* visibleSprings = pm.getVisibleSprings();
        for (int i = 0; i < visibleSprings->size(); i++) {
            Spring* spring = (*visibleSprings)[i];
            cylinderModel.setXform(spring->getXform());
            cylinderModel.setColor(vec3(1.0f, 1.0f, 1.0f));
            cylinderModel.draw(sceneShader);

            if (spring->isInArena()) {
                mat4 xf = spring->getShadowXform();
                cubeModel.setXform(xf);
                cubeModel.setColor(vec3(0.3f, 0.0f, 0.0f));
                cubeModel.draw(sceneShader);
            }
        }

        // // check if player died first
        // if (player->getHealth() <= 0 || player->getControllerPosition().y) {
        //     player->reset();
        // }

        monkeyModel.setXform(player->getXform());
        monkeyModel.setColor(player->getColor());
        monkeyModel.draw(sceneShader);
    
        vec3 playerPosition = player->getControllerPosition();

        // Draw base cylinder shadow
        if (playerPosition.x >= -25 && playerPosition.x <= 25) {
            if (playerPosition.z >= -25 && playerPosition.z <= 25) {
                mat4 xf = Translate(vec3(playerPosition.x, 0.001, playerPosition.z)) * Scale(vec3(3, 0.001, 3)) * RotateX(90);
                cylinderModel.setXform(xf);
                cylinderModel.setColor(vec3(0.3f, 0.0f, 0.0f));
                cylinderModel.draw(sceneShader);
            }
        }

        // Draw healthbar
        glUseProgram(hudShader);
        cubeModel.setXform(Translate(0.0f, 0.97f, 0.0f) * Scale(player->getHealth() * 0.05, 0.03, 0.2));
        cubeModel.setColor(vec3(0.3f, 0.7f, 0.0f));
        cubeModel.draw(hudShader);
    }

private:
    GLFWwindow *window;
    int sceneShader, hudShader;

    Model sphereModel, cubeModel, cylinderModel, monkeyModel;

    PhysicsManager pm;
    GameCamera gameCamera;

    std::vector<GameObject*> gameObjects;

    Player *player;

    float timeToSpawnEnemy = 5;
};

#endif
