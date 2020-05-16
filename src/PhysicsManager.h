#ifndef PHYSICS_MANAGER_H
#define PHYSICS_MANAGER_H

#include "Particle.h"
#include "Spring.h"

#include <stdlib.h>

class PhysicsManager {
public:
    PhysicsManager() {
        int w = 4;
        int h = 4;
        int d = 4;

        for (int z = 0; z < d; z++) {
            for (int y = 0; y < h; y++) {
                for (int x = 0; x < w; x++) {
                    addParticle(new Particle(vec3(-9 + x * 2, 9 + -y * 2, -8 + z * 2), 0.5f, 0.5f));
                }
            }
        }

        for (int z = 0; z < d-1; z++) {
            for (int y = 0; y < h-1; y++) {
                for (int x = 0; x < w-1; x++) {
                        int p = (z * w * h) + (y * w) + x;
                        int xNeighbor = p + 1;
                        int yNeighbor = p + w;
                        int zNeighbor = p + (w * h);
                        int diagNeighbor = p + (w * h) + w + 1;
                        addSpring(new Spring(particles[p], particles[xNeighbor], 2, 0.05, 0.02));
                        addSpring(new Spring(particles[p], particles[yNeighbor], 2, 0.05, 0.02));
                        addSpring(new Spring(particles[p], particles[zNeighbor], 2, 0.05, 0.02));
                        addSpring(new Spring(particles[p], particles[diagNeighbor], 3.46, 0.05, 0.02));
                }
            }
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
                    float bounceStrength = 0.01f / sqrt(length(delta));
                    vec3 bounceForce = -delta * bounceStrength;
                    p1->applyForce(bounceForce);
                    p2->applyForce(-bounceForce);
                }
            }
        }

        // Apply spring forces to particles
        for (int i = 0; i < springs.size(); i++) {
            springs[i]->applyForce();
        }

        // Apply gravity force to particles
        for (int i = 0; i < particles.size(); i++) {
            //vec3 gravityForce(0.0f, -0.01f, 0.0f);
            vec3 gravityForce(0.0f, -0.005f, 0.0f);
            particles[i]->applyForce(gravityForce);
        }

        // Move particles
        for (int i = 0; i < particles.size(); i++) {
            particles[i]->update(timeDelta);
        }
    }

    void addParticle(Particle *particle, bool isVisible) {
        particles.push_back(particle);
        if (isVisible) visibleParticles.push_back(particle);
    }

    void addSpring(Spring *spring, bool isVisible) {
        springs.push_back(spring);
        if (isVisible) visibleSprings.push_back(spring);
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
