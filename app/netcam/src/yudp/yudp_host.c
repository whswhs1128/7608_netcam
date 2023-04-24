
/*
 * yudp for goke platform api.
 *
 * Author : Heyong
 * Date   : 2015.12.2
 * Copyright (c) 2015 Harric He(Heyong)
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#include "sdk_api.h"
#include "sdk_enc.h"
#include "cfg_com.h"
#include "media_fifo.h"
#include "common.h"
#include "netcam_api.h"

#include "yudp_utility.h"
#include "hrtp.h"
#include "yudp.h"


typedef struct hrtp_peer_thrparam_s {
    int peer_id;        /* peer id */
    yudp_host_t* host;  /* host */
    void *mediabuf_handle;
    int now_stream_id;
    int new_stream_id;
} hrtp_peer_thrparam_t;

static hrtp_peer_thrparam_t   g_peers[YUDP_HOST_MAX_PEER_COUNT];

static void peers_init()
{
    int i;
    for(i = 0; i < YUDP_HOST_MAX_PEER_COUNT; i ++)
    {
        g_peers[i].host = NULL;
        g_peers[i].peer_id = -1;
        g_peers[i].mediabuf_handle = NULL;
        g_peers[i].now_stream_id = 0;//default is GK_NET_STREAM_TYPE_STREAM0
        g_peers[i].new_stream_id = 0;
    }
}

static void peer_deinit(int peer_id)
{
    g_peers[peer_id].host = NULL;
    g_peers[peer_id].peer_id = -1;
    g_peers[peer_id].mediabuf_handle = NULL;
    g_peers[peer_id].now_stream_id = 0;//default is GK_NET_STREAM_TYPE_STREAM0
    g_peers[peer_id].new_stream_id = 0;
}

static int set_usable_peerid(int peer_id, yudp_host_t* host)
{
    int i;
    for(i = 0; i < YUDP_HOST_MAX_PEER_COUNT; i ++)
    {
        if(g_peers[i].peer_id != -1)
            continue;
        else
        {
            g_peers[i].host = host;
            g_peers[i].peer_id = peer_id;
        }
        return i;
    }
    return -1;
}

static int peer_set_stream_id(int id, int new_stream_id)
{
    int i;
    for(i = 0; i < YUDP_HOST_MAX_PEER_COUNT; i ++)
    {
        if(g_peers[i].peer_id == id)
        {
            g_peers[i].new_stream_id = new_stream_id;
        }
    }
    return -1;

}

void *host_sendto_peer(void *param)
{
    int client_id = ((int)param);

    yudp_host_t* host = g_peers[client_id].host;
    int peer_id = g_peers[client_id].peer_id;

    printf("host_sendto_peers %d-%d in\n", peer_id, client_id);

    char *dataBuffer ;
    int n = 0;
    int size;
    GK_NET_FRAME_HEADER frame_header = {0};

	//GK_NET_STREAM_TYPE_STREAM1  0~3
    g_peers[client_id].mediabuf_handle = mediabuf_add_reader(g_peers[client_id].now_stream_id);
    mediabuf_set_newest_frame(g_peers[client_id].mediabuf_handle);

    hrtp_packet_t* packet = hrtp_packet_create(200000);
    int findIFrame = 0;
    while(host->peers[peer_id].thread_run)
    {
        if(g_peers[client_id].now_stream_id != g_peers[client_id].new_stream_id)
        {
            //delete old reader
            mediabuf_del_reader(g_peers[client_id].mediabuf_handle);

            // create new reader
            g_peers[client_id].mediabuf_handle = mediabuf_add_reader(g_peers[client_id].new_stream_id);
            printf("steam id changed,from %d to %d\n",g_peers[client_id].now_stream_id, g_peers[client_id].new_stream_id);
            g_peers[client_id].now_stream_id = g_peers[client_id].new_stream_id;

            mediabuf_set_newest_frame(g_peers[client_id].mediabuf_handle);
            findIFrame = 0;
        }

		dataBuffer = NULL;
        size = 0;
        n = mediabuf_read_frame(g_peers[client_id].mediabuf_handle, (void **)&dataBuffer, &size, &frame_header);
        if (n <= 0)
        {
            printf("read no data.\n");
            usleep(30000);
            continue;
        }

        if(findIFrame == 0)
        {
            if(frame_header.frame_type == GK_NET_FRAME_TYPE_I)
            {
                findIFrame = 1;
            }
            else
            {
                continue;
            }
        }

        if(host->peers[peer_id].sock != -1)
        {
			if (frame_header.frame_type == GK_NET_FRAME_TYPE_A)
			{
				hrtp_packet_convert_data(packet, dataBuffer, size, HRTP_TYPE_AUDIO);
			}
			else if(frame_header.frame_type == GK_NET_FRAME_TYPE_I)
			{
				hrtp_packet_convert_data(packet, dataBuffer, size, HRTP_TYPE_VEDIO_I);
			}else{
				hrtp_packet_convert_data(packet, dataBuffer, size, HRTP_TYPE_VEDIO);
			}
            yudp_peer_send(&host->peers[peer_id], packet->fms_data, packet->fms_data_length);
        }
    }

    hrtp_packet_destroy(packet);
    mediabuf_del_reader(g_peers[client_id].mediabuf_handle);

    peer_deinit(client_id);
    printf("host_sendto_peers %d out\n", peer_id);
    return NULL;
}

void *host_monitor(void *param)
{
    printf("host_monitor in\n");

    yudp_host_t* host = (yudp_host_t*)param;
    unsigned char rcv_buffer[YUDP_HOST_MAX_RECV_LENGTH];
    hrtp_recv_event_t recv_event;
    recv_event.data = (char *)rcv_buffer;

    int ret = 0;
    while(host->thread_run)
    {
        ret = yudp_host_monitor(host, &recv_event, 1000);
        if(ret < 0)
        {
            continue;
        }

        if(recv_event.peer_id != -1 && recv_event.data_len == 0)//peer connect.
        {
            int client_id = set_usable_peerid(recv_event.peer_id, host);
            pthread_create(&host->peers[recv_event.peer_id].thread_id, NULL, host_sendto_peer, (void*)client_id);
            continue;
        }

        if(recv_event.peer_id != -1 && recv_event.data_len != 0)
        {
            int streamId = recv_event.data[0];
            if((streamId >= 0) && (streamId <= 4) && (recv_event.data_len == 1))
            {
                peer_set_stream_id(recv_event.peer_id, streamId);
                printf("peer (%d) recv data len = %d, sid = %d\n", recv_event.peer_id, recv_event.data_len, streamId);
            }else
            {
                //OTHER DATA.
            }
        }
    }
    printf("host_monitor out\n");
    return NULL;
}

static yudp_host_t* g_host = NULL;
int netcam_yudp_start()
{
    if(g_host != NULL)
    {
        return -1;
    }

	yudp_addr_t address;
	yudp_fill_addr(&address, NULL, YUDP_SERVER_PORT);

    peers_init();

	g_host = yudp_host_create(&address);
    pthread_create(&g_host->thread_id, NULL, host_monitor, (void*)g_host);
    return 0;
}

int netcam_yudp_stop()
{
    g_host->thread_run = 0;
    pthread_join(g_host->thread_id, NULL);
    yudp_host_destroy(g_host);
    return 0;
}
