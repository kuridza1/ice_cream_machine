// SprinkleSystem.cpp
#include "SprinkleSystem.h"

#include <algorithm>
#include <cmath>

static inline float rand01(std::mt19937& g)
{
    std::uniform_real_distribution<float> d(0.0f, 1.0f);
    return d(g);
}

SprinklesSystem::SprinklesSystem()
    : m_gen(m_rd())
{
    m_sprinkles.reserve(512);
}

void SprinklesSystem::init()
{
    m_sprinkles.clear();
}

void SprinklesSystem::reset()
{
    m_sprinkles.clear();
    m_open = false;
    m_spawnAccum = 0.0f;
    m_exitOccupied = false;
}

void SprinklesSystem::setModels(const std::vector<Model*>& models)
{
    m_models = models;
}

void SprinklesSystem::setNozzle(const glm::vec3& pos, const glm::vec3& dir, float spawnDiscRadius)
{
    m_nozzlePos = pos;
    m_nozzleDir = glm::normalize(dir);
    m_spawnDiscRadius = spawnDiscRadius;
}

void SprinklesSystem::setTunnel(const glm::vec3& entrancePos, const glm::vec3& startPos, const glm::vec3& endPos)
{
    m_tunnelEntrance = entrancePos;
    m_tunnelStart = startPos;
    m_tunnelEnd = endPos;
}

void SprinklesSystem::setCupRegion(const glm::vec3& center, float radius)
{
    m_cupCenter = center;
    m_cupRadius = radius;
}
void SprinklesSystem::setCupMatrix(const glm::mat4& cupWorld)
{
    m_cupM = cupWorld;
    m_cupInvM = glm::inverse(cupWorld);
}

void SprinklesSystem::setIceCollider(const glm::vec3& center, float radius)
{
    m_iceCenter = center;
    m_iceRadius = radius;
}

glm::vec3 SprinklesSystem::tunnelPoint(float t) const
{
    t = std::max(0.0f, std::min(1.0f, t));
    return (1.0f - t) * m_tunnelStart + t * m_tunnelEnd;
}
static inline float sprinkleRadius(const Sprinkle& s)
{

    constexpr float SPRINKLE_MODEL_RADIUS = 0.5f;
    return SPRINKLE_MODEL_RADIUS * s.size;
}


void SprinklesSystem::spawn()
{
    if (!m_open) return;

    Sprinkle s;
    s.pos = m_nozzlePos;

    // random point in disc perpendicular to nozzleDir
    std::uniform_real_distribution<float> a01(0.0f, 1.0f);
    std::uniform_real_distribution<float> aAng(0.0f, 6.2831853f);

    glm::vec3 up = (std::abs(m_nozzleDir.y) < 0.99f) ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
    glm::vec3 right = glm::normalize(glm::cross(up, m_nozzleDir));
    glm::vec3 forward = glm::normalize(glm::cross(m_nozzleDir, right));

    float r = m_spawnDiscRadius * std::sqrt(a01(m_gen));
    float ang = aAng(m_gen);

    s.pos += right * (r * std::cos(ang)) + forward * (r * std::sin(ang));

    s.slideTimer = 0.0f;
    s.waitTimer = 0.0f;
    s.waitingToExit = false;
    s.state = 0;


    s.size = 0.20f;

    std::uniform_real_distribution<float> disSpeed(0.25f, 0.85f);
    float speed = disSpeed(m_gen);

    std::uniform_real_distribution<float> disSpread(-1.0f, 1.0f);
    float spreadA = disSpread(m_gen);
    float spreadB = disSpread(m_gen);

    float spreadStrength = 0.12f * speed;

    glm::vec3 lateral = right * (spreadA * spreadStrength) + forward * (spreadB * spreadStrength);
    s.vel = m_nozzleDir * speed + lateral;


    std::uniform_real_distribution<float> disSlideMul(0.65f, 1.35f);
    s.slideSpeedMul = disSlideMul(m_gen);

    // Rotation
    std::uniform_real_distribution<float> disRotSpeed(-2.0f, 2.0f);
    s.rot = glm::vec3(0.0f);
    s.rotSpeed = glm::vec3(disRotSpeed(m_gen), disRotSpeed(m_gen), disRotSpeed(m_gen));

    if (!m_models.empty()) {
        std::uniform_int_distribution<int> disModel(0, (int)m_models.size() - 1);
        s.modelIndex = disModel(m_gen);
    }
    else {
        s.modelIndex = 0;
    }

    s.active = true;
    m_sprinkles.push_back(s);

    if ((int)m_sprinkles.size() > m_maxCount)
        m_sprinkles.erase(m_sprinkles.begin());
}



