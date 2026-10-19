#pragma once

#include "OpenGLHeaders.h"
#include "Types.h"
#include <vector>

class Mesh3D {
public:
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    GLsizei indexCount = 0;

    Mesh3D() = default;
    ~Mesh3D();

    // Disable copy to avoid double freeing OpenGL resources, allow move
    Mesh3D(const Mesh3D&) = delete;
    Mesh3D& operator=(const Mesh3D&) = delete;
    Mesh3D(Mesh3D&& other) noexcept;
    Mesh3D& operator=(Mesh3D&& other) noexcept;

    void setup(const std::vector<Vertex3D>& verts, const std::vector<GLuint>& inds);
    void draw() const;
    void cleanup();

    static Mesh3D createCube();
    static Mesh3D createSphere(int stacks = 20, int sectors = 24);
    static Mesh3D createOctahedron();
};
