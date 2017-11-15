cmake_minimum_required(VERSION 3.8.2)
project(mandelbrot)

set(CMAKE_CXX_STANDARD 11)

add_executable(mandelbrot mandelbrot.cpp structs.h)
add_executable(mandelCalc mandelbrot-calc.cpp structs.h)
add_executable(mandelDisplay mandelbrot-display.cpp structs.h)