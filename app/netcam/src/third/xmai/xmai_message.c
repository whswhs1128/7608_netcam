/************************************************************************
author:bruce
editor:
description:实现与nvr的交互 ，本代码对讲，回放，和录像下载功能都不支持，
写了代码但功能没实现，以后可以做扩展。支持雄迈私有协议
history: 2014.6.1

************************************************************************/

#include "xmai_session.h"
#include "xmai_common_api.h"
#include "cJSON.h"
#include "netcam_api.h"
#include "gk_cms_protocol.h"
#include "util_sock.h"


extern UtSession g_xmai_session;

extern int g_xmai_pthread_run;
//int g_xmai_stream_open = 0;
extern pthread_mutex_t g_xmai_audio_mutex;
extern int g_xmai_talk_recv_running;
int g_xmai_is_talking = 0;


/////////////////////////////////////////////////
int ResIpsearch(int fSockSearchfd)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root, *NetCommon;
    char csHostIP[30] = {0};
    char csGateWay[30] = {0};
    char csMAC[30] = {0};
    char csSubmask[30] = {0};

#if 0
	//获取本机网络参数，
    DMS_NET_NETWORK_CFG st_ipc_net_info;
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_NETCFG, 0, &st_ipc_net_info, sizeof(st_ipc_net_info)))
    {
       PRINT_ERR("DMS_NET_GET_NETCFG fail\n");
       return -1;
    }   
    //获取无线网络参数，
    DMS_NET_WIFI_CONFIG st_wifi_net_info;
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_WIFICFG, 0, &st_wifi_net_info, sizeof(st_wifi_net_info)))
    {
       PRINT_ERR("DMS_NET_GET_WIFICFG fail\n");
       return -1;
    } 

    //如果无线可用就只传无线的配置
    if(st_wifi_net_info.bWifiEnable && st_wifi_net_info.byStatus == 0)
    {
	    sprintf(csHostIP, "0x%08x", inet_addr(st_wifi_net_info.dwNetIpAddr.csIpV4));
	    sprintf(csGateWay, "0x%08x", inet_addr(st_wifi_net_info.dwGateway.csIpV4));
	    sprintf(csSubmask, "0x%08x", inet_addr(st_wifi_net_info.dwNetMask.csIpV4));	

		PRINT_INFO("Wifi\n");
	    PRINT_INFO("HostIP:%s\n", st_wifi_net_info.dwNetIpAddr.csIpV4);
	    PRINT_INFO("GateWay:%s\n", st_wifi_net_info.dwGateway.csIpV4);
	    PRINT_INFO("Submask:%s\n", st_wifi_net_info.dwNetMask.csIpV4);		
    }
	else
	{
	    sprintf(csHostIP, "0x%08x", inet_addr(st_ipc_net_info.stEtherNet[0].strIPAddr.csIpV4));
	    sprintf(csGateWay, "0x%08x", inet_addr(st_ipc_net_info.stGatewayIpAddr.csIpV4));
	    sprintf(csSubmask, "0x%08x", inet_addr(st_ipc_net_info.stEtherNet[0].strIPMask.csIpV4));	

		PRINT_INFO("localhost\n");
	    PRINT_INFO("HostIP:%s\n", st_ipc_net_info.stEtherNet[0].strIPAddr.csIpV4);
	    PRINT_INFO("GateWay:%s\n", st_ipc_net_info.stGatewayIpAddr.csIpV4);
	    PRINT_INFO("Submask:%s\n", st_ipc_net_info.stEtherNet[0].strIPMask.csIpV4);		
	}
    sprintf(csMAC, "%02x:%02x:%02x:%02x:%02x:%02x", 
						st_ipc_net_info.stEtherNet[0].byMACAddr[0],
                         st_ipc_net_info.stEtherNet[0].byMACAddr[1],
                          st_ipc_net_info.stEtherNet[0].byMACAddr[2],
                           st_ipc_net_info.stEtherNet[0].byMACAddr[3],
                            st_ipc_net_info.stEtherNet[0].byMACAddr[4],
                             st_ipc_net_info.stEtherNet[0].byMACAddr[5]);  
	    
#endif
	ST_SDK_NETWORK_ATTR net_attr;

	if (netcam_net_get_detect("eth0") != 0)
		strncpy(net_attr.name,netcam_net_wifi_get_devname(),sizeof(net_attr.name));
	else
		strncpy(net_attr.name,"eth0",sizeof(net_attr.name));
	
	if (netcam_net_get(&net_attr))
	{
		PRINT_ERR("Fail to get net_attr\n");
		return -1;
	}

	sprintf(csHostIP, "0x%08x", inet_addr(net_attr.ip));
	sprintf(csGateWay, "0x%08x", inet_addr(net_attr.gateway));
	sprintf(csSubmask, "0x%08x", inet_addr(net_attr.mask));
	sprintf(csMAC, "%s", (char*)net_attr.mac);

	PRINT_INFO("net name:%s\n", net_attr.name);
	PRINT_INFO("HostIP:%s\n", net_attr.ip);
	PRINT_INFO("GateWay:%s\n", net_attr.gateway);
	PRINT_INFO("Submask:%s\n", net_attr.mask);
    PRINT_INFO("MAC:%s\n", csMAC);

    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name", "NetWork.NetCommon");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", "0x00000000");
    cJSON_AddItemToObject(root,"NetWork.NetCommon", NetCommon = cJSON_CreateObject());
    cJSON_AddStringToObject(NetCommon, "GateWay", csGateWay);//
    cJSON_AddStringToObject(NetCommon, "HostIP", csHostIP);//
    cJSON_AddStringToObject(NetCommon, "HostName", "LocalHost");
    cJSON_AddNumberToObject(NetCommon, "HttpPort", 80);//st_ipc_net_info.wHttpPort);
    cJSON_AddStringToObject(NetCommon, "MAC", csMAC);//
    cJSON_AddNumberToObject(NetCommon, "MaxBps", 0);
    cJSON_AddStringToObject(NetCommon, "MonMode", "TCP");
    cJSON_AddNumberToObject(NetCommon, "SSLPort", XMAI_SSL_PORT);
    cJSON_AddStringToObject(NetCommon, "Submask", csSubmask);//
    cJSON_AddNumberToObject(NetCommon, "TCPMaxConn", XMAI_MAX_LINK_NUM);
    cJSON_AddNumberToObject(NetCommon, "TCPPort", XMAI_TCP_PORT);
    cJSON_AddStringToObject(NetCommon, "TransferPlan", "Quality");
    cJSON_AddNumberToObject(NetCommon, "UDPPort", XMAI_UDP_PORT);
    cJSON_AddFalseToObject(NetCommon, "UseHSDownLoad");
    
    out = cJSON_PrintUnformatted(root);
    PRINT_INFO("%s\n",out);

    //打包    
    char buf[XMAI_MSG_SEND_BUF_SIZE] = {0};
    xmaiMsgLen = XMaiMakeMsgUseBuf(buf, sizeof(buf), IPSEARCH_RSP, out, 0, 0);

    free(out);
    cJSON_Delete(root);
	if(xmaiMsgLen < 0)
	{
		PRINT_ERR();
		return -1;
	}

	//发送
	//PrintXMaiMsgHeader(buf);
	usleep(400000);
    ret = XMaiBoardCastSendTo(fSockSearchfd, buf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR("Stopsession close socket(%d)![send_ret:%d]\n", fSockSearchfd, ret);
        return -1;
    }
        
    return 0;
}

int ReqIpsearchHandle(int fSockSearchfd, char* msg)
{
    PRINT_INFO("<<ReqIpsearchHandle>>\n");
	#if 0
    XMaiMsgHeader *pXMaiMsgHeader = (XMaiMsgHeader *)msg;
    cJSON *json = NULL;
    char *out = NULL;

    if(pXMaiMsgHeader->dataLen > 0)
	{
        json = cJSON_Parse(msg + 20);
        if (!json)
		{
            PRINT_ERR("Error before: [%s]\n", cJSON_GetErrorPtr());
            return -1;
        }
        
        out = cJSON_PrintUnformatted(json);
        //PRINT_INFO("%s\n",out);
        cJSON_Delete(json);
        free(out);
    }
	#endif
	
    //回应
    return ResIpsearch(fSockSearchfd);
}

int ReqIpsearchRspHandle(int fSockSearchfd, char* msg)
{
    PRINT_INFO("<<ReqIpsearchRspHandle>>\n");
	//PrintXMaiMsgHeader(msg);

#if 0
	XMaiMsgHeader *pXMaiMsgHeader = (XMaiMsgHeader *)msg;
    cJSON *json = NULL;
    char *out = NULL;

    if(pXMaiMsgHeader->dataLen > 0)
	{	
	    json = cJSON_Parse(msg + 20);
	    if (!json)
		{
	        PRINT_ERR("Error before: [%s]\n", cJSON_GetErrorPtr());
	        return -1;
	    }
	    
	    out = cJSON_PrintUnformatted(json);
		//PRINT_INFO("%s\n",out);
	    cJSON_Delete(json);
	    free(out);
    }
#endif	

    //回应
    //ResIpsearch();
    
    return 0;
}

int ResIpSet(int fSockSearchfd, int status)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root;

    root = cJSON_CreateObject();//创建项目
    if(status == 0)
    	cJSON_AddNumberToObject(root, "Ret", 100);
	else
		cJSON_AddNumberToObject(root, "Ret", 101);
	
    cJSON_AddStringToObject(root, "SessionID", "0x00000000");
    
    out = cJSON_PrintUnformatted(root);
    PRINT_INFO("%s\n",out);

    //打包    
    char buf[XMAI_MSG_SEND_BUF_SIZE] = {0};
    xmaiMsgLen = XMaiMakeMsgUseBuf(buf, sizeof(buf), IP_SET_RSP, out, 0, 0);

    free(out);
    cJSON_Delete(root);
	if(xmaiMsgLen < 0)
	{
		PRINT_ERR();
		return -1;
	}

	//发送
	//PrintXMaiMsgHeader(buf);
    ret = XMaiBoardCastSendTo(fSockSearchfd, buf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR("Stopsession close socket(%d)![send_ret:%d]\n", fSockSearchfd, ret);
        return -1;
    }
		
    return 0;
}

int HandleIpSet(cJSON *json)
{
    if(json == NULL)
    {
    	PRINT_ERR();
        return -1;
    }
	cJSON *element;    
    char HostIP[30] = {0};
    char Submask[30] = {0};
	char GateWay[30] = {0};
	char Mac[30] = {0};
	//char csMAC[30] = {0};

    element = cJSON_GetObjectItem(json, "HostIP");
    if(element)
        HandleAddr(HostIP, element->valuestring);
	//printf("ip:0x%08x\n", inet_addr(HostIP));
	

    element = cJSON_GetObjectItem(json, "Submask");
    if(element)
        HandleAddr(Submask, element->valuestring);

    element = cJSON_GetObjectItem(json, "GateWay");
    if(element)
        HandleAddr(GateWay, element->valuestring);

    element = cJSON_GetObjectItem(json, "MAC");
    if(element)
    	sprintf(Mac, "%s", element->valuestring);

#if 0
	//获取本机网络参数，
    DMS_NET_NETWORK_CFG st_ipc_net_info;
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_NETCFG, 0, &st_ipc_net_info, sizeof(st_ipc_net_info)))
    {
       PRINT_ERR("DMS_NET_GET_NETCFG fail\n");
       return -1;
    } 

	//通过MAC地址判断是不是发给自己的包
    sprintf(csMAC, "%02x:%02x:%02x:%02x:%02x:%02x", 
						st_ipc_net_info.stEtherNet[0].byMACAddr[0],
                         st_ipc_net_info.stEtherNet[0].byMACAddr[1],
                          st_ipc_net_info.stEtherNet[0].byMACAddr[2],
                           st_ipc_net_info.stEtherNet[0].byMACAddr[3],
                            st_ipc_net_info.stEtherNet[0].byMACAddr[4],
                             st_ipc_net_info.stEtherNet[0].byMACAddr[5]); 
#endif
	ST_SDK_NETWORK_ATTR net_attr;

	if (netcam_net_get_detect("eth0") != 0)
		strncpy(net_attr.name,netcam_net_wifi_get_devname(),sizeof(net_attr.name));
	else
		strncpy(net_attr.name,"eth0",sizeof(net_attr.name));
	
	if (netcam_net_get(&net_attr))
	{
		PRINT_ERR("Fail to get net_attr\n");
		return -1;
	}

	if(strcmp(Mac, (char*)net_attr.mac) != 0)
	{
		PRINT_INFO("NVR Set IP To Other IPCAM.\n");
		return 1; //没有修改IP，返回1
	}
	else
	{
		PRINT_INFO("NVR Set IP To Me:\n");
	}
	
#if 0
	//修改前先判断IP是否合理
	int is_right = 0, ret = 0;
    ret = set_ip_addr("eth0", HostIP);
    if(ret < 0)
        is_right = -1;
	
    ret =  set_mask_addr("eth0", Submask);
    if(ret < 0)
        is_right = -1;
	
	if(is_right < 0)
		return -1; //IP不合理，返回-1
	
	//修改
	if(st_ipc_net_info.byEnableDHCP == 1)
	{
		system("killall udhcpc");
	}
	st_ipc_net_info.byEnableDHCP = 0;
	strncpy(st_ipc_net_info.stEtherNet[0].strIPAddr.csIpV4, HostIP, sizeof(st_ipc_net_info.stEtherNet[0].strIPAddr.csIpV4));
	strncpy(st_ipc_net_info.stEtherNet[0].strIPMask.csIpV4, Submask, sizeof(st_ipc_net_info.stEtherNet[0].strIPMask.csIpV4));
	strncpy(st_ipc_net_info.stGatewayIpAddr.csIpV4, GateWay, sizeof(st_ipc_net_info.stGatewayIpAddr.csIpV4));
	
	PRINT_INFO("ip:%s\n", st_ipc_net_info.stEtherNet[0].strIPAddr.csIpV4);
	PRINT_INFO("mask:%s\n", st_ipc_net_info.stEtherNet[0].strIPMask.csIpV4);
	PRINT_INFO("gateway:%s\n", st_ipc_net_info.stGatewayIpAddr.csIpV4);

	//设置
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_SET_NETCFG, 0, &st_ipc_net_info, sizeof(st_ipc_net_info)))
    {
       PRINT_ERR("DMS_NET_GET_NETCFG fail\n");
       return -1;
    } 
#endif

	strncpy(net_attr.ip, HostIP, sizeof(net_attr.ip));
	strncpy(net_attr.mask, Submask, sizeof(net_attr.mask));
	strncpy(net_attr.gateway, GateWay, sizeof(net_attr.gateway));

	if(netcam_net_set(&net_attr) == 0)
	{
		netcam_net_cfg_save();
	}
	return 0; ////修改IP成功，返回0
}

int ReqIpSetReqHandle(int fSockSearchfd, char* msg)
{
    PRINT_INFO("<<ReqIpSetReqHandle>>\n");
	//PrintXMaiMsgHeader(msg);
	
	XMaiMsgHeader *pXMaiMsgHeader = (XMaiMsgHeader *)msg;
    cJSON *json = NULL;
    char *out = NULL;
	int ret = 0;

    if(pXMaiMsgHeader->dataLen > 0)
	{	
	    json = cJSON_Parse(msg + 20);
	    if (!json)
		{
	        PRINT_ERR("Error before: [%s]\n", cJSON_GetErrorPtr());
	        return -1;
	    }
	    
	    out = cJSON_PrintUnformatted(json);
		PRINT_INFO("%s\n",out);

		//处理
		//ret 如果为0，则表示成功修改了IP，则需要重启
		//ret 如果为1，则表示不是修改本机的IP
		//ret 如果为-1，则表示设置的IP不合理,或者出错
		ret = HandleIpSet(json);
		
	    cJSON_Delete(json);
	    free(out);
    }
	
    //回应
    ResIpSet(fSockSearchfd, ret);

	//ret 如果为0，则表示成功修改了IP，则需要重启
	if(ret == 0)
	{
		//重启前关闭
		g_xmai_pthread_run = 0;	
		/* 保存修改到配置文件中 */
#if 0
	    int ret = dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_SET_SAVECFG, 0, 0, 0); 
	    if(0 != ret)
	    {
	        PRINT_ERR();
	        return -1;        
	    }
#endif
	}

	return 0;
}


/////////////////////////////////////////////////
int resRegister(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root;

	//产生session id
	pXMaiSessionCtrl->fSessionInt = UtGenSessionInt(&g_xmai_session);
	sprintf(pXMaiSessionCtrl->fSessionId, "0x%08x", pXMaiSessionCtrl->fSessionInt);
	//PRINT_INFO("Register: fSessionInt = %d, fSessionId = %s\n", pXMaiSessionCtrl->fSessionInt, pXMaiSessionCtrl->fSessionId);

	//将会话主socket加入 g_xmai_socket 数组
	ret = UtAddCmdSock(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSessionInt, &g_xmai_session);
	if(ret == -1) // 连接达到最大数，返回-1
	{
		PRINT_INFO("XMaiMainThread，link to xmai is max.\n"); 					
		return -1;
	}
	
    root = cJSON_CreateObject();//创建项目
    cJSON_AddNumberToObject(root, "AliveInterval",        20);
    cJSON_AddNumberToObject(root, "ChannelNum", 1);
    cJSON_AddNumberToObject(root, "ExtraChannel", 0);
    cJSON_AddStringToObject(root, "DeviceType ", "IPC");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);

    out = cJSON_PrintUnformatted(root);
    PRINT_INFO("LOGIN_RSP:%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, LOGIN_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//PrintXMaiMsgHeader(pXMaiSessionCtrl->fSendbuf);
	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0;
}

int ReqRegisterHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg)
{
    pXMaiSessionCtrl->fSessionType = CMD_SESSION;
    PRINT_INFO("<<ReqRegisterHandle>>  sock:%d sessionType:%d\n", pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSessionType);
    XMaiMsgHeader *pXMaiMsgHeader = (XMaiMsgHeader *)msg;
    cJSON *json = NULL;
    cJSON *element = NULL;
    char *out = NULL;
    char EncryptType[128]={0};
    char LoginType[128]={0};
    char PassWord[128]={0};
    char UserName[128]={0};

    if(pXMaiMsgHeader->dataLen == 0)
	{
        PRINT_ERR("dataLen == 0 no json\n");
        goto RES;
    }
    
	json = cJSON_Parse(msg+20);
    if (!json)
	{
        PRINT_ERR("Error before: [%s]\n", cJSON_GetErrorPtr());
        return -1;
    }
    
    out = cJSON_Print(json);
    //PRINT_INFO("LOGIN_REQ:%s\n",out);

    element = cJSON_GetObjectItem(json, "EncryptType");
    if(element)
        sprintf(EncryptType, "%s", element->valuestring);
    //PRINT_INFO("EncryptType:%s\n", EncryptType);

    element = cJSON_GetObjectItem(json, "LoginType");
    if(element)
        sprintf(LoginType, "%s", element->valuestring);
    //PRINT_INFO("LoginType:%s\n", LoginType);

    element = cJSON_GetObjectItem(json, "PassWord");
    if(element)
        sprintf(PassWord, "%s", element->valuestring);
    //PRINT_INFO("PassWord:%s\n", PassWord);

    element = cJSON_GetObjectItem(json, "UserName");
    if(element)
        sprintf(UserName, "%s", element->valuestring);
    //PRINT_INFO("UserName:%s\n", UserName);

    cJSON_Delete(json);
    free(out);
    
    //检验用户
    //todo

RES:
    //回应
    return resRegister(pXMaiSessionCtrl);

}

////////////////////////////////////////////////////
int resKeepAlive(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root;

    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name","KeepAlive");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);

    out=cJSON_PrintUnformatted(root);
    //PRINT_INFO("(socket:%d)%s\n", pXMaiSessionCtrl->accept_sock, out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, KEEPALIVE_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0;

}

int ReqkeepAliveHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg)
{
    //PRINT_INFO("<<ReqkeepAliveHandle>>  sock:%d sessionType:%d\n", pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSessionType);
    XMaiMsgHeader *pXMaiMsgHeader = (XMaiMsgHeader *)msg;
    cJSON *json = NULL;
    cJSON *element = NULL;
    char *out = NULL;
    char Name[128]={0};
    char SessionID[128]={0};

    if(pXMaiMsgHeader->dataLen == 0){
        //PRINT_INFO("recv msg dataLen == 0 no json\n");
        goto RES;
    }
	
	json = cJSON_Parse(msg+20);
    if (!json){
        PRINT_ERR("Error before: [%s]\n", cJSON_GetErrorPtr());
        return -1;
    }
    
    out = cJSON_Print(json);
    //PRINT_INFO("%s\n",out);

    element = cJSON_GetObjectItem(json, "Name");
    if(element)
        sprintf(Name, "%s", element->valuestring);
    //PRINT_INFO("Name:%s\n", Name);

    element = cJSON_GetObjectItem(json, "SessionID");
    if(element)
        sprintf(SessionID, "%s", element->valuestring);
    //PRINT_INFO("SessionID:%s\n", SessionID);

    cJSON_Delete(json);
    free(out);

RES:
    //回应
    return resKeepAlive(pXMaiSessionCtrl);

}

//设置套接字发送缓冲区大小
int TcpSetSocketSendBufSize(int fd, int inNewSize)
{
    socklen_t sizeSize = sizeof(inNewSize);
    if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char*)&inNewSize, sizeSize) < 0)
    {
        PRINT_ERR("SetSocketRcvBufSize error \n");
        return -1;
    }

    return 0;
}

int XmaiSetDataSockAttr(int fd)
{
    int opt = 1;
    int ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (ret < 0) {
        PRINT_ERR("setsockopt SO_REUSEADDR failed, error:%s\n", strerror(errno));
        return -1;
    }

	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1) {
		PRINT_ERR("Fail to get old flags [set_nonblocking]! errno[%d] errinfo[%s]\n",
			errno, strerror(errno));
		return -1;
	}
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		PRINT_ERR("Fail to set flags to O_NONBLOCK [set_nonblocking]! errno[%d] errinfo[%s]\n",
			errno, strerror(errno));
		return -1;
	}

	return 0;
}


////////////////////////////////////////////////////
int resMonitorClaim(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root;	

    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name","OPMonitor");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);

    out=cJSON_PrintUnformatted(root);
    //PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, MONITOR_CLAIM_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0;
}

int HandleMonitorAlarm(XMaiSessionCtrl *pXMaiSessionCtrl, cJSON *root)
{
	PRINT_INFO("SessionInt=%d\n", pXMaiSessionCtrl->fSessionInt);
    if(!root)
        return -1;
    
    cJSON *pOPMonitor ,*pAction,*pParameter,*pChannel,*pStreamType;
    char Action[256] = {0};
    char StreamType[256] = {0};
    int Channel = 0;
    
    pOPMonitor = cJSON_GetObjectItem(root, "OPMonitor");
    if(!pOPMonitor)
        return -1;
    
    pAction = cJSON_GetObjectItem(pOPMonitor, "Action");
    sprintf(Action, "%s", pAction->valuestring);     
    //PRINT_INFO("Action:%s\n", Action);

    pParameter = cJSON_GetObjectItem(pOPMonitor, "Parameter");
    
    pChannel = cJSON_GetObjectItem(pParameter, "Channel");
    if(pChannel)
        Channel = pChannel->valueint;
    //PRINT_INFO("Channel:%d\n", Channel);

    pStreamType = cJSON_GetObjectItem(pParameter, "StreamType"); //main
    if(pStreamType)
        sprintf(StreamType, "%s", pStreamType->valuestring);
    //PRINT_INFO("StreamType:%s\n", StreamType);

	UT_STREAM_TYPE type = UT_UNKNOWN;
    if(!strcmp(StreamType, "Main")){
		type = UT_MAIN_STREAM;
    }
	else if(!strcmp(StreamType, "Extra1") || !strcmp(StreamType, "Extra")){
        type = UT_SUB_STREAM;
    }   

	/* 设置媒体流socket 的发送buf大小 */
	//TcpSetSocketSendBufSize(pXMaiSessionCtrl->accept_sock, XMAI_STREAM_DATA_SEND_BUF_SIZE);

    /* 设置套接字为非阻塞 可重用 */
	XmaiSetDataSockAttr(pXMaiSessionCtrl->accept_sock);

	/* 将当前socket添加进数组，用来做媒体流传输 */
    int ret = UtAddLvSock(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSessionInt, type, &g_xmai_session);
	if(ret == -1) { // 出错，找不到对应的session_id
	
		PRINT_ERR("add lv sock err. data_sock:%d, sid:%d, stream_type:%d", pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSessionInt, type);							
		return -1;
	}	

    return 0;
}

int ReqMonitorClaimHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg)
{   
	pXMaiSessionCtrl->fSessionType = CONNECT_SESSION;
    PRINT_INFO("<<ReqMonitorClaimHandle>>  data sock:%d sessionType:%d\n", pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSessionType);
    XMaiMsgHeader *pXMaiMsgHeader = (XMaiMsgHeader *)msg;
    cJSON *json = NULL;    
    char *out = NULL;
    //char SessionID[128]={0};
    
    if(pXMaiMsgHeader->dataLen == 0){
        PRINT_ERR("dataLen == 0 no json   must have jason\n");
        return -1;
    }
	
	json = cJSON_Parse(msg + 20);
    if (!json){
        PRINT_ERR("Error before: [%s]\n", cJSON_GetErrorPtr());
        return -1;
    }
    
    out = cJSON_Print(json);
    PRINT_INFO("%s\n",out);

    //处理
    int ret = HandleMonitorAlarm(pXMaiSessionCtrl, json);
    if (ret < 0) {
        PRINT_ERR("to close data sock:%d", pXMaiSessionCtrl->accept_sock);
        close(pXMaiSessionCtrl->accept_sock);
        return -1;
    }
    

    cJSON_Delete(json);
    free(out);
	
    //回应
    return resMonitorClaim(pXMaiSessionCtrl);

}


///////////////////////////////////////////////////////////
int HandleMonitorReqStart(XMaiSessionCtrl *pXMaiSessionCtrl, int channel, char *streamtype, char *transmode)
{    
    //PRINT_INFO("HandleMonitorReqStart sock:%d channel:%d\n", pXMaiSessionCtrl->accept_sock, channel);

    if(!strcmp(streamtype, "Main")){
        PRINT_INFO("start main stream, sock:%d, sid:%d\n", pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSessionInt);

		//通过flag打开主码流
        UtSetLvSockStatus(pXMaiSessionCtrl->fSessionInt, 1, UT_MAIN_STREAM, &g_xmai_session);
    }
    if(!strcmp(streamtype, "Extra1") || !strcmp(streamtype, "Extra")){
        PRINT_INFO("start sub stream, sock:%d, sid:%d\n", pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSessionInt);

		//通过flag打开子码流
        UtSetLvSockStatus(pXMaiSessionCtrl->fSessionInt, 1, UT_SUB_STREAM, &g_xmai_session);
    }

    return 0;
}

int HandleMonitorReqStop(XMaiSessionCtrl *pXMaiSessionCtrl, int channel, char *streamtype, char *transmode)
{
    //PRINT_INFO("HandleMonitorReqStop sock:%d channel:%d\n", pXMaiSessionCtrl->accept_sock, channel);

    if(!strcmp(streamtype, "Main")){
        PRINT_INFO("stop main stream, sock:%d, sid:%d\n", pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSessionInt);

		//通过flag关闭主码流
        UtSetLvSockStatus(pXMaiSessionCtrl->fSessionInt, 0, UT_MAIN_STREAM, &g_xmai_session);
    }
    if(!strcmp(streamtype, "Extra1") || !strcmp(streamtype, "Extra")){
        PRINT_INFO("stop sub stream, sock:%d, sid:%d\n", pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSessionInt);

		//通过flag关闭子码流
        UtSetLvSockStatus(pXMaiSessionCtrl->fSessionInt, 0, UT_SUB_STREAM, &g_xmai_session);
    }

    return 0;
}

int HandleMonitorReq(XMaiSessionCtrl *pXMaiSessionCtrl, cJSON *root)
{
    if(!root)
        return -1;
    
    cJSON *pOPMonitor ,*pAction,*pParameter,*pChannel,*pStreamType,*pTransMode;
    char Action[256] = {0};
    char StreamType[256] = {0};
    char TransMode[256] = {0};
    int Channel = 0;
    
    pOPMonitor = cJSON_GetObjectItem(root, "OPMonitor");
    if(!pOPMonitor)
        return -1;
    
    pAction = cJSON_GetObjectItem(pOPMonitor, "Action");
    if(!pAction)
        return -1;
    sprintf(Action, "%s", pAction->valuestring);
     
    //PRINT_INFO("Action:%s\n", Action);

    pParameter = cJSON_GetObjectItem(pOPMonitor, "Parameter");
    if(!pParameter)
        return -1;
    
    pChannel = cJSON_GetObjectItem(pParameter, "Channel");
    if(pChannel)
        Channel = pChannel->valueint;
    //PRINT_INFO("Channel:%d\n", Channel);

    pStreamType = cJSON_GetObjectItem(pParameter, "StreamType"); //main
    if(pStreamType)
        sprintf(StreamType, "%s", pStreamType->valuestring);
    //PRINT_INFO("StreamType:%s\n", StreamType);

    pTransMode = cJSON_GetObjectItem(pParameter, "TransMode"); //TCP
    if(pTransMode)
        sprintf(TransMode, "%s", pTransMode->valuestring);
    //PRINT_INFO("TransMode:%s\n", TransMode);

    if(!strcmp(Action, "Start")){
        HandleMonitorReqStart(pXMaiSessionCtrl, Channel, StreamType, TransMode);
    }
    else if(!strcmp(Action, "Stop")){
        HandleMonitorReqStop(pXMaiSessionCtrl, Channel, StreamType, TransMode);
    }   

    return 0;
}


int resMonitorReq(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root;

    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name","OPMonitor");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);

    out=cJSON_PrintUnformatted(root);
    //PRINT_INFO("%s\n",out);


  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, MONITOR_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0;
}


int ReqMonitorReqHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg)
{
    PRINT_INFO("<<ReqMonitorReqHandle>>  sock:%d sessionType:%d\n", pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSessionType);
    XMaiMsgHeader *pXMaiMsgHeader = (XMaiMsgHeader *)msg;
    cJSON *json = NULL;
    char *out = NULL;

    if(pXMaiMsgHeader->dataLen == 0)
	{
        PRINT_ERR("dataLen == 0 no json   must have jason\n");
        return -1;
    }
    
	json=cJSON_Parse(msg+20);
    if (!json)
	{
        PRINT_ERR("Error before: [%s]\n", cJSON_GetErrorPtr());
        return -1;
    }
    
    out = cJSON_Print(json);
    PRINT_INFO("%s\n",out);

    //处理
    HandleMonitorReq(pXMaiSessionCtrl, json);

    cJSON_Delete(json);
    free(out);
	
    //回应
    return resMonitorReq(pXMaiSessionCtrl);
}


/* */
int resGeneralInfo(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root, *fmt;
//    char str[1024] = {0};

#if 0
	//获取参数
    DMS_NET_DEVICE_INFO stDevicInfo;
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_DEVICECFG, 0, &stDevicInfo, sizeof(stDevicInfo)))
    {
        PRINT_ERR();
		return -1;		
    } 
#endif

    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name", "General.General");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);
    cJSON_AddItemToObject(root, "General.General", fmt=cJSON_CreateObject());//在项目上添加项目

#if 0
    cJSON_AddNumberToObject(fmt, "AutoLogout", 0);
    cJSON_AddNumberToObject(fmt, "LocalNo", 0);
    cJSON_AddNumberToObject(fmt, "ScreenAutoShutdown", 10);
    cJSON_AddNumberToObject(fmt, "ScreenSaveTime", 0);
    cJSON_AddNumberToObject(fmt, "SnapInterval", 2);
    cJSON_AddStringToObject(fmt, "MachineName", "LocalHost");
    cJSON_AddStringToObject(fmt, "OverWrite", "OverWrite");
    cJSON_AddStringToObject(fmt, "VideoOutPut", "Auto");
#else
    cJSON_AddNumberToObject(fmt, "AutoLogout", 0);
    cJSON_AddNumberToObject(fmt, "LocalNo", 0);
    cJSON_AddNumberToObject(fmt, "ScreenAutoShutdown", 10);
    cJSON_AddNumberToObject(fmt, "ScreenSaveTime", 0);
    cJSON_AddNumberToObject(fmt, "SnapInterval", 2);
    cJSON_AddStringToObject(fmt, "MachineName", "LocalHost");

	switch(runRecordCfg.recycleRecord)
	{
		case 0:
		default:
			cJSON_AddStringToObject(fmt, "OverWrite", "StopRecord");
			break;
		case 1:
			cJSON_AddStringToObject(fmt, "OverWrite", "OverWrite");
			break;			
	}	
    cJSON_AddStringToObject(fmt, "VideoOutPut", "Auto");	
#endif

    out=cJSON_PrintUnformatted(root);
    PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, CONFIG_GET_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0;
}

int resLocationInfo(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root, *fmt,*fmt2,*fmt3;
//    char str[1024] = {0};
	DMS_NET_DEVICE_INFO stDevicInfo;
	DMS_NET_ZONEANDDST stZoneAndDst;

#if 0
	//获取参数
    DMS_NET_DEVICE_INFO stDevicInfo;
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_DEVICECFG, 0, &stDevicInfo, sizeof(stDevicInfo)))
    {
        PRINT_ERR();
		return -1;		
    } 

	DMS_NET_ZONEANDDST stZoneAndDst;
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_ZONEANDDSTCFG, 0, &stZoneAndDst, sizeof(stZoneAndDst)))
    {
        PRINT_ERR();
		return -1;		
    }	

	DMS_NET_CHANNEL_OSDINFO stChannelOsdinfo;
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_OSDCFG, 0, &stChannelOsdinfo, sizeof(stChannelOsdinfo)))
    {
        PRINT_ERR();
		return -1;		
    }
