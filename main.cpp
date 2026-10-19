/**
 * ============================================================================
 * CHRONO-DASH: ANTIGRAVITY ARENA
 * Complete, Single-File 3D First-Person Shooter in Modern OpenGL 3.3 Core Profile
 * Technologies: C++17, OpenGL 3.3 Core, GLFW, GLAD, GLM
 * ============================================================================
 */

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <random>
#include <chrono>

// -----------------------------------------------------------------------------
// OpenGL Loader Detection: GLAD (GLAD 2 or GLAD 1)
// -----------------------------------------------------------------------------
#if defined(__has_include)
    #if __has_include(<glad/gl.h>)
        #include <glad/gl.h>
        #define USING_GLAD2 1
    #elif __has_include(<glad/glad.h>)
        #include <glad/glad.h>
        #define USING_GLAD1 1
    #endif
#else
    #include <glad/gl.h>
    #define USING_GLAD2 1
#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>

// =============================================================================
// 1. EMBEDDED GLSL SHADERS
// =============================================================================

// 3D Scene Vertex Shader
static const char* vShader3DSource = R"(#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

void main() {
    vec4 worldPos = uModel * vec4(aPos, 1.0);
    FragPos = worldPos.xyz;
    Normal = mat3(transpose(inverse(uModel))) * aNormal;
    TexCoords = aTexCoords;
    gl_Position = uProj * uView * worldPos;
}
)";

// 3D Scene Fragment Shader
static const char* fShader3DSource = R"(#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 uViewPos;
uniform vec3 uColor;
uniform vec3 uEmissive;
uniform float uShininess;
uniform float uTime;

// Core lighting & procedural visuals
uniform vec3 uCorePos;
uniform float uCoreRadius;
uniform int uIsCore;
uniform int uIsFloorGrid;

// Muzzle flash dynamic light
uniform vec3 uFlashPos;
uniform float uFlashIntensity;

void main() {
    // A. Central Antigravity Core Shimmering Orb
    if (uIsCore == 1) {
        vec3 norm = normalize(Normal);
        vec3 viewDir = normalize(uViewPos - FragPos);
        float fresnel = pow(1.0 - max(dot(viewDir, norm), 0.0), 2.2);

        // Animated swirling energy plasma waves
        float wave1 = sin(FragPos.y * 3.5 + uTime * 4.0);
        float wave2 = cos(FragPos.x * 2.8 + FragPos.z * 2.8 + uTime * 3.0);
        float pattern = 0.5 + 0.5 * (wave1 * 0.5 + wave2 * 0.5);

        vec3 deepPurple = vec3(0.40, 0.05, 0.85);
        vec3 brightCyan = vec3(0.10, 0.88, 1.0);
        vec3 coreGlow = mix(deepPurple, brightCyan, pattern * 0.7 + fresnel * 0.5);
        coreGlow += vec3(0.8, 0.5, 1.0) * fresnel * 1.6;

        FragColor = vec4(coreGlow, 0.90);
        return;
    }

    // B. Minimalist Cyberpunk Wireframe Floor Grid
    if (uIsFloorGrid == 1) {
        vec2 coord = FragPos.xz * 0.5; // 2m grid lines
        vec2 grid = abs(fract(coord - 0.5) - 0.5) / fwidth(coord);
        float line = min(grid.x, grid.y);
        float c = 1.0 - min(line, 1.0);

        // Major grid lines every 10m
        vec2 coordMajor = FragPos.xz * 0.1;
        vec2 gridMajor = abs(fract(coordMajor - 0.5) - 0.5) / fwidth(coordMajor);
        float lineMajor = min(gridMajor.x, gridMajor.y);
        float cMajor = 1.0 - min(lineMajor, 1.0);

        vec3 gridCol = vec3(0.08, 0.50, 0.95) * c * 0.85;
        gridCol += vec3(1.0, 0.42, 0.10) * cMajor * 1.3;
        vec3 baseFloor = vec3(0.015, 0.02, 0.04);

        // Distance fog fading into deep twilight void
        float dist = length(FragPos - uViewPos);
        float fog = clamp((dist - 12.0) / 75.0, 0.0, 1.0);
        vec3 finalCol = mix(baseFloor + gridCol, vec3(0.02, 0.03, 0.06), fog);

        FragColor = vec4(finalCol, 1.0);
        return;
    }

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(uViewPos - FragPos);

    // Directional Ambient & Twilight Overhead Sky Light
    vec3 ambient = vec3(0.09, 0.10, 0.15) * uColor;
    vec3 lightDir1 = normalize(vec3(0.35, 0.80, 0.25));
    float diff1 = max(dot(norm, lightDir1), 0.0);
    vec3 diffuse1 = diff1 * vec3(0.40, 0.45, 0.55) * uColor;

    vec3 halfDir1 = normalize(lightDir1 + viewDir);
    float spec1 = pow(max(dot(norm, halfDir1), 0.0), uShininess);
    vec3 specular1 = spec1 * vec3(0.35, 0.45, 0.55);

    // Antigravity Core Point Light (Blue-Purple glowing illumination)
    vec3 toCore = uCorePos - FragPos;
    float distCore = length(toCore);
    vec3 coreLightDir = normalize(toCore);
    float attCore = 1.0 / (1.0 + 0.04 * distCore + 0.006 * distCore * distCore);
    float diffCore = max(dot(norm, coreLightDir), 0.0);
    vec3 coreLightCol = vec3(0.45, 0.30, 0.95) * 2.8;
    vec3 diffuseCore = diffCore * coreLightCol * uColor * attCore;

    vec3 halfCore = normalize(coreLightDir + viewDir);
    float specCore = pow(max(dot(norm, halfCore), 0.0), uShininess);
    vec3 specCoreLight = specCore * vec3(0.5, 0.4, 1.0) * attCore;

    // Muzzle Flash Point Light (Brief bright neon pulse)
    vec3 flashLight = vec3(0.0);
    if (uFlashIntensity > 0.01) {
        vec3 toFlash = uFlashPos - FragPos;
        float distFlash = length(toFlash);
        float attFlash = 1.0 / (1.0 + 0.25 * distFlash + 0.12 * distFlash * distFlash);
        float diffFlash = max(dot(norm, normalize(toFlash)), 0.0);
        flashLight = diffFlash * vec3(0.2, 0.85, 1.0) * uFlashIntensity * 3.5 * attFlash;
    }

    // Rim lighting (Fresnel silhouette on sleek sci-fi armor / edges)
    float rim = 1.0 - max(dot(viewDir, norm), 0.0);
    rim = pow(rim, 3.2) * 0.45;
    vec3 rimCol = rim * (uEmissive + vec3(0.2, 0.45, 0.85));

    vec3 result = ambient + diffuse1 + specular1 + diffuseCore + specCoreLight + flashLight + uEmissive + rimCol;

    // Atmospheric Distance Fog
    float dist = length(FragPos - uViewPos);
    float fogFactor = clamp((dist - 22.0) / 95.0, 0.0, 1.0);
    vec3 fogColor = vec3(0.025, 0.035, 0.075);
    result = mix(result, fogColor, fogFactor);

    FragColor = vec4(result, 1.0);
}
)";

// 2D HUD Vertex Shader
static const char* vShaderHUDSource = R"(#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec4 aColor;

uniform mat4 uProj;

out vec4 vColor;

void main() {
    vColor = aColor;
    gl_Position = uProj * vec4(aPos, 0.0, 1.0);
}
)";

// 2D HUD Fragment Shader
static const char* fShaderHUDSource = R"(#version 330 core
in vec4 vColor;
out vec4 FragColor;

uniform vec4 uTint;

void main() {
    FragColor = vColor * uTint;
}
)";

// =============================================================================
// 2. SHADER PROGRAM UTILITY
// =============================================================================

class ShaderProgram {
public:
    GLuint id = 0;

