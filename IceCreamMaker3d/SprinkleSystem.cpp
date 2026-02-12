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

    // small spread + controlled forward speed
    std::uniform_real_distribution<float> disSide(-0.02f, 0.02f);
    std::uniform_real_distribution<float> disSpeed(0.35f, 0.55f); // tune

    float sideA = disSide(m_gen);
    float sideB = disSide(m_gen);
    float speed = disSpeed(m_gen);

    s.vel = m_nozzleDir * speed + right * sideA + forward * sideB;

    // Size (increase if your sprinkle mesh is tiny)
    std::uniform_real_distribution<float> disSize(0.2f, 0.2f);
    s.size = disSize(m_gen);

    std::uniform_real_distribution<float> disRotSpeed(-2.0f, 2.0f);
    s.rot = glm::vec3(0.0f);
    s.rotSpeed = glm::vec3(disRotSpeed(m_gen), disRotSpeed(m_gen), disRotSpeed(m_gen));

    std::uniform_int_distribution<int> disFlavor(0, 6);
    int flavor = disFlavor(m_gen);
    switch (flavor) {
    case 0: s.color = glm::vec3(0.09f, 0.09f, 0.639f); break;
    case 1: s.color = glm::vec3(1.0f, 0.0f, 0.0f); break;
    case 2: s.color = glm::vec3(0.0f, 0.73f, 0.0f); break;
    case 3: s.color = glm::vec3(1.0f, 0.9f, 0.0f); break;
    case 4: s.color = glm::vec3(1.0f, 0.75f, 0.80f); break;
    case 5: s.color = glm::vec3(0.30f, 0.192f, 0.078f); break;
    case 6: s.color = glm::vec3(1.0f, 0.94f, 0.86f); break;
    }

    // NEW: pick a model variant
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
            // “Hit entrance” check () 
            // // Use a small sphere around entrance: if passed from above and close enough in XZ. 
            float r = 0.1f; 
            glm::vec2 prevXZ(prevPos.x, prevPos.z); 
            glm::vec2 currXZ(d.pos.x, d.pos.z); 
            glm::vec2 entXZ(m_tunnelEntrance.x, m_tunnelEntrance.z);
            bool closeXZ = glm::length(currXZ - entXZ) <= r; 
            float targetY = 0.25f;
            float eps = 0.005f;   // tolerancija

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

                    float dist = m_slideSpeed * d.slideTimer;
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

        case 2: // exit -> ice
        {
            glm::vec3 to = d.pos - m_iceCenter;
            float dist = glm::length(to);
            float target = m_iceRadius + d.size;

            bool overCup = glm::length(glm::vec2(d.pos.x - m_cupCenter.x, d.pos.z - m_cupCenter.z)) <= m_cupRadius;

            if (overCup && dist <= target)
            {
                glm::vec3 n = (dist > 1e-6f) ? (to / dist) : glm::vec3(0, 1, 0);

                float rf = rand01(m_gen);
                float depthFactor = rf * rf;
                float sink = depthFactor * 0.4f * (d.size);
                d.pos = m_iceCenter + n * (target - sink);

                d.vel = glm::vec3(d.vel.x * 0.4f, 0.0f, d.vel.z * 0.4f);
                d.rotSpeed *= 0.5f;
                d.state = 3;
                // posle što postaviš d.pos u world-u:
                glm::vec4 lp = m_cupInvM * glm::vec4(d.pos, 1.0f);
                d.cupLocalPos = glm::vec3(lp);
                d.attachedToCup = true;

                // opciono: zapamti lokalnu rotaciju (da ne “menja” kad ?aša rotira)
                d.cupLocalRot = d.rot;

            }
            else
            {
                if (d.pos.y - d.size <= m_finalGroundY)
                {
                    d.pos.y = m_finalGroundY + d.size;
                    d.vel.y = 0.0f;
                    d.vel.x *= 0.4f;
                    d.vel.z *= 0.4f;
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

void SprinklesSystem::draw(Shader& sh)
{
    if (m_models.empty()) return;

    for (const auto& s : m_sprinkles)
    {
        if (!s.active) continue;

        glm::vec3 drawPos = s.pos;
        glm::vec3 drawRot = s.rot;

        if (s.state == 3 && s.attachedToCup)
        {
            drawPos = glm::vec3(m_cupM * glm::vec4(s.cupLocalPos, 1.0f));
            drawRot = s.cupLocalRot;
        }

        glm::mat4 M(1.0f);
        M = glm::translate(M, drawPos);
        M = glm::rotate(M, drawRot.y, glm::vec3(0, 1, 0));
        M = glm::rotate(M, drawRot.x, glm::vec3(1, 0, 0));
        M = glm::rotate(M, drawRot.z, glm::vec3(0, 0, 1));
        M = glm::scale(M, glm::vec3(s.size)); // ovo je tvoj scale za sprinkle

        sh.setMat4("uM", M); // <-- OBAVEZNO

        int idx = s.modelIndex;
        if (idx < 0) idx = 0;
        if (idx >= (int)m_models.size()) idx = (int)m_models.size() - 1;

        m_models[idx]->Draw(sh);
    }
}


