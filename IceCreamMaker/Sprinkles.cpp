#include "Sprinkles.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <random>

// Define global variables
std::vector<Sprinkle> sprinkles;
bool leverVertical = false;
std::random_device rd;
std::mt19937 gen(rd());

// Constants
const float GRAVITY = -2.0f;
const float DAMPING = 0.3f;
const float FRICTION = 0.85f;
const float FINAL_GROUND_Y = -0.3f;
const float TUNNEL_START_X = -0.36f;
const float TUNNEL_START_Y = -0.02f;
const float TUNNEL_END_X = 0.05f;
const float TUNNEL_END_Y = -0.15f;
const float SLIDE_SPEED = 0.5f;
const float EXIT_WAIT_TIME = 0.5f;

// Tunnel slope
const float TUNNEL_SLOPE = (TUNNEL_END_Y - TUNNEL_START_Y) / (TUNNEL_END_X - TUNNEL_START_X);

void initSprinkles() {
    sprinkles.clear();
}

float getTunnelY(float x) {
    return TUNNEL_START_Y + TUNNEL_SLOPE * (x - TUNNEL_START_X);
}

void spawnSprinkles() {
    if (!leverVertical) return;

    Sprinkle sprinkle;

    sprinkle.x = TUNNEL_START_X;
    sprinkle.y = TUNNEL_START_Y + 0.1f;
    sprinkle.slideTimer = 0.0f;
    sprinkle.isInTunnel = false;
    sprinkle.waitingToExit = false;
    sprinkle.waitTimer = 0.0f;
    sprinkle.collisionState = 0;

    // Random velocities
    std::uniform_real_distribution<> disX(-0.07f, 0.07f);
    std::uniform_real_distribution<> disY(-0.1f, -0.1f);
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
                drop.collisionState = 1;
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
                drop.collisionState = 3;
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
                            drop.collisionState = 2;
                            drop.isInTunnel = false;
                            exitOccupied = false;
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
                drop.collisionState = 3;
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