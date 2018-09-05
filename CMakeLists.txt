cmake_minimum_required(VERSION 2.8)

project(MEW)

option(BUILD_EXAMPLE_APPS "Build Example Applications" ON)
option(BUILD_TESTS "Build Tests" OFF)

add_subdirectory(lib)

if( BUILD_EXAMPLE_APPS )
    add_subdirectory( examples )
endif( BUILD_EXAMPLE_APPS )

if( BUILD_TESTS )
    add_subdirectory( tests )
endif( BUILD_TESTS )
