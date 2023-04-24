/*!
*****************************************************************************
** FileName     : gk_cms_session.c
**
** Description  : api for cms session.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Date         : 2015-9-9, create.
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>

#include "gk_cms_common.h"
#include "gk_cms_media.h"
#include "gk_cms_sock.h"
#include "gk_cms_protocol.h"
#include "gk_cms_utility.h"
#include "gk_cms_pb.h"
#include "gk_cms_net_api.h"
#include "eventalarm.h"
#include "cfg_all.h"
#include "g711.h"
#include "sdk_network.h"
#include "sdk_sys.h"
#include "netcam_api.h"
#include "work_queue.h"



#define USE_FRAM_MALLOC    0
extern pthread_mutex_t gk_send_cmd_mutex;
//pthread_mutex_t gk_main_second_stream_mutex = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t g_gk_audio_mutex = PTHREAD_MUTEX_INITIALIZER;
//extern pthread_mutex_t gk_send_frame_mutex;


static int g_cms_pt_handle = 0;
static int g_cms_pthread_run = 0;
static int g_is_talking = 0;
pthread_mutex_t gk_talk_mutex = PTHREAD_MUTEX_INITIALIZER;

#define REC_PCM_FOR_TEST 0

#define BC_UPDATE_IP 0

#if BC_UPDATE_IP
struct sockaddr_in eth0_old;
struct sockaddr_in eth0_new;
struct sockaddr_in wifi_old;
struct sockaddr_in wifi_new;
#endif

int Gk_TalkLoop(int sock)
{
    PRINT_INFO("in Gk_TalkLoop\n");
    g_is_talking = 1;
    int aframe_size = 4096;
    char *buf = (char *)malloc(aframe_size);
    if (buf == NULL) {
        PRINT_ERR("malloc error.");
        return -1;
    }
    int recv_len = 0;
    int send_len = 0;
    int write_len = 0;

    #if REC_PCM_FOR_TEST
    FILE *fp = fopen("/tmp/audio_test.pcm", "wb+");
    if (fp == NULL)
    {
        PRINT_ERR("fopen error.\n");
        return -1;
    }

    int ret;
    PRINT_INFO("1, open ok.");
    int recv_no = 0;
    int recv_alllen = 0;
    #endif

    while (1) {
        #if 1
        recv_len = GkSockRecv(sock, buf, aframe_size);
        if (recv_len < 0) {
            PRINT_ERR("GkSockRecv error:%s, recv_len:%d buf_size:%d.", strerror(errno), recv_len, aframe_size);
            break;
        }
        #else
        recv_len = recv(sock, buf, aframe_size, 0);
        if (recv_len <= 0) {
            PRINT_ERR("GkSockRecv error:%s, recv_len:%d buf_size:%d.", strerror(errno), recv_len, aframe_size);
            break;
        }
        #endif

        #if REC_PCM_FOR_TEST
        recv_no ++;
        recv_alllen += recv_len;
        PRINT_INFO("recv_len: %d, no=%d, recv_alllen=%d\n", recv_len, recv_no, recv_alllen);

        ret = fwrite(buf, 1, recv_len, fp);
        if (ret <= 0)
        {
            PRINT_ERR("write error.\n");
            break;
        }
        #endif

        write_len = Gk_CmsWriteAudioData(buf, recv_len, 0);
        if (write_len < 0) {
            PRINT_ERR("GkWriteAudioData error, recv_len:%d write_len:%d.", recv_len, write_len);
            break;
        }

        #if 1
        send_len = GkSockSendTalk(sock, buf, recv_len, 3);
        if (send_len != recv_len) {
            PRINT_ERR("GkSockSendTalk error, recv_len:%d send_len:%d.", recv_len, send_len);
            break;
        }
        #endif
    }

    #if REC_PCM_FOR_TEST
    PRINT_INFO("2, write audio data ok.");
    if (fp)
    {
        fclose(fp);
        fp = NULL;
    }
    PRINT_INFO("3, end audio file ok.");
    #endif

    if (sock < 0)
        close(sock);

    if (buf)
        free(buf);
	#if REC_PCM_FOR_TEST
    write_audio_from_file();
    #endif
    g_is_talking = 0;

    return 0;
}

int Gk_Talk(int sock)
{
    if (g_is_talking == 1) {
        PRINT_INFO("cms is talking, please wait...\n");
        close(sock);
        return 0;
    }

    PRINT_INFO("in Gk_Talk...\n");

    NET_AUDIO_INFO audio_info;
    memset(&audio_info, 0, sizeof(NET_AUDIO_INFO));
    audio_info.nFlag = TMNVHISINET_FLAG;
    audio_info.nChannel = 1;
    audio_info.nBitRate = 16;
    audio_info.nSmapleRate = 8000;

    pthread_mutex_lock(&gk_talk_mutex);
    PRINT_INFO("call send talk resq\n");
    GkSockSendTalk(sock, (char *)&audio_info, sizeof(NET_AUDIO_INFO), 3);
    Gk_TalkLoop(sock);
    pthread_mutex_unlock(&gk_talk_mutex);

    return 0;
}


static int GkSessionLoop(int sock)
{
    //PRINT_INFO("in GkSessionThread\n");
    int ret = 0;

	//int error_times = 0;


    ACCEPT_HEAD t_accept_head = {0};
    ret = recv(sock, (char *)&t_accept_head, sizeof(ACCEPT_HEAD), 0);
    if (ret != sizeof(ACCEPT_HEAD)) {
        PRINT_ERR();
        close(sock);
        return -1;
    }

    #if 1
    //设置成非阻塞,可重用
    ret = SetSockAttr(sock);
    if (ret < 0) {
        PRINT_ERR();
        close(sock);
        return -1;
    }
    #else
    int opt = 1;
    ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (ret < 0) {
        PRINT_ERR("setsockopt SO_REUSEADDR failed, error:%s\n", strerror(errno));
        return -1;
    }
    #endif

    switch (t_accept_head.nSockType) {
        case SOCK_LOGON:
            PRINT_INFO("sock:%d for cmd\n", sock);
            ret = Gk_LogOn(sock);
            if (ret < 0) {
                PRINT_ERR("Gk_LogOn error.");
                close(sock);
                return -1;
            }
            GkDealCmdDispatch(sock);
            GkCloseMainSock(sock);
            break;
        case SOCK_DATA:
            PRINT_INFO("sock:%d for liveview\n", sock);
            GkHandleMediaRequest(sock);
            //GkMediaSend(session_socket);
            break;
        case SOCK_FILE:
            PRINT_INFO("sock:%d for playback\n", sock);
            Gk_Playback(sock);
            break;
        case SOCK_TALKBACK:
            PRINT_INFO("sock:%d for talk\n", sock);
            Gk_Talk(sock);
            break;
        default:
            PRINT_INFO("sock:%d for %lu\n", sock, t_accept_head.nSockType);
            break;
    }

    return 0;
}

static void * GkSessionThread(void *param)
{
    sdk_sys_thread_set_name("GkSessionThread");

    GkSessionLoop((int)param);

    return NULL;
}

static int GkListenLoop()
{
    int ret, listenfd, connectfd;    //socket文件描述符
    listenfd = CreateTcpSock(NULL, GK_CMS_TCP_LISTEN_PORT);
	if (listenfd < 0) {
		PRINT_ERR("create listen socket fail");
		return -1;
	}

    //设置成非阻塞,可重用
    ret = SetSockAttr(listenfd);
    if (ret < 0) {
        PRINT_ERR();
        return -1;
    }

    //循环监听，等待连接
    struct sockaddr_in addr;
    int sin_size;  //地址信息结构体大小
    struct timeval timeout;
    fd_set readfd; //读文件描述符集合
    while (1) {
        if (0 == g_cms_pthread_run) {
            PRINT_INFO("listen, g_cms_pthread_run is 0\n");
            break;
        }

        timeout.tv_sec = 3;  //超时时间为2秒
        timeout.tv_usec = 0;
        //文件描述符清0
        FD_ZERO(&readfd);
        //将套接字文件描述符加入到文件描述符集合中
        FD_SET(listenfd, &readfd);

        select(listenfd + 1, &readfd, NULL, NULL, &timeout);
        if (!FD_ISSET(listenfd, &readfd)) {
            usleep(100000);
            continue;
        }

        sin_size = sizeof(struct sockaddr_in);
        if ((connectfd = accept(listenfd, (struct sockaddr *)&addr, (socklen_t *) & sin_size)) == -1) {
            PRINT_ERR("Server: accept failed%s\n", strerror(errno));
            continue;
        }
        PRINT_INFO("accept session_socket:%d, remote ip:%s, remote port:%d\n",
                    connectfd, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

        //每有一个连接就创建一个会话线程
        CreateDetachThread(GkSessionThread, (void *)connectfd, NULL);

    }

    PRINT_INFO("GkListenLoop end\n");
    //关闭服务器监听socket
	if (listenfd > 0) {
    	close(listenfd);
		listenfd = -1;
	}
    GkCloseAllSock();
    return 0;
}

static void GkCmsInit()
{
    PRINT_INFO("GkCmsInit !\n");
    g_cms_pthread_run = 1;

    //初始化会话连接socket数组
	GkSessionGlobalInit();
    pthread_mutex_init(&gk_send_cmd_mutex, NULL);
    pthread_mutex_init(&gk_talk_mutex, NULL);

    //pthread_mutex_init(&gk_main_second_stream_mutex, NULL);
    //pthread_mutex_init(&gk_send_frame_mutex, NULL);
    //pthread_mutex_init(&g_gk_audio_mutex, NULL);
}

static void GkCmsUninit()
{
    PRINT_INFO("GkCmsUninit !\n");
    g_cms_pthread_run = 0;

	GkSessionGlobalUninit();
    pthread_mutex_destroy(&gk_send_cmd_mutex);
    pthread_mutex_destroy(&gk_talk_mutex);
    //pthread_mutex_destroy(&gk_main_second_stream_mutex);
    //pthread_mutex_destroy(&gk_send_frame_mutex);
    //pthread_mutex_destroy(&g_gk_audio_mutex);
}


int GkSendProcess(int stream_type)
{
	int ret = 0;

    //添加读用户
    MEDIABUF_HANDLE reader = NULL;

    void *frame_data = NULL;
    int frame_len = 0;
    GK_NET_FRAME_HEADER frame_header = {0};
    int try_times = 0;
    int have_audio = 1;
    int have_video = 1;
    int readIframe = 0;

    while (1) {
        if (0 == g_cms_pthread_run) {
            break;
        }

        readIframe = 0;
        //检查是否有客户端连接到cms，如果没有则关闭读者
        if(GkGetSessionStreamNumber(stream_type) == 0)
        {
            if(reader )
            {
                mediabuf_del_reader(reader);
                reader = 0;
                LOG_INFO("Delete reader for stream_%d\n",stream_type);
            }

            usleep(200*1000);
            continue;
        }
        else
        {
            if(!reader)
            {
                reader = mediabuf_add_reader(stream_type);
                if(reader == NULL)
                {
                    PRINT_ERR("cms add read failed stream%d\n", stream_type);
                    break;
                }
                LOG_INFO("Add reader for stream_%d\n",stream_type);

                //首次连接，先发送I帧
                readIframe = 1;
            }

        }
        // 外部请求强制I frame
        if(GkGetIFrameRequest(stream_type) )
        {
            readIframe = 1;
        }

        if(readIframe){

            frame_data = NULL;
            frame_len = 0;
            ret = mediabuf_read_I_frame(reader, (void **)&frame_data, &frame_len, &frame_header);
            if (ret < 0) {
    			PRINT_ERR("cms read stream%d : mediabuf_read_I_frame fail.\n", stream_type);
        		goto err;
            } else if (ret == 0) {
                PRINT_INFO("mediabuf_read_I_frame = 0\n");
                continue;
            } else {
                PRINT_INFO("cms read stream%d : mediabuf_read_I_frame OK.\n", stream_type);
            }

        } else {
            //PRINT_INFO("stream%d read normal.\n", stream_type);

            frame_data = NULL;
            frame_len = 0;
            ret = mediabuf_read_frame(reader, (void **)&frame_data, &frame_len, &frame_header);
            if (ret < 0) {
    			PRINT_ERR("cms read stream%d : mediabuf_read_frame fail.\n", stream_type);
        		goto err;
            } else if (ret == 0) {
                PRINT_INFO("mediabuf_read_frame = 0\n");
                continue;
            } else {
                //PRINT_INFO("cms read stream%d : mediabuf_read_frame OK.\n", stream_type);
            }
        }

        //PRINT_INFO("stream%d to send frame.\n", stream_type);
        if (frame_header.frame_type == GK_NET_FRAME_TYPE_A) {
            #if 0
            /* g711a -> pcm */
            unsigned char dst_buf[4096] = {0};
            int dst_len = 0;
            if (frame_len * 2 > sizeof(dst_buf)) {
                PRINT_ERR("audio bufsize error.\n");
                goto err;
            }
            dst_len = audio_alaw_decode((short *)dst_buf, frame_data, frame_len);

            if (have_audio == 1) {
                PRINT_INFO("******** mediabuf have audio, send to cms ************ \n");
                PRINT_INFO("stream%d, a alaw len:%d pcm len:%d\n", stream_type, frame_len, dst_len);
                have_audio = 0;
            }

            /* 发送每一帧数据 */
            //printf("333 stream%d  pcm len:%d, alaw len:%d headlen:%d\n", stream_type, dst_len, frame_len, frame_header.frame_size);
            frame_header.frame_size = dst_len;
            GkSendFrame(dst_buf, dst_len, &frame_header, stream_type);
            #else
            if (have_audio == 1) {
                PRINT_INFO("******** mediabuf have audio, send to cms ************ \n");
                PRINT_INFO("stream%d, pcm len:%d\n", stream_type, frame_len);
                have_audio = 0;
            }
    
            frame_header.frame_size = frame_len;
            GkSendFrame(frame_data, frame_len, &frame_header, stream_type);
            #endif
        } else {
            if (have_video == 1) {
                PRINT_INFO("******** mediabuf have video, send to cms ************ \n");
                PRINT_INFO("v len:%d\n", frame_len);
                have_video = 0;
            }

            /* 发送每一帧数据 */
            #if 0
            printf("333 stream%d no:%d %s frame len:%d headlen:%d\n", stream_type, frame_header.frame_no,
                  (frame_header.frame_type == GK_NET_FRAME_TYPE_I)? "I": "P",
                  frame_len, frame_header.frame_size);
            #endif
            GkSendFrame(frame_data, frame_len, &frame_header, stream_type);
        }

        //pthread_mutex_lock(&gk_main_second_stream_mutex);
		//pthread_mutex_unlock(&gk_main_second_stream_mutex);

