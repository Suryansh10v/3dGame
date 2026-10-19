#include "Renderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

bool Renderer::init() {
    if (!shader3D.init(vShader3DSource, fShader3DSource)) {
        std::cerr << "3D Shader compilation failed!" << std::endl;
        return false;
    }

    if (!shaderHUD.init(vShaderHUDSource, fShaderHUDSource)) {
        std::cerr << "HUD Shader compilation failed!" << std::endl;
        return false;
    }

    cubeMesh = Mesh3D::createCube();
    sphereMesh = Mesh3D::createSphere(24, 28);
    octaMesh = Mesh3D::createOctahedron();

    ui.init();
    return true;
}

void Renderer::render(const GameWorld& world, int windowWidth, int windowHeight, float currentTimeSec) {
    // Clear Color & Depth Buffers
    glClearColor(0.02f, 0.025f, 0.05f, 1.0f); // Deep twilight cyberpunk sky
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Camera & Projection
    glm::vec3 camPos = world.player.getCameraPos();
    glm::mat4 view = glm::lookAt(camPos, camPos + world.player.getForward(), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 proj = glm::perspective(glm::radians(75.0f), (float)windowWidth / (float)windowHeight, 0.08f, 180.0f);

    // PASS 1: 3D WORLD RENDERING
    renderWorld(world, view, proj, camPos, currentTimeSec);

    // PASS 2: FIRST-PERSON VIEWMODEL GUN (Rendered with depth cleared so it never clips)
    glClear(GL_DEPTH_BUFFER_BIT);
    renderGunViewModel(world.player, camPos);

    // PASS 3: 2D HUD OVERLAY (Orthographic Pass)
    renderHUD(world, windowWidth, windowHeight);
}

void Renderer::renderWorld(const GameWorld& world, const glm::mat4& view, const glm::mat4& proj, const glm::vec3& camPos, float currentTimeSec) {
    shader3D.use();
    shader3D.setMat4("uView", view);
    shader3D.setMat4("uProj", proj);
    shader3D.setVec3("uViewPos", camPos);
    shader3D.setFloat("uTime", currentTimeSec);
    shader3D.setFloat("uShininess", 32.0f);

    // Core light & Flash light uniforms
    shader3D.setVec3("uCorePos", Constants::ARENA_CORE_POS);
    shader3D.setFloat("uCoreRadius", Constants::ARENA_CORE_RADIUS);
    shader3D.setInt("uIsCore", 0);
    shader3D.setInt("uIsFloorGrid", 0);

    glm::vec3 muzzleLightPos = camPos + world.player.getForward() * 0.45f;
    shader3D.setVec3("uFlashPos", muzzleLightPos);
    shader3D.setFloat("uFlashIntensity", world.player.muzzleFlashTimer > 0.0f ? 1.0f : 0.0f);

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
    for (const auto& plat : world.platforms) {
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
        glm::mat4 coreModel = glm::translate(glm::mat4(1.0f), Constants::ARENA_CORE_POS);
        coreModel = glm::scale(coreModel, glm::vec3(3.4f));
        shader3D.setMat4("uModel", coreModel);
        sphereMesh.draw();
        shader3D.setInt("uIsCore", 0);

        // Outer gravitational distortion ring
        glm::mat4 ringModel = glm::translate(glm::mat4(1.0f), Constants::ARENA_CORE_POS);
        ringModel = glm::rotate(ringModel, currentTimeSec * 0.8f, glm::vec3(0.3f, 1.0f, 0.2f));
        ringModel = glm::scale(ringModel, glm::vec3(5.0f, 0.08f, 5.0f));
        shader3D.setMat4("uModel", ringModel);
        shader3D.setVec3("uColor", glm::vec3(0.3f, 0.8f, 1.0f));
        shader3D.setVec3("uEmissive", glm::vec3(0.4f, 1.0f, 1.5f));
        cubeMesh.draw();
    }

    // D. Floating Zero-G Props & Anchor Cubes
    for (const auto& prop : world.floatingProps) {
        glm::mat4 m = glm::translate(glm::mat4(1.0f), prop.pos);
        m = glm::rotate(m, prop.currentRot, prop.rotAxis);
        m = glm::scale(m, prop.scale);
        shader3D.setMat4("uModel", m);
        shader3D.setVec3("uColor", prop.color);
        shader3D.setVec3("uEmissive", prop.emissive * 1.2f);
        cubeMesh.draw();
    }

    // E. Enemies (Sleek Combat Drones)
    for (const auto& e : world.enemies) {
        if (!e.alive) continue;

        glm::mat4 em = glm::translate(glm::mat4(1.0f), e.pos);
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
    for (const auto& p : world.projectiles) {
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
    for (const auto& part : world.particles) {
        glm::mat4 partM = glm::translate(glm::mat4(1.0f), part.pos);
        partM = glm::scale(partM, glm::vec3(part.size));
        shader3D.setMat4("uModel", partM);
        shader3D.setVec3("uColor", part.color);
        shader3D.setVec3("uEmissive", part.color * 2.5f);
        cubeMesh.draw();
    }
}

void Renderer::renderGunViewModel(const Player& player, const glm::vec3& camPos) {
    glm::vec3 fwd = player.getForward();
    glm::vec3 right = player.getRight();
    glm::vec3 up = glm::cross(right, fwd);

    // Dynamic bobbing & sway offsets
    float bobX = std::sin(player.walkTimer * 0.8f) * 0.008f;
    float bobY = std::abs(std::cos(player.walkTimer * 0.8f)) * 0.007f;

    glm::vec3 gunOffset = right * (0.28f + player.gunYawSway + bobX) +
                          up    * (-0.22f + player.gunPitchSway + bobY) +
                          fwd   * (0.48f - player.gunRecoil * 0.08f);

    glm::vec3 gunBasePos = camPos + gunOffset;

    // Build rotation matrix from camera axes with recoil pitch
    glm::mat4 gunRot(1.0f);
    gunRot[0] = glm::vec4(right, 0.0f);
    gunRot[1] = glm::vec4(up, 0.0f);
    gunRot[2] = glm::vec4(-fwd, 0.0f);
    gunRot = glm::rotate(gunRot, glm::radians(player.gunRecoil * 6.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    auto drawPart = [&](const glm::vec3& localPos, const glm::vec3& scale, const glm::vec3& col, const glm::vec3& emissive) {
        glm::mat4 m = glm::translate(glm::mat4(1.0f), gunBasePos);
        m = m * gunRot;
        m = glm::translate(m, localPos);
        m = glm::scale(m, scale);

        shader3D.setMat4("uModel", m);
        shader3D.setVec3("uColor", col);
        shader3D.setVec3("uEmissive", emissive);
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
    if (player.muzzleFlashTimer > 0.0f) {
        glm::vec3 muzzlePos = gunBasePos + right * 0.0f + up * 0.015f + fwd * 0.34f;
        glm::mat4 mf = glm::translate(glm::mat4(1.0f), muzzlePos);
        mf = glm::scale(mf, glm::vec3(0.14f));
        shader3D.setMat4("uModel", mf);
        shader3D.setVec3("uColor", glm::vec3(1.0f));
        shader3D.setVec3("uEmissive", glm::vec3(1.5f, 1.8f, 2.5f));
        sphereMesh.draw();
    }
}

void Renderer::renderHUD(const GameWorld& world, int windowWidth, int windowHeight) {
    ui.clear();

    float w = static_cast<float>(windowWidth);
    float h = static_cast<float>(windowHeight);

    // Colors
    glm::vec4 colCyan(0.10f, 0.88f, 1.0f, 1.0f);
    glm::vec4 colOrange(1.0f, 0.45f, 0.05f, 1.0f);
    glm::vec4 colPurple(0.85f, 0.25f, 1.0f, 1.0f);
    glm::vec4 colRed(1.0f, 0.15f, 0.20f, 1.0f);
    glm::vec4 colWhite(0.95f, 0.98f, 1.0f, 1.0f);
    glm::vec4 colDarkPanel(0.02f, 0.04f, 0.08f, 0.65f);

    // A. TOP RIGHT: SCORE & STREAK DISPLAY
    float scorePanelW = 280.0f;
    float scorePanelH = 95.0f;
    float scoreX = w - scorePanelW - 30.0f;
    float scoreY = 30.0f;

    ui.addRect(scoreX, scoreY, scorePanelW, scorePanelH, colDarkPanel);
    ui.addRectOutline(scoreX, scoreY, scorePanelW, scorePanelH, 2.0f, colCyan * 0.8f);

    // Accent corner tabs
    ui.addRect(scoreX - 2.0f, scoreY - 2.0f, 16.0f, 4.0f, colCyan);
    ui.addRect(scoreX - 2.0f, scoreY - 2.0f, 4.0f, 16.0f, colCyan);
    ui.addRect(scoreX + scorePanelW - 14.0f, scoreY - 2.0f, 16.0f, 4.0f, colCyan);
    ui.addRect(scoreX + scorePanelW - 2.0f, scoreY - 2.0f, 4.0f, 16.0f, colCyan);

    // Score String: "SCORE: 12500"
    std::string scoreStr = "SCORE: " + std::to_string(world.game.score);
    ui.drawString(scoreStr, scoreX + 20.0f, scoreY + 16.0f, 14.0f, 22.0f, 4.0f, 2.5f, colWhite);

    // Streak String: "STREAK: 5X"
    std::string streakStr = "STREAK: " + std::to_string(world.game.streakMultiplier) + "X";
    glm::vec4 streakCol = (world.game.streakMultiplier > 1) ? colOrange : (colWhite * 0.7f);
    ui.drawString(streakStr, scoreX + 20.0f, scoreY + 48.0f, 12.0f, 18.0f, 3.5f, 2.2f, streakCol);

    // Multiplier gauge countdown bar underneath
    if (world.game.streakTimer > 0.0f) {
        float streakPct = world.game.streakTimer / GameState::maxStreakTime;
        float barW = (scorePanelW - 40.0f) * streakPct;
        ui.addRect(scoreX + 20.0f, scoreY + 76.0f, scorePanelW - 40.0f, 6.0f, glm::vec4(0.1f, 0.1f, 0.15f, 0.8f));
        ui.addRect(scoreX + 20.0f, scoreY + 76.0f, barW, 6.0f, colOrange);
    }

    // B. BOTTOM LEFT: HEALTH & STATUS
    float hpPanelW = 300.0f;
    float hpPanelH = 85.0f;
    float hpX = 30.0f;
    float hpY = h - hpPanelH - 30.0f;

    ui.addRect(hpX, hpY, hpPanelW, hpPanelH, colDarkPanel);
    ui.addRectOutline(hpX, hpY, hpPanelW, hpPanelH, 2.0f, colCyan * 0.7f);

    ui.drawString("CHRONO_HUNTER", hpX + 16.0f, hpY + 12.0f, 9.0f, 14.0f, 3.0f, 1.8f, colCyan);

    float hpPct = glm::clamp(world.player.health / world.player.maxHealth, 0.0f, 1.0f);
    glm::vec4 hpCol = (hpPct > 0.35f) ? colCyan : colRed;

    ui.drawString("HP: " + std::to_string((int)world.player.health), hpX + 16.0f, hpY + 34.0f, 11.0f, 16.0f, 3.0f, 2.0f, hpCol);

    float barMaxW = hpPanelW - 32.0f;
    ui.addRect(hpX + 16.0f, hpY + 56.0f, barMaxW, 12.0f, glm::vec4(0.1f, 0.1f, 0.15f, 0.8f));
    ui.addRect(hpX + 16.0f, hpY + 56.0f, barMaxW * hpPct, 12.0f, hpCol);

    // C. BOTTOM RIGHT: AMMO & WEAPON STATUS
    float ammoPanelW = 240.0f;
    float ammoPanelH = 85.0f;
    float ammoX = w - ammoPanelW - 30.0f;
    float ammoY = h - ammoPanelH - 30.0f;

    ui.addRect(ammoX, ammoY, ammoPanelW, ammoPanelH, colDarkPanel);
    ui.addRectOutline(ammoX, ammoY, ammoPanelW, ammoPanelH, 2.0f, colOrange * 0.7f);

    ui.drawString("PULSE_CANNON", ammoX + 16.0f, ammoY + 12.0f, 9.0f, 14.0f, 3.0f, 1.8f, colOrange);

    if (world.player.reloadTimer > 0.0f) {
        ui.drawString("RELOADING...", ammoX + 16.0f, ammoY + 36.0f, 12.0f, 18.0f, 3.0f, 2.0f, colOrange);
    } else {
        std::string ammoStr = "AMMO: " + std::to_string(world.player.ammo) + " / " + std::to_string(world.player.reserveAmmo);
        ui.drawString(ammoStr, ammoX + 16.0f, ammoY + 36.0f, 12.0f, 18.0f, 3.0f, 2.0f, colWhite);
    }

    float ammoPct = (float)world.player.ammo / (float)world.player.maxAmmo;
    ui.addRect(ammoX + 16.0f, ammoY + 60.0f, ammoPanelW - 32.0f, 10.0f, glm::vec4(0.1f, 0.1f, 0.15f, 0.8f));
    ui.addRect(ammoX + 16.0f, ammoY + 60.0f, (ammoPanelW - 32.0f) * ammoPct, 10.0f, colOrange);

    // D. CENTER TOP: ANTIGRAVITY STATUS NOTIFICATION BADGE
    float badgeW = 340.0f;
    float badgeH = 34.0f;
    float badgeX = (w - badgeW) * 0.5f;
    float badgeY = 30.0f;

    if (world.player.inAntigrav) {
        ui.addRect(badgeX, badgeY, badgeW, badgeH, glm::vec4(0.2f, 0.05f, 0.4f, 0.75f));
        ui.addRectOutline(badgeX, badgeY, badgeW, badgeH, 2.0f, colPurple);
        ui.drawString("[ ANTIGRAV: ACTIVE (ZERO-G) ]", badgeX + 14.0f, badgeY + 8.0f, 9.0f, 15.0f, 2.5f, 2.0f, colCyan);
    } else {
        ui.addRect(badgeX, badgeY, badgeW, badgeH, glm::vec4(0.04f, 0.06f, 0.1f, 0.5f));
        ui.addRectOutline(badgeX, badgeY, badgeW, badgeH, 1.5f, colCyan * 0.5f);
        ui.drawString("[ GRAVITY: 1.0G NOMINAL ]", badgeX + 36.0f, badgeY + 8.0f, 9.0f, 15.0f, 2.5f, 1.8f, colWhite * 0.7f);
    }

    // E. CENTER RETICLE / CROSSHAIR
    float cx = w * 0.5f;
    float cy = h * 0.5f;
    float spread = 12.0f + world.player.gunRecoil * 18.0f;
    float retThick = 2.0f;
    float bracketLen = 14.0f;

    ui.addRect(cx - 1.5f, cy - 1.5f, 3.0f, 3.0f, colCyan);

    // 4 Corner brackets [ ]
    ui.addLine({cx - spread - bracketLen, cy - spread}, {cx - spread, cy - spread}, retThick, colCyan);
    ui.addLine({cx - spread, cy - spread}, {cx - spread, cy - spread - bracketLen}, retThick, colCyan);

    ui.addLine({cx + spread + bracketLen, cy - spread}, {cx + spread, cy - spread}, retThick, colCyan);
    ui.addLine({cx + spread, cy - spread}, {cx + spread, cy - spread - bracketLen}, retThick, colCyan);

    ui.addLine({cx - spread - bracketLen, cy + spread}, {cx - spread, cy + spread}, retThick, colCyan);
    ui.addLine({cx - spread, cy + spread}, {cx - spread, cy + spread + bracketLen}, retThick, colCyan);

    ui.addLine({cx + spread + bracketLen, cy + spread}, {cx + spread, cy + spread}, retThick, colCyan);
    ui.addLine({cx + spread, cy + spread}, {cx + spread, cy + spread + bracketLen}, retThick, colCyan);

    // F. DAMAGE VIGNETTE / SCREEN FLASH
    if (world.game.damageFlash > 0.01f) {
        float f = world.game.damageFlash;
        glm::vec4 flashCol(0.9f, 0.05f, 0.1f, f * 0.45f);
        ui.addRect(0, 0, w, 24.0f, flashCol);
        ui.addRect(0, h - 24.0f, w, 24.0f, flashCol);
        ui.addRect(0, 0, 24.0f, h, flashCol);
        ui.addRect(w - 24.0f, 0, 24.0f, h, flashCol);
    }

    // G. GAME OVER OVERLAY
    if (world.game.gameOver) {
        ui.addRect(0, 0, w, h, glm::vec4(0.02f, 0.02f, 0.05f, 0.85f));
        float goW = 480.0f;
        float goH = 220.0f;
        float goX = (w - goW) * 0.5f;
        float goY = (h - goH) * 0.5f;

        ui.addRect(goX, goY, goW, goH, glm::vec4(0.04f, 0.05f, 0.09f, 0.95f));
        ui.addRectOutline(goX, goY, goW, goH, 3.0f, colRed);

        ui.drawString("MISSION FAILED", goX + 80.0f, goY + 35.0f, 20.0f, 30.0f, 5.0f, 3.5f, colRed);
        std::string finalScore = "FINAL SCORE: " + std::to_string(world.game.score);
        ui.drawString(finalScore, goX + 110.0f, goY + 95.0f, 14.0f, 22.0f, 4.0f, 2.5f, colWhite);
        ui.drawString("PRESS [R] TO RESPAWN", goX + 85.0f, goY + 150.0f, 13.0f, 20.0f, 3.5f, 2.2f, colCyan);
    }

    // Draw accumulated HUD vertices
    shaderHUD.use();
    glm::mat4 orthoProj = glm::ortho(0.0f, w, h, 0.0f, -1.0f, 1.0f);
    shaderHUD.setMat4("uProj", orthoProj);
    shaderHUD.setVec4("uTint", glm::vec4(1.0f));

    glDisable(GL_DEPTH_TEST);
    ui.render();
    glEnable(GL_DEPTH_TEST);
}
