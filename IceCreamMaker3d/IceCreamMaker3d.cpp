//Opis: Primjer ucitavanja modela upotrebom ASSIMP biblioteke
//Preuzeto sa learnOpenGL

#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"
#include "model.hpp"

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

    Model machine("res/MachineModel.obj");
    Model iceCream("res/IceCreamModel.obj");
    Model cup("res/Cup.obj");
    Model lever("res/LeverModel.obj");
    Model pour("res/Pour.obj");

    Shader unifiedShader("basic.vert", "basic.frag");

    glEnable(GL_DEPTH_TEST);

    // -------------------------
    // ANIMACIJA SIPANJA (PAUZA/NASTAVAK)
    // -------------------------
    bool pouring = false;
    float pourT = 0.0f;              // 0..1
    const float pourSpeed = 0.3f;    // brzina rasta
    const float pourStartY = 0.0f;
    const float pourEndY = -1.2f;

    const glm::vec3 pourStartOffset(0.0f, pourStartY, 0.0f);
    const glm::vec3 pourEndOffset(0.0f, pourEndY, 0.0f);

    const glm::vec3 iceCreamPosOffset(0.0f, 0.0f, 0.0f);

    float lastTime = (float)glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        float now = (float)glfwGetTime();
        float dt = now - lastTime;
        lastTime = now;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // SPACE: pauza/nastavak (NE VRACA pourT nazad)
        static bool spaceWasDown = false;
        bool spaceDown = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
        if (spaceDown && !spaceWasDown)
            pouring = !pouring;
        spaceWasDown = spaceDown;

        // pourT samo raste dok "pouring" true, inace stoji
        if (pouring)
            pourT = std::min(1.0f, pourT + dt * pourSpeed);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        unifiedShader.use();

        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f),
            (float)wWidth / (float)wHeight,
            0.1f,
            100.0f
        );
        glm::mat4 view = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 8.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

        unifiedShader.setMat4("uP", projection);
        unifiedShader.setMat4("uV", view);

        unifiedShader.setVec3("uLightPos", 2.0f, 3.0f, 4.0f);
        unifiedShader.setVec3("uViewPos", 0.0f, 0.0f, 8.0f);
        unifiedShader.setVec3("uLightColor", 1.0f, 1.0f, 1.0f);

        // Base transform
        glm::mat4 base = glm::mat4(1.0f);
        base = glm::translate(base, glm::vec3(0.0f, -1.0f, 0.0f));
        base = glm::scale(base, glm::vec3(1.4f));

        unifiedShader.setMat4("uM", base);
        machine.Draw(unifiedShader);
        cup.Draw(unifiedShader);
        lever.Draw(unifiedShader);

        // POUR: ide nadole do kraja i staje gde je kad pauziras
        glm::vec3 pourOffset = glm::mix(pourStartOffset, pourEndOffset, pourT);
        glm::mat4 pourM = base;
        pourM = glm::translate(pourM, pourOffset);

        unifiedShader.setMat4("uM", pourM);
        pour.Draw(unifiedShader);

        // ICE CREAM: raste i ostaje na toj visini kad pauziras
        float iceYScale = std::max(0.001f, pourT); // 0..1
        glm::vec3 iceScale(1.0f, iceYScale, 1.0f);

        glm::mat4 iceM = base;
        iceM = glm::translate(iceM, iceCreamPosOffset);
        iceM = glm::scale(iceM, iceScale);

        unifiedShader.setMat4("uM", iceM);
        iceCream.Draw(unifiedShader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
