#ifndef PLAYER_H
#define PLAYER_H

#include "particle.h"
#include "spring.h"
#include "game_object.h"

#include "VecMat.h"
#include <typeinfo>
#include <glad.h>
#include <GLFW/glfw3.h>

#include "math.h"

class Player: protected GameObject {
public:
    Player(PhysicsManager* pm, vec3 controllerPosition) {
        objectId = PLAYER;
        this->controllerPosition = controllerPosition;
        controllerVelocity = vec3(0, 0, 0);
        controllerDirection = vec3(0, 0, 1);
        lookDirection = vec3(0, 0, 1);
        up = vec3(0, 1, 0);
        tailPosition = controllerPosition - vec3(0, 0, 1);
        leftFootTarget = controllerPosition + vec3(FOOT_STRADDLE_OFFSET, 0, 0);
        rightFootTarget = controllerPosition + vec3(-FOOT_STRADDLE_OFFSET, 0, 0);
        shouldMoveLeftFoot = true;
        stride = 0;

        // Define physics components
        base = new Particle(this, objectId, controllerPosition, 1, FOOT_RADIUS);
        torso = new Particle(this, objectId, controllerPosition + vec3(0, HEIGHT, 0), 1, 1);
        leftHand = new Particle(this, objectId, controllerPosition + vec3(ARM_LENGTH, HEIGHT, 0), 0.5, HAND_RADIUS);
        rightHand = new Particle(this, objectId, controllerPosition + vec3(-ARM_LENGTH, HEIGHT, 0), 0.5, HAND_RADIUS);
        leftFoot = new Particle(this, objectId, leftFootTarget, 0.1, FOOT_RADIUS);
        rightFoot = new Particle(this, objectId, rightFootTarget, 0.1, FOOT_RADIUS);

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

    void input(GLFWwindow *window) {
        isMoving = false;

        // Keyboard input for movement
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            vec3 forward = normalize(vec3(lookDirection.x, 0, lookDirection.z));
            controllerVelocity += forward * MOVE_FORCE;
            isMoving = true;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            vec3 backward = normalize(-vec3(lookDirection.x, 0, lookDirection.z));
            controllerVelocity += backward * MOVE_FORCE;
            isMoving = true;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            vec3 left = normalize(cross(up, lookDirection));
            controllerVelocity += left * MOVE_FORCE;
            isMoving = true;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            vec3 right = normalize(cross(lookDirection, up));
            controllerVelocity += right * MOVE_FORCE;
            isMoving = true;
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            // TODO: Fix bug where feet sometimes start stepping in the wrong place after landing a jump
            /*
            if (isOnGround) {
                controllerVelocity.y = 0.3;
                base->setVelocity(controllerVelocity);
                torso->setVelocity(controllerVelocity);
                isOnGround = false;
            }
            */
        }

        // Mouse input for look direction
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        float xOffset = mouseX - lastMouseX;
        float yOffset = lastMouseY  - mouseY;
        lastMouseX = mouseX;
        lastMouseY = mouseY;

        float mouseSensitivity = 0.005f;
        xOffset *= mouseSensitivity;
        yOffset *= mouseSensitivity;
        yaw   += xOffset;
        pitch += yOffset;

        if (pitch > -0.05) pitch = -0.05;
        if (pitch < -1.1) pitch = -1.1;

        vec3 direction = vec3(
            cos(yaw) * cos(pitch),
            sin(pitch),
            sin(yaw) * cos(pitch)
        );

        lookDirection = normalize(direction);
    }

