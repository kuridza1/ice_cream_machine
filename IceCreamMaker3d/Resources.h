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

    Shader shader;

    Resources();
};
