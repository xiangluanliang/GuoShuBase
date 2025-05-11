cmake_minimum_required(VERSION 3.27)
project(GuoShuBase)
set(CMAKE_CXX_STANDARD 17)

add_definitions(-DPC -DPF_LOG -DPF_STATS -D_WIN32)

add_subdirectory(pf)
add_subdirectory(rm)
add_subdirectory(ix)
add_subdirectory(sm)
add_subdirectory(ql)
target_include_directories(pf PUBLIC ${CMAKE_SOURCE_DIR}/utils)
add_executable(GuoShuBase main.cpp)
target_link_libraries(GuoShuBase
        PRIVATE
        pf rm ix sm ql
)

target_include_directories(GuoShuBase
        PRIVATE
        ${CMAKE_SOURCE_DIR}/pf
        ${CMAKE_SOURCE_DIR}/rm
        ${CMAKE_SOURCE_DIR}/ix
        ${CMAKE_SOURCE_DIR}/sm
        ${CMAKE_SOURCE_DIR}/ql
)
