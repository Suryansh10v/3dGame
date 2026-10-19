#pragma once

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
