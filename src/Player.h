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
        controllerVelocity = vec3(0, 0, 0);
        controllerDirection = vec3(0, 0, 0);

        tailPosition = controllerPosition - vec3(0, 0, 1);
        leftFootTarget = controllerPosition + vec3(1, 0, 0);
        rightFootTarget = controllerPosition + vec3(-1, 0, 0);

        base = new Particle(controllerPosition, 1, 0.5);
        hips = new Particle(controllerPosition + vec3(0, 2, 0), 1, 0.8);
        leftFoot = new Particle(leftFootTarget, 1, 0.4);
        rightFoot = new Particle(rightFootTarget, 1, 0.4);
        shouldMoveLeftFoot = true;

        pm->addParticle(base, false);
        pm->addParticle(hips);
        pm->addParticle(leftFoot);
        //pm->addParticle(rightFoot);

        pm->addSpring(new Spring(hips, base, 2, 0.08, 0.08));
        pm->addSpring(new Spring(hips, leftFoot, 2, 0.08, 0.08));
        //pm->addSpring(new Spring(hips, rightFoot, 2, 0.08, 0.08));
    }

    void keyboardInput(GLFWwindow *window, Camera *camera) {
        isMoving = false;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            mat4 cameraAngle = camera->GetRotate();
            vec4 d = cameraAngle * vec4(0, 0, 1, 1);
            vec3 a = normalize(vec3(d.x, 0, -d.z));
            controllerVelocity += a * MOVE_FORCE;
            isMoving = true;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            mat4 cameraAngle = camera->GetRotate();
            vec4 d = cameraAngle * vec4(0, 0, -1, 1);
            vec3 a = normalize(vec3(d.x, 0, -d.z));
            controllerVelocity += a * MOVE_FORCE;
            isMoving = true;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            mat4 cameraAngle = camera->GetRotate();
            vec4 d = cameraAngle * vec4(-1, 0, 0, 1);
            vec3 a = normalize(vec3(d.x, 0, -d.z));
            controllerVelocity += a * MOVE_FORCE;
            isMoving = true;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            mat4 cameraAngle = camera->GetRotate();
            vec4 d = cameraAngle * vec4(1, 0, 0, 1);
            vec3 a = normalize(vec3(d.x, 0, -d.z));
            controllerVelocity += a * MOVE_FORCE;
            isMoving = true;
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            if (isOnGround) {
                controllerVelocity.y = 0.3;
                isOnGround = false;
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
            controllerVelocity.x += controllerVelocity.x * -MOVE_FRICTION;
            controllerVelocity.z += controllerVelocity.z * -MOVE_FRICTION;
        }

        // Limit horizontal movement speed
        if (length(vec3(controllerVelocity.x, 0, controllerVelocity.z)) > MAX_SPEED) {
            float yy = controllerVelocity.y;
            controllerVelocity = normalize(vec3(controllerVelocity.x, 0, controllerVelocity.z)) * MAX_SPEED;
            controllerVelocity.y = yy;
        }

        // Update position
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

        base->setPosition(controllerPosition);
        hips->setPosition(vec3(base->getPosition().x, hips->getPosition().y, base->getPosition().z));

        //float strideLength = length(controllerVelocity) * 20 + 0.5;
        float strideLength = 1.5;
        if (length(leftFootTarget - controllerPosition) > strideLength + 0.5) {
            leftFootTarget = controllerPosition + bodyDirection * strideLength;
        }

        if (isOnGround) {
            leftFoot->setPosition(leftFootTarget);
        }

        //printf("%d\n", shouldMoveLeftFoot);
        /*
        if (shouldMoveLeftFoot) {
            leftFoot->setPosition(leftFoot->getPosition() + (leftFootTarget - leftFoot->getPosition()) * 0.001);
            rightFoot->setPosition(rightFootTarget);
            if (length(rightFootTarget - hips->getPosition()) > 4) {
                rightFootTarget = controllerPosition + bodyDirection * 3;
                shouldMoveLeftFoot = false;
            }
        } else {
            rightFoot->setPosition(rightFoot->getPosition() + (rightFootTarget - rightFoot->getPosition()) * 0.001);
            leftFoot->setPosition(leftFootTarget);
            if (length(leftFootTarget - hips->getPosition()) > 4) {
                leftFootTarget = controllerPosition + bodyDirection * 3;
                shouldMoveLeftFoot = true;
            }
        }
        */
    }

    mat4 getXform() {
        vec3 up(0, 1, 0);
        vec3 z = normalize(bodyDirection);
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
    const float MOVE_FORCE = 0.02f;
    const float MOVE_FRICTION = 0.1f;

    vec3 controllerPosition;
    vec3 controllerDirection;
    vec3 controllerVelocity;
    bool isMoving, isOnGround;

    vec3 tailPosition;
    vec3 bodyDirection;
    vec3 leftFootTarget;
    vec3 rightFootTarget;
    bool shouldMoveLeftFoot;

    Particle *base;
    Particle *hips;
    Particle *leftFoot;
    Particle *rightFoot;
};

#endif
