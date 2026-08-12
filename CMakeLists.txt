cmake_minimum_required(VERSION 3.31)

set(DEFAULT_GCC_VERSION 16)

## Default to release builds, must be set before project
if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

# Project Definition
set(UNFOLD_NAME "unfoldTACPN")

# Select GCC compiler
set(DESIRED_GCC_VERSION "$ENV{GCC_VERSION}")
if (DESIRED_GCC_VERSION STREQUAL "")
    set(DESIRED_GCC_VERSION ${DEFAULT_GCC_VERSION})
endif()

if (NOT CMAKE_C_COMPILER)
    find_program(GCC_REQ_C NAMES "gcc-${DESIRED_GCC_VERSION}" gcc REQUIRED)
    set(CMAKE_C_COMPILER "${GCC_REQ_C}" CACHE STRING "C compiler")
endif()

if (NOT CMAKE_CXX_COMPILER)
    find_program(GCC_REQ_CXX NAMES "g++-${DESIRED_GCC_VERSION}" g++ REQUIRED)
    set(CMAKE_CXX_COMPILER "${GCC_REQ_CXX}" CACHE STRING "CXX compiler")
endif()

project("unfoldTACPN" VERSION 0.0.3 LANGUAGES CXX C)

if (NOT CMAKE_C_COMPILER_ID STREQUAL "GNU" OR NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(FATAL_ERROR "GCC is required, but found ${CMAKE_C_COMPILER_ID} and ${CMAKE_CXX_COMPILER_ID}")
endif()

if (NOT CMAKE_CROSSCOMPILING)
    if (NOT CMAKE_C_COMPILER_VERSION MATCHES "^${DESIRED_GCC_VERSION}([.].*)?$")
        message(FATAL_ERROR "GCC ${DESIRED_GCC_VERSION} is required, but found GCC ${CMAKE_C_COMPILER_VERSION}")
    endif()
    if (NOT CMAKE_CXX_COMPILER_VERSION MATCHES "^${DESIRED_GCC_VERSION}([.].*)?$")
        message(FATAL_ERROR "G++ ${DESIRED_GCC_VERSION} is required, but found G++ ${CMAKE_CXX_COMPILER_VERSION}")
    endif()
endif()

# C++ Definition
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Default Values
## Build Flags
option(UNFOLDTACPN_Static "Link libraries statically" ON)
option(UNFOLDTACPN_GetDependencies "Fetch external dependencies from web." ON)
set(EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external CACHE PATH "Install location for external dependencies")
option(UNFOLDTACPN_TEST "Build unit tests" OFF)
set(UNFOLDTACPN_TARGETDIR "${CMAKE_BINARY_DIR}/${UNFOLD_NAME}" CACHE PATH "Target directory for build files")
set(UNFOLDTACPN_OSX_DEPLOYMENT_TARGET 10.8 CACHE STRING "Specify the minimum version of the target platform for MacOS on which the target binaries are to be deployed ")

# Configure Project

## Configure Static
if (UNFOLDTACPN_Static)
    set(BUILD_SHARED_LIBS OFF)
else()
    set(BUILD_SHARED_LIBS ON)
endif ()


## Enabled INTERPROCEDURAL_OPTIMIZATION for non-macos
if (CMAKE_BUILD_TYPE MATCHES Release)
    if (NOT APPLE AND NOT WIN32)
    	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif ()
endif ()

## CXX Flags
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s -Wall -pedantic-errors -O3 -DNDEBUG")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -fno-omit-frame-pointer ")

## Target File location
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${UNFOLDTACPN_TARGETDIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${UNFOLDTACPN_TARGETDIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${UNFOLDTACPN_TARGETDIR}/bin)

# Required Packages and Tools

find_package(FLEX 2.6.4 REQUIRED)
find_package(BISON 3.0.5 REQUIRED)

if (UNFOLDTACPN_GetDependencies)
    include(ExternalProject)
    # Install location should be ${EXTERNAL_INSTALL_LOCATION}

    ExternalProject_add(rapidxml-ext
        URL https://downloads.sourceforge.net/project/rapidxml/rapidxml/rapidxml%201.13/rapidxml-1.13.zip
        URL_HASH SHA512=6c10583e6631ccdb0217d0a5381172cb4c1046226de6ef1acf398d85e81d145228e14c3016aefcd7b70a1db8631505b048d8b4f5d4b0dbf1811d2482eefdd265
        BUILD_COMMAND ""
        CONFIGURE_COMMAND mkdir -p ${CMAKE_BINARY_DIR}/external/include
        INSTALL_COMMAND cd ../rapidxml-ext && ${CMAKE_COMMAND} -E copy rapidxml.hpp rapidxml_iterators.hpp rapidxml_print.hpp rapidxml_utils.hpp ${EXTERNAL_INSTALL_LOCATION}/include
    )

    set(CMAKE_PREFIX_PATH ${EXTERNAL_INSTALL_LOCATION};${CMAKE_PREFIX_PATH})
    link_directories(${EXTERNAL_INSTALL_LOCATION}/lib)
    link_directories(${EXTERNAL_INSTALL_LOCATION}/lib64)
    include_directories(${EXTERNAL_INSTALL_LOCATION}/include)
endif ()

# Source
include_directories(include)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/)

# Testing
if (BUILD_TESTING AND UNFOLDTACPN_TEST)
    enable_testing()
    add_subdirectory(${CMAKE_SOURCE_DIR}/test)
endif()
