/*!
*****************************************************************************
** FileName     : gk_cms_utility.c
**
** Description  : utility api of cms.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Date         : 2015-9-9, create.
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#include "common.h"
#include "gk_cms_common.h"
#include "gk_cms_utility.h"
#include "gk_cms_sock.h"
#include "sdk_network.h"
#include "netcam_api.h"
#include "utility_api.h"

pthread_mutex_t gk_send_cmd_mutex = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t gk_send_frame_mutex = PTHREAD_MUTEX_INITIALIZER;


int tcp_select_send(int sock, char *szbuf, int len, int timeout)
{
    int ret = 0;
    fd_set send_set;
    struct timeval tv;
    int send_total = 0;
    int n_timeout_num = 0;
    if (timeout == 0)
        timeout = 3;
    while(send_total < len) {
        if(3 == n_timeout_num) {//如果超时3次则关闭此次会话
            return -2;
        }

    	FD_ZERO(&send_set);
    	FD_SET(sock, &send_set);
    	tv.tv_sec = timeout;
    	tv.tv_usec = 0;
        ret = select(sock + 1, NULL, &send_set, NULL, &tv);
       	if (ret < 0) {
            PRINT_ERR("sock:%d select send error:%s\n", sock, strerror(errno));
          	return -1;
    	}
        if(0 == ret) {
            PRINT_ERR("sock:%d select send timeout error:%s\n", sock, strerror(errno));
            n_timeout_num++;
            continue;
        }

		if(FD_ISSET(sock, &send_set)) {
        	ret = send(sock, szbuf + send_total, len - send_total, 0);
            if(ret <= 0) {
                PRINT_ERR("SockSend Failed ret:%d, sock:%d, len:%d error:%s\n", ret, sock, len, strerror(errno));
                return -3;
            }

    		n_timeout_num = 0;
			//break;
		}

        send_total += ret;
    }

    //PRINT_INFO("Leave ==========> SockSend sock:%d, send_total:%d\n",sock,  send_total);
	return send_total;
}



/*************************************************************
* 函数介绍：从socket发送数据
* 输入参数：
* 输出参数：
* 返回值  ：
*************************************************************/
int GkSockSend(int sock, char *szbuf, int len, int timeout)
{
#if 1
    int ret = 0;
    pthread_mutex_lock(&gk_send_cmd_mutex);
    //printf("cms sock %d send.\n", sock);
    ret = tcp_select_send(sock, szbuf, len, 3);
	pthread_mutex_unlock(&gk_send_cmd_mutex);
    return ret;
#else
	int ret = send(sock, szbuf, len, 0);
    if (ret <= 0) {
        PRINT_ERR("SockSend Failed ret:%d, sock:%d, len:%d errno:%s\n", ret, sock, len, strerror(errno));
        return -3;
    }
	if (ret != len)
		PRINT_INFO("send not equil. send_len = %d, len = %d, sock = %d\n", ret, len, sock);

	return ret;
#endif
}

int GkSockSendFrame(int sock, char *szbuf, int len)
{
#if 1
    int ret = 0;
    //pthread_mutex_lock(&gk_send_frame_mutex);
    //printf("data sock %d send.\n", sock);
    ret = tcp_select_send(sock, szbuf, len, 3);
    //pthread_mutex_unlock(&gk_send_frame_mutex);
    return ret;
#else
    //pthread_mutex_lock(&gk_send_frame_mutex);
	int ret = 0;
	ret = send(sock, szbuf, len, 0);
    if(ret <= 0) {
        PRINT_ERR("SockSend Failed ret:%d, sock:%d, len:%d errno:%s\n", ret, sock, len, strerror(errno));
        //pthread_mutex_unlock(&gk_send_frame_mutex);
        return 0;
    }
	if(ret != len)
		PRINT_INFO("send not equil. send_len = %d, len = %d, sock = %d\n", ret, len, sock);

    //pthread_mutex_unlock(&gk_send_frame_mutex);
	return ret;
#endif

}


