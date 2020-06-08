#include "game.h"

#include <glad.h>
#include <GLFW/glfw3.h>
void DisplayText(const char *fontName, const char *text, int y) {
    SetFont(fontName, 16, 60);  // exact affect of charRes, pixelRes unclear
    Text(20, y, vec3(1,0,0), 10, text);
}

void Display() {
    glClearColor(.8f, .8f, .8f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // total build time for following 14 fonts ~ .25 secs
    DisplayText("./fonts/Monkey.ttf", "OpenSans-Bold", 640);
    // DisplayText("./fonts/OpenSans/OpenSans-BoldItalic.ttf", "OpenSans-BoldItalic", 600);
    // DisplayText("./fonts/OpenSans/OpenSans-ExtraBold.ttf", "OpenSans-ExtraBold", 560);
    // DisplayText("./fonts/OpenSans/OpenSans-ExtraBoldItalic.ttf", "OpenSans-ExtraBoldItalic", 520);
    // DisplayText("./fonts/OpenSans/OpenSans-Italic.ttf", "OpenSans-Italic", 480);
    // DisplayText("./fonts/OpenSans/OpenSans-Light.ttf", "OpenSans-Light", 440);
    // DisplayText("./fonts/OpenSans/OpenSans-LightItalic.ttf", "OpenSans-LightItalic", 400);
    // DisplayText("./fonts/OpenSans/OpenSans-Regular.ttf", "OpenSans-Regular", 360);
    // DisplayText("./fonts/OpenSans/OpenSans-SemiBold.ttf", "OpenSans-SemiBold", 320);
    // DisplayText("./fonts/OpenSans/OpenSans-SemiBoldItalic.ttf", "OpenSans-SemiBoldItalic", 280);
    glFlush();
}
int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GL_FALSE);
#endif

    const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    int monitorWidth = mode->width;
    int monitorHeight = mode->height;

    // glfw window creation
    GLFWwindow *window = glfwCreateWindow(monitorWidth, monitorHeight, "SproinGL", glfwGetPrimaryMonitor(), NULL);

    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    //glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // Initialize game
    // Game game(window, monitorWidth, monitorHeight);

    // Game loop
    double lastTime = glfwGetTime();

    glViewport(0,0,monitorWidth, monitorHeight);
    glfwSwapInterval(1);
    while (!glfwWindowShouldClose(window)) {
        // double currentTime = glfwGetTime();
        // double timeDelta = currentTime - lastTime;
        // lastTime = currentTime;

        // if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        //     glfwSetWindowShouldClose(window, GLFW_TRUE);

        // game.update(timeDelta);
        // game.draw();
        Display();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}
