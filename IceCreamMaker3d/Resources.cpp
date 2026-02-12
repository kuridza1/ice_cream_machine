// Resources.cpp
#include "Resources.h"

Resources::Resources()
    : machine("res/MachineModel.obj"),
    iceCream("res/IceCreamModel.obj"),
    cup("res/Cup.obj"),
    lever("res/LeverModel.obj"),
    pour("res/Pour.obj"),
	sprinklesContainer("res/SprinklesContainer.obj"),
	sprinkles("res/Sprinkles.obj"),
	sprinkle("res/Sprinkle.obj"),

    shader("basic.vert", "basic.frag")
{
}
