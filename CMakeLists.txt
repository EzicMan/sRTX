cmake_minimum_required(VERSION 3.1)

project(sRTX)

set(PROG_SRC Source.cpp)
set(PROG_HDR bmplib.hpp kdTree.hpp Vector3.hpp Object.hpp json.hpp)

configure_file(config.json config.json COPYONLY)
configure_file(cube.obj cube.obj COPYONLY)

add_executable(${PROJECT_NAME} ${PROG_SRC} ${PROG_HDR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
