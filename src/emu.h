#ifndef EMU_H
#define EMU_H

#include "game_object.h"
#include "particle.h"
#include "physics_manager.h"
#include "player.h"
#include "spring.h"

#include "VecMat.h"

#include <vector>

class Emu: public GameObject {
public:
    Emu(PhysicsManager *pm, vec3 controllerPosition) {
        objectId = EMU;
        color = vec3(0.6, 0.3, 0.2);

        health = MAX_HEALTH;
        collisionCooldown = MAX_COLLISION_COOLDOWN;
        isCoolingDown = false;
        cooldownFlashTimer = 0;
        isCooldownFlash = false;

        up = vec3(0, 1, 0);
        this->controllerPosition = controllerPosition + vec3(0, 0.4, 0);
        controllerVelocity = vec3(0, 0, 0);
        targetPosition = vec3(rand() % 40 - 20, 0, rand() % 40 - 20);
        tailPosition = controllerPosition - vec3(0, 0, 1);
        leftFootTarget = controllerPosition + vec3(FOOT_STRADDLE_OFFSET, 0, 0);
        rightFootTarget = controllerPosition + vec3(-FOOT_STRADDLE_OFFSET, 0, 0);
        shouldMoveLeftFoot = true;
        stride = 0;

        base = new Particle(this, objectId, controllerPosition, 1, 0.4);
        torso = new Particle(this, objectId, controllerPosition + vec3(0, 4, 0), 1, 1);
        head = new Particle(this, objectId, controllerPosition + vec3(0, 7, 0), 1, 0.5);
        leftFoot = new Particle(this, objectId, controllerPosition + vec3(1, 0, 0), 1, 0.4);
        rightFoot = new Particle(this, objectId, controllerPosition + vec3(-1, 0, 0), 1, 0.4);
        leftKnee = new Particle(this, objectId, controllerPosition + vec3(1, 2, 0), 1, 0.2);
        rightKnee = new Particle(this, objectId, controllerPosition + vec3(-1, 2, 0), 1, 0.2);

        neckSegments.push_back(new Particle(this, objectId, controllerPosition + vec3(0, 5, 0), 2, 0.2));
        neckSegments.push_back(new Particle(this, objectId, controllerPosition + vec3(0, 6, 0), 2, 0.2));

        pm->addParticle(base, false);
        pm->addParticle(torso);
        pm->addParticle(head);
        pm->addParticle(leftKnee);
        pm->addParticle(rightKnee);
        pm->addParticle(leftFoot);
        pm->addParticle(rightFoot);
        pm->addParticle(neckSegments[0]);
        pm->addParticle(neckSegments[1]);

        pm->addSpring(new Spring(base, torso, 3, 0.08, 0.01), false);
        pm->addSpring(new Spring(torso, leftKnee, 1.5, 0.2, 0.2));
        pm->addSpring(new Spring(torso, rightKnee, 1.5, 0.2, 0.2));
        pm->addSpring(new Spring(leftKnee, leftFoot, 1.5, 0.2, 0.2));
        pm->addSpring(new Spring(rightKnee, rightFoot, 1.5, 0.2, 0.2));
        pm->addSpring(new Spring(torso, neckSegments[0], 0.6, 0.2, 0.2));
        pm->addSpring(new Spring(neckSegments[0], neckSegments[1], 0.6, 0.2, 0.2));
        pm->addSpring(new Spring(neckSegments[1], head, 0.6, 0.2, 0.2));
    }

