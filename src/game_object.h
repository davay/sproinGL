#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H


class GameObject {
    
  
    public:
        virtual void onCollision(void*) = 0;
       
    protected:
        virtual void update(double, void*) = 0;
        static const int PLAYER = 0;
        static const int CENTIPEDE = 1;
        int objectId;
};

#endif