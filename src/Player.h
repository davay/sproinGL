#include "Camera.h"
#include "VecMat.h"

#include <glad.h>
#include <GLFW/glfw3.h>

class Player {
public:
    Player(vec3 controllerPosition) {
        this->controllerPosition = controllerPosition;
        controllerVelocity = vec3(0, 0, 0);
        controllerDirection = vec3(0, 0, 0);
    }

    void keyboardInput(GLFWwindow *window, Camera *camera) {
        isMoving = false;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            mat4 cameraAngle = camera->GetRotate();
            vec4 d = cameraAngle * vec4(0, 0, 1, 1);
            vec3 a = normalize(vec3(d.x, 0, -d.z));
            controllerVelocity += a * 0.01;
            isMoving = true;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            mat4 cameraAngle = camera->GetRotate();
            vec4 d = cameraAngle * vec4(0, 0, -1, 1);
            vec3 a = normalize(vec3(d.x, 0, -d.z));
            controllerVelocity += a * 0.01;
            isMoving = true;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            mat4 cameraAngle = camera->GetRotate();
            vec4 d = cameraAngle * vec4(-1, 0, 0, 1);
            vec3 a = normalize(vec3(d.x, 0, -d.z));
            controllerVelocity += a * 0.01;
            isMoving = true;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            mat4 cameraAngle = camera->GetRotate();
            vec4 d = cameraAngle * vec4(1, 0, 0, 1);
            vec3 a = normalize(vec3(d.x, 0, -d.z));
            controllerVelocity += a * 0.01;
            isMoving = true;
        }
    }

    void update(double timeDelta) {
        // Apply gravity
        controllerVelocity += vec3(0, -0.01, 0);

        // Decelerate on ground when not pressing a movement key
        if (isOnGround && !isMoving) {
            controllerVelocity += controllerVelocity * -0.1;
        }

        // Limit horizontal movement speed
        if (length(vec3(controllerVelocity.x, 0, controllerVelocity.z)) > MAX_SPEED) {
            float yy = controllerVelocity.y;
            controllerVelocity = normalize(vec3(controllerVelocity.x, 0, controllerVelocity.z)) * MAX_SPEED;
            controllerVelocity.y = yy;
        }

        // Update position
        controllerPosition += controllerVelocity;

        // Collide with ground
        if (controllerPosition.y < 0) {
            controllerPosition.y = 0;
            controllerVelocity.y = 0;
            isOnGround = true;
        }
    }

    mat4 getXform() {

        vec3 dir = vec3(0, 4, 1) - controllerPosition;

        vec3 up(0, 1, 0);
        vec3 z = normalize(dir);
        vec3 x = normalize(cross(up, z));
        vec3 y = normalize(cross(z, x));

        mat4 m = mat4(
            vec4(x, 0),
            vec4(y, 0),
            vec4(z, 0),
            vec4(0, 0, 0, 1)
        );

        mat4 t = Transpose(m);

        return Translate(controllerPosition) * t;
    }

private:
    const float CONTROLLER_RADIUS = 1.0f;
    const float MAX_SPEED = 0.2f;

    vec3 controllerPosition;
    vec3 controllerDirection;
    vec3 controllerVelocity;
    bool isMoving, isOnGround;
};