#endif

    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name", "General.Location");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);
    cJSON_AddItemToObject(root, "General.Location", fmt=cJSON_CreateObject());//在项目上添加项目


	switch(runChannelCfg.channelInfo[0].osdDatetime.dateFormat)
	{
		case 0:
        case 2:
		default:
			cJSON_AddStringToObject(fmt, "DateFormat", "YYMMDD");
			break;
		case 1:
        case 3:
			cJSON_AddStringToObject(fmt, "DateFormat", "MMDDYY");
			break;
		case 4:
        case 5:
			cJSON_AddStringToObject(fmt, "DateFormat", "DDMMYY");
			break;				
	}

	switch(runChannelCfg.channelInfo[0].osdDatetime.dateSprtr)
	{
		case 0:
			cJSON_AddStringToObject(fmt, "DateSeparator", ":");  //支持 : ,待验证
			break;
		case 1:
        default:
			cJSON_AddStringToObject(fmt, "DateSeparator", "-");
			break;
		case 2:
			cJSON_AddStringToObject(fmt, "DateSeparator", "/");
			break;	
 		case 3:
			cJSON_AddStringToObject(fmt, "DateSeparator", ".");
			break;	
	}	

	switch(runSystemCfg.deviceInfo.languageType)
	{
		case 1:
		default:
			cJSON_AddStringToObject(fmt, "Language", "English");
			break;
		case 0:
			cJSON_AddStringToObject(fmt, "Language", "SimpChinese");
			break;			
	}      

    #if 0
	switch(runChannelCfg.channelInfo[0].osdDatetime.timeFmt)
	{
		case 0:
		default:
			cJSON_AddStringToObject(fmt, "TimeFormat", "24");
			break;
		case 1:
			cJSON_AddStringToObject(fmt, "TimeFormat", "12");
			break;			
	}	
	#else
	cJSON_AddStringToObject(fmt, "TimeFormat", "24"); //只支持24小时制
	#endif

	//stDevicInfo.byVideoStandard = DMS_PAL;
	switch(runSystemCfg.deviceInfo.videoType)
	{
		case GK_PAL:
		default:
			cJSON_AddStringToObject(fmt, "VideoFormat", "PAL");
			break;
		case GK_NTSC:
			cJSON_AddStringToObject(fmt, "VideoFormat", "NTSC");
			break;			
	}	

	#if 0
	stZoneAndDst.dwEnableDST = 0;
	switch(stZoneAndDst.dwEnableDST)
	{
		case 0:
		default:
			cJSON_AddStringToObject(fmt, "DSTRule", "Off");
			break;
		case 1:
			cJSON_AddStringToObject(fmt, "DSTRule", "On");
			break;			
	}
	#else
	cJSON_AddStringToObject(fmt, "DSTRule", "Off"); //不支持夏令时
	#endif

	stZoneAndDst.stEndPoint.dwMonth = 7;
	stZoneAndDst.stEndPoint.dwWeekDate = 0;
	stZoneAndDst.stEndPoint.dwHour = 23;
	stZoneAndDst.stEndPoint.dwMin = 59;
	stZoneAndDst.stEndPoint.dwWeekNo = 0;

	stZoneAndDst.stBeginPoint.dwMonth = 4;
	stZoneAndDst.stBeginPoint.dwWeekDate = 4;
	stZoneAndDst.stBeginPoint.dwHour = 23;
	stZoneAndDst.stBeginPoint.dwMin = 59;
	stZoneAndDst.stBeginPoint.dwWeekNo = 3;
	
    cJSON_AddNumberToObject(fmt, "WorkDay", 62);
    cJSON_AddItemToObject(fmt, "DSTEnd", fmt2=cJSON_CreateObject());
    cJSON_AddNumberToObject(fmt2, "Day", stZoneAndDst.stEndPoint.dwWeekDate);
    cJSON_AddNumberToObject(fmt2, "Hour", stZoneAndDst.stEndPoint.dwHour);
    cJSON_AddNumberToObject(fmt2, "Minute", stZoneAndDst.stEndPoint.dwMin);
    cJSON_AddNumberToObject(fmt2, "Month", stZoneAndDst.stEndPoint.dwMonth);
    cJSON_AddNumberToObject(fmt2, "Week", stZoneAndDst.stEndPoint.dwWeekNo);
    cJSON_AddNumberToObject(fmt2, "Year", 2014);
    cJSON_AddItemToObject(fmt, "DSTStart", fmt3=cJSON_CreateObject());
    cJSON_AddNumberToObject(fmt3, "Day", stZoneAndDst.stBeginPoint.dwWeekDate);
    cJSON_AddNumberToObject(fmt3, "Hour", stZoneAndDst.stBeginPoint.dwHour);
    cJSON_AddNumberToObject(fmt3, "Minute", stZoneAndDst.stBeginPoint.dwMin);
    cJSON_AddNumberToObject(fmt3, "Month", stZoneAndDst.stBeginPoint.dwMonth);
    cJSON_AddNumberToObject(fmt3, "Week", stZoneAndDst.stBeginPoint.dwWeekNo);
    cJSON_AddNumberToObject(fmt3, "Year", 2014);	


    out=cJSON_PrintUnformatted(root);
    PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, CONFIG_GET_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0;      
}

int resCameraParam(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    PRINT_INFO("resCameraParam\n");
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root;
//    char aDns[256] = {0};
 //   char Address[256] = {0};
//    char SpareAddress[256] = {0};

    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name", "Camera.Param.[0]");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);
    
    out=cJSON_PrintUnformatted(root);
    PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, CONFIG_GET_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0;
}

int resVideoColor(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root, *fmtArray, *fmtObject,*fmtObject2;
//    char str[1024] = {0};
	DMS_NET_CHANNEL_COLOR stChannelColor;
	DMS_NET_COLOR_SUPPORT  stColorSupport;
	GK_NET_IMAGE_CFG imagAttr;

#if 0
	//获取参数
    DMS_NET_CHANNEL_COLOR stChannelColor;
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_COLORCFG, 0, &stChannelColor, sizeof(stChannelColor)))
    {
        PRINT_ERR();
		return -1;		
    } 
    //获取设备的颜色参数最大值
    DMS_NET_COLOR_SUPPORT  stColorSupport;
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_COLOR_SUPPORT, 0, &stColorSupport, sizeof(DMS_NET_COLOR_SUPPORT)))
    {
        PRINT_ERR();
		return -1;	
    }
#endif

	ret = netcam_image_get(&imagAttr);
    if(ret != 0)
	{
		PRINT_ERR("XMai: fail to get image attr.\n");
		return -1;
	}

	stChannelColor.nBrightness = imagAttr.brightness;
	stChannelColor.nContrast   = imagAttr.contrast;
	stChannelColor.nHue		  = imagAttr.hue;
	stChannelColor.nSaturation = imagAttr.saturation;
	stChannelColor.nDefinition = imagAttr.sharpness;

	stColorSupport.strBrightness.nMax = 100;
	stColorSupport.strContrast.nMax = 100;
	stColorSupport.strSaturation.nMax = 100;
	stColorSupport.strHue.nMax = 100;
	
    int bright = (int) (stChannelColor.nBrightness *100/stColorSupport.strBrightness.nMax);
    int contrast = (int) (stChannelColor.nContrast *100/stColorSupport.strContrast.nMax);
	int saturation = (int) (stChannelColor.nSaturation *100/stColorSupport.strSaturation.nMax );
    int hue = (int) (stChannelColor.nHue *100/stColorSupport.strHue.nMax);
    //int gain = (int) (stChannelColor.nDefinition *100/stColorSupport.strDefinition.nMax);
    
	PRINT_INFO("get color:\n");
    PRINT_INFO("bright:%d from %d\n", bright, stChannelColor.nBrightness);
	PRINT_INFO("contrast:%d from %d\n", contrast, stChannelColor.nContrast);
	PRINT_INFO("saturation:%d from %d\n", saturation, stChannelColor.nSaturation);
	PRINT_INFO("hue:%d from %d\n", hue, stChannelColor.nHue);    
    //PRINT_INFO("gain:%d from %d\n", gain, stChannelColor.nDefinition);	

    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name", "AVEnc.VideoColor.[0]");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);
    cJSON_AddItemToObject(root, "AVEnc.VideoColor.[0]", fmtArray=cJSON_CreateArray());//在项目上添加项目
    cJSON_AddItemToArray(fmtArray, fmtObject = cJSON_CreateObject());
    cJSON_AddTrueToObject(fmtObject, "Enable");
    cJSON_AddStringToObject(fmtObject, "TimeSection", "0 00:00:00-24:00:00");
    cJSON_AddItemToObject(fmtObject, "VideoColorParam", fmtObject2=cJSON_CreateObject());
    cJSON_AddNumberToObject(fmtObject2, "Brightness", bright);
    cJSON_AddNumberToObject(fmtObject2, "Contrast", contrast);
    cJSON_AddNumberToObject(fmtObject2, "Hue", hue);
    cJSON_AddNumberToObject(fmtObject2, "Gain", 0);
    cJSON_AddNumberToObject(fmtObject2, "Saturation", saturation);

    out=cJSON_PrintUnformatted(root);
    //PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, CONFIG_GET_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0;
}

int resDigManagerShow(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root, *fmt;
//    const char *strings[1]={"CAM01"};

    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name", "NetWork.DigManagerShow");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);
    cJSON_AddItemToObject(root,"NetWork.DigManagerShow", fmt = cJSON_CreateObject());
    cJSON_AddStringToObject(fmt, "DigManagerShow", "ShowAll");
    
    out=cJSON_PrintUnformatted(root);
    //PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, CONFIG_GET_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0;
}

int resStoragePosition(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root, *fmt;
//    const char *strings[1]={"CAM01"};

    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name", "Storage.StoragePosition");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);
    cJSON_AddItemToObject(root,"Storage.StoragePosition", fmt = cJSON_CreateObject());
    cJSON_AddTrueToObject(fmt, "SATA");
    cJSON_AddTrueToObject(fmt, "USB");
    cJSON_AddTrueToObject(fmt, "SD");
    cJSON_AddFalseToObject(fmt, "DVD");
    
    out=cJSON_PrintUnformatted(root);
    //PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, CONFIG_GET_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0;
}

int resEncodeStaticParam(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root, *fmt,*fmt2;
//    const char *strings[1]={"CAM01"};

    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name", "AVEnc.EncodeStaticParam");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);
    cJSON_AddItemToObject(root,"AVEnc.EncodeStaticParam", fmt = cJSON_CreateArray());
    cJSON_AddItemToArray(fmt, fmt2 = cJSON_CreateObject());
    cJSON_AddNumberToObject(fmt2, "Level", 40);//待计算 //kbps为单位
    cJSON_AddNumberToObject(fmt2, "Profile", 3);
    
    out=cJSON_PrintUnformatted(root);
    //PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, CONFIG_GET_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0;
}

int resSimplifyEncode(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root, *fmt ,*ExtraFormat, *MainFormat, *Video, *channel0;
#if 0    
    //获取设备的音视频通道压缩参数
    DMS_NET_CHANNEL_PIC_INFO channel_pic_info;
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_PICCFG, 0, &channel_pic_info, sizeof(DMS_NET_CHANNEL_PIC_INFO)))
    {
        PRINT_ERR();
		return -1;	
    }
#endif
    
    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name", "Simplify.Encode");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);
    cJSON_AddItemToObject(root,"Simplify.Encode", fmt = cJSON_CreateArray());
    cJSON_AddItemToArray(fmt, channel0 = cJSON_CreateObject());

	//DMS_NET_COMPRESSION_INFO *pCompressionInfo = NULL;

	//发主码流信息
	//pCompressionInfo = &(channel_pic_info.stRecordPara);
	cJSON_AddItemToObject(channel0,"MainFormat", MainFormat = cJSON_CreateObject());
    cJSON_AddItemToObject(MainFormat,"Video", Video = cJSON_CreateObject());

    //kbps
    cJSON_AddNumberToObject(Video, "BitRate", runVideoCfg.vencStream[0].h264Conf.bps);//pCompressionInfo->dwBitRate/1000);//
    if(runVideoCfg.vencStream[0].h264Conf.rc_mode == 0)
        cJSON_AddStringToObject(Video, "BitRateControl", "CBR");//
    else if(runVideoCfg.vencStream[0].h264Conf.rc_mode == 1)
        cJSON_AddStringToObject(Video, "BitRateControl", "VBR");
	
    if(runVideoCfg.vencStream[0].enctype == 1)
	{
        cJSON_AddStringToObject(Video, "Compression", "H.264");//
    }
    else if(runVideoCfg.vencStream[0].enctype == 2)
	{
        cJSON_AddStringToObject(Video, "Compression", "MJPG");//
    }
	

    cJSON_AddNumberToObject(Video, "FPS", runVideoCfg.vencStream[0].h264Conf.fps);//	
    cJSON_AddNumberToObject(Video, "GOP", runVideoCfg.vencStream[0].h264Conf.gop / runVideoCfg.vencStream[0].h264Conf.fps);//[2-12]  我们的是1~100

    int quality = runVideoCfg.vencStream[0].h264Conf.quality + 3;
    if (quality > 6)
        quality = 6;
	cJSON_AddNumberToObject(Video, "Quality", quality);//6-pCompressionInfo->dwImageQuality);


    //PRINT_INFO("StreamFormat:%lu\n", pCompressionInfo->dwStreamFormat);
    int width = runVideoCfg.vencStream[0].h264Conf.width;
    int height = runVideoCfg.vencStream[0].h264Conf.height;
	if (width == 1920 && height == 1080) //SDK_CAPTURE_SIZE_1080P
		cJSON_AddStringToObject(Video, "Resolution", "1080P");
	else if (width == 1280 && height == 960 )
		cJSON_AddStringToObject(Video, "Resolution", "1_3M");
	else if (width == 1280 && height == 720)
		cJSON_AddStringToObject(Video, "Resolution", "720P");
    else
        cJSON_AddStringToObject(Video, "Resolution", "720P");

    cJSON_AddTrueToObject(MainFormat, "VideoEnable");		
	if(runAudioCfg.mode > 0)
    	cJSON_AddTrueToObject(MainFormat, "AudioEnable");
	else
		cJSON_AddFalseToObject(MainFormat, "AudioEnable");


	//发子码流信息
	//pCompressionInfo = &(channel_pic_info.stNetPara);
	cJSON_AddItemToObject(channel0,"ExtraFormat", ExtraFormat = cJSON_CreateObject());
    cJSON_AddItemToObject(ExtraFormat,"Video", Video = cJSON_CreateObject());
	
    cJSON_AddNumberToObject(Video, "BitRate", runVideoCfg.vencStream[1].h264Conf.bps);
    if(runVideoCfg.vencStream[1].h264Conf.rc_mode == 0)
        cJSON_AddStringToObject(Video, "BitRateControl", "CBR");//
    else if(runVideoCfg.vencStream[1].h264Conf.rc_mode == 1)
        cJSON_AddStringToObject(Video, "BitRateControl", "VBR");
	
    if(runVideoCfg.vencStream[1].enctype == 1)
	{
        cJSON_AddStringToObject(Video, "Compression", "H.264");//
    }
    else if(runVideoCfg.vencStream[1].enctype == 2)
	{
        cJSON_AddStringToObject(Video, "Compression", "MJPG");//
    }

    cJSON_AddNumberToObject(Video, "FPS", runVideoCfg.vencStream[1].h264Conf.fps);//	
    cJSON_AddNumberToObject(Video, "GOP", runVideoCfg.vencStream[1].h264Conf.gop / runVideoCfg.vencStream[1].h264Conf.fps);//[2-12]  我们的是1~100

    quality = runVideoCfg.vencStream[1].h264Conf.quality + 3;
    if (quality > 6)
        quality = 6;
	cJSON_AddNumberToObject(Video, "Quality", quality);//6-pCompressionInfo->dwImageQuality);


    //PRINT_INFO("StreamFormat:%lu\n", pCompressionInfo->dwStreamFormat);
    width = runVideoCfg.vencStream[1].h264Conf.width;
    height = runVideoCfg.vencStream[1].h264Conf.height;

	if (width == 1920 && height == 1080) //SDK_CAPTURE_SIZE_1080P
		cJSON_AddStringToObject(Video, "Resolution", "1080P");
	else if (width == 1280 && height == 960 )
		cJSON_AddStringToObject(Video, "Resolution", "1_3M");
	else if (width == 1280 && height == 720)
		cJSON_AddStringToObject(Video, "Resolution", "720P");
	else if ( (720 == width && 576 == height) || (720 == width && 480 == height) )
		cJSON_AddStringToObject(Video, "Resolution", "D1");
	else if ( (704 == width && 576 == height) || (704 == width && 480 == height) )
		cJSON_AddStringToObject(Video, "Resolution", "D1");
	else if ( (352 == width && 288 == height) || (352 == width && 240 == height) )
		cJSON_AddStringToObject(Video, "Resolution", "CIF");
	else if ( (176 == width && 144 == height) || (176 == width && 120 == height) )
		cJSON_AddStringToObject(Video, "Resolution", "QCIF");
	else if (width == 640 && height == 480)
		cJSON_AddStringToObject(Video, "Resolution", "VGA");
	else if (width == 320 && height == 240)
		cJSON_AddStringToObject(Video, "Resolution", "QVGA");
	else if (width == 160 && height == 120)
		cJSON_AddStringToObject(Video, "Resolution", "QQVGA");
    else
        cJSON_AddStringToObject(Video, "Resolution", "QVGA");

    cJSON_AddTrueToObject(ExtraFormat, "VideoEnable");			
	if(runAudioCfg.mode > 0)
    	cJSON_AddTrueToObject(ExtraFormat, "AudioEnable");
	else
		cJSON_AddFalseToObject(ExtraFormat, "AudioEnable");


    out=cJSON_Print(root);
    PRINT_INFO("%s\n",out);    

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, CONFIG_GET_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0;
}

int resNetCommon(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root, *fmt;

    char csHostIP[30] = {0};
    char csGateWay[30] = {0};
    char csMAC[30] = {0};
    char csSubmask[30] = {0};
#if 0
	//获取本机网络参数，
    DMS_NET_NETWORK_CFG st_ipc_net_info;
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_NETCFG, 0, &st_ipc_net_info, sizeof(st_ipc_net_info)))
    {
       PRINT_ERR("DMS_NET_GET_NETCFG fail\n");
       return -1;
    }   
    //获取无线网络参数，
    DMS_NET_WIFI_CONFIG st_wifi_net_info;
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_WIFICFG, 0, &st_wifi_net_info, sizeof(st_wifi_net_info)))
    {
       PRINT_ERR("DMS_NET_GET_WIFICFG fail\n");
       return -1;
    } 
#endif

	ST_SDK_NETWORK_ATTR net_attr;

	if (netcam_net_get_detect("eth0") != 0)
		strncpy(net_attr.name,netcam_net_wifi_get_devname(),sizeof(net_attr.name));
	else
		strncpy(net_attr.name,"eth0",sizeof(net_attr.name));
	
	if (netcam_net_get(&net_attr))
	{
		PRINT_ERR("Fail to get net_attr\n");
		return -1;
	}

#if 0
    //如果无线可用就只传无线的配置
    if(st_wifi_net_info.bWifiEnable && st_wifi_net_info.byStatus == 0)
    {
    	PRINT_INFO("wifi config:\n");
	    sprintf(csHostIP, "0x%08x", inet_addr(st_wifi_net_info.dwNetIpAddr.csIpV4));
	    sprintf(csGateWay, "0x%08x", inet_addr(st_wifi_net_info.dwGateway.csIpV4));
	    sprintf(csSubmask, "0x%08x", inet_addr(st_wifi_net_info.dwNetMask.csIpV4));	
	    PRINT_INFO("HostIP:%s\n", st_wifi_net_info.dwNetIpAddr.csIpV4);
	    PRINT_INFO("GateWay:%s\n", st_wifi_net_info.dwGateway.csIpV4);
	    PRINT_INFO("Submask:%s\n", st_wifi_net_info.dwNetMask.csIpV4);
	    		
    }
	else
	{
		PRINT_INFO("localhost config:\n");
	    sprintf(csHostIP, "0x%08x", inet_addr(st_ipc_net_info.stEtherNet[0].strIPAddr.csIpV4));
	    sprintf(csGateWay, "0x%08x", inet_addr(st_ipc_net_info.stGatewayIpAddr.csIpV4));
	    sprintf(csSubmask, "0x%08x", inet_addr(st_ipc_net_info.stEtherNet[0].strIPMask.csIpV4));	
	    PRINT_INFO("HostIP:%s\n", st_ipc_net_info.stEtherNet[0].strIPAddr.csIpV4);
	    PRINT_INFO("GateWay:%s\n", st_ipc_net_info.stGatewayIpAddr.csIpV4);
	    PRINT_INFO("Submask:%s\n", st_ipc_net_info.stEtherNet[0].strIPMask.csIpV4);		
	}
    sprintf(csMAC, "%02x:%02x:%02x:%02x:%02x:%02x", 
						st_ipc_net_info.stEtherNet[0].byMACAddr[0],
                         st_ipc_net_info.stEtherNet[0].byMACAddr[1],
                          st_ipc_net_info.stEtherNet[0].byMACAddr[2],
                           st_ipc_net_info.stEtherNet[0].byMACAddr[3],
                            st_ipc_net_info.stEtherNet[0].byMACAddr[4],
                             st_ipc_net_info.stEtherNet[0].byMACAddr[5]);  
	    
	PRINT_INFO("MAC:%s\n", csMAC);
#endif

	sprintf(csHostIP, "0x%08x", inet_addr(net_attr.ip));
	sprintf(csGateWay, "0x%08x", inet_addr(net_attr.gateway));
	sprintf(csSubmask, "0x%08x", inet_addr(net_attr.mask));
	sprintf(csMAC, "%s", (char*)net_attr.mac);

	PRINT_INFO("net name:%s\n", net_attr.name);
	PRINT_INFO("HostIP:%s\n", net_attr.ip);
	PRINT_INFO("GateWay:%s\n", net_attr.gateway);
	PRINT_INFO("Submask:%s\n", net_attr.mask);
    PRINT_INFO("MAC:%s\n", csMAC);

    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name", "NetWork.NetCommon");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);
    cJSON_AddItemToObject(root,"NetWork.NetCommon", fmt = cJSON_CreateObject());
    cJSON_AddNumberToObject(fmt, "HttpPort", 80); //st_ipc_net_info.wHttpPort);
    cJSON_AddNumberToObject(fmt, "MaxBps", 0);
    cJSON_AddNumberToObject(fmt, "SSLPort", XMAI_SSL_PORT);
    cJSON_AddNumberToObject(fmt, "TCPMaxConn", XMAI_MAX_LINK_NUM);
    cJSON_AddNumberToObject(fmt, "TCPPort", XMAI_TCP_PORT);
    cJSON_AddNumberToObject(fmt, "UDPPort", XMAI_UDP_PORT);
    cJSON_AddStringToObject(fmt, "GateWay", csGateWay);
    cJSON_AddStringToObject(fmt, "HostIP", csHostIP);
    cJSON_AddStringToObject(fmt, "HostName", "LocalHost");
    cJSON_AddStringToObject(fmt, "MAC", csMAC);
    cJSON_AddStringToObject(fmt, "MonMode", "TCP");
    cJSON_AddStringToObject(fmt, "Submask", csSubmask);
    cJSON_AddStringToObject(fmt, "TransferPlan", "Quality");
    cJSON_AddFalseToObject(fmt,"UseHSDownLoad");

    out=cJSON_PrintUnformatted(root);
    //PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, CONFIG_GET_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0;
}

int resNetDHCP(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root, *fmt,*fmt2;
#if 0
	//获取本机网络参数，
	DMS_NET_NETWORK_CFG	stNetWorkConfig;
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_NETCFG, 0, &stNetWorkConfig, sizeof(stNetWorkConfig)))
    {
       PRINT_ERR("DMS_NET_GET_NETCFG fail\n");
       return -1;
    } 
#endif
	int dhcp = 0;
	if (netcam_net_get_detect("eth0") != 0)
		dhcp = runNetworkCfg.wifi.dhcpIp;
	else
		dhcp = runNetworkCfg.lan.dhcpIp;

    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name", "NetWork.NetDHCP");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);
    cJSON_AddItemToObject(root,"NetWork.NetDHCP", fmt = cJSON_CreateArray());
    cJSON_AddItemToArray(fmt, fmt2 = cJSON_CreateObject());
    if(dhcp)
    {    	
        cJSON_AddTrueToObject (fmt2,"Enable");
		PRINT_INFO("enable dhcp.\n");
    }
    else
    {
        cJSON_AddFalseToObject (fmt2,"Enable");
		PRINT_INFO("disable dhcp.\n");
    }
    cJSON_AddStringToObject(fmt2, "Interface", "eth0");
    
    out=cJSON_PrintUnformatted(root);
    //PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, CONFIG_GET_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0;
}

int resNetDNS(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root, *fmt;
    //char aDns[256] = {0};
    char Address[256] = {0};
    char SpareAddress[256] = {0};

#if 0
	//获取本机网络参数，
	DMS_NET_NETWORK_CFG	stNetWorkConfig;
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_NETCFG, 0, &stNetWorkConfig, sizeof(stNetWorkConfig)))
    {
       PRINT_ERR("DMS_NET_GET_NETCFG fail\n");
       return -1;
    } 
#endif

	ST_SDK_NETWORK_ATTR net_attr;

	if (netcam_net_get_detect("eth0") != 0)
		strncpy(net_attr.name,netcam_net_wifi_get_devname(),sizeof(net_attr.name));
	else
		strncpy(net_attr.name,"eth0",sizeof(net_attr.name));
	
	if (netcam_net_get(&net_attr))
	{
		PRINT_ERR("Fail to get net_attr\n");
		return -1;
	}
	
	if(strlen(net_attr.dns1) == 0)
	{
	    sprintf(Address, "0x%08x", inet_addr("8.8.8.8"));		
		PRINT_INFO("dns1:%s\n", "8.8.8.8");	
	}
	else
	{
	    sprintf(Address, "0x%08x", inet_addr(net_attr.dns1));
		PRINT_INFO("dns1:%s\n", net_attr.dns1);
	}
	
	if(strlen(net_attr.dns2) == 0)
	{
	    sprintf(SpareAddress, "0x%08x", inet_addr("114.114.114.114"));		
		PRINT_INFO("dns2:%s\n", "114.114.114.114");	
	}
	else
	{
	    sprintf(SpareAddress, "0x%08x", inet_addr(net_attr.dns2));
	    PRINT_INFO("dns2:%s\n", net_attr.dns2);
	}

    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name", "NetWork.NetDNS");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);
    cJSON_AddItemToObject(root,"NetWork.NetDNS", fmt = cJSON_CreateObject());
    cJSON_AddStringToObject(fmt, "Address", Address);
    cJSON_AddStringToObject(fmt, "SpareAddress", SpareAddress);
    
    out=cJSON_PrintUnformatted(root);
    //PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, CONFIG_GET_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0;
}

int resVideoWidget(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root;
  //  char aDns[256] = {0};
 //   char Address[256] = {0};
//    char SpareAddress[256] = {0};

    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name", "AVEnc.VideoWidget");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);
    
    out=cJSON_PrintUnformatted(root);
    //PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, CONFIG_GET_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0;
}

int resAutoMaintain(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root, *fmt;
  //  char aDns[256] = {0};
 //   char Address[256] = {0};
//    char SpareAddress[256] = {0};

	//IPC没有自动维护结构体
    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name", "General.AutoMaintain");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);
    cJSON_AddItemToObject(root,"General.AutoMaintain", fmt = cJSON_CreateObject());
    cJSON_AddStringToObject(fmt, "AutoRebootDay", "Tuesday"); // 自动重启时间
    cJSON_AddNumberToObject(fmt, "AutoDeleteFilesDays", 0); //自动删除文件时间(天) [0, 365]
    cJSON_AddNumberToObject(fmt, "AutoRebootHour", 1); //自动重启小时[0, 23]
    
    out=cJSON_PrintUnformatted(root);
    //PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, CONFIG_GET_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0;
}

int resNatInfo(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root, *fmt;
    //char aDns[256] = {0};
    //char Address[256] = {0};
    //char SpareAddress[256] = {0};

    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name", "Status.NatInfo");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);
    cJSON_AddItemToObject(root,"Status.NatInfo", fmt = cJSON_CreateObject());
    cJSON_AddStringToObject(fmt, "NaInfoCode", "0:");
    cJSON_AddStringToObject(fmt, "NatStatus", "Probing");
    
    out=cJSON_PrintUnformatted(root);
    //PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, CONFIG_GET_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0;
}

int resSerialNo(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root, *fmt;
  //  char aDns[256] = {0};
 //   char Address[256] = {0};
//    char SpareAddress[256] = {0};

    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name", "Ability.SerialNo");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);
    cJSON_AddItemToObject(root,"Ability.SerialNo", fmt = cJSON_CreateObject());
    cJSON_AddStringToObject(fmt, "ProductType", "81XXF");
    cJSON_AddStringToObject(fmt, "SerialNo", "003e0b075838");
    
    out=cJSON_PrintUnformatted(root);
    //PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, CONFIG_GET_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0;
}

int resMotionDetect(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
	int day = 0, i = 0, j= 0;
    char *out;
    cJSON *root, *MotionDetectObject,*Region,*EventHandler,*TimeSection;
    
    //获取设备的音视频通道压缩参数
    DMS_NET_CHANNEL_MOTION_DETECT stChannelMotionDetect;
#if 0
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_MOTIONCFG, 0, &stChannelMotionDetect, sizeof(DMS_NET_CHANNEL_MOTION_DETECT)))
    {  
		PRINT_ERR();
		return -1;	
    }
#endif

	stChannelMotionDetect.bEnable = runMdCfg.enable;
    stChannelMotionDetect.dwSensitive = runMdCfg.sensitive;
    //md.bManualDefence = runMdCfg.schedule_mode;
    stChannelMotionDetect.stHandle.wDelayTime = runMdCfg.handle.intervalTime;
    stChannelMotionDetect.stHandle.wActionFlag = 0;

    if (runMdCfg.handle.is_rec == 1) {
        stChannelMotionDetect.stHandle.wActionFlag |= GK_ALARM_EXCEPTION_TOREC;
        stChannelMotionDetect.stHandle.byRecordChannel[0] |= 0x01;
        stChannelMotionDetect.stHandle.wRecTime = runMdCfg.handle.recTime;
    } else {
        stChannelMotionDetect.stHandle.wActionFlag &= ~(GK_ALARM_EXCEPTION_TOREC);
        stChannelMotionDetect.stHandle.byRecordChannel[0] &= ~(0x01);
    }

	if (runMdCfg.handle.isSnapUploadToFtp == 1) {
        stChannelMotionDetect.stHandle.wActionFlag |= GK_ALARM_EXCEPTION_TOFTP;
    } else {
        stChannelMotionDetect.stHandle.wActionFlag &= ~(GK_ALARM_EXCEPTION_TOFTP);
    }
	
    if (runMdCfg.handle.isSnapSaveToSd == 1) {
        stChannelMotionDetect.stHandle.wActionFlag |= GK_ALARM_EXCEPTION_TOSNAP;
        stChannelMotionDetect.stHandle.bySnap[0] |= 0x01;
        stChannelMotionDetect.stHandle.wSnapNum = runMdCfg.handle.snapNum;
    } else {
        stChannelMotionDetect.stHandle.wActionFlag &= ~(GK_ALARM_EXCEPTION_TOSNAP);
        //md.stHandle.wActionFlag &= ~(GK_ALARM_EXCEPTION_TOFTP);
        stChannelMotionDetect.stHandle.bySnap[0] &= ~(0x01);
    }

    if (runMdCfg.handle.is_alarmout == 1) {
        stChannelMotionDetect.stHandle.wActionFlag |= GK_ALARM_EXCEPTION_TOALARMOUT;
        stChannelMotionDetect.stHandle.byRelAlarmOut[0] |= 0x01;
        stChannelMotionDetect.stHandle.wDuration = runMdCfg.handle.duration;
    } else {
        stChannelMotionDetect.stHandle.wActionFlag &= ~(GK_ALARM_EXCEPTION_TOALARMOUT);
        stChannelMotionDetect.stHandle.byRelAlarmOut[0] &= ~(0x01);
    }

    if (runMdCfg.handle.is_ptz == 1) {
        stChannelMotionDetect.stHandle.stPtzLink[0].byType = runMdCfg.handle.ptzLink.type;
		stChannelMotionDetect.stHandle.stPtzLink[0].byValue = runMdCfg.handle.ptzLink.value + 1;
    }

    if (runMdCfg.handle.is_beep == 1) {
        stChannelMotionDetect.stHandle.wActionFlag |= GK_ALARM_EXCEPTION_TOBEEP;
        stChannelMotionDetect.stHandle.wBuzzerTime = runMdCfg.handle.beepTime;
    } else {
        stChannelMotionDetect.stHandle.wActionFlag &= ~(GK_ALARM_EXCEPTION_TOBEEP);
    }

    if (runMdCfg.handle.is_email == 1) {
        stChannelMotionDetect.stHandle.wActionFlag |= GK_ALARM_EXCEPTION_TOEMAIL;
    } else {
        stChannelMotionDetect.stHandle.wActionFlag &= ~(GK_ALARM_EXCEPTION_TOEMAIL);
    }

    //md.stHandle.wActionMask = ~0;
    stChannelMotionDetect.stHandle.wActionMask = 0;
    stChannelMotionDetect.stHandle.wActionMask |= GK_ALARM_EXCEPTION_TOREC;
    stChannelMotionDetect.stHandle.wActionMask |= GK_ALARM_EXCEPTION_TOSNAP;
    stChannelMotionDetect.stHandle.wActionMask |= GK_ALARM_EXCEPTION_TOFTP;
    //md.stHandle.wActionMask |= GK_ALARM_EXCEPTION_TOALARMOUT;
    stChannelMotionDetect.stHandle.wActionMask |= GK_ALARM_EXCEPTION_TOPTZ;
    stChannelMotionDetect.stHandle.wActionMask |= GK_ALARM_EXCEPTION_TOBEEP;
    stChannelMotionDetect.stHandle.wActionMask |= GK_ALARM_EXCEPTION_TOEMAIL;

    for(i = 0 ; i < 7; i ++) {
        for(j = 0 ; j < 4; j ++) {
            stChannelMotionDetect.stScheduleTime[i][j].byStartHour = runMdCfg.scheduleTime[i][j].startHour;
            stChannelMotionDetect.stScheduleTime[i][j].byStartMin = runMdCfg.scheduleTime[i][j].startMin;
            stChannelMotionDetect.stScheduleTime[i][j].byStopHour = runMdCfg.scheduleTime[i][j].stopHour;
            stChannelMotionDetect.stScheduleTime[i][j].byStopMin = runMdCfg.scheduleTime[i][j].stopMin;
        }
    }

#if 1	
	/* 从 44 * 36 转换为 22* 18 */
	int fromX = 44;
	int fromY = 36;
	int toX = 22;	
	int toY = 18;
	//22*18格子，整形数组第一个元素的前22位有效，一个位标示一个格子是否选中，总共18个元素有效
	unsigned long mRegion[18] = {0};
	char strings[18][12] = {{0}};	
	int x, y, index;

    //22*18格子，整形数组第一个元素的前22位有效，一个位标示一个格子是否选中，总共18个元素有效
    for(y = 0; y < toY; y++)
    {
        for(x = 0; x < toX; x++)
        {
			index = y*fromX*fromY/toY + x*fromX/toX ;
			//isSet = (stChannelMotionDetect.byMotionArea[index/8] >> (index%8)) & 1 ;
            //if( CHK_BIT(stChannelMotionDetect.byMotionArea, index) ) 
            //{
				mRegion[y] |= (1<<x);
            //}			
        }
    }     

	for(y = 0; y < toY; y++)
	{
        sprintf(strings[y], "0x%08lx", mRegion[y]);
		//printf("1 %d: %s\n", y, strings[y]);
	}
				