    void update(double timeDelta, void*) override {
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

        // Determine body direction from invisible "tail"
        vec3 delta = controllerPosition - tailPosition;
        float r = 0.5 / length(delta);
        tailPosition = controllerPosition - delta * r;
        delta = normalize(controllerPosition - tailPosition);
        bodyDirection.x = delta.x;
        bodyDirection.y = 0;
        bodyDirection.z = delta.z;

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

        // Animate walk cycle while on the ground
        if (isOnGround) {
            vec3 horizontalVelocity = vec3(controllerVelocity.x, 0, controllerVelocity.z);
            stride += length(horizontalVelocity);

            // Determine the length of a stride based on the current horizontal velocity
            strideLength = length(horizontalVelocity) * 12.0;
            if (strideLength < 0.6) strideLength = 0.6;

            // After landing a jump, set new foot target positions
            if (!wasOnGround) {
                vec3 footStraddleOffset = normalize(cross(bodyDirection, up)) * FOOT_STRADDLE_OFFSET;
                vec3 leftOffset = -normalize(cross(bodyDirection, up)) * FOOT_STRADDLE_OFFSET;
                vec3 rightOffset = normalize(cross(bodyDirection, up)) * FOOT_STRADDLE_OFFSET;
                leftFootTarget = controllerPosition + leftOffset;
                rightFootTarget = controllerPosition + rightOffset;
            }

            // Start a new stride with the opposite foot
            if (stride >= strideLength) {
                vec3 footStraddleOffset = normalize(cross(horizontalVelocity, up)) * FOOT_STRADDLE_OFFSET;
                vec3 footTarget = controllerPosition + normalize(horizontalVelocity) * (STRIDE_LENGTH_MIN + length(controllerVelocity) * 15);

                if (shouldMoveLeftFoot) {
                    // Play popping noise
                    rightFootTarget = footTarget + footStraddleOffset;
                } else {
                    // Play popping noise
                    leftFootTarget = footTarget - footStraddleOffset;
                }

                stride = 0;
                shouldMoveLeftFoot = !shouldMoveLeftFoot;
            }

            // Move feet toward their respective target positions
            vec3 leftFootPosition = leftFoot->getPosition();
            vec3 rightFootPosition = rightFoot->getPosition();

            leftFoot->setPosition(leftFootPosition + (leftFootTarget - leftFootPosition) * STEP_SPEED);
            rightFoot->setPosition(rightFootPosition + (rightFootTarget - rightFootPosition) * STEP_SPEED);

            leftFoot->setForceExcemption(true);
            rightFoot->setForceExcemption(true);
        } else {
            // Feet are free-floating while in midair
            leftFoot->setForceExcemption(false);
            rightFoot->setForceExcemption(false);
        }

        // Force the hands down so they don't float up like a weirdo
        leftHand->applyForce(vec3(0, -0.005, 0));
        rightHand->applyForce(vec3(0, -0.005, 0));
    }

    void onCollision(void* other) override {
        Particle* particle = static_cast<Particle*>(other);
        std::cout << "player: " << objectId << std::endl;
        std::cout << "other: " << particle->getObjectId() << std::endl;
    }

    mat4 getXform() {
        vec3 z = normalize(lookDirection);
        vec3 x = normalize(cross(up, z));
        vec3 y = normalize(cross(z, x));

        mat4 m = mat4(
            vec4(x, 0),
            vec4(y, 0),
            vec4(z, 0),
            vec4(0, 0, 0, 1)
        );

        mat4 t = Transpose(m);

        return Translate(torso->getPosition()) * t * Scale(0.8, 0.8, 0.8);
    }

    vec3 getControllerPosition() { return controllerPosition; }
    vec3 getLookDirection() { return lookDirection; }

private:
    const float CONTROLLER_RADIUS = 0.5f;
    const float MAX_SPEED = 0.2f;
    const float MOVE_FORCE = 0.02f;
    const float MOVE_FRICTION = 0.08f;
    const float STRIDE_LENGTH = 2.5f;
    const float STRIDE_LENGTH_MIN = 0.1f;
    const float STEP_SPEED = 0.25;
    const float FOOT_STRADDLE_OFFSET = 0.6;
    const float FOOT_RADIUS = 0.3f;
    const float HAND_RADIUS = 0.3f;

    const float HEIGHT = 2.0f;
    const float ARM_LENGTH = 1.2f;
    const float LEG_LENGTH = 2.0f;

    vec3 controllerPosition;
    vec3 controllerDirection;
    vec3 controllerVelocity;
    vec3 up;
    float pitch, yaw;
    vec3 lookDirection;
    double lastMouseX, lastMouseY;
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
