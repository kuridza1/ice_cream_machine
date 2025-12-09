#include "IceCream.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>

// Global variables
std::vector<IceCreamDrop> iceCreamDrops;
CupFill vanillaFill;
CupFill chocolateFill;
CupFill mixedFill;
bool vanillaPourActive = false;
bool chocolatePourActive = false;
bool mixedPourActive = false;

// Constants
const float NOZZLE_POS_Y = 0.2f;
const float CUP_TOP_POS_Y = -0.5f;
const float CUP_BOTTOM_POS_Y = -0.54f;
const float GRAVITY = 0.5f;
const float DROP_SPAWN_RATE = 0.2f;
const float DROP_WIDTH = 1.0f;
const float CUP_FILL_WIDTH = 1.0f;

// Local timers
static float timeSinceVanillaDrop = 0.0f;
static float timeSinceChocolateDrop = 0.0f;
static float timeSinceMixedDrop = 0.0f;

void initIceCream() {
    iceCreamDrops.clear();
    vanillaFill.fillLevel = CUP_BOTTOM_POS_Y;
    vanillaFill.isFilled = false;
    vanillaFill.isActive = false;
    chocolateFill.fillLevel = CUP_BOTTOM_POS_Y;
    chocolateFill.isFilled = false;
    chocolateFill.isActive = false;
    mixedFill.fillLevel = CUP_BOTTOM_POS_Y;
    mixedFill.isFilled = false;
    mixedFill.isActive = false;

    timeSinceVanillaDrop = 0.0f;
    timeSinceChocolateDrop = 0.0f;
    timeSinceMixedDrop = 0.0f;
}

void resetCup() {
    initIceCream(); // Reset everything
}

void spawnIceCreamDrop(int flavorType) {
    IceCreamDrop drop;
    drop.posY = NOZZLE_POS_Y;
    drop.velocity = 0.0f;
    drop.height = 1.0f;
    drop.active = true;
    drop.lifeTime = 0.0f;
    drop.flavorType = flavorType;
    iceCreamDrops.push_back(drop);
}

void updateIceCreamDrops(float deltaTime) {
    // Update separate timers for each flavor
    if (vanillaPourActive) {
        timeSinceVanillaDrop += deltaTime;
        while (timeSinceVanillaDrop >= DROP_SPAWN_RATE) {
            spawnIceCreamDrop(1); // Vanilla
            timeSinceVanillaDrop -= DROP_SPAWN_RATE;
        }
    }

    if (chocolatePourActive) {
        timeSinceChocolateDrop += deltaTime;
        while (timeSinceChocolateDrop >= DROP_SPAWN_RATE) {
            spawnIceCreamDrop(2); // Chocolate
            timeSinceChocolateDrop -= DROP_SPAWN_RATE;
        }
    }

    if (mixedPourActive) {
        timeSinceMixedDrop += deltaTime;
        while (timeSinceMixedDrop >= DROP_SPAWN_RATE) {
            spawnIceCreamDrop(3); // Mixed
            timeSinceMixedDrop -= DROP_SPAWN_RATE;
        }
    }

    // Update all active drops
    for (auto& drop : iceCreamDrops) {
        if (drop.active) {
            drop.lifeTime += deltaTime;

            // Apply gravity
            drop.velocity -= GRAVITY * deltaTime;
            drop.posY += drop.velocity * deltaTime;

            // Check if drop reached cup
            if (drop.posY <= CUP_TOP_POS_Y) {
                // Add to appropriate fill level
                switch (drop.flavorType) {
                case 1: // Vanilla
                    vanillaFill.fillLevel += 0.02f;
                    if (vanillaFill.fillLevel > CUP_TOP_POS_Y + 0.8f) {
                        vanillaFill.fillLevel = CUP_TOP_POS_Y + 0.8f;
                    }
                    break;
                case 2: // Chocolate
                    chocolateFill.fillLevel += 0.02f;
                    if (chocolateFill.fillLevel > CUP_TOP_POS_Y + 0.8f) {
                        chocolateFill.fillLevel = CUP_TOP_POS_Y + 0.8f;
                    }
                    break;
                case 3: // Mixed
                    mixedFill.fillLevel += 0.02f;
                    if (mixedFill.fillLevel > CUP_TOP_POS_Y + 0.8f) {
                        mixedFill.fillLevel = CUP_TOP_POS_Y + 0.8f;
                    }
                    break;
                }

                // Deactivate this drop
                drop.active = false;
            }

            // Remove drops that fall too low
            if (drop.posY < -2.0f) {
                drop.active = false;
            }

            // Safety cleanup
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

void handleIceCreamKeyPress(int key, int action) {
    if (action == GLFW_PRESS) {
        switch (key) {
        case GLFW_KEY_1:
            vanillaPourActive = !vanillaPourActive;
            vanillaFill.isActive = vanillaPourActive;
            vanillaFill.isFilled = true;
            break;
        case GLFW_KEY_2:
            chocolatePourActive = !chocolatePourActive;
            chocolateFill.isActive = chocolatePourActive;
            chocolateFill.isFilled = true;
            break;
        case GLFW_KEY_SPACE:
            mixedPourActive = !mixedPourActive;
            mixedFill.isActive = mixedPourActive;
            mixedFill.isFilled = true;
            break;
        }
    }
}