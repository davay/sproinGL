#include "particle.h"

#include "VecMat.h"

class Bullet: protected GameObject {
public:
    Bullet();

    void update(double timeDelta, void*) override {
    }

    void collideWith(void *thisCollider, void *otherCollider) override {
    }

private:
    float timer;
    Particle *particle;
};
