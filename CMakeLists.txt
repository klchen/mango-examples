cmake_minimum_required(VERSION 3.5)
project(examples)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(misc)
add_subdirectory(jpeg_benchmark)
add_subdirectory(jpeg_multithread)
add_subdirectory(particle_benchmark)
# add_subdirectory(opengl)
# add_subdirectory(vulkan)
