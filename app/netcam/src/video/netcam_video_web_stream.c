#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/tcp.h>
#include <sys/syscall.h>
#include "sdk_sys.h"

#include "media_fifo.h"

#include "netcam_api.h"

#define RECV_MAX_LEN    2048
#define TCP_SERVER_PORT   1234

struct client_status_s{
    int sockfd;
    void *data_handle;
    int buf_id;
    int stream_id;
    int client_status;
    pthread_t thHandle;
    int tid;
};

typedef struct
{
    int bufId;
    int streamId;
    int maxStreamNum;
    int streamNum;
}VIDEO_STREAM_Porpety;


/****************************************
*
*  for AVTProtocol BEGIN, add by heyong
*
****************************************/

#define GKAVTP_TYPE_DATA_AUDIO	0
#define GKAVTP_TYPE_DATA_H264	1
#define GKAVTP_TYPE_CMD			2
#define GKAVTP_TYPE_DATA_H265	3


typedef struct gk_avtp_header{
    unsigned char signature[4];    //!< Image signature: value is "AVTP".
    unsigned int type;             //!< Payload type. 0: DATA_AUDIO, 1: DATA_VIDEO, 2: CMD.
    union
    {
        unsigned int cmd;          //!< The CMD value (CMD).
        unsigned int length;       //!< Payload length in this packet (DATA).
    };
}GK_AVTP_HEADER;

#pragma pack(1)
typedef struct gk_avtp_packet{
    GK_AVTP_HEADER header;
    unsigned char data[1];
}GK_AVTP_PACKET;
#pragma pack()

static VIDEO_STREAM_Porpety   videoStreamPro[4];
static struct client_status_s *client_s;
static int   maxConstreamNum = 0;
static pthread_t videoServerPid;
static fd_set fd_reads;
static int maxfd;
static int videoWebRunFlag = 1;

static void *tcp_send_video_proc(void *arg);
static int  video_web_get_bufId_by_steam_id(int streamId);
static void video_web_release_bufId_by_stream_id(int streamId);
static int  video_web_get_stream_num_by_stream_id(int streamId);
static void video_web_add_streamNum_by_stream_id(int streamId);

static pid_t debug_gettid()
{
     return syscall(SYS_gettid);
}

static void tcp_select_init()

{
    FD_ZERO(&fd_reads);
}
static void tcp_select_add(int socket)
{
    FD_SET(socket, &fd_reads);
    if(maxfd < socket)
        maxfd = socket;
}
static void tcp_select_del(int socked)
{
    FD_CLR(socked, &fd_reads);
    if(socked  == maxfd)
        --maxfd;
}

/*
output:
    succes:return 0,and sockfd
    fail:return -1,and set sockfd is -1
*/
static int tcp_create_server(int *fd)
{
    int sockfd = -1;
    int opt;
    struct sockaddr_in server_addr;

    if(NULL == fd)
    {
        PRINT_ERR("*fd is NULL");
        return -1;
    }
    *fd = -1;
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        PRINT_ERR("create tcp server error");
        return -1;
    }
    bzero(&server_addr, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    //server_addr.sin_addr.s_addr = inet_addr("192.168.10.5");//htonl(INADDR_ANY);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(TCP_SERVER_PORT);
    //����IP���ã�����bindʱ����Address already in use
    opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if(bind(sockfd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) < 0)
    {
        PRINT_ERR("tcp server bind error");
        return -1;
    }
    if(listen(sockfd, maxConstreamNum) < 0)
    {
        PRINT_ERR("tcp server listen error");
        return -1;
    }
    *fd = sockfd;
    return 0;
}


