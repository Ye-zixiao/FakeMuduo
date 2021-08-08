cmake_minimum_required(VERSION 3.16)
project(docs/oldfiles/libfm LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
    message("Setting build type to 'Debug' as non was specified")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()

set(CXX_FLAGS
        -fno-omit-frame-pointer
        -Wall
        -O3
        -Wextra
        -Werror
#        -Wconversion
        -Wno-unused-parameter
        -Wold-style-cast
        -Woverloaded-virtual
        -Wpointer-arith
        -Wshadow
        -Wwrite-strings
        -std=c++17
        -march=native
        -rdynamic
        )
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/include)

install(DIRECTORY include/libfm DESTINATION include)

add_subdirectory(net)
add_subdirectory(examples)
add_subdirectory(util)