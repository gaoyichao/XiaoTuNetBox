cmake_minimum_required(VERSION 3.1.0)
project(XiaoTuNetBox)
set(PACKAGE_VERSION "0.0.7")

add_compile_options(-std=c++17)
set(CMAKE_BUILD_TYPE RELEASE)

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

file(GLOB PROJECT_SRC_FILES
          ${PROJECT_SOURCE_DIR}/src/*cpp)
file(GLOB PROJECT_INC_FILES
          ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/*.h
          ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/*.hpp)

add_library(${PROJECT_NAME} SHARED
    ${PROJECT_SRC_FILES}
)
target_link_libraries(${PROJECT_NAME})

############################################################################
#
# utils
#
############################################################################

add_executable(u_stdin_ipv4_talk utils/u_stdin_ipv4_talk.cpp)
target_link_libraries(u_stdin_ipv4_talk ${PROJECT_NAME})

add_executable(u_echo_server utils/u_echo_server.cpp)
target_link_libraries(u_echo_server ${PROJECT_NAME})

add_executable(u_http_server utils/u_http_server.cpp)
target_link_libraries(u_http_server ${PROJECT_NAME})

############################################################################
#
# tests 
#
############################################################################

enable_testing()

set(TEST_DEPENDS gtest gtest_main pthread rt)

function(build_test_case NAME EXEC TSRC)
    message(STATUS "build_test_case:"  ${NAME})
    message(STATUS "EXEC = " ${EXEC})
    message(STATUS "TSRC = " ${TSRC})

    add_executable(${EXEC} ${TSRC})
    target_link_libraries(${EXEC} ${TEST_DEPENDS} ${PROJECT_NAME})
    add_test(NAME ${NAME} COMMAND ${EXEC})
endfunction()

build_test_case(DataArray t_DataArray test/t_DataArray.cpp)
build_test_case(DataQueue t_DataQueue test/t_DataQueue.cpp)
build_test_case(Address t_Address test/t_Address.cpp)
build_test_case(Socket t_Socket test/t_Socket.cpp)

############################################################################
#
# documentation 
#
############################################################################

include(cmake/build_doxygen.cmake)
build_doxygen()

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