#endif

#if 0
    const char *strings1[6]={
                            "1 00:00:00-24:00:00", 
                            "0 00:00:00-24:00:00", 
                            "0 00:00:00-24:00:00", 
                            "0 00:00:00-24:00:00", 
                            "0 00:00:00-24:00:00", 
                            "0 00:00:00-24:00:00"};

#endif

	
    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name", "Detect.MotionDetect.[0]");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);
    cJSON_AddItemToObject(root,"Detect.MotionDetect.[0]", MotionDetectObject = cJSON_CreateObject());
	//printf("get MotionDetect Enable:%u\n", stChannelMotionDetect.bEnable);

#if 0	
    if(stChannelMotionDetect.bEnable)
    {    	
        cJSON_AddTrueToObject(MotionDetectObject, "Enable");
	    const char *strings[18]={"0x003FFFFF","0x003FFFFF","0x003FFFFF","0x003FFFFF","0x003FFFFF",
	                             "0x003FFFFF","0x003FFFFF","0x003FFFFF","0x003FFFFF","0x003FFFFF",
	                             "0x003FFFFF","0x003FFFFF","0x003FFFFF","0x003FFFFF","0x003FFFFF",
	                             "0x003FFFFF","0x003FFFFF","0x003FFFFF"};		
		cJSON_AddItemToObject(MotionDetectObject,"Region", Region = cJSON_CreateStringArray(strings,18));
    }
    else
    {
        cJSON_AddFalseToObject(MotionDetectObject, "Enable");
	    const char *strings[18]={"0x00000000","0x00000000","0x00000000","0x00000000","0x00000000",
	                             "0x00000000","0x00000000","0x00000000","0x00000000","0x00000000",
	                             "0x00000000","0x00000000","0x00000000","0x00000000","0x00000000",
	                             "0x00000000","0x00000000","0x00000000"};		
		cJSON_AddItemToObject(MotionDetectObject,"Region", Region = cJSON_CreateStringArray(strings,18));
    }
#else
    if(stChannelMotionDetect.bEnable)   	
        cJSON_AddTrueToObject(MotionDetectObject, "Enable");
    else
        cJSON_AddFalseToObject(MotionDetectObject, "Enable");

		
	cJSON_AddItemToObject(MotionDetectObject, "Region", Region = cJSON_CreateArray());	
	for(i = 0; i < 18; i ++)
		cJSON_AddItemToArray(Region, cJSON_CreateString(strings[i]));
#endif
	int iLevel = stChannelMotionDetect.dwSensitive * 5 / 100 + 1; //dwSensitive等级范围为0-100, Level等级范围为[1, 6]
    cJSON_AddNumberToObject(MotionDetectObject, "Level", iLevel);
    //cJSON_AddItemToObject(MotionDetectObject,"Region", Region = cJSON_CreateStringArray(strings,22));
	//cJSON_AddItemToObject(MotionDetectObject,"Region", Region = cJSON_CreateStringArray(strings,18));
	
    cJSON_AddItemToObject(MotionDetectObject,"EventHandler", EventHandler = cJSON_CreateObject());
    cJSON_AddStringToObject(EventHandler, "AlarmInfo", "");
    if(stChannelMotionDetect.stHandle.wActionFlag & GK_ALARM_EXCEPTION_TOALARMOUT){
        cJSON_AddTrueToObject(EventHandler, "AlarmOutEnable");
        cJSON_AddStringToObject(EventHandler, "AlarmOutMask", "0x00000001");
    }
    else{
        cJSON_AddFalseToObject(EventHandler, "AlarmOutEnable");
        cJSON_AddStringToObject(EventHandler, "AlarmOutMask", "0x00000000");
    }
    cJSON_AddNumberToObject(EventHandler, "AlarmOutLatch", stChannelMotionDetect.stHandle.wDuration); //报警输出延时时间（秒）

	
	if(stChannelMotionDetect.stHandle.wActionFlag & GK_ALARM_EXCEPTION_TOBEEP)
		cJSON_AddTrueToObject(EventHandler, "BeepEnable");
	else
    	cJSON_AddFalseToObject(EventHandler, "BeepEnable");
	
    cJSON_AddNumberToObject(EventHandler, "EventLatch", 1);
    if(stChannelMotionDetect.stHandle.wActionFlag & GK_ALARM_EXCEPTION_TOFTP)
        cJSON_AddTrueToObject(EventHandler, "FTPEnable");
    else
        cJSON_AddFalseToObject(EventHandler, "FTPEnable");
    cJSON_AddFalseToObject(EventHandler, "LogEnable");
    if(stChannelMotionDetect.stHandle.wActionFlag & GK_ALARM_EXCEPTION_TOEMAIL)
        cJSON_AddTrueToObject(EventHandler, "MailEnable");
    else
        cJSON_AddFalseToObject(EventHandler, "MailEnable");
    cJSON_AddFalseToObject(EventHandler, "MatrixEnable");
    cJSON_AddStringToObject(EventHandler, "MatrixMask", "0x00000000");
    cJSON_AddFalseToObject(EventHandler, "MessageEnable");
    cJSON_AddFalseToObject(EventHandler, "MsgtoNetEnable");
    cJSON_AddFalseToObject(EventHandler, "MultimediaMsgEnable");

	if(stChannelMotionDetect.stHandle.wActionFlag & GK_ALARM_EXCEPTION_TOPTZ)
		cJSON_AddTrueToObject(EventHandler, "PtzEnable");
	else
    	cJSON_AddFalseToObject(EventHandler, "PtzEnable");
	

    if(stChannelMotionDetect.stHandle.wActionFlag & GK_ALARM_EXCEPTION_TOREC){
        cJSON_AddTrueToObject(EventHandler, "RecordEnable");
        cJSON_AddStringToObject(EventHandler, "RecordMask", "0x00000001");
    }
    else{
        cJSON_AddFalseToObject(EventHandler, "RecordEnable");
        cJSON_AddStringToObject(EventHandler, "RecordMask", "0x00000000");
    }
    cJSON_AddNumberToObject(EventHandler, "RecordLatch", stChannelMotionDetect.stHandle.wRecTime); //录像延时时间（秒）
    
    cJSON_AddFalseToObject(EventHandler, "ShortMsgEnable");
    cJSON_AddFalseToObject(EventHandler, "ShowInfo");
    cJSON_AddStringToObject(EventHandler, "ShowInfoMask", "0x00000000");
    if(stChannelMotionDetect.stHandle.wActionFlag & GK_ALARM_EXCEPTION_TOSNAP){
        cJSON_AddTrueToObject(EventHandler, "SnapEnable");
        cJSON_AddStringToObject(EventHandler, "SnapShotMask", "0x00000001");
    }
    else{
        cJSON_AddFalseToObject(EventHandler, "SnapEnable");
        cJSON_AddStringToObject(EventHandler, "SnapShotMask", "0x00000000");
    }
    cJSON_AddFalseToObject(EventHandler, "TipEnable");
    cJSON_AddFalseToObject(EventHandler, "TourEnable");
    cJSON_AddStringToObject(EventHandler, "TourMask", "0x00000000");
    if(stChannelMotionDetect.stHandle.wActionFlag & GK_ALARM_EXCEPTION_TOVOICE)
        cJSON_AddTrueToObject(EventHandler, "VoiceEnable");
    else
        cJSON_AddFalseToObject(EventHandler, "VoiceEnable");

	/* 移动监测时间设置 */
	#if 0
	//打印时间段信息
	for(day = 0; day < 7; day++)
	{
		for(i = 0; i < 4; i++) //实际就2个有效
		{		
			printf("get, day:%d index:%d %02u:%02u:00-%02u:%02u:00\n",
			           day, i, 	stChannelMotionDetect.stScheduleTime[day][i].byStartHour,
								stChannelMotionDetect.stScheduleTime[day][i].byStartMin,
								stChannelMotionDetect.stScheduleTime[day][i].byStopHour,
								stChannelMotionDetect.stScheduleTime[day][i].byStopMin);	
		}			
	}
	#endif
	
    cJSON_AddItemToObject(EventHandler,"TimeSection", TimeSection = cJSON_CreateArray());
	for(day = 0; day < 7; day++)
	{
		#if 1
		char strings1[6][20] = {{0}}; //6个时间段
		for(i = 0; i < 2; i++)
		{

			sprintf(strings1[i], "1 %02u:%02u:00-%02u:%02u:00", 
									stChannelMotionDetect.stScheduleTime[day][i].byStartHour,
									stChannelMotionDetect.stScheduleTime[day][i].byStartMin,
									stChannelMotionDetect.stScheduleTime[day][i].byStopHour,
									stChannelMotionDetect.stScheduleTime[day][i].byStopMin);

		}
		sprintf(strings1[2], "%s", "0 00:00:00-00:00:00"); 
		sprintf(strings1[3], "%s", "0 00:00:00-00:00:00");
		sprintf(strings1[4], "%s", "0 00:00:00-00:00:00");
		sprintf(strings1[5], "%s", "0 00:00:00-00:00:00");

		cJSON *tmp = cJSON_CreateArray();
		for(i = 0; i < 6; i++)
		{
			cJSON_AddItemToArray(tmp, cJSON_CreateString(strings1[i]));
		}		
		cJSON_AddItemToArray(TimeSection, tmp);
		#else
	    const char *strings1[6]={
	                            "1 00:00:00-24:00:00", 
	                            "0 00:00:00-24:00:00", 
	                            "0 00:00:00-24:00:00", 
	                            "0 00:00:00-24:00:00", 
	                            "0 00:00:00-24:00:00", 
	                            "0 00:00:00-24:00:00"};	
		cJSON_AddItemToArray(TimeSection, cJSON_CreateStringArray(strings1,6));
		#endif
	}
	
     
    out=cJSON_Print(root);
    PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, CONFIG_GET_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0;
}


int resLocalAlarm(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root, *LocalAlarmObject,*EventHandler,*TimeSection;

    const char *strings1[6]={
                            "1 00:00:00-24:00:00", 
                            "0 00:00:00-24:00:00", 
                            "0 00:00:00-24:00:00", 
                            "0 00:00:00-24:00:00", 
                            "0 00:00:00-24:00:00", 
                            "0 00:00:00-24:00:00"};

    //获取设备的音视频通道压缩参数
    DMS_NET_ALARMINCFG stAlarmIn;
#if 0
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_ALARMINCFG, 0, &stAlarmIn, sizeof(DMS_NET_ALARMINCFG)))
    {  
		PRINT_ERR();
		return -1;	
    }
#endif

	if(runAlarmCfg.alarmIn.handle.is_snap || runAlarmCfg.alarmIn.handle.is_beep)
		stAlarmIn.byAlarmInHandle = 1;
	else
		stAlarmIn.byAlarmInHandle = 0;

	stAlarmIn.stHandle.wActionMask |= GK_ALARM_EXCEPTION_TOSNAP;
    stAlarmIn.stHandle.wActionMask |= GK_ALARM_EXCEPTION_TOBEEP; 
	stAlarmIn.stHandle.wDuration = runAlarmCfg.alarmIn.handle.intervalTime;
	if(runAlarmCfg.alarmIn.handle.is_snap == 1)
	{
		stAlarmIn.stHandle.wActionFlag |= GK_ALARM_EXCEPTION_TOSNAP;
		stAlarmIn.stHandle.bySnap[0] = 0x01;
        stAlarmIn.stHandle.wSnapNum = runAlarmCfg.alarmIn.handle.snapNum;

	}
	if(runAlarmCfg.alarmIn.handle.is_beep == 1)
	{
        stAlarmIn.stHandle.wActionFlag |= GK_ALARM_EXCEPTION_TOBEEP;
        stAlarmIn.stHandle.wBuzzerTime = runAlarmCfg.alarmIn.handle.beepTime;
	}

    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name", "Alarm.LocalAlarm.[0]");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);
    cJSON_AddItemToObject(root,"Alarm.LocalAlarm.[0]", LocalAlarmObject = cJSON_CreateObject());
    if(stAlarmIn.byAlarmInHandle)
        cJSON_AddTrueToObject(LocalAlarmObject, "Enable");
    else
        cJSON_AddFalseToObject(LocalAlarmObject, "Enable");
	
    cJSON_AddItemToObject(LocalAlarmObject,"EventHandler", EventHandler = cJSON_CreateObject());
    cJSON_AddStringToObject(EventHandler, "AlarmInfo", "");
    if(stAlarmIn.stHandle.wActionFlag & GK_ALARM_EXCEPTION_TOALARMOUT){
        cJSON_AddTrueToObject(EventHandler, "AlarmOutEnable");
        cJSON_AddStringToObject(EventHandler, "AlarmOutMask", "0x00000001");
    }
    else{
        cJSON_AddFalseToObject(EventHandler, "AlarmOutEnable");
        cJSON_AddStringToObject(EventHandler, "AlarmOutMask", "0x00000000");
    }
    cJSON_AddNumberToObject(EventHandler, "AlarmOutLatch", stAlarmIn.stHandle.wDuration);

	if(stAlarmIn.stHandle.wActionFlag & GK_ALARM_EXCEPTION_TOBEEP)
		cJSON_AddTrueToObject(EventHandler, "BeepEnable");
	else
    	cJSON_AddFalseToObject(EventHandler, "BeepEnable");
	
    cJSON_AddNumberToObject(EventHandler, "EventLatch", 1);
    if(stAlarmIn.stHandle.wActionFlag & GK_ALARM_EXCEPTION_TOFTP)
        cJSON_AddTrueToObject(EventHandler, "FTPEnable");
    else
        cJSON_AddFalseToObject(EventHandler, "FTPEnable");

    cJSON_AddFalseToObject(EventHandler, "LogEnable");
    if(stAlarmIn.stHandle.wActionFlag & GK_ALARM_EXCEPTION_TOEMAIL)
        cJSON_AddTrueToObject(EventHandler, "MailEnable");
    else
        cJSON_AddFalseToObject(EventHandler, "MailEnable");
    cJSON_AddFalseToObject(EventHandler, "MatrixEnable");
    cJSON_AddStringToObject(EventHandler, "MatrixMask", "0x00000000");
    cJSON_AddFalseToObject(EventHandler, "MessageEnable");
    cJSON_AddFalseToObject(EventHandler, "MsgtoNetEnable");
    cJSON_AddFalseToObject(EventHandler, "MultimediaMsgEnable");

	if(stAlarmIn.stHandle.wActionFlag & GK_ALARM_EXCEPTION_TOPTZ)
		cJSON_AddTrueToObject(EventHandler, "PtzEnable");
	else
    	cJSON_AddFalseToObject(EventHandler, "PtzEnable");
	

    if(stAlarmIn.stHandle.wActionFlag & GK_ALARM_EXCEPTION_TOREC){
        cJSON_AddTrueToObject(EventHandler, "RecordEnable");
        cJSON_AddStringToObject(EventHandler, "RecordMask", "0x00000001");
    }
    else{
        cJSON_AddFalseToObject(EventHandler, "RecordEnable");
        cJSON_AddStringToObject(EventHandler, "RecordMask", "0x00000000");
    }

    cJSON_AddNumberToObject(EventHandler, "RecordLatch", 1);


  
    cJSON_AddFalseToObject(EventHandler, "ShortMsgEnable");
    cJSON_AddFalseToObject(EventHandler, "ShowInfo");
    cJSON_AddStringToObject(EventHandler, "ShowInfoMask", "0x00000000");
    if(stAlarmIn.stHandle.wActionFlag & GK_ALARM_EXCEPTION_TOSNAP){
        cJSON_AddTrueToObject(EventHandler, "SnapEnable");
        cJSON_AddStringToObject(EventHandler, "SnapShotMask", "0x00000001");
    }
    else{
        cJSON_AddFalseToObject(EventHandler, "SnapEnable");
        cJSON_AddStringToObject(EventHandler, "SnapShotMask", "0x00000000");
    }
    cJSON_AddFalseToObject(EventHandler, "TipEnable");
    cJSON_AddFalseToObject(EventHandler, "TourEnable");
    cJSON_AddStringToObject(EventHandler, "TourMask", "0x00000000");
    if(stAlarmIn.stHandle.wActionFlag & GK_ALARM_EXCEPTION_TOVOICE)
        cJSON_AddTrueToObject(EventHandler, "VoiceEnable");
    else
        cJSON_AddFalseToObject(EventHandler, "VoiceEnable");
	

#if 1
    cJSON_AddItemToObject(LocalAlarmObject,"TimeSection", TimeSection = cJSON_CreateArray());
    cJSON_AddItemToArray(TimeSection,cJSON_CreateStringArray(strings1,6));
    cJSON_AddItemToArray(TimeSection,cJSON_CreateStringArray(strings1,6));
    cJSON_AddItemToArray(TimeSection,cJSON_CreateStringArray(strings1,6));
    cJSON_AddItemToArray(TimeSection,cJSON_CreateStringArray(strings1,6));
    cJSON_AddItemToArray(TimeSection,cJSON_CreateStringArray(strings1,6));
    cJSON_AddItemToArray(TimeSection,cJSON_CreateStringArray(strings1,6));
    cJSON_AddItemToArray(TimeSection,cJSON_CreateStringArray(strings1,6));
#endif
     
    out=cJSON_Print(root);
    //PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, CONFIG_GET_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0;
}

int resAlarmOut(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root;
	//cJSON *fmt;
	
    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name", "Alarm.AlarmOut");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);

	#if 0
    cJSON_AddItemToObject(root,"Alarm.AlarmOut", fmt = cJSON_CreateObject());
    cJSON_AddStringToObject(fmt, "NaInfoCode", "0:");
    cJSON_AddStringToObject(fmt, "NatStatus", "Probing");
    #endif
    
    out=cJSON_PrintUnformatted(root);
    //PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, CONFIG_GET_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0;
}

int resRecordConfigGet(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root, *TimeSection,*Record;

	//获取参数
    DMS_NET_DEVICE_INFO stDevicInfo;
	DMS_NET_CHANNEL_RECORD stChannelRecord;

    stDevicInfo.byRecordLen     = runRecordCfg.recordLen;
    stChannelRecord.dwPreRecordTime = runRecordCfg.preRecordTime;
    stChannelRecord.byRecordMode = runRecordCfg.recordMode;
#if 0
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_DEVICECFG, 0, &stDevicInfo, sizeof(DMS_NET_DEVICE_INFO)))
    {
        PRINT_ERR();
		return -1;		
    } 

	//获取参数
	DMS_NET_CHANNEL_RECORD stChannelRecord;
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_RECORDCFG, 0, &stChannelRecord, sizeof(DMS_NET_CHANNEL_RECORD)))
    {
        PRINT_ERR();
		return -1;		
    }
#endif

	#if 0
	int i, j, k;
	int m, n;
	char byHour[7][24];
	for(i = 0; i < 7; i++)
	{
		for(j = 0; j < 24; j++)
		{
			if(1 == byHour[i][j])
			{
				stChannelRecord.stRecordSched.byRecordType[i][j*2] = 1;
				stChannelRecord.stRecordSched.byRecordType[i][j*2 + 1] = 1;
				stChannelRecord.stRecordSched.byHour[i][j*2] = 1;
				stChannelRecord.stRecordSched.byHour[i][j*2 + 1] = 1;
			}else
			{
				stChannelRecord.stRecordSched.byHour[i][j*2] = 0;
				stChannelRecord.stRecordSched.byHour[i][j*2 + 1] = 0;
			}
		}
	}
	#else
    const char *strings1[6]={"1 00:00:00-24:00:00", 
		                     "0 00:00:00-24:00:00",
		                     "0 00:00:00-24:00:00",
		                     "0 00:00:00-24:00:00", 
		                     "0 00:00:00-24:00:00",
		                     "0 00:00:00-24:00:00"}; 	
	#endif
    
    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name", "Record.[0]");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);
    cJSON_AddItemToObject(root,"Record.[0]", Record = cJSON_CreateObject());
    cJSON_AddNumberToObject(Record, "PacketLength", stDevicInfo.byRecordLen);

    cJSON_AddNumberToObject(Record, "PreRecord", stChannelRecord.dwPreRecordTime);

    cJSON_AddFalseToObject(Record, "Redundancy");
    cJSON_AddItemToObject(Record,"TimeSection", TimeSection = cJSON_CreateArray());
    cJSON_AddItemToArray(TimeSection,cJSON_CreateStringArray(strings1,6));
    cJSON_AddItemToArray(TimeSection,cJSON_CreateStringArray(strings1,6));
    cJSON_AddItemToArray(TimeSection,cJSON_CreateStringArray(strings1,6));
    cJSON_AddItemToArray(TimeSection,cJSON_CreateStringArray(strings1,6));
    cJSON_AddItemToArray(TimeSection,cJSON_CreateStringArray(strings1,6));
    cJSON_AddItemToArray(TimeSection,cJSON_CreateStringArray(strings1,6));
    cJSON_AddItemToArray(TimeSection,cJSON_CreateStringArray(strings1,6));

	switch(stChannelRecord.byRecordMode)
	{
		case 0:	// 定时录像
		{
			cJSON_AddStringToObject(Record, "RecordMode", "ConfigRecord");
			break;
		}
		case 1:	//手动录像
		{
			cJSON_AddStringToObject(Record, "RecordMode", "ManualRecord");
			break;
		}
		case 2:		/*2:禁止所有方式触发录像*/
		{
			cJSON_AddStringToObject(Record, "RecordMode", "ClosedRecord");
			break;
		}
		default:
		{
			/*暂时默认为定时录像*/
			cJSON_AddStringToObject(Record, "RecordMode", "ConfigRecord");
			break;
		}
	}
	

    //out=cJSON_PrintUnformatted(root);
    out=cJSON_Print(root);
    //PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, CONFIG_GET_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0;
}

int resNetEmailGet(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root, *NetEmail, *MailServer,*Recievers,*Schedule;

	//获取参数
	DMS_NET_EMAIL_PARAM  stEmaiParam;
	stEmaiParam.bEnableEmail 		 = runNetworkCfg.email.enableEmail;

    strncpy(stEmaiParam.csEMailUser, runNetworkCfg.email.eMailUser, DMS_NAME_LEN-1);
    strncpy(stEmaiParam.csEmailPass, runNetworkCfg.email.eMailPass, DMS_PASSWD_LEN-1);
	stEmaiParam.wServicePort = runNetworkCfg.email.smtpPort;
	//PRINT_INFO("runNetworkCfg.email.smtpServer: %s\n", runNetworkCfg.email.smtpServer);
    strncpy(stEmaiParam.csSmtpServer, runNetworkCfg.email.smtpServer, DMS_MAX_DOMAIN_NAME-1);
	strncpy(stEmaiParam.stSendAddrList.csAddress, runNetworkCfg.email.fromAddr, DMS_MAX_DOMAIN_NAME-1);
	strncpy(stEmaiParam.stToAddrList[0].csAddress, runNetworkCfg.email.toAddrList0, DMS_MAX_DOMAIN_NAME-1);

#if 0
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_EMAILCFG, 0, &stEmaiParam, sizeof(stEmaiParam)))
    {
        PRINT_ERR();
		return -1;		
    }
#endif
	    
    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name", "NetWork.NetEmail");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);
    cJSON_AddItemToObject(root,"NetWork.NetEmail", NetEmail = cJSON_CreateObject());

	if(stEmaiParam.bEnableEmail)
    	cJSON_AddTrueToObject(NetEmail, "Enable");
	else
		cJSON_AddFalseToObject(NetEmail, "Enable");
	
    cJSON_AddFalseToObject(NetEmail, "UseSSL");
    cJSON_AddStringToObject(NetEmail, "Title", "IPCAlert");
    cJSON_AddStringToObject(NetEmail, "SendAddr", stEmaiParam.stSendAddrList.csAddress);
    cJSON_AddItemToObject(NetEmail,"MailServer", MailServer = cJSON_CreateObject());
    cJSON_AddStringToObject(MailServer, "Address", "0x00000000");
    cJSON_AddStringToObject(MailServer, "Name", stEmaiParam.csSmtpServer);
    cJSON_AddStringToObject(MailServer, "Password", stEmaiParam.csEmailPass);
    cJSON_AddStringToObject(MailServer, "UserName", stEmaiParam.csEMailUser);
    cJSON_AddNumberToObject(MailServer, "Port", stEmaiParam.wServicePort);
    cJSON_AddFalseToObject(MailServer, "Anonymity");
    cJSON_AddItemToObject(NetEmail,"Recievers", Recievers = cJSON_CreateArray());
    cJSON_AddItemToArray(Recievers, cJSON_CreateString(stEmaiParam.stToAddrList[0].csAddress));
    cJSON_AddItemToArray(Recievers, cJSON_CreateString(stEmaiParam.stToAddrList[1].csAddress));
    cJSON_AddItemToArray(Recievers, cJSON_CreateString(stEmaiParam.stToAddrList[2].csAddress));
    cJSON_AddItemToArray(Recievers, cJSON_CreateString("none"));
    cJSON_AddItemToArray(Recievers, cJSON_CreateString("none"));
    cJSON_AddItemToObject(NetEmail,"Schedule", Schedule = cJSON_CreateArray());
    cJSON_AddItemToArray(Schedule, cJSON_CreateString("0 00:00:00-24:00:00"));
    cJSON_AddItemToArray(Schedule, cJSON_CreateString("0 00:00:00-24:00:00"));

    
    //out=cJSON_PrintUnformatted(root);
    out=cJSON_Print(root);
    //PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, CONFIG_GET_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0;
}

int resNetFTPGet(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root,*NetFTP, *Server;

	//获取参数
	DMS_NET_FTP_PARAM stFtpParam;

	stFtpParam.bEnableFTP = runNetworkCfg.ftp.enableFTP;
	stFtpParam.dwFTPPort = runNetworkCfg.ftp.port;
    strncpy(stFtpParam.csFTPIpAddress, runNetworkCfg.ftp.address,DMS_MAX_DOMAIN_NAME-1);
    strncpy(stFtpParam.csUserName, runNetworkCfg.ftp.userName, DMS_NAME_LEN-1);
    strncpy(stFtpParam.csPassword, runNetworkCfg.ftp.password, DMS_PASSWD_LEN-1);
    
#if 0
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_FTPCFG, 0, &stFtpParam, sizeof(DMS_NET_FTP_PARAM)))
    {
        PRINT_ERR();
		return -1;		
    } 
#endif

    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name", "NNetWork.NetFTP");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);
    cJSON_AddItemToObject(root,"NetWork.NetFTP", NetFTP = cJSON_CreateObject());

	if(stFtpParam.bEnableFTP)
    	cJSON_AddTrueToObject(NetFTP, "Enable");
	else
		cJSON_AddFalseToObject(NetFTP, "Enable");
	
    cJSON_AddStringToObject(NetFTP, "Directory", stFtpParam.csFTPIpAddress);
    cJSON_AddItemToObject(NetFTP, "Server", Server = cJSON_CreateObject());
    cJSON_AddStringToObject(Server, "Address", "0x00000000");
    cJSON_AddStringToObject(Server, "Name", stFtpParam.csFTPIpAddress);
    cJSON_AddStringToObject(Server, "Password", stFtpParam.csPassword);
    cJSON_AddStringToObject(Server, "UserName", stFtpParam.csUserName);
    cJSON_AddNumberToObject(Server, "Port", stFtpParam.dwFTPPort);

    //out=cJSON_PrintUnformatted(root);
    out=cJSON_Print(root);
    //PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, CONFIG_GET_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0;
}

int resGUISetGet(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root,*GUISet;

    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name", "fVideo.GUISet");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);
    
    cJSON_AddItemToObject(root,"fVideo.GUISet", GUISet = cJSON_CreateObject());
    cJSON_AddFalseToObject(GUISet, "AlarmStateEnable");
    cJSON_AddFalseToObject(GUISet, "CarInfo");
    cJSON_AddFalseToObject(GUISet, "ChanStateBitRateEnable");
    cJSON_AddFalseToObject(GUISet, "ChanStateLckEnable");
    cJSON_AddFalseToObject(GUISet, "ChanStateMtdEnable");
    cJSON_AddFalseToObject(GUISet, "ChanStateVlsEnable");
    
    cJSON_AddFalseToObject(GUISet, "Deflick");
    cJSON_AddFalseToObject(GUISet, "GPSInfo");
    cJSON_AddFalseToObject(GUISet, "RecordStateEnable");
    cJSON_AddFalseToObject(GUISet, "RemoteEnable");

	//读取参数
	DMS_NET_CHANNEL_OSDINFO stChannelOsdinfo;
	GK_NET_CHANNEL_INFO channelInfo;

	netcam_osd_get_info(0,&channelInfo);
	stChannelOsdinfo.bShowTime = channelInfo.osdDatetime.enable;
	stChannelOsdinfo.byShowChanName = channelInfo.osdChannelID.enable;
#if 0
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_OSDCFG, 0, &stChannelOsdinfo, sizeof(stChannelOsdinfo)))
    {
        PRINT_ERR();
		return -1;		
    }	
#endif

	if(stChannelOsdinfo.bShowTime == 1)
    	cJSON_AddTrueToObject(GUISet, "TimeTitleEnable");
	else
		cJSON_AddFalseToObject(GUISet, "TimeTitleEnable");
	if(stChannelOsdinfo.byShowChanName == 1)
		cJSON_AddTrueToObject(GUISet, "ChannelTitleEnable");
	else
		cJSON_AddFalseToObject(GUISet, "ChannelTitleEnable");

    cJSON_AddNumberToObject(root, "WindowAlpha", 175);

    //out=cJSON_PrintUnformatted(root);
    out=cJSON_Print(root);
    //PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, CONFIG_GET_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0;
}



int resConfigGetReq(XMaiSessionCtrl *pXMaiSessionCtrl, char *Name)
{
	if(!strcmp(Name, "General.General"))
	{
		return resGeneralInfo(pXMaiSessionCtrl);
	}
	else if(!strcmp(Name, "General.Location"))
	{
		return resLocationInfo(pXMaiSessionCtrl);
	}
	else if(!strcmp(Name, "Camera.Param.[0]"))
	{
		return resCameraParam(pXMaiSessionCtrl);
	}
	else if(!strcmp(Name, "AVEnc.VideoColor.[0]"))
	{
		return resVideoColor(pXMaiSessionCtrl);
	}
	else if(!strcmp(Name, "NetWork.DigManagerShow"))
	{
		return resDigManagerShow(pXMaiSessionCtrl);
	}
	else if(!strcmp(Name, "Storage.StoragePosition"))
	{
		return resStoragePosition(pXMaiSessionCtrl);
	}
	else if(!strcmp(Name, "AVEnc.EncodeStaticParam"))
	{
		return resEncodeStaticParam(pXMaiSessionCtrl);
	}
	else if(!strcmp(Name, "Simplify.Encode"))
	{
		return resSimplifyEncode(pXMaiSessionCtrl);
	}
	else if(!strcmp(Name, "NetWork.NetCommon"))
	{
		return resNetCommon(pXMaiSessionCtrl);
	}
	else if(!strcmp(Name, "NetWork.NetDHCP"))
	{
		return resNetDHCP(pXMaiSessionCtrl);
	}
	else if(!strcmp(Name, "NetWork.NetDNS")){
		return resNetDNS(pXMaiSessionCtrl);
	}
	else if(!strcmp(Name, "AVEnc.VideoWidget"))
	{
		return resVideoWidget(pXMaiSessionCtrl);
	}
	else if(!strcmp(Name, "General.AutoMaintain"))
	{
		return resAutoMaintain(pXMaiSessionCtrl);
	}
	else if(!strcmp(Name, "Status.NatInfo"))
	{
		return resNatInfo(pXMaiSessionCtrl);
	}
	else if(!strcmp(Name, "Ability.SerialNo"))
	{
		return resSerialNo(pXMaiSessionCtrl);
	}
	else if(!strcmp(Name, "Detect.MotionDetect.[0]"))
	{
		return resMotionDetect(pXMaiSessionCtrl);
	}
	else if(!strcmp(Name, "Alarm.LocalAlarm.[0]"))
	{
		return resLocalAlarm(pXMaiSessionCtrl);
	}
	else if(!strcmp(Name, "Alarm.AlarmOut"))
	{
		return resAlarmOut(pXMaiSessionCtrl);
	}	
	else if(!strcmp(Name, "Record.[0]"))
	{
		return resRecordConfigGet(pXMaiSessionCtrl);
	}
	else if(!strcmp(Name, "NetWork.NetEmail"))
	{
		return resNetEmailGet(pXMaiSessionCtrl);
	}
	else if(!strcmp(Name, "NetWork.NetFTP"))
	{
		return resNetFTPGet(pXMaiSessionCtrl);
	}
	else if(!strcmp(Name, "fVideo.GUISet"))
	{
		return resGUISetGet(pXMaiSessionCtrl);
	}
	else
	{
		return -1;
	}

	return 0;
}


int ReqConfigGetHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg)
{
    PRINT_INFO("<<ReqConfigGetHandle>>  sock:%d sessionType:%d\n", pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSessionType);
    XMaiMsgHeader *pXMaiMsgHeader = (XMaiMsgHeader *)msg;
    cJSON *json = NULL;
    cJSON *element = NULL;
    char *out = NULL;
    char Name[128]={0};
    
    if(pXMaiMsgHeader->dataLen == 0){
        PRINT_ERR("dataLen == 0 no json   must have jason\n");
        return -1;
    }
	
	json=cJSON_Parse(msg+20);
    if (!json){
        PRINT_ERR("Error before: [%s]\n", cJSON_GetErrorPtr());
        return -1;
    }    
    //out = cJSON_Print(json);

    element = cJSON_GetObjectItem(json, "Name");
    if(element)
        sprintf(Name, "%s", element->valuestring);

	PRINT_INFO("============ get %s config ============\n", Name);

    //处理
    //HandleConfigGetReq(json);

    cJSON_Delete(json);
    free(out);

    //回应
    return resConfigGetReq(pXMaiSessionCtrl, Name);
}

//////////////////////////////////////////////////////////////
int HandleVideoColorSet(cJSON *json)
{
    cJSON *object, *array0,*array1, *elem,*elemColorParam;
    int ret;
    int bright = 0;
	int contrast = 0;
	int hue = 0;
	int gain = 0;
	int saturation = 0;
	GK_NET_IMAGE_CFG imagAttr;
#if 0	
	//获取设备的颜色参数 
	DMS_NET_CHANNEL_COLOR stChannelColor;

    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_COLORCFG, 0, &stChannelColor, sizeof(stChannelColor)))
    {
        PRINT_ERR();
		return -1;		
    }	
    //获取设备的颜色参数最大值
    DMS_NET_COLOR_SUPPORT  stColorSupport;
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_COLOR_SUPPORT, 0, &stColorSupport, sizeof(DMS_NET_COLOR_SUPPORT)))
    {
        PRINT_ERR();
		return -1;	
    } 
