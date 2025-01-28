cmake_minimum_required(VERSION 3.12)

include(../../cmake/Hunter/HunterGate.cmake)

HunterGate(
    URL  https://github.com/qdrvm/hunter/archive/refs/tags/v0.23.257-qdrvm10.tar.gz
    SHA1 72b446a4424ba28ea90f9a68a9134b0f8e44b5b2
)

project(ScaleTest CXX)

set(CMAKE_CXX_STANDARD 20)

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_LIST_DIR}/scale-install/")
find_package(scale REQUIRED)

if (PACKAGE_MANAGER STREQUAL "hunter")
    hunter_add_package(Boost COMPONENTS random)
endif ()
find_package(Boost CONFIG REQUIRED COMPONENTS random)

add_executable(scale_test scale_test.cpp)
target_link_libraries(scale_test
    scale::scale
    )
add_test(NAME scale_test COMMAND $<TARGET_FILE:scale_test>)
set_target_properties(scale_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test_bin
    ARCHIVE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/test_lib
    LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/test_lib
    )
