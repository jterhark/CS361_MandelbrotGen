cmake_minimum_required(VERSION 3.9)
project(mandelbrot)

set(CMAKE_CXX_STANDARD 11)

add_executable(mandelbrot mandelbrot.cpp)
add_executable(calc mandelbrot-calc.cpp)
add_executable(display mandelbrot-display.cpp)