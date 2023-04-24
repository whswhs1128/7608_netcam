/*!
*****************************************************************************
** FileName     : util_sock.c
**
** Description  : sock api of cms.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Date         : 2015-9-9, create.
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
//#include "common.h"
#include "util_sock.h"

#include   <stdio.h> 

#include   <stdlib.h> 

#include   <string.h> 

#include   <sys/types.h> 

#include   <sys/socket.h> 

#include   <errno.h> 

#include   <unistd.h> 

#include   <netinet/in.h> 

#include   <limits.h> 

#include   <netdb.h> 

#include   <arpa/inet.h> 

#include   <ctype.h> 



//#include "utility_api.h"
#define PRINT_ERR(fmt,args...) printf("[%s] Line: %d,"fmt" \n",  __FUNCTION__, __LINE__,##args)
#define PRINT_INFO(fmt,args...) printf("Info [%s] Line: %d, "fmt" \n",  __FUNCTION__, __LINE__,##args)

//exist,return 1 . not exist, return 0
static int UtIsSessionIdExist(int value, UtSession *session)
{
    int i = 0;
    int ret = 0;
    pthread_mutex_lock(&(session->global_mutex));
    for (i = 0; i < UT_MAX_LINK_NUM; i++) {
        if (session->session_array[i].session_int == value) {
            ret = 1;
            break;
        }
    }
    pthread_mutex_unlock(&(session->global_mutex));

	return ret;
}

//exist,return 1 . not exist, return 0
static int UtIsConnectIdExist(int c_id, UtSession *session)
{
    int i = 0;
    int ret = 0;
    pthread_mutex_lock(&(session->global_mutex));
    for (i = 0; i < UT_MAX_LINK_NUM; i++) {
        printf("c_id1:%d, c_id2:%d\n", session->session_array[i].c_id, c_id);
        if (session->session_array[i].c_id - c_id == 0) {
            ret = 1;
            break;
        }
    }
    pthread_mutex_unlock(&(session->global_mutex));

	return ret;
}


static int UtGenRandInt(int min, int max)
{
	int value = 0;

	srand( (unsigned int)time(NULL) );

	//产生随机数 [10000, 20000]
	value = rand() % (max + 1 - min) + min;

	return value;
}


int UtGenSessionInt(UtSession *session)
{
	int value = 0;
    do {
        value = UtGenRandInt(10000, 20000);
    } while (UtIsSessionIdExist(value, session) == 1);

	return value;
}

int UtGenConnectId(UtSession *session)
{
	int value = 0;
    do {
        value = UtGenRandInt(100000, 11999999);
    } while (UtIsConnectIdExist(value, session) == 1);

	return value;
}


int UtSessionGlobalInit(UtSession *session)
{
	pthread_mutex_init(&(session->global_mutex), NULL);
    session->online_session_num = 0;
    session->main_stream_num = 0;
    session->sub_stream_num = 0;
    session->main_is_set_key = 0;
    session->sub_is_set_key = 0;
    session->control_index = -1;
    
    int i;
	pthread_mutex_lock(&(session->global_mutex));
    for (i = 0; i < UT_MAX_LINK_NUM; i++) {
        //memset(&session->session_array[i], 0, sizeof(UtSessionGlobal));
		session->session_array[i].flag = 0;
		session->session_array[i].session_int = -1;
		session->session_array[i].cmd_sock = -1;
        session->session_array[i].notice_sock = -1;

		session->session_array[i].lv_sock.main_stream_socket.n_socket = -1;
		session->session_array[i].lv_sock.main_stream_socket.n_flag = 0;

		session->session_array[i].lv_sock.sub_stream_socket.n_socket = -1;
		session->session_array[i].lv_sock.sub_stream_socket.n_flag = 0;

		session->session_array[i].talk_sock.n_socket = -1;
        session->session_array[i].pb_sock.n_socket = -1;
    }
	pthread_mutex_unlock(&(session->global_mutex));

	return 0;
}

void UtSessionGlobalUninit(UtSession *session)
{
    pthread_mutex_destroy(&(session->global_mutex));
}

int UtSetControlIndex(UtSession *session)
{
    int i = 0;
    pthread_mutex_lock(&(session->global_mutex));
	//判断是否已经存在数组中
    for (i = 0; i < UT_MAX_LINK_NUM; i++) {
        if ((session->session_array[i].flag == 1)  && 
            (session->session_array[i].cmd_sock > 0)) {
            session->control_index = i;
			break;
		}
    }
    if (i == UT_MAX_LINK_NUM)
        session->control_index = -1;

    PRINT_INFO("control_index:%d\n", session->control_index);
    pthread_mutex_unlock(&(session->global_mutex));
    return 0;
}

int UtGetControlIndex(UtSession *session)
{
    int index = -1;
    pthread_mutex_lock(&(session->global_mutex));
    index = session->control_index;
    pthread_mutex_unlock(&(session->global_mutex));
    return index;
}


/* 添加成功返回0; 连接达到最大数，返回-1;  已经存在，返回1 */
int UtAddCmdSock(int socket, int session_id, UtSession *session)
{
	int i;
	int ret = -1; // 连接达到最大数，返回-1
	int is_exist = 0;

	pthread_mutex_lock(&(session->global_mutex));
	//判断是否已经存在数组中
    for (i = 0; i < UT_MAX_LINK_NUM; i++) {
        if( (session->session_array[i].flag == 1)  &&
			(session->session_array[i].session_int == session_id) &&
			(session->session_array[i].cmd_sock == socket) ) {
			is_exist = 1; //已经存在
			ret = 1; //已经存在返回1
			break;
        }
    }

	//如果不存在，则添加进数组
	if (is_exist == 0) {
		ret = -1; // 连接达到最大数，返回-1
	    for (i = 0; i < UT_MAX_LINK_NUM; i++) {
	        if (session->session_array[i].flag == 0) {
				session->session_array[i].session_int = session_id;
				session->session_array[i].cmd_sock = socket;
				session->session_array[i].flag = 1;
                session->online_session_num ++;

                /* 如果没有control_index，则设置成 control_index */
                PRINT_INFO("add cmd sock %d, sid:%d, online:%d, control_index:%d, add_cmd_index:%d\n", 
                    socket, session_id, session->online_session_num, session->control_index, i);
                
                /* control_index 默认是 -1 */
                if (session->control_index < 0)
                    session->control_index = i;

				ret = 0; //添加成功返回0
				break;
	        }
	    }
	}
    
	pthread_mutex_unlock(&(session->global_mutex));
	return ret;
}