err:
		/* 发送错误超过一定的次数后退出 */
		if (ret < 0) {
			try_times ++;
			if (try_times > 3) {
				PRINT_ERR();
				try_times = 0;
				break;
			}
		}
    }

    if(reader )
    {
        mediabuf_del_reader(reader);
        reader = 0;
    }
	PRINT_INFO("Send stream%d process end.\n", stream_type);
    return 0;
}

static void * GkSendStream0Thread(void *para)
{
    sdk_sys_thread_set_name("GkSendStream0Thread");

    GkSendProcess(0);

    PRINT_INFO("GkSendStream1Thread end !\n");
    return NULL;
}

static void * GkSendStream1Thread(void *para)
{
    sdk_sys_thread_set_name("GkSendStream1Thread");

	GkSendProcess(1);

    PRINT_INFO("GkSendStream1Thread end !\n");
    return NULL;
}



static void * GkMainThread(void *param)
{
    sdk_sys_thread_set_name("GkMainThread");

    // listen, accept, recv, send
    GkListenLoop();

    return NULL;
}

#if BC_UPDATE_IP
static int BoardCastUpdateIpToMGroup(int sock, char *inet_name, struct sockaddr_in *inet_old, struct sockaddr_in *inet_new)
{
    ST_SDK_NETWORK_ATTR netAttr;
    memset(&netAttr, 0, sizeof(ST_SDK_NETWORK_ATTR));
    strcpy(netAttr.name, inet_name);

    if(netcam_net_get(&netAttr) == 0) {
		inet_new->sin_addr.s_addr = inet_addr(netAttr.ip);
        //PRINT_INFO("%s ip:%s \n", inet_name, netAttr.ip);
		if (inet_old->sin_addr.s_addr != inet_new->sin_addr.s_addr) {

        	/* use setsockopt() to request that the kernel join a multicast group */
            struct ip_mreq mreq;
			memset(&mreq, 0, sizeof(struct ip_mreq));
        	mreq.imr_multiaddr.s_addr = inet_addr(GK_CMS_MC_GROUP_IP);
			mreq.imr_interface.s_addr = inet_old->sin_addr.s_addr;
			if(setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&mreq, sizeof(mreq)) < 0) {
				PRINT_ERR("%s setsockopt drop %s from group error: %s\n", inet_name, inet_ntoa(inet_old->sin_addr), strerror(errno));
                return -1;
            } else {
                PRINT_INFO("%s Success to drop %s from multicast[%s]\n", inet_name, inet_ntoa(inet_old->sin_addr), GK_CMS_MC_GROUP_IP);
            }

			memset(&mreq, 0, sizeof(struct ip_mreq));
			mreq.imr_multiaddr.s_addr = inet_addr(GK_CMS_MC_GROUP_IP);
			mreq.imr_interface.s_addr = inet_new->sin_addr.s_addr;	//htonl(INADDR_ANY)
        	if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        		PRINT_ERR("%s setsockopt add %s to group error: %s\n", inet_name, inet_ntoa(inet_new->sin_addr), strerror(errno));
        		return -1;
        	} else {
                PRINT_INFO("%s Success to add %s to multicast[%s]\n", inet_name, inet_ntoa(inet_new->sin_addr), GK_CMS_MC_GROUP_IP);
            }

            int msgLoop = 0;
			if(setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP, (void *)&msgLoop, sizeof(msgLoop)) < 0) {
				PRINT_ERR("%s IP_MULTICAST_LOOP error", inet_name);
				return -1;
			}

            memcpy(inet_old, inet_new, sizeof(struct sockaddr_in));// strore new ip.
        }
    }

    return 0;
}

