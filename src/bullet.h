#ifndef BULLET_H
#define BULLET_H

#include "particle.h"

#include "VecMat.h"

class Bullet: public GameObject {
public:
    Bullet(vec3 position, vec3 velocity) {
        objectId = BULLET;
        color = vec3(1, 1, 1);

        health = MAX_HEALTH;
        collisionCooldown = MAX_COLLISION_COOLDOWN;
        isCoolingDown = false;
        cooldownFlashTimer = 0;
        isCooldownFlash = false;

        particle = new Particle(this, 0, position, 1, 0.4, 0.9, false, velocity);
    }

    void update(double timeDelta, void*) override {

    }

    void collideWith(void *thisCollider, void *otherCollider) override {
        Particle* thisParticle = static_cast<Particle*>(thisCollider);
        Particle* otherParticle = static_cast<Particle*>(otherCollider);

        int otherObjectId = otherParticle->getObjectId();

        // Centipede collision
        if (otherObjectId == 1) {
            health--;
        }

        // Centipede collision
        if (otherObjectId == 2) {
            health--;
        }

        if (health <= 0) {
            particle->setPosition(vec3(0, 0, -100));
        }
    }

    Particle* getParticle() { return particle; }

private:
    const int MAX_HEALTH = 1;
    const float MAX_COLLISION_COOLDOWN = 0;

    Particle *particle;
};

#endif
