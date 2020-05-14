#ifndef PARTICLE_H
#define PARTICLE_H

#include "VecMat.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

class Particle {
public:
    Particle(vec3 position, vec3 velocity, float mass, float radius, float damping, bool isForceExempt) {
        this->position = position;
        this->velocity = velocity;
        this->mass = mass;
        this->radius = radius;
        this->damping = damping;
        isForceExempt = false;
    }

    Particle(vec3 position, float mass, float radius) {
        this->position = position;
        velocity = vec3(0, 0, 0);
        this->mass = mass;
        this->radius = radius;
        damping = 0.9f;
        isForceExempt = false;
    }

    Particle(vec3 position) {
        this->position = position;
        velocity = vec3(0, 0, 0);
        mass = 1.0f;
        radius = 1.0f;
        damping = 0.9f;
        isForceExempt = false;
    }

    void applyForce(vec3 force) {
        netForce += force;
    }

    void update(double timeDelta) {
        if (!isForceExempt) {
            vec3 acceleration = netForce * (1.0f / mass);
            velocity += acceleration;
            position += velocity;
        }

        /*
        if (position.x - radius < -10.0f) {
            position.x = -10.0f + radius;
            velocity.x *= -damping;
        }
        if (position.x + radius > 10.0f) {
            position.x = 10.0f - radius;
            velocity.x *= -damping;
        }
        */
        if (position.y - radius < 0.0f) {
            position.y = 0.0 + radius;
            velocity.x *= damping;
            velocity.y *= -damping;
            velocity.z *= damping;
        }
        /*
        if (position.z -radius < -10.0f) {
            position.z = -10.0f + radius;
            velocity.z *= -damping;
        }
        if (position.z + radius > 10.0f) {
            position.z = 10.0f - radius;
            velocity.z *= -damping;
        }
        */

        netForce.x = netForce.y = netForce.z = 0.0f;
    }

    void setPosition(vec3 position) {
        this->position = position;
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
    vec3 position;
    vec3 velocity;
    vec3 netForce;
    float mass;
    float radius;
    float damping;
    bool isForceExempt;
};

#endif
