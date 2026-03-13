cmake_minimum_required(VERSION 3.5)
project(explore)

# 库最终编译成so动态库，设置动态库的路径,放在根目录的lib文件夹下面
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 设置调试信息
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++17")

# 添加头文件搜索路径，使得源码中可以直接 #include "xxx.h" 而不用写长路径
include_directories(${PROJECT_SOURCE_DIR}/base)
include_directories(${PROJECT_SOURCE_DIR}/net/core)
include_directories(${PROJECT_SOURCE_DIR}/net/http)
include_directories(${PROJECT_SOURCE_DIR}/net/http/ThirdLib/llhttp/include)

# 递归收集所有子目录下的 .cpp 源文件
file(GLOB_RECURSE SRC_LIST 
    "${PROJECT_SOURCE_DIR}/base/*.cpp"
    "${PROJECT_SOURCE_DIR}/net/core/*.cpp"
    "${PROJECT_SOURCE_DIR}/net/http/*.cpp"
)

# 编译生成 libexplore.so 动态库
add_library(explore SHARED ${SRC_LIST})
target_link_libraries(explore ${PROJECT_SOURCE_DIR}/net/http/ThirdLib/llhttp/lib/libllhttp.a)