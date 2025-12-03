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
    bool isInTunnel;       // Currently in the tunnel
    bool waitingToExit;    // Waiting at tunnel exit
    float waitTimer;       // How long waiting at exit
    int collisionState;    // 0=falling, 1=in tunnel, 2=falling from exit, 3=on ice cream
};

// Add after the Sprinkle struct
struct IceCream {
    float x, y;           // Position (for falling stream)
    float size;
    float color[3];
    bool active;
    bool inCup;           // True when in the cup
    float cupX, cupY;     // Position in cup (spiral coordinates)
    float heightInCup;    // Height in the spiral
    float angle;          // Angle in spiral
    float rotation;       // Rotation for visual effect
};

// Add after sprinkles vector
std::vector<IceCream> iceCreams;
bool iceCreamFlow = false;
float iceCreamSpawnTimer = 0.0f;

// Ice cream constants
const float ICE_CREAM_PIPE_X = -0.15f;    // Pipe position
const float ICE_CREAM_PIPE_Y = 0.3f;
const float ICE_CREAM_CUP_X = 0.0f;       // Cup position (center)
const float ICE_CREAM_CUP_Y = -0.7f;
const float ICE_CREAM_CUP_RADIUS = 0.2f;  // Cup radius
const float ICE_CREAM_CUP_BOTTOM = -0.8f; // Bottom of cup
const float ICE_CREAM_SPIRAL_RADIUS = 0.1f;
const float ICE_CREAM_SPIRAL_SPEED = 2.0f; // How fast it spirals
const float ICE_CREAM_FALL_SPEED = 0.5f;
const float ICE_CREAM_SIZE = 0.02f;

// Spiral parameters
float currentSpiralAngle = 0.0f;
float currentSpiralHeight = ICE_CREAM_CUP_BOTTOM + 0.01f; // Start just above bottom

std::vector<Sprinkle> sprinkles;
bool leverVertical = false;
double lastUpdateTime = 0.0;
std::random_device rd;
std::mt19937 gen(rd());

// Physics constants
const float GRAVITY = -2.0f;
const float DAMPING = 0.3f;
const float FRICTION = 0.85f;
const float FINAL_GROUND_Y = -0.3f;    // Ice cream position

// Tunnel parameters (diagonal line from start to end)
const float TUNNEL_START_X = -0.36f;   // Where sprinkles enter
const float TUNNEL_START_Y = -0.02f;  // Start height
const float TUNNEL_END_X = 0.05f;      // Where they fall out (above ice cream)
const float TUNNEL_END_Y = -0.15f;     // End height

// Calculate tunnel slope
const float TUNNEL_SLOPE = (TUNNEL_END_Y - TUNNEL_START_Y) /
(TUNNEL_END_X - TUNNEL_START_X);

// Calculate expected Y position on tunnel for any X
float getTunnelY(float x) {
    return TUNNEL_START_Y + TUNNEL_SLOPE * (x - TUNNEL_START_X);
}

const float SLIDE_SPEED = 0.5f;  // How fast they slide down the tunnel
const float EXIT_WAIT_TIME = 0.5f; // Time between sprinkles exiting

