// main.cpp

#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Scene.h"
#include "RenderContext.h"

const unsigned int wWidth = 1280;
const unsigned int wHeight = 800;

float lastX = wWidth / 2.0f;
float lastY = wHeight / 2.0f;
bool firstMouse = true;

float yaw = -90.0f;
float pitch = 0.0f;
float fov = 45.0f;

glm::vec3 cameraPos(0.0f, 0.0f, 8.0f);
glm::vec3 cameraFront(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);

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
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_CORE_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(wWidth, wHeight, "Sprinkle Debug Camera", NULL, NULL);
    glfwMakeContextCurrent(window);

    glewInit();

    glViewport(0, 0, wWidth, wHeight);
    glEnable(GL_DEPTH_TEST);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    Scene scene;

    float lastTime = (float)glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        float now = (float)glfwGetTime();
        float dt = now - lastTime;
        lastTime = now;

        float cameraSpeed = 4.0f * dt;
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // --- EDGE KEYS (kao pre) ---
        static bool spaceWasDown = false;
        static bool rWasDown = false;
        static bool fWasDown = false;

        bool spaceDown = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
        if (spaceDown && !spaceWasDown)
            scene.onSpacePressed();
        spaceWasDown = spaceDown;

        bool rDown = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;
        if (rDown && !rWasDown)
            scene.onResetPressed();
        rWasDown = rDown;

        // --- SPRINKLES: HOLD F = emit ON, release = emit OFF ---
        bool fDown = glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS;
        if (fDown && !fWasDown)
            scene.onSPressed();
		fWasDown = fDown;
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
            (float)wWidth / (float)wHeight,
            0.1f,
            100.0f);

        ctx.V = glm::lookAt(
            cameraPos,
            cameraPos + cameraFront,
            cameraUp
        );

        ctx.lightPos = glm::vec3(2.0f, 3.0f, 4.0f);
        ctx.viewPos = cameraPos;
        ctx.lightColor = glm::vec3(1.0f);

        scene.render(ctx);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
