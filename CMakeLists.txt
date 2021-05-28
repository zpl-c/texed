cmake_minimum_required(VERSION 3.0)
project(texed)
include(cmake/utils.cmake)

set(CMAKE_C_STANDARD 11)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_compile_options(/MP /W4)
endif()

include_directories(code/vendors)

include(cmake/FindRaylib.cmake)

add_subdirectory(code/vendors)
add_subdirectory(code/app)
