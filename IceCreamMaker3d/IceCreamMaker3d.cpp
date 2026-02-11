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

static inline float clamp01(float x)
{
    return std::max(0.0f, std::min(1.0f, x));
}

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
    // CONTROL (SPACE start / graceful stop)
    // -------------------------
    bool pouring = false;   // "running" mode (spawns new instances)
    bool stopping = false;  // "graceful stop" mode (no new spawns, let active finish)

    // -------------------------
    // POUR ANIM (2 instances, overlap)
    // -------------------------
    const float pourSpeed = 0.4f;
    float t1 = 0.0f, t2 = 0.0f;
    bool p1Active = true;   // primary instance exists (time starts at 0 when running)
    bool p2Active = false;

    const float overlap = 0.5f; // start next before finish (increase => less chance of gaps)

    const float pourStartY = 0.0f;
    const float pourEndY = -1.2f;
    const glm::vec3 pourStartOffset(0.0f, pourStartY, 0.0f);
    const glm::vec3 pourEndOffset(0.0f, pourEndY, 0.0f);

    // -------------------------
    // ICE CREAM (reveal mask; grows only after pour reaches cup)
    // -------------------------
    float iceT = 0.0f;
    const float iceSpeed = 0.15f;

    const float cupContactY = -0.85f;
    float contactGrace = 0.0f;
    const float contactGraceTime = 0.5f;

    const glm::vec3 iceCreamPosOffset(0.0f, 0.0f, 0.0f);

    float lastTime = (float)glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        float now = (float)glfwGetTime();
        float dt = now - lastTime;
        lastTime = now;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // SPACE: start / graceful stop
        static bool spaceWasDown = false;
        bool spaceDown = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
        if (spaceDown && !spaceWasDown)
        {
            if (!pouring && !stopping)
            {
                // start
                pouring = true;
            }
            else if (pouring)
            {
                // request graceful stop: stop spawning new, but let current finish
                pouring = false;
                stopping = true;
            }
            else if (stopping)
            {
                // optional: re-start while stopping
                stopping = false;
                pouring = true;
            }
        }
        spaceWasDown = spaceDown;

        // R: reset everything
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        {
            pouring = false;
            stopping = false;

            t1 = 0.0f; t2 = 0.0f;
            p1Active = true; p2Active = false;

            iceT = 0.0f;
            contactGrace = 0.0f;
        }

        // -------------------------
        // UPDATE POUR (animate while pouring OR stopping)
        // During stopping: do NOT start new instances; only let active ones finish.
        // -------------------------
        if (pouring || stopping)
        {
            float step = dt * pourSpeed;

            if (p1Active) t1 += step;
            if (p2Active) t2 += step;

            // Start next instance ONLY while pouring (not during stopping)
            if (!stopping)
            {
                if (p1Active && !p2Active && t1 >= 1.0f - overlap)
                {
                    p2Active = true;
                    t2 = 0.0f;
                }

                if (p2Active && !p1Active && t2 >= 1.0f - overlap)
                {
                    p1Active = true;
                    t1 = 0.0f;
                }
            }

            // Finish instances (always)
            if (p1Active && t1 >= 1.0f)
            {
                p1Active = false;
                t1 = 0.0f;
            }

            if (p2Active && t2 >= 1.0f)
            {
                p2Active = false;
                t2 = 0.0f;
            }

            // If we're stopping and both instances are done -> fully stopped
            if (stopping && !p1Active && !p2Active)
            {
                stopping = false;

                // Prepare clean state for next start:
                t1 = 0.0f; t2 = 0.0f;
                p1Active = true;
                p2Active = false;

                contactGrace = 0.0f;
            }
        }

        // -------------------------
        // ICE CREAM GROWTH (based on contact)
        // -------------------------
        float tContact = (cupContactY - pourStartY) / (pourEndY - pourStartY);
        tContact = clamp01(tContact);

        bool contactNow = false;
        if (p1Active && clamp01(t1) > tContact) contactNow = true;
        if (p2Active && clamp01(t2) > tContact) contactNow = true;

        if (contactNow)
            contactGrace = contactGraceTime;
        else
            contactGrace = std::max(0.0f, contactGrace - dt);

        if ((pouring || stopping) && contactGrace > 0.0f)
        {
            iceT = std::min(1.0f, iceT + dt * iceSpeed);
        }

        // -------------------------
        // RENDER
        // -------------------------
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

        // Default: no progress mask
        unifiedShader.setInt("uUseProgressMask", 0);
        unifiedShader.setFloat("uProgress", 1.0f);

        // Base transform
        glm::mat4 base = glm::mat4(1.0f);
        base = glm::translate(base, glm::vec3(0.0f, -1.0f, 0.0f));
        base = glm::scale(base, glm::vec3(1.4f));

        unifiedShader.setMat4("uM", base);
        machine.Draw(unifiedShader);
        cup.Draw(unifiedShader);
        lever.Draw(unifiedShader);

        // Helper: draw one pour instance at time t (0..1)
        auto drawPourAt = [&](float t)
            {
                glm::vec3 off = glm::mix(pourStartOffset, pourEndOffset, clamp01(t));
                glm::mat4 m = base;
                m = glm::translate(m, off);
                unifiedShader.setMat4("uM", m);
                pour.Draw(unifiedShader);
            };

        if (p1Active) drawPourAt(t1);
        if (p2Active) drawPourAt(t2);

        // ICE CREAM: reveal mask using iceT
        unifiedShader.setInt("uUseProgressMask", 1);
        unifiedShader.setFloat("uProgress", iceT);

        glm::mat4 iceM = base;
        iceM = glm::translate(iceM, iceCreamPosOffset);

        unifiedShader.setMat4("uM", iceM);
        iceCream.Draw(unifiedShader);

        // Restore defaults
        unifiedShader.setInt("uUseProgressMask", 0);
        unifiedShader.setFloat("uProgress", 1.0f);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
