#ifndef LEVERSYSTEM_H
#define LEVERSYSTEM_H

#include <GL/glew.h>

// Lever System class
class LeverSystem {
private:
    // Lever positions (normalized: 0.0 = down, 1.0 = up)
    float leverPositionVanilla;
    float leverPositionMixed;
    float leverPositionChocolate;

    // Lever target states
    bool vanillaTarget;
    bool chocolateTarget;
    bool mixedTarget;

    // Constants
    const float leverSpeed = 2.0f;

public:
    // Texture IDs
    static unsigned leverVerticalTexture;
    static unsigned leverHorizontalTexture;

    // Constructor
    LeverSystem();

    // Update method
    void update(float deltaTime);

    // Control methods
    void setVanillaLever(bool active);
    void setChocolateLever(bool active);
    void setMixedLever(bool active);
    void resetLevers();

    // Getters for lever positions
    float getVanillaPosition() const { return leverPositionVanilla; }
    float getChocolatePosition() const { return leverPositionChocolate; }
    float getMixedPosition() const { return leverPositionMixed; }

    // Getters for lever states
    bool isVanillaActive() const { return vanillaTarget; }
    bool isChocolateActive() const { return chocolateTarget; }
    bool isMixedActive() const { return mixedTarget; }

    // Drawing method
    void drawLever(int type, unsigned int rectShader,
        unsigned int VAO_leverVertical,
        unsigned int VAO_leverHorizontal);

    // Individual lever drawing
    void drawVanillaLever(unsigned int rectShader,
        unsigned int VAO_leverVertical,
        unsigned int VAO_leverHorizontal);
    void drawChocolateLever(unsigned int rectShader,
        unsigned int VAO_leverVertical,
        unsigned int VAO_leverHorizontal);
    void drawMixedLever(unsigned int rectShader,
        unsigned int VAO_leverVertical,
        unsigned int VAO_leverHorizontal);

    // Draw all levers
    void drawAllLevers(unsigned int rectShader,
        unsigned int VAO_leverVertical,
        unsigned int VAO_leverHorizontal);
};

#endif // LEVERSYSTEM_H