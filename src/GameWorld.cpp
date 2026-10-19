#include "GameWorld.h"
#include <glm/gtc/constants.hpp>
#include <cmath>
#include <algorithm>

GameWorld::GameWorld() : rng(1337) {
    initArena();
}

float GameWorld::randRange(float a, float b) {
    std::uniform_real_distribution<float> dist(a, b);
    return dist(rng);
}

void GameWorld::spawnExplosion(const glm::vec3& p, const glm::vec3& col, int count) {
    for (int i = 0; i < count; ++i) {
        Particle part;
        part.pos = p;
        part.vel = glm::vec3(randRange(-6.0f, 6.0f), randRange(-4.0f, 8.0f), randRange(-6.0f, 6.0f));
        part.color = col + glm::vec3(randRange(-0.1f, 0.2f));
        part.size = randRange(0.08f, 0.22f);
        part.life = randRange(0.4f, 0.85f);
        part.maxLife = part.life;
        particles.push_back(part);
    }
}

void GameWorld::initArena() {
    platforms.clear();
    floatingProps.clear();

    // 1. Central Main Hub Platform (Lower)
    platforms.push_back({
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(26.0f, 1.2f, 26.0f),
        glm::vec3(0.05f, 0.06f, 0.09f),
        glm::vec3(0.10f, 0.85f, 1.0f) // Cyan trim
    });

    // 2. North Elevated Platform
    platforms.push_back({
        glm::vec3(0.0f, 6.0f, -20.0f),
        glm::vec3(16.0f, 1.0f, 12.0f),
        glm::vec3(0.05f, 0.06f, 0.09f),
        glm::vec3(1.0f, 0.40f, 0.08f) // Orange trim
    });

    // 3. South Elevated Platform
    platforms.push_back({
        glm::vec3(0.0f, 7.5f, 20.0f),
        glm::vec3(16.0f, 1.0f, 12.0f),
        glm::vec3(0.05f, 0.06f, 0.09f),
        glm::vec3(0.10f, 0.85f, 1.0f)
    });

    // 4. West High Platform
    platforms.push_back({
        glm::vec3(-21.0f, 11.5f, 0.0f),
        glm::vec3(12.0f, 1.0f, 14.0f),
        glm::vec3(0.05f, 0.06f, 0.09f),
        glm::vec3(0.80f, 0.20f, 1.0f) // Purple trim
    });

    // 5. East High Platform
    platforms.push_back({
        glm::vec3(21.0f, 11.5f, 0.0f),
        glm::vec3(12.0f, 1.0f, 14.0f),
        glm::vec3(0.05f, 0.06f, 0.09f),
        glm::vec3(0.80f, 0.20f, 1.0f)
    });

    // 6. Corner Stepping Platforms
    platforms.push_back({ glm::vec3(-14.0f, 3.8f, -14.0f), glm::vec3(7.0f, 0.8f, 7.0f), glm::vec3(0.06f), glm::vec3(0.1f, 0.8f, 1.0f) });
    platforms.push_back({ glm::vec3( 14.0f, 3.8f, -14.0f), glm::vec3(7.0f, 0.8f, 7.0f), glm::vec3(0.06f), glm::vec3(1.0f, 0.4f, 0.1f) });
    platforms.push_back({ glm::vec3(-14.0f, 4.2f,  14.0f), glm::vec3(7.0f, 0.8f, 7.0f), glm::vec3(0.06f), glm::vec3(1.0f, 0.4f, 0.1f) });
    platforms.push_back({ glm::vec3( 14.0f, 4.2f,  14.0f), glm::vec3(7.0f, 0.8f, 7.0f), glm::vec3(0.06f), glm::vec3(0.1f, 0.8f, 1.0f) });

    // Floating Zero-G Props around Central Antigravity Core
    // Anchor Cube (as seen in screenshot)
    floatingProps.push_back({
        glm::vec3(-7.5f, 14.5f, -6.5f),
        glm::vec3(-7.5f, 14.5f, -6.5f),
        glm::vec3(0.3f, 1.0f, 0.2f),
        0.35f, 0.0f,
        glm::vec3(3.6f),
        glm::vec3(0.08f, 0.09f, 0.12f),
        glm::vec3(0.1f, 0.85f, 1.0f),
        0.0f
    });

    // Debris cubes & geometric chunks
    for (int i = 0; i < 18; ++i) {
        float angle = (float)i * (glm::two_pi<float>() / 18.0f);
        float dist = randRange(5.5f, 14.5f);
        float h = randRange(8.0f, 16.5f);
        glm::vec3 p(cos(angle) * dist, h, sin(angle) * dist);
        float s = randRange(0.4f, 1.2f);
        floatingProps.push_back({
            p, p,
            glm::normalize(glm::vec3(randRange(-1,1), randRange(-1,1), randRange(-1,1))),
            randRange(0.4f, 1.4f),
            randRange(0.0f, 3.14f),
            glm::vec3(s),
            glm::vec3(0.06f, 0.07f, 0.09f),
            (i % 3 == 0) ? glm::vec3(0.1f, 0.7f, 1.0f) : glm::vec3(0.0f),
            randRange(0.0f, 6.28f)
        });
    }
}

