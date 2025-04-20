cmake_minimum_required(VERSION 3.0)
project(MyMuduo)
#MyMuduo最终能编译成so动态库，设置动态库的路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
#设置调试信息
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -fPIC")
#设置muduo的源代码文件
aux_source_directory(. SRC_LIST)
#编译生成动态库mymoduo
add_library(MyMuduo SHARED ${SRC_LIST})