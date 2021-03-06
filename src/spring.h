#ifndef SPRING_H
#define SPRING_H

#include "particle.h"

#include "Quaternion.h"
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

    mat4 getXform() {
        vec3 p1Position = p1->getPosition();
        vec3 p2Position = p2->getPosition();
        vec3 positionDelta = p2Position - p1Position;
        vec3 middle = (p1Position + p2Position) / 2.0f;
        vec3 up = (dot(positionDelta, up) > 0.00001f) ? vec3(0, 1, 0) : vec3(0, 0, 1);

        vec3 z = normalize(positionDelta);
        vec3 x = normalize(cross(up, z));
        vec3 y = normalize(cross(z, x));

        mat4 m = mat4(
            vec4(x, 0),
            vec4(y, 0),
            vec4(z, 0),
            vec4(0, 0, 0, 1)
        );

        mat4 rotate = Transpose(m);

        return Translate(middle) * rotate * Scale(0.5, 0.5, length(positionDelta));
    }

    mat4 getShadowXform() {
        vec3 p1Position = vec3(p1->getPosition().x, 0.001, p1->getPosition().z);
        vec3 p2Position = vec3(p2->getPosition().x, 0.001, p2->getPosition().z);
        vec3 positionDelta = p2Position - p1Position;
        vec3 middle = (p1Position + p2Position) / 2.0f;
        middle.y = 0.001f;
        vec3 up = (dot(positionDelta, up) > 0.00001f) ? vec3(0, 1, 0) : vec3(0, 0, 1);

        vec3 z = normalize(positionDelta);
        vec3 x = normalize(cross(up, z));
        vec3 y = normalize(cross(z, x));

        mat4 m = mat4(
            vec4(x, 0),
            vec4(y, 0),
            vec4(z, 0),
            vec4(0, 0, 0, 1)
        );

        mat4 rotate = Transpose(m);

        return Translate(middle) * rotate * Scale(0.001, 0.125, length(positionDelta) * 0.5);
    }

    bool isInArena() {
        return p1->isInArena() && p2->isInArena();
    }
    
private:
    Particle *p1, *p2;
    float stiffness, damping;
    float targetLength;
};

#endif