void SprinklesSystem::update(double dt)
{
    if (m_open)
    {
        m_spawnAccum += (float)(m_spawnRate * dt);
        int toSpawn = (int)std::floor(m_spawnAccum);
        if (toSpawn > 0)
        {
            m_spawnAccum -= (float)toSpawn;
            for (int i = 0; i < toSpawn; i++) spawn();
        }
    }

    for (auto& d : m_sprinkles)
    {
        if (!d.active) continue;

        glm::vec3 prevPos = d.pos;

        if (d.state == 0 || d.state == 2)
            d.vel.y += m_gravity * (float)dt;

        if (d.state == 0 || d.state == 2)
            d.rot += d.rotSpeed * (float)dt;

        if (d.state == 0 || d.state == 2)
            d.pos += d.vel * (float)dt;

        switch (d.state)
        {
        case 0: // nozzle -> tunnel entrance 
        {

            float r = 0.1f;
            glm::vec2 prevXZ(prevPos.x, prevPos.z);
            glm::vec2 currXZ(d.pos.x, d.pos.z);
            glm::vec2 entXZ(m_tunnelEntrance.x, m_tunnelEntrance.z);
            bool closeXZ = glm::length(currXZ - entXZ) <= r;
            float targetY = 0.25f;
            float eps = 0.005f;

            bool atExactY = std::abs(d.pos.y - targetY) <= eps;

            if (closeXZ && atExactY)
            {
                d.pos = m_tunnelEntrance;
                d.pos.y = m_tunnelEntrance.y + d.size;
                d.vel = glm::vec3(0.0f);
                d.state = 1;
                d.slideTimer = 0.0f;
                if (!m_exitOccupied)
                    d.waitingToExit = false;
                else
                {
                    d.waitingToExit = true;
                    d.waitTimer = 0.0f;
                }
            }
            else if (d.pos.y < 0.25f)
            {
                d.active = false;
            }
        }
        break;

        case 1: // tunnel slide
        {
            d.slideTimer += (float)dt;

            if (d.pos.y > m_tunnelStart.y + d.size)
            {
                d.pos.y -= 0.5f * (float)dt;
                if (d.pos.y <= m_tunnelStart.y + d.size)
                    d.pos.y = m_tunnelStart.y + d.size;

                d.pos.x = m_tunnelStart.x;
                d.pos.z = m_tunnelStart.z;
            }
            else
            {
                if (d.waitingToExit)
                {
                    d.waitTimer += (float)dt;
                    if (!m_exitOccupied && d.waitTimer > 0.1f)
                    {
                        d.waitingToExit = false;
                        m_exitOccupied = true;
                    }
                }
                else
                {
                    float totalLen = glm::length(m_tunnelEnd - m_tunnelStart);
                    if (totalLen < 1e-6f) { d.state = 2; break; }

                    float dist = (m_slideSpeed * d.slideSpeedMul) * d.slideTimer;
                    float t = dist / totalLen;

                    glm::vec3 p = tunnelPoint(t);
                    d.pos = p;
                    d.pos.y += d.size;

                    if (t >= 1.0f)
                    {
                        d.pos = m_tunnelEnd;
                        d.pos.y = m_tunnelEnd.y + d.size;

                        d.waitTimer += (float)dt;
                        if (d.waitTimer > m_exitWait)
                        {
                            d.state = 2;
                            m_exitOccupied = false;

                            std::uniform_real_distribution<float> disExit(-0.08f, 0.35f);
                            d.vel = glm::vec3(disExit(m_gen), 0.10f, disExit(m_gen));
                        }
                    }
                }
            }
        }
        break;

        case 2: // exit -> free fall OR hit ice
        {
            // -------------------------
            // TUNING (vešta?ki bias)
            // -------------------------
            const float overCupExtra = 0.12f;  // proširi XZ “iznad ?aše”
            const float iceHitExtra = 0.20f;  // proširi “sferu” sladoleda
            const float iceYOffset = 0.150f;  // pomeri centar sladoleda naviše (ako ti treba)
            const float iceXOffset = 0.25f;  // pomeri centar sladoleda u X (ako ti treba)

            // lokalni centar za koliziju (NE diraj m_iceCenter trajno)
            glm::vec3 iceC = m_iceCenter + glm::vec3(iceXOffset, iceYOffset, 0.0f);

            // overCup test (XZ)
            float cupR = m_cupRadius + overCupExtra;
            bool overCup =
                glm::length(glm::vec2(d.pos.x - iceC.x, d.pos.z - iceC.z)) <= cupR;

            // 1) nije iznad: pad na pod
            if (!overCup)
            {
                if (d.pos.y <= m_finalGroundY)
                {
                    d.pos.y = m_finalGroundY;
                    d.vel = glm::vec3(0.0f);
                    d.rotSpeed *= 0.3f;
                    d.state = 3;
                    d.attachedToCup = false;
                }
                break;
            }

            // 2) iznad: proveri sladoled (proširen radius)
            glm::vec3 to = d.pos - iceC;
            float dist = glm::length(to);

            float target = m_iceRadius + iceHitExtra; // <-- OVO je glavni “bias”

            if (dist <= target)
            {
                glm::vec3 n = (dist > 1e-6f) ? (to / dist) : glm::vec3(0, 1, 0);

                float rf = rand01(m_gen);
                float depthFactor = rf * rf;
                float sink = depthFactor * 0.4f * d.size;

                d.pos = iceC + n * (target - sink);

                d.vel = glm::vec3(d.vel.x * 0.4f, 0.0f, d.vel.z * 0.4f);
                d.rotSpeed *= 0.5f;

                d.state = 3;

                glm::vec4 lp = m_cupInvM * glm::vec4(d.pos, 1.0f);
                d.cupLocalPos = glm::vec3(lp);
                d.attachedToCup = true;
                d.cupLocalRot = d.rot;
            }
            else
            {
                // iznad ?aše ali promašio sladoled: pad na pod
                if (d.pos.y <= m_finalGroundY)
                {
                    d.pos.y = m_finalGroundY;
                    d.vel = glm::vec3(0.0f);
                    d.rotSpeed *= 0.3f;
                    d.state = 3;
                    d.attachedToCup = false;
                }
            }
        }
        break;


        case 3: // settled
        {
            d.vel.x *= m_friction;
            d.vel.z *= m_friction;
            d.rotSpeed *= m_friction;

            if (std::abs(d.vel.x) < 0.01f) d.vel.x = 0.0f;
            if (std::abs(d.vel.z) < 0.01f) d.vel.z = 0.0f;

            if (std::abs(d.rotSpeed.x) < 0.01f) d.rotSpeed.x = 0.0f;
            if (std::abs(d.rotSpeed.y) < 0.01f) d.rotSpeed.y = 0.0f;
            if (std::abs(d.rotSpeed.z) < 0.01f) d.rotSpeed.z = 0.0f;

            d.pos += glm::vec3(d.vel.x, 0.0f, d.vel.z) * (float)dt;
        }
        break;
        }

        if (d.state == 0 || d.state == 2)
        {
            const float B = 3.0f;
            if (d.pos.x < -B) { d.pos.x = -B; d.vel.x = -d.vel.x * m_damping; }
            if (d.pos.x > B) { d.pos.x = B; d.vel.x = -d.vel.x * m_damping; }
            if (d.pos.z < -B) { d.pos.z = -B; d.vel.z = -d.vel.z * m_damping; }
            if (d.pos.z > B) { d.pos.z = B; d.vel.z = -d.vel.z * m_damping; }
        }

        if (d.pos.y < -10.0f)
            d.active = false;
    }

    m_sprinkles.erase(
        std::remove_if(m_sprinkles.begin(), m_sprinkles.end(),
            [](const Sprinkle& s) { return !s.active; }),
        m_sprinkles.end()
    );
}
static glm::mat3 extractRotationNoScale(const glm::mat4& M)
{
    glm::vec3 x = glm::vec3(M[0]);
    glm::vec3 y = glm::vec3(M[1]);
    glm::vec3 z = glm::vec3(M[2]);

    x = glm::normalize(x);
    y = glm::normalize(y);
    z = glm::normalize(z);

    return glm::mat3(x, y, z);
}
void SprinklesSystem::draw(Shader& sh)
{
    if (m_models.empty()) return;

    glm::mat3 cupR3 = extractRotationNoScale(m_cupM);
    glm::mat4 cupR4 = glm::mat4(1.0f);
    cupR4[0] = glm::vec4(cupR3[0], 0.0f);
    cupR4[1] = glm::vec4(cupR3[1], 0.0f);
    cupR4[2] = glm::vec4(cupR3[2], 0.0f);

    for (const auto& s : m_sprinkles)
    {
        if (!s.active) continue;

        glm::mat4 M(1.0f);

        if (s.state == 3 && s.attachedToCup)
        {
            // 1) world pozicija (kao u tvom kodu koji “dobro rotira”)
            glm::vec3 drawPos = glm::vec3(m_cupM * glm::vec4(s.cupLocalPos, 1.0f));

            // 2) napravi M: T(worldPos) * R(cupRotationNoScale) * R(localSprinkleRot) * S(size)
            M = glm::translate(glm::mat4(1.0f), drawPos);

            // rotacija ?aše (bez scale-a) – da sprinkle prati okretanje globalno
            M = M * cupR4;

            // lokalna rotacija sprinkle-a koju si zapamtio pri lepljenju
            M = glm::rotate(M, s.cupLocalRot.y, glm::vec3(0, 1, 0));
            M = glm::rotate(M, s.cupLocalRot.x, glm::vec3(1, 0, 0));
            M = glm::rotate(M, s.cupLocalRot.z, glm::vec3(0, 0, 1));

            // scale samo sprinkle size (bez dodatnog 1.4 iz cupM)
            M = glm::scale(M, glm::vec3(s.size));
        }
        else
        {
            // stari kod (dobar scale)
            M = glm::translate(M, s.pos);
            M = glm::rotate(M, s.rot.y, glm::vec3(0, 1, 0));
            M = glm::rotate(M, s.rot.x, glm::vec3(1, 0, 0));
            M = glm::rotate(M, s.rot.z, glm::vec3(0, 0, 1));
            M = glm::scale(M, glm::vec3(s.size));
        }

        sh.setMat4("uM", M);

        int idx = s.modelIndex;
        if (idx < 0) idx = 0;
        if (idx >= (int)m_models.size()) idx = (int)m_models.size() - 1;

        m_models[idx]->Draw(sh);
    }
}


