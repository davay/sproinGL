#ifndef CENTIPEDE_H
#define CENTIPEDE_H

#include "game_object.h"
#include "particle.h"
#include "physics_manager.h"
#include "player.h"
#include "spring.h"

#include "VecMat.h"

#include <vector>

class Centipede: public GameObject {
public:
    Centipede(PhysicsManager *pm, vec3 controllerPosition) {
        objectId = CENTIPEDE;
        color = vec3(1.0, 0.4, 0.5);

        health = MAX_HEALTH;
        collisionCooldown = MAX_COLLISION_COOLDOWN;
        isCoolingDown = false;
        cooldownFlashTimer = 0;
        isCooldownFlash = false;

        this->controllerPosition = controllerPosition;
        controllerDirection = vec3(0, 0, 1);
        controllerVelocity = vec3(0.1, 0, 0);

        head = new Particle(this, objectId, vec3(controllerPosition), 1, 0.8, 0.95, false);
        pm->addParticle(head);

        Particle *prevMadeParticle = head;
        bodySegments.push_back(head);
        for (int i = 1; i < NUM_BODY_SEGMENTS; i++) {
            Particle *nextParticle = new Particle(
                this, objectId, vec3(controllerPosition - controllerDirection * 2.5 * i), 1, 0.8, 0.95
            );
            pm->addParticle(nextParticle);
            pm->addSpring(new Spring(prevMadeParticle, nextParticle, 2.5, 0.01, 0.001));
            prevMadeParticle = nextParticle;
            bodySegments.push_back(nextParticle);
        }
    }

    /**
     * Always follow the player.
     */
    void update(double timeDelta, void* playerPointer) override {
        Player* player = static_cast<Player*>(playerPointer);
        vec3 playerPosition = player->getControllerPosition();
        vec3 targetPosition = vec3(playerPosition.x, 1, playerPosition.z);
        vec3 force = normalize(targetPosition - head->getPosition()) * 0.010f;

        head->applyForce(force);

        if (isCoolingDown) {
            color = isCooldownFlash ? vec3(1.0, 0, 0) : vec3(1.0, 0.4, 0.5);
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
            color = vec3(1.0, 0.4, 0.5);
        }
    }

    void collideWith(void* thisCollider, void* otherCollider) override {
        if (isCoolingDown) return;

        Particle* thisParticle = static_cast<Particle*>(thisCollider);
        Particle* otherParticle = static_cast<Particle*>(otherCollider);

        int otherObjectId = otherParticle->getObjectId();

        // Collision with player
        if (otherObjectId == 0) {
            vec3 responseForce = (thisParticle->getPosition() - otherParticle->getPosition()) * 0.05f;
            thisParticle->applyForce(responseForce);
            health--;
            isCoolingDown = true;
        }

        // Collision with bullet
        if (otherObjectId == -1) {
            vec3 responseForce = (thisParticle->getPosition() - otherParticle->getPosition()) * 0.05f;
            thisParticle->applyForce(responseForce);
            health--;
        }

        if (health < 0) {
            for (int i = 0; i < bodySegments.size(); i++) {
                bodySegments[i]->setPosition(vec3(0, 0, -100));
            }
        }
    }

private:
    const int NUM_BODY_SEGMENTS = 6;
    const int MAX_HEALTH = 1;
    const float MAX_COLLISION_COOLDOWN = 1;

    vec3 controllerPosition;
    vec3 controllerDirection;
    vec3 controllerVelocity;

    Particle *head;
    std::vector<Particle*> bodySegments;
};

#endif
