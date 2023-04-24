/*!
*****************************************************************************
** \file        solution/software/subsystem/onvif-no-gsoap/src/sys/mpool.c
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
#include "mpool.h"
static GK_S32 mpool_os_mutex_locked(void * p_sem);
static void mpool_os_mutex_unlocked(void * p_sem);
static void mpool_os_mutex_destroy(void * ptr);
static void *mpool_os_mutex_create();

MPOOL_CTX *mpool_ctx_freelist_init_assign(GK_U32 mem_addr, GK_U32 mem_len, GK_U32 node_num, GK_U32 content_size, GK_BOOL bNeedMutex)
{
	GK_U32 unit_len = content_size + sizeof(MPOOL);
	GK_U32 content_len = node_num * unit_len;

	if(mem_len < (content_len + sizeof(MPOOL_CTX)))
	{
		return NULL;
	}

	MPOOL_CTX * ctx_ptr = (MPOOL_CTX *)mem_addr;
	memset(ctx_ptr, 0, sizeof(MPOOL_CTX));

	memset((GK_CHAR *)(mem_addr+sizeof(MPOOL_CTX)),0,content_len);

	GK_U32 i=0;
	for(i=0; i<node_num; i++)
	{
		GK_U32 offset = sizeof(MPOOL_CTX) + unit_len * i;
		MPOOL *p_node = (MPOOL *)(mem_addr + offset);
		if(ctx_ptr->head_node == 0)
		{
			ctx_ptr->head_node = offset;
			ctx_ptr->tail_node = offset;
		}
		else
		{
			MPOOL *p_prev_node = (MPOOL *)(mem_addr + ctx_ptr->tail_node);
			p_prev_node->next_node = offset;
			p_node->prev_node = ctx_ptr->tail_node;
			ctx_ptr->tail_node = offset;
		}

		p_node->node_flag = 1;

		(ctx_ptr->node_num)++;
	}

	if(bNeedMutex)
		ctx_ptr->ctx_mutex = mpool_os_mutex_create();
	else
		ctx_ptr->ctx_mutex = 0;

	ctx_ptr->fl_base = (GK_U32)ctx_ptr;
	ctx_ptr->low_offset = sizeof(MPOOL_CTX) + sizeof(MPOOL);
	ctx_ptr->high_offset = sizeof(MPOOL_CTX) + content_len - unit_len + sizeof(MPOOL);
	ctx_ptr->unit_size = unit_len;

	return ctx_ptr;
}

MPOOL_CTX * mpool_ctx_freelist_init(GK_U32 node_num, GK_U32 content_size, GK_BOOL bNeedMutex)
{
	GK_U32 unit_len = content_size + sizeof(MPOOL);
	GK_U32 content_len = node_num * unit_len;

	GK_CHAR *content_ptr = (char *)malloc(content_len + sizeof(MPOOL_CTX)); //MPOOL_CTX/MPOOL+1024/MPOOL+1024/...
	if(content_ptr == NULL)
	{
		return NULL;
	}

	MPOOL_CTX * ctx_ptr = mpool_ctx_freelist_init_assign(
		(GK_U32)content_ptr,content_len+sizeof(MPOOL_CTX),
		node_num,content_size,bNeedMutex);

	return ctx_ptr;
}

void mpool_freelist_free(MPOOL_CTX * fl_ctx)
{
	if(fl_ctx == NULL) return;

	if(fl_ctx->ctx_mutex)
	{
		mpool_os_mutex_destroy(fl_ctx->ctx_mutex);
	}

	free(fl_ctx);
}

/***************************************************************************************/
void mpool_freelist_reinit(MPOOL_CTX * fl_ctx)
{
	if(fl_ctx == NULL) return;

	GK_U32 mem_addr = (GK_U32)fl_ctx;

	mpool_wait_mutex(fl_ctx);

	GK_CHAR * content_start = (GK_CHAR *)(mem_addr + fl_ctx->low_offset - sizeof(MPOOL));
	GK_CHAR * content_end = (GK_CHAR *)(mem_addr + fl_ctx->high_offset - sizeof(MPOOL) + fl_ctx->unit_size);

	GK_U32 content_len = content_end - content_start;
	fl_ctx->node_num = content_len / fl_ctx->unit_size;
	fl_ctx->head_node = 0;
	fl_ctx->tail_node = 0;

	memset(content_start,0,content_len);

	GK_S32 i=0;
	for(i=0; i<fl_ctx->node_num; i++)
	{
		GK_U32 offset = sizeof(MPOOL_CTX) + fl_ctx->unit_size * i;
		MPOOL * p_node = (MPOOL *)(mem_addr + offset);
		if(fl_ctx->head_node == 0)
		{
			fl_ctx->head_node = offset;
			fl_ctx->tail_node = offset;
		}
		else
		{
			MPOOL * p_prev_node = (MPOOL *)(mem_addr + fl_ctx->tail_node);
			p_prev_node->next_node = offset;
			p_node->prev_node = fl_ctx->tail_node;
			fl_ctx->tail_node = offset;
		}

		p_node->node_flag = 1;
	}

	mpool_post_mutex(fl_ctx);
}

