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

        head = new Particle(vec3(controllerPosition), 1, 1, 0.99);
        pm->addParticle(head);

        Particle *prevMadeParticle = head;
        for (int i = 1; i < numBodySegments; i++) {
            Particle *nextParticle = new Particle(vec3(controllerPosition - controllerDirection * 2.5 * i), 1, 1, 0.99);
            pm->addParticle(nextParticle);
            pm->addSpring(new Spring(prevMadeParticle, nextParticle, 2.5, 0.1, 0.01));
            prevMadeParticle = nextParticle;
        }
    }

    void update(double timeDelta) {
        controllerPosition += controllerVelocity;
        head->setPosition(controllerPosition);
    }

    /*
    void update(double timeDelta, Game *game) {
        vec3 playerPosition = game->getPlayer()->getControllerPosition();
        vec3 force = normalize(playerPosition - head->getPosition()) * 0.01f;
        head->applyForce(force);
    }
    */

private:
    vec3 controllerPosition;
    vec3 controllerDirection;
    vec3 controllerVelocity;
    int numBodySegments = 8;

    Particle* head;
};

#endif