void GameWorld::reset() {
    player.reset();
    game.reset();
    enemies.clear();
    projectiles.clear();
    particles.clear();
    initArena();
}

void GameWorld::firePlayerLaser() {
    if (player.ammo <= 0 || player.fireCooldown > 0.0f || player.reloadTimer > 0.0f) {
        if (player.ammo <= 0) player.reload();
        return;
    }

    player.ammo--;
    player.fireCooldown = Player::fireInterval;
    player.gunRecoil = 1.0f;
    player.muzzleFlashTimer = 0.07f;

    glm::vec3 camPos = player.getCameraPos();
    glm::vec3 fwd = player.getForward();
    glm::vec3 right = player.getRight();

    // Spawn projectile aligned with crosshair ray
    Projectile p;
    p.pos = camPos + fwd * 0.5f + right * 0.16f - glm::vec3(0.0f, 0.12f, 0.0f);
    p.vel = fwd * 72.0f;
    p.radius = 0.22f;
    p.life = 2.2f;
    p.isEnemy = false;
    p.color = glm::vec3(0.15f, 0.90f, 1.0f); // Bright Cyan Laser
    projectiles.push_back(p);
}

void GameWorld::updateSimulation(float dt, const bool keys[1024], bool mouseLeftPressed, bool cursorCaptured, float currentTime) {
    if (game.gameOver) return;

    // 1. Antigravity Proximity Check
    float distToCore = glm::length(player.pos - Constants::ARENA_CORE_POS);
    player.inAntigrav = (distToCore < Constants::ARENA_CORE_RADIUS);

    // 2. Player Input Movement
    glm::vec3 fwd = player.getForward();
    glm::vec3 right = player.getRight();

    // Flatten for horizontal walking
    glm::vec3 moveDir(0.0f);
    glm::vec3 horizFwd = glm::normalize(glm::vec3(fwd.x, 0.0f, fwd.z));
    glm::vec3 horizRight = glm::normalize(glm::vec3(right.x, 0.0f, right.z));

    if (keys[GLFW_KEY_W]) moveDir += horizFwd;
    if (keys[GLFW_KEY_S]) moveDir -= horizFwd;
    if (keys[GLFW_KEY_D]) moveDir += horizRight;
    if (keys[GLFW_KEY_A]) moveDir -= horizRight;

    bool isMoving = (glm::length(moveDir) > 0.01f);
    if (isMoving) moveDir = glm::normalize(moveDir);

    // 3. Antigravity Physics vs Normal Physics
    if (player.inAntigrav) {
        // Zero-G / Antigravity floaty dynamics:
        float targetYGravity = -2.2f;
        player.vel.y += targetYGravity * dt;

        // Space = Upward thrusters; Shift/Ctrl = Downward thrusters
        if (keys[GLFW_KEY_SPACE]) {
            player.vel.y += 14.0f * dt;
            if (player.vel.y > 10.0f) player.vel.y = 10.0f;
        }
        if (keys[GLFW_KEY_LEFT_SHIFT] || keys[GLFW_KEY_LEFT_CONTROL]) {
            player.vel.y -= 14.0f * dt;
            if (player.vel.y < -10.0f) player.vel.y = -10.0f;
        }

        // Floaty 3D momentum with soft drag
        player.vel.x += moveDir.x * Player::moveSpeed * 1.5f * dt;
        player.vel.z += moveDir.z * Player::moveSpeed * 1.5f * dt;
        player.vel.x *= std::pow(0.85f, dt * 60.0f);
        player.vel.z *= std::pow(0.85f, dt * 60.0f);
        player.vel.y *= std::pow(0.92f, dt * 60.0f);

        // Visual zero-G floating dust particles
        if (randRange(0.0f, 1.0f) < 0.2f) {
            Particle part;
            part.pos = player.pos + glm::vec3(randRange(-4,4), randRange(-2,4), randRange(-4,4));
            part.vel = glm::vec3(randRange(-0.4f,0.4f), randRange(-0.2f,0.6f), randRange(-0.4f,0.4f));
            part.color = glm::vec3(0.3f, 0.6f, 1.0f);
            part.size = 0.04f;
            part.life = 1.0f;
            part.maxLife = 1.0f;
            particles.push_back(part);
        }
    } else {
        // Standard grounded FPS movement with gravity
        player.vel.y += Player::normalGravity * dt;

        if (player.isGrounded) {
            player.vel.x = moveDir.x * Player::moveSpeed;
            player.vel.z = moveDir.z * Player::moveSpeed;
            if (keys[GLFW_KEY_SPACE]) {
                player.vel.y = Player::normalJumpSpeed;
                player.isGrounded = false;
            }
        } else {
            // Air control
            player.vel.x += moveDir.x * Player::moveSpeed * 1.2f * dt;
            player.vel.z += moveDir.z * Player::moveSpeed * 1.2f * dt;
            player.vel.x *= std::pow(0.92f, dt * 60.0f);
            player.vel.z *= std::pow(0.92f, dt * 60.0f);
        }
    }

    // 4. Player Platform Collision & Position Integration
    glm::vec3 nextPos = player.pos + player.vel * dt;
    player.isGrounded = false;

    for (const auto& plat : platforms) {
        AABB box = plat.getAABB();
        // Check landing on platform top
        float footOld = player.pos.y;
        float footNew = nextPos.y;
        if (nextPos.x >= box.minBound.x - 0.4f && nextPos.x <= box.maxBound.x + 0.4f &&
            nextPos.z >= box.minBound.z - 0.4f && nextPos.z <= box.maxBound.z + 0.4f) {
            if (footOld >= box.maxBound.y - 0.1f && footNew <= box.maxBound.y) {
                nextPos.y = box.maxBound.y;
                player.vel.y = 0.0f;
                player.isGrounded = true;
            }
        }
    }

    // Void fall boundary: antigravity emergency repulsion / damage
    if (nextPos.y < -12.0f) {
        nextPos.y = -10.0f;
        player.vel.y = 19.0f; // launch back upward
        player.health -= 12.0f;
        game.damageFlash = 0.5f;
        if (player.health <= 0.0f) {
            player.health = 0.0f;
            game.gameOver = true;
        }
    }

    player.pos = nextPos;

    // 5. Weapon viewmodel sway, bobbing & recoil update
    if (isMoving && player.isGrounded) {
        player.walkTimer += dt * 10.0f;
    } else {
        player.walkTimer = 0.0f;
    }

    player.gunRecoil = std::max(0.0f, player.gunRecoil - dt * 8.0f);
    player.gunPitchSway *= std::pow(0.85f, dt * 60.0f);
    player.gunYawSway *= std::pow(0.85f, dt * 60.0f);
    if (player.muzzleFlashTimer > 0.0f) player.muzzleFlashTimer -= dt;

    if (player.fireCooldown > 0.0f) player.fireCooldown -= dt;

    // Reload timer
    if (player.reloadTimer > 0.0f) {
        player.reloadTimer -= dt;
        if (player.reloadTimer <= 0.0f) {
            int needed = player.maxAmmo - player.ammo;
            int take = std::min(needed, player.reserveAmmo);
            player.ammo += take;
            player.reserveAmmo -= take;
            player.reloadTimer = 0.0f;
        }
    }

    // Continuous fire when holding left click
    if (mouseLeftPressed && cursorCaptured) {
        firePlayerLaser();
    }

    // 6. Floating Props Animation (orbit & bobbing)
    for (auto& prop : floatingProps) {
        prop.currentRot += prop.rotSpeed * dt;
        float bob = std::sin(currentTime * 1.8f + prop.floatOffset) * 0.4f;
        prop.pos.y = prop.basePos.y + bob;
    }

    // 7. Enemy AI & Spawning
    game.enemySpawnTimer -= dt;
    if (game.enemySpawnTimer <= 0.0f && enemies.size() < 7) {
        game.enemySpawnTimer = randRange(3.5f, 5.5f);
        float angle = randRange(0.0f, glm::two_pi<float>());
        float dist = randRange(16.0f, 26.0f);
        Enemy e;
        e.pos = glm::vec3(cos(angle) * dist, randRange(4.0f, 15.0f), sin(angle) * dist);
        e.vel = glm::vec3(0.0f);
        e.hoverPhase = randRange(0.0f, 6.28f);
        e.shootTimer = randRange(1.5f, 3.0f);
        enemies.push_back(e);
    }

    for (auto& e : enemies) {
        if (!e.alive) continue;
        if (e.hitFlashTimer > 0.0f) e.hitFlashTimer -= dt;

        // Hover & steer towards player
        glm::vec3 toPlayer = player.getCameraPos() - e.pos;
        float dist = glm::length(toPlayer);
        glm::vec3 dir = (dist > 0.01f) ? (toPlayer / dist) : glm::vec3(0,0,1);

        // Desired distance is ~12-18 units
        glm::vec3 targetVel(0.0f);
        if (dist > 18.0f) {
            targetVel = dir * 4.5f;
        } else if (dist < 8.0f) {
            targetVel = -dir * 3.5f;
        } else {
            // Strafe orbit
            glm::vec3 strafe = glm::cross(dir, glm::vec3(0, 1, 0)) * 2.8f;
            targetVel = strafe;
        }

        // Add vertical hovering bob
        float bobY = std::sin(currentTime * 2.5f + e.hoverPhase) * 1.8f;
        targetVel.y += bobY;

        e.vel += (targetVel - e.vel) * 3.0f * dt;
        e.pos += e.vel * dt;

        // Enemy Shooting
        e.shootTimer -= dt;
        if (e.shootTimer <= 0.0f) {
            e.shootTimer = randRange(2.0f, 3.5f);
            // Fire purple laser at player
            Projectile ep;
            ep.pos = e.pos + dir * 0.8f;
            ep.vel = dir * 26.0f;
            ep.radius = 0.28f;
            ep.life = 3.5f;
            ep.isEnemy = true;
            ep.color = glm::vec3(0.95f, 0.15f, 0.85f); // Neon Purple Laser
            projectiles.push_back(ep);
        }
    }

    // 8. Projectile Simulation & Collision Checks
    for (auto& p : projectiles) {
        p.pos += p.vel * dt;
        p.life -= dt;

        if (p.life <= 0.0f) continue;

        if (!p.isEnemy) {
            // Player bullet vs Enemies
            for (auto& e : enemies) {
                if (!e.alive) continue;
                if (glm::length(p.pos - e.pos) < (p.radius + 1.1f)) {
                    e.health -= 30.0f;
                    e.hitFlashTimer = 0.15f;
                    p.life = -1.0f; // destroy bullet
                    spawnExplosion(p.pos, glm::vec3(0.2f, 0.9f, 1.0f), 10);

                    if (e.health <= 0.0f) {
                        e.alive = false;
                        game.addHit(100);
                        spawnExplosion(e.pos, glm::vec3(1.0f, 0.3f, 0.1f), 30);
                        spawnExplosion(e.pos, glm::vec3(0.8f, 0.1f, 1.0f), 20);
                    }
                    break;
                }
            }
        } else {
            // Enemy bullet vs Player
            if (glm::length(p.pos - player.getCameraPos()) < (p.radius + 0.65f)) {
                player.health -= 15.0f;
                game.damageFlash = 0.5f;
                p.life = -1.0f;
                spawnExplosion(p.pos, glm::vec3(0.9f, 0.1f, 0.7f), 16);

                if (player.health <= 0.0f) {
                    player.health = 0.0f;
                    game.gameOver = true;
                }
            }
        }

        // Bullet vs Platforms
        for (const auto& plat : platforms) {
            if (plat.getAABB().contains(p.pos)) {
                p.life = -1.0f;
                spawnExplosion(p.pos, p.color, 8);
                break;
            }
        }
    }

    // Remove expired projectiles
    projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(),
        [](const Projectile& p) { return p.life <= 0.0f; }), projectiles.end());

    // Remove dead enemies
    enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
        [](const Enemy& e) { return !e.alive; }), enemies.end());

    // 9. Particles Update
    for (auto& part : particles) {
        part.pos += part.vel * dt;
        part.life -= dt;
    }
    particles.erase(std::remove_if(particles.begin(), particles.end(),
        [](const Particle& part) { return part.life <= 0.0f; }), particles.end());

    // 10. Streak & Damage Flash Update
    game.updateStreak(dt);
    if (game.damageFlash > 0.0f) {
        game.damageFlash = std::max(0.0f, game.damageFlash - dt * 2.0f);
    }
}
