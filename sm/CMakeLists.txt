add_library(sm
        sm_error.cpp
        sm_manager.cpp
        printer.cpp
)

target_include_directories(sm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(sm pf rm ix)

add_executable(sm_test1 dbcreate.cc)
target_link_libraries(sm_test1 sm)

add_executable(sm_test2 dbdestroy.cc)
target_link_libraries(sm_test2 sm)