GK_BOOL mpool_freelist_push(MPOOL_CTX * pps_ctx,void * content_ptr)
{
	if(pps_ctx == NULL || content_ptr == NULL)
		return GK_FALSE;

	if(mpool_safe_node(pps_ctx, content_ptr) == GK_FALSE)
	{
		return GK_FALSE;
	}

	MPOOL *p_node = (MPOOL *)(((GK_U32)content_ptr) - sizeof(MPOOL));

	GK_U32 offset = (GK_U32)p_node - pps_ctx->fl_base;

	mpool_wait_mutex(pps_ctx);

	if(p_node->node_flag == 1)
	{
		mpool_post_mutex(pps_ctx);
		return GK_FALSE;
	}

	p_node->prev_node = 0;
	p_node->node_flag = 1;

	if(pps_ctx->head_node == 0)
	{
		pps_ctx->head_node = offset;
		pps_ctx->tail_node = offset;
		p_node->next_node = 0;
	}
	else
	{
		MPOOL *p_prev = (MPOOL *)(pps_ctx->head_node + pps_ctx->fl_base);
		p_prev->prev_node = offset;
		p_node->next_node = pps_ctx->head_node;
		pps_ctx->head_node = offset;
	}

	pps_ctx->node_num++;
	pps_ctx->push_cnt++;

	mpool_post_mutex(pps_ctx);

	return GK_TRUE;
}

GK_BOOL mpool_freelist_push_tail(MPOOL_CTX * pps_ctx,void * content_ptr)
{
	if(pps_ctx == NULL || content_ptr == NULL)
		return GK_FALSE;

	if(mpool_safe_node(pps_ctx, content_ptr) == GK_FALSE)
	{
		return GK_FALSE;
	}

	MPOOL *p_node = (MPOOL *)(((GK_U32)content_ptr) - sizeof(MPOOL));

	GK_U32 offset = (GK_U32)p_node - pps_ctx->fl_base;

	mpool_wait_mutex(pps_ctx);

	if(p_node->node_flag == 1)
	{
		mpool_post_mutex(pps_ctx);
		return GK_FALSE;
	}

	p_node->prev_node = 0;
	p_node->next_node = 0;
	p_node->node_flag = 1;

	if(pps_ctx->tail_node == 0)
	{
		pps_ctx->head_node = offset;
		pps_ctx->tail_node = offset;
	}
	else
	{
		p_node->prev_node = pps_ctx->tail_node;
		MPOOL *p_prev = (MPOOL *)(pps_ctx->tail_node + (GK_U32)pps_ctx);
		p_prev->next_node = offset;
		pps_ctx->tail_node = offset;
	}

	pps_ctx->node_num++;
	pps_ctx->push_cnt++;

	mpool_post_mutex(pps_ctx);

	return GK_TRUE;
}

void * mpool_freelist_pop(MPOOL_CTX * pps_ctx)
{
	if(pps_ctx == NULL)
		return NULL;

	mpool_wait_mutex(pps_ctx);

	if(pps_ctx->head_node == 0)
	{
		mpool_post_mutex(pps_ctx);
		return NULL;
	}

	MPOOL *p_node = (MPOOL *)(pps_ctx->fl_base + pps_ctx->head_node);

	pps_ctx->head_node = p_node->next_node;

	if(pps_ctx->head_node == 0)
		pps_ctx->tail_node = 0;
	else
	{
		MPOOL *p_new_head = (MPOOL *)(pps_ctx->fl_base + pps_ctx->head_node);
		p_new_head->prev_node = 0;
	}

	(pps_ctx->node_num)--;
	(pps_ctx->pop_cnt)++;

	mpool_post_mutex(pps_ctx);

	memset(p_node,0,sizeof(MPOOL)); //get HTTPCLN from cln_fl, and then memset corresponding, so node_flag = 0

	void * ret_ptr = (void *)(((GK_U32)p_node) + sizeof(MPOOL));

	return ret_ptr;
}

