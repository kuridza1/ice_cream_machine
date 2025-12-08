#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Util.h"
#include "Sprinkles.h"
#include "IceCream.h"
#include "Lever.h"

// Texture IDs (keep as before)
unsigned machineTexture;
unsigned leverVerticalTexture;
unsigned leverHorizontalTexture;
unsigned sprinklesOpenTexture;
unsigned sprinklesCloseTexture;
unsigned iceCreamVanillaTexture;
unsigned iceCreamChocolateTexture;
unsigned iceCreamMixedTexture;
unsigned vanillaPourTexture;
unsigned chocolatePourTexture;
unsigned mixedPourTexture;
unsigned cupFrontTexture;
unsigned cupBackTexture;
unsigned spoonTexture;
unsigned circularTexture;
unsigned nameTexture;

float spoonX = 0.0f, spoonY = 0.0f;
float spoonSize = 0.1f;
bool mousePressed = false;

// Bite marks - now only store positions, we'll draw them differently
struct BiteMark {
    float x, y;
    float size;
};
std::vector<BiteMark> biteMarks;

unsigned int VAO_machine, VAO_leverVertical, VAO_leverHorizontal;
unsigned int VAO_sprinklesLever, VAO_iceCreamVanilla, VAO_cup, VAO_spoon;

// Global variables
double lastUpdateTime = 0.0;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        sprinklesOpen = !sprinklesOpen;
    }
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE); // Close the window
        return;
    }
    // Handle ice cream key presses
    handleIceCreamKeyPress(key, action);
    
    // Update lever states (for animation)
    if (action == GLFW_PRESS) {
        switch (key) {
        case GLFW_KEY_1:
            vanilla = !vanilla;
            break;
        case GLFW_KEY_2:
            chocolate = !chocolate;
            break;
        case GLFW_KEY_SPACE:
            mixed = !mixed;
            break;
        case GLFW_KEY_R:
            resetCup();
            biteMarks.clear();
            break;
        }   
    }
}

int endProgram(std::string message) {
    std::cout << message << std::endl;
    glfwTerminate();
    return -1;
}

