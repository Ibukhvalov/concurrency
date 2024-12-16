cmake_minimum_required(VERSION 3.25)
project(PPM_image)

set(CMAKE_CXX_STANDARD 17)

add_executable(PPM_image main.cpp
        concurrency/RenderImageTask.h)