int UtCloseCmdSock(int socket, UtSession *session)
{
    int i;
    int ret = -1;
    pthread_mutex_lock(&(session->global_mutex));

    //判断是否已经存在数组中
    for (i = 0; i < UT_MAX_LINK_NUM; i++) {
        if (session->session_array[i].cmd_sock == socket) {
			/* 关闭liveview 主码流socket */
			if (session->session_array[i].lv_sock.main_stream_socket.n_socket > 0) {
				close(session->session_array[i].lv_sock.main_stream_socket.n_socket);
                session->main_stream_num--;
				PRINT_INFO("close liveview main stream socket:%d\n", session->session_array[i].lv_sock.main_stream_socket.n_socket);
			}
			session->session_array[i].lv_sock.main_stream_socket.n_socket = -1;
			session->session_array[i].lv_sock.main_stream_socket.n_flag = 0;

			/* 关闭liveview 子码流socket */
			if (session->session_array[i].lv_sock.sub_stream_socket.n_socket > 0) {
				close(session->session_array[i].lv_sock.sub_stream_socket.n_socket);
                session->sub_stream_num--;
				PRINT_INFO("close liveview sub stream socket:%d\n", session->session_array[i].lv_sock.sub_stream_socket.n_socket);
			}
			session->session_array[i].lv_sock.sub_stream_socket.n_socket = -1;
			session->session_array[i].lv_sock.sub_stream_socket.n_flag = 0;

			/* 关闭session 主会话 socket */
			if (session->session_array[i].cmd_sock > 0) {
				close(session->session_array[i].cmd_sock);
				PRINT_INFO("close cmd socket:%d, index:%d\n", session->session_array[i].cmd_sock, i);
			}
			session->session_array[i].cmd_sock = -1;
			session->session_array[i].session_int = -1;
			session->session_array[i].flag = 0;

        	if (session->session_array[i].notice_sock > 0) {
				close(session->session_array[i].notice_sock);
				PRINT_INFO("close notice socket:%d\n", session->session_array[i].notice_sock);
        		session->session_array[i].notice_sock = -1;
        	}

        	if (session->session_array[i].talk_sock.n_socket > 0) {
				close(session->session_array[i].talk_sock.n_socket);
				PRINT_INFO("close talk socket:%d\n", session->session_array[i].talk_sock.n_socket);
        		session->session_array[i].talk_sock.n_socket = -1;
        	}

        	if (session->session_array[i].pb_sock.n_socket > 0) {
				close(session->session_array[i].pb_sock.n_socket);
				PRINT_INFO("close pb socket:%d\n", session->session_array[i].pb_sock.n_socket);
        		session->session_array[i].pb_sock.n_socket = -1;
        	}

            session->online_session_num --;
			ret = 0; //删除成功返回0
			break;
        }
    }

	pthread_mutex_unlock(&(session->global_mutex));

    UtSetControlIndex(session);
	return ret;
}

int UtCloseAllSock(UtSession *session)
{
	int i;
	int ret = -1;
	pthread_mutex_lock(&(session->global_mutex));

	//判断是否已经存在数组中
    for (i = 0; i < UT_MAX_LINK_NUM; i++) {
		/* 关闭liveview 主码流socket */
		if (session->session_array[i].lv_sock.main_stream_socket.n_socket > 0) {
			close(session->session_array[i].lv_sock.main_stream_socket.n_socket);
			PRINT_INFO("close liveview main stream socket:%d\n", session->session_array[i].lv_sock.main_stream_socket.n_socket);
            //session->main_stream_num--;
		}
		session->session_array[i].lv_sock.main_stream_socket.n_socket = -1;
		session->session_array[i].lv_sock.main_stream_socket.n_flag = 0;

		/* 关闭liveview 子码流socket */
		if (session->session_array[i].lv_sock.sub_stream_socket.n_socket > 0) {
			close(session->session_array[i].lv_sock.sub_stream_socket.n_socket);
			PRINT_INFO("close liveview sub stream socket:%d\n", session->session_array[i].lv_sock.sub_stream_socket.n_socket);
            //session->sub_stream_num--;
		}
		session->session_array[i].lv_sock.sub_stream_socket.n_socket = -1;
		session->session_array[i].lv_sock.sub_stream_socket.n_flag = 0;

		/* 关闭session 主会话 socket */
		if (session->session_array[i].cmd_sock > 0) {
			close(session->session_array[i].cmd_sock);
			PRINT_INFO("close main socket:%d \n", session->session_array[i].cmd_sock);
		}
		session->session_array[i].cmd_sock = -1;
		session->session_array[i].session_int = -1;
		session->session_array[i].flag = 0;

        if (session->session_array[i].notice_sock > 0) {
            close(session->session_array[i].notice_sock);
            PRINT_INFO("close notice socket:%d\n", session->session_array[i].notice_sock);
            session->session_array[i].notice_sock = -1;
        }

    	if (session->session_array[i].talk_sock.n_socket > 0) {
			close(session->session_array[i].talk_sock.n_socket);
			PRINT_INFO("close talk socket:%d\n", session->session_array[i].talk_sock.n_socket);
    		session->session_array[i].talk_sock.n_socket = -1;
    	}

    	if (session->session_array[i].pb_sock.n_socket > 0) {
			close(session->session_array[i].pb_sock.n_socket);
			PRINT_INFO("close pb socket:%d\n", session->session_array[i].pb_sock.n_socket);
    		session->session_array[i].pb_sock.n_socket = -1;
    	}
    }

    session->main_stream_num = 0;
    session->sub_stream_num = 0;
    session->online_session_num = 0;
    session->main_is_set_key = 0;
    session->sub_is_set_key = 0;
    session->control_index = -1;
	pthread_mutex_unlock(&(session->global_mutex));
	return ret;
}


