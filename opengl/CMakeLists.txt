cmake_minimum_required(VERSION 3.5)
link_libraries(mango-opengl mango X11 GL)
add_executable(gltest gltest.cpp)
