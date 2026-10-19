#pragma once

struct GameState {
    int score = 0;
    int streakCount = 0;
    int streakMultiplier = 1;
    float streakTimer = 0.0f;
    static constexpr float maxStreakTime = 4.5f;

    float damageFlash = 0.0f;
    bool gameOver = false;
    bool victory = false;

    float enemySpawnTimer = 2.0f;
    int enemiesKilled = 0;

    void reset();
    void addHit(int points = 100);
    void updateStreak(float dt);
};
