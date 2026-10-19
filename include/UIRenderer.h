#pragma once

#include "OpenGLHeaders.h"
#include "Types.h"
#include <vector>
#include <string>

class UIRenderer {
public:
    GLuint vao = 0;
    GLuint vbo = 0;
    std::vector<UIVertex> verts;

    UIRenderer() = default;
    ~UIRenderer();

    UIRenderer(const UIRenderer&) = delete;
    UIRenderer& operator=(const UIRenderer&) = delete;

    void init();
    void clear();
    void cleanup();

    void addLine(glm::vec2 a, glm::vec2 b, float thickness, glm::vec4 col);
    void addRect(float x, float y, float w, float h, glm::vec4 col);
    void addRectOutline(float x, float y, float w, float h, float thick, glm::vec4 col);
    void drawChar(char c, float x, float y, float sx, float sy, float thick, glm::vec4 col);
    void drawString(const std::string& text, float x, float y, float charW, float charH, float spacing, float thick, glm::vec4 col);
    void render();
};
