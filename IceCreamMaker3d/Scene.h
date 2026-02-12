// Scene.h
#pragma once
#include <glm/glm.hpp>
#include "Resources.h"
#include "CupController.h"
#include "PourSystem.h"
#include "IceCreamSystem.h"
#include "RenderContext.h"
#include "LeverSystem.h"
#include "SprinkleSystem.h"
#include "PowerSystem.h"

class Scene
{
public:
    Scene();

    void onSpacePressed();
    void onResetPressed();
    void onSPressed();
    void onPPressed();

    void update(float dt);

    void render(const RenderContext& ctx);

private:
    Resources res;

    CupController cupCtrl;
    PourSystem pourSys;
    IceCreamSystem iceSys;
    LeverSystem leverSys; 
    SprinklesSystem sprinkleSys;
    PowerSystem powerSys;
    glm::vec3 iceCreamPosOffset;
};