static int BoardCastCheckIp(int sock)
{
    ST_SDK_NETWORK_ATTR net_attr;
    memset(&net_attr, 0, sizeof(ST_SDK_NETWORK_ATTR));

    int ret = netcam_net_get_detect(runNetworkCfg.lan.netName);
    if (ret != 0) {
        //PRINT_INFO("eth0 is disconnect or disable, use wifi.");
        /* check wifi */
        BoardCastUpdateIpToMGroup(sock, runNetworkCfg.wifi.netName, &wifi_old, &wifi_new);
    } else {
        //PRINT_INFO("eth0 is ok.");
        /* check lan */
        BoardCastUpdateIpToMGroup(sock, runNetworkCfg.lan.netName, &eth0_old, &eth0_new);
    }

    return 0;
}
#endif


int cms_notify_alarm_md()
{
    #if 0
    static int last_md_sec = 0;
    struct timeval t1 = {0, 0};
    // 3秒内，不重复响应同一种报警
    int interval = runMdCfg.handle.intervalTime;
    if (interval <= 0 || interval > 60) {
        //PRINT_INFO("interval %d set to 3 seconds\n", interval);
        interval = 3;
    }
    //PRINT_INFO("interval %d\n", interval);
    gettimeofday(&t1, NULL);
    if ((t1.tv_sec - last_md_sec) <= interval) {
        //PRINT_INFO("time is too closed, so not to notify.\n");
        return 0;
    }
    last_md_sec = t1.tv_sec;
    #endif

    cms_notify_to_all_client(JB_MSG_MOVEDETECT);

    return 0;
}


