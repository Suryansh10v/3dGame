#include "UIRenderer.h"
#include <cctype>

UIRenderer::~UIRenderer() {
    cleanup();
}

void UIRenderer::cleanup() {
    if (vbo != 0) { glDeleteBuffers(1, &vbo); vbo = 0; }
    if (vao != 0) { glDeleteVertexArrays(1, &vao); vao = 0; }
    verts.clear();
}

void UIRenderer::init() {
    cleanup();
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(UIVertex), (void*)offsetof(UIVertex, pos));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(UIVertex), (void*)offsetof(UIVertex, color));

    glBindVertexArray(0);
}

void UIRenderer::clear() {
    verts.clear();
}

void UIRenderer::addLine(glm::vec2 a, glm::vec2 b, float thickness, glm::vec4 col) {
    glm::vec2 d = b - a;
    float len = glm::length(d);
    if (len < 0.0001f) return;
    glm::vec2 n = glm::vec2(-d.y, d.x) / len * (thickness * 0.5f);

    verts.push_back({a + n, col});
    verts.push_back({a - n, col});
    verts.push_back({b + n, col});

    verts.push_back({b + n, col});
    verts.push_back({a - n, col});
    verts.push_back({b - n, col});
}

void UIRenderer::addRect(float x, float y, float w, float h, glm::vec4 col) {
    glm::vec2 p0(x, y);
    glm::vec2 p1(x + w, y);
    glm::vec2 p2(x + w, y + h);
    glm::vec2 p3(x, y + h);

    verts.push_back({p0, col});
    verts.push_back({p3, col});
    verts.push_back({p1, col});

    verts.push_back({p1, col});
    verts.push_back({p3, col});
    verts.push_back({p2, col});
}

void UIRenderer::addRectOutline(float x, float y, float w, float h, float thick, glm::vec4 col) {
    addLine({x, y}, {x + w, y}, thick, col);
    addLine({x + w, y}, {x + w, y + h}, thick, col);
    addLine({x + w, y + h}, {x, y + h}, thick, col);
    addLine({x, y + h}, {x, y}, thick, col);
}

