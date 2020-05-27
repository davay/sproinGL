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
#include "shadow.h"
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
        this->screenWidth = screenWidth;
        this->screenHeight = screenHeight;
        this->window = window;
        sphereModel.read("./assets/sphere.obj");
        cubeModel.read("./assets/cube.obj");
        cylinderModel.read("./assets/cylinder.obj");
        monkeyModel.read("./assets/monkey.obj");

        sceneShader = LinkProgramViaFile("./src/shaders/scene_vshader.txt", "./src/shaders/scene_fshader.txt");
        hudShader = LinkProgramViaFile("./src/shaders/hud_vshader", "./src/shaders/hud_fshader.txt");\
        shadowShader = LinkProgramViaFile("./src/shaders/scene_vshader.txt", "./src/shaders/shadow_fshader.txt");

    }

    void update(double timeDelta) {
        // Update physics
        pm.update(timeDelta);

        // Update entities
        player.input(window);
        player.update(timeDelta, nullptr);
        centipede.update(timeDelta, &player);
        emu.update(timeDelta, &player);

        gameCamera.update(timeDelta, &player);
    }

    void draw() {
        // Clear screen
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        generateShadowFBO();
        glEnable(GL_DEPTH_TEST);

        glUseProgram(shadowShader);
        vec3 lp(2, 5.f, 2);
        vec3 ld(-1.f, -1.f, 0.f);
        float penumbraAngle = 50.f;
        // Light space matrices
        // From light space to shadow map screen space
        mat4 projection = Perspective((DegreesToRadians * penumbraAngle*2.f), 1.f, 1.f, 100.f);
        // From world to light
        mat4 worldToLight = LookAt(lp, lp+ld, vec3(0.f, 0.f, -1.f));
        // From object to light (MV for light)
        // @TODO WTF
        mat4 objectToWorld;
        mat4 objectToLight = worldToLight * objectToWorld;
        // From object to shadow map screen space (MVP for light)
        mat4 objectToLightScreen = projection * objectToLight;
        // From world to shadow map screen space 
        mat4 worldToLightScreen = projection * worldToLight;

        // Bind the shadow FBO
        glBindFramebuffer(GL_FRAMEBUFFER, shadowFbo);

        // Set the viewport corresponding to shadow texture resolution
        glViewport(0, 0, 512, 512);

        // Clear only the depth buffer
        glClear(GL_DEPTH_BUFFER_BIT);

        // @TODO MVP
        SetUniform(shadowShader, "modelTrans", &objectToLightScreen[0]);
        // @TODO MV
        SetUniform(shadowShader, "cameraView", &objectToLight[0]);

        // Fallback to default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Revert to window size viewport
        glViewport(0, 0, screenWidth, screenHeight);

        glUseProgram(sceneShader);
        SetUniform(sceneShader, "cameraView", gameCamera.getView());

        // Draw arena
        cubeModel.setXform(Translate(0.0f, -1.0f, 0.0f) * Scale(25, 1, 25));
        cubeModel.draw(sceneShader);

        // Draw particles
        std::vector<Particle*>* visibleParticles = pm.getVisibleParticles();
        for (int i = 0; i < visibleParticles->size(); i++) {
            Particle* particle = (*visibleParticles)[i];
            sphereModel.setXform(particle->getXform());
            sphereModel.draw(sceneShader);
        }

        // Draw springs
        std::vector<Spring*>* visibleSprings = pm.getVisibleSprings();
        for (int i = 0; i < visibleSprings->size(); i++) {
            Spring* spring = (*visibleSprings)[i];
            cylinderModel.setXform(spring->getXform());
            cylinderModel.draw(sceneShader);
        }

        monkeyModel.setXform(player.getXform());
        monkeyModel.draw(sceneShader);

        //glDisable(GL_DEPTH_TEST);

        // Draw healthbar
        //glUseProgram(sceneShader);
        cubeModel.setXform(Translate(0.0f, 0.0f, -10.0f));
        cubeModel.draw(sceneShader);
        
    }

    Player* getPlayer() {
        return &player;
    }

private:

    GLFWwindow *window;

    Model sphereModel, cubeModel, cylinderModel, monkeyModel;

    PhysicsManager pm;
    GameCamera gameCamera;
    Player player;
    Centipede centipede; //TODO: Centipede seems to be the cause of the mouse lock
    Emu emu;
    int sceneShader, hudShader, shadowShader;
    int screenWidth, screenHeight;
};

#endif
