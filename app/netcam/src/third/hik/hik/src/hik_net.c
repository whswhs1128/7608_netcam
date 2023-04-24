/******************************************************************************
  File Name     : hik_net.c
  Version       : v1.0
  Author        : 
  Created       : 2014/11/20
  Description   : 
  History       :
  1.Date        : 2014/11/20
  Author        : 
  Modification: Created file
******************************************************************************/
#include "hik_net.h"
#include "eventalarm.h"


#define gettid() syscall(__NR_gettid)

int g_hik_handle = -1;
pthread_mutex_t usrMutex;
HikUser *pGlobalUserInfo = NULL;
pthread_mutex_t globalMSem;

static int gAlarmSocketfd = -1;
static int gAlarmSocketfd_v30 = -1;

pthread_mutex_t gAlarmMutex; 

static void *hik_alarm_handle;


uint16_t checksum(uint16_t * buffer, int size)
{
    unsigned long cksum = 0;
    while(size > 1)
    {
        cksum += *buffer++;
        size -= sizeof(uint16_t);
    }
    if(size)
    {
        cksum += *(uint8_t *)buffer;
    }
    cksum = (cksum >> 16) + (cksum & 0xffff); 	// 将高 16bit 与低 16bit 相加
    cksum += (cksum >> 16);            			// 将进位到高位的 16bit 与低 16bit 再相加

    return (uint16_t)(~cksum);
}

/* Function: ckSum
 * Description: Check sum for 16-bit buffer.
 * Input:	pBuf - 16-bit buffer
 *			len - buffer size
 * Output:	none
 * Return:	sum
 */
UINT32 ckSum(UINT16 *pBuf, UINT32 len)
{
	int nLeft = len;
	UINT16 *w = pBuf;
	UINT32 sum = 0;

	while (nLeft > 1)
	{
		sum += *w++;
		nLeft -= 2;
	}

	if (nLeft == 1) 
	{
		sum += *(UINT8 *)w;
	}

	return sum;
}

/* Function: checkByteSum
 * Description: Check sum of a buffer by byte.
 * Input:	pBuf - buffer
 *			len - buffer size
 * Output:	none
 * Return:	sum
 */
UINT32 checkByteSum(char *pBuf, int len)
{
	int i;
	UINT32 sum = 0;

	for (i = 0; i < len; i++)
	{
		sum += (UINT8)*pBuf++;
	}

	return sum;
}


/*************************************************
  Function: 	 readn
  Description:	 read n bytes from socket(with timeout 10s)
  Input:	        connfd   -- connetion descriptor
  			 vptr       -- string read from socket
  			 n           -- read bytes
  Output:		 
  Return:		 read bytes
*************************************************/
int readn(int connfd, void *vptr, int n)
{
	int	nleft;
	int	nread;
	char *ptr;
	struct timeval 	select_timeout;
	fd_set rset;

	select_timeout.tv_sec = 10;
	select_timeout.tv_usec = 0;

	ptr = vptr;
	nleft = n;
	while (nleft > 0)
	{
		select_timeout.tv_sec = 10;
		select_timeout.tv_usec = 0;
		FD_ZERO(&rset);
		FD_SET(connfd, &rset);
		if(select(connfd+1, &rset, NULL, NULL, &select_timeout) <= 0) 
		{
			/*PRT_ERR(("readn: select failed, errno = 0x%x\n", errnoGet()));*/
			return -1;
		}
		if((nread = recv(connfd, ptr, nleft, 0)) < 0)
		{
			if(errno == EINTR)
			{
				nread = 0;
			}
			else
			{
				return -1;
			}
		}
		else if (nread == 0)
		{
			break;
		}
			
		nleft -= nread;
		ptr   += nread;
	}
	
	return(n - nleft);
}

