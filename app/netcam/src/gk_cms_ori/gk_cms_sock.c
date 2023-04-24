/*!
*****************************************************************************
** FileName     : gk_cms_sock.c
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
#include "common.h"
#include "gk_cms_common.h"
#include "gk_cms_media.h"
#include "gk_cms_sock.h"
#include "gk_cms_utility.h"

pthread_mutex_t gGkSessionGlobal_mutex = PTHREAD_MUTEX_INITIALIZER;

GkSessionGlobal gGkSessionGlobal[GK_MAX_LINK_NUM];
static int gGksessionMainNumber = 0;
static int gGksessionSecdNumber = 0;


//exist,return 1 . not exist, return 0
int IsSessionIdExist(int value)
{
    int i = 0;
    int ret = 0;
    pthread_mutex_lock(&gGkSessionGlobal_mutex);
    for (i = 0; i < GK_MAX_LINK_NUM; i++) {
        if (gGkSessionGlobal[i].fSessionInt == value) {
            ret = 1;
            break;
        }
    }
    pthread_mutex_unlock(&gGkSessionGlobal_mutex);

	return ret;
}

int GenSessionInt()
{
	int value = 0;
    do {
        value = GenRandInt(10000, 20000);
    } while (IsSessionIdExist(value) == 1);

	return value;
}

int GkSessionGlobalInit(void)
{
	int i;
	pthread_mutex_init(&gGkSessionGlobal_mutex, NULL);

	pthread_mutex_lock(&gGkSessionGlobal_mutex);
    for (i = 0; i < GK_MAX_LINK_NUM; i++) {
        memset(&gGkSessionGlobal[i], 0, sizeof(GkSessionGlobal));

		gGkSessionGlobal[i].flag = 0;
		gGkSessionGlobal[i].fSessionInt = -1;
		gGkSessionGlobal[i].main_sock = -1;

		gGkSessionGlobal[i].lv_sock.main_stream_socket.n_socket = -1;
		gGkSessionGlobal[i].lv_sock.main_stream_socket.n_flag = 0;

		gGkSessionGlobal[i].lv_sock.sub_stream_socket.n_socket = -1;
		gGkSessionGlobal[i].lv_sock.sub_stream_socket.n_flag = 0;

		gGkSessionGlobal[i].talk_sock.n_socket = -1;
        gGkSessionGlobal[i].pb_sock.n_socket = -1;
    }
	pthread_mutex_unlock(&gGkSessionGlobal_mutex);

	return 0;
}

void GkSessionGlobalUninit(void)
{
    pthread_mutex_destroy(&gGkSessionGlobal_mutex);
}

/* 添加成功返回0; 连接达到最大数，返回-1;  已经存在，返回1 */
int GkAddMainSock(int socket, int session_id)
{
	int i;
	int ret = -1; // 连接达到最大数，返回-1
	int is_exist = 0;

	pthread_mutex_lock(&gGkSessionGlobal_mutex);
	//判断是否已经存在数组中
    for (i = 0; i < GK_MAX_LINK_NUM; i++) {
        if( (gGkSessionGlobal[i].flag == 1)  &&
			(gGkSessionGlobal[i].fSessionInt == session_id) &&
			(gGkSessionGlobal[i].main_sock == socket) ) {
			is_exist = 1; //已经存在
			ret = 1; //已经存在返回1
			break;
        }
    }

	//如果不存在，则添加进数组
	if (is_exist == 0) {
		ret = -1; // 连接达到最大数，返回-1
	    for (i = 0; i < GK_MAX_LINK_NUM; i++) {
	        if (gGkSessionGlobal[i].flag == 0) {
				gGkSessionGlobal[i].fSessionInt = session_id;
				gGkSessionGlobal[i].main_sock = socket;
				gGkSessionGlobal[i].flag = 1;

				ret = 0; //添加成功返回0
				break;
	        }
	    }
	}

	pthread_mutex_unlock(&gGkSessionGlobal_mutex);
	return ret;
}

