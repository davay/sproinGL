#include "Model.h"
#include "PhysicsManager.h"
#include "Player.h"

#include "Camera.h"
#include "VecMat.h"

class Game {
public:
    Game() {
        /*
        sphereModel.read("./assets/sphere.obj", "‎⁨.⁩/assets/lily.tga⁩");
        cubeModel.read("./assets/cube.obj", "‎⁨.⁩/assets/lily.tga⁩");
        cylinderModel.read("./assets/cylinder.obj", "‎⁨.⁩/assets/lily.tga⁩");
        monkeyModel.read("./assets/monkey.obj", "‎⁨.⁩/assets/lily.tga⁩");

        for (int i = 0; i < 10; i++) {
            vec3 position(i * 2.2, 9.0f, -3);
            vec3 velocity(0.0f, 0.0f, 0.0f);
            pm.addParticle(new Particle(position, velocity, 1.0f, 0.5f, 0.9f));
        }

        for (int i = 0; i < 9; i++) {
            //springs.push_back(new Spring(particles[i], particles[i + 1], 2.2, 0.1, 0.01));
        }
        */
    }

    void update(float timeDelta) {
        //pm.update(timeDelta);
    }

    void draw() {
        /*
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
        */
    }

private:
    /*
    Model cube(vec3(1.0f, 0.3f, 0.4f));
    Model sphere(vec3(1.0f, 0.5f, 0.2f));
    Model cyclinder(vec3(1.0f, 1.0f, 1.0f));
    Model monkey(vec3(0.3f, 0.7f, 0.0f));

    PhysicsManager pm;

    Camera camera(SCREEN_WIDTH, SCREEN_HEIGHT, vec3(20.0f, -5.0f, 0.0f), vec3(0.0f, -2.0f, -40.0f), 30);
    */
};
