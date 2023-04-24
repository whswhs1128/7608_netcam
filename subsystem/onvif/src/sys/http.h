/*!
*****************************************************************************
** \file        solution/software/subsystem/onvif-no-gsoap/src/http.h
**
** \brief       http
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2013 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/



#ifndef	__H_HTTP_H__
#define	__H_HTTP_H__

#include "onvif_std.h"
#include "mpool.h"
#include "word_analyse.h"
#include "base64.h"
#include "sha1.h"

#define ONVIF_CLIENTSTREAM_SIZE	4096
#define CLOSESOCKET 	        close

/*http*/
#define HTTP_MSGBUF_NUM                 16
#define HTTP_HDRV_NUM                   16
#define HTTP_NETBUF_SIZE                1024
#define HTTP_NETBUF_NUM                 32

#define HTTP_MAX_CLIENT_NUM             16


//#define BOA_OR_SELF_WEBSERVER



/***************************************************************************************/
typedef struct http_client
{
	GK_S32			cfd;    //new http request create socket
	GK_U32	        rip;    //client IP
	GK_U32	        rport;  //client prot
	GK_CHAR			rcv_buf[HTTP_NETBUF_SIZE];
	GK_CHAR        *dyn_recv_buf;
	GK_S32			rcv_dlen;
	GK_S32			hdr_len;            //http header length
	GK_S32			ctt_len;            //http content length
	GK_CHAR        *p_rbuf;				// --> rcv_buf or dyn_recv_buf
	GK_S32			mlen;				// = sizeof(rcv_buf) or size of dyn_recv_buf
}HTTPCLN;

typedef enum http_request_msg_type
{
	HTTP_MT_NULL = 0,
	HTTP_MT_GET,
	HTTP_MT_HEAD,
	HTTP_MT_MPOST,
	HTTP_MT_MSEARCH,
	HTTP_MT_NOTIFY,
	HTTP_MT_POST,
	HTTP_MT_SUBSCRIBE,
	HTTP_MT_UNSUBSCRIBE,
}HTTP_MT;

/***************************************************************************************/
typedef enum http_content_type
{
	CTT_NULL = 0,
	CTT_SDP,
	CTT_TXT,
	CTT_HTM,
	CTT_XML
}HTTPCTT;

typedef struct header_value
{
	GK_CHAR	header[32];
	GK_CHAR *value_string;
} HDRV;


typedef struct _http_msg_content
{
	GK_U32	        msg_type;
	GK_U32	        msg_sub_type;
	HDRV 			first_line;

	MPOOL_CTX		hdr_ctx;
	MPOOL_CTX		ctt_ctx;

	GK_S32			hdr_len;
	GK_S32			ctt_len;

	HTTPCTT			ctt_type;

	GK_CHAR *		msg_buf;    //http header
	GK_S32			buf_offset;

	GK_ULONG        remote_ip;
	GK_U16	        remote_port;
	GK_U16	        local_port;
}HTTPMSG;

/***************************************************************************************/
typedef struct http_srv_s
{
	GK_S32			r_flag; // 1

	GK_S32			sfd;    //bind socket or web socket

	GK_S32		    sport;  //onvif server prot
	GK_U32	        saddr;  //onvif server addr

	MPOOL_CTX *		cln_fl; //HTTPCLN list
	MPOOL_CTX *		cln_ul; //HTTPCLN stack

	pthread_t		rx_tid; //http thread ID
}HTTPSRV;


typedef struct http_req
{
	GK_S32			cfd;
	GK_CHAR         host[256];
	GK_U32	        port;
	GK_CHAR			url[256];
	GK_CHAR 		action[256];
}HTTPREQ;



/***************************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

GK_BOOL  http_parse_is_http_msg(GK_CHAR * msg_buf);
GK_CHAR* http_parse_get_cttdata(HTTPMSG * msg);
GK_S32 	 http_parse_msg_header(GK_CHAR * p_buf,GK_S32 buf_len,HTTPMSG * msg);
GK_S32 	 http_parse_msg_ctt(GK_CHAR * p_buf,GK_S32 buf_len,HTTPMSG * msg);
GK_CHAR* http_parse_get_headline(HTTPMSG * msg, const GK_CHAR * head);

/***************************************************************************************/

/***************************************************************************************/
void	soap_process(HTTPCLN * p_cln, HTTPMSG * rx_msg);

/***************************************************************************************/
GK_S32 http_server_start(HTTPSRV * p_srv, GK_U32 saddr, GK_U16 sport, GK_S32 cln_num, GK_BOOL isFromWeb,GK_S32 webSocket);
void http_server_deinit(HTTPSRV * p_srv, GK_BOOL isFromWeb);
GK_S32 http_server_soapProc_InWeb(HTTPSRV *p_srv, GK_S32 clientFd, GK_U32 clientIp, GK_U32 clientPort, const GK_CHAR *pszClientStream, GK_U32 clientSize);

/***************************************************************************************/
GK_BOOL http_client_socket_request(HTTPREQ * p_req, GK_S32 timeout, const GK_CHAR* bufs, GK_S32 len);

/***************************************************************************************/
HDRV *  http_bm_get_hdrvbuf();
GK_BOOL http_bm_sysbuf_init(GK_S32 netnum, GK_S32 netsize, GK_S32 hdrvnum);
void    http_bm_sysbuf_deinit();
GK_BOOL http_bm_msgbuf_fl_init(GK_S32 msgnum);
void 	http_bm_msgbuf_fl_deinit();
void 	http_bm_free_httpmsg(HTTPMSG * msg);;
HTTPMSG * http_bm_get_msglargebuf(GK_S32 size);
HTTPMSG * http_bm_get_msgbuf();


#ifdef __cplusplus
}
#endif

#endif	//	__H_HTTP_H__




