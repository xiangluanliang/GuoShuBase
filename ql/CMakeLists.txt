add_library(ql
        ql_error.cpp
        ql_manager.cpp
        file_scan.cpp
        index_scan.cpp
        merge_join.cpp
        nested_loop_join.cpp
        projection.cpp
        tokenizer.cpp
        tokenizer.h
        sqlparser.cpp
        sqlparser.h
        parser.cpp
)

target_include_directories(ql PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ql pf rm ix sm)
#
#add_executable(ql_test ql_test.cpp)
#target_link_libraries(ql_test ql)