void UIRenderer::drawChar(char c, float x, float y, float sx, float sy, float thick, glm::vec4 col) {
    c = (char)toupper(c);
    auto S = [&](float x0, float y0, float x1, float y1) {
        addLine({x + x0 * sx, y + y0 * sy}, {x + x1 * sx, y + y1 * sy}, thick, col);
    };

    switch (c) {
        case '0': S(0,0, 1,0); S(1,0, 1,1); S(1,1, 0,1); S(0,1, 0,0); S(1,0, 0,1); break;
        case '1': S(0.5f,0, 0.5f,1); S(0.2f,0.2f, 0.5f,0); S(0.2f,1, 0.8f,1); break;
        case '2': S(0,0, 1,0); S(1,0, 1,0.5f); S(1,0.5f, 0,0.5f); S(0,0.5f, 0,1); S(0,1, 1,1); break;
        case '3': S(0,0, 1,0); S(1,0, 1,0.5f); S(0,0.5f, 1,0.5f); S(1,0.5f, 1,1); S(0,1, 1,1); break;
        case '4': S(0,0, 0,0.5f); S(0,0.5f, 1,0.5f); S(1,0, 1,1); break;
        case '5': S(1,0, 0,0); S(0,0, 0,0.5f); S(0,0.5f, 1,0.5f); S(1,0.5f, 1,1); S(0,1, 1,1); break;
        case '6': S(1,0, 0,0); S(0,0, 0,1); S(0,1, 1,1); S(1,1, 1,0.5f); S(1,0.5f, 0,0.5f); break;
        case '7': S(0,0, 1,0); S(1,0, 0.4f,1); break;
        case '8': S(0,0, 1,0); S(1,0, 1,1); S(1,1, 0,1); S(0,1, 0,0); S(0,0.5f, 1,0.5f); break;
        case '9': S(1,0.5f, 0,0.5f); S(0,0, 0,0.5f); S(0,0, 1,0); S(1,0, 1,1); S(0,1, 1,1); break;
        case ':': addRect(x + 0.35f * sx, y + 0.25f * sy, thick * 1.5f, thick * 1.5f, col);
                  addRect(x + 0.35f * sx, y + 0.65f * sy, thick * 1.5f, thick * 1.5f, col); break;
        case 'X': S(0,0, 1,1); S(1,0, 0,1); break;
        case '/': S(0,1, 1,0); break;
        case '-': S(0.1f,0.5f, 0.9f,0.5f); break;
        case '+': S(0.1f,0.5f, 0.9f,0.5f); S(0.5f,0.1f, 0.5f,0.9f); break;
        case '[': S(0.8f,0, 0.2f,0); S(0.2f,0, 0.2f,1); S(0.2f,1, 0.8f,1); break;
        case ']': S(0.2f,0, 0.8f,0); S(0.8f,0, 0.8f,1); S(0.8f,1, 0.2f,1); break;
        case 'A': S(0,1, 0.5f,0); S(0.5f,0, 1,1); S(0.25f,0.55f, 0.75f,0.55f); break;
        case 'B': S(0,0, 0,1); S(0,0, 0.8f,0); S(0.8f,0, 0.8f,0.5f); S(0,0.5f, 0.8f,0.5f); S(0.8f,0.5f, 0.8f,1); S(0,1, 0.8f,1); break;
        case 'C': S(1,0, 0,0); S(0,0, 0,1); S(0,1, 1,1); break;
        case 'D': S(0,0, 0,1); S(0,0, 0.7f,0.2f); S(0.7f,0.2f, 0.7f,0.8f); S(0.7f,0.8f, 0,1); break;
        case 'E': S(1,0, 0,0); S(0,0, 0,1); S(0,1, 1,1); S(0,0.5f, 0.8f,0.5f); break;
        case 'F': S(1,0, 0,0); S(0,0, 0,1); S(0,0.5f, 0.7f,0.5f); break;
        case 'G': S(1,0, 0,0); S(0,0, 0,1); S(0,1, 1,1); S(1,1, 1,0.5f); S(1,0.5f, 0.5f,0.5f); break;
        case 'H': S(0,0, 0,1); S(1,0, 1,1); S(0,0.5f, 1,0.5f); break;
        case 'I': S(0.5f,0, 0.5f,1); S(0.2f,0, 0.8f,0); S(0.2f,1, 0.8f,1); break;
        case 'J': S(1,0, 1,0.8f); S(1,0.8f, 0.5f,1); S(0.5f,1, 0.1f,0.8f); break;
        case 'K': S(0,0, 0,1); S(1,0, 0,0.5f); S(0,0.5f, 1,1); break;
        case 'L': S(0,0, 0,1); S(0,1, 1,1); break;
        case 'M': S(0,1, 0,0); S(0,0, 0.5f,0.5f); S(0.5f,0.5f, 1,0); S(1,0, 1,1); break;
        case 'N': S(0,1, 0,0); S(0,0, 1,1); S(1,1, 1,0); break;
        case 'O': S(0,0, 1,0); S(1,0, 1,1); S(1,1, 0,1); S(0,1, 0,0); break;
        case 'P': S(0,1, 0,0); S(0,0, 1,0); S(1,0, 1,0.5f); S(1,0.5f, 0,0.5f); break;
        case 'Q': S(0,0, 1,0); S(1,0, 1,1); S(1,1, 0,1); S(0,1, 0,0); S(0.6f,0.6f, 1,1); break;
        case 'R': S(0,1, 0,0); S(0,0, 1,0); S(1,0, 1,0.5f); S(1,0.5f, 0,0.5f); S(0.4f,0.5f, 1,1); break;
        case 'S': S(1,0, 0,0); S(0,0, 0,0.5f); S(0,0.5f, 1,0.5f); S(1,0.5f, 1,1); S(0,1, 1,1); break;
        case 'T': S(0,0, 1,0); S(0.5f,0, 0.5f,1); break;
        case 'U': S(0,0, 0,1); S(0,1, 1,1); S(1,1, 1,0); break;
        case 'V': S(0,0, 0.5f,1); S(0.5f,1, 1,0); break;
        case 'W': S(0,0, 0.25f,1); S(0.25f,1, 0.5f,0.5f); S(0.5f,0.5f, 0.75f,1); S(0.75f,1, 1,0); break;
        case 'Y': S(0,0, 0.5f,0.5f); S(1,0, 0.5f,0.5f); S(0.5f,0.5f, 0.5f,1); break;
        case 'Z': S(0,0, 1,0); S(1,0, 0,1); S(0,1, 1,1); break;
        default: break;
    }
}

void UIRenderer::drawString(const std::string& text, float x, float y, float charW, float charH, float spacing, float thick, glm::vec4 col) {
    float curX = x;
    for (char c : text) {
        if (c != ' ') {
            drawChar(c, curX, y, charW, charH, thick, col);
        }
        curX += charW + spacing;
    }
}

void UIRenderer::render() {
    if (verts.empty()) return;
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(UIVertex), verts.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)verts.size());
    glBindVertexArray(0);
}
