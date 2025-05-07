cmake_minimum_required(VERSION 3.10)
project(pf_tests)
set(CMAKE_CXX_STANDARD 17)

add_definitions(-DPC -DPF_LOG -DPF_STATS)

add_library(pf
        pf_pagehandle.cpp
        pf_filehandle.cpp
        pf_manager.cpp
        pf_buffermgr.cpp
        pf_hashtable.cpp
        pf_error.cpp
        statistics.cpp
        pf_statistics.cpp
)

target_include_directories(pf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(pf_test1 pf_test1.cpp)
target_link_libraries(pf_test1 pf)

add_executable(pf_test2 pf_test2.cpp)
target_link_libraries(pf_test2 pf)

add_executable(pf_test3 pf_test3.cpp)
target_link_libraries(pf_test3 pf)