/***************************************************************************************/
MPOOL_CTX * mpool_ctx_uselist_init(MPOOL_CTX *fl_ctx,GK_BOOL bNeedMutex)
{
	if(fl_ctx == NULL)
		return NULL;

	MPOOL_CTX *ctx_ptr = (MPOOL_CTX *)malloc(sizeof(MPOOL_CTX));
	if(ctx_ptr == NULL)
	{
		return NULL;
	}

	memset(ctx_ptr,0,sizeof(MPOOL_CTX));

	ctx_ptr->fl_base = fl_ctx->fl_base;
	ctx_ptr->high_offset = fl_ctx->high_offset;
	ctx_ptr->low_offset = fl_ctx->low_offset;
	ctx_ptr->unit_size = fl_ctx->unit_size;

	if(bNeedMutex)
		ctx_ptr->ctx_mutex = mpool_os_mutex_create();
	else
		ctx_ptr->ctx_mutex = 0;

	return ctx_ptr;
}

GK_BOOL mpool_ctx_uselist_init_assign(MPOOL_CTX * ul_ctx, MPOOL_CTX * fl_ctx,GK_BOOL bNeedMutex)
{
	if(ul_ctx == NULL || fl_ctx == NULL)
		return GK_FALSE;

	memset(ul_ctx,0,sizeof(MPOOL_CTX));

	ul_ctx->fl_base = fl_ctx->fl_base;
	ul_ctx->high_offset = fl_ctx->high_offset;
	ul_ctx->low_offset = fl_ctx->low_offset;
	ul_ctx->unit_size = fl_ctx->unit_size;

	if(bNeedMutex)
		ul_ctx->ctx_mutex = mpool_os_mutex_create();
	else
		ul_ctx->ctx_mutex = 0;

	return GK_TRUE;
}


GK_BOOL mpool_ctx_uselist_init_nm(MPOOL_CTX * fl_ctx,MPOOL_CTX * ul_ctx)
{
	return mpool_ctx_uselist_init_assign(ul_ctx, fl_ctx, GK_FALSE);
}

/***************************************************************************************/
void mpool_uselist_reinit(MPOOL_CTX * ul_ctx)
{
	if(ul_ctx == NULL) return;

	ul_ctx->node_num = 0;
	ul_ctx->head_node = 0;
	ul_ctx->tail_node = 0;

	mpool_wait_mutex(ul_ctx);
	mpool_post_mutex(ul_ctx);

	if(ul_ctx->ctx_mutex)
	{
		mpool_os_mutex_destroy(ul_ctx->ctx_mutex);
	}
}

void mpool_uselist_free(MPOOL_CTX * ul_ctx)
{
	if(ul_ctx == NULL) return;

	if(ul_ctx->ctx_mutex)
	{
		mpool_os_mutex_destroy(ul_ctx->ctx_mutex);
	}

	free(ul_ctx);
}

GK_BOOL mpool_ctx_uselist_del(MPOOL_CTX * ul_ctx,void * content_ptr)
{
	if(mpool_used_node(ul_ctx, content_ptr) == GK_FALSE)
		return GK_FALSE;

	MPOOL *p_node = (MPOOL *)(((GK_U32)content_ptr) - sizeof(MPOOL));

	mpool_wait_mutex(ul_ctx);

	if(p_node->prev_node == 0)
		ul_ctx->head_node = p_node->next_node;
	else
		((MPOOL *)(ul_ctx->fl_base + p_node->prev_node))->next_node = p_node->next_node;

	if(p_node->next_node == 0)
		ul_ctx->tail_node = p_node->prev_node;
	else
		((MPOOL *)(ul_ctx->fl_base + p_node->next_node))->prev_node = p_node->prev_node;

	(ul_ctx->node_num)--;

	mpool_post_mutex(ul_ctx);

	memset(p_node,0,sizeof(MPOOL));

	return GK_TRUE;
}

