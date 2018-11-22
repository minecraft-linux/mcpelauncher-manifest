include(ExternalProject)

set(GAMECONTROLLERDB_DIR ${CMAKE_BINARY_DIR}/gamecontrollerdb)

ExternalProject_Add(
        gamecontrollerdb
        DOWNLOAD_DIR ${GAMECONTROLLERDB_DIR}
        URL "https://raw.githubusercontent.com/gabomdq/SDL_GameControllerDB/master/gamecontrollerdb.txt"
        "https://raw.githubusercontent.com/gabomdq/SDL_GameControllerDB/master/LICENSE"
        DOWNLOAD_NO_EXTRACT TRUE
        DOWNLOAD_NO_PROGRESS TRUE
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
)

install(FILES ${GAMECONTROLLERDB_DIR}/gamecontrollerdb.txt COMPONENT gamepad-mappings DESTINATION share/mcpelauncher
        PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ)