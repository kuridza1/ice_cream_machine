// main.cpp
// Opis: Primjer ucitavanja modela upotrebom ASSIMP biblioteke
// Preuzeto sa learnOpenGL

#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Scene.h"
#include "RenderContext.h"

const unsigned int wWidth = 800;
const unsigned int wHeight = 600;

int main()
{
    if (!glfwInit())
    {
        std::cout << "GLFW fail!\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(wWidth, wHeight, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Window fail!\n";
        glfwTerminate();
        return -2;
    }
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK)
    {
        std::cout << "GLEW fail!\n";
        glfwTerminate();
        return -3;
    }

    glViewport(0, 0, wWidth, wHeight);
    glClearColor(0.08f, 0.08f, 0.10f, 1.0f);

    glEnable(GL_DEPTH_TEST);

    Scene scene;

    float lastTime = (float)glfwGetTime();
    bool spaceWasDown = false;
    bool rWasDown = false;

    while (!glfwWindowShouldClose(window))
    {
        float now = (float)glfwGetTime();
        float dt = now - lastTime;
        lastTime = now;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        bool spaceDown = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
        if (spaceDown && !spaceWasDown)
            scene.onSpacePressed();
        spaceWasDown = spaceDown;

        bool rDown = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;
        if (rDown && !rWasDown)
            scene.onResetPressed();
        rWasDown = rDown;

        scene.update(dt);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        RenderContext ctx;
        ctx.P = glm::perspective(glm::radians(45.0f), (float)wWidth / (float)wHeight, 0.1f, 100.0f);
        ctx.V = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 8.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

        ctx.lightPos = glm::vec3(2.0f, 3.0f, 4.0f);
        ctx.viewPos = glm::vec3(0.0f, 0.0f, 8.0f);
        ctx.lightColor = glm::vec3(1.0f, 1.0f, 1.0f);

        scene.render(ctx);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
