#include <glm/glm.hpp>

enum CAMERA_MOVEMENT {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

class Camera {
    public:
        Camera(glm::vec3 position, glm::vec3 front, glm::vec3 up) {
            this->position = position;
            this->front = front;
            this->up = up;

            yaw = 0.0f;
            pitch = 0.0f;
        }

        // Use keyboard to update position
        void processKeyboard(CAMERA_MOVEMENT direction, float timeDelta) {
            if (direction == FORWARD)
                position += speed * front;
            if (direction == BACKWARD)
                position -= speed * front;
            if (direction == LEFT)
                position -= glm::normalize(glm::cross(front, up)) * speed;
            if (direction == RIGHT)
                position += glm::normalize(glm::cross(front, up)) * speed;

            position.y = 0.0f;
        }

        // Use mouse to update view direction
        void processMouseMovement(float xoffset, float yoffset) {
            yaw   += xoffset * mouseSensitivity;
            pitch += yoffset * mouseSensitivity;

            if (pitch > 89.0f) pitch =  89.0f;
            if (pitch < -89.0f) pitch = -89.0f;

            glm::vec3 direction;
            direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
            direction.y = sin(glm::radians(pitch));
            direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
            front = glm::normalize(direction);
        }

        // Get the view transform matrix
        glm::mat4 getView() {
            return glm::lookAt(position, position + front, up);
        }

    private:
        glm::vec3 position;
        glm::vec3 front;
        glm::vec3 up;

        float yaw;
        float pitch;

        const float speed = 0.05f;
        const float mouseSensitivity = 0.1f;
};