int GkSockSendTalk(int sock, char *szbuf, int len, int timeout)
{
    int ret = 0;
    ret = tcp_select_send(sock, szbuf, len, 3);
    return ret;
}


int safe_tcp_recv (int sockfd, void *buf, int bufsize)
{
    int cur_len;

recv_again:
    cur_len = recv(sockfd, buf, bufsize, 0);
    //closed by client
    if (cur_len == 0) {
        PRINT_ERR("connection closed by peer, fd=%d", sockfd);
        return 0;
    } else if (cur_len == -1) {
        if (errno == EINTR)
            goto recv_again;
        else
            PRINT_ERR("recv error, fd=%d, errno=%d %m", sockfd, errno);
    }
    return cur_len;
}

//非阻塞方式
static int safe_tcp_recv_n (int sockfd, void *buf, int total)
{
    int recv_bytes, cur_len;
    for (recv_bytes = 0; recv_bytes < total; recv_bytes += cur_len) {
        cur_len = recv (sockfd, buf + recv_bytes, total - recv_bytes, 0);
        //closed by client
        if (cur_len == 0) { //正常关闭
            PRINT_ERR("remote close %d", sockfd);
            return -1;
        } else if (cur_len == -1) {
            if (errno == EINTR) { //还需要在读
                //PRINT_INFO("continue recv.");
                cur_len = 0;
            }  else if (errno == EAGAIN) { /* TCP/IP协议栈的接收缓冲区已经没有数据可读了 */
                //PRINT_INFO("read complete, read %d", recv_bytes);
                return recv_bytes;
            } else {
                PRINT_ERR("recv tcp packet error, socket=%d, error:%s", sockfd, strerror(errno));
                return -1;
            }
        }
    }
    return recv_bytes;
}

int tcp_select_recv(int sock, char *szbuf, int len, int timeout)
{
    //select方式接收
    int ret = 0;
    int recv_len = 0;
	fd_set read_set;
	struct timeval tv;
    int n_timeout_num = 0;
 	if (timeout == 0)
        timeout = 3; //sec
	while (1) {
        if(2 == n_timeout_num) {//如果超时2次则关闭此次会话
            return -1;
        }

		FD_ZERO(&read_set);
		FD_SET(sock, &read_set);
		tv.tv_sec = timeout;
		tv.tv_usec = 0;
	   	if ((ret = select(sock + 1, &read_set, NULL, NULL, &tv)) < 0) {
            PRINT_ERR("sock:%d select recv error:%s\n", sock, strerror(errno));
	      	return -1;
		}
        if(0 == ret) {
            PRINT_ERR("sock:%d select recv timeout error:%s\n", sock, strerror(errno));
            n_timeout_num++;
            continue;
        }

		if(FD_ISSET(sock, &read_set)) {
    		//recv_len = recv(sock, szbuf, len, 0);
            recv_len = safe_tcp_recv_n (sock, szbuf, len);
            if (recv_len < 0) {
                return -1;
            }
            //PRINT_INFO("recv have data, recv %d\n", recv_len);
    		n_timeout_num = 0;
			break;
		}
	};
    return recv_len;
}

/*************************************************************
* 函数介绍：从socket接收数据
* 输入参数：
* 输出参数：
* 返回值  ：
*************************************************************/
int GkSockRecv(int sock, char *szbuf, int len)
{
#if 0
    //阻塞方式接收
	int recv_len = recv(sock, szbuf, len, 0);
    if (recv_len <= 0) {
        PRINT_INFO("recv_len=%d, error:%s\n", recv_len, strerror(errno));
    }
	return recv_len;
#endif
#if 1
    int ret = 0;
    ret = tcp_select_recv(sock, szbuf, len, 3);
    return ret;
#endif
#if 0
    //非阻塞方式接收
    return safe_tcp_recv_n (sock, szbuf, len);
#endif

}

