#ifndef LEVER_H
#define LEVER_H

// Lever state variables
extern bool vanilla;
extern bool chocolate;
extern bool mixed;
extern float leverPositionVanilla;
extern float leverPositionMixed;
extern float leverPositionChocolate;

// Constants
extern const float leverSpeed;

// Function declarations
void updateLevers(float deltaTime);
void drawIceCreamLever(int type, float leverPosition, unsigned int rectShader,
    unsigned int VAO_leverVertical, unsigned int VAO_leverHorizontal);

#endif