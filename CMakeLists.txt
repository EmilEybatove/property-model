cmake_minimum_required(VERSION 3.10.0)
project(Property-Model VERSION 0.1.0 LANGUAGES C CXX)

include_directories(library utils)
add_subdirectory(library)
add_subdirectory(utils)

add_executable(Property-Model property_model.cpp)

target_link_libraries(Property-Model library utils)