int GetTimetick(int *timetick)
{
	if(timetick == NULL)
		return -1;

	GkTimetick *pGkTimetick = (GkTimetick *)timetick;

	long ts = time(NULL);
	struct tm tt = {0};
	struct tm *pTm = localtime_r(&ts, &tt);

	pGkTimetick->year = pTm->tm_year+1900-2000;
	pGkTimetick->month = pTm->tm_mon+1;
	pGkTimetick->day = pTm->tm_mday;
	pGkTimetick->hour = pTm->tm_hour;
	pGkTimetick->minute = pTm->tm_min;
	pGkTimetick->second = pTm->tm_sec;

#if 0
	char strTime[128] = {0};
	sprintf(strTime,"stTime: %d-%02d-%02d %02d:%02d:%02d", pGkTimetick->year, pGkTimetick->month, pGkTimetick->day, pGkTimetick->hour, pGkTimetick->minute, pGkTimetick->second);
	//printf("strTime:%s\n", strTime);
	//printf("timetick:%d\n", *timetick);
#endif

	return 0;
}

int CreateTcpSock(char *ipaddr, int port)
{
	int ret;
	int sockfd;
	in_addr_t s_addr;
	int reuseaddr = 1;
	struct sockaddr_in localaddr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		return -1;

	if (ipaddr && ipaddr[0] && inet_pton(AF_INET, ipaddr, &s_addr) != 1) {
		goto fail;
	} else {
		s_addr = htonl(INADDR_ANY);
	}

	memset(&localaddr, 0, sizeof(localaddr));
	localaddr.sin_family = AF_INET;
	localaddr.sin_port = htons(port);
	localaddr.sin_addr.s_addr = s_addr;

	/* Create socket for listening connections. */
	ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));
	if (ret < 0)
		goto fail;

	ret = bind(sockfd, (struct sockaddr *)&localaddr, sizeof(localaddr));
	if (ret < 0)
		goto fail;

	ret = listen(sockfd, GK_MAX_LINK_NUM);
	if (ret < 0)
		goto fail;

	return sockfd;

fail:
	close(sockfd);
	return -1;
}

int SetSockAttr(int fd)
{
    #if 1
    int opt = 1;
    int ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (ret < 0) {
        PRINT_ERR("setsockopt SO_REUSEADDR failed, error:%s\n", strerror(errno));
        return -1;
    }
    #endif
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1) {
		PRINT_ERR("Fail to get old flags [set_nonblocking]! errno[%d] errinfo[%s]\n",
			errno, strerror(errno));
		return -1;
	}
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		PRINT_ERR("Fail to set flags to O_NONBLOCK [set_nonblocking]! errno[%d] errinfo[%s]\n",
			errno, strerror(errno));
		return -1;
	}

    #if 0
    //设置回阻塞
    flags = fcntl(fd, F_GETFL, 0);
    fcntl(connectfd, F_SETFL, flags & ~O_NONBLOCK);
    #endif

#if 0
    //set send buffer
	int size = 0;
    int len = sizeof(size);
    getsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char *)&size, (socklen_t * )&len);
    PRINT_INFO("Socket: the default tcp send buffer size is %d\n",size);

    size = 204800;
    setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char *)&size, len);
    PRINT_INFO("Socket: set the tcp send buffer size to %d\n",size);

    getsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char *)&size, (socklen_t *)&len);
    PRINT_INFO("Socket: after set, the tcp send buffer size is %d\n",size);

    //set recv buffer
    getsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *)&size, (socklen_t *)&len);
    PRINT_INFO("Socket: the default tcp recv buffer size is %d\n",size);

    size = 204800;
    setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *)&size, len);
    PRINT_INFO("Socket: set the tcp recv buffer size to %d\n",size);

    getsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *)&size, (socklen_t *)&len);
    PRINT_INFO("Socket: after set, the tcp recv buffer size is %d\n",size);