int GkCloseMainSock(int socket)
{
    int i;
    int ret = -1;
    pthread_mutex_lock(&gGkSessionGlobal_mutex);

    //判断是否已经存在数组中
    for (i = 0; i < GK_MAX_LINK_NUM; i++) {
        if (gGkSessionGlobal[i].main_sock == socket) {
			/* 关闭liveview 主码流socket */
			if (gGkSessionGlobal[i].lv_sock.main_stream_socket.n_socket > 0) {
				close(gGkSessionGlobal[i].lv_sock.main_stream_socket.n_socket);
                gGksessionMainNumber--;
				PRINT_INFO("close liveview main stream socket:%d\n", gGkSessionGlobal[i].lv_sock.main_stream_socket.n_socket);
			}
			gGkSessionGlobal[i].lv_sock.main_stream_socket.n_socket = -1;
			gGkSessionGlobal[i].lv_sock.main_stream_socket.n_flag = 0;

			/* 关闭liveview 子码流socket */
			if (gGkSessionGlobal[i].lv_sock.sub_stream_socket.n_socket > 0) {
				close(gGkSessionGlobal[i].lv_sock.sub_stream_socket.n_socket);
                gGksessionSecdNumber--;
				PRINT_INFO("close liveview sub stream socket:%d\n", gGkSessionGlobal[i].lv_sock.sub_stream_socket.n_socket);
			}
			gGkSessionGlobal[i].lv_sock.sub_stream_socket.n_socket = -1;
			gGkSessionGlobal[i].lv_sock.sub_stream_socket.n_flag = 0;

			/* 关闭session 主会话 socket */
			if (gGkSessionGlobal[i].main_sock > 0) {
				close(gGkSessionGlobal[i].main_sock);
				PRINT_INFO("close main socket:%d \n", gGkSessionGlobal[i].main_sock);
			}
			gGkSessionGlobal[i].main_sock = -1;
			gGkSessionGlobal[i].fSessionInt = -1;
			gGkSessionGlobal[i].flag = 0;

        	if (gGkSessionGlobal[i].talk_sock.n_socket > 0) {
				close(gGkSessionGlobal[i].talk_sock.n_socket);
				PRINT_INFO("close talk socket:%d\n", gGkSessionGlobal[i].talk_sock.n_socket);

        		gGkSessionGlobal[i].talk_sock.n_socket = -1;
        	}

        	if (gGkSessionGlobal[i].pb_sock.n_socket > 0) {
				close(gGkSessionGlobal[i].pb_sock.n_socket);
				PRINT_INFO("close pb socket:%d\n", gGkSessionGlobal[i].pb_sock.n_socket);

        		gGkSessionGlobal[i].pb_sock.n_socket = -1;
        	}

			ret = 0; //删除成功返回0
			break;
        }
    }
	pthread_mutex_unlock(&gGkSessionGlobal_mutex);
	return ret;
}

int GkCloseAllSock()
{
	int i;
	int ret = -1;
	pthread_mutex_lock(&gGkSessionGlobal_mutex);

	//判断是否已经存在数组中
    for (i = 0; i < GK_MAX_LINK_NUM; i++) {
		/* 关闭liveview 主码流socket */
		if (gGkSessionGlobal[i].lv_sock.main_stream_socket.n_socket > 0) {
			close(gGkSessionGlobal[i].lv_sock.main_stream_socket.n_socket);
			PRINT_INFO("close liveview main stream socket:%d\n", gGkSessionGlobal[i].lv_sock.main_stream_socket.n_socket);
            gGksessionMainNumber--;
		}
		gGkSessionGlobal[i].lv_sock.main_stream_socket.n_socket = -1;
		gGkSessionGlobal[i].lv_sock.main_stream_socket.n_flag = 0;

		/* 关闭liveview 子码流socket */
		if (gGkSessionGlobal[i].lv_sock.sub_stream_socket.n_socket > 0) {
			close(gGkSessionGlobal[i].lv_sock.sub_stream_socket.n_socket);
			PRINT_INFO("close liveview sub stream socket:%d\n", gGkSessionGlobal[i].lv_sock.sub_stream_socket.n_socket);
            gGksessionSecdNumber--;
		}
		gGkSessionGlobal[i].lv_sock.sub_stream_socket.n_socket = -1;
		gGkSessionGlobal[i].lv_sock.sub_stream_socket.n_flag = 0;

		/* 关闭session 主会话 socket */
		if (gGkSessionGlobal[i].main_sock > 0) {
			close(gGkSessionGlobal[i].main_sock);
			PRINT_INFO("close main socket:%d \n", gGkSessionGlobal[i].main_sock);
		}
		gGkSessionGlobal[i].main_sock = -1;
		gGkSessionGlobal[i].fSessionInt = -1;
		gGkSessionGlobal[i].flag = 0;

    	if (gGkSessionGlobal[i].talk_sock.n_socket > 0) {
			close(gGkSessionGlobal[i].talk_sock.n_socket);
			PRINT_INFO("close talk socket:%d\n", gGkSessionGlobal[i].talk_sock.n_socket);

    		gGkSessionGlobal[i].talk_sock.n_socket = -1;
    	}

    	if (gGkSessionGlobal[i].pb_sock.n_socket > 0) {
			close(gGkSessionGlobal[i].pb_sock.n_socket);
			PRINT_INFO("close pb socket:%d\n", gGkSessionGlobal[i].pb_sock.n_socket);

    		gGkSessionGlobal[i].pb_sock.n_socket = -1;
    	}
    }
	pthread_mutex_unlock(&gGkSessionGlobal_mutex);
	return ret;
}


