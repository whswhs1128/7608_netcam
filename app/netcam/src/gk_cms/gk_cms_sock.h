/*!
*****************************************************************************
** FileName     : gk_cms_sock.h
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


#ifndef _GK_CMS_SOCK_H_
#define _GK_CMS_SOCK_H_

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct tagGkLvSocketInfo{
	int n_socket;
	int n_flag;	// 0¹Ø±Õ£¬1´ò¿ª
} GkLvSocketInfo;

typedef struct tagGkLvSocket{
	GkLvSocketInfo main_stream_socket;
	GkLvSocketInfo sub_stream_socket;
} GkLvSocket;

typedef struct tagGkTalkSocket{
	int n_socket;
} GkTalkSocket;

typedef struct tagGkPbSocket{
	int n_socket;
} GkPbSocket;

typedef struct tagSessionGlobal{
    int flag;
    int fSessionInt;
    int main_sock;
    GkLvSocket lv_sock;
    GkTalkSocket talk_sock;
    GkPbSocket pb_sock;
} GkSessionGlobal;

typedef enum{
    GK_MAIN_STREAM = 0,
    GK_SUB_STREAM,
    GK_UNKNOWN,
} GK_CMS_STREAM_TYPE;

int GkGetSessionIdBySock(int main_sock);
int GkSessionGlobalInit(void);
void GkSessionGlobalUninit(void);
int GenSessionInt();
int GkAddMainSock(int socket, int session_id);
int GkCloseMainSock(int socket);
int GkCloseAllSock();
int GkAddLvSock(int socket, int session_id, GK_CMS_STREAM_TYPE type);
int GkCloseLvSock(int session_id, GK_CMS_STREAM_TYPE type);
int GkSetLvSockStatus(int session_id, int lv_sock_flag, GK_CMS_STREAM_TYPE type);
int GkAddTalkSock(int socket, int session_id);
int GkAddPbSock(int socket, int session_id);
int GkGetTalkSock(int session_id);
int GkGetPbSock(int session_id);
int GkCloseTalkSock(int session_id);
int GkClosePbSock(int session_id);
int GkGetLvSockByIndex(int i, int type);
int GkGetLvSocket(int session_id, GK_CMS_STREAM_TYPE type);
int GkIsHaveLvSock(int type);
int GkCloseLvSockByIndex(int i, int type);
int GkCloseAllSockByIndex(int i);

int GkGetMainSockByIndex(int i);
int GkCloseMainSockByIndex(int i);
int GkGetSessionStreamNumber(int sessionNum);
int GkGetIFrameRequest(int sessionNum);
#ifdef __cplusplus
}
#endif

#endif