#endif
    #if 1
    // set nodelay
    int on = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void *)&on, sizeof(on));
    #endif
	return 0;
}



int BoardCastSendTo(int fSockSearchfd, char *multi_group, int send_port, const char* inData, const int inLength)
{
	int ret = 0;
	int sendlen = 0;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    //addr.sin_addr.s_addr = inet_addr("255.255.255.255");
    addr.sin_addr.s_addr = inet_addr(multi_group);
    addr.sin_port = htons(send_port);

	//因为UDP 发送时，没有缓冲区，故需要对发送后的返回值进行判断后，多次发送
	while(sendlen < inLength) {
	    ret = sendto(fSockSearchfd, inData + sendlen, inLength - sendlen, 0, (struct sockaddr*)&addr,  sizeof(addr));
		if (ret < 0) {
			perror("Send error");
			//非阻塞才有 EAGAIN
			if (errno != EINTR && errno != EAGAIN ) {
				PRINT_ERR("Send() socket %d error :%s\n", fSockSearchfd, strerror(errno));
				return -1;
			}
			else
				continue;
		}

		sendlen += ret;
	}

	PRINT_INFO("ip search SendLen %s: %d,port:%d ret:%s\n", multi_group, sendlen,send_port, strerror(errno));

	return sendlen;
}

int CreateBroadcastSock(int recv_port)
{
	int ret = 0;
	int opt;

	//得到该套接字
    int sockSvr = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockSvr < 0) {
        PRINT_ERR("socket fail, error:%s\n", strerror(errno));
        goto cleanup;
    }

#if 1
	//设置该套接字
    opt = 1;
    ret = setsockopt(sockSvr, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (ret < 0) {
        PRINT_ERR("setsockopt  SO_REUSEADDR fail, error:%s\n", strerror(errno));
        goto cleanup;
    }
#endif

#if 0
	//设置该套接字接收超时退出(UDP发送没有超时)
	struct timeval timeout;
	timeout.tv_sec = 10; // 10秒
	timeout.tv_usec = 0;
	if (setsockopt(sockSvr, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(struct timeval)) < 0) {
        PRINT_ERR("setsockopt SO_RCVTIMEO fail, error:%s\n", strerror(errno));
        goto cleanup;
	}
#endif

	//设置该套接字为广播类型
    opt = 1;
    ret = setsockopt(sockSvr, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt));
    if (ret < 0) {
        PRINT_ERR("setsockopt SO_BROADCAST fail, error:%s\n", strerror(errno));
        goto cleanup;
    }

	struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(recv_port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

	//绑定该套接字
    ret = bind(sockSvr, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        PRINT_ERR("bind  fail, error:%s\n", strerror(errno));
        goto cleanup;
    }

#if 0
	//设置该套接字为非阻塞
    curFlags = fcntl(sockSvr, F_GETFL, 0);
    curFlags = fcntl(sockSvr, F_SETFL, curFlags |O_NONBLOCK);
#endif

    return sockSvr;

cleanup:
    if(sockSvr >= 0)
        close(sockSvr);
    return -1;
}

int CreateMulticastSock(char *multi_group, int recv_port)
{
	int ret = 0;

	//得到该套接字
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        PRINT_ERR("socket fail, error:%s\n", strerror(errno));
        goto cleanup;
    }

	//设置该套接字
    int opt = 1;
    ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (ret < 0) {
        PRINT_ERR("setsockopt  SO_REUSEADDR fail, error:%s\n", strerror(errno));
        goto cleanup;
    }

	/* set up destination address */
    struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY); /* N.B.: differs from sender */
	addr.sin_port = htons(recv_port);
	/* bind to receive address */
	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind");
		goto cleanup;
	}