    bool init(const char* vSrc, const char* fSrc) {
        GLuint vShader = compile(GL_VERTEX_SHADER, vSrc);
        GLuint fShader = compile(GL_FRAGMENT_SHADER, fSrc);
        if (!vShader || !fShader) return false;

        id = glCreateProgram();
        glAttachShader(id, vShader);
        glAttachShader(id, fShader);
        glLinkProgram(id);

        GLint success;
        glGetProgramiv(id, GL_LINK_STATUS, &success);
        if (!success) {
            char log[512];
            glGetProgramInfoLog(id, 512, nullptr, log);
            std::cerr << "[Shader Link Error]: " << log << std::endl;
            return false;
        }

        glDeleteShader(vShader);
        glDeleteShader(fShader);
        return true;
    }

    void use() const { glUseProgram(id); }

    void setMat4(const std::string& name, const glm::mat4& mat) const {
        glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
    }

    void setVec3(const std::string& name, const glm::vec3& val) const {
        glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, glm::value_ptr(val));
    }

    void setVec4(const std::string& name, const glm::vec4& val) const {
        glUniform4fv(glGetUniformLocation(id, name.c_str()), 1, glm::value_ptr(val));
    }

    void setFloat(const std::string& name, float val) const {
        glUniform1f(glGetUniformLocation(id, name.c_str()), val);
    }

    void setInt(const std::string& name, int val) const {
        glUniform1i(glGetUniformLocation(id, name.c_str()), val);
    }

private:
    static GLuint compile(GLenum type, const char* src) {
        GLuint s = glCreateShader(type);
        glShaderSource(s, 1, &src, nullptr);
        glCompileShader(s);
        GLint ok;
        glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            char log[512];
            glGetShaderInfoLog(s, 512, nullptr, log);
            std::cerr << "[Shader Compile Error (" << (type == GL_VERTEX_SHADER ? "VERT" : "FRAG") << ")]: " << log << std::endl;
            return 0;
        }
        return s;
    }
};

// =============================================================================
// 3. PROCEDURAL 3D MESH BUILDER
// =============================================================================

struct Vertex3D {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
};

class Mesh3D {
public:
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    GLsizei indexCount = 0;

