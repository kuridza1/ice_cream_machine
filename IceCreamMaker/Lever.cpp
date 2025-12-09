#include "Lever.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Lever state variables
bool vanilla = false;
bool chocolate = false;
bool mixed = false;
float leverPositionVanilla = 1.0f;
float leverPositionMixed = 1.0f;
float leverPositionChocolate = 1.0f;

// Constants
const float leverSpeed = 2.0f;

void updateLevers(float deltaTime) {
    // Vanilla lever
    if (vanilla && leverPositionVanilla < 1.0f) {
        leverPositionVanilla += leverSpeed * deltaTime;
        if (leverPositionVanilla > 1.0f) leverPositionVanilla = 1.0f;
    }
    else if (!vanilla && leverPositionVanilla > 0.0f) {
        leverPositionVanilla -= leverSpeed * deltaTime;
        if (leverPositionVanilla < 0.0f) leverPositionVanilla = 0.0f;
    }

    // Chocolate lever
    if (chocolate && leverPositionChocolate < 1.0f) {
        leverPositionChocolate += leverSpeed * deltaTime;
        if (leverPositionChocolate > 1.0f) leverPositionChocolate = 1.0f;
    }
    else if (!chocolate && leverPositionChocolate > 0.0f) {
        leverPositionChocolate -= leverSpeed * deltaTime;
        if (leverPositionChocolate < 0.0f) leverPositionChocolate = 0.0f;
    }

    // Mixed lever
    if (mixed && leverPositionMixed < 1.0f) {
        leverPositionMixed += leverSpeed * deltaTime;
        if (leverPositionMixed > 1.0f) leverPositionMixed = 1.0f;
    }
    else if (!mixed && leverPositionMixed > 0.0f) {
        leverPositionMixed -= leverSpeed * deltaTime;
        if (leverPositionMixed < 0.0f) leverPositionMixed = 0.0f;
    }
}

void drawIceCreamLever(int type, float leverPosition, unsigned int rectShader,
    unsigned int VAO_leverVertical, unsigned int VAO_leverHorizontal,
    unsigned int leverVerticalTexture, unsigned int leverHorizontalTexture,
    float posX) {
    // Helper function to draw a rectangle (similar to your existing drawRect)
    auto drawRect = [&](unsigned int shader, unsigned int VAO, unsigned int textureID,
        float x, float y, float sx, float sy) {
            glUseProgram(shader);
            glUniform2f(glGetUniformLocation(shader, "uTranslation"), x, y);
            glUniform2f(glGetUniformLocation(shader, "uScale"), sx, sy);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glUniform1i(glGetUniformLocation(shader, "uTex"), 0);

            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        };

    // Calculate lever positions
    float verticalScaleY = 1.0f - leverPosition * 0.6f;
    float verticalPosY = (1.0f - verticalScaleY) * 0.5f;
    float horizontalPosY = leverPosition * 0.6f;

    // Draw lever parts
    drawRect(rectShader, VAO_leverVertical, leverVerticalTexture,
        0.0f, 0.0f, 1.0f, 1.0f);
    drawRect(rectShader, VAO_leverHorizontal, leverHorizontalTexture,
        posX, horizontalPosY, 1.0f, 1.0f);
}