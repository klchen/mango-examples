cmake_minimum_required(VERSION 3.5)
add_executable(benchmark benchmark.cpp)
find_package(OpenCV REQUIRED)
target_link_libraries(benchmark LINK_PUBLIC mango jpeg ${OpenCV_LIBS})
