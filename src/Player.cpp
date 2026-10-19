#include "Player.h"
#include <cmath>

void Player::reset() {
    pos = glm::vec3(0.0f, 2.5f, 6.0f);
    vel = glm::vec3(0.0f);
    yaw = -90.0f;
    pitch = 0.0f;
    health = 100.0f;
    ammo = 30;
    reserveAmmo = 90;
    reloadTimer = 0.0f;
    fireCooldown = 0.0f;
    isGrounded = false;
    inAntigrav = false;
    walkTimer = 0.0f;
    gunRecoil = 0.0f;
    gunPitchSway = 0.0f;
    gunYawSway = 0.0f;
    muzzleFlashTimer = 0.0f;
}

glm::vec3 Player::getCameraPos() const {
    return pos + glm::vec3(0.0f, eyeHeight, 0.0f);
}

glm::vec3 Player::getForward() const {
    float rYaw = glm::radians(yaw);
    float rPitch = glm::radians(pitch);
    glm::vec3 f;
    f.x = cos(rYaw) * cos(rPitch);
    f.y = sin(rPitch);
    f.z = sin(rYaw) * cos(rPitch);
    return glm::normalize(f);
}

glm::vec3 Player::getRight() const {
    return glm::normalize(glm::cross(getForward(), glm::vec3(0.0f, 1.0f, 0.0f)));
}

AABB Player::getAABB() const {
    glm::vec3 half(0.45f, eyeHeight * 0.5f, 0.45f);
    glm::vec3 center = pos + glm::vec3(0.0f, eyeHeight * 0.5f, 0.0f);
    return {center - half, center + half};
}

void Player::reload() {
    if (reloadTimer > 0.0f || ammo == maxAmmo || reserveAmmo <= 0) return;
    reloadTimer = reloadDuration;
}
