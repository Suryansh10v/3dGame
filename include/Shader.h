#pragma once

#include "OpenGLHeaders.h"
#include <glm/glm.hpp>
#include <string>

// Embedded GLSL shader sources
extern const char* vShader3DSource;
extern const char* fShader3DSource;
extern const char* vShaderHUDSource;
extern const char* fShaderHUDSource;

class ShaderProgram {
public:
    GLuint id = 0;

    ShaderProgram() = default;
    ~ShaderProgram();

    bool init(const char* vSrc, const char* fSrc);
    void use() const;

    void setMat4(const std::string& name, const glm::mat4& mat) const;
    void setVec3(const std::string& name, const glm::vec3& val) const;
    void setVec4(const std::string& name, const glm::vec4& val) const;
    void setFloat(const std::string& name, float val) const;
    void setInt(const std::string& name, int val) const;

private:
    static GLuint compile(GLenum type, const char* src);
};
