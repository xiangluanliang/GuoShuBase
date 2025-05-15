add_library(ix ix.h
        ix_error.cpp
        ix_indexhandle.cpp
        ix_indexscan.cpp
        ix_manager.cpp
        btree_node.cpp
)

target_include_directories(ix PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ix pf rm)
#
#add_executable(ix_test ix_test.cpp)
#target_link_libraries(ix_test ix)