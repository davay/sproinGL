#include "VecMat.h"

#include <iostream>
#include <stdio.h>

class Particle {
public:
    vec3 position;
    vec3 velocity;
    vec3 netForce;
    float mass;
    float radius;

    Particle(vec3 position) {
        this->position = position;
        velocity = vec3(0.1f, 0.3f, 0.4f);
    }

    void applyForce(vec3 force) {
        netForce += force;
    }

    void update(double timeDelta) {
        velocity += vec3(0.0f, -0.01f, 0.0f);
        position += velocity;

        if (position.x < -10.0f) {
            position.x = -10.0f;
            velocity.x *= -0.9f;
        }
        if (position.x > 10.0f) {
            position.x = 10.0f;
            velocity.x *= -0.9f;
        }
        if (position.y <= 0.0f) {
            position.y = 0.0f;
            velocity.x *= 0.9f;
            velocity.y *= -0.9f;
            velocity.z *= 0.9f;
        }
        if (position.z < -10.0f) {
            position.z = -10.0f;
            velocity.z *= -0.9f;
        }
        if (position.z > 10.0f) {
            position.z = 10.0f;
            velocity.z *= -0.9f;
        }

        netForce.x = netForce.y = netForce.z = 0.0f;
    }
};
