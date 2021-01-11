/*
 * Copyright(C) 2020 Ruijie Network. All rights reserved.
 */

/*!
* \file xxx.x
* \brief xxx
* 
* 包含..
*
* \copyright 2020 Ruijie Network. All rights reserved.
* \author hongchunhua@ruijie.com.cn
* \version v1.0.0
* \date 2020.08.05
* \note none 
*/
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>


#include "fsmsg.h"

#define loop_num 10
int main(int argc, char *argv[])
{
	int ret;
	int i;
	uint32_t buf_len = 0;
	void *buf;
	const char *file_name = "file_aaaa";
	CaMdsRspReaddir test_in;
	CaMdsRspReaddir *test_out;
	CommonDentry	*pdtry[loop_num];
	CommonDentry	dtry[loop_num];
	CaMdsRspHead	rsp_head;
	FSMSG_BOX_INIT(ca_mds_rsp, NULL);
	FSMSG_BOX_INIT(common, NULL);

	fsmsg_encode_init(FSMSG_GET_OPS(ca_mds_rsp, MSG_ID_CA_MDS_REQ_HEAD), &rsp_head, sizeof(CaMdsRspHead));
	rsp_head.flags = 10086;
	rsp_head.rsp_result = 0;

	for (i = 0; i < loop_num; i ++) {
		fsmsg_encode_init(FSMSG_GET_OPS(common, MSG_ID_COMMON_DENTRY), &dtry[i], sizeof(CommonDentry));
		dtry[i].ino = 2*i + 10000;
		dtry[i].mode = 3;
		dtry[i].name = (char*)file_name;
		dtry[i].offset = i;
		buf_len = fsmsg_get_encoded_size(FSMSG_GET_OPS(common, MSG_ID_COMMON_DENTRY), &dtry[i], sizeof(CommonDentry));
		printf("------fsmsg_get_encoded_size is %u-------\n", buf_len);
		pdtry[i] = &dtry[i];
	}

	fsmsg_encode_init(FSMSG_GET_OPS(ca_mds_rsp, MSG_ID_CA_MDS_RSP_READDIR), &test_in, sizeof(CaMdsRspReaddir));
	test_in.dentry = pdtry;
	test_in.n_dentry = loop_num;
	test_in.head = &rsp_head;

	buf_len = fsmsg_get_encoded_size(FSMSG_GET_OPS(ca_mds_rsp, MSG_ID_CA_MDS_RSP_READDIR), &test_in, sizeof(CaMdsRspReaddir));
	if (!buf_len) {
		printf("------fsmsg_get_encoded_size is 0-------\n");
		return 0;
	}
	printf("------fsmsg_get_encoded_size is %u-------\n", buf_len);

	buf = malloc(buf_len);
	// 编码
	ret = fsmsg_encode(FSMSG_GET_OPS(ca_mds_rsp, MSG_ID_CA_MDS_RSP_READDIR), &test_in, buf);

	if (!ret){
		printf("------fsmsg_encode-------\n");
	}else{
		printf("message_encode_new fail:");
		goto end;
	}

	// 解码
	test_out = fsmsg_decode_new(FSMSG_GET_OPS(ca_mds_rsp, MSG_ID_CA_MDS_RSP_READDIR), buf, buf_len);

	if (test_out){
		printf("------decode-------\n");
		printf("n_dentry:%lu\n", test_out->n_dentry);
		printf("rsp_result:%d\n", test_out->head->rsp_result);
		for (i = 0; i < loop_num; i++) {
			printf("dentry[%d]->ino:%lu\n", i, test_out->dentry[i]->ino);
			printf("dentry[%d]->name:%s\n", i, test_out->dentry[i]->name);
		}
	}else{
		printf("message_decode_new fail:");
	}
end:
	// 释放资源
	if (buf)
		free(buf);

	if (test_out)
		fsmsg_decode_delete(FSMSG_GET_OPS(ca_mds_rsp, MSG_ID_CA_MDS_RSP_READDIR), test_out);

	FSMSG_BOX_EXIT(ca_mds_rsp);
	FSMSG_BOX_EXIT(common);

	return 0;
}

