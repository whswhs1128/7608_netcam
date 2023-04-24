
/*
 * yudp api define.
 *
 * Author : Heyong
 * Date   : 2015.11.18
 * Copyright (c) 2015 Harric He(Heyong)
 *
 */

#include "stdafx.h"

#ifndef _WIN32
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif


#include "yudp.h"

static int g_connect_ack_len = sizeof(YUDP_CONNECT_ACK_MSG);
static int g_connect_len = sizeof(YUDP_CONNECT_MSG);
static int g_disconnect_len = sizeof(YUDP_DISCONNECT_MSG);
static int g_heart_len = sizeof(YUDP_HEART_MSG);

y_time_t yudp_gettime_ms(void)
{
#ifdef _WIN32
    return ::timeGetTime();
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (y_time_t)tv.tv_sec * 1000 + (y_time_t)tv.tv_usec / 1000;
#endif
}

static int socket_set_nonblock(int socket, int enable)
{
#ifdef _WIN32
	if (enable)
	{
        unsigned long val = 1;
        return ioctlsocket(socket, FIONBIO, &val);
	}
	return 0;
#else
	if (enable)
		return fcntl(socket, F_SETFL, fcntl(socket, F_GETFL) | O_NONBLOCK);
	else
		return fcntl(socket, F_SETFL, fcntl(socket, F_GETFL) & ~O_NONBLOCK);
#endif
}
/*
static void yudp_addr_to_socketaddr_in(yudp_addr_t * addr, struct sockaddr_in *sockaddr)
{
    bzero(sockaddr, sizeof(struct sockaddr_in));
    sockaddr->sin_family = AF_INET;
    sockaddr->sin_addr.s_addr = htonl(addr->ip);
    sockaddr->sin_port = htons(addr->port);
}
*/
static void yudp_init_host_peers(yudp_host_t* host)
{
    int j;
    for(j = 0; j < YUDP_HOST_MAX_PEER_COUNT; j ++)
    {
        host->peers[j].sock  = -1;
        host->peers[j].thread_run  = 1;
    }
}

static void yudp_remove_peer_by_id(yudp_host_t* host, int id)
{
    host->peers[id].sock  = -1;

    //close the peer thread.
    host->peers[id].thread_run = 0;
	Y_SLEEP_MS(100);
}

static int yudp_get_usable_peerid(yudp_host_t* host)
{
    int i;
    for(i = 0; i < YUDP_HOST_MAX_PEER_COUNT; i ++)
    {
        if(host->peers[i].sock != -1)
            continue;
        else
        {
            host->peers[i].sock = -1;
            host->peers[i].thread_id = -1;
            host->peers[i].thread_run = 1;
        }
        return i;
    }
    return -1;
}
static int yudp_peer_exist(yudp_host_t* host, unsigned long ip, unsigned short port)
{
    int i;
    for(i = 0; i < YUDP_HOST_MAX_PEER_COUNT; i ++)
    {
        if((host->peers[i].addr.ip == ip)&&(host->peers[i].addr.port == port))
            return i;
    }
    return -1;
}

static char * yudp_get_peer_ip(yudp_peer_t* peer)
{
    struct in_addr addr;
    addr.s_addr = peer->addr.ip;
    char * ip = inet_ntoa(addr);
    return ip;
}
/*static char * yudp_get_sockaddr_ip(struct sockaddr_in * sockaddr)
{
    char * ip = inet_addr(inet_ntoa(sockaddr->sin_addr));
    return ip;
}*/
static int yudp_get_peer_port(yudp_peer_t* peer)
{
    return peer->addr.port;
}

void yudp_fill_addr(yudp_addr_t * addr, char *ip, int port)
{
	if(ip == NULL)
		addr->ip = 0;
	else
		addr->ip = inet_addr(ip);
	addr->port = port;
}