#endif

    object = cJSON_GetObjectItem(json, "AVEnc.VideoColor.[0]");
    if(!object){
        PRINT_ERR("get AVEnc.VideoColor.[0] error\n");
        return -1;
    }

    array0 = cJSON_GetArrayItem(object, 0);
    if(!array0){
        PRINT_ERR("get AVEnc.VideoColor.[0] Array1 error\n");
        return -1;
    }

    elem = cJSON_GetObjectItem(array0, "TimeSection");
    if(!elem){
        return -1;
    }
    PRINT_INFO("TimeSection:%s\n", elem->valuestring);
    if(strstr(elem->valuestring, "24")){
        PRINT_INFO("=================1\n");
        elemColorParam = cJSON_GetObjectItem(array0, "VideoColorParam");
        if(!elemColorParam)
            return -1;

        elem = cJSON_GetObjectItem(elemColorParam, "Brightness");
        if(elem)
            bright = elem->valueint;

        elem = cJSON_GetObjectItem(elemColorParam, "Contrast");
        if(elem)    
            contrast= elem->valueint;
            
        elem = cJSON_GetObjectItem(elemColorParam, "Gain");
        if(elem)
            gain= elem->valueint;

        elem = cJSON_GetObjectItem(elemColorParam, "Hue");
        if(elem)
            hue= elem->valueint;

        elem = cJSON_GetObjectItem(elemColorParam, "Saturation");
        if(elem)
            saturation= elem->valueint;
    }
    else{
        PRINT_INFO("=================2\n");
        array1 = cJSON_GetArrayItem(object, 1);
        if(!array1){
            PRINT_ERR("get AVEnc.VideoColor.[0] Array1 error\n");
            return -1;
        }

        elemColorParam = cJSON_GetObjectItem(array1, "VideoColorParam");
        if(!elemColorParam)
            return -1;

        elem = cJSON_GetObjectItem(elemColorParam, "Brightness");
        if(elem)
            bright = elem->valueint;

        elem = cJSON_GetObjectItem(elemColorParam, "Contrast");
        if(elem)    
            contrast= elem->valueint;
            
        elem = cJSON_GetObjectItem(elemColorParam, "Gain");
        if(elem)
            gain= elem->valueint;

        elem = cJSON_GetObjectItem(elemColorParam, "Hue");
        if(elem)
            hue= elem->valueint;

        elem = cJSON_GetObjectItem(elemColorParam, "Saturation");
        if(elem)
            saturation= elem->valueint;
    }

	memset(&imagAttr, 0, sizeof(GK_NET_IMAGE_CFG));
	ret = netcam_image_get(&imagAttr);
    if(ret != 0)
	{
		PRINT_ERR("XMAI: fail to get image attr.\n");
		return -1;
	}

	imagAttr.brightness = bright;
	imagAttr.contrast = contrast;
	imagAttr.saturation = saturation;
	imagAttr.hue = hue;

#if 0	
    stChannelColor.nBrightness = (int) (bright * stColorSupport.strBrightness.nMax / 100);
    stChannelColor.nContrast = (int) (contrast * stColorSupport.strContrast.nMax / 100);
	stChannelColor.nSaturation = (int) (saturation * stColorSupport.strSaturation.nMax / 100);
    stChannelColor.nHue = (int) (hue * stColorSupport.strHue.nMax / 100);          
	//stChannelColor.nDefinition = (int) (gain * stColorSupport.strDefinition.nMax / 100);
#endif

	PRINT_INFO("set color:\n");
    PRINT_INFO("bright:%d to %d\n", bright, imagAttr.brightness);
	PRINT_INFO("contrast:%d to %d\n", contrast, imagAttr.contrast);
	PRINT_INFO("saturation:%d to %d\n", saturation, imagAttr.saturation);
	PRINT_INFO("hue:%d to %d\n", hue, imagAttr.hue);    
    //PRINT_INFO("gain:%d to %d\n", gain, stChannelColor.nDefinition);
	
#if 0
	DMS_NET_DAY_NIGHT_DETECTION_EX stDayNight;
	s32Ret = dms_sysnetapi_ioctrl(s32DMSHandle, DMS_NET_GET_COLOR_BLACK_DETECTION, 0, &stDayNight, sizeof(DMS_NET_CHANNEL_COLOR));
	
	
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_SET_COLORCFG, 0, &stChannelColor, sizeof(stChannelColor)))
    {
        PRINT_ERR();
		return -1;		
    }

	/* 保存修改到配置文件中 */ 
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_SET_SAVECFG, 0, 0, 0))
    {
        PRINT_ERR();
        return -1;        
    }	
#endif

	netcam_image_set(imagAttr);
    return 0;
}

static int xmai_get_ip_addr(char *name,char *net_ip)
{
    struct ifreq ifr;
    int fd;
    struct sockaddr_in stSockAddrIn;

    if((name == NULL) || (net_ip== NULL))
    {
        PRINT_ERR("get_ip_addr: invalid argument!\n");
        return -1;
    }

    strcpy(ifr.ifr_name, name);
    ifr.ifr_addr.sa_family = AF_INET;

    fd = socket(PF_INET, SOCK_DGRAM, 0);
    if(fd < 0)
    {
        return -1;
    }

    if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) 
    {
        close(fd);
        return -1;
    }

    memset(&stSockAddrIn, 0, sizeof(stSockAddrIn));
    memcpy(&stSockAddrIn, &ifr.ifr_addr, sizeof(stSockAddrIn));
    strcpy(net_ip,inet_ntoa(stSockAddrIn.sin_addr));	

    close(fd);

    return 0;
}

static int xmai_get_mask_addr(char *name,char *net_mask)
{
	struct ifreq ifr;
	int ret = 0;
	int fd;
    struct sockaddr_in stSockAddrIn;
	
	strcpy(ifr.ifr_name, name);
	ifr.ifr_addr.sa_family = AF_INET;
	
	fd = socket(PF_INET, SOCK_DGRAM, 0);
    //printf("TTTTTTTTTTTTTTTTTTTTTTTTTT %s, %d\n", __FILE__, __LINE__);
	if(fd < 0)
		ret = -1;
		
	if (ioctl(fd, SIOCGIFNETMASK, &ifr) < 0) 
	{
		ret = -1;
	}

    memset(&stSockAddrIn, 0, sizeof(stSockAddrIn));
    memcpy(&stSockAddrIn, &ifr.ifr_addr, sizeof(stSockAddrIn));
	strcpy(net_mask,inet_ntoa(stSockAddrIn.sin_addr));	
	//strcpy(net_mask,inet_ntoa(((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr));

	close(fd);

	return	ret;
}

static int xmai_get_gateway_addr(char *gateway_addr)
{
	char buff[256];
	int  nl = 0 ;
	struct in_addr gw;
	int flgs, ref, use, metric;
	unsigned long int d,g,m;
	unsigned long addr;
	unsigned long *pgw = &addr;

	FILE	 *fp = NULL;
	
	if (pgw == NULL)
		return -1;
		
	*pgw = 0;

	fp = fopen("/proc/net/route", "r");
//	printf("TTTTTTTTTTTTTTTTTTTTTTTTTT %s, %d\n", __FILE__, __LINE__);
	if (fp == NULL)
	{
		return -1;
	}
		
	nl = 0 ;
	while( fgets(buff, sizeof(buff), fp) != NULL ) 
	{
		if(nl) 
		{
			int ifl = 0;
			while(buff[ifl]!=' ' && buff[ifl]!='\t' && buff[ifl]!='\0')
				ifl++;
			buff[ifl]=0;    /* interface */
			if(sscanf(buff+ifl+1, "%lx%lx%X%d%d%d%lx",
				   &d, &g, &flgs, &ref, &use, &metric, &m)!=7) 
			{
				//continue;
				fclose(fp);
				return -2;
			}


			ifl = 0;        /* parse flags */
			if(flgs&RTF_UP) 
			{			
				gw.s_addr   = g;
					
				if(d==0)
				{
					*pgw = g;		
					strcpy(gateway_addr,inet_ntoa(gw));
					fclose(fp);
					return 0;
				}				
			}
		}
		nl++;
	}	
	
	if(fp)
	{
		fclose(fp);
		fp = NULL;
	}
	
	return	0;
}


int HandleNetDHCPSet(cJSON *json)
{
    cJSON *element, *fmt;
	ST_SDK_NETWORK_ATTR net_attr;
	
#if 0
	//获取本机网络参数，
	DMS_NET_NETWORK_CFG	stNetWorkConfig;
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_NETCFG, 0, &stNetWorkConfig, sizeof(stNetWorkConfig)))
    {
       PRINT_ERR("DMS_NET_GET_NETCFG fail\n");
       return -1;
    }
#endif
	
    element = cJSON_GetObjectItem(json, "NetWork.NetDHCP");
    if(!element){
        PRINT_ERR("get NetWork.NetDHCP error\n");
        return -1;
    }

    element= cJSON_GetArrayItem(element, 0);
    if(!element){
        PRINT_ERR("get NetWork.NetDHCP Array0 error\n");
        return -1;
    }

    fmt = cJSON_GetObjectItem(element,"Enable");
    if(!fmt){
        PRINT_ERR("get NetWork.NetDHCP Enable error\n");
        return -1;
    }

	if (netcam_net_get_detect("eth0") != 0)
		strncpy(net_attr.name,netcam_net_wifi_get_devname(),sizeof(net_attr.name));
	else
		strncpy(net_attr.name,"eth0",sizeof(net_attr.name));
	
	if (netcam_net_get(&net_attr))
	{
		PRINT_ERR("Fail to get net_attr\n");
		return -1;
	}

	if(net_attr.dhcp != fmt->type)
	{
	    if(fmt->type == 0) //关闭dhcp
		{		
			PRINT_INFO("Set Config: disable dhcp.\n");

			net_attr.dhcp = 0;
#if 0
			system("killall udhcpc");
			
		    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_SET_NETCFG, 0, &stNetWorkConfig, sizeof(stNetWorkConfig)))
		    {
		       PRINT_ERR("DMS_NET_GET_NETCFG fail\n");
		       return -1;
		    }			
#endif
	    }
	    else if(fmt->type == 1) //开启dhcp
		{
			PRINT_INFO("Set Config: enable dhcp.\n");
			net_attr.dhcp = 1;
#if 0
			system("udhcpc -ieth0");

			char  NetAddr[24] = {0};
	        xmai_get_ip_addr(ETH_DEV, NetAddr);	        
	        strcpy(stNetWorkConfig.stEtherNet[0].strIPAddr.csIpV4, NetAddr);			
	        xmai_get_mask_addr(ETH_DEV, NetAddr);	        
	        strcpy(stNetWorkConfig.stEtherNet[0].strIPMask.csIpV4, NetAddr);			
	        xmai_get_gateway_addr(NetAddr);	        
	        strcpy(stNetWorkConfig.stGatewayIpAddr.csIpV4, NetAddr);	

			PRINT_INFO("Dhcp Get Network IP :%s\n", stNetWorkConfig.stEtherNet[0].strIPAddr.csIpV4);
			PRINT_INFO("Dhcp Get NetMask IP :%s\n", stNetWorkConfig.stEtherNet[0].strIPMask.csIpV4);
			PRINT_INFO("Dhcp Get Gateway IP :%s\n", stNetWorkConfig.stGatewayIpAddr.csIpV4);

		    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_SET_NETCFG, 0, &stNetWorkConfig, sizeof(stNetWorkConfig)))
		    {
		       PRINT_ERR("DMS_NET_GET_NETCFG fail\n");
		       return -1;
		    }	

			#if 1
			//重启前关闭
			g_xmai_pthread_run = 0;
			/* 保存修改到配置文件中 */
		    int ret = dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_SET_SAVECFG, 0, 0, 0); 
		    if(0 != ret)
		    {
		        PRINT_ERR();
		        return -1;        
		    }	
			#endif
#endif
	    }

	}

	netcam_net_set(&net_attr);
	return 0;
}


int HandleNetFTPCfgSet(cJSON *json)
{
    cJSON *fmt, *NetFTP, *Server;

#if 0
	DMS_NET_FTP_PARAM stFtpParam;
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_FTPCFG, 0, &stFtpParam, sizeof(DMS_NET_FTP_PARAM)))
    {
        PRINT_ERR();
		return -1;		
    } 
#endif
	
    NetFTP = cJSON_GetObjectItem(json, "NetWork.NetFTP");
    if(!NetFTP){
        PRINT_ERR("get NetWork.NetFTP error\n");
        return -1;
    }
    
    fmt = cJSON_GetObjectItem(NetFTP, "Directory");
    if(!fmt ){
        PRINT_ERR("get Directory error\n");
        return -1;
    }
    sprintf(runNetworkCfg.ftp.datapath, "%s", fmt->valuestring);

    Server = cJSON_GetObjectItem(NetFTP, "Server");
    if(!Server){
        PRINT_ERR("get Server error\n");
        return -1;
    }

    fmt = cJSON_GetObjectItem(Server, "Name");
    if(!fmt ){
        PRINT_ERR("get Password error\n");
        return -1;
    }
    sprintf(runNetworkCfg.ftp.address, "%s", fmt->valuestring);

    fmt = cJSON_GetObjectItem(Server, "UserName");
    if(!fmt ){
        PRINT_ERR("get UserName error\n");
        return -1;
    }
    sprintf(runNetworkCfg.ftp.userName, "%s", fmt->valuestring);

    fmt = cJSON_GetObjectItem(Server, "Password");
    if(!fmt ){
        PRINT_ERR("get UserName error\n");
        return -1;
    }
    sprintf(runNetworkCfg.ftp.password, "%s", fmt->valuestring);        

#if 0
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_SET_FTPCFG, 0, &stFtpParam, sizeof(DMS_NET_FTP_PARAM)))
    {
        PRINT_ERR();
		return -1;		
    } 

	/* 保存修改到配置文件中 */ 
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_SET_SAVECFG, 0, 0, 0))
    {
        PRINT_ERR();
        return -1;        
    }
#endif

	NetworkCfgSave();
    return 0;
}

int HandleNetEmailCfgSet(cJSON *json)
{
    cJSON *fmt, *NetEmail,*MailServer, *Recievers;
    char serverName[26] = {0};
    char Password[26] = {0};
    char UserName[26] = {0};
    char toAddr[26] = {0};

#if 0
	DMS_NET_EMAIL_PARAM  stEmaiParam;
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_EMAILCFG, 0, &stEmaiParam, sizeof(stEmaiParam)))
    {
        PRINT_ERR();
		return -1;		
    } 
#endif

    NetEmail = cJSON_GetObjectItem(json, "NetWork.NetEmail");
    if(!NetEmail){
        PRINT_ERR("get NetWork.NetEmail error\n");
        return -1;
    }

    MailServer = cJSON_GetObjectItem(NetEmail, "MailServer");
    if(!MailServer ){
        PRINT_ERR("get MailServer error\n");
        return -1;
    }
    
    fmt = cJSON_GetObjectItem(MailServer, "Name");
    if(!fmt ){
        PRINT_ERR("get serverName error\n");
        return -1;
    }
    sprintf(serverName, "%s", fmt->valuestring);

    fmt = cJSON_GetObjectItem(MailServer, "Password");
    if(!fmt ){
        PRINT_ERR("get Password error\n");
        return -1;
    }
    sprintf(Password, "%s", fmt->valuestring);

    fmt = cJSON_GetObjectItem(MailServer, "UserName");
    if(!fmt ){
        PRINT_ERR("get UserName error\n");
        return -1;
    }
    sprintf(UserName, "%s", fmt->valuestring);

    Recievers = cJSON_GetObjectItem(NetEmail, "Recievers");
    if(!MailServer ){
        PRINT_ERR("get Recievers error\n");
        return -1;
    }    

    fmt = cJSON_GetArrayItem(Recievers, 0);
    if(!fmt ){
        PRINT_ERR("get Recievers error\n");
        return -1;
    }
    sprintf(toAddr, "%s", fmt->valuestring);
        
	strncpy(runNetworkCfg.email.smtpServer, serverName, sizeof(runNetworkCfg.email.smtpServer));
	strncpy(runNetworkCfg.email.eMailPass, Password, sizeof(runNetworkCfg.email.eMailPass));
	strncpy(runNetworkCfg.email.eMailUser, UserName, sizeof(runNetworkCfg.email.eMailUser));
	strncpy(runNetworkCfg.email.toAddrList0, toAddr, sizeof(runNetworkCfg.email.toAddrList0));

#if 0
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_SET_EMAILCFG, 0, &stEmaiParam, sizeof(stEmaiParam)))
    {
        PRINT_ERR();
		return -1;		
    } 	

	/* 保存修改到配置文件中 */ 
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_SET_SAVECFG, 0, 0, 0))
    {
        PRINT_ERR();
        return -1;        
    }
#endif

	NetworkCfgSave();
    return 0;
}

int HandleEncodeCfgSet(cJSON *json)
{
    cJSON *fmt, *Encode,*MainFormat,*ExtraFormat,*mVideo,*exVideo;
    //int PacketLength = 0;
    //int PreRecord = 0;
    //int Redundancy = 0;
    //char RecordMode[26] = {0}; 
    //int mEnctype = 0, exEnctype = 0;
    int ret;
    int mbrmode = 0, exbrmode = 0;
    int mAudioEnable = 0, exAudioEnable = 0;
    int mVideoEnable = 0, exVideoEnable = 0;
    int mBitRate = 0, exBitRate = 0;
    int mFPS = 0, exFPS = 0;
    int mGOP = 0, exGOP = 0;
    int mQuality = 0, exQuality = 0;
    char mBitRateControl[26] = {0}, exBitRateControl[26] = {0};
    char mCompression[26] = {0}, exCompression[26] = {0};
    char mResolution[26] = {0}, exResolution[26] = {0};
	ST_GK_ENC_STREAM_H264_ATTR stH264Config;

#if 0	
    //获取设备的音视频通道压缩参数
    DMS_NET_COMPRESSION_INFO *pCompressionInfo = NULL;
    DMS_NET_CHANNEL_PIC_INFO stChannelPicInfo;
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_PICCFG, 0, &stChannelPicInfo, sizeof(DMS_NET_CHANNEL_PIC_INFO)))
    {
        PRINT_ERR();
		return -1;	
    }
#endif

    Encode = cJSON_GetObjectItem(json, "Simplify.Encode.[0]");
    if(!Encode){
        PRINT_ERR("get Simplify.Encode.[0] error\n");
        return -1;
    }

    MainFormat = cJSON_GetObjectItem(Encode, "MainFormat");
    if(!MainFormat ){
        PRINT_ERR("get Simplify.Encode.[0] ExtraFormat error\n");
        return -1;
    }

    fmt = cJSON_GetObjectItem(MainFormat, "AudioEnable");
    if(!fmt ){
        PRINT_ERR("get Simplify.Encode.[0] AlarmOutLatch error\n");
        return -1;
    }
    mAudioEnable =  fmt->type;

    fmt = cJSON_GetObjectItem(MainFormat, "VideoEnable");
    if(!fmt ){
        PRINT_ERR("get Simplify.Encode.[0] FTPEnable error\n");
        return -1;
    }
    mVideoEnable =  fmt->type;

    mVideo = cJSON_GetObjectItem(MainFormat, "Video");
    if(!mVideo ){
        PRINT_ERR("get Simplify.Encode.[0] ExtraFormat error\n");
        return -1;
    }

    fmt = cJSON_GetObjectItem(mVideo, "BitRate");
    if(!fmt ){
        PRINT_ERR("get Simplify.Encode.[0] FTPEnable error\n");
        return -1;
    }
    mBitRate =  fmt->valueint;

    fmt = cJSON_GetObjectItem(mVideo, "FPS");
    if(!fmt ){
        PRINT_ERR("get Simplify.Encode.[0] FTPEnable error\n");
        return -1;
    }
    mFPS =  fmt->valueint;

    fmt = cJSON_GetObjectItem(mVideo, "GOP");
    if(!fmt ){
        PRINT_ERR("get Simplify.Encode.[0] FTPEnable error\n");
        return -1;
    }
    mGOP =  fmt->valueint;

    fmt = cJSON_GetObjectItem(mVideo, "Quality");
    if(!fmt ){
        PRINT_ERR("get Simplify.Encode.[0] FTPEnable error\n");
        return -1;
    }
    mQuality =  fmt->valueint;

    fmt = cJSON_GetObjectItem(mVideo, "BitRateControl");
    if(!fmt ){
        PRINT_ERR("get Simplify.Encode.[0] FTPEnable error\n");
        return -1;
    }
    sprintf(mBitRateControl, "%s", fmt->valuestring);
    if(!strcmp(mBitRateControl, "VBR"))
        mbrmode = 1;
    else if(!strcmp(mBitRateControl, "CBR"))
        mbrmode = 0;

    fmt = cJSON_GetObjectItem(mVideo, "Compression");
    if(!fmt ){
        PRINT_ERR("get Simplify.Encode.[0] FTPEnable error\n");
        return -1;
    }
    sprintf(mCompression, "%s", fmt->valuestring);

    fmt = cJSON_GetObjectItem(mVideo, "Resolution");
    if(!fmt ){
        PRINT_ERR("get Simplify.Encode.[0] FTPEnable error\n");
        return -1;
    }
    sprintf(mResolution, "%s", fmt->valuestring);

  	memset(&stH264Config, 0, sizeof(ST_GK_ENC_STREAM_H264_ATTR));
    ret = netcam_video_get(0, 0, &stH264Config);
    if(ret != 0)
    {
        PRINT_ERR("XMAI: fail to get video parameterss.");
        return -1;
    }
	
	stH264Config.bps = mBitRate;
	stH264Config.fps = mFPS;

    stH264Config.quality = mQuality - 3;
    if (stH264Config.quality < 0)
        stH264Config.quality = 0;
    if (stH264Config.quality > 3)
        stH264Config.quality = 3;

	stH264Config.gop = mGOP * mFPS;
	stH264Config.rc_mode = mbrmode;
#if 0
	//设置主码流参数
	pCompressionInfo = &(stChannelPicInfo.stRecordPara);
	#if 0
	pCompressionInfo->wEncodeAudio = mAudioEnable;
	pCompressionInfo->wEncodeVideo = mVideoEnable;
	#else
	pCompressionInfo->wEncodeAudio = 1;
	pCompressionInfo->wEncodeVideo = 1;	
	#endif
	pCompressionInfo->dwBitRate = mBitRate*1000; //mBitRate(Kb/s)
    pCompressionInfo->dwFrameRate = mFPS;
	pCompressionInfo->dwMaxKeyInterval = mGOP * mFPS;
	if(pCompressionInfo->dwMaxKeyInterval > 100)
		pCompressionInfo->dwMaxKeyInterval = 100;
	pCompressionInfo->dwImageQuality = 6-mQuality; //图像质量,只有在可变码流下才起作用,取值[1-6]，值越大，图像质量越好
	if(pCompressionInfo->dwImageQuality == 5)
		pCompressionInfo->dwImageQuality = 4;
	pCompressionInfo->dwRateType = mbrmode;

    if(!strcmp(mCompression, "H.264")){
        pCompressionInfo->dwCompressionType = DMS_PT_H264;
    }
    else if(!strcmp(mCompression, "MJPG")){
        pCompressionInfo->dwCompressionType = DMS_PT_MJPEG;
    }
#endif
	//分辨率
    if(!strcmp(mResolution, "D1")){
        //pCompressionInfo->dwStreamFormat = DMS_VIDEO_FORMAT_D1;
        stH264Config.width = 704;
		stH264Config.height = 576;
    }
    else if(!strcmp(mResolution, "HD1")){
        //pCompressionInfo->dwStreamFormat = DMS_VIDEO_FORMAT_HD1;
        stH264Config.width = 704;
		stH264Config.height = 288;
    }	
    else if(!strcmp(mResolution, "CIF")){
        //pCompressionInfo->dwStreamFormat = DMS_VIDEO_FORMAT_CIF;
        stH264Config.width = 352;
		stH264Config.height = 288;
    }	
    else if(!strcmp(mResolution, "QCIF")){
        //pCompressionInfo->dwStreamFormat = DMS_VIDEO_FORMAT_QCIF;
        stH264Config.width = 176;
		stH264Config.height = 144;
    }
    else if(!strcmp(mResolution, "VGA")){
        //pCompressionInfo->dwStreamFormat = DMS_VIDEO_FORMAT_VGA;
        stH264Config.width = 640;
		stH264Config.height = 480;
    }	
    else if(!strcmp(mResolution, "QVGA")){
        //pCompressionInfo->dwStreamFormat = DMS_VIDEO_FORMAT_QVGA;
        stH264Config.width = 320;
		stH264Config.height = 240;
    }
	else if(!strcmp(mResolution, "QQVGA")){
        //pCompressionInfo->dwStreamFormat = DMS_VIDEO_FORMAT_QQVGA;
        stH264Config.width = 160;
		stH264Config.height = 120;
    }
    //else if(!strcmp(exResolution, "SVCD")){
    //    pCompressionInfo->dwStreamFormat = DMS_VIDEO_FORMAT_XGA;
    //}
    //else if(!strcmp(exResolution, "650TVL")){
    //    pCompressionInfo->dwStreamFormat = DMS_VIDEO_FORMAT_960H;
    //}
    else if(!strcmp(mResolution, "720P")){
        //pCompressionInfo->dwStreamFormat = DMS_VIDEO_FORMAT_720P;
        stH264Config.width = 1080;
		stH264Config.height = 720;
    }
    else if(!strcmp(mResolution, "1_3M")){
        //pCompressionInfo->dwStreamFormat = DMS_VIDEO_FORMAT_960P;
        stH264Config.width = 1080;
		stH264Config.height = 960;
    }	
    //else if(!strcmp(exResolution, "UXGA")){
        //pCompressionInfo->dwStreamFormat = DMS_VIDEO_FORMAT_UXGA;
    //    stH264Config.width = 1900;
	//	stH264Config.height = 1200;
    //}	
    else if(!strcmp(mResolution, "1080P")){
        //pCompressionInfo->dwStreamFormat = DMS_VIDEO_FORMAT_1080P;
        stH264Config.width = 1920;
		stH264Config.height = 1080;
    }
    printf("stream0, %s, %d * %d", mResolution, stH264Config.width, stH264Config.height);
	ret = netcam_video_set(0, 0, &stH264Config);
    if(ret != 0)
    {
        PRINT_ERR("XMAI: fail to set  video parameters.");
        return -1;
    }	

	/////////////////////////////////////////
    ExtraFormat = cJSON_GetObjectItem(Encode, "ExtraFormat");
    if(!ExtraFormat ){
        PRINT_ERR("get Simplify.Encode.[0] ExtraFormat error\n");
        return -1;
    }

    fmt = cJSON_GetObjectItem(ExtraFormat, "AudioEnable");
    if(!fmt ){
        PRINT_ERR("get Simplify.Encode.[0] AlarmOutLatch error\n");
        return -1;
    }
    exAudioEnable =  fmt->type;

    fmt = cJSON_GetObjectItem(ExtraFormat, "VideoEnable");
    if(!fmt ){
        PRINT_ERR("get Simplify.Encode.[0] FTPEnable error\n");
        return -1;
    }
    exVideoEnable =  fmt->type;

    exVideo = cJSON_GetObjectItem(ExtraFormat, "Video");
    if(!exVideo ){
        PRINT_ERR("get Simplify.Encode.[0] ExtraFormat error\n");
        return -1;
    }

    fmt = cJSON_GetObjectItem(exVideo, "BitRate");
    if(!fmt ){
        PRINT_ERR("get Simplify.Encode.[0] FTPEnable error\n");
        return -1;
    }
    exBitRate =  fmt->valueint;

    fmt = cJSON_GetObjectItem(exVideo, "FPS");
    if(!fmt ){
        PRINT_ERR("get Simplify.Encode.[0] FTPEnable error\n");
        return -1;
    }
    exFPS =  fmt->valueint;

    fmt = cJSON_GetObjectItem(exVideo, "GOP");
    if(!fmt ){
        PRINT_ERR("get Simplify.Encode.[0] FTPEnable error\n");
        return -1;
    }
    exGOP =  fmt->valueint;

    fmt = cJSON_GetObjectItem(exVideo, "Quality");
    if(!fmt ){
        PRINT_ERR("get Simplify.Encode.[0] FTPEnable error\n");
        return -1;
    }
    exQuality =  fmt->valueint;

    fmt = cJSON_GetObjectItem(exVideo, "BitRateControl");
    if(!fmt ){
        PRINT_ERR("get Simplify.Encode.[0] FTPEnable error\n");
        return -1;
    }
    sprintf(exBitRateControl, "%s", fmt->valuestring);
    if(!strcmp(exBitRateControl, "VBR"))
        exbrmode= 1;
    else if(!strcmp(exBitRateControl, "CBR"))
        exbrmode= 0;

    fmt = cJSON_GetObjectItem(exVideo, "Compression");
    if(!fmt ){
        PRINT_ERR("get Simplify.Encode.[0] FTPEnable error\n");
        return -1;
    }
    sprintf(exCompression, "%s", fmt->valuestring);

    fmt = cJSON_GetObjectItem(exVideo, "Resolution");
    if(!fmt ){
        PRINT_ERR("get Simplify.Encode.[0] FTPEnable error\n");
        return -1;
    }
    sprintf(exResolution, "%s", fmt->valuestring);

	memset(&stH264Config, 0, sizeof(ST_GK_ENC_STREAM_H264_ATTR));
    ret = netcam_video_get(0, 1, &stH264Config);
    if(ret != 0)
    {
        PRINT_ERR("XMAI: fail to get video parameterss.");
        return -1;
    }

	stH264Config.bps = exBitRate;
	stH264Config.fps = exFPS;
    stH264Config.quality = exQuality - 3;
    if (stH264Config.quality < 0)
        stH264Config.quality = 0;
    if (stH264Config.quality > 3)
        stH264Config.quality = 3;
	stH264Config.gop = exGOP * exFPS;
	stH264Config.rc_mode = exbrmode;

#if 0
	//设置子码流参数
	pCompressionInfo = &(stChannelPicInfo.stNetPara);
	#if 0
	pCompressionInfo->wEncodeAudio = exAudioEnable;
	pCompressionInfo->wEncodeVideo = exVideoEnable;
	#else
	pCompressionInfo->wEncodeAudio = 1;
	pCompressionInfo->wEncodeVideo = 1;	
	#endif
	pCompressionInfo->dwBitRate = exBitRate*1000; //mBitRate(Kb/s)
    pCompressionInfo->dwFrameRate = exFPS;
	pCompressionInfo->dwMaxKeyInterval = exGOP * exFPS;
	if(pCompressionInfo->dwMaxKeyInterval > 100)
		pCompressionInfo->dwMaxKeyInterval = 100;
	pCompressionInfo->dwImageQuality = 6-exQuality; //图像质量,只有在可变码流下才起作用,取值[1-6]，值越大，图像质量越好
	if(pCompressionInfo->dwImageQuality == 5)
		pCompressionInfo->dwImageQuality = 4;
	pCompressionInfo->dwRateType = exbrmode;

    if(!strcmp(exCompression, "H.264")){
        pCompressionInfo->dwCompressionType = DMS_PT_H264;
    }
    else if(!strcmp(exCompression, "MJPG")){
        pCompressionInfo->dwCompressionType = DMS_PT_MJPEG;
    }
#endif

	//分辨率
    if(!strcmp(exResolution, "D1")){
        //pCompressionInfo->dwStreamFormat = DMS_VIDEO_FORMAT_D1;
        stH264Config.width = 704;
		stH264Config.height = 576;
    }
    else if(!strcmp(exResolution, "HD1")){
        //pCompressionInfo->dwStreamFormat = DMS_VIDEO_FORMAT_HD1;
        stH264Config.width = 704;
		stH264Config.height = 288;
    }	
    else if(!strcmp(exResolution, "CIF")){
        //pCompressionInfo->dwStreamFormat = DMS_VIDEO_FORMAT_CIF;
        stH264Config.width = 352;
		stH264Config.height = 288;
    }	
    else if(!strcmp(exResolution, "QCIF")){
        //pCompressionInfo->dwStreamFormat = DMS_VIDEO_FORMAT_QCIF;
        stH264Config.width = 176;
		stH264Config.height = 144;
    }
    else if(!strcmp(exResolution, "VGA")){
        //pCompressionInfo->dwStreamFormat = DMS_VIDEO_FORMAT_VGA;
        stH264Config.width = 640;
		stH264Config.height = 480;
    }	
    else if(!strcmp(exResolution, "QVGA")){
        //pCompressionInfo->dwStreamFormat = DMS_VIDEO_FORMAT_QVGA;
        stH264Config.width = 320;
		stH264Config.height = 240;
    }
	else if(!strcmp(exResolution, "QQVGA")){
        //pCompressionInfo->dwStreamFormat = DMS_VIDEO_FORMAT_QQVGA;
        stH264Config.width = 160;
		stH264Config.height = 120;
    }
    //else if(!strcmp(exResolution, "SVCD")){
    //    pCompressionInfo->dwStreamFormat = DMS_VIDEO_FORMAT_XGA;
    //}
    //else if(!strcmp(exResolution, "650TVL")){
    //    pCompressionInfo->dwStreamFormat = DMS_VIDEO_FORMAT_960H;
    //}
    else if(!strcmp(exResolution, "720P")){
        //pCompressionInfo->dwStreamFormat = DMS_VIDEO_FORMAT_720P;
        stH264Config.width = 1080;
		stH264Config.height = 720;
    }
    else if(!strcmp(exResolution, "1_3M")){
        //pCompressionInfo->dwStreamFormat = DMS_VIDEO_FORMAT_960P;
        stH264Config.width = 1080;
		stH264Config.height = 960;
    }	
    //else if(!strcmp(exResolution, "UXGA")){
        //pCompressionInfo->dwStreamFormat = DMS_VIDEO_FORMAT_UXGA;
    //    stH264Config.width = 1900;
	//	stH264Config.height = 1200;
    //}	
    else if(!strcmp(exResolution, "1080P")){
        //pCompressionInfo->dwStreamFormat = DMS_VIDEO_FORMAT_1080P;
        stH264Config.width = 1920;
		stH264Config.height = 1080;
    }	
    
    #if 1
    PRINT_INFO("mAudioEnable:%d,exAudioEnable:%d\n", mAudioEnable, exAudioEnable);
    PRINT_INFO("mVideoEnable:%d,exVideoEnable:%d\n", mVideoEnable, exVideoEnable);
    PRINT_INFO("mBitRate:%d,exBitRate:%d\n", mBitRate, exBitRate);
    PRINT_INFO("mFPS:%d,exFPS:%d\n", mFPS, exFPS);
    PRINT_INFO("mGOP:%d,exGOP:%d\n", mGOP, exGOP);
    PRINT_INFO("mQuality:%d,exQuality:%d\n", mQuality, exQuality);
    PRINT_INFO("mBitRateControl:%s,exBitRateControl:%s\n", mBitRateControl, exBitRateControl);
    PRINT_INFO("mCompression:%s,exCompression:%s\n", mCompression, exCompression);
    PRINT_INFO("mResolution:%s,exResolution:%s\n", mResolution, exResolution);
    #endif    

