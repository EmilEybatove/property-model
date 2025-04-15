cmake_minimum_required(VERSION 3.10.0)
project(Property-Model VERSION 0.1.0 LANGUAGES C CXX)


add_executable(Property-Model kernel/delta_blue.cpp test.cpp)
target_compile_features(Property-Model PUBLIC cxx_std_20)