/* 没有对应sessionID返回-1; 设置成功返回0 */
int GkAddTalkSock(int socket, int session_id)
{
	int i;
	int ret = -1;

	pthread_mutex_lock(&gGkSessionGlobal_mutex);
	//判断是否已经存在数组中
    for (i = 0; i < GK_MAX_LINK_NUM; i++) {
        if (gGkSessionGlobal[i].fSessionInt == session_id) {
        	gGkSessionGlobal[i].talk_sock.n_socket = socket;
			ret = 0;
			break;
        }
    }
	pthread_mutex_unlock(&gGkSessionGlobal_mutex);
	return ret;
}

/* 没有对应sessionID返回-1; 设置成功返回0 */
int GkAddPbSock(int socket, int session_id)
{
	int i;
	int ret = -1;

	pthread_mutex_lock(&gGkSessionGlobal_mutex);
	//判断是否已经存在数组中
    for (i = 0; i < GK_MAX_LINK_NUM; i++) {
        if (gGkSessionGlobal[i].fSessionInt == session_id) {
        	gGkSessionGlobal[i].pb_sock.n_socket = socket;
			ret = 0;
            PRINT_INFO("fSessionInt:%d, cmd sock :%d, pb sock:%d \n", gGkSessionGlobal[i].fSessionInt, gGkSessionGlobal[i].main_sock, gGkSessionGlobal[i].pb_sock.n_socket);
			break;
        }
    }
	pthread_mutex_unlock(&gGkSessionGlobal_mutex);
	return ret;
}

int GkGetTalkSock(int session_id)
{
	int i;
	int sock = -1;

	pthread_mutex_lock(&gGkSessionGlobal_mutex);
	//判断是否已经存在数组中
    for (i = 0; i < GK_MAX_LINK_NUM; i++) {
        if (gGkSessionGlobal[i].fSessionInt == session_id) {
			sock = gGkSessionGlobal[i].talk_sock.n_socket;
			break;
		}
    }
	pthread_mutex_unlock(&gGkSessionGlobal_mutex);
	return sock;
}

int GkGetPbSock(int session_id)
{
	int i;
	int sock = -1;

	pthread_mutex_lock(&gGkSessionGlobal_mutex);
	//判断是否已经存在数组中
    for (i = 0; i < GK_MAX_LINK_NUM; i++) {
        if (gGkSessionGlobal[i].fSessionInt == session_id) {
			sock = gGkSessionGlobal[i].pb_sock.n_socket;
			break;
		}
    }
	pthread_mutex_unlock(&gGkSessionGlobal_mutex);
	return sock;
}

