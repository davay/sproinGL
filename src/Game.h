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

class Game {
public:
    Game(GLFWwindow *window, unsigned int screenWidth, int screenHeight)
        : camera(screenWidth, screenHeight, vec3(20.0f, -5.0f, 0.0f), vec3(0.0f, -2.0f, -40.0f), 30)
        , player(vec3(0, 0, 0), &pm)
        , sphereModel(vec3(1.0f, 0.5f, 0.2f))
        , cubeModel(vec3(1.0f, 0.3f, 0.4f))
        , cylinderModel(vec3(1.0f, 1.0f, 1.0f))
        , monkeyModel(vec3(0.3f, 0.7f, 0.0f))
    {
        this->window = window;
        sphereModel.read("./assets/sphere.obj", "‎⁨.⁩/assets/lily.tga⁩");
        cubeModel.read("./assets/cube.obj", "‎⁨.⁩/assets/lily.tga⁩");
        cylinderModel.read("./assets/cylinder.obj", "‎⁨.⁩/assets/lily.tga⁩");
        monkeyModel.read("./assets/monkey.obj", "‎⁨.⁩/assets/lily.tga⁩");
    }

    void update(double timeDelta) {
        // Update physics
        pm.update(timeDelta);

        // Update player
        player.keyboardInput(window, &camera);
        player.update(timeDelta, &camera);
    }

    void draw(int shaderProgram) {
        glUseProgram(shaderProgram);
        SetUniform(shaderProgram, "cameraView", camera.fullview);

        // Draw arena
        cubeModel.setXform(Translate(0.0f, -10.0f, 0.0f));
        cubeModel.draw(shaderProgram);

        // Draw particles
        std::vector<Particle*>* visibleParticles = pm.getVisibleParticles();
        for (int i = 0; i < visibleParticles->size(); i++) {
            Particle* particle = (*visibleParticles)[i];
            sphereModel.setXform(particle->getXform());
            sphereModel.draw(shaderProgram);
        }

        // Draw springs
        std::vector<Spring*>* visibleSprings = pm.getVisibleSprings();
        for (int i = 0; i < visibleSprings->size(); i++) {
            Spring* spring = (*visibleSprings)[i];
            cylinderModel.setXform(spring->getXform());
            cylinderModel.draw(shaderProgram);
        }

        monkeyModel.setXform(player.getXform());
        monkeyModel.draw(shaderProgram);
    }

private:
    GLFWwindow *window;
    PhysicsManager pm;
    Camera camera;
    Player player;

    Model sphereModel, cubeModel, cylinderModel, monkeyModel;
};
