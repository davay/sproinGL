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
        leftFootTarget = controllerPosition + vec3(0.5, 0, 0);
        rightFootTarget = controllerPosition + vec3(-0.5, 0, 0);

        base = new Particle(controllerPosition, 1, 0.5);
        hips = new Particle(controllerPosition + vec3(0, 2, 0), 1, 0.8);
        leftFoot = new Particle(leftFootTarget, 1, 0.3);
        rightFoot = new Particle(rightFootTarget, 1, 0.3);
        //hips = new Particle(controllerPosition + vec3(0, 2, 0), 1, 0.8);
        leftHand = new Particle(controllerPosition + vec3(1.2, 2, 0), 1, 0.3);
        rightHand = new Particle(controllerPosition + vec3(-1.2, 2, 0), 1, 0.3);
        shouldMoveLeftFoot = true;

        stride = 0;

        pm->addParticle(base, false);
        pm->addParticle(hips);
        pm->addParticle(leftFoot);
        pm->addParticle(rightFoot);
        pm->addParticle(leftHand);
        pm->addParticle(rightHand);

        pm->addSpring(new Spring(hips, base, 2, 0.08, 0.08), false);
        pm->addSpring(new Spring(hips, leftFoot, 2, 0.08, 0.08));
        pm->addSpring(new Spring(hips, rightFoot, 2, 0.08, 0.08));
        pm->addSpring(new Spring(hips, leftHand, 1.2, 0.08, 0.08));
        pm->addSpring(new Spring(hips, rightHand, 1.2, 0.08, 0.08));
        pm->addSpring(new Spring(leftHand, rightHand, 3, 0.02, 0.001), false);
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

        stride += length(vec3(controllerVelocity.x, 0, controllerVelocity.z));

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


        if (isOnGround) {
            strideLength = length(controllerVelocity) * 13.0;
            if (strideLength < 0.7) strideLength = 0.7;
            if (stride >= strideLength) {
                vec3 up(0, 1, 0);
                if (shouldMoveLeftFoot) {
                    vec3 rightOffset = normalize(cross(bodyDirection, up)) * 0.5;
                    rightFootTarget = controllerPosition + rightOffset + bodyDirection * (STRIDE_LENGTH_MIN + length(controllerVelocity) * 14);
                } else {
                    vec3 leftOffset = -normalize(cross(bodyDirection, up)) * 0.5;
                    leftFootTarget = controllerPosition + leftOffset + bodyDirection * (STRIDE_LENGTH_MIN + length(controllerVelocity) * 14);
                }

                stride = 0;
                shouldMoveLeftFoot = !shouldMoveLeftFoot;
            }

            vec3 leftFootPosition = leftFoot->getPosition();
            vec3 rightFootPosition = rightFoot->getPosition();

            leftFoot->setPosition(leftFootPosition + (leftFootTarget - leftFootPosition) * 0.3);
            rightFoot->setPosition(rightFootPosition + (rightFootTarget - rightFootPosition) * 0.3);
            leftFoot->setForceExcemption(true);
            rightFoot->setForceExcemption(true);
        } else {
            leftFoot->setForceExcemption(false);
            rightFoot->setForceExcemption(false);
        }
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
    const float MAX_SPEED = 0.18f;
    const float MOVE_FORCE = 0.015f;
    const float MOVE_FRICTION = 0.08f;
    const float STRIDE_LENGTH = 2.5f;
    const float STRIDE_LENGTH_MIN = 0.1f;

    vec3 controllerPosition;
    vec3 controllerDirection;
    vec3 controllerVelocity;
    bool isMoving, isOnGround;

    vec3 tailPosition;
    vec3 bodyDirection;
    vec3 leftFootTarget;
    vec3 rightFootTarget;
    float stride, strideLength;
    bool shouldMoveLeftFoot;

    Particle *base;
    Particle *hips;
    Particle *leftFoot;
    Particle *rightFoot;
    Particle *leftHand;
    Particle *rightHand;
};

#endif
