#include "Sprinkles.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <random>

// Define global variables
std::vector<Sprinkle> sprinkles;
bool sprinklesOpen = false;
std::random_device rd;
std::mt19937 gen(rd());

// Constants
const float GRAVITYS = -5.0f;
const float DAMPING = 0.3f;
const float FRICTION = 0.85f;
const float FINAL_GROUND_Y = -0.76f;

// Tunnel coordinates - ADD NEW COORDINATES
const float SPRINKLE_NOZZLE_X = -0.17f;  // Where sprinkles spawn from container
const float SPRINKLE_NOZZLE_Y = 0.03f;    // Height of sprinkle nozzle
const float TUNNEL_ENTRANCE_X = -0.17f;  // Where sprinkles ENTER the tunnel
const float TUNNEL_ENTRANCE_Y = -0.05f;  // Height of tunnel entrance
const float TUNNEL_START_X = -0.21f;     // Start of SLOPE (same as entrance for now)
const float TUNNEL_START_Y = -0.05f;     // Start of SLOPE
const float TUNNEL_END_X = 0.16f;        // End of tunnel slope
const float TUNNEL_END_Y = -0.17f;       // End of tunnel slope

const float SLIDE_SPEED = 0.5f;
const float EXIT_WAIT_TIME = 0.5f;

// Tunnel slope
const float TUNNEL_SLOPE = (TUNNEL_END_Y - TUNNEL_START_Y) / (TUNNEL_END_X - TUNNEL_START_X);

// Ice cream data (extern - will be set from main.cpp)
extern bool vanillaFilled;
extern float vanillaLevel;
extern bool chocolateFilled;
extern float chocolateLevel;
extern bool mixedFilled;
extern float mixedLevel;
extern float cupBottomY;

void initSprinkles() {
    sprinkles.clear();
}

float getTunnelY(float x) {
    return TUNNEL_START_Y + TUNNEL_SLOPE * (x - TUNNEL_START_X);
}

