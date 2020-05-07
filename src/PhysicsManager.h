#ifndef PHYSICS_MANAGER_H
#define PHYSICS_MANAGER_H

#include "Particle.h"
#include "Spring.h"

#include <stdlib.h>

class PhysicsManager {
public:
    PhysicsManager() { }

    void addParticle(Particle *particle) {
        particles.push_back(particle);
    }

    std::vector<Particle*> *getParticles() {
        return &particles;
    }

private:
    std::vector<Particle*> particles;
};

#endif
