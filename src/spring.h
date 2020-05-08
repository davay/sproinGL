#ifndef SPRING_H
#define SPRING_H

#include "Particle.h"

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
        vec3 middle = (p1Position + p2Position) / 2.0f;
        vec3 positionDelta = p2Position - p1Position;

        //Quaternion q(p2Position - p1Position, 0);
        //Quaternion q(vec3(0, 0, 1), 1.57);
        //mat4 m = q.GetMatrix();
        //return Translate(vec3(0, 2, 0)) * m * Scale(0.5, length(positionDelta), 0.5);
        //return Translate(middle) * m * Scale(0.5, length(positionDelta), 0.5);

        vec3 up(0, 1, 0);
        /*
        vec3 z = normalize(positionDelta);
        vec3 x = normalize(cross(up, z));
        vec3 y = normalize(cross(z, x));
        mat4 m = mat4(vec4(x, 0), vec4(y, 0), vec4(z, 0), vec4(0, 0, 0, 1));
        */

        //float angle = acos(dot(normalize(positionDelta), up));
        //Quaternion q(x, angle);
        //mat4 m = q.GetMatrix();
        //printf("%f\n", angle);

        vec3 zaxis = normalize(positionDelta);
        vec3 xaxis = normalize(cross(up, zaxis));
        vec3 yaxis = cross(zaxis, xaxis);

        mat4 m = mat4(
            vec4(xaxis.x, yaxis.x, zaxis.x, 0),
            vec4(xaxis.y, yaxis.y, zaxis.y, 0),
            vec4(xaxis.z, yaxis.z, zaxis.z, 0),
            vec4(0, 0, 0, 1)
        );

        return Translate(middle) * m;

        //return Translate(vec3(0, 3, 0)) * m * Scale(0.5, 0.5, length(positionDelta));
        //return LookAt(middle, p1Position, vec3(0, 1, 0));
    }

private:
    Particle *p1, *p2;
    float stiffness, damping;
    float targetLength;
};

#endif
