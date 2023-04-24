/*!
*****************************************************************************
** \file        solution/software/subsystem/onvif-no-gsoap/src/http_bm.c
**
**
** \brief       http buffer manager
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2013 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#include "http.h"


/***************************************************************************************/
static MPOOL_CTX * http_netbuf_freelist = NULL;
static MPOOL_CTX * http_hdrvbuf_freelist = NULL;
static MPOOL_CTX * http_msgbuf_freelist = NULL;


/***************************************************************************************/
static GK_BOOL bm_netbuf_fl_init(GK_S32 num, GK_S32 size)
{
	http_netbuf_freelist = mpool_ctx_freelist_init(num,size,GK_TRUE);
	if (http_netbuf_freelist == NULL)
		return GK_FALSE;
	return GK_TRUE;
}

static GK_BOOL bm_hdrvbuf_fl_init(GK_S32 num)
{
	http_hdrvbuf_freelist = mpool_ctx_freelist_init(num, sizeof(HDRV), GK_TRUE);
	if (http_hdrvbuf_freelist == NULL)
		return GK_FALSE;
	return GK_TRUE;
}

static void bm_netbuf_fl_deinit()
{
	if (http_netbuf_freelist)
	{
		mpool_freelist_free(http_netbuf_freelist);
		http_netbuf_freelist = NULL;
	}
}

static void bm_hdrvbuf_fl_deinit()
{
	if (http_hdrvbuf_freelist)
	{
		mpool_freelist_free(http_hdrvbuf_freelist);
		http_hdrvbuf_freelist = NULL;
	}
}


static GK_CHAR * bm_get_idle_netbuf()
{
	return (char *)mpool_freelist_pop(http_netbuf_freelist);
}
static void bm_http_msg_ctx_init(HTTPMSG * msg)
{
	mpool_ctx_uselist_init_nm(http_hdrvbuf_freelist,&(msg->hdr_ctx));//http header
	mpool_ctx_uselist_init_nm(http_hdrvbuf_freelist,&(msg->ctt_ctx));//http ctt
}

static void bm_free_netbuf(GK_CHAR * rbuf)
{
	if (rbuf == NULL)
		return;

	if (mpool_safe_node(http_netbuf_freelist, rbuf))
		mpool_freelist_push_tail(http_netbuf_freelist,rbuf);
	else
		free(rbuf);
}

static void bm_free_hdrvbuf(HDRV * pHdrv)
{
	if (pHdrv == NULL)
		return;

	pHdrv->header[0] = '\0';
	pHdrv->value_string = NULL;
	mpool_freelist_push(http_hdrvbuf_freelist,pHdrv);
}

void bm_free_ctx_hdrv(MPOOL_CTX * p_ctx)
{
	if (p_ctx == NULL)
		return;

	HDRV * p_free = (HDRV *)mpool_lookup_start(p_ctx);
	while (p_free != NULL)
	{
		HDRV * p_next = (HDRV *)mpool_lookup_next(p_ctx,p_free);
		mpool_ctx_uselist_del(p_ctx,p_free);
		bm_free_hdrvbuf(p_free);

		p_free = p_next;
	}
	mpool_lookup_end(p_ctx);
}

static void bm_free_http_msgbuf(HTTPMSG * msg)
{
	mpool_freelist_push(http_msgbuf_freelist,msg);
}

static void bm_free_http_msgctx(HTTPMSG * msg,GK_S32 type)	//0:sip list; 1:sdp list;
{
	MPOOL_CTX * p_free_ctx = NULL;

	switch(type)
	{
	case 0:
		p_free_ctx = &(msg->hdr_ctx);
		break;

	case 1:
		p_free_ctx = &(msg->ctt_ctx);
		break;
	}

	if(p_free_ctx == NULL)
		return;

	bm_free_ctx_hdrv(p_free_ctx);
}

static void bm_free_http_msgcontent(HTTPMSG * msg)
{
	if(msg == NULL)	return;

	bm_free_http_msgctx(msg,0);//http header
	bm_free_http_msgctx(msg,1);//http ctt
	bm_free_netbuf(msg->msg_buf);
}


GK_BOOL http_bm_sysbuf_init(GK_S32 netnum, GK_S32 netsize, GK_S32 hdrvnum)
{
	if (bm_netbuf_fl_init(netnum, netsize) == GK_FALSE)
	{
		return GK_FALSE;
	}

	if (bm_hdrvbuf_fl_init(hdrvnum) == GK_FALSE)
	{
		return GK_FALSE;
	}

	return GK_TRUE;
}

void http_bm_sysbuf_deinit()
{
	bm_netbuf_fl_deinit();
	bm_hdrvbuf_fl_deinit();
}

GK_BOOL http_bm_msgbuf_fl_init(GK_S32 msgnum)
{
	http_msgbuf_freelist = mpool_ctx_freelist_init(msgnum,sizeof(HTTPMSG),GK_TRUE);
	if(http_msgbuf_freelist == NULL)
		return GK_FALSE;

	return GK_TRUE;
}

void http_bm_msgbuf_fl_deinit()
{
	if(http_msgbuf_freelist)
	{
		mpool_freelist_free(http_msgbuf_freelist);
		http_msgbuf_freelist = NULL;
	}
}


HDRV * http_bm_get_hdrvbuf()
{
	HDRV * p_hdrv = (HDRV *)mpool_freelist_pop(http_hdrvbuf_freelist);
	return p_hdrv;
}


HTTPMSG * http_bm_get_msgbuf()
{
	HTTPMSG * tx_msg = (HTTPMSG *)mpool_freelist_pop(http_msgbuf_freelist);
	if(tx_msg == NULL)
	{
		return NULL;
	}

	memset(tx_msg,0,sizeof(HTTPMSG));
	tx_msg->msg_buf = bm_get_idle_netbuf();
	if(tx_msg->msg_buf == NULL)
	{
		bm_free_http_msgbuf(tx_msg);
		return NULL;
	}

	bm_http_msg_ctx_init(tx_msg);

	return tx_msg;
}


HTTPMSG * http_bm_get_msglargebuf(int size)
{
	HTTPMSG * tx_msg = (HTTPMSG *)mpool_freelist_pop(http_msgbuf_freelist);
	if(tx_msg == NULL)
	{
		return NULL;
	}

	memset(tx_msg,0,sizeof(HTTPMSG));
	tx_msg->msg_buf = (char *)malloc(size);
	if(tx_msg->msg_buf == NULL)
	{
		 bm_free_http_msgbuf(tx_msg);
		return NULL;
	}

	bm_http_msg_ctx_init(tx_msg);

	return tx_msg;
}


void http_bm_free_httpmsg(HTTPMSG * msg)
{
	if(msg == NULL)return;

	bm_free_http_msgcontent(msg);
	bm_free_http_msgbuf(msg);
}

