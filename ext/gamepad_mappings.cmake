set(GAMECONTROLLERDB_DIR ${CMAKE_BINARY_DIR}/gamecontrollerdb)

add_custom_target(gamecontrollerdb ALL
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_SOURCE_DIR}/ext/gamepad_mappings_download.cmake)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${GAMECONTROLLERDB_DIR})

install(FILES ${GAMECONTROLLERDB_DIR}/gamecontrollerdb.txt COMPONENT gamepad-mappings DESTINATION share/mcpelauncher
        PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ)