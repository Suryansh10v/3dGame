/**
 * ============================================================================
 * CHRONO-DASH: ANTIGRAVITY ARENA
 * Modular 3D First-Person Shooter in Modern OpenGL 3.3 Core Profile
 * Technologies: C++17, OpenGL 3.3 Core, GLFW, GLAD, GLM
 * ============================================================================
 */

#include "OpenGLHeaders.h"
#include "Constants.h"
#include "GameWorld.h"
#include "Renderer.h"
#include <iostream>
#include <algorithm>

// -----------------------------------------------------------------------------
// Global Application & Input State
// -----------------------------------------------------------------------------
static bool g_Keys[1024] = {false};
static bool g_MouseLeftPressed = false;
static bool g_MouseRightPressed = false;
static double g_LastMouseX = 640.0;
static double g_LastMouseY = 360.0;
static bool g_FirstMouse = true;
static int g_WindowWidth = Constants::DEFAULT_WINDOW_WIDTH;
static int g_WindowHeight = Constants::DEFAULT_WINDOW_HEIGHT;
static bool g_CursorCaptured = true;

static GameWorld* g_WorldInstance = nullptr;

// -----------------------------------------------------------------------------
// GLFW Callbacks
// -----------------------------------------------------------------------------
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        g_CursorCaptured = !g_CursorCaptured;
        glfwSetInputMode(window, GLFW_CURSOR, g_CursorCaptured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) g_Keys[key] = true;
        else if (action == GLFW_RELEASE) g_Keys[key] = false;
    }

    if (key == GLFW_KEY_R && action == GLFW_PRESS && g_WorldInstance) {
        if (g_WorldInstance->game.gameOver) {
            g_WorldInstance->reset();
        } else {
            g_WorldInstance->player.reload();
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
    if (!g_CursorCaptured || !g_WorldInstance) return;

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

    Player& player = g_WorldInstance->player;
    player.yaw += xoffset;
    player.pitch += yoffset;

    // Clamp pitch
    if (player.pitch > 89.0f) player.pitch = 89.0f;
    if (player.pitch < -89.0f) player.pitch = -89.0f;

    // Sway gun viewmodel with mouse delta
    player.gunYawSway = glm::clamp(player.gunYawSway - xoffset * 0.003f, -0.04f, 0.04f);
    player.gunPitchSway = glm::clamp(player.gunPitchSway + yoffset * 0.003f, -0.04f, 0.04f);
}

static void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    g_WindowWidth = width > 0 ? width : 1;
    g_WindowHeight = height > 0 ? height : 1;
    glViewport(0, 0, g_WindowWidth, g_WindowHeight);
}

// -----------------------------------------------------------------------------
// Main Function
// -----------------------------------------------------------------------------
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

    GLFWwindow* window = glfwCreateWindow(g_WindowWidth, g_WindowHeight, Constants::WINDOW_TITLE, nullptr, nullptr);
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

    // 4. Initialize Game Simulation & Renderer
    GameWorld world;
    g_WorldInstance = &world;

    Renderer renderer;
    if (!renderer.init()) {
        std::cerr << "Failed to initialize Renderer subsystems!" << std::endl;
        return -1;
    }

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
        if (dt > 0.05f) dt = 0.05f; // Clamped for physics numerical stability

        // Poll Events & Step Gameplay Simulation
        glfwPollEvents();
        float currentTimeSec = static_cast<float>(glfwGetTime());
        world.updateSimulation(dt, g_Keys, g_MouseLeftPressed, g_CursorCaptured, currentTimeSec);

        // Render Frame
        renderer.render(world, g_WindowWidth, g_WindowHeight, currentTimeSec);

        // Swap buffers
        glfwSwapBuffers(window);
    }

    // Cleanup resources
    glfwDestroyWindow(window);
    glfwTerminate();
    std::cout << "[INFO] Game terminated cleanly. Goodbye!\n";
    return 0;
}
