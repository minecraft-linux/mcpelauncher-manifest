include(ExternalProject)

ExternalProject_Add(
        glfw3_ext
        URL "https://github.com/glfw/glfw/archive/master.zip"
        INSTALL_DIR ${CMAKE_BINARY_DIR}/ext/glfw
        CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/ext/glfw" "-DBUILD_SHARED_LIBS=OFF" "-DCMAKE_C_FLAGS=-m32" "-DCMAKE_LINK_FLAGS=-m32" "-DCMAKE_LIBRARY_ARCHITECTURE=${CMAKE_LIBRARY_ARCHITECTURE}"
)
add_library(glfw3 STATIC IMPORTED)
add_dependencies(glfw3 glfw3_ext)
set_property(TARGET glfw3 PROPERTY IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/ext/glfw/lib/libglfw3.a)
target_link_libraries(glfw3 INTERFACE ${X11_X11_LIB} ${X11_Xcursor_LIB} ${X11_Xrandr_LIB} ${X11_Xxf86vm_LIB} ${X11_Xinerama_LIB})
target_include_directories(glfw3 INTERFACE ${CMAKE_BINARY_DIR}/ext/glfw/include/)