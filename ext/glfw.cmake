include(ExternalProject)

ExternalProject_Add(
        glfw3_ext
        URL "https://github.com/glfw/glfw/archive/master.zip"
        INSTALL_DIR ${CMAKE_BINARY_DIR}/ext/glfw
        CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/ext/glfw" "-DBUILD_SHARED_LIBS=OFF" "-DCMAKE_C_FLAGS=-m32" "-DCMAKE_LINK_FLAGS=-m32" "-DCMAKE_LIBRARY_ARCHITECTURE=${CMAKE_LIBRARY_ARCHITECTURE}"
)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/ext/glfw/include/)
add_library(glfw3 STATIC IMPORTED)
add_dependencies(glfw3 glfw3_ext)
set_property(TARGET glfw3 PROPERTY IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/ext/glfw/lib/libglfw3.a)
if (APPLE)
    set_property(TARGET glfw3 PROPERTY INTERFACE_LINK_LIBRARIES "-framework Cocoa" "-framework IOKit" "-framework CoreFoundation" "-framework CoreVideo")
else()
    set_property(TARGET glfw3 PROPERTY INTERFACE_LINK_LIBRARIES ${X11_X11_LIB} ${X11_Xcursor_LIB} ${X11_Xrandr_LIB} ${X11_Xxf86vm_LIB} ${X11_Xinerama_LIB})
endif()
set_property(TARGET glfw3 PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_BINARY_DIR}/ext/glfw/include/)
