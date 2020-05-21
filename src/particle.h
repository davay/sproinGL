#ifndef PARTICLE_H
#define PARTICLE_H

#include "game_object.h"
#include "VecMat.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

class Particle {
public:
    Particle(GameObject* owner, int objectId, vec3 position, float mass, float radius, float damping=DEFAULT_DAMPING, bool isForceExempt=false, vec3 velocity=vec3(0,0,0))
        : owner(owner)
        , objectId(objectId)
        , position(position)
        , mass(mass)
        , radius(radius)
        , damping(damping)
        , isForceExempt(isForceExempt)
        , velocity(velocity) { }

    void applyForce(vec3 force) {
        netForce += force;
    }

    void update(double timeDelta) {
        vec3 acceleration = isForceExempt ? 0.0f : netForce / mass;
        velocity += acceleration;
        position += velocity;

        // Collide with the ground
        if (position.x > -25 && position.x < 25) {
            if (position.z > -25 && position.z < 25) {
                if (position.y - radius < 0.0f) {
                    position.y = 0.0 + radius;
                    velocity.x *= damping;
                    velocity.y *= -damping;
                    velocity.z *= damping;

                }
            }
        }

        // Reset net force
        netForce.x = netForce.y = netForce.z = 0.0f;
    }

    void onCollision(Particle* other) {
        owner->onCollision(other);
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
    int getObjectId() { return objectId; }

private:
    static constexpr float DEFAULT_DAMPING = 0.9f;

    vec3 position;
    vec3 velocity;
    vec3 netForce;
    float mass;
    float radius;
    float damping;
    float frictionStrength;
    bool isForceExempt;
    int objectId;
    GameObject* owner;

};


#endif
