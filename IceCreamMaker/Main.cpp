#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
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

// Ice cream layer structure
struct IceCreamLayer {
    float height = 0.0f;
    float posY = 0.0f;
    bool isActive = false;
    bool isSettling = false;
    float startPosY = 0.0f; // Starting Y position for this layer
};

// Store multiple ice cream layers
std::vector<IceCreamLayer> vanillaLayers;
std::vector<IceCreamLayer> chocolateLayers;
std::vector<IceCreamLayer> mixedLayers;

// Current pouring states
bool vanillaPouring = false;
bool chocolatePouring = false;
bool mixedPouring = false;
bool vanillaPourActive = false;
float vanillaPourHeight = 0.0f;
float vanillaPourSpeed = 0.5f; // Adjust speed as needed
float currentVanillaPourHeight = 0.0f;
float currentVanillaPourPosY = 0.0f;

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
        vanillaPourActive = vanilla;

        // Reset pour height when starting
        if (vanilla) {
            vanillaPourHeight = 0.0f;
        }
    }
    if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        chocolate = !chocolate;
        chocolatePouring = chocolate;
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        mixed = !mixed;
        mixedPouring = mixed;
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
    float positionX = 0.0f;
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
    preprocessTexture(iceCreamVanillaTexture, "res/vanillaPour.png");

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

    glClearColor(0.392156862745098f, 0.4470588235294118f, 0.4901960784313725f, 1.0f);
    lastUpdateTime = glfwGetTime();

    const float MAX_POUR_HEIGHT = 0.8f; // Max height before creating a new layer
    const float POUR_SPEED = 0.5f;
    const float SETTLE_SPEED = 2.0f;
    const float CUP_BOTTOM = -0.5f; // Where settled layers accumulate

    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        double deltaTime = currentTime - lastUpdateTime;
        lastUpdateTime = currentTime;

        // Update lever positions
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

        if (vanillaPourActive) {
            vanillaPourHeight += vanillaPourSpeed * deltaTime;
            // Keep growing indefinitely while active
        }

        glClear(GL_COLOR_BUFFER_BIT);

        if (vanillaPourHeight > 0.0f) {
            drawRect(rectShader, VAO_iceCreamVanilla, iceCreamVanillaTexture,
                0.0f, 0.5f, 1.0f, vanillaPourHeight);
        }
        // Draw machine (full screen)
        drawRect(rectShader, VAO_machine, machineTexture, 0.0f, 0.0f, 1.0f, 1.0f);
        iceCreamLever(1, leverPositionVanilla, rectShader, VAO_leverVertical, VAO_leverHorizontal);
        iceCreamLever(2, leverPositionMixed, rectShader, VAO_leverVertical, VAO_leverHorizontal);
        iceCreamLever(3, leverPositionChocolate, rectShader, VAO_leverVertical, VAO_leverHorizontal);

        // Draw all settled vanilla layers (from bottom to top for proper ordering)
       

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
        if (sprinklesOpen && currentTime - lastSprinkleSpawnTime > 0.08f) {
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