# CMake 最低版本号要求
cmake_minimum_required (VERSION 2.8)
# 项目信息
project (Demo3)
# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
aux_source_directory(. DIR_SRCS)
# 添加 math 子目录
add_subdirectory(src)
# 指定生成目标 
add_executable(mainPc mainPc.cpp)
add_executable(otherPc otherPc.cpp)
# 添加链接库
target_link_libraries(mainPc srcs)
target_link_libraries(otherPc srcs)

#设置输出目录
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

#设置编译器及编译选项,cmake多线程应加上-pthread
SET(CMAKE_C_COMPILER g++)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread ")