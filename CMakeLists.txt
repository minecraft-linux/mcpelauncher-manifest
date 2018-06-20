cmake_minimum_required(VERSION 3.0)

set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -m32")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

if(APPLE)
    set(CMAKE_LIBRARY_ARCHITECTURE "i386-apple-darwin")
else()
    set(CMAKE_LIBRARY_ARCHITECTURE "i386-linux-gnu")
endif(APPLE)

add_definitions(-DDEV_EXTRA_PATHS="${CMAKE_SOURCE_DIR}/mcpelauncher-linux-bin:${CMAKE_SOURCE_DIR}/mcpelauncher-proprietary")


include(ext/json.cmake)
include(ext/glfw.cmake)

add_subdirectory(logger)
add_subdirectory(base64)
add_subdirectory(file-util)
add_subdirectory(properties-parser)
add_subdirectory(arg-parser)

add_subdirectory(simple-ipc)
add_subdirectory(daemon-utils/client)
add_subdirectory(msa-daemon-client)

add_subdirectory(libhybris)
add_subdirectory(eglut)
add_subdirectory(linux-gamepad)
add_subdirectory(game-window)
add_subdirectory(file-picker)

add_subdirectory(cll-telemetry)

add_subdirectory(minecraft-symbols)
add_subdirectory(minecraft-imported-symbols)
add_subdirectory(mcpelauncher-common)
add_subdirectory(mcpelauncher-core)
add_subdirectory(mcpelauncher-client)
add_subdirectory(mcpelauncher-server)

