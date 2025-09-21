cmake_minimum_required(VERSION 3.10.0)
project(opencv-project VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_executable(opencv-project main.cpp)

find_package(OpenCV REQUIRED)
include_directories(
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(opencv-project
    ${OpenCV_LIBS}
)