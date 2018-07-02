include(ExternalProject)

ExternalProject_Add(
        nlohmann_json_ext
        URL "https://github.com/nlohmann/json/releases/download/v3.1.2/include.zip"
        CONFIGURE_COMMAND "" BUILD_COMMAND "" INSTALL_DIR "" INSTALL_COMMAND ""
)
ExternalProject_Get_Property(nlohmann_json_ext SOURCE_DIR)
file(MAKE_DIRECTORY ${SOURCE_DIR})
add_library(nlohmann_json INTERFACE IMPORTED)
add_dependencies(nlohmann_json nlohmann_json_ext)
set_property(TARGET nlohmann_json PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${SOURCE_DIR})
