#include "Shader.h"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

// -----------------------------------------------------------------------------
// EMBEDDED GLSL SHADER SOURCES
// -----------------------------------------------------------------------------

// 3D Scene Vertex Shader
const char* vShader3DSource = R"(#version 330 core
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
const char* fShader3DSource = R"(#version 330 core
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
const char* vShaderHUDSource = R"(#version 330 core
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
const char* fShaderHUDSource = R"(#version 330 core
in vec4 vColor;
out vec4 FragColor;

uniform vec4 uTint;

void main() {
    FragColor = vColor * uTint;
}
)";

// -----------------------------------------------------------------------------
// SHADERPROGRAM IMPLEMENTATION
// -----------------------------------------------------------------------------

ShaderProgram::~ShaderProgram() {
    if (id != 0) {
        glDeleteProgram(id);
        id = 0;
    }
}

bool ShaderProgram::init(const char* vSrc, const char* fSrc) {
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

void ShaderProgram::use() const {
    glUseProgram(id);
}

void ShaderProgram::setMat4(const std::string& name, const glm::mat4& mat) const {
    glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

void ShaderProgram::setVec3(const std::string& name, const glm::vec3& val) const {
    glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, glm::value_ptr(val));
}

void ShaderProgram::setVec4(const std::string& name, const glm::vec4& val) const {
    glUniform4fv(glGetUniformLocation(id, name.c_str()), 1, glm::value_ptr(val));
}

void ShaderProgram::setFloat(const std::string& name, float val) const {
    glUniform1f(glGetUniformLocation(id, name.c_str()), val);
}

void ShaderProgram::setInt(const std::string& name, int val) const {
    glUniform1i(glGetUniformLocation(id, name.c_str()), val);
}

GLuint ShaderProgram::compile(GLenum type, const char* src) {
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
