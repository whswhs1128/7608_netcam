/*!
*****************************************************************************
** \file        solution/software/subsystem/onvif-no-gsoap/src/http_parse.c
**
**
** \brief       http parse
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

typedef struct request_message_type_value
{
	HTTP_MT	msg_type;
	GK_CHAR	msg_str[32];
	GK_S32	msg_len;
}REQMTV;



static const REQMTV req_mtvs[]={
	{HTTP_MT_GET,			"GET",			3},
	{HTTP_MT_HEAD,			"HEAD",			4},
	{HTTP_MT_MPOST,			"M-POST",		6},
	{HTTP_MT_MSEARCH,		"M-SEARCH",		8},
	{HTTP_MT_NOTIFY,		"NOTIFY",		6},
	{HTTP_MT_POST,			"POST",			4},
	{HTTP_MT_SUBSCRIBE,		"SUBSCRIBE",	9},
	{HTTP_MT_UNSUBSCRIBE,	"UNSUBSCRIBE",	11}
};
static void http_parse_headerline(GK_CHAR * pline, GK_S32 llen, HTTPMSG * p_msg)
{
	GK_CHAR	word_buf[256];
	GK_S32	word_len;
	GK_S32	next_word_offset;
	GK_BOOL	bHaveNextWord;

	bHaveNextWord = GetLineWord(pline,0,llen,word_buf,
		sizeof(word_buf),&next_word_offset,WORD_TYPE_STRING);
	word_len = strlen(word_buf);
	if(word_len > 0 && word_len < 31)
	{
		memcpy(p_msg->first_line.header,pline,word_len);
		p_msg->first_line.header[word_len] = '\0';

		while(pline[next_word_offset] == ' ')next_word_offset++;

		p_msg->first_line.value_string = pline+next_word_offset;
		if(strcasecmp(word_buf,"HTTP/1.1") == 0)
		{
			if(bHaveNextWord)
			{
				word_len = sizeof(word_buf);
				bHaveNextWord = GetLineWord(pline,next_word_offset,llen,
											word_buf,sizeof(word_buf),&next_word_offset,WORD_TYPE_NUM);
				word_len = strlen(word_buf);
				if(word_len > 0)
				{
					p_msg->msg_type = 1;
					p_msg->msg_sub_type = atoi(word_buf);
				}
			}
		}
		else
		{
			p_msg->msg_type = 0;
			GK_U32 i;
			for(i=0; i<sizeof(req_mtvs)/sizeof(REQMTV);i++)
			{
				if(strcasecmp(word_buf,(char *)(req_mtvs[i].msg_str)) == 0)
				{
					p_msg->msg_sub_type = req_mtvs[i].msg_type;
					break;
				}
			}
		}
	}
}

static GK_S32 http_parse_line(GK_CHAR * p_buf, GK_S32 max_len, GK_CHAR sep_char, MPOOL_CTX * p_ctx)
{
	GK_CHAR word_buf[256];
	GK_BOOL bHaveNextLine = GK_TRUE;
	GK_S32 line_len = 0;
	GK_S32 parse_len = 0;

	GK_CHAR * ptr = p_buf;

	do{
		if(GetSipLine(ptr, max_len, &line_len, &bHaveNextLine) == GK_FALSE)
		{
			ONVIF_ERR("http_parse_line::get sip line error!!!\r\n");
			return -1;
		}

		if(line_len == 2)
		{
			return(parse_len + 2);
		}

		GK_S32	next_word_offset = 0;
		GetLineWord(ptr,0,line_len-2,word_buf,sizeof(word_buf),&next_word_offset,WORD_TYPE_STRING);
		GK_CHAR nchar = *(ptr + next_word_offset);
		if(nchar != sep_char)
		{
			ONVIF_ERR("http_parse_line::format error!!!\r\n");
			return -1;
		}

		next_word_offset++;
		while(ptr[next_word_offset] == ' ') next_word_offset++;

		HDRV * pHdrV = http_bm_get_hdrvbuf();
		if(pHdrV == NULL)
		{
			ONVIF_ERR("http_parse_line::http_bm_get_hdrvbuf return NULL!!!\r\n");
			return -1;
		}

		strncpy(pHdrV->header,word_buf,32);
		pHdrV->value_string = ptr+next_word_offset;
   		mpool_ctx_uselist_add(p_ctx, pHdrV);

		ptr += line_len;
		max_len -= line_len;
		parse_len += line_len;

	}while(bHaveNextLine);

	return parse_len;
}

static GK_S32 http_parse_get_cttinfo(HTTPMSG * p_msg)
{
	GK_S32 flag = 0;
	HTTPCTT w_ctx_type;

	HDRV * pHdrV = (HDRV *)mpool_lookup_start(&(p_msg->hdr_ctx));
	while (pHdrV != NULL)
	{
		if(strcasecmp(pHdrV->header,"Content-Length") == 0)
		{
			p_msg->ctt_len = atol(pHdrV->value_string);
			flag++;
		}
		else if(strcasecmp(pHdrV->header,"Content-Type") == 0)
		{
			char type_word[64];
			int  next_tmp;
			GetLineWord(pHdrV->value_string,0,strlen(pHdrV->value_string),type_word,sizeof(type_word),&next_tmp,WORD_TYPE_STRING);

			if(strcasecmp(type_word,"application/sdp") == 0)
				w_ctx_type = CTT_SDP;
			else if(strcasecmp(type_word,"application/soap+xml") == 0)
				w_ctx_type = CTT_XML;
			else if(strcasecmp(type_word,"text/plain") == 0)
				w_ctx_type = CTT_TXT;
			else if(strcasecmp(type_word,"text/html") == 0)
				w_ctx_type = CTT_HTM;
			else
				w_ctx_type = CTT_NULL;

			p_msg->ctt_type = w_ctx_type;
			flag++;
		}
		pHdrV = (HDRV *)mpool_lookup_next(&(p_msg->hdr_ctx),pHdrV);
	}
	mpool_lookup_end(&(p_msg->hdr_ctx));

	if(p_msg->ctt_type && p_msg->ctt_len)
		return 1;

	return 0;
}

static HDRV * http_parse_find_headerline(HTTPMSG * msg, const GK_CHAR * head)
{
	if(msg == NULL || head == NULL)
		return NULL;

	HDRV * line = (HDRV *)mpool_lookup_start(&(msg->hdr_ctx));
	while (line != NULL)
	{
		if(strcasecmp(line->header,head) == 0)
		{
			mpool_lookup_end(&(msg->hdr_ctx));
			return line;
		}

		line = (HDRV *)mpool_lookup_next(&(msg->hdr_ctx),line);
	}
	mpool_lookup_end(&(msg->hdr_ctx));

	return NULL;
}

GK_BOOL http_parse_is_http_msg(GK_CHAR * msg_buf)
{
	GK_U32 i;
	for(i=0; i<sizeof(req_mtvs)/sizeof(REQMTV);i++)
	{
		if(memcmp(msg_buf, req_mtvs[i].msg_str, req_mtvs[i].msg_len) == 0)
		{
			return GK_TRUE;
		}
	}

	if(memcmp(msg_buf,"HTTP/1.1",strlen("HTTP/1.1")) == 0)
		return GK_TRUE;

	return GK_FALSE;
}


GK_S32 http_parse_msg_header(GK_CHAR * p_buf,int buf_len,HTTPMSG * msg)
{
	GK_BOOL bHaveNextLine;
	GK_S32 line_len = 0;

	msg->msg_type = -1;

	if(GetSipLine(p_buf, buf_len,&line_len,&bHaveNextLine) == GK_FALSE)
		return -1;
	if(line_len > 0)
		http_parse_headerline(p_buf, line_len-2, msg);
	if(msg->msg_type == -1)
		return -1;

	p_buf += line_len;
	msg->hdr_len = http_parse_line(p_buf, buf_len-line_len, ':', &(msg->hdr_ctx));
	if(msg->hdr_len <= 0)
		return -1;

	http_parse_get_cttinfo(msg);

	return (line_len + msg->hdr_len);
}

GK_S32 http_parse_msg_ctt(GK_CHAR * p_buf,GK_S32 buf_len,HTTPMSG * msg)
{
	HDRV * pHdrV = http_bm_get_hdrvbuf();
	if(pHdrV == NULL)
	{
		ONVIF_ERR("http_parse_msg_ctt::http_bm_get_hdrvbuf return NULL!!!\r\n");
		return -1;
	}

	strcpy(pHdrV->header,"");
	pHdrV->value_string = p_buf;
	mpool_ctx_uselist_add(&(msg->ctt_ctx), pHdrV);
	int slen = strlen(p_buf);
	if(slen != msg->ctt_len)
	{
		ONVIF_ERR("http_parse_msg_ctt::text xml strlen[%d] != ctx len[%d]!!!\r\n", slen, msg->ctt_len);
	}

	return slen;
}



GK_CHAR * http_parse_get_headline(HTTPMSG * msg, const GK_CHAR * head)
{
	HDRV * p_hdrv = http_parse_find_headerline(msg, head);
	if(p_hdrv == NULL)
		return NULL;
	return p_hdrv->value_string;
}

GK_CHAR * http_parse_get_cttdata(HTTPMSG * msg)
{
	if(msg == NULL)
		return NULL;

	HDRV * line = (HDRV *)mpool_lookup_start(&(msg->ctt_ctx));
	mpool_lookup_end(&(msg->ctt_ctx));
	if(line)
		return line->value_string;

	return NULL;
}
/***********************************************************************/



