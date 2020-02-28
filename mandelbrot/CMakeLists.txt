cmake_minimum_required(VERSION 3.5)
link_libraries(mango-framebuffer mango X11)
add_executable(mandelbrot mandelbrot.cpp)
