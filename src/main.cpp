#include <glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "Camera.h"
#include "GLXtras.h"
#include "VecMat.h"


// Shaders
const char *vertexShaderSource = R"(
    #version 410 core

    layout (location = 0) in vec3 vPos;
    layout (location = 1) in vec3 vCol;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    out vec3 ourColor;

    void main() {
        gl_Position = projection * view * model * vec4(vPos, 1.0);
        ourColor = vCol;
    }
)";

const char *fragmentShaderSource = R"(
    #version 410 core
    #define PI 3.14159265359

    uniform vec2 u_resolution;

    in vec3 ourColor;
    out vec4 FragColor;

    void main() {
      vec2 gridSize = vec2(8.0, 8.0);
      vec2 normalCoord = gl_FragCoord.xy / u_resolution.x;
      vec2 value = sign(sin(normalCoord * PI * gridSize));
      vec3 intensity = vec3(value.x * value.y);
      //FragColor = vec4(ourColor * intensity, 1.0f);
      FragColor = vec4(ourColor, 1.0f);
    }
)";

unsigned int SCR_WIDTH = 640;
unsigned int SCR_HEIGHT = 400;

float lastX = 320, lastY = 200;
bool firstMouse = true;

Camera camera((float) SCR_WIDTH / (2 * SCR_HEIGHT), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -10.0f), 30);

double timeDelta;

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

        /*
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.processKeyboard(FORWARD, timeDelta);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.processKeyboard(BACKWARD, timeDelta);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.processKeyboard(LEFT, timeDelta);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.processKeyboard(RIGHT, timeDelta);
        */
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    //camera.processMouseMovement(xoffset, yoffset);
}

int main() {
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GL_FALSE);
#endif

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Chessboard", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader program
    int shaderProgram = LinkProgramViaCode(&vertexShaderSource, &fragmentShaderSource);

    float vertices[] = {
        // Position          // Color
         1.0f,  1.0f, 0.0f,  1.0f, 0.0f, 0.0f, // Top right
         1.0f, -1.0f, 0.0f,  0.0f, 1.0f, 0.0f, // Bottom right
        -1.0f, -1.0f, 0.0f,  1.0f, 0.0f, 0.0f, // Bottom left
        -1.0f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f  // Top left
    };

    unsigned int indices[] = {
        0, 1, 3,  // First triangle
        1, 2, 3   // Second triangle
    };

    unsigned int vbo, vao, ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    // Get shader uniform locations
    int resolutionLocation = glGetUniformLocation(shaderProgram, "u_resolution");
    int modelLoc = glGetUniformLocation(shaderProgram, "model");
    int viewLoc = glGetUniformLocation(shaderProgram, "view");
    int projectionLoc = glGetUniformLocation(shaderProgram, "projection");

    double lastTime = glfwGetTime();
    double currentTime = lastTime;

    mat4 model = Translate(0.0f, 0.0f, 0.0f);
    mat4 view = Translate(0.0f, 0.0f, -10.0f) * RotateX(10.0f);
    float angle = 0.0f;

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // Get timestep
        currentTime = glfwGetTime();
        timeDelta = currentTime = lastTime;
        lastTime = currentTime;

        // Input
        processInput(window);

        angle += currentTime * 20.0f;

        // Define vertex transformations
        model = RotateY(angle);
        mat4 projection = Perspective(45.0f, (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);

        // Render
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Set shader uniforms
        glUniform2f(resolutionLocation, SCR_WIDTH, SCR_HEIGHT);
        //glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        //glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        //glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        //SetUniform(shaderProgram, "u_resolution", SCR_WIDTH, SCR_HEIGHT);
        SetUniform(shaderProgram, "model", model);
        SetUniform(shaderProgram, "view", view);
        SetUniform(shaderProgram, "projection", projection);


        glBindVertexArray(vao); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
        //glDrawArrays(GL_TRIANGLES, 0, 6);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        // glBindVertexArray(0); // no need to unbind it every time

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();

    return 0;
}
