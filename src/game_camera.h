#ifndef GAME_CAMERA_H
#define GAME_CAMERA_H

#include "player.h"

#include "VecMat.h"

class GameCamera {
public:
    GameCamera(vec3 position, float aspectRatio) {
        this->position = vec3(0, 5, 20);
        up = vec3(0, 1, 0);
        persp = Perspective(45, aspectRatio, 1, 200);
    }

    void update(double timeDelta, Player *player) {
        vec3 playerPosition = player->getControllerPosition();
        vec3 lookDirection = player->getLookDirection();
        position = playerPosition - lookDirection * DISTANCE_FROM_TARGET;
        vec3 target = position + lookDirection;
        view = persp * LookAt(position, target, up);
    }

    mat4 getView() {
        return view;
    }

private:
    const float DISTANCE_FROM_TARGET = 18.0f;

    vec3 position;
    vec3 direction;
    vec3 up;

    mat4 persp, view;
};

#endif