int cms_alarm_cb_func(int nChannel, int nAlarmType, int nAction, void* pParam)
{
    //PRINT_INFO("nAlarmType:%d, nAction:%d\n", nAlarmType, nAction);

	if(nAction == 0)
	{
		//PRINT_INFO("\n");
		return 0;
	}

    char eventType[20] = {0};
	char strAction[20] = {0};

    #if 0
    long ts = time(NULL);
    struct tm tt = {0};
    struct tm *pTm = localtime_r(&ts, &tt);
    char startTime[128] = {0};
    sprintf(startTime,"%d-%02d-%02d %02d:%02d:%02d",pTm->tm_year+1900,pTm->tm_mon+1,pTm->tm_mday,pTm->tm_hour,pTm->tm_min,pTm->tm_sec);
    #endif

    switch(nAlarmType) {
        case GK_ALARM_TYPE_ALARMIN:     //0:信号量报警开始
        	sprintf(eventType ,"%s", "LocalIO");
			sprintf(strAction ,"%s", "Start");
			break;

        case GK_ALARM_TYPE_DISK_FULL:       //1:硬盘满
			sprintf(eventType ,"%s", "StorrageLowSpace");
			break;

        case GK_ALARM_TYPE_VLOST:            //2:信号丢失
        	sprintf(eventType ,"%s", "VideoLoss");
			sprintf(strAction ,"%s", "Start");
			break;

        case GK_ALARM_TYPE_VMOTION:          //3:移动侦测
			sprintf(eventType ,"%s", "VideoMotion");
			sprintf(strAction ,"%s", "Start");
            cms_notify_alarm_md();
            break;
        case GK_ALARM_TYPE_DISK_UNFORMAT:    //4:硬盘未格式化
			sprintf(eventType ,"%s", "StorageNotExist");
			break;

        case GK_ALARM_TYPE_DISK_RWERR:       //5:读写硬盘出错,
			sprintf(eventType ,"%s", "StorageFailure");
			break;

        case GK_ALARM_TYPE_VSHELTER:         //6:遮挡报警
			sprintf(eventType ,"%s", "VideoBlind");
			break;

        case GK_ALARM_TYPE_ALARMIN_RESUME:         //9:信号量报警恢复
			sprintf(eventType ,"%s", "LocalIO");
			sprintf(strAction ,"%s", "Stop");
			break;

        case GK_ALARM_TYPE_VLOST_RESUME:         //10:视频丢失报警恢复
			sprintf(eventType ,"%s", "VideoLoss");
			sprintf(strAction ,"%s", "Stop");
			break;

        case GK_ALARM_TYPE_VMOTION_RESUME:         //11:视频移动侦测报警恢复
			sprintf(eventType ,"%s", "VideoMotion");
			sprintf(strAction ,"%s", "Stop");
			break;

        case GK_ALARM_TYPE_NET_BROKEN:       //12:网络断开
			sprintf(eventType ,"%s", "NetAbort");
			break;

        case GK_ALARM_TYPE_IP_CONFLICT:      //13:IP冲突
			sprintf(eventType ,"%s", "IPConfict");
			break;

        default:
			PRINT_INFO("Alarm: Type = %d", nAlarmType);
        	break;
    }

    //PRINT_INFO("alarm_type:%s action:%s\n", eventType, strAction);

    return 0;
}


int gk_cms_start()
{
    PRINT_INFO("========================= Entry gk_cms_start \n");

    g_cms_pt_handle = (int)event_alarm_open(cms_alarm_cb_func);


    GkCmsInit();

    //创建主线程
    CreateDetachThread(GkMainThread, NULL, NULL);

    //创建发送线程
    CreateDetachThread(GkSendStream0Thread, NULL, NULL);
    CreateDetachThread(GkSendStream1Thread, NULL, NULL);

	//30秒后初始化定时抓拍
	//CREATE_WORK(InitTimeSnap, EVENT_TIMER_WORK, GkInitTimeSnap);
	//INIT_WORK(InitTimeSnap, COMPUTE_TIME(0,0,0,30,0), 0);
	//SCHEDULE_DEFAULT_WORK(InitTimeSnap);
	CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,30,0), GkInitTimeSnap);
    return 0;
}


int gk_cms_stop()
{
    //int ret = 0;
    GkCmsUninit();

	event_alarm_close((void*)g_cms_pt_handle);

    PRINT_ERR("lib_gk_close bye bye bye bye bye11111 \n");
    return 0;
}


