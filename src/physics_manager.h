#ifndef PHYSICS_MANAGER_H
#define PHYSICS_MANAGER_H

#include "particle.h"
#include "spring.h"

#include <vector>

class PhysicsManager {
public:
    PhysicsManager() {
    }

    /**
     * Update physics objects.
     */
    void update(float timeDelta) {
        // Collide particles with each other
        for (int i = 0; i < particles.size(); i++) {
            Particle *p1 = particles[i];

            for (int j = i + 1; j < particles.size(); j++) {
                Particle *p2 = particles[j];
                vec3 delta = p2->getPosition() - p1->getPosition();

                if (length(delta) > 0 && length(delta) < p1->getRadius() + p2->getRadius()) {
                    float bounceStrength = 0.01f / sqrt(length(delta));
                    vec3 bounceForce = -delta * bounceStrength;
                    p1->applyForce(bounceForce);
                    p2->applyForce(-bounceForce);
                    p1->collideWith(p2);
                    p2->collideWith(p1);
                }
            }
        }

        // Apply spring forces to particles
        for (int i = 0; i < springs.size(); i++) {
            springs[i]->applyForce();
        }

        // Apply gravity force to particles
        for (int i = 0; i < particles.size(); i++) {
            vec3 gravityForce(0.0f, -GRAVITY_STRENGTH, 0.0f);
            particles[i]->applyForce(gravityForce);
        }

        // Move particles
        for (int i = 0; i < particles.size(); i++) {
            particles[i]->update(timeDelta);
        }
    }

    void addParticle(Particle *particle, bool isVisible=true) {
        particles.push_back(particle);
        if (isVisible) visibleParticles.push_back(particle);
    }

    void addSpring(Spring *spring, bool isVisible=true) {
        springs.push_back(spring);
        if (isVisible) visibleSprings.push_back(spring);
    }

    std::vector<Particle*> *getVisibleParticles() {
        return &visibleParticles;
    }

    std::vector<Spring*> *getVisibleSprings() {
        return &visibleSprings;
    }

private:
    const float GRAVITY_STRENGTH = 0.005f;

    std::vector<Particle*> particles;
    std::vector<Spring*> springs;

    std::vector<Particle*> visibleParticles;
    std::vector<Spring*> visibleSprings;
};

#endif
