#!/bin/bash
# 获取当前路径的最后一个文件夹名称
current_dir_name=$(basename "$PWD")

# 检查是否为ReactorServer
if [ "$current_dir_name" != "ReactorServer" ]; then
  echo "请在项目根目录下运行此脚本文件"
  exit 1
fi

cd build
cmake ..
make
