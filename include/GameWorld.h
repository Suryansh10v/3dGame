#pragma once

#include "OpenGLHeaders.h"
#include "Player.h"
#include "GameState.h"
#include "Types.h"
#include "Constants.h"
#include <vector>
#include <random>

class GameWorld {
public:
    Player player;
    GameState game;

    std::vector<Platform> platforms;
    std::vector<FloatingProp> floatingProps;
    std::vector<Enemy> enemies;
    std::vector<Projectile> projectiles;
    std::vector<Particle> particles;

    GameWorld();

    void initArena();
    void reset();
    void spawnExplosion(const glm::vec3& p, const glm::vec3& col, int count = 22);
    void firePlayerLaser();
    void updateSimulation(float dt, const bool keys[1024], bool mouseLeftPressed, bool cursorCaptured, float currentTime);

private:
    std::mt19937 rng;
    float randRange(float a, float b);
};
