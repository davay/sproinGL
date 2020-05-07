#ifndef SPRING_H
#define SPRING_H

#include "Particle.h"

#include "VecMat.h"

#include "math.h"

class Spring {
public:
    Spring(Particle *p1, Particle *p2, float targetLength, float stiffness, float damping) {
        this->p1 = p1;
        this->p2 = p2;
        this->targetLength = targetLength;
        this->stiffness = stiffness;
        this->damping = damping;
    }

    /**
     * Applies an elastic force (Hooke's Law) on each of the two end particles of the spring.
     */
    void applyForce() {
        vec3 p1Position = p1->getPosition();
        vec3 p2Position = p2->getPosition();

        vec3 p1Velocity = p1->getVelocity();
        vec3 p2Velocity = p2->getVelocity();

        vec3 positionDelta = p2Position - p1Position;

        float length = sqrt(
            positionDelta.x * positionDelta.x +
            positionDelta.y * positionDelta.y +
            positionDelta.z * positionDelta.z
        );

        vec3 p1Target = p2Position + (-positionDelta / length) * targetLength;
        vec3 p2Target = p1Position + ( positionDelta / length) * targetLength;

        vec3 p1RelativeVelocity = p1Velocity - p2Velocity;
        vec3 p2RelativeVelocity = p2Velocity - p1Velocity;

        vec3 p1ElasticForce = stiffness * (p1Target - p1Position);
        vec3 p2ElasticForce = stiffness * (p2Target - p2Position);

        vec3 p1FrictionForce = -p1RelativeVelocity * damping;
        vec3 p2FrictionForce = -p2RelativeVelocity * damping;

        vec3 p1Force = p1ElasticForce + p1FrictionForce;
        vec3 p2Force = p2ElasticForce + p2FrictionForce;

        p1->applyForce(p1Force);
        p2->applyForce(p2Force);
    }

private:
    Particle *p1, *p2;
    float stiffness, damping;
    float targetLength;
};

#endif
