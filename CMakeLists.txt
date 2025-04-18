cmake_minimum_required(VERSION 3.27)
#project(GuoShuBase)
set(CMAKE_CXX_STANDARD 17)
add_definitions(-DPC)
add_definitions(-DPF_LOG)
#add_executable(GuoShuBase test.cpp
#        pf_pagehandle.cpp
#        pf_filehandle.cpp
#        pf_manager.cpp
#        pf_buffermgr.cpp
#        pf_hashtable.cpp
#        pf_error.cpp
#        statistics.cpp
#        pf_statistics.cpp)

project(testpf_buf)
add_executable(testpf_buf testpf_buf.cpp pf_buffermgr.cpp pf_hashtable.cpp)

project(test_parse)
add_executable(test_parse test_parse.cpp
        Table.cpp
        DataBase.cpp
        DBMSParser.cpp
        #----PF组件----
        test.cpp
        pf_pagehandle.cpp
        pf_filehandle.cpp
        pf_manager.cpp
        pf_buffermgr.cpp
        pf_hashtable.cpp
        pf_error.cpp
        statistics.cpp
        pf_statistics.cpp)

