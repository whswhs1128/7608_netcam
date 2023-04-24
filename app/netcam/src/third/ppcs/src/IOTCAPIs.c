#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <AVSTREAM_IO_Proto.h>
#include <PPCS_API.h>
#include <IOTCAPIs.h>
#include <AVAPIs.h>
#include <PPCS_Type.h>
#include <AVFRAMEINFO.h>
#include <AVIOCTRLDEFs.h>
#include "utility.h"
loginInfoCB logincb = NULL;
logCB logcbmsg = NULL;
CHAR  g_bLoginStatus = 0;
const char *g_ppcsDefaultInitString = "EFGHFDBJKDIHGEJJFDHGFIEHGPJJDJNBGNFLFHCHACMPLAOEGIAPHKODGFPFNJKLBOMEPHCMODJGADGPMFNONHBMMO";
char g_szUID[32] = {0};
char g_szAPILicense[16] = {0};
unsigned short g_nUDPPort =0;
void IOTC_Get_Version(unsigned int *pnVersion)
{
   	 	unsigned int iotcVer;
   	 	iotcVer = PPCS_GetAPIVersion();
   	 	*pnVersion = iotcVer;
}
void IOTC_Set_Max_Session_Number(unsigned int nMaxSessionNum)
{
	
}
int IOTC_Initialize(unsigned short nUDPPort, const char* cszP2PHostNamePrimary,
								const char* cszP2PHostNameSecondary, const char* cszP2PHostNameThird,
								const char* cszP2PHostNameFourth)								
{
	 int ret;
	 g_nUDPPort = nUDPPort;
	 int result = PPCS_Initialize((char*)g_ppcsDefaultInitString);
	 switch(result)
	 {
	 	 case ERROR_PPCS_SUCCESSFUL:
	 	 	//return IOTC_ER_NoERROR;
	 	 	break;
	 	 case ERROR_PPCS_ALREADY_INITIALIZED:
	 	 	return IOTC_ER_ALREADY_INITIALIZED;
	 	 	break;
	 	 default:
	 	 	break;
	 }
#if 1
	 printf("-------------- PPCS_NetworkDetect: -------------------\n");
	 	// 2.Õ¯¬Á’Ï≤‚
	st_PPCS_NetInfo NetInfo;
	ret = PPCS_NetworkDetect(&NetInfo, 10000);
	if (ret < 0)
		printf("PPCS_NetworkDetect() ret = %d\n", ret);
	
	printf("-------------- NetInfo: -------------------\n");
	printf("	Internet Reachable     : %s\n", (NetInfo.bFlagInternet == 1) ? "YES":"NO");
	printf("	P2P Server IP resolved : %s\n", (NetInfo.bFlagHostResolved == 1) ? "YES":"NO");
	printf("	P2P Server Hello Ack   : %s\n", (NetInfo.bFlagServerHello == 1) ? "YES":"NO");
#endif		
	 return result;
}
int IOTC_Initialize2(unsigned short nUDPPort,char *DefaultInitString)
{
	 int ret;
	 g_nUDPPort = nUDPPort;
	 int result = PPCS_Initialize((char*)DefaultInitString);
	 switch(result)
	 {
	 	 case ERROR_PPCS_SUCCESSFUL:
	 	 	//return IOTC_ER_NoERROR;
	 	 	break;
	 	 case ERROR_PPCS_ALREADY_INITIALIZED:
	 	 	return IOTC_ER_ALREADY_INITIALIZED;
	 	 	break;
	 	 default:
	 	 	break;
	 }
#if 1
	 printf("-------------- PPCS_NetworkDetect: -------------------\n");
	 	// 2.Õ¯¬Á’Ï≤‚
	st_PPCS_NetInfo NetInfo;
	ret = PPCS_NetworkDetect(&NetInfo, 10000);
	if (ret < 0)
		printf("PPCS_NetworkDetect() ret = %d\n", ret);
	
	printf("-------------- NetInfo: -------------------\n");
	printf("	Internet Reachable     : %s\n", (NetInfo.bFlagInternet == 1) ? "YES":"NO");
	printf("	P2P Server IP resolved : %s\n", (NetInfo.bFlagHostResolved == 1) ? "YES":"NO");
	printf("	P2P Server Hello Ack   : %s\n", (NetInfo.bFlagServerHello == 1) ? "YES":"NO");
#endif		
	 return result;
}
int IOTC_DeInitialize(void)
{
	   return PPCS_DeInitialize();
}
int IOTC_Device_Login(const char *cszUID, const char *cszDeviceName, const char *cszDevicePWD)
{
	   CHAR  bLoginStatus;
	   unsigned int nLoginInfo = 0;
	   if(cszUID!= NULL && strcmp(g_szUID,cszUID)!=0)
	   {
	   	  sprintf(g_szUID,"%s",cszUID);
	   	  printf("IOTC_Device_Login() DID = %s\n", g_szUID);
	   }
	   if(cszDeviceName!= NULL && strcmp(g_szAPILicense,cszDeviceName)!=0)
	   {
	   	  sprintf(g_szAPILicense,"%s",cszDeviceName);
	   	  printf("IOTC_Device_Login() APILicense = %s\n", g_szAPILicense);
	   }
     int result = PPCS_LoginStatus_Check(&bLoginStatus); 	
     if(logincb!= NULL)
     	{
     		  if(bLoginStatus != g_bLoginStatus)
     		  {
     		  	  g_bLoginStatus = bLoginStatus;
     		  	  if(g_bLoginStatus == 1)
     		  	  {
     		  	  	  nLoginInfo |=0x04;
     		  	  }
     		  	  logincb(nLoginInfo);
     		  }
     	}
     	if(result == ERROR_PPCS_SUCCESSFUL)
     	{
     		
     	}
     	if(result == ERROR_PPCS_NOT_INITIALIZED)
     	{
     		
     	}
     	if(result == ERROR_PPCS_INVALID_PARAMETER)
     	{
     		
     	}
     	return result;
}
int IOTC_Get_Login_Info(unsigned int *pnLoginInfo)
{
	    unsigned int nLoginInfo = 0;
     	if(g_bLoginStatus == 1)
     	{
     		  	nLoginInfo |=0x04;
     	}
     	*pnLoginInfo = nLoginInfo;
     	return 0;
}
void IOTC_Get_Login_Info_ByCallBackFn(loginInfoCB pfxLoginInfoFn)
{
	    logincb = pfxLoginInfoFn;
}
void IOTC_Set_log_ByCallBackFn(logCB pfxLogcbFn)
{
	    logcbmsg = pfxLogcbFn;
}

