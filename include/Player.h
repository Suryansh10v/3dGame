#pragma once

#include "Types.h"
#include <glm/glm.hpp>

class Player {
public:
    glm::vec3 pos = glm::vec3(0.0f, 2.5f, 6.0f);
    glm::vec3 vel = glm::vec3(0.0f);

    float yaw = -90.0f;
    float pitch = 0.0f;

    float eyeHeight = 1.70f;
    float health = 100.0f;
    float maxHealth = 100.0f;

    int ammo = 30;
    int maxAmmo = 30;
    int reserveAmmo = 90;
    float reloadTimer = 0.0f;
    static constexpr float reloadDuration = 1.25f;

    float fireCooldown = 0.0f;
    static constexpr float fireInterval = 0.14f;

    bool isGrounded = false;
    bool inAntigrav = false;

    // Gun viewmodel dynamics
    float walkTimer = 0.0f;
    float gunRecoil = 0.0f;
    float gunPitchSway = 0.0f;
    float gunYawSway = 0.0f;
    float muzzleFlashTimer = 0.0f;

    // Movement speeds
    static constexpr float moveSpeed = 13.5f;
    static constexpr float normalGravity = -25.0f;
    static constexpr float normalJumpSpeed = 9.2f;

    Player() = default;

    void reset();
    glm::vec3 getCameraPos() const;
    glm::vec3 getForward() const;
    glm::vec3 getRight() const;
    AABB getAABB() const;
    void reload();
};
