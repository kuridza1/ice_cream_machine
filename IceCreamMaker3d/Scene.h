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
#include "ButtonSystem.h"

class Scene
{
public:
    Scene();

    void onEnterPressed();
    void onResetPressed();
    void onSPressed();
    void onPPressed();

    void on1Pressed();
    void on2Pressed();
    void onSpacePressed();

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
    ButtonSystem btnSys;
    glm::vec3 iceCreamPosOffset;
};