int GkCloseTalkSock(int session_id)
{
	int i;
	int ret = -1;

	pthread_mutex_lock(&gGkSessionGlobal_mutex);
	//判断是否已经存在数组中
    for (i = 0; i < GK_MAX_LINK_NUM; i++) {
        if (gGkSessionGlobal[i].fSessionInt == session_id) {
        	if (gGkSessionGlobal[i].talk_sock.n_socket > 0) {
				close(gGkSessionGlobal[i].talk_sock.n_socket);
				PRINT_INFO("close talk socket:%d\n", gGkSessionGlobal[i].talk_sock.n_socket);

        		gGkSessionGlobal[i].talk_sock.n_socket = -1;
        	}

			ret = 0; //删除成功返回0
			break;
		}
    }
	pthread_mutex_unlock(&gGkSessionGlobal_mutex);
	return ret;
}

int GkClosePbSock(int session_id)
{
	int i;
	int ret = -1;

	pthread_mutex_lock(&gGkSessionGlobal_mutex);
	//判断是否已经存在数组中
    for (i = 0; i < GK_MAX_LINK_NUM; i++) {
        if (gGkSessionGlobal[i].fSessionInt == session_id) {
        	if (gGkSessionGlobal[i].pb_sock.n_socket > 0) {
				close(gGkSessionGlobal[i].pb_sock.n_socket);
				PRINT_INFO("close pb socket:%d\n", gGkSessionGlobal[i].pb_sock.n_socket);

        		gGkSessionGlobal[i].pb_sock.n_socket = -1;
        	}

			ret = 0; //删除成功返回0
			break;
		}
    }
	pthread_mutex_unlock(&gGkSessionGlobal_mutex);
	return ret;
}


/* 没有对应sessionID返回-1; 设置成功返回0 */
int GkAddLvSock(int socket, int session_id, GK_CMS_STREAM_TYPE type)
{
	int i;
	int ret = -1;

	pthread_mutex_lock(&gGkSessionGlobal_mutex);
	//判断是否已经存在数组中
    for (i = 0; i < GK_MAX_LINK_NUM; i++) {
        if(gGkSessionGlobal[i].fSessionInt == session_id) {
			if (type == GK_MAIN_STREAM) {
				gGkSessionGlobal[i].lv_sock.main_stream_socket.n_socket = socket;
				gGkSessionGlobal[i].lv_sock.main_stream_socket.n_flag = 0; //默认先关闭，0关闭1打开

                gGksessionMainNumber++;
                PRINT_INFO("cms sock %d add lv main sock %d,sessionNum:%d\n", gGkSessionGlobal[i].main_sock, socket,gGksessionMainNumber);
			} else if(type == GK_SUB_STREAM) {
				gGkSessionGlobal[i].lv_sock.sub_stream_socket.n_socket = socket;
				gGkSessionGlobal[i].lv_sock.sub_stream_socket.n_flag = 0; //默认先关闭，0关闭1打开
                gGksessionSecdNumber++;
                PRINT_INFO("cms sock %d add lv sub sock %d,sessionNum:%d\n", gGkSessionGlobal[i].main_sock, socket,gGksessionSecdNumber);

			}

			ret = 0; //添加成功返回0
			break;
        }
    }
	pthread_mutex_unlock(&gGkSessionGlobal_mutex);
	return ret;
}

int GkCloseLvSock(int session_id, GK_CMS_STREAM_TYPE type)
{
	int i;
	int ret = -1;

	pthread_mutex_lock(&gGkSessionGlobal_mutex);
	//判断是否已经存在数组中
    for (i = 0; i < GK_MAX_LINK_NUM; i++) {
        if (gGkSessionGlobal[i].fSessionInt == session_id) {
			if (type == GK_MAIN_STREAM) {
				if (gGkSessionGlobal[i].lv_sock.main_stream_socket.n_socket > 0) {
					close(gGkSessionGlobal[i].lv_sock.main_stream_socket.n_socket);
                    gGksessionMainNumber--;
					PRINT_INFO("cmd %d close liveview main stream socket:%d\n", gGkSessionGlobal[i].main_sock, gGkSessionGlobal[i].lv_sock.main_stream_socket.n_socket);
				}

				gGkSessionGlobal[i].lv_sock.main_stream_socket.n_socket = -1;
				gGkSessionGlobal[i].lv_sock.main_stream_socket.n_flag = 0;
			} else if(type == GK_SUB_STREAM) {
				if (gGkSessionGlobal[i].lv_sock.sub_stream_socket.n_socket > 0) {
					close(gGkSessionGlobal[i].lv_sock.sub_stream_socket.n_socket);
					gGksessionSecdNumber--;
					PRINT_INFO("cmd %d close liveview sub stream socket:%d\n", gGkSessionGlobal[i].main_sock, gGkSessionGlobal[i].lv_sock.sub_stream_socket.n_socket);

                }

				gGkSessionGlobal[i].lv_sock.sub_stream_socket.n_socket = -1;
				gGkSessionGlobal[i].lv_sock.sub_stream_socket.n_flag = 0;
			}

			ret = 0; //删除成功返回0
			break;
		}
    }
	pthread_mutex_unlock(&gGkSessionGlobal_mutex);
	return ret;
}

