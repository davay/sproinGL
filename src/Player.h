#ifndef PLAYER_H
#define PLAYER_H

#include "Particle.h"
#include "Spring.h"

#include "Camera.h"
#include "VecMat.h"

#include <glad.h>
#include <GLFW/glfw3.h>

class Player {
public:
    Player(vec3 controllerPosition, PhysicsManager* pm) {
        this->controllerPosition = controllerPosition;
        tailPosition = controllerPosition - vec3(0, 0, 1);
        controllerVelocity = vec3(0, 0, 0);
        controllerDirection = vec3(0, 0, 0);

        feet = new Particle(controllerPosition, 1, 0.5);
        hips = new Particle(controllerPosition + vec3(0, 2, 0), 1, 0.8);

        pm->addParticle(feet);
        pm->addParticle(hips);
        pm->addSpring(new Spring(hips, feet, 2, 0.08, 0.08));
    }

    void keyboardInput(GLFWwindow *window, Camera *camera) {
        isMoving = false;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            mat4 cameraAngle = camera->GetRotate();
            vec4 d = cameraAngle * vec4(0, 0, 1, 1);
            vec3 a = normalize(vec3(d.x, 0, -d.z));
            controllerVelocity += a * 0.01;
            isMoving = true;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            mat4 cameraAngle = camera->GetRotate();
            vec4 d = cameraAngle * vec4(0, 0, -1, 1);
            vec3 a = normalize(vec3(d.x, 0, -d.z));
            controllerVelocity += a * 0.01;
            isMoving = true;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            mat4 cameraAngle = camera->GetRotate();
            vec4 d = cameraAngle * vec4(-1, 0, 0, 1);
            vec3 a = normalize(vec3(d.x, 0, -d.z));
            controllerVelocity += a * 0.01;
            isMoving = true;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            mat4 cameraAngle = camera->GetRotate();
            vec4 d = cameraAngle * vec4(1, 0, 0, 1);
            vec3 a = normalize(vec3(d.x, 0, -d.z));
            controllerVelocity += a * 0.01;
            isMoving = true;
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            if (isOnGround) {
                controllerVelocity.y = 0.3;
            }
        }
    }

    void update(double timeDelta, Camera* camera) {
        vec4 cameraDirection = camera->GetRotate() * vec4(0, 0, 1, 1);
        bodyDirection.x = cameraDirection.x;
        bodyDirection.y = cameraDirection.y;
        bodyDirection.z = cameraDirection.z;

        // Apply gravity
        controllerVelocity += vec3(0, -0.01, 0);

        // Decelerate on ground when not pressing a movement key
        if (isOnGround && !isMoving) {
            controllerVelocity.x += controllerVelocity.x * -0.1;
            controllerVelocity.z += controllerVelocity.z * -0.1;
        }

        // Limit horizontal movement speed
        if (length(vec3(controllerVelocity.x, 0, controllerVelocity.z)) > MAX_SPEED) {
            float yy = controllerVelocity.y;
            controllerVelocity = normalize(vec3(controllerVelocity.x, 0, controllerVelocity.z)) * MAX_SPEED;
            controllerVelocity.y = yy;
        }

        // Update position
        //vec3 lastControllerPosition = controllerPosition;
        controllerPosition += controllerVelocity;

        vec3 delta = controllerPosition - tailPosition;
        float r = 0.5 / length(delta);
        tailPosition = controllerPosition - delta * r;

        delta = normalize(controllerPosition - tailPosition);
        bodyDirection.x = delta.x;
        bodyDirection.y = 0;
        bodyDirection.z = delta.z;

        // Collide with ground
        isOnGround = false;
        if (controllerPosition.y < 0 + CONTROLLER_RADIUS) {
            controllerPosition.y = CONTROLLER_RADIUS;
            controllerVelocity.y = 0;
            isOnGround = true;
        }

        feet->setPosition(controllerPosition);
        hips->setPosition(vec3(feet->getPosition().x, hips->getPosition().y, feet->getPosition().z));
    }

    mat4 getXform() {
        //vec3 dir = vec3(0, 4, 1) - controllerPosition;
        vec3 dir = bodyDirection;

        vec3 up(0, 1, 0);
        vec3 z = normalize(dir);
        vec3 x = normalize(cross(up, z));
        vec3 y = normalize(cross(z, x));

        mat4 m = mat4(
            vec4(x, 0),
            vec4(y, 0),
            vec4(z, 0),
            vec4(0, 0, 0, 1)
        );

        mat4 t = Transpose(m);

        return Translate(controllerPosition) * t;
    }

    vec3 getControllerPosition() {
        return controllerPosition;
    }

private:
    const float CONTROLLER_RADIUS = 0.5f;
    const float MAX_SPEED = 0.15f;

    vec3 tailPosition;
    vec3 controllerPosition;
    vec3 controllerDirection;
    vec3 controllerVelocity;
    vec3 bodyDirection;
    bool isMoving, isOnGround;

    Particle* feet;
    Particle* hips;
};

#endif
