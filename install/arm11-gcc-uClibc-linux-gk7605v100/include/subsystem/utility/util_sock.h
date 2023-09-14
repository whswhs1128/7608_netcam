/*!
*****************************************************************************
** FileName     : util_sock.h
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


#ifndef __UTIL_SOCK_H__
#define __UTIL_SOCK_H__

#ifdef __cplusplus
extern "C"
{
#endif

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



#define UT_MAX_LINK_NUM 10


typedef struct {
	int n_socket;
	int n_flag;	// 0关闭，1打开
	//int is_set_key; // 是否设置成发送关键帧
	int v_no;
    int is_wait_i; // 是否需要等待关键帧
} UtLvSocketInfo;

typedef struct {
	UtLvSocketInfo main_stream_socket;
	UtLvSocketInfo sub_stream_socket;
} UtLvSocket;

typedef struct {
	int n_socket;
} UtTalkSocket;

typedef struct {
	int n_socket;
} UtPbSocket;

typedef struct {
    int flag;
    int session_int;
    int cmd_sock;
    int notice_sock;
    UtLvSocket lv_sock;
    UtTalkSocket talk_sock;
    UtPbSocket pb_sock;
    char client_ip[16];
    int tmp_sock;
    int c_id; //connection_id
    int send_stream_type;
} UtSessionGlobal;

typedef struct {
    pthread_mutex_t global_mutex;
    UtSessionGlobal session_array[UT_MAX_LINK_NUM];
    int online_session_num;
    int main_stream_num;
    int sub_stream_num;
    int main_is_set_key;
    int sub_is_set_key;
    int control_index;
} UtSession;


typedef enum{
    UT_MAIN_STREAM = 0,
    UT_SUB_STREAM,
    UT_UNKNOWN = 10,
} UT_STREAM_TYPE;



int UtGenSessionInt(UtSession *session);
int UtGenConnectId(UtSession *session);

int UtSessionGlobalInit(UtSession *session);
void UtSessionGlobalUninit(UtSession *session);

int UtAddCmdSock(int socket, int session_id, UtSession *session);
int UtCloseCmdSock(int socket, UtSession *session);
int UtCloseAllSock(UtSession *session);

int UtAddNoticeSock(int socket, int session_id, UtSession *session);
int UtAddTalkSock(int socket, int session_id, UtSession *session);
int UtAddPbSock(int socket, int session_id, UtSession *session);
int UtGetNoticeSock(int session_id, UtSession *session);
int UtGetTalkSock(int session_id, UtSession *session);
int UtGetPbSock(int session_id, UtSession *session);
int UtCloseNoticeSock(int session_id, UtSession *session);
int UtCloseTalkSock(int session_id, UtSession *session);
int UtClosePbSock(int session_id, UtSession *session);

int UtAddLvSock(int socket, int session_id, UT_STREAM_TYPE type, UtSession *session);
int UtAddLvSock2(int socket, char *client_ip, int session_id, int connect_id, UtSession *session);



int UtGetLvSocket(int session_id, UT_STREAM_TYPE type, UtSession *session);
int UtCloseLvSock(int session_id, UT_STREAM_TYPE type, UtSession *session);

int UtSetLvSockStatus(int session_id, int lv_sock_flag, UT_STREAM_TYPE type, UtSession *session);
int UtSetLvSockStatus2(int session_id, int lv_sock_flag, UT_STREAM_TYPE type, UtSession *session);

int UtGetLvSockStatus(int session_id, UT_STREAM_TYPE type, UtSession *session);

int UtSetLvSockStatusByIp(char *ip, int lv_sock_flag, UT_STREAM_TYPE type, UtSession *session);



int UtSetLvKeyFrameStatus(int is_set_key, int type, UtSession *session);
int UtGetLvKeyFrameStatus(int type, UtSession *session);


/////
int UtGetSessionIdByCmdSock(int cmd_sock, UtSession *session);

int UtGetLvSockByIndex(int i, int type, UtSession *session);
int UtCloseLvSockByIndex(int i, int type, UtSession *session);

int UtSetLvSockVNoByIndex(int i, int type, int vno, UtSession *session);
int UtGetLvSockVNoByIndex(int i, int type, UtSession *session);


int UtGetCmdSockByIndex(int i, UtSession *session);
int UtCloseCmdSockByIndex(int i, UtSession *session);

int UtCloseSessionSockByIndex(int i, UtSession *session);

int UtGetStreamOnlineNum(int stream_type, UtSession *session);
int UtGetStreamOpenNum(int stream_type, UtSession *session);


int UtIsControlSession(int sock, UtSession *session);
int UtGetNoticeSockByIndex(int i, UtSession *session);


int UtSetControlIndex(UtSession *session);
int UtGetControlIndex(UtSession *session);

int UtGetLvSockWaitIByIndex(int i, int type, UtSession *session);
int UtSetLvSockWaitIByIndex(int i, int type, int flag, UtSession *session);
int UtGetSidByIndex(int i, UtSession *session);
int UtSetLvSockWaitI(int session_id, UT_STREAM_TYPE type, int flag, UtSession *session);

int utility_net_resolve_host(struct in_addr *sin_addr, const char *hostname);


///



#ifdef __cplusplus
}
#endif

#endif

