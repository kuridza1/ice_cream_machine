#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>
#include <iostream>
#include <vector>
#include <random>
#include <iterator>  // ADD THIS LINE
#include "Util.h"

unsigned machineTexture;
unsigned sprinklesOpenTexture;
unsigned sprinklesCloseTexture;

struct Sprinkle {
    float x, y;
    float vx, vy;
    float size;
    float rotation;
    float rotationSpeed;
    bool active;
    float color[7];
    float slideTimer;      // Timer for sliding phase
    bool isSliding;        // Currently in sliding phase
    bool hasFallenOff;     // Has fallen off the shelf
    int collisionState;    // 0=falling, 1=on shelf, 2=on final ground
};

std::vector<Sprinkle> sprinkles;
bool leverVertical = false;
double lastUpdateTime = 0.0;
std::random_device rd;
std::mt19937 gen(rd());

// Physics constants
const float GRAVITY = -2.0f;           // Slightly stronger gravity
const float DAMPING = 0.3f;            // Less bounce
const float FRICTION = 1.0f;          // Slower sliding
const float SHELF_Y = -0.065f;           // Higher shelf position
const float FINAL_GROUND_Y = -0.3f;    // Ice cream position
const float SLIDE_DURATION = 0.0004f;     // How long to slide before falling
const float SHELF_START_X = -0.36f;        // Width of the shelf
const float SHELF_END_X = 0.0;

