/*!
*****************************************************************************
** FileName     : ds_search.c
**
** Description  : search for ds.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create Date  : 2016-9-29
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#include "ds_search.h"
#include "netcam_api.h"
#include "utility_api.h"
#include "gk_cms_utility.h"
#include "cfg_all.h"
#include "xml.h"
#include "sdk_cfg.h"


static int mtws_search_running = 0;

static int mtws_search_set_resp(char *resq, int len)
{
    /* 设备类型，序列号 */
    char device_type[100] = {0};
    char device_module[100] = {0};
    char device_serial[32] = {0};
    char device_p2pid[32] = {0};
    //strcpy(device_type, "NVS-IPCAM-GK_TYJ");
    sprintf(device_type, "NVS-GK_%s", sdk_cfg.name);
    strcpy(device_module, "HN");
    strncpy(device_serial, runSystemCfg.deviceInfo.deviceType, sizeof(runSystemCfg.deviceInfo.deviceType));
    strncpy(device_p2pid, runSystemCfg.deviceInfo.serialNumber, sizeof(runSystemCfg.deviceInfo.serialNumber));
    //strcpy(device_p2pid, "HY5W94123ELJB287111A");


    /* 网络地址信息 */
    char m_mac_addr[20] = {0};
    char m_ip_addr[16] = {0};
    char m_net_mask[16] = {0};
    char m_gate_way[16] = {0};
    char m_dns1[16] = {0};
    char m_dns2[16] = {0};
    
    ST_SDK_NETWORK_ATTR net_attr;
    memset(&net_attr, 0, sizeof(ST_SDK_NETWORK_ATTR));
    int ret = netcam_net_get_detect(runNetworkCfg.lan.netName);
    if (ret != 0) {
        PRINT_INFO("eth0 is not running.");
        #if 0
        strcpy(net_attr.name, runNetworkCfg.wifi.netName);
        if(netcam_net_get(&net_attr) != 0) {
            PRINT_ERR("get network config error.\n");
            return -1;
        }
        PRINT_INFO("bc -- wifi ip :%s & static ip :%s\n", net_attr.ip, runNetworkCfg.wifi.ip);
        strncpy(msg_data.jbServerPack.jspack.szIp, (char *)net_attr.ip, DMS_MAX_IP_LENGTH);
        #else
        strncpy(m_mac_addr, runNetworkCfg.wifi.mac, sizeof(runNetworkCfg.wifi.mac));
        strncpy(m_ip_addr, runNetworkCfg.wifi.ip, sizeof(runNetworkCfg.wifi.ip));
        strncpy(m_net_mask, runNetworkCfg.wifi.netmask, sizeof(runNetworkCfg.wifi.netmask));
        strncpy(m_gate_way, runNetworkCfg.wifi.gateway, sizeof(runNetworkCfg.wifi.gateway));
        strncpy(m_dns1, runNetworkCfg.wifi.dns1, sizeof(runNetworkCfg.wifi.dns1));
        strncpy(m_dns2, runNetworkCfg.wifi.dns1, sizeof(runNetworkCfg.wifi.dns2));
        #endif
    } else {
        PRINT_INFO("eth0 is ok.");
        #if 0
        strcpy(net_attr.name, runNetworkCfg.lan.netName);
        if(netcam_net_get(&net_attr) != 0) {
        PRINT_ERR("get network config error.\n");
        return -1;
        }
        PRINT_INFO("bc -- lan ip :%s & static ip :%s\n", net_attr.ip, runNetworkCfg.lan.ip);
        strncpy(msg_data.jbServerPack.jspack.szIp, (char *)net_attr.ip, DMS_MAX_IP_LENGTH);
        #else
        strncpy(m_mac_addr, runNetworkCfg.lan.mac, sizeof(runNetworkCfg.lan.mac));
        strncpy(m_ip_addr, runNetworkCfg.lan.ip, sizeof(runNetworkCfg.lan.ip));
        strncpy(m_net_mask, runNetworkCfg.lan.netmask, sizeof(runNetworkCfg.lan.netmask));
        strncpy(m_gate_way, runNetworkCfg.lan.gateway, sizeof(runNetworkCfg.lan.gateway));
        strncpy(m_dns1, runNetworkCfg.lan.dns1, sizeof(runNetworkCfg.lan.dns1));
        strncpy(m_dns2, runNetworkCfg.lan.dns1, sizeof(runNetworkCfg.lan.dns2));
        #endif
    }
    PRINT_INFO("mac:%s, ip:%s mask:%s gateway:%s dns1:%s dns2:%s\n", 
        m_mac_addr, m_ip_addr, m_net_mask, m_gate_way, m_dns1, m_dns2);

    /* 用户名密码， 端口号 */
    char m_loginUser[64] = {0};
    char m_loginPwd[64] = {0};
    //strcpy(m_loginUser, "admin");
    //strcpy(m_loginPwd, "123456");
    PRINT_INFO("user:%s, psd:%s\n", runUserCfg.user[0].userName, runUserCfg.user[0].password);
    strncpy(m_loginUser, runUserCfg.user[0].userName, sizeof(runUserCfg.user[0].userName));
    strncpy(m_loginPwd, runUserCfg.user[0].password, sizeof(runUserCfg.user[0].password));
    int m_videoPort = 554;
    int m_ptzPort = 8091;

    /* 组包 */
    snprintf(resq, len,
    "<?xml version=\"1.0\" encoding=\"GB2312\" ?>\n"
    "<XML_TOPSEE>\n"
    "<MESSAGE_HEADER\n"
    "Msg_type=\"SYSTEM_SEARCHIPC_MESSAGE\"\n"
    "Msg_code=\"3\"\n"
    "Msg_flag=\"0\"\n"
    "/>\n"
    "<MESSAGE_BODY>\n"
    "<DEVICE_TYPE\n"
    "DeviceType=\"%s\"\n"
    "DeviceModule=\"%s\"\n"
    "/>\n"
    "<IPC_SERIALNUMBER\n"
    "SerialNumber=\"%s\"  PLATFORMID=\"\" P2PID=\"%s\"\n"
    "/>\n"
    "<LANConfig\n"
    "MacAddress=\"%s\"\n"
    "DHCP=\"0\"\n"
    "IPAddress=\"%s\"\n"
    "Netmask=\"%s\"\n"
    "Gateway=\"%s\"\n"
    "DNS1=\"%s\"\n"
    "DNS2=\"%s\"\n"
    "/>\n"
    "<UserConfig>\n"
    "<Account\n"
    "Username=\"%s\"\n"
    "Password=\"%s\"\n"
    "Group=\"Administrator\"\n"
    "Status=\"Enable\"\n"
    "/>\n"
    "</UserConfig>\n"
    "<MediaStreamConfig>\n"
    "<StreamAccess\n"
    "Auth=\"1\"\n"
    "VideoPort=\"%d\"\n"
    "RTPOverRTSP=\"1\"\n"
    "PTZPort=\"%d\"\n"
    "WEBPort=\"80\"\n"
    "/>\n"
    "</MediaStreamConfig>\n"
    "</MESSAGE_BODY>\n"
    "</XML_TOPSEE>\n", device_type, device_module, device_serial, device_p2pid,
    m_mac_addr, m_ip_addr, m_net_mask, m_gate_way, m_dns1, m_dns2,
    m_loginUser, m_loginPwd, m_videoPort, m_ptzPort);

    return 0;
}

