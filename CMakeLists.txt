cmake_minimum_required(VERSION 3.27)
project(GuoShuBase)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(pf)
add_subdirectory(rm)
#add_subdirectory(ix)
#add_subdirectory(sm)
#add_subdirectory(ql)

#add_executable(GuoShuBase main.cpp)
#
#target_link_libraries(GuoShuBase pf rm ix sm ql)
#
#target_include_directories(GuoShuBase PRIVATE
#        ${CMAKE_SOURCE_DIR}/pf
#        ${CMAKE_SOURCE_DIR}/rm
#        ${CMAKE_SOURCE_DIR}/ix
#        ${CMAKE_SOURCE_DIR}/sm
#        ${CMAKE_SOURCE_DIR}/ql
#)