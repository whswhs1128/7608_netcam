
/*
 * yudp api define.
 *
 * Author : Heyong
 * Date   : 2015.11.18
 * Copyright (c) 2015 Harric He(Heyong)
 *
 */
#ifndef _YUDP_H__
#define _YUDP_H__


// include
#include "yudp_utility.h"
#include "hrtp.h"


#ifdef _WIN32
#include "GKNet.h"
#else
#endif

#define YUDP_SERVER_PORT 10726

#define YUDP_CONNECT_MSG            "CONNECT"  //7
#define YUDP_CONNECT_ACK_MSG        "CONNECT_ACK"  //11
#define YUDP_DISCONNECT_MSG         "DISCONNECT"  //10
#define YUDP_HEART_MSG              "HEART"  //5
typedef enum{
    YUDP_CONNECT = 0, //"CONNECT"  //7
    YUDP_CONNECT_ACK, //"CONNECT_ACK"  //11
    YUDP_DISCONNECT, //"DISCONNECT"  //10
    YUDP_HEART //"HEART"  //5
}yudp_event;

typedef struct yudp_addr_s
{
   unsigned long ip;
   unsigned short port;
} yudp_addr_t;

typedef struct yudp_peer_s
{
    int sock;

    pthread_t thread_id;
    int thread_run;

    y_time_t last_heart_time;
    y_time_t last_recv_time;

    int index_in_host;

    yudp_addr_t addr;
}yudp_peer_t;

#define YUDP_DEFAULT_SEND_COUNT            	3

#define YUDP_HOST_MAX_PEER_COUNT            4
#define YUDP_HOST_MAX_RECV_LENGTH           51200//50k.

#define YUDP_PEER_MAX_HEART_TIMEOUT         5000//MS
#define YUDP_PEER_MAX_HEART_INTERVAL        1000//MS

#define YUDP_MAX_ONE_SEND_RECV_LENGTH       64768//== 44*HRTP_FRAGMENT_SIZE

#define YUDP_MAX_SEND_RECV_LENGTH           270000//194304//== 3*44*HRTP_FRAGMENT_SIZE


typedef struct yudp_host_s
{
    int sock;

    pthread_t thread_id;
    int thread_run;

    yudp_addr_t addr;

    yudp_peer_t peers[YUDP_HOST_MAX_PEER_COUNT];
}yudp_host_t;

typedef struct hrtp_recv_event_s {
    int peer_id;        /* if 0, have normal data */
    char *data;         /* recv data buffer, length should be YUDP_HOST_MAX_RECV_LENGTH, point to "unsigned char rcv_buffer[YUDP_HOST_MAX_RECV_LENGTH];" */
    int data_len;       /* recv data length */
} hrtp_recv_event_t;



y_time_t yudp_gettime_ms(void);

void yudp_fill_addr(yudp_addr_t * addr, char *ip, int port);
yudp_host_t* yudp_host_create(const yudp_addr_t * addr);
void yudp_host_add_peer(yudp_host_t* host, yudp_peer_t* peer);
void yudp_host_destroy(yudp_host_t* host);
int yudp_host_monitor(yudp_host_t* host, hrtp_recv_event_t *recv_event, int timeout_ms);

int yudp_cmd_check(char* data, const int length);

yudp_peer_t* yudp_peer_create(const yudp_addr_t * addr);
int yudp_peer_connect(yudp_peer_t* peer, int timeout_sec);
int yudp_peer_disconnect(yudp_peer_t* peer);

int yudp_peer_send_heart(yudp_peer_t* peer);
void yudp_peer_destroy(yudp_peer_t* peer);
int yudp_peer_send(yudp_peer_t* peer, const char* data, const int length);
int yudp_peer_send_n(yudp_peer_t* peer, const char* data, const int length, int send_count);

int yudp_peer_recv_normal(yudp_peer_t* peer, char* data, int length, int timeout_sec);
int yudp_peer_recv(yudp_peer_t* peer, char* data, int length, int timeout_sec);

#if 0
int yudp_peer_recv_hrtp(yudp_peer_t* peer, hrtp_recombine_t* recombine, int timeout_sec);
int yudp_peer_analyse_fragment(hrtp_recombine_t* recombine, char* data, int length);
#endif


#endif /* _YUDP_H__ */