MPOOL * mpool_ctx_uselist_del_node_unlock(MPOOL_CTX * ul_ctx,MPOOL * p_node)
{
	if(p_node->node_flag != 2)
	{
		return NULL;
	}

	if(ul_ctx->head_node == 0)
	{
		return NULL;
	}

	if(p_node->prev_node == 0)
		ul_ctx->head_node = p_node->next_node;
	else
		((MPOOL *)(ul_ctx->fl_base + p_node->prev_node))->next_node = p_node->next_node;

	if(p_node->next_node == 0)
		ul_ctx->tail_node = p_node->prev_node;
	else
		((MPOOL *)(ul_ctx->fl_base + p_node->next_node))->prev_node = p_node->prev_node;

	(ul_ctx->node_num)--;

	if(p_node->next_node == 0)
		return NULL;
	else
		return (MPOOL *)(ul_ctx->fl_base + p_node->next_node);
}

void *mpool_ctx_uselist_del_unlock(MPOOL_CTX *ul_ctx,void *content_ptr)
{
	if(mpool_used_node(ul_ctx, content_ptr) == GK_FALSE)
		return GK_FALSE;

	MPOOL *p_node = (MPOOL *)(((GK_U32)content_ptr) - sizeof(MPOOL));

	MPOOL *p_ret = mpool_ctx_uselist_del_node_unlock(ul_ctx, p_node);
	if(p_ret == NULL)
		return NULL;
	else
	{
		void * ret_ptr = (void *)(((GK_U32)p_ret) + sizeof(MPOOL));
		return ret_ptr;
	}
}

GK_BOOL mpool_ctx_uselist_add(MPOOL_CTX *ul_ctx, void *content_ptr)
{
	if(mpool_safe_node(ul_ctx, content_ptr) == GK_FALSE)
		return GK_FALSE;

	MPOOL *p_node = (MPOOL *)(((GK_U32)content_ptr) - sizeof(MPOOL));
	if(p_node->node_flag != 0)//then get HTTPCLN from cln_fl, will memset the MPOOL
		return GK_FALSE;

	mpool_wait_mutex(ul_ctx);

	p_node->next_node = 0;
	p_node->node_flag = 2; //in used

	GK_U32 offset = ((GK_U32)p_node) - ul_ctx->fl_base;
	if(ul_ctx->tail_node == 0)
	{
		ul_ctx->tail_node = offset;
		ul_ctx->head_node = offset;
		p_node->prev_node = 0;
	}
	else
	{
		MPOOL *p_tail = (MPOOL *)(ul_ctx->fl_base + ul_ctx->tail_node);
		p_tail->next_node = offset;
		p_node->prev_node = ul_ctx->tail_node;
		ul_ctx->tail_node = offset;
	}

	(ul_ctx->node_num)++;

	mpool_post_mutex(ul_ctx);

	return GK_TRUE;
}

GK_BOOL mpool_ctx_uselist_add_head(MPOOL_CTX * ul_ctx,void * content_ptr)
{
	if(mpool_safe_node(ul_ctx, content_ptr) == GK_FALSE)
		return GK_FALSE;

	MPOOL *p_node = (MPOOL *)(((GK_U32)content_ptr) - sizeof(MPOOL));
	if(p_node->node_flag != 0)
		return GK_FALSE;

	mpool_wait_mutex(ul_ctx);

	GK_U32 offset = ((GK_U32)p_node) - ul_ctx->fl_base;
	p_node->node_flag = 2;
	p_node->prev_node = 0;

	if(ul_ctx->head_node == 0)
	{
		ul_ctx->tail_node = offset;
		ul_ctx->head_node = offset;
		p_node->next_node = 0;
	}
	else
	{
		MPOOL *p_head = (MPOOL *)(ul_ctx->fl_base + ul_ctx->head_node);
		p_head->prev_node = offset;
		p_node->next_node = ul_ctx->head_node;
		ul_ctx->head_node = offset;
	}

	(ul_ctx->node_num)++;

	mpool_post_mutex(ul_ctx);

	return GK_TRUE;
}


void * mpool_lookup_start(MPOOL_CTX * pps_ctx)
{
	if(pps_ctx == NULL) return NULL;

	mpool_wait_mutex(pps_ctx);

	if(pps_ctx->head_node)
	{
		void  *ret_ptr = (void *)(pps_ctx->fl_base + pps_ctx->head_node + sizeof(MPOOL));
		return ret_ptr;
	}

	return NULL;
}

