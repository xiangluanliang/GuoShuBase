add_library(rm rm.h
        rm_error.cpp
        rm_rid.h
        ../utils/predicate.cpp
        rm_record.cpp
        rm_manager.cpp
        rm_filescan.cpp
        rm_filehandle.cpp
        bitmap.cpp
)

target_include_directories(rm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(rm pf ix)

add_executable(rm_test rm_test.cpp)
target_link_libraries(rm_test rm)