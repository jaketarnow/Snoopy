cmake_minimum_required(VERSION 3.3)
project(UpnpDiscover)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.c utils.h utils.c)
add_executable(UpnpDiscover ${SOURCE_FILES} utils.h utils.c)