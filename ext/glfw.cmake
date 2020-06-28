include(FetchContent)

FetchContent_Declare(
        glfw3_ext
        URL "https://github.com/minecraft-linux/glfw/archive/master.zip"
)

FetchContent_GetProperties(glfw3_ext)
if(NOT glfw3_ext_POPULATED)
  FetchContent_Populate(glfw3_ext)
  add_subdirectory(${glfw3_ext_SOURCE_DIR} ${glfw3_ext_BINARY_DIR})
endif()
add_library(glfw3 ALIAS glfw)