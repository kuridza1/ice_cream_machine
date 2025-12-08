#ifndef ICECREAM_H
#define ICECREAM_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>

// Ice Cream Drop structure
struct IceCreamDrop {
    float posY = 0.0f;      // Current Y position
    float velocity = 0.0f;  // Falling velocity
    float height = 0.1f;    // Height of each drop
    bool active = false;    // Whether this drop is active
    float lifeTime = 0.0f;  // How long the drop has existed
    int flavorType = 0;     // 1=vanilla, 2=chocolate, 3=mixed
};

// Cup Fill structure
struct CupFill {
    float fillLevel = -0.5f;  // Current fill level
    bool isFilled = false;    // Whether this flavor is in the cup
    bool isActive = false;    // Whether currently pouring
};

// Ice Cream System class
class IceCream {
private:
    // Drop management
    std::vector<IceCreamDrop> iceCreamDrops;

    // Cup fills for different flavors
    CupFill vanillaFill;
    CupFill chocolateFill;
    CupFill mixedFill;

    // Pour timers
    float timeSinceVanillaDrop = 0.0f;
    float timeSinceChocolateDrop = 0.0f;
    float timeSinceMixedDrop = 0.0f;

    // Pour states
    bool vanillaPourActive = false;
    bool chocolatePourActive = false;
    bool mixedPourActive = false;

    // Constants
    const float NOZZLE_POS_Y = 0.2f;
    const float CUP_TOP_POS_Y = -0.5f;
    const float CUP_BOTTOM_POS_Y = -0.5f;
    const float GRAVITY = 0.5f;
    const float DROP_SPAWN_RATE = 0.2f;
    const float CUP_FILL_WIDTH = 1.0f;

    // Helper methods
    void spawnIceCreamDrop(int flavorType);
    void updateDropPhysics(IceCreamDrop& drop, float deltaTime);
    void handleDropInCup(const IceCreamDrop& drop);

public:
    // Public pour states (for key callback)
    bool vanilla = false;
    bool chocolate = false;
    bool mixed = false;

    // Constructor
    IceCream();

    // Update and render
    void update(float deltaTime);
    void drawDrops(unsigned int rectShader, unsigned int VAO,
        unsigned int vanillaTexture, unsigned int chocolateTexture,
        unsigned int mixedTexture);
    void drawFilledCup(unsigned int rectShader, unsigned int VAO,
        unsigned int vanillaTexture, unsigned int chocolateTexture,
        unsigned int mixedTexture);

    // Control methods
    void toggleVanilla();
    void toggleChocolate();
    void toggleMixed();
    void resetCup();

    // Getters for fill levels (useful for visual effects)
    float getVanillaFillLevel() const { return vanillaFill.fillLevel; }
    float getChocolateFillLevel() const { return chocolateFill.fillLevel; }
    float getMixedFillLevel() const { return mixedFill.fillLevel; }

    // Getters for active states
    bool isVanillaActive() const { return vanillaPourActive; }
    bool isChocolateActive() const { return chocolatePourActive; }
    bool isMixedActive() const { return mixedPourActive; }

    // Texture IDs (you can also move these to main or keep them here)
    static unsigned vanillaPourTexture;
    static unsigned chocolatePourTexture;
    static unsigned mixedPourTexture;
    static unsigned iceCreamVanillaTexture;
    static unsigned iceCreamChocolateTexture;
    static unsigned iceCreamMixedTexture;
};

#endif // ICECREAM_H