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
        //Emu *emu = new Emu(&pm, vec3(10, 0, 8));
        Centipede *centipede = new Centipede(&pm, vec3(-10, 0, 8));

        gameObjects.push_back(player);
        //gameObjects.push_back(emu);
        gameObjects.push_back(centipede);
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
        cubeModel.setColor(vec3(1.0f, 0.3f, 0.4f));
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
        }

        // Draw springs
        std::vector<Spring*>* visibleSprings = pm.getVisibleSprings();
        for (int i = 0; i < visibleSprings->size(); i++) {
            Spring* spring = (*visibleSprings)[i];
            cylinderModel.setXform(spring->getXform());
            cylinderModel.setColor(vec3(1.0f, 1.0f, 1.0f));
            cylinderModel.draw(sceneShader);
        }

        monkeyModel.setXform(player->getXform());
        monkeyModel.setColor(player->getColor());
        monkeyModel.draw(sceneShader);

        // Draw healthbar
        glUseProgram(hudShader);
        cubeModel.setXform(Translate(0.0f, 0.98f, 0.0f) * Scale(player->getHealth() * 0.01, 0.02, 0.2));
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
