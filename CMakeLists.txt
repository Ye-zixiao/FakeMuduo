cmake_minimum_required(VERSION 3.16)
project(FakeMuduo LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
    message("Setting build type to 'Debug' as non was specified")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3")

add_subdirectory(src/base)
add_subdirectory(src/net)
add_subdirectory(src/net/http)
add_subdirectory(example)