int GkSetLvSockStatus(int session_id, int lv_sock_flag, GK_CMS_STREAM_TYPE type)
{
	int i;
	int ret = -1;

	pthread_mutex_lock(&gGkSessionGlobal_mutex);
	//判断是否已经存在数组中
    for (i = 0; i < GK_MAX_LINK_NUM; i++) {
        if (gGkSessionGlobal[i].fSessionInt == session_id) {
			if (type == GK_MAIN_STREAM) {
				gGkSessionGlobal[i].lv_sock.main_stream_socket.n_flag = lv_sock_flag;
			} else if(type == GK_SUB_STREAM) {
				gGkSessionGlobal[i].lv_sock.sub_stream_socket.n_flag = lv_sock_flag;
			}

			ret = 0;
			break;
		}
    }
	pthread_mutex_unlock(&gGkSessionGlobal_mutex);
	return ret;
}


int GkGetLvSocket(int session_id, GK_CMS_STREAM_TYPE type)
{
	int i;
	int sock = -1;

	pthread_mutex_lock(&gGkSessionGlobal_mutex);
	//判断是否已经存在数组中
    for (i = 0; i < GK_MAX_LINK_NUM; i++) {
        if (gGkSessionGlobal[i].fSessionInt == session_id) {
			if (type == GK_MAIN_STREAM) {
				sock = gGkSessionGlobal[i].lv_sock.main_stream_socket.n_socket;
			} else if(type == GK_SUB_STREAM) {
				sock = gGkSessionGlobal[i].lv_sock.sub_stream_socket.n_socket;
			}

			break;
		}
    }
	pthread_mutex_unlock(&gGkSessionGlobal_mutex);
	return sock;
}

int GkGetLvSockStatus(int session_id, GK_CMS_STREAM_TYPE type)
{
	int i;
	int status = -1;

	pthread_mutex_lock(&gGkSessionGlobal_mutex);
	//判断是否已经存在数组中
    for (i = 0; i < GK_MAX_LINK_NUM; i++) {
        if (gGkSessionGlobal[i].fSessionInt == session_id) {
			if (type == GK_MAIN_STREAM) {
				status = gGkSessionGlobal[i].lv_sock.main_stream_socket.n_flag;
			} else if(type == GK_SUB_STREAM) {
				status = gGkSessionGlobal[i].lv_sock.sub_stream_socket.n_flag;
			}

			break;
		}
    }
	pthread_mutex_unlock(&gGkSessionGlobal_mutex);
	return status;
}

int GkGetSessionIdBySock(int main_sock)
{
	int i;
	int session_id = -1;

	pthread_mutex_lock(&gGkSessionGlobal_mutex);
	//判断是否已经存在数组中
    for (i = 0; i < GK_MAX_LINK_NUM; i++) {
        if (gGkSessionGlobal[i].main_sock == main_sock) {
			session_id = gGkSessionGlobal[i].fSessionInt;
			break;
		}
    }
	pthread_mutex_unlock(&gGkSessionGlobal_mutex);
	return session_id;
}