void * mpool_lookup_next(MPOOL_CTX * pps_ctx, void * ctx_ptr)
{
	if(pps_ctx == NULL || ctx_ptr == NULL) return NULL;

	if((GK_U32)ctx_ptr < (pps_ctx->fl_base + pps_ctx->low_offset) ||
	   (GK_U32)ctx_ptr > (pps_ctx->fl_base + pps_ctx->high_offset))
	{
		return NULL;
	}

	MPOOL * p_node = (MPOOL *)(((GK_U32)ctx_ptr) - sizeof(MPOOL));

	if(p_node->next_node == 0)
    {
		return NULL;
    }
	else
	{
		void * ret_ptr = (void *)(pps_ctx->fl_base + p_node->next_node + sizeof(MPOOL));
		return ret_ptr;
	}
}

void mpool_lookup_end(MPOOL_CTX * pps_ctx)
{
	mpool_post_mutex(pps_ctx);
}



void mpool_wait_mutex(MPOOL_CTX * pps_ctx)
{
	if(pps_ctx == NULL)
	{
		return;
	}

	if(pps_ctx->ctx_mutex)
	{
		mpool_os_mutex_locked(pps_ctx->ctx_mutex);
	}
}

void mpool_post_mutex(MPOOL_CTX * pps_ctx)
{
	if(pps_ctx == NULL)
	{
		return;
	}

	if(pps_ctx->ctx_mutex)
	{
		mpool_os_mutex_unlocked(pps_ctx->ctx_mutex);
	}
}

GK_BOOL mpool_safe_node(MPOOL_CTX * pps_ctx,void * content_ptr)
{
	if(pps_ctx == NULL || content_ptr == NULL) return GK_FALSE;

	if((GK_U32)content_ptr < (pps_ctx->low_offset + pps_ctx->fl_base) ||
		(GK_U32)content_ptr > (pps_ctx->high_offset + pps_ctx->fl_base))
	{
		return GK_FALSE;
	}

	GK_U32 index = (GK_U32)content_ptr - pps_ctx->low_offset - pps_ctx->fl_base;
	GK_U32 offset = index % pps_ctx->unit_size;
	if(offset != 0)
	{
		index = index /pps_ctx->unit_size;
		return GK_FALSE;
	}

	return GK_TRUE;
}

GK_BOOL mpool_idle_node(MPOOL_CTX * pps_ctx,void * content_ptr)
{
	if(mpool_safe_node(pps_ctx, content_ptr) == GK_FALSE)
		return GK_FALSE;

	MPOOL * p_node = (MPOOL *)(((GK_U32)content_ptr) - sizeof(MPOOL));
	return (p_node->node_flag == 1);
}

GK_BOOL mpool_exist_node(MPOOL_CTX * pps_ctx,void * content_ptr)
{
	if(mpool_safe_node(pps_ctx, content_ptr) == GK_FALSE)
		return GK_FALSE;

	MPOOL * p_node = (MPOOL *)(((GK_U32)content_ptr) - sizeof(MPOOL));
	return (p_node->node_flag != 1);
}

GK_BOOL mpool_used_node(MPOOL_CTX * pps_ctx,void * content_ptr)
{
	if(mpool_safe_node(pps_ctx, content_ptr) == GK_FALSE)
		return GK_FALSE;

	if(pps_ctx->head_node == 0)
	{
		return GK_FALSE;
	}

	MPOOL * p_node = (MPOOL *)(((GK_U32)content_ptr) - sizeof(MPOOL));
	return (p_node->node_flag == 2);
}



static void *mpool_os_mutex_create()
{
	void * p_mutex = NULL;

	p_mutex = (sem_t *)malloc(sizeof(sem_t));
	int ret = sem_init((sem_t *)p_mutex,0,1);
	if (ret != 0)
	{
		free(p_mutex);
		return NULL;
	}

	return p_mutex;
}

static GK_S32 mpool_os_mutex_locked(void * p_sem)
{
	if (p_sem == NULL)
		return -1;

	int ret = sem_wait((sem_t *)p_sem);
	if (ret != 0)
		return -1;
	return 0;
}

static void mpool_os_mutex_unlocked(void * p_sem)
{
	if (p_sem == NULL)
		return;

	sem_post((sem_t *)p_sem);
}

static void mpool_os_mutex_destroy(void * ptr)
{
	if (ptr == NULL)
		return;

	sem_destroy((sem_t *)ptr);
	free(ptr);
}

