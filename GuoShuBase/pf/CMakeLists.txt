add_library(pf
        pf_pagehandle.cpp
        pf_filehandle.cpp
        pf_manager.cpp
        pf_buffermgr.cpp
        pf_hashtable.cpp
        pf_error.cpp
        ../utils/statistics.cpp
        pf_statistics.cpp
        ../utils/fileio.cpp
        ../utils/fileio.h
)
#target_include_directories(pf PUBLIC ${CMAKE_SOURCE_DIR}/utils) #暂时先不单独编译
target_include_directories(pf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
#
#add_executable(pf_test1 pf_test1.cpp)
#target_link_libraries(pf_test1 pf)
#
#add_executable(pf_test2 pf_test2.cpp)
#target_link_libraries(pf_test2 pf)
#
#add_executable(pf_test3 pf_test3.cpp)
#target_link_libraries(pf_test3 pf)
