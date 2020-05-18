#ifndef PARTICLE_H
#define PARTICLE_H

#include "VecMat.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

class Particle {
public:
    Particle(vec3 position, vec3 velocity, float mass, float radius, float damping, bool isForceExempt)
        : position(position)
        , velocity(velocity)
        , mass(mass)
        , radius(radius)
        , damping(damping)
        , isForceExempt(isForceExempt) { }

    Particle(vec3 position, float mass, float radius, float damping)
        : Particle(position, vec3(0, 0, 0), mass, radius, damping, false) { }

    Particle(vec3 position, float mass, float radius)
        : Particle(position, vec3(0, 0, 0), mass, radius, DEFAULT_DAMPING, false) { }

    void applyForce(vec3 force) {
        netForce += force;
    }

    void update(double timeDelta) {
        vec3 acceleration = isForceExempt ? 0.0f : netForce / mass;
        velocity += acceleration;
        position += velocity;

        // Collide with the ground
        if (position.y - radius < 0.0f) {
            position.y = 0.0 + radius;
            velocity.x *= damping;
            velocity.y *= -damping;
            velocity.z *= damping;
        }

        // Reset net force
        netForce.x = netForce.y = netForce.z = 0.0f;
    }

    void setPosition(vec3 position) {
        this->position = position;
    }

    void setVelocity(vec3 velocity) {
        this->velocity = velocity;
    }

    void setForceExcemption(bool isForceExempt) {
        this->isForceExempt = isForceExempt;
    }

    mat4 getXform() {
        return Translate(position) * Scale(radius, radius, radius);
    }

    vec3 getPosition() { return position; }
    vec3 getVelocity() { return velocity; }
    float getRadius() { return radius; }

private:
    const float DEFAULT_DAMPING = 0.9f;

    vec3 position;
    vec3 velocity;
    vec3 netForce;
    float mass;
    float radius;
    float damping;
    bool isForceExempt;
};

#endif