/*************************************************
  Function: 	 writen
  Description:	 write n bytes to socket
  Input:	        connfd   -- connetion descriptor
  			 vptr       -- string to write
  			 n           -- write bytes
  Output:		 
  Return:		 write bytes
*************************************************/
int writen(int connfd, void *vptr, size_t n)
{
	int nleft, nwritten;
 	char	*ptr;

	ptr = vptr;
	nleft = n;

	while(nleft>0)
	{
		if((nwritten = send(connfd, ptr, nleft, MSG_NOSIGNAL)) == -1)
		{
			if(errno == EINTR)
			{
				HPRINT_ERR("EINTR\n");
				nwritten = 0;
			}
			else 
			{
				HPRINT_ERR("Send() error, %d\n", errno);
				return -1;
			}
		}
		nleft -= nwritten;
		ptr   += nwritten;
	}

	return(n);
}


/*************************************************************
* 函数介绍：报警回调函数
* 输入参数：
* 输出参数：
* 返回值  ：
* 备注    ：1:$外部$报警2:$动态检测$报警3:$视频丢失$报警4:$遮挡检测$报警
*************************************************************/
typedef struct tagAlarmUploadInfo{
	char value[32];
}AlarmUploadInfo;


int fHikNetAlarmCallback(int nChannel, int nAlarmType, int nAction, void* pParam)
{
	//HPRINT_INFO("nChannel:%d AlarmType:%d, Action:%d\n", nChannel, nAlarmType, nAction);
	if (nAction == 0){
		return -1;
	}

	int toType = 0x03;
	char buf[108] = {0};
	int nRet = -1;

    // 1.convert
	switch(nAlarmType)
    {
        case GK_ALARM_TYPE_ALARMIN:     //0:信号量报警开始
            //toType = 0x06;
            buf[3] = 0x6c;
            buf[7] = 0x68;
            buf[12] = 0x01;
            break;

        case GK_ALARM_TYPE_DISK_FULL:       //1:硬盘满
			return 0;

        case GK_ALARM_TYPE_VLOST:            //2:信号丢失
			return 0;

        case GK_ALARM_TYPE_VMOTION:          //3:移动侦测
            //tutk_alarm_md();
            toType = 0x03;
		    break;
        case GK_ALARM_TYPE_DISK_UNFORMAT:    //4:硬盘未格式化
			return 0;

        case GK_ALARM_TYPE_DISK_RWERR:       //5:读写硬盘出错,
			return 0;

        case GK_ALARM_TYPE_VSHELTER:         //6:遮挡报警
            toType = 0x06;
            break;

        case GK_ALARM_TYPE_ALARMIN_RESUME:   //9:信号量报警恢复
            toType = 0x06;
            break;

        case GK_ALARM_TYPE_VLOST_RESUME:     //10:视频丢失报警恢复
			return 0;

        case GK_ALARM_TYPE_VMOTION_RESUME:   //11:视频移动侦测报警恢复
			return 0;

        case GK_ALARM_TYPE_NET_BROKEN:       //12:网络断开
			return 0;

        case GK_ALARM_TYPE_IP_CONFLICT:      //13:IP冲突
			return 0;

        default:
			HPRINT_INFO("Alarm: Type = %d", nAlarmType);
        	return 0;
    }

    // 2.packing
	AlarmUploadInfo objAlarmUploadInfo;
	bzero(&objAlarmUploadInfo, sizeof(AlarmUploadInfo));
	objAlarmUploadInfo.value[3] = 0x20;
	objAlarmUploadInfo.value[7] = 0x14;
	objAlarmUploadInfo.value[11] = toType;
	objAlarmUploadInfo.value[27] = 0x01;

	int nLen = sizeof(AlarmUploadInfo);
	char buffer[32] = {0};
	memcpy(buffer, &objAlarmUploadInfo, nLen);

	// 3.alarm upload to hkvs nvr
	if (gAlarmSocketfd < 0){
		HPRINT_ERR("HiK alarm sockedfd is null. \n");
	}
	else
	{
	    HPRINT_INFO("hik upload alarm, sock:%d, type:%d\n", gAlarmSocketfd, nAlarmType);
		//pthread_mutex_lock(&gAlarmMutex);
		if(nAlarmType == GK_ALARM_TYPE_ALARMIN)
		{
			nRet = writen(gAlarmSocketfd, buf, 108);
		}
		else
		{
			nRet = writen(gAlarmSocketfd, buffer, nLen);
			if (nRet < 0){
				gAlarmSocketfd = -1;
			}
		}
		//pthread_mutex_unlock(&gAlarmMutex);
		//HPRINT_INFO("HiK upload alarm, nRet=%d\n", nRet);
	}

	if (gAlarmSocketfd_v30 < 0){
		HPRINT_ERR("HiK alarm sockedfd_v30 is null. \n");
	}
	else
	{
	    HPRINT_INFO("hik upload alarm, sock_v30:%d, type:%d\n", gAlarmSocketfd_v30, nAlarmType);
		//pthread_mutex_lock(&gAlarmMutex);
		if(nAlarmType == GK_ALARM_TYPE_ALARMIN)
		{
			nRet = writen(gAlarmSocketfd_v30, buf, 108);
		}
		else
		{
			nRet = writen(gAlarmSocketfd_v30, buffer, nLen);
			if (nRet < 0){
				gAlarmSocketfd_v30 = -1;
			}
		}
		//pthread_mutex_unlock(&gAlarmMutex);
		//HPRINT_INFO("HiK upload alarm, nRet=%d\n", nRet);
	}

    return 0;
}

