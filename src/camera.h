#include <glm/glm.hpp>

class Camera
{
    public:

        Camera(glm::vec3 position, glm::vec3 front, glm::vec3 up) {
            this->position = position;
            this->front = front;
            this->up = up;
        }

        glm::mat4 getView() {
            return glm::lookAt(position, position + front, up);
        }

        void processInput(GLFWwindow *window)
        {
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
                glfwSetWindowShouldClose(window, true);

            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                position += speed * front;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                position -= speed * front;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                position -= glm::normalize(glm::cross(front, up)) * speed;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                position += glm::normalize(glm::cross(front, up)) * speed;

            position.y = 0.0f;
        }

        void processMouseMovement(float xoffset, float yoffset) {
            yaw   += xoffset;
            pitch += yoffset;

            if(pitch > 89.0f)
              pitch =  89.0f;
            if(pitch < -89.0f)
              pitch = -89.0f;

            glm::vec3 direction;
            direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
            direction.y = sin(glm::radians(pitch));
            direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
            front = glm::normalize(direction);
        }
 
    private:
        glm::vec3 position;
        glm::vec3 front;
        glm::vec3 up;
        
        float yaw = 270.0f;
        float pitch = 0.0f;
        
        const float speed = 0.05f;
        const float sensitivity = 0.01f;
};
   
      
