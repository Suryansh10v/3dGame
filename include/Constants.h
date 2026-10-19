#pragma once

#include <glm/glm.hpp>

// Arena geometry and physics constants
namespace Constants {
    inline const glm::vec3 ARENA_CORE_POS = glm::vec3(0.0f, 10.5f, 0.0f);
    inline constexpr float ARENA_CORE_RADIUS = 16.5f;

    inline constexpr int DEFAULT_WINDOW_WIDTH = 1280;
    inline constexpr int DEFAULT_WINDOW_HEIGHT = 720;
    inline constexpr const char* WINDOW_TITLE = "Chrono-Dash: Antigravity Arena";
}
