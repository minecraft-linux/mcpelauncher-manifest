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

option(BUILD_TESTING "Build tests for cll-telemetry (requires GTest)" OFF)

if (APPLE)
    set(NATIVES_PATH_DIR "${CMAKE_SOURCE_DIR}/mcpelauncher-mac-bin")
else()
    set(NATIVES_PATH_DIR "${CMAKE_SOURCE_DIR}/mcpelauncher-linux-bin")
endif()

option(ENABLE_DEV_PATHS "Enables lookup in source directories for misc files required by the launcher. This should be disabled for package builds." ON)
if (ENABLE_DEV_PATHS)
    add_definitions(-DDEV_EXTRA_PATHS="${NATIVES_PATH_DIR}")
endif()


include(ext/json.cmake)

if (APPLE)
    add_subdirectory(osx-elf-header)
    add_subdirectory(epoll-shim)
endif()

add_subdirectory(logger)
add_subdirectory(base64)
add_subdirectory(file-util)
add_subdirectory(properties-parser)
add_subdirectory(arg-parser)

add_subdirectory(simple-ipc)
add_subdirectory(daemon-utils/client)
add_subdirectory(msa-daemon-client)

add_subdirectory(libhybris)
add_subdirectory(file-picker)
add_subdirectory(game-window)
if (GAMEWINDOW_SYSTEM STREQUAL "EGLUT")
    add_subdirectory(eglut)
    add_subdirectory(linux-gamepad)
elseif (GAMEWINDOW_SYSTEM STREQUAL "GLFW")
    include(ext/glfw.cmake)
endif()

add_subdirectory(cll-telemetry)

add_subdirectory(minecraft-symbols)
add_subdirectory(minecraft-imported-symbols)
add_subdirectory(mcpelauncher-common)
add_subdirectory(mcpelauncher-core)
add_subdirectory(mcpelauncher-client)
add_subdirectory(mcpelauncher-server)

install(DIRECTORY ${NATIVES_PATH_DIR}/libs COMPONENT mcpelauncher-bin-libs DESTINATION share/mcpelauncher PATTERN "*"
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)

option(MSA_DAEMON_PATH "Additional paths in which the MSA daemon should be looked in. Relative to the application directory." )
if (MSA_DAEMON_PATH)
    target_compile_definitions(mcpelauncher-client PRIVATE -DMSA_DAEMON_PATH="${MSA_DAEMON_PATH}")
endif()
