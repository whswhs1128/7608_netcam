#include "hik_netfun.h"

extern int writen(int connfd, void *vptr, size_t n);
UINT32 checkByteSum(char *pBuf, int len);

/***********************************************************************
 * Function: verifyNetClientOperation
 * Description:verify the net client operation valid
 * Input:  
 *	  pHeader:pointer point to NETCMD_HEADER
 *      pClientSockAddr:pointer point to sockaddr_in 
 *      operation:net client operation
 * Output:  N/A
 * Return:  OK if successful, otherwise return error status number
 ***********************************************************************/
int verifyNetClientOperation(NETCMD_HEADER *pHeader, struct sockaddr_in *pClientSockAddr, UINT32 operation)
{
	struct in_addr clientIpAddr;

	convertNetCmdHeaderByteOrder(pHeader);
	clientIpAddr.s_addr = pHeader->clientIp;
	return verifyUserID( pHeader->userID, &clientIpAddr, &(pClientSockAddr->sin_addr), (char *)pHeader->clientMac, operation);
}

/***********************************************************************
 * Function:sendNetRetVal 
 * Description:send return value to network client
 * Input:  
 *      connfd:sock conn fd
 *      retVal:the value return to network client
 * Output:  N/A
 * Return:  OK if successful, otherwise return error status number
 ***********************************************************************/
int sendNetRetval(int connfd, UINT32 retVal)
{
	NETRET_HEADER  netRetHeader;
	int sendlen;

	sendlen = sizeof(NETRET_HEADER);
	bzero((char *)&netRetHeader, sendlen);
	netRetHeader.length = htonl(sendlen);
	netRetHeader.retVal = htonl(retVal);
	netRetHeader.checkSum = htonl(checkByteSum((char *)&(netRetHeader.retVal), sendlen-8));
	HPRINT_INFO("netRetHeader.retVal=0x%X\n", netRetHeader.retVal);
	return writen(connfd, (char *)&netRetHeader, sendlen);
}


/***********************************************************************
 * Function: isAnalogAlarmOutNum
 * Description:judge if is analog alarmOutNum
 * Input:  alarmOutNum
 * Output:  N/A
 * Return: TRUE or FALSE
 ***********************************************************************/
BOOL isAnalogAlarmOutNum(int alarmOutNum)
{
	BOOL bAnalogAlarmOutNum = FALSE;
	
	//if(alarmOutNum>=0 && alarmOutNum<devHardInfo.alarmOutNums)
	{
		bAnalogAlarmOutNum  = TRUE;	
	}
	
	return bAnalogAlarmOutNum;
}

/***********************************************************************
 * Function: isAnalogAlarmInNum
 * Description:judge if is analog alarmInNum
 * Input:  alarmInNum
 * Output:  N/A
 * Return: TRUE or FALSE
 ***********************************************************************/
BOOL isAnalogAlarmInNum(int alarmInNum)
{
	BOOL bAnalogAlarmInNum = FALSE;
	
	//if(alarmInNum>=0 && alarmInNum<devHardInfo.alarmInNums)
	{
		bAnalogAlarmInNum  = TRUE;	
	}
	
	return bAnalogAlarmInNum;
}


/***********************************************************************
 * Function: alarmOutNum
 * Description:judge if is netIp alarmOutNum
 * Input:  alarmOutNum
 * Output:  N/A
 * Return: TRUE or FALSE
 ***********************************************************************/
BOOL isNetIpAlarmOutNum(int alarmOutNum)
{
	BOOL bNetIpAlarmOutNum = FALSE;
	
	if(alarmOutNum>=MAX_ALARMOUT && alarmOutNum<(MAX_ALARMOUT + MAX_IP_ALARMOUT))
	{
		bNetIpAlarmOutNum  = TRUE;	
	}
	
	return bNetIpAlarmOutNum;
}

/***********************************************************************
 * Function: isNetIpAlarmInNum
 * Description:judge if is netIp alarmInNum
 * Input:  alarmInNum
 * Output:  N/A
 * Return: TRUE or FALSE
 ***********************************************************************/
BOOL isNetIpAlarmInNum(int alarmInNum)
{
	BOOL bNetIpAlarmInNum = FALSE;
	
	if(alarmInNum>=MAX_ALARMIN && alarmInNum<(MAX_ALARMIN + MAX_IP_ALARMIN))
	{
		bNetIpAlarmInNum  = TRUE;	
	}
	
	return bNetIpAlarmInNum;
}


/***********************************************************************
 * Function: checkAlarmInNumValid
 * Description:check alarmInNum valid
 * Input:  alarmInNum
 * Output:  N/A
 * Return: OK or ERROR
 ***********************************************************************/
int checkAlarmInNumValid(int alarmInNum)
{
	int retVal;
	int alarmInNumValid;

	alarmInNumValid = isAnalogAlarmInNum(alarmInNum) || isNetIpAlarmInNum(alarmInNum);
	
	if(alarmInNumValid)
	{
		retVal = 0;	
	}
	else
	{
		retVal = -1;	
	}

	return retVal;
}


/***********************************************************************
 * Function: checkAlarmOutNumValid
 * Description:check alarmOutNum valid
 * Input:  alarmOutNum
 * Output:  N/A
 * Return: TRUE or ERROR
 ***********************************************************************/
int checkAlarmOutNumValid(int alarmOutNum)
{
	int retVal;
	int alarmOutNumValid;

	alarmOutNumValid = isAnalogAlarmOutNum(alarmOutNum) || isNetIpAlarmOutNum(alarmOutNum);
	
	if(alarmOutNumValid)
	{
		retVal = 0;	
	}
	else
	{
		retVal = -1;	
	}

	return retVal;
}
















