#ifndef SPRINKLES_H
#define SPRINKLES_H

#include <vector>
#include <random>

struct Sprinkle {
    float x, y;
    float vx, vy;
    float size;
    float rotation;
    float rotationSpeed;
    bool active;
    float color[7];
    float slideTimer;
    bool isInTunnel;
    bool waitingToExit;
    float waitTimer;
    int collisionState;
};

// Declare extern for global variables
extern std::vector<Sprinkle> sprinkles;
extern bool sprinklesOpen;
extern std::mt19937 gen;

// Constants
extern const float GRAVITS;
extern const float DAMPING;
extern const float FRICTION;
extern const float FINAL_GROUND_Y;
extern const float TUNNEL_START_X;
extern const float TUNNEL_START_Y;
extern const float TUNNEL_END_X;
extern const float TUNNEL_END_Y;
extern const float SLIDE_SPEED;
extern const float EXIT_WAIT_TIME;

// Function declarations
void initSprinkles();
void spawnSprinkles();
void updateSprinklesPhysics(double deltaTime);
void drawSprinkles(const Sprinkle& drop, unsigned int shader, unsigned int VAO);

// Helper function
float getTunnelY(float x);

#endif