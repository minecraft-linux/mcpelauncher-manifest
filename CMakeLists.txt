cmake_minimum_required(VERSION 3.0)

enable_language(C CXX ASM)

set(CMAKE_CXX_STANDARD 17)

set(IS_64BIT FALSE)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(IS_64BIT TRUE)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-delete-null-pointer-checks")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

string(TIMESTAMP BUILD_TIMESTAMP "%Y%m%d-%H%M%S" UTC)

set(IS_ARM_BUILD FALSE)
set(IS_ARMHF_BUILD FALSE)
if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm" OR ${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64")
    set(IS_ARM_BUILD TRUE)
    set(IS_ARMHF_BUILD TRUE)
elseif(BUILD_X86)
    set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -m32")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
    if(APPLE)
        set(CMAKE_LIBRARY_ARCHITECTURE "i386-apple-darwin")
        set(CMAKE_CXX_COMPILER_TARGET "i686-apple-darwin")
    else()
        set(CMAKE_LIBRARY_ARCHITECTURE "i386-linux-gnu")
        set(CMAKE_CXX_COMPILER_TARGET "i686-linux-gnu")
    endif()
endif()

set(DEB_OS_NAME "none" CACHE STRING "Specifies the target OS for building the debs (ubuntu-xenial, ubuntu-bionic, ubuntu-disco)")
mark_as_advanced(DEB_OS_NAME)

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
option(BUILD_UI "Enables building of the client ui requires qt." ON)
option(BUILD_TESTING "Build tests for cll-telemetry (requires GTest)" OFF)

if (APPLE)
    set(NATIVES_PATH_DIR "${CMAKE_SOURCE_DIR}/mcpelauncher-mac-bin")
    set(NATIVES_PATH_LIB_DIR "${NATIVES_PATH_DIR}/lib")
else()
    set(NATIVES_PATH_DIR "${CMAKE_SOURCE_DIR}/mcpelauncher-linux-bin")
    set(NATIVES_PATH_LIB_DIR "${NATIVES_PATH_DIR}/lib")
endif()

option(ENABLE_DEV_PATHS "Enables lookup in source directories for misc files required by the launcher. This should be disabled for package builds." ON)
if (ENABLE_DEV_PATHS)
    add_definitions(-DDEV_EXTRA_PATHS="${NATIVES_PATH_DIR}:${CMAKE_BINARY_DIR}/gamecontrollerdb")
endif()

option(USE_OWN_CURL "Builds CURL as an ExternalProject dependency" ON)
if (USE_OWN_CURL)
    include(ext/curl.cmake)
endif()

include(ext/json.cmake)

option(USE_GAMECONTROLLERDB "Downloads gamecontrollerdb.txt from gabomdq/SDL_GameControllerDB" ON)
if (USE_GAMECONTROLLERDB AND BUILD_CLIENT)
    include(ext/gamepad_mappings.cmake)
endif()

option(USE_SNMALLOC "Use the snmalloc allocator" OFF)
option(USE_SDL3_AUDIO "Always use SDL audio" ON)

if (BUILD_CLIENT)
    if (USE_SNMALLOC)
        include(ext/snmalloc.cmake)
    endif()

    include(game-window/BuildSettings.cmake)
    if (USE_SDL3_AUDIO OR GAMEWINDOW_SYSTEM STREQUAL "SDL3")
        if(NOT GAMEWINDOW_SYSTEM STREQUAL "SDL3")
            set(SDL_WAYLAND OFF CACHE BOOL "" FORCE)
        endif()
        include(ext/sdl3.cmake)
    endif()
    if (GAMEWINDOW_SYSTEM STREQUAL "EGLUT")
        add_subdirectory(eglut)
        add_subdirectory(linux-gamepad)
    elseif (GAMEWINDOW_SYSTEM STREQUAL "GLFW")
        include(ext/glfw.cmake)
    endif()
endif()

option(BUILD_WEBVIEW "Build webview for xboxlive login" ON)
if (BUILD_WEBVIEW AND BUILD_UI)
    add_subdirectory(mcpelauncher-webview)
endif()

if (APPLE)
    add_subdirectory(osx-elf-header)
    add_subdirectory(epoll-shim)
endif()

if (BUILD_CLIENT)
    add_subdirectory(logger)
    add_subdirectory(base64)
    add_subdirectory(file-util)
    add_subdirectory(properties-parser)
    add_subdirectory(arg-parser)

    add_subdirectory(mcpelauncher-linker)
    add_subdirectory(libc-shim)
    # MSA is only required with the client
    add_subdirectory(simple-ipc)
    add_subdirectory(daemon-utils/client)
    add_subdirectory(daemon-utils/server)
    add_subdirectory(msa-daemon-client)

    add_subdirectory(file-picker)
    add_subdirectory(game-window)

    add_subdirectory(cll-telemetry)

    add_subdirectory(minecraft-imported-symbols)
    add_subdirectory(mcpelauncher-common)
    add_subdirectory(mcpelauncher-core)
endif()

if (BUILD_CLIENT)
    add_subdirectory(android-support-headers)
    set(JNIVM_ENABLE_RETURN_NON_ZERO ON CACHE BOOL "" FORCE)
    set(JNIVM_FAKE_JNI_MINECRAFT_LINUX_COMPAT ON CACHE BOOL "" FORCE)
    add_subdirectory(libjnivm)
    add_subdirectory(mcpelauncher-client)
endif()
if (BUILD_CLIENT OR BUILD_UI)
    add_subdirectory(mcpelauncher-errorwindow)
endif()

install(DIRECTORY ${NATIVES_PATH_LIB_DIR} COMPONENT mcpelauncher-bin-libs DESTINATION share/mcpelauncher PATTERN "*"
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)

option(MSA_DAEMON_PATH "Additional paths in which the MSA daemon should be looked in. Relative to the application directory." )
if (MSA_DAEMON_PATH)
    target_compile_definitions(mcpelauncher-client PRIVATE -DMSA_DAEMON_PATH="${MSA_DAEMON_PATH}")
endif()
