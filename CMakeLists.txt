cmake_minimum_required(VERSION 3.16)
project(Libfm LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
    message("Setting build type to 'Debug' as non was specified")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O")

#set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
#set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(libfm/base)
add_subdirectory(libfm/net)
add_subdirectory(libfm/net/http)
add_subdirectory(example)
add_subdirectory(unittest)