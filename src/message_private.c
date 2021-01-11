/*
 * Copyright(C) 2019 Ruijie Network. All rights reserved.
 */
/*!
* \file message_private.c
* \brief 消息编解码内部实现 
* 
* 主要用于实现消息编解码内部设计逻辑，但对调用者无需感知 
* 
* \author hongchunhua@ruijie.com.cn
* \version v1.0.0
* \date 2020.08.05 
*/

#include "khash.h"
#include "message_private.h"


/*! @brief   定义哈希表所用的方法 */
KHASH_MAP_INIT_INT64(_hashmap_int64_to_ptr, struct message_method*);

static int64_t _check_continuity_id(const struct message_method *box, uint32_t box_ranks);
static enum _method_type _get_method_type(const struct message_method *box, uint32_t box_ranks, struct _message_handle *handle);
static struct message_method* get_method_by_array_group(message_handle handle, uint32_t id);
static void* create_hashtable(const struct message_method *box, uint32_t box_ranks);
static struct message_method* get_method_by_hashtable(message_handle handle, uint32_t id);

void create_message_box(struct _message_handle *handle, const struct message_method *box, uint32_t method_num)
{
	handle->box = (struct message_method *)box;
	handle->type = _get_method_type(box, method_num, handle);
	switch (handle->type)
	{
	case METHOD_E_ARRAY_GROUP:
		handle->get = &get_method_by_array_group;
		//break;
	case METHOD_E_HASH_TABLE:
		handle->hash_table = create_hashtable(handle->box, method_num);
		if (!handle->hash_table){
			MSG_LOG_ERROR("kh_init fail");
			goto error;
		}
		handle->get = &get_method_by_hashtable;
		break;
	default:
		goto error;
	}
	handle->flag = _MESSAGE_FLAG;
	handle->num = method_num;
	handle->init = 1;
	MSG_LOG_DEBUG("create_message_box success.");
	return;
error:
	handle->init =0;
	handle->flag =0;
	handle->get = NULL;
	handle->hash_table =NULL;
	return;
}

void destroy_message_box(struct _message_handle *handle)
{
	handle->type = _get_method_type(handle->box, handle->num, handle);
	switch (handle->type)
	{
	case METHOD_E_ARRAY_GROUP:
		//break;
	case METHOD_E_HASH_TABLE:
		if (handle->hash_table){
			kh_clear(_hashmap_int64_to_ptr, (khash_t(_hashmap_int64_to_ptr) *)handle->hash_table);
			kh_destroy(_hashmap_int64_to_ptr, (khash_t(_hashmap_int64_to_ptr) *)handle->hash_table);
			handle->hash_table = NULL;
		}
		break;
	default:
		goto error;
	}
	handle->flag = 0;
	handle->num = 0;
	handle->init = 0;
	MSG_LOG_DEBUG("destroy_message_box success.");
	return;
error:
	return;
}

static int64_t _check_continuity_id(const struct message_method *box, uint32_t box_ranks)
{
	uint32_t pre_id = 0;
	int64_t base_id = -1;
	uint32_t i;

	for (i = 0; i < box_ranks; i++) {
		if((box[i].id > 0) && (pre_id > 0) && 
			((box[i].id == pre_id + 1) || (box[i].id +1 == pre_id))){
			pre_id = box[i].id;
			continue;
		}else if (box[i].id == 0){
			base_id = box[i].id;
			continue;
		}else  if (base_id < 0){
			base_id = box[i].id;
			pre_id = box[i].id;
			continue;
		}
		return -1;
	}
	return base_id;
}


static struct message_method* get_method_by_array_group(message_handle handle, uint32_t id)
{
	struct _message_handle *h = (struct _message_handle *)handle;
	LOG_THEN_RETURN_VAL_IF_TRUE((id < h->array.base), NULL, "id[%u] can't less with base[%u].", id, h->array.base);
	LOG_THEN_RETURN_VAL_IF_TRUE((id >= h->array.base + h->num), NULL, 
								"id[%u] can't over base[%u] + num[%u].", 
								id, h->array.base, h->num);
	return &h->box[(id - h->array.base)];
}

static void* create_hashtable(const struct message_method *box, uint32_t box_ranks)
{
	uint32_t i;
	khiter_t iter = 0; //iter
	int ret = 0;
	khash_t(_hashmap_int64_to_ptr) *map = kh_init(_hashmap_int64_to_ptr);

	LOG_THEN_RETURN_VAL_IF_TRUE((!box_ranks), NULL, "box_ranks is 0,fail.");
	LOG_THEN_GOTO_TAG_IF_VAL_TRUE(!map, error, "kh_init hashmap fail.");
	for (i = 0; i < box_ranks; i++) {
		if (!box[i].encode || !box[i].decode) {
			MSG_LOG_ERROR("encode_cb or decode_cb is empty, it not allow.");
			goto error;
		}
		iter = kh_get(_hashmap_int64_to_ptr, map, box[i].id);
		if (iter != kh_end(map)){
			MSG_LOG_ERROR("the msg id[%u] is not unique, not allow.", box[i].id);
			goto error;
		}
		iter = kh_put(_hashmap_int64_to_ptr, map, (int64_t)box[i].id, &ret);
		if (ret < 0) {
			MSG_LOG_ERROR("kh_put msg_id[%u] fail.", box[i].id);
			goto error;
		}
		kh_value(map, iter) = (struct message_method*)&box[i];
		//MSG_LOG_DEBUG("iter[%u] insert key[%u] and value[%p] success.", (uint32_t)iter, box[i].id, &box[i]);
	}
	
	return (void*)map;
error:
	if (map){
		kh_clear(_hashmap_int64_to_ptr, map);
		kh_destroy(_hashmap_int64_to_ptr, map);
	}
	return NULL;
}

static struct message_method* get_method_by_hashtable(message_handle handle, uint32_t id)
{
	struct _message_handle *h = (struct _message_handle *)handle;
	khash_t(_hashmap_int64_to_ptr) *map =NULL;
	khiter_t iter = 0; //iter

	LOG_THEN_RETURN_VAL_IF_TRUE((!h->hash_table), NULL, "hash_table is null.");
	map = (khash_t(_hashmap_int64_to_ptr) *)h->hash_table;
	iter = kh_get(_hashmap_int64_to_ptr, map, id);
	if (iter != kh_end(map)){
		return kh_val(map, iter);
	}
	MSG_LOG_ERROR("msg id[%u] not exist.", id);
	return NULL;
}

//
static enum _method_type _get_method_type(const struct message_method *box, uint32_t box_ranks, struct _message_handle *handle)
{
	int64_t ret_base = -1;

	ret_base = _check_continuity_id(box, box_ranks);
	LOG_THEN_RETURN_VAL_IF_TRUE((ret_base < 0), METHOD_E_UNKOWN_TYPE, "unkown type."); // 目前先支持连续的
	if (ret_base >= 0) {
		handle->array.base = ret_base;
		return METHOD_E_ARRAY_GROUP;
	}
	return METHOD_E_HASH_TABLE;
}
/*! @brief 内存分配*/
void *_system_alloc(void *ops_ctx, size_t size)
{
	return malloc(size);
}

/*! @brief 内存释放*/
void _system_free(void *ops_ctx, void *pointer)
{
	free(pointer);
	return;
}