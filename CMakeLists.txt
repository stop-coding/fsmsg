###############################################################
#*【项目】CA
#*【描述】
#*【作者】hongchunhua
#*【时间】2020.07.22
###############################################################

cmake_minimum_required(VERSION 3.0)
project(fsmsg)

set(COM_ROOT_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

#设置依赖的文件路径
set(MESSAGE_ROOT_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

	#开源
set(OPENSRC_PATH "${MESSAGE_ROOT_PATH}/open_src")

set(DEMO_PATH "${MESSAGE_ROOT_PATH}/demo")

include("${COM_ROOT_PATH}/common.cmake")

#设定源码
set(MSG_INCLUDE ${MESSAGE_ROOT_PATH}/inc)
set(PROTOBUF_C ${OPENSRC_PATH}/protobuf-c/)
set(SRC_MESSAGE ${COM_ROOT_PATH}/src)
set(SRC_RPOTO ${COM_ROOT_PATH}/proto)
set(SRC_KLIB ${OPENSRC_PATH}/klib)

execute_process(COMMAND python3 ${SRC_RPOTO}/build_message.py ${SRC_RPOTO} ${MESSAGE_ROOT_PATH} RESULT_VARIABLE ret_val ERROR_VARIABLE ret_val)

if (NOT ${ret_val} EQUAL 0)
	message("error, build_message.py exec. error message: ${ret_val}")
endif()

set(SOURCE_FILES "")
aux_source_directory(${SRC_MESSAGE} SOURCE_FILES)
aux_source_directory(${PROTOBUF_C} SOURCE_FILES)
 
#设定头文件路径
include_directories(${MSG_INCLUDE} ${SRC_KLIB} ${PROTOBUF_C} ${SRC_MESSAGE} )
 
 
#设定链接库的路径（一般使用第三方非系统目录下的库）
LINK_DIRECTORIES(${LIBRARY_OUTPUT_PATH} )
 
 
#添加依赖项子目录

#动态库
add_library(fsmsg SHARED ${SOURCE_FILES})



# 使动态库和静态库同时存在
set_target_properties(fsmsg PROPERTIES CLEAN_DIRECT_OUTPUT 1)

INSTALL(TARGETS fsmsg LIBRARY DESTINATION ${LIBRARY_OUTPUT_PATH})
configure_file(${PROTOBUF_C}/protobuf-c.h ${MSG_INCLUDE}/protobuf-c/protobuf-c.h)

#子项目
add_subdirectory("${DEMO_PATH}/message_demo")