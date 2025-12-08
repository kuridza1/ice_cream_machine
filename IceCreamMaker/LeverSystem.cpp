#include "LeverSystem.h"
#include "Util.h"  // For drawRect function if needed

// Initialize static texture variables
unsigned LeverSystem::leverVerticalTexture = 0;
unsigned LeverSystem::leverHorizontalTexture = 0;

LeverSystem::LeverSystem()
    : leverPositionVanilla(1.0f),  // Start in up position
    leverPositionMixed(1.0f),
    leverPositionChocolate(1.0f),
    vanillaTarget(false),
    chocolateTarget(false),
    mixedTarget(false) {
}

void LeverSystem::update(float deltaTime) {
    // Update vanilla lever
    if (vanillaTarget && leverPositionVanilla < 1.0f) {
        leverPositionVanilla += leverSpeed * deltaTime;
        if (leverPositionVanilla > 1.0f) leverPositionVanilla = 1.0f;
    }
    else if (!vanillaTarget && leverPositionVanilla > 0.0f) {
        leverPositionVanilla -= leverSpeed * deltaTime;
        if (leverPositionVanilla < 0.0f) leverPositionVanilla = 0.0f;
    }

    // Update chocolate lever
    if (chocolateTarget && leverPositionChocolate < 1.0f) {
        leverPositionChocolate += leverSpeed * deltaTime;
        if (leverPositionChocolate > 1.0f) leverPositionChocolate = 1.0f;
    }
    else if (!chocolateTarget && leverPositionChocolate > 0.0f) {
        leverPositionChocolate -= leverSpeed * deltaTime;
        if (leverPositionChocolate < 0.0f) leverPositionChocolate = 0.0f;
    }

    // Update mixed lever
    if (mixedTarget && leverPositionMixed < 1.0f) {
        leverPositionMixed += leverSpeed * deltaTime;
        if (leverPositionMixed > 1.0f) leverPositionMixed = 1.0f;
    }
    else if (!mixedTarget && leverPositionMixed > 0.0f) {
        leverPositionMixed -= leverSpeed * deltaTime;
        if (leverPositionMixed < 0.0f) leverPositionMixed = 0.0f;
    }
}

void LeverSystem::setVanillaLever(bool active) {
    vanillaTarget = active;
}

void LeverSystem::setChocolateLever(bool active) {
    chocolateTarget = active;
}

void LeverSystem::setMixedLever(bool active) {
    mixedTarget = active;
}

void LeverSystem::resetLevers() {
    vanillaTarget = false;
    chocolateTarget = false;
    mixedTarget = false;
}

// This function replaces the old iceCreamLever function
void LeverSystem::drawLever(int type, unsigned int rectShader,
    unsigned int VAO_leverVertical,
    unsigned int VAO_leverHorizontal) {

    // Get the current lever position based on type
    float leverPosition = 0.0f;
    float positionX = 0.0f;

    switch (type) {
    case 1: // Vanilla
        leverPosition = leverPositionVanilla;
        positionX = 0.0f;
        break;
    case 2: // Mixed
        leverPosition = leverPositionMixed;
        positionX = 0.16f;
        break;
    case 3: // Chocolate
        leverPosition = leverPositionChocolate;
        positionX = 0.31f;
        break;
    default:
        return; // Invalid lever type
    }

    // Calculate lever transformations
    float verticalScaleY = 1.0f - leverPosition * 0.7f;
    float verticalPosY = (1.0f - verticalScaleY) * 0.5f;
    float horizontalPosY = leverPosition * -0.23f;

    // Draw vertical part of lever
    glUseProgram(rectShader);
    glUniform2f(glGetUniformLocation(rectShader, "uTranslation"), positionX, verticalPosY);
    glUniform2f(glGetUniformLocation(rectShader, "uScale"), 1.0f, verticalScaleY);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, leverVerticalTexture);
    glUniform1i(glGetUniformLocation(rectShader, "uTex"), 0);

    glBindVertexArray(VAO_leverVertical);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    // Draw horizontal part (handle)
    glUseProgram(rectShader);
    glUniform2f(glGetUniformLocation(rectShader, "uTranslation"), positionX, horizontalPosY);
    glUniform2f(glGetUniformLocation(rectShader, "uScale"), 1.0f, 1.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, leverHorizontalTexture);
    glUniform1i(glGetUniformLocation(rectShader, "uTex"), 0);

    glBindVertexArray(VAO_leverHorizontal);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void LeverSystem::drawVanillaLever(unsigned int rectShader,
    unsigned int VAO_leverVertical,
    unsigned int VAO_leverHorizontal) {
    drawLever(1, rectShader, VAO_leverVertical, VAO_leverHorizontal);
}

void LeverSystem::drawChocolateLever(unsigned int rectShader,
    unsigned int VAO_leverVertical,
    unsigned int VAO_leverHorizontal) {
    drawLever(3, rectShader, VAO_leverVertical, VAO_leverHorizontal);
}

void LeverSystem::drawMixedLever(unsigned int rectShader,
    unsigned int VAO_leverVertical,
    unsigned int VAO_leverHorizontal) {
    drawLever(2, rectShader, VAO_leverVertical, VAO_leverHorizontal);
}

void LeverSystem::drawAllLevers(unsigned int rectShader,
    unsigned int VAO_leverVertical,
    unsigned int VAO_leverHorizontal) {
    drawVanillaLever(rectShader, VAO_leverVertical, VAO_leverHorizontal);
    drawMixedLever(rectShader, VAO_leverVertical, VAO_leverHorizontal);
    drawChocolateLever(rectShader, VAO_leverVertical, VAO_leverHorizontal);
}