#cmake_minimum_required(VERSION 3.27)
#project(GuoShuBase)
#set(CMAKE_CXX_STANDARD 17)
#
add_definitions(-DPC -DPF_LOG -DPF_STATS -D_WIN32 -D_GUI)

add_subdirectory(pf)
add_subdirectory(rm)
add_subdirectory(ix)
add_subdirectory(sm)
add_subdirectory(ql)


set(GUOSHUBASE_INTERFACE_SRC
        guoshubase_interface.cpp
        guoshubase_interface.h
        guoshubase.h
)

add_library(guoshubase STATIC ${GUOSHUBASE_INTERFACE_SRC})

target_include_directories(guoshubase PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/pf
        ${CMAKE_CURRENT_SOURCE_DIR}/rm
        ${CMAKE_CURRENT_SOURCE_DIR}/ix
        ${CMAKE_CURRENT_SOURCE_DIR}/sm
        ${CMAKE_CURRENT_SOURCE_DIR}/ql
)

target_link_libraries(guoshubase
        pf rm ix sm ql
)
