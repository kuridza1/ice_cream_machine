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
	Model nozzle;
	Model tunnelStart;
	Model tunnelEnd;
    Shader shader;


    Resources();
};
