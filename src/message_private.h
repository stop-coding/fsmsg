/*
 * Copyright(C) 2020 Ruijie Network. All rights reserved.
 */

/*!
* \file fsmsg.h
* \brief 消息编解码方法定义
* 
* ....
*
* \copyright 2020 Ruijie Network. All rights reserved.
* \author hongchunhua@ruijie.com.cn
* \version v1.0.0
* \date 2020.08.05
* \note none 
*/

#ifndef _MESSAGE_PRIVATE_H_
#define _MESSAGE_PRIVATE_H_

#include "message_com.h"


#ifdef __cplusplus
extern "C"
{
#endif

/*!
 *  @brief  实例化一个消息
 *
 *  @param[in] 	 msgid  	  消息id
 *  @param[in]   msg_name     消息名称
 *
 */
/*! @brief   通过名称注册全部回调函数，用于模板化生成的回调函数，如protobuf */
#define REGISTER_MESSAGE(msgid, msg_name)																\
{	.id=msgid,																							\
	.encode_init=&_##msg_name##_encode_init,															\
	.get_encoded_size = &_##msg_name##_get_encoded_size,												\
	.encode=&_##msg_name##_encode,                        												\
	.decode=&_##msg_name##_decode,                        												\
	.decode_free = &_##msg_name##_decode_free															\
},

/*!
 *  @brief  实例化一个消息组
 *
 *  @param[in] 	 box_name  	  消息组名称
 *  @param[in]   msg_register     需要注册的消息
 *
 */
#define INIT_MESSAGE_BOX(box_name, msg_register)												    \
static const struct message_method                                                                  \
_##box_name##_message_box[] ={msg_register};                                                      	\
static struct _message_handle _##box_name##_handle = {0};									        \
void _init_##box_name##_message_box(struct fsmsg_allocator *ops)										\
{																						            \
	if(!_##box_name##_handle.init){															        \
		create_message_box(&_##box_name##_handle, _##box_name##_message_box,					    \
		 sizeof(_##box_name##_message_box)/sizeof(struct message_method));					        \
		 if (ops && ops->alloc && ops->free){														\
			 _##box_name##_handle.buf_ops = *ops;													\
		 }else{																						\
			 _##box_name##_handle.buf_ops.alloc = &_system_alloc;									\
			 _##box_name##_handle.buf_ops.free  = &_system_free;									\
			 _##box_name##_handle.buf_ops.ops_ctx = NULL;											\
		 }																							\
	}																					            \
	return;																				            \
}																						            \
message_handle _get_##box_name##_message_handle()											        \
{																						            \
	LOG_THEN_RETURN_VAL_IF_TRUE((!_##box_name##_handle.init), NULL, "box not init.");		        \
	return (message_handle)&_##box_name##_handle;										            \
};																						            \
void _exit_##box_name##_message_box()														        \
{																						            \
	if(_##box_name##_handle.init){															        \
		destroy_message_box(&_##box_name##_handle);											        \
	}																					            \
	return;																				            \
}

void create_message_box (struct _message_handle *handle,
			                  const struct message_method *box,
			                  uint32_t method_num);
void destroy_message_box (struct _message_handle *handle);

/*! @brief 内存分配*/
void *_system_alloc(void *ops_ctx, size_t size);

/*! @brief 内存释放*/
void _system_free(void *ops_ctx, void *pointer);

#ifdef __cplusplus
}
#endif

#endif