#if 0
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_SET_PICCFG, 0, &stChannelPicInfo, sizeof(DMS_NET_CHANNEL_PIC_INFO)))
    {
        PRINT_ERR();
		return -1;	
    }

	/* 保存修改到配置文件中 */ 
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_SET_SAVECFG, 0, 0, 0))
    {
        PRINT_ERR();
        return -1;        
    }   
#endif
    printf("stream1, %s, %d * %d", exResolution, stH264Config.width, stH264Config.height);
	ret = netcam_video_set(0, 1, &stH264Config);
    if(ret != 0)
    {
        PRINT_ERR("XMAI: fail to set  video parameters.");
        return -1;
    }
    return 0;
}

int HandleRecordSet(cJSON *json)
{
    cJSON  *fmt, *Record;
    int PacketLength = 0;
    int PreRecord = 0;
//    int Redundancy = 0;
    char RecordMode[26] = {0};
#if 0

	//获取参数
    DMS_NET_DEVICE_INFO stDevicInfo;
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_DEVICECFG, 0, &stDevicInfo, sizeof(DMS_NET_DEVICE_INFO)))
    {
        PRINT_ERR();
		return -1;		
    } 

	//获取参数
	DMS_NET_CHANNEL_RECORD stChannelRecord;
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_RECORDCFG, 0, &stChannelRecord, sizeof(DMS_NET_CHANNEL_RECORD)))
    {
        PRINT_ERR();
		return -1;		
    }
#endif
    Record = cJSON_GetObjectItem(json, "Record.[0]");
    if(!Record){
        PRINT_ERR("get Record.[0] error\n");
        return -1;
    }

    fmt = cJSON_GetObjectItem(Record, "PacketLength");
    if(!fmt ){
        PRINT_ERR("get Detect.LocalAlarm.[0] AlarmOutEnable error\n");
        return -1;
    }
    PacketLength =  fmt->valueint;

    fmt = cJSON_GetObjectItem(Record, "PreRecord");
    if(!fmt ){
        PRINT_ERR("get Detect.LocalAlarm.[0] AlarmOutLatch error\n");
        return -1;
    }
    PreRecord =  fmt->valueint;
    if(PreRecord)
        PreRecord = 1;

    fmt = cJSON_GetObjectItem(Record, "RecordMode");
    if(!fmt ){
        PRINT_ERR("get Detect.LocalAlarm.[0] FTPEnable error\n");
        return -1;
    }
    sprintf(RecordMode, "%s", fmt->valuestring);


	runRecordCfg.recordLen = PacketLength;
	runRecordCfg.preRecordTime = PreRecord;
	

	if(!strcmp(RecordMode, "ConfigRecord"))
	{
		runRecordCfg.recordMode = 0;
	}
	else if(!strcmp(RecordMode, "ManualRecord"))
	{
		runRecordCfg.recordMode = 1;
	}
	else if(!strcmp(RecordMode, "ClosedRecord"))
	{
		runRecordCfg.recordMode = 2;
	}
	else
	{
		PRINT_ERR();
		return -1;
	}

#if 0
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_SET_DEVICECFG, 0, &stDevicInfo, sizeof(DMS_NET_DEVICE_INFO)))
    {
        PRINT_ERR();
		return -1;		
    } 

    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_SET_RECORDCFG, 0, &stChannelRecord, sizeof(DMS_NET_CHANNEL_RECORD)))
    {
        PRINT_ERR();
		return -1;		
    }

	/* 保存修改到配置文件中 */ 
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_SET_SAVECFG, 0, 0, 0))
    {
        PRINT_ERR();
        return -1;        
    }
#endif

	RecordCfgSave();
    return 0;
}


int HandleLocalAlarmSet(cJSON *json)
{
    cJSON  *fmt, *LocalAlarm,*EventHandler;
    int AlarmOutEnable = 0;
    int AlarmOutLatch = 0;
    int FTPEnable = 0;
    int MailEnable = 0;
    int RecordEnable = 0;
    int RecordLatch = 0;
    int SnapEnable = 0;
    int VoiceEnable = 0;

#if 0
    //获取设备的音视频通道压缩参数
    DMS_NET_ALARMINCFG stAlarmIn;
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_ALARMINCFG, 0, &stAlarmIn, sizeof(DMS_NET_ALARMINCFG)))
    {  
		PRINT_ERR();
		return -1;	
    }
#endif
	
    LocalAlarm = cJSON_GetObjectItem(json, "Alarm.LocalAlarm.[0]");
    if(!LocalAlarm){
        PRINT_ERR("get Detect.LocalAlarm.[0] error\n");
        return -1;
    }
    
    fmt = cJSON_GetObjectItem(LocalAlarm, "Enable");
    if(!fmt ){
        PRINT_ERR("get Detect.LocalAlarm.[0] Enable error\n");
        return -1;
    }
	//if(fmt->type)
	//	stAlarmIn.byAlarmInHandle = 1;
	//else
	//	stAlarmIn.byAlarmInHandle = 0;

	/* 设置布防时间 */
	if(fmt->type)
	{
		//todo
	}

    EventHandler = cJSON_GetObjectItem(LocalAlarm, "EventHandler");
    if(!EventHandler ){
        PRINT_ERR("get Detect.LocalAlarm.[0] EventHandler error\n");
        return -1;
    }

    fmt = cJSON_GetObjectItem(EventHandler, "AlarmOutEnable");
    if(!fmt ){
        PRINT_ERR("get Detect.LocalAlarm.[0] AlarmOutEnable error\n");
        return -1;
    }
    AlarmOutEnable =  fmt->type;
	//if(AlarmOutEnable)
	//{
	//	stAlarmIn.stHandle.wActionFlag |= DMS_ALARM_EXCEPTION_TOALARMOUT;
	//	SET_BIT(stAlarmIn.stHandle.byRelAlarmOut, 0); // 0通道
	//}
	//else
	//{
	//	stAlarmIn.stHandle.wActionFlag &= (~DMS_ALARM_EXCEPTION_TOALARMOUT);
	//	CLR_BIT(stAlarmIn.stHandle.byRelAlarmOut, 0); // 0通道		
	//}	

    fmt = cJSON_GetObjectItem(EventHandler, "AlarmOutLatch");
    if(!fmt ){
        PRINT_ERR("get Detect.LocalAlarm.[0] AlarmOutLatch error\n");
        return -1;
    }
    AlarmOutLatch =  fmt->valueint;
	runAlarmCfg.alarmIn.handle.intervalTime = AlarmOutLatch;

    fmt = cJSON_GetObjectItem(EventHandler, "FTPEnable");
    if(!fmt ){
        PRINT_ERR("get Detect.LocalAlarm.[0] FTPEnable error\n");
        return -1;
    }
    FTPEnable =  fmt->type;
	if(FTPEnable)
	{
		//stAlarmIn.stHandle.wActionFlag |= DMS_ALARM_EXCEPTION_TOFTP;
		runAlarmCfg.alarmIn.handle.isSnapUploadToFtp = 1;
	}
	else
	{
		//stAlarmIn.stHandle.wActionFlag &= (~DMS_ALARM_EXCEPTION_TOFTP);	
		runAlarmCfg.alarmIn.handle.isSnapUploadToFtp = 0;
	}	

    fmt = cJSON_GetObjectItem(EventHandler, "MailEnable");
    if(!fmt ){
        PRINT_ERR("get Detect.LocalAlarm.[0] MailEnable error\n");
        return -1;
    }
    MailEnable =  fmt->type;
	if(MailEnable)
	{
		//stAlarmIn.stHandle.wActionFlag |= DMS_ALARM_EXCEPTION_TOEMAIL;
		runAlarmCfg.alarmIn.handle.is_email = 1;
	}
	else
	{
		//stAlarmIn.stHandle.wActionFlag &= (~DMS_ALARM_EXCEPTION_TOEMAIL);	
		runAlarmCfg.alarmIn.handle.is_email = 0;
	}	

    fmt = cJSON_GetObjectItem(EventHandler, "RecordEnable");
    if(!fmt ){
        PRINT_ERR("get Detect.LocalAlarm.[0] RecordEnable error\n");
        return -1;
    }
    RecordEnable =  fmt->type;
	if(RecordEnable)
	{
		//stAlarmIn.stHandle.wActionFlag |= DMS_ALARM_EXCEPTION_TOREC;
		//SET_BIT(stAlarmIn.stHandle.byRecordChannel, 0); // 0通道
		runAlarmCfg.alarmIn.handle.is_rec = 1;
	}
	else
	{
		//stAlarmIn.stHandle.wActionFlag &= (~DMS_ALARM_EXCEPTION_TOREC);
		//CLR_BIT(stAlarmIn.stHandle.byRecordChannel, 0); // 0通道
		runAlarmCfg.alarmIn.handle.is_rec = 0;
	}	

    fmt = cJSON_GetObjectItem(EventHandler, "RecordLatch");
    if(!fmt ){
        PRINT_ERR("get Detect.LocalAlarm.[0] RecordEnable error\n");
        return -1;
    }
    RecordLatch =  fmt->valueint;
	runAlarmCfg.alarmIn.handle.recTime = RecordLatch;

    fmt = cJSON_GetObjectItem(EventHandler, "SnapEnable");
    if(!fmt ){
        PRINT_ERR("get Detect.LocalAlarm.[0] SnapEnable error\n");
        return -1;
    }
    SnapEnable =  fmt->type;
	if(SnapEnable)
	{
		//stAlarmIn.stHandle.wActionFlag |= DMS_ALARM_EXCEPTION_TOSNAP;
		//SET_BIT(stAlarmIn.stHandle.bySnap, 0); // 0通道
		runAlarmCfg.alarmIn.handle.is_snap = 1;
	}
	else
	{
		//stAlarmIn.stHandle.wActionFlag &= (~DMS_ALARM_EXCEPTION_TOSNAP);
		//CLR_BIT(stAlarmIn.stHandle.bySnap, 0); // 0通道
		runAlarmCfg.alarmIn.handle.is_snap = 0;
	}		

#if 0
    fmt = cJSON_GetObjectItem(EventHandler, "VoiceEnable");
    if(!fmt ){
        PRINT_ERR("get Detect.MotionDetect.[0] VoiceEnable error\n");
        return -1;
    }
    VoiceEnable =  fmt->type;
	if(VoiceEnable)
	{
		stAlarmIn.stHandle.wActionFlag |= DMS_ALARM_EXCEPTION_TOVOICE;
	}
	else
	{
		stAlarmIn.stHandle.wActionFlag &= (~DMS_ALARM_EXCEPTION_TOVOICE);		
	}	

    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_SET_ALARMINCFG, 0, &stAlarmIn, sizeof(DMS_NET_ALARMINCFG)))
    {  
		PRINT_ERR();
		return -1;	
    }

	/* 保存修改到配置文件中 */ 
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_SET_SAVECFG, 0, 0, 0))
    {
        PRINT_ERR();
        return -1;        
    }
#endif

	AlarmCfgSave();
    return 0;
}


int HandleMotionDetectSet(cJSON *json)
{
    cJSON  *fmt, *MotionDetect,*EventHandler, *RegionArray, *TimeSection, *TimeSectionItem;
    int Level;
    int AlarmOutEnable = 0;
    int AlarmOutLatch = 0;
    int FTPEnable = 0;
    int MailEnable = 0;
    int RecordEnable = 0;
    int RecordLatch = 0;
    int SnapEnable = 0;
    int VoiceEnable = 0;
	int i = 0, day = 0;
	int x = 0, y = 0;
	int RegionSize = 0, TimeSectionSize = 0, TimeSectionItemSize = 0;	
	char strRegion[32][12] = {{0}};
	char strTimeSection[20] = {0};
	int TSEnable = 0, TSStartHour = 0, TSStartMin = 0, TSStartSec = 0;
	int TSEndHour = 0, TSEndMin = 0, TSEndSec = 0;
	unsigned long tmpRegion;

#if 0
    char *out = cJSON_Print(json);
    PRINT_INFO("%s\n",out);


    //获取设备的音视频通道压缩参数
    DMS_NET_CHANNEL_MOTION_DETECT stChannelMotionDetect;
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_MOTIONCFG, 0, &stChannelMotionDetect, sizeof(DMS_NET_CHANNEL_MOTION_DETECT)))
    {  
		PRINT_ERR();
		return -1;	
    }	
#endif

    MotionDetect = cJSON_GetObjectItem(json, "Detect.MotionDetect.[0]");
    if(!MotionDetect){
        PRINT_ERR("get Detect.MotionDetect.[0] error\n");
        return -1;
    }

    fmt = cJSON_GetObjectItem(MotionDetect, "Level");
    if(!fmt ){
        PRINT_ERR("get Detect.MotionDetect.[0] Level error\n");
        return -1;
    }
    Level = fmt->valueint;
	runMdCfg.sensitive = (Level - 1) * 100 / 5; //dwSensitive等级范围为[0-100], Level等级范围为[1, 6]

	int tmp = runMdCfg.sensitive / 20;
	SYS_MD_SENSITIVITY_t sensitivity = SYS_MD_SENSITIVITY_INVALID;
    if (tmp >=0 && tmp < 1)
        sensitivity= SYS_MD_SENSITIVITY_HIGHEST;
    else if (tmp >=1 && tmp < 2)
        sensitivity= SYS_MD_SENSITIVITY_HIGH;
    else if (tmp >=2 && tmp < 3)
        sensitivity= SYS_MD_SENSITIVITY_MEDIUM;
    else if (tmp >=3 && tmp < 4)
        sensitivity= SYS_MD_SENSITIVITY_LOW;
    else
        sensitivity= SYS_MD_SENSITIVITY_LOWEST;
    netcam_md_set_sensitivity(0, sensitivity);
	
    fmt = cJSON_GetObjectItem(MotionDetect, "Enable");
    if(!fmt ){
        PRINT_ERR("get Detect.MotionDetect.[0] Enable error\n");
        return -1;
    }
	if(fmt->type)
		runMdCfg.enable = 1;
	else
		runMdCfg.enable = 0;
	PRINT_INFO("open motiondetect:%u\n", runMdCfg.enable);

	/* 监测区域设置 */
    RegionArray = cJSON_GetObjectItem(MotionDetect, "Region");
    if(!RegionArray) {
        PRINT_ERR();
        return -1;
    }	
	RegionSize = cJSON_GetArraySize(RegionArray);
	PRINT_INFO("RegionSize=%d\n", RegionSize);
#if 0	
	for(y = 0; y < 18; y++)
	{
		sprintf(strRegion[y], "%s", cJSON_GetArrayItem(RegionArray, y)->valuestring);
		//printf("strRegion[%d]=%s\n", i, strRegion[i]);
		
		tmpRegion = strtoul(strRegion[y]+2, NULL, 16);
		
		for(x = 0; x < 22; x++)
		{
			if( tmpRegion & (1<<x) )
			{
				SET_BIT(stChannelMotionDetect.byMotionArea, 2*y*44 + 2*x);
				SET_BIT(stChannelMotionDetect.byMotionArea, 2*y*44 + 2*x + 1);
				SET_BIT(stChannelMotionDetect.byMotionArea, (2*y+1)*44 + 2*x);
				SET_BIT(stChannelMotionDetect.byMotionArea, (2*y+1)*44 + 2*x + 1);				
			}
			else
			{
				CLR_BIT(stChannelMotionDetect.byMotionArea, 2*y*44 + 2*x);
				CLR_BIT(stChannelMotionDetect.byMotionArea, 2*y*44 + 2*x + 1);
				CLR_BIT(stChannelMotionDetect.byMotionArea, (2*y+1)*44 + 2*x);
				CLR_BIT(stChannelMotionDetect.byMotionArea, (2*y+1)*44 + 2*x + 1);					
			}
		}		
		
	}



//验证打印
	/* 从 44 * 36 转换为 22* 18 */
	int fromX = 44;
	int fromY = 36;
	int toX = 22;	
	int toY = 18;
	//22*18格子，整形数组第一个元素的前22位有效，一个位标示一个格子是否选中，总共18个元素有效
	unsigned long mRegion[18] = {0};
	char strings[18][12] = {{0}};	
	int index;

    //22*18格子，整形数组第一个元素的前22位有效，一个位标示一个格子是否选中，总共18个元素有效
    for(y = 0; y < toY; y++)
    {
        for(x = 0; x < toX; x++)
        {
			index = y*fromX*fromY/toY + x*fromX/toX ;
			//isSet = (stChannelMotionDetect.byMotionArea[index/8] >> (index%8)) & 1 ;
            if( CHK_BIT(stChannelMotionDetect.byMotionArea, index) ) 
            {
				mRegion[y] |= (1<<x);
            }
        }
    }     

	for(y = 0; y < toY; y++)
	{
        sprintf(strings[y], "0x%08lx", mRegion[y]);
		printf("2 %d: %s\n", y, strings[y]);
	}				
#endif

	
    EventHandler = cJSON_GetObjectItem(MotionDetect, "EventHandler");
    if(!EventHandler ){
        PRINT_ERR("get Detect.MotionDetect.[0] EventHandler error\n");
        return -1;
    }

	/* 监测时间设置 */
    TimeSection = cJSON_GetObjectItem(EventHandler, "TimeSection");
    if(!TimeSection ){
        PRINT_ERR();
        return -1;
    }
	TimeSectionSize = cJSON_GetArraySize(TimeSection); //TimeSectionArraySize应该为7
	for(day = 0; day < 7; day++)
	{
		TimeSectionItem  = cJSON_GetArrayItem(TimeSection, day);
		TimeSectionItemSize = cJSON_GetArraySize(TimeSectionItem);
		for(i = 0; i < 2; i++) //实际就2个有效
		{
			memset(strTimeSection, 0, sizeof(strTimeSection));
		    sprintf(strTimeSection, "%s", cJSON_GetArrayItem(TimeSectionItem, i)->valuestring);
			
			//0 00:00:00-00:00:00
			sscanf(strTimeSection, "%d %02d:%02d:%02d-%02d:%02d:%02d",
						           &TSEnable, &TSStartHour, &TSStartMin, &TSStartSec,
						           &TSEndHour, &TSEndMin, &TSEndSec);
			
			if(TSEnable == 1)
			{
			    runMdCfg.scheduleTime[day][i].startHour= TSStartHour;
			    runMdCfg.scheduleTime[day][i].startMin= TSStartMin;
			    runMdCfg.scheduleTime[day][i].stopHour= TSEndHour;
			    runMdCfg.scheduleTime[day][i].stopMin= TSEndMin;	
			}
			else
			{
				runMdCfg.scheduleTime[day][i].startHour = 0;
			    runMdCfg.scheduleTime[day][i].startMin = 0;
			    runMdCfg.scheduleTime[day][i].stopHour = 0;
			    runMdCfg.scheduleTime[day][i].stopMin = 0;
			}
			
		}

		for(i = 2; i < 4; i++) //实际就2个有效
		{		
			runMdCfg.scheduleTime[day][i].startHour = 0;
		    runMdCfg.scheduleTime[day][i].startMin = 0;
		    runMdCfg.scheduleTime[day][i].stopHour = 0;
		    runMdCfg.scheduleTime[day][i].stopMin = 0;	
		}
	}	

	#if 0
	//打印时间段信息
	for(day = 0; day < 7; day++)
	{
		for(i = 0; i < 4; i++) //实际就2个有效
		{		
			printf("set, day:%d index:%d %02u:%02u:00-%02u:%02u:00\n",
			           day, i, 	stChannelMotionDetect.stScheduleTime[day][i].byStartHour,
								stChannelMotionDetect.stScheduleTime[day][i].byStartMin,
								stChannelMotionDetect.stScheduleTime[day][i].byStopHour,
								stChannelMotionDetect.stScheduleTime[day][i].byStopMin);	
		}			
	}
	#endif

	
    fmt = cJSON_GetObjectItem(EventHandler, "AlarmOutEnable");
    if(!fmt ){
        PRINT_ERR("get Detect.MotionDetect.[0] AlarmOutEnable error\n");
        return -1;
    }
    AlarmOutEnable =  fmt->type;
	if(AlarmOutEnable)
	{
		runMdCfg.handle.is_alarmout = 1;
		//stChannelMotionDetect.stHandle.wActionFlag |= DMS_ALARM_EXCEPTION_TOALARMOUT;
		//SET_BIT(stChannelMotionDetect.stHandle.byRelAlarmOut, 0); // 0通道
	}
	else
	{
		runMdCfg.handle.is_alarmout = 0;
		//stChannelMotionDetect.stHandle.wActionFlag &= (~DMS_ALARM_EXCEPTION_TOALARMOUT);
		//CLR_BIT(stChannelMotionDetect.stHandle.byRelAlarmOut, 0); // 0通道		
	}

    fmt = cJSON_GetObjectItem(EventHandler, "AlarmOutLatch");
    if(!fmt ){
        PRINT_ERR("get Detect.MotionDetect.[0] AlarmOutLatch error\n");
        return -1;
    }
    AlarmOutLatch =  fmt->valueint;
	runMdCfg.handle.duration = AlarmOutLatch;

    fmt = cJSON_GetObjectItem(EventHandler, "FTPEnable");
    if(!fmt ){
        PRINT_ERR("get Detect.MotionDetect.[0] FTPEnable error\n");
        return -1;
    }
    FTPEnable =  fmt->type;
	if(FTPEnable)
	{
		//stChannelMotionDetect.stHandle.wActionFlag |= DMS_ALARM_EXCEPTION_TOFTP;
		runMdCfg.handle.isSnapUploadToFtp = 1;
	}
	else
	{
		//stChannelMotionDetect.stHandle.wActionFlag &= (~DMS_ALARM_EXCEPTION_TOFTP);	
		runMdCfg.handle.isSnapUploadToFtp = 0;
	}	

    fmt = cJSON_GetObjectItem(EventHandler, "MailEnable");
    if(!fmt ){
        PRINT_ERR("get Detect.MotionDetect.[0] MailEnable error\n");
        return -1;
    }
    MailEnable =  fmt->type;
	if(MailEnable)
	{
		//stChannelMotionDetect.stHandle.wActionFlag |= DMS_ALARM_EXCEPTION_TOEMAIL;
		runMdCfg.handle.is_email = 1;
		runNetworkCfg.email.enableEmail = 1;
	}
	else
	{
		//stChannelMotionDetect.stHandle.wActionFlag &= (~DMS_ALARM_EXCEPTION_TOEMAIL);	
		runMdCfg.handle.is_email = 0;
		runNetworkCfg.email.enableEmail = 0;
	}	

    fmt = cJSON_GetObjectItem(EventHandler, "RecordEnable");
    if(!fmt ){
        PRINT_ERR("get Detect.MotionDetect.[0] RecordEnable error\n");
        return -1;
    }
    RecordEnable =  fmt->type;
	if(RecordEnable)
	{
		//stChannelMotionDetect.stHandle.wActionFlag |= DMS_ALARM_EXCEPTION_TOREC;
		//SET_BIT(stChannelMotionDetect.stHandle.byRecordChannel, 0); // 0通道
		runMdCfg.handle.is_rec = 1;
        runMdCfg.handle.recStreamNo = 1;
	}
	else
	{
		//stChannelMotionDetect.stHandle.wActionFlag &= (~DMS_ALARM_EXCEPTION_TOREC);
		//CLR_BIT(stChannelMotionDetect.stHandle.byRecordChannel, 0); // 0通道
		runMdCfg.handle.is_rec = 0;
	}	

    fmt = cJSON_GetObjectItem(EventHandler, "RecordLatch");
    if(!fmt ){
        PRINT_ERR("get Detect.MotionDetect.[0] RecordEnable error\n");
        return -1;
    }
    RecordLatch =  fmt->valueint;
	runMdCfg.handle.recTime = RecordLatch;
	

    fmt = cJSON_GetObjectItem(EventHandler, "SnapEnable");
    if(!fmt ){
        PRINT_ERR("get Detect.MotionDetect.[0] SnapEnable error\n");
        return -1;
    }
    SnapEnable =  fmt->type;
	if(SnapEnable)
	{
		//stChannelMotionDetect.stHandle.wActionFlag |= DMS_ALARM_EXCEPTION_TOSNAP;
		//SET_BIT(stChannelMotionDetect.stHandle.bySnap, 0); // 0通道
		runMdCfg.handle.is_snap = 1;
	}
	else
	{
		//stChannelMotionDetect.stHandle.wActionFlag &= (~DMS_ALARM_EXCEPTION_TOSNAP);
		//CLR_BIT(stChannelMotionDetect.stHandle.bySnap, 0); // 0通道
		runMdCfg.handle.is_snap = 0;
	}	

#if 0
    fmt = cJSON_GetObjectItem(EventHandler, "VoiceEnable");
    if(!fmt ){
        PRINT_ERR("get Detect.MotionDetect.[0] VoiceEnable error\n");
        return -1;
    }
    VoiceEnable =  fmt->type;
	if(VoiceEnable)
	{
		stChannelMotionDetect.stHandle.wActionFlag |= DMS_ALARM_EXCEPTION_TOVOICE;
	}
	else
	{
		stChannelMotionDetect.stHandle.wActionFlag &= (~DMS_ALARM_EXCEPTION_TOVOICE);		
	}	

	PRINT_INFO("open motiondetect:%u\n", stChannelMotionDetect.bEnable);

    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_SET_MOTIONCFG, 0, &stChannelMotionDetect, sizeof(DMS_NET_CHANNEL_MOTION_DETECT)))
    {  
		PRINT_ERR();
		return -1;	
    }

	/* 保存修改到配置文件中 */ 
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_SET_SAVECFG, 0, 0, 0))
    {
        PRINT_ERR();
        return -1;        
    }
#endif

	MdCfgSave();
	NetworkCfgSave();

    return 0;
}



int HandleAddr(char *resultBuf, char *sourceBuf)
{	
    char *p = NULL;
    unsigned int iAddress = 0;
    struct in_addr in;
    
    memset(&in, 0, sizeof(in));
    
    p = strstr(sourceBuf, "0x");
    if(p == NULL){
        PRINT_ERR("error p == NULL \n");
        return -1;
    }
    p += strlen("0x");
    iAddress = strtoul(p, NULL, 16); //strtol 出错
    //PRINT_INFO("sourceBuf:%s  iAddress:0x%x, iAddress:%d\n", sourceBuf,iAddress,iAddress);

    in.s_addr = iAddress;
    //PRINT_INFO("iAddress:%s\n", inet_ntoa(in));

    sprintf(resultBuf, "%s", inet_ntoa(in));

	//PRINT_INFO("sourceBuf:%s resultBuf:%s\n", sourceBuf, resultBuf);
	return 0;
}


int HandleNetCommonSet(cJSON *json)
{
    cJSON *element, *NetCommon;

    int httpPort = 0;
    char GateWay[128] = {0};
    char HostIP[128] = {0};
    char Submask[128] = {0};
#if 0
	//获取本机网络参数，
    DMS_NET_NETWORK_CFG st_ipc_net_info;
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_NETCFG, 0, &st_ipc_net_info, sizeof(st_ipc_net_info)))
    {
       PRINT_ERR("DMS_NET_GET_NETCFG fail\n");
       return -1;
    } 
#endif

    NetCommon = cJSON_GetObjectItem(json, "NetWork.NetCommon");
    if(!NetCommon){
        PRINT_ERR("get NetWork.NetCommon error\n");
        return -1;
    }

    element = cJSON_GetObjectItem(NetCommon, "HttpPort");
    if(!element){
        PRINT_ERR("get NetWork.NetCommon HttpPort error\n");
        return -1;
    }
    httpPort = element->valueint;
    
    element = cJSON_GetObjectItem(NetCommon, "GateWay");
    if(!element){
        PRINT_ERR("get NetWork.NetCommon GateWay error\n");
        return -1;
    }
    HandleAddr(GateWay, element->valuestring);

    element = cJSON_GetObjectItem(NetCommon, "HostIP");
    if(!element){
        PRINT_ERR("get NetWork.NetCommon HostIP error\n");
        return -1;
    }
    HandleAddr(HostIP, element->valuestring);

    element = cJSON_GetObjectItem(NetCommon, "Submask");
    if(!element){
        PRINT_ERR("get NetWork.NetCommon Submask error\n");
        return -1;
    }
    HandleAddr(Submask, element->valuestring);

	ST_SDK_NETWORK_ATTR net_attr;

	if (netcam_net_get_detect("eth0") != 0)
		strncpy(net_attr.name,netcam_net_wifi_get_devname(),sizeof(net_attr.name));
	else
		strncpy(net_attr.name,"eth0",sizeof(net_attr.name));
	
	if (netcam_net_get(&net_attr))
	{
		PRINT_ERR("Fail to get net_attr\n");
		return -1;
	}

	//st_ipc_net_info.wHttpPort = httpPort;
	strncpy(net_attr.ip, HostIP, sizeof(net_attr.ip));
	strncpy(net_attr.mask, Submask, sizeof(net_attr.mask));
	strncpy(net_attr.gateway, GateWay, sizeof(net_attr.gateway));

	PRINT_INFO("set config:\n");
	PRINT_INFO("httpport:%hu\n", httpPort);
	PRINT_INFO("ip:%s\n", net_attr.ip);
	PRINT_INFO("mask:%s\n", net_attr.mask);
	PRINT_INFO("gateway:%s\n", net_attr.gateway);
	
#if 0
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_SET_NETCFG, 0, &st_ipc_net_info, sizeof(st_ipc_net_info)))
    {
       PRINT_ERR("DMS_NET_GET_NETCFG fail\n");
       return -1;
    } 

	
	//重启前关闭
	g_xmai_pthread_run = 0;	
	/* 保存修改到配置文件中 */
    int ret = dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_SET_SAVECFG, 0, 0, 0); 
    if(0 != ret)
    {
        PRINT_ERR();
        return -1;        
    }
#endif

	netcam_net_set(&net_attr);
	
    //fxmaiSvr->fEnv->taskScheduler().scheduleDelayedTask(1000*1000UL,xmaiServer::reInitXmaiServer,this);
    //delete fxmaiSvr;
    return 0;
}

int HandleNetDNSSet(cJSON *json)
{
    cJSON *element;
    char aAddress[256] = {0};
    unsigned int iAddress = 0;
    char *p = NULL;
    struct in_addr in;
	char Address[16] = {0};

    element = cJSON_GetObjectItem(json, "NetWork.NetDNS");
    if(!element){
        PRINT_ERR("get NetWork.NetDHCP error\n");
        return -1;
    }

    element = cJSON_GetObjectItem(element, "Address");
    if(!element){
        PRINT_ERR("get NetWork.NetDNS Address error\n");
        return -1;
    }
    sprintf(aAddress, "%s", element->valuestring);
    //PRINT_INFO("aAddress:%s\n", aAddress);

    p = strstr(aAddress, "0x");
    if(p == NULL){
        PRINT_ERR("error p == NULL \n");
        return -1;
    }
    p += strlen("0x");
    iAddress = strtoul(p, NULL, 16);
    //PRINT_INFO("aAddress:%s  iAddress:%x, iAddress:%d\n", aAddress,iAddress,iAddress);

    memset(&in, 0, sizeof(in));
    in.s_addr = iAddress;
	sprintf(Address, "%s", inet_ntoa(in));
    //PRINT_INFO("Address:%s\n", Address);

#if 0
	//读取
	DMS_NET_NETWORK_CFG	stNetWorkConfig;
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_NETCFG, 0, &stNetWorkConfig, sizeof(stNetWorkConfig)))
    {
       PRINT_ERR("DMS_NET_GET_NETCFG fail\n");
       return -1;
    } 	
#endif

	ST_SDK_NETWORK_ATTR net_attr;

	if (netcam_net_get_detect("eth0") != 0)
		strncpy(net_attr.name,netcam_net_wifi_get_devname(),sizeof(net_attr.name));
	else
		strncpy(net_attr.name,"eth0",sizeof(net_attr.name));
	
	if (netcam_net_get(&net_attr))
	{
		PRINT_ERR("Fail to get net_attr\n");
		return -1;
	}
	
    //修改
	strncpy(net_attr.dns1, Address, sizeof(net_attr.dns1));
	PRINT_INFO("DNS1:%s\n", net_attr.dns1);

#if 0
	//设置
	if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_SET_NETCFG, 0, &stNetWorkConfig, sizeof(stNetWorkConfig)))
    {
       PRINT_ERR("DMS_NET_GET_NETCFG fail\n");
       return -1;
    }	


	//重启前关闭
	g_xmai_pthread_run = 0;	
	/* 保存修改到配置文件中 */
    int ret = dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_SET_SAVECFG, 0, 0, 0); 
    if(0 != ret)
    {
        PRINT_ERR();
        return -1;        
    }	
#endif

	netcam_net_set(&net_attr);
    return 0;
}

int HandleNetOrderSet(cJSON *json)
{
#if 0
	//重启前关闭
	g_xmai_pthread_run = 0;

	/* 保存修改到配置文件中 */ 
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_SET_SAVECFG, 0, 0, 0))
    {
        PRINT_ERR();
        return -1;        
    }	
#endif
	return 0;	
}

int HandleGeneralSet(cJSON *json)
{
    cJSON *element, *general;
	int autoLogout = 0;
	int localNo = 0;
	int snapInterval = 0;
	int bOverWrite = 0;
	char strOverWrite[100] = {0};
	char strMachineName[100] = {0};
	char strVideoOutPut[100] = {0};

	/* 解析获取参数 */
    general = cJSON_GetObjectItem(json, "General.General");
    if(!general){
        PRINT_ERR();
        return -1;
    }

    element = cJSON_GetObjectItem(general, "AutoLogout");
	if(element)
    	autoLogout = element->valueint;

    element = cJSON_GetObjectItem(general, "LocalNo");
	if(element)
    	localNo = element->valueint;	

    element = cJSON_GetObjectItem(general, "SnapInterval");
	if(element)
    	snapInterval = element->valueint;	

    element = cJSON_GetObjectItem(general, "OverWrite");
	if(element)
	{
	    sprintf(strOverWrite, "%s", element->valuestring);			
		if(!strcmp(strOverWrite, "StopRecord")) {
			bOverWrite = 0;
		}
		else if(!strcmp(strOverWrite, "OverWrite")){
			bOverWrite = 1;
		}
	}

    element = cJSON_GetObjectItem(general, "MachineName");
	if(element)
		sprintf(strMachineName, "%s", element->valuestring);

    element = cJSON_GetObjectItem(general, "VideoOutPut");
	if(element)
		sprintf(strVideoOutPut, "%s", element->valuestring);	

	runRecordCfg.recycleRecord = bOverWrite;
#if 0
	//读取参数
    DMS_NET_DEVICE_INFO stDevicInfo;
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_DEVICECFG, 0, &stDevicInfo, sizeof(stDevicInfo)))
    {
        PRINT_ERR();
		return -1;		
    }  	

	//修改
	stDevicInfo.byRecycleRecord = bOverWrite;
	
	//设置
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_SET_DEVICECFG, 0, &stDevicInfo, sizeof(stDevicInfo)))
    {
        PRINT_ERR();
		return -1;		
    } 

	/* 保存修改到配置文件中 */ 
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_SET_SAVECFG, 0, 0, 0))
    {
        PRINT_ERR();
        return -1;        
    }