#if 1
    struct ip_mreq mreq;
	mreq.imr_multiaddr.s_addr = inet_addr(multi_group);

    ST_SDK_NETWORK_ATTR net_attr;
    memset(&net_attr, 0, sizeof(ST_SDK_NETWORK_ATTR));

    ret = netcam_net_get_detect(runNetworkCfg.lan.netName);
    if (ret != 0) {
        PRINT_INFO("eth0 is disconnect or disable, use wifi.");
        strcpy(net_attr.name, runNetworkCfg.wifi.netName);

        if(netcam_net_get(&net_attr) != 0) {
            PRINT_ERR("get network config error.\n");
            return -1;
        }
        PRINT_INFO("bc add membership -- wifi ip :%s & static ip :%s\n", net_attr.ip, runNetworkCfg.wifi.ip);
        mreq.imr_interface.s_addr = inet_addr(net_attr.ip);
    } else {
        PRINT_INFO("eth0 is ok.");
        strcpy(net_attr.name, runNetworkCfg.lan.netName);

        if(netcam_net_get(&net_attr) != 0) {
            PRINT_ERR("get network config error.\n");
            return -1;
        }
        PRINT_INFO("bc add membership -- lan ip :%s & static ip :%s\n", net_attr.ip, runNetworkCfg.lan.ip);
        mreq.imr_interface.s_addr = inet_addr(net_attr.ip);
    }

	if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
		perror("setsockopt");
		goto cleanup;
	}
#endif

#if 0
	/* use setsockopt() to request that the kernel join a multicast group */
    struct ip_mreq mreq;
	mreq.imr_multiaddr.s_addr = inet_addr(multi_group);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        PRINT_ERR("setsockopt add to memership error: %s\n", strerror(errno));
		goto cleanup;
	}
#endif

#if 0
	//设置该套接字接收超时退出(UDP发送没有超时)
	struct timeval timeout;
	timeout.tv_sec = 10; // 10秒
	timeout.tv_usec = 0;
	if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(struct timeval)) < 0) {
        PRINT_ERR("setsockopt SO_RCVTIMEO fail, error:%s\n", strerror(errno));
        goto cleanup;
	}
#endif

#if 0
	//设置该套接字为非阻塞
    curFlags = fcntl(sockSvr, F_GETFL, 0);
    curFlags = fcntl(sockSvr, F_SETFL, curFlags |O_NONBLOCK);
#endif

    return fd;

cleanup:
    if(fd >= 0)
        close(fd);
    return -1;
}

void PrintStringToUInt(char *str, int len)
{
    int i;
    for (i = 0; i < len; i ++) {
        if (str[i] != 0)
            printf("[%d]:%02x,", i, str[i]);
    }
    printf("\n");
}

int RecvExtData(int sock, char *recv_buf, int buf_size, int ext_len)
{
    int ret;

    if (ext_len > buf_size) {
        PRINT_ERR("recv buf is too small.\n");
        return -1;
    }
    if (ext_len > 0) {
        PRINT_INFO("ext_len:%d \n", ext_len);
        ret = GkSockRecv(sock, recv_buf, ext_len);
        if (ret != ext_len) {
            PRINT_ERR();
            return -1;
        }
        PrintStringToUInt(recv_buf, ext_len);
    } else {
        PRINT_INFO("ext_len:%d.\n", ext_len);
    }
    return 0;
}

int gk_cms_rate_to_data(int rate, int total)
{
    int ret = ((rate * total)/100);
    return (ret >= total)? total:ret;
}

int gk_cms_data_to_rate(int data, int total)
{
    int ret = ((data * 100)/total);
    return (ret >= 100)? 100:ret;
}

int gk_cms_float_rate_to_data(float rate, int total)
{
    int ret = (int)(rate * total);
    return (ret >= total)? total:ret;
}

float gk_cms_data_to_float_rate(int data, int total)
{
    float ret = (float)data /total;
    return (ret >= 1.0)? 1.0:ret;
}

