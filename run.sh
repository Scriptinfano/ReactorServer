#!/bin/bash
if [ ! -f "bin/main" ];then
    echo "请运行根目录下的 build.sh 在 bin 目录下生成可执行文件"
    exit 1
fi
./bin/main