/**************
return valu:
-1:fail
0:success
**************/
static int tcp_add_client(int client_sock, int client_id)
{
    struct timeval timeout = {1, 0};

    PRINT_INFO("add client num :%d\n", client_id);
    if((setsockopt(client_sock, SOL_SOCKET, SO_SNDTIMEO, (void*)&timeout, sizeof(struct timeval)))  ||
            (setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, (void*)&timeout, sizeof(struct timeval))))
        return -1;

    tcp_select_add(client_sock);
    client_s[client_id].sockfd = client_sock;
    client_s[client_id].data_handle = 0;
    client_s[client_id].buf_id = -1;
    client_s[client_id].client_status = 1;
    client_s[client_id].stream_id = -1;
    return 0;
}
void tcp_destroy_client_recod(struct client_status_s *client, int id)
{
   // PRINT_INFO("sock id :%d, buf id:%d, stream:%d\n", client->sockfd,client->stream_id,client->stream_id);
    tcp_select_del(client->sockfd);

    video_web_release_bufId_by_stream_id(client->stream_id );

    //PRINT_INFO("destroy start sock id :%d,buf id:%d,stream:%d,stream num:%d,\n", client->sockfd,client->buf_id,client->stream_id,video_web_get_stream_num_by_stream_id(client->stream_id));

    client->client_status = -1;

    if(client->thHandle != 0)
    {
        pthread_join(client->thHandle,NULL);
        PRINT_INFO("destroy thread :tid %d,sock id:%d,buf id:%d,stream:%d,stream num:%d ok\n", client->tid,client->sockfd,client->buf_id,client->stream_id,video_web_get_stream_num_by_stream_id(client->stream_id));
    }
    else
    {
        PRINT_INFO("No http video thread destroy:tid %d,sock id:%d,buf id:%d,stream:%d,stream num:%d ok\n", client->tid,client->sockfd,client->buf_id,client->stream_id,video_web_get_stream_num_by_stream_id(client->stream_id));
    }
    close(client->sockfd);
    if(client->data_handle != 0)
    {
        //MEDIABUF_close_byhandle(client->data_handle);
        mediabuf_del_reader(client->data_handle);
    }
    client->sockfd = -1;
    client->client_status = -1;
    client->buf_id = -1;
    client->thHandle = 0;
    client->data_handle = 0;
    client->stream_id = -1;
    client->tid = -1;
    PRINT_INFO("destroy end");
}
void tcp_init_client()
{
    int j;
    client_s = malloc(maxConstreamNum*sizeof(struct client_status_s));
    for(j = 0; j < maxConstreamNum; j ++)
    {
        client_s[j].sockfd = -1;
        client_s[j].data_handle = 0;
        client_s[j].buf_id = -1;
        client_s[j].client_status = 0;
        client_s[j].thHandle = 0;
        client_s[j].stream_id = -1;
    }
}
/**************
return valu:
-1:fail
>= 0:success
**************/
int tcp_client_usable_id()
{
    int i;

    for(i = 0; i < maxConstreamNum; i ++)
    {
        if(client_s[i].sockfd != -1)
            continue;
        else
            return i;
    }
    return -1;
}
static void *tcp_video_server_proc(void *arg)
{
    sdk_sys_thread_set_name("tcp_video_server");
    int server_sock, client_sock;
    int ret, addr_size, i, usable_id;
    fd_set select_fds;
    struct timeval timeout;
    struct sockaddr_in client_addr;
    unsigned char rcv_buffer[RECV_MAX_LEN];

    printf("Create tcp_video_server_proc  :tid %d\n",debug_gettid());
    tcp_init_client();
    //init select fd_reads
    tcp_select_init();
    signal(SIGPIPE, SIG_IGN);
    if(tcp_create_server(&server_sock) < 0)
    {
        PRINT_ERR("create tcp server error");
        return (void *)-1;
    }
    tcp_select_add(server_sock);
    //PRINT_INFO("tcp server fd:%d, port:%d\n", server_sock, TCP_SERVER_PORT);


    //create video send thread
    videoWebRunFlag = 1;
    maxfd = server_sock;

    while(videoWebRunFlag)
    {
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        select_fds = fd_reads;
        ret = select(maxfd + 1, &select_fds, NULL, NULL, &timeout);
        if(ret < 0)
        {
            //if (ret != EINTR && ret != EAGAIN)
            {
                PRINT_ERR("select error");
                return (void *)-1;
            }
        }

        //tcp server accept
        if(FD_ISSET(server_sock, &select_fds))
        {
            addr_size = sizeof(client_addr);

            client_sock = accept(server_sock, (struct sockaddr *)&client_addr, (socklen_t *)&addr_size);
            if(client_sock < 0)
            {
                PRINT_ERR("accept error");
                continue;
            }

            if(((usable_id = tcp_client_usable_id()) >= 0) && (!tcp_add_client(client_sock, usable_id)))
            {
                PRINT_INFO("new socket fd:%d\n", client_sock);
            }else{
                close(client_sock);
            }
        }
        //tcp server recv data form client
        for(i = 0; i < maxConstreamNum; i ++)
        {
            if(client_s[i].sockfd == -1)
                continue;
            if(FD_ISSET(client_s[i].sockfd, &select_fds))
            {
                //recv data;
                //AVTP check
                if((ret = recv(client_s[i].sockfd, rcv_buffer, sizeof(GK_AVTP_HEADER), MSG_PEEK))>= 1)
                {
                    GK_AVTP_HEADER *recvHdr = (GK_AVTP_HEADER *)rcv_buffer;
                    if(recvHdr->type == 2)
                    {
                        //destory head
                        if((ret = recv(client_s[i].sockfd, rcv_buffer, sizeof(GK_AVTP_HEADER), 0))< 1)
                        {
                            PRINT_ERR("client recv error ret:%d, error:%s", ret, strerror(errno));
                            tcp_destroy_client_recod(&client_s[i], i);
                            continue;
                        }
                        //voice talk
                        if((ret = recv(client_s[i].sockfd, rcv_buffer, recvHdr->length, 0))< 1)
                        {
                            PRINT_ERR("client recv error ret:%d, error:%s", ret, strerror(errno));
                            tcp_destroy_client_recod(&client_s[i], i);
                            continue;
                        }
                        else
                        {
                            // TODO : Start Talk Thread
                            //netcam_audio_output(rcv_buffer, ret, NETCAM_AUDIO_ENC_RAW_PCM, SDK_TRUE);		xqq
                        }
                    }
                    else
                    {
                        //video stream add or change
                        if((ret = recv(client_s[i].sockfd, rcv_buffer, 1, 0))< 1)
                        {
                            PRINT_ERR("client recv error ret:%d, error:%s", ret, strerror(errno));
                            tcp_destroy_client_recod(&client_s[i], i);
                            continue;
                        }
                        else
                        {
                            int bufId;
                            rcv_buffer[0] &=0x03;
                            bufId =  video_web_get_bufId_by_steam_id(rcv_buffer[0]);
                            PRINT_INFO("client conncet stream id :%d,buf id:%d", rcv_buffer[0],bufId);
                            if(bufId == -1)
                            {
                                tcp_destroy_client_recod(&client_s[i], i);
                                PRINT_INFO("Unvalid bufid ,close it");
                            }
                            else
                            {
                                client_s[i].buf_id = bufId;
                                client_s[i].stream_id = rcv_buffer[0];

                                if(client_s[i].client_status != 2)
                                {
                                    client_s[i].client_status = 2;
                                    video_web_add_streamNum_by_stream_id(rcv_buffer[0]);
                                    ret = pthread_create(&client_s[i].thHandle, NULL, tcp_send_video_proc, (void *)i);
                                    if(ret == 0)
                                    {
                                        PRINT_INFO("bufid is ok, media buffer is ok,begin send data");
                                    }
                                    else
                                    {
                                        PRINT_ERR("Create Web stream thread error:%s",strerror(errno));
                                    }
                                }
                            }
                        }
                    }
                }
            }
            //PRINT_INFO("i:%d, fd:%d, status:%d", i, client_s[i].sockfd,client_s[i].client_status);
            if((client_s[i].sockfd > 0) && (client_s[i].client_status == -1))
            {
                PRINT_INFO("del client %d", client_s[i].sockfd);
                tcp_destroy_client_recod(&client_s[i], i);
            }
        }
    }
    return (void *)0;
}



