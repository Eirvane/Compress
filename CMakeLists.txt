cmake_minimum_required(VERSION 3.14)
project(Compress VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

add_executable(Compress
    src/main.cpp
    src/core/bitstream.cpp
)