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
    Shader shader;


    Resources();
    void load();
};
