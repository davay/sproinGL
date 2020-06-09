#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include "VecMat.h"

class GameObject {

    public:
        virtual void update(double, void*) = 0;
        virtual void collideWith(void*, void*) = 0;
        virtual vec3 getColor() { return color; }
        virtual ~GameObject() { };

    protected:
        const int PLAYER = 0;
        const int CENTIPEDE = 1;
        const int EMU = 2;
        const int BULLET = 3;
        const float MAX_COOLDOWN_FLASH_TIME = 0.08f;

        int objectId;
        int health;
        float collisionCooldown;
        float cooldownFlashTimer;
        bool isCooldownFlash;

        bool isCoolingDown;
        vec3 color;

        // bizzare bug on Devin's Linux, without these 3 extra unused fields, screen will stay on background
        float a;
        float b;
        float c;
};

#endif