int GkGetLvSockByIndex(int i, int type)
{
    int sock = -1;

	pthread_mutex_lock(&gGkSessionGlobal_mutex);
    if (gGkSessionGlobal[i].flag == 1) {
        if (type == GK_MAIN_STREAM) {
            if ((gGkSessionGlobal[i].lv_sock.main_stream_socket.n_flag == 1) &&
                (gGkSessionGlobal[i].lv_sock.main_stream_socket.n_socket > 0)) {
                sock = gGkSessionGlobal[i].lv_sock.main_stream_socket.n_socket;
            }
        } else if (type == GK_SUB_STREAM) {
            if ((gGkSessionGlobal[i].lv_sock.sub_stream_socket.n_flag == 1) &&
                (gGkSessionGlobal[i].lv_sock.sub_stream_socket.n_socket > 0)) {
                sock = gGkSessionGlobal[i].lv_sock.sub_stream_socket.n_socket;
            }
        }

	}
	pthread_mutex_unlock(&gGkSessionGlobal_mutex);

    return sock;
}

int GkCloseLvSockByIndex(int i, int type)
{
	pthread_mutex_lock(&gGkSessionGlobal_mutex);
    if (gGkSessionGlobal[i].flag == 1) {
		if (type == GK_MAIN_STREAM) {
			if (gGkSessionGlobal[i].lv_sock.main_stream_socket.n_socket > 0) {
				close(gGkSessionGlobal[i].lv_sock.main_stream_socket.n_socket);
				PRINT_INFO("close liveview main stream socket:%d\n", gGkSessionGlobal[i].lv_sock.main_stream_socket.n_socket);
                gGksessionMainNumber--;
			}
			gGkSessionGlobal[i].lv_sock.main_stream_socket.n_socket = -1;
			gGkSessionGlobal[i].lv_sock.main_stream_socket.n_flag = 0;
		} else if(type == GK_SUB_STREAM) {
			if (gGkSessionGlobal[i].lv_sock.sub_stream_socket.n_socket > 0) {
				close(gGkSessionGlobal[i].lv_sock.sub_stream_socket.n_socket);
				PRINT_INFO("close liveview sub stream socket:%d\n", gGkSessionGlobal[i].lv_sock.sub_stream_socket.n_socket);
                gGksessionSecdNumber--;
            }
			gGkSessionGlobal[i].lv_sock.sub_stream_socket.n_socket = -1;
			gGkSessionGlobal[i].lv_sock.sub_stream_socket.n_flag = 0;
		}
	}
	pthread_mutex_unlock(&gGkSessionGlobal_mutex);

    return 0;
}

int GkCloseAllSockByIndex(int i)
{
	pthread_mutex_lock(&gGkSessionGlobal_mutex);

	/* 关闭liveview 主码流socket */
	if (gGkSessionGlobal[i].lv_sock.main_stream_socket.n_socket > 0) {
		close(gGkSessionGlobal[i].lv_sock.main_stream_socket.n_socket);
		PRINT_INFO("close liveview main stream socket:%d\n", gGkSessionGlobal[i].lv_sock.main_stream_socket.n_socket);
        gGksessionMainNumber--;
	}
	gGkSessionGlobal[i].lv_sock.main_stream_socket.n_socket = -1;
	gGkSessionGlobal[i].lv_sock.main_stream_socket.n_flag = 0;

	/* 关闭liveview 子码流socket */
	if (gGkSessionGlobal[i].lv_sock.sub_stream_socket.n_socket > 0) {
		close(gGkSessionGlobal[i].lv_sock.sub_stream_socket.n_socket);
		PRINT_INFO("close liveview sub stream socket:%d\n", gGkSessionGlobal[i].lv_sock.sub_stream_socket.n_socket);
        gGksessionSecdNumber--;
	}
	gGkSessionGlobal[i].lv_sock.sub_stream_socket.n_socket = -1;
	gGkSessionGlobal[i].lv_sock.sub_stream_socket.n_flag = 0;

	/* 关闭session cmd socket */
	if (gGkSessionGlobal[i].main_sock > 0) {
		close(gGkSessionGlobal[i].main_sock);
		PRINT_INFO("close main socket:%d \n", gGkSessionGlobal[i].main_sock);
	}
	gGkSessionGlobal[i].main_sock = -1;
	gGkSessionGlobal[i].fSessionInt = -1;
	gGkSessionGlobal[i].flag = 0;

    /* 关闭session talk socket */
	if (gGkSessionGlobal[i].talk_sock.n_socket > 0) {
		close(gGkSessionGlobal[i].talk_sock.n_socket);
		PRINT_INFO("close talk socket:%d\n", gGkSessionGlobal[i].talk_sock.n_socket);

		gGkSessionGlobal[i].talk_sock.n_socket = -1;
	}

    /* 关闭session playback socket */
	if (gGkSessionGlobal[i].pb_sock.n_socket > 0) {
		close(gGkSessionGlobal[i].pb_sock.n_socket);
		PRINT_INFO("close pb socket:%d\n", gGkSessionGlobal[i].pb_sock.n_socket);

		gGkSessionGlobal[i].pb_sock.n_socket = -1;
	}

	pthread_mutex_unlock(&gGkSessionGlobal_mutex);

    return 0;
}