void fillAVTPPacket(GK_AVTP_PACKET *packet, int type, int length)
{
    packet->header.signature[0] = 'A';
    packet->header.signature[1] = 'V';
    packet->header.signature[2] = 'T';
    packet->header.signature[3] = 'P';

    packet->header.type = type;
    packet->header.length = length;
}



/****************************************
*
*  for AVTProtocol END, add by heyong
*
****************************************/
#define USE_NONBLOCK_SOCK 0
#if USE_NONBLOCK_SOCK
typedef enum
{
    WF_READ,
    WF_WRITE
}WAIT_EVENT;

static int time_wait(int sock, WAIT_EVENT e, int sec, int usec, int times)
{
	int ret = 0;
	fd_set fset;
	struct timeval tv;

	while(times > 0)
	{
		FD_ZERO(&fset);
		FD_SET(sock, &fset);

		tv.tv_sec = sec;
		tv.tv_usec = usec;

		if (e == WF_READ)
			ret = select(sock + 1, &fset, NULL, NULL, &tv);
		else
			ret = select(sock + 1, NULL, &fset, NULL, &tv);

		if (ret > 0)
			goto finish;

		if (ret <= 0)
		{
			if (errno != EINTR && errno !=EAGAIN)
				goto error;       //other error

			times--;               //time out or EINTR
		}
	}

error:
	printf("select %s [time_wait]! fd[%d] times[%d] errno[%d] errinfo[%s]\n",
		errno == 0 ? "timeout" : "error", sock,
		times, errno, strerror(errno));

finish:
	return ret;
}

