cmake_minimum_required(VERSION 3.16)
project(ChronoDash LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 QUIET)
find_package(glm QUIET)

add_executable(ChronoDash
    main.cpp
    src/gl.c
)

target_include_directories(ChronoDash PRIVATE include)

if(TARGET glfw)
    target_link_libraries(ChronoDash PRIVATE glfw)
elseif(glfw3_FOUND)
    target_link_libraries(ChronoDash PRIVATE glfw)
else()
    target_link_libraries(ChronoDash PRIVATE glfw3)
endif()

if(TARGET glm::glm)
    target_link_libraries(ChronoDash PRIVATE glm::glm)
elseif(glm_FOUND)
    target_link_libraries(ChronoDash PRIVATE glm)
endif()

target_link_libraries(ChronoDash PRIVATE OpenGL::GL)

if(WIN32)
    target_link_libraries(ChronoDash PRIVATE gdi32)
endif()
