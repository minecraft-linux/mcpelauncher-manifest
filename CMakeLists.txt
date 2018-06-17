cmake_minimum_required(VERSION 3.0)

set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -m32")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

option(BUILD_TESTING "Build tests for cll-telemetry (requires GTest)" OFF)

if(APPLE)
    set(CMAKE_LIBRARY_ARCHITECTURE "i386-apple-darwin")
else()
    set(CMAKE_LIBRARY_ARCHITECTURE "i386-linux-gnu")
endif(APPLE)

include(ext/json.cmake)
include(ext/glfw.cmake)

add_subdirectory(logger)
add_subdirectory(base64)
add_subdirectory(file-util)
add_subdirectory(properties-parser)

add_subdirectory(simple-ipc)
add_subdirectory(daemon-utils/client)
add_subdirectory(msa-daemon-client)

add_subdirectory(libhybris)

if (NOT APPLE)
    add_subdirectory(eglut)
    add_subdirectory(linux-gamepad)
else()
    add_subdirectory(osx-elf-header)
    add_subdirectory(epoll-shim)
endif(NOT APPLE)

add_subdirectory(game-window)
add_subdirectory(file-picker)

add_subdirectory(cll-telemetry)

add_subdirectory(minecraft-symbols)
add_subdirectory(minecraft-imported-symbols)
add_subdirectory(mcpelauncher-common)
add_subdirectory(mcpelauncher-core)
add_subdirectory(mcpelauncher-client)
add_subdirectory(mcpelauncher-server)
