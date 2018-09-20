cmake_minimum_required(VERSION 2.8)
project(LIBMEW_EXAMPLE)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fpermissive " )

include_directories(${LIBMEW_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR})

# Global shit
file (GLOB_RECURSE LIBMEW_APP_SRC ./*.c* )
file (GLOB_RECURSE LIBMEW_APP_HEADERS ./*.h )
add_executable( mew_example ${LIBMEW_APP_SRC} ${LIBMEW_APP_HEADERS} )
set( DEPS "${DEPS};rt;pthread" )

target_link_libraries ( mew_example ${DEPS} )