#endif
	RecordCfgSave();
    return 0;
}


int HandleLocationSet(cJSON *json)
{
    cJSON *element, *general;
	int nDateFormat = 0;
	int nDateSeparator = 0;
	int nLanguage = 0;
	int nTimeFormat = 0;
	int nVideoFormat = 0;
	char strDateFormat[10] = {0};
	char strDateSeparator[4] = {0};
	char strLanguage[100] = {0};
	char strTimeFormat[4] = {0};
	char strVideoFormat[10] = {0};
		
	cJSON *dstStart, *dstEnd;
	int nDSTRule = 0;
	int nWorkDay = 0;
	int nStartYear = 0;
	int nStartMonth = 0;
	int nStartDay = 0;
	int nStartHour = 0;
	int nStartMinute = 0;
	int nStartWeek = 0;
	int nEndYear = 0;
	int nEndMonth = 0;
	int nEndDay = 0;
	int nEndHour = 0;
	int nEndMinute = 0;
	int nEndWeek = 0;
	char strDSTRule[10] = {0};
	

	/* 解析获取参数 */
    general = cJSON_GetObjectItem(json, "General.Location");
    if(!general){
        PRINT_ERR();
        return -1;
    }

    element = cJSON_GetObjectItem(general, "DateFormat");
    if(element)
    {
	    sprintf(strDateFormat, "%s", element->valuestring);	
		if(!strcmp(strDateFormat, "YYMMDD")) {
			nDateFormat = 0;
		}
		else if(!strcmp(strDateFormat, "MMDDYY")){
			nDateFormat = 1;
		}
		else if(!strcmp(strDateFormat, "DDMMYY")){
			nDateFormat = 2;
		}	
		//PRINT_INFO("DateFormat = %d\n", nDateFormat);
    }

    element = cJSON_GetObjectItem(general, "DateSeparator");
    if(element)
    {
	    sprintf(strDateSeparator, "%s", element->valuestring);	
		if(!strcmp(strDateSeparator, ".")) {
			nDateSeparator = 0;
		}
		else if(!strcmp(strDateSeparator, "-")){
			nDateSeparator = 1;
		}
		else if(!strcmp(strDateSeparator, "/")){
			nDateSeparator = 2;
		}	
		//PRINT_INFO("DateSeparator = %d\n", nDateSeparator);
    }	

    element = cJSON_GetObjectItem(general, "Language");
    if(element)
    {
	    sprintf(strLanguage, "%s", element->valuestring);	
		if(!strcmp(strLanguage, "SimpChinese")) {
			nLanguage = DMS_LANGUAGE_CHINESE_SIMPLIFIED;
		}
		else if(!strcmp(strLanguage, "English")){
			nLanguage = DMS_LANGUAGE_ENGLISH;
		}
		else {
			nLanguage = DMS_LANGUAGE_ENGLISH;
		}	
		//PRINT_INFO("Language = %d\n", nLanguage);
    }	

    element = cJSON_GetObjectItem(general, "TimeFormat");
    if(element)
    {
	    sprintf(strTimeFormat, "%s", element->valuestring);	
		if(!strcmp(strTimeFormat, "24")) {
			nTimeFormat = 0;
		}
		else if(!strcmp(strTimeFormat, "12")){
			nTimeFormat = 1;
		}
		
		//PRINT_INFO("TimeFormat = %d\n", nTimeFormat);
		nTimeFormat = 0; //只支持24小时制
    }

    element = cJSON_GetObjectItem(general, "VideoFormat");
    if(element)
    {
	    sprintf(strVideoFormat, "%s", element->valuestring);	
		if(!strcmp(strVideoFormat, "PAL")) {
			nVideoFormat = DMS_PAL;
		}
		else if(!strcmp(strVideoFormat, "NTSC")){
			nVideoFormat = DMS_NTSC;
		}
		//PRINT_INFO("VideoFormat = %d\n", nVideoFormat);
    }	

	/* 夏令时 */
    element = cJSON_GetObjectItem(general, "WorkDay");
	if(element)
	{
    	nWorkDay = element->valueint;
		//PRINT_INFO("WorkDay = %d\n", nWorkDay);
	}
	
    element = cJSON_GetObjectItem(general, "DSTRule");
    if(element)
    {
	    sprintf(strDSTRule, "%s", element->valuestring);	
		if(!strcmp(strDSTRule, "On")) {
			nDSTRule = 1;			
		}
		else if(!strcmp(strDSTRule, "Off")){
			nDSTRule = 0;
		}
		//PRINT_INFO("DSTRule = %d\n", nDSTRule);
    }

    dstStart = cJSON_GetObjectItem(general, "DSTStart");
    if(dstStart)
    {
	    element = cJSON_GetObjectItem(dstStart, "Year");
	    if(element)
	    	nStartYear = element->valueint;
	    element = cJSON_GetObjectItem(dstStart, "Month");
	    if(element)
	    	nStartMonth = element->valueint;
	    element = cJSON_GetObjectItem(dstStart, "Day");
	    if(element)
	    	nStartDay = element->valueint;
	    element = cJSON_GetObjectItem(dstStart, "Hour");
	    if(element)
	    	nStartHour = element->valueint;
	    element = cJSON_GetObjectItem(dstStart, "Minute");
	    if(element)
	    	nStartMinute = element->valueint;	
	    element = cJSON_GetObjectItem(dstStart, "Week");
	    if(element)
	    	nStartWeek = element->valueint;		
    }

    dstEnd = cJSON_GetObjectItem(general, "DSTEnd");
    if(dstEnd)
    {
	    element = cJSON_GetObjectItem(dstEnd, "Year");
	    if(element)
	    	nEndYear = element->valueint;
	    element = cJSON_GetObjectItem(dstEnd, "Month");
	    if(element)
	    	nEndMonth = element->valueint;
	    element = cJSON_GetObjectItem(dstEnd, "Day");
	    if(element)
	    	nEndDay = element->valueint;
	    element = cJSON_GetObjectItem(dstEnd, "Hour");
	    if(element)
	    	nEndHour = element->valueint;
	    element = cJSON_GetObjectItem(dstEnd, "Minute");
	    if(element)
	    	nEndMinute = element->valueint;	
	    element = cJSON_GetObjectItem(dstEnd, "Week");
	    if(element)
	    	nEndWeek = element->valueint;					
    }	

#if 0
	//读取参数
    DMS_NET_DEVICE_INFO stDevicInfo;
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_DEVICECFG, 0, &stDevicInfo, sizeof(stDevicInfo)))
    {
        PRINT_ERR();
		return -1;		
    }  	

	DMS_NET_ZONEANDDST stZoneAndDst;
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_ZONEANDDSTCFG, 0, &stZoneAndDst, sizeof(stZoneAndDst)))
    {
        PRINT_ERR();
		return -1;		
    }	

	DMS_NET_CHANNEL_OSDINFO stChannelOsdinfo;
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_OSDCFG, 0, &stChannelOsdinfo, sizeof(stChannelOsdinfo)))
    {
        PRINT_ERR();
		return -1;		
    }		
	PRINT_INFO("get osd date-time format:%u", stChannelOsdinfo.byReserve1);
#endif

	//修改
	runChannelCfg.channelInfo[0].osdDatetime.dateFormat = nDateFormat;
	runChannelCfg.channelInfo[0].osdDatetime.dateSprtr = nDateSeparator;
	runSystemCfg.deviceInfo.languageType = nLanguage;
	runChannelCfg.channelInfo[0].osdDatetime.timeFmt = nTimeFormat; //只支持24小时
	runSystemCfg.deviceInfo.videoType = nVideoFormat;

	ChannelCfgSave();
	SystemCfgSave();
#if 0
	stZoneAndDst.dwEnableDST = nDSTRule;
//#else
	stZoneAndDst.dwEnableDST = 0;

	stZoneAndDst.stBeginPoint.dwMonth = nStartMonth;
	stZoneAndDst.stBeginPoint.dwWeekNo = nStartWeek;
	stZoneAndDst.stBeginPoint.dwWeekDate = nStartDay;
	stZoneAndDst.stBeginPoint.dwHour = nStartHour;
	stZoneAndDst.stBeginPoint.dwMin = nStartMinute;
	
	stZoneAndDst.stEndPoint.dwMonth = nEndMonth;
	stZoneAndDst.stEndPoint.dwWeekNo = nEndWeek;
	stZoneAndDst.stEndPoint.dwWeekDate = nEndDay;
	stZoneAndDst.stEndPoint.dwHour = nEndHour;
	stZoneAndDst.stEndPoint.dwMin = nEndMinute;	


	//0: yyyy-mm-dd hh:mm:ss
	//1: mm-dd-yyyy hh:mm:ss
	//2: yyyy/mm/dd hh:mm:ss
	//3: mm/dd/yyyy hh:mm:ss
	//4: dd-mm-yyyy hh:mm:ss
	//5: dd/mm/yyyy hh:mm:ss
	if(stDevicInfo.byDateFormat == 0)
	{
		if( (stDevicInfo.byDateSprtr == 0) || (stDevicInfo.byDateSprtr == 1) )
			stChannelOsdinfo.byReserve1 = 0;
		else if(stDevicInfo.byDateSprtr == 2)
			stChannelOsdinfo.byReserve1 = 2;
	}
	else if(stDevicInfo.byDateFormat == 1)
	{
		if( (stDevicInfo.byDateSprtr == 0) || (stDevicInfo.byDateSprtr == 1) )
			stChannelOsdinfo.byReserve1 = 1;
		else if(stDevicInfo.byDateSprtr == 2)
			stChannelOsdinfo.byReserve1 = 3;
	}	
	else if(stDevicInfo.byDateFormat == 2)
	{
		if( (stDevicInfo.byDateSprtr == 0) || (stDevicInfo.byDateSprtr == 1) )
			stChannelOsdinfo.byReserve1 = 4;
		else if(stDevicInfo.byDateSprtr == 2)
			stChannelOsdinfo.byReserve1 = 5;
	}	
	PRINT_INFO("set osd date-time format:%u", stChannelOsdinfo.byReserve1);
	
	//设置
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_SET_DEVICECFG, 0, &stDevicInfo, sizeof(stDevicInfo)))
    {
        PRINT_ERR();
		return -1;		
    } 

    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_SET_ZONEANDDSTCFG, 0, &stZoneAndDst, sizeof(stZoneAndDst)))
    {
        PRINT_ERR();
		return -1;		
    }	

    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_SET_OSDCFG, 0, &stChannelOsdinfo, sizeof(stChannelOsdinfo)))
    {
        PRINT_ERR();
		return -1;		
    }

	/* 保存修改到配置文件中 */ 
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_SET_SAVECFG, 0, 0, 0))
    {
        PRINT_ERR();
        return -1;        
    }
#endif	
    return 0;
}


int HandleGUISet(cJSON *json)
{
    cJSON *element, *guiSet;
	int nWindowAlpha = 0;
	int nTimeTitleEnable = 0;
	int nChannelTitleEnable = 0;

	/* 解析获取参数 */
    guiSet = cJSON_GetObjectItem(json, "fVideo.GUISet");
    if(!guiSet){
        PRINT_ERR();
        return -1;
    }

    element = cJSON_GetObjectItem(guiSet, "WindowAlpha");
	if(element)
    	nWindowAlpha = element->valueint;

    element = cJSON_GetObjectItem(guiSet, "TimeTitleEnable");
	if(element)
    	nTimeTitleEnable = element->type;
    element = cJSON_GetObjectItem(guiSet, "ChannelTitleEnable");
	if(element)
    	nChannelTitleEnable = element->type;

	GK_NET_CHANNEL_INFO channelInfo;
	netcam_osd_get_info(0,&channelInfo);
#if 0
	//读取参数
	DMS_NET_CHANNEL_OSDINFO stChannelOsdinfo;
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_OSDCFG, 0, &stChannelOsdinfo, sizeof(stChannelOsdinfo)))
    {
        PRINT_ERR();
		return -1;		
    }	  		
#endif	

	//修改
	channelInfo.osdDatetime.enable = nTimeTitleEnable;
	channelInfo.osdChannelID.enable = nChannelTitleEnable;

#if 0	
	//设置
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_SET_OSDCFG, 0, &stChannelOsdinfo, sizeof(stChannelOsdinfo)))
    {
        PRINT_ERR();
		return -1;		
    }	 

	/* 保存修改到配置文件中 */ 
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_SET_SAVECFG, 0, 0, 0))
    {
        PRINT_ERR();
        return -1;        
    }
#endif

	int i;
	for(i = 0; i < netcam_video_get_channel_number(); i++)
	{
		memcpy(&runChannelCfg.channelInfo[i].osdDatetime, &channelInfo.osdDatetime, sizeof(GK_NET_OSD_DATETIME));
		memcpy(&runChannelCfg.channelInfo[i].osdChannelID, &channelInfo.osdChannelID, sizeof(GK_NET_OSD_CHANNEL_ID));
	}
    netcam_osd_update_id();
    //todo
    netcam_timer_add_task(netcam_osd_pm_save, NETCAM_TIMER_TWO_SEC, SDK_FALSE, SDK_TRUE);	// 8*250ms = 2s
    return 0;
}

int HandleEncodeStaticParamSet(cJSON *json)
{
	return 0;
}


int HandleConfigSet(char *Name, cJSON *json)
{
    if(!strcmp(Name, "AVEnc.VideoColor.[0]")){
        return HandleVideoColorSet(json);
    }
    else if(!strcmp(Name, "NetWork.NetDHCP")){
        return HandleNetDHCPSet(json);
    }
    else if(!strcmp(Name, "NetWork.NetFTP")){
        return HandleNetFTPCfgSet(json);
    }	
    else if(!strcmp(Name, "NetWork.NetEmail")){
        return HandleNetEmailCfgSet(json);
    }
    else if(!strcmp(Name, "Simplify.Encode.[0]")){
        return HandleEncodeCfgSet(json);
    }
    else if(!strcmp(Name, "Record.[0]")){
        return HandleRecordSet(json);
    }
    else if(!strcmp(Name, "Alarm.LocalAlarm.[0]")){
        return HandleLocalAlarmSet(json);
    }
    else if(!strcmp(Name, "Detect.MotionDetect.[0]")){
        return HandleMotionDetectSet(json);
    }
    else if(!strcmp(Name, "NetWork.NetCommon")){
        return HandleNetCommonSet(json);
    }	
    else if(!strcmp(Name, "NetWork.NetDNS")){
        return HandleNetDNSSet(json);
    }	
    else if(!strcmp(Name, "NetWork.NetOrder")){
        return HandleNetOrderSet(json);
    }
    else if(!strcmp(Name, "General.General")){
        return HandleGeneralSet(json);
    }	
    else if(!strcmp(Name, "General.Location")){
        return HandleLocationSet(json);
    }	
    else if(!strcmp(Name, "fVideo.GUISet")){
        return HandleGUISet(json);
    }
    else if(!strcmp(Name, "AVEnc.EncodeStaticParam")){
        return HandleEncodeStaticParamSet(json);
    }	
	
	
		
	return 0;
}

int resConfigSet(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root;

    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name","");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);

    out=cJSON_PrintUnformatted(root);
    //PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, CONFIG_SET_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0;  
}


int ReqConfigSetHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg)
{
    PRINT_INFO("<<ReqConfigSetHandle>>  sock:%d sessionType:%d\n", pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSessionType);
    XMaiMsgHeader *pXMaiMsgHeader = (XMaiMsgHeader *)msg;
    cJSON *json = NULL;
    cJSON *element = NULL;
    char *out = NULL;
    char Name[128]={0};
    
    if(pXMaiMsgHeader->dataLen == 0){
        PRINT_ERR("dataLen == 0 no json   must have jason\n");
        goto res;
    }
    
    json=cJSON_Parse(msg+20);
    if (!json){
        PRINT_ERR("Error before: [%s]\n", cJSON_GetErrorPtr());
        return -1;
    }

    out = cJSON_Print(json);
	//PRINT_INFO("%s\n",out);

    element = cJSON_GetObjectItem(json, "Name");
    if(element){
        sprintf(Name, "%s", element->valuestring);
    }

	PRINT_INFO("============ set %s config ============\n", Name);

	/* 将json中的内容设置到系统参数中去 */
    HandleConfigSet(Name, json);
    cJSON_Delete(json);
    free(out);	
	
res:
	//回应
	return resConfigSet(pXMaiSessionCtrl);
}

//////////////////////////////////////////////////////////////
int resChannelTileGet(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root, *ChannelTitle;

#if 0
	DMS_NET_CHANNEL_OSDINFO stChannelOsdinfo;
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_OSDCFG, 0, &stChannelOsdinfo, sizeof(stChannelOsdinfo)))
    {
        PRINT_ERR();
		return -1;		
    }	

	PRINT_INFO("get osd name:%s, show:%u\n", stChannelOsdinfo.csChannelName, stChannelOsdinfo.byShowChanName);
#endif

	GK_NET_CHANNEL_INFO channelInfo;
	netcam_osd_get_info(0,&channelInfo);

    root = cJSON_CreateObject();//创建项目
    cJSON_AddItemToObject(root,"ChannelTitle", ChannelTitle = cJSON_CreateArray());
    cJSON_AddItemToArray(ChannelTitle, cJSON_CreateString(channelInfo.osdChannelID.text));
    cJSON_AddStringToObject(root, "Name", "ChannelTitle");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);
    
    out=cJSON_PrintUnformatted(root);
    PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, CONFIG_CHANNELTILE_GET_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0;
}

int ReqChannelTileGetHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg)
{
    PRINT_INFO("<<ReqChannelTileGetHandle>>  sock:%d sessionType:%d\n", pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSessionType);
    XMaiMsgHeader *pXMaiMsgHeader = (XMaiMsgHeader *)msg;
    cJSON *json = NULL;
//    cJSON *element = NULL;
    char *out = NULL;

    if(pXMaiMsgHeader->dataLen == 0){
        PRINT_ERR("dataLen == 0 no json\n");
        goto RES;
    }
    
	json=cJSON_Parse(msg+20);
    if (!json){
        PRINT_ERR("Error before: [%s]\n", cJSON_GetErrorPtr());
        return -1;
    }
    
    out = cJSON_Print(json);
    //PRINT_INFO("%s\n",out);

    cJSON_Delete(json);
    free(out);

RES:
    //回应
    return resChannelTileGet(pXMaiSessionCtrl);
}

/////////////////////////////////////////////////
int HandleChannelTileSet(char *Name)
{
    char strTitleName[100] = {0};    
    sprintf(strTitleName, "%s", Name);

#if 0
	DMS_NET_CHANNEL_PIC_INFO channel_info;
    int ret = dms_sysnetapi_ioctrl (g_xmai_handle, DMS_NET_GET_PICCFG, 0, &channel_info, sizeof(channel_info));
    if(0 != ret)
    {
        PRINT_ERR();
		return -1;	 
    } 
	PRINT_INFO("osd name 1: %s \n", channel_info.csChannelName);

	DMS_NET_CHANNEL_OSDINFO stChannelOsdinfo;
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_OSDCFG, 0, &stChannelOsdinfo, sizeof(stChannelOsdinfo)))
    {
        PRINT_ERR();
		return -1;		
    }
	//PRINT_INFO("enable: %u\n", stChannelOsdinfo.byShowChanName);
	//PRINT_INFO("osd name: %s \n", stChannelOsdinfo.csChannelName);	
	//PRINT_INFO("osd name x: %lu \n", stChannelOsdinfo.dwShowNameTopLeftX);
	//PRINT_INFO("osd name y: %lu \n", stChannelOsdinfo.dwShowNameTopLeftY);

	stChannelOsdinfo.byShowChanName = 1;
#endif
	GK_NET_CHANNEL_INFO channelInfo;
	netcam_osd_get_info(0,&channelInfo);

	channelInfo.osdChannelID.enable = 1;

	strncpy(channelInfo.osdChannelID.text, strTitleName, sizeof(channelInfo.osdChannelID.text));
	PRINT_INFO("set osd name: %s, enable:%u\n", channelInfo.osdChannelID.text, channelInfo.osdChannelID.enable);

#if 0
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_SET_OSDCFG, 0, &stChannelOsdinfo, sizeof(stChannelOsdinfo)))
    {
        PRINT_ERR();
		return -1;		
    }	

    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_SET_SAVECFG, 0, 0, 0))
    {
        PRINT_ERR();
		return -1;	        
    } 
#endif

	int i;
	for(i = 0; i < netcam_video_get_channel_number(); i++)
	{
		memcpy(&runChannelCfg.channelInfo[i].osdDatetime, &channelInfo.osdDatetime, sizeof(GK_NET_OSD_DATETIME));
		memcpy(&runChannelCfg.channelInfo[i].osdChannelID, &channelInfo.osdChannelID, sizeof(GK_NET_OSD_CHANNEL_ID));
	}
    netcam_osd_update_id();
    //todo
    netcam_timer_add_task(netcam_osd_pm_save, NETCAM_TIMER_TWO_SEC, SDK_FALSE, SDK_TRUE);	// 8*250ms = 2s
	return 0;
}

int resChannelTileSet(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root;

    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name","");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);

    out=cJSON_PrintUnformatted(root);
    //PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, CONFIG_CHANNELTILE_SET_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0; 
}

int ReqChannelTileSetHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg)
{
    PRINT_INFO("<<ReqChannelTileSetHandle>>  sock:%d sessionType:%d\n", pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSessionType);
    XMaiMsgHeader *pXMaiMsgHeader = (XMaiMsgHeader *)msg;
    cJSON *json = NULL, *element,*ChannelTitle;
//    cJSON *element = NULL;
    char *out = NULL;
    char strName[128] = {0};

    if(pXMaiMsgHeader->dataLen == 0){
        PRINT_ERR("dataLen == 0 no json\n");
        goto RES;
    }
    
    json=cJSON_Parse(msg+20);
    if (!json){
        PRINT_ERR("Error before: [%s]\n", cJSON_GetErrorPtr());
        return -1;
    }

    
    element = cJSON_GetObjectItem(json, "ChannelTitle");
    if(element){
        ChannelTitle = cJSON_GetArrayItem(element,0);
        sprintf(strName, "%s", ChannelTitle->valuestring);
        PRINT_INFO("strName:%s\n", strName);
    }
    
    out = cJSON_Print(json);
    //PRINT_INFO("%s\n",out);
    
    cJSON_Delete(json);
    free(out);
    
    HandleChannelTileSet(strName);

RES:
    //回应
    return resChannelTileSet(pXMaiSessionCtrl);
}

/////////////////////////////////////////////////
int resChannelTileDotSet(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root;

    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name","");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);

    out=cJSON_PrintUnformatted(root);
	//PRINT_INFO("%s\n", out);
    
  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, CONFIG_CHANNELTILE_DOT_SET_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0;

}

int ReqChannelTileDotSetHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg)
{
    PRINT_INFO("<<ReqChannelTileDotSetHandle>>  sock:%d sessionType:%d\n", pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSessionType);
    XMaiMsgHeader *pXMaiMsgHeader = (XMaiMsgHeader *)msg;
    cJSON *json = NULL;
//    cJSON *element = NULL;
    char *out = NULL;

    if(pXMaiMsgHeader->dataLen == 0){
        PRINT_ERR("dataLen == 0 no json\n");
        goto RES;
    }
    
    json=cJSON_Parse(msg+20);
    if (!json){
        PRINT_ERR("Error before: [%s]\n", cJSON_GetErrorPtr());
        //return -1;
    }
    
    out = cJSON_Print(json);
    //PRINT_INFO("%s\n",out);
    
    cJSON_Delete(json);
    free(out);

RES:
    //回应
    return resChannelTileDotSet(pXMaiSessionCtrl);
}

/////////////////////////////////////////////////
int resTimeQuery(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root;

    

	/*
    DMS_SYSTEMTIME stSysTime;
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_CMD_GET_SYSTIME, 0, &stSysTime, sizeof(stSysTime)))
    {
        PRINT_ERR();
		return -1;		
    }	
	*/

	char strTime[128] = {0};
    long ts = time(NULL); 
    struct tm tt = {0}; 	
    struct tm *pTm = localtime_r(&ts, &tt);

    sprintf(strTime,"%d-%02d-%02d %02d:%02d:%02d",pTm->tm_year+1900,pTm->tm_mon+1,pTm->tm_mday,pTm->tm_hour,pTm->tm_min,pTm->tm_sec);
    PRINT_INFO("strTime:%s\n", strTime);
    
    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name","OPTimeQuery");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);
    cJSON_AddStringToObject(root, "OPTimeQuery",strTime);

    out=cJSON_PrintUnformatted(root);
    //PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, TIMEQUERY_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0; 
}

int ReqTimeQueryHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg)
{
    PRINT_INFO("<<ReqTimeQueryHandle>>  sock:%d sessionType:%d\n", pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSessionType);
    XMaiMsgHeader *pXMaiMsgHeader = (XMaiMsgHeader *)msg;
    cJSON *json = NULL;
    cJSON *element = NULL;
    char *out = NULL;
    char Name[128]={0};
    
    if(pXMaiMsgHeader->dataLen == 0){
        PRINT_ERR("dataLen == 0 no json   must have jason\n");
        goto res;
    }
    
    json=cJSON_Parse(msg+20);
    if (!json){
        PRINT_ERR("Error before: [%s]\n", cJSON_GetErrorPtr());
        return -1;
    }

    out = cJSON_Print(json);

    element = cJSON_GetObjectItem(json, "Name");

    if(element){
        sprintf(Name, "%s", element->valuestring);
        //PRINT_INFO("Name:%s\n", Name);
    }

    cJSON_Delete(json);
    free(out);

res:
    //回应
    return resTimeQuery(pXMaiSessionCtrl);
}

////////////////////////////////////////////////////
int resSyncTime(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root;
   // char str[1024] = {0};

    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name","");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);

    out=cJSON_PrintUnformatted(root);
    //PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, SYNC_TIME_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0; 
}


int ReqSyncTimeHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg)
{
    PRINT_INFO("<<ReqSyncTimeHandle>>  sock:%d sessionType:%d\n", pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSessionType);
    XMaiMsgHeader *pXMaiMsgHeader = (XMaiMsgHeader *)msg;
    cJSON *json = NULL;
    cJSON *element = NULL;
    char *out = NULL;
    char Name[128]={0};
    char OPTimeSettingNoRTC[128]={0};

    if(pXMaiMsgHeader->dataLen == 0){
        PRINT_ERR("dataLen == 0 no json\n");
        goto RES;
    }
    
	json=cJSON_Parse(msg+20);
    if (!json){
        PRINT_ERR("Error before: [%s]\n", cJSON_GetErrorPtr());
        return -1;
    }

    element = cJSON_GetObjectItem(json, "Name");
    if(element)
        sprintf(Name, "%s", element->valuestring);
    //PRINT_INFO("Name:%s\n", Name);

    element = cJSON_GetObjectItem(json, "OPTimeSettingNoRTC");
    if(element)
        sprintf(OPTimeSettingNoRTC, "%s", element->valuestring);
    //PRINT_INFO("OPTimeSettingNoRTC:%s\n", OPTimeSettingNoRTC);
    
    out = cJSON_Print(json);
    //PRINT_INFO("%s\n",out);
        
    cJSON_Delete(json);
    free(out);
    
RES:
    //处理 todo
    //handleSyncTime();
    
    //回应
    return resSyncTime(pXMaiSessionCtrl);
}


////////////////////////////////////////////////////
int resSetIframeReq(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;

    UtSetLvSockWaitI(pXMaiSessionCtrl->fSessionInt, UT_MAIN_STREAM, 1, &g_xmai_session);
    UtSetLvSockWaitI(pXMaiSessionCtrl->fSessionInt, UT_SUB_STREAM, 1, &g_xmai_session);

  	//加包头
    xmaiMsgLen = XMaiMakeMsgNoJsonMsg(pXMaiSessionCtrl, SET_IFRAME_RSP);
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0;	
}

int ReqSetIframeHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg)
{
    PRINT_INFO("<<ReqSetIframeHandle>>  sock:%d sessionType:%d\n", pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSessionType);
    XMaiMsgHeader *pXMaiMsgHeader = (XMaiMsgHeader *)msg;
    cJSON *json = NULL;
    char *out = NULL;
    
    if(pXMaiMsgHeader->dataLen == 0){
        PRINT_INFO("dataLen == 0 no json\n");
        goto res;
    }
    
    json=cJSON_Parse(msg+20);
    if (!json){
        PRINT_ERR("Error before: [%s]\n", cJSON_GetErrorPtr());
        return -1;
    }

    out = cJSON_Print(json);
    cJSON_Delete(json);
    free(out);

res:
    //回应
    return resSetIframeReq(pXMaiSessionCtrl);
  
}

////////////////////////////////////////////////////
int resSysTemInfo(XMaiSessionCtrl *pXMaiSessionCtrl)//todo
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root, *fmt;
//    char str[1024] = {0};

#if 0
	/*获取信息*/
	DMS_NET_DEVICE_INFO stDeviceInfo;
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_DEVICECFG, 0, &stDeviceInfo, sizeof(stDeviceInfo)))
    {
        PRINT_ERR();
		return -1;		
    }
	DMS_NET_PLATFORM_INFO stPlatformInfo;
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_PLATFORMCFG, 0, &stPlatformInfo, sizeof(stPlatformInfo)))
    {
        PRINT_ERR();
		return -1;		
    }	
#endif

	char SoftWareVersion[128] = {0};
	sprintf(SoftWareVersion, "%s", runSystemCfg.deviceInfo.softwareVersion);

	char HardWareVersion[128] = {0};
	sprintf(HardWareVersion, "%s", runSystemCfg.deviceInfo.hardwareVersion);	
	
	char SerialNo[40] = {0};
	sprintf(SerialNo, "%s", runSystemCfg.deviceInfo.serialNumber);	

	char BuildTime[40] = {0};
	sprintf(BuildTime, "%s", runSystemCfg.deviceInfo.softwareBuildDate);	//0x20131114;

	
	char HardWare[40] = {0};
	#if 0
	switch(stDeviceInfo.dwServerCPUType)
	{
		case CPUTYPE_3510:
			sprintf(HardWare, "%s", "CPU3510");
			break;
		case CPUTYPE_3511:
			sprintf(HardWare, "%s", "CPU3511");
			break;
		case CPUTYPE_3515:
			sprintf(HardWare, "%s", "CPU3515");
			break;
		case CPUTYPE_3516:
			sprintf(HardWare, "%s", "CPU3516");
			break;
		case CPUTYPE_3518:
			sprintf(HardWare, "%s", "CPU3518");
			break;			
		case CPUTYPE_8120B:
			sprintf(HardWare, "%s", "CPU8120B");
			break;	
		default:
			sprintf(HardWare, "%s", "CPUxxxx");
			break;
	}
	#else
	sprintf(HardWare, "%s", "GK710X");
	#endif



	#if 1
    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name", "SystemInfo");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);
    cJSON_AddItemToObject(root, "SystemInfo", fmt=cJSON_CreateObject());//在项目上添加项目
    cJSON_AddNumberToObject(fmt, "AlarmInChannel", 0);
    cJSON_AddNumberToObject(fmt, "AlarmOutChannel", 0);
    cJSON_AddNumberToObject(fmt, "AudioInChannel", 1);
    cJSON_AddStringToObject(fmt, "BuildTime", "2014-06-14 12:03:12");//todo
    cJSON_AddNumberToObject(fmt, "CombineSwitch", 0);
    cJSON_AddStringToObject(fmt, "DeviceRunTime", "0x0001CB2A");//todo
    cJSON_AddNumberToObject(fmt, "DigChannel", 0);
    cJSON_AddStringToObject(fmt, "EncryptVersion", "Unknown");
    cJSON_AddNumberToObject(fmt, "ExtraChannel", 0);
    cJSON_AddStringToObject(fmt, "HardWare", HardWare);
    cJSON_AddStringToObject(fmt, "HardWareVersion", HardWareVersion);
    cJSON_AddStringToObject(fmt, "SerialNo", SerialNo);//todo
    cJSON_AddStringToObject(fmt, "SoftWareVersion", SoftWareVersion);//todo
    cJSON_AddNumberToObject(fmt, "TalkInChannel", 1);
    cJSON_AddNumberToObject(fmt, "TalkOutChannel", 1);
    cJSON_AddNumberToObject(fmt, "VideoInChannel", 1);
    cJSON_AddNumberToObject(fmt, "VideoOutChannel", 1);
	#else
    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name", "SystemInfo");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);
    cJSON_AddItemToObject(root, "SystemInfo", fmt=cJSON_CreateObject());//在项目上添加项目
    cJSON_AddNumberToObject(fmt, "AlarmInChannel", 1);
    cJSON_AddNumberToObject(fmt, "AlarmOutChannel", 0);
    cJSON_AddNumberToObject(fmt, "AudioInChannel", 1);
    cJSON_AddStringToObject(fmt, "BuildTime", "2014-06-14 12:03:12");//todo
    cJSON_AddNumberToObject(fmt, "CombineSwitch", 0);
    cJSON_AddStringToObject(fmt, "DeviceRunTime", "0x0001CB2A");//todo
    cJSON_AddNumberToObject(fmt, "DigChannel", 0);
    cJSON_AddStringToObject(fmt, "EncryptVersion", "Unknown");
    cJSON_AddNumberToObject(fmt, "ExtraChannel", 0);
    cJSON_AddStringToObject(fmt, "HardWare", "50H10L");
    cJSON_AddStringToObject(fmt, "HardWareVersion", "Unknown");
    cJSON_AddStringToObject(fmt, "SerialNo", "3b5a42b369206937");//todo
    cJSON_AddStringToObject(fmt, "SoftWareVersion", "V4.02.R11.00001510.10010");//todo
    cJSON_AddNumberToObject(fmt, "TalkInChannel", 1);
    cJSON_AddNumberToObject(fmt, "TalkOutChannel", 1);
    cJSON_AddNumberToObject(fmt, "VideoInChannel", 1);
    cJSON_AddNumberToObject(fmt, "VideoOutChannel", 1);	
	#endif
    
    out=cJSON_PrintUnformatted(root);
    PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, SYSINFO_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//PrintXMaiMsgHeader(pXMaiSessionCtrl->fSendbuf);

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }

    return 0; 
}


