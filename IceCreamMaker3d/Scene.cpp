// Scene.cpp
#include "Scene.h"
#include <glm/gtc/matrix_transform.hpp>
#include "TextureUtils.h"

Scene::Scene()
    : res(),
    cupCtrl(glm::radians(50.0f)),
    leverSys(glm::radians(60.0f),
        glm::vec3(0.0f, 1.5f, 0.0f),  
        glm::vec3(-1.0f, 0.0f, 0.0f),    
        5.0f),                         
    pourSys(0.4f, 0.5f, 0.0f, -1.2f),
    iceSys(0.08f, 0.5f, -0.85f, 0.0f, -1.2f),
    iceCreamPosOffset(0.0f, 0.0f, 0.0f),
    powerSys(glm::vec3(0.0f, 0.0f, -1.8),
         glm::vec3(1, 0, 0),
        0.0f, -30.0f, 220.0f),
    btnSys()

{
    btnSys.init(0.03f, 18.0f, 12.0f, glm::vec3(0,0,-1));

}


void Scene::initGL()
{
    res.texVanilla = LoadTexture2D("res/ice_vanilla.png", true);
    res.texChoco = LoadTexture2D("res/ice_choco.png", true);
    res.texMix = LoadTexture2D("res/ice_mixed.png", true);
}
void Scene::onEnterPressed()
{
    leverSys.toggle();
}


void Scene::onResetPressed()
{
    pourSys.reset();
    iceSys.reset();
    leverSys.resetUp();
    sprinkleSys.reset();
	btnSys.reset();
}

void Scene::onSPressed()
{
    sprinkleSys.setOpen(!sprinkleSys.isOpen());
}

void Scene::onPPressed() 
{
	powerSys.toggle();
}

void Scene::on1Pressed() { btnSys.press(ButtonId::One); }
void Scene::on2Pressed() { btnSys.press(ButtonId::Two); }
void Scene::onSpacePressed() { btnSys.press(ButtonId::Mix); } 

void Scene::update(float dt)
{
    powerSys.update(dt);

    if (powerSys.isOn()) {
        cupCtrl.update(dt);
        leverSys.update(dt);

        if (leverSys.isFullyDown())
            pourSys.start();
        else
            pourSys.requestStop();

        pourSys.update(dt);
        btnSys.update(dt);

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

        glm::vec3 iceCenterW = glm::vec3(iceM * glm::vec4(0, 0, 0, 1));
        glm::vec3 cupCenterW = glm::vec3(cupM * glm::vec4(0, 0, 0, 1));


        sprinkleSys.setCupRegion(cupCenterW, 0.3f);
        sprinkleSys.setIceCollider(iceCenterW, 0.27f);
        sprinkleSys.setCupMatrix(cupM);

        sprinkleSys.update(dt);
    }
}

static void DebugDrawLines(const glm::mat4& VP, const std::vector<glm::vec3>& pts)
{
    // pts = lista verteksa u parovima (linije): [a0,b0,a1,b1,...]
    static GLuint vao = 0, vbo = 0;
    if (vao == 0)
    {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glBindVertexArray(0);
    }

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, pts.size() * sizeof(glm::vec3), pts.data(), GL_DYNAMIC_DRAW);

    // koristi tvoj postoje?i shader: crtaj kao emission-only crveno
    // o?ekuje: uUseEmission, uEmissionColor, uEmissionStrength, uM, uV, uP
    // M=I jer su pts ve? u world space
    glDrawArrays(GL_LINES, 0, (GLsizei)pts.size());

    glBindVertexArray(0);
}



static void BuildCenterCross(std::vector<glm::vec3>& out, const glm::vec3& c, float s = 0.05f)
{
    out.push_back(c + glm::vec3(-s, 0, 0)); out.push_back(c + glm::vec3(s, 0, 0));
    out.push_back(c + glm::vec3(0, -s, 0)); out.push_back(c + glm::vec3(0, s, 0));
    out.push_back(c + glm::vec3(0, 0, -s)); out.push_back(c + glm::vec3(0, 0, s));
}
static void DrawDebugLines(Shader& sh,
    const std::vector<glm::vec3>& pts,
    const glm::vec3& color)
{
    static GLuint vao = 0, vbo = 0;

    if (vao == 0)
    {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glBindVertexArray(0);
    }

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
        pts.size() * sizeof(glm::vec3),
        pts.data(),
        GL_DYNAMIC_DRAW);

    sh.setInt("uUseEmission", 1);
    sh.setVec3("uEmissionColor", color.x, color.y, color.z);
    sh.setFloat("uEmissionStrength", 5.0f);
    sh.setMat4("uM", glm::mat4(1.0f));

    glDrawArrays(GL_LINES, 0, (GLsizei)pts.size());

    sh.setInt("uUseEmission", 0);
    sh.setFloat("uEmissionStrength", 0.0f);

    glBindVertexArray(0);
}

