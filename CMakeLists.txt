cmake_minimum_required(VERSION 3.27)
set(CMAKE_CXX_STANDARD 17)
project(pf_test3)
add_definitions(-DPC)
add_definitions(-DPF_LOG)
add_definitions(-DPF_STATS)
add_executable(pf_test3 pf_test3.cpp
        pf_pagehandle.cpp
        pf_filehandle.cpp
        pf_manager.cpp
        pf_buffermgr.cpp
        pf_hashtable.cpp
        pf_error.cpp
        statistics.cpp
        pf_statistics.cpp)

project(pf_test1)
add_definitions(-DPC)
add_definitions(-DPF_LOG)
add_definitions(-DPF_STATS)
add_executable(pf_test1 pf_test1.cpp
        pf_pagehandle.cpp
        pf_filehandle.cpp
        pf_manager.cpp
        pf_buffermgr.cpp
        pf_hashtable.cpp
        pf_error.cpp
        statistics.cpp
        pf_statistics.cpp)

project(pf_test2)
add_definitions(-DPC)
add_definitions(-DPF_LOG)
add_definitions(-DPF_STATS)
add_executable(pf_test2 pf_test2.cpp
        pf_pagehandle.cpp
        pf_filehandle.cpp
        pf_manager.cpp
        pf_buffermgr.cpp
        pf_hashtable.cpp
        pf_error.cpp
        statistics.cpp
        pf_statistics.cpp)

#project(test_parse)
#add_executable(test_parse test_parse.cpp
#        Table.cpp
#        DataBase.cpp
#        DBMSParser.cpp
#        #----PF组件----
#        test.cpp
#        pf_pagehandle.cpp
#        pf_filehandle.cpp
#        pf_manager.cpp
#        pf_buffermgr.cpp
#        pf_hashtable.cpp
#        pf_error.cpp
#        statistics.cpp
#        pf_statistics.cpp)

