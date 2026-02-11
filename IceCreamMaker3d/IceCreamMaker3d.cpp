//Opis: Primjer ucitavanja modela upotrebom ASSIMP biblioteke
//Preuzeto sa learnOpenGL

#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>

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
    bool pouring = false;   // running (spawns new pours)
    bool stopping = false;  // graceful stop (no new spawns, let active finish)

    // -------------------------
    // POUR ANIM (2 instances, overlap)
    // -------------------------
    const float pourSpeed = 0.4f;
    float t1 = 0.0f, t2 = 0.0f;
    bool p1Active = true;
    bool p2Active = false;

    const float overlap = 0.5f;

    const float pourStartY = 0.0f;
    const float pourEndY = -1.2f;
    const glm::vec3 pourStartOffset(0.0f, pourStartY, 0.0f);
    const glm::vec3 pourEndOffset(0.0f, pourEndY, 0.0f);

    // -------------------------
    // ICE CREAM (reveal mask; grows only after pour reaches cup)
    // -------------------------
    float iceT = 0.0f;
    const float iceSpeed = 0.07f;

    const float cupContactY = -0.85f;
    float contactGrace = 0.0f;
    const float contactGraceTime = 0.5f;

    // Offset sladoleda u odnosu na čašu (u koordinatama čaše)
    const glm::vec3 iceCreamPosOffset(0.0f, 0.0f, 0.0f);

    // -------------------------
    // CUP ROTATION while pouring
    // -------------------------
    const float cupSpinSpeed = glm::radians(80.0f); // menjaj po potrebi

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
                pouring = true;
            }
            else if (pouring)
            {
                pouring = false;
                stopping = true;
            }
            else if (stopping)
            {
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
        // -------------------------
        if (pouring || stopping)
        {
            float step = dt * pourSpeed;

            if (p1Active) t1 += step;
            if (p2Active) t2 += step;

            // Start next ONLY while pouring (not during stopping)
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

            // If stopping and both done -> fully stopped
            if (stopping && !p1Active && !p2Active)
            {
                stopping = false;

                // Clean state for next start
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

        // MACHINE
        unifiedShader.setMat4("uM", base);
        machine.Draw(unifiedShader);

        // LEVER
        unifiedShader.setMat4("uM", base);
        lever.Draw(unifiedShader);

        // CUP transform (računamo jednom i koristimo i za sladoled)
        glm::mat4 cupM = base;
        if (pouring || stopping)
        {
            float angle = now * cupSpinSpeed;
            cupM = glm::rotate(cupM, angle, glm::vec3(0.0f, 1.0f, 0.0f));
        }

        unifiedShader.setMat4("uM", cupM);
        cup.Draw(unifiedShader);

        // POUR instances
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

        // ICE CREAM: da se pomera/rotira zajedno sa čašom -> koristi cupM kao parent
        unifiedShader.setInt("uUseProgressMask", 1);
        unifiedShader.setFloat("uProgress", iceT);

        glm::mat4 iceM = cupM; // KLJUČ: prati čašu
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