int GkIsHaveLvSock(int type)
{
    int flag = 0;
    int i = 0;
    int sock = 0;

    for (i = 0; i < GK_MAX_LINK_NUM; i++) {
        sock = GkGetLvSockByIndex(i, type);
        if (sock > 0) {
            flag = 1;
            break;
        }
    }

    return flag;
}


int GkGetMainSockByIndex(int i)
{
    int sock = -1;

	pthread_mutex_lock(&gGkSessionGlobal_mutex);
    if (gGkSessionGlobal[i].flag == 1) {
        if (gGkSessionGlobal[i].main_sock > 0) {
            sock = gGkSessionGlobal[i].main_sock;
        }
	}
	pthread_mutex_unlock(&gGkSessionGlobal_mutex);

    return sock;
}


int GkCloseMainSockByIndex(int i)
{
	pthread_mutex_lock(&gGkSessionGlobal_mutex);

    /* 关闭liveview 主码流socket */
    if (gGkSessionGlobal[i].lv_sock.main_stream_socket.n_socket > 0) {
    	close(gGkSessionGlobal[i].lv_sock.main_stream_socket.n_socket);
    	PRINT_INFO("close liveview main stream socket:%d\n", gGkSessionGlobal[i].lv_sock.main_stream_socket.n_socket);
        gGksessionMainNumber--;
    }
    gGkSessionGlobal[i].lv_sock.main_stream_socket.n_socket = -1;
    gGkSessionGlobal[i].lv_sock.main_stream_socket.n_flag = 0;

    /* 关闭liveview 子码流socket */
    if (gGkSessionGlobal[i].lv_sock.sub_stream_socket.n_socket > 0) {
    	close(gGkSessionGlobal[i].lv_sock.sub_stream_socket.n_socket);
    	PRINT_INFO("close liveview sub stream socket:%d\n", gGkSessionGlobal[i].lv_sock.sub_stream_socket.n_socket);
        gGksessionSecdNumber--;
    }
    gGkSessionGlobal[i].lv_sock.sub_stream_socket.n_socket = -1;
    gGkSessionGlobal[i].lv_sock.sub_stream_socket.n_flag = 0;

    /* 关闭session 主会话 socket */
    if (gGkSessionGlobal[i].main_sock > 0) {
    	close(gGkSessionGlobal[i].main_sock);
    	PRINT_INFO("close main socket:%d \n", gGkSessionGlobal[i].main_sock);
    }
    gGkSessionGlobal[i].main_sock = -1;
    gGkSessionGlobal[i].fSessionInt = -1;
    gGkSessionGlobal[i].flag = 0;

    if (gGkSessionGlobal[i].talk_sock.n_socket > 0) {
    	close(gGkSessionGlobal[i].talk_sock.n_socket);
    	PRINT_INFO("close talk socket:%d\n", gGkSessionGlobal[i].talk_sock.n_socket);

    	gGkSessionGlobal[i].talk_sock.n_socket = -1;
    }

    if (gGkSessionGlobal[i].pb_sock.n_socket > 0) {
    	close(gGkSessionGlobal[i].pb_sock.n_socket);
    	PRINT_INFO("close talk socket:%d\n", gGkSessionGlobal[i].pb_sock.n_socket);

    	gGkSessionGlobal[i].pb_sock.n_socket = -1;
    }

	pthread_mutex_unlock(&gGkSessionGlobal_mutex);

    return 0;
}

//获取主次码流有多少个客户端请求
int GkGetSessionStreamNumber(int sessionNum)
{
    return sessionNum == 0? gGksessionMainNumber:gGksessionSecdNumber;

}