static void BuildCircleXZ(std::vector<glm::vec3>& out,
    const glm::vec3& c,
    float r,
    int segs = 64)
{
    for (int i = 0; i < segs; i++)
    {
        float a0 = i * 6.2831853f / segs;
        float a1 = (i + 1) * 6.2831853f / segs;

        glm::vec3 p0 = c + glm::vec3(cos(a0) * r, 0, sin(a0) * r);
        glm::vec3 p1 = c + glm::vec3(cos(a1) * r, 0, sin(a1) * r);

        out.push_back(p0);
        out.push_back(p1);
    }
}



static void BuildCross(std::vector<glm::vec3>& out,
    const glm::vec3& c,
    float s = 0.05f)
{
    out.push_back(c + glm::vec3(-s, 0, 0));
    out.push_back(c + glm::vec3(s, 0, 0));

    out.push_back(c + glm::vec3(0, -s, 0));
    out.push_back(c + glm::vec3(0, s, 0));

    out.push_back(c + glm::vec3(0, 0, -s));
    out.push_back(c + glm::vec3(0, 0, s));
}

// “sfera” kao 3 kruga (XY, XZ, YZ)
static void BuildSphere3Circles(std::vector<glm::vec3>& out, const glm::vec3& c, float r, int segs = 64)
{
    // XZ
    BuildCircleXZ(out, c, r, segs);

    // XY
    for (int i = 0; i < segs; i++)
    {
        float a0 = (float)i * 6.2831853f / (float)segs;
        float a1 = (float)(i + 1) * 6.2831853f / (float)segs;
        glm::vec3 p0 = c + glm::vec3(std::cos(a0) * r, std::sin(a0) * r, 0.0f);
        glm::vec3 p1 = c + glm::vec3(std::cos(a1) * r, std::sin(a1) * r, 0.0f);
        out.push_back(p0); out.push_back(p1);
    }

    // YZ
    for (int i = 0; i < segs; i++)
    {
        float a0 = (float)i * 6.2831853f / (float)segs;
        float a1 = (float)(i + 1) * 6.2831853f / (float)segs;
        glm::vec3 p0 = c + glm::vec3(0.0f, std::cos(a0) * r, std::sin(a0) * r);
        glm::vec3 p1 = c + glm::vec3(0.0f, std::cos(a1) * r, std::sin(a1) * r);
        out.push_back(p0); out.push_back(p1);
    }
}


