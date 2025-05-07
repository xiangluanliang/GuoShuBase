add_library(sm
        sm_error.cpp
)

target_include_directories(sm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(sm pf rm ix)
