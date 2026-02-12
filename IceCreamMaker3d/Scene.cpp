// Scene.cpp
#include "Scene.h"
#include <glm/gtc/matrix_transform.hpp>

Scene::Scene()
    : res(),
    cupCtrl(glm::radians(50.0f)),
    // pivot/osa su "na slepo" default; dotegni pivotLocal da pogodi model
    leverSys(glm::radians(60.0f),
        glm::vec3(0.0f, 1.5f, 0.0f),   // pivotLocal (DOTEGNI)
        glm::vec3(-1.0f, 0.0f, 0.0f),    // axisLocal (promeni u (0,0,1) ako treba)
        5.0f),                          // brzina animacije
    pourSys(0.4f, 0.5f, 0.0f, -1.2f),
    iceSys(0.08f, 0.5f, -0.85f, 0.0f, -1.2f),
    iceCreamPosOffset(0.0f, 0.0f, 0.0f)
{
}



void Scene::onSpacePressed()
{
    leverSys.toggle();
}


void Scene::onResetPressed()
{
    pourSys.reset();
    iceSys.reset();
    leverSys.resetUp(); // NEW
    // cupCtrl intentionally NOT reset
}

void Scene::onSPressed()
{
    // Toggle sprinkle emission
    sprinkleSys.setOpen(!sprinkleSys.isOpen());
}



// Scene.cpp — minimal changes: read FBX node markers and feed SprinkleSystem
// Assumes your res.machine is the FBX that contains the empties/nodes.

void Scene::update(float dt)
{
    cupCtrl.update(dt);
    leverSys.update(dt);

    if (leverSys.isFullyDown())
        pourSys.start();
    else
        pourSys.requestStop();

    pourSys.update(dt);

    bool contactNow = pourSys.isContactPast(iceSys.contactT());
    iceSys.update(dt, pourSys.isActiveOrStopping(), contactNow);

    glm::mat4 base(1.0f);
    base = glm::translate(base, glm::vec3(0.0f, -1.0f, 0.0f));
    base = glm::scale(base, glm::vec3(1.4f));

    glm::mat4 cupM = cupCtrl.apply(base);
    glm::mat4 iceM = glm::translate(cupM, iceCreamPosOffset);

    glm::vec3 nozzleLocal(-1.1f, 0.9f, 0.0f);
    glm::vec3 entLocal(-1.1f, 0.48f, 0.0f);

    glm::vec3 startLocal(-1.1f, 0.48f, 0.0f);
    glm::vec3 endLocal(-0.2f, 0.24f, 0.0f);

    glm::vec3 nozzlePosW = glm::vec3(base * glm::vec4(nozzleLocal, 1));
    glm::vec3 entW = glm::vec3(base * glm::vec4(entLocal, 1));
    glm::vec3 startW = glm::vec3(base * glm::vec4(startLocal, 1));
    glm::vec3 endW = glm::vec3(base * glm::vec4(endLocal, 1));

    glm::vec3 nozzleDirW = glm::normalize(entW - nozzlePosW);

    sprinkleSys.setNozzle(nozzlePosW, nozzleDirW, 0.02f);
    sprinkleSys.setTunnel(entW, startW, endW);



    // Cup/ice collider in world (simple sphere)
    glm::vec3 iceCenterW = glm::vec3(iceM * glm::vec4(0, 0, 0, 1));
    glm::vec3 cupCenterW = glm::vec3(cupM * glm::vec4(0, 0, 0, 1));

    sprinkleSys.setCupRegion(cupCenterW, 0.1f);
    sprinkleSys.setIceCollider(iceCenterW, 0.23f);

    sprinkleSys.update(dt);
}



void Scene::render(const RenderContext& ctx)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    auto& sh = res.shader;
    sh.use();

    sh.setMat4("uP", ctx.P);
    sh.setMat4("uV", ctx.V);
    sh.setVec3("uLightPos", ctx.lightPos.x, ctx.lightPos.y, ctx.lightPos.z);
    sh.setVec3("uViewPos", ctx.viewPos.x, ctx.viewPos.y, ctx.viewPos.z);
    sh.setVec3("uLightColor", ctx.lightColor.x, ctx.lightColor.y, ctx.lightColor.z);

    // Default: no progress mask
    sh.setInt("uUseProgressMask", 0);
    sh.setFloat("uProgress", 1.0f);

    glm::mat4 base(1.0f);
    base = glm::translate(base, glm::vec3(0.0f, -1.0f, 0.0f));
    base = glm::scale(base, glm::vec3(1.4f));

    // 1) OPAQUE PASS (bez blendinga)
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);          // normalno upisuj depth

    sh.setInt("uUseProgressMask", 0);
    sh.setFloat("uProgress", 1.0f);

    sh.setMat4("uM", base);
    res.machine.Draw(sh);

    res.sprinkles.Draw(sh);
    sprinkleSys.setModels({ &res.sprinkle });
    sprinkleSys.draw(sh);

    // lever
    glm::mat4 leverM = leverSys.apply(base);
    sh.setMat4("uM", leverM);
    res.lever.Draw(sh);

    // cup
    glm::mat4 cupM = cupCtrl.apply(base);
    sh.setMat4("uM", cupM);
    res.cup.Draw(sh);

    // pour
    pourSys.draw(base, sh, res.pour);

    // ice cream (mask)
    sh.setInt("uUseProgressMask", 1);
    sh.setFloat("uProgress", iceSys.progress());
    glm::mat4 iceM = glm::translate(cupM, iceCreamPosOffset);
    sh.setMat4("uM", iceM);
    res.iceCream.Draw(sh);

    // 2) TRANSPARENT PASS (glass last)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDepthMask(GL_FALSE);         // klju?: NE upisuj depth za staklo
    sh.setInt("uUseProgressMask", 0);
    sh.setFloat("uProgress", 1.0f);

    sh.setMat4("uM", base);
    res.sprinklesContainer.Draw(sh);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

}
