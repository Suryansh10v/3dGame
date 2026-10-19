#pragma once

#include <glm/glm.hpp>
#include <vector>

// 3D Vertex representation
struct Vertex3D {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
};

// 2D UI Vertex representation
struct UIVertex {
    glm::vec2 pos;
    glm::vec4 color;
};

// Axis-Aligned Bounding Box for collisions
struct AABB {
    glm::vec3 minBound;
    glm::vec3 maxBound;

    bool contains(const glm::vec3& p) const {
        return (p.x >= minBound.x && p.x <= maxBound.x &&
                p.y >= minBound.y && p.y <= maxBound.y &&
                p.z >= minBound.z && p.z <= maxBound.z);
    }

    bool intersects(const AABB& o) const {
        return (minBound.x <= o.maxBound.x && maxBound.x >= o.minBound.x) &&
               (minBound.y <= o.maxBound.y && maxBound.y >= o.minBound.y) &&
               (minBound.z <= o.maxBound.z && maxBound.z >= o.minBound.z);
    }
};

// Platform geometry and collider
struct Platform {
    glm::vec3 pos;
    glm::vec3 size;
    glm::vec3 color;
    glm::vec3 trimColor;

    AABB getAABB() const {
        glm::vec3 half = size * 0.5f;
        return {pos - half, pos + half};
    }
};

// Fast ballistic laser projectile
struct Projectile {
    glm::vec3 pos;
    glm::vec3 vel;
    float radius;
    float life;
    bool isEnemy;
    glm::vec3 color;
};

// Visual spark and explosion particles
struct Particle {
    glm::vec3 pos;
    glm::vec3 vel;
    glm::vec3 color;
    float size;
    float life;
    float maxLife;
};

// Floating Zero-G debris prop
struct FloatingProp {
    glm::vec3 pos;
    glm::vec3 basePos;
    glm::vec3 rotAxis;
    float rotSpeed;
    float currentRot;
    glm::vec3 scale;
    glm::vec3 color;
    glm::vec3 emissive;
    float floatOffset;
};

// Autonomous hostile drone
struct Enemy {
    glm::vec3 pos;
    glm::vec3 vel;
    float health = 50.0f;
    float shootTimer = 1.8f;
    float hoverPhase = 0.0f;
    float hitFlashTimer = 0.0f;
    bool alive = true;

    AABB getAABB() const {
        return {pos - glm::vec3(0.9f), pos + glm::vec3(0.9f)};
    }
};
