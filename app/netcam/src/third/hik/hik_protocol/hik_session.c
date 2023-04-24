#include "hik_session.h"
#include "hik_net.h"


int g_hik_pthread_run = 1;
int connect_num = 0;


int hik_connect_add()
{
    connect_num ++;
    return connect_num;
}

int hik_connect_del()
{
    connect_num --;
    return connect_num;
}

void * HikSearchThread(void *param)
{
    sdk_sys_thread_set_name("HikSearchThread");

    int sockType = 0; //0:UDP 1:raw socket
	int fSockSearchfd = HikSearchCreateSocket(&sockType);
	if(fSockSearchfd < 0)
	{
        HPRINT_ERR("Create Broadcast Sock error.\n");
        return NULL; 		
	}
	HPRINT_INFO("Broadcast Socket:%d \n", fSockSearchfd);

    int ret;
    struct sockaddr_in addr;
    //int sin_size;
    
	int fd_max = -1;
	fd_set readfd;
	struct timeval timeout;
    while(1)
    {
        if (g_hik_pthread_run == 0)
        {
            HPRINT_INFO("g_hik_pthread_run == 0, so break\n");
            break;
        }

        FD_ZERO(&readfd);
        FD_SET(fSockSearchfd, &readfd);
        if(fSockSearchfd > fd_max)
            fd_max = fSockSearchfd;
        
        timeout.tv_sec = 3;
        timeout.tv_usec = 0;

        ret = select(fd_max + 1, &readfd, NULL, NULL, &timeout);
        
        if(0 == ret)
            continue;
        else if(ret < 0){
            HPRINT_ERR("call select error.\n");
            break;
        }

        //sin_size = sizeof(struct sockaddr_in);
        if(FD_ISSET(fSockSearchfd, &readfd))
        {
            HikSearchRequsetProc(0, fSockSearchfd, 0);
        }

    }


	if(fSockSearchfd > 0)
	{
		//关闭socket
		close(fSockSearchfd);		
	}	
     
    HPRINT_INFO("HikSearchThread end !\n");
    return NULL;  
}  

static int HikListenLoop()
{
    int ret, listenfd, connectfd;    //socket文件描述符
    //listenfd = CreateTcpSock(NULL, 8000);
    listenfd = hik_CreateServerSocket();
	if (listenfd < 0) {
		HPRINT_ERR("create listen socket fail");
		return -1;
	}

    //设置成非阻塞,可重用
    ret = SetSockAttr(listenfd);
    if (ret < 0) {
        HPRINT_ERR();
        return -1;
    }

    //循环监听，等待连接
    struct sockaddr_in addr;
    int sin_size;  //地址信息结构体大小
    struct timeval timeout;
    fd_set readfd; //读文件描述符集合

    HikUser *hik_user = hik_ServiceInit();
    lib_hik_init(hik_user);

    while (1) {
        if (0 == g_hik_pthread_run) {
            HPRINT_INFO("listen, g_cms_pthread_run is 0\n");
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
            HPRINT_ERR("Server: accept failed%s\n", strerror(errno));
            continue;
        }
        
        int client_num = hik_connect_add();
        HPRINT_INFO("accept session_socket:%d, remote ip:%s:%d, connect_num:%d\n",
                    connectfd, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), client_num);

		HIK_CLIENT_INFO *pInfo = (HIK_CLIENT_INFO *)malloc(sizeof(HIK_CLIENT_INFO));
		if (!pInfo)
		{
			HPRINT_ERR("Malloc failed!\n");
			break;
		}
		pInfo->ClientSocket = connectfd;
		memcpy(&pInfo->ClientAddr, &addr, sizeof(pInfo->ClientAddr));
        
        //每有一个连接就创建一个会话线程
        CreateDetachThread(hik_Session_Thread, (void *)pInfo, NULL);

    }

    HPRINT_INFO("HikListenLoop end\n");
    //关闭服务器监听socket
	if (listenfd > 0) {
    	close(listenfd);
		listenfd = -1;
	}
    lib_hik_uninit();
    return 0;
}

void * HikMainThread(void *param)
{ 
    sdk_sys_thread_set_name("HikMainThread");
    HikListenLoop();
    return NULL;
}

int lib_hik_start()
{
    g_hik_pthread_run = 1;
    HPRINT_INFO("lib_hik_start\n");
    CreateDetachThread(HikSearchThread, NULL, NULL);
    CreateDetachThread(HikMainThread, NULL, NULL);
    return 0;
}

int lib_hik_stop()
{
    //int ret = 0;
    g_hik_pthread_run = 0; 
    HPRINT_INFO("lib_hik_stop\n");
    return 0;
}

