cmake_minimum_required(VERSION 3.0)

enable_language(C CXX ASM)

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -m32")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

string(TIMESTAMP BUILD_TIMESTAMP "%Y%m%d-%H%M%S" UTC)

set(IS_ARM_BUILD FALSE)
set(IS_ARMHF_BUILD FALSE)
if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm" OR ${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64")
    set(IS_ARM_BUILD TRUE)
    set(IS_ARMHF_BUILD TRUE)
endif()

if(APPLE)
    set(CMAKE_LIBRARY_ARCHITECTURE "i386-apple-darwin")
elseif(NOT IS_ARM_BUILD)
    set(CMAKE_LIBRARY_ARCHITECTURE "i386-linux-gnu")
endif()

set(DEB_OS_NAME "none" CACHE STRING "Specifies the target OS for building the debs (ubuntu-xenial, ubuntu-bionic, ubuntu-disco)")
mark_as_advanced(DEB_XENIAL_DEPENDENCIES)

find_program(GIT_EXEC git)
mark_as_advanced(GIT_EXEC ADVANCED)
function(git_commit_hash DIR OUT_VAL)
    if (GIT_EXEC)
        execute_process(
                COMMAND ${GIT_EXEC} log -1 --format=%h
                WORKING_DIRECTORY ${DIR}
                OUTPUT_VARIABLE OUTPUT
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        set(${OUT_VAL} ${OUTPUT} PARENT_SCOPE)
    else()
        set(${OUT_VAL} "" PARENT_SCOPE)
    endif()
endfunction()
git_commit_hash(${CMAKE_SOURCE_DIR} MANIFEST_GIT_COMMIT_HASH)

option(BUILD_CLIENT "Enables building of the client launcher." ON)
option(BUILD_SERVER "Enables building of the server launcher." ON)
option(BUILD_TESTING "Build tests for cll-telemetry (requires GTest)" OFF)

if (APPLE)
    set(NATIVES_PATH_DIR "${CMAKE_SOURCE_DIR}/mcpelauncher-mac-bin")
else()
    set(NATIVES_PATH_DIR "${CMAKE_SOURCE_DIR}/mcpelauncher-linux-bin")
endif()

option(ENABLE_DEV_PATHS "Enables lookup in source directories for misc files required by the launcher. This should be disabled for package builds." ON)
if (ENABLE_DEV_PATHS)
    add_definitions(-DDEV_EXTRA_PATHS="${NATIVES_PATH_DIR}:${CMAKE_BINARY_DIR}/gamecontrollerdb")
endif()

option(USE_OWN_CURL "Builds CURL as an ExternalProject dependency" OFF)
if (USE_OWN_CURL)
    include(ext/curl.cmake)
endif()

include(ext/json.cmake)

option(USE_GAMECONTROLLERDB "Downloads gamecontrollerdb.txt from gabomdq/SDL_GameControllerDB" ON)
if (USE_GAMECONTROLLERDB)
    include(ext/gamepad_mappings.cmake)
endif()

if (BUILD_CLIENT)
    include(game-window/BuildSettings.cmake)
    if (GAMEWINDOW_SYSTEM STREQUAL "EGLUT")
        add_subdirectory(eglut)
        add_subdirectory(linux-gamepad)
    elseif (GAMEWINDOW_SYSTEM STREQUAL "GLFW")
        include(ext/glfw.cmake)
    endif()
endif()

if (APPLE)
    add_subdirectory(osx-elf-header)
    add_subdirectory(epoll-shim)
endif()

add_subdirectory(logger)
add_subdirectory(base64)
add_subdirectory(file-util)
add_subdirectory(properties-parser)
add_subdirectory(arg-parser)

add_subdirectory(libhybris)

if (BUILD_CLIENT)
    # MSA is only required with the client
    add_subdirectory(simple-ipc)
    add_subdirectory(daemon-utils/client)
    add_subdirectory(msa-daemon-client)

    add_subdirectory(file-picker)
    add_subdirectory(game-window)

    add_subdirectory(cll-telemetry)
endif()

add_subdirectory(minecraft-symbols)
add_subdirectory(minecraft-imported-symbols)
add_subdirectory(mcpelauncher-common)
add_subdirectory(mcpelauncher-core)

if (BUILD_CLIENT)
    add_subdirectory(mcpelauncher-client)
endif()
if (BUILD_SERVER)
    add_subdirectory(mcpelauncher-server)
    add_subdirectory(mcpelauncher-just)
endif()

install(DIRECTORY ${NATIVES_PATH_DIR}/libs COMPONENT mcpelauncher-bin-libs DESTINATION share/mcpelauncher PATTERN "*"
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)

option(MSA_DAEMON_PATH "Additional paths in which the MSA daemon should be looked in. Relative to the application directory." )
if (MSA_DAEMON_PATH)
    target_compile_definitions(mcpelauncher-client PRIVATE -DMSA_DAEMON_PATH="${MSA_DAEMON_PATH}")
endif()