int mtws_bc_send_to(int fSockSearchfd, char *multi_group, int send_port, const char* inData, const int inLength)
{
	int ret = 0;
	int sendlen = 0;
    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    //addr.sin_addr.s_addr = inet_addr("255.255.255.255");
    addr.sin_addr.s_addr = inet_addr(multi_group);
    //addr.sin_addr.s_addr = htonl(INADDR_BROADCAST); 
    addr.sin_port = htons(send_port);

	//因为UDP 发送时，没有缓冲区，故需要对发送后的返回值进行判断后，多次发送
	while(sendlen < inLength) {
	    ret = sendto(fSockSearchfd, inData + sendlen, inLength - sendlen, 0, (struct sockaddr*)&addr,  sizeof(addr));
		if (ret < 0) {
			//perror("Send error");
			//非阻塞才有 EAGAIN
			if (errno != EINTR && errno != EAGAIN ) {
				PRINT_ERR("Send() socket %d error :%s\n", fSockSearchfd, strerror(errno));
				return -1;
			}
			else
				continue;
		}

		sendlen += ret;
	}

	PRINT_INFO("ip search SendLen %s: %d, ret:%s\n", multi_group, sendlen, strerror(errno));

	return sendlen;
}


static int mtws_bc_handle_msg(int send_sock, int send_port, char* pBuf, int buf_len)
{
    PRINT_INFO("mtws_bc_handle_msg.\n");

    char msgType[100] = {0};
    char msgCode[32] = {0};
    char msgFlag[32] = {0};

    XMLN *rootNode = xml_stream_parse(pBuf, buf_len);
    if (!rootNode) {
        PRINT_ERR("get rootNode error.");
        return -1;
    }
    
	XMLN *header_node = xml_node_get_child(rootNode, "MESSAGE_HEADER");
    if (!header_node) {
        PRINT_ERR("Unable to read MESSAGE_HEADER\n");
        return -1;
    }
    
    strcpy(msgType, xml_attr_get_data(header_node, "Msg_type"));
    strcpy(msgCode, xml_attr_get_data(header_node, "Msg_code"));
    strcpy(msgFlag, xml_attr_get_data(header_node, "Msg_flag"));

    xml_node_del(header_node);
    xml_node_del(rootNode);

	PRINT_INFO("msgType %s msgCode %s msgFlag %s \n", msgType, msgCode, msgFlag);

    if(strcmp(msgType, "SYSTEM_SEARCHIPC_MESSAGE") == 0)
	{
        PRINT_INFO("bc get search.");
        char resp[4096] = {0};
        memset(resp, 0x0, sizeof(resp));
        mtws_search_set_resp(resp, sizeof(resp));
        printf("len:%d resp:%s\n", strlen(resp), resp);
        mtws_bc_send_to(send_sock, MTWS_SEARCH_DEST_ADDR, send_port, resp, strlen(resp));
    }

    return 0;
}

