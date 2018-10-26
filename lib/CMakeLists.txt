cmake_minimum_required(VERSION 2.8)
project (LIBMEW)

# C99
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

# C++11
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

# Logging
if( USE_LOGGER )
INCLUDE_DIRECTORIES( ${LIBMEW_SOURCE_DIR}/extdeps/loguru )
add_definitions( -DUSE_LOGGING )
set( MEW_DEPS "${MEW_DEPS};dl;pthread" )
set( LOGURU_CPP "${LIBMEW_SOURCE_DIR}/extdeps/loguru/loguru.cpp" )
endif()

# Add Dynamic Library Loader dependency, pthread, uuid (if UNIX-based)
set( MEW_DEPS "${MEW_DEPS}" )

# Set include directories
include_directories( ${LIBMEW_SOURCE_DIR}/include )

# Global source fetching
file ( GLOB_RECURSE LIBMEW_CPP ./src/*.c* )

# Nodes source fetching
file(GLOB_RECURSE NODES_SRC ${PROJECT_SOURCE_DIR}/nodes/*.c*)
file(GLOB_RECURSE NODES_HEADERS ${PROJECT_SOURCE_DIR}/nodes/*.h)
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/nodes/ )

# Remotery

# SET SOURCES
set( LIBMEW_ALL_CPP ${LIBMEW_CPP} ${NODES_SRC} ${LOGURU_CPP} )

IF( LIBMEW_ALL_CPP )
message( "LIBMEW_ALL_CPP=${LIBMEW_ALL_CPP}" )
if( BUILD_STATIC_LIB )
    add_library( mew STATIC ${LIBMEW_ALL_CPP} )
else()
    add_library( mew SHARED ${LIBMEW_ALL_CPP} )
endif()

# libuv
add_dependencies( mew uv_a )
set( MEW_DEPS "${MEW_DEPS};uv_a" )

# fPIC
# set_property(TARGET mew PROPERTY POSITION_INDEPENDENT_CODE OFF)

# Link libraries
target_link_libraries( mew ${MEW_DEPS} )
INSTALL( TARGETS mew DESTINATION "lib" )
endif()

# Install targets
install( FILES ${LIBMEW_SOURCE_DIR}/include/*.h DESTINATION include/mew )