int resStorageInfo(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root, *fmt, *fmt2, *fmt3, *fmt4;
//    char str[1024] = {0};

    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name", "StorageInfo");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);
    cJSON_AddItemToObject(root,"StorageInfo", fmt = cJSON_CreateArray());
    cJSON_AddItemToArray(fmt, fmt2 = cJSON_CreateObject());
    cJSON_AddNumberToObject(fmt2, "PartNumber", 0);
    cJSON_AddNumberToObject(fmt2, "PlysicalNo", 0);
    cJSON_AddItemToObject(fmt2,"Partition", fmt3 = cJSON_CreateArray());
    cJSON_AddItemToArray(fmt3, fmt4 = cJSON_CreateObject());
    cJSON_AddStringToObject(fmt4, "NewEndTime", "0000-00-00 00:00:00");
    cJSON_AddStringToObject(fmt4, "NewStartTime", "0000-00-00 00:00:00");
    cJSON_AddStringToObject(fmt4, "OldEndTime", "0000-00-00 00:00:00");
    cJSON_AddStringToObject(fmt4, "OldStartTime", "0000-00-00 00:00:00");
    cJSON_AddStringToObject(fmt4, "RemainSpace", "0x00000000");
    cJSON_AddStringToObject(fmt4, "TotalSpace", "0x00000000");
    cJSON_AddNumberToObject(fmt4, "DirverType", 0);  
    cJSON_AddNumberToObject(fmt4, "LogicSerialNo", 0); 
    cJSON_AddNumberToObject(fmt4, "Status", 0); 
    
    out=cJSON_PrintUnformatted(root);
    //PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, SYSINFO_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0; 
}

int resWorkState(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    PRINT_INFO("resWorkState\n");
	return 0;
}

int resSysInfo(XMaiSessionCtrl *pXMaiSessionCtrl, char *Name)
{
	PRINT_INFO("Name:%s\n", Name);
    if(!strcmp(Name, "SystemInfo")){
        return resSysTemInfo(pXMaiSessionCtrl);
    }
    else if(!strcmp(Name, "StorageInfo")){
        return resStorageInfo(pXMaiSessionCtrl);
    }   
    else if(!strcmp(Name, "WorkState")){
        return resWorkState(pXMaiSessionCtrl);
    } 
	else {
		return -1;
	}
	
	return 0;
}

int ReqSysinfoHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg)
{
    PRINT_INFO("<<ReqSysinfoHandle>>  sock:%d sessionType:%d\n", pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSessionType);
    XMaiMsgHeader *pXMaiMsgHeader = (XMaiMsgHeader *)msg;
    cJSON *json = NULL;
    cJSON *element = NULL;
    char *out = NULL;
    char Name[128]={0};
    char SessionID[128]={0};
    
    if(pXMaiMsgHeader->dataLen == 0){
        PRINT_ERR("***********dataLen == 0 must have json****************\n");
        return -1;
    }
	
	json=cJSON_Parse(msg+20);
    if (!json){
        PRINT_ERR("Error before: [%s]\n", cJSON_GetErrorPtr());
        return -1;
    }
    
    out = cJSON_Print(json);
    //PRINT_INFO("%s\n",out);

    element = cJSON_GetObjectItem(json, "Name");
    if(element)
        sprintf(Name, "%s", element->valuestring);
    //PRINT_INFO("Name:%s\n", Name);

    element = cJSON_GetObjectItem(json, "SessionID");
    if(element)
        sprintf(SessionID, "%s", element->valuestring);
    //PRINT_INFO("SessionID:%s\n", SessionID);
    cJSON_Delete(json);
    free(out);
    
//RES:
    //回应
    return resSysInfo(pXMaiSessionCtrl, Name);

}


////////////////////////////////////////////////////
int HandleOPMachine(cJSON *json)
{
    cJSON *element;
    char Action[256] = {0};

    element = cJSON_GetObjectItem(json, "OPMachine");
    if(!element)
        return -1;

    element = cJSON_GetObjectItem(element, "Action");
    if(element)
        sprintf(Action, "%s", element->valuestring);
    PRINT_INFO("Action:%s\n", Action);

    if(!strcmp(Action, "Reboot")){
		//重启前关闭线程
		g_xmai_pthread_run = 0;

#if 0
		/* 保存修改到配置文件中 */
	    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_SET_SAVECFG, 0, 0, 0))
	    {
	        PRINT_ERR();
	        return -1;        
	    }

		//重启
        dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_CMD_REBOOT, 0, 0, 0);
#endif
    }
    
    return 0;
}

int HandleOPTimeSetting(cJSON *json)
{
    cJSON *element;
    char OPTimeSetting[256] = {0};
    
    element = cJSON_GetObjectItem(json, "OPTimeSetting");
    if(element)
        sprintf(OPTimeSetting, "%s", element->valuestring);
    PRINT_INFO("OPTimeSetting:%s\n", OPTimeSetting);

    #if 0
	DMS_SYSTEMTIME stSysTime;
    memset(&stSysTime, 0, sizeof(stSysTime));

    sscanf(OPTimeSetting, "%hu-%02hu-%02hu %02hu:%02hu:%02hu",
           &stSysTime.wYear,&stSysTime.wMonth,&stSysTime.wDay,
           &stSysTime.wHour,&stSysTime.wMinute,&stSysTime.wSecond);
    
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_CMD_SET_SYSTIME, 0, &stSysTime, sizeof(stSysTime)))
    {
        PRINT_ERR();
		return -1;
    } 
    #else
    int year, month, day, hour, min, second;
    //OPTimeSetting:2017-12-29 00:05:42
    sscanf(OPTimeSetting, "%04d-%02d-%02d %02d:%02d:%02d",
           &year,&month,&day,
           &hour,&min,&second);
    printf("get : year=%d, month=%d, day=%d, hour=%d, minute=%d, senond=%d\n",
        year, month, day, hour, min, second);

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


    printf("localtime: %04d%02d%02d%02d%02d%02d\n", p->tm_year + 1900, p->tm_mon + 1, p->tm_mday,
                                                   p->tm_hour, p->tm_min, p->tm_sec);

    printf("gmtime: %04d%02d%02d%02d%02d%02d\n", p2->tm_year + 1900, p2->tm_mon + 1, p2->tm_mday,
                                                   p2->tm_hour, p2->tm_min, p2->tm_sec);

    
    #endif
    
    return 0;
}


int HandleSysManagerReq(char *Name, cJSON *json)
{
    if(!strcmp(Name, "OPMachine")){
        return HandleOPMachine(json);
    }
    else if(!strcmp(Name, "OPLogManager")){
        //return resOPLogManager();
    }   
    else if(!strcmp(Name, "OPDefaultConfig")){
        //return resOPDefaultConfig();
    } 
    else if(!strcmp(Name, "OPTimeSetting")){
        return HandleOPTimeSetting(json);
    }
	else
	{
		return -1;
	}	

	return 0;
}


int resSysManagerReq(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root;

    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name","");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);

    out=cJSON_PrintUnformatted(root);
    //PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, SYSMANAGER_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0;   
}


int ReqSysManagerHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg)
{
    PRINT_INFO("<<ReqSysManagerHandle>>  sock:%d sessionType:%d\n", pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSessionType);
    XMaiMsgHeader *pXMaiMsgHeader = (XMaiMsgHeader *)msg;
    cJSON *json = NULL;
    cJSON *element = NULL;
    char *out = NULL;
    char Name[128]={0};
    
    if(pXMaiMsgHeader->dataLen == 0){
        PRINT_ERR("dataLen == 0 no json   must have jason\n");
        return -1;
    }
    
    json=cJSON_Parse(msg+20);
    if (!json){
        PRINT_ERR("Error before: [%s]\n", cJSON_GetErrorPtr());
        return -1;
    }
    
    element = cJSON_GetObjectItem(json, "Name");
    if(element)
        sprintf(Name, "%s", element->valuestring);
    PRINT_INFO("Name:%s\n", Name);

    out = cJSON_Print(json);

    //处理
    HandleSysManagerReq(Name, json);
    cJSON_Delete(json);
    free(out);	

    //回应
    return resSysManagerReq(pXMaiSessionCtrl);

}

////////////////////////////////////////////////////
int resUsersGet(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root, *fmt, *fmt2, *fmt3, *fmt4;
    //const char *strings3[2]={"Monitor_01"};
    const char *strings2[2]={"Monitor_01", "Replay_01"};
    const char *strings1[24]={
            "ShutDown", "ChannelTitle", "RecordConfig", "Backup", "StorageManager", 
            "Account", "SysInfo", "QueryLog", 
    		"DelLog", "SysUpgrade", "AutoMaintain", "GeneralConfig", 
    		"EncodeConfig", "CommConfig", "NetConfig", "AlarmConfig", "VideoConfig", 
    		"PtzConfig", "PTZControl", "DefaultConfig", "Talk_01", "IPCCamera", "Monitor_01", 
    		"Replay_01"};

    root = cJSON_CreateObject();//创建项目
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);
    cJSON_AddItemToObject(root,"Users", fmt = cJSON_CreateArray());
    cJSON_AddItemToArray(fmt,fmt2 = cJSON_CreateObject());
    cJSON_AddStringToObject(fmt2, "Group", "admin");
    cJSON_AddStringToObject(fmt2, "Memo", "admin 's account");
    cJSON_AddStringToObject(fmt2, "Password", "tlJwpbo6");
    cJSON_AddStringToObject(fmt2, "Name", "admin");
    cJSON_AddTrueToObject(fmt2, "Reserved");
    cJSON_AddTrueToObject(fmt2, "Sharable");
    cJSON_AddItemToObject(fmt2,"AuthorityList", cJSON_CreateStringArray(strings1,24));

    cJSON_AddItemToArray(fmt,fmt3 = cJSON_CreateObject());
    cJSON_AddStringToObject(fmt3, "Group", "user");
    cJSON_AddStringToObject(fmt3, "Memo", "guest 's account");
    cJSON_AddStringToObject(fmt3, "Password", "tlJwpbo6");
    cJSON_AddStringToObject(fmt3, "Name", "guest");
    cJSON_AddTrueToObject(fmt3, "Reserved");
    cJSON_AddTrueToObject(fmt3, "Sharable");
    cJSON_AddItemToObject(fmt3,"AuthorityList", cJSON_CreateStringArray(strings2,2));

    cJSON_AddItemToArray(fmt,fmt4 = cJSON_CreateObject());
    cJSON_AddStringToObject(fmt4, "Group", "user");
    cJSON_AddStringToObject(fmt4, "Memo", "default account");
    cJSON_AddStringToObject(fmt4, "Password", "OxhlwSG8");
    cJSON_AddStringToObject(fmt4, "Name", "default");
    cJSON_AddFalseToObject(fmt4, "Reserved");
    cJSON_AddFalseToObject(fmt4, "Sharable");
    cJSON_AddItemToObject(fmt4,"AuthorityList", cJSON_CreateStringArray(strings2,2));
    
    out=cJSON_PrintUnformatted(root);
    //PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, USERS_GET_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0; 
}


int ReqUsersGetHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg)
{
    PRINT_INFO("<<ReqUsersGetHandle>>  sock:%d sessionType:%d\n", pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSessionType);
    XMaiMsgHeader *pXMaiMsgHeader = (XMaiMsgHeader *)msg;
    cJSON *json = NULL;
    cJSON *element = NULL;
    char *out = NULL;
    char Name[128]={0};
    
    if(pXMaiMsgHeader->dataLen == 0){
        PRINT_ERR("dataLen == 0 no json   must have jason\n");
        goto res;
    }
    
    json=cJSON_Parse(msg+20);
    if (!json){
        PRINT_ERR("Error before: [%s]\n", cJSON_GetErrorPtr());
        return -1;
    }

    out = cJSON_Print(json);

    element = cJSON_GetObjectItem(json, "Name");
    if(element){
        sprintf(Name, "%s", element->valuestring);
        PRINT_INFO("Name:%s\n", Name);
    }
    
    cJSON_Delete(json);
    free(out);

res:
    //回应
    return resUsersGet(pXMaiSessionCtrl);
 
}

////////////////////////////////////////////////////
int resGroupsGet(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root, *fmt, *fmt2, *fmt3;
    const char *strings2[2]={"Monitor_01", "Replay_01"};
    const char *strings[24]={
            "ShutDown", "ChannelTitle", "RecordConfig", "Backup", "StorageManager", "Account", 
            "SysInfo", "QueryLog", "DelLog", "SysUpgrade", "AutoMaintain", "GeneralConfig", 
            "EncodeConfig", "CommConfig", "NetConfig", "AlarmConfig", "VideoConfig", 
			"PtzConfig", "PTZControl", "DefaultConfig", "Talk_01", "IPCCamera", "Monitor_01", 
			"Replay_01"};
    
    root = cJSON_CreateObject();//创建项目
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);
    cJSON_AddItemToObject(root,"Groups", fmt = cJSON_CreateArray());
    cJSON_AddItemToArray(fmt,fmt2 = cJSON_CreateObject());
    cJSON_AddStringToObject(fmt2, "Memo", "administrator group");
    cJSON_AddStringToObject(fmt2, "Name", "admin");
    cJSON_AddItemToObject(fmt2,"AuthorityList", cJSON_CreateStringArray(strings,24));

    cJSON_AddItemToArray(fmt,fmt3 = cJSON_CreateObject());
    cJSON_AddStringToObject(fmt3, "Memo", "user group");
    cJSON_AddStringToObject(fmt3, "Name", "user");
    cJSON_AddItemToObject(fmt3,"AuthorityList", cJSON_CreateStringArray(strings2,2));
    
    out=cJSON_PrintUnformatted(root);
    //PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, GROUPS_GET_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0; 
}



int ReqGroupsGetHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg)
{
    PRINT_INFO("<<ReqGroupsGetHandle>>  sock:%d sessionType:%d\n", pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSessionType);
    XMaiMsgHeader *pXMaiMsgHeader = (XMaiMsgHeader *)msg;
    cJSON *json = NULL;
    cJSON *element = NULL;
    char *out = NULL;
    char Name[128]={0};
    
    if(pXMaiMsgHeader->dataLen == 0){
        PRINT_ERR("dataLen == 0 no json   must have jason\n");
        goto res;
    }
    
    json=cJSON_Parse(msg+20);
    if (!json){
        PRINT_ERR("Error before: [%s]\n", cJSON_GetErrorPtr());
        return -1;
    }

    out = cJSON_Print(json);

    element = cJSON_GetObjectItem(json, "Name");
    if(element){
        sprintf(Name, "%s", element->valuestring);
        PRINT_INFO("Name:%s\n", Name);
    }
    
    cJSON_Delete(json);
    free(out);

res:
    //回应
    return resGroupsGet(pXMaiSessionCtrl);
  
}

////////////////////////////////////////////////////
int resFullAuthorityList(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root;
    const char *strings[24]={
            "ShutDown", "ChannelTitle", "RecordConfig", "Backup", "StorageManager", 
            "Account", "SysInfo", "QueryLog", "DelLog", "SysUpgrade", "AutoMaintain", 
            "GeneralConfig", "EncodeConfig", "CommConfig", "NetConfig", "AlarmConfig", 
            "VideoConfig", "PtzConfig", "PTZControl", "DefaultConfig", "Talk_01", 
            "IPCCamera", "Monitor_01", "Replay_01"};
    
    root = cJSON_CreateObject();//创建项目
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);
    cJSON_AddItemToObject(root,"AuthorityList", cJSON_CreateStringArray(strings,24));
     
    out=cJSON_PrintUnformatted(root);
    //PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, FULLAUTHORITYLIST_GET_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0; 
}


int ReqFullAuthorityListHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg)
{
    PRINT_INFO("<<ReqFullAuthorityListHandle>>  sock:%d sessionType:%d\n", pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSessionType);
    XMaiMsgHeader *pXMaiMsgHeader = (XMaiMsgHeader *)msg;
    cJSON *json = NULL;
    cJSON *element = NULL;
    char *out = NULL;
    char Name[128]={0};
    
    if(pXMaiMsgHeader->dataLen == 0){
        PRINT_ERR("dataLen == 0 no json   must have jason\n");
        goto res;
    }
    
    json=cJSON_Parse(msg+20);
    if (!json){
        PRINT_ERR("Error before: [%s]\n", cJSON_GetErrorPtr());
        return -1;
    }

    out = cJSON_Print(json);

    element = cJSON_GetObjectItem(json, "Name");
    if(element){
        sprintf(Name, "%s", element->valuestring);
        PRINT_INFO("Name:%s\n", Name);
    }
    
    cJSON_Delete(json);
    free(out);

res:
    //回应
    return resFullAuthorityList(pXMaiSessionCtrl);
   
}

////////////////////////////////////////////////////
int HandleGuard()
{
#if 0
    DMS_NET_CHANNEL_MOTION_DETECT stChannelMotionDetect;
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_MOTIONCFG, 0, &stChannelMotionDetect, sizeof(DMS_NET_CHANNEL_MOTION_DETECT)))
    {  
		PRINT_ERR();
		return -1;	
    }

	stChannelMotionDetect.bEnable = 1;

    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_SET_MOTIONCFG, 0, &stChannelMotionDetect, sizeof(DMS_NET_CHANNEL_MOTION_DETECT)))
    {  
		PRINT_ERR();
		return -1;	
    }	
#endif
	return 0;
}

int resGuard(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root;
    //char str[1024] = {0};

    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name","");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);

    out=cJSON_PrintUnformatted(root);
    //PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, GUARD_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0; 
} 

int ReqGuardHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg)
{
    PRINT_INFO("<<ReqGuardHandle>>  sock:%d sessionType:%d\n", pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSessionType);
    XMaiMsgHeader *pXMaiMsgHeader = (XMaiMsgHeader *)msg;
    cJSON *json = NULL;
//    cJSON *element = NULL;
    char *out = NULL;
    
    if(pXMaiMsgHeader->dataLen == 0){
        PRINT_INFO("dataLen == 0 no json\n");
        goto RES;
    }
	
	json=cJSON_Parse(msg+20);
    if (!json){
        PRINT_ERR("Error before: [%s]\n", cJSON_GetErrorPtr());
        return -1;
    }
    
    out = cJSON_Print(json);
    //PRINT_INFO("%s\n",out);

    cJSON_Delete(json);
    free(out);

RES:
    //处理
    //HandleGuard(); //todo
    //fAlarmEnable = 1;

    //回应
    return resGuard(pXMaiSessionCtrl);

}

////////////////////////////////////////////////////
int HandlePtzReq(XMaiSessionCtrl *pXMaiSessionCtrl, cJSON *root)
{
    if(root == NULL)
        return -1;
    
    cJSON *OPPTZControl,*Pcommand,*pParameter,*pChannel,*pPreset,*pStep, *pTour;
    char Command[256] = {0};

    int Channel = 0;
    int Preset = -2;
    int Step = 0;
    int Tour = 0;

    OPPTZControl = cJSON_GetObjectItem(root, "OPPTZControl");
    if(!OPPTZControl){
        PRINT_ERR("OPPTZControl empty\n");
        return -1;
    }
    
    Pcommand = cJSON_GetObjectItem(OPPTZControl, "Command");
    if(!Pcommand){
        PRINT_ERR("Command empty\n");
        return -1;
    }
    sprintf(Command, "%s", Pcommand->valuestring);
    //PRINT_INFO("Command:%s\n", Command);

    pParameter = cJSON_GetObjectItem(OPPTZControl, "Parameter");
    if(!pParameter){
        PRINT_ERR("pParameter empty\n");
        return -1;
    }
    
    pChannel = cJSON_GetObjectItem(pParameter, "Channel");
    if(pChannel){
        Channel = pChannel->valueint;
        //PRINT_INFO("Channel:%d\n", Channel);
    }

    pPreset = cJSON_GetObjectItem(pParameter, "Preset");
    if(pPreset){
        Preset = pPreset->valueint;
        //PRINT_INFO("Preset:%d\n", Preset);
    }

    pStep = cJSON_GetObjectItem(pParameter, "Step");
    if(pStep){
        Step = pStep->valueint;
        //PRINT_INFO("Step:%d\n", Step);
    }
    
    pTour = cJSON_GetObjectItem(pParameter, "Tour");
    if(pTour){
        Tour = pTour->valueint;
        //PRINT_INFO("Tour:%d\n", Tour);
    }

	/* 使用得到的命令信息，去操作云台 */
	DMS_NET_PTZ_CONTROL struPtzControl;
	memset(&struPtzControl,0,sizeof(DMS_NET_PTZ_CONTROL) );

	struPtzControl.dwSize = sizeof(DMS_NET_PTZ_CONTROL);
	struPtzControl.nChannel = 0;

	int speed = 3;
    int step  = 10;
	
	/* Preset 为-1 则表示停止 */
    if(Preset == -1){
		PRINT_INFO("PTZ STOP\n");
		//struPtzControl.dwCommand = DMS_PTZ_CMD_STOP;
		//struPtzControl.dwParam = Step * 8; 
		netcam_ptz_stop();
    }    
    else if(!strcmp(Command, "DirectionUp")){
        PRINT_INFO("PTZ UP\n");
		//struPtzControl.dwCommand = DMS_PTZ_CMD_UP;  
		//struPtzControl.dwParam = Step * 8;
		netcam_ptz_up(step, speed);
    }
    else if(!strcmp(Command, "DirectionDown")){
        PRINT_INFO("PTZ DOWN\n");
		//struPtzControl.dwCommand = DMS_PTZ_CMD_DOWN; 
		//struPtzControl.dwParam = Step * 8;
		netcam_ptz_down(step, speed);
    }
    else if(!strcmp(Command, "DirectionLeft")){
        PRINT_INFO("PTZ LEFT\n");
		//struPtzControl.dwCommand = DMS_PTZ_CMD_LEFT;
		//struPtzControl.dwParam = Step * 8;
		netcam_ptz_left(step, speed);
    } 
    else if(!strcmp(Command, "DirectionRight")){
        PRINT_INFO("PTZ RIGHT\n");
		//struPtzControl.dwCommand = DMS_PTZ_CMD_RIGHT; 
		//struPtzControl.dwParam = Step * 8;
		netcam_ptz_right(step, speed);
    } 
    else if(!strcmp(Command, "DirectionLeftUp")){
        PRINT_INFO("PTZ UP_LEFT\n");
		//struPtzControl.dwCommand = DMS_PTZ_CMD_UP_LEFT; 
		//struPtzControl.dwParam = Step * 8;
		netcam_ptz_left_up(step, speed);
    } 
    else if(!strcmp(Command, "DirectionLeftDown")){
        PRINT_INFO("PTZ DOWN_LEFT\n");
		//struPtzControl.dwCommand = DMS_PTZ_CMD_DOWN_LEFT;  
		//struPtzControl.dwParam = Step * 8;
		netcam_ptz_left_down(step, speed);
    } 
    else if(!strcmp(Command, "DirectionRightUp")){
        PRINT_INFO("PTZ UP_RIGHT\n");
		//struPtzControl.dwCommand = DMS_PTZ_CMD_UP_RIGHT;
		//struPtzControl.dwParam = Step * 8;
		netcam_ptz_right_up(step, speed);
    } 
    else if(!strcmp(Command, "DirectionRightDown")){
        PRINT_INFO("PTZ DOWN_RIGHT\n");
		//struPtzControl.dwCommand = DMS_PTZ_CMD_DOWN_RIGHT;
		//struPtzControl.dwParam = Step * 8;
		netcam_ptz_right_down(step, speed);
    } 
    else if(!strcmp(Command, "ZoomWide")){ //变倍大
        PRINT_INFO("PTZ ZOOM SUB\n");
		//struPtzControl.dwCommand = DMS_PTZ_CMD_ZOOM_SUB;  
		//struPtzControl.dwParam = Step * 8;
    } 
    else if(!strcmp(Command, "ZoomTile")){  //变倍小
        PRINT_INFO("PTZ ZOOM ADD\n");
		//struPtzControl.dwCommand = DMS_PTZ_CMD_ZOOM_ADD; 
		//struPtzControl.dwParam = Step * 8;
    }
    else if(!strcmp(Command, "FocusNear")){ //聚焦近
        PRINT_INFO("PTZ FOCUS ADD\n");
		//struPtzControl.dwCommand = DMS_PTZ_CMD_FOCUS_ADD; 
		//struPtzControl.dwParam = Step * 8;
    } 
    else if(!strcmp(Command, "FocusFar")){  //聚焦远
        PRINT_INFO("PTZ FOCUS SUB\n");
		//struPtzControl.dwCommand = DMS_PTZ_CMD_FOCUS_SUB; 
		//struPtzControl.dwParam = Step * 8;
    } 
    else if((!strcmp(Command, "IRISLarge")) || (!strcmp(Command, "IrisLarge")) ){  //光圈变大
        PRINT_INFO("PTZ IRIS ADD\n");
		//struPtzControl.dwCommand = DMS_PTZ_CMD_IRIS_ADD;     
		//struPtzControl.dwParam = Step * 8;
    } 
    else if((!strcmp(Command, "IRISSmall")) || (!strcmp(Command, "IrisSmall")) ){  //光圈变小
        PRINT_INFO("PTZ IRIS SUB\n");
		//struPtzControl.dwCommand = DMS_PTZ_CMD_IRIS_SUB;   
		//struPtzControl.dwParam = Step * 8;
    }
	
    else if(!strcmp(Command, "SetPreset")){  //设置预置点
        PRINT_INFO("PTZ PRESET SET\n");
		//struPtzControl.dwCommand = DMS_PTZ_CMD_PRESET;  
		//struPtzControl.dwParam = Preset;

		int num = Preset;
		GK_NET_PRESET_INFO   gkPresetCfg;
		
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
		netcam_ptz_set_preset(num, NULL);
    } 
    else if(!strcmp(Command, "ClearPreset")){  //清除预置点
        PRINT_INFO("PTZ PRESET CLEAR, NOT SUPPORT.\n");
		//struPtzControl.dwCommand = DMS_PTZ_CMD_PRESET_CLS;
		//struPtzControl.dwParam = Preset;

		int num = 0;
		GK_NET_PRESET_INFO   gkPresetCfg;
		
		//num = ptz_control.dwParam;
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
		netcam_ptz_clr_preset(num);
    } 
    else if(!strcmp(Command, "GotoPreset")){ //转至预置点
        PRINT_INFO("PTZ PRESET GO\n");
		//struPtzControl.dwCommand = DMS_PTZ_CMD_CALL;  
		//struPtzControl.dwParam = Preset;

		int num = 0;
		GK_NET_CRUISE_GROUP  cruise_info;
		num = Preset;
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
        netcam_ptz_stop();
        netcam_ptz_preset_cruise(&cruise_info);
    } 	
	
    else if(!strcmp(Command, "AddTour")){  //增加巡航路线中的预置点
	    PRINT_INFO("PTZ ADD PRESET TO TOUR\n");
		#if 0
		struPtzControl.dwCommand = DMS_PTZ_CMD_ADD_POS_CRU;
	    struPtzControl.dwParam = Tour - 1; //巡航线路
	    
	    DMS_NET_CRUISE_POINT *stru_cruise_point = (DMS_NET_CRUISE_POINT *)struPtzControl.byRes;
	    stru_cruise_point->byPointIndex = 33; //统一用33,预置点添加到末尾
		stru_cruise_point->byPresetNo = Preset; //预置点编号
		stru_cruise_point->byRemainTime = 10;  //因为对方没传滞留时间过来，所以这写死
		stru_cruise_point->bySpeed = (5 * 8); //速度[1－8] 因为对方没传速度过来，所以这写死
		//#else
		//DMS_NET_CRUISE_POINT *stru_cruise_point = (DMS_NET_CRUISE_POINT *)struPtzControl.byRes;
		struPtzControl.byRes[0] = 33;     //统一用33,预置点添加到末尾
		struPtzControl.byRes[1] = Preset; //预置点编号
		struPtzControl.byRes[2] = Step;   //滞留时间
		struPtzControl.byRes[3] = 1;	  //速度	 
		#endif
		
    } 	
    else if(!strcmp(Command, "DeleteTour")){ //删除巡航路线中的预置点
		PRINT_INFO("PTZ DEL PRESET FROM TOUR\n");
		//struPtzControl.dwCommand = DMS_PTZ_CMD_DEL_PRE_CRU;
		
		//struPtzControl.dwParam = Tour - 1; //巡航线路
		//memcpy(struPtzControl.byRes, &Preset, sizeof(int)); 
		
    }
    else if(!strcmp(Command, "ClearTour")){  //清除巡航
        PRINT_INFO("PTZ CLEAR TOUR\n");
		#if 0
        DMS_NET_CRUISE_INFO stCruiseInfo;
        memset(&stCruiseInfo, 0, sizeof(DMS_NET_CRUISE_INFO));
        stCruiseInfo.byCruiseIndex = Tour - 1;
        PRINT_INFO("clear tour:%d\n", Tour - 1);
      	if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_SET_CRUISE_INFO, 0, &stCruiseInfo, sizeof(DMS_NET_CRUISE_INFO)))
    	{
            PRINT_ERR();
			return -1;
        }  
		#endif
		return 0;
    } 
    else if(!strcmp(Command, "StartTour")){  //开始巡航
        PRINT_INFO("PTZ start tour\n");
		//struPtzControl.dwCommand = DMS_PTZ_CMD_START_CRU;    
		//struPtzControl.dwParam = Tour - 1; //巡航线路	

		int grp_idx = Tour - 1;
		if (grp_idx >= PTZ_MAX_CRUISE_GROUP_NUM)
		{
			PRINT_ERR("START_CRU grp_idx:%d > PTZ_MAX_CRUISE_GROUP_NUM:%d\n", grp_idx, PTZ_MAX_CRUISE_GROUP_NUM);
			return -1;
		}

		int cnt;
		GK_NET_CRUISE_CFG    gkCruiseCfg;
		get_param(PTZ_CRUISE_PARAM_ID, &gkCruiseCfg);
		for (cnt = 0; cnt < PTZ_MAX_CRUISE_POINT_NUM; cnt++)
 		{
			if (gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt].byPresetNo > 0)
			{
				gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt].byPresetNo--;
			}
		}
		netcam_ptz_preset_cruise(&gkCruiseCfg.struCruise[grp_idx]);
    } 
    else if(!strcmp(Command, "StopTour")){  //停止巡航
        PRINT_INFO("PTZ stop tour\n");
		//struPtzControl.dwCommand = DMS_PTZ_CMD_STOP_CRU;  
		//struPtzControl.dwParam = Tour - 1; //巡航线路
		netcam_ptz_stop();
    }
    else if(!strcmp(Command, "AutoScanOn")){  //巡迹开始
        PRINT_INFO("PTZ AutoScanOn\n");
		//struPtzControl.dwCommand = DMS_PTZ_CMD_AUTO_STRAT;
		netcam_ptz_hor_ver_cruise(speed);
    } 
    else if(!strcmp(Command, "AutoScanOff")){ //巡迹结束
        PRINT_INFO("PTZ AutoScanOff\n");
		//struPtzControl.dwCommand = DMS_PTZ_CMD_AUTO_STOP;
		netcam_ptz_stop();
    } 
    else if(!strcmp(Command, "AutoPanOn")){  //开始水平旋转
        PRINT_INFO("PTZ AutoPanOn\n");
		//struPtzControl.dwCommand = DMS_PTZ_CMD_AUTO_STRAT;
		netcam_ptz_hor_ver_cruise(speed);
    } 
    else if(!strcmp(Command, "AutoPanOff")){  //停止水平旋转
        PRINT_INFO("PTZ AutoPanOff\n");
		//struPtzControl.dwCommand = DMS_PTZ_CMD_AUTO_STOP;
		netcam_ptz_stop();
    }
    else if(!strcmp(Command, "SetLimitLeft")){ //设置左边界     
        PRINT_INFO("PTZ SetLimitLeft, Not Support.\n");
    } 
    else if(!strcmp(Command, "SetLimitRight")){ //设置右边界
        PRINT_INFO("PTZ SetLimitRight, Not Support.\n");
    }	
    else if(!strcmp(Command, "LightOn")){
        PRINT_INFO("PTZ LIGHT OPEN\n");
		//struPtzControl.dwCommand = DMS_PTZ_CMD_LIGHT_OPEN;        
    } 
    else if(!strcmp(Command, "LightOff")){
		PRINT_INFO("PTZ LIGHT CLOSE\n");
        //struPtzControl.dwCommand = DMS_PTZ_CMD_LIGHT_CLOSE;
    }
    else if(!strcmp(Command, "Position")){  //快速定位,暂不支持
        PRINT_INFO("PTZ Position, Not Support.\n");
    } 
    else if(!strcmp(Command, "Menu")){  //菜单
        PRINT_INFO("PTZ Menu, Not Support.\n");
    } 
    else if(!strcmp(Command, "Default")){  //恢复出产默认配置
        PRINT_INFO("PTZ Default, Not Support.\n");
    } 
    else if(!strcmp(Command, "AUX")){ //辅助功能
        PRINT_INFO("PTZ AUX, Not Support.\n");
    } 
    else if(!strcmp(Command, "Flip")){
        PRINT_INFO("PTZ Flip, Not Support.\n");
    } 
    else if(!strcmp(Command, "Reset")){
        PRINT_INFO("PTZ Reset, Not Support.\n");
    }

#if 0
	if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_CMD_PTZ_CONTROL, 0, &struPtzControl, sizeof(DMS_NET_PTZ_CONTROL)))
	{
        PRINT_ERR();
        return -1;
    }	

    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_SET_SAVECFG, 0, 0, 0))
    {
        PRINT_ERR();
		return -1;	        
    }	
#endif
    return 0;
}

int resPtzReq(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root;

    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name","");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);

    out=cJSON_PrintUnformatted(root);
    //PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, PTZ_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0;   
}

int ReqPtzHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg)
{
    PRINT_INFO("<<ReqPtzHandle>>  sock:%d sessionType:%d\n", pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSessionType);
    XMaiMsgHeader *pXMaiMsgHeader = (XMaiMsgHeader *)msg;
    cJSON *json = NULL;
    char *out = NULL;
    
    if(pXMaiMsgHeader->dataLen == 0){
        PRINT_ERR("dataLen == 0 no json   must have jason\n");
        return -1;
    }
	
	json=cJSON_Parse(msg+20);
    if (!json){
        PRINT_ERR("Error before: [%s]\n", cJSON_GetErrorPtr());
        return -1;
    }
    
    out = cJSON_Print(json);
	//PRINT_INFO("%s\n",out);

    //处理
    HandlePtzReq(pXMaiSessionCtrl, json);
    cJSON_Delete(json);
    free(out);	

    //回应
    return resPtzReq(pXMaiSessionCtrl);

}


///////////////////////////////////////////////////////
int abilityEncodeObject(cJSON *pObject)
{
    cJSON_AddTrueToObject(pObject, "DoubleStream");
    cJSON_AddFalseToObject(pObject, "CombineStream");
    cJSON_AddFalseToObject(pObject, "SnapStream");

    return 0;
}

