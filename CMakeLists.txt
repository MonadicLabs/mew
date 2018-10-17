cmake_minimum_required(VERSION 2.8)

project(MEW)

option(BUILD_EXAMPLE_APPS "Build Example Applications" ON)
option(BUILD_TESTS "Build Tests" OFF)
option(BUILD_DAEMON "Build mew dameon" ON )
option(BUILD_STATIC_LIB "Build static library" OFF)
option(USE_LOGGER "Use logging capabilities" OFF)
option(BUILD_GUI "Build mewd GUI" ON)

# libuv
add_subdirectory( ${MEW_SOURCE_DIR}/extdeps/libuv )

#
add_subdirectory(lib)

if( BUILD_EXAMPLE_APPS )
    add_subdirectory( examples )
endif( BUILD_EXAMPLE_APPS )

if( BUILD_TESTS )
    add_subdirectory( tests )
endif( BUILD_TESTS )

if( BUILD_DAEMON )
    add_subdirectory( server )
endif()

if( BUILD_GUI )
	add_subdirectory( client )
endif()