/* 没有对应sessionID返回-1; 设置成功返回0 */
int UtAddNoticeSock(int socket, int session_id, UtSession *session)
{
	int i;
	int ret = -1;

	pthread_mutex_lock(&(session->global_mutex));
	//覆盖添加
    for (i = 0; i < UT_MAX_LINK_NUM; i++) {
        if (session->session_array[i].session_int == session_id) {
        	session->session_array[i].notice_sock = socket;
			ret = 0;
            PRINT_INFO("session_int:%d, cmd sock :%d add notice sock:%d \n", 
                session->session_array[i].session_int, 
                session->session_array[i].cmd_sock, 
                session->session_array[i].notice_sock);

			break;
        }
    }
	pthread_mutex_unlock(&(session->global_mutex));
	return ret;
}


/* 没有对应sessionID返回-1; 设置成功返回0 */
int UtAddTalkSock(int socket, int session_id, UtSession *session)
{
	int i;
	int ret = -1;

	pthread_mutex_lock(&(session->global_mutex));
	//覆盖添加
    for (i = 0; i < UT_MAX_LINK_NUM; i++) {
        if (session->session_array[i].session_int == session_id) {
        	session->session_array[i].talk_sock.n_socket = socket;
			ret = 0;
            PRINT_INFO("session_int:%d, cmd sock :%d, talk sock:%d \n", 
                session->session_array[i].session_int, 
                session->session_array[i].cmd_sock, 
                session->session_array[i].talk_sock.n_socket);

			break;
        }
    }
	pthread_mutex_unlock(&(session->global_mutex));
	return ret;
}

/* 没有对应sessionID返回-1; 设置成功返回0 */
int UtAddPbSock(int socket, int session_id, UtSession *session)
{
	int i;
	int ret = -1;

	pthread_mutex_lock(&(session->global_mutex));
	//覆盖添加
    for (i = 0; i < UT_MAX_LINK_NUM; i++) {
        if (session->session_array[i].session_int == session_id) {
        	session->session_array[i].pb_sock.n_socket = socket;
			ret = 0;
            PRINT_INFO("session_int:%d, cmd sock :%d, pb sock:%d \n", 
                session->session_array[i].session_int, 
                session->session_array[i].cmd_sock, 
                session->session_array[i].pb_sock.n_socket);

            break;
        }
    }
	pthread_mutex_unlock(&(session->global_mutex));
	return ret;
}

int UtGetNoticeSock(int session_id, UtSession *session)
{
	int i;
	int sock = -1;

	pthread_mutex_lock(&(session->global_mutex));
	//判断是否已经存在数组中
    for (i = 0; i < UT_MAX_LINK_NUM; i++) {
        if (session->session_array[i].session_int == session_id) {
			sock = session->session_array[i].notice_sock;
			break;
		}
    }
	pthread_mutex_unlock(&(session->global_mutex));
	return sock;
}

int UtGetTalkSock(int session_id, UtSession *session)
{
	int i;
	int sock = -1;

	pthread_mutex_lock(&(session->global_mutex));
	//判断是否已经存在数组中
    for (i = 0; i < UT_MAX_LINK_NUM; i++) {
        if (session->session_array[i].session_int == session_id) {
			sock = session->session_array[i].talk_sock.n_socket;
			break;
		}
    }
	pthread_mutex_unlock(&(session->global_mutex));
	return sock;
}

int UtGetPbSock(int session_id, UtSession *session)
{
	int i;
	int sock = -1;

	pthread_mutex_lock(&(session->global_mutex));
	//判断是否已经存在数组中
    for (i = 0; i < UT_MAX_LINK_NUM; i++) {
        if (session->session_array[i].session_int == session_id) {
			sock = session->session_array[i].pb_sock.n_socket;
			break;
		}
    }
	pthread_mutex_unlock(&(session->global_mutex));
	return sock;
}

int UtCloseNoticeSock(int session_id, UtSession *session)
{
	int i;
	int ret = -1;

	pthread_mutex_lock(&(session->global_mutex));
	//判断是否已经存在数组中
    for (i = 0; i < UT_MAX_LINK_NUM; i++) {
        if (session->session_array[i].session_int == session_id) {
        	if (session->session_array[i].notice_sock > 0) {
				close(session->session_array[i].notice_sock);
				PRINT_INFO("close notice socket:%d\n", session->session_array[i].notice_sock);
        		session->session_array[i].notice_sock = -1;
        	}

			ret = 0; //删除成功返回0
			break;
		}
    }
	pthread_mutex_unlock(&(session->global_mutex));
	return ret;
}


int UtCloseTalkSock(int session_id, UtSession *session)
{
	int i;
	int ret = -1;

	pthread_mutex_lock(&(session->global_mutex));
	//判断是否已经存在数组中
    for (i = 0; i < UT_MAX_LINK_NUM; i++) {
        if (session->session_array[i].session_int == session_id) {
        	if (session->session_array[i].talk_sock.n_socket > 0) {
				close(session->session_array[i].talk_sock.n_socket);
				PRINT_INFO("close talk socket:%d\n", session->session_array[i].talk_sock.n_socket);

        		session->session_array[i].talk_sock.n_socket = -1;
        	}

			ret = 0; //删除成功返回0
			break;
		}
    }
	pthread_mutex_unlock(&(session->global_mutex));
	return ret;
}

int UtClosePbSock(int session_id, UtSession *session)
{
	int i;
	int ret = -1;

	pthread_mutex_lock(&(session->global_mutex));
	//判断是否已经存在数组中
    for (i = 0; i < UT_MAX_LINK_NUM; i++) {
        if (session->session_array[i].session_int == session_id) {
        	if (session->session_array[i].pb_sock.n_socket > 0) {
				close(session->session_array[i].pb_sock.n_socket);
				PRINT_INFO("close pb socket:%d\n", session->session_array[i].pb_sock.n_socket);

        		session->session_array[i].pb_sock.n_socket = -1;
        	}

			ret = 0; //删除成功返回0
			break;
		}
    }
	pthread_mutex_unlock(&(session->global_mutex));
	return ret;
}