void Scene::render(const RenderContext& ctx)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    auto& sh = res.shader;
    sh.use();

    // ----------------------------------------------------
    // GLOBAL UNIFORMS
    // ----------------------------------------------------

    sh.setMat4("uP", ctx.P);
    sh.setMat4("uV", ctx.V);
    sh.setVec3("uLightPos", ctx.lightPos.x, ctx.lightPos.y, ctx.lightPos.z);
    sh.setVec3("uViewPos", ctx.viewPos.x, ctx.viewPos.y, ctx.viewPos.z);
    sh.setVec3("uLightColor", ctx.lightColor.x, ctx.lightColor.y, ctx.lightColor.z);

    sh.setInt("uFlavor", (int)btnSys.selectedFlavor());

    sh.setInt("uUseProgressMask", 0);
    sh.setFloat("uProgress", 1.0f);

    

    // vrati emission nazad (da ti ne oboji ostalo)
    sh.setInt("uUseEmission", 0);
    sh.setFloat("uEmissionStrength", 0.0f);
    // ----------------------------------------------------
    // BUTTON POINT LIGHT (affects whole scene)
    // ----------------------------------------------------

    if (powerSys.isOn())
    {
        sh.setInt("uUseBtnLight", 1);
        sh.setVec3("uBtnLightColor", 1.0f, 0.85f, 0.7f);
        sh.setFloat("uBtnLightIntensity", 2.5f);

        // world position of LED (adjust numbers if needed)
        glm::mat4 base(1.0f);
        base = glm::translate(base, glm::vec3(0.0f, -1.0f, 0.0f));
        base = glm::scale(base, glm::vec3(1.4f));

        glm::vec3 btnLightPos =
            glm::vec3(base * glm::vec4(-0.2f, 1.0f, -0.2f, 1.0f));

        sh.setVec3("uBtnLightPos",
            btnLightPos.x,
            btnLightPos.y,
            btnLightPos.z);
    }
    else
    {
        sh.setInt("uUseBtnLight", 0);
    }

    // ----------------------------------------------------
    // BASE MATRIX
    // ----------------------------------------------------

    glm::mat4 base(1.0f);
    base = glm::translate(base, glm::vec3(0.0f, -1.0f, 0.0f));
    base = glm::scale(base, glm::vec3(1.4f));

    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);

    // ----------------------------------------------------
    // MACHINE
    // ----------------------------------------------------

    sh.setMat4("uM", base);
    res.machine.Draw(sh);
    res.sprinkles.Draw(sh);
    res.table.Draw(sh);
    sprinkleSys.setModels({ &res.sprinkle });
    sprinkleSys.draw(sh);

    // ----------------------------------------------------
    // POWER BUTTON (rotating switch)
    // ----------------------------------------------------

    glm::mat4 powerM = powerSys.applyTo(base);
    sh.setMat4("uM", powerM);
    res.power.Draw(sh);

    // ----------------------------------------------------
    // LEVER
    // ----------------------------------------------------

    glm::mat4 leverM = leverSys.apply(base);
    sh.setMat4("uM", leverM);
    res.lever.Draw(sh);

    // ----------------------------------------------------
    // CUP
    // ----------------------------------------------------

    glm::mat4 cupM = cupCtrl.apply(base);
    sh.setMat4("uM", cupM);
    res.cup.Draw(sh);

    // ----------------------------------------------------
    // ICE CREAM (progress mask)
    // ----------------------------------------------------
    unsigned int iceTex = 0;
    switch (btnSys.selectedFlavor())
    {
    case IceFlavor::One: iceTex = res.texVanilla; break;
    case IceFlavor::Two: iceTex = res.texChoco;   break;
    case IceFlavor::Mix: iceTex = res.texMix;     break;
    }
    res.iceCream.SetOverrideDiffuse(iceTex);

    // ICE CREAM draw
    sh.setInt("uUseProgressMask", 1);
    sh.setFloat("uProgress", iceSys.progress());

    glm::mat4 iceM = glm::translate(cupM, iceCreamPosOffset);
    sh.setMat4("uM", iceM);
    res.iceCream.Draw(sh);

    // reset mask
    sh.setInt("uUseProgressMask", 0);
    sh.setFloat("uProgress", 1.0f);

    // ----------------------------------------------------
    // BUTTONS (normal body)
    // ----------------------------------------------------

    sh.setInt("uUseEmission", 0);
    sh.setFloat("uEmissionStrength", 0.0f);

    sh.setMat4("uM", btnSys.applyTo(base, ButtonId::One));
    res.button1.Draw(sh);

    sh.setMat4("uM", btnSys.applyTo(base, ButtonId::Two));
    res.button2.Draw(sh);

    sh.setMat4("uM", btnSys.applyTo(base, ButtonId::Mix));
    res.buttonMix.Draw(sh);

    // ----------------------------------------------------
    // LED PART (EMISSION ONLY)
    // ----------------------------------------------------

    if (powerSys.isOn())
    {
        sh.setInt("uUseEmission", 1);
        sh.setVec3("uEmissionColor", 1.0f, 0.85f, 0.7f);
        sh.setFloat("uEmissionStrength", 4.5f);
    }
    else
    {
        sh.setInt("uUseEmission", 0);
        sh.setFloat("uEmissionStrength", 0.0f);
    }

    sh.setMat4("uM", base);
    res.buttonLed.Draw(sh);

    // reset emission
    sh.setInt("uUseEmission", 0);
    sh.setFloat("uEmissionStrength", 0.0f);

    // ----------------------------------------------------
    // POUR
    // ----------------------------------------------------

    pourSys.draw(base, sh, res.pour);

    // ----------------------------------------------------
    // GLASS (transparent last)
    // ----------------------------------------------------

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDepthMask(GL_FALSE);

    sh.setMat4("uM", base);
    res.sprinklesContainer.Draw(sh);
   

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

