cmake_minimum_required(VERSION 3.27)
project(GuoShuBase)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(GuoShuBase STATIC test.cpp
        pf_manager.cpp
        pf_filehandle.cpp
        pf_error.cpp
        pf_hashtable.cpp
        pf_pagehandle.cpp
        pf_statistics.cpp
        statistics.cpp
        pf_buffermgr.cpp)

set_target_properties(GuoShuBase PROPERTIES LINKER_LANGUAGE CXX)