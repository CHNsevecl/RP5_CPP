cmake_minimum_required(VERSION 3.10)
project(QD4310)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(qd 
                main.cpp
                src/uart.cpp
                src/QD4310.cpp
                )

