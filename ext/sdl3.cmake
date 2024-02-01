# Create an option to switch between a system sdl library and a vendored SDL library
option(SDL3_VENDORED "Use vendored libraries" OFF)

if(SDL3_VENDORED)
    set(SDL3_VENDORED ON CACHE BOOL "" FORCE)
    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC ON CACHE BOOL "" FORCE)
    add_subdirectory(sdl3 EXCLUDE_FROM_ALL)
else()
    # 1. Look for a SDL3 package,
    # 2. look for the SDL3-shared component, and
    # 3. fail if the shared component cannot be found.
    find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
endif()