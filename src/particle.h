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
    float damping;

    Particle(vec3 position) {
        this->position = position;
        velocity = vec3(0.1f, 0.3f, 0.4f);
        radius = 1.0f;
        damping = 0.95f;
    }

    void applyForce(vec3 force) {
        netForce += force;
    }

    void update(double timeDelta) {
        velocity += vec3(0.0f, -0.01f, 0.0f);
        position += velocity;

        if (position.x - radius < -10.0f) {
            position.x = -10.0f + radius;
            velocity.x *= -damping;
        }
        if (position.x + radius > 10.0f) {
            position.x = 10.0f - radius;
            velocity.x *= -damping;
        }
        if (position.y - radius < 0.0f) {
            position.y = 0.0 + radius;
            velocity.x *= damping;
            velocity.y *= -damping;
            velocity.z *= damping;
        }
        if (position.z -radius < -10.0f) {
            position.z = -10.0f + radius;
            velocity.z *= -damping;
        }
        if (position.z + radius > 10.0f) {
            position.z = 10.0f - radius;
            velocity.z *= -damping;
        }

        netForce.x = netForce.y = netForce.z = 0.0f;
    }
};
