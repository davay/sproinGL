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
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            mat4 cameraAngle = camera->GetRotate();
            vec4 d = cameraAngle * vec4(0, 0, 1, 1);
            vec3 a = normalize(vec3(d.x, 0, -d.z));
            controllerVelocity += a * 0.01;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            mat4 cameraAngle = camera->GetRotate();
            vec4 d = cameraAngle * vec4(0, 0, -1, 1);
            vec3 a = normalize(vec3(d.x, 0, -d.z));
            controllerVelocity += a * 0.01;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            mat4 cameraAngle = camera->GetRotate();
            vec4 d = cameraAngle * vec4(-1, 0, 0, 1);
            vec3 a = normalize(vec3(d.x, 0, -d.z));
            controllerVelocity += a * 0.01;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            mat4 cameraAngle = camera->GetRotate();
            vec4 d = cameraAngle * vec4(1, 0, 0, 1);
            vec3 a = normalize(vec3(d.x, 0, -d.z));
            controllerVelocity += a * 0.01;
        }


        /*
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            mat4 cameraAngle = camera.GetRotate();
            vec4 dir = cameraAngle * vec4(0, 0, moveSpeed, 1);
            player->applyForce(vec3(dir.x, 0, -dir.z));
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            mat4 cameraAngle = camera.GetRotate();
            vec4 dir = cameraAngle * vec4(0, 0, -moveSpeed, 1);
            player->applyForce(vec3(dir.x, 0, -dir.z));
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            mat4 cameraAngle = camera.GetRotate();
            vec4 dir = cameraAngle * vec4(-moveSpeed, 0, 0, 1);
            player->applyForce(vec3(dir.x, 0, -dir.z));
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            mat4 cameraAngle = camera.GetRotate();
            vec4 dir = cameraAngle * vec4(moveSpeed, 0, 0, 1);
            player->applyForce(vec3(dir.x, 0, -dir.z));
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            player->applyForce(vec3(0, 0.02, 0));
        }
        */
    }

    void update(double timeDelta) {
        //controllerVelocity += vec3(0, -0.0002, 0);
        controllerPosition += controllerVelocity;
    }

    mat4 getXform() {
        return Translate(controllerPosition);
    }

private:
    const float CONTROLLER_RADIUS = 1.0f;
    const float MAX_SPEED = 2.0f;

    vec3 controllerPosition;
    vec3 controllerDirection;
    vec3 controllerVelocity;
    bool isOnGround;
};
