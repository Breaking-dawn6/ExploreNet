#!/bin/bash

set -e

# 构建选项：是否包含HTTP模块（默认包含）
BUILD_HTTP=${1:-"ON"}

# 若没有build目录，创建该目录
if [ ! -d $(pwd)/build ]; then
    mkdir $(pwd)/build
fi

# 清理旧的编译缓存
rm -rf $(pwd)/build/*

# 进入 build 目录，执行 cmake 生成 Makefile，并调用 make 编译
cd $(pwd)/build &&
    cmake .. -DBUILD_HTTP_MODULE=${BUILD_HTTP} &&
    make

# 回到项目根目录
cd ..

# 把头文件拷贝到 /usr/include/explore，so库拷贝到 /usr/lib 
if [ ! -d /usr/include/explore ]; then
    sudo mkdir /usr/include/explore
fi

# 使用 find 命令递归查找 base 和 net/core 目录下的所有 .h 文件并拷贝
for header in $(find base net/core -name "*.h")
do
    sudo cp $header /usr/include/explore
done

# 如果构建了HTTP模块，也拷贝HTTP头文件
if [ "$BUILD_HTTP" = "ON" ]; then
    for header in $(find net/http -name "*.h")
    do
        sudo cp $header /usr/include/explore
    done
fi

# 拷贝编译好的动态库到系统库路径
sudo cp $(pwd)/lib/libexplore.so /usr/lib

# 刷新系统动态库缓存
sudo ldconfig

echo "Build and install completed successfully!"