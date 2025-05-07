add_library(rm
        rm_error.cpp
)

target_include_directories(rm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(rm pf)  # 如果 rm 依赖 pf，可以这样链接
