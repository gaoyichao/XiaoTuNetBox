cmake_minimum_required(VERSION 3.1.0)
project(XiaoTuNetBox)
set(PACKAGE_VERSION "0.0.7")

add_compile_options(-std=c++17)
set(CMAKE_BUILD_TYPE DEBUG)

message("${PROJECT_NAME}")
message("${PROJECT_SOURCE_DIR}")
message("${PROJECT_BINARY_DIR}")
message("${CMAKE_INSTALL_PREFIX}")

include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

############################################################################
#
# libXiaoTuNetBox.so
#
############################################################################

file(GLOB PROJECT_SRC_FILES ${PROJECT_SOURCE_DIR}/src/*cpp)
message("${PROJECT_SRC_FILES}")

file(GLOB PROJECT_INC_FILES
          ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/*.h
          ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/*.hpp)
message("${PROJECT_INC_FILES}")

add_library(${PROJECT_NAME} SHARED
    ${PROJECT_SRC_FILES}
)
target_link_libraries(${PROJECT_NAME})

############################################################################
#
# utils
#
############################################################################

############################################################################
#
# tests 
#
############################################################################

add_executable(t_Address test/t_Address.cpp)
target_link_libraries(t_Address gtest gtest_main pthread rt ${PROJECT_NAME})

#add_executable(t_HttpServer test/非阻塞HttpServer.cpp)
#target_link_libraries(t_HttpServer ${PROJECT_NAME})

############################################################################
#
# install 
#
############################################################################

include(cmake/install_package.cmake)
install_package()

INSTALL(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
    ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
)

INSTALL(FILES ${PROJECT_INC_FILES}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${PROJECT_NAME}
)

add_custom_target("uninstall" COMMENT "Uninstall installed files")
add_custom_command(
    TARGET "uninstall"
    POST_BUILD
    COMMENT "Uninstall files with install_manifest.txt"
    COMMAND xargs rm -vf < install_manifest.txt || echo Nothing in
            install_manifest.txt to be uninstalled!
)


