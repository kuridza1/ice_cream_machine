// SprinkleSystem.h
#pragma once

#include <vector>
#include <random>
#include <glm/glm.hpp>

#include "shader.hpp"
#include "model.hpp"

struct Sprinkle
{
    glm::vec3 pos{0.0f};
    glm::vec3 vel{0.0f};

    glm::vec3 rot{0.0f};        // Euler radians
    glm::vec3 rotSpeed{0.0f};   // rad/s

    glm::vec3 color{1.0f};
    float size = 0.05f;

    int state = 0;              // 0 free -> entrance, 1 slide, 2 exit fall, 3 settled
    float slideTimer = 0.0f;
    float waitTimer = 0.0f;
    bool waitingToExit = false;

    bool attachedToCup = false;
    glm::vec3 cupLocalPos{ 0.0f };  // pozicija u koordinatama ?aše
    glm::vec3 cupLocalRot{ 0.0f };

    int modelIndex = 0;         // NEW: which sprinkle model to render

    bool active = true;
};

class SprinklesSystem
{
public:
    SprinklesSystem();

    void init();
    void reset();

    void setOpen(bool open) { m_open = open; }
    bool isOpen() const { return m_open; }

    // NEW: set multiple model variants; if empty -> draw does nothing
    void setModels(const std::vector<Model*>& models);

    void setNozzle(const glm::vec3& pos, const glm::vec3& dir, float spawnDiscRadius);
    void setTunnel(const glm::vec3& entrancePos, const glm::vec3& startPos, const glm::vec3& endPos);
    void setCupRegion(const glm::vec3& center, float radius);
    void setIceCollider(const glm::vec3& center, float radius);

    void update(double dt);

    // NEW: no longer takes a model; picks from setModels()
    void draw(Shader& sh);

    // knobs
    void setSpawnRate(float r) { m_spawnRate = r; }
    void setMaxCount(int n) { m_maxCount = n; }
    void setFinalGroundY(float y) { m_finalGroundY = y; }
    void setGravity(float g) { m_gravity = g; }
    void setCupMatrix(const glm::mat4& cupWorld);


private:
    void spawn();
    glm::vec3 tunnelPoint(float t) const;

private:
    std::random_device m_rd;
    std::mt19937 m_gen;

    std::vector<Sprinkle> m_sprinkles;
    std::vector<Model*>   m_models;

    bool  m_open = false;

    float m_spawnRate = 3.0f;
    float m_spawnAccum = 0.2f;
    int   m_maxCount = 400;

    float m_gravity = -2.0f;      // sporije, jer je scena mala
    float m_damping = 0.2f;
    float m_friction = 0.88f;
    float m_finalGroundY = -1.3f; // jer base translate ide oko -1

    glm::vec3 m_nozzlePos{ 0.0f };
    glm::vec3 m_nozzleDir{ 0.0f, -1.0f, 0.0f };
    float m_spawnDiscRadius = 0.006f;   // jako mali disk

    glm::mat4 m_cupM{ 1.0f };
    glm::mat4 m_cupInvM{ 1.0f };

    glm::vec3 m_tunnelEntrance{ 0.0f };
    glm::vec3 m_tunnelStart{ 0.0f };
    glm::vec3 m_tunnelEnd{ 0.0f };

    float m_slideSpeed = 0.8f;   // sporije klizanje
    float m_exitWait = 0.05f;

    bool m_exitOccupied = false;

    glm::vec3 m_cupCenter{ 0.0f };
    float m_cupRadius = 0.6;    // realno u odnosu na model

    glm::vec3 m_iceCenter{ 0.0f };
    float m_iceRadius = 0.5f;    // znatno manje od 0.53


};
