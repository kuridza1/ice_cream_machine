#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Util.h"
#include "Sprinkles.h"

// Texture IDs
unsigned machineTexture;
unsigned leverVerticalTexture;
unsigned leverHorizontalTexture;
unsigned sprinklesOpenTexture;
unsigned sprinklesCloseTexture;
unsigned iceCreamVanillaTexture;

// Lever state variables
bool vanilla = false;
bool mixed = false;
bool chocolate = false;
float leverPositionVanilla = 1.0f;
float leverPositionMixed = 1.0f;
float leverPositionChocolate = 1.0f;

const float leverSpeed = 2.0f; 


float vanillaHeight = 0.0f;
float chocolateHeight = 0.0f;
float mixedHeight = 0.0f;
float vanillaPosY = 0.0f;
float chocolatePosY = 0.0f;
float mixedPosY = 0.0f;
bool vanillaSettling = false;
bool chocolateSettling = false;
bool mixedSettling = false;


// Global variables
double lastUpdateTime = 0.0;

int endProgram(std::string message) {
    std::cout << message << std::endl;
    glfwTerminate();
    return -1;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        sprinklesOpen = !sprinklesOpen;
    }
    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        vanilla = !vanilla;
        if (!vanilla) {
            vanillaSettling = true;
        }
    }
    if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        chocolate = !chocolate;
        if (!chocolate) {
            chocolateSettling = true;
        }
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        mixed = !mixed;
        if (!mixed) {
            mixedSettling = true;
        }
    }


}

