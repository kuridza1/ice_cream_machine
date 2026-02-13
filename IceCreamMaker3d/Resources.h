// Resources.h
#pragma once
#include "model.hpp"
#include "shader.hpp"

struct Resources
{
    Model machine;
    Model iceCream;
    Model cup;
    Model lever;
    Model pour;
    Model sprinklesContainer;
    Model sprinkles;
    Model sprinkle;
    Model power;
    Model button1;
    Model button2;
    Model buttonMix;
	Model buttonLed;
    Model table;
    unsigned int texVanilla = 0;
    unsigned int texChoco = 0;
    unsigned int texMix = 0;


    Shader shader;

    unsigned int hudTex = 0;
    Shader* hudShader = nullptr;   
    unsigned int hudVAO = 0;
    unsigned int hudVBO = 0;
    void CreateHudQuad(GLuint& vao, GLuint& vbo);
    Resources();
};
