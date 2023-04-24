/*!
*****************************************************************************
** \file        solution/software/subsystem/onvif-no-gsoap/src/http_client.c
**
**
** \brief       http client
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
static GK_S32 http_client_socket_init(GK_U32 rip, GK_S32 port, GK_S32 timeout)
{
	GK_S32 s32cfd = socket(AF_INET, SOCK_STREAM, 0);
	if (s32cfd < 0)
	{
	    perror("[http_client_connect_timeout]socket failed\n");
		return -1;
    }

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = rip;
	addr.sin_port = htons((unsigned short)port);

	struct timeval tv;
	tv.tv_sec = timeout/1000;
	tv.tv_usec = (timeout%1000) * 1000;

	setsockopt(s32cfd, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, sizeof(tv));
	if (connect(s32cfd, (struct sockaddr *)&addr, sizeof(addr)) == 0)
	{
	    return s32cfd;
	}
	else
	{
	    perror("[http_client_connect_timeout]connect failed!\n");
	    CLOSESOCKET(s32cfd);
	    return -1;
	}
}



static GK_BOOL http_client_soap_tx(HTTPREQ * p_req, const GK_CHAR * p_data, GK_S32 len)
{
	if (p_req->cfd <= 0)
		return GK_FALSE;

	GK_S32 slen = send(p_req->cfd, p_data, len, 0);
	if (slen != len)
	{
	    ONVIF_ERR("[http_client_soap_tx]send len = %d, len = %d\n", slen, len);
		return GK_FALSE;
    }

	return GK_TRUE;
}

static GK_BOOL http_client_soap_req(HTTPREQ * p_req, const GK_CHAR * p_xml, GK_S32 len)
{
	GK_S32 offset = 0;
	GK_CHAR aszbufs[1024 * 4];//4K
	if (len > 3072)//3K
		return GK_FALSE;
    memset(aszbufs, 0, 1024 * 4);
	offset += sprintf(aszbufs+offset, "POST %s HTTP/1.1\r\n", p_req->url);
	offset += sprintf(aszbufs+offset, "Host: %s:%d\r\n", p_req->host, p_req->port);
	offset += sprintf(aszbufs+offset, "Content-Type: application/soap+xml; charset=utf-8; action=\"%s\"\r\n", p_req->action);
	offset += sprintf(aszbufs+offset, "Content-Length: %d\r\n", len);
	offset += sprintf(aszbufs+offset, "Connection: close\r\n\r\n");

	memcpy(aszbufs+offset, p_xml, len);
	offset += len;
	aszbufs[offset] = '\0';

	return http_client_soap_tx(p_req, aszbufs, offset);
}


GK_BOOL http_client_socket_request(HTTPREQ * p_req, GK_S32 timeout, const GK_CHAR* bufs, GK_S32 len)
{
	p_req->cfd = http_client_socket_init(inet_addr(p_req->host), p_req->port, timeout);
	if (p_req->cfd <= 0)
	{
	    ONVIF_ERR("[http_client_socket_request]socket init failed!!!!\n");
		return GK_FALSE;
    }

	http_client_soap_req(p_req, bufs, len);

	CLOSESOCKET(p_req->cfd);

	return GK_TRUE;
}