const char *hik_GetCmdInfo(int cmd)
{
	switch(cmd)
	{
		case NETCMD_LOGIN	:	return "User Login";
		case NETCMD_RELOGIN	:	return "User ReLogin";
		case NETCMD_LOGOUT	:	return "User Logout";
		case NETCMD_GET_DEVICECFG:          return "Get DevCfg";
		case NETCMD_GET_DEVICECFG_V40	:	return "Get DevCfg V40";
		case NETCMD_GET_COMPRESSCFG_V30	:	return "Get CompressCfg V30";
		case NETCMD_GET_NETCFG	:	return "Get NetCfg";
		case NETCMD_GET_DEVICECAPACITY	:	return "Get DevAbility";
		case NETCMD_GET_PICCFG_EX	:	return "Get PicEx";
		case NETCMD_SET_PICCFG_EX	:	return "Set PicEx";
		case NETCMD_GET_VIDEOEFFECT	:	return "Get VideoCfg";
		case NETCMD_SET_VIDEOEFFECT	:	return "Set VideoCfg";
		case NETCMD_GET_COLORENHANCE	:	return "Get ColorEnhance";
		case NETCMD_ALARMCHAN_V30	:   return "Set AlarmChan V30";
		case NETCMD_ALARMCHAN	:	return "Set AlarmChan";
		case NETCMD_GET_ALARMINCFG	:	return "Get AlarmInCfg";
		case NETCMD_SET_TIMECFG	:	return "Set Time";
		case NETCMD_GET_RTSPPORT	:	return "Get RtspPort";
		case NETCMD_MAKEIFRAME	:	return "Request IFrame";
		default	:	return "UnKown cmd"; 
	}	
}

