// main.cpp (fullscreen + 75 FPS limiter; everything else unchanged)

#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <thread>
#include <chrono>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Scene.h"
#include "RenderContext.h"

static const double TARGET_FPS = 75.0;
static const double TARGET_DT = 1.0 / TARGET_FPS;

static void limit_fps(double frameStart)
{
    double frameTime = glfwGetTime() - frameStart;
    if (frameTime < TARGET_DT)
    {
        std::this_thread::sleep_for(std::chrono::duration<double>(TARGET_DT - frameTime));
    }
}

int gWidth = 1280;
int gHeight = 800;

float lastX = 1280 / 2.0f;
float lastY = 800 / 2.0f;
bool firstMouse = true;

float yaw = -90.0f;
float pitch = 0.0f;
float fov = 45.0f;

glm::vec3 cameraPos(0.0f, 0.0f, 8.0f);
glm::vec3 cameraFront(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);

static void framebuffer_size_callback(GLFWwindow*, int w, int h)
{
    gWidth = (w > 0) ? w : 1;
    gHeight = (h > 0) ? h : 1;
    glViewport(0, 0, gWidth, gHeight);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos;
    lastX = (float)xpos;
    lastY = (float)ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= (float)yoffset;
    if (fov < 20.0f) fov = 20.0f;
    if (fov > 80.0f) fov = 80.0f;
}

int main()
{
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_CORE_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // -----------------------------
    // FULLSCREEN (primary monitor)
    // -----------------------------
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    if (mode)
    {
        gWidth = mode->width;
        gHeight = mode->height;
    }

    GLFWwindow* window = glfwCreateWindow(gWidth, gHeight, "Sprinkle Debug Camera", monitor, NULL);
    if (!window) { glfwTerminate(); return -1; }

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) { glfwTerminate(); return -1; }

    // viewport + resize callback
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glViewport(0, 0, gWidth, gHeight);

    glEnable(GL_DEPTH_TEST);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // IMPORTANT: initGL after GL context exists
    Scene scene;
    scene.initGL();

    float lastTime = (float)glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        const double frameStart = glfwGetTime();

        glfwPollEvents();

        float now = (float)glfwGetTime();
        float dt = now - lastTime;
        lastTime = now;

        float cameraSpeed = 4.0f * dt;
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // --- EDGE KEYS (kao pre) ---
        static bool oneWasDown = false;
        static bool twoWasDown = false;
        static bool spaceWasDown = false;
        static bool enterWasDown = false;
        static bool rWasDown = false;
        static bool fWasDown = false;
        static bool pWasDown = false;

        bool oneDown = glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS;
        if (oneDown && !oneWasDown) scene.on1Pressed();
        oneWasDown = oneDown;

        bool twoDown = glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS;
        if (twoDown && !twoWasDown) scene.on2Pressed();
        twoWasDown = twoDown;

        bool spaceDown = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
        if (spaceDown && !spaceWasDown) scene.onSpacePressed();
        spaceWasDown = spaceDown;

        bool enterDown = glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS;
        if (enterDown && !enterWasDown) scene.onEnterPressed();
        enterWasDown = enterDown;

        bool rDown = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;
        if (rDown && !rWasDown) scene.onResetPressed();
        rWasDown = rDown;

        bool fDown = glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS;
        if (fDown && !fWasDown) scene.onSPressed();
        fWasDown = fDown;

        bool pDown = glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS;
        if (pDown && !pWasDown) scene.onPPressed();
        pWasDown = pDown;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cameraPos += cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cameraPos -= cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            cameraPos.y -= cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            cameraPos.y += cameraSpeed;

        scene.update(dt);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        RenderContext ctx;
        ctx.P = glm::perspective(glm::radians(fov),
            (float)gWidth / (float)gHeight,
            0.1f,
            100.0f);

        ctx.V = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        ctx.lightPos = glm::vec3(-2.0f, 4.0f, 3.0f);
        ctx.viewPos = cameraPos;
        ctx.lightColor = glm::vec3(1.0f);

        scene.render(ctx);

        glfwSwapBuffers(window);

        // -----------------------------
        // 75 FPS LIMITER
        // -----------------------------
        limit_fps(frameStart);
    }

    glfwTerminate();
    return 0;
}