/* 没有对应sessionID返回-1; 设置成功返回0 */
int UtAddLvSock(int socket, int session_id, UT_STREAM_TYPE type, UtSession *session)
{
	int i;
	int ret = -1;

	pthread_mutex_lock(&(session->global_mutex));
	//判断是否已经存在数组中
    for (i = 0; i < UT_MAX_LINK_NUM; i++) {
        if(session->session_array[i].session_int == session_id) {
			if (type == UT_MAIN_STREAM) {
				session->session_array[i].lv_sock.main_stream_socket.n_socket = socket;
				session->session_array[i].lv_sock.main_stream_socket.n_flag = 0; //默认先关闭，0关闭1打开
				session->session_array[i].lv_sock.main_stream_socket.is_wait_i = 1;
                session->main_stream_num++;
                PRINT_INFO("cmd sock %d add lv main sock %d, main stream num:%d, flag:%d\n", 
                    session->session_array[i].cmd_sock, 
                    socket,
                    session->main_stream_num, session->session_array[i].lv_sock.main_stream_socket.n_flag);
			} else if(type == UT_SUB_STREAM) {
				session->session_array[i].lv_sock.sub_stream_socket.n_socket = socket;
				session->session_array[i].lv_sock.sub_stream_socket.n_flag = 0; //默认先关闭，0关闭1打开
				session->session_array[i].lv_sock.sub_stream_socket.is_wait_i = 1;
                session->sub_stream_num++;
                PRINT_INFO("cmd sock %d add lv sub sock %d, sub stream num:%d, flag:%d\n", 
                    session->session_array[i].cmd_sock, 
                    socket,
                    session->sub_stream_num, session->session_array[i].lv_sock.main_stream_socket.n_flag);
			}

			ret = 0; //添加成功返回0
			break;
        }
    }
	pthread_mutex_unlock(&(session->global_mutex));
	return ret;
}

/* 没有对应sessionID返回-1; 设置成功返回0 */
int UtAddLvSock2(int socket, char *client_ip, int session_id, int connect_id, UtSession *session)
{
	int i;
	int ret = -1;

	pthread_mutex_lock(&(session->global_mutex));
	//判断是否已经存在数组中
    for (i = 0; i < UT_MAX_LINK_NUM; i++) {
        if(session->session_array[i].session_int == session_id) {
            session->session_array[i].tmp_sock = socket;
            session->session_array[i].c_id = connect_id;
            strncpy(session->session_array[i].client_ip, client_ip, sizeof(session->session_array[i].client_ip));

            PRINT_INFO("cmd sock %d add lv tmp sock %d, connect_id:%d, ip:%s\n",
                session->session_array[i].cmd_sock, 
                session->session_array[i].tmp_sock, 
                session->session_array[i].c_id,
                session->session_array[i].client_ip);

			ret = 0; //添加成功返回0
			break;
        }
    }
	pthread_mutex_unlock(&(session->global_mutex));
	return ret;
}



int UtGetLvSocket(int session_id, UT_STREAM_TYPE type, UtSession *session)
{
	int i;
	int sock = -1;

	pthread_mutex_lock(&(session->global_mutex));
	//判断是否已经存在数组中
    for (i = 0; i < UT_MAX_LINK_NUM; i++) {
        if (session->session_array[i].session_int == session_id) {
			if (type == UT_MAIN_STREAM) {
				sock = session->session_array[i].lv_sock.main_stream_socket.n_socket;
			} else if(type == UT_SUB_STREAM) {
				sock = session->session_array[i].lv_sock.sub_stream_socket.n_socket;
			}

			break;
		}
    }
	pthread_mutex_unlock(&(session->global_mutex));
	return sock;
}


int UtCloseLvSock(int session_id, UT_STREAM_TYPE type, UtSession *session)
{
	int i;
	int ret = -1;

	pthread_mutex_lock(&(session->global_mutex));
	//判断是否已经存在数组中
    for (i = 0; i < UT_MAX_LINK_NUM; i++) {
        if (session->session_array[i].session_int == session_id) {
			if (type == UT_MAIN_STREAM) {
				if (session->session_array[i].lv_sock.main_stream_socket.n_socket > 0) {
					close(session->session_array[i].lv_sock.main_stream_socket.n_socket);
                    session->main_stream_num--;
					PRINT_INFO("cmd %d close liveview main stream socket:%d\n", 
                        session->session_array[i].cmd_sock, 
                        session->session_array[i].lv_sock.main_stream_socket.n_socket);
				}

				session->session_array[i].lv_sock.main_stream_socket.n_socket = -1;
				session->session_array[i].lv_sock.main_stream_socket.n_flag = 0;
			} else if(type == UT_SUB_STREAM) {
				if (session->session_array[i].lv_sock.sub_stream_socket.n_socket > 0) {
					close(session->session_array[i].lv_sock.sub_stream_socket.n_socket);
					session->sub_stream_num--;
					PRINT_INFO("cmd %d close liveview sub stream socket:%d\n", 
                        session->session_array[i].cmd_sock, 
                        session->session_array[i].lv_sock.sub_stream_socket.n_socket);
                }

				session->session_array[i].lv_sock.sub_stream_socket.n_socket = -1;
				session->session_array[i].lv_sock.sub_stream_socket.n_flag = 0;
			}

			ret = 0; //删除成功返回0
			break;
		}
    }
	pthread_mutex_unlock(&(session->global_mutex));
	return ret;
}

