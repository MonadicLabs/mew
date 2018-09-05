cmake_minimum_required(VERSION 2.8)
project(LIBMEW_EXAMPLE)

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

SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fpermissive " )

include_directories(${LIBMEW_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR})

# Global shit
file (GLOB_RECURSE LIBMEW_APP_SRC ./*.c* )
file (GLOB_RECURSE LIBMEW_APP_HEADERS ./*.h )
add_executable( mew_example ${LIBMEW_APP_SRC} ${LIBMEW_APP_HEADERS} )
set( DEPS "${DEPS};rt;pthread" )

target_link_libraries ( mew_example ${DEPS} )
