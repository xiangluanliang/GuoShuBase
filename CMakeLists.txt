cmake_minimum_required(VERSION 3.27)
project(GuoShuBase)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(GuoShuBase STATIC
        guoshubase.h
        pf.h
        pf_hashtable.h
        pf_internal.h
        pf_buffermgr.h
        linkedlist.h
        rm_rid.h
        statistics.h
        pf_manager.cpp
        pf_filehandle.cpp
        pf_error.cpp
        pf_hashtable.cpp
        pf_pagehandle.cpp
        pf_statistics.cpp
        statistics.cpp
        pf_buffermgr.cpp
        rm.h
        test.cpp
        rm_manager.cpp
        rm_filehandle.cpp
        rm_filescan.cpp
        rm_scanhandle.cpp
        rm_scanhandle.h
)

set_target_properties(GuoShuBase PROPERTIES LINKER_LANGUAGE CXX)