int endProgram(std::string message) {
    std::cout << message << std::endl;
    glfwTerminate();
    return -1;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        leverVertical = !leverVertical;
    }
    if (key == GLFW_KEY_I && action == GLFW_PRESS) {  // 'I' for ice cream
        iceCreamFlow = !iceCreamFlow;
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

void spawnIceCream() {
    if (!iceCreamFlow) return;

    IceCream cream;

    // Start at the pipe
    cream.x = ICE_CREAM_PIPE_X;
    cream.y = ICE_CREAM_PIPE_Y;
    cream.size = ICE_CREAM_SIZE;
    cream.active = true;
    cream.inCup = false;
    cream.heightInCup = 0.0f;
    cream.angle = 0.0f;
    cream.rotation = 0.0f;

    // Vanilla ice cream color
    cream.color[0] = 1.0f;    // R
    cream.color[1] = 0.95f;   // G
    cream.color[2] = 0.85f;   // B

    iceCreams.push_back(cream);

    // Limit ice creams
    if (iceCreams.size() > 500) {
        iceCreams.erase(iceCreams.begin());
    }
}

void spawnSprinkles() {
    if (!leverVertical) return;

    Sprinkle sprinkle;

    sprinkle.x = TUNNEL_START_X;
    sprinkle.y = TUNNEL_START_Y + 0.1f;  // Slightly above entrance
    sprinkle.slideTimer = 0.0f;
    sprinkle.isInTunnel = false;
    sprinkle.waitingToExit = false;
    sprinkle.waitTimer = 0.0f;
    sprinkle.collisionState = 0;  // Start in falling state
    sprinkle.collisionState = 0;  // Start in falling state
    // Random velocities
    std::uniform_real_distribution<> disX(-0.07f, 0.07f);  // Less horizontal
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

void updateIceCreamPhysics(double deltaTime) {
    for (auto& cream : iceCreams) {
        if (!cream.active) continue;

        if (!cream.inCup) {
            // Falling from pipe to cup
            cream.y -= ICE_CREAM_FALL_SPEED * deltaTime;
            cream.rotation += 1.0f * deltaTime; // Gentle rotation

            // Check if reached cup
            float distanceToCup = sqrt(
                (cream.x - ICE_CREAM_CUP_X) * (cream.x - ICE_CREAM_CUP_X) +
                (cream.y - ICE_CREAM_CUP_Y) * (cream.y - ICE_CREAM_CUP_Y)
            );

            if (cream.y <= ICE_CREAM_CUP_Y || distanceToCup <= ICE_CREAM_CUP_RADIUS) {
                // Enter cup and start spiraling
                cream.inCup = true;
                cream.heightInCup = currentSpiralHeight;
                cream.angle = currentSpiralAngle;

                // Update cup position based on spiral
                cream.cupX = ICE_CREAM_CUP_X + ICE_CREAM_SPIRAL_RADIUS * cos(cream.angle);
                cream.cupY = cream.heightInCup;

                // Update spiral parameters for next ice cream
                currentSpiralAngle += ICE_CREAM_SPIRAL_SPEED * deltaTime;
                if (currentSpiralAngle > 2 * M_PI) {
                    currentSpiralAngle -= 2 * M_PI;
                    currentSpiralHeight += cream.size * 1.5f; // Increase height
                }
            }
        }
        else {
            // Already in cup - gentle movement in spiral
            cream.angle += 0.1f * deltaTime; // Slow rotation in cup
            cream.rotation += 0.5f * deltaTime;

            // Slight bobbing motion for realism
            cream.cupY = cream.heightInCup + 0.01f * sin(glfwGetTime() * 2.0f + cream.angle);
        }
    }

    // Remove inactive ice creams
    iceCreams.erase(
        std::remove_if(iceCreams.begin(), iceCreams.end(),
            [](const IceCream& c) { return !c.active; }),
        iceCreams.end()
    );
}

void updatePhysics(double deltaTime) {
    static float lastExitTime = 0.0f;
    static bool exitOccupied = false;  // Track if a sprinkle is at exit

    for (auto& drop : sprinkles) {
        if (!drop.active) continue;

        // Store previous position
        float prevX = drop.x - drop.vx * deltaTime;
        float prevY = drop.y - drop.vy * deltaTime;

        // Apply gravity only if not in tunnel or settled
        if (drop.collisionState == 0 || drop.collisionState == 2) {
            drop.vy += GRAVITY * deltaTime;
        }

        // Update position
        drop.x += drop.vx * deltaTime;
        drop.y += drop.vy * deltaTime;

        // Update rotation (only when falling)
        if (drop.collisionState == 0 || drop.collisionState == 2) {
            drop.rotation += drop.rotationSpeed * deltaTime;
        }

        // State machine for sprinkles
        switch (drop.collisionState) {

        case 0: // Initial falling
            // Check if sprinkle hits the tunnel entrance
            if (prevY > TUNNEL_START_Y && drop.y <= TUNNEL_START_Y + drop.size &&
                drop.x >= TUNNEL_START_X - drop.size && drop.x <= TUNNEL_START_X + drop.size) {

                // Place sprinkle at tunnel start
                drop.x = TUNNEL_START_X;
                drop.y = TUNNEL_START_Y + drop.size;
                drop.vx = 0.0f;
                drop.vy = 0.0f;
                drop.collisionState = 1; // Enter tunnel
                drop.isInTunnel = true;
                drop.slideTimer = 0.0f;

                // Only proceed if exit is not occupied
                if (!exitOccupied) {
                    drop.waitingToExit = false;
                }
                else {
                    drop.waitingToExit = true;
                    drop.waitTimer = 0.0f;
                }
            }
            // Check for ice cream collision (skip tunnel)
            else if (drop.y - drop.size <= FINAL_GROUND_Y) {
                drop.y = FINAL_GROUND_Y + drop.size;
                drop.vy = 0.0f;
                drop.vx = 0.0f;
                drop.rotationSpeed = 0.0f;
                drop.collisionState = 3; // On ice cream
            }
            break;

        case 1: // Sliding in tunnel
            drop.slideTimer += deltaTime;

            if (drop.waitingToExit) {
                // Wait at current position
                drop.waitTimer += deltaTime;

                // Check if exit is now free
                if (!exitOccupied && drop.waitTimer > 0.1f) {
                    drop.waitingToExit = false;
                    exitOccupied = true;
                }
            }
            else {
                // Move along tunnel towards exit
                if (drop.x < TUNNEL_END_X) {
                    drop.x += SLIDE_SPEED * deltaTime;
                    drop.y = getTunnelY(drop.x) + drop.size;

                    // Check if reached exit
                    if (drop.x >= TUNNEL_END_X) {
                        drop.x = TUNNEL_END_X;
                        drop.y = TUNNEL_END_Y + drop.size;

                        // Wait a bit at exit, then fall
                        drop.waitTimer += deltaTime;
                        if (drop.waitTimer > EXIT_WAIT_TIME) {
                            drop.collisionState = 2; // Start falling from exit
                            drop.isInTunnel = false;
                            exitOccupied = false; // Free the exit
                        }
                    }
                }
            }
            break;

        case 2: // Falling from tunnel exit to ice cream
            // Check for ice cream collision
            if (drop.y - drop.size <= FINAL_GROUND_Y) {
                drop.y = FINAL_GROUND_Y + drop.size;
                drop.vy = 0.0f;
                drop.vx = 0.0f;
                drop.rotationSpeed = 0.0f;
                drop.collisionState = 3; // On ice cream
            }
            break;

        case 3: // On ice cream - no movement
            drop.vx = 0.0f;
            drop.vy = 0.0f;
            drop.rotationSpeed = 0.0f;
            break;
        }

        // Side boundaries (only for falling sprinkles)
        if (drop.collisionState == 0 || drop.collisionState == 2) {
            if (drop.x - drop.size < -1.0f) {
                drop.x = -1.0f + drop.size;
                drop.vx = 0.0f;
            }
            if (drop.x + drop.size > 1.0f) {
                drop.x = 1.0f - drop.size;
                drop.vx = 0.0f;
            }
        }

        // Stop completely if velocities are very small
        if (fabs(drop.vy) < 0.01f) drop.vy = 0.0f;
        if (fabs(drop.vx) < 0.01f) drop.vx = 0.0f;
        if (fabs(drop.rotationSpeed) < 0.01f) drop.rotationSpeed = 0.0f;

        // Deactivate if below screen
        if (drop.y < -2.0f) {
            drop.active = false;
        }
    }

    // Clean up inactive sprinkles
    sprinkles.erase(
        std::remove_if(sprinkles.begin(), sprinkles.end(),
            [](const Sprinkle& d) { return !d.active; }),
        sprinkles.end()
    );
}

void drawIceCream(const IceCream& cream, unsigned int shader, unsigned int VAO,
    float screenWidth, float screenHeight) {
    glUseProgram(shader);

    // Calculate position
    float drawX, drawY;
    if (cream.inCup) {
        drawX = cream.cupX;
        drawY = cream.cupY;
    }
    else {
        drawX = cream.x;
        drawY = cream.y;
    }

    // Set uniforms
    GLint posLoc = glGetUniformLocation(shader, "uPosition");
    GLint sizeLoc = glGetUniformLocation(shader, "uSize");
    GLint colorLoc = glGetUniformLocation(shader, "uColor");
    GLint screenSizeLoc = glGetUniformLocation(shader, "uScreenSize");

    if (posLoc != -1) glUniform2f(posLoc, drawX, drawY);
    if (sizeLoc != -1) glUniform1f(sizeLoc, cream.size);
    if (colorLoc != -1) glUniform3f(colorLoc, cream.color[0], cream.color[1], cream.color[2]);
    if (screenSizeLoc != -1) glUniform2f(screenSizeLoc, screenWidth, screenHeight);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}
void drawCup(unsigned int shader, unsigned int VAO) {
    glUseProgram(shader);

    // Draw cup base (simple circle)
    int segments = 32;
    float radius = ICE_CREAM_CUP_RADIUS;

    // Draw multiple circles to create cup shape
    for (int i = 0; i < 3; i++) {
        float currentRadius = radius - i * 0.02f;
        float currentY = ICE_CREAM_CUP_Y - i * 0.01f;

        glBegin(GL_TRIANGLE_FAN);
        // Cup color (light brown)
        glColor3f(0.9f, 0.8f, 0.7f);
        glVertex2f(ICE_CREAM_CUP_X, currentY); // Center

        for (int j = 0; j <= segments; j++) {
            float angle = j * 2.0f * M_PI / segments;
            float x = ICE_CREAM_CUP_X + cos(angle) * currentRadius;
            float y = currentY + sin(angle) * currentRadius * 0.5f; // Elliptical cup
            glVertex2f(x, y);
        }
        glEnd();
    }
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

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Ice Cream Machine", monitor, NULL);

   
    //GLFWwindow* window = glfwCreateWindow(800, 800, "Vezba 2", NULL, NULL);
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

        // Draw cup (optional)
        drawCup(rectShader, VAO_machine); // You might need a different VAO for this

        // Update physics
        updatePhysics(deltaTime);

        // Update ice cream physics
        updateIceCreamPhysics(deltaTime);

        // Spawn sprinkles when lever is open
        static double lastSprinkleSpawnTime = 0.0;
        if (leverVertical && currentTime - lastSprinkleSpawnTime > 0.08f) {
            spawnSprinkles();
            lastSprinkleSpawnTime = currentTime;
        }

        // Spawn ice cream continuously when flow is on
        static double lastIceCreamSpawnTime = 0.0;
        if (iceCreamFlow && currentTime - lastIceCreamSpawnTime > 0.05f) {
            spawnIceCream();
            lastIceCreamSpawnTime = currentTime;
        }

        // Draw all sprinkles
        for (const auto& drop : sprinkles) {
            drawSprinkles(drop, particleShader, particleVAO, width, height);
        }

        // Draw all ice cream
        for (const auto& cream : iceCreams) {
            drawIceCream(cream, particleShader, particleVAO, width, height);
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