/*!
*****************************************************************************
** \file        nvision/software/packages/onvif/src/gk7101/gk7101_cmd.c
**
**
** \brief       Command Communicate
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2013 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "gk7101_cmd.h"
//*****************************************************************************
//*****************************************************************************
//** Local Defines
//*****************************************************************************
//*****************************************************************************
#define MSGSEND_COMM_TYPE 	        10
#define MSGACK_COMM_TYPE 	        11
#define COMM_DATA_MAX   	        200
#define CAMCONTROL_SERVER_PORT      18000
#define HOST                        "127.0.0.1"


//*****************************************************************************
//*****************************************************************************
//** Local structures
//*****************************************************************************
//*****************************************************************************

typedef struct msg_info
{
	unsigned char 	ucCmdType;
	unsigned char 	ucDataLen;
	unsigned short 	reserved;
	unsigned char 	ucCmdData[COMM_DATA_MAX];
	int 		  	AckValue;
}MSG_INFO_t;

//*****************************************************************************
//*****************************************************************************
//** Global Data
//*****************************************************************************
//*****************************************************************************



//*****************************************************************************
//*****************************************************************************
//** Local Data
//*****************************************************************************
//*****************************************************************************

static pthread_mutex_t g_socketMutex;

//*****************************************************************************
//*****************************************************************************
//** Local Functions Declaration
//*****************************************************************************
//*****************************************************************************



//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************

GK_S32 gk7101_Comm_Init()
{
	if(pthread_mutex_init(&g_socketMutex, NULL) < 0)
    {
		return -1;
	}

	return 0;
}

GK_S32 gk7101_Comm_CmdSend(const GK_S8 cmdtype, const GK_U8 datalen,GK_U8 *pCmdData, GK_U8 revdatalen, GK_U8 *pRevData)
{
#if 0
	pthread_mutex_lock(&g_socketMutex);
    GK_S32 retVal;
	MSG_INFO_t  stMsgUserInfo;
	memset(&stMsgUserInfo, 0, sizeof(stMsgUserInfo));
	stMsgUserInfo.ucCmdType = cmdtype;
	stMsgUserInfo.ucDataLen = datalen;
	if(pCmdData != NULL)
    {
		memcpy(stMsgUserInfo.ucCmdData, pCmdData, datalen);
	}
    GK_S32 cltSockFd = -1;
	struct sockaddr_in client_addr;
	memset(&client_addr, 0, sizeof(client_addr));
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(CAMCONTROL_SERVER_PORT);
	client_addr.sin_addr.s_addr = inet_addr(HOST);
    cltSockFd = socket(AF_INET, SOCK_STREAM, 0);
	if (cltSockFd == -1)
    {
		goto run_error;
	}
    /* connect to server */
    retVal = connect(cltSockFd, (struct sockaddr *)&client_addr, sizeof(struct sockaddr));
	if (retVal == -1)
    {
        ONVIF_ERR("connect() socket connect faild!\n");
        close(cltSockFd);
		goto run_error;
	}


	/* send message */
    retVal = send(cltSockFd, &stMsgUserInfo, sizeof(MSG_INFO_t), 0);
	if(retVal == -1)
    {    
        close(cltSockFd);
		goto run_error;
	}
	/* send ack only or ack with video, imaging, PTZ infomation */
	memset(&stMsgUserInfo, 0, sizeof(stMsgUserInfo));
    retVal = recv(cltSockFd, &stMsgUserInfo, sizeof(MSG_INFO_t), MSG_WAITALL);
	if(retVal == -1)
    {        
        close(cltSockFd);
		goto run_error;
	}

    close(cltSockFd);
	if(stMsgUserInfo.AckValue != 0)
    {    
		goto run_error;
	}
	if(revdatalen != stMsgUserInfo.ucDataLen)
    {  
		goto run_error;
	}
	if(stMsgUserInfo.ucDataLen != 0)
    {
		if(pRevData != NULL)
        {
			memcpy(pRevData, stMsgUserInfo.ucCmdData, stMsgUserInfo.ucDataLen);
		}

	}
	
	return 0;
	
run_error:
	pthread_mutex_unlock(&g_socketMutex);
	return -1;
#endif
    return 0;
}

//*****************************************************************************
//*****************************************************************************
//** Local Functions
//*****************************************************************************
//*****************************************************************************

