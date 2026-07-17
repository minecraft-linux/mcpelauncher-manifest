cmake_minimum_required(VERSION 3.0...4.0)

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

function(git_full_commit_hash DIR OUT_VAL)
    set(OUTPUT "")
    if (GIT_EXEC)
        execute_process(
                COMMAND ${GIT_EXEC} -C ${DIR} rev-parse HEAD
                RESULT_VARIABLE RESULT
                OUTPUT_VARIABLE OUTPUT
                ERROR_QUIET
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if (NOT RESULT EQUAL 0)
            set(OUTPUT "")
        endif()
    endif()
    set(${OUT_VAL} "${OUTPUT}" PARENT_SCOPE)
endfunction()

function(git_describe_version DIR OUT_VAL)
    set(OUTPUT "")
    if (GIT_EXEC)
        execute_process(
                COMMAND ${GIT_EXEC} -C ${DIR} describe --tags --always
                RESULT_VARIABLE RESULT
                OUTPUT_VARIABLE OUTPUT
                ERROR_QUIET
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if (NOT RESULT EQUAL 0)
            set(OUTPUT "")
        endif()
    endif()
    set(${OUT_VAL} "${OUTPUT}" PARENT_SCOPE)
endfunction()

function(git_exact_tag DIR OUT_VAL)
    set(OUTPUT "")
    if (GIT_EXEC)
        execute_process(
                COMMAND ${GIT_EXEC} -C ${DIR} describe --tags --exact-match
                RESULT_VARIABLE RESULT
                OUTPUT_VARIABLE OUTPUT
                ERROR_QUIET
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if (NOT RESULT EQUAL 0)
            set(OUTPUT "")
        endif()
    endif()
    set(${OUT_VAL} "${OUTPUT}" PARENT_SCOPE)
endfunction()

function(git_worktree_dirty DIR OUT_VAL)
    set(OUTPUT "")
    set(DIRTY "")
    if (GIT_EXEC)
        execute_process(
                COMMAND ${GIT_EXEC} -C ${DIR} status --porcelain --untracked-files=no
                RESULT_VARIABLE RESULT
                OUTPUT_VARIABLE OUTPUT
                ERROR_QUIET
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if (RESULT EQUAL 0)
            if (OUTPUT STREQUAL "")
                set(DIRTY "false")
            else()
                set(DIRTY "true")
            endif()
        endif()
    endif()
    set(${OUT_VAL} "${DIRTY}" PARENT_SCOPE)
endfunction()

function(git_dependency_entries DIR OUT_VAL)
    set(ENTRIES "")
    if (GIT_EXEC)
        execute_process(
                COMMAND ${GIT_EXEC} -C ${DIR} ls-tree HEAD
                RESULT_VARIABLE RESULT
                OUTPUT_VARIABLE OUTPUT
                ERROR_QUIET
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if (RESULT EQUAL 0)
            string(REPLACE "\n" ";" LINES "${OUTPUT}")
            foreach(LINE IN LISTS LINES)
                if (LINE MATCHES "^160000[ \t]+commit[ \t]+([0-9a-f]+)[\t](.+)$")
                    set(PINNED_REVISION "${CMAKE_MATCH_1}")
                    set(DEPENDENCY_PATH "${CMAKE_MATCH_2}")
                    set(ACTUAL_REVISION "")
                    set(DEPENDENCY_DIRTY "")
                    if (EXISTS "${DIR}/${DEPENDENCY_PATH}")
                        git_full_commit_hash("${DIR}/${DEPENDENCY_PATH}" ACTUAL_REVISION)
                        git_worktree_dirty("${DIR}/${DEPENDENCY_PATH}" DEPENDENCY_DIRTY)
                    endif()
                    set(ENTRIES "${ENTRIES}    {\"source/${DEPENDENCY_PATH}\", \"${ACTUAL_REVISION}\", \"${PINNED_REVISION}\", \"${DEPENDENCY_DIRTY}\"},\n")
                endif()
            endforeach()
        endif()
    endif()
    set(${OUT_VAL} "${ENTRIES}" PARENT_SCOPE)
endfunction()

git_full_commit_hash(${CMAKE_SOURCE_DIR} MANIFEST_GIT_COMMIT_HASH_FULL)
git_describe_version(${CMAKE_SOURCE_DIR} MANIFEST_GIT_DESCRIBE)
git_exact_tag(${CMAKE_SOURCE_DIR} MANIFEST_VERSION)
git_worktree_dirty(${CMAKE_SOURCE_DIR} MANIFEST_GIT_WORKTREE_DIRTY)
git_dependency_entries(${CMAKE_SOURCE_DIR} MANIFEST_DEPENDENCY_REVISION_ENTRIES)

# The manifest can be built standalone. Only identify a containing source
# baseline when it tracks this manifest directory as a gitlink.
set(SOURCE_BASELINE_ROOT_GIT_COMMIT_HASH "")
set(SOURCE_BASELINE_ROOT_GIT_DESCRIBE "")
set(SOURCE_BASELINE_ROOT_VERSION "")
set(SOURCE_BASELINE_ROOT_GIT_WORKTREE_DIRTY "")
if (GIT_EXEC)
    get_filename_component(MANIFEST_PARENT_DIR "${CMAKE_SOURCE_DIR}/.." ABSOLUTE)
    execute_process(
            COMMAND ${GIT_EXEC} -C ${MANIFEST_PARENT_DIR} rev-parse --show-toplevel
            RESULT_VARIABLE ROOT_RESULT
            OUTPUT_VARIABLE SOURCE_BASELINE_ROOT_DIR
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (ROOT_RESULT EQUAL 0)
        file(RELATIVE_PATH MANIFEST_ROOT_RELATIVE_PATH "${SOURCE_BASELINE_ROOT_DIR}" "${CMAKE_SOURCE_DIR}")
        execute_process(
                COMMAND ${GIT_EXEC} -C ${SOURCE_BASELINE_ROOT_DIR} ls-files --stage -- ${MANIFEST_ROOT_RELATIVE_PATH}
                RESULT_VARIABLE GITLINK_RESULT
                OUTPUT_VARIABLE GITLINK_ENTRY
                ERROR_QUIET
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if (GITLINK_RESULT EQUAL 0 AND GITLINK_ENTRY MATCHES "^160000[ \t]")
            git_full_commit_hash(${SOURCE_BASELINE_ROOT_DIR} SOURCE_BASELINE_ROOT_GIT_COMMIT_HASH)
            git_describe_version(${SOURCE_BASELINE_ROOT_DIR} SOURCE_BASELINE_ROOT_GIT_DESCRIBE)
            git_exact_tag(${SOURCE_BASELINE_ROOT_DIR} SOURCE_BASELINE_ROOT_VERSION)
            git_worktree_dirty(${SOURCE_BASELINE_ROOT_DIR} SOURCE_BASELINE_ROOT_GIT_WORKTREE_DIRTY)
        endif()
    endif()
endif()

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
            set(SDL_JOYSTICK OFF CACHE BOOL "" FORCE)
            set(SDL_VIRTUAL_JOYSTICK OFF CACHE BOOL "" FORCE)
            set(SDL_OFFSCREEN OFF CACHE BOOL "" FORCE)
            set(SDL_POWER OFF CACHE BOOL "" FORCE)
            set(SDL_GPU OFF CACHE BOOL "" FORCE)
            set(SDL_HAPTIC OFF CACHE BOOL "" FORCE)
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
    add_subdirectory(mcpelauncher-apkinfo)
    add_subdirectory(axml-parser)
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
