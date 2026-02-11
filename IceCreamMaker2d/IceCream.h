#ifndef ICE_CREAM_H
#define ICE_CREAM_H

#include <vector>

struct IceCreamDrop {
    float posY = 0.0f;
    float velocity = 0.0f;
    float height = 0.1f;
    bool active = false;
    float lifeTime = 0.0f;
    int flavorType = 0; // 1=vanilla, 2=chocolate, 3=mixed
};

struct CupFill {
    float fillLevel = 0.0f;
    bool isFilled = false;
    bool isActive = false;
};

// Global variables
extern std::vector<IceCreamDrop> iceCreamDrops;
extern CupFill vanillaFill;
extern CupFill chocolateFill;
extern CupFill mixedFill;
extern bool vanillaPourActive;
extern bool chocolatePourActive;
extern bool mixedPourActive;

// Constants
extern const float NOZZLE_POS_Y;
extern const float CUP_TOP_POS_Y;
extern const float CUP_BOTTOM_POS_Y;
extern const float GRAVITY;
extern const float DROP_SPAWN_RATE;
extern const float DROP_WIDTH;
extern const float CUP_FILL_WIDTH;

// Function declarations
void initIceCream();
void resetCup();
void spawnIceCreamDrop(int flavorType);
void updateIceCreamDrops(float deltaTime);
void handleIceCreamKeyPress(int key, int action);

#endif