    void setup(const std::vector<Vertex3D>& verts, const std::vector<GLuint>& inds) {
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

    void draw() const {
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }

    static Mesh3D createCube() {
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

    static Mesh3D createSphere(int stacks = 20, int sectors = 24) {
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

    static Mesh3D createOctahedron() {
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
};

// =============================================================================
// 4. DYNAMIC 2D HUD VECTOR STROKE & UI RENDERER
// =============================================================================

struct UIVertex {
    glm::vec2 pos;
    glm::vec4 color;
};

class UIRenderer {
public:
    GLuint vao = 0;
    GLuint vbo = 0;
    std::vector<UIVertex> verts;

    void init() {
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

    void clear() { verts.clear(); }

    void addLine(glm::vec2 a, glm::vec2 b, float thickness, glm::vec4 col) {
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

    void addRect(float x, float y, float w, float h, glm::vec4 col) {
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

    void addRectOutline(float x, float y, float w, float h, float thick, glm::vec4 col) {
        addLine({x, y}, {x + w, y}, thick, col);
        addLine({x + w, y}, {x + w, y + h}, thick, col);
        addLine({x + w, y + h}, {x, y + h}, thick, col);
        addLine({x, y + h}, {x, y}, thick, col);
    }

    void drawChar(char c, float x, float y, float sx, float sy, float thick, glm::vec4 col) {
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

    void drawString(const std::string& text, float x, float y, float charW, float charH, float spacing, float thick, glm::vec4 col) {
        float curX = x;
        for (char c : text) {
            if (c != ' ') {
                drawChar(c, curX, y, charW, charH, thick, col);
            }
            curX += charW + spacing;
        }
    }

    void render() {
        if (verts.empty()) return;
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(UIVertex), verts.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)verts.size());
        glBindVertexArray(0);
    }
};

// =============================================================================
// 5. COLLISION & PHYSICS STRUCTURES
// =============================================================================

struct AABB {
    glm::vec3 minBound;
    glm::vec3 maxBound;

    bool contains(const glm::vec3& p) const {
        return (p.x >= minBound.x && p.x <= maxBound.x &&
                p.y >= minBound.y && p.y <= maxBound.y &&
                p.z >= minBound.z && p.z <= maxBound.z);
    }

    bool intersects(const AABB& o) const {
        return (minBound.x <= o.maxBound.x && maxBound.x >= o.minBound.x) &&
               (minBound.y <= o.maxBound.y && maxBound.y >= o.minBound.y) &&
               (minBound.z <= o.maxBound.z && maxBound.z >= o.minBound.z);
    }
};

struct Platform {
    glm::vec3 pos;
    glm::vec3 size;
    glm::vec3 color;
    glm::vec3 trimColor;
    AABB getAABB() const {
        glm::vec3 half = size * 0.5f;
        return {pos - half, pos + half};
    }
};

struct Projectile {
    glm::vec3 pos;
    glm::vec3 vel;
    float radius;
    float life;
    bool isEnemy;
    glm::vec3 color;
};

struct Particle {
    glm::vec3 pos;
    glm::vec3 vel;
    glm::vec3 color;
    float size;
    float life;
    float maxLife;
};

struct FloatingProp {
    glm::vec3 pos;
    glm::vec3 basePos;
    glm::vec3 rotAxis;
    float rotSpeed;
    float currentRot;
    glm::vec3 scale;
    glm::vec3 color;
    glm::vec3 emissive;
    float floatOffset;
};

struct Enemy {
    glm::vec3 pos;
    glm::vec3 vel;
    float health = 50.0f;
    float shootTimer = 1.8f;
    float hoverPhase = 0.0f;
    float hitFlashTimer = 0.0f;
    bool alive = true;

    AABB getAABB() const {
        return {pos - glm::vec3(0.9f), pos + glm::vec3(0.9f)};
    }
};

// =============================================================================
// 6. PLAYER CONTROLLER & WEAPON SYSTEM
// =============================================================================

class Player {
public:
    glm::vec3 pos = glm::vec3(0.0f, 2.5f, 6.0f);
    glm::vec3 vel = glm::vec3(0.0f);

    float yaw = -90.0f;
    float pitch = 0.0f;

    float eyeHeight = 1.70f;
    float health = 100.0f;
    float maxHealth = 100.0f;

    int ammo = 30;
    int maxAmmo = 30;
    int reserveAmmo = 90;
    float reloadTimer = 0.0f;
    static constexpr float reloadDuration = 1.25f;

    float fireCooldown = 0.0f;
    static constexpr float fireInterval = 0.14f;

    bool isGrounded = false;
    bool inAntigrav = false;

    // Gun viewmodel dynamics
    float walkTimer = 0.0f;
    float gunRecoil = 0.0f;
    float gunPitchSway = 0.0f;
    float gunYawSway = 0.0f;
    float muzzleFlashTimer = 0.0f;

    // Movement speeds
    static constexpr float moveSpeed = 13.5f;
    static constexpr float normalGravity = -25.0f;
    static constexpr float normalJumpSpeed = 9.2f;

    void reset() {
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

    glm::vec3 getCameraPos() const {
        return pos + glm::vec3(0.0f, eyeHeight, 0.0f);
    }

    glm::vec3 getForward() const {
        float rYaw = glm::radians(yaw);
        float rPitch = glm::radians(pitch);
        glm::vec3 f;
        f.x = cos(rYaw) * cos(rPitch);
        f.y = sin(rPitch);
        f.z = sin(rYaw) * cos(rPitch);
        return glm::normalize(f);
    }

    glm::vec3 getRight() const {
        return glm::normalize(glm::cross(getForward(), glm::vec3(0.0f, 1.0f, 0.0f)));
    }

    AABB getAABB() const {
        glm::vec3 half(0.45f, eyeHeight * 0.5f, 0.45f);
        glm::vec3 center = pos + glm::vec3(0.0f, eyeHeight * 0.5f, 0.0f);
        return {center - half, center + half};
    }

    void reload() {
        if (reloadTimer > 0.0f || ammo == maxAmmo || reserveAmmo <= 0) return;
        reloadTimer = reloadDuration;
    }
};

// =============================================================================
// 7. GLOBAL GAME STATE & MANAGER
// =============================================================================

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

    void reset() {
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

    void addHit(int points = 100) {
        streakCount++;
        streakTimer = maxStreakTime;
        if (streakCount >= 9) streakMultiplier = 5;
        else if (streakCount >= 5) streakMultiplier = 3;
        else if (streakCount >= 2) streakMultiplier = 2;
        else streakMultiplier = 1;

        score += points * streakMultiplier;
        enemiesKilled++;
    }

    void updateStreak(float dt) {
        if (streakTimer > 0.0f) {
            streakTimer -= dt;
            if (streakTimer <= 0.0f) {
                streakTimer = 0.0f;
                streakCount = 0;
                streakMultiplier = 1;
            }
        }
    }
};

// Global instances
static Player g_Player;
static GameState g_Game;
static std::vector<Platform> g_Platforms;
static std::vector<FloatingProp> g_FloatingProps;
static std::vector<Enemy> g_Enemies;
static std::vector<Projectile> g_Projectiles;
static std::vector<Particle> g_Particles;

// Arena geometry constants
static const glm::vec3 ARENA_CORE_POS = glm::vec3(0.0f, 10.5f, 0.0f);
static const float ARENA_CORE_RADIUS = 16.5f;

// Input tracking
static bool g_Keys[1024] = {false};
static bool g_MouseLeftPressed = false;
static bool g_MouseRightPressed = false;
static double g_LastMouseX = 640.0;
static double g_LastMouseY = 360.0;
static bool g_FirstMouse = true;
static int g_WindowWidth = 1280;
static int g_WindowHeight = 720;
static bool g_CursorCaptured = true;

// Random number generator
static std::mt19937 g_Rng(1337);
static float randRange(float a, float b) {
    std::uniform_real_distribution<float> dist(a, b);
    return dist(g_Rng);
}

// Particle spawner
static void spawnExplosion(const glm::vec3& p, const glm::vec3& col, int count = 22) {
    for (int i = 0; i < count; ++i) {
        Particle part;
        part.pos = p;
        part.vel = glm::vec3(randRange(-6.0f, 6.0f), randRange(-4.0f, 8.0f), randRange(-6.0f, 6.0f));
        part.color = col + glm::vec3(randRange(-0.1f, 0.2f));
        part.size = randRange(0.08f, 0.22f);
        part.life = randRange(0.4f, 0.85f);
        part.maxLife = part.life;
        g_Particles.push_back(part);
    }
}

// Arena Initialization
static void initArena() {
    g_Platforms.clear();
    g_FloatingProps.clear();

    // 1. Central Main Hub Platform (Lower)
    g_Platforms.push_back({
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(26.0f, 1.2f, 26.0f),
        glm::vec3(0.05f, 0.06f, 0.09f),
        glm::vec3(0.10f, 0.85f, 1.0f) // Cyan trim
    });

    // 2. North Elevated Platform
    g_Platforms.push_back({
        glm::vec3(0.0f, 6.0f, -20.0f),
        glm::vec3(16.0f, 1.0f, 12.0f),
        glm::vec3(0.05f, 0.06f, 0.09f),
        glm::vec3(1.0f, 0.40f, 0.08f) // Orange trim
    });

    // 3. South Elevated Platform
    g_Platforms.push_back({
        glm::vec3(0.0f, 7.5f, 20.0f),
        glm::vec3(16.0f, 1.0f, 12.0f),
        glm::vec3(0.05f, 0.06f, 0.09f),
        glm::vec3(0.10f, 0.85f, 1.0f)
    });

    // 4. West High Platform
    g_Platforms.push_back({
        glm::vec3(-21.0f, 11.5f, 0.0f),
        glm::vec3(12.0f, 1.0f, 14.0f),
        glm::vec3(0.05f, 0.06f, 0.09f),
        glm::vec3(0.80f, 0.20f, 1.0f) // Purple trim
    });

    // 5. East High Platform
    g_Platforms.push_back({
        glm::vec3(21.0f, 11.5f, 0.0f),
        glm::vec3(12.0f, 1.0f, 14.0f),
        glm::vec3(0.05f, 0.06f, 0.09f),
        glm::vec3(0.80f, 0.20f, 1.0f)
    });

    // 6. Corner Stepping Platforms
    g_Platforms.push_back({ glm::vec3(-14.0f, 3.8f, -14.0f), glm::vec3(7.0f, 0.8f, 7.0f), glm::vec3(0.06f), glm::vec3(0.1f, 0.8f, 1.0f) });
    g_Platforms.push_back({ glm::vec3( 14.0f, 3.8f, -14.0f), glm::vec3(7.0f, 0.8f, 7.0f), glm::vec3(0.06f), glm::vec3(1.0f, 0.4f, 0.1f) });
    g_Platforms.push_back({ glm::vec3(-14.0f, 4.2f,  14.0f), glm::vec3(7.0f, 0.8f, 7.0f), glm::vec3(0.06f), glm::vec3(1.0f, 0.4f, 0.1f) });
    g_Platforms.push_back({ glm::vec3( 14.0f, 4.2f,  14.0f), glm::vec3(7.0f, 0.8f, 7.0f), glm::vec3(0.06f), glm::vec3(0.1f, 0.8f, 1.0f) });

    // Floating Zero-G Props around Central Antigravity Core
    // Anchor Cube (as seen in screenshot)
    g_FloatingProps.push_back({
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
        g_FloatingProps.push_back({
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

// Reset Game State
static void resetGame() {
    g_Player.reset();
    g_Game.reset();
    g_Enemies.clear();
    g_Projectiles.clear();
    g_Particles.clear();
    initArena();
}

// =============================================================================
// 8. GLFW INPUT CALLBACKS
// =============================================================================

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        g_CursorCaptured = !g_CursorCaptured;
        glfwSetInputMode(window, GLFW_CURSOR, g_CursorCaptured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) g_Keys[key] = true;
        else if (action == GLFW_RELEASE) g_Keys[key] = false;
    }

    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        if (g_Game.gameOver) {
            resetGame();
        } else {
            g_Player.reload();
        }
    }
}

static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        g_MouseLeftPressed = (action == GLFW_PRESS);
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        g_MouseRightPressed = (action == GLFW_PRESS);
    }
}

static void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (!g_CursorCaptured) return;

    if (g_FirstMouse) {
        g_LastMouseX = xpos;
        g_LastMouseY = ypos;
        g_FirstMouse = false;
    }

    float xoffset = static_cast<float>(xpos - g_LastMouseX);
    float yoffset = static_cast<float>(g_LastMouseY - ypos); // Reversed since y goes bottom to top
    g_LastMouseX = xpos;
    g_LastMouseY = ypos;

    const float sensitivity = 0.10f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    g_Player.yaw += xoffset;
    g_Player.pitch += yoffset;

    // Clamp pitch
    if (g_Player.pitch > 89.0f) g_Player.pitch = 89.0f;
    if (g_Player.pitch < -89.0f) g_Player.pitch = -89.0f;

    // Sway gun viewmodel with mouse delta
    g_Player.gunYawSway = glm::clamp(g_Player.gunYawSway - xoffset * 0.003f, -0.04f, 0.04f);
    g_Player.gunPitchSway = glm::clamp(g_Player.gunPitchSway + yoffset * 0.003f, -0.04f, 0.04f);
}

static void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    g_WindowWidth = width > 0 ? width : 1;
    g_WindowHeight = height > 0 ? height : 1;
    glViewport(0, 0, g_WindowWidth, g_WindowHeight);
}

// =============================================================================
// 9. GAME SIMULATION & UPDATES
// =============================================================================

static void firePlayerLaser() {
    if (g_Player.ammo <= 0 || g_Player.fireCooldown > 0.0f || g_Player.reloadTimer > 0.0f) {
        if (g_Player.ammo <= 0) g_Player.reload();
        return;
    }

    g_Player.ammo--;
    g_Player.fireCooldown = g_Player.fireInterval;
    g_Player.gunRecoil = 1.0f;
    g_Player.muzzleFlashTimer = 0.07f;

    glm::vec3 camPos = g_Player.getCameraPos();
    glm::vec3 fwd = g_Player.getForward();
    glm::vec3 right = g_Player.getRight();

    // Spawn projectile aligned with crosshair ray
    Projectile p;
    p.pos = camPos + fwd * 0.5f + right * 0.16f - glm::vec3(0.0f, 0.12f, 0.0f);
    p.vel = fwd * 72.0f;
    p.radius = 0.22f;
    p.life = 2.2f;
    p.isEnemy = false;
    p.color = glm::vec3(0.15f, 0.90f, 1.0f); // Bright Cyan Laser
    g_Projectiles.push_back(p);
}

static void updateSimulation(float dt) {
    if (g_Game.gameOver) return;

    // 1. Antigravity Proximity Check
    float distToCore = glm::length(g_Player.pos - ARENA_CORE_POS);
    g_Player.inAntigrav = (distToCore < ARENA_CORE_RADIUS);

    // 2. Player Input Movement
    glm::vec3 fwd = g_Player.getForward();
    glm::vec3 right = g_Player.getRight();

    // Flatten for horizontal walking
    glm::vec3 moveDir(0.0f);
    glm::vec3 horizFwd = glm::normalize(glm::vec3(fwd.x, 0.0f, fwd.z));
    glm::vec3 horizRight = glm::normalize(glm::vec3(right.x, 0.0f, right.z));

    if (g_Keys[GLFW_KEY_W]) moveDir += horizFwd;
    if (g_Keys[GLFW_KEY_S]) moveDir -= horizFwd;
    if (g_Keys[GLFW_KEY_D]) moveDir += horizRight;
    if (g_Keys[GLFW_KEY_A]) moveDir -= horizRight;

    bool isMoving = (glm::length(moveDir) > 0.01f);
    if (isMoving) moveDir = glm::normalize(moveDir);

    // 3. Antigravity Physics vs Normal Physics
    if (g_Player.inAntigrav) {
        // Zero-G / Antigravity floaty dynamics:
        // Gravity is drastically reduced or hovers
        float targetYGravity = -2.2f;
        g_Player.vel.y += targetYGravity * dt;

        // Space = Upward thrusters; Shift/Ctrl = Downward thrusters
        if (g_Keys[GLFW_KEY_SPACE]) {
            g_Player.vel.y += 14.0f * dt;
            if (g_Player.vel.y > 10.0f) g_Player.vel.y = 10.0f;
        }
        if (g_Keys[GLFW_KEY_LEFT_SHIFT] || g_Keys[GLFW_KEY_LEFT_CONTROL]) {
            g_Player.vel.y -= 14.0f * dt;
            if (g_Player.vel.y < -10.0f) g_Player.vel.y = -10.0f;
        }

        // Floaty 3D momentum with soft drag
        g_Player.vel.x += moveDir.x * g_Player.moveSpeed * 1.5f * dt;
        g_Player.vel.z += moveDir.z * g_Player.moveSpeed * 1.5f * dt;
        g_Player.vel.x *= std::pow(0.85f, dt * 60.0f);
        g_Player.vel.z *= std::pow(0.85f, dt * 60.0f);
        g_Player.vel.y *= std::pow(0.92f, dt * 60.0f);

        // Visual zero-G floating dust particles
        if (randRange(0.0f, 1.0f) < 0.2f) {
            Particle part;
            part.pos = g_Player.pos + glm::vec3(randRange(-4,4), randRange(-2,4), randRange(-4,4));
            part.vel = glm::vec3(randRange(-0.4f,0.4f), randRange(-0.2f,0.6f), randRange(-0.4f,0.4f));
            part.color = glm::vec3(0.3f, 0.6f, 1.0f);
            part.size = 0.04f;
            part.life = 1.0f;
            part.maxLife = 1.0f;
            g_Particles.push_back(part);
        }
    } else {
        // Standard grounded FPS movement with gravity
        g_Player.vel.y += g_Player.normalGravity * dt;

        if (g_Player.isGrounded) {
            g_Player.vel.x = moveDir.x * g_Player.moveSpeed;
            g_Player.vel.z = moveDir.z * g_Player.moveSpeed;
            if (g_Keys[GLFW_KEY_SPACE]) {
                g_Player.vel.y = g_Player.normalJumpSpeed;
                g_Player.isGrounded = false;
            }
        } else {
            // Air control
            g_Player.vel.x += moveDir.x * g_Player.moveSpeed * 1.2f * dt;
            g_Player.vel.z += moveDir.z * g_Player.moveSpeed * 1.2f * dt;
            g_Player.vel.x *= std::pow(0.92f, dt * 60.0f);
            g_Player.vel.z *= std::pow(0.92f, dt * 60.0f);
        }
    }

    // 4. Player Platform Collision & Position Integration
    glm::vec3 nextPos = g_Player.pos + g_Player.vel * dt;
    g_Player.isGrounded = false;

    for (const auto& plat : g_Platforms) {
        AABB box = plat.getAABB();
        // Check landing on platform top
        float footOld = g_Player.pos.y;
        float footNew = nextPos.y;
        if (nextPos.x >= box.minBound.x - 0.4f && nextPos.x <= box.maxBound.x + 0.4f &&
            nextPos.z >= box.minBound.z - 0.4f && nextPos.z <= box.maxBound.z + 0.4f) {
            if (footOld >= box.maxBound.y - 0.1f && footNew <= box.maxBound.y) {
                nextPos.y = box.maxBound.y;
                g_Player.vel.y = 0.0f;
                g_Player.isGrounded = true;
            }
        }
    }

    // Void fall boundary: antigravity emergency repulsion / damage
    if (nextPos.y < -12.0f) {
        nextPos.y = -10.0f;
        g_Player.vel.y = 19.0f; // launch back upward
        g_Player.health -= 12.0f;
        g_Game.damageFlash = 0.5f;
        if (g_Player.health <= 0.0f) {
            g_Player.health = 0.0f;
            g_Game.gameOver = true;
        }
    }

    g_Player.pos = nextPos;

    // 5. Weapon viewmodel sway, bobbing & recoil update
    if (isMoving && g_Player.isGrounded) {
        g_Player.walkTimer += dt * 10.0f;
    } else {
        g_Player.walkTimer = 0.0f;
    }

    g_Player.gunRecoil = std::max(0.0f, g_Player.gunRecoil - dt * 8.0f);
    g_Player.gunPitchSway *= std::pow(0.85f, dt * 60.0f);
    g_Player.gunYawSway *= std::pow(0.85f, dt * 60.0f);
    if (g_Player.muzzleFlashTimer > 0.0f) g_Player.muzzleFlashTimer -= dt;

    if (g_Player.fireCooldown > 0.0f) g_Player.fireCooldown -= dt;

    // Reload timer
    if (g_Player.reloadTimer > 0.0f) {
        g_Player.reloadTimer -= dt;
        if (g_Player.reloadTimer <= 0.0f) {
            int needed = g_Player.maxAmmo - g_Player.ammo;
            int take = std::min(needed, g_Player.reserveAmmo);
            g_Player.ammo += take;
            g_Player.reserveAmmo -= take;
            g_Player.reloadTimer = 0.0f;
        }
    }

    // Continuous fire when holding left click
    if (g_MouseLeftPressed && g_CursorCaptured) {
        firePlayerLaser();
    }

    // 6. Floating Props Animation (orbit & bobbing)
    for (auto& prop : g_FloatingProps) {
        prop.currentRot += prop.rotSpeed * dt;
        float bob = std::sin(glfwGetTime() * 1.8f + prop.floatOffset) * 0.4f;
        prop.pos.y = prop.basePos.y + bob;
    }

    // 7. Enemy AI & Spawning
    g_Game.enemySpawnTimer -= dt;
    if (g_Game.enemySpawnTimer <= 0.0f && g_Enemies.size() < 7) {
        g_Game.enemySpawnTimer = randRange(3.5f, 5.5f);
        float angle = randRange(0.0f, glm::two_pi<float>());
        float dist = randRange(16.0f, 26.0f);
        Enemy e;
        e.pos = glm::vec3(cos(angle) * dist, randRange(4.0f, 15.0f), sin(angle) * dist);
        e.vel = glm::vec3(0.0f);
        e.hoverPhase = randRange(0.0f, 6.28f);
        e.shootTimer = randRange(1.5f, 3.0f);
        g_Enemies.push_back(e);
    }

    for (auto& e : g_Enemies) {
        if (!e.alive) continue;
        if (e.hitFlashTimer > 0.0f) e.hitFlashTimer -= dt;

        // Hover & steer towards player
        glm::vec3 toPlayer = g_Player.getCameraPos() - e.pos;
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
        float bobY = std::sin(glfwGetTime() * 2.5f + e.hoverPhase) * 1.8f;
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
            g_Projectiles.push_back(ep);
        }
    }

    // 8. Projectile Simulation & Collision Checks
    for (auto& p : g_Projectiles) {
        p.pos += p.vel * dt;
        p.life -= dt;

        if (p.life <= 0.0f) continue;

        if (!p.isEnemy) {
            // Player bullet vs Enemies
            for (auto& e : g_Enemies) {
                if (!e.alive) continue;
                if (glm::length(p.pos - e.pos) < (p.radius + 1.1f)) {
                    e.health -= 30.0f;
                    e.hitFlashTimer = 0.15f;
                    p.life = -1.0f; // destroy bullet
                    spawnExplosion(p.pos, glm::vec3(0.2f, 0.9f, 1.0f), 10);

                    if (e.health <= 0.0f) {
                        e.alive = false;
                        g_Game.addHit(100);
                        spawnExplosion(e.pos, glm::vec3(1.0f, 0.3f, 0.1f), 30);
                        spawnExplosion(e.pos, glm::vec3(0.8f, 0.1f, 1.0f), 20);
                    }
                    break;
                }
            }
        } else {
            // Enemy bullet vs Player
            if (glm::length(p.pos - g_Player.getCameraPos()) < (p.radius + 0.65f)) {
                g_Player.health -= 15.0f;
                g_Game.damageFlash = 0.5f;
                p.life = -1.0f;
                spawnExplosion(p.pos, glm::vec3(0.9f, 0.1f, 0.7f), 16);

                if (g_Player.health <= 0.0f) {
                    g_Player.health = 0.0f;
                    g_Game.gameOver = true;
                }
            }
        }

        // Bullet vs Platforms
        for (const auto& plat : g_Platforms) {
            if (plat.getAABB().contains(p.pos)) {
                p.life = -1.0f;
                spawnExplosion(p.pos, p.color, 8);
                break;
            }
        }
    }

    // Remove expired projectiles
    g_Projectiles.erase(std::remove_if(g_Projectiles.begin(), g_Projectiles.end(),
        [](const Projectile& p) { return p.life <= 0.0f; }), g_Projectiles.end());

    // Remove dead enemies
    g_Enemies.erase(std::remove_if(g_Enemies.begin(), g_Enemies.end(),
        [](const Enemy& e) { return !e.alive; }), g_Enemies.end());

    // 9. Particles Update
    for (auto& part : g_Particles) {
        part.pos += part.vel * dt;
        part.life -= dt;
    }
    g_Particles.erase(std::remove_if(g_Particles.begin(), g_Particles.end(),
        [](const Particle& part) { return part.life <= 0.0f; }), g_Particles.end());

    // 10. Streak & Damage Flash Update
    g_Game.updateStreak(dt);
    if (g_Game.damageFlash > 0.0f) {
        g_Game.damageFlash = std::max(0.0f, g_Game.damageFlash - dt * 2.0f);
    }
}

// =============================================================================
// 10. RENDERING PASSES
// =============================================================================

// Helper to draw a sleek futuristic weapon model
static void renderGunViewModel(const ShaderProgram& shader, const Mesh3D& cubeMesh, const Mesh3D& sphereMesh) {
    // Gun camera-relative positioning
    glm::vec3 camPos = g_Player.getCameraPos();
    glm::vec3 fwd = g_Player.getForward();
    glm::vec3 right = g_Player.getRight();
    glm::vec3 up = glm::cross(right, fwd);

    // Dynamic bobbing & sway offsets
    float bobX = std::sin(g_Player.walkTimer * 0.8f) * 0.008f;
    float bobY = std::abs(std::cos(g_Player.walkTimer * 0.8f)) * 0.007f;

    glm::vec3 gunOffset = right * (0.28f + g_Player.gunYawSway + bobX) +
                          up    * (-0.22f + g_Player.gunPitchSway + bobY) +
                          fwd   * (0.48f - g_Player.gunRecoil * 0.08f);

    glm::vec3 gunBasePos = camPos + gunOffset;

    // Build rotation matrix from camera axes with recoil pitch
    glm::mat4 gunRot(1.0f);
    gunRot[0] = glm::vec4(right, 0.0f);
    gunRot[1] = glm::vec4(up, 0.0f);
    gunRot[2] = glm::vec4(-fwd, 0.0f);
    gunRot = glm::rotate(gunRot, glm::radians(g_Player.gunRecoil * 6.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    auto drawPart = [&](const glm::vec3& localPos, const glm::vec3& scale, const glm::vec3& col, const glm::vec3& emissive) {
        glm::mat4 m = glm::translate(glm::mat4(1.0f), gunBasePos);
        m = m * gunRot;
        m = glm::translate(m, localPos);
        m = glm::scale(m, scale);

        shader.setMat4("uModel", m);
        shader.setVec3("uColor", col);
        shader.setVec3("uEmissive", emissive);
        cubeMesh.draw();
    };

    // 1. Main Dark Carbon Receiver Body
    drawPart(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.065f, 0.08f, 0.32f), glm::vec3(0.06f, 0.07f, 0.09f), glm::vec3(0.0f));

    // 2. White Futuristic Armor Shroud (Top & Sides)
    drawPart(glm::vec3(0.0f, 0.045f, -0.04f), glm::vec3(0.072f, 0.035f, 0.24f), glm::vec3(0.92f, 0.94f, 0.96f), glm::vec3(0.0f));

    // 3. Vibrant Orange Cyberpunk Trim Accent
    drawPart(glm::vec3(0.038f, 0.02f, -0.02f), glm::vec3(0.008f, 0.04f, 0.18f), glm::vec3(1.0f, 0.45f, 0.05f), glm::vec3(0.4f, 0.15f, 0.0f));
    drawPart(glm::vec3(-0.038f, 0.02f, -0.02f), glm::vec3(0.008f, 0.04f, 0.18f), glm::vec3(1.0f, 0.45f, 0.05f), glm::vec3(0.4f, 0.15f, 0.0f));

    // 4. Dual Glowing Cyan Magnetic Rail Barrels
    drawPart(glm::vec3(0.018f, 0.015f, 0.22f), glm::vec3(0.016f, 0.016f, 0.20f), glm::vec3(0.1f, 0.1f, 0.15f), glm::vec3(0.1f, 0.85f, 1.0f));
    drawPart(glm::vec3(-0.018f, 0.015f, 0.22f), glm::vec3(0.016f, 0.016f, 0.20f), glm::vec3(0.1f, 0.1f, 0.15f), glm::vec3(0.1f, 0.85f, 1.0f));

    // 5. Holographic Sight Mount & Glass Reticle
    drawPart(glm::vec3(0.0f, 0.075f, -0.06f), glm::vec3(0.04f, 0.03f, 0.08f), glm::vec3(0.08f), glm::vec3(0.0f));
    drawPart(glm::vec3(0.0f, 0.082f, -0.03f), glm::vec3(0.032f, 0.032f, 0.004f), glm::vec3(0.1f, 0.9f, 1.0f), glm::vec3(0.2f, 0.9f, 1.0f));

    // 6. Angled Energy Battery Magazine (Bottom)
    drawPart(glm::vec3(0.0f, -0.08f, -0.05f), glm::vec3(0.05f, 0.12f, 0.07f), glm::vec3(0.12f, 0.12f, 0.15f), glm::vec3(0.8f, 0.35f, 0.05f));

    // 7. Muzzle Flash Flare
    if (g_Player.muzzleFlashTimer > 0.0f) {
        glm::vec3 muzzlePos = gunBasePos + right * 0.0f + up * 0.015f + fwd * 0.34f;
        glm::mat4 mf = glm::translate(glm::mat4(1.0f), muzzlePos);
        mf = glm::scale(mf, glm::vec3(0.14f));
        shader.setMat4("uModel", mf);
        shader.setVec3("uColor", glm::vec3(1.0f));
        shader.setVec3("uEmissive", glm::vec3(1.5f, 1.8f, 2.5f));
        sphereMesh.draw();
    }
}

// Render Complete 2D Sci-Fi HUD Overlay
static void renderHUD(UIRenderer& ui, const ShaderProgram& hudShader) {
    ui.clear();

    float w = static_cast<float>(g_WindowWidth);
    float h = static_cast<float>(g_WindowHeight);

    // Colors
    glm::vec4 colCyan(0.10f, 0.88f, 1.0f, 1.0f);
    glm::vec4 colOrange(1.0f, 0.45f, 0.05f, 1.0f);
    glm::vec4 colPurple(0.85f, 0.25f, 1.0f, 1.0f);
    glm::vec4 colRed(1.0f, 0.15f, 0.20f, 1.0f);
    glm::vec4 colWhite(0.95f, 0.98f, 1.0f, 1.0f);
    glm::vec4 colDarkPanel(0.02f, 0.04f, 0.08f, 0.65f);

    // -------------------------------------------------------------------------
    // A. TOP RIGHT: SCORE & STREAK DISPLAY (Exactly matching user prompt)
    // -------------------------------------------------------------------------
    float scorePanelW = 280.0f;
    float scorePanelH = 95.0f;
    float scoreX = w - scorePanelW - 30.0f;
    float scoreY = 30.0f;

    // Dark backdrop with glowing border
    ui.addRect(scoreX, scoreY, scorePanelW, scorePanelH, colDarkPanel);
    ui.addRectOutline(scoreX, scoreY, scorePanelW, scorePanelH, 2.0f, colCyan * 0.8f);

    // Accent corner tabs
    ui.addRect(scoreX - 2.0f, scoreY - 2.0f, 16.0f, 4.0f, colCyan);
    ui.addRect(scoreX - 2.0f, scoreY - 2.0f, 4.0f, 16.0f, colCyan);
    ui.addRect(scoreX + scorePanelW - 14.0f, scoreY - 2.0f, 16.0f, 4.0f, colCyan);
    ui.addRect(scoreX + scorePanelW - 2.0f, scoreY - 2.0f, 4.0f, 16.0f, colCyan);

    // Score String: "SCORE: 12500"
    std::string scoreStr = "SCORE: " + std::to_string(g_Game.score);
    ui.drawString(scoreStr, scoreX + 20.0f, scoreY + 16.0f, 14.0f, 22.0f, 4.0f, 2.5f, colWhite);

    // Streak String: "STREAK: 5X"
    std::string streakStr = "STREAK: " + std::to_string(g_Game.streakMultiplier) + "X";
    glm::vec4 streakCol = (g_Game.streakMultiplier > 1) ? colOrange : (colWhite * 0.7f);
    ui.drawString(streakStr, scoreX + 20.0f, scoreY + 48.0f, 12.0f, 18.0f, 3.5f, 2.2f, streakCol);

    // Multiplier gauge countdown bar underneath
    if (g_Game.streakTimer > 0.0f) {
        float streakPct = g_Game.streakTimer / g_Game.maxStreakTime;
        float barW = (scorePanelW - 40.0f) * streakPct;
        ui.addRect(scoreX + 20.0f, scoreY + 76.0f, scorePanelW - 40.0f, 6.0f, glm::vec4(0.1f, 0.1f, 0.15f, 0.8f));
        ui.addRect(scoreX + 20.0f, scoreY + 76.0f, barW, 6.0f, colOrange);
    }

    // -------------------------------------------------------------------------
    // B. BOTTOM LEFT: HEALTH & STATUS
    // -------------------------------------------------------------------------
    float hpPanelW = 300.0f;
    float hpPanelH = 85.0f;
    float hpX = 30.0f;
    float hpY = h - hpPanelH - 30.0f;

    ui.addRect(hpX, hpY, hpPanelW, hpPanelH, colDarkPanel);
    ui.addRectOutline(hpX, hpY, hpPanelW, hpPanelH, 2.0f, colCyan * 0.7f);

    // Tag: "CHRONO_HUNTER"
    ui.drawString("CHRONO_HUNTER", hpX + 16.0f, hpY + 12.0f, 9.0f, 14.0f, 3.0f, 1.8f, colCyan);

    // Health Bar
    float hpPct = glm::clamp(g_Player.health / g_Player.maxHealth, 0.0f, 1.0f);
    glm::vec4 hpCol = (hpPct > 0.35f) ? colCyan : colRed;

    ui.drawString("HP: " + std::to_string((int)g_Player.health), hpX + 16.0f, hpY + 34.0f, 11.0f, 16.0f, 3.0f, 2.0f, hpCol);

    float barMaxW = hpPanelW - 32.0f;
    ui.addRect(hpX + 16.0f, hpY + 56.0f, barMaxW, 12.0f, glm::vec4(0.1f, 0.1f, 0.15f, 0.8f));
    ui.addRect(hpX + 16.0f, hpY + 56.0f, barMaxW * hpPct, 12.0f, hpCol);

    // -------------------------------------------------------------------------
    // C. BOTTOM RIGHT: AMMO & WEAPON STATUS
    // -------------------------------------------------------------------------
    float ammoPanelW = 240.0f;
    float ammoPanelH = 85.0f;
    float ammoX = w - ammoPanelW - 30.0f;
    float ammoY = h - ammoPanelH - 30.0f;

    ui.addRect(ammoX, ammoY, ammoPanelW, ammoPanelH, colDarkPanel);
    ui.addRectOutline(ammoX, ammoY, ammoPanelW, ammoPanelH, 2.0f, colOrange * 0.7f);

    ui.drawString("PULSE_CANNON", ammoX + 16.0f, ammoY + 12.0f, 9.0f, 14.0f, 3.0f, 1.8f, colOrange);

    if (g_Player.reloadTimer > 0.0f) {
        ui.drawString("RELOADING...", ammoX + 16.0f, ammoY + 36.0f, 12.0f, 18.0f, 3.0f, 2.0f, colOrange);
    } else {
        std::string ammoStr = "AMMO: " + std::to_string(g_Player.ammo) + " / " + std::to_string(g_Player.reserveAmmo);
        ui.drawString(ammoStr, ammoX + 16.0f, ammoY + 36.0f, 12.0f, 18.0f, 3.0f, 2.0f, colWhite);
    }

    // Ammo gauge pips
    float ammoPct = (float)g_Player.ammo / (float)g_Player.maxAmmo;
    ui.addRect(ammoX + 16.0f, ammoY + 60.0f, ammoPanelW - 32.0f, 10.0f, glm::vec4(0.1f, 0.1f, 0.15f, 0.8f));
    ui.addRect(ammoX + 16.0f, ammoY + 60.0f, (ammoPanelW - 32.0f) * ammoPct, 10.0f, colOrange);

    // -------------------------------------------------------------------------
    // D. CENTER TOP: ANTIGRAVITY STATUS NOTIFICATION BADGE
    // -------------------------------------------------------------------------
    float badgeW = 340.0f;
    float badgeH = 34.0f;
    float badgeX = (w - badgeW) * 0.5f;
    float badgeY = 30.0f;

    if (g_Player.inAntigrav) {
        ui.addRect(badgeX, badgeY, badgeW, badgeH, glm::vec4(0.2f, 0.05f, 0.4f, 0.75f));
        ui.addRectOutline(badgeX, badgeY, badgeW, badgeH, 2.0f, colPurple);
        ui.drawString("[ ANTIGRAV: ACTIVE (ZERO-G) ]", badgeX + 14.0f, badgeY + 8.0f, 9.0f, 15.0f, 2.5f, 2.0f, colCyan);
    } else {
        ui.addRect(badgeX, badgeY, badgeW, badgeH, glm::vec4(0.04f, 0.06f, 0.1f, 0.5f));
        ui.addRectOutline(badgeX, badgeY, badgeW, badgeH, 1.5f, colCyan * 0.5f);
        ui.drawString("[ GRAVITY: 1.0G NOMINAL ]", badgeX + 36.0f, badgeY + 8.0f, 9.0f, 15.0f, 2.5f, 1.8f, colWhite * 0.7f);
    }

    // -------------------------------------------------------------------------
    // E. CENTER RETICLE / CROSSHAIR
    // -------------------------------------------------------------------------
    float cx = w * 0.5f;
    float cy = h * 0.5f;
    float spread = 12.0f + g_Player.gunRecoil * 18.0f;
    float retThick = 2.0f;
    float bracketLen = 14.0f;

    // Center Aim Dot
    ui.addRect(cx - 1.5f, cy - 1.5f, 3.0f, 3.0f, colCyan);

    // 4 Corner brackets [ ]
    // Top-Left
    ui.addLine({cx - spread - bracketLen, cy - spread}, {cx - spread, cy - spread}, retThick, colCyan);
    ui.addLine({cx - spread, cy - spread}, {cx - spread, cy - spread - bracketLen}, retThick, colCyan);
    // Top-Right
    ui.addLine({cx + spread + bracketLen, cy - spread}, {cx + spread, cy - spread}, retThick, colCyan);
    ui.addLine({cx + spread, cy - spread}, {cx + spread, cy - spread - bracketLen}, retThick, colCyan);
    // Bottom-Left
    ui.addLine({cx - spread - bracketLen, cy + spread}, {cx - spread, cy + spread}, retThick, colCyan);
    ui.addLine({cx - spread, cy + spread}, {cx - spread, cy + spread + bracketLen}, retThick, colCyan);
    // Bottom-Right
    ui.addLine({cx + spread + bracketLen, cy + spread}, {cx + spread, cy + spread}, retThick, colCyan);
    ui.addLine({cx + spread, cy + spread}, {cx + spread, cy + spread + bracketLen}, retThick, colCyan);

    // -------------------------------------------------------------------------
    // F. DAMAGE VIGNETTE / SCREEN FLASH
    // -------------------------------------------------------------------------
    if (g_Game.damageFlash > 0.01f) {
        float f = g_Game.damageFlash;
        glm::vec4 flashCol(0.9f, 0.05f, 0.1f, f * 0.45f);
        ui.addRect(0, 0, w, 24.0f, flashCol);
        ui.addRect(0, h - 24.0f, w, 24.0f, flashCol);
        ui.addRect(0, 0, 24.0f, h, flashCol);
        ui.addRect(w - 24.0f, 0, 24.0f, h, flashCol);
    }

    // -------------------------------------------------------------------------
    // G. GAME OVER OVERLAY
    // -------------------------------------------------------------------------
    if (g_Game.gameOver) {
        ui.addRect(0, 0, w, h, glm::vec4(0.02f, 0.02f, 0.05f, 0.85f));
        float goW = 480.0f;
        float goH = 220.0f;
        float goX = (w - goW) * 0.5f;
        float goY = (h - goH) * 0.5f;

        ui.addRect(goX, goY, goW, goH, glm::vec4(0.04f, 0.05f, 0.09f, 0.95f));
        ui.addRectOutline(goX, goY, goW, goH, 3.0f, colRed);

        ui.drawString("MISSION FAILED", goX + 80.0f, goY + 35.0f, 20.0f, 30.0f, 5.0f, 3.5f, colRed);
        std::string finalScore = "FINAL SCORE: " + std::to_string(g_Game.score);
        ui.drawString(finalScore, goX + 110.0f, goY + 95.0f, 14.0f, 22.0f, 4.0f, 2.5f, colWhite);
        ui.drawString("PRESS [R] TO RESPAWN", goX + 85.0f, goY + 150.0f, 13.0f, 20.0f, 3.5f, 2.2f, colCyan);
    }

    // Draw all accumulated HUD vertices
    hudShader.use();
    glm::mat4 orthoProj = glm::ortho(0.0f, w, h, 0.0f, -1.0f, 1.0f);
    hudShader.setMat4("uProj", orthoProj);
    hudShader.setVec4("uTint", glm::vec4(1.0f));

    glDisable(GL_DEPTH_TEST);
    ui.render();
    glEnable(GL_DEPTH_TEST);
}

// =============================================================================
// 11. MAIN ENTRY POINT
// =============================================================================

int main() {
    std::cout << "========================================================\n";
    std::cout << "   Chrono-Dash: Antigravity Arena (OpenGL 3.3 Core)     \n";
    std::cout << "========================================================\n";

    // 1. Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4); // 4x MSAA

    GLFWwindow* window = glfwCreateWindow(g_WindowWidth, g_WindowHeight, "Chrono-Dash: Antigravity Arena", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable VSync

    // Callbacks
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // 2. Initialize GLAD
#if defined(USING_GLAD2)
    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "Failed to load OpenGL via gladLoadGL" << std::endl;
        return -1;
    }
#elif defined(USING_GLAD1)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to load OpenGL via gladLoadGLLoader" << std::endl;
        return -1;
    }
#else
    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "Failed to load OpenGL" << std::endl;
        return -1;
    }
#endif

    // 3. Configure OpenGL Global States
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 4. Compile Shaders
    ShaderProgram shader3D;
    if (!shader3D.init(vShader3DSource, fShader3DSource)) {
        std::cerr << "3D Shader compilation failed!" << std::endl;
        return -1;
    }

    ShaderProgram shaderHUD;
    if (!shaderHUD.init(vShaderHUDSource, fShaderHUDSource)) {
        std::cerr << "HUD Shader compilation failed!" << std::endl;
        return -1;
    }

    // 5. Generate Procedural Meshes
    Mesh3D cubeMesh = Mesh3D::createCube();
    Mesh3D sphereMesh = Mesh3D::createSphere(24, 28);
    Mesh3D octaMesh = Mesh3D::createOctahedron();

    UIRenderer ui;
    ui.init();

    // 6. Initialize Arena
    initArena();

    std::cout << "[INFO] Engine started successfully. Enjoy Chrono-Dash!\n";

    // Timing
    double lastFrameTime = glfwGetTime();

    // =========================================================================
    // MAIN GAME LOOP
    // =========================================================================
    while (!glfwWindowShouldClose(window)) {
        double currentFrameTime = glfwGetTime();
        float dt = static_cast<float>(currentFrameTime - lastFrameTime);
        lastFrameTime = currentFrameTime;
        if (dt > 0.05f) dt = 0.05f; // Clamped for physics stability

        // Poll Events & Update Gameplay Simulation
        glfwPollEvents();
        updateSimulation(dt);

        // Clear Color & Depth Buffers
        glClearColor(0.02f, 0.025f, 0.05f, 1.0f); // Deep twilight cyberpunk sky
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Camera & Projection
        glm::vec3 camPos = g_Player.getCameraPos();
        glm::mat4 view = glm::lookAt(camPos, camPos + g_Player.getForward(), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 proj = glm::perspective(glm::radians(75.0f), (float)g_WindowWidth / (float)g_WindowHeight, 0.08f, 180.0f);

        float currentTimeSec = static_cast<float>(glfwGetTime());

        // ---------------------------------------------------------------------
        // PASS 1: 3D WORLD RENDERING
        // ---------------------------------------------------------------------
        shader3D.use();
        shader3D.setMat4("uView", view);
        shader3D.setMat4("uProj", proj);
        shader3D.setVec3("uViewPos", camPos);
        shader3D.setFloat("uTime", currentTimeSec);
        shader3D.setFloat("uShininess", 32.0f);

        // Core light & Flash light uniforms
        shader3D.setVec3("uCorePos", ARENA_CORE_POS);
        shader3D.setFloat("uCoreRadius", ARENA_CORE_RADIUS);
        shader3D.setInt("uIsCore", 0);
        shader3D.setInt("uIsFloorGrid", 0);

        glm::vec3 muzzleLightPos = camPos + g_Player.getForward() * 0.45f;
        shader3D.setVec3("uFlashPos", muzzleLightPos);
        shader3D.setFloat("uFlashIntensity", g_Player.muzzleFlashTimer > 0.0f ? 1.0f : 0.0f);

        // A. Wireframe Ground Grid Void
        {
            shader3D.setInt("uIsFloorGrid", 1);
            glm::mat4 floorModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -14.0f, 0.0f));
            floorModel = glm::scale(floorModel, glm::vec3(180.0f, 0.2f, 180.0f));
            shader3D.setMat4("uModel", floorModel);
            cubeMesh.draw();
            shader3D.setInt("uIsFloorGrid", 0);
        }

        // B. Platforms
        for (const auto& plat : g_Platforms) {
            // Main platform body
            glm::mat4 m = glm::translate(glm::mat4(1.0f), plat.pos);
            m = glm::scale(m, plat.size);
            shader3D.setMat4("uModel", m);
            shader3D.setVec3("uColor", plat.color);
            shader3D.setVec3("uEmissive", glm::vec3(0.0f));
            cubeMesh.draw();

            // Luminous glowing neon border strip on top of platform
            glm::mat4 mt = glm::translate(glm::mat4(1.0f), plat.pos + glm::vec3(0.0f, plat.size.y * 0.5f + 0.02f, 0.0f));
            mt = glm::scale(mt, glm::vec3(plat.size.x * 0.98f, 0.04f, plat.size.z * 0.98f));
            shader3D.setMat4("uModel", mt);
            shader3D.setVec3("uColor", plat.trimColor);
            shader3D.setVec3("uEmissive", plat.trimColor * 1.5f);
            cubeMesh.draw();
        }

        // C. Central Antigravity Core Orb
        {
            shader3D.setInt("uIsCore", 1);
            glm::mat4 coreModel = glm::translate(glm::mat4(1.0f), ARENA_CORE_POS);
            coreModel = glm::scale(coreModel, glm::vec3(3.4f));
            shader3D.setMat4("uModel", coreModel);
            sphereMesh.draw();
            shader3D.setInt("uIsCore", 0);

            // Outer gravitational distortion ring
            glm::mat4 ringModel = glm::translate(glm::mat4(1.0f), ARENA_CORE_POS);
            ringModel = glm::rotate(ringModel, currentTimeSec * 0.8f, glm::vec3(0.3f, 1.0f, 0.2f));
            ringModel = glm::scale(ringModel, glm::vec3(5.0f, 0.08f, 5.0f));
            shader3D.setMat4("uModel", ringModel);
            shader3D.setVec3("uColor", glm::vec3(0.3f, 0.8f, 1.0f));
            shader3D.setVec3("uEmissive", glm::vec3(0.4f, 1.0f, 1.5f));
            cubeMesh.draw();
        }

        // D. Floating Zero-G Props & Anchor Cubes
        for (const auto& prop : g_FloatingProps) {
            glm::mat4 m = glm::translate(glm::mat4(1.0f), prop.pos);
            m = glm::rotate(m, prop.currentRot, prop.rotAxis);
            m = glm::scale(m, prop.scale);
            shader3D.setMat4("uModel", m);
            shader3D.setVec3("uColor", prop.color);
            shader3D.setVec3("uEmissive", prop.emissive * 1.2f);
            cubeMesh.draw();
        }

        // E. Enemies (Sleek Combat Drones)
        for (const auto& e : g_Enemies) {
            if (!e.alive) continue;

            glm::mat4 em = glm::translate(glm::mat4(1.0f), e.pos);
            // Face player
            glm::vec3 toPlayer = glm::normalize(camPos - e.pos);
            float angleY = atan2(toPlayer.x, toPlayer.z);
            em = glm::rotate(em, angleY, glm::vec3(0, 1, 0));

            // Core Octahedron
            glm::mat4 emCore = glm::scale(em, glm::vec3(1.1f));
            shader3D.setMat4("uModel", emCore);
            glm::vec3 droneCol = (e.hitFlashTimer > 0.0f) ? glm::vec3(1.0f) : glm::vec3(0.12f, 0.13f, 0.18f);
            shader3D.setVec3("uColor", droneCol);
            shader3D.setVec3("uEmissive", (e.hitFlashTimer > 0.0f) ? glm::vec3(1.0f) : glm::vec3(0.1f, 0.02f, 0.08f));
            octaMesh.draw();

            // Glowing Red Eye Sensor in front
            glm::mat4 eyeM = glm::translate(em, glm::vec3(0.0f, 0.0f, 0.65f));
            eyeM = glm::scale(eyeM, glm::vec3(0.28f, 0.18f, 0.18f));
            shader3D.setMat4("uModel", eyeM);
            shader3D.setVec3("uColor", glm::vec3(1.0f, 0.1f, 0.15f));
            shader3D.setVec3("uEmissive", glm::vec3(2.5f, 0.2f, 0.3f));
            cubeMesh.draw();

            // Lateral Thruster Wings
            glm::mat4 wingL = glm::translate(em, glm::vec3(-0.95f, 0.0f, -0.2f));
            wingL = glm::scale(wingL, glm::vec3(0.35f, 0.12f, 0.5f));
            shader3D.setMat4("uModel", wingL);
            shader3D.setVec3("uColor", glm::vec3(0.08f));
            shader3D.setVec3("uEmissive", glm::vec3(1.8f, 0.2f, 0.2f));
            cubeMesh.draw();

            glm::mat4 wingR = glm::translate(em, glm::vec3(0.95f, 0.0f, -0.2f));
            wingR = glm::scale(wingR, glm::vec3(0.35f, 0.12f, 0.5f));
            shader3D.setMat4("uModel", wingR);
            shader3D.setVec3("uColor", glm::vec3(0.08f));
            shader3D.setVec3("uEmissive", glm::vec3(1.8f, 0.2f, 0.2f));
            cubeMesh.draw();
        }

        // F. Projectiles (Laser Bolts)
        for (const auto& p : g_Projectiles) {
            glm::mat4 pm = glm::translate(glm::mat4(1.0f), p.pos);
            glm::vec3 dir = glm::normalize(p.vel);
            glm::vec3 up = (std::abs(dir.y) > 0.95f) ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(0.0f, 1.0f, 0.0f);
            glm::vec3 right = glm::normalize(glm::cross(up, dir));
            up = glm::cross(dir, right);

            glm::mat4 rot(1.0f);
            rot[0] = glm::vec4(right, 0.0f);
            rot[1] = glm::vec4(up, 0.0f);
            rot[2] = glm::vec4(dir, 0.0f);

            pm = pm * rot;
            pm = glm::scale(pm, glm::vec3(0.12f, 0.12f, 0.95f));

            shader3D.setMat4("uModel", pm);
            shader3D.setVec3("uColor", p.color);
            shader3D.setVec3("uEmissive", p.color * 3.0f);
            cubeMesh.draw();
        }

        // G. Sparks & Explosion Particles
        for (const auto& part : g_Particles) {
            glm::mat4 partM = glm::translate(glm::mat4(1.0f), part.pos);
            partM = glm::scale(partM, glm::vec3(part.size));
            shader3D.setMat4("uModel", partM);
            shader3D.setVec3("uColor", part.color);
            shader3D.setVec3("uEmissive", part.color * 2.5f);
            cubeMesh.draw();
        }

        // ---------------------------------------------------------------------
        // PASS 2: FIRST-PERSON VIEWMODEL GUN (Rendered with depth cleared so it
        // never clips into arena walls)
        // ---------------------------------------------------------------------
        glClear(GL_DEPTH_BUFFER_BIT);
        renderGunViewModel(shader3D, cubeMesh, sphereMesh);

        // ---------------------------------------------------------------------
        // PASS 3: 2D HUD OVERLAY (Orthographic Pass)
        // ---------------------------------------------------------------------
        renderHUD(ui, shaderHUD);

        // Swap buffers
        glfwSwapBuffers(window);
    }

    // Cleanup resources
    glfwDestroyWindow(window);
    glfwTerminate();
    std::cout << "[INFO] Game terminated cleanly. Goodbye!\n";
    return 0;
}
