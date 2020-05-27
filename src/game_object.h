#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

class GameObject {

    public:
        virtual void collideWith(void*, void*) = 0;

    protected:
        const int PLAYER = 0;
        const int CENTIPEDE = 1;
        const int EMU = 2;

        int objectId;
        bool shouldDelete;

        virtual void update(double, void*) = 0;
};

#endif
