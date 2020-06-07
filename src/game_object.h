#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

class GameObject {

    public:
        virtual void update(double, void*) = 0;
        virtual void collideWith(void*, void*) = 0;

    protected:
        const int PLAYER = 0;
        const int CENTIPEDE = 1;
        const int EMU = 2;
        const int BULLET = 3;

        int objectId;
        bool shouldDelete;
        int health;
        float collisionCooldown;
};

#endif
