cmake_minimum_required(VERSION 2.8)
project(MEW_CLIENT)

#
#
## CHECK FOR COMPILER CAPABILITIES
#
#
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
  message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()
##

SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fpermissive" )

include_directories(${LIBMEW_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR})

# websocketpp
INCLUDE_DIRECTORIES( ${MEWD_SOURCE_DIR}/extdeps/websocketpp/ )

# asio
INCLUDE_DIRECTORIES( ${MEWD_SOURCE_DIR}/extdeps/asio/include )

# JSON11
file( GLOB JSON11_SRC ${MEWD_SOURCE_DIR}/extdeps/json11/json11.cpp )
INCLUDE_DIRECTORIES( ${MEWD_SOURCE_DIR}/extdeps/json11/ )

add_definitions( -DASIO_HEADER_ONLY )

# Global shit
file ( GLOB_RECURSE LIBMEW_APP_SRC ./src/*.c* )
file ( GLOB_RECURSE LIBMEW_APP_HEADERS ./src/*.h )

add_executable( mew_client ${LIBMEW_APP_SRC} ${LIBMEW_APP_HEADERS} ${JSON11_SRC} )
set( DEPS "${DEPS};rt;pthread" )

target_link_libraries ( mew_client ${DEPS} )