int abilityAlarmObject(cJSON *pObject)
{
    cJSON_AddFalseToObject(pObject, "AlarmConfig");
    cJSON_AddFalseToObject(pObject, "BlindDetect");
    cJSON_AddFalseToObject(pObject, "LossDetect");
    cJSON_AddTrueToObject(pObject, "MotionDetect");
    cJSON_AddFalseToObject(pObject, "NetAbort");
    cJSON_AddFalseToObject(pObject, "NetAlarm");
    cJSON_AddFalseToObject(pObject, "NetIpConflict");
    cJSON_AddFalseToObject(pObject, "StorageFailure");
    cJSON_AddFalseToObject(pObject, "StorageLowSpace");
    cJSON_AddFalseToObject(pObject, "StorageNotExist");

    return 0;
}

int abilityNetObject(cJSON *pObject)
{
    cJSON_AddFalseToObject(pObject, "Net3G");
    cJSON_AddFalseToObject(pObject, "NetARSP");
    cJSON_AddFalseToObject(pObject, "NetAlarmCenter");
    cJSON_AddFalseToObject(pObject, "NetDDNS");
    cJSON_AddTrueToObject(pObject, "NetDHCP");
    cJSON_AddTrueToObject(pObject, "NetDNS");
    cJSON_AddTrueToObject(pObject, "NetEmail");
    cJSON_AddTrueToObject(pObject, "NetFTP");
    cJSON_AddFalseToObject(pObject, "NetIPFilter");
    cJSON_AddFalseToObject(pObject, "NetMobile");
    cJSON_AddFalseToObject(pObject, "NetMutliCast");
    cJSON_AddFalseToObject(pObject, "NetNTP");
    cJSON_AddFalseToObject(pObject, "NetPPPoE");
    cJSON_AddFalseToObject(pObject, "NetPlatMega");
    cJSON_AddFalseToObject(pObject, "NetPlatShiSou");
    cJSON_AddFalseToObject(pObject, "NetPlatVVEye");
    cJSON_AddFalseToObject(pObject, "NetPlatXingWang");
    cJSON_AddFalseToObject(pObject, "NetRTSP");
    cJSON_AddTrueToObject(pObject, "NetDeviceDescUPNP");
    cJSON_AddFalseToObject(pObject, "StorageNotExist");
    cJSON_AddFalseToObject(pObject, "NetWifi");

    return 0;
} 


int resSystemFunction(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root, *SysObject, *AlarmObject, *EncodeObject, *NetObject;

    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name", "SystemFunction");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);
    cJSON_AddItemToObject(root,"SystemFunction", SysObject = cJSON_CreateObject());
    cJSON_AddItemToObject(SysObject,"EncodeFunction", EncodeObject = cJSON_CreateObject());
    abilityEncodeObject(EncodeObject);
    cJSON_AddItemToObject(SysObject,"AlarmFunction", AlarmObject = cJSON_CreateObject());
    abilityAlarmObject(AlarmObject);
    cJSON_AddItemToObject(SysObject,"NetServerFunction", NetObject = cJSON_CreateObject());
    abilityNetObject(NetObject); 
    
    out=cJSON_PrintUnformatted(root);
    PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, ABILITY_GET_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0; 
}

int resTalkAudioFormat(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root, *TalkArray, *TalkObject;
//    const char *strings[1]={"CAM01"};

    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name", "TalkAudioFormat");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);
    cJSON_AddItemToObject(root,"TalkAudioFormat", TalkArray = cJSON_CreateArray());
    cJSON_AddItemToArray(TalkArray, TalkObject = cJSON_CreateObject());
    cJSON_AddNumberToObject(TalkObject, "BitRate", 128);//待计算 //kbps为单位
    cJSON_AddNumberToObject(TalkObject, "SampleBit", 8);
    cJSON_AddNumberToObject(TalkObject, "SampleRate", 8000);
    cJSON_AddStringToObject(TalkObject, "EncodeType", "G711_ALAW");
    
    out=cJSON_PrintUnformatted(root);
    //PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, ABILITY_GET_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0; 
}

int resPTZcontrol(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root, *fmt;
//    const char *strings[1]={"CAM01"};

    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name", "PTZcontrol");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);
    cJSON_AddItemToObject(root,"PTZcontrol", fmt = cJSON_CreateObject());
    cJSON_AddStringToObject(fmt, "PTZPositon", "0x00000000");
    
    out=cJSON_PrintUnformatted(root);
    //PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, ABILITY_GET_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0; 
}

int resCamera(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root, *fmt;
    const char *strings[8]={"0x00004E20","0x0000208D","0x00000FA0","0x000007D0","0x000003E8","0x000001F4","0x000000FA","0x00000064"};

    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name", "Camera");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);
    cJSON_AddItemToObject(root,"Camera", fmt = cJSON_CreateObject());
    cJSON_AddNumberToObject(fmt, "Count", 8);
    cJSON_AddNumberToObject(fmt, "ElecLevel", 0);
    cJSON_AddNumberToObject(fmt, "Luminance", 0);
    cJSON_AddItemToObject(fmt,"Speeds", cJSON_CreateStringArray(strings,8));
    cJSON_AddNumberToObject(fmt, "Status", 0);
    cJSON_AddStringToObject(fmt, "Version", "");
    
    out=cJSON_PrintUnformatted(root);
    //PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, ABILITY_GET_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0; 
}

int resMaxPreRecord(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root;
    
    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name", "");
    cJSON_AddNumberToObject(root, "Ret", 101);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);
     
    out=cJSON_PrintUnformatted(root);
    //PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, ABILITY_GET_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0; 
}

int resMultiLanguage(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root;
	#if 0
    const char *strings[20]={
            "Arabic", "Brazilian", "English", "French", "German", "Greek", "Hebrew", 
            "Hungarian", "Italian", "Japanese", "Poland", "Portugal", "Romanian", 
            "Russian", "SimpChinese", "Spanish", "Finnish", "Thai", "TradChinese", 
            "Turkey"};
    #else
    const char *strings[2]={"English", "SimpChinese"};	
	#endif
    
    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name", "MultiLanguage");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);
    //cJSON_AddItemToObject(root,"MultiLanguage", cJSON_CreateStringArray(strings,20));
    cJSON_AddItemToObject(root,"MultiLanguage", cJSON_CreateStringArray(strings, 2));
     
    out=cJSON_PrintUnformatted(root);
    //PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, ABILITY_GET_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0; 
}

int resMultiVstd(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root;
    
    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name", "MultiVstd");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);
    cJSON_AddStringToObject(root, "MultiVstd", "PAL|NTSC");
         
    out=cJSON_PrintUnformatted(root);
    //PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, ABILITY_GET_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0; 
}

int resEncodeCapability(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0, i = 0;
	int nWidth = 0, nHeight = 0;
    char *out;
    cJSON *root, *fmtCapability, *CombEncodeInfo, *EncodeInfo,*elemObject, *fmt, *fmt2;
	/*
    const char *stringsMain1[32]={"0x00000801", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", 
													"0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", 
													"0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", 
													"0x00000000", "0x00000000"};
    */
   
#if 0	
    const char *stringsExtra2[19]={"0x00000021", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", 
							"0x00000000", "0x00000000", "0x00000021", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000" };
#endif
    const char *stringsExtra2[19]={"0x00000018", "0x00000000", "0x00000000", "0x00000018", "0x00000010", "0x00000078", "0x00000040", "0x00000000", "0x00000000", 
							"0x00000000", "0x00000000", "0x00000009", "0x00000009", "0x00000000", "0x00000009", "0x00000000", "0x00000000", "0x00000000", "0x00000000" };

	const char *stringsExtra3[32]={"0x00000019", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", 
					  "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", 
					  "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000"};
    const char *stringsExtra4[32]={"0x03441400", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", 
																"0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", 
																"0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", 
																"0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000"};




	int nMainCompressionMask = 0, nExCompressionMask = 0;
	int nMainResolutionMask = 0, nExResolutionMask = 0;
	char mCompressionMask[16] = {0};    
	char mResolutionMask[16] = {0};
	char exCompressionMask[16] = {0};    
	char exResolutionMask[16] = {0};	
    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name", "EncodeCapability");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);
    cJSON_AddItemToObject(root,"EncodeCapability", fmtCapability = cJSON_CreateObject());
    cJSON_AddNumberToObject(fmtCapability, "ChannelMaxSetSync", 0);	

	

#if 0
    //获取设备的音视频通道压缩参数
    DMS_NET_SUPPORT_STREAM_FMT stSupportFmt;
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_SUPPORT_STREAM_FMT, 0, &stSupportFmt, sizeof(stSupportFmt)))
    {
        PRINT_ERR();
		return -1;	
    }
	
	
	DMS_NET_DEVICE_INFO stDevicInfo;
    if(0 != dms_sysnetapi_ioctrl(g_xmai_handle, DMS_NET_GET_DEVICECFG, 0, &stDevicInfo, sizeof(stDevicInfo)))
    {
        PRINT_ERR();
		return -1;	
    }		
	printf("byVideoStandard = %d \n", stDevicInfo.byVideoStandard);
	printf("DMS_PAL = %d, DMS_NTSC = %d \n", DMS_PAL, DMS_NTSC);

    // 主码流支持的压缩方式，H264, MJPEG等
	for(i = 0; i < 4; i++)
    { 
    	// 0 主码流
		if(stSupportFmt.dwVideoSupportFmt[0][i] == PT_H264) 
			nMainCompressionMask |= (1<<SDK_CAPTURE_COMP_H264);
		else if(stSupportFmt.dwVideoSupportFmt[0][i] == PT_MJPEG)
			nMainCompressionMask |= (1<<SDK_CAPTURE_COMP_MJPG);		
		//else if(stSupportFmt.dwVideoSupportFmt[0][i] == DMS_PT_H264_BASELINE) //没有对应的
	}
    
	// 子码流支持的压缩方式，H264, MJPEG等
	for(i = 0; i < 4; i++)
    { 
    	// 1 子码流
		if(stSupportFmt.dwVideoSupportFmt[1][i] == PT_H264)
			nExCompressionMask |= (1<<SDK_CAPTURE_COMP_H264);
		else if(stSupportFmt.dwVideoSupportFmt[1][i] == PT_MJPEG)
			nExCompressionMask |= (1<<SDK_CAPTURE_COMP_MJPG);		
		//else if(stSupportFmt.dwVideoSupportFmt[0][i] == DMS_PT_H264_BASELINE) //没有对应的
	}
	

#else
    //DMS_NET_SUPPORT_STREAM_FMT stSupportFmt = {0};
    nMainCompressionMask |= (1<<SDK_CAPTURE_COMP_H264);
    nExCompressionMask |= (1<<SDK_CAPTURE_COMP_H264);
	sprintf(mCompressionMask, "0x%08x", nMainCompressionMask );
    sprintf(exCompressionMask, "0x%08x", nExCompressionMask );
#endif

#if 1
    //必须先填充video2Arr...
    cms_video_get_def_map();

    int MAX_STREAMNUM = 2;
    DMS_SIZE    videoSize[MAX_STREAMNUM][10];
    memset(videoSize, 0, sizeof(videoSize));
	//Video Size
	for(i = 0; i < MAX_STREAMNUM; i++)
	{
		ret = cms_get_resolution(videoSize[i], i, 10);
		if(ret != 0)
		{
			PRINT_ERR("CMS: fail to get resolution options.");
			return -1;
		}
	}
    
    int j = 0;
    for(i = 0; i < MAX_STREAMNUM; i++)
    {
        for(j = 0; j < 10; j++)
        {
            if (videoSize[i][j].nWidth != 0)
                PRINT_INFO("stream%d -- index:%d, %d * %d", i, j, 
                    videoSize[i][j].nWidth, videoSize[i][j].nHeight);
        }
    }
    j = 0;
    
#endif

    for(i = 0; i < 10; i++)
    {
        if (videoSize[0][i].nWidth == 0)
            continue;

        nWidth = videoSize[0][i].nWidth;
        nHeight = videoSize[0][i].nHeight;
        
        //printf("main stVideoSize[0][%d]: width=%d, height=%d\n", i, nWidth, nHeight);

        if( (704 == nWidth && 576 == nHeight)||(704 == nWidth && 480 == nHeight) )  
            nMainResolutionMask |= (1 << SDK_CAPTURE_SIZE_D1);
        else if( (704 == nWidth && 288 == nHeight)||(704 == nWidth && 240 == nHeight) )        
            nMainResolutionMask |= (1 << SDK_CAPTURE_SIZE_HD1);                 
        else if( (352 == nWidth && 288 == nHeight)||(352 == nWidth && 240 == nHeight) )         
            nMainResolutionMask |= (1 << SDK_CAPTURE_SIZE_CIF);    
        else if( (176 == nWidth && 144 == nHeight)||(176 == nWidth && 120 == nHeight) )         
            nMainResolutionMask |= (1 << SDK_CAPTURE_SIZE_QCIF);    
   
        if(640 == nWidth && 480 == nHeight)         
            nMainResolutionMask |= (1 << SDK_CAPTURE_SIZE_VGA);  
        else if(320 == nWidth && 240 == nHeight)        
            nMainResolutionMask |= (1 << SDK_CAPTURE_SIZE_QVGA);   
        else if(160 == nWidth && 120 == nHeight)         
            nMainResolutionMask |= (1 << SDK_CAPTURE_SIZE_QQVGA);
        else if(1024 == nWidth && 768 == nHeight)         
            nMainResolutionMask |= (1 << SDK_CAPTURE_SIZE_SVCD);
        else if(960 == nWidth && 576 == nHeight)
            nMainResolutionMask |= (1 << SDK_CAPTURE_SIZE_650TVL);      
        else if(1280 == nWidth && 720 == nHeight)
            nMainResolutionMask |= (1 << SDK_CAPTURE_SIZE_720P);        
        else if(1280 == nWidth && 960 == nHeight)         
            nMainResolutionMask |= (1 << SDK_CAPTURE_SIZE_1_3M); 
        else if(1900 == nWidth && 1200 == nHeight)         
            nMainResolutionMask |= (1 << SDK_CAPTURE_SIZE_UXGA);        
        else if(1920 == nWidth && 1080 == nHeight)         
            nMainResolutionMask |= (1 << SDK_CAPTURE_SIZE_1080P);   
  
        //不支持 SVGA 360P
    }
    sprintf(mResolutionMask, "0x%08x", nMainResolutionMask);

    for(i = 0; i < 10; i++)
    {
        if (videoSize[1][i].nWidth == 0)
            continue;

        nWidth = videoSize[1][i].nWidth;
        nHeight = videoSize[1][i].nHeight;
        
        //printf("main stVideoSize[0][%d]: width=%d, height=%d\n", i, nWidth, nHeight);

        if( (704 == nWidth && 576 == nHeight)||(704 == nWidth && 480 == nHeight) )  
            nExResolutionMask |= (1 << SDK_CAPTURE_SIZE_D1);
        else if( (704 == nWidth && 288 == nHeight)||(704 == nWidth && 240 == nHeight) )        
            nExResolutionMask |= (1 << SDK_CAPTURE_SIZE_HD1);                 
        else if( (352 == nWidth && 288 == nHeight)||(352 == nWidth && 240 == nHeight) )         
            nExResolutionMask |= (1 << SDK_CAPTURE_SIZE_CIF);    
        else if( (176 == nWidth && 144 == nHeight)||(176 == nWidth && 120 == nHeight) )         
            nExResolutionMask |= (1 << SDK_CAPTURE_SIZE_QCIF);    
   
        if(640 == nWidth && 480 == nHeight)         
            nExResolutionMask |= (1 << SDK_CAPTURE_SIZE_VGA);  
        else if(320 == nWidth && 240 == nHeight)        
            nExResolutionMask |= (1 << SDK_CAPTURE_SIZE_QVGA);   
        else if(160 == nWidth && 120 == nHeight)         
            nExResolutionMask |= (1 << SDK_CAPTURE_SIZE_QQVGA);
        else if(1024 == nWidth && 768 == nHeight)         
            nExResolutionMask |= (1 << SDK_CAPTURE_SIZE_SVCD);
        else if(960 == nWidth && 576 == nHeight)
            nExResolutionMask |= (1 << SDK_CAPTURE_SIZE_650TVL);      
        else if(1280 == nWidth && 720 == nHeight)
            nExResolutionMask |= (1 << SDK_CAPTURE_SIZE_720P);        
        else if(1280 == nWidth && 960 == nHeight)         
            nExResolutionMask |= (1 << SDK_CAPTURE_SIZE_1_3M); 
        else if(1900 == nWidth && 1200 == nHeight)         
            nExResolutionMask |= (1 << SDK_CAPTURE_SIZE_UXGA);        
        else if(1920 == nWidth && 1080 == nHeight)         
            nExResolutionMask |= (1 << SDK_CAPTURE_SIZE_1080P);   
  
        //不支持 SVGA 360P
    }
    sprintf(exResolutionMask, "0x%08x", nExResolutionMask);


	cJSON_AddItemToObject(fmtCapability,"CombEncodeInfo", CombEncodeInfo = cJSON_CreateArray());
    cJSON_AddItemToArray(CombEncodeInfo, elemObject = cJSON_CreateObject());	
    cJSON_AddStringToObject(elemObject, "CompressionMask", "0x00000000"); //SDK_CAPTURE_COMP_H264	
    cJSON_AddStringToObject(elemObject, "ResolutionMask", "0x00000000"); //SDK_CAPTURE_SIZE_D1 
    cJSON_AddStringToObject(elemObject, "StreamType", "MainStream");
    cJSON_AddTrueToObject(elemObject, "Enable");//需要细化判断
    cJSON_AddFalseToObject(elemObject, "HaveAudio");	
    cJSON_AddItemToArray(CombEncodeInfo, elemObject = cJSON_CreateObject());	
    cJSON_AddStringToObject(elemObject, "CompressionMask", "0x00000000");	
    cJSON_AddStringToObject(elemObject, "ResolutionMask", "0x00000000");
    cJSON_AddStringToObject(elemObject, "StreamType", "ExtraStream2");
    cJSON_AddTrueToObject(elemObject, "Enable");//需要细化判断
    cJSON_AddFalseToObject(elemObject, "HaveAudio");

    cJSON_AddItemToObject(fmtCapability,"EncodeInfo", EncodeInfo = cJSON_CreateArray());
    cJSON_AddItemToArray(EncodeInfo, elemObject = cJSON_CreateObject());
    cJSON_AddStringToObject(elemObject, "CompressionMask", mCompressionMask);//需要支持其他的
    cJSON_AddStringToObject(elemObject, "ResolutionMask", mResolutionMask);
    cJSON_AddStringToObject(elemObject, "StreamType", "MainStream");
    cJSON_AddTrueToObject(elemObject, "Enable");//需要细化判断
    cJSON_AddTrueToObject(elemObject, "HaveAudio");
    cJSON_AddItemToArray(EncodeInfo, elemObject = cJSON_CreateObject());
    cJSON_AddStringToObject(elemObject, "CompressionMask", exCompressionMask);
    cJSON_AddStringToObject(elemObject, "ResolutionMask", exResolutionMask);//暂时写死
    cJSON_AddStringToObject(elemObject, "StreamType", "ExtraStream2");
    cJSON_AddTrueToObject(elemObject, "Enable");//需要细化判断
    cJSON_AddTrueToObject(elemObject, "HaveAudio");
#if 0
    cJSON_AddItemToObject(fmtCapability,"ImageSizePerChannel", cJSON_CreateStringArray(stringsMain1,32));
#else
	cJSON_AddItemToObject(fmtCapability,"ImageSizePerChannel", fmt = cJSON_CreateArray());
	//char strExtra1[32][16] = {{0}}; 
	//for(i = 0; i < 32; i++)
	char strExtra1[1][16] = {{0}}; 
	for(i = 0; i < 1; i++)
	{
		sprintf(strExtra1[i], "0x%08x", nMainResolutionMask);
		cJSON_AddItemToArray(fmt, cJSON_CreateString(strExtra1[i]));
	}
#endif
    //cJSON_AddItemToObject(fmtCapability,"ExImageSizePerChannel", cJSON_CreateStringArray(stringsExtra3,32));
	cJSON_AddItemToObject(fmtCapability,"ExImageSizePerChannel", fmt = cJSON_CreateArray());
	char strExtra3[1][16] = {{0}}; 
	for(i = 0; i < 1; i++)
	{
		sprintf(strExtra3[i], "0x%08x", nExResolutionMask);
		cJSON_AddItemToArray(fmt, cJSON_CreateString(strExtra3[i]));
	}

    cJSON_AddItemToObject(fmtCapability, "ExImageSizePerChannelEx", fmt = cJSON_CreateArray());
    
    #if 0
    cJSON_AddItemToObject(fmt, "ExImageSizePerChannelEx", fmt2= cJSON_CreateArray()); 
	char strExtra2[19][16] = {{0}}; 
	for(i = 0; i < 19; i++)
	{
		//sprintf(strExtra2[i], "0x%08x", nExResolutionMask);
        //sprintf(strExtra2[i], "0x00000041");
        sprintf(strExtra2[i], "0x00000018");
		cJSON_AddItemToArray(fmt2, cJSON_CreateString(strExtra2[i]));
	}
    #else
    cJSON_AddItemToObject(fmt, "ExImageSizePerChannelEx", cJSON_CreateStringArray(stringsExtra2,19)); 
    #endif
    
    cJSON_AddNumberToObject(fmtCapability, "MaxBitrate", 49152);//暂时写死
    //cJSON_AddNumberToObject(fmtCapability, "MaxEncodePower", 54793216);//暂时写死
    cJSON_AddNumberToObject(fmtCapability, "MaxEncodePower", 72576000);//暂时写死
    
    //cJSON_AddItemToObject(fmtCapability,"MaxEncodePowerPerChannel", cJSON_CreateStringArray(stringsExtra4,32));
	cJSON_AddItemToObject(fmtCapability,"MaxEncodePowerPerChannel", fmt = cJSON_CreateArray());
	char strExtra4[1][16] = {{0}}; 
	for(i = 0; i < 1; i++)
	{
		//sprintf(strExtra4[i], "0x%08x", 54793216);
        sprintf(strExtra4[i], "0x%08x", 72576000);
		cJSON_AddItemToArray(fmt, cJSON_CreateString(strExtra4[i]));
	}

    
    out=cJSON_Print(root);
    PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, ABILITY_GET_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0; 
}

int resNetOrder(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root;
    
    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name", "");
    cJSON_AddNumberToObject(root, "Ret", 101);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);
         
    out=cJSON_PrintUnformatted(root);
    //PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, ABILITY_GET_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0; 
}

int resBlindCapability(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root;
    
    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name", "");
    cJSON_AddNumberToObject(root, "Ret", 101);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);
         
    out=cJSON_PrintUnformatted(root);
    //PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, ABILITY_GET_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}
	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }        
    return 0; 
}

int resMotionArea(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root, *fmt;
    
    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name", "MotionArea.[0]");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);
    cJSON_AddItemToObject(root,"MotionArea.[0]", fmt = cJSON_CreateObject());
	#if 0
	cJSON_AddNumberToObject(fmt, "GridRow", 32);
    cJSON_AddNumberToObject(fmt, "GridColumn", 32);
	#else
	cJSON_AddNumberToObject(fmt, "GridColumn", 22);
	cJSON_AddNumberToObject(fmt, "GridRow", 18);    	
	#endif
	
    out=cJSON_PrintUnformatted(root);
    PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, ABILITY_GET_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}
	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }        
    return 0; 
}

int resAbilityGet(XMaiSessionCtrl *pXMaiSessionCtrl, char *Name)
{
	PRINT_INFO("Name:%s\n", Name);
    if(!strcmp(Name, "SystemFunction")){
        return resSystemFunction(pXMaiSessionCtrl);
    }
    else if(!strcmp(Name, "TalkAudioFormat")){
        return resTalkAudioFormat(pXMaiSessionCtrl);
    }
    else if(!strcmp(Name, "PTZcontrol")){
        return resPTZcontrol(pXMaiSessionCtrl);
    }
    else if(!strcmp(Name, "Camera")){
        return resCamera(pXMaiSessionCtrl);
    }
    else if(!strcmp(Name, "MaxPreRecord")){
        return resMaxPreRecord(pXMaiSessionCtrl);
    }
    else if(!strcmp(Name, "MultiLanguage")){
        return resMultiLanguage(pXMaiSessionCtrl);
    }
    else if(!strcmp(Name, "MultiVstd")){
        return resMultiVstd(pXMaiSessionCtrl);
    }
    else if(!strcmp(Name, "EncodeCapability")){
        return resEncodeCapability(pXMaiSessionCtrl);
    }
    else if(!strcmp(Name, "NetOrder")){
        return resNetOrder(pXMaiSessionCtrl);
    }
    else if(!strcmp(Name, "BlindCapability")){
        return resBlindCapability(pXMaiSessionCtrl);
    }
    else if(!strcmp(Name, "MotionArea.[0]")){
        return resMotionArea(pXMaiSessionCtrl);
    }
    else
    {
		return -1;
    }

	return 0;
}

int ReqAbilityGetHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg)
{
    PRINT_INFO("<<ReqAbilityGetHandle>>  sock:%d sessionType:%d\n", pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSessionType);
    XMaiMsgHeader *pXMaiMsgHeader = (XMaiMsgHeader *)msg;
    cJSON *json = NULL;
    cJSON *element = NULL;
    char *out = NULL;
    char Name[128]={0};

    if(pXMaiMsgHeader->dataLen == 0){
        PRINT_ERR("dataLen == 0 no json\n");
        goto RES;
    }
    
	json=cJSON_Parse(msg+20);
    if (!json){
        PRINT_ERR("Error before: [%s]\n", cJSON_GetErrorPtr());
        return -1;
    }

    element = cJSON_GetObjectItem(json, "Name");
    if(element){
        sprintf(Name, "%s", element->valuestring);
        //PRINT_INFO("Name:%s\n", Name);
    }
    
    out = cJSON_Print(json);
    //PRINT_INFO("%s\n",out);
    
    cJSON_Delete(json);
    free(out);

RES:
    //回应
    return resAbilityGet(pXMaiSessionCtrl, Name);
	
}


int resTalkClaim(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root;	

    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name","OPTalk");
	if(g_xmai_is_talking == 0)
		cJSON_AddNumberToObject(root, "Ret", 100);
	else
    	cJSON_AddNumberToObject(root, "Ret", 503);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);

    out=cJSON_PrintUnformatted(root);
    PRINT_INFO("%s\n",out);

  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, TALK_CLAIM_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0;
}

int HandleTalkClaim(XMaiSessionCtrl *pXMaiSessionCtrl, cJSON *root)
{
	PRINT_INFO("SessionInt=%d\n", pXMaiSessionCtrl->fSessionInt);
    if(!root)
        return -1;
    
    cJSON *pOPTalk ,*pAction,*pParameter,*pBitRate,*pEncodeType;
	cJSON *pSampleBit, *pSampleRate;
    char Action[256] = {0};
    char EncodeType[256] = {0};
    int nBitRate = 0;
	int nSampleBit = 0, nSampleRate = 0;
    
    pOPTalk = cJSON_GetObjectItem(root, "OPTalk");
    if(!pOPTalk)
    {
    	PRINT_ERR();
        return -1;
    }
    
    pAction = cJSON_GetObjectItem(pOPTalk, "Action");
	if(pAction)
	{
    	sprintf(Action, "%s", pAction->valuestring);     
    	PRINT_INFO("Action:%s\n", Action);
	}

    pParameter = cJSON_GetObjectItem(pOPTalk, "AudioFormat");
	if(pParameter)	
	{    
	    pBitRate = cJSON_GetObjectItem(pParameter, "BitRate");
	    if(pBitRate)
	    {
	        nBitRate = pBitRate->valueint;
	    	PRINT_INFO("nBitRate:%d\n", nBitRate);
	    }

	    pEncodeType = cJSON_GetObjectItem(pParameter, "EncodeType"); //main
	    if(pEncodeType)
	    {
	        sprintf(EncodeType, "%s", pEncodeType->valuestring);
	    	PRINT_INFO("EncodeType:%s\n", EncodeType);
	    }
	  	
		pSampleBit = cJSON_GetObjectItem(pParameter, "SampleBit");
		if(pSampleBit)
		{
			nSampleBit = pSampleBit->valueint;
			PRINT_INFO("nSampleBit:%d\n", nSampleBit);
		}
				 
		pSampleRate = cJSON_GetObjectItem(pParameter, "SampleRate");
		if(pSampleRate)
		{
			nSampleRate = pSampleRate->valueint;
			PRINT_INFO("nSampleRate:%d\n", nSampleRate);
		}
	}

	/* 将当前socket添加进数组，用来做媒体流传输 */
	int ret = UtAddTalkSock(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSessionInt, &g_xmai_session);
	if(ret == -1) // 出错，找不到对应的session_id
	{
		PRINT_ERR();							
		return -1;
	}	

    return 0;
}

int ReqTalkClaimHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg)
{
	pXMaiSessionCtrl->fSessionType = CONNECT_SESSION;
    PRINT_INFO("<<ReqTalkClaimHandle>>  sock:%d sessionType:%d\n", pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSessionType);
    XMaiMsgHeader *pXMaiMsgHeader = (XMaiMsgHeader *)msg;
    cJSON *json = NULL;    
    char *out = NULL;
    //char SessionID[128]={0};
    
    if(pXMaiMsgHeader->dataLen == 0){
        PRINT_ERR("dataLen == 0 no json   must have jason\n");
        return -1;
    }
	
	json = cJSON_Parse(msg + 20);
    if (!json){
        PRINT_ERR("Error before: [%s]\n", cJSON_GetErrorPtr());
        return -1;
    }
    
    out = cJSON_Print(json);
    PRINT_INFO("%s\n",out);

	pthread_mutex_lock(&g_xmai_audio_mutex);	
    //处理
    if(g_xmai_is_talking == 0)
    {
    	HandleTalkClaim(pXMaiSessionCtrl, json);
    }
    cJSON_Delete(json);
    free(out);
    //回应
    int ret = resTalkClaim(pXMaiSessionCtrl);
	g_xmai_is_talking = 1;
	pthread_mutex_unlock(&g_xmai_audio_mutex);
	
	return ret;
}

////////////////////////////////////
int HandleTalkReqStart(XMaiSessionCtrl *pXMaiSessionCtrl)
{    
    PRINT_INFO("HandleTalkReqStart sock:%d\n", pXMaiSessionCtrl->accept_sock);

	XMaiCreateThread(XMaiTalkPthread, (void *)pXMaiSessionCtrl->fSessionInt, NULL);
    return 0;
}

int HandleTalkReqStop(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    PRINT_INFO("HandleTalkReqStop sock:%d\n", pXMaiSessionCtrl->accept_sock);

	g_xmai_talk_recv_running = 0;
	//todo

    return 0;
}

int HandleTalkReq(XMaiSessionCtrl *pXMaiSessionCtrl, cJSON *root)
{
    if(!root)
        return -1;
    
    cJSON *pOPTalk ,*pAction,*pParameter,*pBitRate,*pEncodeType;
	cJSON *pSampleBit, *pSampleRate;
    char Action[256] = {0};
    char EncodeType[256] = {0};
    int nBitRate = 0;
	int nSampleBit = 0, nSampleRate = 0;
    
    pOPTalk = cJSON_GetObjectItem(root, "OPTalk");
    if(!pOPTalk)
    {
    	PRINT_ERR();
        return -1;
    }
    
    pParameter = cJSON_GetObjectItem(pOPTalk, "AudioFormat");
    if(pParameter)
    {
	    pBitRate = cJSON_GetObjectItem(pParameter, "BitRate");
	    if(pBitRate)
	    {
	        nBitRate = pBitRate->valueint;
	    	PRINT_INFO("nBitRate:%d\n", nBitRate);
	    }

	    pEncodeType = cJSON_GetObjectItem(pParameter, "EncodeType"); //main
	    if(pEncodeType)
	    {
	        sprintf(EncodeType, "%s", pEncodeType->valuestring);
	    	PRINT_INFO("EncodeType:%s\n", EncodeType);
	    }

	  	pSampleBit = cJSON_GetObjectItem(pParameter, "SampleBit");
		if(pSampleBit)
		{
			nSampleBit = pSampleBit->valueint;
			PRINT_INFO("nSampleBit:%d\n", nSampleBit);
		}
		
		pSampleRate = cJSON_GetObjectItem(pParameter, "SampleRate");
		if(pSampleRate)
		{
			nSampleRate = pSampleRate->valueint;
			PRINT_INFO("nSampleRate:%d\n", nSampleRate);
		}
    }

    pAction = cJSON_GetObjectItem(pOPTalk, "Action");
	if(pAction)
	{
    	sprintf(Action, "%s", pAction->valuestring);     
    	PRINT_INFO("Action:%s\n", Action);
		
	    if(!strcmp(Action, "Start")){
	        HandleTalkReqStart(pXMaiSessionCtrl);
	    }
	    else if(!strcmp(Action, "Stop")){
	        HandleTalkReqStop(pXMaiSessionCtrl);
	    } 		
	}
	
    return 0;
}

int resTalkReq(XMaiSessionCtrl *pXMaiSessionCtrl)
{
    int xmaiMsgLen = 0;
    int ret = 0;
    char *out;
    cJSON *root;

    root = cJSON_CreateObject();//创建项目
    cJSON_AddStringToObject(root, "Name","OPTalk");
    cJSON_AddNumberToObject(root, "Ret", 100);
    cJSON_AddStringToObject(root, "SessionID", pXMaiSessionCtrl->fSessionId);

    out=cJSON_PrintUnformatted(root);
    //PRINT_INFO("%s\n",out);


  	//加包头
    xmaiMsgLen = XMaiMakeMsg(pXMaiSessionCtrl, TALK_RSP, out);
    free(out);
    cJSON_Delete(root);	
	if(xmaiMsgLen < 0)
	{
        PRINT_ERR();
		return -1;		
	}

	//加发送
    ret = XMaiSockSend(pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSendbuf, xmaiMsgLen);
    if(ret < 0)
	{
        PRINT_ERR();
		return -1;	
    }
        
    return 0;
}

int ReqTalkReqHandle(XMaiSessionCtrl *pXMaiSessionCtrl, char* msg)
{
    PRINT_INFO("<<ReqTalkReqHandle>>  sock:%d sessionType:%d\n", pXMaiSessionCtrl->accept_sock, pXMaiSessionCtrl->fSessionType);
    XMaiMsgHeader *pXMaiMsgHeader = (XMaiMsgHeader *)msg;
    cJSON *json = NULL;    
    char *out = NULL;
    
    if(pXMaiMsgHeader->dataLen == 0){
        PRINT_ERR("dataLen == 0 no json   must have jason\n");
        return -1;
    }
	
	json = cJSON_Parse(msg + 20);
    if (!json){
        PRINT_ERR("Error before: [%s]\n", cJSON_GetErrorPtr());
        return -1;
    }
    
    out = cJSON_Print(json);
    //PRINT_INFO("%s\n",out);


    //处理
    HandleTalkReq(pXMaiSessionCtrl, json);

    cJSON_Delete(json);
    free(out);
	
    //回应
    return resTalkReq(pXMaiSessionCtrl);	
}


