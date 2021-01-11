/*
 * Copyright(C) 2020 Ruijie Network. All rights reserved.
 */

/*!
* \file xxx.x
* \brief 补充概述
* 
* 待补充详细描述功能
*
* \copyright 2020 Ruijie Network. All rights reserved.
* \author hongchunhua@ruijie.com.cn
* \version v1.0.0
* \date 2020.08.05
* \note none 
*/

#ifndef _MESSAGE_COM_H_
#define _MESSAGE_COM_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>

#include "fsmsg.h"

#define BASE_ERROR			-1
#define BASE_SUCCESS		 0

//#define BASE_DEBUG_ON
#define BASE_LOG_ERROR(format, arg...) syslog(LOG_ERR, 	"[FSMSG] [ ERROR] file:%s func: %s|%d---"format"\n", __FILE__, __FUNCTION__, __LINE__, ##arg)
#define BASE_LOG_NOTICE(format, arg...) syslog(LOG_NOTICE, "[FSMSG] [NOTICE] func: %s|%d---"format"\n",__FUNCTION__, __LINE__, ##arg)

#ifdef BASE_DEBUG_ON
#define BASE_LOG_DEBUG(format, arg...) syslog(LOG_DEBUG, "[FSMSG] [ DEBUG] func: %s|%d---"format"\n",__FUNCTION__, __LINE__, ##arg)
#else
#define BASE_LOG_DEBUG(format, arg...)
#endif

#define unlikely(x)    __builtin_expect(!!(x), 0)

#define LOG_THEN_GOTO_TAG_IF_VAL_TRUE(val, tag, format, arg...)	\
do{\
	if(unlikely((val))){\
		BASE_LOG_ERROR(format,##arg);\
		goto tag;\
	}\
}while(0);

#define LOG_DEBUG_GOTO_TAG_IF_VAL_TRUE(val, tag, format, arg...)	\
do{\
	if(unlikely((val))){\
		BASE_LOG_DEBUG(format,##arg);\
		goto tag;\
	}\
}while(0);

#define LOG_THEN_RETURN_IF_VAL_TRUE(val, format, arg...)	\
do{\
	if(unlikely((val))){\
		BASE_LOG_ERROR(format,##arg);\
		return;\
	}\
}while(0);

#define LOG_THEN_RETURN_VAL_IF_TRUE(val, ret, format, arg...)\
do{\
	if(unlikely((val))){\
		BASE_LOG_ERROR(format, ##arg);\
		return ret;\
	}\
}while(0);

#define BASE_ASSERT(condition, format, arg...)	\
do{\
	if(unlikely((condition))){\
		BASE_LOG_ERROR(format, ##arg);\
		assert(!condition);\
	}\
}while(0);

#define LOG_ERROR_IF_VAL_TRUE(val, format, arg...)	\
do{\
	if(unlikely((val))){\
		BASE_LOG_ERROR(format,##arg);\
	}\
}while(0);

#define MSG_LOG_ERROR(format, arg...) BASE_LOG_ERROR(format, ##arg)
#define MSG_LOG_NOTICE(format, arg...) BASE_LOG_NOTICE(format, ##arg)
#define MSG_LOG_DEBUG(format, arg...) 	BASE_LOG_DEBUG(format,  ##arg)
#define MSG_ASSERT(condition, format, arg...) BASE_ASSERT(condition, format, ##arg)

#define MSG_MEM_ALLOC(size, usr_context) malloc(size)
#define MSG_MEM_FREE(p, usr_context) free(p)

/*! @brief 内部结构体标识码 */
#define _MESSAGE_FLAG  0xa5ee


/**
 * @brief
 *  注册的编解码方法
 *
 * @details
 *  用于编码方法回调函数
 *  
 */
struct message_method{
	uint32_t id;
    void (*encode_init)(void *in, uint32_t in_len);
    uint32_t (*get_encoded_size)(void *in, uint32_t in_len);
    int (*encode)(void *in, void *out);
    void* (*decode)(struct fsmsg_allocator *ops, void *in, uint32_t in_len);
    void (*decode_free)(struct fsmsg_allocator *ops, void *out);
};

/**
 * @brief  消息编解码索引方法
 *
 * @details
 *  主要包含数据索引和哈希表索引两种方式
 *  
 */
enum _method_type{
    METHOD_E_UNKOWN_TYPE = 0,	
    METHOD_E_ARRAY_GROUP,
    METHOD_E_HASH_TABLE,	
};

struct _method_array{
    uint32_t base;
};


/**
 * @brief
 *  消息操作句柄
 *
 * @details
 *  存储一个消息组的所有消息id和其实现方法的变量
 *  
 */
struct _message_handle{
    uint8_t init;
    uint32_t flag;
    struct fsmsg_allocator buf_ops;
    struct message_method *box;
    uint32_t num;
    enum _method_type type;
    union
    {
      struct _method_array array;
    };
    struct message_method *(*get)(void *, uint32_t);
    void *hash_table;
    void *ex;
};

struct _handle_ex_data{
    void *p;
};

typedef void *message_handle;

#endif
