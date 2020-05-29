#include "particle.h"

#include "VecMat.h"

class Bullet: public GameObject {
public:
    Bullet(vec3 position, vec3 velocity)
    : particle(this, BULLET, position, 1, 0.4, 0.1, false, velocity)
    {
        objectId = BULLET;
        timer = DELETE_TIME;
    }

    void update(double timeDelta, void*) override {
        timer -= timeDelta;
        if (timer <= 0) {
            shouldDelete = true;
        }
    }

    void collideWith(void *thisCollider, void *otherCollider) override {
    }

private:
    const int DELETE_TIME = 5;

    float timer;
    Particle particle;
};
