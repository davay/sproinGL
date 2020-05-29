#include "particle.h"

#include "VecMat.h"

class Ball: private GameObject {
public:
    Ball(vec3 position)
    : particle(this, 3, position, 1, 0.5)
    {

    }

private:
    Particle particle;
};
