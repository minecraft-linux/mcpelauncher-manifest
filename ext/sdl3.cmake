# Create an option to switch between a system sdl library and a vendored SDL library
option(SDL3_VENDORED "Use vendored sdl3 libraries" ON)

if(SDL3_VENDORED)
    set(SDL3_VENDORED ON CACHE BOOL "" FORCE)
    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC ON CACHE BOOL "" FORCE)
    set(SDL_CAMERA OFF CACHE BOOL "" FORCE)
    set(SDL_RENDER OFF CACHE BOOL "" FORCE)
    set(SDL_DIALOG OFF CACHE BOOL "" FORCE)
    set(SDL_SENSOR OFF CACHE BOOL "" FORCE)
    set(SDL_VULKAN OFF CACHE BOOL "" FORCE)
    set(SDL_METAL OFF CACHE BOOL "" FORCE)
    set(SDL_VIDEO_DRIVER_X11_SUPPORTS_GENERIC_EVENTS 1 CACHE STRING "" FORCE)
    add_subdirectory(sdl3 EXCLUDE_FROM_ALL)
else()
    # 1. Look for a SDL3 package,
    # 2. look for the SDL3-shared component, and
    # 3. fail if the shared component cannot be found.
    find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
endif()