int UtSetLvSockStatus(int session_id, int lv_sock_flag, UT_STREAM_TYPE type, UtSession *session)
{
	int i;
	int ret = -1;

	pthread_mutex_lock(&(session->global_mutex));
	//判断是否已经存在数组中
    for (i = 0; i < UT_MAX_LINK_NUM; i++) {
        if (session->session_array[i].session_int == session_id) {
			if (type == UT_MAIN_STREAM) {
                PRINT_INFO("set main stream flag:%d", lv_sock_flag);
				session->session_array[i].lv_sock.main_stream_socket.n_flag = lv_sock_flag;
			} else if(type == UT_SUB_STREAM) {
			    PRINT_INFO("set sub stream flag:%d", lv_sock_flag);
				session->session_array[i].lv_sock.sub_stream_socket.n_flag = lv_sock_flag;
			}

			ret = 0;
			break;
		}
    }
	pthread_mutex_unlock(&(session->global_mutex));
	return ret;
}

int UtSetLvSockStatus2(int session_id, int lv_sock_flag, UT_STREAM_TYPE type, UtSession *session)
{
	int i;
	int ret = -1;

	pthread_mutex_lock(&(session->global_mutex));
	//判断是否已经存在数组中
    for (i = 0; i < UT_MAX_LINK_NUM; i++) {
        if(session->session_array[i].session_int == session_id) {
			if (type == UT_MAIN_STREAM) {
                PRINT_INFO("set main stream flag:%d", lv_sock_flag);
				session->session_array[i].lv_sock.main_stream_socket.n_socket = session->session_array[i].tmp_sock;
				session->session_array[i].lv_sock.main_stream_socket.n_flag = lv_sock_flag;
                session->main_stream_num++;
                PRINT_INFO("cmd sock %d add lv main sock %d, flag:%d, main stream num:%d, sub stream num:%d\n", 
                    session->session_array[i].cmd_sock, 
                    session->session_array[i].lv_sock.main_stream_socket.n_socket,
                    session->session_array[i].lv_sock.main_stream_socket.n_flag,
                    session->main_stream_num,
                    session->sub_stream_num);

			} else if(type == UT_SUB_STREAM) {
			    PRINT_INFO("set sub stream flag:%d", lv_sock_flag);
				session->session_array[i].lv_sock.sub_stream_socket.n_socket = session->session_array[i].tmp_sock;
				session->session_array[i].lv_sock.sub_stream_socket.n_flag = lv_sock_flag;
                session->sub_stream_num++;
                PRINT_INFO("cmd sock %d add lv sub sock %d, flag:%d, main stream num:%d, sub stream num:%d\n", 
                    session->session_array[i].cmd_sock, 
                    session->session_array[i].lv_sock.sub_stream_socket.n_socket,
                    session->session_array[i].lv_sock.sub_stream_socket.n_flag,
                    session->main_stream_num,
                    session->sub_stream_num);

            }
            ret = 0;
			break;
        }
    }
	pthread_mutex_unlock(&(session->global_mutex));
	return ret;
}


int UtSetLvSockStatusByIp(char *ip, int lv_sock_flag, UT_STREAM_TYPE type, UtSession *session)
{
	int i;
	int ret = -1;

	pthread_mutex_lock(&(session->global_mutex));
	//判断是否已经存在数组中
    for (i = 0; i < UT_MAX_LINK_NUM; i++) {
        if (0 == strcmp(session->session_array[i].client_ip, ip)) {
			if (type == UT_MAIN_STREAM) {
                if (lv_sock_flag && session->session_array[i].lv_sock.main_stream_socket.n_socket > 0) {
				    session->session_array[i].lv_sock.main_stream_socket.n_flag = lv_sock_flag;
                } else {
                    session->session_array[i].lv_sock.main_stream_socket.n_flag = 0;
                }
                PRINT_INFO("index:%d, ip:%s, cmd sock %d set lv main sock %d, flag:%d, sid:%d\n",
                    i, ip,
                    session->session_array[i].cmd_sock, 
                    session->session_array[i].lv_sock.main_stream_socket.n_socket,
                    session->session_array[i].lv_sock.main_stream_socket.n_flag,
                    session->session_array[i].session_int);
			} else if(type == UT_SUB_STREAM) {
                if (lv_sock_flag && session->session_array[i].lv_sock.sub_stream_socket.n_socket > 0) {
				    session->session_array[i].lv_sock.sub_stream_socket.n_flag = lv_sock_flag;
                } else {
                    session->session_array[i].lv_sock.sub_stream_socket.n_flag = 0;
                }

                PRINT_INFO("index:%d, ip:%s, cmd sock %d set lv sub sock %d, flag:%d, sid:%d\n",
                    i, ip,
                    session->session_array[i].cmd_sock, 
                    session->session_array[i].lv_sock.sub_stream_socket.n_socket,
                    session->session_array[i].lv_sock.sub_stream_socket.n_flag,
                    session->session_array[i].session_int);
			}

			ret = 0;
			//break; //不退出
		}
    }
	pthread_mutex_unlock(&(session->global_mutex));
	return ret;
}


int UtSetLvSockWaitI(int session_id, UT_STREAM_TYPE type, int flag, UtSession *session)
{
	int i;
	int ret = -1;

	pthread_mutex_lock(&(session->global_mutex));
	//判断是否已经存在数组中
    for (i = 0; i < UT_MAX_LINK_NUM; i++) {
        if (session->session_array[i].session_int == session_id) {
			if (type == UT_MAIN_STREAM) {
                PRINT_INFO("set main stream wait I:%d", flag);
				session->session_array[i].lv_sock.main_stream_socket.is_wait_i = flag;
			} else if(type == UT_SUB_STREAM) {
			    PRINT_INFO("set sub stream wait I:%d", flag);
				session->session_array[i].lv_sock.sub_stream_socket.is_wait_i = flag;
			}

			ret = 0;
			break;
		}
    }
	pthread_mutex_unlock(&(session->global_mutex));
	return ret;
}


int UtGetLvSockStatus(int session_id, UT_STREAM_TYPE type, UtSession *session)
{
	int i;
	int status = -1;

	pthread_mutex_lock(&(session->global_mutex));
	//判断是否已经存在数组中
    for (i = 0; i < UT_MAX_LINK_NUM; i++) {
        if (session->session_array[i].session_int == session_id) {
			if (type == UT_MAIN_STREAM) {
				status = session->session_array[i].lv_sock.main_stream_socket.n_flag;
			} else if(type == UT_SUB_STREAM) {
				status = session->session_array[i].lv_sock.sub_stream_socket.n_flag;
			}

			break;
		}
    }
	pthread_mutex_unlock(&(session->global_mutex));
	return status;
}

