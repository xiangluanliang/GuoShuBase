add_library(ql
        ql_error.cpp
)

target_include_directories(ql PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ql pf rm ix sm)
