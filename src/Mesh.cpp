#include "Mesh.h"
#include <glm/gtc/constants.hpp>
#include <cmath>

Mesh3D::~Mesh3D() {
    cleanup();
}

Mesh3D::Mesh3D(Mesh3D&& other) noexcept 
    : vao(other.vao), vbo(other.vbo), ebo(other.ebo), indexCount(other.indexCount) {
    other.vao = 0;
    other.vbo = 0;
    other.ebo = 0;
    other.indexCount = 0;
}

Mesh3D& Mesh3D::operator=(Mesh3D&& other) noexcept {
    if (this != &other) {
        cleanup();
        vao = other.vao;
        vbo = other.vbo;
        ebo = other.ebo;
        indexCount = other.indexCount;
        other.vao = 0;
        other.vbo = 0;
        other.ebo = 0;
        other.indexCount = 0;
    }
    return *this;
}

void Mesh3D::cleanup() {
    if (ebo != 0) { glDeleteBuffers(1, &ebo); ebo = 0; }
    if (vbo != 0) { glDeleteBuffers(1, &vbo); vbo = 0; }
    if (vao != 0) { glDeleteVertexArrays(1, &vao); vao = 0; }
    indexCount = 0;
}

void Mesh3D::setup(const std::vector<Vertex3D>& verts, const std::vector<GLuint>& inds) {
    cleanup();
    indexCount = static_cast<GLsizei>(inds.size());

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex3D), verts.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size() * sizeof(GLuint), inds.data(), GL_STATIC_DRAW);

    // aPos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, pos));
    // aNormal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, normal));
    // aTexCoords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, uv));

    glBindVertexArray(0);
}

void Mesh3D::draw() const {
    if (vao == 0 || indexCount == 0) return;
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

Mesh3D Mesh3D::createCube() {
    std::vector<Vertex3D> verts = {
        // Front (+Z)
        {{-0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 0.0f}},
        {{ 0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 1.0f}},
        {{-0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 1.0f}},
        // Back (-Z)
        {{ 0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 0.0f}},
        {{-0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 0.0f}},
        {{-0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 1.0f}},
        {{ 0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 1.0f}},
        // Left (-X)
        {{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},
        {{-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}},
        {{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}},
        {{-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},
        // Right (+X)
        {{ 0.5f, -0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}},
        {{ 0.5f,  0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}},
        {{ 0.5f,  0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},
        // Top (+Y)
        {{-0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, {0.0f, 0.0f}},
        {{ 0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 0.0f}},
        {{ 0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 1.0f}},
        {{-0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, {0.0f, 1.0f}},
        // Bottom (-Y)
        {{-0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 0.0f}},
        {{ 0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 1.0f}},
        {{-0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, {0.0f, 1.0f}},
    };

    std::vector<GLuint> inds;
    for (int i = 0; i < 6; ++i) {
        GLuint base = i * 4;
        inds.push_back(base + 0);
        inds.push_back(base + 1);
        inds.push_back(base + 2);
        inds.push_back(base + 2);
        inds.push_back(base + 3);
        inds.push_back(base + 0);
    }

    Mesh3D m;
    m.setup(verts, inds);
    return m;
}

Mesh3D Mesh3D::createSphere(int stacks, int sectors) {
    std::vector<Vertex3D> verts;
    std::vector<GLuint> inds;

    for (int i = 0; i <= stacks; ++i) {
        float phi = glm::pi<float>() * (float)i / (float)stacks;
        for (int j = 0; j <= sectors; ++j) {
            float theta = 2.0f * glm::pi<float>() * (float)j / (float)sectors;
            float x = std::sin(phi) * std::cos(theta);
            float y = std::cos(phi);
            float z = std::sin(phi) * std::sin(theta);
            glm::vec3 p(x, y, z);
            verts.push_back({p, p, {(float)j / sectors, (float)i / stacks}});
        }
    }

    for (int i = 0; i < stacks; ++i) {
        int k1 = i * (sectors + 1);
        int k2 = k1 + sectors + 1;
        for (int j = 0; j < sectors; ++j, ++k1, ++k2) {
            if (i != 0) {
                inds.push_back(k1);
                inds.push_back(k2);
                inds.push_back(k1 + 1);
            }
            if (i != (stacks - 1)) {
                inds.push_back(k1 + 1);
                inds.push_back(k2);
                inds.push_back(k2 + 1);
            }
        }
    }

    Mesh3D m;
    m.setup(verts, inds);
    return m;
}

Mesh3D Mesh3D::createOctahedron() {
    std::vector<glm::vec3> p = {
        { 0,  1,  0}, // 0 Top
        { 1,  0,  0}, // 1 Right
        { 0,  0,  1}, // 2 Front
        {-1,  0,  0}, // 3 Left
        { 0,  0, -1}, // 4 Back
        { 0, -1,  0}  // 5 Bottom
    };
    int faces[8][3] = {
        {0, 2, 1}, {0, 3, 2}, {0, 4, 3}, {0, 1, 4},
        {5, 1, 2}, {5, 2, 3}, {5, 3, 4}, {5, 4, 1}
    };
    std::vector<Vertex3D> verts;
    std::vector<GLuint> inds;

    for (int i = 0; i < 8; ++i) {
        glm::vec3 v0 = p[faces[i][0]];
        glm::vec3 v1 = p[faces[i][1]];
        glm::vec3 v2 = p[faces[i][2]];
        glm::vec3 norm = glm::normalize(glm::cross(v1 - v0, v2 - v0));
        GLuint base = (GLuint)verts.size();
        verts.push_back({v0, norm, {0, 0}});
        verts.push_back({v1, norm, {1, 0}});
        verts.push_back({v2, norm, {0.5f, 1}});
        inds.push_back(base);
        inds.push_back(base + 1);
        inds.push_back(base + 2);
    }

    Mesh3D m;
    m.setup(verts, inds);
    return m;
}
