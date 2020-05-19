#ifndef CENTIPEDE_H
#define CENTIPEDE_H

#include "VecMat.h"

#include <stdlib.h>

class Centipede {
public:
    Centipede(PhysicsManager *pm, vec3 controllerPosition) {
        this->controllerPosition = controllerPosition;
        controllerDirection = vec3(0, 0, 1);
        controllerVelocity = vec3(0.1, 0, 0);

        head = new Particle(vec3(controllerPosition), vec3(0, 0, 0), 1, 1, 0.99, true);
        pm->addParticle(head);

        Particle *prevMadeParticle = head;
        for (int i = 1; i < NUM_BODY_SEGMENTS; i++) {
            Particle *nextParticle = new Particle(vec3(controllerPosition - controllerDirection * 2.5 * i), 1, 1, 0.99);
            pm->addParticle(nextParticle);
            pm->addSpring(new Spring(prevMadeParticle, nextParticle, 2.5, 0.01, 0.001));
            prevMadeParticle = nextParticle;
        }
    }

    /**
     * Always follow the player.
     */
    void update(double timeDelta, Player *player) {
        vec3 playerPosition = player->getControllerPosition();
        vec3 targetPosition = vec3(playerPosition.x, 1, playerPosition.z);
        vec3 acceleration = normalize(targetPosition - controllerPosition) * 0.05f;

        controllerVelocity = acceleration;
        controllerPosition += controllerVelocity;
        head->setPosition(controllerPosition);
    }

private:
    const int NUM_BODY_SEGMENTS = 6;

    vec3 controllerPosition;
    vec3 controllerDirection;
    vec3 controllerVelocity;

    Particle* head;
};

#endif
