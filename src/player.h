#ifndef PLAYER_H
#define PLAYER_H

#include "bullet.h"
#include "game_object.h"
#include "particle.h"
#include "physics_manager.h"
#include "spring.h"

#include "VecMat.h"

#include <typeinfo>
#include <glad.h>
#include <GLFW/glfw3.h>

#include "math.h"
#include <vector>

class Player: public GameObject {
public:
    Player(PhysicsManager *pm, vec3 controllerPosition) {
        objectId = PLAYER;
        color = vec3(0.3f, 0.7f, 0.0f);

        health = MAX_HEALTH;
        collisionCooldown = MAX_COLLISION_COOLDOWN;
        isCoolingDown = false;
        cooldownFlashTimer = 0;
        isCooldownFlash = false;

        // Set up controller
        this->controllerPosition = controllerPosition;
        controllerVelocity = vec3(0, 0, 0);
        lookDirection = vec3(0, 0, 1);
        up = vec3(0, 1, 0);
        tailPosition = controllerPosition - vec3(0, 0, 1);
        leftFootTarget = controllerPosition + vec3(FOOT_STRADDLE_OFFSET, 0, 0);
        rightFootTarget = controllerPosition + vec3(-FOOT_STRADDLE_OFFSET, 0, 0);
        shouldMoveLeftFoot = true;
        stride = 0;

        // Set up physics components
        base = new Particle(this, objectId, controllerPosition, 1, FOOT_RADIUS);
        torso = new Particle(this, objectId, controllerPosition + vec3(0, HEIGHT, 0), 1, 0.5);
        leftHand = new Particle(this, objectId, controllerPosition + vec3(ARM_LENGTH, HEIGHT, 0), 1.5, HAND_RADIUS);
        rightHand = new Particle(this, objectId, controllerPosition + vec3(-ARM_LENGTH, HEIGHT, 0), 1.5, HAND_RADIUS);
        leftFoot = new Particle(this, objectId, leftFootTarget, 0.1, FOOT_RADIUS);
        rightFoot = new Particle(this, objectId, rightFootTarget, 0.1, FOOT_RADIUS);

        pm->addParticle(base, false);
        pm->addParticle(torso, false);
        pm->addParticle(leftHand);
        pm->addParticle(rightHand);
        pm->addParticle(leftFoot);
        pm->addParticle(rightFoot);

        pm->addSpring(new Spring(torso, base, HEIGHT, 0.08, 0.08), false);
        pm->addSpring(new Spring(torso, leftHand, ARM_LENGTH, 0.04, 0.05));
        pm->addSpring(new Spring(torso, rightHand, ARM_LENGTH, 0.04, 0.05));
        pm->addSpring(new Spring(torso, leftFoot, LEG_LENGTH, 0.04, 0.1));
        pm->addSpring(new Spring(torso, rightFoot, LEG_LENGTH, 0.04, 0.1));
        pm->addSpring(new Spring(leftHand, rightHand, 2, 0.01, 0.1), false);
    }

    Bullet* input(GLFWwindow *window, PhysicsManager *pm) {
        Bullet *bullet = nullptr;

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
            if (isOnGround) {
                controllerVelocity.y = 0.3;
                base->setVelocity(controllerVelocity);
                torso->setVelocity(controllerVelocity);
                isOnGround = false;
            }
        }

        int mouseButtonState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
        if (mouseButtonState == GLFW_PRESS && !isMousePressed) {
            isMousePressed = true;
            vec3 bulletPosition = controllerPosition + vec3(0, 2, 0) + bodyDirection;
            vec3 bulletVelocity = bodyDirection * 0.4 + vec3(0, 0.02, 0);
            bullet = new Bullet(bulletPosition, bulletVelocity);
        }

