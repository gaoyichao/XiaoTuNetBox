include(CMakeParseArguments)
get_filename_component(modules_dir ${CMAKE_CURRENT_LIST_FILE} PATH)

function(install_package)
    message(STATUS "install package:"  ${PROJECT_NAME})

    configure_file(${modules_dir}/config.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake" @ONLY )

    configure_file(${modules_dir}/config_version.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake" @ONLY )

    INSTALL(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
        DESTINATION
        ${CMAKE_INSTALL_PREFIX}/lib/cmake/${PROJECT_NAME}
    )

endfunction()