int endProgram(std::string message) {
    std::cout << message << std::endl;
    glfwTerminate();
    return -1;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        leverVertical = !leverVertical;
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

void drawRect(unsigned int rectShader, unsigned int VAOrect, unsigned int textureID) {
    glUseProgram(rectShader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glUniform1i(glGetUniformLocation(rectShader, "uTex"), 0);
    glBindVertexArray(VAOrect);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void spawnSprinkles() {
    if (!leverVertical) return;

    Sprinkle sprinkle;

    sprinkle.x = -0.35f;      
    sprinkle.y = 0.08f;    
    sprinkle.slideTimer = 0.0f;
    sprinkle.isSliding = false;
    sprinkle.hasFallenOff = false;
    sprinkle.collisionState = 0;  // Start in falling state
    // Random velocities
    std::uniform_real_distribution<> disX(-0.05f, 0.05f);  // Less horizontal
    std::uniform_real_distribution<> disY(-0.1f, -0.1f);  // More downward
    sprinkle.vx = disX(gen);
    sprinkle.vy = disY(gen);

    // Random size
    std::uniform_real_distribution<> disSize(0.01f, 0.02f);
    sprinkle.size = disSize(gen);

    // Random rotation
    sprinkle.rotation = 0.0f;
    std::uniform_real_distribution<> disRotSpeed(-1.0f, 1.0f);
    sprinkle.rotationSpeed = disRotSpeed(gen);

    // Random color
    std::uniform_real_distribution<> disColor(0.0f, 1.0f);
    std::uniform_int_distribution<> disFlavor(0, 6);

    int flavor = disFlavor(gen);
    switch (flavor) {
    case 0:  // Blue
        sprinkle.color[0] = 0.09f;
        sprinkle.color[1] = 0.09f;
        sprinkle.color[2] = 0.639f;
        break;
    case 1:  // Red
        sprinkle.color[0] = 1.0f;
        sprinkle.color[1] = 0.0f;
        sprinkle.color[2] = 0.0f;
        break;
    case 2:  // Green
        sprinkle.color[0] = 0.0f;
        sprinkle.color[1] = 0.73f;
        sprinkle.color[2] = 0.0f;
        break;
    case 3:  // Yellow
        sprinkle.color[0] = 1.0f;
        sprinkle.color[1] = 0.9f;
        sprinkle.color[2] = 0.0f;
        break;
    case 4:  // Pink
        sprinkle.color[0] = 1.0f;
        sprinkle.color[1] = 0.75f;
        sprinkle.color[2] = 0.80f;
        break;
    case 5:  // Chocolate
        sprinkle.color[0] = 0.30f;
        sprinkle.color[1] = 0.192f;
        sprinkle.color[2] = 0.078f;
        break;
    case 6:  // Vanilla
        sprinkle.color[0] = 1.0f;
        sprinkle.color[1] = 0.94f;
        sprinkle.color[2] = 0.86f;
        break;
    }



    sprinkle.active = true;
    sprinkles.push_back(sprinkle);

    // Limit sprinkles
    if (sprinkles.size() > 300) {
        sprinkles.erase(sprinkles.begin());
    }
}

void updatePhysics(double deltaTime) {
    for (auto& drop : sprinkles) {
        if (!drop.active) continue;

        // Store previous position for collision detection
        float prevY = drop.y - drop.vy * deltaTime;  // Position before update

        // Apply gravity only if not settled
        if (drop.collisionState < 2) {
            drop.vy += GRAVITY * deltaTime;
        }

        // Update position
        drop.x += drop.vx * deltaTime;
        drop.y += drop.vy * deltaTime;

        // Update rotation
        if (drop.collisionState < 2) {
            drop.rotation += drop.rotationSpeed * deltaTime;
        }

        // Check collisions based on current state
        if (drop.collisionState == 0) {
            // Check if we passed through the shelf this frame
            // The sprinkle was above the shelf on previous frame and is now below/at it
            bool wasAboveShelf = (prevY - drop.size) > SHELF_Y;
            bool isAtOrBelowShelf = (drop.y - drop.size) <= SHELF_Y;
            bool isWithinShelfWidth = drop.x > SHELF_START_X && drop.x < SHELF_END_X;

            if (wasAboveShelf && isAtOrBelowShelf && isWithinShelfWidth) {
                // Land on the shelf
                drop.y = SHELF_Y + drop.size;  // Place on top of shelf
                drop.vy = 0.0f;  // No bounce
                drop.vx *= 0.8f; // Reduce horizontal velocity
                drop.collisionState = 1; // Now on shelf
                drop.isSliding = true;
                drop.slideTimer = 0.0f;

                // Reduce rotation
                drop.rotationSpeed *= 0.5f;

                std::cout << "Sprinkle landed on shelf at y=" << drop.y << std::endl;
            }
            // Check for final ground collision (similar logic)
            else if ((prevY - drop.size) > FINAL_GROUND_Y &&
                (drop.y - drop.size) <= FINAL_GROUND_Y) {
                drop.y = FINAL_GROUND_Y + drop.size;
                drop.vy = 0.0f;
                drop.vx = 0.0f;  // Stop completely
                drop.rotationSpeed = 0.0f;
                drop.collisionState = 2; // Final resting place
                drop.isSliding = false;

                std::cout << "Sprinkle landed on ground at y=" << drop.y << std::endl;
            }
        }
        else if (drop.collisionState == 1) {
            // On the shelf - sliding phase
            drop.slideTimer += deltaTime;

            // Apply shelf friction
            drop.vx *= FRICTION;

            // Boundary collision on shelf
            if (drop.x - drop.size < SHELF_START_X) {
                drop.x = SHELF_START_X + drop.size;
                drop.vx = 0.0f;  // Stop at edge
            }
            else if (drop.x + drop.size > SHELF_END_X) {
                drop.x = SHELF_END_X - drop.size;
                drop.vx = 0.0f;  // Stop at edge
            }

            // After sliding period, fall off shelf
            if (drop.slideTimer >= SLIDE_DURATION) {
                drop.vx = 0.1f; // Small push outward
                
                drop.collisionState = 0; // Back to falling
                drop.isSliding = false;
                drop.hasFallenOff = true;
            }
        }
        else if (drop.collisionState == 2) {
            // On final ground - no movement
            drop.vx = 0.0f;
            drop.vy = 0.0f;
            drop.rotationSpeed = 0.0f;
        }

        // Side boundaries (walls)
        if (drop.x - drop.size < -1.0f) {
            drop.x = -1.0f + drop.size;
            drop.vx = 0.0f;  // No bounce, just stop
        }
        if (drop.x + drop.size > 1.0f) {
            drop.x = 1.0f - drop.size;
            drop.vx = 0.0f;  // No bounce, just stop
        }

        // Stop completely if velocities are very small
        if (fabs(drop.vy) < 0.01f) drop.vy = 0.0f;
        if (fabs(drop.vx) < 0.01f) drop.vx = 0.0f;
        if (fabs(drop.rotationSpeed) < 0.01f) drop.rotationSpeed = 0.0f;

        // Deactivate if below screen (safety)
        if (drop.y < -2.0f) {
            drop.active = false;
        }
    }

    // Remove inactive sprinkles
    sprinkles.erase(
        std::remove_if(sprinkles.begin(), sprinkles.end(),
            [](const Sprinkle& d) { return !d.active; }),
        sprinkles.end()
    );
}

void drawSprinkles(const Sprinkle& drop, unsigned int shader, unsigned int VAO,
    float screenWidth, float screenHeight) {
    glUseProgram(shader);

    // Set uniforms
    GLint posLoc = glGetUniformLocation(shader, "uPosition");
    GLint sizeLoc = glGetUniformLocation(shader, "uSize");
    GLint colorLoc = glGetUniformLocation(shader, "uColor");
    GLint screenSizeLoc = glGetUniformLocation(shader, "uScreenSize");

    if (posLoc != -1) glUniform2f(posLoc, drop.x, drop.y);
    if (sizeLoc != -1) glUniform1f(sizeLoc, drop.size);
    if (colorLoc != -1) glUniform3f(colorLoc, drop.color[0], drop.color[1], drop.color[2]);
    if (screenSizeLoc != -1) glUniform2f(screenSizeLoc, screenWidth, screenHeight);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}


int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /*GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Ice Cream Machine", monitor, NULL);

   */
    GLFWwindow* window = glfwCreateWindow(800, 800, "Vezba 2", NULL, NULL);
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    if (!window) return endProgram("Failed to create window");

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);

    if (glewInit() != GLEW_OK) return endProgram("GLEW failed to initialize");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Load textures
    preprocessTexture(machineTexture, "res/machine.png");
    preprocessTexture(sprinklesCloseTexture, "res/sprinklesClose.png");
    preprocessTexture(sprinklesOpenTexture, "res/sprinklesOpen.png");

    // Create shaders
    unsigned int rectShader = createShader("rect.vert", "rect.frag");
    if (rectShader == 0) return endProgram("Failed to create rectangle shader");

    // Create particle shader
    unsigned int particleShader = createShader("particle.vert", "particle.frag");
    if (particleShader == 0) {
        std::cout << "Failed to create particle shader" << std::endl;
        return endProgram("Failed to create particle shader");
    }

    // Create VAOs
    unsigned int VAO_machine, VAO_lever;

    // Full screen machine
    float machineRect[] = {
        -1.0f,  1.0f,   0.0f, 1.0f,
        -1.0f, -1.0f,   0.0f, 0.0f,
         1.0f, -1.0f,   1.0f, 0.0f,
         1.0f,  1.0f,   1.0f, 1.0f
    };

    // Smaller lever rectangle (adjust position based on your image)
    float leverRect[] = {
        -1.0f,  1.0f,   0.0f, 1.0f,
        -1.0f, -1.0f,   0.0f, 0.0f,
         1.0f, -1.0f,   1.0f, 0.0f,
         1.0f,  1.0f,   1.0f, 1.0f
    };

    formVAOs(machineRect, sizeof(machineRect), VAO_machine);
    formVAOs(leverRect, sizeof(leverRect), VAO_lever);

    // Create particle VAO
    unsigned int particleVAO, particleVBO;
    float particleVertices[] = {
        -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.0f, 1.0f
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

        glClear(GL_COLOR_BUFFER_BIT);

        // Draw machine
        drawRect(rectShader, VAO_machine, machineTexture);

        // Draw lever
        if (leverVertical) {
            drawRect(rectShader, VAO_lever, sprinklesOpenTexture);
        }
        else {
            drawRect(rectShader, VAO_lever, sprinklesCloseTexture);
        }

        // Update physics
        updatePhysics(deltaTime);

        // Spawn drops when lever is open
        static double lastSpawnTime = 0.0;
        if (leverVertical && currentTime - lastSpawnTime > 0.08f) {
            spawnSprinkles();
            lastSpawnTime = currentTime;
        }

        // Draw all ice cream drops
        for (const auto& drop : sprinkles) {
            drawSprinkles(drop, particleShader, particleVAO, height, width);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(rectShader);
    glDeleteProgram(particleShader);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}