int UtSetLvKeyFrameStatus(int is_set_key, int type, UtSession *session)
{
	pthread_mutex_lock(&(session->global_mutex));
    if (type == UT_MAIN_STREAM) {
        session->main_is_set_key = is_set_key;
    } else if(type == UT_SUB_STREAM) {
        session->sub_is_set_key = is_set_key;
    }
	pthread_mutex_unlock(&(session->global_mutex));

	return 0;
}

int UtGetLvKeyFrameStatus(int type, UtSession *session)
{
	int is_set_key = 0;

	pthread_mutex_lock(&(session->global_mutex));
    if (type == UT_MAIN_STREAM) {
        is_set_key = session->main_is_set_key;
    } else if(type == UT_SUB_STREAM) {
        is_set_key = session->sub_is_set_key;
    }
	pthread_mutex_unlock(&(session->global_mutex));

	return is_set_key;
}



///////////////////////////////////

int UtGetSessionIdByCmdSock(int cmd_sock, UtSession *session)
{
	int i;
	int session_id = -1;

	pthread_mutex_lock(&(session->global_mutex));
	//判断是否已经存在数组中
    for (i = 0; i < UT_MAX_LINK_NUM; i++) {
        if (session->session_array[i].cmd_sock == cmd_sock) {
			session_id = session->session_array[i].session_int;
			break;
		}
    }
	pthread_mutex_unlock(&(session->global_mutex));
	return session_id;
}

int UtGetLvSockByIndex(int i, int type, UtSession *session)
{
    int sock = -1;

    if (i < 0 || i >= UT_MAX_LINK_NUM) {
        PRINT_ERR("index error. index:%d max_link_num:%d\n", i, UT_MAX_LINK_NUM);
        return -1;
    }

	pthread_mutex_lock(&(session->global_mutex));
    if (session->session_array[i].flag == 1) {
        if (type == UT_MAIN_STREAM) {
            if (session->session_array[i].lv_sock.main_stream_socket.n_socket > 0) {
                sock = session->session_array[i].lv_sock.main_stream_socket.n_socket;
            }
        } else if (type == UT_SUB_STREAM) {
            if (session->session_array[i].lv_sock.sub_stream_socket.n_socket > 0) {
                sock = session->session_array[i].lv_sock.sub_stream_socket.n_socket;
            }
        }
	}
	pthread_mutex_unlock(&(session->global_mutex));

    return sock;
}

int UtSetLvSockVNoByIndex(int i, int type, int vno, UtSession *session)
{
    if (i < 0 || i >= UT_MAX_LINK_NUM) {
        PRINT_ERR("index error. index:%d max_link_num:%d\n", i, UT_MAX_LINK_NUM);
        return -1;
    }

	pthread_mutex_lock(&(session->global_mutex));
    if (session->session_array[i].flag == 1) {
        if (type == UT_MAIN_STREAM) {
            if (session->session_array[i].lv_sock.main_stream_socket.n_socket > 0) {
                session->session_array[i].lv_sock.main_stream_socket.v_no = vno;
            }
        } else if (type == UT_SUB_STREAM) {
            if (session->session_array[i].lv_sock.sub_stream_socket.n_socket > 0) {
                session->session_array[i].lv_sock.sub_stream_socket.v_no = vno;
            }
        }
	}
	pthread_mutex_unlock(&(session->global_mutex));

    return 0;
}


int UtGetLvSockVNoByIndex(int i, int type, UtSession *session)
{
    int vno = -1;

    if (i < 0 || i >= UT_MAX_LINK_NUM) {
        PRINT_ERR("index error. index:%d max_link_num:%d\n", i, UT_MAX_LINK_NUM);
        return -1;
    }

	pthread_mutex_lock(&(session->global_mutex));
    if (session->session_array[i].flag == 1) {
        if (type == UT_MAIN_STREAM) {
            if (session->session_array[i].lv_sock.main_stream_socket.n_socket > 0) {
                vno = session->session_array[i].lv_sock.main_stream_socket.v_no;
            }
        } else if (type == UT_SUB_STREAM) {
            if (session->session_array[i].lv_sock.sub_stream_socket.n_socket > 0) {
                vno = session->session_array[i].lv_sock.sub_stream_socket.v_no;
            }
        }
	}
	pthread_mutex_unlock(&(session->global_mutex));

    return vno;
}

int UtGetLvSockWaitIByIndex(int i, int type, UtSession *session)
{
    int is_wait_i = 0;

    if (i < 0 || i >= UT_MAX_LINK_NUM) {
        PRINT_ERR("index error. index:%d max_link_num:%d\n", i, UT_MAX_LINK_NUM);
        return -1;
    }

	pthread_mutex_lock(&(session->global_mutex));
    if (session->session_array[i].flag == 1) {
        if (type == UT_MAIN_STREAM) {
            is_wait_i = session->session_array[i].lv_sock.main_stream_socket.is_wait_i;
        } else if (type == UT_SUB_STREAM) {
            is_wait_i = session->session_array[i].lv_sock.sub_stream_socket.is_wait_i;
        }
	}
	pthread_mutex_unlock(&(session->global_mutex));

    return is_wait_i;
}

int UtSetLvSockWaitIByIndex(int i, int type, int flag, UtSession *session)
{
    if (i < 0 || i >= UT_MAX_LINK_NUM) {
        PRINT_ERR("index error. index:%d max_link_num:%d\n", i, UT_MAX_LINK_NUM);
        return -1;
    }

	pthread_mutex_lock(&(session->global_mutex));
    if (session->session_array[i].flag == 1) {
        if (type == UT_MAIN_STREAM) {
            session->session_array[i].lv_sock.main_stream_socket.is_wait_i = flag;
        } else if (type == UT_SUB_STREAM) {
            session->session_array[i].lv_sock.sub_stream_socket.is_wait_i = flag;
        }
	}
	pthread_mutex_unlock(&(session->global_mutex));

    return 0;
}


