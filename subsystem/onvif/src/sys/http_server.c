/*!
*****************************************************************************
** \file        solution/software/subsystem/onvif-no-gsoap/src/http_server.c
**
**
** \brief       http server
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
//#include "sdk_sys.h"

static GK_S32 http_server_hdr_find_end(GK_CHAR * p_buf);
static void http_server_rx_msg(HTTPCLN * p_cln, HTTPMSG * rx_msg);
static GK_BOOL http_server_tcp_rx(HTTPCLN * p_cln);
static GK_S32 http_server_tcp_listen_rx(HTTPSRV * p_srv);
static void * http_server_rx_thread(void * argv);
static GK_S32 http_server_net_init(HTTPSRV * p_srv);
static HTTPCLN * http_server_get_idle_cln(HTTPSRV * p_srv);
static void http_server_free_used_cln(HTTPSRV * p_srv, HTTPCLN * p_cln);

GK_S32 http_server_start(HTTPSRV *p_srv, GK_U32 saddr, GK_U16 sport, GK_S32 cln_num, GK_BOOL isFromWeb,GK_S32 webSocket)
{
	memset(p_srv, 0, sizeof(HTTPSRV));

	p_srv->saddr = saddr;
	p_srv->sport = sport;

	p_srv->cln_fl = mpool_ctx_freelist_init(cln_num, sizeof(HTTPCLN), GK_TRUE); //malloc for p_srv->cln_fl 
	if(p_srv->cln_fl == NULL)
		return -1;

	p_srv->cln_ul = mpool_ctx_uselist_init(p_srv->cln_fl, GK_TRUE);//malloc header for p_srv->cln_ul
	if(p_srv->cln_ul == NULL)
		return -1;
		
    if(isFromWeb == GK_FALSE)
    {
        if(http_server_net_init(p_srv) != 0)
            return -1;
    }
    else
    {
        p_srv->sfd = webSocket;
    }

	p_srv->r_flag = 1;


    if(isFromWeb == GK_FALSE)
    {
        pthread_attr_t attr;	
        if(pthread_attr_init(&attr) < 0)
        {
            ONVIF_ERR("HTTP server error.\n");
            return -1;
        }
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        GK_S32 ret = pthread_create(&p_srv->rx_tid, &attr, http_server_rx_thread, p_srv);
        if (ret != 0)
        {
            ONVIF_ERR("pthread_create failed!!!");
            return -1;
        }
    }
    return 0;
}

void http_server_deinit(HTTPSRV * p_srv, GK_BOOL isFromWeb)
{
	p_srv->r_flag = 0;
    if(isFromWeb == GK_FALSE)
    {
        while(p_srv->rx_tid != 0)
        {
            usleep(1000);
        }
    }
	mpool_uselist_free(p_srv->cln_ul);
	mpool_freelist_free(p_srv->cln_fl);
    if(isFromWeb == GK_FALSE)
    {
        CLOSESOCKET(p_srv->sfd);
        p_srv->sfd = -1;
    }
}

static GK_S32 http_server_hdr_find_end(GK_CHAR * p_buf)
{
	GK_S32 end_off = 0;
	GK_S32 http_pkt_finish = 0;
	while(p_buf[end_off] != '\0')
	{
		if((p_buf[end_off] == '\r' && p_buf[end_off+1] == '\n') &&
			(p_buf[end_off+2] == '\r' && p_buf[end_off+3] == '\n'))
		{
			http_pkt_finish = 1;
			break;
		}

		end_off++;
	}
	if(http_pkt_finish)
		return(end_off + 4);
	return 0;
}

static void http_server_rx_msg(HTTPCLN * p_cln, HTTPMSG * rx_msg)
{
	soap_process(p_cln, rx_msg);
#ifdef BOA_OR_SELF_WEBSERVER
	CLOSESOCKET(p_cln->cfd);
	p_cln->cfd = 0;
#endif
}

static GK_BOOL http_server_tcp_rx(HTTPCLN * p_cln)
{
	if(p_cln->p_rbuf == NULL)
	{
		p_cln->p_rbuf = p_cln->rcv_buf;
		p_cln->mlen = sizeof(p_cln->rcv_buf)-1;
		p_cln->rcv_dlen = 0;
		p_cln->ctt_len = 0;
		p_cln->hdr_len = 0;
	}
	
	GK_S32 rlen = recv(p_cln->cfd, p_cln->p_rbuf+p_cln->rcv_dlen, p_cln->mlen-p_cln->rcv_dlen, 0);
	if(rlen <= 0)
	{
		CLOSESOCKET(p_cln->cfd);
		p_cln->cfd = 0;
		return GK_FALSE;
	}

	p_cln->rcv_dlen += rlen;
	p_cln->p_rbuf[p_cln->rcv_dlen] = '\0';

rx_analyse_point:

	if(p_cln->rcv_dlen < 16)
		return GK_TRUE;

	if(http_parse_is_http_msg(p_cln->p_rbuf) == GK_FALSE)
		return GK_FALSE;

	HTTPMSG *rx_msg = NULL;

	if(p_cln->hdr_len == 0)
	{
		GK_S32 http_pkt_len = http_server_hdr_find_end(p_cln->p_rbuf);
		if(http_pkt_len == 0)
		{
			return GK_TRUE;
		}
		p_cln->hdr_len = http_pkt_len;

		rx_msg = http_bm_get_msgbuf();
		if(rx_msg == NULL)
		{
			ONVIF_ERR("server_tcp_rx::get_msg_buf ret null!!!\r\n");
			return GK_FALSE;
		}

		memcpy(rx_msg->msg_buf, p_cln->p_rbuf, http_pkt_len);
		rx_msg->msg_buf[http_pkt_len] = '\0';

		int parse_len = http_parse_msg_header(rx_msg->msg_buf, http_pkt_len, rx_msg);
		if(parse_len != http_pkt_len)
		{
			ONVIF_ERR("server_tcp_rx::parse_len=%d, sip_pkt_len=%d!!!\r\n",parse_len,http_pkt_len);
			http_bm_free_httpmsg(rx_msg);
			return GK_FALSE;
		}
		p_cln->ctt_len = rx_msg->ctt_len;
	}
	if((p_cln->ctt_len + p_cln->hdr_len) > p_cln->mlen)
	{
		if(p_cln->dyn_recv_buf)
		{
			ONVIF_ERR("server_tcp_rx::dyn_recv_buf=%p, mlen=%d!\r\n",p_cln->dyn_recv_buf, p_cln->mlen);
			free(p_cln->dyn_recv_buf);
		}

		p_cln->dyn_recv_buf = (GK_CHAR *)malloc(p_cln->ctt_len + p_cln->hdr_len + 1);
		memcpy(p_cln->dyn_recv_buf, p_cln->rcv_buf, p_cln->rcv_dlen);
		p_cln->p_rbuf = p_cln->dyn_recv_buf;
		p_cln->mlen = p_cln->ctt_len + p_cln->hdr_len;

		if(rx_msg) http_bm_free_httpmsg(rx_msg);
		return GK_TRUE;
	}
	if(p_cln->rcv_dlen >= (p_cln->ctt_len + p_cln->hdr_len))
	{
		if(rx_msg == NULL)
		{
			GK_S32 nlen = p_cln->ctt_len + p_cln->hdr_len;
			if(nlen > HTTP_NETBUF_SIZE)//2K
			{
				rx_msg = http_bm_get_msglargebuf(nlen+1);//here may has memery leak.
				if(rx_msg == NULL)
					return GK_FALSE;
			}
			else
			{
				rx_msg = http_bm_get_msgbuf();
				if(rx_msg == NULL)
					return GK_FALSE;
			}

			memcpy(rx_msg->msg_buf,p_cln->p_rbuf,p_cln->hdr_len);
			rx_msg->msg_buf[p_cln->hdr_len] = '\0';
			GK_S32 parse_len = http_parse_msg_header(rx_msg->msg_buf,p_cln->hdr_len,rx_msg);
			if(parse_len != p_cln->hdr_len)
			{
				ONVIF_ERR("server_tcp_rx::parse_len=%d, sip_pkt_len=%d!!!\r\n",parse_len,p_cln->hdr_len);
				http_bm_free_httpmsg(rx_msg);
				return GK_FALSE;
			}

		}

		if(p_cln->ctt_len > 0)
		{
			memcpy(rx_msg->msg_buf+p_cln->hdr_len, p_cln->p_rbuf+p_cln->hdr_len, p_cln->ctt_len);
			rx_msg->msg_buf[p_cln->hdr_len + p_cln->ctt_len] = '\0';
			GK_S32 parse_len = http_parse_msg_ctt(rx_msg->msg_buf+p_cln->hdr_len,p_cln->ctt_len,rx_msg);
			if(parse_len != p_cln->ctt_len)
			{
				ONVIF_ERR("server_tcp_rx::parse_len=%d, sdp_pkt_len=%d!!!\r\n",parse_len,p_cln->ctt_len);
				http_bm_free_httpmsg(rx_msg);
				return GK_FALSE;
			}
		}

		http_server_rx_msg(p_cln, rx_msg);
		
		p_cln->rcv_dlen -= p_cln->hdr_len + p_cln->ctt_len;

		if(p_cln->dyn_recv_buf == NULL)
		{
			if(p_cln->rcv_dlen > 0)
			{
				memmove(p_cln->rcv_buf, p_cln->rcv_buf+p_cln->hdr_len + p_cln->ctt_len, p_cln->rcv_dlen);
				p_cln->rcv_buf[p_cln->rcv_dlen] = '\0';
			}
			p_cln->p_rbuf = p_cln->rcv_buf;
			p_cln->mlen = sizeof(p_cln->rcv_buf)-1;
			p_cln->hdr_len = 0;
			p_cln->ctt_len = 0;

			if(p_cln->rcv_dlen > 16)
				goto rx_analyse_point;
		}
		else
		{
			free(p_cln->dyn_recv_buf);
			p_cln->dyn_recv_buf = NULL;
			p_cln->hdr_len = 0;
			p_cln->ctt_len = 0;
			p_cln->p_rbuf = 0;
			p_cln->rcv_dlen = 0;
		}
	}

    if(rx_msg)
        http_bm_free_httpmsg(rx_msg);

    return GK_TRUE;
}

static GK_S32 http_server_tcp_listen_rx(HTTPSRV * p_srv)
{
	struct sockaddr_in caddr;
	socklen_t size = sizeof(struct sockaddr_in);
	GK_S32 cfd = accept(p_srv->sfd, (struct sockaddr *)&caddr, &size);	//new http request
	if(cfd < 0)
	{
		perror("accept");
		return -1;
	}

	HTTPCLN *p_cln = http_server_get_idle_cln(p_srv);   //allow new HTTPCLN buffer
	if(p_cln == NULL)
	{
		ONVIF_ERR("http_server_get_idle_cln::ret null!!!\r\n");
		CLOSESOCKET(cfd);
		return -1;
	}

	p_cln->cfd = cfd;
	p_cln->rip = caddr.sin_addr.s_addr;
	p_cln->rport = ntohs(caddr.sin_port);
	mpool_ctx_uselist_add(p_srv->cln_ul, p_cln);

	ONVIF_DBG("\nlisten client[0x%08x,%u]\r\n", p_cln->rip, p_cln->rport);

	return 0;
}

static void *http_server_rx_thread(void *argv)
{
    sdk_sys_thread_set_name("http_server_rx_thread");
	fd_set fdr;
	HTTPSRV * p_srv = (HTTPSRV *)argv;
	if(p_srv == NULL)
		return NULL;
		
	while(p_srv->r_flag == 1)
	{
		FD_ZERO(&fdr);
		GK_S32 max_fd = p_srv->sfd;
		FD_SET(p_srv->sfd, &fdr);
		HTTPCLN *p_cln = (HTTPCLN *)mpool_lookup_start(p_srv->cln_ul);
		while(p_cln)
		{
			if(p_cln->cfd > 0)
			{
				FD_SET(p_cln->cfd, &fdr);
				max_fd = (p_cln->cfd > max_fd)? p_cln->cfd : max_fd;
			}
			else
			{
				HTTPCLN * p_next_cln = (HTTPCLN *)mpool_ctx_uselist_del_unlock(p_srv->cln_ul, p_cln);
                http_server_free_used_cln(p_srv, p_cln);
                p_cln = p_next_cln;
                continue;
			}

			p_cln = (HTTPCLN *)mpool_lookup_next(p_srv->cln_ul, p_cln);
		}
        mpool_lookup_end(p_srv->cln_ul);

		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 100 * 1000; //100ms
		int sret = select(max_fd+1, &fdr, NULL, NULL, &tv);
		if(sret == 0)
		{
			continue;
		}
		else if(sret < 0)
		{
			break;
		}
		if(FD_ISSET(p_srv->sfd, &fdr))
		{
			http_server_tcp_listen_rx(p_srv);
		}		

		p_cln = (HTTPCLN *)mpool_lookup_start(p_srv->cln_ul);
		while(p_cln)
		{
			if(p_cln->cfd > 0 && FD_ISSET(p_cln->cfd, &fdr))
			{
				if(http_server_tcp_rx(p_cln) == GK_FALSE)
				{

					HTTPCLN *p_next_cln = (HTTPCLN *)mpool_ctx_uselist_del_unlock(p_srv->cln_ul, p_cln);    //pop node which had been handed in p_srv->cln_ul

                    http_server_free_used_cln(p_srv, p_cln);    //push node which had been handed to p_srv->cln_fl

					p_cln = p_next_cln;
					continue;
				}
			}
			else if(p_cln->cfd == 0)
			{

			    HTTPCLN * p_next_cln = (HTTPCLN *)mpool_ctx_uselist_del_unlock(p_srv->cln_ul, p_cln);

                http_server_free_used_cln(p_srv, p_cln);

			    p_cln = p_next_cln;
			    continue;
			}

			p_cln = (HTTPCLN *)mpool_lookup_next(p_srv->cln_ul, p_cln); //find next node in p_srv->cln_ul
		}
		mpool_lookup_end(p_srv->cln_ul);
	}

	p_srv->rx_tid = 0;
	return NULL;
}

static GK_S32 http_server_net_init(HTTPSRV * p_srv)
{
	p_srv->sfd = socket(AF_INET,SOCK_STREAM,0);
	if(p_srv->sfd < 0)
	{
		ONVIF_ERR("http_server_net_init::socket failed!!!\r\n");
		return -1;
	}

	GK_S32 val = 1;
	setsockopt(p_srv->sfd, SOL_SOCKET, SO_REUSEADDR, (char *)&val, 4);

	struct sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = p_srv->saddr;
	addr.sin_port = htons(p_srv->sport);

	if(bind(p_srv->sfd,(struct sockaddr *)&addr,sizeof(addr)) == -1)
	{
		ONVIF_ERR("http_server_net_init::bind tcp socket faild!!!\n");
		CLOSESOCKET(p_srv->sfd);
		p_srv->sfd = -1;
		return -1;
	}

	if(listen(p_srv->sfd, 10) < 0)
	{
		ONVIF_ERR("http_server_net_init::listen tcp socket faild!!!\n");
		CLOSESOCKET(p_srv->sfd);
		return -1;
	}

	return 0;
}


static HTTPCLN *http_server_get_idle_cln(HTTPSRV * p_srv)
{
	HTTPCLN *p_cln = (HTTPCLN *)mpool_freelist_pop(p_srv->cln_fl);
	if(p_cln)
	{
		memset(p_cln, 0, sizeof(HTTPCLN));
	}

	return p_cln;
}

static void http_server_free_used_cln(HTTPSRV *p_srv, HTTPCLN *p_cln)
{
	if(p_cln->dyn_recv_buf)
		free(p_cln->dyn_recv_buf);
	
#ifdef BOA_OR_SELF_WEBSERVER
	if(p_cln->cfd > 0)
	{
		CLOSESOCKET(p_cln->cfd);
		p_cln->cfd = 0;
	}
#endif
	mpool_freelist_push_tail(p_srv->cln_fl, p_cln);
}

static GK_CHAR aszClientStream[ONVIF_CLIENTSTREAM_SIZE];

GK_S32 http_server_soapProc_InWeb(HTTPSRV *p_srv, GK_S32 clientFd, GK_U32 clientIp, GK_U32 clientPort, const GK_CHAR *pszClientStream, GK_U32 clientSize)
{
	if(pszClientStream == NULL || clientSize > 4096)
	{
		ONVIF_ERR("Invalid request soap message.");
		return -1;
	}
    GK_S32 rlen = 0;
    GK_S32 readlen = 0;
    GK_CHAR *pszClient = NULL;
    HTTPCLN *p_cln = http_server_get_idle_cln(p_srv);	// get idle HTTPCLN buffer
    if(p_cln == NULL)
    {
        ONVIF_ERR("ret null!\n");
#ifdef BOA_OR_SELF_WEBSERVER
        CLOSESOCKET(clientFd);
#endif
        return -1;
    }
    //ret = mpool_ctx_uselist_add(p_srv->cln_ul, p_cln);	// add idle HTTPCLN buffer to uselist
    //if(ret == GK_FALSE)
    //{
		//ONVIF_ERR("Fail to add to uselist.");
		//return -1;
    //}
#ifdef BOA_OR_SELF_WEBSERVER
    p_cln->cfd = clientFd;
    p_cln->rip = clientIp;
    p_cln->rport = clientPort;
#endif
    memset(aszClientStream, 0, sizeof(aszClientStream));
    memcpy(aszClientStream, pszClientStream, clientSize);
    pszClient = aszClientStream;
    while(readlen < clientSize)
    {
        if(p_cln->p_rbuf == NULL)
        {
            p_cln->p_rbuf = p_cln->rcv_buf;
            p_cln->mlen = sizeof(p_cln->rcv_buf)-1;
            p_cln->rcv_dlen = 0;
            p_cln->ctt_len = 0;
            p_cln->hdr_len = 0;
        }
        if(clientSize - p_cln->rcv_dlen > HTTP_NETBUF_SIZE)
        {
            memcpy(p_cln->p_rbuf+p_cln->rcv_dlen, pszClient, HTTP_NETBUF_SIZE);
            rlen = HTTP_NETBUF_SIZE;
        }
        else
        {
            memcpy(p_cln->p_rbuf+p_cln->rcv_dlen, pszClient, clientSize - readlen);
            rlen = clientSize - p_cln->rcv_dlen;
        }
        pszClient += rlen;
        readlen +=  rlen;
        p_cln->rcv_dlen += rlen;
        p_cln->p_rbuf[p_cln->rcv_dlen] = '\0';

    rx_analyse_point:

        if(p_cln->rcv_dlen < 16)
            continue;

        if(http_parse_is_http_msg(p_cln->p_rbuf) == GK_FALSE)
            break;

        HTTPMSG * rx_msg = NULL;

        if(p_cln->hdr_len == 0)
        {
            GK_S32 http_pkt_len = http_server_hdr_find_end(p_cln->p_rbuf);
            if(http_pkt_len == 0)
            {
                continue;
            }
            p_cln->hdr_len = http_pkt_len;

            rx_msg = http_bm_get_msgbuf();
            if(rx_msg == NULL)
            {
                ONVIF_ERR("server_tcp_rx::get_msg_buf ret null!!!\r\n");
                break;
            }

            memcpy(rx_msg->msg_buf,p_cln->p_rbuf,http_pkt_len);
            rx_msg->msg_buf[http_pkt_len] = '\0';

            int parse_len = http_parse_msg_header(rx_msg->msg_buf,http_pkt_len,rx_msg);
            if(parse_len != http_pkt_len)
            {
                ONVIF_ERR("server_tcp_rx::parse_len=%d, sip_pkt_len=%d!!!\r\n",parse_len,http_pkt_len);
                http_bm_free_httpmsg(rx_msg);
                break;
            }
            p_cln->ctt_len = rx_msg->ctt_len;
        }
        if((p_cln->ctt_len + p_cln->hdr_len) > p_cln->mlen)
        {
            if(p_cln->dyn_recv_buf)
            {
                ONVIF_ERR("server_tcp_rx::dyn_recv_buf=%p, mlen=%d!!!\r\n",p_cln->dyn_recv_buf, p_cln->mlen);
                free(p_cln->dyn_recv_buf);
            }

            p_cln->dyn_recv_buf = (GK_CHAR *)malloc(p_cln->ctt_len + p_cln->hdr_len + 1);
            memcpy(p_cln->dyn_recv_buf, p_cln->rcv_buf, p_cln->rcv_dlen);
            p_cln->p_rbuf = p_cln->dyn_recv_buf;
            p_cln->mlen = p_cln->ctt_len + p_cln->hdr_len;

            if(rx_msg) 
                http_bm_free_httpmsg(rx_msg);
                
            continue;
        }
        if(p_cln->rcv_dlen >= (p_cln->ctt_len + p_cln->hdr_len))
        {
            if(rx_msg == NULL)
            {
                GK_S32 nlen = p_cln->ctt_len + p_cln->hdr_len;
                if(nlen > HTTP_NETBUF_SIZE)//1K
                {
                    rx_msg = http_bm_get_msglargebuf(nlen+1);
                    if(rx_msg == NULL)
                        break;
                }
                else
                {
                    rx_msg = http_bm_get_msgbuf();
                    if(rx_msg == NULL)
                        break;
                }

                memcpy(rx_msg->msg_buf,p_cln->p_rbuf,p_cln->hdr_len);
                rx_msg->msg_buf[p_cln->hdr_len] = '\0';
                GK_S32 parse_len = http_parse_msg_header(rx_msg->msg_buf,p_cln->hdr_len,rx_msg);
                if(parse_len != p_cln->hdr_len)
                {
                    ONVIF_ERR("server_tcp_rx::parse_len=%d, sip_pkt_len=%d!!!\r\n",parse_len,p_cln->hdr_len);
                    http_bm_free_httpmsg(rx_msg);
                    break;
                }

            }

            if(p_cln->ctt_len > 0)
            {
                memcpy(rx_msg->msg_buf+p_cln->hdr_len, p_cln->p_rbuf+p_cln->hdr_len, p_cln->ctt_len);
                rx_msg->msg_buf[p_cln->hdr_len + p_cln->ctt_len] = '\0';
                GK_S32 parse_len = http_parse_msg_ctt(rx_msg->msg_buf+p_cln->hdr_len,p_cln->ctt_len,rx_msg);
                if(parse_len != p_cln->ctt_len)
                {
                    ONVIF_ERR("server_tcp_rx::parse_len=%d, sdp_pkt_len=%d!!!\r\n",parse_len,p_cln->ctt_len);
                    http_bm_free_httpmsg(rx_msg);
                    break;
                }
            }
			ONVIF_INFO("start handle soap message.");
            http_server_rx_msg(p_cln, rx_msg);            
			ONVIF_INFO("handle soap message ended.");
            p_cln->rcv_dlen -= p_cln->hdr_len + p_cln->ctt_len;

            if(p_cln->dyn_recv_buf == NULL)
            {
                if(p_cln->rcv_dlen > 0)
                {
                    memmove(p_cln->rcv_buf, p_cln->rcv_buf+p_cln->hdr_len + p_cln->ctt_len, p_cln->rcv_dlen);
                    p_cln->rcv_buf[p_cln->rcv_dlen] = '\0';
                }
                p_cln->p_rbuf = p_cln->rcv_buf;
                p_cln->mlen = sizeof(p_cln->rcv_buf)-1;
                p_cln->hdr_len = 0;
                p_cln->ctt_len = 0;

                if(p_cln->rcv_dlen > 16)
                {
                	ONVIF_ERR("rcv_dlen > hdr_len + ctt_len, jump to rx_analyse_point");
                    goto rx_analyse_point;
                }
            }
            else
            {
                free(p_cln->dyn_recv_buf);
                p_cln->dyn_recv_buf = NULL;
                p_cln->hdr_len = 0;
                p_cln->ctt_len = 0;
                p_cln->p_rbuf = 0;
                p_cln->rcv_dlen = 0;
            }
        }

        if (rx_msg)
            http_bm_free_httpmsg(rx_msg);
    }
    
    http_server_free_used_cln(p_srv, p_cln);
    
    return 0;
}

