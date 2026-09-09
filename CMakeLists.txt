cmake_minimum_required(VERSION 3.10)
project(F32C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(f32c 
                main.cpp
                src/uart.cpp
                src/F32C.cpp
                )