int UtCloseLvSockByIndex(int i, int type, UtSession *session)
{
    if (i < 0 || i >= UT_MAX_LINK_NUM) {
        PRINT_ERR("index error. index:%d max_link_num:%d\n", i, UT_MAX_LINK_NUM);
        return -1;
    }

	pthread_mutex_lock(&(session->global_mutex));
    if (session->session_array[i].flag == 1) {
		if (type == UT_MAIN_STREAM) {
			if (session->session_array[i].lv_sock.main_stream_socket.n_socket > 0) {
				close(session->session_array[i].lv_sock.main_stream_socket.n_socket);
				PRINT_INFO("close liveview main stream socket:%d, sid:%d\n", 
                    session->session_array[i].lv_sock.main_stream_socket.n_socket,
                    session->session_array[i].session_int);
                session->main_stream_num--;
			}
			session->session_array[i].lv_sock.main_stream_socket.n_socket = -1;
			session->session_array[i].lv_sock.main_stream_socket.n_flag = 0;
		} else if(type == UT_SUB_STREAM) {
			if (session->session_array[i].lv_sock.sub_stream_socket.n_socket > 0) {
				close(session->session_array[i].lv_sock.sub_stream_socket.n_socket);
				PRINT_INFO("close liveview sub stream socket:%d, sid:%d\n", 
                    session->session_array[i].lv_sock.sub_stream_socket.n_socket,
                    session->session_array[i].session_int);
                session->sub_stream_num--;
            }
			session->session_array[i].lv_sock.sub_stream_socket.n_socket = -1;
			session->session_array[i].lv_sock.sub_stream_socket.n_flag = 0;
		}
	}
	pthread_mutex_unlock(&(session->global_mutex));

    return 0;
}



int UtGetSidByIndex(int i, UtSession *session)
{
    if (i < 0 || i >= UT_MAX_LINK_NUM) {
        PRINT_ERR("index error. index:%d max_link_num:%d\n", i, UT_MAX_LINK_NUM);
        return -1;
    }

    int sid = 0;
    pthread_mutex_lock(&(session->global_mutex));
    if (session->session_array[i].flag == 1) {
        sid = session->session_array[i].session_int;
    }
    pthread_mutex_unlock(&(session->global_mutex));

    return sid;
}

int UtGetCmdSockByIndex(int i, UtSession *session)
{
    if (i < 0 || i >= UT_MAX_LINK_NUM) {
        PRINT_ERR("index error. index:%d max_link_num:%d\n", i, UT_MAX_LINK_NUM);
        return -1;
    }

    int sock = -1;

	pthread_mutex_lock(&(session->global_mutex));
    if (session->session_array[i].flag == 1) {
        if (session->session_array[i].cmd_sock > 0) {
            sock = session->session_array[i].cmd_sock;
        }
	}
	pthread_mutex_unlock(&(session->global_mutex));

    return sock;
}


int UtCloseCmdSockByIndex(int i, UtSession *session)
{
    if (i < 0 || i >= UT_MAX_LINK_NUM) {
        PRINT_ERR("index error. index:%d max_link_num:%d\n", i, UT_MAX_LINK_NUM);
        return -1;
    }

	pthread_mutex_lock(&(session->global_mutex));

    /* 关闭liveview 主码流socket */
    if (session->session_array[i].lv_sock.main_stream_socket.n_socket > 0) {
    	close(session->session_array[i].lv_sock.main_stream_socket.n_socket);
    	PRINT_INFO("close liveview main stream socket:%d\n", session->session_array[i].lv_sock.main_stream_socket.n_socket);
        session->main_stream_num--;
    }
    session->session_array[i].lv_sock.main_stream_socket.n_socket = -1;
    session->session_array[i].lv_sock.main_stream_socket.n_flag = 0;

    /* 关闭liveview 子码流socket */
    if (session->session_array[i].lv_sock.sub_stream_socket.n_socket > 0) {
    	close(session->session_array[i].lv_sock.sub_stream_socket.n_socket);
    	PRINT_INFO("close liveview sub stream socket:%d\n", session->session_array[i].lv_sock.sub_stream_socket.n_socket);
        session->sub_stream_num--;
    }
    session->session_array[i].lv_sock.sub_stream_socket.n_socket = -1;
    session->session_array[i].lv_sock.sub_stream_socket.n_flag = 0;

    /* 关闭session 主会话 socket */
    if (session->session_array[i].cmd_sock > 0) {
    	close(session->session_array[i].cmd_sock);
    	PRINT_INFO("close main socket:%d \n", session->session_array[i].cmd_sock);
    }
    session->session_array[i].cmd_sock = -1;
    session->session_array[i].session_int = -1;
    session->session_array[i].flag = 0;

    if (session->session_array[i].notice_sock > 0) {
        close(session->session_array[i].notice_sock);
        PRINT_INFO("close notice socket:%d\n", session->session_array[i].notice_sock);
        session->session_array[i].notice_sock = -1;
    }

    if (session->session_array[i].talk_sock.n_socket > 0) {
    	close(session->session_array[i].talk_sock.n_socket);
    	PRINT_INFO("close talk socket:%d\n", session->session_array[i].talk_sock.n_socket);
    	session->session_array[i].talk_sock.n_socket = -1;
    }

    if (session->session_array[i].pb_sock.n_socket > 0) {
    	close(session->session_array[i].pb_sock.n_socket);
    	PRINT_INFO("close talk socket:%d\n", session->session_array[i].pb_sock.n_socket);
    	session->session_array[i].pb_sock.n_socket = -1;
    }

    session->online_session_num --;
	pthread_mutex_unlock(&(session->global_mutex));

    UtSetControlIndex(session);

    return 0;
}




