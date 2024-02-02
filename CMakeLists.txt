#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.12)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.27")
    cmake_policy(SET CMP0144 NEW)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmake/HunterGate.cmake)

HunterGate(
    URL  https://github.com/qdrvm/hunter/archive/refs/tags/v0.25.3-qdrvm2.zip
    SHA1 8599cf0bcf24db83ab0d14e4cf5c3dcc744bd20c
)

project(Scale LANGUAGES CXX VERSION 1.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_TESTS "Whether to include the test suite in build" OFF)

hunter_add_package(Boost)
find_package(Boost CONFIG REQUIRED)

add_subdirectory(src)

if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(test ${CMAKE_BINARY_DIR}/test_bin)
endif ()

###############################################################################
#   INSTALLATION
###############################################################################

include(GNUInstallDirs)

install(TARGETS scale EXPORT scaleConfig
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_PREFIX}
    )

install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/include/scale
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/scaleConfigVersion.cmake
    COMPATIBILITY SameMajorVersion
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/scaleConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/scale
)

install(
    EXPORT scaleConfig
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/scale
    NAMESPACE scale::
)
