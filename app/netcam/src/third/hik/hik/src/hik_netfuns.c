/******************************************************************************
  File Name     : hik_netfuns.c
  Version       : v1.0
  Author        : 
  Created       : 2014/11/20
  Description   : 
  History       :
  1.Date        : 2014/11/24
  Author        : 
  Modification: Created file
******************************************************************************/
#include "hik_netfun.h"
#include "hik_netlogin.h"
#include "netcam_api.h"
#include "cfg_all.h"
#include "gk_cms_protocol.h"


extern int g_hik_handle;

extern int writen(int connfd, void *vptr, size_t n);
UINT32 checkByteSum(char *pBuf, int len);

/***********************************************************************
 * Function: convertNetCmdHeaderByteOrder
 * Description:convert network byte order
 * Input:  pHeader:pointer point to NETCMD_HEADER
 * Output:  N/A
 * Return:  NONE 
 ***********************************************************************/
void convertNetCmdHeaderByteOrder(NETCMD_HEADER *pHeader)
{
	pHeader->length 	= ntohl(pHeader->length);
	pHeader->checkSum 	= ntohl(pHeader->checkSum);
	pHeader->netCmd 	= ntohl(pHeader->netCmd);
	pHeader->userID		= ntohl(pHeader->userID);
	pHeader->clientIp	= ntohl(pHeader->clientIp);

	return;
}

/***********************************************************************
 * Function: convertNetLoginReqByteOrder
 * Description:convert network login request byte order
 * Input:  pHeader:pointer point to NETCMD_HEADER
 * Output:  N/A
 * Return:  NONE 
 ***********************************************************************/
void convertNetLoginReqByteOrder(NET_LOGIN_REQ *pLoginReq)
{
	pLoginReq->length 	= ntohl(pLoginReq->length);
	pLoginReq->checkSum = ntohl(pLoginReq->checkSum);
	pLoginReq->netCmd 	= ntohl(pLoginReq->netCmd);
	pLoginReq->version  = ntohl(pLoginReq->version);
	pLoginReq->clientIp	= ntohl(pLoginReq->clientIp); 

	return;
}

int getLoginRetval(int userID,NET_LOGIN_REQ loginData,NET_LOGIN_RET* loginRet)
{
	if(loginRet == NULL)
	{
		return -1;
	}
	
    #if 0//GET DEVICE CFG
 	DMS_NET_DEVICE_INFO stDeviceInfo;
	memset(&stDeviceInfo,0,sizeof(stDeviceInfo));
	int ret = dms_sysnetapi_ioctrl(g_hik_handle, DMS_NET_GET_DEVICECFG, 0, &stDeviceInfo, sizeof(stDeviceInfo)); 
	if(0 != ret)
	{
		HPRINT_ERR("\n");
		return -1;		
	}
    #endif
 
	loginRet->length = htonl(sizeof(NET_LOGIN_RET));
	loginRet->retVal = htonl(NETRET_QUALIFIED);
	//loginRet->devSdkVer = htonl(NETSDK_VERSION4_0);
	loginRet->devSdkVer = htonl(NETSDK_VERSION5_1);
	loginRet->dwUserID = htonl(userID);
    
	//sprintf((char *)loginRet->sSerialNumber,"C001-JBNV%X",(unsigned int)stDeviceInfo.csSerialNumber);
	sprintf((char *)loginRet->sSerialNumber,"C001-GK%X",(unsigned int)112233);

	loginRet->byDVRType = 0;
	loginRet->byChanNum = 1;
	loginRet->byStartChan = 1;
	loginRet->byAlarmInPortNum = 1;
	loginRet->byAlarmOutPortNum = 1;
	loginRet->byDiskNum = 0;
	
	loginRet->byRes1[0] = 0xf0;
	loginRet->byRes1[1] = 0x01;
	loginRet->byRes2[3] = 0x80;
	loginRet->byRes2[4] = 0x25;
	loginRet->byRes2[5] = 0x06;

	loginRet->byRes2[7] = 0x1f;
	loginRet->byRes2[8] = 0x43;	
	loginRet->byRes2[9] = 0x01;	
	//loginRet->byRes2[13] = 0x95;  //次字节决定客户端视频流的协议
	
	loginRet->byRes2[14] = 0x04;
	loginRet->byRes2[15] = 0x01;
	loginRet->byRes2[25] = 0x32;
#if 1
	loginRet->byRes2[4] = 0x61;//0x65;
	loginRet->byRes2[8] = 0x00;//0x43;
	//loginRet->byRes2[13] = 0xd5;
	//loginRet->byRes2[20] = 0x02;
	//loginRet->byRes2[26] = 0x94;
	//loginRet->byRes2[27] = 0x07;

	loginRet->byRes2[14] = 0x00;
	loginRet->byRes2[15] = 0x00;
	loginRet->byRes2[25] = 0x00;
#endif	
	return 0;
}

/*
 * =====================================================================
 * Function: netClientLogin
 * Description:user login 
 * Input:  connfd - socket connfd 
 *         recvbuff - recevie data buff
 *         pClientSockAddr - client socket address(not need)
 * Output:  N/A
 * Return:  OK if successful, otherwise return error status number
 *======================================================================
 */
int netClientLogin(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	struct in_addr clientIpAddr;
	struct sockaddr_in peer;
	NET_LOGIN_REQ loginData;
	NET_LOGIN_RET loginRet;
	int userID;

   	bzero(&peer, sizeof(struct sockaddr_in));
   	bzero(&clientIpAddr, sizeof(struct in_addr));
   	bzero(&loginData, sizeof(NET_LOGIN_REQ));
   	bzero(&loginRet, sizeof(NET_LOGIN_RET));
       
	int len = sizeof(struct sockaddr_in);
	getpeername(connfd, (struct sockaddr*)&peer, (socklen_t *)&len);

	memcpy((char *)&loginData, recvbuff, sizeof(NET_LOGIN_REQ)); 
	convertNetLoginReqByteOrder(&loginData);
	loginRet.length = htonl(sizeof(NET_LOGIN_RET));

	/* check version */
	if(loginData.ifVer != NEW_NETSDK_INTERFACE)
   	{
		HPRINT_ERR("Invalid ifVer 0x%x\n", loginData.ifVer);
		loginRet.retVal = htonl(NETRET_VER_DISMATCH);
	}
	else
	{
		if(loginData.version >= NETSDK_VERSION3_0)
		{
			int retVal = challenge_login(connfd, &loginData, &userID, &peer);
			if(retVal != 0)
			{
				HPRINT_ERR("DS9000 login failed, retVal=0x%X\n", retVal);
				loginRet.retVal = htonl(retVal);
				loginRet.devSdkVer = htonl(CURRENT_NETSDK_VERSION);
			}
			else
			{
				getLoginRetval(userID, loginData, &loginRet);
			}
		}
	}

	loginRet.checkSum = checkByteSum((char *)&(loginRet.retVal), sizeof(NET_LOGIN_RET)-8);
    
	return writen(connfd, (char *)&loginRet, sizeof(NET_LOGIN_RET));
}

/*
 * =====================================================================
 * Function: netClientReLogin
 * Description:user login again 
 * Input:  connfd - socket connfd 
 *         recvbuff - recevie data buff
 *         pClientSockAddr - client socket address(not need)
 * Output:  N/A
 * Return:  OK if successful, otherwise return error status number
 *======================================================================
 */
int netClientReLogin(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	NET_LOGIN_REQ loginData;
	NET_LOGIN_RET loginRet;
	struct in_addr clientIpAddr;
    struct sockaddr_in peer;
	int userID;
	int retVal;
	int len;
	
	unsigned char PrivateKey[16] = {0x6a,0x68,0xa3,0x61,0xbf,0x6e,0xb5,0x67,0xcd,0x7a,0xfe,0x68,0xca,0x6f,0xde,0x75};
	unsigned char desIn[16];

	bzero(&peer,sizeof(struct sockaddr_in));
	bzero(&clientIpAddr,sizeof(struct in_addr));
	bzero(&loginData,sizeof(NET_LOGIN_REQ));
	bzero(&loginRet, sizeof(NET_LOGIN_RET));

	len = sizeof(struct sockaddr_in);
	getpeername(connfd, (struct sockaddr*)&peer, (socklen_t *)&len);

	memcpy((char *)&loginData, recvbuff, sizeof(NET_LOGIN_REQ));
	convertNetLoginReqByteOrder(&loginData);

	loginRet.length = htonl(sizeof(NET_LOGIN_RET));
	if ( loginData.ifVer != NEW_NETSDK_INTERFACE )
   	{
		HPRINT_ERR("Invalid ifVer 0x%x\n", loginData.ifVer);
		loginRet.retVal = htonl(NETRET_VER_DISMATCH);
	}
	else
   	{	
		HPRINT_INFO("netClientLogin: sdkVersion = 0x%08x\n", loginData.version);

		//抓包分析需要屏蔽此代码
#if 1
		if ( loginData.version>CURRENT_NETSDK_VERSION )
	   	{
			loginData.version = CURRENT_NETSDK_VERSION;
		}
#endif
		/*username encrypt*/
		bzero(desIn, 16);
		memcpy(desIn, loginData.username, 16);
		bzero(loginData.username, 32);
		des2key(PrivateKey, 1);
		D2des(desIn, loginData.username);

		/*password encrypt*/
		bzero(desIn, 16);
		memcpy(desIn, loginData.password, 16);
		bzero(loginData.password, 16);
		des2key(PrivateKey, 1);
		D2des(desIn, loginData.password);

		HPRINT_INFO("username:%s, password:%s\n", loginData.username, loginData.password);
		/* try to login */
		clientIpAddr.s_addr = loginData.clientIp;	
		retVal = userLogin((char *)loginData.username, (char *)loginData.password, &clientIpAddr,
								&peer.sin_addr, (char *)loginData.clientMac, &userID, loginData.version|RELOGIN_FLAG);
		if(retVal == 0) 
		{
			getLoginRetval(userID,loginData, &loginRet);
            loginRet.checkSum = 0x0ac2;
            loginRet.retVal   = 0x01000000;
            loginRet.devSdkVer = 0xd1240202;
		}
		else
		{
			HPRINT_ERR("userLogin failed: retval = 0x%x\n", retVal);
			loginRet.retVal = htonl(retVal);
			loginRet.devSdkVer = htonl(CURRENT_NETSDK_VERSION);
		}
	}
    
	loginRet.checkSum = checkByteSum((char *)&(loginRet.retVal), sizeof(NET_LOGIN_RET)-8);
    HPRINT_INFO("loginRet.retVal=0x%X\n", loginRet.retVal);    
	return writen(connfd, (char *)&loginRet, sizeof(NET_LOGIN_RET));
}


/*
* =====================================================================
* Function: netClientLogout
* Description:user logout
* Input:	connfd - socket connfd 
* 		recvbuff - recevie data buff
* 		pClientSockAddr - client socket address(not need)
* Output:  N/A
* Return:  OK if successful, otherwise return error status number
*======================================================================
*/
int netClientLogout(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	NETCMD_HEADER	netCmdHeader;
	UINT32 retVal;
	struct in_addr clientIpAddr;
	struct sockaddr_in peer;

	bzero(&clientIpAddr,sizeof(struct in_addr));
	bzero(&peer, sizeof(struct sockaddr_in));	

	memcpy(&netCmdHeader, recvbuff, sizeof(NETCMD_HEADER));
	convertNetCmdHeaderByteOrder(&netCmdHeader);

	retVal = sizeof(struct sockaddr_in);
	getpeername(connfd, (struct sockaddr*)&peer, &retVal);

	clientIpAddr.s_addr = netCmdHeader.clientIp;
	/* logout */
	if (userLogout(FALSE, netCmdHeader.userID, &clientIpAddr, &peer.sin_addr, (char *)netCmdHeader.clientMac) == 0)
	{
		retVal = NETRET_QUALIFIED;
	}
	else
	{
		retVal = NETRET_NO_USERID;
	}

	return sendNetRetval(connfd, retVal);
}

/*
* =====================================================================
* Function: netClientUserExchange
* Description:user exchange - keep alive
* Input:	connfd - socket connfd 
* 		recvbuff - recevie data buff
* 		pClientSockAddr - client socket address(not need)
* Output:  N/A
* Return:  OK if successful, otherwise return error status number
*======================================================================
*/
int netClientUserExchange(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	NETCMD_HEADER	netCmdHeader;
	struct in_addr clientIpAddr;
	struct sockaddr_in peer;
	UINT32 retVal;

	bzero(&peer,sizeof(struct sockaddr_in));

	memcpy((char *)&netCmdHeader, recvbuff, sizeof(NETCMD_HEADER));
	//HPRINT_INFO("netCmdHeader.userID=%d\n", netCmdHeader.userID);
	convertNetCmdHeaderByteOrder(&netCmdHeader);
	HPRINT_INFO("netCmdHeader.userID=%d\n", netCmdHeader.userID);

	retVal = sizeof(struct sockaddr_in);
	getpeername(connfd, (struct sockaddr*)&peer, &retVal);

	clientIpAddr.s_addr = netCmdHeader.clientIp;
	if(userKeepAlive(netCmdHeader.userID, &clientIpAddr, &peer.sin_addr, (char *)netCmdHeader.clientMac) == 0)
	{
		retVal = NETRET_QUALIFIED;
	}
	else
	{
		retVal = NETRET_NO_USERID;
	}

	return sendNetRetval(connfd, retVal);
}


 /**********************************************************************
 * Function:  netClientGetIpCoreCfgEx
 * Description: net get netIp core config, supports domain name
 * Input: 
 *	   connfd - socket connfd 
 *       recvbuff - recevie data buff
 *       pClientSockAddr - client socket address(not need)
 * Output:  N/A
 * Return:  OK if successful, otherwise return error status number
 **********************************************************************/
int netClientGetIpCoreCfgV31(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	NETCMD_HEADER  netCmdHeader;
	struct{
		NETRET_HEADER header;
		NETPARAM_IPCORE_CFG_V31 ipCoreCfg;
	}netRetIpCfg;
	UINT32 retVal;
	int sendlen = 0;

	sendlen = sizeof(netRetIpCfg);
	bzero((char *)&netRetIpCfg, sendlen);
	netRetIpCfg.header.length = htonl(sendlen);
	
	memcpy((char *)&netCmdHeader, recvbuff, sizeof(NETCMD_HEADER));
	retVal = verifyNetClientOperation(&netCmdHeader, pClientSockAddr, REMOTESHOWPARAMETER);
	if(retVal == NETRET_QUALIFIED)
	{
		//pthreadMutexLock(&netIpcCfgSem, WAIT_FOREVER);
		//convert_IpCoreCfg_To_NetIpCfg_V31();
		//pthreadMutexUnlock(&netIpcCfgSem);
					
		netRetIpCfg.ipCoreCfg.length = htonl(sizeof(NETPARAM_IPCORE_CFG_V31));		
		//getNetIpDevCfgV31(&netRetIpCfg.ipCoreCfg.ipDevCfg);	
	}

	netRetIpCfg.header.retVal = htonl(retVal);
	netRetIpCfg.header.checkSum = htonl(checkByteSum((char *)&(netRetIpCfg.header.retVal), sendlen-8));
	HPRINT_INFO("netRetIpCfg.header.retVal=0x%X\n", netRetIpCfg.header.retVal);
	return writen(connfd, (char *)&netRetIpCfg, sendlen);
}


/**********************************************************************
 * Function:  netClientGetIpAlarmInCfg
 * Description: net get netIp alarmIn config
 * Input: 
 *	   connfd - socket connfd 
 *       recvbuff - recevie data buff
 *       pClientSockAddr - client socket address(not need)
 * Output:  N/A
 * Return:  OK if successful, otherwise return error status number
 **********************************************************************/
int netClientGetIpAlarmInCfg(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	NETCMD_HEADER  netCmdHeader;
	struct{
		NETRET_HEADER header;
		NETPARAM_IPALARMIN_CFG ipAlarmInCfg;
	}netRetIpCfg;
	UINT32 retVal;
	int sendlen = 0;

	sendlen = sizeof(netRetIpCfg);
	bzero((char *)&netRetIpCfg, sendlen);
	netRetIpCfg.header.length = htonl(sendlen);
	
	memcpy((char *)&netCmdHeader, recvbuff, sizeof(NETCMD_HEADER));
	retVal = verifyNetClientOperation(&netCmdHeader, pClientSockAddr, REMOTESHOWPARAMETER);
	if(retVal == NETRET_QUALIFIED)
	{
		//pthreadMutexLock(&netIpcCfgSem, WAIT_FOREVER);
		//convert_IpAlarmInCfg_To_NetIpCfg();
		//pthreadMutexUnlock(&netIpcCfgSem);
		
		netRetIpCfg.ipAlarmInCfg.length = htonl(sizeof(NETPARAM_IPALARMIN_CFG));		
		//getNetIpAlarmInCfg(&netRetIpCfg.ipAlarmInCfg);
	}

	netRetIpCfg.header.retVal = htonl(retVal);
	netRetIpCfg.header.checkSum = htonl(checkByteSum((char *)&(netRetIpCfg.header.retVal), sendlen-8));
	HPRINT_INFO("netRetIpCfg.header.retVal=0x%X\n", netRetIpCfg.header.retVal);
	return writen(connfd, (char *)&netRetIpCfg, sendlen);
}


/**********************************************************************
 * Function:  checkNetIpAlarmInCfgValid
 * Description: check netIp alarmIn config valid
 * Input: ipAlarmInCfg----the struct of NETPARAM_IPALARMIN_CFG
 * Output:  N/A
 * Return:  OK if successful, otherwise return error status number
 **********************************************************************/
static int checkNetIpAlarmInCfgValid(NETPARAM_IPALARMIN_CFG ipAlarmInCfg)
{
	int i;
	int alarmIn;
	int retVal = 0;

	for(i = 0; i < MAX_IP_ALARMIN; i++)
	{
		alarmIn = ipAlarmInCfg.ipAlarmInInfo[i].alarmIn;
		if(alarmIn > 6)
		{
			retVal = -1;
			break;	
		}
	}

	return retVal;
}

/**********************************************************************
 * Function:  netClientSetIpAlarmInCfg
 * Description: net set netIp alarmIn config
 * Input: 
 *	   connfd - socket connfd 
 *       recvbuff - recevie data buff
 *       pClientSockAddr - client socket address(not need)
 * Output:  N/A
 * Return:  OK if successful, otherwise return error status number
 **********************************************************************/
int netClientSetIpAlarmInCfg(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	struct{
		NETCMD_HEADER header;
		NETPARAM_IPALARMIN_CFG ipAlarmInCfg;
	}netCmdIpCfg;
	int retVal;

	memcpy((char *)&netCmdIpCfg, recvbuff, sizeof(netCmdIpCfg));
	retVal = verifyNetClientOperation(&(netCmdIpCfg.header), pClientSockAddr, REMOTESETPARAMETER);
	if(retVal == NETRET_QUALIFIED)
	{
		if(ntohl(netCmdIpCfg.ipAlarmInCfg.length) != sizeof(NETPARAM_IPALARMIN_CFG))
		{	/* check length */
			HPRINT_INFO("Error length : %d, %d\n", ntohl(netCmdIpCfg.ipAlarmInCfg.length), sizeof(NETPARAM_IPALARMIN_CFG));
			retVal = NETRET_ERROR_DATA;
			goto errExit;
		}
		else
		{
			if(checkNetIpAlarmInCfgValid(netCmdIpCfg.ipAlarmInCfg) != 0)
			{
				retVal = NETRET_ERROR_DATA;
				goto errExit;	
			}

			/* copy to pDevCfgParam */
			//pthreadMutexLock(&netIpcCfgSem, WAIT_FOREVER);
			//setNetIpAlarmInCfg(netCmdIpCfg.ipAlarmInCfg);
			//pthreadMutexUnlock(&netIpcCfgSem);
			
			pthreadMutexLock(&globalMSem, WAIT_FOREVER);
			//if(writeDevParam(pDevCfgParam)!=OK)
			//{
			//	retVal = NETRET_DVR_OPER_FAILED;
			//}
			pthreadMutexUnlock(&globalMSem);

		}
	}

errExit:
	return sendNetRetval(connfd, retVal);
}

extern pthread_mutex_t gAlarmMutex;
//extern int gAlarmSocketfd;
void netClientStartAlarmUpChannel(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr, BOOL b9000Sdk)
{
	NETCMD_HEADER netCmdHeader;
	UINT32 retVal;

	memcpy((char *)&netCmdHeader, recvbuff, sizeof(NETCMD_HEADER));

	// 1. check valid
	retVal = verifyNetClientOperation(&netCmdHeader, pClientSockAddr, REMOTEALARM);
	if(retVal != NETRET_QUALIFIED)
	{
		HPRINT_ERR("netClientStartAlarmUpChannel.\n");
	}
	
	// 2. respond
	sendNetRetval(connfd, retVal);
	//sendNetRetval(gAlarmSocketfd, retVal);

	// 3. livekeep
	HPRINT_INFO("alarm start.\n");
	typedef struct tagAlarmKeep{
		char value[8];
	}AlarmKeep;
	AlarmKeep objAlarmKeep;
	bzero(&objAlarmKeep, sizeof(AlarmKeep));
	objAlarmKeep.value[3] = 0x08;
	objAlarmKeep.value[7] = 0x02;		// NETRET_EXCHANGE

	char buffer[64] = {0};
	memcpy(buffer, &objAlarmKeep, sizeof(AlarmKeep));

	int gAlarmSocketfd = connfd;
	int gWorkState = 1;
	while (gWorkState)
	{
	    int nLen = 0;
		//pthread_mutex_lock(&gAlarmMutex);
        if (gAlarmSocketfd > 0)
        {
    		nLen = writen(gAlarmSocketfd, buffer, sizeof(AlarmKeep));
    		if (nLen < 0){
    			gWorkState = 0;
    			HPRINT_ERR("alarm send failed\n");
    		}
        }
		//pthread_mutex_unlock(&gAlarmMutex);

		sleep(5);
		//HPRINT_INFO("alarm livekeep. ret:%d sockfd:%d\n", nLen, gAlarmSocketfd);
	}

	// 4. close
	//pthread_mutex_lock(&gAlarmMutex);
	//HPRINT_INFO("close gAlarmSocketfd:%d\n", gAlarmSocketfd);
	//close(gAlarmSocketfd);
	//gAlarmSocketfd = -1;
	//pthread_mutex_unlock(&gAlarmMutex);
	HPRINT_INFO("alarm stop.\n");
}

int netClientGetWorkStatus(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	NETCMD_HEADER netCmdHeader;
	struct{
		NETRET_HEADER header;
		DVR_WORKSTATUS workStatus;
	}netRetDVRStatus;
	//ENC_CHAN_STATUS *pEncStatus;
	//HDISK_STATUS *pHdStatus;
	//UINT8 *pAlarmInStatus, *pAlarmOutStatus;
	UINT32 retVal, sendLen=0;

	memcpy((char *)&netCmdHeader, recvbuff, sizeof(NETCMD_HEADER));

	retVal = verifyNetClientOperation(&netCmdHeader, pClientSockAddr, REMOTESHOWSTATUS);
	if(retVal == NETRET_QUALIFIED)
	{
		sendLen = sizeof(netRetDVRStatus);
		bzero((char *)&netRetDVRStatus, sendLen);
		netRetDVRStatus.header.length = htonl(sendLen);
		netRetDVRStatus.header.retVal = htonl(retVal);
		netRetDVRStatus.workStatus.deviceStatus = htonl(0);
		netRetDVRStatus.workStatus.localDispStatus = htonl(0);
		//pEncStatus = netRetDVRStatus.workStatus.chanStatus;
		//pHdStatus = netRetDVRStatus.workStatus.hdStatus;
		//pAlarmInStatus = netRetDVRStatus.workStatus.alarmInStatus;
		//pAlarmOutStatus = netRetDVRStatus.workStatus.alarmOutStatus;

#if 0
		/* channel status */
		for(chanIdx=FIRST_CHAN_NO; chanIdx<=maxEnChanNo; chanIdx++)
		{
			pChan = &chanPara[chanIdx];
			getEncStatus(chanIdx,&encStatus);
			
			pEncStatus[MAP_CHANIDX(chanIdx)].bRecStarted
				= pChan->recStarted?1:0;
			pEncStatus[MAP_CHANIDX(chanIdx)].bViLost
				= encStatus.bHaveSignal?0:1;
			pEncStatus[MAP_CHANIDX(chanIdx)].bitRate
				= htonl(encStatus.bps);
			pEncStatus[MAP_CHANIDX(chanIdx)].chanStatus
				= (encStatus.bps>0)?0:1;

			pthreadMutexLock(&pChan->mutexSem, WAIT_FOREVER);
			pEncStatus[MAP_CHANIDX(chanIdx)].netLinks
				= htonl(pChan->netLinks+pChan->subNetLinks);
			for(i=0, pSlot=(NET_CONNECT *)lstFirst(&pChan->netConnectList);
				pSlot!=NULL&&(i<MAX_LINK); pSlot=(NET_CONNECT *)lstNext(&pSlot->node), i++)
			{
				pEncStatus[MAP_CHANIDX(chanIdx)].clientIP[i]
					= htonl(pSlot->peerAddr.sin_addr.s_addr);
			}
			pthreadMutexUnlock(&pChan->mutexSem);
		}
		
		/* hard disk status */
		for(ctrl=0; ctrl<MAX_DISK_CTRLS; ctrl++)
		{
			for(drive=0; drive<ATA_MAX_DRIVES; drive++)
			{
				int hdIdx=0;

				getHdiskParam(ctrl, drive, &pHd);
				if(pHd->exist)
				{
					/*if(pHd->hdType==HD_TYPE_NFS)
					{
						hdIdx = NFS_CTRL + MK_NFSDSK_IDX(ctrl, drive);
					}
					else
					{*/
						//hdIdx = isSATACtrl()?ctrl:(ctrl*2+drive);
						hdIdx = ctrl*2+drive;
					//}

					if(pHd->hdType==HD_TYPE_NFS)
					{
						if((pHd->hdStatus&HD_NFSDSK_MOUNT_FAILED) != 0)
						{
							pHdStatus[hdIdx].totalSpace = htonl(0);
						}
						else
						{
							pHdStatus[hdIdx].totalSpace = htonl(getHdTotalSpace(ctrl, drive));
						}
					}
					else
					{
						pHdStatus[hdIdx].totalSpace = htonl(getHdTotalSpace(ctrl, drive));
					}
					pHdStatus[hdIdx].freeSpace = htonl(getHdFreeSpace(ctrl, drive));
					temp = 0;
					if(pHd->hdStatus&HD_IDLE)
					{
						temp |= 1;
					}
					if(pHd->hdStatus&HD_ERROR)
					{
						temp |= 2;
					}
					pHdStatus[hdIdx].diskStatus = htonl(temp);
				}
			}
		}
		
		/* alarmIn */
		temp = getAlarmInStatus();
		for(i=0; i<devHardInfo.alarmInNums; i++)
		{
			if((temp&(1<<i)) != 0)
			{
				pAlarmInStatus[i] = 1;
			}
			else
			{
				pAlarmInStatus[i] = 0;
			}
		}

		/* alarmOut */
		temp = getAlarmOutStatus();
		for(i=0; i<devHardInfo.alarmOutNums; i++)
		{
			if((temp&(1<<i)) != 0)
			{
				pAlarmOutStatus[i] = 1;
			}	
			else
			{
				pAlarmOutStatus[i] = 0;
			}
		}
#endif
	}

	netRetDVRStatus.header.retVal = htonl(retVal);
	netRetDVRStatus.header.checkSum = htonl(checkByteSum((char *)&(netRetDVRStatus.header.retVal), sendLen-8));
	return writen(connfd, (char *)&netRetDVRStatus, sendLen);
}


int netClientGetWorkStatusV30(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{

#if 0
	NETCMD_HEADER netCmdHeader;
	struct{
		NETRET_HEADER header;
		DVR_WORKSTATUS_V30 workStatus;
	}netRetDVRStatusV30;
	//ENC_CHAN_STATUS_V30 *pEncStatus;
	//HDISK_STATUS *pHdStatus;
	//UINT8 *pAlarmInStatus, *pAlarmOutStatus;
	UINT32 retVal, sendLen=0;

	memcpy((char *)&netCmdHeader, recvbuff, sizeof(NETCMD_HEADER));

	retVal = verifyNetClientOperation(&netCmdHeader, pClientSockAddr, REMOTESHOWSTATUS);
	if(retVal == NETRET_QUALIFIED)
	{
		sendLen = sizeof(netRetDVRStatusV30);
		bzero((char *)&netRetDVRStatusV30, sendLen);
		netRetDVRStatusV30.header.length = htonl(sendLen);
		netRetDVRStatusV30.header.retVal = htonl(retVal);
		netRetDVRStatusV30.workStatus.deviceStatus = htonl(0);
		netRetDVRStatusV30.workStatus.localDispStatus = htonl(0);
		//pEncStatus = netRetDVRStatusV30.workStatus.chanStatus;
		//pHdStatus = netRetDVRStatusV30.workStatus.hdStatus;
		//pAlarmInStatus = netRetDVRStatusV30.workStatus.alarmInStatus;
		//pAlarmOutStatus = netRetDVRStatusV30.workStatus.alarmOutStatus;
		//netRetDVRStatusV30.workStatus.audioInChanStatus = getAudioInChanStatus(); /*audioInChanStatus*/
		netRetDVRStatusV30.workStatus.audioInChanStatus = 0;


		/*analog channel status */
		for(chanIdx=FIRST_CHAN_NO; chanIdx<=maxEnChanNo; chanIdx++)
		{
			pChan = getChanPara(chanIdx);
			getEncStatus(chanIdx,&encStatus);
			pEncStatus[MAP_CHANIDX(chanIdx)].bRecStarted = pChan->recStarted?1:0;
			pEncStatus[MAP_CHANIDX(chanIdx)].bViLost = encStatus.bHaveSignal?0:1;
			pEncStatus[MAP_CHANIDX(chanIdx)].bitRate = htonl(encStatus.bps);
			pEncStatus[MAP_CHANIDX(chanIdx)].chanStatus = (encStatus.bps>0)?0:1;
			
			pthreadMutexLock(&pChan->mutexSem, WAIT_FOREVER);
			pEncStatus[MAP_CHANIDX(chanIdx)].netLinks = htonl(pChan->netLinks+pChan->subNetLinks);
			for(i=0, pSlot=(NET_CONNECT *)lstFirst(&pChan->netConnectList);
				pSlot!=NULL&&(i<MAX_LINK); pSlot=(NET_CONNECT *)lstNext(&pSlot->node), i++)
			{
				pEncStatus[MAP_CHANIDX(chanIdx)].clientIP[i].v4.s_addr = htonl(pSlot->peerAddr.sin_addr.s_addr);
			}
			pthreadMutexUnlock(&pChan->mutexSem);
		}
		
		/*ip chan status*/
		for(chanIdx=FIRST_CHAN_NO; chanIdx<=maxIPCcount; chanIdx++)
		{		
			pChan = getChanPara(maxEnChanNo + chanIdx);
			pEncStatus[MAP_CHANIDX(MAX_CHANNUM + chanIdx)].bRecStarted = pChan->recStarted?1:0;
			
			pthreadMutexLock(&pChan->mutexSem, WAIT_FOREVER);
			pEncStatus[MAP_CHANIDX(MAX_CHANNUM+chanIdx)].netLinks = htonl(pChan->netLinks+pChan->subNetLinks);
			for(i=0, pSlot=(NET_CONNECT *)lstFirst(&pChan->netConnectList);
				pSlot!=NULL&&(i<MAX_LINK); pSlot=(NET_CONNECT *)lstNext(&pSlot->node), i++)
			{
				pEncStatus[MAP_CHANIDX(MAX_CHANNUM+chanIdx)].clientIP[i].v4.s_addr = htonl(pSlot->peerAddr.sin_addr.s_addr);
			}
			pthreadMutexUnlock(&pChan->mutexSem);

			if(ipcIsConnected(chanIdx)==FALSE)
			{
				pEncStatus[MAP_CHANIDX(MAX_CHANNUM + chanIdx)].bViLost = 1;
				continue;
			}
			
			if(getIPCWorkStatus(chanIdx, &ipcWorkStatus)==OK)
			{
				ipcPara = getIPCameraCfgPara(chanIdx);
				ipcChan = ipcPara->channel;
				pEncStatus[MAP_CHANIDX(MAX_CHANNUM + chanIdx)].bViLost = ipcWorkStatus.chanStatus[ipcChan-1].bViLost; 
				pEncStatus[MAP_CHANIDX(MAX_CHANNUM + chanIdx)].bitRate = ipcWorkStatus.chanStatus[ipcChan-1].bitRate; 
				pEncStatus[MAP_CHANIDX(MAX_CHANNUM + chanIdx)].chanStatus = ipcWorkStatus.chanStatus[ipcChan-1].chanStatus;
				pEncStatus[MAP_CHANIDX(MAX_CHANNUM + chanIdx)].ipcNetLinks = ipcWorkStatus.chanStatus[ipcChan-1].netLinks;
			}
		}
		
		/* hard disk status */
		for(ctrl=0; ctrl<MAX_DISK_CTRLS; ctrl++)
		{
			for(drive=0; drive<ATA_MAX_DRIVES; drive++)
			{
				int hdIdx=0;

				getHdiskParam(ctrl, drive, &pHd);
				if(pHd->exist)
				{
					/*if(pHd->hdType==HD_TYPE_NFS)
					{
						hdIdx = NFS_CTRL + MK_NFSDSK_IDX(ctrl, drive);
					}
					else
					{*/
						//hdIdx = isSATACtrl()?ctrl:(ctrl*2+drive);
						hdIdx = ctrl*2+drive;
					//}

					if(pHd->hdType==HD_TYPE_NFS)
					{
						if((pHd->hdStatus&HD_NFSDSK_MOUNT_FAILED) != 0)
						{
							pHdStatus[hdIdx].totalSpace = htonl(0);
						}
						else
						{
							pHdStatus[hdIdx].totalSpace = htonl(getHdTotalSpace(ctrl, drive));
						}
					}
					else
					{
						pHdStatus[hdIdx].totalSpace = htonl(getHdTotalSpace(ctrl, drive));
					}
					pHdStatus[hdIdx].freeSpace = htonl(getHdFreeSpace(ctrl, drive));
					temp = 0;
					if(pHd->hdStatus&HD_IDLE)
					{
						temp |= 1;
					}
					if(pHd->hdStatus&HD_ERROR)
					{
						temp |= 2;
					}
					pHdStatus[hdIdx].diskStatus = htonl(temp);
				}
			}
		}
		
		/* alarmIn */
		temp = getAlarmInStatus();
		for(i=0; i<devHardInfo.alarmInNums; i++)
		{
			if((temp&(1<<i)) != 0)
			{
				pAlarmInStatus[i] = 1;
			}
			else
			{
				pAlarmInStatus[i] = 0;
			}
		}

		/* alarmOut */
		temp = getAlarmOutStatus();
		for(i=0; i<devHardInfo.alarmOutNums; i++)
		{
			if((temp&(1<<i)) != 0)
			{
				pAlarmOutStatus[i] = 1;
			}	
			else
			{
				pAlarmOutStatus[i] = 0;
			}
		}

	}

	netRetDVRStatusV30.header.retVal = htonl(retVal);
	netRetDVRStatusV30.header.checkSum = htonl(checkByteSum((char *)&(netRetDVRStatusV30.header.retVal), sendLen-8));
	HPRINT_INFO("netRetDVRStatusV30.header.retVal=0x%X\n", netRetDVRStatusV30.header.retVal);

	return writen(connfd, (char *)&netRetDVRStatusV30, sendLen);
#endif
	char sendbuf[40] = {0};
	return writen(connfd, (char *)&sendbuf, sizeof(sendbuf));
}


/*********************************************************************
 * Function: netClientGetDeviceCfg
 * Description:get device parameter
 * Input:
 *		  connfd - socket connfd 
 *        recvbuff - recevie data buff
 *        pClientSockAddr - client socket address(not need)
 * Output:  N/A
 * Return:  OK if successful, otherwise return error status number
 *********************************************************************/
int netClientGetDeviceCfg(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	NETCMD_HEADER  netCmdHeader;
	struct{
		NETRET_HEADER header;
		NETPARAM_DEVICE_CFG deviceCfg;
	}netRetDeviceCfg;
	UINT32 retVal;
	int sendlen;

	sendlen = sizeof(netRetDeviceCfg);
	bzero((char *)&netRetDeviceCfg, sendlen);
	netRetDeviceCfg.header.length = htonl(sendlen);

	memcpy((char *)&netCmdHeader, recvbuff, sizeof(NETCMD_HEADER));
	retVal = verifyNetClientOperation(&netCmdHeader, pClientSockAddr, REMOTESHOWPARAMETER);
	if(retVal == NETRET_QUALIFIED)
	{
	    //QMAPI_NET_DEVICE_INFO DeviceInfo;
		//memset(&DeviceInfo,0,sizeof(DeviceInfo));
		//QMapi_sys_ioctrl(g_hik_handle,QMAPI_SYSCFG_GET_DEVICECFG, 0,&DeviceInfo, sizeof(DeviceInfo));
		
		netRetDeviceCfg.deviceCfg.length = htonl(sizeof(NETPARAM_DEVICE_CFG));
		//strcpy((char *)netRetDeviceCfg.deviceCfg.DVRName, "Hikvision C1");
		strncpy((char *)netRetDeviceCfg.deviceCfg.DVRName, runSystemCfg.deviceInfo.deviceName, sizeof(netRetDeviceCfg.deviceCfg.DVRName));

		//strcpy((char *)netRetDeviceCfg.deviceCfg.DVRName,DeviceInfo.csDeviceName);
		
		netRetDeviceCfg.deviceCfg.deviceID = htonl(112233);
		//netRetDeviceCfg.deviceCfg.deviceID = htonl(DeviceInfo.dwDeviceID);
		
		netRetDeviceCfg.deviceCfg.recycleRecord = htonl(runRecordCfg.recordLen);
		//netRetDeviceCfg.deviceCfg.recycleRecord = DeviceInfo.byRecordLen;
		//strcpy((char *)netRetDeviceCfg.deviceCfg.serialno, "CS-C1-11WPFR0120130720BBRR096600991");
		//strcpy((char *)netRetDeviceCfg.deviceCfg.serialno,DeviceInfo.csSerialNumber);
		strncpy((char *)netRetDeviceCfg.deviceCfg.serialno, runSystemCfg.deviceInfo.serialNumber, sizeof(netRetDeviceCfg.deviceCfg.serialno));
		netRetDeviceCfg.deviceCfg.softwareVersion = htonl(0x05020002);//htonl(DeviceInfo.dwSoftwareVersion);//0x03000104;//0X04010003;
		netRetDeviceCfg.deviceCfg.softwareBuildDate = htonl(0x000e091c);//htonl(DeviceInfo.dwSoftwareBuildDate);//0x19090d00;//0X0D0919;
		netRetDeviceCfg.deviceCfg.dspSoftwareVersion = htonl(0x00060000);//htonl(DeviceInfo.dwDspSoftwareVersion);//0x0400;//0X040000;
		netRetDeviceCfg.deviceCfg.dspSoftwareBuildDate = htonl(0x000e091e);//htonl(DeviceInfo.dwDspSoftwareBuildDate);//0x0a080d00;//0X0D080A;
		netRetDeviceCfg.deviceCfg.panelVersion = 0;//htonl(DeviceInfo.dwPanelVersion);//0;
		netRetDeviceCfg.deviceCfg.hardwareVersion = 0;//htonl(DeviceInfo.dwHardwareVersion);//0;
		netRetDeviceCfg.deviceCfg.alarmInNums = 1;//DeviceInfo.byAlarmInNum;//0;
		netRetDeviceCfg.deviceCfg.alarmOutNums = 1;//DeviceInfo.byAlarmOutNum;//0;
		netRetDeviceCfg.deviceCfg.rs232Nums = 1;//DeviceInfo.byRS232Num;//1;
		netRetDeviceCfg.deviceCfg.rs485Nums = 1;//DeviceInfo.byRS485Num;//1;
		netRetDeviceCfg.deviceCfg.netIfNums = 1;//DeviceInfo.byNetworkPortNum;//1;
		netRetDeviceCfg.deviceCfg.hdiskCtrlNums = 0;//DeviceInfo.byDiskCtrlNum;//0;
		netRetDeviceCfg.deviceCfg.hdiskNums = 0;//DeviceInfo.byDiskNum;//0;
		netRetDeviceCfg.deviceCfg.devType = NETRET_IPCAMERA;//NETRET_MEGA_IPCAMERA;//IPC_6F_20P
		netRetDeviceCfg.deviceCfg.channelNums = 2;
		netRetDeviceCfg.deviceCfg.firstChanNo = 0;
		netRetDeviceCfg.deviceCfg.decodeChans = 1;//DeviceInfo.byDecordChans;//1;
		netRetDeviceCfg.deviceCfg.vgaNums = 0;//DeviceInfo.byVGANum;//0;
		netRetDeviceCfg.deviceCfg.usbNums = 0;//DeviceInfo.byUSBNum;//0;
		netRetDeviceCfg.deviceCfg.auxOutNum = 0;//DeviceInfo.byAuxOutNum;//1;
		netRetDeviceCfg.deviceCfg.audioNum = 1;//DeviceInfo.byAudioInNum;//0;
		netRetDeviceCfg.deviceCfg.ipChanNum = 0;
		//HPRINT_INFO("alarmIn = %d, alarmOut = %d, hdiskCtrl = %d, hdisk = %d, enc = %d, dec = %d\n",
		//				netRetDeviceCfg.deviceCfg.alarmInNums, netRetDeviceCfg.deviceCfg.alarmOutNums,
		//				netRetDeviceCfg.deviceCfg.hdiskCtrlNums, netRetDeviceCfg.deviceCfg.hdiskNums,
		//				netRetDeviceCfg.deviceCfg.channelNums, netRetDeviceCfg.deviceCfg.decodeChans);
	}

	netRetDeviceCfg.header.retVal = htonl(retVal);
	netRetDeviceCfg.header.checkSum = htonl(checkByteSum((char *)&(netRetDeviceCfg.header.retVal), sendlen-8));
	//HPRINT_INFO("netRetDeviceCfg.header.retVal=0x%X\n", netRetDeviceCfg.header.retVal);

	return writen(connfd, (char *)&netRetDeviceCfg, sendlen);
}

/******************************************************************************
 * Function: netClientSetDeviceCfg
 * Description:set device parameter
 * Input:  
 *		   connfd - socket connfd 
 *         recvbuff - recevie data buff
 *         pClientSockAddr - client socket address(not need)
 * Output:  N/A
 * Return:  OK if successful, otherwise return error status number
 *****************************************************************************/
int netClientSetDeviceCfg(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	struct{
		NETCMD_HEADER header;
		NETPARAM_DEVICE_CFG deviceCfg;
	}netCmdDeviceCfg;
	UINT32 retVal;
	//DVR_HDDTOREC_CFG logHddToRecCfg;

	memcpy((char *)&netCmdDeviceCfg, recvbuff, sizeof(netCmdDeviceCfg));
	retVal = verifyNetClientOperation(&(netCmdDeviceCfg.header), pClientSockAddr, REMOTESETPARAMETER);

	if(retVal == NETRET_QUALIFIED)
	{
		if(ntohl(netCmdDeviceCfg.deviceCfg.length) != sizeof(NETPARAM_DEVICE_CFG))
		{	/* check length */
			HPRINT_ERR("Error length : %u, %u\n", ntohl(netCmdDeviceCfg.deviceCfg.length), sizeof(NETPARAM_DEVICE_CFG));
			retVal = NETRET_ERROR_DATA;
		}
		else
		{
			//if (pDevCfgParam->hdiskPara.cyclicRecord[0] != (ntohl(netCmdDeviceCfg.deviceCfg.recycleRecord)==0)?0:1)
			//{
				//logHddToRecCfg.cyclicRecord = (ntohl(netCmdDeviceCfg.deviceCfg.recycleRecord)==0)?0:1;
				//writeOperationLogInfo(netCmdDeviceCfg.header.userID, MINOR_REMOTE_CFG_PARM, PARA_ENCODE, MODIFY_HDDREC,
				//		(char *)&logHddToRecCfg, sizeof(DVR_HDDTOREC_CFG) );

			//}
			/* copy to pDevCfgParam */
			pthreadMutexLock(&globalMSem, WAIT_FOREVER);
			//memcpy(pDevCfgParam->deviceName, netCmdDeviceCfg.deviceCfg.DVRName, NAME_LEN);
			//pDevCfgParam->deviceId = min(ntohl(netCmdDeviceCfg.deviceCfg.deviceID), 255);
			//pDevCfgParam->deviceId = max(1,pDevCfgParam->deviceId);
			//pDevCfgParam->hdiskPara.cyclicRecord[0] = (ntohl(netCmdDeviceCfg.deviceCfg.recycleRecord)==0)?0:1;
			//if(writeDevParam(pDevCfgParam)!=OK)
			//{
			//	retVal = NETRET_DVR_OPER_FAILED;
			//}
			pthreadMutexUnlock(&globalMSem);

			/* take effect */
			//sendPanelCommand(SET_DEVNO_CMD, (UINT8)pDevCfgParam->deviceId, 0);
		}
	}

	return sendNetRetval(connfd, retVal);
}

/*********************************************************************
 * Function: netClientGetDeviceCfgV40
 * Description:get device parameter
 * Input:
 *        connfd - socket connfd 
 *        recvbuff - recevie data buff
 *        pClientSockAddr - client socket address(not need)
 * Output:  N/A
 * Return:  OK if successful, otherwise return error status number
 *********************************************************************/
int netClientGetDeviceCfgV40(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
    NETCMD_HEADER  netCmdHeader;
    struct{
        NETRET_HEADER header;
        NETPARAM_DEVICE_CFG_V40 deviceCfg;
    }netRetDeviceCfg;
    UINT32 retVal;
    int sendlen=0;

    sendlen = sizeof(netRetDeviceCfg);
    bzero((char *)&netRetDeviceCfg, sendlen);
    netRetDeviceCfg.header.length = htonl(sendlen);

    memcpy((char *)&netCmdHeader, recvbuff, sizeof(NETCMD_HEADER));
    retVal = verifyNetClientOperation(&netCmdHeader, pClientSockAddr, REMOTESHOWPARAMETER);
    if(retVal == NETRET_QUALIFIED)
    {
		#if 0//GET DEVICE CFG
		DMS_NET_DEVICE_INFO DeviceInfo;
		memset(&DeviceInfo,0,sizeof(DeviceInfo));
		dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_GET_DEVICECFG,0,&DeviceInfo,sizeof(DeviceInfo));	
   		#endif
		
		netRetDeviceCfg.deviceCfg.length = htonl(sizeof(NETPARAM_DEVICE_CFG_V40));		
        //strcpy((char *)netRetDeviceCfg.deviceCfg.DVRName,DeviceInfo.csDeviceName);
        strcpy((char *)netRetDeviceCfg.deviceCfg.DVRName, "Hikvision C1");
        
        //netRetDeviceCfg.deviceCfg.deviceID = htonl(DeviceInfo.dwDeviceID);
        netRetDeviceCfg.deviceCfg.deviceID = htonl(112233);
        
        //netRetDeviceCfg.deviceCfg.recycleRecord = htonl(DeviceInfo.byRecycleRecord);
        netRetDeviceCfg.deviceCfg.recycleRecord = htonl(runRecordCfg.recordLen);
        
		//sprintf((char *)netRetDeviceCfg.deviceCfg.serialno,"C001-JBNV%X",(unsigned int)DeviceInfo.csSerialNumber);
        //strcpy((char *)netRetDeviceCfg.deviceCfg.serialno, "CS-C1-11WPFR0120130720BBRR096600991");
        sprintf((char *)netRetDeviceCfg.deviceCfg.serialno,"C001-GOKE%X",112233);

        
        //netRetDeviceCfg.deviceCfg.softwareVersion = htonl(DeviceInfo.dwSoftwareVersion);
        //netRetDeviceCfg.deviceCfg.softwareBuildDate = htonl(DeviceInfo.dwSoftwareBuildDate);
        //netRetDeviceCfg.deviceCfg.dspSoftwareVersion = htonl(DeviceInfo.dwDspSoftwareVersion);
        //netRetDeviceCfg.deviceCfg.dspSoftwareBuildDate = htonl(DeviceInfo.dwDspSoftwareBuildDate);
        //netRetDeviceCfg.deviceCfg.panelVersion = htonl(DeviceInfo.dwPanelVersion);
        //netRetDeviceCfg.deviceCfg.hardwareVersion = htonl(DeviceInfo.dwHardwareVersion);
		netRetDeviceCfg.deviceCfg.softwareVersion = htonl(0x05020002);//htonl(DeviceInfo.dwSoftwareVersion);//0x03000104;//0X04010003;
		netRetDeviceCfg.deviceCfg.softwareBuildDate = htonl(0x000e091c);//htonl(DeviceInfo.dwSoftwareBuildDate);//0x19090d00;//0X0D0919;
		netRetDeviceCfg.deviceCfg.dspSoftwareVersion = htonl(0x00060000);//htonl(DeviceInfo.dwDspSoftwareVersion);//0x0400;//0X040000;
		netRetDeviceCfg.deviceCfg.dspSoftwareBuildDate = htonl(0x000e091e);//htonl(DeviceInfo.dwDspSoftwareBuildDate);//0x0a080d00;//0X0D080A;
		netRetDeviceCfg.deviceCfg.panelVersion = 0;//htonl(DeviceInfo.dwPanelVersion);//0;
		netRetDeviceCfg.deviceCfg.hardwareVersion = 0;//htonl(DeviceInfo.dwHardwareVersion);//0;

        
        //netRetDeviceCfg.deviceCfg.alarmInNums = DeviceInfo.byAlarmInNum;
        //netRetDeviceCfg.deviceCfg.alarmOutNums = DeviceInfo.byAlarmOutNum;
        //netRetDeviceCfg.deviceCfg.rs232Nums = DeviceInfo.byRS232Num;
        //netRetDeviceCfg.deviceCfg.rs485Nums = DeviceInfo.byRS485Num;
        //netRetDeviceCfg.deviceCfg.netIfNums = DeviceInfo.byNetworkPortNum;
        //netRetDeviceCfg.deviceCfg.hdiskCtrlNums = DeviceInfo.byDiskCtrlNum;
        //netRetDeviceCfg.deviceCfg.hdiskNums = DeviceInfo.byDiskNum;
		netRetDeviceCfg.deviceCfg.alarmInNums = 0;//DeviceInfo.byAlarmInNum;//0;
		netRetDeviceCfg.deviceCfg.alarmOutNums = 0;//DeviceInfo.byAlarmOutNum;//0;
		netRetDeviceCfg.deviceCfg.rs232Nums = 0;//DeviceInfo.byRS232Num;//1;
		netRetDeviceCfg.deviceCfg.rs485Nums = 0;//DeviceInfo.byRS485Num;//1;
		netRetDeviceCfg.deviceCfg.netIfNums = 1;//DeviceInfo.byNetworkPortNum;//1;
		netRetDeviceCfg.deviceCfg.hdiskCtrlNums = 0;//DeviceInfo.byDiskCtrlNum;//0;
		netRetDeviceCfg.deviceCfg.hdiskNums = 0;//DeviceInfo.byDiskNum;//0;

        netRetDeviceCfg.deviceCfg.unknown1 = 0;
        netRetDeviceCfg.deviceCfg.channelNums = 1;
        netRetDeviceCfg.deviceCfg.firstChanNo = 0;
        netRetDeviceCfg.deviceCfg.decodeChans = 1;
        netRetDeviceCfg.deviceCfg.vgaNums = 0;
        netRetDeviceCfg.deviceCfg.usbNums = 0;
        netRetDeviceCfg.deviceCfg.auxOutNum = 0;
        netRetDeviceCfg.deviceCfg.audioNum = 1;
        netRetDeviceCfg.deviceCfg.ipChanNum = 0;
        
		netRetDeviceCfg.deviceCfg.zeroChanNum = 0;
		netRetDeviceCfg.deviceCfg.supportAbility = 0;
        netRetDeviceCfg.deviceCfg.bESataFun = 0;
		netRetDeviceCfg.deviceCfg.bEnableIPCPnp = 0;
      	netRetDeviceCfg.deviceCfg.unknown2 = 1;
      	netRetDeviceCfg.deviceCfg.unknown3 = 1;
      	netRetDeviceCfg.deviceCfg.unknown4 = 0;
      	netRetDeviceCfg.deviceCfg.CPUType = NETRET_IPCAMERA;		
		strcpy((char *)netRetDeviceCfg.deviceCfg.devType,"IPC");
    }

    netRetDeviceCfg.header.retVal = htonl(retVal);
    netRetDeviceCfg.header.checkSum = htonl(checkByteSum((char *)&(netRetDeviceCfg.header.retVal), sendlen-8));

	return writen(connfd, (char *)&netRetDeviceCfg, sendlen);
}

/******************************************************************************
 * Function: netClientSetDeviceCfgV40
 * Description:set device parameter
 * Input:  
 *         connfd - socket connfd 
 *         recvbuff - recevie data buff
 *         pClientSockAddr - client socket address(not need)
 * Output:  N/A
 * Return:  OK if successful, otherwise return error status number
 *****************************************************************************/
int netClientSetDeviceCfgV40(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
    struct{
        NETCMD_HEADER header;
        NETPARAM_DEVICE_CFG_V40 deviceCfg;
    }netCmdDeviceCfg;
    UINT32 retVal;

	memset(&netCmdDeviceCfg,0,sizeof(netCmdDeviceCfg));
    memcpy((char *)&netCmdDeviceCfg, recvbuff, sizeof(netCmdDeviceCfg));
    retVal = verifyNetClientOperation(&(netCmdDeviceCfg.header), pClientSockAddr, REMOTESETPARAMETER);
    if(retVal == NETRET_QUALIFIED)
    {
        if(ntohl(netCmdDeviceCfg.deviceCfg.length) != sizeof(NETPARAM_DEVICE_CFG_V40))
        {   
            HPRINT_ERR("Error length : %d, %d\n", ntohl(netCmdDeviceCfg.deviceCfg.length), sizeof(NETPARAM_DEVICE_CFG_V40));
            retVal = NETRET_ERROR_DATA;
        }
        else
        {
            #if 0//GET DEVICE CFG
        	DMS_NET_DEVICE_INFO DeviceInfo;
			memset(&DeviceInfo,0,sizeof(DeviceInfo));
			dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_GET_DEVICECFG,0,&DeviceInfo,sizeof(DeviceInfo));
			strcpy(DeviceInfo.csDeviceName,(char *)netCmdDeviceCfg.deviceCfg.DVRName);
			DeviceInfo.dwDeviceID = ntohl(netCmdDeviceCfg.deviceCfg.deviceID);
			DeviceInfo.byRecycleRecord = ntohl(netCmdDeviceCfg.deviceCfg.recycleRecord);
			dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_SET_DEVICECFG,0,&DeviceInfo,sizeof(DeviceInfo));
			dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_SET_SAVECFG,0,NULL,0);
           #else
            runRecordCfg.recordLen = ntohl(netCmdDeviceCfg.deviceCfg.recycleRecord);
            #endif
        }
    }

    return sendNetRetval(connfd, retVal);
}


/**********************************************************************
 * Function: netClientGetAlarmInCfg
 * Description:get alarmIn config param
 * Input: 
 *		 connfd - socket connfd 
 *       recvbuff - recevie data buff
 *       pClientSockAddr - client socket address(not need)
 * Output:  N/A
 * Return:  OK if successful, otherwise return error status number
 **********************************************************************/
int netClientGetAlarmInCfgV30(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	NETCMD_CHAN_HEADER netGetChanCfg;
	struct{
		NETRET_HEADER header;
		NETPARAM_ALARMIN_CFG_V30 alarmInCfg;
	}netRetAlarmInCfg;
	UINT32 retVal;
	int sendlen, alarmInNum;
	
	sendlen = sizeof(netRetAlarmInCfg);
	bzero((char *)&netRetAlarmInCfg, sendlen);
	netRetAlarmInCfg.header.length = htonl(sendlen);

	memcpy((char *)&netGetChanCfg, recvbuff, sizeof(NETCMD_CHAN_HEADER));
	alarmInNum = netGetChanCfg.channel = ntohl(netGetChanCfg.channel);	/* alarmIn chan */

	if(checkAlarmInNumValid(alarmInNum) != 0)
	{
		retVal = NETRET_NO_ALARMIN;
		HPRINT_ERR("Invalid alarmIn channel %d\n", alarmInNum);
	}
	else
	{
		retVal = verifyNetClientOperation(&(netGetChanCfg.header), pClientSockAddr, REMOTESHOWPARAMETER);
		if(retVal == NETRET_QUALIFIED)
		{
			if(isAnalogAlarmInNum(alarmInNum))
			{
				//netGetAnalogAlarmInCfgV30(alarmInNum, &(netRetAlarmInCfg.alarmInCfg));
			}
			else if(isNetIpAlarmInNum(alarmInNum))
			{
				alarmInNum = alarmInNum - MAX_ALARMIN;/*alarmInNum:0~128 !!!*/
				HPRINT_INFO("IPC alarmInNum is %d\n", alarmInNum);

				#if 0				/*mask the third factory's IPC*/
				if(getIpChanByAlarmInNum(alarmInNum + 1, &ipChan) == -1)
				{
					HPRINT_ERR("getIpChanByAlarmInNum failed !\n");
					retVal = NETRET_DVR_OPER_FAILED;
					goto errExit;
				}
				if(getIPCFactory(ipChan)!=FAC_HIKVISION)
				{
					HPRINT_ERR("channel %d  Invalid  factory!\n", ipChan);
					retVal = NETRET_NOT_SUPPORT;
					goto errExit;
				}
				#endif
				
				//if(netGetIpAlarmInCfgV30(alarmInNum, &(netRetAlarmInCfg.alarmInCfg)) != 0)
				//{
				//	retVal = NETRET_DVR_OPER_FAILED;	
				//}
			}
		}
	}

	netRetAlarmInCfg.header.retVal = htonl(retVal);
	netRetAlarmInCfg.header.checkSum = htonl(checkByteSum((char *)&(netRetAlarmInCfg.header.retVal), sendlen-8));
	HPRINT_INFO("netRetAlarmInCfg.header.retVal=0x%X\n", netRetAlarmInCfg.header.retVal);

	return writen(connfd, (char *)&netRetAlarmInCfg, sendlen);
}


/**********************************************************************
 * Function: netClientSetAlarmInCfg 
 * Description:set alarmIn config param
 * Input: 
 *		 connfd - socket connfd 
 *       recvbuff - recevie data buff
 *       pClientSockAddr - client socket address(not need)
 * Output:  N/A
 * Return:  OK if successful, otherwise return error status number
 **********************************************************************/
int netClientSetAlarmInCfgV30(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	struct{
		NETCMD_HEADER header;
		UINT32 channel;
		NETPARAM_ALARMIN_CFG_V30 alarmInCfg;
	}netCmdAlarmInCfg;
	UINT32 retVal;
	int alarmInNum;
	//int userID;

	memcpy((char *)&netCmdAlarmInCfg, recvbuff, sizeof(netCmdAlarmInCfg));
	alarmInNum = netCmdAlarmInCfg.channel = ntohl(netCmdAlarmInCfg.channel);	/* alarmIn chan */

	if(checkAlarmInNumValid(alarmInNum) != 0)
	{
		retVal = NETRET_NO_ALARMIN;
		HPRINT_ERR("Invalid alarmIn num %d\n", alarmInNum);
	}
	else
	{
		retVal = verifyNetClientOperation(&(netCmdAlarmInCfg.header), pClientSockAddr, REMOTESETPARAMETER);
		//userID = netCmdAlarmInCfg.header.userID;
		if(retVal == NETRET_QUALIFIED)
		{
			//check length 
			if(ntohl(netCmdAlarmInCfg.alarmInCfg.length) != sizeof(NETPARAM_ALARMIN_CFG_V30))
			{	
				HPRINT_ERR("Error length : %d, %d\n", ntohl(netCmdAlarmInCfg.alarmInCfg.length), sizeof(NETPARAM_ALARMIN_CFG_V30));
				retVal = NETRET_ERROR_DATA;
			}
			else
			{
#if 0
				//check parameter 
				if(checkAlarmInCfgValid(netCmdAlarmInCfg.alarmInCfg) != OK)
				{
				    HPRINT_ERR("Alarm in parameter error\n");
					retVal = NETRET_ERROR_DATA;
					goto errExit;
				}

				//copy to pDevCfgParam 
				if(isAnalogAlarmInNum(alarmInNum))
				{
					pthreadMutexLock(&globalMSem, WAIT_FOREVER);
					netSetAnalogAlarmInCfgV30(alarmInNum, netCmdAlarmInCfg.alarmInCfg);
					if(writeDevParam(pDevCfgParam)!=OK)
					{
						retVal = NETRET_DVR_OPER_FAILED;
					}	
					pthreadMutexUnlock(&globalMSem);
				}
				else if(isNetIpAlarmInNum(alarmInNum))
				{
					alarmInNum = alarmInNum - MAX_ALARMIN;
					#if 1				/*mask the third factory's IPC*/
					retVal = getIpChanByAlarmInNum(alarmInNum + 1, &ipChan);
					if(retVal == -1)
					{
						HPRINT_ERR("getIpChanByAlarmInNum failed !\n");
						retVal = NETRET_DVR_OPER_FAILED;
						goto errExit;
					}
					if(getIPCFactory(ipChan)!=FAC_HIKVISION)
					{
						HPRINT_ERR("channel %d  Invalid  factory!\n", ipChan);
						retVal = NETRET_NOT_SUPPORT;
						goto errExit;
					}
					#endif					
					if(netSetIpAlarmInCfgV30(userID, alarmInNum, netCmdAlarmInCfg.alarmInCfg) != 0)
					{
						HPRINT_ERR("netSetIpAlarmInCfgV30 failed!!!\n");
						retVal = NETRET_DVR_OPER_FAILED;
						goto errExit;
					}
					retVal = netSaveDevParam();
				}
#endif
			}
		}
	}

	return sendNetRetval(connfd, retVal);
}

/**********************************************************************
 * Function:  netClientGetAlarmOutCfg
 * Description: get alarmOut config param
 * Input: 
 *		 connfd - socket connfd 
 *       recvbuff - recevie data buff
 *       pClientSockAddr - client socket address(not need)
 * Output:  N/A
 * Return:  OK if successful, otherwise return error status number
 **********************************************************************/
int netClientGetAlarmOutCfgV30(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	NETCMD_CHAN_HEADER netGetChanCfg;
	struct{
		NETRET_HEADER header;
		NETPARAM_ALARMOUT_CFG_V30 alarmOutCfg;
	}netRetAlarmOutCfg;
	UINT32 retVal;
	int sendlen, alarmOutNum;

	sendlen = sizeof(netRetAlarmOutCfg);
	bzero((char *)&netRetAlarmOutCfg, sendlen);
	netRetAlarmOutCfg.header.length = htonl(sendlen);

	memcpy((char *)&netGetChanCfg, recvbuff, sizeof(NETCMD_CHAN_HEADER));
	alarmOutNum = netGetChanCfg.channel = ntohl(netGetChanCfg.channel);	

	if(checkAlarmOutNumValid(alarmOutNum) != 0)
	{
		retVal = NETRET_NO_ALARMOUT;
		HPRINT_ERR("Invalid alarmOut channel %d\n", alarmOutNum);
	}
	else
	{
		retVal = verifyNetClientOperation(&(netGetChanCfg.header), pClientSockAddr, REMOTESHOWPARAMETER);
		if(retVal == NETRET_QUALIFIED)
		{
			if(isAnalogAlarmOutNum(alarmOutNum))
			{
				//netGetAnalogAlarmOutCfgV30(alarmOutNum, &(netRetAlarmOutCfg.alarmOutCfg));
			}
			else if(isNetIpAlarmOutNum(alarmOutNum))
			{
				alarmOutNum = alarmOutNum - MAX_ALARMOUT;
#if 0
				if(getIpChanByAlarmOutNum(alarmOutNum + 1, &ipChan))
				{
					HPRINT_ERR("getIpChanByAlarmInNum failed !\n");
					retVal = NETRET_DVR_OPER_FAILED;
					goto errExit;
				}
				if(getIPCFactory(ipChan)!=FAC_HIKVISION)
				{
					HPRINT_ERR("channel %d  Invalid  factory!\n", ipChan);
					retVal = NETRET_NOT_SUPPORT;
					goto errExit;
				}				
#endif
				//if(netGetIpAlarmOutCfgV30(alarmOutNum, &(netRetAlarmOutCfg.alarmOutCfg)) != OK)
				//{
				//	retVal = NETRET_DVR_OPER_FAILED;	
				//}
			}
		}
	}

	netRetAlarmOutCfg.header.retVal = htonl(retVal);
	netRetAlarmOutCfg.header.checkSum = htonl(checkByteSum((char *)&(netRetAlarmOutCfg.header.retVal), sendlen-8));
	HPRINT_INFO("netRetAlarmOutCfg.header.retVal=0x%X\n", netRetAlarmOutCfg.header.retVal);

	return writen(connfd, (char *)&netRetAlarmOutCfg, sendlen);
}

/**********************************************************************
 * Function: netClientSetAlarmOutCfg
 * Description:set alarmOut config param
 * Input: 
 *		 connfd - socket connfd 
 *       recvbuff - recevie data buff
 *       pClientSockAddr - client socket address(not need)
 * Output:  N/A
 * Return:  OK if successful, otherwise return error status number
 **********************************************************************/
int netClientSetAlarmOutCfgV30(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	struct{
		NETCMD_HEADER header;
		UINT32 channel;
		NETPARAM_ALARMOUT_CFG_V30 alarmOutCfg;
	}netCmdAlarmOutCfg;
	UINT32 retVal;
	int alarmOutNum;
	//int userID;

	memcpy((char *)&netCmdAlarmOutCfg, recvbuff, sizeof(netCmdAlarmOutCfg));
	alarmOutNum = netCmdAlarmOutCfg.channel = ntohl(netCmdAlarmOutCfg.channel);	

	if(checkAlarmOutNumValid(alarmOutNum) != 0)
	{
		retVal = NETRET_NO_ALARMOUT;
		HPRINT_ERR("Invalid alarmOut channel %d\n",alarmOutNum);
	}
	else
	{
		retVal = verifyNetClientOperation(&(netCmdAlarmOutCfg.header), pClientSockAddr, REMOTESETPARAMETER);
		//userID = netCmdAlarmOutCfg.header.userID;
		if(retVal == NETRET_QUALIFIED)
		{
			//check length 
			if(ntohl(netCmdAlarmOutCfg.alarmOutCfg.length) != sizeof(NETPARAM_ALARMOUT_CFG_V30))
			{	
				HPRINT_ERR("Error length : %d, %d\n", ntohl(netCmdAlarmOutCfg.alarmOutCfg.length), sizeof(NETPARAM_ALARMOUT_CFG_V30));
				retVal = NETRET_ERROR_DATA;
			}
			else
			{
#if 0
				//check parameter 
				if(checkAlarmOutCfgValidV30(netCmdAlarmOutCfg.alarmOutCfg) != 0)
				{
					retVal = NETRET_ERROR_DATA;
					goto errExit;
				}
			
				//copy to pDevCfgParam 
				if(isAnalogAlarmOutNum(alarmOutNum))
				{
					pthreadMutexLock(&globalMSem, WAIT_FOREVER);
					netSetAnalogAlarmOutCfgV30(alarmOutNum, netCmdAlarmOutCfg.alarmOutCfg);
					if(writeDevParam(pDevCfgParam) != 0)
					{
						retVal = NETRET_DVR_OPER_FAILED;
					}
					pthreadMutexUnlock(&globalMSem);
				}
				else if(isNetIpAlarmOutNum(alarmOutNum))
				{
					alarmOutNum = alarmOutNum - MAX_ALARMOUT;
					HPRINT_INFO("IPC alarmOutNum is %d\n", alarmOutNum);
					#if 1
					if(getIpChanByAlarmOutNum(alarmOutNum + 1, &ipChan))
					{
						HPRINT_ERR("getIpChanByAlarmInNum failed !\n");
						retVal = NETRET_DVR_OPER_FAILED;
						goto errExit;
					}
					if(getIPCFactory(ipChan)!=FAC_HIKVISION)
					{
						HPRINT_ERR("channel %d  Invalid  factory!\n", ipChan);
						retVal = NETRET_NOT_SUPPORT;
						goto errExit;
					}				
					#endif
					if(netSetIpAlarmOutCfgV30(userID, alarmOutNum, netCmdAlarmOutCfg.alarmOutCfg) != OK)
					{
						retVal = NETRET_DVR_OPER_FAILED;
					}
					else
					{
						retVal = netSaveDevParam();
					}
				}
#endif
			}
		}
	}

	return sendNetRetval(connfd, retVal);
}

int netClientPTZControl(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	NET_PTZ_CTRL_DATA netPTZCtrlData;
	UINT32 retVal;
	UINT32 position=1, cruNum=1, ListNum=1;
	

    static unsigned char byPresetNo = 0;
    static unsigned char byRemainTime = 0;
    static unsigned char bySpeed = 0;
    
	memcpy((char *)&netPTZCtrlData, recvbuff, sizeof(netPTZCtrlData));
	netPTZCtrlData.channel = ntohl(netPTZCtrlData.channel);
	netPTZCtrlData.command = ntohl(netPTZCtrlData.command);
	netPTZCtrlData.presetNo = ntohl(netPTZCtrlData.presetNo);

    //DMS_NET_PTZ_CONTROL stPtzControl;
	//memset(&stPtzControl, 0, sizeof(stPtzControl));
	//stPtzControl.dwSize = sizeof(stPtzControl);
	//stPtzControl.nChannel = 0;


    //DMS_NET_PTZ_CONTROL  ptz_control;
    DMS_NET_CRUISE_POINT cruisePoint;
    GK_NET_PRESET_INFO   gkPresetCfg;
    GK_NET_CRUISE_CFG    gkCruiseCfg;
    GK_NET_CRUISE_GROUP  cruise_info;

    
    int cnt     = 0;
    //int speed   = 0;
    //int step    = 0;
    int num     = 0;
    int grp_idx = 0;
    int pot_idx = 0;
    int ret = 0;
#if 1
	retVal = verifyNetClientOperation(&(netPTZCtrlData.header), pClientSockAddr, REMOTEPTZCONTROL);
	if(retVal == NETRET_QUALIFIED)
	{
		cruNum  = (netPTZCtrlData.presetNo>>24) & 0xff;
		ListNum = (netPTZCtrlData.presetNo>>16) & 0xff;
		position = netPTZCtrlData.presetNo & 0xffff;
		/* control PTZ */
		HPRINT_INFO("PTZ control: channel:%u, cmd=%d, cruNum=%d, ListNum=%d, position=%d, speed=%x\n",
		    netPTZCtrlData.channel, netPTZCtrlData.command, cruNum, ListNum, position, netPTZCtrlData.speed);

		if(SET_PRESET == netPTZCtrlData.command)
		{
			HPRINT_INFO("set preset\n");	
			//stPtzControl.dwCommand = DMS_PTZ_CMD_PRESET;
			//stPtzControl.dwParam = position;

            //num = stPtzControl.dwParam;
            num = position;
            if (num > PTZ_MAX_PRESET)
                num = PTZ_MAX_PRESET - 1;
            else if (num > 0)
                num -= 1;
            else
                num = 0;
            get_param(PTZ_PRESET_PARAM_ID, &gkPresetCfg);
            gkPresetCfg.nPresetNum++;
            if (gkPresetCfg.nPresetNum >= NETCAM_PTZ_MAX_PRESET_NUM)
            {
                gkPresetCfg.nPresetNum = NETCAM_PTZ_MAX_PRESET_NUM;
            }
            gkPresetCfg.no[gkPresetCfg.nPresetNum-1] = num;
            set_param(PTZ_PRESET_PARAM_ID, &gkPresetCfg);
            PresetCruiseCfgSave();
            if ((ret = netcam_ptz_set_preset(num, NULL)))
            {
                PRINT_ERR("call  DMS_PTZ_CMD_PRESET error!\n");
                return -1;
            }

		}
		else if(CLE_PRESET == netPTZCtrlData.command)
		{
            HPRINT_INFO("cle preset\n");	
			//stPtzControl.dwCommand = DMS_PTZ_CMD_PRESET_CLS;	
			//stPtzControl.dwParam = position;

            //num = ptz_control.dwParam;
            num = position;
            if (num > PTZ_MAX_PRESET)
                num = PTZ_MAX_PRESET - 1;
            else if (num > 0)
                num -= 1;
            else
                num = 0;
            get_param(PTZ_PRESET_PARAM_ID, &gkPresetCfg);
            gkPresetCfg.nPresetNum--;
            if (gkPresetCfg.nPresetNum <= 0)
            {
                gkPresetCfg.nPresetNum = 0;
            }
            set_param(PTZ_PRESET_PARAM_ID, &gkPresetCfg);
            PresetCruiseCfgSave();
            if ((ret = netcam_ptz_clr_preset(num)))
            {
                PRINT_ERR("call  DMS_PTZ_CMD_PRESET_CLS error!\n");
                return -1;
            }

		}
		else if(GOTO_PRESET == netPTZCtrlData.command)
		{
            HPRINT_INFO("call preset\n");
			//stPtzControl.dwCommand = DMS_PTZ_CMD_CALL;	
			//stPtzControl.dwParam = position;

            //num = ptz_control.dwParam;
            num = position;
            if (num > PTZ_MAX_PRESET)
                num = PTZ_MAX_PRESET - 1;
            else if (num > 0)
                num -= 1;
            else
                num = 0;
            
            cruise_info.byPointNum    = 1;
            cruise_info.byCruiseIndex = 0;
            cruise_info.struCruisePoint[0].byPointIndex = 0;
            cruise_info.struCruisePoint[0].byPresetNo   = num;
            cruise_info.struCruisePoint[0].byRemainTime = 0;
            cruise_info.struCruisePoint[0].bySpeed      = -1;
            if ((ret = netcam_ptz_stop()))
            {
                PRINT_ERR("call  netcam_ptz_stop error!\n");
                return -1;
            }
            if ((ret = netcam_ptz_preset_cruise(&cruise_info)))
            {
                PRINT_ERR("call  DMS_PTZ_CMD_AUTO_STRAT error!\n");
                return -1;
            }
            
		}
		else if(CLE_PRE_SEQ == netPTZCtrlData.command)
		{
			HPRINT_INFO("%s(%d) dell pos\n",__func__,__LINE__);	
            #if 0
			stPtzControl.dwCommand = DMS_PTZ_CMD_DEL_PRE_CRU;
			stPtzControl.dwParam = cruNum;			//巡航路线
			stPtzControl.byRes[1] =  position+1;	//预置点编号
            #endif
		}
        #if 0
		else if(DEL_CRUISE == netPTZCtrlData.command)
		{
			HPRINT_INFO("%s(%d) dell tour\n",__func__,__LINE__);		
            #if 0
			stPtzControl.dwCommand = DMS_PTZ_CMD_DEL_POS_CRU;

	        DMS_NET_CRUISE_INFO stCruiseInfo;
	        memset(&stCruiseInfo, 0, sizeof(DMS_NET_CRUISE_INFO));
	        stCruiseInfo.byCruiseIndex = cruNum;
	      	dms_sysnetapi_ioctrl(g_hik_handle, DMS_NET_SET_CRUISE_INFO, 0, &stCruiseInfo, sizeof(DMS_NET_CRUISE_INFO));
            #endif
        }
        #endif
		else if(FILL_PRE_SEQ == netPTZCtrlData.command) /* 将预置点加入巡航序列 */
		{
            HPRINT_INFO("cruise_index:%d, pos_index_in_cruise:%d, preset_index:%d\n", cruNum, ListNum, position);
            byPresetNo = position;
        }
		else if(SET_SEQ_DWELL == netPTZCtrlData.command)
		{
            HPRINT_INFO("cruise_index:%d, pos_index_in_cruise:%d, remain_time:%d\n", cruNum, ListNum, position);
            byRemainTime = position;
		}
		else if(SET_SEQ_SPEED == netPTZCtrlData.command)
		{
            HPRINT_INFO("cruise_index:%d, pos_index_in_cruise:%d, speed:%d\n", cruNum, ListNum, position);
            bySpeed = position;
		}
        else if(BEG_ADD_PRESET_TO_CRUISE == netPTZCtrlData.command)
		{
            HPRINT_INFO("begin add preset to cruise\n");
		}
		else if(END_ADD_PRESET_TO_CRUISE == netPTZCtrlData.command)
		{
            HPRINT_INFO("end add preset to cruise\n");
            HPRINT_INFO("cruise_index:%d, pos_index_in_cruise:%d, preset_index:%d, remain_time:%d, speed:%d\n", 
                cruNum, ListNum, byPresetNo, byRemainTime, bySpeed);

            grp_idx = cruNum - 1;		
            if (grp_idx >= PTZ_MAX_CRUISE_GROUP_NUM)
            {
                PRINT_ERR("index:%d > PTZ_MAX_CRUISE_GROUP_NUM!\n", grp_idx);
                return -1;
            }

            cruisePoint.byPointIndex = ListNum - 1;
            cruisePoint.byPresetNo = byPresetNo - 1;
            cruisePoint.byRemainTime = byRemainTime;
            //cruisePoint.bySpeed = bySpeed;
            cruisePoint.bySpeed = 30;
            
            get_param(PTZ_CRUISE_PARAM_ID, &gkCruiseCfg);
        
            if(gkCruiseCfg.struCruise[grp_idx].byPointNum < PTZ_MAX_CRUISE_POINT_NUM)
            {
                gkCruiseCfg.struCruise[grp_idx].byPointNum++;
            }
            else
            {
                gkCruiseCfg.struCruise[grp_idx].byPointNum = PTZ_MAX_CRUISE_POINT_NUM;
                HPRINT_ERR("total preset point num:PTZ_MAX_CRUISE_POINT_NUM ,don't add preset point!\n");
                return -1;
            }
        
            /* 巡航组中的下标,如果值大于PTZ_MAX_CRUISE_POINT_NUM 表示添加到末尾 */
            if (cruisePoint.byPointIndex >= gkCruiseCfg.struCruise[grp_idx].byPointNum)
            {
                cruisePoint.byPointIndex = gkCruiseCfg.struCruise[grp_idx].byPointNum - 1;
            }
            pot_idx = cruisePoint.byPointIndex;
            for (cnt = gkCruiseCfg.struCruise[grp_idx].byPointNum -1; cnt > pot_idx; cnt--)
            {
                gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt+1].byPointIndex =
                                            gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt].byPointIndex;
        
                gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt+1].byPresetNo   =
                                            gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt].byPresetNo;
        
                gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt+1].byRemainTime =
                                            gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt].byRemainTime;
        
                gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt+1].bySpeed      =
                                            gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt].bySpeed;
            }
        
            gkCruiseCfg.nChannel =0;
            gkCruiseCfg.struCruise[grp_idx].byCruiseIndex = grp_idx;
            gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cruisePoint.byPointIndex].byPointIndex =
                                                                                    cruisePoint.byPointIndex;
            gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cruisePoint.byPointIndex].byPresetNo   =
                                                                                    cruisePoint.byPresetNo;
            gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cruisePoint.byPointIndex].byRemainTime =
                                                                                    cruisePoint.byRemainTime;
            gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cruisePoint.byPointIndex].bySpeed      =
                                                                                    cruisePoint.bySpeed;
        
            set_param(PTZ_CRUISE_PARAM_ID, &gkCruiseCfg);
            PresetCruiseCfgSave();
		}
		else if(RUN_CRUISE == netPTZCtrlData.command)
		{
            HPRINT_INFO("start cruise, cruNum:%d\n", position);

            grp_idx = position - 1;
            if (grp_idx >= PTZ_MAX_CRUISE_GROUP_NUM)
            {
                HPRINT_ERR("START_CRU grp_idx:%d > PTZ_MAX_CRUISE_GROUP_NUM:%d\n", grp_idx, PTZ_MAX_CRUISE_GROUP_NUM);
                return -1;
            }
            get_param(PTZ_CRUISE_PARAM_ID, &gkCruiseCfg);
            for (cnt = 0; cnt < PTZ_MAX_CRUISE_POINT_NUM; cnt++)
            {
                if (gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt].byPresetNo > 0)
                {
                    gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt].byPresetNo--;
                }
            }
            HPRINT_INFO("byPointNum:%u, index:0-byPointIndex:%u, byPresetNo:%u, byRemainTime:%u, bySpeed:%u\n", 
                gkCruiseCfg.struCruise[0].byPointNum, 
                gkCruiseCfg.struCruise[0].struCruisePoint[0].byPointIndex,
                gkCruiseCfg.struCruise[0].struCruisePoint[0].byPresetNo,
                gkCruiseCfg.struCruise[0].struCruisePoint[0].byRemainTime,
                gkCruiseCfg.struCruise[0].struCruisePoint[0].bySpeed);
            ret = netcam_ptz_preset_cruise(&gkCruiseCfg.struCruise[grp_idx]);
            if (ret < 0)
            {
                HPRINT_ERR("Call netcam_ptz_preset_cruise error!\n");
                return -1;
            }
		}
		else if(STOP_CRUISE == netPTZCtrlData.command)
		{
            HPRINT_INFO("stop cruise\n");
            if ((ret = netcam_ptz_ciruise_stop()))
            {
                HPRINT_ERR("call  netcam_ptz_stop error!\n");
                return -1;
            }
		}
        #if 0
		else
		{
			HPRINT_INFO("%s(%d) stop\n",__func__,__LINE__);
			if(1 == nflag) //巡航的停止命令跟向左的停止命令一样，无法按正常逻辑处理
			{
				stPtzControl.dwCommand = DMS_PTZ_CMD_STOP_CRU;
				dms_sysnetapi_ioctrl(g_hik_handle, DMS_NET_CMD_PTZ_CONTROL,0, &stPtzControl, sizeof(stPtzControl));
				nflag = 0;
			}
			stPtzControl.dwCommand = DMS_PTZ_CMD_STOP;
		}

		if(stPtzControl.dwCommand != DMS_PTZ_CMD_DEL_POS_CRU || stPtzControl.dwCommand != 0)
			dms_sysnetapi_ioctrl(g_hik_handle, DMS_NET_CMD_PTZ_CONTROL,0, &stPtzControl, sizeof(stPtzControl));

		if(stPtzControl.dwCommand == DMS_PTZ_CMD_DEL_PRE_CRU)//要重新获取一次(目前只能)
		{
			DMS_NET_CRUISE_CFG tmpCruise;
			dms_sysnetapi_ioctrl(g_hik_handle, DMS_NET_GET_CRUISE_CFG, 0, &tmpCruise, sizeof(tmpCruise));
		}
        #endif
	}

#endif

	return sendNetRetval(connfd, retVal);
}


int netClientPTZControlWithSpeed(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	int retVal;
	NET_PTZ_CTRL_DATA netPTZCtrlData;
	//QMAPI_NET_PTZ_CONTROL stPtzControl;

	memcpy((char *)&netPTZCtrlData, recvbuff, sizeof(netPTZCtrlData));
	netPTZCtrlData.channel = ntohl(netPTZCtrlData.channel);
	netPTZCtrlData.command = ntohl(netPTZCtrlData.command);
	netPTZCtrlData.speed = ntohl(netPTZCtrlData.speed);

	//stPtzControl.dwSize = sizeof(QMAPI_NET_PTZ_CONTROL);
	//stPtzControl.nChannel = 0;


	retVal = verifyNetClientOperation(&(netPTZCtrlData.header), pClientSockAddr, REMOTEPTZCONTROL);
	if(retVal == NETRET_QUALIFIED)
	{
        int step = 20;

        int speed = netPTZCtrlData.speed * 64 / 40;
        HPRINT_INFO("ptz cmd:%u, speed:0x%x, gk_ptz_speed:%d\n", 
            netPTZCtrlData.command, netPTZCtrlData.speed, speed);

        int ret = 0;
        if(TILT_UP == netPTZCtrlData.command)
            ret = netcam_ptz_up(step, speed);
        else if(TILT_DOWN == netPTZCtrlData.command)
            ret = netcam_ptz_down(step, speed);
        else if(PAN_LEFT == netPTZCtrlData.command)
            ret = netcam_ptz_left(step, speed);
        else if(PAN_RIGHT == netPTZCtrlData.command)
            ret = netcam_ptz_right(step, speed);
        else if(UP_LEFT == netPTZCtrlData.command)
            ret = netcam_ptz_left_up(step, speed);
        else if(UP_RIGHT == netPTZCtrlData.command)
            ret = netcam_ptz_right_up(step, speed); 
        else if(DOWN_LEFT == netPTZCtrlData.command)
            ret = netcam_ptz_left_down(step, speed);
        else if(DOWN_RIGHT == netPTZCtrlData.command)
            ret = netcam_ptz_right_down(step, speed);
        else if(ZOOM_IN == netPTZCtrlData.command)
            ;//stPtzControl.dwCommand = QMAPI_PTZ_STA_ZOOM_ADD;
        else if(ZOOM_OUT == netPTZCtrlData.command)
            ;//stPtzControl.dwCommand = QMAPI_PTZ_STA_ZOOM_SUB;
        else if(FOCUS_IN == netPTZCtrlData.command)
            ;//stPtzControl.dwCommand = QMAPI_PTZ_STA_FOCUS_SUB;
        else if(FOCUS_OUT == netPTZCtrlData.command)
            ;//stPtzControl.dwCommand = QMAPI_PTZ_STA_FOCUS_ADD;
        else if(IRIS_ENLARGE == netPTZCtrlData.command)
            ;//stPtzControl.dwCommand = QMAPI_PTZ_STA_IRIS_SUB;
        else if(IRIS_SHRINK == netPTZCtrlData.command)
            ;//stPtzControl.dwCommand = QMAPI_PTZ_STA_IRIS_ADD;
        else if(AUTO_PAN == netPTZCtrlData.command)
            ret = netcam_ptz_hor_ver_cruise(speed);
            //stPtzControl.dwCommand = QMAPI_PTZ_STA_AUTO_STRAT; 
        else if((~AUTO_PAN) == netPTZCtrlData.command)
            //stPtzControl.dwCommand = QMAPI_PTZ_STA_AUTO_STOP;
            ret = netcam_ptz_stop();
        else if(LIGHT_PWRON == netPTZCtrlData.command)
            ;//stPtzControl.dwCommand = QMAPI_PTZ_STA_LIGHT_OPEN;
        else if((~LIGHT_PWRON) == netPTZCtrlData.command)
            ;//stPtzControl.dwCommand = QMAPI_PTZ_STA_LIGHT_CLOSE;   
        else if(WIPER_PWRON == netPTZCtrlData.command)
            ;//stPtzControl.dwCommand = QMAPI_PTZ_STA_BRUSH_START;
        else if((~WIPER_PWRON) == netPTZCtrlData.command)
            ;//stPtzControl.dwCommand = QMAPI_PTZ_STA_BRUSH_STOP;
        else
            //stPtzControl.dwCommand = QMAPI_PTZ_STA_STOP;  //停止
            ret = netcam_ptz_stop();
        //stPtzControl.dwParam = (netPTZCtrlData.speed+1) * 8;  //[1,7]->[1,64]

		//pthreadMutexLock(&globalMSem, WAIT_FOREVER);	//其实不需要进行加锁操作
		//QMapi_sys_ioctrl(g_hik_handle, QMAPI_NET_STA_PTZ_CONTROL,0, &stPtzControl, sizeof(stPtzControl));
		//pthreadMutexUnlock(&globalMSem);
		//usleep(500*1000);
		//stPtzControl.dwCommand = QMAPI_PTZ_STA_STOP;	//停止
		//QMapi_sys_ioctrl(g_hik_handle, QMAPI_NET_STA_PTZ_CONTROL, netPTZCtrlData.channel, &stPtzControl, sizeof(stPtzControl));
	}

	return sendNetRetval(connfd, retVal);
}

static int convertResoloution(int hk_resoloution)
{
#if 0
	int resolution = hk_resoloution;
	if (0x1b == resolution){		//1080p
		resolution = DMS_VIDEO_FORMAT_1080P;
	}
	else if(0x43 == resolution){
		resolution = DMS_VIDEO_FORMAT_5M;
	}
	else if(0x46 == resolution){
		resolution = DMS_VIDEO_FORMAT_2560x1440;
	}
	else if(0x1e == resolution){
		resolution = DMS_VIDEO_FORMAT_2048x1520;
	}
	else if (0x14 == resolution){	//1280*960p
		resolution = DMS_VIDEO_FORMAT_960P;		
	}
	else if (0x13 == resolution){	//720p
		resolution = DMS_VIDEO_FORMAT_720P;
	}
	else if (0x12 == resolution){	//svag
		resolution = DMS_VIDEO_FORMAT_SVGA;
	}
	else if (0x03 == resolution){	//4cif D1		
		resolution = DMS_VIDEO_FORMAT_D1;
	}
	else if (0x10 == resolution){	//vga
		resolution = DMS_VIDEO_FORMAT_VGA;
	}
	else if (0x56 == resolution){	//360P
		resolution = DMS_VIDEO_FORMAT_640x360;
	}
	else if (0x06 == resolution){	//Qvga
		resolution = DMS_VIDEO_FORMAT_QVGA;
	}		
	else if (0x01 == resolution){	//cif	
		resolution = DMS_VIDEO_FORMAT_CIF;
	}
	else if (0x11 == resolution){	//uxga
		resolution = DMS_VIDEO_FORMAT_UXGA;
	}
	else{	//720P
		resolution = DMS_VIDEO_FORMAT_720P;
	}
	return resolution;
#else
    return 0;
#endif
}

static int convertResoloution2(int hk_resoloution, int *width, int *height)
{
	int resolution = hk_resoloution;
	if (RESOLOUTION_SIZE_1080p == resolution){		//1080p
		*width = 1920;
		*height = 1080;
	}
	else if(RESOLOUTION_SIZE_5M_1 == resolution)
	{
		*width = 2592;
		*height = 1944;
	}
	else if(RESOLOUTION_SIZE_4M == resolution)
	{
		*width = 2560;
		*height = 1440;
	}
    else if(RESOLOUTION_SIZE_4M_1 == resolution)
	{
		*width = 2688;
		*height = 1520;
	}
	else if(RESOLOUTION_SIZE_3M_1 == resolution)
	{
		*width = 2048;
		*height = 1520;
	}
    else if(RESOLOUTION_SIZE_3M_2 == resolution)
	{
		*width = 2304;
		*height = 1296;
	}
	else if (RESOLOUTION_SIZE_1_3M == resolution){	//1280*960p
		*width = 1280;
		*height = 960;
	}
	else if (RESOLOUTION_SIZE_720p == resolution){	//720p
		*width = 1280;
		*height = 720;
	}
	else if (RESOLOUTION_SIZE_SVGA == resolution){	//svga
		*width = 800;
		*height = 600;
	}
    else if (RESOLOUTION_SIZE_D1 == resolution){	//4cif/D1		
		*width = 720;
		*height = 576;
	}
	else if (RESOLOUTION_SIZE_4CIF == resolution){	//4cif/D1		
		*width = 704;
		*height = 576;
	}
	else if (RESOLOUTION_SIZE_VGA == resolution){	//vga
		*width = 640;
		*height = 480;
	}
	else if(RESOLOUTION_SIZE_HVGAW == resolution){	//360P
		*width = 640;
		*height = 360;
	}
	else if (RESOLOUTION_SIZE_QVGA == resolution){  //Qvga
		*width = 320;
		*height = 240;
	}
	else if (RESOLOUTION_SIZE_CIF == resolution){	//cif	
        *width = 352;
		*height = 288;
	}
	else if (RESOLOUTION_SIZE_UXGA == resolution){	//uxga
		*width = 1600;
		*height = 1200;
	}
	else{	//1080P
		*width = 1280;
		*height = 720;
	}
	return 0;
}

static int convertResoloutionToHik(int ANI_resoloution)
{
#if 0
	int resoloution = ANI_resoloution;
	if(resoloution == QMAPI_VIDEO_FORMAT_CIF){
		resoloution = 0x01;
	}
	else if(resoloution == QMAPI_VIDEO_FORMAT_D1){
		resoloution = 0x03;
	}
	else if(resoloution == QMAPI_VIDEO_FORMAT_QCIF){
		resoloution = 0x02;
	}
	else if(resoloution == QMAPI_VIDEO_FORMAT_VGA){
		resoloution = 0x10;
	}
	else if(resoloution == QMAPI_VIDEO_FORMAT_QVGA){
		resoloution = 0x06;
	}
	else if(resoloution == QMAPI_VIDEO_FORMAT_720P){
		resoloution = 0x13;
	}
	else if(resoloution == QMAPI_VIDEO_FORMAT_130H){
		resoloution = 0x23;
	}
	else if(resoloution == QMAPI_VIDEO_FORMAT_300H){
		resoloution = 0x1E;
	}
	else if(resoloution == QMAPI_VIDEO_FORMAT_QQVGA){
		resoloution = 0x07;
	}
	else if(resoloution == QMAPI_VIDEO_FORMAT_UXGA){
		resoloution = 0x11;
	}
	else if(resoloution == QMAPI_VIDEO_FORMAT_SVGA){
		resoloution = 0x12;
	}
	else if(resoloution == QMAPI_VIDEO_FORMAT_1080P){
		resoloution = 0x1b;
	}	
	else if(resoloution == QMAPI_VIDEO_FORMAT_960P){
		resoloution = 0x14;
	}
	else if(resoloution == QMAPI_VIDEO_FORMAT_640x360){
		resoloution = 0x56;
	}
//	else if(resoloution == DMS_VIDEO_FORMAT_5M){
//		resoloution = 0x43;
//	}
//	else if(resoloution == DMS_VIDEO_FORMAT_2560x1440){
//		resoloution = 0x46;
//	}
	else{
		resoloution = 0x13;
	}

		
	return resoloution;	
#else
    return 0x13;
#endif
	
}

static int convertResoloutionToHik2(int width, int height)
{
#if 0
	int resoloution = ANI_resoloution;
	if(resoloution == DMS_VIDEO_FORMAT_CIF){
		resoloution = 0x01;
	}
	else if(resoloution == DMS_VIDEO_FORMAT_D1){
		resoloution = 0x03;
	}
	else if(resoloution == DMS_VIDEO_FORMAT_QCIF){
		resoloution = 0x02;
	}
	else if(resoloution == DMS_VIDEO_FORMAT_VGA){
		resoloution = 0x10;
	}
	else if(resoloution == DMS_VIDEO_FORMAT_QVGA){
		resoloution = 0x06;
	}
	else if(resoloution == DMS_VIDEO_FORMAT_720P){
		resoloution = 0x13;
	}
	else if(resoloution == DMS_VIDEO_FORMAT_130H){
		resoloution = 0x23;
	}
	else if(resoloution == DMS_VIDEO_FORMAT_300H){
		resoloution = 0x1E;
	}
	else if(resoloution == DMS_VIDEO_FORMAT_QQVGA){
		resoloution = 0x07;
	}
	else if(resoloution == DMS_VIDEO_FORMAT_UXGA){
		resoloution = 0x11;
	}
	else if(resoloution == DMS_VIDEO_FORMAT_SVGA){
		resoloution = 0x12;
	}
	else if(resoloution == DMS_VIDEO_FORMAT_1080P){
		resoloution = 0x1b;
	}	
	else if(resoloution == DMS_VIDEO_FORMAT_960P){
		resoloution = 0x14;
	}
	else if(resoloution == DMS_VIDEO_FORMAT_640x360){
		resoloution = 0x56;
	}
	else if(resoloution == DMS_VIDEO_FORMAT_5M){
		resoloution = 0x43;
	}
	else if(resoloution == DMS_VIDEO_FORMAT_2560x1440){
		resoloution = 0x46;
	}
	else{
		resoloution = 0x13;
	}

	return resoloution;	
#else

    int resoloution = RESOLOUTION_SIZE_HVGAW;

    if ((width == 2592) && (height == 1944))
        resoloution = RESOLOUTION_SIZE_5M_1;
    else if ((width == 2688) && (height == 1520))
        resoloution = RESOLOUTION_SIZE_4M_1;
    else if ((width == 2560) && (height == 1440))
        resoloution = RESOLOUTION_SIZE_4M;
    else if ((width == 2048) && (height == 1536))
        resoloution = RESOLOUTION_SIZE_3M;
    else if ((width == 2048) && (height == 1520))
        resoloution = RESOLOUTION_SIZE_3M_1;
    else if ((width == 2304) && (height == 1296))
        resoloution = RESOLOUTION_SIZE_3M_2;
    else if ((width == 1600) && (height == 1200))
        resoloution = RESOLOUTION_SIZE_UXGA;
    else if ((width == 1920) && (height == 1080))
        resoloution = RESOLOUTION_SIZE_1080p;
    else if ((width == 1280) && (height == 960))
        resoloution = RESOLOUTION_SIZE_1_3M;
    else if ((width == 1280) && (height == 720))
        resoloution = RESOLOUTION_SIZE_720p;
    else if ((width == 640) && (height == 360))
        resoloution = RESOLOUTION_SIZE_HVGAW;
    else if ((width == 800) && (height == 600))
        resoloution = RESOLOUTION_SIZE_SVGA;
    else if ((width == 704) && (height == 576))
        resoloution = RESOLOUTION_SIZE_4CIF;
    else if ((width == 720) && (height == 576)) //D1
        resoloution = RESOLOUTION_SIZE_D1; //0x01;
    else if ((width == 720) && (height == 480)) //D1
        resoloution = RESOLOUTION_SIZE_BVGA; //0x01;
    else if ((width == 640) && (height == 480)) //VGA
        resoloution = RESOLOUTION_SIZE_VGA;
    else if ((width == 352) && (height == 288)) //CIF
        resoloution = RESOLOUTION_SIZE_CIF;
    else if ((width == 320) && (height == 240))
        resoloution = RESOLOUTION_SIZE_QVGA;

    return resoloution;	
#endif
}


static int converFrameRate(int hk_frameRate)
{
	int frameRate = hk_frameRate;
	if (0x11 == frameRate){
		frameRate = 25;
	}
	else if (0x10 == frameRate){
		frameRate = 22;
	}
	else if (0x0d == frameRate){
		frameRate = 20;
	}
	else if (0x0f == frameRate){
		frameRate = 18;
	}
	else if (0x0c == frameRate){
		frameRate = 16;
	}
	else if (0x0e == frameRate){
		frameRate = 15;
	}
	else if (0x0b == frameRate){
		frameRate = 12;
	}
	else if (0x0a == frameRate){
		frameRate = 10;
	}
	else if (0x09 == frameRate){
		frameRate = 8;
	}
	else if (0x08 == frameRate){
		frameRate = 6;
	}
	else if (0x07 == frameRate){
		frameRate = 4;
	}
	else if (0x06 == frameRate){
		frameRate = 2;
	}
	else if (0x05 == frameRate){
		frameRate = 1;
	}
	else{
		frameRate = 25;
	}
	return frameRate;
}

static int converFrameRateToHik(int ANI_frameRate)
{
	int frameRate = ANI_frameRate;
	if (25 == frameRate){
		frameRate = 0x11;
	}
	else if (22 == frameRate){
		frameRate = 0x10;
	}
	else if (20 == frameRate){
		frameRate = 0x0d;
	}
	else if (18 == frameRate){
		frameRate = 0x0f;
	}
	else if (16 == frameRate){
		frameRate = 0x0c;
	}
	else if (15 == frameRate){
		frameRate = 0x0e;
	}
	else if (12 == frameRate){
		frameRate = 0x0b;
	}
	else if (10 == frameRate){
		frameRate = 0x0a;
	}
	else if (8 == frameRate){
		frameRate = 0x09;
	}
	else if (6 == frameRate){
		frameRate = 0x08;
	}
	else if (4 == frameRate){
		frameRate = 0x07;
	}
	else if (2 == frameRate){
		frameRate = 0x06;
	}
	else if (1 == frameRate){
		frameRate = 0x05;
	}
	else{
		frameRate = 0x11;
	}
	return frameRate;
}

int convertStreamType(int streamType)
{
	if (1 == streamType){
		return 0;
	}
	else if (3 == streamType){
		return 1;
	}
	return 0;
}

/**********************************************************************
 * Function:  netClientSetCompressCfg
 * Description:set compress config param
 * Input: 
 *		 connfd - socket connfd 
 *       recvbuff - recevie data buff
 *       pClientSockAddr - client socket address(not need)
 * Output:  N/A
 * Return:  OK if successful, otherwise return error status number
 **********************************************************************/
int netClientSetCompressCfgV30(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	struct{
		NETCMD_HEADER header;
		UINT32 channel;
		NETPARAM_COMPRESS_CFG_V30 compressCfg;
	}netCmdCompressCfg;

	UINT32 retVal;

	bzero(&netCmdCompressCfg, sizeof(netCmdCompressCfg));
	memcpy((char *)&netCmdCompressCfg, recvbuff, sizeof(netCmdCompressCfg));

	retVal = verifyNetClientOperation(&(netCmdCompressCfg.header), pClientSockAddr, REMOTESETPARAMETER);
	if(retVal == NETRET_QUALIFIED)
	{
		if(ntohl(netCmdCompressCfg.compressCfg.length) != sizeof(NETPARAM_COMPRESS_CFG_V30))
		{	/* check length */
		    HPRINT_ERR("NETRET_ERROR_DATA\n");
            return sendNetRetval(connfd, NETRET_ERROR_DATA);
		}
		else
		{
#if 0
			HPRINT_INFO("normHighCompPara: streamType:%d resolution:%d bitrateType:%d quality:%d,",
						netCmdCompressCfg.compressCfg.normHighCompPara.streamType,
						netCmdCompressCfg.compressCfg.normHighCompPara.resolution,
						netCmdCompressCfg.compressCfg.normHighCompPara.bitrateType,
						netCmdCompressCfg.compressCfg.normHighCompPara.quality);
			HPRINT_INFO("maxBitRate:%08X maxBitRate:%d videoFrameRate:%08X intervalFrameI:%d videoEncType:%d audioEncType:%d\n",
						ntohl(netCmdCompressCfg.compressCfg.normHighCompPara.maxBitRate),
						netCmdCompressCfg.compressCfg.normHighCompPara.maxBitRate,
						ntohl(netCmdCompressCfg.compressCfg.normHighCompPara.videoFrameRate),
						ntohs(netCmdCompressCfg.compressCfg.normHighCompPara.intervalFrameI),
					    netCmdCompressCfg.compressCfg.normHighCompPara.videoEncType,
						netCmdCompressCfg.compressCfg.normHighCompPara.audioEncType);
			HPRINT_INFO("netCompPara: streamType:%d resolution:%d bitrateType:%d quality:%d,",
						netCmdCompressCfg.compressCfg.netCompPara.streamType,
						netCmdCompressCfg.compressCfg.netCompPara.resolution,
						netCmdCompressCfg.compressCfg.netCompPara.bitrateType,
						netCmdCompressCfg.compressCfg.netCompPara.quality);
			HPRINT_INFO("maxBitRate:%08X videoFrameRate:%08X intervalFrameI:%d videoEncType:%d audioEncType:%d\n",
						ntohl(netCmdCompressCfg.compressCfg.netCompPara.maxBitRate),
						ntohl(netCmdCompressCfg.compressCfg.netCompPara.videoFrameRate),
						netCmdCompressCfg.compressCfg.netCompPara.intervalFrameI,
						netCmdCompressCfg.compressCfg.netCompPara.videoEncType,
						netCmdCompressCfg.compressCfg.netCompPara.audioEncType);
#endif	
			int width, height, frameRate, resolution, rateType, bitRate, imageQuality, encodeAudio, compressionType, ret;
            // set main stream params.
			bitRate = ((ntohl(netCmdCompressCfg.compressCfg.normHighCompPara.maxBitRate) & 0x00FFFFFF)/1024);
            if (bitRate) //FIXME(heyong): bitrate = 0, means set another stream.
            {
    			frameRate = converFrameRate(ntohl(netCmdCompressCfg.compressCfg.normHighCompPara.videoFrameRate));
    			resolution = convertResoloution(netCmdCompressCfg.compressCfg.normHighCompPara.resolution);
    			convertResoloution2(netCmdCompressCfg.compressCfg.normHighCompPara.resolution, 
                    &width, &height);
                if(1 == netCmdCompressCfg.compressCfg.normHighCompPara.bitrateType)
                    rateType = 0; //?¨
                else
                    rateType = 1; //±?
    			if (netCmdCompressCfg.compressCfg.normHighCompPara.quality > 3)//default >4
    				netCmdCompressCfg.compressCfg.normHighCompPara.quality = 3;//4;
    			imageQuality = netCmdCompressCfg.compressCfg.normHighCompPara.quality;
    			encodeAudio = convertStreamType(netCmdCompressCfg.compressCfg.normHighCompPara.streamType);
    			if(netCmdCompressCfg.compressCfg.normHighCompPara.videoEncType == 0x0a)
    				compressionType = 3;//DMS_PT_H265;
    			else
    				compressionType = 1;//DMS_PT_H264;
            
                // set netcam
                ST_GK_ENC_STREAM_H264_ATTR stH264Config;
                memset(&stH264Config, 0, sizeof(ST_GK_ENC_STREAM_H264_ATTR));
                ret = netcam_video_get(0, 0, &stH264Config);
                if (ret != 0)
                    return sendNetRetval(connfd, NETRET_ERROR_DATA);

                stH264Config.width = width;
                stH264Config.height = height;
                stH264Config.fps = frameRate;
                stH264Config.rc_mode = rateType;
                stH264Config.bps = bitRate;
                stH264Config.quality = imageQuality;
				stH264Config.enctype = compressionType;
                
                HPRINT_INFO("ipc-main: frameRate:%d, reso:%dx%d, bitRate:(%d->%dkbps), quality:%d\n", 
                        stH264Config.fps, stH264Config.width,stH264Config.height,
                        stH264Config.rc_mode, stH264Config.bps, stH264Config.quality);
                
                ret = netcam_video_set(0, 0, &stH264Config);
                if (ret != 0)
                    return sendNetRetval(connfd, NETRET_ERROR_DATA);
            }

            // set second stream params.
            bitRate = ((ntohl(netCmdCompressCfg.compressCfg.netCompPara.maxBitRate) & 0x00FFFFFF)/1024);
            if (bitRate) //FIXME(heyong): bitrate = 0, means set another stream.
            {
				frameRate = converFrameRate(ntohl(netCmdCompressCfg.compressCfg.netCompPara.videoFrameRate));
                resolution = convertResoloution(netCmdCompressCfg.compressCfg.netCompPara.resolution);
				convertResoloution2(netCmdCompressCfg.compressCfg.netCompPara.resolution, 
                    &width, &height);
				if(1 == netCmdCompressCfg.compressCfg.netCompPara.bitrateType)
                    rateType = 0; //?¨
                else
                    rateType = 1; //±?

				if (netCmdCompressCfg.compressCfg.normHighCompPara.quality > 3)
					netCmdCompressCfg.compressCfg.normHighCompPara.quality = 3;
				imageQuality = netCmdCompressCfg.compressCfg.netCompPara.quality;
				encodeAudio = convertStreamType(netCmdCompressCfg.compressCfg.netCompPara.streamType);
				if(netCmdCompressCfg.compressCfg.netCompPara.videoEncType == 0x0a)
    				compressionType = 3;//DMS_PT_H265;
    			else
    				compressionType = 1;//DMS_PT_H264;

                // set netcam
                ST_GK_ENC_STREAM_H264_ATTR stH264Config;
                memset(&stH264Config, 0, sizeof(ST_GK_ENC_STREAM_H264_ATTR));
                ret = netcam_video_get(0, 1, &stH264Config);
                if (ret != 0)
	                return sendNetRetval(connfd, NETRET_ERROR_DATA);

                stH264Config.width = width;
                stH264Config.height = height;
                stH264Config.fps = frameRate;
                stH264Config.rc_mode = rateType;
                stH264Config.bps = bitRate;
                stH264Config.quality = imageQuality;
				stH264Config.enctype = compressionType;
                
                HPRINT_INFO("ipc-sec: frameRate:%d, reso:%dx%d, bitRate:(%d->%dkbps), quality:%d\n", 
                    stH264Config.fps, stH264Config.width,stH264Config.height,
                    stH264Config.rc_mode, stH264Config.bps, stH264Config.quality);
            
                ret = netcam_video_set(0, 1, &stH264Config);
                if (ret != 0)
	                return sendNetRetval(connfd, NETRET_ERROR_DATA);
            }

            // save to cfg.
            netcam_timer_add_task(netcam_video_cfg_save, NETCAM_TIMER_ONE_SEC, SDK_FALSE, SDK_TRUE);
            netcam_osd_update_clock();
            netcam_osd_update_title();
            netcam_osd_update_id();
		}
	}
	return sendNetRetval(connfd, NETRET_QUALIFIED);
}


/***********************************************************************
 * Function: netGetIpCompressCfgV30
 * Description:net get Ip signle compress config V30
 * Input: 
	chan---net Ip chan
 	pCompressCfg---a pointer point to NETPARAM_COMPRESS_CFG_V30
 * Output:N/A
 * Return: OK 
 **********************************************************************/
int netGetIpCompressCfgV30(int chan,NETPARAM_COMPRESS_CFG_V30 *pCompressCfg)
{
	UINT32 retVal = 0;

#if 0
	DMS_NET_CHANNEL_PIC_INFO ChannelCfg;
	memset(&ChannelCfg,0,sizeof(ChannelCfg));
	dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_GET_PICCFG,0,&ChannelCfg,sizeof(ChannelCfg));
	
	pCompressCfg->length = htonl(sizeof(NETPARAM_COMPRESS_CFG_V30));
	if(1 == ChannelCfg.stRecordPara.wEncodeAudio)
		pCompressCfg->normHighCompPara.streamType= 0x03;
	else
		pCompressCfg->normHighCompPara.streamType = 0x01;
	pCompressCfg->normHighCompPara.resolution = convertResoloutionToHik(ChannelCfg.stRecordPara.dwStreamFormat);
	if(1 == ChannelCfg.stRecordPara.dwRateType)
		pCompressCfg->normHighCompPara.bitrateType = 0x00;
	else
		pCompressCfg->normHighCompPara.bitrateType = 0x01;
	pCompressCfg->normHighCompPara.quality = ChannelCfg.stRecordPara.dwImageQuality;
	pCompressCfg->normHighCompPara.maxBitRate = htonl((ChannelCfg.stRecordPara.dwBitRate/1000*1024) | 0x80000000);
	pCompressCfg->normHighCompPara.videoFrameRate = htonl(converFrameRateToHik(ChannelCfg.stRecordPara.dwFrameRate));
	pCompressCfg->normHighCompPara.intervalFrameI = htons(ChannelCfg.stRecordPara.dwMaxKeyInterval);
	pCompressCfg->normHighCompPara.BFrameNum = 0x02;
	pCompressCfg->normHighCompPara.EFrameNum = 0x00;

	if(ChannelCfg.stRecordPara.dwCompressionType == DMS_PT_H265)
		pCompressCfg->normHighCompPara.videoEncType = 0x0a;
	else
		pCompressCfg->normHighCompPara.videoEncType = 0x01;
	//pCompressCfg->normHighCompPara.videoEncType = ChannelCfg.stRecordPara.wEncodeVideo;
	pCompressCfg->normHighCompPara.audioEncType = ChannelCfg.stRecordPara.wEncodeAudio;
	pCompressCfg->normHighCompPara.res[0] = 0x02;
	pCompressCfg->normHighCompPara.res[1] = 0x00;
	pCompressCfg->normHighCompPara.res[2] = 0x00;
	pCompressCfg->normHighCompPara.res[3] = 0x04;
	pCompressCfg->normHighCompPara.res[4] = 0x32;
	pCompressCfg->normHighCompPara.res[5] = 0x02;

	if(1 == ChannelCfg.stNetPara.wEncodeAudio)
		pCompressCfg->netCompPara.streamType = 0x03;
	else
		pCompressCfg->netCompPara.streamType = 0x01;
	pCompressCfg->netCompPara.resolution = convertResoloutionToHik(ChannelCfg.stNetPara.dwStreamFormat);//0x03;
	if(1 == ChannelCfg.stNetPara.dwRateType)
		pCompressCfg->netCompPara.bitrateType = 0x00;
	else
		pCompressCfg->netCompPara.bitrateType = 0x01;
	pCompressCfg->netCompPara.quality = ChannelCfg.stNetPara.dwImageQuality;//0x02;
	if(ChannelCfg.stNetPara.dwBitRate > 2048000)
		ChannelCfg.stNetPara.dwBitRate = 2048;
	else if(ChannelCfg.stNetPara.dwBitRate < 32000)
		ChannelCfg.stNetPara.dwBitRate = 32;
	pCompressCfg->netCompPara.maxBitRate = htonl((ChannelCfg.stNetPara.dwBitRate/1000*1024) | 0x80000000);//htonl(0X0000000F);
	pCompressCfg->netCompPara.videoFrameRate = htonl(converFrameRateToHik(ChannelCfg.stNetPara.dwFrameRate));//htonl(0x0000000d);//
	pCompressCfg->netCompPara.intervalFrameI = htons(ChannelCfg.stNetPara.dwMaxKeyInterval);//htons(0x0019);//
	pCompressCfg->netCompPara.BFrameNum = 0x02;
	pCompressCfg->netCompPara.EFrameNum = 0x00;
	//pCompressCfg->netCompPara.videoEncType = ChannelCfg.stNetPara.wEncodeVideo;
	if(ChannelCfg.stNetPara.dwCompressionType == DMS_PT_H265)
		pCompressCfg->netCompPara.videoEncType = 0x0a;
	else
		pCompressCfg->netCompPara.videoEncType = 0x01;
	
	pCompressCfg->netCompPara.audioEncType = ChannelCfg.stNetPara.wEncodeAudio;//0x01;
#else

    ST_GK_ENC_STREAM_H264_ATTR stH264Config;
    memset(&stH264Config, 0, sizeof(ST_GK_ENC_STREAM_H264_ATTR));
    int ret = netcam_video_get(0, 0, &stH264Config);
    if (ret != 0)
    {
       PRINT_ERR("netcam_video_get failed.");
       return -1;
    }

    HPRINT_INFO("stream0 -- runAudioCfg.mode:%d, resolution: %d * %d, rc_mode:%d, bps:%d, fps:%d, gop:%d \n", 
        runAudioCfg.mode, stH264Config.width, stH264Config.height, stH264Config.rc_mode,
        stH264Config.bps, stH264Config.fps, stH264Config.gop);
    
	pCompressCfg->length = htonl(sizeof(NETPARAM_COMPRESS_CFG_V30));
	//if(1 == ChannelCfg.stRecordPara.wEncodeAudio)
	if (runAudioCfg.mode)
		pCompressCfg->normHighCompPara.streamType= 0x03;
	else
		pCompressCfg->normHighCompPara.streamType = 0x01;

	//pCompressCfg->normHighCompPara.resolution = convertResoloutionToHik(ChannelCfg.stRecordPara.dwStreamFormat);
	pCompressCfg->normHighCompPara.resolution = convertResoloutionToHik2(stH264Config.width, stH264Config.height);
	//if(1 == ChannelCfg.stRecordPara.dwRateType)
	if (1 == stH264Config.rc_mode)
		pCompressCfg->normHighCompPara.bitrateType = 0x00;
	else
		pCompressCfg->normHighCompPara.bitrateType = 0x01;
	pCompressCfg->normHighCompPara.quality = stH264Config.quality;
	pCompressCfg->normHighCompPara.maxBitRate = htonl((stH264Config.bps*1024) | 0x80000000);
	pCompressCfg->normHighCompPara.videoFrameRate = htonl(converFrameRateToHik(stH264Config.fps));
	pCompressCfg->normHighCompPara.intervalFrameI = htons(stH264Config.gop);
	pCompressCfg->normHighCompPara.BFrameNum = 0x02;
	pCompressCfg->normHighCompPara.EFrameNum = 0x00;

	//if(ChannelCfg.stRecordPara.dwCompressionType == QMAPI_PT_H265)	
    if(stH264Config.enctype == 3)
		pCompressCfg->normHighCompPara.videoEncType = 0x0a;
	else
		pCompressCfg->normHighCompPara.videoEncType = 0x01;

    
	//pCompressCfg->normHighCompPara.audioEncType = ChannelCfg.stRecordPara.wEncodeAudio;
	if (runAudioCfg.mode)
        pCompressCfg->normHighCompPara.audioEncType = 1;
    else
        pCompressCfg->normHighCompPara.audioEncType = 0;
	pCompressCfg->normHighCompPara.audioEncType = 1;
	pCompressCfg->normHighCompPara.res[0] = 0x02;
	pCompressCfg->normHighCompPara.res[1] = 0x00;
	pCompressCfg->normHighCompPara.res[2] = 0x00;
	pCompressCfg->normHighCompPara.res[3] = 0x04;
	pCompressCfg->normHighCompPara.res[4] = 0x32;
	pCompressCfg->normHighCompPara.res[5] = 0x02;

    /////////////////////////////////////////////////////////////////////
    memset(&stH264Config, 0, sizeof(ST_GK_ENC_STREAM_H264_ATTR));
    ret = netcam_video_get(0, 1, &stH264Config);
    if (ret != 0)
    {
       HPRINT_ERR("netcam_video_get failed.");
       return -1;
    }

    HPRINT_INFO("stream1 -- runAudioCfg.mode:%d, resolution: %d * %d, rc_mode:%d, bps:%d, fps:%d, gop:%d \n", 
        runAudioCfg.mode, stH264Config.width, stH264Config.height, stH264Config.rc_mode,
        stH264Config.bps, stH264Config.fps, stH264Config.gop);


    //if(1 == ChannelCfg.stNetPara.wEncodeAudio)
    if (runAudioCfg.mode)
		pCompressCfg->netCompPara.streamType = 0x03;
	else
		pCompressCfg->netCompPara.streamType = 0x01;
	//pCompressCfg->netCompPara.resolution = convertResoloutionToHik(ChannelCfg.stNetPara.dwStreamFormat);//0x03;
	pCompressCfg->netCompPara.resolution = convertResoloutionToHik2(stH264Config.width, stH264Config.height);
	//if(1 == ChannelCfg.stNetPara.dwRateType)
	if (1 == stH264Config.rc_mode)
		pCompressCfg->netCompPara.bitrateType = 0x00;
	else
		pCompressCfg->netCompPara.bitrateType = 0x01;

	//pCompressCfg->netCompPara.quality = ChannelCfg.stNetPara.dwImageQuality;//0x02;
	pCompressCfg->netCompPara.quality = stH264Config.quality;

	pCompressCfg->netCompPara.maxBitRate = htonl((stH264Config.bps*1024) | 0x80000000);//htonl(0X0000000F);
	pCompressCfg->netCompPara.videoFrameRate = htonl(converFrameRateToHik(stH264Config.fps));//htonl(0x0000000d);//
	pCompressCfg->netCompPara.intervalFrameI = htons(stH264Config.gop);//htons(0x0019);//
	pCompressCfg->netCompPara.BFrameNum = 0x02;
	pCompressCfg->netCompPara.EFrameNum = 0x00;
	//pCompressCfg->netCompPara.videoEncType = ChannelCfg.stNetPara.wEncodeVideo;
//	if(ChannelCfg.stNetPara.dwCompressionType == QMAPI_PT_H265)	
    if(stH264Config.enctype == 3)
		pCompressCfg->netCompPara.videoEncType = 0x0a;
	else
		pCompressCfg->netCompPara.videoEncType = 0x01;
	
	//pCompressCfg->netCompPara.audioEncType = ChannelCfg.stNetPara.wEncodeAudio;//0x01;
	if (runAudioCfg.mode)
        pCompressCfg->netCompPara.audioEncType = 1;
    else
        pCompressCfg->netCompPara.audioEncType = 0;

#endif
	pCompressCfg->netCompPara.res[0] = 0x01;
	pCompressCfg->netCompPara.res[1] = 0x00;
	pCompressCfg->netCompPara.res[2] = 0x00;
	pCompressCfg->netCompPara.res[3] = 0x04;
	pCompressCfg->netCompPara.res[4] = 0x32;
	pCompressCfg->netCompPara.res[5] = 0x02;

	return retVal;
}


/***********************************************************************
 * Function: netClientGetCompressCfg
 * Description: get compress config param
 * Input: 
 *		 connfd - socket connfd 
 *       recvbuff - recevie data buff
 *       pClientSockAddr - client socket address(not need)
 * Output:  N/A
 * Return:  OK if successful, otherwise return error status number
 ***********************************************************************/
int netClientGetCompressCfgV30(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	NETCMD_CHAN_HEADER netGetChanCfg;
	int chan;
	struct{
		NETRET_HEADER header;
		NETPARAM_COMPRESS_CFG_V30 compressCfg;
	}netRetCompressCfg;
	UINT32 retVal;
	int sendlen;

	sendlen = sizeof(netRetCompressCfg);
	bzero((char *)&netRetCompressCfg, sendlen);
	netRetCompressCfg.header.length = htonl(sendlen);
	
	memcpy((char *)&netGetChanCfg, recvbuff, sizeof(NETCMD_CHAN_HEADER));
	chan = netGetChanCfg.channel = ntohl(netGetChanCfg.channel);

	retVal = verifyNetClientOperation(&(netGetChanCfg.header), pClientSockAddr, REMOTESHOWPARAMETER);
	if(retVal == NETRET_QUALIFIED)
	{
		if(netGetIpCompressCfgV30(chan,&netRetCompressCfg.compressCfg) != 0)
		{
			HPRINT_ERR("netGetIpCompressCfgV30 failed!\n");
			retVal = NETRET_DVR_OPER_FAILED;
		}
	}

	netRetCompressCfg.header.retVal = htonl(retVal);
	netRetCompressCfg.header.checkSum = htonl(checkByteSum((char *)&(netRetCompressCfg.header.retVal), sendlen-8));

	return writen(connfd, (char *)&netRetCompressCfg, sendlen);
}




/******************************************************************************
 * Function: netClientGetNetCfg
 * Description:8000 net client get network cfg
 * Input:  
 *		   connfd - socket connfd 
 *         recvbuff - recevie data buff
 *         pClientSockAddr - client socket address(not need)
 * Output:  N/A
 * Return:  OK if successful, otherwise return error status number
 *****************************************************************************/
int netClientGetNetCfg(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	NETCMD_HEADER  netCmdHeader;
	struct{
		NETRET_HEADER header;
		NETPARAM_NETWORK_CFG networkCfg;
	}netRetNetworkCfg;
	UINT32 retVal;
	int sendlen, ret;
    
	#if 0
	DMS_NET_NETWORK_CFG stNetworkCfg;
	DMS_NET_PPPOECFG stPppoeCfg;
    #endif

	sendlen = sizeof(netRetNetworkCfg);
	bzero((char *)&netRetNetworkCfg, sendlen);
	netRetNetworkCfg.header.length = htonl(sendlen);

	memcpy((char *)&netCmdHeader, recvbuff, sizeof(NETCMD_HEADER));
	retVal = verifyNetClientOperation(&netCmdHeader, pClientSockAddr, REMOTESHOWPARAMETER);
	if(retVal == NETRET_QUALIFIED)
	{
	    #if 0
		ret = dms_sysnetapi_ioctrl(g_hik_handle, DMS_NET_GET_NETCFG, 0, &stNetworkCfg, sizeof(stNetworkCfg)); 
		if(0 != ret)
		{
			HPRINT_ERR("\n");
			return -1;		
		}
		ret = dms_sysnetapi_ioctrl(g_hik_handle, DMS_NET_GET_PPPOECFG, 0, &stPppoeCfg, sizeof(stPppoeCfg)); 
		if(0 != ret)
		{
			HPRINT_ERR("\n");
			return -1;		
		}
		#endif

        ST_SDK_NETWORK_ATTR net_attr;
        sprintf(net_attr.name,"eth0");
        if(netcam_net_get(&net_attr) != 0)
        {
			HPRINT_ERR("\n");
			return -1;	
        }
        
		netRetNetworkCfg.networkCfg.length = htonl(sizeof(NETPARAM_NETWORK_CFG));

		//if (1 == stNetworkCfg.byEnableDHCP)
        if (1 == net_attr.dhcp)
		{
			netRetNetworkCfg.networkCfg.etherCfg[0].devIp = htonl(0);
		}
		else
		{
			//netRetNetworkCfg.networkCfg.etherCfg[0].devIp = htonl(inet_addr(stNetworkCfg.stEtherNet[0].strIPAddr.csIpV4));
            netRetNetworkCfg.networkCfg.etherCfg[0].devIp = htonl(inet_addr((char *)net_attr.ip));
        }
		//netRetNetworkCfg.networkCfg.etherCfg[0].devIpMask = htonl(inet_addr(stNetworkCfg.stEtherNet[0].strIPMask.csIpV4));
        netRetNetworkCfg.networkCfg.etherCfg[0].devIp = htonl(inet_addr((char *)net_attr.mask));

        //netRetNetworkCfg.networkCfg.etherCfg[0].mediaType = htonl(stNetworkCfg.stEtherNet[0].dwNetInterface);//htonl(5);	
        netRetNetworkCfg.networkCfg.etherCfg[0].mediaType = htonl(5);//htonl(5);	
		
		netRetNetworkCfg.networkCfg.etherCfg[0].ipPortNo = htons(8000);
		//memcpy(netRetNetworkCfg.networkCfg.etherCfg[0].macAddr, stNetworkCfg.stEtherNet[0].byMACAddr, MACADDR_LEN);
        get_hw_array(netRetNetworkCfg.networkCfg.etherCfg[0].macAddr, net_attr.mac);

        #if 0
		//normal
		if(0 == strlen(stNetworkCfg.stManagerIpAddr.csIpV4))
			netRetNetworkCfg.networkCfg.manageHostIp = 0;
		else
			netRetNetworkCfg.networkCfg.manageHostIp = htonl(inet_addr(stNetworkCfg.stManagerIpAddr.csIpV4));
		netRetNetworkCfg.networkCfg.manageHostPort =  htons(stNetworkCfg.wManagerPort);
        #else
        netRetNetworkCfg.networkCfg.manageHostIp = 0;
        netRetNetworkCfg.networkCfg.manageHostPort = 0;
        #endif
		//netRetNetworkCfg.networkCfg.httpPort= htons(stNetworkCfg.wHttpPort);
        netRetNetworkCfg.networkCfg.httpPort= htons(net_attr.httpPort);
		netRetNetworkCfg.networkCfg.ipResolverIpAddr = htonl(0);
        #if 0
		if(0 == strlen(stNetworkCfg.stMulticastIpAddr.csIpV4))
			netRetNetworkCfg.networkCfg.mcastAddr = 0;
		else
			netRetNetworkCfg.networkCfg.mcastAddr = htonl(inet_addr(stNetworkCfg.stMulticastIpAddr.csIpV4));
		netRetNetworkCfg.networkCfg.gatewayIp = htonl(inet_addr(stNetworkCfg.stGatewayIpAddr.csIpV4));
        #else
        netRetNetworkCfg.networkCfg.mcastAddr = 0;
        netRetNetworkCfg.networkCfg.gatewayIp = htonl(inet_addr(net_attr.gateway));
        #endif

		//nfs
		netRetNetworkCfg.networkCfg.nfsIp = htonl(0);
		//memcpy(netRetNetworkCfg.networkCfg.nfsDirectory, pNetCfg->nfsDiskParam[0].nfsDirectory, PATHNAME_LEN);

		//PPPoE
		#if 0
		netRetNetworkCfg.networkCfg.bEnablePPPoE = htonl(stPppoeCfg.bPPPoEEnable);
		memcpy(netRetNetworkCfg.networkCfg.pppoeName, stPppoeCfg.csPPPoEUser, NAME_LEN);
		memcpy(netRetNetworkCfg.networkCfg.pppoePassword, stPppoeCfg.csPPPoEPassword, PASSWD_LEN);
		if(0 == strlen(stPppoeCfg.stPPPoEIP.csIpV4))
			netRetNetworkCfg.networkCfg.pppoeIp = 0;
		else
			netRetNetworkCfg.networkCfg.pppoeIp = htonl(inet_addr(stPppoeCfg.stPPPoEIP.csIpV4));
        #else
        netRetNetworkCfg.networkCfg.bEnablePPPoE = htonl(0);
        #endif
	}

	netRetNetworkCfg.header.retVal = htonl(retVal);
	netRetNetworkCfg.header.checkSum = htonl(checkByteSum((char *)&(netRetNetworkCfg.header.retVal), sendlen-8));
	
	return writen(connfd, (char *)&netRetNetworkCfg, sendlen);
}

/******************************************************************************
 * Function: netClientSetNetCfg
 * Description:8000 net client set network cfg
 * Input:  
 *		   connfd - socket connfd 
 *         recvbuff - recevie data buff
 *         pClientSockAddr - client socket address(not need)
 * Output:  N/A
 * Return:  OK if successful, otherwise return error status number
 *****************************************************************************/
int netClientSetNetCfg(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	struct{
		NETCMD_HEADER header;
		NETPARAM_NETWORK_CFG networkCfg;
	}netCmdNetworkCfg;
	UINT32 retVal, val32;

	NETWORK *pNetCfg = NULL;

	memcpy((char *)&netCmdNetworkCfg, recvbuff, sizeof(netCmdNetworkCfg));
	retVal = verifyNetClientOperation(&(netCmdNetworkCfg.header), pClientSockAddr, REMOTESETPARAMETER);

	if(retVal == NETRET_QUALIFIED)
	{
		if(ntohl(netCmdNetworkCfg.networkCfg.length) != sizeof(NETPARAM_NETWORK_CFG)){	/* check length */
			HPRINT_ERR("Error length : %d, %d\n", ntohl(netCmdNetworkCfg.networkCfg.length), sizeof(NETPARAM_NETWORK_CFG));
			retVal = NETRET_ERROR_DATA;
		}
		else
		{
			/* check parameter */
			val32 = ntohl(netCmdNetworkCfg.networkCfg.etherCfg[0].mediaType);
			if(val32 < NET_IF_10M_HALF || val32 > NET_IF_AUTO)
			{
				HPRINT_ERR("Invalid mediaType(1--5): %d\n", val32);
				retVal = NETRET_ERROR_DATA;
				goto errExit;
			}
// 			val32 = (UINT32)(ntohs(netCmdNetworkCfg.networkCfg.etherCfg[0].ipPortNo)&0xffff);
// 			if(val32 < 2000 || val32 > 65535)
// 			{
// 				HPRINT_ERR("Invalid PortNo(2000--65535): %d\n", val32);
// 				retVal = NETRET_ERROR_DATA;
// 				goto errExit;
// 			}
			if(netCmdNetworkCfg.networkCfg.httpPort == netCmdNetworkCfg.networkCfg.etherCfg[0].ipPortNo)
			{
				HPRINT_ERR("Invalid portNo and httpPortNo.\n");
				retVal = NETRET_ERROR_DATA;
				goto errExit;
			}
			val32 = (UINT32)(ntohs(netCmdNetworkCfg.networkCfg.httpPort)&0xffff);
			if(val32 < 0 || val32 > 65535)
			{
				HPRINT_ERR("Invalid httpPortNo.\n");
				retVal = NETRET_ERROR_DATA;
				goto errExit;
			}
			val32 = (UINT32)(ntohs(netCmdNetworkCfg.networkCfg.manageHostPort)&0xffff);
			if(val32 < 0 || val32 > 65535)
			{
				HPRINT_ERR("Invalid manageHostPort.\n");
				retVal = NETRET_ERROR_DATA;
				goto errExit;
			}

			/* copy to pDevCfgParam */
			pthreadMutexLock(&globalMSem, WAIT_FOREVER);
			//pNetCfg = &(pDevCfgParam->networkPara);
			memset(pNetCfg,0,sizeof(NETWORK));
			if (ntohl(netCmdNetworkCfg.networkCfg.etherCfg[0].devIp) == 0)
			{
				pNetCfg->bUseDhcp = 1;
			}
			else
			{
				pNetCfg->etherNet[0].ipAddress.v4.s_addr = ntohl(netCmdNetworkCfg.networkCfg.etherCfg[0].devIp);
			}
			pNetCfg->etherNet[0].ipMask.v4.s_addr = ntohl(netCmdNetworkCfg.networkCfg.etherCfg[0].devIpMask);
			if(ntohl(netCmdNetworkCfg.networkCfg.etherCfg[0].mediaType) == 5)
			{
				pNetCfg->etherNet[0].mediaType = NET_IF_AUTO;
			}
			else if(ntohl(netCmdNetworkCfg.networkCfg.etherCfg[0].mediaType) == 6)
			{
				pNetCfg->etherNet[0].mediaType = NET_IF_100M_FULL;
			}
			else
			{
				pNetCfg->etherNet[0].mediaType = ntohl(netCmdNetworkCfg.networkCfg.etherCfg[0].mediaType);
			}
			pNetCfg->etherNet[0].ipPortNo = (UINT32)(ntohs(netCmdNetworkCfg.networkCfg.etherCfg[0].ipPortNo)&0xffff);
			
			pNetCfg->manageHost1IpAddr.v4.s_addr = ntohl(netCmdNetworkCfg.networkCfg.manageHostIp);
			pNetCfg->manageHost1Port = (UINT32)(ntohs(netCmdNetworkCfg.networkCfg.manageHostPort)&0xffff);
			pNetCfg->httpPortNo = ntohs(netCmdNetworkCfg.networkCfg.httpPort);
			pNetCfg->ipResolverAddr.v4.s_addr = ntohl(netCmdNetworkCfg.networkCfg.ipResolverIpAddr);
			pNetCfg->multicastIpAddr.v4.s_addr = ntohl(netCmdNetworkCfg.networkCfg.mcastAddr);
			pNetCfg->gatewayIpAddr.v4.s_addr = ntohl(netCmdNetworkCfg.networkCfg.gatewayIp);

			pNetCfg->nfsDiskParam[0].nfsHostIPaddr.v4.s_addr = ntohl(netCmdNetworkCfg.networkCfg.nfsIp);
			memcpy(pNetCfg->nfsDiskParam[0].nfsDirectory, netCmdNetworkCfg.networkCfg.nfsDirectory, PATHNAME_LEN);

			// 			pDevCfgParam->pppoePara.enablePPPoE = (ntohl(netCmdNetworkCfg.networkCfg.bEnablePPPoE)==0)?0:1;
			// 			memcpy(pDevCfgParam->pppoePara.username, netCmdNetworkCfg.networkCfg.pppoeName, NAME_LEN);
			// 			memcpy(pDevCfgParam->pppoePara.password, netCmdNetworkCfg.networkCfg.pppoePassword, PASSWD_LEN);
			// 			if(writeDevParam(pDevCfgParam) != 0)
			// 			{
			// 				retVal = NETRET_DVR_OPER_FAILED;
			// 			}
			#if 0
			{
				DMS_NET_NETWORK_CFG stNetworkCfg;
				DMS_NET_PPPOECFG stPppoeCfg;				
				int ret = dms_sysnetapi_ioctrl(g_hik_handle, DMS_NET_GET_NETCFG, 0, &stNetworkCfg, sizeof(stNetworkCfg));
				if (0 != ret)
				{
					HPRINT_ERR("get network cfg failed.\n");
					retVal = NETRET_DVR_OPER_FAILED;
				}
				ret = dms_sysnetapi_ioctrl(g_hik_handle, DMS_NET_GET_PPPOECFG, 0, &stPppoeCfg, sizeof(stPppoeCfg));
				if (0 != ret)
				{
					HPRINT_ERR("get pppoe cfg failed.\n");
					retVal = NETRET_DVR_OPER_FAILED;
				}

				char *tmpBufAddr;
				struct in_addr inaddr;
				inaddr.s_addr = ntohl(netCmdNetworkCfg.networkCfg.etherCfg[0].devIp);/*netCmdNetworkCfg.networkCfg.manageHostIp*/
				tmpBufAddr = (char *)inet_ntoa(inaddr);
				strcpy(stNetworkCfg.stEtherNet[0].strIPAddr.csIpV4, tmpBufAddr);

				ret = dms_sysnetapi_ioctrl(g_hik_handle, DMS_NET_SET_NETCFG, 0, &stNetworkCfg, sizeof(stNetworkCfg));
				if (ret != 0){
					retVal = NETRET_DVR_OPER_FAILED;
				}
				retVal = NETRET_QUALIFIED;
				HPRINT_ERR("fix ip addr:%s result:%d tmp:%s ret:%d\n", stNetworkCfg.stEtherNet[0].strIPAddr.csIpV4, ret, tmpBufAddr, retVal);
			}
            #else
            ST_SDK_NETWORK_ATTR net_attr;
            sprintf(net_attr.name,"eth0");
            if(netcam_net_get(&net_attr) != 0)
            {
                HPRINT_ERR("\n");
                return -1;  
            }
            char *tmpBufAddr;
            struct in_addr inaddr;
            inaddr.s_addr = ntohl(netCmdNetworkCfg.networkCfg.etherCfg[0].devIp);/*netCmdNetworkCfg.networkCfg.manageHostIp*/
            tmpBufAddr = (char *)inet_ntoa(inaddr);
            strcpy(net_attr.ip, tmpBufAddr);
            if(netcam_net_set(&net_attr) != 0)
            {
                PRINT_ERR("set eth0 parameter error");
            }

            #endif
			pthreadMutexUnlock(&globalMSem);

			/*send retval before initialize netIf, in case SDK receive timeout*/
			retVal = sendNetRetval(connfd, retVal);

//			initNetIf();
			
			return retVal;
		}
	}
    
errExit:
	return sendNetRetval(connfd, retVal);
}

static int netClientSetBasicCapability(char *p)
{
	p = p+sizeof(NETRET_HEADER);
	p += sprintf(p, "<BasicCapability version=\"2.0\">\r\n");
	p += sprintf(p, "<HardwareCapability>\r\n");
	p += sprintf(p, "<HardwareVersion>0x0</HardwareVersion>\r\n");
	p += sprintf(p, "<AlarmInPortNum>0</AlarmInPortNum>\r\n");
	p += sprintf(p, "<AlarmOutPortNum>0</AlarmOutPortNum>\r\n");
	p += sprintf(p, "<RS232Num>1</RS232Num>\r\n");
	p += sprintf(p, "<RS485Num>0</RS485Num>\r\n");
	p += sprintf(p, "<NetworkPortNum>1</NetworkPortNum>\r\n");
	p += sprintf(p, "<USBNum>0</USBNum>\r\n");
	p += sprintf(p, "<FlashSize>64</FlashSize>\r\n");
	p += sprintf(p, "<RamSize>1024</RamSize>\r\n");
	p += sprintf(p, "<USBVersion>0</USBVersion>\r\n");
	p += sprintf(p, "<SDNum>0</SDNum>\r\n");
	p += sprintf(p, "<HardDiskNum>0</HardDiskNum>\r\n");
	p += sprintf(p, "<SATANum>0</SATANum>\r\n");
	p += sprintf(p, "<eSATANum>0</eSATANum>\r\n");
	p += sprintf(p, "<miniSASNum>0</miniSASNum>\r\n");
	p += sprintf(p, "<VideoInNum>0</VideoInNum>\r\n");
	p += sprintf(p, "<AudioInNum>1</AudioInNum>\r\n");
	p += sprintf(p, "<VideoOutNum>1</VideoOutNum>\r\n");
	p += sprintf(p, "<AudioOutNum>1</AudioOutNum>\r\n");
	p += sprintf(p, "<AudioTalkNum>1</AudioTalkNum>\r\n");
	p += sprintf(p, "<SDSupport>0</SDSupport>\r\n");
	p += sprintf(p, "<POESupport>1</POESupport>\r\n");
	p += sprintf(p, "<IRSupport>1</IRSupport>\r\n");
	p += sprintf(p, "<VideoOutSupport>1</VideoOutSupport>\r\n");
	p += sprintf(p, "<ResetSupport>1</ResetSupport>\r\n");
	p += sprintf(p, "<CompleteRestoreSupport>1</CompleteRestoreSupport>\r\n");
	p += sprintf(p, "<AnalogChannelNum>1</AnalogChannelNum>\r\n");
	p += sprintf(p, "<CVBSNumber>1</CVBSNumber>\r\n");
	p += sprintf(p, "</HardwareCapability>\r\n");
	p += sprintf(p, "<SoftwareCapability>\r\n");
	p += sprintf(p, "<NewHdNo>1</NewHdNo>\r\n");
	p += sprintf(p, "<MaxNetworkHDNum>8</MaxNetworkHDNum>\r\n");
	p += sprintf(p, "<NasSupport>0</NasSupport>\r\n");
	p += sprintf(p, "<NasNumber>8</NasNumber>\r\n");
	p += sprintf(p, "<NetDiskIdentification>\r\n");
	p += sprintf(p, "<NASIdentification>\r\n");
	p += sprintf(p, "<NFSMountType>true</NFSMountType>\r\n");
	p += sprintf(p, "<CIFSMountType>\r\n");
	p += sprintf(p, "<usernameLen min=\"1\" max=\"32\"/>\r\n");
	p += sprintf(p, "<passwordLen min=\"1\" max=\"16\"/>\r\n");
	p += sprintf(p, "</CIFSMountType>\r\n");
	p += sprintf(p, "</NASIdentification>\r\n");
	p += sprintf(p, "</NetDiskIdentification>\r\n");
	p += sprintf(p, "<ShowStringNumber>4</ShowStringNumber>\r\n");	
	p += sprintf(p, "<MotionDetectAlarmSupport>1</MotionDetectAlarmSupport>\r\n");
	p += sprintf(p, "<HideAlarmSupport>1</HideAlarmSupport>\r\n");
	p += sprintf(p, "<ShelterSupport>1</ShelterSupport>\r\n");		
	p += sprintf(p, "<RtspSupport>1</RtspSupport>\r\n");
	p += sprintf(p, "<RtpoverRtspSupport>1</RtpoverRtspSupport>\r\n");
	p += sprintf(p, "<RtspoverHttpSupport>1</RtspoverHttpSupport>\r\n");
	p += sprintf(p, "<overHttpSupport>1</overHttpSupport>\r\n");
	p += sprintf(p, "<NtpSupport>1</NtpSupport>\r\n");
	p += sprintf(p, "<PtzSupport>1</PtzSupport>\r\n");		//ptz modify
	p += sprintf(p, "<DDNSSupport>1</DDNSSupport>\r\n");
	p += sprintf(p, "<DDNSHostType>0,1,3,4</DDNSHostType>\r\n");
	p += sprintf(p, "<SNMPSupport>1</SNMPSupport>\r\n");
	p += sprintf(p, "<SNMPVersion>1,2,3</SNMPVersion>\r\n");
	p += sprintf(p, "<UPNPSupport>1</UPNPSupport>\r\n");
	p += sprintf(p, "<Ipv6Support>1</Ipv6Support>\r\n");
	p += sprintf(p, "<MultipleStreamSupport>1</MultipleStreamSupport>\r\n");
	p += sprintf(p, "<SubStreamSupport>1</SubStreamSupport>\r\n");
	p += sprintf(p, "<EmailSupport>1</EmailSupport>\r\n");
	p += sprintf(p, "<SADPVersion>0,1</SADPVersion>\r\n");
	p += sprintf(p, "<MaxLoginNum>128</MaxLoginNum>\r\n");
	p += sprintf(p, "<MaxPreviewNum>20</MaxPreviewNum>\r\n");
	p += sprintf(p, "<MaxPlayBackNum>2</MaxPlayBackNum>\r\n");
	p += sprintf(p, "<MaxChanLinkNum>24</MaxChanLinkNum>\r\n");
	p += sprintf(p, "<RS232Config>1</RS232Config>\r\n");
	p += sprintf(p, "<PPPoEConfig>0</PPPoEConfig>\r\n");
	p += sprintf(p, "<UploadFTP>1</UploadFTP>\r\n");
	p += sprintf(p, "<QuotaRatio>1</QuotaRatio>\r\n");
	p += sprintf(p, "<DevModuleServerCfg>\r\n");
	p += sprintf(p, "<irLampServer opt=\"disable,enable\"/>\r\n");
	p += sprintf(p, "</DevModuleServerCfg>\r\n");
	p += sprintf(p, "<GBT28181AccessAbilitySupport>1</GBT28181AccessAbilitySupport>\r\n");
	p += sprintf(p, "<CameraParaDynamicAbilitySupport>1</CameraParaDynamicAbilitySupport>\r\n");
	p += sprintf(p, "<Language>\r\n");
	p += sprintf(p, "<supportType opt=\"1-chinese\"/>\r\n");
	p += sprintf(p, "</Language>\r\n");
	p += sprintf(p, "<NeedReboot>\r\n");
	p += sprintf(p, "<ImportConfigurationFileReboot>2</ImportConfigurationFileReboot>\r\n");
	p += sprintf(p, "<RestoreConfig>2</RestoreConfig>\r\n");
	p += sprintf(p, "<RS232workModeChange>1</RS232workModeChange>\r\n");
	p += sprintf(p, "<NetPortChange>1</NetPortChange>\r\n");
	p += sprintf(p, "<DhcpEnableChange>1</DhcpEnableChange>\r\n");
	p += sprintf(p, "<HttpPortChange>1</HttpPortChange>\r\n");
	p += sprintf(p, "<PPPoEChange>1</PPPoEChange>\r\n");
	p += sprintf(p, "<StandardTypeChange>1</StandardTypeChange>\r\n");
	p += sprintf(p, "<LineCodingEnableChange>1</LineCodingEnableChange>\r\n");
	p += sprintf(p, "<NetworkCardTypeChange>0</NetworkCardTypeChange>\r\n");
	p += sprintf(p, "<CompleteRestoreReboot>1</CompleteRestoreReboot>\r\n");		
	p += sprintf(p, "</NeedReboot>\r\n");
	p += sprintf(p, "</SoftwareCapability>\r\n");
	p += sprintf(p, "</BasicCapability>\r\n");
	
	return 0;
}
static int netClientSetCameraPara_v1(char *p)
{
	p = p+sizeof(NETRET_HEADER);
	p += sprintf(p, "<CAMERAPARA version=\"1.0\">\r\n");
    p += sprintf(p, "<PowerLineFrequencyMode>\r\n");
    p += sprintf(p, "<Range>0,1</Range>\r\n");
    p += sprintf(p, "</PowerLineFrequencyMode>\r\n");
    p += sprintf(p, "<WhiteBalance>\r\n");
    p += sprintf(p, "<WhiteBalanceMode>\r\n");
    p += sprintf(p, "<Range>0,1,3,6,14,15,16</Range>\r\n");
    p += sprintf(p, "</WhiteBalanceMode>\r\n");
    p += sprintf(p, "</WhiteBalance>\r\n");		
    p += sprintf(p, "<Exposure>\r\n");
    p += sprintf(p, "<ExposureMode>\r\n");
    p += sprintf(p, "<Range>1</Range>\r\n");
    p += sprintf(p, "</ExposureMode>\r\n");
    p += sprintf(p, "<ExposureSet>\r\n");
    p += sprintf(p, "<Range>20,0,2,3,4,5,6,17,18,7,8,9,10,11,12,13,14</Range>\r\n");
    p += sprintf(p, "</ExposureSet>\r\n");
    p += sprintf(p, "<exposureUSERSET>\r\n");
    p += sprintf(p, "<Min>1</Min>\r\n");
    p += sprintf(p, "<Max>40000</Max>\r\n");
    p += sprintf(p, "</exposureUSERSET>\r\n");
    p += sprintf(p, "</Exposure>\r\n");
    p += sprintf(p, "<IrisMode>\r\n");
    p += sprintf(p, "<Range>0,1</Range>\r\n");
    p += sprintf(p, "</IrisMode>\r\n");
    p += sprintf(p, "<AutoApertureLevel>\r\n");
    p += sprintf(p, "<Min>0</Min>\r\n");
    p += sprintf(p, "<Max>100</Max>\r\n");
    p += sprintf(p, "<Default>50</Default>\r\n");
    p += sprintf(p, "</AutoApertureLevel>\r\n");		
    p += sprintf(p, "<GainLevel>\r\n");
    p += sprintf(p, "<Min>0</Min>\r\n");
    p += sprintf(p, "<Max>100</Max>\r\n");
    p += sprintf(p, "<Default>100</Default>\r\n");
    p += sprintf(p, "</GainLevel>\r\n");
    p += sprintf(p, "<BrightnessLevel>\r\n");
    p += sprintf(p, "<Min>0</Min>\r\n");
    p += sprintf(p, "<Max>100</Max>\r\n");
    p += sprintf(p, "<Default>50</Default>\r\n");
    p += sprintf(p, "</BrightnessLevel>\r\n");		
    p += sprintf(p, "<ContrastLevel>\r\n");
    p += sprintf(p, "<Min>0</Min>\r\n");
    p += sprintf(p, "<Max>100</Max>\r\n");
    p += sprintf(p, "<Default>50</Default>\r\n");
    p += sprintf(p, "</ContrastLevel>\r\n");
    p += sprintf(p, "<SharpnessLevel>\r\n");
    p += sprintf(p, "<Min>0</Min>\r\n");
    p += sprintf(p, "<Max>100</Max>\r\n");
    p += sprintf(p, "<Default>50</Default>\r\n");
    p += sprintf(p, "</SharpnessLevel>\r\n");
    p += sprintf(p, "<SaturationLevel>\r\n");
    p += sprintf(p, "<Min>0</Min>\r\n");
    p += sprintf(p, "<Max>100</Max>\r\n");
    p += sprintf(p, "<Default>50</Default>\r\n");
    p += sprintf(p, "</SaturationLevel>\r\n");
    p += sprintf(p, "<WDR>\r\n");
    p += sprintf(p, "<WDREnabled>\r\n");
    p += sprintf(p, "<Range>0,1</Range>\r\n");
    p += sprintf(p, "</WDREnabled>\r\n");
    p += sprintf(p, "<WDRLevel1>\r\n");
    p += sprintf(p, "<Min>0</Min>\r\n");
    p += sprintf(p, "<Max>15</Max>\r\n");
    p += sprintf(p, "</WDRLevel1>\r\n");
    p += sprintf(p, "</WDR>\r\n");
	p += sprintf(p, "<DayNightFilter>\r\n");
    p += sprintf(p, "<DayNightFilterType>\r\n");
    p += sprintf(p, "<Range>0,1,2,3,4</Range>\r\n");
    p += sprintf(p, "</DayNightFilterType>\r\n");
    p += sprintf(p, "<SwitchSchedule>\r\n");
    p += sprintf(p, "<SwitchScheduleEnabled>\r\n");
    p += sprintf(p, "<Range>0,1</Range>\r\n");
    p += sprintf(p, "</SwitchScheduleEnabled>\r\n");
    p += sprintf(p, "<DayToNightFilterLevel>\r\n");
    p += sprintf(p, "<Range>0,1,2,3,4,5,6,7</Range>\r\n");
    p += sprintf(p, "</DayToNightFilterLevel>\r\n");
    p += sprintf(p, "<NightToDayFilterLevel>\r\n");
    p += sprintf(p, "<Range>0,1,2,3,4,5,6,7</Range>\r\n");
    p += sprintf(p, "</NightToDayFilterLevel>\r\n");
    p += sprintf(p, "<DayNightFilterTime>\r\n");
    p += sprintf(p, "<Min>5</Min>\r\n");
    p += sprintf(p, "<Max>120</Max>\r\n");
    p += sprintf(p, "</DayNightFilterTime>\r\n");
    p += sprintf(p, "</SwitchSchedule>\r\n");
    p += sprintf(p, "</DayNightFilter>\r\n");
    p += sprintf(p, "<Backlight>\r\n");
    p += sprintf(p, "<BacklightMode>\r\n");
    p += sprintf(p, "<Range>0,1,2,3,4,5,6</Range>\r\n");
    p += sprintf(p, "</BacklightMode>\r\n");
    p += sprintf(p, "</Backlight>\r\n");
    p += sprintf(p, "<Mirror>\r\n");
    p += sprintf(p, "<Range>0,1,2,3</Range>\r\n");
    p += sprintf(p, "</Mirror>\r\n");
    p += sprintf(p, "<LOCALOUTPUT>\r\n");
    p += sprintf(p, "<Range>0,1</Range>\r\n");
    p += sprintf(p, "</LOCALOUTPUT>\r\n");		
    p += sprintf(p, "<DigitalNoiseReduction>\r\n");
    p += sprintf(p, "<DigitalNoiseReductionEnable>\r\n");
    p += sprintf(p, "<Range>0,1,2</Range>\r\n");
    p += sprintf(p, "</DigitalNoiseReductionEnable>\r\n");
    p += sprintf(p, "<DigitalNoiseReductionLevel>\r\n");
    p += sprintf(p, "<Min>0</Min>\r\n");
    p += sprintf(p, "<Max>100</Max>\r\n");
    p += sprintf(p, "</DigitalNoiseReductionLevel>\r\n");
    p += sprintf(p, "<DigitalNoiseSpectralLevel>\r\n");
    p += sprintf(p, "<Min>0</Min>\r\n");
    p += sprintf(p, "<Max>100</Max>\r\n");
    p += sprintf(p, "</DigitalNoiseSpectralLevel>\r\n");
    p += sprintf(p, "<DigitalNoiseTemporalLevel>\r\n");
    p += sprintf(p, "<Min>0</Min>\r\n");
    p += sprintf(p, "<Max>100</Max>\r\n");
    p += sprintf(p, "</DigitalNoiseTemporalLevel>\r\n");
    p += sprintf(p, "</DigitalNoiseReduction>\r\n");
    p += sprintf(p, "<SceneMode>\r\n");
    p += sprintf(p, "<Range>0,1</Range>\r\n");
    p += sprintf(p, "</SceneMode>\r\n");
    p += sprintf(p, "<Dehaze>\r\n");
    p += sprintf(p, "<Range>0,2</Range>\r\n");
    p += sprintf(p, "</Dehaze>\r\n");
    p += sprintf(p, "<OnepushFocus>1</OnepushFocus>\r\n");
    p += sprintf(p, "</CAMERAPARA>\r\n");

	return 0;
}

static int netClientSetCameraPara_v2(char *p)
{
	p = p+sizeof(NETRET_HEADER);
	p += sprintf(p, "<CAMERAPARA version=\"2.0\">\r\n");
	p += sprintf(p, "<ChannelList>\r\n");
	p += sprintf(p, "<ChannelEntry>\r\n");
	p += sprintf(p, "<ChannelNumber>1</ChannelNumber>\r\n");
	p += sprintf(p, "<devType opt=\"ipc\"/>\r\n");
	p += sprintf(p, "<PowerLineFrequencyMode>\r\n");
	p += sprintf(p, "<Range>0,1</Range>\r\n");
	p += sprintf(p, "</PowerLineFrequencyMode>\r\n");
	p += sprintf(p, "<CaptureMode>\r\n");
	p += sprintf(p, "<captureModeP opt=\"close,1920*1080@25fps,1920*1080@50fps\"/>\r\n");
	p += sprintf(p, "<captureModeN opt=\"close,1920*1080@30fps,1920*1080@60fps\"/>\r\n");
	p += sprintf(p, "<captureModePWithIndex opt=\"0-close,17-1920*1080@25fps,19-1920*1080@50fps\"/>\r\n");
	p += sprintf(p, "<captureModeNWithIndex opt=\"0-close,18-1920*1080@30fps,20-1920*1080@60fps\"/>\r\n");
	p += sprintf(p, "<CaptureModeIndex19>\r\n");
	p += sprintf(p, "<mutexAbility opt=\"WDR\"/>\r\n");
	p += sprintf(p, "</CaptureModeIndex19>\r\n");
	p += sprintf(p, "<CaptureModeIndex20>\r\n");
	p += sprintf(p, "<mutexAbility opt=\"WDR\"/>\r\n");
	p += sprintf(p, "</CaptureModeIndex20>\r\n");
	p += sprintf(p, "</CaptureMode>\r\n");
	p += sprintf(p, "<WhiteBalance>\r\n");
	p += sprintf(p, "<WhiteBalanceMode>\r\n");
	p += sprintf(p, "<Range>0,1,3,6,14,15,16</Range>\r\n");
	p += sprintf(p, "<Default>1</Default>\r\n");
	p += sprintf(p, "</WhiteBalanceMode>\r\n");
	p += sprintf(p, "<WhiteBalanceModeRGain>\r\n");
	p += sprintf(p, "<Min>0</Min>\r\n");
	p += sprintf(p, "<Max>100</Max>\r\n");
	p += sprintf(p, "<Default>0</Default>\r\n");
	p += sprintf(p, "</WhiteBalanceModeRGain>\r\n");
	p += sprintf(p, "<WhiteBalanceModeBGain>\r\n");
	p += sprintf(p, "<Min>0</Min>\r\n");
	p += sprintf(p, "<Max>100</Max>\r\n");
	p += sprintf(p, "<Default>0</Default>\r\n");
	p += sprintf(p, "</WhiteBalanceModeBGain>\r\n");
	p += sprintf(p, "</WhiteBalance>\r\n");
	p += sprintf(p, "<Exposure>\r\n");
	p += sprintf(p, "<ExposureMode>\r\n");
	p += sprintf(p, "<Range>1</Range>\r\n");
	p += sprintf(p, "<Default>1</Default>\r\n");
	p += sprintf(p, "</ExposureMode>\r\n");
	p += sprintf(p, "<ExposureSet>\r\n");
	p += sprintf(p, "<Range>20,0,2,3,4,5,6,17,18,7,8,9,10,11,12,13,14</Range>\r\n");
	p += sprintf(p, "<ExposureSetIndex20>\r\n");
	p += sprintf(p, "<mutexAbility opt=\"WDR\"/>\r\n");
	p += sprintf(p, "</ExposureSetIndex20>\r\n");
	p += sprintf(p, "<ExposureSetIndex0>\r\n");
	p += sprintf(p, "<mutexAbility opt=\"WDR\"/>\r\n");
	p += sprintf(p, "</ExposureSetIndex0>\r\n");
	p += sprintf(p, "<ExposureSetIndex2>\r\n");
	p += sprintf(p, "<mutexAbility opt=\"WDR\"/>\r\n");
	p += sprintf(p, "</ExposureSetIndex2>\r\n");
	p += sprintf(p, "<ExposureSetIndex3>\r\n");
	p += sprintf(p, "<mutexAbility opt=\"WDR\"/>\r\n");
	p += sprintf(p, "</ExposureSetIndex3>\r\n");
	p += sprintf(p, "</ExposureSet>\r\n");
	p += sprintf(p, "<exposureUSERSET>\r\n");
	p += sprintf(p, "<Min>1</Min>\r\n");
	p += sprintf(p, "<Max>40000</Max>\r\n");
	p += sprintf(p, "</exposureUSERSET>\r\n");
	p += sprintf(p, "</Exposure>\r\n");
	p += sprintf(p, "<IrisMode>\r\n");
	p += sprintf(p, "<Range>0,1</Range>\r\n");
	p += sprintf(p, "<Default>0</Default>\r\n");
	p += sprintf(p, "</IrisMode>\r\n");
	p += sprintf(p, "<AutoApertureLevel>\r\n");
	p += sprintf(p, "<Min>0</Min>\r\n");
	p += sprintf(p, "<Max>100</Max>\r\n");
	p += sprintf(p, "<Default>50</Default>\r\n");
	p += sprintf(p, "</AutoApertureLevel>\r\n");
	p += sprintf(p, "<GainLevel>\r\n");
	p += sprintf(p, "<Min>0</Min>\r\n");
	p += sprintf(p, "<Max>100</Max>\r\n");
	p += sprintf(p, "<Default>100</Default>\r\n");
	p += sprintf(p, "</GainLevel>\r\n");
	p += sprintf(p, "<BrightnessLevel>\r\n");
	p += sprintf(p, "<Min>0</Min>\r\n");
	p += sprintf(p, "<Max>100</Max>\r\n");
	p += sprintf(p, "<Default>50</Default>\r\n");
	p += sprintf(p, "</BrightnessLevel>\r\n");
	p += sprintf(p, "<ContrastLevel>\r\n");
	p += sprintf(p, "<Min>0</Min>\r\n");
	p += sprintf(p, "<Max>100</Max>\r\n");
	p += sprintf(p, "<Default>50</Default>\r\n");
	p += sprintf(p, "</ContrastLevel>\r\n");
	p += sprintf(p, "<SharpnessLevel>\r\n");
	p += sprintf(p, "<Min>0</Min>\r\n");
	p += sprintf(p, "<Max>100</Max>\r\n");
	p += sprintf(p, "<Default>50</Default>\r\n");
	p += sprintf(p, "</SharpnessLevel>\r\n");
	p += sprintf(p, "<SaturationLevel>\r\n");
	p += sprintf(p, "<Min>0</Min>\r\n");
	p += sprintf(p, "<Max>100</Max>\r\n");
	p += sprintf(p, "<Default>50</Default>\r\n");
	p += sprintf(p, "</SaturationLevel>\r\n");
	p += sprintf(p, "<WDR>\r\n");
	p += sprintf(p, "<WDREnabled>\r\n");
	p += sprintf(p, "<Range>0,1</Range>\r\n");
	p += sprintf(p, "<Default>0</Default>\r\n");
	p += sprintf(p, "</WDREnabled>\r\n");
	p += sprintf(p, "<WDRLevel1>\r\n");
	p += sprintf(p, "<Min>0</Min>\r\n");
	p += sprintf(p, "<Max>15</Max>\r\n");
	p += sprintf(p, "<Default>10</Default>\r\n");
	p += sprintf(p, "</WDRLevel1>\r\n");
	p += sprintf(p, "<mutexAbility opt=\"8-1280*720@60fps,7-1280*720@50fps,19-1920*1080@50fps,20-1920*1080@60fps,exposureIndex20,exposureIndex0,exposureIndex2,exposureI\"/>\r\n");
	p += sprintf(p, "</WDR>\r\n");
	p += sprintf(p, "<DayNightFilter>\r\n");
	p += sprintf(p, "<DayNightFilterType>\r\n");
	p += sprintf(p, "<Range>0,1,2,3,4</Range>\r\n");
	p += sprintf(p, "<Default>2</Default>\r\n");
	p += sprintf(p, "</DayNightFilterType>\r\n");
	p += sprintf(p, "<SwitchSchedule>\r\n");
	p += sprintf(p, "<SwitchScheduleEnabled>\r\n");
	p += sprintf(p, "<Range>0,1</Range>\r\n");
	p += sprintf(p, "<Default>0</Default>\r\n");
	p += sprintf(p, "</SwitchScheduleEnabled>\r\n");
	p += sprintf(p, "<NightToDayFilterLevel>\r\n");
	p += sprintf(p, "<Range>0,1,2,3,4,5,6,7</Range>\r\n");
	p += sprintf(p, "<Default>4</Default>\r\n");
	p += sprintf(p, "</NightToDayFilterLevel>\r\n");
	p += sprintf(p, "<DayNightFilterTime>\r\n");
	p += sprintf(p, "<Min>5</Min>\r\n");
	p += sprintf(p, "<Max>120</Max>\r\n");
	p += sprintf(p, "<Default>5</Default>\r\n");
	p += sprintf(p, "</DayNightFilterTime>\r\n");
	p += sprintf(p, "<TimeSchedule>\r\n");
	p += sprintf(p, "<BeginTime>1</BeginTime>\r\n");
	p += sprintf(p, "<EndTime>1</EndTime>\r\n");
	p += sprintf(p, "</TimeSchedule>\r\n");
	p += sprintf(p, "</SwitchSchedule>\r\n");
	p += sprintf(p, "<AlarmInTrigType>\r\n");
	p += sprintf(p, "<Range>0,1</Range>\r\n");
	p += sprintf(p, "<Default>0</Default>\r\n");
	p += sprintf(p, "</AlarmInTrigType>\r\n");
	p += sprintf(p, "<DayNightFilterandGain>\r\n");
	p += sprintf(p, "<enabled>true</enabled>\r\n");
	p += sprintf(p, "</DayNightFilterandGain>\r\n");
	p += sprintf(p, "</DayNightFilter>\r\n");
	p += sprintf(p, "<Backlight>\r\n");
	p += sprintf(p, "<BacklightMode>\r\n");
	p += sprintf(p, "<Range>0,1,2,3,4,5,6</Range>\r\n");
	p += sprintf(p, "<Default>0</Default>\r\n");
	p += sprintf(p, "</BacklightMode>\r\n");
	p += sprintf(p, "</Backlight>\r\n");
	p += sprintf(p, "<Mirror>\r\n");
	p += sprintf(p, "<Range>0,1,2,3</Range>\r\n");
	p += sprintf(p, "<Default>0</Default>\r\n");
	p += sprintf(p, "</Mirror>\r\n");
	p += sprintf(p, "<LOCALOUTPUT>\r\n");
	p += sprintf(p, "<Range>0,1</Range>\r\n");
	p += sprintf(p, "<Default>1</Default>\r\n");
	p += sprintf(p, "</LOCALOUTPUT>\r\n");
	p += sprintf(p, "<DigitalNoiseReduction>\r\n");
	p += sprintf(p, "<DigitalNoiseReductionEnable>\r\n");
	p += sprintf(p, "<Range>0,1,2</Range>\r\n");
	p += sprintf(p, "<Default>1</Default>\r\n");
	p += sprintf(p, "</DigitalNoiseReductionEnable>\r\n");
	p += sprintf(p, "<DigitalNoiseReductionLevel>\r\n");
	p += sprintf(p, "<Min>0</Min>\r\n");
	p += sprintf(p, "<Max>100</Max>\r\n");
	p += sprintf(p, "<Default>50</Default>\r\n");
	p += sprintf(p, "</DigitalNoiseReductionLevel>\r\n");
	p += sprintf(p, "<DigitalNoiseSpectralLevel>\r\n");
	p += sprintf(p, "<Min>0</Min>\r\n");
	p += sprintf(p, "<Max>100</Max>\r\n");
	p += sprintf(p, "<Default>50</Default>\r\n");
	p += sprintf(p, "</DigitalNoiseSpectralLevel>\r\n");
	p += sprintf(p, "<DigitalNoiseTemporalLevel>\r\n");
	p += sprintf(p, "<Min>0</Min>\r\n");
	p += sprintf(p, "<Max>100</Max>\r\n");
	p += sprintf(p, "<Default>50</Default>\r\n");
	p += sprintf(p, "</DigitalNoiseTemporalLevel>\r\n");
	p += sprintf(p, "</DigitalNoiseReduction>\r\n");
	p += sprintf(p, "<SceneMode>\r\n");
	p += sprintf(p, "<Range>0,1</Range>\r\n");
	p += sprintf(p, "<Default>0</Default>\r\n");
	p += sprintf(p, "</SceneMode>\r\n");
	p += sprintf(p, "<OnepushFocus>1</OnepushFocus>\r\n");
	p += sprintf(p, "<Dehaze>\r\n");
	p += sprintf(p, "<DehazeEnable>0,2</DehazeEnable>\r\n");
	p += sprintf(p, "</Dehaze>\r\n");
	p += sprintf(p, "<SmartIR>\r\n");
	p += sprintf(p, "<Range>0,1</Range>\r\n");
	p += sprintf(p, "<modeType opt=\"automatic,manual\"/>\r\n");
	p += sprintf(p, "<IRDistance min=\"1\" max=\"100\"/>\r\n");
	p += sprintf(p, "</SmartIR>\r\n");
	p += sprintf(p, "<LightInhibit>\r\n");
	p += sprintf(p, "<LightInhibitEnable opt=\"true,false\"/>\r\n");
	p += sprintf(p, "</LightInhibit>\r\n");
	p += sprintf(p, "<GrayLevel>\r\n");
	p += sprintf(p, "<Range>0,1</Range>\r\n");
	p += sprintf(p, "<Default>0</Default>\r\n");
	p += sprintf(p, "</GrayLevel>\r\n");
	p += sprintf(p, "<FocusMode>\r\n");
	p += sprintf(p, "<FocusModeSet>\r\n");
	p += sprintf(p, "<Range>0,1</Range>\r\n");
	p += sprintf(p, "<Default>1</Default>\r\n");
	p += sprintf(p, "</FocusModeSet>\r\n");
	p += sprintf(p, "</FocusMode>\r\n");
	p += sprintf(p, "<CorridorMode>\r\n");
	p += sprintf(p, "<corridorModeFunEnable opt=\"true,false\"/>\r\n");
	p += sprintf(p, "</CorridorMode>\r\n");
	p += sprintf(p, "<ISPAdvanceCfg>\r\n");
	p += sprintf(p, "<ISPSupportMode opt=\"dayMode,nightMode\"/>\r\n");
	p += sprintf(p, "<workMode opt=\"auto,schedule\"/>\r\n");
	p += sprintf(p, "<TimeSchedule>\r\n");
	p += sprintf(p, "<beginTime opt=\"hour,min,sec,millisec\"/>\r\n");
	p += sprintf(p, "<endTime opt=\"hour,min,sec,millisec\"/>\r\n");
	p += sprintf(p, "</TimeSchedule>\r\n");
	p += sprintf(p, "<ISPCfgSupport opt=\"whiteBalanceMode,whiteBalanceModeRGain,whiteBalanceModeBGain,exposureSet,exposureUserSet,gainLevel,brightnessLevel,contrastLevel,sharpnessLevel,WDREnabled,WDRLevel1,backlightMode,digitalNoiseReductionEnable,digitalNoiseReductionLevel,digitalNoiseSpectralLevel,digitalNoiseTemporalLevel,dehazeEnable,lightInhibitEnable,grayLevel\"/>\r\n");
	p += sprintf(p, "</ISPAdvanceCfg>\r\n");
	p += sprintf(p, "</ChannelEntry>\r\n");
	p += sprintf(p, "</ChannelList>\r\n");
	p += sprintf(p, "</CAMERAPARA>\r\n");
		
	return 0;
}

static int netClientSetJpegCaptureAbility(char *p)
{
	p = p+sizeof(NETRET_HEADER);
	p += sprintf(p, "<JpegCaptureAbility version=\"2.0\">\r\n");
	p += sprintf(p, "<channelNO>1</channelNO>\r\n");
	p += sprintf(p, "<FindPicInfo>\r\n");
	p += sprintf(p, "<supportFileType opt=\"CMR,MOTION,ALARM,EDR,ALARMANDMOTION,manual,facedetection,LineDetection,FieldDetection,scenechangedetection,regionEntrance,regionExiting,loitering,group,rapidMove,parking,unattendedBaggage,attendedBaggage,allType\"/>\r\n");
	p += sprintf(p, "<province opt=\"1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,0xff\"/>\r\n");
	p += sprintf(p, "<StartTime>\r\n");
	p += sprintf(p, "<year min=\"1970\" max=\"2038\"/>\r\n");
	p += sprintf(p, "<month min=\"1\" max=\"12\"/>\r\n");
	p += sprintf(p, "<day min=\"1\" max=\"30\"/>\r\n");
	p += sprintf(p, "<hour min=\"1\" max=\"24\"/>\r\n");
	p += sprintf(p, "<minute min=\"1\" max=\"60\"/>\r\n");
	p += sprintf(p, "<second min=\"1\" max=\"60\"/>\r\n");
	p += sprintf(p, "</StartTime>\r\n");
	p += sprintf(p, "<StopTime>\r\n");
	p += sprintf(p, "<year min=\"1970\" max=\"2038\"/>\r\n");
	p += sprintf(p, "<month min=\"1\" max=\"12\"/>\r\n");
	p += sprintf(p, "<day min=\"1\" max=\"30\"/>\r\n");
	p += sprintf(p, "<hour min=\"1\" max=\"24\"/>\r\n");
	p += sprintf(p, "<minute min=\"1\" max=\"60\"/>\r\n");
	p += sprintf(p, "<second min=\"1\" max=\"60\"/>\r\n");
	p += sprintf(p, "</StopTime>\r\n");
	p += sprintf(p, "</FindPicInfo>\r\n");
	p += sprintf(p, "<ManualCapture>\r\n");
	p += sprintf(p, "<ResolutionEntry>\r\n");
	p += sprintf(p, "<resolutionName>HD1080P</resolutionName>\r\n");
	p += sprintf(p, "<index>9</index>\r\n");
	p += sprintf(p, "</ResolutionEntry>\r\n");
	p += sprintf(p, "</ManualCapture>\r\n");
	p += sprintf(p, "<SchedCapture>\r\n");
	p += sprintf(p, "<TimingCap>\r\n");
	p += sprintf(p, "<ResolutionEntry>\r\n");
	p += sprintf(p, "<resolutionName>HD1080P</resolutionName>\r\n");
	p += sprintf(p, "<index>9</index>\r\n");
	p += sprintf(p, "</ResolutionEntry>\r\n");
	p += sprintf(p, "<intervalUnit>ms</intervalUnit>\r\n");
	p += sprintf(p, "<interval min=\"500\" max=\"604800000\" opt=\"0\"/>\n");
	p += sprintf(p, "<DayCapture>\r\n");
	p += sprintf(p, "<captureType>timing</captureType>\r\n");
	p += sprintf(p, "</DayCapture>\r\n");
	p += sprintf(p, "<TimeSlot>\r\n");
	p += sprintf(p, "<slotNum>8</slotNum>\r\n");
	p += sprintf(p, "<captureType>timing</captureType>\r\n");
	p += sprintf(p, "</TimeSlot>\r\n");
	p += sprintf(p, "</TimingCap>\r\n");
	p += sprintf(p, "<EventCap>\r\n");
	p += sprintf(p, "<eventType opt=\"motion,faceDetect,lineDetection,fieldDetection,sceneChangeDetection,regionEntrance,regionExiting,loitering,group,rapidMove,parking,unattendedBaggage,attendedBaggage\"/>\r\n");
	p += sprintf(p, "<ResolutionEntry>\r\n");
	p += sprintf(p, "<resolutionName>HD1080P</resolutionName>\r\n");
	p += sprintf(p, "<index>9</index>\r\n");
	p += sprintf(p, "</ResolutionEntry>\r\n");
	p += sprintf(p, "<intervalUnit>ms</intervalUnit>\r\n");
	p += sprintf(p, "<interval min=\"500\" max=\"65535\" opt=\"0\"/>\n");
	p += sprintf(p, "<capTimes min=\"1\" max=\"120\"/>\r\n");
	p += sprintf(p, "<eventCapChan opt=\"1\"/>\r\n");
	p += sprintf(p, "<alarmInCapChan opt=\"1\"/>\r\n");
	p += sprintf(p, "</EventCap>\r\n");
	p += sprintf(p, "</SchedCapture>\r\n");
	p += sprintf(p, "</JpegCaptureAbility>\r\n");

	return 0;
}

static int netClientSetSerialAbility(char *p)
{
	p = p+sizeof(NETRET_HEADER);
	p += sprintf(p, "<SerialAbility version=\"2.0\">\r\n");
	p += sprintf(p, "<RS232>\r\n");
	p += sprintf(p, "<RS232Num>1</RS232Num>\r\n");
	p += sprintf(p, "<RS232Entry>\r\n");
	p += sprintf(p, "<baudRate opt=\"2400,4800,9600,19200,38400,57600,76800,115.2k\"/>\r\n");
	p += sprintf(p, "<dataBit opt=\"5,6,7,8\"/>\r\n");
	p += sprintf(p, "<stopBit opt=\"1,2\"/>\r\n");
	p += sprintf(p, "<parity opt=\"none,odd,even\"/>\r\n");
	p += sprintf(p, "<flowcontrol opt=\"none,soft\"/>\r\n");
	p += sprintf(p, "<workMode opt=\"transparentChan,console\"/>\r\n");
	p += sprintf(p, "</RS232Entry>\r\n");
	p += sprintf(p, "</RS232>\r\n");
	p += sprintf(p, "</SerialAbility>\r\n");

	return 0;
}
static int netClientSetAlarmAbility(char *p)
{
	p = p+sizeof(NETRET_HEADER);
	p += sprintf(p, "<AlarmAbility version=\"2.0\">\r\n");
	p += sprintf(p, "<channelEntry>\r\n");
	p += sprintf(p, "<channelID>1</channelID>\r\n");
	p += sprintf(p, "</channelEntry>\r\n");
	p += sprintf(p, "</AlarmAbility>\r\n");

	return 0;
}

static int netClientSetUserAbility(char *p)
{
	p = p+sizeof(NETRET_HEADER);
	p += sprintf(p, "<UserAbility version=\"2.0\">\r\n");
	p += sprintf(p, "<userNum>32</userNum>\r\n");
	p += sprintf(p, "<userNameLength min=\"1\" max=\"32\"/>\r\n");
	p += sprintf(p, "<userPasswordLength min=\"1\" max=\"16\"/>\r\n");
	p += sprintf(p, "<RemotePermission>\r\n");
	p += sprintf(p, "<permissionType type=\"admin\" opt=\"preview,alarmOutOrUpload,record,playback,parameterConfig,logOrStateCheck,restartOrShutdown,upgrade,voiceTalk,PTZControl,contorlLocalOut,transParentChannel\"/>\r\n");
	p += sprintf(p, "</RemotePermission>\r\n");
	p += sprintf(p, "<RemotePermission>\r\n");
	p += sprintf(p, "<permissionType type=\"viewer\" opt=\"preview,playback,logOrStateCheck\"/>\r\n");
	p += sprintf(p, "</RemotePermission>\r\n");
	p += sprintf(p, "<RemotePermission>\r\n");
	p += sprintf(p, "<permissionType type=\"operator\" opt=\"preview,alarmOutOrUpload,record,playback,parameterConfig,logOrStateCheck,restartOrShutdown,upgrade,voiceTalk,PTZControl,contorlLocalOut,transParentChannel\"/>\r\n");
	p += sprintf(p, "</RemotePermission>\r\n");
	p += sprintf(p, "<UserNet>\r\n");
	p += sprintf(p, "<IPV4Address>true</IPV4Address>\r\n");
	p += sprintf(p, "<IPV6Address>true</IPV6Address>\r\n");
	p += sprintf(p, "<MACAddress>true</MACAddress>\r\n");
	p += sprintf(p, "</UserNet>\r\n");
	p += sprintf(p, "<ViewerDefaultPermission>\r\n");
	p += sprintf(p, "<RemotePermission>\r\n");
	p += sprintf(p, "<permissionType opt=\"preview\"/>\r\n");
	p += sprintf(p, "</RemotePermission>\r\n");
	p += sprintf(p, "</ViewerDefaultPermission>\r\n");
	p += sprintf(p, "<OperatorDefaultPermission>\r\n");
	p += sprintf(p, "<RemotePermission>\r\n");
	p += sprintf(p, "<permissionType opt=\"preview,record,playback,logOrStateCheck,voiceTalk,PTZControl\"/>\r\n");
	p += sprintf(p, "</RemotePermission>\r\n");
	p += sprintf(p, "</OperatorDefaultPermission>\r\n");
	p += sprintf(p, "<PasswordManage>\r\n");
	p += sprintf(p, "<lockCount opt=\"0,3,5,10\"/>\r\n");
	p += sprintf(p, "<lockTime opt=\"10,20,30\"/>\r\n");
	p += sprintf(p, "<lockType opt=\"invalid,userName,IP\"/>\r\n");
	p += sprintf(p, "<charNum opt=\"noLimit,limit\"/>\r\n");
	p += sprintf(p, "<complexity opt=\"simple,complex\"/>\r\n");
	p += sprintf(p, "</PasswordManage>\r\n");
	p += sprintf(p, "<UnlockUser>\r\n");
	p += sprintf(p, "<unLockType opt=\"userName,allUser,IP,allIP\"/>\r\n");
	p += sprintf(p, "<ipVersion opt=\"ipV4,ipV6\"/>\r\n");
	p += sprintf(p, "<userNameLength min=\"1\" max=\"32\"/>\r\n");
	p += sprintf(p, "<ipAddress>true</ipAddress>\r\n");
	p += sprintf(p, "</UnlockUser>\r\n");
	p += sprintf(p, "</UserAbility>\r\n");

	return 0;
}

static int netClientSetNetAppAbility(char *p)
{
	p = p+sizeof(NETRET_HEADER);
	p += sprintf(p, "<NetAppAbility version=\"2.0\">\r\n");
	p += sprintf(p, "<NTP>\r\n");
	p += sprintf(p, "<intervalUnit>hour</intervalUnit>\r\n");
	p += sprintf(p, "<serverTest>true</serverTest>\r\n");
	p += sprintf(p, "</NTP>\r\n");
	p += sprintf(p, "<Net>\r\n");
	p += sprintf(p, "<NetworkInterface>\r\n");
	p += sprintf(p, "<networkInterfaceNum>1</networkInterfaceNum>\r\n");
	p += sprintf(p, "<NetworkInterfaceEntry>\r\n");
	p += sprintf(p, "<id>1</id>\r\n");
	p += sprintf(p, "<type opt=\"10Mbase-T,10MBase-T-full,100MBase-TX,100M-full,10M/100M/1000M-adapt,1000M-full\"/>\r\n");
	p += sprintf(p, "<MTU min=\"1280\" max=\"1500\"/>\r\n");
	p += sprintf(p, "</NetworkInterfaceEntry>\r\n");
	p += sprintf(p, "</NetworkInterface>\r\n");
	p += sprintf(p, "<multicastIpAddr opt=\"IPV4,IPV6\"/>\r\n");
	p += sprintf(p, "<IPv6Address>\r\n");
	p += sprintf(p, "<IPv6List>\r\n");
	p += sprintf(p, "<enabled>true</enabled>\r\n");
	p += sprintf(p, "</IPv6List>\r\n");
	p += sprintf(p, "<IPv6Mode opt=\"routerAdvertisement,DHCP,manual\"/>\r\n");
	p += sprintf(p, "</IPv6Address>\r\n");
	p += sprintf(p, "<DHCPandPPPoE>\r\n");
	p += sprintf(p, "<enabled>true</enabled>\r\n");
	p += sprintf(p, "</DHCPandPPPoE>\r\n");
	p += sprintf(p, "<IPTest>true</IPTest>\r\n");
	p += sprintf(p, "</Net>\r\n");
	p += sprintf(p, "<Email>\r\n");
	p += sprintf(p, "<receiverNum>3</receiverNum>\r\n");	
	p += sprintf(p, "<emailTest>true</emailTest>\r\n");
	p += sprintf(p, "<emailTestWithParam>true</emailTestWithParam>\r\n");
	p += sprintf(p, "</Email>\r\n");
	p += sprintf(p, "<UPNP>\r\n");
	p += sprintf(p, "<NATType opt=\"manual,auto\"/>\r\n");
	p += sprintf(p, "<friendNameLen min=\"1\" max=\"64\"/>\r\n");
	p += sprintf(p, "<serverPort>\r\n");
	p += sprintf(p, "<enabled>true</enabled>\r\n");
	p += sprintf(p, "</serverPort>\r\n");
	p += sprintf(p, "<HTTPPort>\r\n");
	p += sprintf(p, "<enabled>true</enabled>\r\n");
	p += sprintf(p, "</HTTPPort>\r\n");
	p += sprintf(p, "<RTSPPort>\r\n");
	p += sprintf(p, "<enabled>true</enabled>\r\n");
	p += sprintf(p, "</RTSPPort>\r\n");
	p += sprintf(p, "</UPNP>\r\n");
	p += sprintf(p, "<IPAddrFilter>\r\n");
	p += sprintf(p, "<IPAddrType opt=\"IPV4\"/>\r\n");
	p += sprintf(p, "<filterType opt=\"forbid,permit\"/>\r\n");
	p += sprintf(p, "</IPAddrFilter>\r\n");
	p += sprintf(p, "<FTP>\r\n");
	p += sprintf(p, "<AnonyFTP>\r\n");
	p += sprintf(p, "<enabled>true</enabled>\r\n");
	p += sprintf(p, "</AnonyFTP>\r\n");
	p += sprintf(p, "<dirLevel opt=\"rootDir,topDir,subDir\"/>\r\n");
	p += sprintf(p, "<topDirMode opt=\"deviceName,deviceNO,deviceIP,custom\"/>\r\n");
	p += sprintf(p, "<subDirMode opt=\"chanName,chanNO,custom\"/>\r\n");
	p += sprintf(p, "<serverTest>true</serverTest>\r\n");
	p += sprintf(p, "</FTP>\r\n");
	p += sprintf(p, "<QoS>\r\n");
	p += sprintf(p, "<manageDscp>\r\n");
	p += sprintf(p, "<Min>0</Min>\r\n");
	p += sprintf(p, "<Max>63</Max>\r\n");
	p += sprintf(p, "</manageDscp>\r\n");
	p += sprintf(p, "<alarmDscp>\r\n");
	p += sprintf(p, "<Min>0</Min>\r\n");
	p += sprintf(p, "<Max>63</Max>\r\n");
	p += sprintf(p, "</alarmDscp>\r\n");
	p += sprintf(p, "<videoDscp>\r\n");
	p += sprintf(p, "<Min>0</Min>\r\n");
	p += sprintf(p, "<Max>63</Max>\r\n");
	p += sprintf(p, "</videoDscp>\r\n");
	p += sprintf(p, "<audioDscp >\r\n");
	p += sprintf(p, "<Min>0</Min>\r\n");
	p += sprintf(p, "<Max>63</Max>\r\n");
	p += sprintf(p, "</audioDscp >\r\n");
	p += sprintf(p, "<flagType opt=\"videoAudio\"/>\r\n");
	p += sprintf(p, "</QoS>\r\n");
	p += sprintf(p, "<HTTPS>\r\n");
	p += sprintf(p, "<Certificate>\r\n");
	p += sprintf(p, "<certtype opt=\"CA,Certificate,privateKey\"/>\r\n");
	p += sprintf(p, "<fileType opt=\"PEM,PFX\"/>\r\n");
	p += sprintf(p, "<keyAlgorithm opt=\"RSA,DSA\"/>\r\n");
	p += sprintf(p, "<keyLen opt=\"512,1024,2048\"/>\r\n");
	p += sprintf(p, "<SignatureAlgorithm opt=\"MD5,RSA,DSA\"/>\r\n");
	p += sprintf(p, "</Certificate>\r\n");
	p += sprintf(p, "</HTTPS>\r\n");
	p += sprintf(p, "<NAS>\r\n");
	p += sprintf(p, "<serverTest>true</serverTest>\r\n");
	p += sprintf(p, "</NAS>\r\n");
	p += sprintf(p, "<FuzzyUpgrade>\r\n");
	p += sprintf(p, "<enabled>true</enabled>\r\n");
	p += sprintf(p, "</FuzzyUpgrade>\r\n");
	p += sprintf(p, "<CMS>\r\n");
	p += sprintf(p, "<cmsNo min=\"1\" max=\"1\"/>\r\n");
	p += sprintf(p, "<ehomeNo min=\"1\" max=\"1\"/>\r\n");
	p += sprintf(p, "<CmsParam>\r\n");
	p += sprintf(p, "<serverIpv4>true</serverIpv4>\r\n");
	p += sprintf(p, "<serverIpv6>true</serverIpv6>\r\n");
	p += sprintf(p, "<serverPort min=\"1024\" max=\"65535\"/>\r\n");
	p += sprintf(p, "<serverProtocolType opt=\"private,EHome\"/>\r\n");
	p += sprintf(p, "<deviceStatus opt=\"offline,online\"/>\r\n");
	p += sprintf(p, "<deviceIdLength min=\"1\" max=\"32\"/>\r\n");
	p += sprintf(p, "<platformEhomeVersionLength attri=\"readonly\" min=\"1\" max=\"32\"/>\r\n");
	p += sprintf(p, "<mutexAbility opt=\"gbt28181\"/>\r\n");
	p += sprintf(p, "</CmsParam>\r\n");
	p += sprintf(p, "</CMS>\r\n");
	p += sprintf(p, "<NetCfg>\r\n");
	p += sprintf(p, "<Ethernet1>\r\n");
	p += sprintf(p, "<IPAddrType opt=\"IPV4,IPV6\"/>\r\n");
	p += sprintf(p, "<IPAddrMaskType opt=\"IPV4,IPV6\"/>\r\n");
	p += sprintf(p, "<netInterface opt=\"10MBase-T,10MBase-T_FullDuplex,100MBase-TX,100M_FullDuplex,Adaptive\"/>\r\n");
	p += sprintf(p, "<mtu min=\"1280\" max=\"1500\"/>\r\n");
	p += sprintf(p, "<macAddrLen min=\"0\" max=\"65535\"/>\r\n");
	p += sprintf(p, "</Ethernet1>\r\n");
	p += sprintf(p, "<privateMulticastDiscovery opt=\"enable,disable\"/>\r\n");
	p += sprintf(p, "<onvifMulticastDiscovery opt=\"enable,disable\"/>\r\n");
	p += sprintf(p, "</NetCfg>\r\n");
	p += sprintf(p, "</NetAppAbility>\r\n");

	return 0;
}

static int netClientSetVideoPicAbility(char *p)
{
	p = p+sizeof(NETRET_HEADER);
    p += sprintf(p, "<VideoPicAbility version=\"2.0\">\r\n");
    p += sprintf(p, "<channelNO>1</channelNO>\r\n");
    p += sprintf(p, "<OSD>\r\n");
    p += sprintf(p, "<ChannelName>\r\n");
    p += sprintf(p, "<enabled>true</enabled>\r\n");
    p += sprintf(p, "</ChannelName>\r\n");
    p += sprintf(p, "<Week>\r\n");
    p += sprintf(p, "<enabled>true</enabled>\r\n");
    p += sprintf(p, "</Week>\r\n");
    //p += sprintf(p, "<OSDType opt=\"xxxx-xx-xxYMD,xx-xx-xxxxMDY,xx-xx-xxxxDMY,xxxxYxxMxxD,xxMxxDxxxxY,xxDxxMxxxxY,xxxx/xx/xxY/M/D,xx/xx/xxxxM/D/Y,xx/xx/xxxxD/M/Y\"/>\r\n");
    p += sprintf(p, "<OSDType opt=\"xxxx-xx-xxYMD,xx-xx-xxxxMDY,xxxx/xx/xxY/M/D,xx/xx/xxxxM/D/Y,xx-xx-xxxxDMY,xx/xx/xxxxD/M/Y\"/>\r\n");
	p += sprintf(p, "<OSDAttrib opt=\"1,2,3,4\"/>\r\n");
    p += sprintf(p, "<OSDHourType opt=\"24Hour,12Hour\"/>\r\n");
    p += sprintf(p, "<FontSize opt=\"16*16,32*32,48*48,64*64,adaptive\"/>\r\n");
    p += sprintf(p, "<OSDColorType opt=\"0,1\"/>\r\n");
    p += sprintf(p, "</OSD>\r\n");
    p += sprintf(p, "<MotionDetection>\r\n");
    p += sprintf(p, "<regionType opt=\"grid,area\"/>\r\n");
    p += sprintf(p, "<Grid>\r\n");
    p += sprintf(p, "<VideoFormatP>\r\n");
    p += sprintf(p, "<rowGranularity>18</rowGranularity>\r\n");
    p += sprintf(p, "<columnGranularity>22</columnGranularity>\r\n");
    p += sprintf(p, "</VideoFormatP>\r\n");
    p += sprintf(p, "<VideoFormatN>\r\n");
    p += sprintf(p, "<rowGranularity>15</rowGranularity>\r\n");
    p += sprintf(p, "<columnGranularity>22</columnGranularity>\r\n");
    p += sprintf(p, "</VideoFormatN>\r\n");
    p += sprintf(p, "</Grid>\r\n");
    p += sprintf(p, "<Area>\r\n");
    p += sprintf(p, "<areaNo min=\"1\" max=\"8\"/>\r\n");
    p += sprintf(p, "<switchDayNightSet opt=\"off,autoSwitch,scheduleSwitch\"/>\r\n");
    p += sprintf(p, "<Off>\r\n");
    p += sprintf(p, "<objectAreaProportion min=\"0\" max=\"100\"/>\r\n");
    p += sprintf(p, "<sensitivityLevel min=\"1\" max=\"100\"/>\r\n");
    p += sprintf(p, "</Off>\r\n");
    p += sprintf(p, "<AutoSwitch>\r\n");
    p += sprintf(p, "<supportType opt=\"day,night\"/>\r\n");
    p += sprintf(p, "<dayObjectAreaProportion min=\"0\" max=\"100\"/>\r\n");
    p += sprintf(p, "<daySensitivityLevel min=\"1\" max=\"100\"/>\r\n");
    p += sprintf(p, "<nightObjectAreaProportion min=\"0\" max=\"100\"/>\r\n");
    p += sprintf(p, "<nightSensitivityLevel min=\"1\" max=\"100\"/>\r\n");
    p += sprintf(p, "</AutoSwitch>\r\n");
    p += sprintf(p, "<ScheduleSwitch>\r\n");
    p += sprintf(p, "<supportType opt=\"day,night\"/>\r\n");
    p += sprintf(p, "<dayObjectAreaProportion min=\"0\" max=\"100\"/>\r\n");
    p += sprintf(p, "<daySensitivityLevel min=\"1\" max=\"100\"/>\r\n");
    p += sprintf(p, "<nightObjectAreaProportion min=\"0\" max=\"100\"/>\r\n");
    p += sprintf(p, "<nightSensitivityLevel min=\"1\" max=\"100\"/>\r\n");
    p += sprintf(p, "<TimeSchedule>\r\n");
    p += sprintf(p, "<beginTime opt=\"hour,min,sec,millisec\"/>\r\n");
    p += sprintf(p, "<endTime opt=\"hour,min,sec,millisec\"/>\r\n");
    p += sprintf(p, "</TimeSchedule>\r\n");
    p += sprintf(p, "</ScheduleSwitch>\r\n");
    p += sprintf(p, "</Area>\r\n");	
    p += sprintf(p, "<sensitivityLevel min=\"0\" max=\"100\"/>\r\n");
    p += sprintf(p, "<NormalSensitivity>\r\n");	
    p += sprintf(p, "<level min=\"0\" max=\"5\"/>\r\n");	
    p += sprintf(p, "<step>20</step>\r\n");	
    p += sprintf(p, "<offStatus>true</offStatus>\r\n");	
    p += sprintf(p, "</NormalSensitivity>\r\n");	
    p += sprintf(p, "<alarmTime>8</alarmTime>\r\n");
    p += sprintf(p, "<alarmHandleType opt=\"center,alarmout,picture,uploadftp\"/>\r\n");
    p += sprintf(p, "<displayMotion opt=\"true,false\"/>\r\n");
    p += sprintf(p, "</MotionDetection>\r\n");
    p += sprintf(p, "<HideDetection>\r\n");
    p += sprintf(p, "<HideAreaNum>1</HideAreaNum>\r\n");
    p += sprintf(p, "<HideArea>\r\n");
    p += sprintf(p, "<id>1</id>\r\n");
    p += sprintf(p, "<PAL>\r\n");
    p += sprintf(p, "<AreaX min=\"0\" max=\"704\"/>\r\n");
    p += sprintf(p, "<AreaY min=\"0\" max=\"576\"/>\r\n");
    p += sprintf(p, "</PAL>\r\n");
    p += sprintf(p, "<NTSC>\r\n");
    p += sprintf(p, "<AreaX min=\"0\" max=\"704\"/>\r\n");
    p += sprintf(p, "<AreaY min=\"0\" max=\"480\"/>\r\n");
    p += sprintf(p, "</NTSC>\r\n");
    p += sprintf(p, "</HideArea>\r\n");
    p += sprintf(p, "<sensitivity opt=\"none,low,middle,high\"/>\r\n");
    p += sprintf(p, "<alarmTime>8</alarmTime>\r\n");
    p += sprintf(p, "<alarmHandleType opt=\"center,alarmout,picture\"/>\r\n");
    p += sprintf(p, "</HideDetection>\r\n");
    p += sprintf(p, "<PrivacyMask>\r\n");
    p += sprintf(p, "<PrivacyMaskAreaNum>4</PrivacyMaskAreaNum>\r\n");
    p += sprintf(p, "<PrivacyMaskArea>\r\n");
    p += sprintf(p, "<id>1</id>\r\n");
    p += sprintf(p, "<PAL>\r\n");
    p += sprintf(p, "<AreaX min=\"0\" max=\"704\"/>\r\n");
    p += sprintf(p, "<AreaY min=\"0\" max=\"576\"/>\r\n");
    p += sprintf(p, "</PAL>\r\n");
    p += sprintf(p, "<NTSC>\r\n");
    p += sprintf(p, "<AreaX min=\"0\" max=\"704\"/>\r\n");
    p += sprintf(p, "<AreaY min=\"0\" max=\"480\"/>\r\n");
    p += sprintf(p, "</NTSC>\r\n");
    p += sprintf(p, "<id>2</id>\r\n");
    p += sprintf(p, "<PAL>\r\n");
    p += sprintf(p, "<AreaX min=\"0\" max=\"704\"/>\r\n");
    p += sprintf(p, "<AreaY min=\"0\" max=\"576\"/>\r\n");
    p += sprintf(p, "</PAL>\r\n");
    p += sprintf(p, "<NTSC>\r\n");
    p += sprintf(p, "<AreaX min=\"0\" max=\"704\"/>\r\n");
    p += sprintf(p, "<AreaY min=\"0\" max=\"480\"/>\r\n");
    p += sprintf(p, "</NTSC>\r\n");
    p += sprintf(p, "<id>3</id>\r\n");
    p += sprintf(p, "<PAL>\r\n");
    p += sprintf(p, "<AreaX min=\"0\" max=\"704\"/>\r\n");
    p += sprintf(p, "<AreaY min=\"0\" max=\"576\"/>\r\n");
    p += sprintf(p, "</PAL>\r\n");
    p += sprintf(p, "<NTSC>\r\n");
    p += sprintf(p, "<AreaX min=\"0\" max=\"704\"/>\r\n");
    p += sprintf(p, "<AreaY min=\"0\" max=\"480\"/>\r\n");
    p += sprintf(p, "</NTSC>\r\n");
    p += sprintf(p, "<id>4</id>\r\n");
    p += sprintf(p, "<PAL>\r\n");
    p += sprintf(p, "<AreaX min=\"0\" max=\"704\"/>\r\n");
    p += sprintf(p, "<AreaY min=\"0\" max=\"576\"/>\r\n");
    p += sprintf(p, "</PAL>\r\n");
    p += sprintf(p, "<NTSC>\r\n");
    p += sprintf(p, "<AreaX min=\"0\" max=\"704\"/>\r\n");
    p += sprintf(p, "<AreaY min=\"0\" max=\"480\"/>\r\n");
    p += sprintf(p, "</NTSC>\r\n");
    p += sprintf(p, "</PrivacyMaskArea>\r\n");
    p += sprintf(p, "</PrivacyMask>\r\n");
    p += sprintf(p, "<LogoOverlay>\r\n");
    p += sprintf(p, "<logoFormat opt=\"bmp\"/>\r\n");
    p += sprintf(p, "<logoWidth min=\"4\" max=\"128\"/>\r\n");
    p += sprintf(p, "<logoHeight min=\"4\" max=\"128\"/>\r\n");
    p += sprintf(p, "</LogoOverlay>\r\n");
    p += sprintf(p, "</VideoPicAbility>\r\n");
	
	return 0;
}

static int netClientSetGBT28181AccessAbility(char *p)
{
	p = p+sizeof(NETRET_HEADER);
    p += sprintf(p, "<GBT28181AccessAbility version=\"2.0\">\r\n");
    p += sprintf(p, "<GBT28181AccessCfg>\r\n");
    p += sprintf(p, "<enable opt=\"true,false\"/>\r\n");
    p += sprintf(p, "<localSipPort min=\"1024\" max=\"65535\"/>\r\n");
    p += sprintf(p, "<serverIDLen min=\"1\" max=\"64\"/>\r\n");
    p += sprintf(p, "<serverDomainLen min=\"1\" max=\"128\"/>\r\n");
    p += sprintf(p, "<serverSipAddressLen min=\"1\" max=\"128\"/>\r\n");
    p += sprintf(p, "<serverSipPort min=\"1024\" max=\"65535\"/>\r\n");
    p += sprintf(p, "<sipUserNameLen min=\"1\" max=\"64\"/>\r\n");
    p += sprintf(p, "<sipAuthenticateIDLen min=\"1\" max=\"64\"/>\r\n");
    p += sprintf(p, "<SipAuthenticatePasswdLen min=\"1\" max=\"32\"/>\r\n");
    p += sprintf(p, "<registerValid min=\"3600\" max=\"100000\"/>\r\n");
    p += sprintf(p, "<heartbeatInterval min=\"5\" max=\"255\"/>\r\n");
    p += sprintf(p, "<maxHeartbeatTimeOut min=\"3\" max=\"255\"/>\r\n");
    p += sprintf(p, "<streamType opt=\"mainstream,substream,stream3\"/>\r\n");
    p += sprintf(p, "<cameraNoCompressionIDLen min=\"13\" max=\"64\"/>\r\n");
    p += sprintf(p, "<alarmInputCompressionIDLen min=\"13\" max=\"64\"/>\r\n");
    p += sprintf(p, "<mutexAbility opt=\"ehome\"/>\r\n");
    p += sprintf(p, "<deviceStatus attri=\"readonly\" opt=\"offline,online\"/>\r\n");
    p += sprintf(p, "</GBT28181AccessCfg>\r\n");
    p += sprintf(p, "</GBT28181AccessAbility>\r\n");

	return 0;
}

static int netClientSetEventAbility(char *p,UINT8 ProtocolType)
{
	p = p+sizeof(NETRET_HEADER);
	p += sprintf(p, "<EventAbility version=\"2.0\">\r\n");
    p += sprintf(p, "<channelNO>1</channelNO>\r\n");
    p += sprintf(p, "<ExceptionAlarm>\r\n");
    p += sprintf(p, "<exceptionType opt=\"diskFull,diskError,nicBroken,ipConflict,illAccess\"/>\r\n");
    p += sprintf(p, "<alarmHandleType opt=\"center,alarmout,picture\"/>\r\n");
    p += sprintf(p, "<DetailedExceptionAlarm>\r\n");
    p += sprintf(p, "<DiskFull>\r\n");
    p += sprintf(p, "<alarmHandleType opt=\"center,alarmout,picture\"/>\r\n");
    p += sprintf(p, "</DiskFull>\r\n");
    p += sprintf(p, "<DiskError>\r\n");
    p += sprintf(p, "<alarmHandleType opt=\"center,alarmout,picture\"/>\r\n");
    p += sprintf(p, "</DiskError>\r\n");
    p += sprintf(p, "<NicBroken>\r\n");
    p += sprintf(p, "<alarmHandleType opt=\"alarmout\"/>\r\n");
    p += sprintf(p, "</NicBroken>\r\n");
    p += sprintf(p, "<IPConflict>\r\n");
    p += sprintf(p, "<alarmHandleType opt=\"alarmout\"/>\r\n");
    p += sprintf(p, "</IPConflict>\r\n");
    p += sprintf(p, "<IllAccess>\r\n");
    p += sprintf(p, "<alarmHandleType opt=\"center,alarmout,picture\"/>\r\n");
    p += sprintf(p, "</IllAccess>\r\n");
    p += sprintf(p, "</DetailedExceptionAlarm>\r\n");	
    p += sprintf(p, "</ExceptionAlarm>\r\n");
    p += sprintf(p, "<AlarmIn>\r\n");
    p += sprintf(p, "<alarmTime>8</alarmTime>\r\n");
    p += sprintf(p, "<alarmHandleType opt=\"center,alarmout,picture,uploadftp\"/>\r\n");
    p += sprintf(p, "<notSupportPTZLinkage>true</notSupportPTZLinkage>\r\n");
    p += sprintf(p, "</AlarmIn>\r\n");
    p += sprintf(p, "<AlarmOut>\r\n");
    p += sprintf(p, "<alarmTime>8</alarmTime>\r\n");
    p += sprintf(p, "<pulseDuration opt=\"5,10,30,60,120,300,600,manual\"/>\r\n");
    p += sprintf(p, "</AlarmOut>\r\n");

	if(ProtocolType == CLIENT_SDK_VERSION)
	{
	    p += sprintf(p, "<FaceDetection>\r\n");
	    p += sprintf(p, "<detectFaceEnable opt=\"true,false\"/>\r\n");
	    p += sprintf(p, "<detectFaceSensitive min=\"1\" max=\"5\"/>\r\n");
	    p += sprintf(p, "<alarmTime>8</alarmTime>\r\n");
	    p += sprintf(p, "<alarmHandleType opt=\"center,picture,alarmout,uploadftp\"/>\r\n");
	    p += sprintf(p, "<triggerRecord>true</triggerRecord>\r\n");
	    p += sprintf(p, "</FaceDetection>\r\n");
	    p += sprintf(p, "<VoiceDetection>\r\n");
	    p += sprintf(p, "<enable opt=\"true,false\"/>\r\n");
	    p += sprintf(p, "<Abnormal>\r\n");
	    p += sprintf(p, "<sensitivityLevel min=\"1\" max=\"100\"/>\r\n");
	    p += sprintf(p, "<audioMode opt=\"all\"/>\r\n");
	    p += sprintf(p, "<enable  opt=\"true,false\"/>\r\n");
	    p += sprintf(p, "<threshold min=\"1\" max=\"100\"/>\r\n");
	    p += sprintf(p, "</Abnormal>\r\n");
	    p += sprintf(p, "<alarmTime>8</alarmTime>\r\n");
	    p += sprintf(p, "<alarmHandleType opt=\"center,alarmout,picture\"/>\r\n");
	    p += sprintf(p, "<audioSteepDrop>\r\n");
	    p += sprintf(p, "<enable opt=\"true,false\"/>\r\n");
	    p += sprintf(p, "<sensitivityLevel min=\"1\" max=\"100\"/>\r\n");
	    p += sprintf(p, "</audioSteepDrop>\r\n");
	    p += sprintf(p, "</VoiceDetection>\r\n");
	    p += sprintf(p, "<TraversingVirtualPlane>\r\n");
	    p += sprintf(p, "<enable opt=\"true,false\"/>\r\n");
	    p += sprintf(p, "<enableDualVca opt=\"true,false\"/>\r\n");
	    p += sprintf(p, "<alertlineNum>4</alertlineNum>\r\n");
	    p += sprintf(p, "<AlertLine>\r\n");
	    p += sprintf(p, "<id>1</id>\r\n");
	    p += sprintf(p, "<crossDirection opt=\"bothway,leftToRight,rightToLeft\"/>\r\n");
	    p += sprintf(p, "<sensitivityLevel min=\"1\" max=\"100\"/>\r\n");
	    p += sprintf(p, "</AlertLine>\r\n");
	    p += sprintf(p, "<AlertLine>\r\n");
	    p += sprintf(p, "<id>2</id>\r\n");
	    p += sprintf(p, "<crossDirection opt=\"bothway,leftToRight,rightToLeft\"/>\r\n");
	    p += sprintf(p, "<sensitivityLevel min=\"1\" max=\"100\"/>\r\n");
	    p += sprintf(p, "</AlertLine>\r\n");
	    p += sprintf(p, "<AlertLine>\r\n");
	    p += sprintf(p, "<id>3</id>\r\n");
	    p += sprintf(p, "<crossDirection opt=\"bothway,leftToRight,rightToLeft\"/>\r\n");
	    p += sprintf(p, "<sensitivityLevel min=\"1\" max=\"100\"/>\r\n");
	    p += sprintf(p, "</AlertLine>\r\n");
	    p += sprintf(p, "<AlertLine>\r\n");
	    p += sprintf(p, "<id>4</id>\r\n");
	    p += sprintf(p, "<crossDirection opt=\"bothway,leftToRight,rightToLeft\"/>\r\n");
	    p += sprintf(p, "<sensitivityLevel min=\"1\" max=\"100\"/>\r\n");
	    p += sprintf(p, "</AlertLine>\r\n");
	    p += sprintf(p, "<alarmTime>8</alarmTime>\r\n");
	    p += sprintf(p, "<alarmHandleType opt=\"center,alarmout,picture,uploadftp\"/>\r\n");
	    p += sprintf(p, "</TraversingVirtualPlane>\r\n");
	    p += sprintf(p, "<FieldDetection>\r\n");
	    p += sprintf(p, "<enable opt=\"true,false\"/>\r\n");
	    p += sprintf(p, "<enableDualVca opt=\"true,false\"/>\r\n");
	    p += sprintf(p, "<intrusiongionNum>4</intrusiongionNum>\r\n");
	    p += sprintf(p, "<Intrusiongion>\r\n");
	    p += sprintf(p, "<id>1</id>\r\n");
	    p += sprintf(p, "<regionNum min=\"4\" max=\"4\"/>\r\n");
	    p += sprintf(p, "<duration min=\"0\" max=\"10\"/>\r\n");
	    p += sprintf(p, "<sensitivityLevel  min=\"1\" max=\"100\"/>\r\n");
	    p += sprintf(p, "<rate min=\"1\" max=\"100\"/>\r\n");
	    p += sprintf(p, "</Intrusiongion>\r\n");
	    p += sprintf(p, "<Intrusiongion>\r\n");
	    p += sprintf(p, "<id>2</id>\r\n");
	    p += sprintf(p, "<regionNum min=\"4\" max=\"4\"/>\r\n");
	    p += sprintf(p, "<duration min=\"0\" max=\"10\"/>\r\n");
	    p += sprintf(p, "<sensitivityLevel  min=\"1\" max=\"100\"/>\r\n");
	    p += sprintf(p, "<rate min=\"1\" max=\"100\"/>\r\n");
	    p += sprintf(p, "</Intrusiongion>\r\n");
	    p += sprintf(p, "<Intrusiongion>\r\n");
	    p += sprintf(p, "<id>3</id>\r\n");
	    p += sprintf(p, "<regionNum min=\"4\" max=\"4\"/>\r\n");
	    p += sprintf(p, "<duration min=\"0\" max=\"10\"/>\r\n");
	    p += sprintf(p, "<sensitivityLevel  min=\"1\" max=\"100\"/>\r\n");
	    p += sprintf(p, "<rate min=\"1\" max=\"100\"/>\r\n");
	    p += sprintf(p, "</Intrusiongion>\r\n");
	    p += sprintf(p, "<Intrusiongion>\r\n");
	    p += sprintf(p, "<id>4</id>\r\n");
	    p += sprintf(p, "<regionNum min=\"4\" max=\"4\"/>\r\n");
	    p += sprintf(p, "<duration min=\"0\" max=\"10\"/>\r\n");
	    p += sprintf(p, "<sensitivityLevel  min=\"1\" max=\"100\"/>\r\n");
	    p += sprintf(p, "<rate min=\"1\" max=\"100\"/>\r\n");
	    p += sprintf(p, "</Intrusiongion>\r\n");
	    p += sprintf(p, "<alarmTime>8</alarmTime>\r\n");
	    p += sprintf(p, "<alarmHandleType opt=\"center,alarmout,picture,uploadftp\"/>\r\n");
	    p += sprintf(p, "</FieldDetection>\r\n");
	    p += sprintf(p, "<DefousDetection>\r\n");
	    p += sprintf(p, "<enable opt=\"true,false\"/>\r\n");
	    p += sprintf(p, "<alarmHandleType opt=\"center,alarmout,picture\"/>\r\n");
	    p += sprintf(p, "<sensitivityLevel min=\"1\" max=\"100\"/>\r\n");
	    p += sprintf(p, "</DefousDetection>\r\n");
	    p += sprintf(p, "<SceneChangeDetection>\r\n");
	    p += sprintf(p, "<enable opt=\"true,false\"/>\r\n");
	    p += sprintf(p, "<sensitiveLevel  min=\"1\" max=\"100\"/>\r\n");
	    p += sprintf(p, "<alarmTime>8</alarmTime>\r\n");
	    p += sprintf(p, "<alarmHandleType opt=\"center,alarmout,picture,uploadftp\"/>\r\n");
	    p += sprintf(p, "<sceneChangeDetectionRecord>true</sceneChangeDetectionRecord>\r\n");
	    p += sprintf(p, "</SceneChangeDetection>\r\n");
	}
    p += sprintf(p, "</EventAbility>\r\n");
	
	return 0;
}

    static int netClientSetHardDiskAbility(char *p)
    {
    	p = p+sizeof(NETRET_HEADER);
        p += sprintf(p, "<HardDiskAbility version=\"2.0\">\r\n");
        p += sprintf(p, "<hdAttribute opt=\"2\"/>\r\n");
        p += sprintf(p, "</HardDiskAbility>\r\n");

    	return 0;
    }

static int netClientSetPTZAbility(char *p)
{
	p = p+sizeof(NETRET_HEADER);
    p += sprintf(p, "<PTZAbility  version=\"2.0\">\r\n");
    p += sprintf(p, "<channelNO>1</channelNO>\r\n");
    p += sprintf(p, "<PTZControl>\r\n");
    p += sprintf(p, "<controlType opt=\"light,wiper,fan,heater,aux1,aux2,zoomIn,zoomOut,focusNear,focusFar,irisOpen,irisClose,ttiltUp,tiltDown,panLeft,panRight,upLeft,upRight,downLeft,downRight,panAuto,panCircle\"/>\r\n");
    p += sprintf(p, "</PTZControl>\r\n");
    p += sprintf(p, "<Patrol>\r\n");
    p += sprintf(p, "<patrolNum min=\"1\" max=\"8\"/>\r\n");
    p += sprintf(p, "<presetNum min=\"1\" max=\"32\"/>\r\n");
    p += sprintf(p, "<dwellTime min=\"0\" max=\"120\"/>\r\n");
    p += sprintf(p, "<speed min=\"1\" max=\"40\"/>\r\n");
    p += sprintf(p, "</Patrol>\r\n");
    p += sprintf(p, "<Preset>\r\n");
    p += sprintf(p, "<presetNum min=\"1\" max=\"256\"/>\r\n");
    p += sprintf(p, "</Preset>\r\n");
    p += sprintf(p, "</PTZAbility>\r\n");
	
	return 0;
}

static int netClientSetRecordAbility(char *p)
{
	p = p+sizeof(NETRET_HEADER);
    p += sprintf(p, "<RecordAbility version=\"2.0\">\r\n");
    p += sprintf(p, "<RecordScheduleNum>8</RecordScheduleNum>\r\n");
    p += sprintf(p, "<sceneChangeDetectionRecord>true</sceneChangeDetectionRecord>\r\n");
    p += sprintf(p, "<faceDetectionRecord>true</faceDetectionRecord>\r\n");
    p += sprintf(p, "<allAlarmTypeRecord>true</allAlarmTypeRecord>\r\n");
    p += sprintf(p, "<ANR>true</ANR>\r\n");
    p += sprintf(p, "</RecordAbility>\r\n");

	return 0;
}

static int netClientSetROIAbility(char *p)
{
	p = p+sizeof(NETRET_HEADER);
	p += sprintf(p, "<ROIAbility version=\"2.0\">\r\n");
    p += sprintf(p, "<channelNO>1</channelNO>\r\n");
    p += sprintf(p, "<ROIRectCfg>\r\n");
    p += sprintf(p, "<MainStream>\r\n");
    p += sprintf(p, "<FixROIRectCfg>\r\n");
    p += sprintf(p, "<ROISupportEncodeType opt=\"standardh264\"/>\r\n");
    p += sprintf(p, "<singleScreenFixROITotalNum>3</singleScreenFixROITotalNum>\r\n");
    p += sprintf(p, "<singleScreenFixROIID min=\"1\" max=\"3\"/>\r\n");
    p += sprintf(p, "<fixROINameLength min=\"1\" max=\"32\"/>\r\n");
    p += sprintf(p, "<fixROIDetectEnable opt=\"true,false\"/>\r\n");
    p += sprintf(p, "<ROIImageQualityLevel min=\"1\" max=\"6\"/>\r\n");
    p += sprintf(p, "</FixROIRectCfg>\r\n");
    p += sprintf(p, "<TrackROIRectCfg>\r\n");
    p += sprintf(p, "<ROISupportEncodeType opt=\"standardh264\"/>\r\n");
    p += sprintf(p, "<singleScreenTrackROITotalNum>1</singleScreenTrackROITotalNum>\r\n");
    p += sprintf(p, "<singleScreenTrackROIID min=\"1\" max=\"1\"/>\r\n");
    p += sprintf(p, "<trackROIDetectEnable opt=\"true,false\"/>\r\n");
    p += sprintf(p, "<ROIImageQualityLevel min=\"1\" max=\"6\"/>\r\n");
    p += sprintf(p, "<trackROIModeType opt=\"faceDetect\"/>\r\n");
    p += sprintf(p, "</TrackROIRectCfg>\r\n");
    p += sprintf(p, "</MainStream>\r\n");
    p += sprintf(p, "<SubStream>\r\n");
    p += sprintf(p, "<FixROIRectCfg>\r\n");
    p += sprintf(p, "<ROISupportEncodeType opt=\"standardh264\"/>\r\n");
    p += sprintf(p, "<singleScreenFixROITotalNum>3</singleScreenFixROITotalNum>\r\n");
    p += sprintf(p, "<singleScreenFixROIID min=\"1\" max=\"2\"/>\r\n");
    p += sprintf(p, "<fixROINameLength min=\"1\" max=\"32\"/>\r\n");
    p += sprintf(p, "<fixROIDetectEnable opt=\"true,false\"/>\r\n");
    p += sprintf(p, "<ROIImageQualityLevel min=\"1\" max=\"6\"/>\r\n");
    p += sprintf(p, "</FixROIRectCfg>\r\n");
    p += sprintf(p, "<TrackROIRectCfg>\r\n");
    p += sprintf(p, "<ROISupportEncodeType opt=\"standardh264\"/>\r\n");
    p += sprintf(p, "<singleScreenTrackROITotalNum>1</singleScreenTrackROITotalNum>\r\n");
    p += sprintf(p, "<singleScreenTrackROIID min=\"1\" max=\"1\"/>\r\n");
    p += sprintf(p, "<trackROIDetectEnable opt=\"true,false\"/>\r\n");
    p += sprintf(p, "<ROIImageQualityLevel min=\"1\" max=\"6\"/>\r\n");
    p += sprintf(p, "<trackROIModeType opt=\"faceDetect\"/>\r\n");
    p += sprintf(p, "</TrackROIRectCfg>\r\n");
    p += sprintf(p, "</SubStream>\r\n");
    p += sprintf(p, "<Stream3>\r\n");
    p += sprintf(p, "<FixROIRectCfg>\r\n");
    p += sprintf(p, "<ROISupportEncodeType opt=\"standardh264\"/>\r\n");
    p += sprintf(p, "<singleScreenFixROITotalNum>3</singleScreenFixROITotalNum>\r\n");
    p += sprintf(p, "<singleScreenFixROIID min=\"1\" max=\"2\"/>\r\n");
    p += sprintf(p, "<fixROINameLength min=\"1\" max=\"32\"/>\r\n");
    p += sprintf(p, "<fixROIDetectEnable opt=\"true,false\"/>\r\n");
    p += sprintf(p, "<ROIImageQualityLevel min=\"1\" max=\"6\"/>\r\n");
    p += sprintf(p, "</FixROIRectCfg>\r\n");
    p += sprintf(p, "<TrackROIRectCfg>\r\n");
    p += sprintf(p, "<ROISupportEncodeType opt=\"standardh264\"/>\r\n");
    p += sprintf(p, "<singleScreenTrackROITotalNum>1</singleScreenTrackROITotalNum>\r\n");
    p += sprintf(p, "<singleScreenTrackROIID min=\"1\" max=\"1\"/>\r\n");
    p += sprintf(p, "<trackROIDetectEnable opt=\"true,false\"/>\r\n");
    p += sprintf(p, "<ROIImageQualityLevel min=\"1\" max=\"6\"/>\r\n");
    p += sprintf(p, "<trackROIModeType opt=\"faceDetect\"/>\r\n");
    p += sprintf(p, "</TrackROIRectCfg>\r\n");
    p += sprintf(p, "</Stream3>\r\n");
    p += sprintf(p, "</ROIRectCfg>\r\n");
    p += sprintf(p, "</ROIAbility>\r\n");

	return 0;
}

static int netClientSetSecurityAbility(char *p)
{
	p = p+sizeof(NETRET_HEADER);
	p += sprintf(p, "<SecurityAbility version=\"2.0\">\r\n");
	p += sprintf(p, "<channelNO>1</channelNO>\r\n");
	p += sprintf(p, "<SecurityCfgParam>\r\n");
	p += sprintf(p, "<securityLevel opt=\"0-close,1-open,2-custom\"/>\r\n");
	p += sprintf(p, "<commadCheckLevel opt=\"0-close,1-open\"/>\r\n");
	p += sprintf(p, "<loginLevel opt=\"0-nolimit,1-MD5\"/>\r\n");
	p += sprintf(p, "<sshServer opt=\"0-enable,1-disable\"/>\r\n");
	p += sprintf(p, "<webAuthentication opt=\"0-digest,1-basic\"/>\r\n");
	p += sprintf(p, "<rtspAuthentication opt=\"0-disable,1-basic\"/>\r\n");
	p += sprintf(p, "<telnetServer opt=\"0-disable,1-enable\"/>\r\n");
	p += sprintf(p, "</SecurityCfgParam>\r\n");
	p += sprintf(p, "</SecurityAbility>\r\n");

	return 0;
}

static int GetDevAbilityCmdType(char buf[100],char AbilityType[50])
{
	char *p1 = buf+1;
	if(p1 == NULL)
	{
		HPRINT_ERR("CmdType err\n");
		return -1;
	}
	char *p2 = strstr(p1," ");
	if(p2 == NULL)
	{
		HPRINT_ERR("CmdType err\n");
		return -1;
	}
	memcpy(AbilityType,p1,p2-p1);
		
	return 0;
}

static int netClientSetAudioVideoCompressInfo(char *p)
{
	p = p+sizeof(NETRET_HEADER);
	p += sprintf(p, "<AudioVideoCompressInfo version=\"2.0\">\r\n");
	p += sprintf(p, "<AudioCompressInfo>\r\n");
	p += sprintf(p, "<Audio>\r\n");
	p += sprintf(p, "<ChannelList>\r\n");
	p += sprintf(p, "<ChannelEntry>\r\n");
	p += sprintf(p, "<ChannelNumber>1</ChannelNumber>\r\n");
	p += sprintf(p, "<MainAudioEncodeType>\r\n");
	p += sprintf(p, "<Range>1,2,6</Range>\r\n");
	p += sprintf(p, "</MainAudioEncodeType>\r\n");
	p += sprintf(p, "<SubAudioEncodeType>\r\n");
	p += sprintf(p, "<Range>1,2,6</Range>\r\n");
	p += sprintf(p, "</SubAudioEncodeType>\r\n");
	p += sprintf(p, "<AudioInType>\r\n");
	p += sprintf(p, "<Range>0</Range>\r\n");
	p += sprintf(p, "</AudioInType>\r\n");
	p += sprintf(p, "<AudioInVolume>\r\n");
	p += sprintf(p, "<Min>0</Min>\r\n");
	p += sprintf(p, "<Max>100</Max>\r\n");
	p += sprintf(p, "</AudioInVolume>\r\n");
	p += sprintf(p, "</ChannelEntry>\r\n");
	p += sprintf(p, "</ChannelList>\r\n");
	p += sprintf(p, "</Audio>\r\n");
	p += sprintf(p, "<VoiceTalk>\r\n");
	p += sprintf(p, "<ChannelList>\r\n");
	p += sprintf(p, "<ChannelEntry>\r\n");
	p += sprintf(p, "<ChannelNumber>1</ChannelNumber>\r\n");
	p += sprintf(p, "<VoiceTalkEncodeType>\r\n");
	p += sprintf(p, "<Range>1,2,6</Range>\r\n");
	p += sprintf(p, "</VoiceTalkEncodeType>\r\n");
	p += sprintf(p, "<VoiceTalkInType>\r\n");
	p += sprintf(p, "<Range>0</Range>\r\n");
	p += sprintf(p, "</VoiceTalkInType>\r\n");
	p += sprintf(p, "</ChannelEntry>\r\n");
	p += sprintf(p, "</ChannelList>\r\n");
	p += sprintf(p, "</VoiceTalk>\r\n");
	p += sprintf(p, "</AudioCompressInfo>\r\n");
	p += sprintf(p, "<VideoCompressInfo>\r\n");
	p += sprintf(p, "<ChannelList>\r\n");
	p += sprintf(p, "<ChannelEntry>\r\n");
	p += sprintf(p, "<ChannelNumber>1</ChannelNumber>\r\n");

	p += sprintf(p, "<MainChannel>\r\n");
	p += sprintf(p, "<VideoEncodeType>\r\n");

#if 0
	//DMS_NET_DEVICE_INFO DeviceCfg;
	//memset(&DeviceCfg,0,sizeof(DeviceCfg));
	//dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_GET_DEVICECFG,0,&DeviceCfg,sizeof(DeviceCfg));
	//if(DeviceCfg.dwServerCPUType == DMS_CPU_3516A || DeviceCfg.dwServerCPUType == DMS_CPU_3516D)	
	//	p += sprintf(p, "<Range>1,10</Range>\r\n"); //此值决定是否显示H265的视频编码方式
	//else
		p += sprintf(p, "<Range>1,2</Range>\r\n");
#else

	p += sprintf(p, "<Range>1,10</Range>\r\n"); //此值决定是否显示H265的视频编码方式

#endif
	p += sprintf(p, "</VideoEncodeType>\r\n");
	p += sprintf(p, "<VideoEncodeEfficiency>\r\n");
	p += sprintf(p, "<Range>0,1,2</Range>\r\n");
	//p += sprintf(p, "<Range>0,1</Range>\r\n");

	p += sprintf(p, "<suportEncodeType opt=\"1,10\"/>\r\n");
	p += sprintf(p, "<EncodeTypeList>\r\n");
	p += sprintf(p, "<EncodeType>\r\n");
	p += sprintf(p, "<index>10</index>\r\n");
	p += sprintf(p, "<Range>1</Range>\r\n");
	p += sprintf(p, "</EncodeType>\r\n");
	p += sprintf(p, "</EncodeTypeList>\r\n");

	p += sprintf(p, "</VideoEncodeEfficiency>\r\n");
	p += sprintf(p, "<VideoResolutionList>\r\n");

    
    int i=0;
    #if 1
    //必须先填充video2Arr...
    cms_video_get_def_map();
    
    int ret = 0;
    int MAX_STREAMNUM = 2;
    DMS_SIZE    videoSize[MAX_STREAMNUM][10];
    memset(videoSize, 0, sizeof(videoSize));
	//Video Size
	for(i = 0; i < MAX_STREAMNUM; i++)
	{
		ret = cms_get_resolution(videoSize[i], i, 10);
		if(ret != 0)
		{
			HPRINT_ERR("CMS: fail to get resolution options.");
			return -1;
		}
	}
    
    int j;
    for(i = 0; i < MAX_STREAMNUM; i++)
    {
        for(j = 0; j < 10; j++)
        {
            if (videoSize[i][j].nWidth != 0)
                HPRINT_INFO("stream%d -- index:%d, %d * %d", i, j, 
                    videoSize[i][j].nWidth, videoSize[i][j].nHeight);
        }
    }
    
    #else
	DMS_NET_SUPPORT_STREAM_FMT FmtInfo;
	memset(&FmtInfo,0,sizeof(FmtInfo));
	dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_GET_SUPPORT_STREAM_FMT,0,&FmtInfo,sizeof(FmtInfo));
    #endif

    int Index, BitrateMin, BitrateMax;
    char Name[32], VideoFrameRate[128];
    i = 0;
	while(videoSize[0][i].nWidth != 0 && (i < 10))
	{
        Index = RESOLOUTION_SIZE_NR;
		if(videoSize[0][i].nWidth == 2592 && videoSize[0][i].nHeight == 1944)
		{
            Index = RESOLOUTION_SIZE_5M_2;
            strcpy(Name, "2592x1944");
            strcpy(VideoFrameRate, "0,1,2,3,4,5,6,7,8,9,10,11,14,12,15,13,16,17,22");
            BitrateMin = 32;
            BitrateMax = 16384;
		}
		else if(videoSize[0][i].nWidth == 2592 && videoSize[0][i].nHeight == 1520)
		{
            Index = RESOLOUTION_SIZE_4M;
            strcpy(Name, "2592x1520");
            strcpy(VideoFrameRate, "0,1,2,3,4,5,6,7,8,9,10,11,14,12,15,13,16,17,22");
            BitrateMin = 32;
            BitrateMax = 16384;
		}
		else if(videoSize[0][i].nWidth == 2560 && videoSize[0][i].nHeight == 1440)
		{
            Index = RESOLOUTION_SIZE_4M;
            strcpy(Name, "2560x1440");
            strcpy(VideoFrameRate, "0,1,2,3,4,5,6,7,8,9,10,11,14,12,15,13,16,17,22");
            BitrateMin = 32;
            BitrateMax = 16384;
		}
		else if(videoSize[0][i].nWidth == 2048 && videoSize[0][i].nHeight == 1520)
		{
            Index = RESOLOUTION_SIZE_3M_1;
            strcpy(Name, "2048x1520");
            strcpy(VideoFrameRate, "0,1,2,3,4,5,6,7,8,9,10,11,14,12,15,13,16,17,22");
            BitrateMin = 32;
            BitrateMax = 16384;
		}
		else if(videoSize[0][i].nWidth == 1920 && videoSize[0][i].nHeight == 1080)
		{
            Index = RESOLOUTION_SIZE_1080p;
            strcpy(Name, "HD1080P");
            strcpy(VideoFrameRate, "5,6,7,8,9,10,11,12,13,14,15,16,17");
            BitrateMin = 32;
            BitrateMax = 8192;
		}
		else if(videoSize[0][i].nWidth == 1280 && videoSize[0][i].nHeight == 960)
		{
            Index = RESOLOUTION_SIZE_1_3M;
            strcpy(Name, "XVGA");
            strcpy(VideoFrameRate, "5,6,7,8,9,10,11,12,13,14,15,16,17");
            BitrateMin = 32;
            BitrateMax = 8192;
		}
		else if(videoSize[0][i].nWidth == 1280 && videoSize[0][i].nHeight == 720)
		{
            Index = RESOLOUTION_SIZE_720p;
            strcpy(Name, "720P");
            strcpy(VideoFrameRate, "5,6,7,8,9,10,11,12,13,14,15,16,17");
            BitrateMin = 32;
            BitrateMax = 8192;
		}					
		else if(videoSize[0][i].nWidth == 800 && videoSize[0][i].nHeight == 600)
		{
            Index = RESOLOUTION_SIZE_SVGA;
            strcpy(Name, "SVGA");
            strcpy(VideoFrameRate, "5,6,7,8,9,10,11,12,13,14,15,16,17");
            BitrateMin = 32;
            BitrateMax = 8192;
		}
		else if(videoSize[0][i].nWidth == 720 && videoSize[0][i].nHeight == 576)
		{
            Index = RESOLOUTION_SIZE_D1;
            strcpy(Name, "D1");
            strcpy(VideoFrameRate, "5,6,7,8,9,10,11,12,13,14,15,16,17");
            BitrateMin = 32;
            BitrateMax = 8192;
		}
		else if(videoSize[0][i].nWidth == 640 && videoSize[0][i].nHeight == 480)
		{
            Index = RESOLOUTION_SIZE_VGA;
            strcpy(Name, "VGA");
            strcpy(VideoFrameRate, "5,6,7,8,9,10,11,12,13,14,15,16,17");
            BitrateMin = 32;
            BitrateMax = 8192;
		}
        if(RESOLOUTION_SIZE_NR != Index)
		{
			p += sprintf(p, "<VideoResolutionEntry>\r\n");
			p += sprintf(p, "<Index>%d</Index>\r\n", Index);
			p += sprintf(p, "<Name>%s</Name>\r\n", Name);
			p += sprintf(p, "<Resolution>%d*%d</Resolution>\r\n", videoSize[0][i].nWidth, videoSize[0][i].nHeight);
			p += sprintf(p, "<VideoFrameRate>%s</VideoFrameRate>\r\n", VideoFrameRate);
			p += sprintf(p, "<VideoBitrate>\r\n");
			p += sprintf(p, "<Min>%d</Min>\r\n", BitrateMin);
			p += sprintf(p, "<Max>%d</Max>\r\n", BitrateMax);
			p += sprintf(p, "</VideoBitrate>\r\n");
			p += sprintf(p, "</VideoResolutionEntry>\r\n");
		}
		i++;
	}	   

	p += sprintf(p, "</VideoResolutionList>\r\n");
	p += sprintf(p, "<IntervalBPFrame>\r\n");
	p += sprintf(p, "<Range>2</Range>\r\n");
	p += sprintf(p, "</IntervalBPFrame>\r\n");
	p += sprintf(p, "<EFrame>0</EFrame>\r\n");
	p += sprintf(p, "</MainChannel>\r\n");

	p += sprintf(p, "<SubChannelList>\r\n");
	p += sprintf(p, "<SubChannelEntry>\r\n");
	p += sprintf(p, "<index>1</index>\r\n");
	p += sprintf(p, "<VideoEncodeType>\r\n");
#if 0
//	if(DeviceCfg.dwServerCPUType == DMS_CPU_3516A || DeviceCfg.dwServerCPUType == DMS_CPU_3516D)	
//		p += sprintf(p, "<Range>1,10</Range>\r\n");
//	else
		p += sprintf(p, "<Range>1,2,7</Range>\r\n");
#else
	p += sprintf(p, "<Range>1,10</Range>\r\n");

#endif
	p += sprintf(p, "</VideoEncodeType>\r\n");
	p += sprintf(p, "<VideoEncodeEfficiency>\r\n");
	p += sprintf(p, "<Range>0,1</Range>\r\n");
	p += sprintf(p, "</VideoEncodeEfficiency>\r\n");
	p += sprintf(p, "<VideoResolutionList>\r\n");

	i=0;
	while(videoSize[1][i].nWidth != 0 && (i < 10))
	{
        Index = RESOLOUTION_SIZE_NR;
		if(videoSize[1][i].nWidth == 1920 && videoSize[1][i].nHeight == 1080)
		{
            Index = RESOLOUTION_SIZE_1080p;
            strcpy(Name, "HD1080P");
            strcpy(VideoFrameRate, "0,1,2,3,4,5,6,7,8,9,10,11,14,12,15,13,16,17,22");
            BitrateMin = 32;
            BitrateMax = 8192;
		}
		else if(videoSize[1][i].nWidth == 1280 && videoSize[1][i].nHeight == 960)
		{
            Index = RESOLOUTION_SIZE_1_3M;
            strcpy(Name, "XVGA");
            strcpy(VideoFrameRate, "5,6,7,8,9,10,11,12,13,14,15,16,17");
            BitrateMin = 32;
            BitrateMax = 8192;
		}
		if(videoSize[1][i].nWidth == 1280 && videoSize[1][i].nHeight == 720)
		{
            Index = RESOLOUTION_SIZE_720p;
            strcpy(Name, "720P");
            strcpy(VideoFrameRate, "5,6,7,8,9,10,11,12,13,14,15,16,17");
            BitrateMin = 32;
            BitrateMax = 8192;
		}					
		else if(videoSize[1][i].nWidth == 800 && videoSize[1][i].nHeight == 600)
		{
            Index = RESOLOUTION_SIZE_SVGA;
            strcpy(Name, "SVGA");
            strcpy(VideoFrameRate, "5,6,7,8,9,10,11,12,13,14,15,16,17");
            BitrateMin = 32;
            BitrateMax = 8192;
		}
		else if(videoSize[1][i].nWidth == 704 && videoSize[1][i].nHeight == 576)
		{
            Index = RESOLOUTION_SIZE_4CIF;
            strcpy(Name, "4CIF");
            strcpy(VideoFrameRate, "5,6,7,8,9,10,11,12,13,14,15,16,17");
            BitrateMin = 32;
            BitrateMax = 8192;
		}
		else if(videoSize[1][i].nWidth == 720 && videoSize[1][i].nHeight == 576)
		{
            Index = RESOLOUTION_SIZE_D1;
            strcpy(Name, "D1");
            strcpy(VideoFrameRate, "5,6,7,8,9,10,11,12,13,14,15,16,17");
            BitrateMin = 32;
            BitrateMax = 8192;
		}		
		else if(videoSize[1][i].nWidth == 640 && videoSize[1][i].nHeight == 480)
		{
            Index = RESOLOUTION_SIZE_VGA;
            strcpy(Name, "VGA");
            strcpy(VideoFrameRate, "5,6,7,8,9,10,11,12,13,14,15,16,17");
            BitrateMin = 32;
            BitrateMax = 8192;
		}
		else if(videoSize[1][i].nWidth == 640 && videoSize[1][i].nHeight == 360)
		{
            Index = RESOLOUTION_SIZE_HVGAW;
            strcpy(Name, "360P");
            strcpy(VideoFrameRate, "5,6,7,8,9,10,11,12,13,14,15,16,17");
            BitrateMin = 32;
            BitrateMax = 8192;
		}
		else if(videoSize[1][i].nWidth == 352 && videoSize[1][i].nHeight == 288)
		{
            Index = RESOLOUTION_SIZE_CIF;
            strcpy(Name, "CIF");
            strcpy(VideoFrameRate, "5,6,7,8,9,10,11,12,13,14,15,16,17");
            BitrateMin = 32;
            BitrateMax = 8192;
		}
		else if(videoSize[1][i].nWidth == 320 && videoSize[1][i].nHeight == 240)
		{
            Index = RESOLOUTION_SIZE_QVGA;
            strcpy(Name, "QVGA");
            strcpy(VideoFrameRate, "5,6,7,8,9,10,11,12,13,14,15,16,17");
            BitrateMin = 32;
            BitrateMax = 8192;
		}
        if(RESOLOUTION_SIZE_NR != Index)
		{
			p += sprintf(p, "<VideoResolutionEntry>\r\n");
			p += sprintf(p, "<Index>%d</Index>\r\n", Index);
			p += sprintf(p, "<Name>%s</Name>\r\n", Name);
			p += sprintf(p, "<Resolution>%d*%d</Resolution>\r\n", videoSize[1][i].nWidth, videoSize[1][i].nHeight);
			p += sprintf(p, "<VideoFrameRate>%s</VideoFrameRate>\r\n", VideoFrameRate);
			p += sprintf(p, "<VideoBitrate>\r\n");
			p += sprintf(p, "<Min>%d</Min>\r\n", BitrateMin);
			p += sprintf(p, "<Max>%d</Max>\r\n", BitrateMax);
			p += sprintf(p, "</VideoBitrate>\r\n");
			p += sprintf(p, "</VideoResolutionEntry>\r\n");
		}
		i++;
	}

	p += sprintf(p, "</VideoResolutionList>\r\n");
	p += sprintf(p, "<IntervalBPFrame>\r\n");
	p += sprintf(p, "<Range>2</Range>\r\n");
	p += sprintf(p, "</IntervalBPFrame>\r\n");
	p += sprintf(p, "<EFrame>0</EFrame>\r\n");
	p += sprintf(p, "</SubChannelEntry>\r\n");
	p += sprintf(p, "</SubChannelList>\r\n");
	p += sprintf(p, "</ChannelEntry>\r\n");
	p += sprintf(p, "</ChannelList>\r\n");
	p += sprintf(p, "</VideoCompressInfo>\r\n");
	p += sprintf(p, "</AudioVideoCompressInfo>\r\n");
	
	return 0;
}

int netClientGetDeviceCapacity(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	struct
	{
		NETCMD_HEADER		CmdHeader;
		NET_DEVABILITY_RET	Ability;
	}NetCmdHeader;
	NETRET_HEADER header;
	char sendbuf[8192];
	UINT32 retVal;
	
	memset(&NetCmdHeader,0,sizeof(NetCmdHeader));
	memset(&header,0,sizeof(header));
	memset(sendbuf,0,sizeof(sendbuf));
	
	memcpy((char *)&NetCmdHeader, recvbuff, sizeof(NetCmdHeader));
	retVal = verifyNetClientOperation(&NetCmdHeader.CmdHeader, pClientSockAddr, REMOTESHOWPARAMETER);
	if(retVal == NETRET_QUALIFIED)
	{
	    HPRINT_INFO("############# Ability.netCmd=%d #############\n", ntohl(NetCmdHeader.Ability.netCmd));
		if(0x01 == ntohl(NetCmdHeader.Ability.netCmd))
		{
			netClientSetBasicCapability(sendbuf);
		}
		else if(0x05 == ntohl(NetCmdHeader.Ability.netCmd))
		{
			netClientSetCameraPara_v1(sendbuf);
		}
		else if(0x08 == ntohl(NetCmdHeader.Ability.netCmd))
		{
			netClientSetAudioVideoCompressInfo(sendbuf);
		} 
		else if(0x09 == ntohl(NetCmdHeader.Ability.netCmd))
		{
			netClientSetCameraPara_v2(sendbuf);		
		}
		else if(0x0a == ntohl(NetCmdHeader.Ability.netCmd))
		{
			netClientSetAlarmAbility(sendbuf);
		}
		else if(0x0c == ntohl(NetCmdHeader.Ability.netCmd))
		{
			netClientSetUserAbility(sendbuf);
		}
		else if(0x0d == ntohl(NetCmdHeader.Ability.netCmd))
		{
			netClientSetNetAppAbility(sendbuf);
		}
	    else if(0x0e == ntohl(NetCmdHeader.Ability.netCmd))
	    {
	    	netClientSetVideoPicAbility(sendbuf);
	    }
	    else if(0x0f == ntohl(NetCmdHeader.Ability.netCmd))
	    {
	    	netClientSetJpegCaptureAbility(sendbuf);
	    }
		else if(0x10 == ntohl(NetCmdHeader.Ability.netCmd))
		{
			netClientSetSerialAbility(sendbuf);		
		}
	    else if(0x11 == ntohl(NetCmdHeader.Ability.netCmd))
		{
			char CmdType[50] = {0};
			if(strlen(NetCmdHeader.Ability.CmdType) != 0)
				GetDevAbilityCmdType(NetCmdHeader.Ability.CmdType,CmdType);

			if(strcmp(CmdType,"EventAbility") == 0)
			{
				netClientSetEventAbility(sendbuf,NetCmdHeader.CmdHeader.ifVer);
			}
			else if(strcmp(CmdType,"GBT28181AccessAbility") == 0)
			{
				netClientSetGBT28181AccessAbility(sendbuf);
			}
			else if(strcmp(CmdType,"HardDiskAbility") == 0)
			{
				netClientSetHardDiskAbility(sendbuf);
			}
			else if(strcmp(CmdType,"PTZAbility") == 0)
			{
				netClientSetPTZAbility(sendbuf);
			}	
			else if(strcmp(CmdType,"RecordAbility") == 0)
			{
				netClientSetRecordAbility(sendbuf);
			}	
			else if(strcmp(CmdType,"ROIAbility") == 0)
			{
				netClientSetROIAbility(sendbuf);
			}	
			else if(strcmp(CmdType,"SecurityAbility") == 0)
			{
				netClientSetSecurityAbility(sendbuf);
			}
			else
			{
				retVal = ABILITY_NOT_SUPPORT;
			}
	    }
		else
		{
			retVal = ABILITY_NOT_SUPPORT;
		}
	}	

	header.length = htonl(strlen(sendbuf+sizeof(header)) + sizeof(header));
	header.retVal = htonl(retVal);
	header.checkSum = htonl(checkByteSum((char *)&header.retVal, strlen(sendbuf+sizeof(header)) + sizeof(header) - 8));
	memcpy(sendbuf, &header, sizeof(header));

	return writen(connfd, sendbuf, ntohl(header.length));
}

int netClientCmd11300b(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	NETRET_HEADER header;
	bzero(&header, sizeof(header));

	header.length = htonl(sizeof(header));
	header.retVal = htonl(NETRET_NOT_SUPPORT);
	header.checkSum = htonl(checkByteSum((char *)&header.retVal, ntohl(header.length) - 8));

	return writen(connfd, &header, ntohl(header.length));
}

#define   _CMS_WIDTH   704
#define   _CMS_HEIGHT  576
static int _float_rate_to_data(float rate, int total)
{
    int ret = (int)(rate * total);
    return (ret >= total)? total:ret;
}

static float _data_to_float_rate(int data, int total)
{
    float ret = (float)data /total;
    return (ret >= 1.0)? 1.0:ret;
}

static BYTE _dateFormat_hik2goke(BYTE hikDateFormat)
{
    BYTE dateFormat = 0;
    if(0 == hikDateFormat)
		dateFormat = 0;
	else if(1 == hikDateFormat)
		dateFormat = 1;
	else if(4 == hikDateFormat)
		dateFormat = 4;
	else if(7 == hikDateFormat)
		dateFormat = 2;
	else if(6 == hikDateFormat)
		dateFormat = 3;
	else if(8 == hikDateFormat)
		dateFormat = 5;
	else
		dateFormat = 0;
    return dateFormat;
}

static BYTE _dateFormat_goke2hik(BYTE dateFormat)
{
    BYTE hikDateFormat = 0;
    if(1 == dateFormat)
		hikDateFormat = 1;
	else if(2 == dateFormat)
		hikDateFormat = 7;
	else if(3 == dateFormat)
		hikDateFormat = 6;
	else if(4 == dateFormat)
		hikDateFormat = 4;
	else if(5 == dateFormat)
		hikDateFormat = 8;
	else
		hikDateFormat = 0;
    return hikDateFormat;
}

int netClientGetPicCfgEx(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	NETCMD_HEADER  netCmdHeader;
	struct{
		NETRET_HEADER header;
		INTER_PICCFG_EX CfgInfo;;
	}netRetParamCfg;
	UINT32 retVal;
	int sendlen,index;
	sendlen = sizeof(netRetParamCfg);
	memset(&netRetParamCfg,0,sendlen);
	netRetParamCfg.header.length = htonl(sendlen);

#if 0
    HPRINT_INFO("\n\n\n\n\n***************************\n");
    MdCfgPrint();

    HPRINT_INFO("channel:%d\n", runMdCfg.channel);
    HPRINT_INFO("enable:%d\n", runMdCfg.enable);
    HPRINT_INFO("sensitive:%d\n", runMdCfg.sensitive);
    HPRINT_INFO("compensation:%d\n", runMdCfg.compensation);
    HPRINT_INFO("detectionType:%d\n", runMdCfg.detectionType);
    
    HPRINT_INFO("row:%d\n", runMdCfg.mdGrid.row);
    HPRINT_INFO("column:%d\n", runMdCfg.mdGrid.column);
    HPRINT_INFO("granularity:%s\n", runMdCfg.mdGrid.granularity);
    int k = 0;
    for (k = 0; k < 4; k ++) {
        HPRINT_INFO("mdRegion%d: enable:%d\n", k, runMdCfg.mdRegion[k].enable);
        HPRINT_INFO("mdRegion%d: x:%d\n", k, runMdCfg.mdRegion[k].x);
        HPRINT_INFO("mdRegion%d: y:%d\n", k, runMdCfg.mdRegion[k].y);
        HPRINT_INFO("mdRegion%d: width:%d\n", k, runMdCfg.mdRegion[k].width);
        HPRINT_INFO("mdRegion%d: height:%d\n", k, runMdCfg.mdRegion[k].height);
    }
    
    HPRINT_INFO("intervalTime:%u 0x%x\n", runMdCfg.handle.intervalTime, &(runMdCfg.handle.intervalTime));
    HPRINT_INFO("is_email:%u 0x%x\n", runMdCfg.handle.is_email, &(runMdCfg.handle.is_email));
    HPRINT_INFO("is_rec:%u 0x%x\n", runMdCfg.handle.is_rec, &(runMdCfg.handle.is_rec));
    HPRINT_INFO("recTime:%u 0x%x\n", runMdCfg.handle.recTime, &(runMdCfg.handle.recTime));

    HPRINT_INFO("recStreamNo:%u\n", runMdCfg.handle.recStreamNo);
    HPRINT_INFO("is_snap:%u\n", runMdCfg.handle.is_snap);
    HPRINT_INFO("isSnapUploadToFtp:%u\n", runMdCfg.handle.isSnapUploadToFtp);
    HPRINT_INFO("isSnapUploadToWeb:%u\n", runMdCfg.handle.isSnapUploadToWeb);
    HPRINT_INFO("isSnapUploadToCms:%u\n", runMdCfg.handle.isSnapUploadToCms);
    HPRINT_INFO("isSnapSaveToSd:%u\n", runMdCfg.handle.isSnapSaveToSd);

    HPRINT_INFO("snapNum:%d\n", runMdCfg.handle.snapNum);
    HPRINT_INFO("interval:%d\n", runMdCfg.handle.interval);
    HPRINT_INFO("is_alarmout:%d\n", runMdCfg.handle.is_alarmout);
    HPRINT_INFO("duration:%d\n", runMdCfg.handle.duration);

    HPRINT_INFO("is_beep:%u\n", runMdCfg.handle.is_beep);
    HPRINT_INFO("beepTime:%u\n", runMdCfg.handle.beepTime);


    HPRINT_INFO("ptzLink.type:%u\n", runMdCfg.handle.ptzLink.type);
    HPRINT_INFO("ptzLink.value:%u\n", runMdCfg.handle.ptzLink.value);

    HPRINT_INFO("is_pushMsg:%u\n", runMdCfg.handle.is_pushMsg);
    HPRINT_INFO("schedule_mode:%u\n", runMdCfg.schedule_mode);

    int m,n = 0;
    for (m = 0; m < 7; m++) {
        for (n = 0; n < 2; n++) {
            HPRINT_INFO("!!!!get day[%d]-index[%d] time:[%u:%u] - [%u:%u]\n", m, n, 
                runMdCfg.scheduleTime[m][n].startHour, runMdCfg.scheduleTime[m][n].startMin,
                runMdCfg.scheduleTime[m][n].stopHour, runMdCfg.scheduleTime[m][n].stopMin);

        }
    }

    #if 0
    printf("%s : [\n", "schetime");
    GK_SCHEDTIME *stime = (GK_SCHEDTIME *)&(runMdCfg.scheduleTime[0][0]);
    
    int i, j;
    for (i = 0; i < 7; i ++) {
        printf("    date %d: ", i);
        for (j = 0; j < 4; j ++) {
            printf("%d:%d - %d:%d, ", stime->startHour, \
                                      stime->startMin, \
                                      stime->stopHour, \
                                      stime->stopMin);
            stime ++;
        }
        printf("\n");
    }
    printf("]\n");
    #endif
#endif


	memcpy((char *)&netCmdHeader, recvbuff, sizeof(NETCMD_HEADER));
	retVal = verifyNetClientOperation(&netCmdHeader, pClientSockAddr, REMOTESHOWPARAMETER);

    if(retVal == NETRET_QUALIFIED)
	{
		HPRINT_INFO("Get OSD relevant param\n");
		
		netRetParamCfg.CfgInfo.dwSize = htonl(sizeof(INTER_PICCFG_EX));
        #if 0
        DMS_NET_CHANNEL_OSDINFO OSDInfo;
		memset(&OSDInfo,0,sizeof(OSDInfo));
		dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_GET_OSDCFG, 0,&OSDInfo,sizeof(OSDInfo));
		strcpy((char *)netRetParamCfg.CfgInfo.sChanName,OSDInfo.csChannelName);

		DMS_NET_DEVICE_INFO DeviceInfo;
		memset(&DeviceInfo,0,sizeof(DeviceInfo));
		dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_GET_DEVICECFG,0,&DeviceInfo,sizeof(DeviceInfo));
		if(0 == DeviceInfo.byVideoStandard)
			netRetParamCfg.CfgInfo.dwVideoFormat = htonl(1);
		else
			netRetParamCfg.CfgInfo.dwVideoFormat = htonl(2);
		
		DMS_NET_CHANNEL_COLOR ColorInfo;
		memset(&ColorInfo,0,sizeof(ColorInfo));
		dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_GET_COLORCFG,0,&ColorInfo,sizeof(ColorInfo));
		netRetParamCfg.CfgInfo.byBrightness = ColorInfo.nBrightness*255/100;
		netRetParamCfg.CfgInfo.byContrast = ColorInfo.nContrast*255/100;
		netRetParamCfg.CfgInfo.bySaturation = ColorInfo.nSaturation;
		netRetParamCfg.CfgInfo.byHue = ColorInfo.nHue*255/100;	

		netRetParamCfg.CfgInfo.dwShowChanName = htonl(OSDInfo.byShowChanName);
		netRetParamCfg.CfgInfo.wShowNameTopLeftX = htons(OSDInfo.dwShowNameTopLeftX);
		netRetParamCfg.CfgInfo.wShowNameTopLeftY = htons(OSDInfo.dwShowNameTopLeftY);

		//D?o??aê§±¨?ˉ
		DMS_NET_CHANNEL_VILOST ViLostInfo;
		memset(&ViLostInfo,0,sizeof(ViLostInfo));
		dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_GET_VLOSTCFG,0,&ViLostInfo,sizeof(ViLostInfo));
		netRetParamCfg.CfgInfo.struVILost.dwEnableVILostAlarm = htonl(ViLostInfo.bEnable);
		netRetParamCfg.CfgInfo.struVILost.strVILostAlarmHandleType.dwHandleType |= UPTOCENTER;
		netRetParamCfg.CfgInfo.struVILost.strVILostAlarmHandleType.dwAlarmOutTriggered = htonl(ViLostInfo.stHandle.byRelAlarmOut[0]);
#ifdef SDK_V13
		memcpy(netRetParamCfg.CfgInfo.struVILost.struAlarmTime,ViLostInfo.stScheduleTime, sizeof(ViLostInfo.stScheduleTime));
#endif

        //关闭移动侦测区域设置，by bruce
		//移动侦测
		DMS_NET_CHANNEL_MOTION_DETECT MDInfo;
		memset(&MDInfo,0,sizeof(MDInfo));
		dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_GET_MOTIONCFG,0,&MDInfo,sizeof(MDInfo));

		int x,y;
		int fromX = 44,fromY = 36;
		int toX = 22,toY = 18;	

	    for(y = 0; y < toY; y++)
	    {
	        for(x = 0; x < toX; x++)
	        {
				index = y*fromX*fromY/toY + x*fromX/toX ;
	            if(CHK_BIT(MDInfo.byMotionArea, index)) 
				{
					netRetParamCfg.CfgInfo.struMotion.dwMotionScope[y] |= ntohl((1<<x));
	            }
	        }
	    } 
		
		if(MDInfo.dwSensitive>=5 && MDInfo.dwSensitive<20)
			netRetParamCfg.CfgInfo.struMotion.byMotionSenstive = 0x05;
		else if(MDInfo.dwSensitive>=20 && MDInfo.dwSensitive<36)
			netRetParamCfg.CfgInfo.struMotion.byMotionSenstive = 0x04;
		else if(MDInfo.dwSensitive>=36 && MDInfo.dwSensitive<52)
			netRetParamCfg.CfgInfo.struMotion.byMotionSenstive = 0x03;
		else if(MDInfo.dwSensitive>=52 && MDInfo.dwSensitive<68)
			netRetParamCfg.CfgInfo.struMotion.byMotionSenstive = 0x02;
		else if(MDInfo.dwSensitive>=68 && MDInfo.dwSensitive<84)
			netRetParamCfg.CfgInfo.struMotion.byMotionSenstive = 0x01;
		else if(MDInfo.dwSensitive>=84 && MDInfo.dwSensitive<100)
			netRetParamCfg.CfgInfo.struMotion.byMotionSenstive = 0x00;
		else
			netRetParamCfg.CfgInfo.struMotion.byMotionSenstive = 0xFF;
		netRetParamCfg.CfgInfo.struMotion.byEnableHandleMotion = MDInfo.bEnable;
		netRetParamCfg.CfgInfo.struMotion.struMotionHandleType.dwHandleType |= UPTOCENTER;
		netRetParamCfg.CfgInfo.struMotion.struMotionHandleType.dwAlarmOutTriggered = htonl(MDInfo.stHandle.byRelAlarmOut[0]);
#ifdef SDK_V13
		memcpy(netRetParamCfg.CfgInfo.struMotion.struAlarmTime,MDInfo.stScheduleTime, sizeof(MDInfo.stScheduleTime));
#endif
		netRetParamCfg.CfgInfo.struMotion.dwRelRecordChan = htonl(MDInfo.stHandle.byRecordChannel[0]);	

		//遮挡报警
		DMS_NET_CHANNEL_HIDEALARM HideAlarmInfo;
		memset(&HideAlarmInfo,0,sizeof(HideAlarmInfo));
		dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_GET_HIDEALARMCFG,0,&HideAlarmInfo,sizeof(HideAlarmInfo));
		netRetParamCfg.CfgInfo.struHideAlarm.dwEnableHideAlarm = htonl(HideAlarmInfo.bEnable);
		netRetParamCfg.CfgInfo.struHideAlarm.wHideAlarmAreaTopLeftX = htons(HideAlarmInfo.wHideAlarmAreaTopLeftX);
		netRetParamCfg.CfgInfo.struHideAlarm.wHideAlarmAreaTopLeftY = htons(HideAlarmInfo.wHideAlarmAreaTopLeftY);
		netRetParamCfg.CfgInfo.struHideAlarm.wHideAlarmAreaWidth = htons(HideAlarmInfo.wHideAlarmAreaWidth);
		netRetParamCfg.CfgInfo.struHideAlarm.wHideAlarmAreaHeight = htons(HideAlarmInfo.wHideAlarmAreaHeight);
		netRetParamCfg.CfgInfo.struHideAlarm.strHideAlarmHandleType.dwHandleType |= UPTOCENTER;
		netRetParamCfg.CfgInfo.struHideAlarm.strHideAlarmHandleType.dwAlarmOutTriggered = htonl(HideAlarmInfo.stHandle.byRelAlarmOut[0]);
#ifdef SDK_V13
		memcpy(netRetParamCfg.CfgInfo.struHideAlarm.struAlarmTime,HideAlarmInfo.stScheduleTime, sizeof(HideAlarmInfo.stScheduleTime));
#endif


		//遮挡区域
		DMS_NET_CHANNEL_SHELTER ShelterCfg;
		memset(&ShelterCfg,0,sizeof(ShelterCfg));
		dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_GET_SHELTERCFG,0,&ShelterCfg,sizeof(ShelterCfg));
		netRetParamCfg.CfgInfo.dwEnableHide = htonl(ShelterCfg.bEnable);
		for(index=0;index<4;index++)
		{
			netRetParamCfg.CfgInfo.struShelter[index].wHideAreaTopLeftX = htons(ShelterCfg.strcShelter[index].wLeft);
			netRetParamCfg.CfgInfo.struShelter[index].wHideAreaTopLeftY = htons(ShelterCfg.strcShelter[index].wTop);
			netRetParamCfg.CfgInfo.struShelter[index].wHideAreaWidth = htons(ShelterCfg.strcShelter[index].wWidth);
			netRetParamCfg.CfgInfo.struShelter[index].wHideAreaHeight = htons(ShelterCfg.strcShelter[index].wHeight);
		}

		//OSD
		netRetParamCfg.CfgInfo.dwShowDate = htonl(OSDInfo.bShowTime);
		netRetParamCfg.CfgInfo.wShowDateX = htons(OSDInfo.dwTimeTopLeftX);
		netRetParamCfg.CfgInfo.wShowDateY = htons(OSDInfo.dwTimeTopLeftY);
		if(1 == DeviceInfo.byDateFormat)
			netRetParamCfg.CfgInfo.byDateType = 1;
		else if(2 == DeviceInfo.byDateFormat)
			netRetParamCfg.CfgInfo.byDateType = 7;
		else if(3 == DeviceInfo.byDateFormat)
			netRetParamCfg.CfgInfo.byDateType = 6;
		else if(4 == DeviceInfo.byDateFormat)
			netRetParamCfg.CfgInfo.byDateType = 4;
		else if(5 == DeviceInfo.byDateFormat)
			netRetParamCfg.CfgInfo.byDateType = 8;
		else
			netRetParamCfg.CfgInfo.byDateType = 0;
		netRetParamCfg.CfgInfo.byDispWeek = OSDInfo.bDispWeek;
		netRetParamCfg.CfgInfo.byOSDAttrib = OSDInfo.byOSDAttrib;
		netRetParamCfg.CfgInfo.byTimeFmt = DeviceInfo.byTimeFmt;
        #else

		netRetParamCfg.CfgInfo.dwSize = htonl(sizeof(INTER_PICCFG_EX));
        //strcpy((char *)netRetParamCfg.CfgInfo.sChanName, runChannelCfg.channelInfo[0].osdChannelName.text);
        utility_utf8_to_gbk(runChannelCfg.channelInfo[0].osdChannelName.text,(char *)netRetParamCfg.CfgInfo.sChanName,sizeof(netRetParamCfg.CfgInfo.sChanName));
        if(runSystemCfg.deviceInfo.videoType == GK_NTSC)
            netRetParamCfg.CfgInfo.dwVideoFormat = htonl(1);
        else //GK_PAL
            netRetParamCfg.CfgInfo.dwVideoFormat = htonl(2);
        
        // 50/x = 100/255, x = 255*50/100 = 127
    	netRetParamCfg.CfgInfo.byBrightness = runImageCfg.brightness*255/100;
    	netRetParamCfg.CfgInfo.byContrast   = runImageCfg.contrast*255/100;
    	netRetParamCfg.CfgInfo.bySaturation = runImageCfg.saturation*255/100;	
    	//netRetParamCfg.CfgInfo.byHue        = runImageCfg.hue*255/100; 
		netRetParamCfg.CfgInfo.dwShowChanName = htonl(runChannelCfg.channelInfo[0].osdChannelName.enable);
        netRetParamCfg.CfgInfo.wShowNameTopLeftX = htons(_float_rate_to_data(runChannelCfg.channelInfo[0].osdChannelName.x, _CMS_WIDTH));
        netRetParamCfg.CfgInfo.wShowNameTopLeftY = htons(_float_rate_to_data(runChannelCfg.channelInfo[0].osdChannelName.y, _CMS_HEIGHT));


        HPRINT_INFO("brightness:%u, contrast:%u, saturation:%u, hue:%u\n",
            runImageCfg.brightness, runImageCfg.contrast, runImageCfg.saturation, runImageCfg.hue);

        netRetParamCfg.CfgInfo.struVILost.dwEnableVILostAlarm = htonl(0);


        //移动侦测区域设置
        #if 0
        int x,y;
        for (y = 0; y < 22; y++) {
            for (x = 0; x < 18; x++) {
                int gkY = (y < 20)?y:19;
                int gkX = (x < 15)?x:14;
                if (vin_get_md_bitmap_one_mask(0, gkX, gkY)) {
                    netRetParamCfg.CfgInfo.struMotion.dwMotionScope[y] |= ntohl((1<<x));
                    
                }
            }
        }
        #else
		int x,y;
		//int fromX = 44,fromY = 36;
		int toX = 22,toY = 18;	
	    for(y = 0; y < toY; y++)
	    {
	        for(x = 0; x < toX; x++)
	        {
				//index = y*fromX*fromY/toY + x*fromX/toX ;
	            //if(CHK_BIT(MDInfo.byMotionArea, index))
	            if (1)
				{
					netRetParamCfg.CfgInfo.struMotion.dwMotionScope[y] |= ntohl((1<<x));
	            }
	        }
	    }
        #endif
        
        if(runMdCfg.sensitive>=5 && runMdCfg.sensitive<20)
            netRetParamCfg.CfgInfo.struMotion.byMotionSenstive = 0x05;
        else if(runMdCfg.sensitive>=20 && runMdCfg.sensitive<36)
            netRetParamCfg.CfgInfo.struMotion.byMotionSenstive = 0x04;
        else if(runMdCfg.sensitive>=36 && runMdCfg.sensitive<52)
            netRetParamCfg.CfgInfo.struMotion.byMotionSenstive = 0x03;
        else if(runMdCfg.sensitive>=52 && runMdCfg.sensitive<68)
            netRetParamCfg.CfgInfo.struMotion.byMotionSenstive = 0x02;
        else if(runMdCfg.sensitive>=68 && runMdCfg.sensitive<84)
            netRetParamCfg.CfgInfo.struMotion.byMotionSenstive = 0x01;
        else if(runMdCfg.sensitive>=84 && runMdCfg.sensitive<100)
            netRetParamCfg.CfgInfo.struMotion.byMotionSenstive = 0x00;
        else
            netRetParamCfg.CfgInfo.struMotion.byMotionSenstive = 0xFF;

        HPRINT_INFO("runMdCfg.enable:%d\n", runMdCfg.enable);
        netRetParamCfg.CfgInfo.struMotion.byEnableHandleMotion = runMdCfg.enable;
        //#define NOACTION 0x0 /*无响应*/
        //#define WARNONMONITOR 0x1 /*监视器上警告*/
        //#define WARNONAUDIOOUT 0x2 /*声音警告*/
        //#define UPTOCENTER 0x4 /*上传中心*/
        //#define TRIGGERALARMOUT 0x8 /*触发报警输出*/
        netRetParamCfg.CfgInfo.struMotion.struMotionHandleType.dwHandleType |= WARNONMONITOR;
        netRetParamCfg.CfgInfo.struMotion.struMotionHandleType.dwAlarmOutTriggered = htonl(0);

        #ifdef SDK_V13
        int i,j = 0;
        for (i = 0; i < 7; i++) {
            for (j = 0; j < 2; j++) {
                #if 1
                HPRINT_INFO("get day[%d]-index[%d] time:[%u:%u] - [%u:%u]\n", i, j, 
                    runMdCfg.scheduleTime[i][j].startHour, runMdCfg.scheduleTime[i][j].startMin,
                    runMdCfg.scheduleTime[i][j].stopHour, runMdCfg.scheduleTime[i][j].stopMin);
                #endif
                #if 1
                netRetParamCfg.CfgInfo.struMotion.struAlarmTime[i][j].byStartHour = 
                    (BYTE)runMdCfg.scheduleTime[i][j].startHour;
                netRetParamCfg.CfgInfo.struMotion.struAlarmTime[i][j].byStartMin = 
                    (BYTE)runMdCfg.scheduleTime[i][j].startMin;
                netRetParamCfg.CfgInfo.struMotion.struAlarmTime[i][j].byStopHour = 
                    (BYTE)runMdCfg.scheduleTime[i][j].stopHour;
                netRetParamCfg.CfgInfo.struMotion.struAlarmTime[i][j].byStopMin = 
                    (BYTE)runMdCfg.scheduleTime[i][j].stopMin;
                #endif
            }
        }
        #endif

        netRetParamCfg.CfgInfo.struMotion.dwRelRecordChan = htonl(0);  

        //遮挡区域
        int Shelter_enable = runChannelCfg.shelterRect[0].enable | runChannelCfg.shelterRect[1].enable |
            runChannelCfg.shelterRect[2].enable | runChannelCfg.shelterRect[3].enable;
        netRetParamCfg.CfgInfo.dwEnableHide = htonl(Shelter_enable);
        for(index=0;index<4;index++)
        {
            netRetParamCfg.CfgInfo.struShelter[index].wHideAreaTopLeftX = htons(_float_rate_to_data(runChannelCfg.shelterRect[index].x, _CMS_WIDTH));
            netRetParamCfg.CfgInfo.struShelter[index].wHideAreaTopLeftY = htons(_float_rate_to_data(runChannelCfg.shelterRect[index].y, _CMS_HEIGHT));
            netRetParamCfg.CfgInfo.struShelter[index].wHideAreaWidth = htons(_float_rate_to_data(runChannelCfg.shelterRect[index].width, _CMS_WIDTH));
            netRetParamCfg.CfgInfo.struShelter[index].wHideAreaHeight = htons(_float_rate_to_data(runChannelCfg.shelterRect[index].height, _CMS_HEIGHT));
        }

        //OSD
        netRetParamCfg.CfgInfo.dwShowDate = htonl(runChannelCfg.channelInfo[0].osdDatetime.enable);
        netRetParamCfg.CfgInfo.wShowDateX = htons(_float_rate_to_data(runChannelCfg.channelInfo[0].osdDatetime.x, _CMS_WIDTH));
        netRetParamCfg.CfgInfo.wShowDateY = htons(_float_rate_to_data(runChannelCfg.channelInfo[0].osdDatetime.y, _CMS_HEIGHT));
        netRetParamCfg.CfgInfo.byDateType = _dateFormat_goke2hik(runChannelCfg.channelInfo[0].osdDatetime.dateFormat);
		netRetParamCfg.CfgInfo.byDispWeek = runChannelCfg.channelInfo[0].osdDatetime.displayWeek;
		netRetParamCfg.CfgInfo.byOSDAttrib = 4;
		netRetParamCfg.CfgInfo.byTimeFmt = runChannelCfg.channelInfo[0].osdDatetime.timeFmt;
        #endif
	}

	netRetParamCfg.header.retVal = htonl(retVal);
	netRetParamCfg.header.checkSum = htonl(checkByteSum((char *)&(netRetParamCfg.header.retVal), sendlen-8));

	return writen(connfd, (char *)&netRetParamCfg, sendlen);
}

int netClientSetPicCfgEx(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	struct{
		NETCMD_HEADER header;
		UINT32 channel;
		INTER_PICCFG_EX CfgInfo;;
	}netSetParamCfg;
	UINT32 retVal;

	memset(&netSetParamCfg,0, sizeof(netSetParamCfg));
	memcpy((char *)&netSetParamCfg, recvbuff, sizeof(netSetParamCfg));

	retVal = verifyNetClientOperation(&(netSetParamCfg.header), pClientSockAddr, REMOTESETPARAMETER);
	if(retVal == NETRET_QUALIFIED)
	{
		if(ntohl(netSetParamCfg.CfgInfo.dwSize) != sizeof(INTER_PICCFG_EX))
		{	
			HPRINT_ERR("check length err\n");
			retVal = NETRET_ERROR_DATA;
		}
		else
		{
#if 0
			HPRINT_INFO("dwSize:%d,videoFormat:%d br:%d, contrast:%d stauration:%d hue:%d \n", 
							ntohl(netSetParamCfg.CfgInfo.dwSize),
							ntohl(netSetParamCfg.CfgInfo.dwVideoFormat), 
							netSetParamCfg.CfgInfo.byBrightness,
							netSetParamCfg.CfgInfo.byContrast, 
							netSetParamCfg.CfgInfo.bySaturation, 
							netSetParamCfg.CfgInfo.byHue);	
			HPRINT_INFO("OSD Param: dwShowChanName:%d-%ld wShowNameTopLeftX:%d wShowNameTopLeftY:%d",
							ntohl(netSetParamCfg.CfgInfo.dwShowChanName), 
							netSetParamCfg.CfgInfo.dwShowChanName,
							ntohs(netSetParamCfg.CfgInfo.wShowNameTopLeftX), 
							ntohs(netSetParamCfg.CfgInfo.wShowNameTopLeftY));
			HPRINT_INFO("dwShowDate:%d,wShowDateX:%d,wShowDateY:%d,byDateType:%d,byDispWeek:%d,byOSDAttrib:%d,byTimeFmt:%d\n",
							ntohl(netSetParamCfg.CfgInfo.dwShowDate),
							ntohs(netSetParamCfg.CfgInfo.wShowDateX),
							ntohs(netSetParamCfg.CfgInfo.wShowDateY),
							netSetParamCfg.CfgInfo.byDateType,
							netSetParamCfg.CfgInfo.byDispWeek,
							netSetParamCfg.CfgInfo.byOSDAttrib,
							netSetParamCfg.CfgInfo.byTimeFmt);
			HPRINT_INFO("VILOST Param: EnableVILostAlarm:%d,handleType:0x%08X\n",
							ntohl(netSetParamCfg.CfgInfo.struVILost.dwEnableVILostAlarm),
							ntohl(netSetParamCfg.CfgInfo.struVILost.strVILostAlarmHandleType.dwHandleType));
			HPRINT_INFO("Motion Param: motion area:-- stive:0x%02X enable:%d handleType:0x%08X AlarmOut:0x%08X\n", 
							netSetParamCfg.CfgInfo.struMotion.byMotionSenstive, 
							netSetParamCfg.CfgInfo.struMotion.byEnableHandleMotion,
							ntohl(netSetParamCfg.CfgInfo.struMotion.struMotionHandleType.dwHandleType),
							ntohl(netSetParamCfg.CfgInfo.struMotion.struMotionHandleType.dwAlarmOutTriggered));
			HPRINT_INFO("HIDEALARM Param: EnableHideAlarm:%d,wHideAlarmAreaTopLeftX:%d,wHideAlarmAreaTopLeftY:%d",
							ntohl(netSetParamCfg.CfgInfo.struHideAlarm.dwEnableHideAlarm),
							ntohs(netSetParamCfg.CfgInfo.struHideAlarm.wHideAlarmAreaTopLeftX),
							ntohs(netSetParamCfg.CfgInfo.struHideAlarm.wHideAlarmAreaTopLeftY));
			HPRINT_INFO("wHideAlarmAreaWidth:%d,wHideAlarmAreaHeight:%d,HandleType:0x%08X\n",
							ntohs(netSetParamCfg.CfgInfo.struHideAlarm.wHideAlarmAreaWidth),
							ntohs(netSetParamCfg.CfgInfo.struHideAlarm.wHideAlarmAreaHeight),
							ntohl(netSetParamCfg.CfgInfo.struHideAlarm.strHideAlarmHandleType.dwHandleType));
			HPRINT_INFO("Hide Param: EnableHide:%d,X:%d,Y:%d,W:%d,H:%d\n",
							ntohl(netSetParamCfg.CfgInfo.dwEnableHide),
							ntohs(netSetParamCfg.CfgInfo.struShelter[0].wHideAreaTopLeftX),
							ntohs(netSetParamCfg.CfgInfo.struShelter[0].wHideAreaTopLeftY),
							ntohs(netSetParamCfg.CfgInfo.struShelter[0].wHideAreaWidth),
							ntohs(netSetParamCfg.CfgInfo.struShelter[0].wHideAreaHeight));

#endif
            #if 0
			//OSD′|àí
			DMS_NET_CHANNEL_OSDINFO OSDCfg;
			memset(&OSDCfg,0,sizeof(OSDCfg));
			dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_GET_OSDCFG, 0,&OSDCfg,sizeof(OSDCfg));
			strcpy(OSDCfg.csChannelName,(char *)netSetParamCfg.CfgInfo.sChanName);
			OSDCfg.byShowChanName = ntohl(netSetParamCfg.CfgInfo.dwShowChanName);
			OSDCfg.dwShowNameTopLeftX = ntohs(netSetParamCfg.CfgInfo.wShowNameTopLeftX);
			OSDCfg.dwShowNameTopLeftY = ntohs(netSetParamCfg.CfgInfo.wShowNameTopLeftY);
			OSDCfg.bShowTime = ntohl(netSetParamCfg.CfgInfo.dwShowDate);
			OSDCfg.dwTimeTopLeftX = ntohs(netSetParamCfg.CfgInfo.wShowDateX);
			OSDCfg.dwTimeTopLeftY = ntohs(netSetParamCfg.CfgInfo.wShowDateY);
			OSDCfg.bDispWeek = netSetParamCfg.CfgInfo.byDispWeek;
			OSDCfg.byOSDAttrib = netSetParamCfg.CfgInfo.byOSDAttrib;
			dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_SET_OSDCFG, 0,&OSDCfg,sizeof(OSDCfg));

			DMS_NET_DEVICE_INFO DeviceCfg;			
			memset(&DeviceCfg,0,sizeof(DeviceCfg));
			dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_GET_DEVICECFG,0,&DeviceCfg,sizeof(DeviceCfg));
			if(1 == netSetParamCfg.CfgInfo.byDateType)
				DeviceCfg.byDateFormat = 1;
			else if(7 == netSetParamCfg.CfgInfo.byDateType)
				DeviceCfg.byDateFormat = 2;
			else if(6 == netSetParamCfg.CfgInfo.byDateType)
				DeviceCfg.byDateFormat = 3;
			else if(4 == netSetParamCfg.CfgInfo.byDateType)
				DeviceCfg.byDateFormat = 4;
			else if(8 == netSetParamCfg.CfgInfo.byDateType)
				DeviceCfg.byDateFormat = 5;
			else
				DeviceCfg.byDateFormat = 0;
			DeviceCfg.byTimeFmt = netSetParamCfg.CfgInfo.byTimeFmt;
			dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_SET_DEVICECFG,0,&DeviceCfg,sizeof(DeviceCfg));	

			//视频丢失报警
			DMS_NET_CHANNEL_VILOST ViLostInfo;
			memset(&ViLostInfo,0,sizeof(ViLostInfo));
			dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_GET_VLOSTCFG,0,&ViLostInfo,sizeof(ViLostInfo));
			ViLostInfo.bEnable = ntohl(netSetParamCfg.CfgInfo.struVILost.dwEnableVILostAlarm);
			ViLostInfo.stHandle.wActionMask |= DMS_ALARM_EXCEPTION_UPTOCENTER;
			memcpy(ViLostInfo.stScheduleTime, netSetParamCfg.CfgInfo.struVILost.struAlarmTime, sizeof(netSetParamCfg.CfgInfo.struVILost.struAlarmTime));
			dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_SET_VLOSTCFG,0,&ViLostInfo,sizeof(ViLostInfo));

			//移动侦测处理
			int column, row;
			unsigned long tmpRegion;
			DMS_NET_CHANNEL_MOTION_DETECT objMotionParam;
			memset(&objMotionParam,0,sizeof(DMS_NET_CHANNEL_MOTION_DETECT));
			dms_sysnetapi_ioctrl(g_hik_handle, DMS_NET_GET_MOTIONCFG, 0, &objMotionParam, sizeof(DMS_NET_CHANNEL_MOTION_DETECT));
	
			for (row = 0; row < 18; row++)
			{
				tmpRegion = ntohl(netSetParamCfg.CfgInfo.struMotion.dwMotionScope[row]);
				for (column = 0; column < 22; column++)
				{
					if (tmpRegion & (1 << column))
					{
						SET_BIT(objMotionParam.byMotionArea, 2 * row * 44 + 2 * column);
						SET_BIT(objMotionParam.byMotionArea, 2 * row * 44 + 2 * column + 1);
						SET_BIT(objMotionParam.byMotionArea, (2 * row + 1) * 44 + 2 * column);
						SET_BIT(objMotionParam.byMotionArea, (2 * row + 1) * 44 + 2 * column + 1);
					}
					else
					{
						CLR_BIT(objMotionParam.byMotionArea, 2 * row * 44 + 2 * column);
						CLR_BIT(objMotionParam.byMotionArea, 2 * row * 44 + 2 * column + 1);
						CLR_BIT(objMotionParam.byMotionArea, (2 * row + 1) * 44 + 2 * column);
						CLR_BIT(objMotionParam.byMotionArea, (2 * row + 1) * 44 + 2 * column + 1);
					}
				}
			}		
			if(0x05 == netSetParamCfg.CfgInfo.struMotion.byMotionSenstive)
				objMotionParam.dwSensitive = 5;
			else if(0x04 == netSetParamCfg.CfgInfo.struMotion.byMotionSenstive)
				objMotionParam.dwSensitive = 20;
			else if(0x03 == netSetParamCfg.CfgInfo.struMotion.byMotionSenstive)
				objMotionParam.dwSensitive = 40;
			else if(0x02 == netSetParamCfg.CfgInfo.struMotion.byMotionSenstive)
				objMotionParam.dwSensitive = 60;
			else if(0x01 == netSetParamCfg.CfgInfo.struMotion.byMotionSenstive)
				objMotionParam.dwSensitive = 80;
			else 
				objMotionParam.dwSensitive = 100;
			objMotionParam.bEnable = netSetParamCfg.CfgInfo.struMotion.byEnableHandleMotion;
			objMotionParam.stHandle.wActionMask |= DMS_ALARM_EXCEPTION_UPTOCENTER;		//not convert
			memcpy(objMotionParam.stScheduleTime, netSetParamCfg.CfgInfo.struMotion.struAlarmTime, sizeof(netSetParamCfg.CfgInfo.struMotion.struAlarmTime));
			objMotionParam.stHandle.byRelAlarmOut[0] = ntohl(netSetParamCfg.CfgInfo.struMotion.dwRelRecordChan);
			dms_sysnetapi_ioctrl(g_hik_handle, DMS_NET_SET_MOTIONCFG, 0, &objMotionParam, sizeof(DMS_NET_CHANNEL_MOTION_DETECT));

			//3518EV200暂不支持遮挡报警
			if(DeviceCfg.dwServerCPUType != DMS_CPU_HI3518EV2)
			{
				//遮挡报警
				DMS_NET_CHANNEL_HIDEALARM HideAlarmInfo;
				memset(&HideAlarmInfo,0,sizeof(HideAlarmInfo));
				dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_GET_HIDEALARMCFG,0,&HideAlarmInfo,sizeof(HideAlarmInfo));
				HideAlarmInfo.bEnable = ntohl(netSetParamCfg.CfgInfo.struHideAlarm.dwEnableHideAlarm);
				HideAlarmInfo.wHideAlarmAreaTopLeftX = ntohs(netSetParamCfg.CfgInfo.struHideAlarm.wHideAlarmAreaTopLeftX);
				HideAlarmInfo.wHideAlarmAreaTopLeftY = ntohs(netSetParamCfg.CfgInfo.struHideAlarm.wHideAlarmAreaTopLeftY);
				HideAlarmInfo.wHideAlarmAreaWidth = ntohs(netSetParamCfg.CfgInfo.struHideAlarm.wHideAlarmAreaWidth);
				HideAlarmInfo.wHideAlarmAreaHeight = ntohs(netSetParamCfg.CfgInfo.struHideAlarm.wHideAlarmAreaHeight);
				HideAlarmInfo.stHandle.wActionMask |= DMS_ALARM_EXCEPTION_UPTOCENTER;
				memcpy(HideAlarmInfo.stScheduleTime, netSetParamCfg.CfgInfo.struHideAlarm.struAlarmTime, sizeof(netSetParamCfg.CfgInfo.struHideAlarm.struAlarmTime));
				dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_SET_HIDEALARMCFG,0,&HideAlarmInfo,sizeof(HideAlarmInfo));
			}

			//遮挡配置
			int i=0;
			DMS_NET_CHANNEL_SHELTER ShelterCfg;
			memset(&ShelterCfg,0,sizeof(ShelterCfg));
			dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_GET_SHELTERCFG,0,&ShelterCfg,sizeof(ShelterCfg));
			ShelterCfg.bEnable = ntohl(netSetParamCfg.CfgInfo.dwEnableHide);
			for(i=0;i<4;i++)
			{
				ShelterCfg.strcShelter[i].wLeft = ntohs(netSetParamCfg.CfgInfo.struShelter[i].wHideAreaTopLeftX);
				ShelterCfg.strcShelter[i].wTop = ntohs(netSetParamCfg.CfgInfo.struShelter[i].wHideAreaTopLeftY);
				ShelterCfg.strcShelter[i].wWidth = ntohs(netSetParamCfg.CfgInfo.struShelter[i].wHideAreaWidth);
				ShelterCfg.strcShelter[i].wHeight = ntohs(netSetParamCfg.CfgInfo.struShelter[i].wHideAreaHeight);
			}
			dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_SET_SHELTERCFG,0,&ShelterCfg,sizeof(ShelterCfg));
			dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_SET_SAVECFG,0,NULL,0);
            #else

            #if 0
            HPRINT_INFO("dwSize:%d,videoFormat:%d br:%d, contrast:%d stauration:%d hue:%d \n", 
                            ntohl(netSetParamCfg.CfgInfo.dwSize),
                            ntohl(netSetParamCfg.CfgInfo.dwVideoFormat), 
                            netSetParamCfg.CfgInfo.byBrightness,
                            netSetParamCfg.CfgInfo.byContrast, 
                            netSetParamCfg.CfgInfo.bySaturation, 
                            netSetParamCfg.CfgInfo.byHue);  
            HPRINT_INFO("OSD Param: dwShowChanName:%d-%ld wShowNameTopLeftX:%d wShowNameTopLeftY:%d",
                            ntohl(netSetParamCfg.CfgInfo.dwShowChanName),
                            ntohs(netSetParamCfg.CfgInfo.wShowNameTopLeftX), 
                            ntohs(netSetParamCfg.CfgInfo.wShowNameTopLeftY));
            HPRINT_INFO("dwShowDate:%d,wShowDateX:%d,wShowDateY:%d,byDateType:%d,byDispWeek:%d,byOSDAttrib:%d,byTimeFmt:%d\n",
                            ntohl(netSetParamCfg.CfgInfo.dwShowDate),
                            ntohs(netSetParamCfg.CfgInfo.wShowDateX),
                            ntohs(netSetParamCfg.CfgInfo.wShowDateY),
                            netSetParamCfg.CfgInfo.byDateType,
                            netSetParamCfg.CfgInfo.byDispWeek,
                            netSetParamCfg.CfgInfo.byOSDAttrib,
                            netSetParamCfg.CfgInfo.byTimeFmt);
            HPRINT_INFO("VILOST Param: EnableVILostAlarm:%d,handleType:0x%08X\n",
                            ntohl(netSetParamCfg.CfgInfo.struVILost.dwEnableVILostAlarm),
                            ntohl(netSetParamCfg.CfgInfo.struVILost.strVILostAlarmHandleType.dwHandleType));
            HPRINT_INFO("Motion Param: motion area:-- stive:0x%02X enable:%d handleType:0x%08X AlarmOut:0x%08X\n", 
                            netSetParamCfg.CfgInfo.struMotion.byMotionSenstive, 
                            netSetParamCfg.CfgInfo.struMotion.byEnableHandleMotion,
                            ntohl(netSetParamCfg.CfgInfo.struMotion.struMotionHandleType.dwHandleType),
                            ntohl(netSetParamCfg.CfgInfo.struMotion.struMotionHandleType.dwAlarmOutTriggered));
            HPRINT_INFO("HIDEALARM Param: EnableHideAlarm:%d,wHideAlarmAreaTopLeftX:%d,wHideAlarmAreaTopLeftY:%d",
                            ntohl(netSetParamCfg.CfgInfo.struHideAlarm.dwEnableHideAlarm),
                            ntohs(netSetParamCfg.CfgInfo.struHideAlarm.wHideAlarmAreaTopLeftX),
                            ntohs(netSetParamCfg.CfgInfo.struHideAlarm.wHideAlarmAreaTopLeftY));
            HPRINT_INFO("wHideAlarmAreaWidth:%d,wHideAlarmAreaHeight:%d,HandleType:0x%08X\n",
                            ntohs(netSetParamCfg.CfgInfo.struHideAlarm.wHideAlarmAreaWidth),
                            ntohs(netSetParamCfg.CfgInfo.struHideAlarm.wHideAlarmAreaHeight),
                            ntohl(netSetParamCfg.CfgInfo.struHideAlarm.strHideAlarmHandleType.dwHandleType));
            HPRINT_INFO("Hide Param: EnableHide:%d,X:%d,Y:%d,W:%d,H:%d\n",
                            ntohl(netSetParamCfg.CfgInfo.dwEnableHide),
                            ntohs(netSetParamCfg.CfgInfo.struShelter[0].wHideAreaTopLeftX),
                            ntohs(netSetParamCfg.CfgInfo.struShelter[0].wHideAreaTopLeftY),
                            ntohs(netSetParamCfg.CfgInfo.struShelter[0].wHideAreaWidth),
                            ntohs(netSetParamCfg.CfgInfo.struShelter[0].wHideAreaHeight));
            #endif
			int i = 0, ret = 0;
            char buffer[32] = {0};
            GK_NET_CHANNEL_INFO channelInfo;
            HPRINT_INFO("***********  dateFormat:%d\n", netSetParamCfg.CfgInfo.byDateType);
            ret = netcam_osd_get_info(0, &channelInfo);;
            if (ret != 0)
            {
                HPRINT_ERR("get osd parameters failed.");
                return sendNetRetval(connfd, NETRET_NOT_SUPPORT);
            }
            channelInfo.osdChannelName.enable   = ntohl(netSetParamCfg.CfgInfo.dwShowChanName);
            utility_gbk_to_utf8((char *)netSetParamCfg.CfgInfo.sChanName, buffer, sizeof(buffer));
        	netcam_osd_text_copy(channelInfo.osdChannelName.text, buffer, sizeof(buffer));
            channelInfo.osdChannelName.x        = _data_to_float_rate(ntohs(netSetParamCfg.CfgInfo.wShowNameTopLeftX), _CMS_WIDTH);
            channelInfo.osdChannelName.y        = _data_to_float_rate(ntohs(netSetParamCfg.CfgInfo.wShowNameTopLeftY), _CMS_HEIGHT);
            
            channelInfo.osdDatetime.enable      = ntohl(netSetParamCfg.CfgInfo.dwShowDate);
            channelInfo.osdDatetime.dateFormat  = _dateFormat_hik2goke(netSetParamCfg.CfgInfo.byDateType);
            
            channelInfo.osdDatetime.timeFmt     = netSetParamCfg.CfgInfo.byTimeFmt;
            channelInfo.osdDatetime.x           = _data_to_float_rate(ntohs(netSetParamCfg.CfgInfo.wShowDateX), _CMS_WIDTH);
            channelInfo.osdDatetime.y           = _data_to_float_rate(ntohs(netSetParamCfg.CfgInfo.wShowDateY), _CMS_HEIGHT);
            channelInfo.osdDatetime.displayWeek = netSetParamCfg.CfgInfo.byDispWeek;
            for (i = 0; i < 2; i++) {
    			netcam_video_set_stream_name(i, channelInfo.osdChannelName.text);
                ret = netcam_osd_set_info(i, &channelInfo);
                if (ret != 0)
                {
                    HPRINT_ERR("set osd parameters failed.");
                }
            }

            //set video effect
            GK_NET_IMAGE_CFG stImagingConfig;
            
            ret = netcam_image_get(&stImagingConfig);
            if(ret != 0) {
                return sendNetRetval(connfd, NETRET_NOT_SUPPORT);
            }

            #if 1
            HPRINT_INFO("set -- saturation:%u to %u, contrast:%u to %u, brightness:%u to %u, hue:%u to %u\n",
                stImagingConfig.saturation, netSetParamCfg.CfgInfo.bySaturation,
                stImagingConfig.contrast, netSetParamCfg.CfgInfo.byContrast,
                stImagingConfig.brightness, netSetParamCfg.CfgInfo.byBrightness,
                stImagingConfig.hue, netSetParamCfg.CfgInfo.byHue);
            #endif
            

            stImagingConfig.brightness = netSetParamCfg.CfgInfo.byBrightness*100/255;
            stImagingConfig.contrast = netSetParamCfg.CfgInfo.byContrast*100/255;
            stImagingConfig.saturation = netSetParamCfg.CfgInfo.bySaturation*100/255;
            //stImagingConfig.hue = netSetParamCfg.CfgInfo.byHue*100/255;

            ret = netcam_image_set(stImagingConfig);
            if(ret != 0) {
                return sendNetRetval(connfd, NETRET_NOT_SUPPORT);
            }

            #if 0
            //ò??ˉ?ì2a′|àí
            int column, row, tmpRegion;
            for (row = 0; row < 18; row++) {
                tmpRegion = ntohl(netSetParamCfg.CfgInfo.struMotion.dwMotionScope[row]);
                for (column = 0; column < 22; column++) {
                    int newx = (column < 20)?column:19;
                    int newy = (row < 15)?row:14;
                    if (tmpRegion & (1 << column)) {//n = x*fx/tx
                        netcam_md_set_bitmap_one_mask(0, newx, newy, 1);
                        sdk_vin_set_md_bitmap_mask
                    } else {
                        netcam_md_set_bitmap_one_mask(0, newx, newy, 0);
                    }
                }
            }
            #endif
            #if 0
            HPRINT_INFO("MD ZONE-SIZE: %dx%d\r\n", 22, 18);
            HPRINT_INFO("MD ZONE-MASK:\n");
            for (row = 0; row < 18; row++) {
                tmpRegion = ntohl(netSetParamCfg.CfgInfo.struMotion.dwMotionScope[row]);
                for (column = 0; column < 22; column++) {
                    printf("%c", tmpRegion & (1 << column) ? '#' : '.');
                }
                printf("\n");
            }
            sdk_vin_dump_md(0);
            netcam_md_print_cfg_mask();
            #endif

            runMdCfg.enable = netSetParamCfg.CfgInfo.struMotion.byEnableHandleMotion;
			if(0x05 == netSetParamCfg.CfgInfo.struMotion.byMotionSenstive)
				runMdCfg.sensitive = 5;
			else if(0x04 == netSetParamCfg.CfgInfo.struMotion.byMotionSenstive)
				runMdCfg.sensitive = 20;
			else if(0x03 == netSetParamCfg.CfgInfo.struMotion.byMotionSenstive)
				runMdCfg.sensitive = 40;
			else if(0x02 == netSetParamCfg.CfgInfo.struMotion.byMotionSenstive)
				runMdCfg.sensitive = 60;
			else if(0x01 == netSetParamCfg.CfgInfo.struMotion.byMotionSenstive)
				runMdCfg.sensitive = 80;
			else 
				runMdCfg.sensitive = 100;
            netcam_md_set_sensitivity_percentage(0, runMdCfg.sensitive);

            HPRINT_INFO("SetMotionDetect: enable:%d, sensitive:%d\n", 
                    runMdCfg.enable, runMdCfg.sensitive);

            
#ifdef SDK_V13
			int j = 0;
            for (i = 0; i < 7; i++) {
                for (j = 0; j < 2; j++) {
                    #if 1
                    HPRINT_INFO("set day1[%d]-index[%d] time:[%u:%u] - [%u:%u]\n", i, j, 
                        netSetParamCfg.CfgInfo.struMotion.struAlarmTime[i][j].byStartHour,
                        netSetParamCfg.CfgInfo.struMotion.struAlarmTime[i][j].byStartMin,
                        netSetParamCfg.CfgInfo.struMotion.struAlarmTime[i][j].byStopHour,
                        netSetParamCfg.CfgInfo.struMotion.struAlarmTime[i][j].byStopMin);
                    #endif

                    #if 1
                    runMdCfg.scheduleTime[i][j].startHour = 
                        (SDK_S32)netSetParamCfg.CfgInfo.struMotion.struAlarmTime[i][j].byStartHour;
                    runMdCfg.scheduleTime[i][j].startMin = 
                        (SDK_S32)netSetParamCfg.CfgInfo.struMotion.struAlarmTime[i][j].byStartMin;
                    runMdCfg.scheduleTime[i][j].stopHour = 
                        (SDK_S32)netSetParamCfg.CfgInfo.struMotion.struAlarmTime[i][j].byStopHour;
                    runMdCfg.scheduleTime[i][j].stopMin = 
                        (SDK_S32)netSetParamCfg.CfgInfo.struMotion.struAlarmTime[i][j].byStopMin;
                    #endif
                    #if 0
                    HPRINT_INFO("set day2[%d]-index[%d] time:[%d:%d] - [%d:%d]\n", i, j, 
                        runMdCfg.scheduleTime[i][j].startHour, runMdCfg.scheduleTime[i][j].startMin,
                        runMdCfg.scheduleTime[i][j].stopHour, runMdCfg.scheduleTime[i][j].stopMin);
                    #endif
                }
            }
#endif
            runMdCfg.handle.recStreamNo = ntohl(netSetParamCfg.CfgInfo.struMotion.dwRelRecordChan);


            //?úμ2????
			int Shelter_enable = ntohl(netSetParamCfg.CfgInfo.dwEnableHide);
			for (i = 0; i < 4; i++) {
			    runChannelCfg.shelterRect[i].enable = Shelter_enable;
			    runChannelCfg.shelterRect[i].x = _data_to_float_rate(ntohs(netSetParamCfg.CfgInfo.struShelter[i].wHideAreaTopLeftX), _CMS_WIDTH);
				runChannelCfg.shelterRect[i].y = _data_to_float_rate(ntohs(netSetParamCfg.CfgInfo.struShelter[i].wHideAreaTopLeftY), _CMS_HEIGHT);
				runChannelCfg.shelterRect[i].width = _data_to_float_rate(ntohs(netSetParamCfg.CfgInfo.struShelter[i].wHideAreaWidth), _CMS_WIDTH);
				runChannelCfg.shelterRect[i].height = _data_to_float_rate(ntohs(netSetParamCfg.CfgInfo.struShelter[i].wHideAreaHeight), _CMS_HEIGHT);

#if 1
                HPRINT_INFO("Shelter Param: [%d-%d] (X:%d,Y:%d,W:%d,H:%d) -> (X:%2.2f,Y:%2.2f,W:%2.2f,H:%2.2f)\n",
				i, 
				ntohl(netSetParamCfg.CfgInfo.dwEnableHide),
				ntohs(netSetParamCfg.CfgInfo.struShelter[i].wHideAreaTopLeftX),
				ntohs(netSetParamCfg.CfgInfo.struShelter[i].wHideAreaTopLeftY),
				ntohs(netSetParamCfg.CfgInfo.struShelter[i].wHideAreaWidth),
				ntohs(netSetParamCfg.CfgInfo.struShelter[i].wHideAreaHeight),
                runChannelCfg.shelterRect[i].x,
                runChannelCfg.shelterRect[i].y,
                runChannelCfg.shelterRect[i].width ,
                runChannelCfg.shelterRect[i].height);
#endif
			    netcam_pm_set_cover(0, i, runChannelCfg.shelterRect[i]);
            }
            
            int const stream_cnt = netcam_video_get_channel_number();
            for (i = 0; i < stream_cnt; ++i) {
                osd_set_enable(i, 1, 0);
            }
            netcam_osd_update_title();
            netcam_osd_update_id();
            netcam_timer_add_task(/*netcam_osd_pm_save*/(TIMER_HANDLER)CfgSaveAll, NETCAM_TIMER_TWO_SEC, SDK_FALSE, SDK_TRUE);
#endif
		}
	}
	
	return sendNetRetval(connfd, retVal);
}


int netClientGetRtspPort(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{

	char buffer[76] = { 0 };		
	int length = 0;		//0x4c = 76

	NETRET_HEADER header;
	bzero(&header, sizeof(header));
	header.length = htonl(76); //htonl(sizeof(header));
	header.retVal = htonl(1);
	//header.checkSum = htonl(checkByteSum((char *)&header.retVal, ntohl(header.length) - 8));
	header.checkSum = htonl(863);
	memcpy(buffer, &header, sizeof(header));
	length += sizeof(header);

	char contextData[60] = {0};
	contextData[3] = 0x3c;	
	contextData[4] = 0x02;	//0x022a = 554
	contextData[5] = 0x2a;
	contextData[6] = 0x02;
	contextData[7] = 0x2a;
	contextData[8] = 0xf2;	//
	contextData[9] = 0x78;
	contextData[10] = 0x14;
	contextData[11] = 0xac;
	contextData[49] = 0x50;	//0x50 = 80
	contextData[51] = 0x50;
	memcpy(buffer + length, contextData, sizeof(contextData));
	length += sizeof(contextData);

	return writen(connfd, buffer, length);
}


int netClientGetVideoEffect(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	int nRetVal = NETRET_QUALIFIED;

	struct{
		NETRET_HEADER header;
		INTER_VIDEOPARA objEffect;
	}netVideoEffect;
	bzero(&netVideoEffect, sizeof(netVideoEffect));


	DMS_NET_CHANNEL_COLOR stChlColor;
	bzero(&stChlColor, sizeof(stChlColor));

    #if 0
	int nRet = dms_sysnetapi_ioctrl(g_hik_handle, DMS_NET_GET_COLORCFG, 0, &stChlColor, sizeof(DMS_NET_CHANNEL_COLOR));
	if (nRet != 0){
		HPRINT_INFO("HKvs-Protocol get channel color param failed, result %d", nRet);
		return -1;
	}

	netVideoEffect.objEffect.byBrightness = (stChlColor.nBrightness*255)/100;
	netVideoEffect.objEffect.byContrast = (stChlColor.nContrast*255)/100;
	netVideoEffect.objEffect.bySaturation = (stChlColor.nSaturation);	
	netVideoEffect.objEffect.byHue = (stChlColor.nHue*255)/100;

	netVideoEffect.header.length = htonl(sizeof(netVideoEffect));
	netVideoEffect.header.retVal = htonl(nRetVal);	
	netVideoEffect.header.checkSum = htonl(checkByteSum((char *)&(netVideoEffect.header.retVal), sizeof(netVideoEffect)-8));

    #else

    // 50/x = 100/255, x = 255*50/100 = 127
	netVideoEffect.objEffect.byBrightness = runImageCfg.brightness*255/100;
	netVideoEffect.objEffect.byContrast   = runImageCfg.contrast*255/100;
	netVideoEffect.objEffect.bySaturation = runImageCfg.saturation*255/100;	
	//netVideoEffect.objEffect.byHue        = runImageCfg.hue*255/100;


	netVideoEffect.header.length = htonl(sizeof(netVideoEffect));
	netVideoEffect.header.retVal = htonl(nRetVal);	
	netVideoEffect.header.checkSum = htonl(checkByteSum((char *)&(netVideoEffect.header.retVal), sizeof(netVideoEffect)-8));
    #endif
    
	return writen(connfd, (char *)&netVideoEffect, sizeof(netVideoEffect));
}

int netClientSetVideoEffect(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	INTER_VIDEOPARA objEffect;
	bzero(&objEffect, sizeof(INTER_VIDEOPARA));

	memcpy(&objEffect, recvbuff+sizeof(NETCMD_CHAN_HEADER), sizeof(INTER_VIDEOPARA));	

    #if 0
	DMS_NET_CHANNEL_COLOR stChlColor;
	bzero(&stChlColor, sizeof(stChlColor));
	int nRet = dms_sysnetapi_ioctrl(g_hik_handle, DMS_NET_GET_COLORCFG, 0, &stChlColor, sizeof(DMS_NET_CHANNEL_COLOR));
	if (nRet != 0){
		HPRINT_INFO("HKvs-Protocol get channel color param failed, result %d", nRet);
		return -1;
	}	
	stChlColor.nSaturation = objEffect.bySaturation;
	if((objEffect.byContrast*100)%255 > 127)
		stChlColor.nContrast = (objEffect.byContrast*100)/255+1;
	else
		stChlColor.nContrast = (objEffect.byContrast*100)/255;
	if((objEffect.byBrightness*100)%255 > 127)
		stChlColor.nBrightness = (objEffect.byBrightness*100)/255+1;
	else
		stChlColor.nBrightness = (objEffect.byBrightness*100)/255;
		
	nRet = dms_sysnetapi_ioctrl(g_hik_handle, DMS_NET_SET_COLORCFG, 0, &stChlColor, sizeof(DMS_NET_CHANNEL_COLOR));
	if (nRet != 0){
		HPRINT_INFO("HKvs-Protocol set channel color param failed, result %d", nRet);
		return -1;
	}
	dms_sysnetapi_ioctrl(g_hik_handle, DMS_NET_SET_SAVECFG, 0,NULL, 0);	
    #else
    int ret;
    GK_NET_IMAGE_CFG stImagingConfig;
    memset(&stImagingConfig, 0, sizeof(GK_NET_IMAGE_CFG));
    ret = netcam_image_get(&stImagingConfig);
    if(ret != 0)
    {
        HPRINT_ERR("CMS:fail to get imaging parameters.\n");
        return sendNetRetval(connfd, NETRET_NOT_SUPPORT);
    }
    HPRINT_INFO("set -- saturation:%u to %u, contrast:%u to %u, brightness:%u to %u, hue:%u to %u\n",
        stImagingConfig.saturation, objEffect.bySaturation,
        stImagingConfig.contrast, objEffect.byContrast,
        stImagingConfig.brightness, objEffect.byBrightness,
        stImagingConfig.hue, objEffect.byHue);


    stImagingConfig.saturation = objEffect.bySaturation * 100 / 255;
    stImagingConfig.contrast = objEffect.byContrast * 100 / 255;
    stImagingConfig.brightness = objEffect.byBrightness * 100 / 255;
    //stImagingConfig.hue = objEffect.byHue * 100 / 255;

    ret = netcam_image_set(stImagingConfig);
    if(ret != 0)
    {
        HPRINT_ERR("CMS: fail to set image attr.\n");
        return sendNetRetval(connfd, NETRET_NOT_SUPPORT);
    }

    #endif

	int nRetVal = NETRET_QUALIFIED;
    return sendNetRetval(connfd, nRetVal);

}

int netClientGetColorEnhance(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	NETCMD_HEADER  netCmdHeader;
	struct{
		NETRET_HEADER header;
		INTER_COLORENHANCE ColorEnInfo;;
	}netRetParamCfg;
	UINT32 retVal;
	int sendlen;
	
	sendlen = sizeof(netRetParamCfg);
	memset(&netRetParamCfg,0,sendlen);
	netRetParamCfg.header.length = htonl(sendlen);

	memcpy((char *)&netCmdHeader, recvbuff, sizeof(NETCMD_HEADER));
	retVal = verifyNetClientOperation(&netCmdHeader, pClientSockAddr, REMOTESHOWPARAMETER);
	if(retVal == NETRET_QUALIFIED)
	{
		netRetParamCfg.ColorEnInfo.unknown0 = htonl(0x00000098);
		netRetParamCfg.ColorEnInfo.unknown1 = 0x51;
		netRetParamCfg.ColorEnInfo.unknown2 = 0x40;
		netRetParamCfg.ColorEnInfo.unknown3 = 0x32;
		netRetParamCfg.ColorEnInfo.unknown4 = 0x64;
		netRetParamCfg.ColorEnInfo.unknown5 = 0x32;
		netRetParamCfg.ColorEnInfo.unknown6 = 0x64;
		netRetParamCfg.ColorEnInfo.unknown7 = 0x01;
		netRetParamCfg.ColorEnInfo.unknown8 = htonl(0x01320000);
		netRetParamCfg.ColorEnInfo.unknown9 = htonl(0x00009c40);
		netRetParamCfg.ColorEnInfo.unknown10 = htonl(0x000a0000);
		netRetParamCfg.ColorEnInfo.unknown11 = 0x02;
		netRetParamCfg.ColorEnInfo.unknown13 = 0x07;
		netRetParamCfg.ColorEnInfo.unknown14 = 0x12;
		netRetParamCfg.ColorEnInfo.unknown15 = htonl(0x00040500);
		netRetParamCfg.ColorEnInfo.unknown16 = htonl(0x000f0000);
		netRetParamCfg.ColorEnInfo.unknown17 = htonl(0x01323232);

        #if 0
		DMS_NET_CHANNEL_ENHANCED_COLOR EnhColor;
		memset(&EnhColor,0,sizeof(EnhColor));
		dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_GET_ENHANCED_COLOR,0,&EnhColor,sizeof(EnhColor));
		if(1 == EnhColor.nMirror)
			netRetParamCfg.ColorEnInfo.Mirror = 0x02;
		else if(2 == EnhColor.nMirror)
			netRetParamCfg.ColorEnInfo.Mirror = 0x01;
		else if(3 == EnhColor.nMirror)
			netRetParamCfg.ColorEnInfo.Mirror = 0x03;
		else
			netRetParamCfg.ColorEnInfo.Mirror = 0x00;
        #else
        int ret;
        GK_NET_IMAGE_CFG imagEnhancedAttr;
        memset(&imagEnhancedAttr, 0, sizeof(GK_NET_IMAGE_CFG));
        ret = netcam_image_get(&imagEnhancedAttr);
        if(ret != 0)
        {
            HPRINT_ERR("CMS: fail to get image attr.\n");
            return -1;
        }

        if(imagEnhancedAttr.flipEnabled == 1 && imagEnhancedAttr.mirrorEnabled == 1)
            netRetParamCfg.ColorEnInfo.Mirror = 3;
        else if(imagEnhancedAttr.flipEnabled == 0 && imagEnhancedAttr.mirrorEnabled == 1)
            netRetParamCfg.ColorEnInfo.Mirror = 1;
        else if(imagEnhancedAttr.flipEnabled == 1 && imagEnhancedAttr.mirrorEnabled == 0)
            netRetParamCfg.ColorEnInfo.Mirror = 2;
        else if(imagEnhancedAttr.flipEnabled == 0 && imagEnhancedAttr.mirrorEnabled == 0)
            netRetParamCfg.ColorEnInfo.Mirror = 0;


        #endif

		netRetParamCfg.ColorEnInfo.unknown18 = htonl(0x00000001);
		netRetParamCfg.ColorEnInfo.unknown19 = htonl(0x01580100);
		netRetParamCfg.ColorEnInfo.unknown20 = htonl(0x00320000);
		netRetParamCfg.ColorEnInfo.unknown21 = htonl(0x00323232);
		netRetParamCfg.ColorEnInfo.unknown22 = htonl(0x00320000);
		netRetParamCfg.ColorEnInfo.unknown23 = htonl(0x01000000);
		netRetParamCfg.ColorEnInfo.unknown24 = htonl(0x00640000);
		
	}
	netRetParamCfg.header.retVal = htonl(retVal);
	netRetParamCfg.header.checkSum = htonl(checkByteSum((char *)&(netRetParamCfg.header.retVal), sendlen-8));

	return writen(connfd, (char *)&netRetParamCfg, sendlen);
}

int netClientSetColorEnhance(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	struct{
        NETCMD_HEADER header;
		INTER_COLORENHANCE ColorEnInfo;;
	}netSetParamCfg;
	UINT32 retVal;

	memset(&netSetParamCfg,0, sizeof(netSetParamCfg));
	memcpy((char *)&netSetParamCfg, recvbuff, sizeof(netSetParamCfg));
	retVal = verifyNetClientOperation(&(netSetParamCfg.header), pClientSockAddr, REMOTESETPARAMETER);
	if(retVal == NETRET_QUALIFIED)
	{
	    #if 0
		DMS_NET_CHANNEL_COLOR_SINGLE SingleColor;
		memset(&SingleColor,0,sizeof(SingleColor));
		SingleColor.dwSize = sizeof(SingleColor);
		SingleColor.dwChannel = 0;
		SingleColor.dwSetFlag = DMS_COLOR_SET_MIRROR;
		SingleColor.dwSaveFlag = DMS_COLOR_SAVE;
		
		if(0x01 == netSetParamCfg.ColorEnInfo.Mirror)
			SingleColor.nValue = 2;
		else if(0x02 == netSetParamCfg.ColorEnInfo.Mirror)
			SingleColor.nValue = 1;
		else if(0x03 == netSetParamCfg.ColorEnInfo.Mirror)
			SingleColor.nValue = 3;
		else
			SingleColor.nValue = 0;
		dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_SET_COLORCFG_SINGLE,0,&SingleColor,sizeof(SingleColor));

        #else
        int ret;
        GK_NET_IMAGE_CFG imagEnhancedAttr;
        memset(&imagEnhancedAttr, 0, sizeof(GK_NET_IMAGE_CFG));
        ret = netcam_image_get(&imagEnhancedAttr);
        if(ret != 0)
        {
            HPRINT_ERR("CMS: fail to get image attr.\n");
            return -1;
        }


        if(0x01 == netSetParamCfg.ColorEnInfo.Mirror)
        {           
            imagEnhancedAttr.flipEnabled   = 0;
            imagEnhancedAttr.mirrorEnabled = 1;
        }
        else if(0x02 == netSetParamCfg.ColorEnInfo.Mirror)
        {
            imagEnhancedAttr.flipEnabled   = 1;
            imagEnhancedAttr.mirrorEnabled = 0;
        }
        else if(0x03 == netSetParamCfg.ColorEnInfo.Mirror)
        {
            imagEnhancedAttr.flipEnabled   = 1;
            imagEnhancedAttr.mirrorEnabled = 1;
        }
        else
        {
            imagEnhancedAttr.flipEnabled   = 0;
            imagEnhancedAttr.mirrorEnabled = 0;
        }

        ret = netcam_image_set(imagEnhancedAttr);
		if(ret != 0)
		{
			HPRINT_ERR("CMS: fail to set image attr.\n");
			return -1;
		}

        netcam_image_cfg_save();

        #endif
	}
	return sendNetRetval(connfd, retVal);
}

int netClientGetAlarmIncfg(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	NETCMD_HEADER  netCmdHeader;
	struct{
		NETRET_HEADER header;
		NETPARAM_ALARMIN_CFG_V30 alarmInCfg;
	}netRetAlarmInCfg;
	UINT32 retVal;

	int sendlen;
	
	sendlen = sizeof(netRetAlarmInCfg);
	memset(&netRetAlarmInCfg,0,sendlen);
	netRetAlarmInCfg.header.length = htonl(sendlen);

	memcpy((char *)&netCmdHeader, recvbuff, sizeof(NETCMD_HEADER));
	retVal = verifyNetClientOperation(&netCmdHeader, pClientSockAddr, REMOTESHOWPARAMETER);
	if(retVal == NETRET_QUALIFIED)
	{
	    #if 0
		DMS_NET_ALARMINCFG AlarmInCfg1;
		memset(&AlarmInCfg1,0,sizeof(AlarmInCfg1));
		dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_GET_ALARMINCFG,0,&AlarmInCfg1,sizeof(AlarmInCfg1));
		
		netRetAlarmInCfg.alarmInCfg.length = htonl(sizeof(NETPARAM_ALARMIN_CFG_V30));
		strcpy((char *)netRetAlarmInCfg.alarmInCfg.alarmIn.alarmInName,AlarmInCfg1.csAlarmInName);
		netRetAlarmInCfg.alarmInCfg.alarmIn.sensorType = AlarmInCfg1.byAlarmType;
		netRetAlarmInCfg.alarmInCfg.alarmIn.bEnableAlarmIn = AlarmInCfg1.byAlarmInHandle;
		netRetAlarmInCfg.alarmInCfg.alarmIn.alarmInAlarmHandleType.handleType |= htonl(UPTOCENTER);
		memcpy(netRetAlarmInCfg.alarmInCfg.alarmIn.armTime,AlarmInCfg1.stScheduleTime,sizeof(AlarmInCfg1.stScheduleTime));
        #endif
    }
	
	netRetAlarmInCfg.header.retVal = htonl(retVal);
	netRetAlarmInCfg.header.checkSum = htonl(checkByteSum((char *)&(netRetAlarmInCfg.header.retVal), sendlen-8));

	return writen(connfd, (char *)&netRetAlarmInCfg, sendlen);
}

int netClientSetAlarmIncfg(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	struct{
        NETCMD_HEADER header;
		UINT32 channel;
		NETPARAM_ALARMIN_CFG_V30 alarmInCfg;
	}netSetAlarmInCfg;
	UINT32 retVal;

	memset(&netSetAlarmInCfg,0, sizeof(netSetAlarmInCfg));
	memcpy((char *)&netSetAlarmInCfg, recvbuff, sizeof(netSetAlarmInCfg));
	retVal = verifyNetClientOperation(&(netSetAlarmInCfg.header), pClientSockAddr, REMOTESETPARAMETER);
	if(retVal == NETRET_QUALIFIED)
	{
		if(ntohl(netSetAlarmInCfg.alarmInCfg.length) != sizeof(NETPARAM_ALARMIN_CFG_V30))
		{	
			HPRINT_ERR("check length err\n");
			retVal = NETRET_ERROR_DATA;
		}
		else
		{
			#if 0
			DMS_NET_ALARMINCFG AlarmIn;
			memset(&AlarmIn,0,sizeof(AlarmIn));
			dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_GET_ALARMINCFG,0,&AlarmIn,sizeof(AlarmIn));
			strcpy(AlarmIn.csAlarmInName,(char *)netSetAlarmInCfg.alarmInCfg.alarmIn.alarmInName);
			AlarmIn.byAlarmType = netSetAlarmInCfg.alarmInCfg.alarmIn.sensorType;
			AlarmIn.byAlarmInHandle = netSetAlarmInCfg.alarmInCfg.alarmIn.bEnableAlarmIn;
			AlarmIn.stHandle.wActionMask |= DMS_ALARM_EXCEPTION_UPTOCENTER;
			
			memcpy(AlarmIn.stScheduleTime,netSetAlarmInCfg.alarmInCfg.alarmIn.armTime, sizeof(netSetAlarmInCfg.alarmInCfg.alarmIn.armTime));
			dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_SET_ALARMINCFG,0,&AlarmIn,sizeof(AlarmIn));
			dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_SET_SAVECFG,0,NULL,0);
			#endif
		}
	}
	return sendNetRetval(connfd, retVal);
}

static int HikTZToAniTZ(signed char cTimeDifferenceH,signed char cTimeDifferenceM,int *timezone)
{
	if(cTimeDifferenceH <= -5)
		*timezone = cTimeDifferenceH + 12;
	else if(-4 == cTimeDifferenceH)
	{	
		if(0 == cTimeDifferenceM)
			*timezone = cTimeDifferenceH + 13;
		else
			*timezone = cTimeDifferenceH + 12;
	}
	else if(-3 == cTimeDifferenceH)
	{	
		if(0 == cTimeDifferenceM)
			*timezone = cTimeDifferenceH + 14;
		else
			*timezone = cTimeDifferenceH + 13;
	}
	else if((cTimeDifferenceH >= -2) && (cTimeDifferenceH < 3))
	{
		*timezone = cTimeDifferenceH + 14;
	}
	else if(3 == cTimeDifferenceH)
	{	
		if(0 == cTimeDifferenceM)
			*timezone = cTimeDifferenceH + 14;
		else
			*timezone = cTimeDifferenceH + 15;
	}
	else if(4 == cTimeDifferenceH)
	{	
		if(0 == cTimeDifferenceM)
			*timezone = cTimeDifferenceH + 15;
		else
			*timezone = cTimeDifferenceH + 16;
	}
	else if(5 == cTimeDifferenceH)
	{	
		if(0 == cTimeDifferenceM)
			*timezone = cTimeDifferenceH + 16;
		else if(30 == cTimeDifferenceM)
			*timezone = cTimeDifferenceH + 17;
		else if(45 == cTimeDifferenceM)
			*timezone = cTimeDifferenceH + 18;
	}
	else if(6 == cTimeDifferenceH)
	{	
		if(0 == cTimeDifferenceM)
			*timezone = cTimeDifferenceH + 18;
		else
			*timezone = cTimeDifferenceH + 19;
	}
	else if((cTimeDifferenceH >= 7) && (cTimeDifferenceH <= 9) && (cTimeDifferenceM == 0))
	{
		*timezone = cTimeDifferenceH + 19;
	}
	else 
		*timezone = cTimeDifferenceH + 20;
	HPRINT_INFO("timezone:%d\n",*timezone);

	return 0;
}

static int AniTZToHikTZ(signed char * cTimeDifferenceH,signed char * cTimeDifferenceM,int timezone)
{
	if(timezone >0 && timezone <=7)
	{
		*cTimeDifferenceH = timezone-12;
		*cTimeDifferenceM = 0;
	}
	else if(8 == timezone)
	{
		*cTimeDifferenceH = -4;
		*cTimeDifferenceM = 30;
	}
	else if(9 == timezone)
	{
		*cTimeDifferenceH = -4;
		*cTimeDifferenceM = 0;
	}
	else if(10 == timezone)
	{
		*cTimeDifferenceH = -3;
		*cTimeDifferenceM = 30;
	}
	else if(11 == timezone)
	{
		*cTimeDifferenceH = -3;
		*cTimeDifferenceM = 0;
	}
	else if(timezone >11 && timezone <=16)
	{
		*cTimeDifferenceH = timezone-14;
		*cTimeDifferenceM = 0;
	}
	else if(17 == timezone)
	{
		*cTimeDifferenceH = 3;
		*cTimeDifferenceM = 0;
	}
	else if(18 == timezone)
	{
		*cTimeDifferenceH = 3;
		*cTimeDifferenceM = 30;
	}
	else if(19 == timezone)
	{
		*cTimeDifferenceH = 4;
		*cTimeDifferenceM = 0;
	}
	else if(20 == timezone)
	{
		*cTimeDifferenceH = 4;
		*cTimeDifferenceM = 30;
	}
	else if(21 == timezone)
	{
		*cTimeDifferenceH = 5;
		*cTimeDifferenceM = 0;
	}
	else if(22 == timezone)
	{
		*cTimeDifferenceH = 5;
		*cTimeDifferenceM = 30;
	}
	else if(23 == timezone)
	{
		*cTimeDifferenceH = 5;
		*cTimeDifferenceM = 45;
	}
	else if(24 == timezone)
	{
		*cTimeDifferenceH = 6;
		*cTimeDifferenceM = 0;
	}
	else if(25 == timezone)
	{
		*cTimeDifferenceH = 6;
		*cTimeDifferenceM = 30;
	}
	else if(timezone>= 26 && timezone<= 28)
	{
		*cTimeDifferenceH = timezone - 19;
		*cTimeDifferenceM = 0;
	}
	else if(29 == timezone)
	{
		*cTimeDifferenceH = 9;
		*cTimeDifferenceM = 30;
	}
	else 
	{
		*cTimeDifferenceH = timezone - 20;
		*cTimeDifferenceM = 0;
	}
		
	return 0;
}

int netClientGetNtpcfg(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	NETCMD_HEADER  netCmdHeader;
	struct{
		NETRET_HEADER header;
		INTER_NTPPARA NtpPara;;
	}netRetNtpCfg;
	UINT32 retVal;
	int sendlen;
	
	sendlen = sizeof(netRetNtpCfg);
	memset(&netRetNtpCfg,0,sendlen);
	netRetNtpCfg.header.length = htonl(sendlen);

	memcpy((char *)&netCmdHeader, recvbuff, sizeof(NETCMD_HEADER));
	retVal = verifyNetClientOperation(&netCmdHeader, pClientSockAddr, REMOTESHOWPARAMETER);
	if(retVal == NETRET_QUALIFIED)
	{
	    #if 0
		signed char TimeH=0,TimeM=0;
		DMS_NET_NTP_CFG NtpCfg;
		memset(&NtpCfg,0,sizeof(NtpCfg));
		dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_GET_NTPCFG,0,&NtpCfg,sizeof(NtpCfg));
		memcpy((char *)netRetNtpCfg.NtpPara.sNTPServer,NtpCfg.csNTPServer,sizeof(NtpCfg.csNTPServer));
		netRetNtpCfg.NtpPara.wInterval = htons(NtpCfg.wInterval);
		netRetNtpCfg.NtpPara.byEnableNTP = NtpCfg.byEnableNTP;

		DMS_NET_ZONEANDDST DstCfg;
		memset(&DstCfg,0,sizeof(DstCfg));
		dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_GET_ZONEANDDSTCFG,0,&DstCfg,sizeof(DstCfg));
		AniTZToHikTZ(&TimeH,&TimeM,DstCfg.nTimeZone);
		HPRINT_INFO("TimeH=%d,TimeM=%d\n",TimeH,TimeM);
		netRetNtpCfg.NtpPara.cTimeDifferenceH = TimeH;
		netRetNtpCfg.NtpPara.cTimeDifferenceM = TimeM;
		netRetNtpCfg.NtpPara.wNtpPort = htons(NtpCfg.wNtpPort);
        #else

		strncpy((char *)netRetNtpCfg.NtpPara.sNTPServer, runSystemCfg.ntpCfg.serverDomain, sizeof(netRetNtpCfg.NtpPara.sNTPServer));
		netRetNtpCfg.NtpPara.wInterval = htons(runSystemCfg.ntpCfg.sync_time / 3600);
		netRetNtpCfg.NtpPara.byEnableNTP = runSystemCfg.ntpCfg.enable;

		netRetNtpCfg.NtpPara.cTimeDifferenceH = 0;
		netRetNtpCfg.NtpPara.cTimeDifferenceM = 0;
		netRetNtpCfg.NtpPara.wNtpPort = htons(runSystemCfg.ntpCfg.port);

        #endif
	}
	netRetNtpCfg.header.retVal = htonl(retVal);
	netRetNtpCfg.header.checkSum = htonl(checkByteSum((char *)&(netRetNtpCfg.header.retVal), sendlen-8));

	return writen(connfd, (char *)&netRetNtpCfg, sendlen);
}

int netClientSetNtpcfg(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	struct{
        NETCMD_HEADER header;
		INTER_NTPPARA NtpPara;
	}netSetNtpCfg;
	UINT32 retVal;

	memset(&netSetNtpCfg,0, sizeof(netSetNtpCfg));
	memcpy((char *)&netSetNtpCfg, recvbuff, sizeof(netSetNtpCfg));
	retVal = verifyNetClientOperation(&(netSetNtpCfg.header), pClientSockAddr, REMOTESETPARAMETER);
	if(retVal == NETRET_QUALIFIED)
	{
		HPRINT_INFO("cTimeDifferenceH:%d,cTimeDifferenceM:%d\n",
			netSetNtpCfg.NtpPara.cTimeDifferenceH,netSetNtpCfg.NtpPara.cTimeDifferenceM);
        #if 0
		int timez=0;
		DMS_NET_NTP_CFG NtpCfg;
		memset(&NtpCfg,0,sizeof(NtpCfg));
		dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_GET_NTPCFG,0,&NtpCfg,sizeof(NtpCfg));
		memcpy(NtpCfg.csNTPServer,(char *)netSetNtpCfg.NtpPara.sNTPServer,sizeof(netSetNtpCfg.NtpPara.sNTPServer));
		NtpCfg.wInterval = ntohs(netSetNtpCfg.NtpPara.wInterval);
		NtpCfg.byEnableNTP = netSetNtpCfg.NtpPara.byEnableNTP;
		NtpCfg.cTimeDifferenceH = netSetNtpCfg.NtpPara.cTimeDifferenceH;
		NtpCfg.cTimeDifferenceM = netSetNtpCfg.NtpPara.cTimeDifferenceM;

		DMS_NET_ZONEANDDST DstCfg;
		memset(&DstCfg,0,sizeof(DstCfg));
		dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_GET_ZONEANDDSTCFG,0,&DstCfg,sizeof(DstCfg));
		HikTZToAniTZ(netSetNtpCfg.NtpPara.cTimeDifferenceH,netSetNtpCfg.NtpPara.cTimeDifferenceM,&timez);
		DstCfg.nTimeZone = timez;
		HPRINT_INFO("nTimeZone:%d\n",DstCfg.nTimeZone);
		dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_SET_ZONEANDDSTCFG,0,&DstCfg,sizeof(DstCfg));

		NtpCfg.wNtpPort = ntohs(netSetNtpCfg.NtpPara.wNtpPort);
		dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_SET_NTPCFG,0,&NtpCfg,sizeof(NtpCfg));
		dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_SET_SAVECFG,0,NULL,0);
        #else
        netcam_sys_ntp_start_stop(netSetNtpCfg.NtpPara.byEnableNTP);
        ntpc_set_addr(netSetNtpCfg.NtpPara.sNTPServer, sizeof(netSetNtpCfg.NtpPara.sNTPServer), ntohs(netSetNtpCfg.NtpPara.wNtpPort));
        ntpc_set_synctime(ntohs(netSetNtpCfg.NtpPara.wInterval) * 3600);
	
        #endif
	}
	
	return sendNetRetval(connfd, retVal);
}

int netClientGetDstcfg(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	NETCMD_HEADER  netCmdHeader;
	struct{
		NETRET_HEADER header;
		INTER_DSTPARA DstPara;
	}netRetDstCfg;
	UINT32 retVal;
	int sendlen;
	
	sendlen = sizeof(netRetDstCfg);
	memset(&netRetDstCfg,0,sendlen);
	netRetDstCfg.header.length = htonl(sendlen);

	memcpy((char *)&netCmdHeader, recvbuff, sizeof(NETCMD_HEADER));
	retVal = verifyNetClientOperation(&netCmdHeader, pClientSockAddr, REMOTESHOWPARAMETER);
	if(retVal == NETRET_QUALIFIED)
	{
	    #if 0
		DMS_NET_ZONEANDDST DstCfg;
		memset(&DstCfg,0,sizeof(DstCfg));
		dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_GET_ZONEANDDSTCFG,0,&DstCfg,sizeof(DstCfg));
		HPRINT_INFO("nTimeZone:%d\n",DstCfg.nTimeZone);
		netRetDstCfg.DstPara.dwSize = htonl(sizeof(INTER_DSTPARA));
		netRetDstCfg.DstPara.nTimeZone = htonl(DstCfg.nTimeZone);
		netRetDstCfg.DstPara.byDstMode = DstCfg.byDstMode;
		netRetDstCfg.DstPara.byStartDst = DstCfg.byStartDst;
		netRetDstCfg.DstPara.dwEnableDST = htonl(DstCfg.dwEnableDST);
		netRetDstCfg.DstPara.byDSTBias = DstCfg.byDSTBias;
		netRetDstCfg.DstPara.stBeginPoint.dwMonth = htonl(DstCfg.stBeginPoint.dwMonth);
		netRetDstCfg.DstPara.stBeginPoint.dwWeekNo = htonl(DstCfg.stBeginPoint.dwWeekNo);
		netRetDstCfg.DstPara.stBeginPoint.dwWeekDate = htonl(DstCfg.stBeginPoint.dwWeekDate);
		netRetDstCfg.DstPara.stBeginPoint.dwHour = htonl(DstCfg.stBeginPoint.dwHour);
		netRetDstCfg.DstPara.stBeginPoint.dwMin = htonl(DstCfg.stBeginPoint.dwMin);
		netRetDstCfg.DstPara.stEndPoint.dwMonth = htonl(DstCfg.stEndPoint.dwMonth);
		netRetDstCfg.DstPara.stEndPoint.dwWeekNo = htonl(DstCfg.stEndPoint.dwWeekNo);
		netRetDstCfg.DstPara.stEndPoint.dwWeekDate = htonl(DstCfg.stEndPoint.dwWeekDate);
		netRetDstCfg.DstPara.stEndPoint.dwHour = htonl(DstCfg.stEndPoint.dwHour);
		netRetDstCfg.DstPara.stEndPoint.dwMin = htonl(DstCfg.stEndPoint.dwMin);

        #endif
	}
	netRetDstCfg.header.retVal = htonl(retVal);
	netRetDstCfg.header.checkSum = htonl(checkByteSum((char *)&(netRetDstCfg.header.retVal), sendlen-8));

	return writen(connfd, (char *)&netRetDstCfg, sendlen);

}

int netClientSetDstcfg(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	struct{
        NETCMD_HEADER header;
		INTER_DSTPARA DstPara;
	}netSetDstCfg;
	UINT32 retVal;

	memset(&netSetDstCfg,0, sizeof(netSetDstCfg));
	memcpy((char *)&netSetDstCfg, recvbuff, sizeof(netSetDstCfg));
	retVal = verifyNetClientOperation(&(netSetDstCfg.header), pClientSockAddr, REMOTESETPARAMETER);
	if(retVal == NETRET_QUALIFIED)
	{
	    #if 0
		HPRINT_INFO("nTimeZone:%d\n",ntohl(netSetDstCfg.DstPara.nTimeZone));
		DMS_NET_ZONEANDDST DstCfg;
		memset(&DstCfg,0,sizeof(DstCfg));
		dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_GET_ZONEANDDSTCFG,0,&DstCfg,sizeof(DstCfg));
		DstCfg.nTimeZone = ntohl(netSetDstCfg.DstPara.nTimeZone);
		DstCfg.byDstMode = netSetDstCfg.DstPara.byDstMode;
		DstCfg.byStartDst = netSetDstCfg.DstPara.byStartDst;
		DstCfg.dwEnableDST = ntohl(netSetDstCfg.DstPara.dwEnableDST);
		DstCfg.stBeginPoint.dwMonth = ntohl(netSetDstCfg.DstPara.stBeginPoint.dwMonth);
		DstCfg.stBeginPoint.dwWeekNo = ntohl(netSetDstCfg.DstPara.stBeginPoint.dwWeekNo);
		DstCfg.stBeginPoint.dwWeekDate = ntohl(netSetDstCfg.DstPara.stBeginPoint.dwWeekDate);
		DstCfg.stBeginPoint.dwHour = ntohl(netSetDstCfg.DstPara.stBeginPoint.dwHour);
		DstCfg.stBeginPoint.dwMin = ntohl(netSetDstCfg.DstPara.stBeginPoint.dwMin);
		DstCfg.stEndPoint.dwMonth = ntohl(netSetDstCfg.DstPara.stEndPoint.dwMonth);
		DstCfg.stEndPoint.dwWeekNo = ntohl(netSetDstCfg.DstPara.stEndPoint.dwWeekNo);
		DstCfg.stEndPoint.dwWeekDate = ntohl(netSetDstCfg.DstPara.stEndPoint.dwWeekDate);
		DstCfg.stEndPoint.dwHour = ntohl(netSetDstCfg.DstPara.stEndPoint.dwHour);
		DstCfg.stEndPoint.dwMin = ntohl(netSetDstCfg.DstPara.stEndPoint.dwMin);
		dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_SET_ZONEANDDSTCFG,0,&DstCfg,sizeof(DstCfg));
		dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_SET_SAVECFG,0,NULL,0);
        #endif
	}
	
	return sendNetRetval(connfd, retVal);
}

int netClientSetSoftStore(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	int retVal = NETRET_QUALIFIED;

	//dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_SET_RESTORECFG,0,NULL,0);
	Gk_CmsRestore();

	return sendNetRetval(connfd, retVal);
}

int netClientSetReBoot(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	int retVal = NETRET_QUALIFIED;

	//dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_CMD_REBOOT,0,NULL,0);
    Gk_Reboot();

	return sendNetRetval(connfd, retVal);
}

int netClientGetNetWorkCfg(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	NETCMD_HEADER  netCmdHeader;
	struct{
		NETRET_HEADER header;
		INTER_NET_ETHER_CFG NetWorkPara;
	}netRetNetWorkCfg;
	UINT32 retVal;
	int sendlen;
	
	sendlen = sizeof(netRetNetWorkCfg);
	memset(&netRetNetWorkCfg,0,sendlen);
	netRetNetWorkCfg.header.length = htonl(sendlen);

	memcpy((char *)&netCmdHeader, recvbuff, sizeof(NETCMD_HEADER));
	retVal = verifyNetClientOperation(&netCmdHeader, pClientSockAddr, REMOTESHOWPARAMETER);
	if(retVal == NETRET_QUALIFIED)
	{
	    #if 0
		DMS_NET_NETWORK_CFG NetCfg;
		memset(&NetCfg,0,sizeof(NetCfg));
		dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_GET_NETCFG,0,&NetCfg,sizeof(NetCfg));
		netRetNetWorkCfg.NetWorkPara.dwSize = htonl(sizeof(INTER_NET_ETHER_CFG));
		netRetNetWorkCfg.NetWorkPara.stEtherNet[0].strIPAddr.U32IpV4 = htonl(inet_addr(NetCfg.stEtherNet[0].strIPAddr.csIpV4));
		netRetNetWorkCfg.NetWorkPara.stEtherNet[0].strIPMask.U32IpV4 = htonl(inet_addr(NetCfg.stEtherNet[0].strIPMask.csIpV4));
		netRetNetWorkCfg.NetWorkPara.stEtherNet[0].dwNetInterface = (NetCfg.stEtherNet[0].dwNetInterface==0) ? htonl(5):htonl(NetCfg.stEtherNet[0].dwNetInterface);
		netRetNetWorkCfg.NetWorkPara.stEtherNet[0].wDataPort = htons(NetCfg.stEtherNet[0].wDataPort);
		netRetNetWorkCfg.NetWorkPara.stEtherNet[0].wMTU = htons(NetCfg.stEtherNet[0].wMTU);
		memcpy(netRetNetWorkCfg.NetWorkPara.stEtherNet[0].macAddr,NetCfg.stEtherNet[0].byMACAddr,6);
		netRetNetWorkCfg.NetWorkPara.byIpV4Enable = NetCfg.byEnableDHCP;
		netRetNetWorkCfg.NetWorkPara.wMulticastSearchEnable = 0x101;
		netRetNetWorkCfg.NetWorkPara.stDnsServer1IpAddr.U32IpV4 = htonl(inet_addr(NetCfg.stDnsServer1IpAddr.csIpV4));
		if(strlen(NetCfg.stDnsServer2IpAddr.csIpV4) == 0)
			netRetNetWorkCfg.NetWorkPara.stDnsServer2IpAddr.U32IpV4 = 0;
		else
			netRetNetWorkCfg.NetWorkPara.stDnsServer2IpAddr.U32IpV4 = htonl(inet_addr(NetCfg.stDnsServer2IpAddr.csIpV4));
		netRetNetWorkCfg.NetWorkPara.wHttpPort = htons(NetCfg.wHttpPort);
		if(strlen(NetCfg.stMulticastIpAddr.csIpV4) == 0)
			netRetNetWorkCfg.NetWorkPara.stMulticastIpAddr.U32IpV4 = 0;
		else
			netRetNetWorkCfg.NetWorkPara.stMulticastIpAddr.U32IpV4 = htonl(inet_addr(NetCfg.stMulticastIpAddr.csIpV4));
		netRetNetWorkCfg.NetWorkPara.stGatewayIpAddr.U32IpV4 = htonl(inet_addr(NetCfg.stGatewayIpAddr.csIpV4));

		DMS_NET_PPPOECFG PPPoECfg;
		memset(&PPPoECfg,0,sizeof(PPPoECfg));
		dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_GET_PPPOECFG,0,&PPPoECfg,sizeof(PPPoECfg));
		netRetNetWorkCfg.NetWorkPara.dwPPPoEEnable = htonl(PPPoECfg.bPPPoEEnable);
		memcpy(netRetNetWorkCfg.NetWorkPara.csPPPoEUser,PPPoECfg.csPPPoEUser,strlen(PPPoECfg.csPPPoEUser));
		memcpy(netRetNetWorkCfg.NetWorkPara.csPPPoEPassword,PPPoECfg.csPPPoEPassword,strlen(PPPoECfg.csPPPoEPassword));
        #else

        ST_SDK_NETWORK_ATTR net_attr;
        sprintf(net_attr.name,"eth0");

        if(netcam_net_get(&net_attr) != 0)
        {
            HPRINT_ERR("netcam_net_get error\n");
        }

        netRetNetWorkCfg.NetWorkPara.dwSize = htonl(sizeof(INTER_NET_ETHER_CFG));
        
        netRetNetWorkCfg.NetWorkPara.stEtherNet[0].strIPAddr.U32IpV4 = htonl(inet_addr((char *)net_attr.ip));
		netRetNetWorkCfg.NetWorkPara.stEtherNet[0].strIPMask.U32IpV4 = htonl(inet_addr((char *)net_attr.mask));
		netRetNetWorkCfg.NetWorkPara.stEtherNet[0].dwNetInterface = htonl(5);
		netRetNetWorkCfg.NetWorkPara.stEtherNet[0].wDataPort = htons(8200);
		netRetNetWorkCfg.NetWorkPara.stEtherNet[0].wMTU = htons(1500);
        
        get_hw_array(netRetNetWorkCfg.NetWorkPara.stEtherNet[0].macAddr, net_attr.mac);
        
		netRetNetWorkCfg.NetWorkPara.byIpV4Enable = net_attr.dhcp;
		netRetNetWorkCfg.NetWorkPara.wMulticastSearchEnable = 0x101;
		netRetNetWorkCfg.NetWorkPara.stDnsServer1IpAddr.U32IpV4 = htonl(inet_addr((char *)net_attr.dns1));
		if(strlen(net_attr.dns2) == 0)
			netRetNetWorkCfg.NetWorkPara.stDnsServer2IpAddr.U32IpV4 = 0;
		else
			netRetNetWorkCfg.NetWorkPara.stDnsServer2IpAddr.U32IpV4 = htonl(inet_addr((char *)net_attr.dns2));
		netRetNetWorkCfg.NetWorkPara.wHttpPort = htons(net_attr.httpPort);
		//if(strlen(NetCfg.stMulticastIpAddr.csIpV4) == 0)
			netRetNetWorkCfg.NetWorkPara.stMulticastIpAddr.U32IpV4 = 0;
		//else
			//netRetNetWorkCfg.NetWorkPara.stMulticastIpAddr.U32IpV4 = htonl(inet_addr("192.168.10.255"));
		netRetNetWorkCfg.NetWorkPara.stGatewayIpAddr.U32IpV4 = htonl(inet_addr((char *)net_attr.gateway));

        #if 0
		QMAPI_NET_PPPOECFG PPPoECfg;
		memset(&PPPoECfg,0,sizeof(PPPoECfg));
		QMapi_sys_ioctrl(g_hik_handle,QMAPI_SYSCFG_GET_PPPOECFG,0,&PPPoECfg,sizeof(PPPoECfg));
		netRetNetWorkCfg.NetWorkPara.dwPPPoEEnable = htonl(PPPoECfg.bPPPoEEnable);
		memcpy(netRetNetWorkCfg.NetWorkPara.csPPPoEUser,PPPoECfg.csPPPoEUser,strlen(PPPoECfg.csPPPoEUser));
		memcpy(netRetNetWorkCfg.NetWorkPara.csPPPoEPassword,PPPoECfg.csPPPoEPassword,strlen(PPPoECfg.csPPPoEPassword));
        #else

		netRetNetWorkCfg.NetWorkPara.dwPPPoEEnable = htonl(0);
		//memcpy(netRetNetWorkCfg.NetWorkPara.csPPPoEUser,PPPoECfg.csPPPoEUser,strlen(PPPoECfg.csPPPoEUser));
		//memcpy(netRetNetWorkCfg.NetWorkPara.csPPPoEPassword,PPPoECfg.csPPPoEPassword,strlen(PPPoECfg.csPPPoEPassword));
       
        #endif
        #endif
    }
	netRetNetWorkCfg.header.retVal = htonl(retVal);
	netRetNetWorkCfg.header.checkSum = htonl(checkByteSum((char *)&(netRetNetWorkCfg.header.retVal), sendlen-8));

	return writen(connfd, (char *)&netRetNetWorkCfg, sendlen);
}

int netClientSetNetWorkCfg(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	struct{
        NETCMD_HEADER header;
		INTER_NET_ETHER_CFG NetWorkPara;
	}netSetNetWorkCfg;
	UINT32 retVal;

	memset(&netSetNetWorkCfg,0, sizeof(netSetNetWorkCfg));
	memcpy((char *)&netSetNetWorkCfg, recvbuff, sizeof(netSetNetWorkCfg));
	retVal = verifyNetClientOperation(&(netSetNetWorkCfg.header), pClientSockAddr, REMOTESETPARAMETER);
	if(retVal == NETRET_QUALIFIED)
	{
		HPRINT_INFO("strIPAddr:%08X  ",ntohl(netSetNetWorkCfg.NetWorkPara.stEtherNet[0].strIPAddr.U32IpV4));
		HPRINT_INFO("strIPMask:%08X  ",ntohl(netSetNetWorkCfg.NetWorkPara.stEtherNet[0].strIPMask.U32IpV4));
		HPRINT_INFO("Enable:%d\n",netSetNetWorkCfg.NetWorkPara.byIpV4Enable);

        #if 0
		UINT32	IpV4addr;
		struct in_addr addr1;
		DMS_NET_NETWORK_CFG NetCfg;
		memset(&NetCfg,0,sizeof(NetCfg));
		dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_GET_NETCFG,0,&NetCfg,sizeof(NetCfg));
		
		memset(&addr1,0,sizeof(addr1));

		IpV4addr = ntohl(netSetNetWorkCfg.NetWorkPara.stEtherNet[0].strIPAddr.U32IpV4);
		memcpy(&addr1,&IpV4addr,4);
		strcpy(NetCfg.stEtherNet[0].strIPAddr.csIpV4,inet_ntoa(addr1));

		IpV4addr = ntohl(netSetNetWorkCfg.NetWorkPara.stEtherNet[0].strIPMask.U32IpV4);
		memcpy(&addr1,&IpV4addr,4);
		strcpy(NetCfg.stEtherNet[0].strIPMask.csIpV4,inet_ntoa(addr1));

		NetCfg.stEtherNet[0].dwNetInterface = ntohl(netSetNetWorkCfg.NetWorkPara.stEtherNet[0].dwNetInterface);
		NetCfg.stEtherNet[0].wDataPort = ntohs(netSetNetWorkCfg.NetWorkPara.stEtherNet[0].wDataPort);
		NetCfg.stEtherNet[0].wMTU = ntohs(netSetNetWorkCfg.NetWorkPara.stEtherNet[0].wMTU);
		memcpy(NetCfg.stEtherNet[0].byMACAddr,netSetNetWorkCfg.NetWorkPara.stEtherNet[0].macAddr,6);
		NetCfg.byEnableDHCP = netSetNetWorkCfg.NetWorkPara.byIpV4Enable;

		IpV4addr = ntohl(netSetNetWorkCfg.NetWorkPara.stDnsServer1IpAddr.U32IpV4);
		memcpy(&addr1,&IpV4addr,4);
		strcpy(NetCfg.stDnsServer1IpAddr.csIpV4,inet_ntoa(addr1));

		IpV4addr = ntohl(netSetNetWorkCfg.NetWorkPara.stDnsServer2IpAddr.U32IpV4);
		memcpy(&addr1,&IpV4addr,4);
		strcpy(NetCfg.stDnsServer2IpAddr.csIpV4,inet_ntoa(addr1));

		NetCfg.wHttpPort = ntohs(netSetNetWorkCfg.NetWorkPara.wHttpPort);

		IpV4addr = ntohl(netSetNetWorkCfg.NetWorkPara.stMulticastIpAddr.U32IpV4);
		memcpy(&addr1,&IpV4addr,4);
		strcpy(NetCfg.stMulticastIpAddr.csIpV4,inet_ntoa(addr1));

		IpV4addr = ntohl(netSetNetWorkCfg.NetWorkPara.stGatewayIpAddr.U32IpV4);
		memcpy(&addr1,&IpV4addr,4);
		strcpy(NetCfg.stGatewayIpAddr.csIpV4,inet_ntoa(addr1));

		DMS_NET_PPPOECFG PPPoECfg;
		memset(&PPPoECfg,0,sizeof(PPPoECfg));
		dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_GET_PPPOECFG,0,&PPPoECfg,sizeof(PPPoECfg));
		PPPoECfg.bPPPoEEnable = ntohl(netSetNetWorkCfg.NetWorkPara.dwPPPoEEnable);
		memcpy(PPPoECfg.csPPPoEUser,netSetNetWorkCfg.NetWorkPara.csPPPoEUser,strlen(netSetNetWorkCfg.NetWorkPara.csPPPoEUser));
		memcpy(PPPoECfg.csPPPoEPassword,netSetNetWorkCfg.NetWorkPara.csPPPoEPassword,strlen(netSetNetWorkCfg.NetWorkPara.csPPPoEPassword));
		dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_SET_PPPOECFG,0,&PPPoECfg,sizeof(PPPoECfg));

		sendNetRetval(connfd, retVal);	
		dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_SET_NETCFG,0,&NetCfg,sizeof(NetCfg));
		dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_SET_SAVECFG,0,NULL,0);
        
		#else
        UINT32	IpV4addr;
		struct in_addr addr1;
		memset(&addr1,0,sizeof(addr1));
        
        ST_SDK_NETWORK_ATTR net_attr;
        sprintf(net_attr.name,"eth0");
        net_attr.enable = 1;
        netcam_net_get(&net_attr);

		IpV4addr = ntohl(netSetNetWorkCfg.NetWorkPara.stEtherNet[0].strIPAddr.U32IpV4);
		memcpy(&addr1,&IpV4addr,4);
		strcpy(net_attr.ip, inet_ntoa(addr1));

		IpV4addr = ntohl(netSetNetWorkCfg.NetWorkPara.stEtherNet[0].strIPMask.U32IpV4);
		memcpy(&addr1,&IpV4addr,4);
		strcpy(net_attr.mask, inet_ntoa(addr1));

		//NetCfg.stEtherNet[0].dwNetInterface = ntohl(netSetNetWorkCfg.NetWorkPara.stEtherNet[0].dwNetInterface);
		//NetCfg.stEtherNet[0].wDataPort = ntohs(netSetNetWorkCfg.NetWorkPara.stEtherNet[0].wDataPort);
		//NetCfg.stEtherNet[0].wMTU = ntohs(netSetNetWorkCfg.NetWorkPara.stEtherNet[0].wMTU);
		//memcpy(NetCfg.stEtherNet[0].byMACAddr,netSetNetWorkCfg.NetWorkPara.stEtherNet[0].macAddr,6);

        sprintf((char *)net_attr.mac, "%02X:%02X:%02X:%02X:%02X:%02X", 
            netSetNetWorkCfg.NetWorkPara.stEtherNet[0].macAddr[0],
            netSetNetWorkCfg.NetWorkPara.stEtherNet[0].macAddr[1],
            netSetNetWorkCfg.NetWorkPara.stEtherNet[0].macAddr[2],
            netSetNetWorkCfg.NetWorkPara.stEtherNet[0].macAddr[3],
            netSetNetWorkCfg.NetWorkPara.stEtherNet[0].macAddr[4],
            netSetNetWorkCfg.NetWorkPara.stEtherNet[0].macAddr[5]);
		net_attr.dhcp =(int)netSetNetWorkCfg.NetWorkPara.byIpV4Enable;

		IpV4addr = ntohl(netSetNetWorkCfg.NetWorkPara.stDnsServer1IpAddr.U32IpV4);
		memcpy(&addr1,&IpV4addr,4);
		strcpy(net_attr.dns1,inet_ntoa(addr1));

		IpV4addr = ntohl(netSetNetWorkCfg.NetWorkPara.stDnsServer2IpAddr.U32IpV4);
		memcpy(&addr1,&IpV4addr,4);
		strcpy(net_attr.dns2,inet_ntoa(addr1));

		//NetCfg.wHttpPort = ntohs(netSetNetWorkCfg.NetWorkPara.wHttpPort);

		//IpV4addr = ntohl(netSetNetWorkCfg.NetWorkPara.stMulticastIpAddr.U32IpV4);
		//memcpy(&addr1,&IpV4addr,4);
		//strcpy(NetCfg.stMulticastIpAddr.csIpV4,inet_ntoa(addr1));

		IpV4addr = ntohl(netSetNetWorkCfg.NetWorkPara.stGatewayIpAddr.U32IpV4);
		memcpy(&addr1,&IpV4addr,4);
		strcpy(net_attr.gateway,inet_ntoa(addr1));

        if (netcam_net_set(&net_attr) == 0) {
            netcam_net_cfg_save();
        }


		//QMAPI_NET_PPPOECFG PPPoECfg;
		//memset(&PPPoECfg,0,sizeof(PPPoECfg));
		//QMapi_sys_ioctrl(g_hik_handle,QMAPI_SYSCFG_GET_PPPOECFG,0,&PPPoECfg,sizeof(PPPoECfg));
		//PPPoECfg.bPPPoEEnable = ntohl(netSetNetWorkCfg.NetWorkPara.dwPPPoEEnable);
		//memcpy(PPPoECfg.csPPPoEUser,netSetNetWorkCfg.NetWorkPara.csPPPoEUser,strlen(netSetNetWorkCfg.NetWorkPara.csPPPoEUser));
		//memcpy(PPPoECfg.csPPPoEPassword,netSetNetWorkCfg.NetWorkPara.csPPPoEPassword,strlen(netSetNetWorkCfg.NetWorkPara.csPPPoEPassword));
		//QMapi_sys_ioctrl(g_hik_handle,QMAPI_SYSCFG_SET_PPPOECFG,0,&PPPoECfg,sizeof(PPPoECfg));

		sendNetRetval(connfd, retVal);	
		//QMapi_sys_ioctrl(g_hik_handle,QMAPI_SYSCFG_SET_NETCFG,0,&NetCfg,sizeof(NetCfg));
		//QMapi_sys_ioctrl(g_hik_handle,QMAPI_SYSCFG_SET_SAVECFG_ASYN,0,NULL,0);
        
   
        #endif
        
	}
	
	return 0;
}

static int Hik_CheckNtpServerType(char FTPIpAddress[64])
{
	int NtpServerType = 0;
	char *p1 = strstr(FTPIpAddress,".");
	if(p1 == NULL)
	{
		HPRINT_ERR("FTPIpAddress err\n");
		goto exit;
	}
	
	char *p2 = strstr(p1,".");
	if(p2 == NULL)
	{
		HPRINT_ERR("FTPIpAddress err\n");
		goto exit;
	}

	p1 = strstr(p2,".");
	if(p1 == NULL)
		NtpServerType = 1;		
	
exit:
	return NtpServerType;
}

int netClientGetFtpCfg(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	NETCMD_HEADER  netCmdHeader;
	struct{
		NETRET_HEADER header;
		INTER_NET_FTP_CFG FtpPara;
	}netRetFtpCfg;
	UINT32 retVal;
	int sendlen;
	
	sendlen = sizeof(netRetFtpCfg);
	memset(&netRetFtpCfg,0,sendlen);
	netRetFtpCfg.header.length = htonl(sendlen);

	memcpy((char *)&netCmdHeader, recvbuff, sizeof(NETCMD_HEADER));
	retVal = verifyNetClientOperation(&netCmdHeader, pClientSockAddr, REMOTESHOWPARAMETER);
	if(retVal == NETRET_QUALIFIED)
	{
	    #if 0
		DMS_NET_FTP_PARAM Ftpcfg;
		memset(&Ftpcfg,0,sizeof(Ftpcfg));
		dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_GET_FTPCFG,0,&Ftpcfg,sizeof(Ftpcfg));
		netRetFtpCfg.FtpPara.dwSize = htonl(sizeof(INTER_NET_FTP_CFG));
		netRetFtpCfg.FtpPara.byEnableFTP = Ftpcfg.bEnableFTP;
		netRetFtpCfg.FtpPara.byFTPPort = Ftpcfg.dwFTPPort;
		memcpy(netRetFtpCfg.FtpPara.csFTPIpAddress,Ftpcfg.csFTPIpAddress,strlen(Ftpcfg.csFTPIpAddress));
		memcpy(netRetFtpCfg.FtpPara.csUserName,Ftpcfg.csUserName,strlen(Ftpcfg.csUserName));
		if(strlen(Ftpcfg.csPassword) > PASSWD_LEN)
			memcpy(netRetFtpCfg.FtpPara.csPassword,Ftpcfg.csPassword,PASSWD_LEN);
		else
			memcpy(netRetFtpCfg.FtpPara.csPassword,Ftpcfg.csPassword,strlen(Ftpcfg.csPassword));
		if((Ftpcfg.wTopDirMode == 0x00) && (Ftpcfg.wSubDirMode == 0x00))
		{
			netRetFtpCfg.FtpPara.byDirMode = 0x2;
			netRetFtpCfg.FtpPara.byTopDirMode = 0x3;
			netRetFtpCfg.FtpPara.bySubDirMode = 0x2;
		}
		else if((Ftpcfg.wTopDirMode == 0x00) && (Ftpcfg.wSubDirMode == 0x01))
		{
			netRetFtpCfg.FtpPara.byDirMode = 0x2;
			netRetFtpCfg.FtpPara.byTopDirMode = 0x3;
			netRetFtpCfg.FtpPara.bySubDirMode = 0x1;
		}
		else if((Ftpcfg.wTopDirMode == 0x00) && (Ftpcfg.wSubDirMode == 0xFF))
		{
			netRetFtpCfg.FtpPara.byDirMode = 0x1;
			netRetFtpCfg.FtpPara.byTopDirMode = 0x3;
			netRetFtpCfg.FtpPara.bySubDirMode = 0x1;
		}
		else if((Ftpcfg.wTopDirMode == 0x1) && (Ftpcfg.wSubDirMode == 0x0))
		{
			netRetFtpCfg.FtpPara.byDirMode = 0x2;
			netRetFtpCfg.FtpPara.byTopDirMode = 0x1;
			netRetFtpCfg.FtpPara.bySubDirMode = 0x2;
		}
		else if((Ftpcfg.wTopDirMode == 0x1) && (Ftpcfg.wSubDirMode == 0x1))
		{
			netRetFtpCfg.FtpPara.byDirMode = 0x2;
			netRetFtpCfg.FtpPara.byTopDirMode = 0x1;
			netRetFtpCfg.FtpPara.bySubDirMode = 0x1;
		}
		else if((Ftpcfg.wTopDirMode == 0x1) && (Ftpcfg.wSubDirMode == 0xFF))
		{
			netRetFtpCfg.FtpPara.byDirMode = 0x1;
			netRetFtpCfg.FtpPara.byTopDirMode = 0x1;
			netRetFtpCfg.FtpPara.bySubDirMode = 0x1;
		}
		else
		{
			netRetFtpCfg.FtpPara.byDirMode = 0x0;
			netRetFtpCfg.FtpPara.byTopDirMode = 0x1;
			netRetFtpCfg.FtpPara.bySubDirMode = 0x1;
		}
		netRetFtpCfg.FtpPara.byServerType = Hik_CheckNtpServerType(Ftpcfg.csFTPIpAddress);
        #else
        netRetFtpCfg.FtpPara.dwSize = htonl(sizeof(INTER_NET_FTP_CFG));
		netRetFtpCfg.FtpPara.byEnableFTP = 0;
        #endif
		
	}
	netRetFtpCfg.header.retVal = htonl(retVal);
	netRetFtpCfg.header.checkSum = htonl(checkByteSum((char *)&(netRetFtpCfg.header.retVal), sendlen-8));

	return writen(connfd, (char *)&netRetFtpCfg, sendlen);
}

int netClientSetFtpCfg(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	struct{
        NETCMD_HEADER header;
		INTER_NET_FTP_CFG FtpPara;
	}netSetFtpCfg;
	UINT32 retVal;

	memset(&netSetFtpCfg,0, sizeof(netSetFtpCfg));
	memcpy((char *)&netSetFtpCfg, recvbuff, sizeof(netSetFtpCfg));
	retVal = verifyNetClientOperation(&(netSetFtpCfg.header), pClientSockAddr, REMOTESETPARAMETER);
	if(retVal == NETRET_QUALIFIED)
	{
	    #if 0
		DMS_NET_FTP_PARAM Ftpcfg;
		memset(&Ftpcfg,0,sizeof(Ftpcfg));
		dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_GET_FTPCFG,0,&Ftpcfg,sizeof(Ftpcfg));
		Ftpcfg.bEnableFTP = netSetFtpCfg.FtpPara.byEnableFTP;
		memcpy(Ftpcfg.csFTPIpAddress,netSetFtpCfg.FtpPara.csFTPIpAddress,strlen(netSetFtpCfg.FtpPara.csFTPIpAddress));
		Ftpcfg.dwFTPPort = netSetFtpCfg.FtpPara.byFTPPort;
		memcpy(Ftpcfg.csUserName,netSetFtpCfg.FtpPara.csUserName,strlen(netSetFtpCfg.FtpPara.csUserName));
		memcpy(Ftpcfg.csPassword,netSetFtpCfg.FtpPara.csPassword,strlen(netSetFtpCfg.FtpPara.csPassword));
		if(netSetFtpCfg.FtpPara.byDirMode == 0x0)
		{
			Ftpcfg.wTopDirMode = 0xFF;
			Ftpcfg.wSubDirMode = 0xFF;
		}
		if((netSetFtpCfg.FtpPara.byDirMode == 0x1) && (netSetFtpCfg.FtpPara.byTopDirMode == 0x3))
		{
			Ftpcfg.wTopDirMode = 0x0;
			Ftpcfg.wSubDirMode = 0xFF;
		}
		else if((netSetFtpCfg.FtpPara.byDirMode == 0x1) && (netSetFtpCfg.FtpPara.byTopDirMode == 0x1))
		{
			Ftpcfg.wTopDirMode = 0x1;
			Ftpcfg.wSubDirMode = 0xFF;
		}
		else if((netSetFtpCfg.FtpPara.byDirMode == 0x2) && (netSetFtpCfg.FtpPara.byTopDirMode == 0x3) && (netSetFtpCfg.FtpPara.bySubDirMode == 0x2))
		{
			Ftpcfg.wTopDirMode = 0x0;
			Ftpcfg.wSubDirMode = 0x0;
		}
		else if((netSetFtpCfg.FtpPara.byDirMode == 0x2) && (netSetFtpCfg.FtpPara.byTopDirMode == 0x3) && (netSetFtpCfg.FtpPara.bySubDirMode == 0x1))
		{
			Ftpcfg.wTopDirMode = 0x0;
			Ftpcfg.wSubDirMode = 0x1;
		}
		else if((netSetFtpCfg.FtpPara.byDirMode == 0x2) && (netSetFtpCfg.FtpPara.byTopDirMode == 0x1) && (netSetFtpCfg.FtpPara.bySubDirMode == 0x2))
		{
			Ftpcfg.wTopDirMode = 0x1;
			Ftpcfg.wSubDirMode = 0x0;
		}
		else if((netSetFtpCfg.FtpPara.byDirMode == 0x2) && (netSetFtpCfg.FtpPara.byTopDirMode == 0x1) && (netSetFtpCfg.FtpPara.bySubDirMode == 0x1))
		{
			Ftpcfg.wTopDirMode = 0x1;
			Ftpcfg.wSubDirMode = 0x1;
		}
		else
		{
			Ftpcfg.wTopDirMode = 0x0;
			Ftpcfg.wSubDirMode = 0x0;
		}
        #endif
	}	
	return sendNetRetval(connfd, retVal);
}

int netClientGetRtspCfg(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	NETCMD_HEADER  netCmdHeader;
	struct{
		NETRET_HEADER header;
		INTER_NET_RTSP_CFG RtspPara;
	}netRetRtspCfg;
	UINT32 retVal;
	int sendlen;
	
	sendlen = sizeof(netRetRtspCfg);
	memset(&netRetRtspCfg,0,sendlen);
	netRetRtspCfg.header.length = htonl(sendlen);

	memcpy((char *)&netCmdHeader, recvbuff, sizeof(NETCMD_HEADER));
	retVal = verifyNetClientOperation(&netCmdHeader, pClientSockAddr, REMOTESHOWPARAMETER);
	if(retVal == NETRET_QUALIFIED)
	{
	    #if 0
		DMS_NET_RTSP_CFG RtspCfg;
		memset(&RtspCfg,0,sizeof(RtspCfg));
		dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_GET_RTSPCFG,0,&RtspCfg,sizeof(RtspCfg));
		netRetRtspCfg.RtspPara.dwSize = htonl(sizeof(INTER_NET_RTSP_CFG));
		netRetRtspCfg.RtspPara.wPort = htons(RtspCfg.dwPort);
        #else
        netRetRtspCfg.RtspPara.dwSize = htonl(sizeof(INTER_NET_RTSP_CFG));
		netRetRtspCfg.RtspPara.wPort = htons(554);
        #endif
	}
	netRetRtspCfg.header.retVal = htonl(retVal);
	netRetRtspCfg.header.checkSum = htonl(checkByteSum((char *)&(netRetRtspCfg.header.retVal), sendlen-8));

	return writen(connfd, (char *)&netRetRtspCfg, sendlen);
}

int netClientSetRtspCfg(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	struct{
        NETCMD_HEADER header;
		INTER_NET_RTSP_CFG RtspPara;
	}netSetRtspCfg;
	UINT32 retVal;

	memset(&netSetRtspCfg,0, sizeof(netSetRtspCfg));
	memcpy((char *)&netSetRtspCfg, recvbuff, sizeof(netSetRtspCfg));
	retVal = verifyNetClientOperation(&(netSetRtspCfg.header), pClientSockAddr, REMOTESETPARAMETER);
	if(retVal == NETRET_QUALIFIED)
	{
		HPRINT_INFO("wPort:%d\n",ntohs(netSetRtspCfg.RtspPara.wPort));
        #if 0
		DMS_NET_RTSP_CFG RtspCfg;
		memset(&RtspCfg,0,sizeof(RtspCfg));
		dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_GET_RTSPCFG,0,&RtspCfg,sizeof(RtspCfg));
		RtspCfg.dwPort = ntohs(netSetRtspCfg.RtspPara.wPort);
		dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_SET_RTSPCFG,0,&RtspCfg,sizeof(RtspCfg));
		dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_SET_SAVECFG,0,NULL,0);
		#endif
	}
	
	return sendNetRetval(connfd, retVal);
}

int netClientTcpPrev(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	NETCMD_HEADER  netCmdHeader;
	struct{
		NETRET_HEADER header;
		INTER_NET_P2PTCP_CFG tcpPara;
	}netRetTcpCfg;
	UINT32 retVal;
	int sendlen;
	
	sendlen = sizeof(netRetTcpCfg);
	memset(&netRetTcpCfg,0,sendlen);
	netRetTcpCfg.header.length = htonl(sendlen);

	memcpy((char *)&netCmdHeader, recvbuff, sizeof(NETCMD_HEADER));
	retVal = verifyNetClientOperation(&netCmdHeader, pClientSockAddr, REMOTESHOWPARAMETER);
	if(retVal == NETRET_QUALIFIED)
	{
		netRetTcpCfg.tcpPara.length = htonl(sizeof(netRetTcpCfg.tcpPara.byFileHead));
		netRetTcpCfg.tcpPara.byFileHead[0] = 0x49;
		netRetTcpCfg.tcpPara.byFileHead[1] = 0x4d;
		netRetTcpCfg.tcpPara.byFileHead[2] = 0x4b;
		netRetTcpCfg.tcpPara.byFileHead[3] = 0x48;
		netRetTcpCfg.tcpPara.byFileHead[4] = 0x01;
		netRetTcpCfg.tcpPara.byFileHead[5] = 0x01;
		netRetTcpCfg.tcpPara.byFileHead[8] = 0x04;
		netRetTcpCfg.tcpPara.byFileHead[10] = 0x01;
		netRetTcpCfg.tcpPara.byFileHead[12] = 0x11;
		netRetTcpCfg.tcpPara.byFileHead[13] = 0x71;
		netRetTcpCfg.tcpPara.byFileHead[14] = 0x01;
		netRetTcpCfg.tcpPara.byFileHead[15] = 0x10;
		netRetTcpCfg.tcpPara.byFileHead[16] = 0x40;
		netRetTcpCfg.tcpPara.byFileHead[17] = 0x1f;
		netRetTcpCfg.tcpPara.byFileHead[21] = 0xfa;
		netRetTcpCfg.tcpPara.byReserve[0] = 0x04;
	}
	netRetTcpCfg.header.retVal = htonl(retVal);
	netRetTcpCfg.header.checkSum = htonl(checkByteSum((char *)&(netRetTcpCfg.header.retVal), sendlen-8));

	return writen(connfd, (char *)&netRetTcpCfg, sendlen);
}

int netClientUNKNOWN113305(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	NETCMD_HEADER  netCmdHeader;
	struct{
		NETRET_HEADER header;
		NET_UNKNOWN113305 UNKNOWN113305;;
	}netRetCfg;
	UINT32 retVal;
	int sendlen;
	
	sendlen = sizeof(netRetCfg);
	memset(&netRetCfg,0,sendlen);
	netRetCfg.header.length = htonl(sendlen);

	memcpy((char *)&netCmdHeader, recvbuff, sizeof(NETCMD_HEADER));
	retVal = verifyNetClientOperation(&netCmdHeader, pClientSockAddr, REMOTESHOWPARAMETER);
	if(retVal == NETRET_QUALIFIED)
	{
		netRetCfg.UNKNOWN113305.dwSize = htonl(sizeof(NET_UNKNOWN113305));
		netRetCfg.UNKNOWN113305.res1 = 0x01;
		netRetCfg.UNKNOWN113305.res3 = 0x01;
		netRetCfg.UNKNOWN113305.res4 = 0x01;
	}
	netRetCfg.header.retVal = htonl(retVal);
	netRetCfg.header.checkSum = htonl(checkByteSum((char *)&(netRetCfg.header.retVal), sendlen-8));

	return writen(connfd, (char *)&netRetCfg, sendlen);
	
}
int netClientSetDeviceDatetime(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	typedef struct tagDateTime{
		int	year;
		int month;
		int day;
		int hour;
		int min;
		int second;
	}DateTime;
    
    DateTime *pDateTime = (DateTime *)(recvbuff + sizeof(NETCMD_HEADER));
    
#if 0
	DMS_SYSTEMTIME dms_time;
	memset(&dms_time, 0, sizeof(dms_time));
	int s32Ret = dms_sysnetapi_ioctrl(g_hik_handle, DMS_NET_CMD_GET_SYSTIME, 0, (void *)&dms_time, sizeof(DMS_SYSTEMTIME));
	if (s32Ret != 0)
	{
		HPRINT_INFO("HKvs-Protocol DMS_NET_CMD_GET_SYSTIME failed %d\n", s32Ret);
		return -1;
	}

	dms_time.wHour = htonl(pDateTime->hour);
	dms_time.wMinute = htonl(pDateTime->min);
	dms_time.wSecond = htonl(pDateTime->second);
	dms_time.wDay = htonl(pDateTime->day);
	dms_time.wMonth = htonl(pDateTime->month);
	dms_time.wYear = htonl(pDateTime->year);

	s32Ret = dms_sysnetapi_ioctrl(g_hik_handle, DMS_NET_CMD_SET_SYSTIME, 0, (void *)&dms_time, sizeof(DMS_SYSTEMTIME));
	if (s32Ret != 0)
	{
		HPRINT_INFO("HKvs-Protocol QMAPI_NET_STA_SET_SYSTIME failure %d\n", s32Ret);
		return -2;
	}

	dms_sysnetapi_ioctrl(g_hik_handle, DMS_NET_SET_SAVECFG, 0,NULL,0);
#else

    int year, month, day, hour, min, second;
    year = ntohl(pDateTime->year);
    month = ntohl(pDateTime->month);
    day = ntohl(pDateTime->day);
    hour = ntohl(pDateTime->hour);
    min = ntohl(pDateTime->min);
    second = ntohl(pDateTime->second);

    HPRINT_INFO("Set System time1 : year = %d, month = %d, day = %d, : hour = %d, minute = %d, senond = %d\n",
        year, month, day, hour, min, second);

    #if 0
    year = htonl(pDateTime->year);
    month = htonl(pDateTime->month);
    day = htonl(pDateTime->day);
    hour = htonl(pDateTime->hour);
    min = htonl(pDateTime->min);
    second = htonl(pDateTime->second);

    HPRINT_INFO("Set System time2 : year = %d, month = %d, day = %d, : hour = %d, minute = %d, senond = %d\n",
        year, month, day, hour, min, second);
    #endif

    char command[128] = {0};
    sprintf(command, "date -s %d.%d.%d-%d:%d:%d", year, month, day, hour, min, second);
    new_system_call(command);
    new_system_call("hwclock -w");


    //验证
    time_t timep;
    struct tm *p;
    time(&timep);
    struct tm tt = {0};
    p = localtime_r(&timep, &tt);

    time_t timep2;
    struct tm *p2;
    time(&timep2);
    struct tm tt2 = {0};
    p2 = gmtime_r(&timep2, &tt2);


    HPRINT_INFO("localtime: %04d%02d%02d%02d%02d%02d\n", p->tm_year + 1900, p->tm_mon + 1, p->tm_mday,
                                                   p->tm_hour, p->tm_min, p->tm_sec);

    HPRINT_INFO("gmtime: %04d%02d%02d%02d%02d%02d\n", p2->tm_year + 1900, p2->tm_mon + 1, p2->tm_mday,
                                                   p2->tm_hour, p2->tm_min, p2->tm_sec);

#endif

	int nRetVal = NETRET_QUALIFIED;
 	return sendNetRetval(connfd, nRetVal);
}

int netMakeIFrame(int connfd, char *recvbuff, struct sockaddr_in *pClientSockAddr)
{
	struct{
		NETCMD_HEADER header;
		int StreamType;
	}netCfg;
	UINT32 retVal;
	
	memset(&netCfg,0, sizeof(netCfg));
	memcpy((char *)&netCfg, recvbuff, sizeof(netCfg));
	retVal = verifyNetClientOperation(&(netCfg.header), pClientSockAddr, REMOTESETPARAMETER);
	if(retVal == NETRET_QUALIFIED)
	{
		int StreamType = ntohl(netCfg.StreamType);
		HPRINT_INFO("StreamType:%d\n",StreamType);
		
		//StreamType = DMS_MAIN_STREAM; //暂时不知道 海康主次码流分别用什么值表示，先统一请求主码流
		
		//dms_sysnetapi_ioctrl(g_hik_handle,DMS_NET_CMD_IFRAME_REQUEST,0,&StreamType, sizeof(int));
	}
	
	return sendNetRetval(connfd, retVal);
}