void *hik_Session_Thread(void *param)
{
    prctl(PR_SET_NAME, (unsigned long)"hik_Session_Thread", 0, 0, 0);   
    HPRINT_INFO("in hik_Session_Thread\n");

	int cmdLength, leftLen;
	int nread,retVal = 0;
	NETCMD_HEADER netCmdHeader;
	char recvbuff[8*1024]={0};

	HIK_CLIENT_INFO stThreadinfo;
	memcpy(&stThreadinfo, param, sizeof(HIK_CLIENT_INFO));
	free(param);

	struct sockaddr_in *pClientSockAddr = &stThreadinfo.ClientAddr;	
	int connfd = stThreadinfo.ClientSocket;
	
	fd_set rset;
	FD_ZERO(&rset);
	FD_SET(connfd, &rset);

	struct timeval	select_timeout;
	select_timeout.tv_sec = 5;
	select_timeout.tv_usec = 0;

	if(select(connfd+1, &rset, NULL, NULL, &select_timeout) > 0)
	{
		if(FD_ISSET(connfd, &rset))
		{
			memset(&netCmdHeader,0,sizeof(netCmdHeader));
            //memset(recvbuff, 0, sizeof(recvbuff));
			/* receive length, 4bytes */
			if((nread = readn(connfd, &cmdLength, 4)) != 4) //总长度
			{
				HPRINT_ERR("readn() failed nread = %d, connfd = %d, errno=%d\n", nread, connfd, errno);
				retVal = -1;
				goto errExit;
			}

			cmdLength = ntohl(cmdLength);	
            HPRINT_INFO("cmdLength:%d\n", cmdLength);

			if(cmdLength>=24 && cmdLength<=MAX_CLIENT_DATA)
			{
				/* valid command length */
				leftLen = cmdLength - 4;
				if((nread = readn(connfd, recvbuff+4, leftLen)) != leftLen)
				{
					HPRINT_ERR("readn() length error: %d, %d\n", nread, leftLen);
					retVal = -1;
					goto errExit;
				}

				memcpy((char *)&netCmdHeader, recvbuff, sizeof(NETCMD_HEADER));
				netCmdHeader.netCmd = ntohl(netCmdHeader.netCmd);
				//HPRINT_INFO("connfd:%d,cmd:%s(0x%06X)\n", connfd, hik_GetCmdInfo(netCmdHeader.netCmd), netCmdHeader.netCmd);
                HPRINT_INFO("connfd:%d,cmd:0x%06X\n", connfd, netCmdHeader.netCmd);
				switch(netCmdHeader.netCmd)
				{
					/*==================================================================
					*										user login and logout
					* ================================================================== */ 
					case NETCMD_LOGIN:						/*user login*/	
                        HPRINT_INFO("NETCMD_LOGIN. sock:%d\n", connfd);
						retVal = netClientLogin(connfd, recvbuff, pClientSockAddr);
                        //HPRINT_INFO("=== 000\n");
						break;

					case NETCMD_RELOGIN:
                        HPRINT_INFO("NETCMD_RELOGIN. sock:%d\n", connfd);
						retVal = netClientReLogin(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_LOGOUT:
                        HPRINT_INFO("NETCMD_LOGOUT. sock:%d\n", connfd);
						retVal = netClientLogout(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_USEREXCHANGE:
                        HPRINT_INFO("NETCMD_USEREXCHANGE. sock:%d\n", connfd);
						retVal = netClientUserExchange(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_GET_IPCORECFG_V31:		  /*get IPC core config param, supports domain name*/
                        HPRINT_INFO("NETCMD_GET_IPCORECFG_V31. sock:%d\n", connfd);
						retVal = netClientGetIpCoreCfgV31(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_GET_IPALARMINCFG:
                        HPRINT_INFO("NETCMD_GET_IPALARMINCFG. sock:%d\n", connfd);
						retVal = netClientGetIpAlarmInCfg(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_SET_IPALARMINCFG:
                        HPRINT_INFO("NETCMD_SET_IPALARMINCFG. sock:%d\n", connfd);
						retVal = netClientSetIpAlarmInCfg(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_GET_IPALARMOUTCFG:
                        HPRINT_INFO("NETCMD_GET_IPALARMOUTCFG. sock:%d\n", connfd);
						retVal = -1;
						break;

					case NETCMD_SET_IPALARMOUTCFG:
                        HPRINT_INFO("NETCMD_SET_IPALARMOUTCFG. sock:%d\n", connfd);
						retVal = -1;
						break;

					case NETCMD_ALARMCHAN:					/*alarm up to client*/
                        HPRINT_INFO("NETCMD_ALARMCHAN. sock:%d\n", connfd);
						gAlarmSocketfd = connfd;
						netClientStartAlarmUpChannel(connfd, recvbuff, pClientSockAddr, FALSE);
						break;

					case NETCMD_ALARMCHAN_V30:
                        HPRINT_INFO("NETCMD_ALARMCHAN_V30. sock:%d\n", connfd);
                        gAlarmSocketfd_v30 = connfd;
						netClientStartAlarmUpChannel(connfd, recvbuff, pClientSockAddr, TRUE);
                        break;
						
					case NETCMD_GET_WORKSTATUS:
                        HPRINT_INFO("NETCMD_GET_WORKSTATUS. sock:%d\n", connfd);
						retVal = netClientGetWorkStatus(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_GET_WORKSTATUS_V30:
                        HPRINT_INFO("NETCMD_GET_WORKSTATUS_V30. sock:%d\n", connfd);
						retVal = netClientGetWorkStatusV30(connfd, recvbuff, pClientSockAddr);
						break;

					/*====================================================================
					*								   parameter config
					* ====================================================================*/   
					case NETCMD_GET_DEVICECFG:		   /*get device config param*/ 
                        HPRINT_INFO("NETCMD_GET_DEVICECFG. sock:%d\n", connfd);
						retVal = netClientGetDeviceCfg(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_SET_DEVICECFG:		   /*set device config param*/ 
                        HPRINT_INFO("NETCMD_SET_DEVICECFG. sock:%d\n", connfd);
						retVal = netClientSetDeviceCfg(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_GET_DEVICECFG_V40:		   /*get device config param*/ 
                        HPRINT_INFO("NETCMD_GET_DEVICECFG_V40. sock:%d\n", connfd);
						retVal = netClientGetDeviceCfgV40(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_SET_DEVICECFG_V40:		   /*set device config param*/ 
                        HPRINT_INFO("NETCMD_SET_DEVICECFG_V40. sock:%d\n", connfd);
						retVal = netClientSetDeviceCfgV40(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_GET_ALARMINCFG_V30:    /*get alarm in config param*/
                        HPRINT_INFO("NETCMD_GET_ALARMINCFG_V30. sock:%d\n", connfd);
						retVal = netClientGetAlarmInCfgV30(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_SET_ALARMINCFG_V30: /*set alarm in config param*/	
                        HPRINT_INFO("NETCMD_SET_ALARMINCFG_V30. sock:%d\n", connfd);
						retVal = netClientSetAlarmInCfgV30(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_GET_ALARMOUTCFG_V30:   /*get alarm out config param*/	
                        HPRINT_INFO("NETCMD_GET_ALARMOUTCFG_V30. sock:%d\n", connfd);
						retVal = netClientGetAlarmOutCfgV30(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_SET_ALARMOUTCFG_V30:   /*set alarm out config param*/
                        HPRINT_INFO("NETCMD_SET_ALARMOUTCFG_V30. sock:%d\n", connfd);
						retVal = netClientSetAlarmOutCfgV30(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_PTZ:
                        HPRINT_INFO("NETCMD_PTZ. sock:%d\n", connfd);
						retVal = netClientPTZControl(connfd, recvbuff, pClientSockAddr);
						break;
					case DVR_PTZWITHSPEED:
                        HPRINT_INFO("DVR_PTZWITHSPEED. sock:%d\n", connfd);
						retVal = netClientPTZControlWithSpeed(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_GET_DEVICECAPACITY:
                        HPRINT_INFO("NETCMD_GET_DEVICECAPACITY. sock:%d\n", connfd);
						retVal = netClientGetDeviceCapacity(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_GET_COMPRESSCFG_V30:   /*get compress config param*/
                        HPRINT_INFO("NETCMD_GET_COMPRESSCFG_V30. sock:%d\n", connfd);
						retVal = netClientGetCompressCfgV30(connfd, recvbuff, pClientSockAddr);
						break;
					case NETCMD_SET_COMPRESSCFG_V30:   /*set compress config param*/
                        HPRINT_INFO("NETCMD_SET_COMPRESSCFG_V30. sock:%d\n", connfd);
						retVal = netClientSetCompressCfgV30(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_GET_NETCFG: 			/*get network config param*/
                        HPRINT_INFO("NETCMD_GET_NETCFG. sock:%d\n", connfd);
						retVal = netClientGetNetCfg(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_SET_NETCFG: 		/*set network config param*/
                        HPRINT_INFO("NETCMD_SET_NETCFG. sock:%d\n", connfd);
						retVal = netClientSetNetCfg(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_GET_PICCFG_EX:	  
                        HPRINT_INFO("NETCMD_GET_PICCFG_EX. sock:%d\n", connfd);
						retVal = netClientGetPicCfgEx(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_SET_PICCFG_EX:
                        HPRINT_INFO("NETCMD_SET_PICCFG_EX. sock:%d\n", connfd);
						retVal = netClientSetPicCfgEx(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_GET_RTSPPORT:
                        HPRINT_INFO("NETCMD_GET_RTSPPORT. sock:%d\n", connfd);
						//retVal = netClientGetRtspPort(connfd, recvbuff, pClientSockAddr);
						retVal = netClientGetRtspCfg(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_GET_PICCFG: //0x020200		NETPARAM_COMPRESS_CFG_V30	
					{
						HPRINT_INFO("NETCMD_GET_PICCFG 111111111111111111.\n");
						break;
					}
					case NETCMD_SET_PICCFG: //0x020201	
					{
						HPRINT_INFO("NETCMD_SET_PICCFG 2222222222222222222.\n");
						break;
					}
					case NETCMD_GET_VIDEOEFFECT:
                        HPRINT_INFO("NETCMD_GET_VIDEOEFFECT. sock:%d\n", connfd);
						retVal = netClientGetVideoEffect(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_SET_VIDEOEFFECT:
                        HPRINT_INFO("NETCMD_SET_VIDEOEFFECT. sock:%d\n", connfd);
						retVal = netClientSetVideoEffect(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_GET_COLORENHANCE:
                        HPRINT_INFO("NETCMD_GET_COLORENHANCE. sock:%d\n", connfd);
						retVal = netClientGetColorEnhance(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_SET_COLORENHANCE:
                        HPRINT_INFO("NETCMD_SET_COLORENHANCE. sock:%d\n", connfd);
						retVal = netClientSetColorEnhance(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_GET_ALARMINCFG:
                        HPRINT_INFO("NETCMD_GET_ALARMINCFG. sock:%d\n", connfd);
						retVal = netClientGetAlarmIncfg(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_SET_ALARMINCFG:
                        HPRINT_INFO("NETCMD_SET_ALARMINCFG. sock:%d\n", connfd);
						retVal = netClientSetAlarmIncfg(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_GET_NTPCFG:
                        HPRINT_INFO("NETCMD_GET_NTPCFG. sock:%d\n", connfd);
						retVal = netClientGetNtpcfg(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_SET_NTPCFG:
                        HPRINT_INFO("NETCMD_SET_NTPCFG. sock:%d\n", connfd);
						retVal = netClientSetNtpcfg(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_GET_DSTCFG:
                        HPRINT_INFO("NETCMD_GET_DSTCFG. sock:%d\n", connfd);
						retVal = netClientGetDstcfg(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_SET_DSTCFG:
                        HPRINT_INFO("NETCMD_SET_DSTCFG. sock:%d\n", connfd);
						retVal = netClientSetDstcfg(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_SOFTRESTORE_CFG:
                        HPRINT_INFO("NETCMD_SOFTRESTORE_CFG. sock:%d\n", connfd);
						//retVal = netClientSetSoftStore(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_REBOOT:
                        HPRINT_INFO("NETCMD_REBOOT. sock:%d\n", connfd);
						retVal = netClientSetReBoot(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_GET_NETWORKCFG:
                        HPRINT_INFO("NETCMD_GET_NETWORKCFG. sock:%d\n", connfd);
						retVal = netClientGetNetWorkCfg(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_SET_NETWORKCFG:
                        HPRINT_INFO("NETCMD_SET_NETWORKCFG. sock:%d\n", connfd);
						retVal = netClientSetNetWorkCfg(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_GET_FTPCFG:
                        HPRINT_INFO("NETCMD_GET_FTPCFG. sock:%d\n", connfd);
						retVal = netClientGetFtpCfg(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_SET_FTPCFG:
                        HPRINT_INFO("NETCMD_SET_FTPCFG. sock:%d\n", connfd);
						retVal = netClientSetFtpCfg(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_GET_RTSPCFG:
                        HPRINT_INFO("NETCMD_GET_RTSPCFG. sock:%d\n", connfd);
						retVal = netClientGetRtspCfg(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_SET_RTSPCFG:
                        HPRINT_INFO("NETCMD_SET_RTSPCFG. sock:%d\n", connfd);
						retVal = netClientSetRtspCfg(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_TCP_PREV:
                        HPRINT_INFO("NETCMD_TCP_PREV. sock:%d\n", connfd);
						retVal = netClientTcpPrev(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_SET_TIMECFG:
                        HPRINT_INFO("NETCMD_SET_TIMECFG. sock:%d\n", connfd);
						retVal = netClientSetDeviceDatetime(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_UNKNOWN_113305:
                        HPRINT_INFO("NETCMD_UNKNOWN_113305. sock:%d\n", connfd);
						retVal = netClientUNKNOWN113305(connfd, recvbuff, pClientSockAddr);
						break;

					case NETCMD_MAKEIFRAME:
                        HPRINT_INFO("NETCMD_MAKEIFRAME. sock:%d\n", connfd);
						retVal = netMakeIFrame(connfd, recvbuff, pClientSockAddr);
						break;
						
					case NETCMD_UNKNOWN_11300b:
					case NETCMD_UNKNOWN_113004:
					case NETCMD_UNKNOWN_113003:
					default:
						HPRINT_ERR("################# un support cmd: 0x%X\n", netCmdHeader.netCmd);
						retVal = netClientCmd11300b(connfd, recvbuff, pClientSockAddr);
						break;
				}
			}
		}	/* FD_ISSET */
		else
		{
			retVal = -1;
		}
	}
	else
	{
		HPRINT_ERR("select failed.\n");
		retVal = -1;
	}

errExit:
	if(retVal != -1)
	{	/* read all datas from socket */
		time_t sTime = time(NULL);
		while((nread = readn(connfd, recvbuff, 1)) > 0 && netCmdHeader.netCmd != 0x111095)
		{
			HPRINT_ERR("Read %d byte. connfd=%d  tid=%d\n", nread, connfd, (int)gettid());
			usleep(100000);
			if((time(NULL)-sTime)>120)	/* wait up to 120 seconds */
			{		
				nread = -1;
				break;
			}
		}
		//HPRINT_ERR("nread = %d\n", nread);
		if(nread == -1)
		{
			retVal = -1;
		}
	}

	struct linger tcpLinger;
	tcpLinger.l_onoff = 1;
	tcpLinger.l_linger = 0;/*close socket 后内核丢弃缓冲区的数据强制关闭连接*/
	if(setsockopt(connfd, SOL_SOCKET, SO_LINGER, (char*)&tcpLinger, sizeof(struct linger)) != 0)
	{
		HPRINT_ERR("setsockopt() error,  pid:%d tid:%d, connfd=%d, 0x%x\n", (int)getpid(), (int)gettid(), connfd, errno);
	}

    HPRINT_INFO("close sock:%d \n", connfd);
	close(connfd);
	connfd = -1;
	
	//stThreadinfo.OnLogoff(stThreadinfo.id);
	int client_num = hik_connect_del();
    HPRINT_INFO("client_num:%d \n\n", client_num);
    
	return (void *)retVal;
}

int lib_hik_init(void *param)
{
	pGlobalUserInfo = (HikUser *)param;
	//g_hik_handle = pGlobalUserInfo->hikhandle;
	pthreadMutexInit(&gAlarmMutex);
	pthreadMutexInit(&usrMutex);

    #if 0
	//注册报警回调函数
    int ret = QMapi_sys_ioctrl(g_hik_handle, QMAPI_NET_REG_ALARMCALLBACK, 0, fHikNetAlarmCallback, sizeof(fHikNetAlarmCallback)); 
    if(0 != ret)
    {
        HPRINT_ERR("\n");
        return -1;      
    }
    #else
    hik_alarm_handle = event_alarm_open(fHikNetAlarmCallback);
    #endif
    
	return 0;
}

int lib_hik_uninit()
{	
	//int ret = QMapi_sys_ioctrl(g_hik_handle, QMAPI_NET_UNREG_ALARMCALLBACK, 0, NULL,0); 
	//if(ret != 0)
	//	HPRINT_ERR("free ALARMCALLBACK faile\n");
    if (hik_alarm_handle)
    {
        event_alarm_close(hik_alarm_handle);
        hik_alarm_handle = NULL;
    }

	pthreadMutexDestroy(&gAlarmMutex);
	pthreadMutexDestroy(&usrMutex);

    if (pGlobalUserInfo)
    {
        free(pGlobalUserInfo);
        pGlobalUserInfo = NULL;
    }

	return 0;
}
