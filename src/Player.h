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
        controllerDirection = vec3(0, 0, 1);
        tailPosition = controllerPosition - vec3(0, 0, 1);
        leftFootTarget = controllerPosition + vec3(FOOT_STRADDLE_OFFSET, 0, 0);
        rightFootTarget = controllerPosition + vec3(-FOOT_STRADDLE_OFFSET, 0, 0);
        shouldMoveLeftFoot = true;
        stride = 0;

        // Physics components
        base = new Particle(controllerPosition, 1, FOOT_RADIUS);
        torso = new Particle(controllerPosition + vec3(0, HEIGHT, 0), 1, 0.8);
        leftHand = new Particle(controllerPosition + vec3(ARM_LENGTH, HEIGHT, 0), 0.1, HAND_RADIUS);
        rightHand = new Particle(controllerPosition + vec3(-ARM_LENGTH, HEIGHT, 0), 0.1, HAND_RADIUS);
        leftFoot = new Particle(leftFootTarget, 0.1, FOOT_RADIUS);
        rightFoot = new Particle(rightFootTarget, 0.1, FOOT_RADIUS);

        pm->addParticle(base, false);
        pm->addParticle(torso, false);
        pm->addParticle(leftHand);
        pm->addParticle(rightHand);
        pm->addParticle(leftFoot);
        pm->addParticle(rightFoot);

        pm->addSpring(new Spring(torso, base, HEIGHT, 0.08, 0.08), false);
        pm->addSpring(new Spring(torso, leftHand, ARM_LENGTH, 0.08, 0.08));
        pm->addSpring(new Spring(torso, rightHand, ARM_LENGTH, 0.08, 0.08));
        pm->addSpring(new Spring(torso, leftFoot, LEG_LENGTH, 0.08, 0.08));
        pm->addSpring(new Spring(torso, rightFoot, LEG_LENGTH, 0.08, 0.08));
        pm->addSpring(new Spring(leftHand, rightHand, 5, 0.004, 0.001), false);
    }

    void keyboardInput(GLFWwindow *window, Camera *camera) {
        mat4 cameraAngle = camera->GetRotate();
        isMoving = false;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            vec4 d = cameraAngle * vec4(0, 0, 1, 1);
            vec3 a = normalize(vec3(d.x, 0, -d.z));
            controllerVelocity += a * MOVE_FORCE;
            isMoving = true;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            vec4 d = cameraAngle * vec4(0, 0, -1, 1);
            vec3 a = normalize(vec3(d.x, 0, -d.z));
            controllerVelocity += a * MOVE_FORCE;
            isMoving = true;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            vec4 d = cameraAngle * vec4(-1, 0, 0, 1);
            vec3 a = normalize(vec3(d.x, 0, -d.z));
            controllerVelocity += a * MOVE_FORCE;
            isMoving = true;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            vec4 d = cameraAngle * vec4(1, 0, 0, 1);
            vec3 a = normalize(vec3(d.x, 0, -d.z));
            controllerVelocity += a * MOVE_FORCE;
            isMoving = true;
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            if (isOnGround) {
                controllerVelocity.y = 0.3;
                base->setVelocity(controllerVelocity);
                torso->setVelocity(controllerVelocity);
                isOnGround = false;
            }
        }
    }

    void update(double timeDelta, Camera* camera) {
        /*
        vec4 cameraDirection = camera->GetRotate() * vec4(0, 0, 1, 1);
        bodyDirection.x = cameraDirection.x;
        bodyDirection.y = cameraDirection.y;
        bodyDirection.z = cameraDirection.z;
        */

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

        stride += length(vec3(controllerVelocity.x, 0, controllerVelocity.z));

        // Collide with ground
        bool wasOnGround = isOnGround;
        isOnGround = false;
        if (controllerPosition.y < 0 + base->getRadius()) {
            controllerPosition.y = base->getRadius();
            controllerVelocity.y = 0;
            isOnGround = true;
        }

        base->setPosition(controllerPosition);
        torso->setPosition(vec3(base->getPosition().x, torso->getPosition().y, base->getPosition().z));

        if (isOnGround) {
            // Determine the length of a stride based on the current horizontal velocity
            strideLength = length(controllerVelocity) * 13.0;
            if (strideLength < 0.7) strideLength = 0.7;

            if (!wasOnGround) {
                vec3 up(0, 1, 0);
                vec3 footStraddleOffset = normalize(cross(bodyDirection, up)) * FOOT_STRADDLE_OFFSET;
                vec3 leftOffset = -normalize(cross(bodyDirection, up)) * FOOT_STRADDLE_OFFSET;
                vec3 rightOffset = normalize(cross(bodyDirection, up)) * FOOT_STRADDLE_OFFSET;
                leftFootTarget = controllerPosition + leftOffset;
                rightFootTarget = controllerPosition + rightOffset;
            }

            // Start a new stride with the opposite foot
            if (stride >= strideLength) {
                vec3 up(0, 1, 0);
                vec3 footStraddleOffset = normalize(cross(bodyDirection, up)) * FOOT_STRADDLE_OFFSET;
                vec3 footTarget = controllerPosition + normalize(controllerVelocity) * (STRIDE_LENGTH_MIN + length(controllerVelocity) * 14);

                if (shouldMoveLeftFoot) {
                    // Play popping noise
                    rightFootTarget = footTarget + footStraddleOffset;
                } else {
                    // Play popping noise
                    leftFootTarget = footTarget - footStraddleOffset;
                }

                // Swap which foot to move next
                stride = 0;
                shouldMoveLeftFoot = !shouldMoveLeftFoot;
            }

            vec3 leftFootPosition = leftFoot->getPosition();
            vec3 rightFootPosition = rightFoot->getPosition();

            leftFoot->setPosition(leftFootPosition + (leftFootTarget - leftFootPosition) * STEP_SPEED);
            rightFoot->setPosition(rightFootPosition + (rightFootTarget - rightFootPosition) * STEP_SPEED);
            leftFoot->setForceExcemption(true);
            rightFoot->setForceExcemption(true);
        } else {
            leftFoot->setForceExcemption(false);
            rightFoot->setForceExcemption(false);
        }

        // Force the hands down so they don't float up like a weirdo
        leftHand->applyForce(vec3(0, -0.005, 0));
        rightHand->applyForce(vec3(0, -0.005, 0));
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

        return Translate(torso->getPosition()) * t;
    }

    vec3 getControllerPosition() {
        return controllerPosition;
    }

private:
    const float CONTROLLER_RADIUS = 0.5f;
    const float MAX_SPEED = 0.15f;
    const float MOVE_FORCE = 0.015f;
    const float MOVE_FRICTION = 0.08f;
    const float STRIDE_LENGTH = 2.5f;
    const float STRIDE_LENGTH_MIN = 0.1f;
    const float STEP_SPEED = 0.3;
    const float FOOT_STRADDLE_OFFSET = 0.5;
    const float FOOT_RADIUS = 0.3f;
    const float HAND_RADIUS = 0.3f;

    const float HEIGHT = 2.0f;
    const float ARM_LENGTH = 1.0f;
    const float LEG_LENGTH = 2.0f;

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
    Particle *torso;
    Particle *leftHand;
    Particle *rightHand;
    Particle *leftFoot;
    Particle *rightFoot;
};

#endif
