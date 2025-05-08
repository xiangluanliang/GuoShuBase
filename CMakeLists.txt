cmake_minimum_required(VERSION 3.27)
project(GuoShuBase)
set(CMAKE_CXX_STANDARD 17)

set(GENERATED_DIR "${CMAKE_BINARY_DIR}/generated")
set(FLEX_ROOT "C:/ProgramData/chocolatey/lib/winflexbison")
set(BISON_ROOT "C:/ProgramData/chocolatey/lib/winflexbison")
include_directories("C:/ProgramData/chocolatey/lib/winflexbison/tools")
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

# 2. 生成解析器代码（在根目录处理）
BISON_TARGET(
        Parser
        "${CMAKE_SOURCE_DIR}/sm/parser.y"
        "${GENERATED_DIR}/parser.cpp"
        DEFINES_FILE "${GENERATED_DIR}/y.tab.hpp"  # 生成头文件
        COMPILE_FLAGS "--language=c++"
)
FLEX_TARGET(
        Lexer
        "${CMAKE_SOURCE_DIR}/sm/scan.l"
        "${GENERATED_DIR}/scan.cpp" COMPILE_FLAGS "--c++"
)

# 3. 添加子模块（确保子模块能访问生成的代码）
add_subdirectory(pf)
add_subdirectory(rm)
add_subdirectory(ix)
add_subdirectory(sm)
add_subdirectory(ql)

# 4. 主可执行文件
add_executable(GuoShuBase
        main.cpp
        ${BISON_Parser_OUTPUTS}  # 自动包含 parser.tab.c
        ${FLEX_Lexer_OUTPUTS}    # 自动包含 lex.yy.c
)

# 5. 链接库和头文件
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
        "${GENERATED_DIR}"
)
