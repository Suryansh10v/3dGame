@echo off
setlocal
echo ========================================================
echo   Building Chrono-Dash: Antigravity Arena (OpenGL 3.3)
echo ========================================================

where cmake >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo [INFO] CMake detected. Building via CMake...
    cmake -B build -S .
    cmake --build build --config Release
    if exist "build\ChronoDash.exe" copy /y "build\ChronoDash.exe" "ChronoDash.exe" >nul
    echo [SUCCESS] Build complete! Executable: ChronoDash.exe
    exit /b 0
)

echo [INFO] Building directly with g++...
g++ -std=c++17 -O2 -Iinclude main.cpp src\gl.c src\Shader.cpp src\Mesh.cpp src\UIRenderer.cpp src\Player.cpp src\GameState.cpp src\GameWorld.cpp src\Renderer.cpp -lglfw3 -lopengl32 -lgdi32 -o ChronoDash.exe
if %ERRORLEVEL% EQU 0 (
    echo [SUCCESS] ChronoDash.exe built successfully!
) else (
    echo [ERROR] Build failed. Please verify g++, GLFW, and GLM.
)
endlocal
