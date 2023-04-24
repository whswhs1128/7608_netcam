/*!
*****************************************************************************
** \file        solution/software/subsystem/onvif-no-gsoap/src/sys/mpool.h
**
** \brief       memory pool
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2013 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#ifndef	__H_MPOOL_H__
#define __H_MPOOL_H__
#include "onvif_std.h"

/***************************************************************************************/

typedef struct
{
	GK_U32		prev_node;  //prev node offser addr, but = 0 in head node
	GK_U32		next_node;  //next node offset addr, but = 0 in tail node 
	GK_U32		node_flag;	//0:idle    1:in FreeList   2:in UsedList
}MPOOL;

typedef struct
{
	GK_U32		fl_base;    //header addr
	GK_U32		head_node;  //head node addr
	GK_U32		tail_node;  //tail node addr
	GK_U32		node_num;   //node number in cln_fl or cln_ul
	GK_U32		low_offset; //head data add
	GK_U32      high_offset;//tail data addr
	GK_U32      unit_size;  //node size
	void	   *ctx_mutex;  //mutex
	GK_U32		pop_cnt;    //hao many node data to cln_ul to use
	GK_U32		push_cnt;   //hao many node get from cln_ul when one http message handling ended.
}MPOOL_CTX;

#ifdef __cplusplus
extern "C" {
#endif


MPOOL_CTX  * mpool_ctx_freelist_init(GK_U32 node_num,GK_U32 content_size,GK_BOOL bNeedMutex);
MPOOL_CTX  * mpool_ctx_freelist_init_assign(GK_U32 mem_addr, GK_U32 mem_len, GK_U32 node_num, GK_U32 content_size, GK_BOOL bNeedMutex);

void 		mpool_freelist_free(MPOOL_CTX * fl_ctx);
void 		mpool_freelist_reinit(MPOOL_CTX * fl_ctx);
GK_BOOL 	mpool_freelist_push(MPOOL_CTX * pps_ctx,void * content_ptr);
GK_BOOL 	mpool_freelist_push_tail(MPOOL_CTX * pps_ctx,void * content_ptr);
void 	  * mpool_freelist_pop(MPOOL_CTX * pps_ctx);

MPOOL_CTX  * mpool_ctx_uselist_init(MPOOL_CTX * fl_ctx,GK_BOOL bNeedMutex);
GK_BOOL 	mpool_ctx_uselist_init_assign(MPOOL_CTX * ul_ctx, MPOOL_CTX * fl_ctx,GK_BOOL bNeedMutex);
GK_BOOL 	mpool_ctx_uselist_init_nm(MPOOL_CTX * fl_ctx,MPOOL_CTX * ul_ctx);

void 		mpool_uselist_reinit(MPOOL_CTX * ul_ctx);
void 		mpool_uselist_free(MPOOL_CTX * ul_ctx);

GK_BOOL 	mpool_ctx_uselist_del(MPOOL_CTX * ul_ctx,void * content_ptr);

MPOOL 	  * mpool_ctx_uselist_del_node_unlock(MPOOL_CTX * ul_ctx,MPOOL * p_node);
void 	  * mpool_ctx_uselist_del_unlock(MPOOL_CTX * ul_ctx,void * content_ptr);

GK_BOOL 	mpool_ctx_uselist_add(MPOOL_CTX * ul_ctx,void * content_ptr);
GK_BOOL 	mpool_ctx_uselist_add_head(MPOOL_CTX * ul_ctx,void * content_ptr);


void 	  * mpool_lookup_start(MPOOL_CTX * pps_ctx);
void 	  * mpool_lookup_next(MPOOL_CTX * pps_ctx, void * ct_ptr);
void		mpool_lookup_end(MPOOL_CTX * pps_ctx);

void 		mpool_wait_mutex(MPOOL_CTX * pps_ctx);
void 		mpool_post_mutex(MPOOL_CTX * pps_ctx);

GK_BOOL 	mpool_safe_node(MPOOL_CTX * pps_ctx,void * content_ptr);
GK_BOOL 	mpool_idle_node(MPOOL_CTX * pps_ctx,void * content_ptr);
GK_BOOL 	mpool_exist_node(MPOOL_CTX * pps_ctx,void * content_ptr);
GK_BOOL 	mpool_used_node(MPOOL_CTX * pps_ctx,void * content_ptr);


#ifdef __cplusplus
}
#endif

#endif


