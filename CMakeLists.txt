cmake_minimum_required(VERSION 3.5.1)
project(REACTOR VERSION 0.1.0 LANGUAGES CXX)

add_compile_options(-std=c++11)  # 添加编译选项

set(CMAKE_BUILD_TYPE Debug)
include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB IMPL_FILES src/impl/*.cpp)
set(EXEC_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_executable(main ${CMAKE_SOURCE_DIR}/src/main.cpp ${IMPL_FILES})
set_target_properties(main PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${EXEC_DIRECTORY})
# 以下内容就可以自定义了，上面基本上不用动了
add_executable(epollclient ${CMAKE_SOURCE_DIR}/src/epollclient.cpp ${IMPL_FILES})
set_target_properties(epollclient PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${EXEC_DIRECTORY})

add_executable(epollserver ${CMAKE_SOURCE_DIR}/src/epollserver.cpp ${IMPL_FILES})
set_target_properties(epollserver PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${EXEC_DIRECTORY})