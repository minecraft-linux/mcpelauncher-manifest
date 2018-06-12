include(ExternalProject)

ExternalProject_Add(
        nlohmann_json_ext
        URL "https://github.com/nlohmann/json/releases/download/v3.1.2/include.zip"
        CONFIGURE_COMMAND "" BUILD_COMMAND "" INSTALL_DIR "" INSTALL_COMMAND ""
)
ExternalProject_Get_Property(nlohmann_json_ext SOURCE_DIR)
add_library(nlohmann_json INTERFACE IMPORTED)
add_dependencies(nlohmann_json nlohmann_json_ext)
target_include_directories(nlohmann_json INTERFACE ${SOURCE_DIR})