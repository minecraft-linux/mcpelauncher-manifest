set(GAMECONTROLLERDB_DIR ${CMAKE_BINARY_DIR}/gamecontrollerdb)

if (NOT EXISTS ${GAMECONTROLLERDB_DIR}/gamecontrollerdb.txt OR NOT EXISTS ${GAMECONTROLLERDB_DIR}/LICENSE)
    message("Downloading gamecontrollerdb.txt")
    file(DOWNLOAD "https://raw.githubusercontent.com/gabomdq/SDL_GameControllerDB/master/gamecontrollerdb.txt"
            ${GAMECONTROLLERDB_DIR}/gamecontrollerdb.txt)
    file(DOWNLOAD "https://raw.githubusercontent.com/gabomdq/SDL_GameControllerDB/master/LICENSE"
            ${GAMECONTROLLERDB_DIR}/LICENSE)
endif()