yudp_host_t* yudp_host_create(const yudp_addr_t * addr)
{
    int opt;
    yudp_host_t* host = NULL;
    Y_SMALLOC(host, sizeof(yudp_host_t), yudp_host_t);

    host->sock = -1;
    host->thread_id = -1;
    host->thread_run = 1;

    memcpy(&host->addr, addr, sizeof(host->addr));
    yudp_init_host_peers(host);

    if((host->sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        Y_ERROR("yudp_host_create create socket error\n");
        yudp_host_destroy(host);
        return NULL;
    }

	if (socket_set_nonblock(host->sock, 1) < 0)
	{
        Y_ERROR("yudp_host_create create error, socket_set_nonblock\n");
        yudp_host_destroy(host);
        return NULL;
	}

    opt = 1;
    setsockopt(host->sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));

    int oldsize = 0;
	int newsize = YUDP_MAX_SEND_RECV_LENGTH;
	int endsize = 0;
    int len = sizeof(oldsize);
    //set send buffer.
    getsockopt(host->sock, SOL_SOCKET, SO_SNDBUF, (char *)&oldsize, (socklen_t * )&len);
    setsockopt(host->sock, SOL_SOCKET, SO_SNDBUF, (char *)&newsize, len);
    getsockopt(host->sock, SOL_SOCKET, SO_SNDBUF, (char *)&endsize, (socklen_t *)&len);
    Y_INFO("yudp_host_create: set SO_SNDBUF %d->%d (real = %d)\n",oldsize,newsize,endsize);

    oldsize = 0;
    newsize = YUDP_MAX_SEND_RECV_LENGTH;
    endsize = 0;
    //set recv buffer.
    getsockopt(host->sock, SOL_SOCKET, SO_RCVBUF, (char *)&oldsize, (socklen_t * )&len);
    setsockopt(host->sock, SOL_SOCKET, SO_RCVBUF, (char *)&newsize, len);
    getsockopt(host->sock, SOL_SOCKET, SO_RCVBUF, (char *)&endsize, (socklen_t *)&len);
    Y_INFO("yudp_host_create: set SO_RCVBUF %d->%d (real = %d)\n",oldsize,newsize,endsize);

    struct sockaddr_in sock_addr;
    memset(&sock_addr, 0, sizeof(struct sockaddr_in));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = addr->ip;
    sock_addr.sin_port = addr->port;
    if(bind(host->sock, (struct sockaddr*)&sock_addr, sizeof(struct sockaddr)) < 0)
    {
        Y_ERROR("yudp_host_create bind error\n");
        yudp_host_destroy(host);
        return NULL;
    }

    return host;
}

void yudp_host_destroy(yudp_host_t* host)
{
    int j;
    for(j = 0; j < YUDP_HOST_MAX_PEER_COUNT; j ++)
    {
        if(host->peers[j].sock != -1)
        {
            //close the peer thread.
            host->peers[j].thread_run = 0;
        	Y_SLEEP_MS(100);
        	host->peers[j].sock = -1;
    	}
    }

    if(host->sock != -1)
    {
        y_socketclose(host->sock);
        host->sock = -1;
    }
    Y_SFREE(host);
}
int yudp_host_connect_msg(yudp_host_t* host, unsigned long ip, unsigned short port)
{
    int id = yudp_peer_exist(host, ip, port);
    if(id >= 0)//exist.
    {
        //yudp_remove_peer_by_id(host, id);//close at first.
        return -1;//id;
    }

    int usable_id = yudp_get_usable_peerid(host);
    host->peers[usable_id].sock = host->sock;
    host->peers[usable_id].addr.ip = ip;
    host->peers[usable_id].addr.port = port;
    host->peers[usable_id].last_heart_time = yudp_gettime_ms();

    yudp_peer_send_n(&host->peers[usable_id], YUDP_CONNECT_ACK_MSG, g_connect_ack_len, YUDP_DEFAULT_SEND_COUNT);
    return usable_id;
}
int yudp_host_heart_msg(yudp_host_t* host, unsigned long ip, unsigned short port)
{
    int id = yudp_peer_exist(host, ip, port);
    if(id < 0)//not exist.
    {
        Y_ERROR("unknow heart msg\n");
        return -1;
    }
    host->peers[id].last_heart_time = yudp_gettime_ms();
    return id;
}
int yudp_host_disconnect_msg(yudp_host_t* host, unsigned long ip, unsigned short port)
{
    int id = yudp_peer_exist(host, ip, port);
    if(id >= 0)//exist.
    {
        yudp_remove_peer_by_id(host, id);//close.
    }
    return id;
}
int yudp_host_check_peers_state(yudp_host_t* host)
{
    int i;
    for(i = 0; i < YUDP_HOST_MAX_PEER_COUNT; i ++)
    {
        if(host->peers[i].sock != -1)
        {
            if((yudp_gettime_ms() - host->peers[i].last_heart_time) > YUDP_PEER_MAX_HEART_TIMEOUT)
            {
                Y_ERROR("YUDP_DISCONNECT_MSG-> ip:%s port:%d (heart timeout)\n", yudp_get_peer_ip(&host->peers[i]), yudp_get_peer_port(&host->peers[i]));
                yudp_remove_peer_by_id(host, i);
            }
        }
    }
    return 0;
}

