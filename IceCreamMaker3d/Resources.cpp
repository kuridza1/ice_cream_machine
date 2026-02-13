// Resources.cpp
#include "Resources.h"
#include "TextureUtils.h"

void Resources::CreateHudQuad(GLuint& vao, GLuint& vbo)
{
    // x: -1..1, y: -1..1
    const float x0 = -1.0f;
    const float y0 = -1.0f;
    const float x1 = 1.0f;
    const float y1 = 1.0f;

    // pos(x,y), uv(u,v)
    float v[] = {
        // tri 1
        x0, y1,  0.0f, 1.0f,   // TL
        x0, y0,  0.0f, 0.0f,   // BL
        x1, y0,  1.0f, 0.0f,   // BR
        // tri 2
        x0, y1,  0.0f, 1.0f,   // TL
        x1, y0,  1.0f, 0.0f,   // BR
        x1, y1,  1.0f, 1.0f    // TR
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
}


Resources::Resources()
    : machine("res/MachineModel.obj"),
    iceCream("res/IceCreamModel.obj"),
    cup("res/Cup.obj"),
    lever("res/LeverModel.obj"),
    pour("res/Pour.obj"),
    sprinklesContainer("res/SprinklesContainer.obj"),
    sprinkles("res/Sprinkles.obj"),
    sprinkle("res/Sprinkle.obj"),
    power("res/Power.obj"),
    button1("res/Button1.obj"),
    button2("res/Button2.obj"),
    buttonMix("res/ButtonMix.obj"),
    buttonLed("res/ButtonLed.obj"),
    table("res/Table.obj"),
    shader("basic.vert", "basic.frag")
{
}