        if (mouseButtonState == GLFW_RELEASE) {
            isMousePressed = false;
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

        if (pitch > 0) pitch = 0;
        if (pitch < -1.3) pitch = -1.3;

        vec3 direction = vec3(
            cos(yaw) * cos(pitch),
            sin(pitch),
            sin(yaw) * cos(pitch)
        );

        lookDirection = normalize(direction);

        return bullet;
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

        if (controllerPosition.x > -25 && controllerPosition.x < 25) {
            if (controllerPosition.z > -25 && controllerPosition.z < 25) {
                if (controllerPosition.y + FOOT_RADIUS > -2 && controllerPosition.y - FOOT_RADIUS < 0.0f) {
                    controllerPosition.y = FOOT_RADIUS;
                    controllerVelocity.y = 0;
                    isOnGround = true;
                }
            }
        }

        base->setPosition(controllerPosition);
        torso->setPosition(vec3(base->getPosition().x, torso->getPosition().y, base->getPosition().z));

        // Animate walk cycle while on the ground
        if (isOnGround) {
            const vec3 horizontalVelocity = vec3(controllerVelocity.x, 0, controllerVelocity.z);
            stride += length(horizontalVelocity);

            // Determine the length of a stride based on the current horizontal velocity
            float strideLength = length(horizontalVelocity) * 12.0;
            if (strideLength < 0.6) strideLength = 0.6;

            // After landing a jump, set new foot target positions
            if (!wasOnGround) {
                const vec3 footStraddleOffset = normalize(cross(bodyDirection, up)) * FOOT_STRADDLE_OFFSET;
                const vec3 leftOffset = -normalize(cross(bodyDirection, up)) * FOOT_STRADDLE_OFFSET;
                const vec3 rightOffset = normalize(cross(bodyDirection, up)) * FOOT_STRADDLE_OFFSET;
                leftFootTarget = controllerPosition + leftOffset;
                rightFootTarget = controllerPosition + rightOffset;
            }

            // Start a new stride with the opposite foot
            if (stride >= strideLength) {
                const vec3 footStraddleOffset = normalize(cross(horizontalVelocity, up)) * FOOT_STRADDLE_OFFSET;
                const vec3 footTarget = controllerPosition + normalize(horizontalVelocity) * (STRIDE_LENGTH_MIN + length(controllerVelocity) * 13);

                if (shouldMoveLeftFoot) {
                    rightFootTarget = footTarget + footStraddleOffset;
                } else {
                    leftFootTarget = footTarget - footStraddleOffset;
                }

                stride = 0;
                shouldMoveLeftFoot = !shouldMoveLeftFoot;
            }

            // Move feet toward their respective target positions
            const vec3 leftFootPosition = leftFoot->getPosition();
            const vec3 rightFootPosition = rightFoot->getPosition();

            leftFoot->applyForce((leftFootTarget - leftFootPosition) * 0.04);
            rightFoot->applyForce((rightFootTarget - rightFootPosition) * 0.04);

            leftFoot->setPosition(vec3(leftFootPosition.x, leftFootTarget.y, leftFootPosition.z));
            rightFoot->setPosition(vec3(rightFootPosition.x, rightFootTarget.y, rightFootPosition.z));
        } else {
            // Feet are free-floating while in midair
            leftFoot->setForceExcemption(false);
            rightFoot->setForceExcemption(false);
        }

        if (isCoolingDown) {
            color = isCooldownFlash ? vec3(1.0, 0, 0) : vec3(0.3f, 0.7f, 0.0f);
            cooldownFlashTimer += timeDelta;
            if (cooldownFlashTimer >= MAX_COOLDOWN_FLASH_TIME) {
                isCooldownFlash = !isCooldownFlash;
                cooldownFlashTimer = 0;
            }
            collisionCooldown -= timeDelta;
            if (collisionCooldown <= 0) {
                isCoolingDown = false;
                collisionCooldown = MAX_COLLISION_COOLDOWN;
            }
        } else {
            color = vec3(0.3f, 0.7f, 0.0f);
        }
    }

    void collideWith(void *thisCollider, void *otherCollider) override {
        if (isCoolingDown) return;

        Particle *thisParticle = static_cast<Particle*>(thisCollider);
        Particle *otherParticle = static_cast<Particle*>(otherCollider);

        int otherObjectId = otherParticle->getObjectId();

        // Centipede collision
        if (otherObjectId == 1) {
            vec3 responseVelocity = (torso->getPosition() - otherParticle->getPosition()) * 0.05f;
            responseVelocity.y = 0.1f;
            controllerVelocity = responseVelocity;
            base->setVelocity(responseVelocity);
            torso->setVelocity(responseVelocity);
            isOnGround = false;
            health--;
            isCoolingDown = true;
        }

        // Emu collision
        if (otherObjectId == 2) {
            vec3 responseVelocity = (torso->getPosition() - otherParticle->getPosition()) * 0.05f;
            responseVelocity.y = 0.1f;
            controllerVelocity = responseVelocity;
            base->setVelocity(responseVelocity);
            torso->setVelocity(responseVelocity);
            isOnGround = false;
            health--;
            isCoolingDown = true;
        }
    }

    mat4 getXform() {
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

        return Translate(torso->getPosition()) * t * Scale(0.8, 0.8, 0.8);
    }

    ~Player() {
    }

    vec3 getControllerPosition() { return controllerPosition; }
    vec3 getLookDirection() { return lookDirection; }
    int getHealth() { return health; }
    bool getIsShooting() { return isShooting; }

private:
    const float CONTROLLER_RADIUS = 0.5f;
    const float MAX_SPEED = 0.18f;
    const float MOVE_FORCE = 0.02f;
    const float MOVE_FRICTION = 0.08f;
    const float STRIDE_LENGTH = 2.5f;
    const float STRIDE_LENGTH_MIN = 0.1f;
    const float STEP_SPEED = 0.4;
    const float FOOT_STRADDLE_OFFSET = 0.6;
    const float FOOT_RADIUS = 0.3f;
    const float HAND_RADIUS = 0.3f;

    const float HEIGHT = 2.0f;
    const float ARM_LENGTH = 1.2f;
    const float LEG_LENGTH = 2.1f;

    const int MAX_HEALTH = 10;
    const float MAX_COLLISION_COOLDOWN = 1.5;

    vec3 controllerPosition;
    vec3 controllerVelocity;
    vec3 up;
    float pitch, yaw;
    vec3 lookDirection;
    double lastMouseX, lastMouseY;
    bool isMoving, isOnGround, isMousePressed;
    bool isShooting;

    vec3 tailPosition;
    vec3 bodyDirection;
    vec3 leftFootTarget;
    vec3 rightFootTarget;
    float stride;
    bool shouldMoveLeftFoot;

    Particle *base;
    Particle *torso;
    Particle *leftHand, *rightHand;
    Particle *leftFoot, *rightFoot;
};

#endif
