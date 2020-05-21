#ifndef CENTIPEDE_H
#define CENTIPEDE_H

#include "game_object.h"
#include "VecMat.h"

#include <stdlib.h>

class Centipede: protected GameObject {
public:
    Centipede(PhysicsManager *pm, vec3 controllerPosition) {
        objectId = CENTIPEDE;
        this->controllerPosition = controllerPosition;
        controllerDirection = vec3(0, 0, 1);
        controllerVelocity = vec3(0.1, 0, 0);

        head = new Particle(this, objectId, vec3(controllerPosition), 1, 1, 0.95, false);
        pm->addParticle(head);

        Particle *prevMadeParticle = head;
        for (int i = 1; i < NUM_BODY_SEGMENTS; i++) {
            Particle *nextParticle = new Particle(
                this, objectId, vec3(controllerPosition - controllerDirection * 2.5 * i), 1, 1, 0.95
            );
            pm->addParticle(nextParticle);
            pm->addSpring(new Spring(prevMadeParticle, nextParticle, 2.5, 0.01, 0.001));
            prevMadeParticle = nextParticle;
        }


    }

    /**
     * Always follow the player.
     */
    void update(double timeDelta, void* playerPointer) override {
        Player* player = static_cast<Player*>(playerPointer);
        vec3 playerPosition = player->getControllerPosition();
        vec3 targetPosition = vec3(playerPosition.x, 1, playerPosition.z);
        vec3 force = normalize(targetPosition - head->getPosition()) * 0.01f;

        head->applyForce(force);
    }

    void onCollision(void* other) override {
        Particle* otherParticle = static_cast<Particle*>(other);
        int otherObjectId = otherParticle->getObjectId();

        if (otherObjectId == 0) {
            vec3 responseForce = (head->getPosition() - otherParticle->getPosition()) * 0.1f;
            responseForce.y = 0.1f;
            head->applyForce(responseForce);
            //health -= 5
        }
    }

private:
    const int NUM_BODY_SEGMENTS = 6;

    vec3 controllerPosition;
    vec3 controllerDirection;
    vec3 controllerVelocity;

    Particle* head;
    std::vector<Particle*> bases;
    std::vector<Particle*> segments;
};

#endif
