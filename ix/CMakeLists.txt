add_library(ix
        ix_error.cpp
)

target_include_directories(ix PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ix pf rm)  # 如果 ix 依赖 pf 和 rm