int mtws_search_loop(int fSockSearchfd)
{
    int recvLen = 0;
    char bufRecv[4096] = {0};
    // 广播地址
    struct sockaddr_in from;

    int len = 0;
    struct timeval timeout;
    fd_set readfd; //读文件描述符集合
    int ret = 0;
    while (1) {
        if (0 == mtws_search_running) {
            PRINT_INFO("bc, g_cms_pthread_run is 0\n");
            break;
        }

        timeout.tv_sec = 5;  //超时时间为5秒
        timeout.tv_usec = 0;
        //文件描述符清0
        FD_ZERO(&readfd);
        //将套接字文件描述符加入到文件描述符集合中
        FD_SET(fSockSearchfd, &readfd);

        ret = select(fSockSearchfd + 1, &readfd, NULL, NULL, &timeout);
        if (ret < 0) {
            PRINT_ERR("sock:%d select send error:%s\n", fSockSearchfd, strerror(errno));
            break;
        }
        if(0 == ret) {
            //PRINT_INFO("sock:%d select send timeout error:%s\n", fSockSearchfd, strerror(errno));
            continue;
        }

        if (FD_ISSET(fSockSearchfd, &readfd) == 0) {
			PRINT_ERR("cms select set error\n");
            continue;
        }
        
    	// 初始化置0
    	memset(bufRecv, 0, sizeof(bufRecv));
    	bzero(&from, sizeof(struct sockaddr_in));

    	//接收发来的消息
    	len = sizeof(struct sockaddr_in);
    	//PRINT_INFO("Begin receive serarch data\n");
    	recvLen = recvfrom(fSockSearchfd, bufRecv, sizeof(bufRecv)-1, 0, (struct sockaddr *)&from, (socklen_t *)&len);
    	if (recvLen < 0) {
    	    PRINT_ERR("recvfrom error:%s\n", strerror(errno));
    	    continue;
    	}
    	//打印接收到的地址信息
    	PRINT_INFO("search from ip:%s, port:%d ", (char *)inet_ntoa(from.sin_addr), ntohs(from.sin_port));
        mtws_bc_handle_msg(fSockSearchfd, MTWS_SEARCH_DEST_PORT, bufRecv, strlen(bufRecv));
    }
    
    PRINT_INFO("mtws_search_loop end.\n");
    return 0;
}


static int mtws_search_func()
{
    int fSockSearchfd = CreateBroadcastSock(MTWS_SEARCH_SRC_PORT);
    if (fSockSearchfd < 0) {
        PRINT_ERR("create ipcsearch_thread_socket error.\n");
        return -1;
    }

    mtws_search_loop(fSockSearchfd);

	if (fSockSearchfd > 0) {
		close(fSockSearchfd);
	}

	PRINT_INFO("mtws_search_func end\n");
	return 0;
}

static void * mtws_search_thread(void *para)
{
    SetThreadName("mtws_search_thread");

    mtws_search_func();

    PRINT_INFO("mtws_search_thread end !\n");
    return NULL;
}


int ds_search_start()
{
    mtws_search_running = 1;
    CreateDetachThread(mtws_search_thread, NULL, NULL);

    return 0;
}


void ds_search_exit()
{
    mtws_search_running = 0;
    return;
}

