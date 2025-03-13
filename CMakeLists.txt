cmake_minimum_required(VERSION 3.27)
project(GuoShuBase)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(GuoShuBase STATIC test.cpp)

set_target_properties(GuoShuBase PROPERTIES LINKER_LANGUAGE CXX)