int yudp_host_monitor(yudp_host_t* host, hrtp_recv_event_t *recv_event, int timeout_ms)
{
    fd_set select_fds;
    struct timeval timeout;
    int ret, addr_size;
    struct sockaddr_in client_addr;

    recv_event->data_len = 0;

    timeout.tv_sec = 0;
    timeout.tv_usec = timeout_ms*1000;
    FD_ZERO(&select_fds);
    FD_SET(host->sock, &select_fds);
    ret = select(host->sock + 1, &select_fds, NULL, NULL, &timeout);
    if(ret < 0)
    {
        Y_ERROR("select error\n");
        goto Y_ERROR_RET;
    }

    if(FD_ISSET(host->sock, &select_fds))
    {
        addr_size = sizeof(client_addr);
        if((ret = recvfrom(host->sock, (char *)recv_event->data, YUDP_HOST_MAX_RECV_LENGTH, 0, (struct sockaddr *)&client_addr, (socklen_t *)&addr_size))< 1)
        {
            Y_ERROR("host recv error ret:%d\n", ret);
            goto Y_ERROR_RET;
        }
        switch(yudp_cmd_check((char *)recv_event->data, ret))
        {
        case YUDP_CONNECT:
            Y_ERROR("YUDP_CONNECT_MSG-> ip:%s port:%d\n",inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
            recv_event->peer_id = yudp_host_connect_msg(host, inet_addr(inet_ntoa(client_addr.sin_addr)), client_addr.sin_port);
            recv_event->data_len = 0;
            break;
        case YUDP_DISCONNECT:
            Y_ERROR("YUDP_DISCONNECT_MSG-> ip:%s port:%d\n",inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
            yudp_host_disconnect_msg(host, inet_addr(inet_ntoa(client_addr.sin_addr)), client_addr.sin_port);
            recv_event->peer_id = -1;
            recv_event->data_len = 0;
            break;
        case YUDP_HEART:
            //Y_INFO("YUDP_HEART_MSG-> ip:%s port:%d\n",inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
            yudp_host_heart_msg(host, inet_addr(inet_ntoa(client_addr.sin_addr)), client_addr.sin_port);
            recv_event->peer_id = -1;
            recv_event->data_len = 0;
            break;
        default:
            recv_event->peer_id = yudp_peer_exist(host, inet_addr(inet_ntoa(client_addr.sin_addr)), client_addr.sin_port);
            if(recv_event->peer_id != -1)
            {
                //Y_INFO("YUDP_RECV_NORMAL_DATA-> ip:%s port:%d, len = %d\n",inet_ntoa(client_addr.sin_addr), client_addr.sin_port, ret);
                recv_event->data_len = ret;
            }else
            {
                Y_ERROR("YUDP_RECV_NORMAL_DATA-> ip:%s port:%d, error peer not exist\n",inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
                recv_event->data_len = 0;
            }
            break;
        }
    }else
    {
        goto Y_ERROR_RET;
    }

    yudp_host_check_peers_state(host);
    return 0;
Y_ERROR_RET:

    yudp_host_check_peers_state(host);
    return -1;
}

yudp_peer_t* yudp_peer_create(const yudp_addr_t * addr)
{
    yudp_peer_t* peer = NULL;
    Y_SMALLOC(peer, sizeof(yudp_peer_t), yudp_peer_t);

    peer->sock = -1;
    peer->thread_id = -1;
    peer->thread_run = 1;

    memcpy(&peer->addr, addr, sizeof(peer->addr));

    if((peer->sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        Y_ERROR("create tcp server error\n");
        yudp_peer_destroy(peer);
        return NULL;
    }


	if (socket_set_nonblock(peer->sock, 1) < 0)
	{
        Y_ERROR("create tcp server error, socket_set_nonblock\n");
        yudp_peer_destroy(peer);
        return NULL;
	}

    int oldsize = 0;
	int newsize = YUDP_MAX_SEND_RECV_LENGTH;
	int endsize = 0;
    int len = sizeof(oldsize);
    //set send buffer.
    getsockopt(peer->sock, SOL_SOCKET, SO_SNDBUF, (char *)&oldsize, (socklen_t * )&len);
    setsockopt(peer->sock, SOL_SOCKET, SO_SNDBUF, (char *)&newsize, len);
    getsockopt(peer->sock, SOL_SOCKET, SO_SNDBUF, (char *)&endsize, (socklen_t *)&len);
    Y_INFO("yudp_peer_create: set SO_SNDBUF %d->%d (real = %d)\n",oldsize,newsize,endsize);

    oldsize = 0;
    newsize = YUDP_MAX_SEND_RECV_LENGTH;
    endsize = 0;
    //set recv buffer.
    getsockopt(peer->sock, SOL_SOCKET, SO_RCVBUF, (char *)&oldsize, (socklen_t * )&len);
    setsockopt(peer->sock, SOL_SOCKET, SO_RCVBUF, (char *)&newsize, len);
    getsockopt(peer->sock, SOL_SOCKET, SO_RCVBUF, (char *)&endsize, (socklen_t *)&len);
    Y_INFO("yudp_peer_create: set SO_RCVBUF %d->%d (real = %d)\n",oldsize,newsize,endsize);
    return peer;
}

void yudp_peer_destroy(yudp_peer_t* peer)
{
    yudp_peer_send(peer, YUDP_DISCONNECT_MSG, g_disconnect_len);

    //close the peer thread.
    peer->thread_run = 0;
	Y_SLEEP_MS(100);

    if(peer->sock != -1)
    {
        y_socketclose(peer->sock);
        peer->sock = -1;
    }
    Y_SFREE(peer);
}


int yudp_peer_connect(yudp_peer_t* peer, int timeout_sec)
{
    int ret = 0;
    char rcv_buffer[YUDP_HOST_MAX_RECV_LENGTH];
    while(peer->thread_run)
    {
        yudp_peer_send_n(peer, YUDP_CONNECT_MSG, g_connect_len, YUDP_DEFAULT_SEND_COUNT);
        ret = yudp_peer_recv_normal(peer, rcv_buffer, YUDP_HOST_MAX_RECV_LENGTH, timeout_sec);
        if(ret<=0)break;
        if(yudp_cmd_check(rcv_buffer, ret) == YUDP_CONNECT_ACK)//connect successed.
        {
            Y_INFO("YUDP_CONNECT_ACK_MSG-> ip:%s port:%d\n",yudp_get_peer_ip(peer), yudp_get_peer_port(peer));
            peer->last_recv_time = yudp_gettime_ms();
            peer->thread_run = 1;
            return 0;
        }
    }
    return -1;
}

int yudp_peer_disconnect(yudp_peer_t* peer)
{
    Y_INFO("yudp_peer_disconnect-> ip:%s port:%d\n",yudp_get_peer_ip(peer), yudp_get_peer_port(peer));
    yudp_peer_send_n(peer, YUDP_DISCONNECT_MSG, g_disconnect_len, YUDP_DEFAULT_SEND_COUNT);//disconnect.
    return 0;
}

int yudp_peer_send_heart(yudp_peer_t* peer)
{
    y_time_t now_time = yudp_gettime_ms();
    if((now_time - peer->last_heart_time) > YUDP_PEER_MAX_HEART_INTERVAL)
    {
        //Y_INFO("YUDP_HEART_MSG -> ip:%s port:%d\n",yudp_get_peer_ip(peer), yudp_get_peer_port(peer));
        yudp_peer_send(peer, YUDP_HEART_MSG, g_heart_len);
        peer->last_heart_time = yudp_gettime_ms();
    }

    if(((now_time - peer->last_recv_time) > YUDP_PEER_MAX_HEART_TIMEOUT)&&(peer->thread_run == 1))
    {
        Y_INFO("YUDP_DISCONNECT_MSG-> ip:%s port:%d\n",yudp_get_peer_ip(peer), yudp_get_peer_port(peer));
        yudp_peer_send_n(peer, YUDP_DISCONNECT_MSG, g_disconnect_len, YUDP_DEFAULT_SEND_COUNT);//disconnect.
        //peer->thread_run = 0;
    }
    return 0;
}

int yudp_peer_send(yudp_peer_t* peer, const char* data, const int length)
{
    if(length > YUDP_MAX_SEND_RECV_LENGTH)
    {
        Y_ERROR("yudp_peer_send: too long %d-%d\n", length, YUDP_MAX_SEND_RECV_LENGTH);
        return -1;
    }

    int sock = peer->sock;
    int ret = 0;
    int sendlen = 0;
    int max_send_len = 0;

    struct sockaddr_in sock_addr;
    memset(&sock_addr,0, sizeof(struct sockaddr_in));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = peer->addr.ip;
    sock_addr.sin_port = peer->addr.port;

    while(sendlen < length)
    {
        max_send_len = (((length - sendlen)>=YUDP_MAX_ONE_SEND_RECV_LENGTH)?YUDP_MAX_ONE_SEND_RECV_LENGTH:(length - sendlen));
        ret = sendto(sock, data + sendlen, max_send_len, 0, (struct sockaddr*)&sock_addr,  sizeof(sock_addr));
        if(ret < 0)
        {
            if (errno != EINTR && errno != EAGAIN )//no blocked error = EAGAIN
            {
                Y_ERROR("yudp_peer_send socket %d error :%s\n", sock, strerror(errno));
                return -1;
            }
            else
                continue;
        }

        sendlen += ret;
    }

    return sendlen;
}

int yudp_peer_send_n(yudp_peer_t* peer, const char* data, const int length, int send_count)
{
    int result = 0;
    while((send_count--) > 0)
    {
        result = yudp_peer_send(peer, (char *)data, length);
        if (result <= 0)
        {
            Y_ERROR("yudp_peer_send error, send_count = %d\n", send_count);
        }
    }
    return result;
}

int yudp_cmd_check(char* data, const int length)
{
    if((length == g_connect_ack_len)
        &&(strncmp(data, YUDP_CONNECT_ACK_MSG, g_connect_ack_len) == 0))
    {
        return YUDP_CONNECT_ACK;
    }
    else if((length == g_connect_len)
        &&(strncmp(data, YUDP_CONNECT_MSG, g_connect_len) == 0))
    {
        return YUDP_CONNECT;
    }
    else if((length == g_disconnect_len)
        &&(strncmp(data, YUDP_DISCONNECT_MSG, g_disconnect_len) == 0))
    {
        return YUDP_DISCONNECT;
    }
    else if((length == g_heart_len)
        &&(strncmp(data, YUDP_HEART_MSG, g_heart_len) == 0))
    {
        return YUDP_HEART;
    }
    return -1;
}

//Get the Length in the socket before use receive_from function
/*int sock_nread(int sck)
{
   int ires;
   ulong nread = 0;


   fcntl(sck,F_SETFL,O_NONBLOCK));

#ifdef __WIN32
   ires= ioctlsocket ( sck, FIONREAD, &nread );
#else
   ires = ioctl(sck,FIONREAD,&nread);
#endif

   if ( ires != 0 )
       return 0;
   return nread;
}*/

int yudp_peer_recv_normal(yudp_peer_t* peer, char* data, int length, int timeout_sec)
{
    fd_set select_fds;
    struct timeval timeout;
    int ret = 0, addr_size;

    struct sockaddr_in sock_addr;
    memset(&sock_addr,0 , sizeof(struct sockaddr_in));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = peer->addr.ip;
    sock_addr.sin_port = peer->addr.port;

    while(peer->thread_run && (timeout_sec > 0))
    {
        timeout.tv_sec = 1;//1s.
        timeout.tv_usec = 0;
        FD_ZERO(&select_fds);
        FD_SET(peer->sock, &select_fds);
        ret = select(peer->sock + 1, &select_fds, NULL, NULL, &timeout);
        if(ret < 0)
        {
            Y_ERROR("select error\n");
            return -1;
        }

        if(FD_ISSET(peer->sock, &select_fds))
        {
            addr_size = sizeof(sock_addr);
            if((ret = recvfrom(peer->sock, data, length, 0, (struct sockaddr *)&sock_addr, (socklen_t *)&addr_size))< 1)
            {
                Y_ERROR("peer recv error ret:%d\n", ret);
                continue;
            }
            //last_recv_time.
            peer->last_recv_time = yudp_gettime_ms();
            return ret;//recv successed.
        }
        timeout_sec--;
    }

    if(timeout_sec <= 0)
    {
        //Y_ERROR("yudp_peer_recv timeout.\n");
    }else
    {
        Y_ERROR("yudp_peer_recv yudp close. %d\n", ret);
    }
    return -1;
}

int yudp_peer_recv(yudp_peer_t* peer, char* data, int length, int timeout_sec)
{
    int ret = yudp_peer_recv_normal(peer, data, length, timeout_sec);
    if(yudp_cmd_check(data, ret) < 0)
        return ret;
    else
        return 0;
}

#if 0
int yudp_peer_recv_hrtp(yudp_peer_t* peer, hrtp_recombine_t* recombine, int timeout_sec)
{
    char recv_buffer[YUDP_MAX_ONE_SEND_RECV_LENGTH];
    int ret = yudp_peer_recv(peer, recv_buffer, YUDP_MAX_ONE_SEND_RECV_LENGTH, timeout_sec);
    if(ret <= 0)
    {
        return -1;
    }

    if(ret%(HRTP_FRAGMENT_SIZE) != 0)
    {
        Y_INFO("yudp_peer_recv: erro recv(%d), ret%%(HRTP_FRAGMENT_SIZE) = %d\n",ret,ret%(HRTP_FRAGMENT_SIZE));
        return -1;
    }

    //last_recv_time.
    peer->last_recv_time = yudp_gettime_ms();

    yudp_peer_analyse_fragment(recombine, recv_buffer, ret);
    return 0;
}

int yudp_peer_analyse_fragment(hrtp_recombine_t* recombine, char* data, int length)
{
    //int size = 0;
    hrtp_fragment_t *fragment = NULL;
    int totle_len = 0;
    while(length > 0)
    {
        fragment = (hrtp_fragment_t *)data;
        //size = (sizeof(fragment->hrtp_hdr)+fragment->hrtp_hdr.payload_len);
        totle_len += fragment->hrtp_hdr.payload_len;

        //hrtp_fragment_printf(fragment);
        hrtp_recombine_add(recombine, fragment);

        data += sizeof(hrtp_fragment_t);//size;
        length -= sizeof(hrtp_fragment_t);//size;
    }
    if(totle_len != 0)
        Y_INFO("yudp_peer_analyse_fragment: recv (%d) data.\n",totle_len);
    return 0;
}
#endif
