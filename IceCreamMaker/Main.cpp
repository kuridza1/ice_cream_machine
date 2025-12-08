#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include "Util.h"
#include "Sprinkles.h"

// Texture IDs
unsigned machineTexture;
unsigned leverVerticalTexture;
unsigned leverHorizontalTexture;
unsigned sprinklesOpenTexture;
unsigned sprinklesCloseTexture;
unsigned iceCreamVanillaTexture;
unsigned vanillaPourTexture;
unsigned cupFrontTexture;
unsigned cupBackTexture;

// Lever state variables
bool vanilla = false;
bool mixed = false;
bool chocolate = false;
float leverPositionVanilla = 1.0f;
float leverPositionMixed = 1.0f;
float leverPositionChocolate = 1.0f;

const float leverSpeed = 2.0f;

// Cup filling variables
struct IceCreamDrop {
    float posY = 0.0f;      // Current Y position (starts at nozzle)
    float velocity = 0.0f;  // Falling velocity
    float height = 0.1f;    // Height of each drop
    bool active = false;    // Whether this drop is active
    float lifeTime = 0.0f;  // How long the drop has existed
};

std::vector<IceCreamDrop> iceCreamDrops;
const float NOZZLE_POS_Y = 0.2f;     // Where ice cream comes out (adjust as needed)
const float CUP_TOP_POS_Y = -0.5f;   // Where cup opening is (adjust as needed)
const float CUP_BOTTOM_POS_Y = -0.5f; // Where cup bottom is (adjust as needed)
float cupFillLevel = CUP_BOTTOM_POS_Y; // Current fill level in cup
const float GRAVITY = 0.5f;          // Gravity strength (adjust for faster/slower falling)
const float DROP_SPAWN_RATE = 0.2f; // Time between drops (smaller = more frequent)
const float DROP_WIDTH = 1.0f;      // Width of falling drops
const float CUP_FILL_WIDTH = 1.0f;   // Width of filled area in cup
float timeSinceLastDrop = 0.0f;
bool vanillaPourActive = false;

// Global variables
double lastUpdateTime = 0.0;

int endProgram(std::string message) {
    std::cout << message << std::endl;
    glfwTerminate();
    return -1;
}

void spawnIceCreamDrop() {
    IceCreamDrop drop;
    drop.posY = NOZZLE_POS_Y;
    drop.velocity = 0.0f;
    drop.height = 1.0f; 
    drop.active = true;
    drop.lifeTime = 0.0f;
    iceCreamDrops.push_back(drop);
}

void updateIceCreamDrops(float deltaTime) {
    // Spawn new drops if pouring is active
    if (vanillaPourActive) {
        timeSinceLastDrop += deltaTime;
        if (timeSinceLastDrop >= DROP_SPAWN_RATE) {
            spawnIceCreamDrop();
            timeSinceLastDrop = 0.0f;
        }
    }

    // Update existing drops with gravity
    for (auto& drop : iceCreamDrops) {
        if (drop.active) {
            drop.lifeTime += deltaTime;

            // Apply gravity
            drop.velocity -= GRAVITY * deltaTime;
            drop.posY += drop.velocity * deltaTime;

            // Check if drop reached cup
            if (drop.posY <= CUP_TOP_POS_Y) {
                // Drop entered the cup - add to fill level
                cupFillLevel += 0.02f; // Adjust fill rate as needed

                // Remove or deactivate the drop
                drop.active = false;

                // Clamp fill level to cup top
                if (cupFillLevel > CUP_TOP_POS_Y + 0.8f) {
                    cupFillLevel = CUP_TOP_POS_Y + 0.8f;
                }
            }

            // Remove drops that fall below screen
            if (drop.posY < -2.0f) {
                drop.active = false;
            }

            // Remove old drops (safety)
            if (drop.lifeTime > 5.0f) {
                drop.active = false;
            }
        }
    }

    // Clean up inactive drops
    iceCreamDrops.erase(
        std::remove_if(iceCreamDrops.begin(), iceCreamDrops.end(),
            [](const IceCreamDrop& drop) { return !drop.active; }),
        iceCreamDrops.end()
    );
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        sprinklesOpen = !sprinklesOpen;
    }
    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        vanilla = !vanilla;
        vanillaPourActive = vanilla;

        // Reset cup fill when starting
        if (vanilla) {
            cupFillLevel = CUP_BOTTOM_POS_Y;
            iceCreamDrops.clear();
            timeSinceLastDrop = 0.0f;
        }
    }
    if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        chocolate = !chocolate;
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        mixed = !mixed;
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
    preprocessTexture(vanillaPourTexture, "res/vanillaPour.png");
    preprocessTexture(iceCreamVanillaTexture, "res/iceCreamVanilla.png");
    preprocessTexture(cupFrontTexture, "res/cupFront.png");
    preprocessTexture(cupBackTexture, "res/cupBack.png");

    // Create shaders
    unsigned int rectShader = createShader("rect.vert", "rect.frag");
    if (rectShader == 0) return endProgram("Failed to create rectangle shader");

    unsigned int particleShader = createShader("particle.vert", "particle.frag");
    if (particleShader == 0) return endProgram("Failed to create particle shader");

    unsigned int VAO_machine, VAO_leverVertical, VAO_leverHorizontal, VAO_sprinklesLever, VAO_iceCreamVanilla, VAO_cup;

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
    formVAOs(machineRect, sizeof(machineRect), VAO_cup);

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

        // Update ice cream drops with gravity
        updateIceCreamDrops(deltaTime);

        glClear(GL_COLOR_BUFFER_BIT);

        // 1. Draw the cup first (background)
        drawRect(rectShader, VAO_cup, cupBackTexture, 0.0f, 0.0f, 1.0f, 1.0f);




        // 3. Draw falling ice cream drops (above the cup fill)
        for (const auto& drop : iceCreamDrops) {
            if (drop.active && drop.posY > CUP_TOP_POS_Y) {
                drawRect(rectShader, VAO_iceCreamVanilla, vanillaPourTexture,
                    0.0f, drop.posY, DROP_WIDTH, drop.height);
            }
        }

        // 4. Draw the machine (on top of everything)
        drawRect(rectShader, VAO_machine, machineTexture, 0.0f, 0.0f, 1.0f, 1.0f);
        // 2. Draw the filled ice cream in the cup
        float fillHeight = cupFillLevel - CUP_BOTTOM_POS_Y;
        if (fillHeight > 0.0f) {
            // Calculate position: bottom of cup + half the fill height
            float fillPosY = CUP_BOTTOM_POS_Y + (fillHeight / 2.0f);
            drawRect(rectShader, VAO_iceCreamVanilla, iceCreamVanillaTexture,
                0.0f, fillPosY, CUP_FILL_WIDTH, fillHeight);
        }
        drawRect(rectShader, VAO_cup, cupFrontTexture, 0.0f, 0.0f, 1.0f, 1.0f);

        // 5. Draw levers (on top of machine)
        iceCreamLever(1, leverPositionVanilla, rectShader, VAO_leverVertical, VAO_leverHorizontal);
        iceCreamLever(2, leverPositionMixed, rectShader, VAO_leverVertical, VAO_leverHorizontal);
        iceCreamLever(3, leverPositionChocolate, rectShader, VAO_leverVertical, VAO_leverHorizontal);

        // 6. Draw sprinkles lever
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