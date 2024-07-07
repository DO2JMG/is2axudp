cmake_minimum_required(VERSION 3.5)

project (is2axudp)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

include_directories(include)

set(SOURCES
    src/main.cpp
    src/aprsstr.cpp
    include/aprsstr.h
)

add_executable(is2axudp ${SOURCES})
