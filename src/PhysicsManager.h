#ifndef PHYSICS_MANAGER_H
#define PHYSICS_MANAGER_H

#include "Particle.h"
#include "Spring.h"

#include <stdlib.h>

class PhysicsManager {
public:
    PhysicsManager() {
        for (int i = 0; i < 10; i++) {
            vec3 position(i * 2.2, 9.0f, -3);
            vec3 velocity(0.0f, 0.0f, 0.0f);
            addParticle(new Particle(position, velocity, 1.0f, 0.5f, 0.9f));
        }

        for (int i = 0; i < 9; i++) {
            addSpring(new Spring(particles[i], particles[i + 1], 2.2, 0.2, 0.05));
        }
    }

    void update(float timeDelta) {
        // Collide particles with each other
        for (int i = 0; i < particles.size(); i++) {
            Particle *p1 = particles[i];
            for (int j = i + 1; j < particles.size(); j++) {
                Particle *p2 = particles[j];
                vec3 delta = p2->getPosition() - p1->getPosition();
                if (length(delta) < p1->getRadius() + p2->getRadius()) {
                    float bounceStrength = 0.05f / sqrt(length(delta));
                    vec3 bounceForce = -delta * bounceStrength;
                    p1->applyForce(bounceForce);
                    p2->applyForce(bounceForce * -1.0f);
                }
            }
        }

        // Apply spring forces to particles
        for (int i = 0; i < springs.size(); i++) {
            springs[i]->applyForce();
        }

        // Apply gravity force to particles
        for (int i = 0; i < particles.size(); i++) {
            vec3 gravityForce(0.0f, -0.01f, 0.0f);
            particles[i]->applyForce(gravityForce);
        }

        // Move particles
        for (int i = 0; i < particles.size(); i++) {
            if (i > 0)
            particles[i]->update(timeDelta);
        }
    }

    void addParticle(Particle *particle, bool isVisible) {
        particles.push_back(particle);
        if (isVisible) visibleParticles.push_back(particle);
        // else invisibleParticles.push_back(particle);
    }

    void addSpring(Spring *spring, bool isVisible) {
        springs.push_back(spring);
        if (isVisible) visibleSprings.push_back(spring);
        // else invisibleSprings.push_back(spring);
    }

    void addParticle(Particle *particle) {
        addParticle(particle, true);
    }

    void addSpring(Spring *spring) {
        addSpring(spring, true);
    }

    std::vector<Particle*> *getVisibleParticles() {
        return &visibleParticles;
    }

    std::vector<Spring*> *getVisibleSprings() {
        return &visibleSprings;
    }

private:
    std::vector<Particle*> particles;
    std::vector<Spring*> springs;

    std::vector<Particle*> visibleParticles;
    std::vector<Spring*> visibleSprings;
};

#endif
