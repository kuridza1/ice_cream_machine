#include "IceCream.h"
#include <algorithm>
#include "Util.h"

// Initialize static texture variables
unsigned IceCream::vanillaPourTexture = 0;
unsigned IceCream::chocolatePourTexture = 0;
unsigned IceCream::mixedPourTexture = 0;
unsigned IceCream::iceCreamVanillaTexture = 0;
unsigned IceCream::iceCreamChocolateTexture = 0;
unsigned IceCream::iceCreamMixedTexture = 0;

IceCream::IceCream() {
    // Initialize fill levels
    vanillaFill.fillLevel = CUP_BOTTOM_POS_Y;
    chocolateFill.fillLevel = CUP_BOTTOM_POS_Y;
    mixedFill.fillLevel = CUP_BOTTOM_POS_Y;
}

void IceCream::spawnIceCreamDrop(int flavorType) {
    IceCreamDrop drop;
    drop.posY = NOZZLE_POS_Y;
    drop.velocity = 0.0f;
    drop.height = 1.0f;
    drop.active = true;
    drop.lifeTime = 0.0f;
    drop.flavorType = flavorType;
    iceCreamDrops.push_back(drop);
}

void IceCream::updateDropPhysics(IceCreamDrop& drop, float deltaTime) {
    drop.lifeTime += deltaTime;

    // Apply gravity
    drop.velocity -= GRAVITY * deltaTime;
    drop.posY += drop.velocity * deltaTime;

    // Check if drop reached cup
    if (drop.posY <= CUP_TOP_POS_Y) {
        handleDropInCup(drop);
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

void IceCream::handleDropInCup(const IceCreamDrop& drop) {
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
}

void IceCream::update(float deltaTime) {
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
            updateDropPhysics(drop, deltaTime);
        }
    }

    // Clean up inactive drops
    iceCreamDrops.erase(
        std::remove_if(iceCreamDrops.begin(), iceCreamDrops.end(),
            [](const IceCreamDrop& drop) { return !drop.active; }),
        iceCreamDrops.end()
    );
}

void IceCream::drawDrops(unsigned int rectShader, unsigned int VAO,
    unsigned int vanillaTexture, unsigned int chocolateTexture,
    unsigned int mixedTexture) {
    for (const auto& drop : iceCreamDrops) {
        if (drop.active && drop.posY > CUP_TOP_POS_Y) {
            unsigned textureID = 0;
            switch (drop.flavorType) {
            case 1: textureID = vanillaTexture; break;
            case 2: textureID = chocolateTexture; break;
            case 3: textureID = mixedTexture; break;
            }

            // Simple rectangle drawing - you might want to use your existing drawRect function
            glUseProgram(rectShader);

            // Set uniforms
            glUniform2f(glGetUniformLocation(rectShader, "uTranslation"), 0.0f, drop.posY);
            glUniform2f(glGetUniformLocation(rectShader, "uScale"), 1.0f, drop.height);

            // Bind texture
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glUniform1i(glGetUniformLocation(rectShader, "uTex"), 0);

            // Draw
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        }
    }
}

void IceCream::drawFilledCup(unsigned int rectShader, unsigned int VAO,
    unsigned int vanillaTexture, unsigned int chocolateTexture,
    unsigned int mixedTexture) {
    // Draw vanilla fill
    if (vanillaFill.isFilled && vanillaFill.fillLevel > CUP_BOTTOM_POS_Y) {
        float fillHeight = vanillaFill.fillLevel - CUP_BOTTOM_POS_Y;
        float fillPosY = CUP_BOTTOM_POS_Y + (fillHeight / 2.0f);

        glUseProgram(rectShader);
        glUniform2f(glGetUniformLocation(rectShader, "uTranslation"), 0.0f, fillPosY);
        glUniform2f(glGetUniformLocation(rectShader, "uScale"), CUP_FILL_WIDTH, fillHeight);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, vanillaTexture);
        glUniform1i(glGetUniformLocation(rectShader, "uTex"), 0);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    // Draw chocolate fill
    if (chocolateFill.isFilled && chocolateFill.fillLevel > CUP_BOTTOM_POS_Y) {
        float fillHeight = chocolateFill.fillLevel - CUP_BOTTOM_POS_Y;
        float fillPosY = CUP_BOTTOM_POS_Y + (fillHeight / 2.0f);

        glUseProgram(rectShader);
        glUniform2f(glGetUniformLocation(rectShader, "uTranslation"), 0.0f, fillPosY);
        glUniform2f(glGetUniformLocation(rectShader, "uScale"), CUP_FILL_WIDTH, fillHeight);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, chocolateTexture);
        glUniform1i(glGetUniformLocation(rectShader, "uTex"), 0);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    // Draw mixed fill
    if (mixedFill.isFilled && mixedFill.fillLevel > CUP_BOTTOM_POS_Y) {
        float fillHeight = mixedFill.fillLevel - CUP_BOTTOM_POS_Y;
        float fillPosY = CUP_BOTTOM_POS_Y + (fillHeight / 2.0f);

        glUseProgram(rectShader);
        glUniform2f(glGetUniformLocation(rectShader, "uTranslation"), 0.0f, fillPosY);
        glUniform2f(glGetUniformLocation(rectShader, "uScale"), CUP_FILL_WIDTH, fillHeight);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mixedTexture);
        glUniform1i(glGetUniformLocation(rectShader, "uTex"), 0);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
}

void IceCream::toggleVanilla() {
    vanilla = !vanilla;
    vanillaPourActive = vanilla;
    vanillaFill.isActive = vanilla;

    if (vanilla) {
        timeSinceVanillaDrop = DROP_SPAWN_RATE;
        vanillaFill.isFilled = true;
    }
}

void IceCream::toggleChocolate() {
    chocolate = !chocolate;
    chocolatePourActive = chocolate;
    chocolateFill.isActive = chocolate;

    if (chocolate) {
        iceCreamDrops.clear();
        timeSinceChocolateDrop = 0.0f;
        chocolateFill.isFilled = true;
    }
}

void IceCream::toggleMixed() {
    mixed = !mixed;
    mixedPourActive = mixed;
    mixedFill.isActive = mixed;

    if (mixed) {
        iceCreamDrops.clear();
        timeSinceMixedDrop = 0.0f;
        mixedFill.isFilled = true;
    }
}

void IceCream::resetCup() {
    vanillaFill.fillLevel = CUP_BOTTOM_POS_Y;
    vanillaFill.isFilled = false;
    chocolateFill.fillLevel = CUP_BOTTOM_POS_Y;
    chocolateFill.isFilled = false;
    mixedFill.fillLevel = CUP_BOTTOM_POS_Y;
    mixedFill.isFilled = false;

    // Reset pour states
    vanilla = false;
    vanillaPourActive = false;
    chocolate = false;
    chocolatePourActive = false;
    mixed = false;
    mixedPourActive = false;

    // Clear existing drops
    iceCreamDrops.clear();

    // Reset timers
    timeSinceVanillaDrop = 0.0f;
    timeSinceChocolateDrop = 0.0f;
    timeSinceMixedDrop = 0.0f;
}