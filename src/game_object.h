#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include "VecMat.h"

class GameObject {

    public:
        virtual void update(double, void*) = 0;
        virtual void collideWith(void*, void*) = 0;
        virtual vec3 getColor() { return color; }

    protected:
        const int PLAYER = 0;
        const int CENTIPEDE = 1;
        const int EMU = 2;
        const int BULLET = 3;

        int objectId;
        bool shouldBeDeleted;
        int health;
        float collisionCooldown;
        bool isCoolingDown;
        vec3 color;
};

#endif
