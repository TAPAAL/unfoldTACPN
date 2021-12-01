cmake_minimum_required(VERSION 3.16)

## Default to release builds, must be set before project
if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

# Project Definition
set(VERIFYPN_NAME "unfoldTACPN")
set(VERIFYPN_VERSION 0.0.2)
project(${VERIFYPN_NAME} VERSION ${VERIFYPN_VERSION} LANGUAGES CXX C)

# C++ Definition
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Default Values
## Build Flags
option(UNFOLDTACPN_Static "Link libraries statically" ON)
option(UNFOLDTACPN_GetDependencies "Fetch external dependencies from web." ON)
set(EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external CACHE PATH "Install location for external dependencies")
option(UNFOLDTACPN_TEST "Build unit tests" OFF)
set(UNFOLDTACPN_TARGETDIR "${CMAKE_BINARY_DIR}/${VERIFYPN_NAME}" CACHE PATH "Traget directory for build files")
set(UNFOLDTACPN_OSX_DEPLOYMENT_TARGET 10.8 CACHE STRING "Specify the minimum version of the target platform for MacOS on which the target binaries are to be deployed ") 

# Configure Project

## Configure Static
if (UNFOLDTACPN_Static)
    set(BUILD_SHARED_LIBS OFF)
    if (NOT APPLE)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
    else () 
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
    endif()
endif ()

## Configure Traget Platform 
if (UNIX AND NOT APPLE)
    set(ARCH_TYPE "linux64")
elseif (APPLE)
    set(ARCH_TYPE "osx64")
    set(CMAKE_OSX_DEPLOYMENT_TARGET ${VERIFYPN_OSX_DEPLOYMENT_TARGET})
else ()
    set(ARCH_TYPE "win64")
endif ()

## Enabled INTERPROCEDURAL_OPTIMIZATION for non-macos
if (CMAKE_BUILD_TYPE MATCHES Release)
    if (NOT APPLE)
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

    link_directories(${EXTERNAL_INSTALL_LOCATION}/lib)
    link_directories(${EXTERNAL_INSTALL_LOCATION}/lib64)
    include_directories(${EXTERNAL_INSTALL_LOCATION}/include)
endif ()

# Set Macros
add_compile_definitions(UNFOLDTACPN_VERSION=\"${UNFOLDTACPN_VERSION}\")

# Source
include_directories(include)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/)

# Testing
if (UNFOLDTACPN_TEST)
    enable_testing()
    add_subdirectory(${CMAKE_SOURCE_DIR}/test)
endif()

