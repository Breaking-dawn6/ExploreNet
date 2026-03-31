cmake_minimum_required(VERSION 3.11)
project(explore)

# 库最终编译成so动态库，设置动态库的路径,放在根目录的lib文件夹下面
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 设置调试信息
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++17")

# 选项：是否构建HTTP模块（默认ON）
option(BUILD_HTTP_MODULE "Build HTTP module" ON)

# 添加头文件搜索路径，使得源码中可以直接 #include "xxx.h" 而不用写长路径
include_directories(${PROJECT_SOURCE_DIR}/base)
include_directories(${PROJECT_SOURCE_DIR}/net/core)

# 递归收集核心模块的 .cpp 源文件
file(GLOB_RECURSE SRC_LIST 
    "${PROJECT_SOURCE_DIR}/base/*.cpp"
    "${PROJECT_SOURCE_DIR}/net/core/*.cpp"
)

# 根据选项决定是否包含HTTP模块
if(BUILD_HTTP_MODULE)
    # 优先查找本地llhttp
    find_path(LLHTTP_INCLUDE_DIR llhttp.h
        PATHS /usr/local/include
        PATH_SUFFIXES llhttp
    )
    
    find_library(LLHTTP_LIBRARY 
        NAMES llhttp libllhttp
        PATHS /usr/local/lib
    )
    
    if(LLHTTP_INCLUDE_DIR AND LLHTTP_LIBRARY)
        message(STATUS "Found local llhttp: ${LLHTTP_LIBRARY}")
        include_directories(${LLHTTP_INCLUDE_DIR})
        set(LLHTTP_FOUND TRUE)
    else()
        message(STATUS "llhttp not found locally, fetching from GitHub...")
        include(FetchContent)
        
        FetchContent_Declare(
            llhttp
            GIT_REPOSITORY https://github.com/nodejs/llhttp.git
            GIT_TAG release/v9.2.1
        )
        
        FetchContent_MakeAvailable(llhttp)
        set(LLHTTP_FOUND TRUE)
    endif()
    
    include_directories(${PROJECT_SOURCE_DIR}/net/http)
    file(GLOB_RECURSE HTTP_SRC_LIST "${PROJECT_SOURCE_DIR}/net/http/*.cpp")
    list(APPEND SRC_LIST ${HTTP_SRC_LIST})
endif()

# 编译生成 libexplore.so 动态库
add_library(explore SHARED ${SRC_LIST})

# 根据选项链接HTTP依赖库
if(BUILD_HTTP_MODULE)
    if(LLHTTP_INCLUDE_DIR AND LLHTTP_LIBRARY)
        target_link_libraries(explore ${LLHTTP_LIBRARY})
    else()
        target_link_libraries(explore llhttp)
    endif()
endif()