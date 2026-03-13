#!/bin/bash

set -e

# 若没有build目录，创建该目录
if [ ! -d $(pwd)/build ]; then
    mkdir $(pwd)/build
fi

# 清理旧的编译缓存
rm -rf $(pwd)/build/*

# 进入 build 目录，执行 cmake 生成 Makefile，并调用 make 编译
cd $(pwd)/build &&
    cmake .. &&
    make

# 回到项目根目录
cd ..

# 把头文件拷贝到 /usr/include/explore，so库拷贝到 /usr/lib 
if [ ! -d /usr/include/explore ]; then
    sudo mkdir /usr/include/explore
fi

# 使用 find 命令递归查找 base 和 net 目录下的所有 .h 文件并拷贝
for header in $(find base net -name "*.h")
do
    sudo cp $header /usr/include/explore
done

# 拷贝编译好的动态库到系统库路径
sudo cp $(pwd)/lib/libexplore.so /usr/lib

# 刷新系统动态库缓存
sudo ldconfig

echo "Build and install completed successfully!"