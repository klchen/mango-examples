cmake_minimum_required(VERSION 3.5)
project(examples)

set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_CXX_FLAGS_RELEASE "-O3")

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm") 
    add_definitions (-mfpu=neon -mfloat-abi=hard)
    set(CMAKE_CXX_FLAGS "-fpermissive -Wno-psabi")
endif ()

add_subdirectory(misc)
add_subdirectory(jpeg_benchmark)
add_subdirectory(jpeg_multithread)
add_subdirectory(particle_benchmark)
# add_subdirectory(opengl)
# add_subdirectory(framebuffer)
# add_subdirectory(vulkan)