int  IOTC_Listen(unsigned int nTimeout)
{
	   int result = -1;
	   char szAPILicense[32];
	   
	   if(strlen(g_szUID)>0 && strlen(g_szAPILicense)>0)
	   {
	   	   sprintf(szAPILicense,"%s",g_szAPILicense);
	   	   printf("PPCS_Listen() g_szUID = %s  APILicense = %s timeout = %d g_nUDPPort = %d\n", g_szUID,g_szAPILicense,nTimeout,g_nUDPPort);
	   		 result  = PPCS_Listen(g_szUID,nTimeout,g_nUDPPort,1,szAPILicense);
	   		 switch(result)
	   		 {
	   		    case 	ERROR_PPCS_NOT_INITIALIZED:
	   		    return IOTC_ER_NOT_INITIALIZED;
	   		    break;
	   		    case 	ERROR_PPCS_TIME_OUT:
	   		    return IOTC_ER_TIMEOUT;
	   		    break;
	   		    case 	ERROR_PPCS_MAX_SESSION:
	   		    return IOTC_ER_EXCEED_MAX_SESSION ;
	   		    break;
	   		    case 	ERROR_PPCS_INVALID_PARAMETER:
	   		    printf("PPCS_Listen() g_szUID = %s  APILicense = %s  result = %d \n", g_szUID,szAPILicense,result);
	   		    break;
	   		    default:
	   		    break;
	   		 }
	   }
	   return result;
}
void IOTC_Listen_Exit(void)
{
	   PPCS_Listen_Break();
}
int  IOTC_Listen2(unsigned int nTimeout, const char *cszAESKey, IOTCSessionMode nSessionMode)
{
    return -1;	
}
int  IOTC_Connect_ByUID(const char *cszUID)
{	  
	   int result =  PPCS_Connect(cszUID,0,0);
	   switch(result)
	   {
	      case ERROR_PPCS_NOT_INITIALIZED:
	      	    return IOTC_ER_NOT_INITIALIZED ;
	      	break;
	      case ERROR_PPCS_TIME_OUT:
	      	break;	
	      case ERROR_PPCS_INVALID_ID:
	      	break;	
	      case ERROR_PPCS_INVALID_PREFIX:
	      	break;	
	      case ERROR_PPCS_DEVICE_NOT_ONLINE:
	      	break;	
	      case ERROR_PPCS_NO_RELAY_SERVER_AVAILABLE:
	      	break;
	      case ERROR_PPCS_MAX_SESSION:
	      	break;
	      case ERROR_PPCS_UDP_PORT_BIND_FAILED:
	      	break;	
	      case ERROR_PPCS_USER_CONNECT_BREAK:
	      	break;	
	      default:
	        break;
	   	
	   }
	   return result;
	   
}
int  IOTC_Get_SessionID(void)
{
	   return 0;
}
int  IOTC_Connect_ByUID_Parallel(const char *cszUID, int SID)
{
	   
	   int result =  PPCS_Connect(cszUID,0,0);
	   switch(result)
	   {
	      case ERROR_PPCS_NOT_INITIALIZED:
	      	    return IOTC_ER_NOT_INITIALIZED ;
	      	break;
	      case ERROR_PPCS_TIME_OUT:
	      	break;	
	      case ERROR_PPCS_INVALID_ID:
	      	break;	
	      case ERROR_PPCS_INVALID_PREFIX:
	      	break;	
	      case ERROR_PPCS_DEVICE_NOT_ONLINE:
	      	break;	
	      case ERROR_PPCS_NO_RELAY_SERVER_AVAILABLE:
	      	break;
	      case ERROR_PPCS_MAX_SESSION:
	      	break;
	      case ERROR_PPCS_UDP_PORT_BIND_FAILED:
	      	break;	
	      case ERROR_PPCS_USER_CONNECT_BREAK:
	      	break;	
	      default:
	        break;
	   	
	   }
	   return result;
	   
}
int  IOTC_Connect_ByUID2(const char *cszUID, const char *cszAESKey, IOTCSessionMode nSessionMode)
{

	  
	   int result =  PPCS_Connect(cszUID,0,0);
	   switch(result)
	   {
	      case ERROR_PPCS_NOT_INITIALIZED:
	      	    return IOTC_ER_NOT_INITIALIZED ;
	      	break;
	      case ERROR_PPCS_TIME_OUT:
	      	break;	
	      case ERROR_PPCS_INVALID_ID:
	      	break;	
	      case ERROR_PPCS_INVALID_PREFIX:
	      	break;	
	      case ERROR_PPCS_DEVICE_NOT_ONLINE:
	      	break;	
	      case ERROR_PPCS_NO_RELAY_SERVER_AVAILABLE:
	      	break;
	      case ERROR_PPCS_MAX_SESSION:
	      	break;
	      case ERROR_PPCS_UDP_PORT_BIND_FAILED:
	      	break;	
	      case ERROR_PPCS_USER_CONNECT_BREAK:
	      	break;	
	      default:
	        break;
	   	
	   }
	   return result;
	   
}
void IOTC_Connect_Stop(void)
{
	   PPCS_Connect_Break();
}
int IOTC_Connect_Stop_BySID(int SID)
{
	   PPCS_ForceClose(SID);
	   return 0;
}
int  IOTC_Session_Read_Check_Lost(int nIOTCSessionID, char *abBuf, int nMaxBufSize,unsigned int nTimeout, unsigned short *pnPacketSN,char *pbFlagLost, unsigned char nIOTCChannelID)
{
	   return -1;
}
int  IOTC_Session_Check(int nIOTCSessionID, struct st_SInfo *psSessionInfo)
{
	    st_PPCS_Session SInfo;
	    int result = PPCS_Check(nIOTCSessionID,&SInfo);
	    switch(result)
	    {
	    	        case ERROR_PPCS_NOT_INITIALIZED:
	    	        	return IOTC_ER_NOT_INITIALIZED;
	    		break;
	    		    	case ERROR_PPCS_INVALID_PARAMETER:
	    		break;
	    		    	case ERROR_PPCS_INVALID_SESSION_HANDLE:
	    		break;
	    		    	case ERROR_PPCS_SESSION_CLOSED_CALLED:
	    		break;
	    		    	case ERROR_PPCS_SESSION_CLOSED_TIMEOUT:
	    		    		return IOTC_ER_REMOTE_TIMEOUT_DISCONNECT;
	    		break;
	    		    	case ERROR_PPCS_SESSION_CLOSED_REMOTE:
	    		    		return IOTC_ER_SESSION_CLOSE_BY_REMOTE;
	    		break;
	    }
	    return result;
}
int  IOTC_Session_Check_Ex(int nIOTCSessionID, struct st_SInfoEx *psSessionInfo)
{
	    st_PPCS_Session SInfo;
	    int result = PPCS_Check(nIOTCSessionID,&SInfo);
	    return result;	
}
int IOTC_Session_Check_ByCallBackFn(int nIOTCSessionID, sessionStatusCB pfxSessionStatusFn)
{
	    return 0;
}
int  IOTC_Session_Read(int nIOTCSessionID, char *abBuf, int nMaxBufSize, unsigned int nTimeout, unsigned char nIOTCChannelID)
{
	  return PPCS_Read(nIOTCSessionID,nIOTCChannelID,abBuf,&nMaxBufSize,nTimeout);
}
int IOTC_Session_Write(int nIOTCSessionID, const char *cabBuf, int nBufSize, unsigned char nIOTCChannelID)
{
	  return PPCS_Write(nIOTCSessionID,nIOTCChannelID,(char *)cabBuf,nBufSize);
}
void IOTC_Session_Close(int nIOTCSessionID)
{
	  PPCS_Close(nIOTCSessionID);
}
int IOTC_Session_Get_Free_Channel(int nIOTCSessionID)
{
	   return 3;
}
int IOTC_Session_Channel_ON(int nIOTCSessionID, unsigned char nIOTCChannelID)
{
	 	return -1;
}
int IOTC_Session_Channel_OFF(int nIOTCSessionID, unsigned char nIOTCChannelID)
{
	 	return -1;
}
int IOTC_Lan_Search(struct st_LanSearchInfo *psLanSearchInfo, int nArrayLen, int nWaitTimeMs)
{
		return -1;
}
void IOTC_Set_Log_Path(char *path, int nMaxSize)
{
	
}
int IOTC_Set_Partial_Encryption(int nIOTCSessionID, unsigned char bPartialEncryption)
{
	    return -1;
}
void IOTC_Set_Device_Name(const char *cszDeviceName)
{
	    
}
int IOTC_Lan_Search2(struct st_LanSearchInfo2 *psLanSearchInfo2, int nArrayLen, int nWaitTimeMs)
{
	 	return -1;
}
int IOTC_Lan_Search2_Ex(struct st_LanSearchInfo2 *psLanSearchInfo2, int nArrayLen, int nWaitTimeMs, int nSendIntervalMs)
{
		return -1;
}
int IOTC_Search_Device_Start(int nWaitTimeMs, int nSendIntervalMs)
{
		return -1;
}
int IOTC_Search_Device_Result(struct st_SearchDeviceInfo *psSearchDeviceInfo, int nArrayLen, int nGetAll)
{
		return -1;
}
int IOTC_Search_Device_Stop()
{
		return -1;
}
void IOTC_TCPRelayOnly_TurnOn(void)
{
	
}
void IOTC_Setup_LANConnection_Timeout(unsigned int nTimeout)
{
	
}
void IOTC_Setup_P2PConnection_Timeout(unsigned int nTimeout)
{
	
}
int IOTC_ReInitSocket(unsigned short nUDPPort)
{
	  return -1;
}
void IOTC_Setup_DetectNetwork_Timeout(unsigned int nTimeout)
{
	
}
int IOTC_Session_Write_Reliable(int nIOTCSessionID, const char *cabBuf, int nBufSize, unsigned char nIOTCChannelID, unsigned int nTimeout)
{
	return -1;
}
int IOTC_Session_Write_Reliable_Abort(int nIOTCSessionID, unsigned char nIOTCChannelID)
{
	 	return -1;
}
void IOTC_Setup_Session_Alive_Timeout(unsigned int nTimeout)
{
		
}
int IOTC_Set_Connection_Option(struct st_ConnectOption *S_ConnectOption)
{
		return -1;
}
void IOTC_Setup_ErrorUpload(int bEnable)
{
	
}
