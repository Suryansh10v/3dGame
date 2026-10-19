#include "GameState.h"

void GameState::reset() {
    score = 0;
    streakCount = 0;
    streakMultiplier = 1;
    streakTimer = 0.0f;
    damageFlash = 0.0f;
    gameOver = false;
    victory = false;
    enemySpawnTimer = 2.0f;
    enemiesKilled = 0;
}

void GameState::addHit(int points) {
    streakCount++;
    streakTimer = maxStreakTime;
    if (streakCount >= 9) streakMultiplier = 5;
    else if (streakCount >= 5) streakMultiplier = 3;
    else if (streakCount >= 2) streakMultiplier = 2;
    else streakMultiplier = 1;

    score += points * streakMultiplier;
    enemiesKilled++;
}

void GameState::updateStreak(float dt) {
    if (streakTimer > 0.0f) {
        streakTimer -= dt;
        if (streakTimer <= 0.0f) {
            streakTimer = 0.0f;
            streakCount = 0;
            streakMultiplier = 1;
        }
    }
}