int UtCloseSessionSockByIndex(int i, UtSession *session)
{
    if (i < 0 || i >= UT_MAX_LINK_NUM) {
        PRINT_ERR("index error. index:%d max_link_num:%d\n", i, UT_MAX_LINK_NUM);
        return -1;
    }

	pthread_mutex_lock(&(session->global_mutex));

	/* 关闭liveview 主码流socket */
	if (session->session_array[i].lv_sock.main_stream_socket.n_socket > 0) {
		close(session->session_array[i].lv_sock.main_stream_socket.n_socket);
		PRINT_INFO("close liveview main stream socket:%d\n", session->session_array[i].lv_sock.main_stream_socket.n_socket);
        session->main_stream_num--;
	}
	session->session_array[i].lv_sock.main_stream_socket.n_socket = -1;
	session->session_array[i].lv_sock.main_stream_socket.n_flag = 0;

	/* 关闭liveview 子码流socket */
	if (session->session_array[i].lv_sock.sub_stream_socket.n_socket > 0) {
		close(session->session_array[i].lv_sock.sub_stream_socket.n_socket);
		PRINT_INFO("close liveview sub stream socket:%d\n", session->session_array[i].lv_sock.sub_stream_socket.n_socket);
        session->sub_stream_num--;
	}
	session->session_array[i].lv_sock.sub_stream_socket.n_socket = -1;
	session->session_array[i].lv_sock.sub_stream_socket.n_flag = 0;

	/* 关闭session cmd socket */
	if (session->session_array[i].cmd_sock > 0) {
		close(session->session_array[i].cmd_sock);
		PRINT_INFO("close main socket:%d \n", session->session_array[i].cmd_sock);
	}
	session->session_array[i].cmd_sock = -1;
	session->session_array[i].session_int = -1;
	session->session_array[i].flag = 0;

    if (session->session_array[i].notice_sock > 0) {
        close(session->session_array[i].notice_sock);
        PRINT_INFO("close notice socket:%d\n", session->session_array[i].notice_sock);
        session->session_array[i].notice_sock = -1;
    }

    /* 关闭session talk socket */
	if (session->session_array[i].talk_sock.n_socket > 0) {
		close(session->session_array[i].talk_sock.n_socket);
		PRINT_INFO("close talk socket:%d\n", session->session_array[i].talk_sock.n_socket);
		session->session_array[i].talk_sock.n_socket = -1;
	}

    /* 关闭session playback socket */
	if (session->session_array[i].pb_sock.n_socket > 0) {
		close(session->session_array[i].pb_sock.n_socket);
		PRINT_INFO("close pb socket:%d\n", session->session_array[i].pb_sock.n_socket);
		session->session_array[i].pb_sock.n_socket = -1;
	}

    session->online_session_num --;
	pthread_mutex_unlock(&(session->global_mutex));

    UtSetControlIndex(session);
    
    return 0;
}


//获取主次码流有多少个客户端请求
int UtGetStreamOnlineNum(int stream_type, UtSession *session)
{
    int num = 0;

    pthread_mutex_lock(&(session->global_mutex));
    if (stream_type == UT_MAIN_STREAM) {
        num = session->main_stream_num;
    } else if(stream_type == UT_SUB_STREAM) {
        num = session->sub_stream_num;
    } else {
        PRINT_ERR("stream_type :%d error.\n", stream_type);
    }
    pthread_mutex_unlock(&(session->global_mutex));

    return num;
}

int UtGetStreamOpenNum(int stream_type, UtSession *session)
{
    int num = 0;
    int i = 0;
    
    pthread_mutex_lock(&(session->global_mutex));

    for (i = 0; i < UT_MAX_LINK_NUM; i++) {
        if (stream_type == UT_MAIN_STREAM) {
            if (session->session_array[i].lv_sock.main_stream_socket.n_flag)
                num ++;
        } else if(stream_type == UT_SUB_STREAM) {
            if (session->session_array[i].lv_sock.sub_stream_socket.n_flag)
                num ++;
        } else {
            PRINT_ERR("stream_type :%d error.\n", stream_type);
        }
    }

    pthread_mutex_unlock(&(session->global_mutex));
    return num;
}



//是的话，返回1； 不是则返回0; 错误返回 -1
int UtIsControlSession(int session_id, UtSession *session)
{
    int index = -1;
    int is_control_session = 0;

    pthread_mutex_lock(&(session->global_mutex));
    index = session->control_index;
    if ((index >=0) && (index < UT_MAX_LINK_NUM)) {
        if (session->session_array[index].session_int == session_id)
            is_control_session = 1;
        else
            is_control_session = 0;
    } else {
        is_control_session = -1; // 错误返回 -1
    }
    //PRINT_INFO("is_control_session:%d, control_index:%d, session_id:%d\n", is_control_session, index, session_id);
    pthread_mutex_unlock(&(session->global_mutex));

    return is_control_session;
}


int UtGetNoticeSockByIndex(int i, UtSession *session)
{
    int sock = -1;

    if (i < 0 || i >= UT_MAX_LINK_NUM) {
        PRINT_ERR("index error. index:%d max_link_num:%d\n", i, UT_MAX_LINK_NUM);
        return -1;
    }

	pthread_mutex_lock(&(session->global_mutex));
    if ((session->session_array[i].flag == 1) &&
        (session->session_array[i].cmd_sock > 0)) {
        sock = session->session_array[i].notice_sock;
	}
	pthread_mutex_unlock(&(session->global_mutex));

    return sock;
}

int utility_net_resolve_host(struct in_addr *sin_addr, const char *hostname)
{

	if (!inet_aton(hostname, sin_addr)) {
		struct addrinfo *ai, *cur;
		struct addrinfo hints = { 0 };
		hints.ai_family = AF_INET;
		if (getaddrinfo(hostname, NULL, &hints, &ai))
			return -1;
		/* getaddrinfo returns a linked list of addrinfo structs.
		 * Even if we set ai_family = AF_INET above, make sure
		 * that the returned one actually is of the correct type. */
		for (cur = ai; cur; cur = cur->ai_next) {
			if (cur->ai_family == AF_INET) {
				*sin_addr = ((struct sockaddr_in *)cur->ai_addr)->sin_addr;
				freeaddrinfo(ai);
				return 0;
			}
		}
		freeaddrinfo(ai);
		return -1;
	}
	return 0;
}