void preprocessTexture(unsigned& texture, const char* filepath) {
    texture = loadImageToTexture(filepath);
    glBindTexture(GL_TEXTURE_2D, texture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void formVAOs(float* verticesRect, size_t rectSize, unsigned int& VAOrect) {
    unsigned int VBOrect;
    glGenVertexArrays(1, &VAOrect);
    glGenBuffers(1, &VBOrect);

    glBindVertexArray(VAOrect);
    glBindBuffer(GL_ARRAY_BUFFER, VBOrect);
    glBufferData(GL_ARRAY_BUFFER, rectSize, verticesRect, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void drawRect(unsigned int rectShader, unsigned int VAOrect, unsigned int textureID,
    float posX = 0.0f, float posY = 0.0f, float scaleX = 1.0f, float scaleY = 1.0f) {
    glUseProgram(rectShader);

    // Set uniforms
    glUniform2f(glGetUniformLocation(rectShader, "uTranslation"), posX, posY);
    glUniform2f(glGetUniformLocation(rectShader, "uScale"), scaleX, scaleY);

    // Bind texture and draw
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glUniform1i(glGetUniformLocation(rectShader, "uTex"), 0);

    glBindVertexArray(VAOrect);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void iceCreamLever(int type, float leverPosition, unsigned int rectShader, unsigned int VAO_leverVertical,
    unsigned int VAO_leverHorizontal) {

    float verticalScaleY = 1.0f - leverPosition * 0.7f;
    float verticalPosY = (1.0f - verticalScaleY) * 0.5f;
    float horizontalPosY = leverPosition * -0.23f;
    float positionX =0.0f;
    switch (type) {
    case 1:
        positionX = 0.0f;
        break;
    case 2:        
        positionX = 0.16f;
        break;
    case 3:
        positionX = 0.31f;
        break;
    }
    drawRect(rectShader, VAO_leverVertical, leverVerticalTexture, positionX, verticalPosY, 1.0f, verticalScaleY);
    drawRect(rectShader, VAO_leverHorizontal, leverHorizontalTexture, positionX, horizontalPosY, 1.0f, 1.0f);
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Ice Cream Machine", monitor, NULL);

    if (!window) return endProgram("Failed to create window");

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);

    if (glewInit() != GLEW_OK) return endProgram("GLEW failed to initialize");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Initialize systems
    initSprinkles();

    // Load textures
    preprocessTexture(machineTexture, "res/machine.png");
    preprocessTexture(leverVerticalTexture, "res/lever.png");
    preprocessTexture(leverHorizontalTexture, "res/handle.png"); 
    preprocessTexture(sprinklesCloseTexture, "res/sprinklesClose.png");
    preprocessTexture(sprinklesOpenTexture, "res/sprinklesOpen.png");
    preprocessTexture(iceCreamVanillaTexture, "res/iceCreamVanilla.png");

    // Create shaders
    unsigned int rectShader = createShader("rect.vert", "rect.frag");
    if (rectShader == 0) return endProgram("Failed to create rectangle shader");

    unsigned int particleShader = createShader("particle.vert", "particle.frag");
    if (particleShader == 0) return endProgram("Failed to create particle shader");

    unsigned int VAO_machine, VAO_leverVertical, VAO_leverHorizontal, VAO_sprinklesLever, VAO_iceCreamVanilla;

    float machineRect[] = {
        -1.0f,  1.0f,   0.0f, 1.0f,
        -1.0f, -1.0f,   0.0f, 0.0f,
         1.0f, -1.0f,   1.0f, 0.0f,
         1.0f,  1.0f,   1.0f, 1.0f
    };

    float leverVerticalRect[] = {
        -1.0f,  1.0f,   0.0f, 1.0f,
        -1.0f, -1.0f,   0.0f, 0.0f,
         1.0f, -1.0f,   1.0f, 0.0f,
         1.0f,  1.0f,   1.0f, 1.0f
    };

    float leverHorizontalRect[] = {
        -1.0f,  1.0f,   0.0f, 1.0f,
        -1.0f, -1.0f,   0.0f, 0.0f,
         1.0f, -1.0f,   1.0f, 0.0f,
         1.0f,  1.0f,   1.0f, 1.0f
    };

    float sprinklesLeverRect[] = {
        -1.0f,  1.0f,   0.0f, 1.0f,
        -1.0f, -1.0f,   0.0f, 0.0f,
         1.0f, -1.0f,   1.0f, 0.0f,
         1.0f,  1.0f,   1.0f, 1.0f
        };

    formVAOs(machineRect, sizeof(machineRect), VAO_machine);
    formVAOs(leverVerticalRect, sizeof(leverVerticalRect), VAO_leverVertical);
    formVAOs(leverHorizontalRect, sizeof(leverHorizontalRect), VAO_leverHorizontal);
    formVAOs(sprinklesLeverRect, sizeof(sprinklesLeverRect), VAO_sprinklesLever);
    formVAOs(machineRect, sizeof(machineRect), VAO_iceCreamVanilla);

    unsigned int particleVAO, particleVBO;
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    float aspect = (float)width / height;

    float particleVertices[] = {
        -0.5f / aspect, -0.5f,  0.0f, 0.0f,
         0.5f / aspect, -0.5f,  1.0f, 0.0f,
         0.5f / aspect,  0.5f,  1.0f, 1.0f,
        -0.5f / aspect,  0.5f,  0.0f, 1.0f
    };

    glGenVertexArrays(1, &particleVAO);
    glGenBuffers(1, &particleVBO);
    glBindVertexArray(particleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(particleVertices), particleVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    glClearColor(0.5f, 0.6f, 1.0f, 1.0f);
    lastUpdateTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        double deltaTime = currentTime - lastUpdateTime;
        lastUpdateTime = currentTime;
        if (vanilla && leverPositionVanilla < 1.0f) {
            leverPositionVanilla += leverSpeed * deltaTime;
            if (leverPositionVanilla > 1.0f) leverPositionVanilla = 1.0f;
        }
        else if (!vanilla && leverPositionVanilla > 0.0f) {
            leverPositionVanilla -= leverSpeed * deltaTime;
            if (leverPositionVanilla < 0.0f) leverPositionVanilla = 0.0f;
        }
        if (chocolate && leverPositionChocolate < 1.0f) {
            leverPositionChocolate += leverSpeed * deltaTime;
            if (leverPositionChocolate > 1.0f) leverPositionChocolate = 1.0f;
        }
        else if (!chocolate && leverPositionChocolate > 0.0f) {
            leverPositionChocolate -= leverSpeed * deltaTime;
            if (leverPositionChocolate < 0.0f) leverPositionChocolate = 0.0f;
        }
        if (mixed && leverPositionMixed < 1.0f) {
            leverPositionMixed += leverSpeed * deltaTime;
            if (leverPositionMixed > 1.0f) leverPositionMixed = 1.0f;
        }
        else if (!mixed && leverPositionMixed > 0.0f) {
            leverPositionMixed -= leverSpeed * deltaTime;
            if (leverPositionMixed < 0.0f) leverPositionMixed = 0.0f;
        }

        
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw machine (full screen)
        drawRect(rectShader, VAO_machine, machineTexture, 0.0f, 0.0f, 1.0f, 1.0f);
        iceCreamLever(1, leverPositionVanilla, rectShader, VAO_leverVertical, VAO_leverHorizontal);
        iceCreamLever(2, leverPositionMixed, rectShader, VAO_leverVertical, VAO_leverHorizontal);
        iceCreamLever(3, leverPositionChocolate, rectShader, VAO_leverVertical, VAO_leverHorizontal);

        if (vanilla) {
            vanillaHeight += 0.5f * deltaTime;
            if (vanillaHeight > 1.0f) vanillaHeight = 1.0f;
        }
        else if (vanillaSettling) {
            vanillaPosY -= 2.0f * deltaTime;
            if (vanillaPosY < -1.5f) {
                vanillaSettling = false;
                vanillaHeight = 0.0f;
                vanillaPosY = 0.0f;
            }
        }
        else {
            vanillaHeight = 0.0f;
        }
        if (chocolate) {
            chocolateHeight += 0.5f * deltaTime;
            if (chocolateHeight > 1.0f) chocolateHeight = 1.0f;
        }
        else if (chocolateSettling) {
            chocolatePosY -= 2.0f * deltaTime;
            if (chocolatePosY < -1.5f) {
                chocolateSettling = false;
                chocolateHeight = 0.0f;
                chocolatePosY = 0.0f;
            }
        }
        else {
            chocolateHeight = 0.0f;
        }
        if (mixed) {
            mixedHeight += 0.5f * deltaTime;
            if (mixedHeight > 1.0f) mixedHeight = 1.0f;
        }
        else if (mixedSettling) {
            mixedPosY -= 2.0f * deltaTime;
            if (mixedPosY < -1.5f) {
                mixedSettling = false;
                mixedHeight = 0.0f;
                mixedPosY = 0.0f;
            }
        }
        else {
            mixedHeight = 0.0f;
        }
        if (vanillaHeight > 0.0f || vanillaSettling) {
            float drawY = vanillaSettling ? vanillaPosY : (0.8f - vanillaHeight * 0.5f);
            drawRect(rectShader, VAO_iceCreamVanilla, iceCreamVanillaTexture, -0.3f, drawY, 1.0f, vanillaHeight);
        }


        if (sprinklesOpen) {
            drawRect(rectShader, VAO_sprinklesLever, sprinklesOpenTexture, 0.0f, 0.0f, 1.0f, 1.0f);
        }
        else {
            drawRect(rectShader, VAO_sprinklesLever, sprinklesCloseTexture, 0.0f, 0.0f, 1.0f, 1.0f);
        }

        // Update physics
        updateSprinklesPhysics(deltaTime);

        // Spawn sprinkles when lever is down
        static double lastSprinkleSpawnTime = 0.0;
        if (sprinklesOpen > 0.5f && currentTime - lastSprinkleSpawnTime > 0.08f) {
            spawnSprinkles();
            lastSprinkleSpawnTime = currentTime;
        }

        // Draw all sprinkles
        for (const auto& drop : sprinkles) {
            drawSprinkles(drop, particleShader, particleVAO);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(rectShader);
    glDeleteProgram(particleShader);
    glDeleteVertexArrays(1, &VAO_machine);
    glDeleteVertexArrays(1, &VAO_leverVertical);
    glDeleteVertexArrays(1, &VAO_leverHorizontal);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}