    void update(double timeDelta, void* playerPtr) override {
        Player* player = static_cast<Player*>(playerPtr);

        isTargetingPlayer = length(controllerPosition - player->getControllerPosition()) < 12;

        if (isTargetingPlayer) {
            targetPosition = player->getControllerPosition();
            controllerVelocity = normalize(targetPosition - base->getPosition()) * MAX_SPEED;
        } else {
            timeToSwitchTarget -= timeDelta;
            if (timeToSwitchTarget <= 0) {
                targetPosition = vec3(rand() % 40 - 20, 0, rand() % 40 - 20);
                timeToSwitchTarget = 2 + rand() % 5;
            }
            controllerVelocity = normalize(targetPosition - base->getPosition()) * MAX_SPEED * 0.6;
        }

        controllerVelocity.y = 0;
        controllerPosition += controllerVelocity;

        vec3 delta = controllerPosition - tailPosition;
        const float r = 0.5 / length(delta);
        tailPosition = controllerPosition - delta * r;
        delta = normalize(controllerPosition - tailPosition);
        bodyDirection.x = delta.x;
        bodyDirection.y = 0;
        bodyDirection.z = delta.z;

        base->setPosition(controllerPosition);
        torso->setPosition(vec3(base->getPosition().x, torso->getPosition().y, base->getPosition().z));
        head->applyForce((bodyDirection * 0.03 + vec3(0, 0.02, 0)));

        const vec3 horizontalVelocity = vec3(controllerVelocity.x, 0, controllerVelocity.z);
        stride += length(horizontalVelocity);

        // Determine the length of a stride based on the current horizontal velocity
        strideLength = length(horizontalVelocity) * 19.0;
        if (strideLength < 0.6) strideLength = 0.6;

        // Start a new stride with the opposite foot
        if (stride >= strideLength) {
            const vec3 footStraddleOffset = normalize(cross(horizontalVelocity, up)) * FOOT_STRADDLE_OFFSET;
            const vec3 footTarget = controllerPosition + normalize(horizontalVelocity) * (STRIDE_LENGTH_MIN + length(controllerVelocity) * 21);

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

        leftFoot->setPosition(leftFootPosition + (leftFootTarget - leftFootPosition) * STEP_SPEED);
        rightFoot->setPosition(rightFootPosition + (rightFootTarget - rightFootPosition) * STEP_SPEED);

        leftFoot->setForceExcemption(true);
        rightFoot->setForceExcemption(true);

        if (isCoolingDown) {
            color = isCooldownFlash ? vec3(1.0, 0, 0) : vec3(0.6, 0.3, 0.2);
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
            color = vec3(0.6, 0.3, 0.2);
        }
    }

    void collideWith(void *thisCollider, void *otherCollider) override {
        if (isCoolingDown) return;

        Particle* thisParticle = static_cast<Particle*>(thisCollider);
        Particle* otherParticle = static_cast<Particle*>(otherCollider);

        int otherObjectId = otherParticle->getObjectId();

        // Collision with player
        if (otherObjectId == 0) {
            vec3 responseForce = (thisParticle->getPosition() - otherParticle->getPosition()) * 0.1f;
            thisParticle->applyForce(responseForce);
            health--;
            isCoolingDown = true;
        }

        // Collision with bullet
        if (otherObjectId == -1) {
            vec3 responseForce = (thisParticle->getPosition() - otherParticle->getPosition()) * 0.1f;
            thisParticle->applyForce(responseForce);
            health--;
            isCoolingDown = true;
        }

        if (health < 0) {
            shouldBeDeleted = true;
            controllerPosition = vec3(1000, 0, -100);
            leftFootTarget = vec3(1000, 0, -100);
            rightFootTarget = vec3(1000, 0, -100);
            base->setPosition(vec3(1000, 0, -100));
            torso->setPosition(vec3(1000, 0, -100));
            head->setPosition(vec3(1000, 0, -100));
            leftFoot->setPosition(vec3(1000, 0, -100));
            rightFoot->setPosition(vec3(1000, 0, -100));
            leftKnee->setPosition(vec3(1000, 0, -100));
            rightKnee->setPosition(vec3(1000, 0, -100));
            for (int i = 0; i < neckSegments.size(); i++) {
                neckSegments[i]->setPosition(vec3(1000, 0, -100));
            }
        }
    }

private:
    const float MAX_SPEED = 0.10f;
    const float STRIDE_LENGTH_MIN = 0.1f;
    const float STEP_SPEED = 0.6;
    const float FOOT_STRADDLE_OFFSET = 0.6;

    const int MAX_HEALTH = 3;
    const float MAX_COLLISION_COOLDOWN = 1;

    vec3 controllerPosition;
    vec3 controllerVelocity;
    vec3 targetPosition;
    vec3 up;
    vec3 tailPosition;
    vec3 bodyDirection;
    vec3 leftFootTarget, rightFootTarget;
    float stride, strideLength;
    bool shouldMoveLeftFoot;
    bool isTargetingPlayer;
    float timeToSwitchTarget = 5;

    Particle *base;
    Particle *torso;
    Particle *head;
    Particle *leftKnee, *rightKnee;
    Particle *leftFoot, *rightFoot;
    std::vector<Particle*> neckSegments;
};

#endif
