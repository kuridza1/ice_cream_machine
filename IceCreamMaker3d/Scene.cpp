// Scene.cpp
#include "Scene.h"
#include <glm/gtc/matrix_transform.hpp>

Scene::Scene()
    : res(),
    cupCtrl(glm::radians(50.0f)),
    pourSys(0.4f, 0.5f, 0.0f, -1.2f),
    iceSys(0.08f, 0.5f, -0.85f, 0.0f, -1.2f),
    iceCreamPosOffset(0.0f, 0.0f, 0.0f)
{
}

void Scene::onSpacePressed()
{
    pourSys.toggle();
}

void Scene::onResetPressed()
{
    pourSys.reset();
    iceSys.reset();
    // cupCtrl intentionally NOT reset -> cup keeps spinning through reset
}

void Scene::update(float dt)
{
    cupCtrl.update(dt);

    pourSys.update(dt);

    bool contactNow = pourSys.isContactPast(iceSys.contactT());
    iceSys.update(dt, pourSys.isActiveOrStopping(), contactNow);
}

void Scene::render(const RenderContext& ctx)
{
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

    // Base transform
    glm::mat4 base = glm::mat4(1.0f);
    base = glm::translate(base, glm::vec3(0.0f, -1.0f, 0.0f));
    base = glm::scale(base, glm::vec3(1.4f));

    // MACHINE
    sh.setMat4("uM", base);
    res.machine.Draw(sh);

    // LEVER
    sh.setMat4("uM", base);
    res.lever.Draw(sh);

    // CUP (spins always)
    glm::mat4 cupM = cupCtrl.apply(base);
    sh.setMat4("uM", cupM);
    res.cup.Draw(sh);

    // POUR
    pourSys.draw(base, sh, res.pour);

    // ICE CREAM (follows cup)
    sh.setInt("uUseProgressMask", 1);
    sh.setFloat("uProgress", iceSys.progress());

    glm::mat4 iceM = cupM;
    iceM = glm::translate(iceM, iceCreamPosOffset);
    sh.setMat4("uM", iceM);
    res.iceCream.Draw(sh);

    // Restore defaults
    sh.setInt("uUseProgressMask", 0);
    sh.setFloat("uProgress", 1.0f);
}
