
project (MEWD)

# Set custom cmake scripts directory
SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -pthread" )
SET( CMAKE_EXE_LINKER_FLAGS "-pthread")

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

# CMake modules
set( CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${NOVALINKD_SOURCE_DIR}/cmake")

include_directories( ${MEW_SOURCE_DIR}/lib/include )
INCLUDE_DIRECTORIES( ./include )

# JSON11
# file( GLOB JSON11_SRC ${NOVALINK_SOURCE_DIR}/extdeps/json11/json11.cpp )
# INCLUDE_DIRECTORIES( ${NOVALINK_SOURCE_DIR}/extdeps/json11/ )

# websocketpp
INCLUDE_DIRECTORIES( ${MEWD_SOURCE_DIR}/extdeps/websocketpp/ )

# asio
INCLUDE_DIRECTORIES( ${MEWD_SOURCE_DIR}/extdeps/asio/include )

# JSON11
file( GLOB JSON11_SRC ${MEWD_SOURCE_DIR}/extdeps/json11/json11.cpp )
INCLUDE_DIRECTORIES( ${MEWD_SOURCE_DIR}/extdeps/json11/ )

# Global source fetching
file ( GLOB_RECURSE MEWD_CORE_CPP ./src/*.c* )
file ( GLOB_RECURSE MEWD_CORE_HEADERS ./include/*.h* )
set( MEWD_CPP ${MEWD_CORE_CPP} ${MEWD_CORE_HEADERS} ${JSON11_SRC} )

set( DEPS "${DEPS}" )

add_executable( mewd ${MEWD_CPP} )

target_link_libraries( mewd ${DEPS} )

INSTALL(TARGETS mewd DESTINATION "bin" )
