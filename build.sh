##########################################################################
# File Name: build.sh
# Author: amoscykl
# mail: amoscykl980629@163.com
# Created Time: 三 10/30 19:58:22 2024
#########################################################################
#!/bin/zsh
cd build
cmake ..
make
chmod +x ../bin/main
chmod +x ../bin/producer
chmod +x ../bin/consumer