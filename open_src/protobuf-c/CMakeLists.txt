cmake_minimum_required(VERSION 2.8)
project(protobuf-c)

include("${COM_ROOT_PATH}/common.cmake")

set(PB_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

message("## NOTE ##: project to build for protobuf.")



#设定源码
set(SOURCE_FILES "")
aux_source_directory(${PB_ROOT_PATH}/ SOURCE_FILES)
 
#设定头文件路径
include_directories(${PB_ROOT_PATH})
 
 
#设定链接库的路径（一般使用第三方非系统目录下的库）
set(LINK_LIB_PATH ${DPENDENCY_LIB_PATH})
LINK_DIRECTORIES(${LIBRARY_OUTPUT_PATH} ${LINK_LIB_PATH})

 
#添加依赖项子目录
#add_dependencies(xio)
set(BUILD_STATIC true)

if(BUILD_STATIC)
	add_library(protobuf-c STATIC ${SOURCE_FILES})        
	INSTALL(TARGETS protobuf-c ARCHIVE DESTINATION ${LINK_LIB_PATH})
else()
	#动态库
	add_library(protobuf-c SHARED ${SOURCE_FILES})
	INSTALL(TARGETS protobuf-c LIBRARY DESTINATION ${LINK_LIB_PATH})
	INSTALL(TARGETS protobuf-c LIBRARY DESTINATION /usr/lib)
endif()
 

