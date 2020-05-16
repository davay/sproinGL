#include "VecMat.h"

class Centipede {
public:
    Centipede(vec3 controllerPosition {
        this->controllerPosition = controllerPosition;
    }

private:
    vec3 controllerPosition;
    vec3 controllerDirection;
    vec3 controllerVelocity;
};