void preprocessTexture(unsigned& texture, const char* filepath) {
    texture = loadImageToTexture(filepath);
    glBindTexture(GL_TEXTURE_2D, texture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void formVAOs(float* verticesRect, size_t rectSize, unsigned int& VAOrect) {
    unsigned int VBOrect;
    glGenVertexArrays(1, &VAOrect);
    glGenBuffers(1, &VBOrect);

    glBindVertexArray(VAOrect);
    glBindBuffer(GL_ARRAY_BUFFER, VBOrect);
    glBufferData(GL_ARRAY_BUFFER, rectSize, verticesRect, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void drawRect(unsigned int rectShader, unsigned int VAOrect, unsigned int textureID,
    float posX = 0.0f, float posY = 0.0f, float scaleX = 1.0f, float scaleY = 1.0f) {
    glUseProgram(rectShader);

    // Set uniforms
    glUniform2f(glGetUniformLocation(rectShader, "uTranslation"), posX, posY);
    glUniform2f(glGetUniformLocation(rectShader, "uScale"), scaleX, scaleY);

    // Bind texture and draw
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glUniform1i(glGetUniformLocation(rectShader, "uTex"), 0);

    glBindVertexArray(VAOrect);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void iceCreamLever(int type, float leverPosition, unsigned int rectShader, unsigned int VAO_leverVertical,
    unsigned int VAO_leverHorizontal) {

    float verticalScaleY = 1.0f - leverPosition * 0.7f;
    float verticalPosY = (1.0f - verticalScaleY) * 0.5f;
    float horizontalPosY = leverPosition * -0.23f;
    float positionX = 0.0f;
    switch (type) {
    case 1:
        positionX = 0.0f;
        break;
    case 2:
        positionX = 0.16f;
        break;
    case 3:
        positionX = 0.31f;
        break;
    }
    drawRect(rectShader, VAO_leverVertical, leverVerticalTexture, positionX, verticalPosY, 1.0f, verticalScaleY);
    drawRect(rectShader, VAO_leverHorizontal, leverHorizontalTexture, positionX, horizontalPosY, 1.0f, 1.0f);
}
void drawIceCreamDrops(unsigned int rectShader, unsigned int VAO) {
    for (const auto& drop : iceCreamDrops) {
        if (drop.active && drop.posY > CUP_TOP_POS_Y) {
            unsigned textureID = 0;
            switch (drop.flavorType) {
            case 1: textureID = vanillaPourTexture; break;
            case 2: textureID = chocolatePourTexture; break;
            case 3: textureID = mixedPourTexture; break;
            }
            drawRect(rectShader, VAO, textureID,
                0.0f, drop.posY, DROP_WIDTH, drop.height);
        }
    }
}
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Convert to OpenGL coordinates (-1 to 1)
    spoonX = (float)(xpos / width * 2.0 - 1.0);
    spoonY = (float)(1.0 - ypos / height * 2.0);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        mousePressed = (action == GLFW_PRESS);

        // Add bite mark when clicking ON ICE CREAM ONLY
        if (mousePressed) {
            // Check if clicking inside the cup area where ice cream is
            bool isOnVanilla = vanillaFill.isFilled &&
                spoonY < vanillaFill.fillLevel &&
                spoonY > CUP_BOTTOM_POS_Y &&
                spoonX > -0.5f && spoonX < 0.5f;

            bool isOnChocolate = chocolateFill.isFilled &&
                spoonY < chocolateFill.fillLevel &&
                spoonY > CUP_BOTTOM_POS_Y &&
                spoonX > -0.5f && spoonX < 0.5f;

            bool isOnMixed = mixedFill.isFilled &&
                spoonY < mixedFill.fillLevel &&
                spoonY > CUP_BOTTOM_POS_Y &&
                spoonX > -0.5f && spoonX < 0.5f;

            // Only add bite if clicking on actual ice cream
            if (isOnVanilla || isOnChocolate || isOnMixed) {
                BiteMark bite;
                bite.x = spoonX;
                bite.y = spoonY;
                bite.size = 0.05f; // Small bite size
                biteMarks.push_back(bite);
            }
        }
    }
}


void drawBiteMarks(unsigned int rectShader, unsigned int VAO, unsigned int circleTexture) {
    glUseProgram(rectShader);

    for (const auto& bite : biteMarks) {
        drawRect(rectShader, VAO, circleTexture, bite.x, bite.y, bite.size, bite.size);
    }
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Ice Cream Machine", monitor, NULL);

    if (!window) return endProgram("Failed to create window");

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);

    if (glewInit() != GLEW_OK) return endProgram("GLEW failed to initialize");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    /*cursorReleased = loadImageToCursor("res/cursor.png");
    cursorPressed = loadImageToCursor("res/cursor_pressed.png");*/
    //glfwSetCursor(window, cursorReleased);
    // Initialize systems
    initSprinkles();
    initIceCream();
    // Load textures
    preprocessTexture(machineTexture, "res/machine.png");
    preprocessTexture(leverVerticalTexture, "res/lever.png");
    preprocessTexture(leverHorizontalTexture, "res/handle.png");
    preprocessTexture(sprinklesCloseTexture, "res/sprinklesClose.png");
    preprocessTexture(sprinklesOpenTexture, "res/sprinklesOpen.png");
    preprocessTexture(vanillaPourTexture, "res/vanillaPour.png");
    preprocessTexture(chocolatePourTexture, "res/chocolatePour.png");
    preprocessTexture(mixedPourTexture, "res/mixedPour.png");
    preprocessTexture(iceCreamVanillaTexture, "res/iceCreamVanilla.png");
    preprocessTexture(iceCreamChocolateTexture, "res/iceCreamChocolate.png");
    preprocessTexture(iceCreamMixedTexture, "res/iceCreamMixed.png");
    preprocessTexture(cupFrontTexture, "res/cupFront.png");
    preprocessTexture(cupBackTexture, "res/cupBack.png");
    preprocessTexture(spoonTexture, "res/spoon.png"); // You'll need a spoon.png image
    preprocessTexture(circularTexture, "res/circle.png");
    preprocessTexture(nameTexture, "res/nameTag.png");

    // Create shaders
    unsigned int rectShader = createShader("rect.vert", "rect.frag");
    if (rectShader == 0) return endProgram("Failed to create rectangle shader");

    unsigned int particleShader = createShader("particle.vert", "particle.frag");
    if (particleShader == 0) return endProgram("Failed to create particle shader");

    unsigned int VAO_machine, VAO_leverVertical, VAO_leverHorizontal, VAO_sprinklesLever, VAO_iceCreamVanilla, VAO_cup, VAO_name;

    float rectVertices[] = {
        -1.0f,  1.0f,   0.0f, 1.0f,
        -1.0f, -1.0f,   0.0f, 0.0f,
         1.0f, -1.0f,   1.0f, 0.0f,
         1.0f,  1.0f,   1.0f, 1.0f
    };


    formVAOs(rectVertices, sizeof(rectVertices), VAO_machine);
    formVAOs(rectVertices, sizeof(rectVertices), VAO_leverVertical);
    formVAOs(rectVertices, sizeof(rectVertices), VAO_leverHorizontal);
    formVAOs(rectVertices, sizeof(rectVertices), VAO_sprinklesLever);
    formVAOs(rectVertices, sizeof(rectVertices), VAO_iceCreamVanilla);
    formVAOs(rectVertices, sizeof(rectVertices), VAO_cup);
    formVAOs(rectVertices, sizeof(rectVertices), VAO_spoon);
    formVAOs(rectVertices, sizeof(rectVertices), VAO_name);

    unsigned int particleVAO, particleVBO;
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    float aspect = (float)width / height;

    float particleVertices[] = {
        -0.5f / aspect, -0.5f,  0.0f, 0.0f,
         0.5f / aspect, -0.5f,  1.0f, 0.0f,
         0.5f / aspect,  0.5f,  1.0f, 1.0f,
        -0.5f / aspect,  0.5f,  0.0f, 1.0f
    };

    formVAOs(particleVertices, sizeof(particleVertices), particleVAO);

    glClearColor(0.392156862745098f, 0.4470588235294118f, 0.4901960784313725f, 1.0f);
    lastUpdateTime = glfwGetTime();
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // Hide default cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        double deltaTime = currentTime - lastUpdateTime;
        lastUpdateTime = currentTime;

        updateLevers(deltaTime);
        updateIceCreamDrops(deltaTime);
        updateSprinklesPhysics(deltaTime);

        glClear(GL_COLOR_BUFFER_BIT);

        // Draw background elements first
        drawRect(rectShader, VAO_cup, cupBackTexture, 0.0f, 0.0f, 1.0f, 1.0f);

        // Draw the piled ice cream drops (inside the cup)
        drawIceCreamDrops(rectShader, VAO_iceCreamVanilla);

        // Draw the vanilla fill layer (optional - can remove if using only piled drops)
        if (vanillaFill.isFilled && vanillaFill.fillLevel > CUP_BOTTOM_POS_Y) {
            float fillHeight = vanillaFill.fillLevel - CUP_BOTTOM_POS_Y;
            float fillPosY = CUP_BOTTOM_POS_Y + (fillHeight / 2.0f);
            drawRect(rectShader, VAO_iceCreamVanilla, iceCreamVanillaTexture,
                0.0f, fillPosY, CUP_FILL_WIDTH, fillHeight);
        }

        // Draw chocolate fill layer
        if (chocolateFill.isFilled && chocolateFill.fillLevel > CUP_BOTTOM_POS_Y) {
            float fillHeight = chocolateFill.fillLevel - CUP_BOTTOM_POS_Y;
            float fillPosY = CUP_BOTTOM_POS_Y + (fillHeight / 2.0f);
            drawRect(rectShader, VAO_iceCreamVanilla, iceCreamChocolateTexture,
                0.0f, fillPosY, CUP_FILL_WIDTH, fillHeight);
        }

        // Draw mixed fill layer
        if (mixedFill.isFilled && mixedFill.fillLevel > CUP_BOTTOM_POS_Y) {
            float fillHeight = mixedFill.fillLevel - CUP_BOTTOM_POS_Y;
            float fillPosY = CUP_BOTTOM_POS_Y + (fillHeight / 2.0f);
            drawRect(rectShader, VAO_iceCreamVanilla, iceCreamMixedTexture,
                0.0f, fillPosY, CUP_FILL_WIDTH, fillHeight);
        }

        // Draw cup front (on top of everything)
        drawBiteMarks(rectShader, VAO_spoon, circularTexture); // or drawBiteMarks() if using immediate mode

        // Draw the machine and levers (on top of cup)
        drawRect(rectShader, VAO_machine, machineTexture, 0.0f, 0.0f, 1.0f, 1.0f);
        drawRect(rectShader, VAO_name, nameTexture, 0.0f, 0.0f, 1.0f, 1.0f);
        drawRect(rectShader, VAO_cup, cupFrontTexture, 0.0f, 0.0f, 1.0f, 1.0f);

        iceCreamLever(1, leverPositionVanilla, rectShader, VAO_leverVertical, VAO_leverHorizontal);
        iceCreamLever(2, leverPositionMixed, rectShader, VAO_leverVertical, VAO_leverHorizontal);
        iceCreamLever(3, leverPositionChocolate, rectShader, VAO_leverVertical, VAO_leverHorizontal);

        if (sprinklesOpen) {
            drawRect(rectShader, VAO_sprinklesLever, sprinklesOpenTexture, 0.0f, 0.0f, 1.0f, 1.0f);
        }
        else {
            drawRect(rectShader, VAO_sprinklesLever, sprinklesCloseTexture, 0.0f, 0.0f, 1.0f, 1.0f);
        }

        
        static double lastSprinkleSpawnTime = 0.0;
        if (sprinklesOpen && currentTime - lastSprinkleSpawnTime > 0.08f) {
            spawnSprinkles();
            lastSprinkleSpawnTime = currentTime;
        }

        for (const auto& drop : sprinkles) {
            drawSprinkles(drop, particleShader, particleVAO);
        }
        drawRect(rectShader, VAO_spoon, spoonTexture, spoonX, spoonY, spoonSize, spoonSize);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(rectShader);
    glDeleteProgram(particleShader);
    glDeleteVertexArrays(1, &VAO_machine);
    glDeleteVertexArrays(1, &VAO_leverVertical);
    glDeleteVertexArrays(1, &VAO_leverHorizontal);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}