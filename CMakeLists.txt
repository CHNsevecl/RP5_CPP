cmake_minimum_required(VERSION 3.10)
project(POSITION_INDEPENDENT_CODE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include)
file(GLOB SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(pipe ${SRC_FILES})

target_link_libraries(pipe ${OpenCV_LIBS} zmq)
