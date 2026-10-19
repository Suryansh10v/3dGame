#pragma once

#include "Shader.h"
#include "Mesh.h"
#include "UIRenderer.h"
#include "GameWorld.h"

class Renderer {
public:
    ShaderProgram shader3D;
    ShaderProgram shaderHUD;
    Mesh3D cubeMesh;
    Mesh3D sphereMesh;
    Mesh3D octaMesh;
    UIRenderer ui;

    Renderer() = default;
    ~Renderer() = default;

    bool init();
    void render(const GameWorld& world, int windowWidth, int windowHeight, float currentTimeSec);

private:
    void renderWorld(const GameWorld& world, const glm::mat4& view, const glm::mat4& proj, const glm::vec3& camPos, float currentTimeSec);
    void renderGunViewModel(const Player& player, const glm::vec3& camPos);
    void renderHUD(const GameWorld& world, int windowWidth, int windowHeight);
};
