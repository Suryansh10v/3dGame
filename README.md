# Chrono-Dash: Antigravity Arena 🎮🌌

A fast-paced, high-fidelity 3D first-person shooter set in a minimalist cyberpunk floating arena, built with **C++17** and **Modern OpenGL 3.3 Core Profile** (GLFW, GLAD, and GLM).

---

## 🌟 Key Features

### 1. Visuals & Graphics (Clean Cyberpunk Sci-Fi Aesthetic)
* **Modern OpenGL 3.3 Pipeline**: Custom GLSL vertex and fragment shaders embedded directly as string literals.
* **Minimalist Wireframe Floor Grid**: Procedural GLSL anti-aliased dual-frequency grid floating over an atmospheric twilight void.
* **Floating Geometric Arena**: Clean dark metallic cubic and rectangular platforms trimmed with vibrant neon cyan, amber, and magenta light strips.
* **Pulsing Antigravity Core**: A central shimmering blue-purple gravitational orb rendered with animated Fresnel rim lighting, swirling plasma waves, and dynamic point illumination.
* **Zero-G Floating Props**: Floating anchor cubes and geometric debris drifting weightlessly around the central anomaly.
* **Rendered Viewmodel Weapon**: Sleek white-and-orange futuristic railgun rendered in first-person with dynamic walk bobbing, mouse look sway, kickback recoil, and glowing muzzle flash.

### 2. Gameplay & Physics Engine
* **Fluid FPS Movement**: Responsive WASD ground acceleration, air control, and mouse freelook camera control.
* **Dynamic Antigravity Field Mechanics**:
  * Proximity detection against the central energy core (`radius = 16.5m`).
  * **Outside field**: Standard downward gravity (`-25 m/s²`) and crisp ground jumps.
  * **Inside field**: Gravitational field flips to near zero-G floatiness (`-2.2 m/s²`), enabling 3D vertical thrusting via `Space` (ascend) and `Shift`/`Ctrl` (descend).
* **Shooting & Weapon Mechanics**: Rapid-fire glowing cyan laser bolts (`72 m/s`) with muzzle flash, dynamic point lighting, and projectile trail particles.
* **Collision System**: Continuous AABB detection for player-platform surfaces, laser-to-enemy hits, enemy laser-to-player hits, and platform barriers.
* **Void Recovery**: Antigravity emergency safety field that propels the player upward if falling into the void.

### 3. Enemy AI & Combat
* **Hostile Drone Interceptors**: Sleek glowing octahedron combat drones with glowing red optical sensors and lateral thrusters.
* **Tracking & Strafing AI**: Drones hover, adjust distance, circle the player, and fire purple laser bolts.
* **Particle FX**: Multi-stage spark and debris explosions on drone destruction.

### 4. 2D HUD Overlay (Pure OpenGL Vector Renderer)
* **Zero External Texture Dependencies**: Renders high-resolution vector strokes directly on screen with anti-aliasing and depth-tested isolation.
* **Score System**: Live score tracking (`+100` base per kill) displayed in the top right.
* **Streak & Multiplier**: Dynamic multiplier (`1x`, `2x`, `3x`, `5x`) with active countdown bar.
* **Health & Ammo Telemetry**: Neon-styled HP gauge, current/reserve ammo indicators, and reload prompts.
* **Cyberpunk Crosshair Reticle**: Dynamic center dot with 4 corner brackets that expand with recoil and movement.
* **Antigravity Status Indicator**: Real-time badge alerting the player when zero-G is engaged.

---

## 🕹️ Controls

| Action | Key / Input |
|---|---|
| **Move** | `W`, `A`, `S`, `D` |
| **Look Around** | Mouse |
| **Jump / Ascend (Zero-G)** | `Spacebar` |
| **Descend (Zero-G)** | `Left Shift` / `Left Ctrl` |
| **Fire Pulse Cannon** | `Left Mouse Button` |
| **Reload** | `R` |
| **Toggle Cursor Lock / Pause** | `Escape` |
| **Respawn (After Game Over)** | `R` |

---

## 🛠️ Build & Run Instructions

### Prerequisites
* Windows 10/11
* MinGW-w64 (`g++` with C++17 support)
* GLFW3 & GLM headers (included / in MSYS2 path)

### Option 1: Quick Batch Script (Recommended)
Simply double-click or run:
```cmd
build.bat
```
Then launch:
```cmd
run.bat
```

### Option 2: Direct Compiler Command
```bash
g++ -std=c++17 -O2 -Iinclude main.cpp src/gl.c src/Shader.cpp src/Mesh.cpp src/UIRenderer.cpp src/Player.cpp src/GameState.cpp src/GameWorld.cpp src/Renderer.cpp -lglfw3 -lopengl32 -lgdi32 -o ChronoDash.exe
./ChronoDash.exe
```

### Option 3: CMake
```bash
cmake -B build -S .
cmake --build build --config Release
./build/ChronoDash.exe
```

---

## 📂 Project Structure

```
CgGameCie3/
├── CMakeLists.txt            # CMake configuration with modular source targets
├── build.bat                 # One-click Windows build script (CMake / direct g++)
├── run.bat                   # One-click launcher for ChronoDash.exe
├── include/
│   ├── OpenGLHeaders.h       # Unified GLAD & GLFW cross-loader inclusion
│   ├── Constants.h           # Arena center, core radius, window presets
│   ├── Types.h               # Vertex3D, UIVertex, AABB, Platform, Projectile, Particle, Enemy
│   ├── Shader.h              # ShaderProgram wrapper class and GLSL declarations
│   ├── Mesh.h                # Mesh3D class and procedural geometry generators
│   ├── UIRenderer.h          # 2D HUD vector stroke typography & quad renderer
│   ├── Player.h              # Player FPS camera, movement physics, weapon dynamics
│   ├── GameState.h           # Score tracking, hit streak multipliers, countdown timers
│   ├── GameWorld.h           # Arena entity management, physics loop, AI behavior, firing
│   ├── Renderer.h            # 3-Pass graphics rendering pipeline (World, Gun, HUD)
│   ├── glad/                 # GLAD 2.0.8 OpenGL 3.3 Core headers
│   └── KHR/                  # Khronos platform headers
├── src/
│   ├── gl.c                  # GLAD OpenGL symbol loader implementation
│   ├── Shader.cpp            # Shader compilation, linking, and embedded GLSL sources
│   ├── Mesh.cpp              # Vertex arrays / buffers and procedural mesh algorithms
│   ├── UIRenderer.cpp        # Vector font glyph strokes, string rasterizer, HUD geometry
│   ├── Player.cpp            # Camera vectors, AABB bounds, reload logic
│   ├── GameState.cpp         # Hit processing, streak multipliers, countdown, reset
│   ├── GameWorld.cpp         # Arena generation, antigravity physics, AI steering, collisions
│   ├── Renderer.cpp          # Pass 1 (World), Pass 2 (Viewmodel Gun), Pass 3 (2D HUD)
│   └── main.cpp              # Application entry point, GLFW window, input callbacks, loop
```
