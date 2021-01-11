cmake_minimum_required(VERSION 2.8)
project(message_demo)

include("${COM_ROOT_PATH}/common.cmake")

#设定源码
set(ARPC_INCLUDE ${COM_ROOT_PATH}/inc)


set(SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/main.c)

 
#设定头文件路径
include_directories(${ARPC_INCLUDE})
 
 
#设定链接库的路径（一般使用第三方非系统目录下的库）
set(LINK_LIB_PATH ${DPENDENCY_LIB_PATH})
LINK_DIRECTORIES(${LIBRARY_OUTPUT_PATH} ${LINK_LIB_PATH})
 
 
#添加依赖项子目录

#生成可执行文件
add_executable(message_demo ${SOURCE_FILES})

target_link_libraries(message_demo -lfsmsg -lpthread)