static int write_i(int fd, char* buffer, int length)
{
	int bytes_left = length;
	int written = 0;
	int n, ret = 0;
	char* ptr = buffer;

	while (bytes_left > 0)
	{
		n = write(fd, ptr, bytes_left);
		if (n < 0)
		{
			if(errno == EINTR || errno == EAGAIN)
			{
				//wait 200ms every time
				ret = time_wait(fd, WF_WRITE, 5, 0, 5);
				if (ret > 0)
					continue;
				else if (ret == 0)
				{
					printf("no data to write[write_i]! fd[%d] n[%d] ret[%d] "
						"length[%d] written[%d] bytes_left[%d] errno[%d] errinfo[%s]\n",
						fd, n, ret, length, written, bytes_left, errno, strerror(errno));
					break;
				}
				else
				{goto error;}
			}
			else
				{ret = 1; goto error;}
		}

		bytes_left -= n;
		written += n;
		ptr += n;
	}

	return written;

error:
	printf("write error[write_i]! fd[%d] n[%d] ret[%d] "
		"length[%d] written[%d] bytes_left[%d] errno[%d] errinfo[%s]\n",
		fd, n, ret, length, written, bytes_left, errno, strerror(errno));

	return -1;
}

static int set_sock(int fd)
{
    int opt = 1;
    //set reuse addr
    int ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if(ret < 0)
	{
        PRINT_ERR("setsockopt SO_REUSEADDR failed, error:%s\n", strerror(errno));
        return -1;
    }

    //set no block
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
	{
		PRINT_ERR("Fail to get old flags [set_nonblocking]! errno[%d] errinfo[%s]\n",
			errno, strerror(errno));
		return -1;
	}
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		PRINT_ERR("Fail to set flags to O_NONBLOCK [set_nonblocking]! errno[%d] errinfo[%s]\n",
			errno, strerror(errno));
		return -1;
	}

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

    // set nodelay
    int on = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void *)&on, sizeof(on));
	return 0;
}
#else
static int socket_send_data(int socket, char *data, int len)
{
    int ret = -1;
    int count = 0;
    int sendLen = 0;
    /* 5*100= 500ms*/
    while(count < 100 && (ret < 0 || sendLen < len))
    {
         ret = send(socket, data+sendLen, len-sendLen, 0);
         if (ret <= 0)
         {
            count++;
            if(errno == EINTR)
            {
                //PRINT_INFO(" send error1:%s\n",strerror(errno));
            }
            else if(errno == EAGAIN)
            {
                //PRINT_INFO(" send error2:%s\n",strerror(errno));
                usleep(5000);
            }
            else
            {
                PRINT_INFO(" send error3:%s\n",strerror(errno));
                count = 500;
            }
         }
         else
         {
            sendLen += ret;
            if(sendLen != len)
            {
                count++;
            }
         }

    }
    if(sendLen != len)
    {
        PRINT_INFO(" send data len error: send %d, %d,count:%d\n",sendLen , len,count);
    }else if(count > 1)
    {
        //PRINT_INFO(" send data len %d ok,count:%d\n",sendLen, count);
    }
    return ret;
}
#endif

static int64_t av_gettime_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)tv.tv_sec * 1000 + (int64_t)tv.tv_usec / 1000;
}