void spawnSprinkles() {
    if (!sprinklesOpen) return;

    Sprinkle sprinkle;

    // Spawn from NOZZLE, not tunnel entrance
    sprinkle.x = SPRINKLE_NOZZLE_X;
    sprinkle.y = SPRINKLE_NOZZLE_Y;
    sprinkle.slideTimer = 0.0f;
    sprinkle.isInTunnel = false;
    sprinkle.waitingToExit = false;
    sprinkle.waitTimer = 0.0f;
    sprinkle.collisionState = 0; // Start falling from nozzle

    // Random velocities - give them MORE horizontal spread
    std::uniform_real_distribution<> disX(-0.08f, 0.08f); // Increased from -0.05 to -0.08
    std::uniform_real_distribution<> disY(-0.15f, -0.08f);
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

void updateSprinklesPhysics(double deltaTime) {
    static bool exitOccupied = false;

    for (auto& drop : sprinkles) {
        if (!drop.active) continue;

        // Store previous position
        float prevX = drop.x - drop.vx * deltaTime;
        float prevY = drop.y - drop.vy * deltaTime;

        // Apply gravity only if not in tunnel or settled
        if (drop.collisionState == 0) { // Only when falling from nozzle
            drop.vy += GRAVITYS * deltaTime;
        }

        // Update position
        drop.x += drop.vx * deltaTime;
        drop.y += drop.vy * deltaTime;

        // Update rotation (only when falling)
        if (drop.collisionState == 0) {
            drop.rotation += drop.rotationSpeed * deltaTime;
        }

        // State machine for sprinkles
        switch (drop.collisionState) {
        case 0: // Falling from nozzle to tunnel entrance
            // Check if sprinkle hits the tunnel entrance
            if (prevY > TUNNEL_ENTRANCE_Y && drop.y <= TUNNEL_ENTRANCE_Y + drop.size &&
                drop.x >= TUNNEL_ENTRANCE_X - 0.05f && drop.x <= TUNNEL_ENTRANCE_X + 0.05f) {

                // Place sprinkle at tunnel entrance
                drop.x = TUNNEL_ENTRANCE_X;
                drop.y = TUNNEL_ENTRANCE_Y + drop.size;
                drop.vx = 0.0f;
                drop.vy = 0.0f;
                drop.collisionState = 1; // Now in tunnel
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
            // If sprinkle misses tunnel and falls too low, deactivate it
            else if (drop.y < -1.0f) {
                drop.active = false;
            }
            break;

        case 1: // Sliding in tunnel (vertical drop before slope)
            drop.slideTimer += deltaTime;

            // First, drop vertically a bit before starting slope
            if (drop.y > TUNNEL_START_Y + drop.size) {
                // Still falling vertically to reach slope start
                drop.y -= 0.5f * deltaTime; // Controlled vertical drop

                // Check if reached slope start
                if (drop.y <= TUNNEL_START_Y + drop.size) {
                    drop.y = TUNNEL_START_Y + drop.size;
                }
            }
            else {
                // Now on the slope, handle waiting/exit logic
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
                    // Move along tunnel slope towards exit
                    if (drop.x < TUNNEL_END_X) {
                        drop.x += SLIDE_SPEED * deltaTime;
                        drop.y = getTunnelY(drop.x) + drop.size;

                        // Check if reached exit
                        if (drop.x >= TUNNEL_END_X) {
                            drop.x = TUNNEL_END_X;
                            drop.y = TUNNEL_END_Y + drop.size;

                            // Wait a bit at exit, then fall to ice cream
                            drop.waitTimer += deltaTime;
                            if (drop.waitTimer > EXIT_WAIT_TIME) {
                                drop.collisionState = 2; // Falling from exit
                                drop.isInTunnel = false;
                                exitOccupied = false;
                            }
                        }
                    }
                }
            }
            if (drop.x >= TUNNEL_END_X - 0.01f && drop.collisionState == 1) {
                drop.collisionState = 2;
                drop.isInTunnel = false;

                //random velocities to spread sprinkles across cup
                std::uniform_real_distribution<> disExitVX(-0.05f, 0.2f); 
                std::uniform_real_distribution<> disExitVY(-0.02f, 0.1f);

                drop.vx = disExitVX(gen);
                drop.vy = disExitVY(gen);
            }
            break;

        case 2: // Falling from tunnel exit to ice cream
        {
            // Apply gravity
            float highestIceCream = FINAL_GROUND_Y;

            drop.vy += GRAVITYS * deltaTime;
            drop.rotation += drop.rotationSpeed * deltaTime;

            // Update position
            drop.x += drop.vx * deltaTime;
            drop.y += drop.vy * deltaTime;

            // Calculate surface height at this position
            float surfaceHeight = FINAL_GROUND_Y; // Default to floor

            // Check if sprinkle is over the cup area (-0.5 to 0.5)
            if (drop.x > 0.18f && drop.x < 0.9f) {
                // Find the highest ice cream flavor

                if (vanillaFilled && vanillaLevel > highestIceCream) {
                    highestIceCream = vanillaLevel - 0.8;
                }
                if (chocolateFilled && chocolateLevel > highestIceCream) {
                    highestIceCream = chocolateLevel - 0.8;
                }
                if (mixedFilled && mixedLevel > highestIceCream) {
                    highestIceCream = mixedLevel - 0.8;
                }

                surfaceHeight = highestIceCream;
            }

            // Check for collision with surface (ice cream or floor)
            if (drop.y - drop.size <= surfaceHeight) {
                // Check if we're in the cup area
                bool isInCup = (drop.x > 0.18f && drop.x < 0.9f);

                if (isInCup && surfaceHeight > FINAL_GROUND_Y + 0.01f) {
                    static std::uniform_real_distribution<> heightDist(0.01f, abs(highestIceCream/2));
                    float heightVariation = heightDist(gen);

                    // Ensure we don't go below the base ice cream level
                    drop.y = std::max(surfaceHeight, surfaceHeight + heightVariation) + drop.size;
                }
                else {
                    // On floor or no ice cream - use exact surface
                    drop.y = surfaceHeight + drop.size;
                }

                drop.vy = 0.0f;
                drop.vx *= 0.4f;
                drop.rotationSpeed *= 0.5f;
                drop.collisionState = 3;
            }
        }
        break;

        case 3: // On surface (settled)
            drop.vx *= FRICTION; // Apply friction
            drop.rotationSpeed *= FRICTION;

            // Stop completely if velocities are very small
            if (fabs(drop.vx) < 0.01f) drop.vx = 0.0f;
            if (fabs(drop.rotationSpeed) < 0.01f) drop.rotationSpeed = 0.0f;
            break;
        }

        // Side boundaries only for falling sprinkles (state 0 or 2)
        if (drop.collisionState == 0 || drop.collisionState == 2) {
            if (drop.x - drop.size < -1.0f) {
                drop.x = -1.0f + drop.size;
                drop.vx = -drop.vx * DAMPING;
            }
            if (drop.x + drop.size > 1.0f) {
                drop.x = 1.0f - drop.size;
                drop.vx = -drop.vx * DAMPING;
            }
        }

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

void drawSprinkles(const Sprinkle& drop, unsigned int shader, unsigned int VAO) {
    glUseProgram(shader);

    // Set uniforms
    GLint posLoc = glGetUniformLocation(shader, "uPosition");
    GLint sizeLoc = glGetUniformLocation(shader, "uSize");
    GLint colorLoc = glGetUniformLocation(shader, "uColor");

    if (posLoc != -1) glUniform2f(posLoc, drop.x, drop.y);
    if (sizeLoc != -1) glUniform1f(sizeLoc, drop.size);
    if (colorLoc != -1) glUniform3f(colorLoc, drop.color[0], drop.color[1], drop.color[2]);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}
void resetSprinkles() {
    sprinkles.clear();
}