cmake_minimum_required(VERSION 3.5)
add_executable(particle particle.cpp)
target_compile_options(particle PUBLIC "-msse4")
target_compile_options(particle PUBLIC "-O3")
target_link_libraries(particle LINK_PUBLIC mango dl)