static void *tcp_send_video_proc(void *arg)
{
    sdk_sys_thread_set_name("tcp_send_video");
    int client_id = ((int)arg);
    int bufId ;
    int streamId;
    //MEDIA_BUF_S media;
    int readlen;
    int on = 1;
    GK_AVTP_PACKET packet;
    int ret;
    //frame_head_t  *fifoHead;
    int findIFrame = 0;
    char *dataBuf;
    int data_size = 0;
    GK_NET_FRAME_HEADER header = {0};
    char alaw_data[4096];
    int alaw_len;
    //pthread_detach(pthread_self());

#if USE_NONBLOCK_SOCK
    set_sock(client_s[client_id].sockfd);
#else
    int  size = 204800;
    setsockopt(client_s[client_id].sockfd, SOL_SOCKET, SO_SNDBUF, (char *)&size, sizeof(size));
    PRINT_INFO("Socket: set the tcp send buffer size to %d\n",size);

    // set nodelay
    setsockopt(client_s[client_id].sockfd, IPPROTO_TCP, TCP_NODELAY, (void *)&on, sizeof(on));
#endif

    client_s[client_id].tid = debug_gettid();
    streamId =  client_s[client_id].stream_id;
    bufId = client_s[client_id].buf_id;
    client_s[client_id].data_handle = mediabuf_add_reader(client_s[client_id].buf_id);
    mediabuf_set_newest_frame(client_s[client_id].data_handle);

    while(client_s[client_id].client_status == 2 && client_s[client_id].data_handle != 0)
    {
        if(bufId != client_s[client_id].buf_id)
        {
            //delete old reader
            mediabuf_del_reader(client_s[client_id].data_handle);

            video_web_release_bufId_by_stream_id(streamId);
            video_web_add_streamNum_by_stream_id(client_s[client_id].stream_id);

            // create new reader
            client_s[client_id].data_handle = mediabuf_add_reader(client_s[client_id].buf_id);
            PRINT_INFO("steam id changed,from %d to %d",bufId,client_s[client_id].buf_id);
            bufId = client_s[client_id].buf_id;
            streamId =  client_s[client_id].stream_id;

            mediabuf_set_newest_frame(client_s[client_id].data_handle);
            findIFrame = 0;
        }
        data_size = 0;
        dataBuf = NULL;

        readlen = mediabuf_read_frame(client_s[client_id].data_handle, (void **)&dataBuf, &data_size, &header);
        if((readlen > 0))
        {
            if(findIFrame == 0)
            {
                if(header.frame_type == GK_NET_FRAME_TYPE_I)
                {
                    findIFrame = 1;
                }
                else
                {
                    continue;
                }
            }

            if(data_size == 0)
            {
                PRINT_INFO(">>>>>>>>>>>>>>data_size = 0, maybe wrong, just continue.\n");
                continue;
            }

            if(header.frame_type == GK_NET_FRAME_TYPE_A)
            {
                //pcm->alaw
                if(data_size / 2 > sizeof(alaw_data))
                {
                    PRINT_ERR("data_size[%d] error\n", data_size);
                    continue;
                }
                alaw_len = audio_alaw_encode(alaw_data, dataBuf, data_size);
                dataBuf = alaw_data;
                fillAVTPPacket(&packet, GKAVTP_TYPE_DATA_AUDIO, alaw_len);
                data_size = alaw_len;
                //continue;
            }
            else
            {
                if(header.media_codec_type)
                {
                    fillAVTPPacket(&packet, GKAVTP_TYPE_DATA_H265, data_size);
                }
                else
                {
                    fillAVTPPacket(&packet, GKAVTP_TYPE_DATA_H264, data_size);
                }
            }

#if USE_NONBLOCK_SOCK
            ret = write_i(client_s[client_id].sockfd, (char *)&packet, sizeof(packet)-1);
#else
            ret = socket_send_data(client_s[client_id].sockfd, (char *)&packet, sizeof(packet)-1);
#endif
            if(ret <= 0)
            {
                client_s[client_id].client_status = -1;
                continue;
            }

#if USE_NONBLOCK_SOCK
            ret = write_i(client_s[client_id].sockfd, dataBuf, data_size);
#else
            ret = socket_send_data(client_s[client_id].sockfd, dataBuf, data_size);
#endif
            if(ret <= 0)
            {
                client_s[client_id].client_status = -1;
                continue;
            }
        }
        else
        {
            usleep(10000);
        }
    }
    //pthread_exit(NULL);

    return (void *)0;
}

static int video_web_get_stream_num_by_stream_id(int streamId)
{
    int i;
    int ret = 0;
    for(i = 0; i < 4; i++)
    {
        if(streamId == videoStreamPro[i].streamId &&  videoStreamPro[i].streamId != -1)
        {
            ret = videoStreamPro[i].streamNum ;
            break;
        }
    }
    return ret;
}
static void video_web_release_bufId_by_stream_id(int streamId)
{
    int i;
    for(i = 0; i < 4; i++)
    {
        if(streamId == videoStreamPro[i].streamId &&  videoStreamPro[i].streamId != -1)
        {
            if(videoStreamPro[i].streamNum >= 0)
            {
                videoStreamPro[i].streamNum--;
                break;
            }
        }
    }
}

static void video_web_add_streamNum_by_stream_id(int streamId)
{
    int i;
    for(i = 0; i < 4; i++)
    {
        if(streamId == videoStreamPro[i].streamId &&  videoStreamPro[i].streamId != -1)
        {
            videoStreamPro[i].streamNum++;
            PRINT_INFO("Add new steam id:%d, num:%d,max num:%d\n",streamId,videoStreamPro[i].streamNum,videoStreamPro[i].maxStreamNum);
            break;
        }
    }
}

static int video_web_get_bufId_by_steam_id(int streamId)
{
    int i;
    for(i = 0; i < 4; i++)
    {
        if(streamId == videoStreamPro[i].streamId && videoStreamPro[i].streamId != -1)
        {
            if(videoStreamPro[i].streamNum >= videoStreamPro[i].maxStreamNum)
            {
                PRINT_INFO("steam id:%d, excee max num:%d,reject connect\n",streamId,videoStreamPro[i].maxStreamNum);
                return -1;
            }
            else
            {
                //videoStreamPro[i].streamNum++;
                //PRINT_INFO("steam id:%d, num:%d,max num:%d\n",streamId,videoStreamPro[i].streamNum,videoStreamPro[i].maxStreamNum);
                return i;
            }
        }
    }
    return -1;
}

int netcam_video_web_stream_init(NETCAM_VIDEO_STREAM_Porpety *streamPro, int streamNum)
{
    int i;
    void *handle;
    if(streamNum > 4 && streamNum < 1)
    {
        PRINT_ERR("steamnum error\n");
        return -1;
    }
    if(streamPro == NULL)
    {
        PRINT_ERR("streamPro is NULL\n");
        return -1;
    }
    for(i = 0; i < 4; i++)
    {
        videoStreamPro[i].streamId = -1;
        videoStreamPro[i].bufId = -1;
        videoStreamPro[i].maxStreamNum= -1;
        videoStreamPro[i].streamNum = -1;

    }


    for(i =0 ; i < streamNum; i++)
    {
        if(streamPro[i].maxStreamNum > 3)
        {
            streamPro[i].maxStreamNum = 3;
        }
        //handle = MEDIABUF_open_byid(streamPro[i].bufId);
        handle = mediabuf_add_reader(streamPro[i].bufId);
        if(handle == 0)
        {
            PRINT_ERR("Stream info %d, buf id error:%d\n", i,streamPro[i].bufId);
            continue;
        }
        else
        {
            mediabuf_del_reader(handle);
        }

        videoStreamPro[i].maxStreamNum = streamPro[i].maxStreamNum ;
        videoStreamPro[i].bufId = streamPro[i].bufId;
        videoStreamPro[i].streamId  = streamPro[i].streamId;
        videoStreamPro[i].streamNum = 0;
        maxConstreamNum += streamPro[i].maxStreamNum;
        PRINT_INFO("bufId:%d,stream id:%d,Max stream num:%d\n",streamPro[i].bufId,streamPro[i].streamId,streamPro[i].maxStreamNum);
    }

    // pthread_create(&videoServerPid, NULL, tcp_video_server_proc, NULL);
    // if(videoServerPid != 0)
    // {
    //     PRINT_INFO("Create web stream server success\n");
    // }
    // else
    // {
    //     PRINT_INFO("Create web stream server failed\n");
    // }        xqq
    return 0;
}
