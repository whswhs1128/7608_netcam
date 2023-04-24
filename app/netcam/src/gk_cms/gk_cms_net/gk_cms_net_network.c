#include "ctype.h"

#include "gk_cms_common.h"
#include "gk_cms_media.h"
#include "gk_cms_sock.h"
#include "gk_cms_protocol.h"
#include "gk_cms_utility.h"
#include "gk_cms_net_api.h"
#include "cfg_all.h"
#include "netcam_api.h"
#include "ipcsearch.h"

int serverKeyRet = -1;

int get_hw_array(unsigned char *hwaddr, unsigned char *addr)
{
	unsigned char *paddr = addr;
	unsigned char str[4];
	int number;
	int i;

	if(!addr || !hwaddr)
	{
		PRINT_INFO("bad parameter");
		return -1;
	}

	i = 0;
	while(*paddr && (i < strlen((char *)addr)) && i < 6)
	{
		memset(str,0,sizeof(str));
		if(':' != *paddr)
		{
			str[0] = toupper(*paddr);
			if(str[0]>='A' && str[0] <='Z')
			{
			  number = 0xf0&((str[0]-'A'+10)<<4);
			}
			else if(str[0]>='0' && str[0] <='9')
			{
			  number = 0xf0&((str[0]-'0')<<4);
			}
			else
			{
				PRINT_ERR("hwaddr content error");
				return -1;
			}

			paddr ++;
			if(':' != *paddr)
			{
				str[0] = toupper(*paddr);
				paddr ++;

				if(str[0]>='A' && str[0] <='Z')
				{
					number |= ((str[0]-'A'+10)&0x0f);
				}
				else if(str[0]>='0' && str[0] <='9')
				{
					number |= ((str[0]-'0')&0x0f);
				}
				else
				{
					PRINT_ERR("hwaddr content error");
					return -1;
				}
			}

			hwaddr[i] = number;
			i ++;
		}
		else
		  paddr ++;
	}


	return 0;
}

int Gk_CmsGetFtp(int sock)
{
    DMS_NET_FTP_PARAM ftp;
	memset(&ftp,0,sizeof(DMS_NET_FTP_PARAM));
	//Get
	ftp.dwSize = sizeof(DMS_NET_FTP_PARAM);
    ftp.bEnableFTP = runNetworkCfg.ftp.enableFTP;
    strncpy(ftp.csFTPIpAddress, runNetworkCfg.ftp.address,DMS_MAX_DOMAIN_NAME-1);
	ftp.dwFTPPort = runNetworkCfg.ftp.port;
    strncpy(ftp.csUserName, runNetworkCfg.ftp.userName, DMS_NAME_LEN-1);
    strncpy(ftp.csPassword, runNetworkCfg.ftp.password, DMS_PASSWD_LEN-1);
    ftp.wTopDirMode = 0x0000;
    ftp.wSubDirMode = 0x0000;

    GkCmsCmdResq(sock, (char *)&ftp, sizeof(DMS_NET_FTP_PARAM), DMS_NET_GET_FTPCFG);
    return 0;
}

int Gk_CmsSetFtp(int sock)
{
    DMS_NET_FTP_PARAM ftp;
    int ret = GkSockRecv(sock, (char *)&ftp, sizeof(DMS_NET_FTP_PARAM));
    if(ret != sizeof(DMS_NET_FTP_PARAM))
    {
        PRINT_ERR();
        return -1;
    }
    runNetworkCfg.ftp.enableFTP = ftp.bEnableFTP;
    strncpy(runNetworkCfg.ftp.address, ftp.csFTPIpAddress, MAX_STR_LEN_128-1);
	runNetworkCfg.ftp.port = ftp.dwFTPPort;
    strncpy(runNetworkCfg.ftp.userName, ftp.csUserName, MAX_STR_LEN_64-1);
    strncpy(runNetworkCfg.ftp.password, ftp.csPassword, MAX_STR_LEN_64-1);
    //ftp.wTopDirMode = 0x0000;
    //ftp.wSubDirMode = 0x0000;

	//PRINT_INFO("\n%d\n%s\n%d\n%s\n%s\n%x\n%x\n",
		//(int)ftp.bEnableFTP, ftp.csFTPIpAddress, (int)ftp.dwFTPPort,
		//ftp.csUserName, ftp.csPassword,
		//ftp.wTopDirMode, ftp.wSubDirMode);

    return 0;
}

int Gk_CmsTestFtp(int sock)
{
    int ret = 0;
    DMS_NET_FTP_PARAM ftp;

    ret = GkSockRecv(sock, (char *)&ftp, sizeof(DMS_NET_FTP_PARAM));
    if(ret != sizeof(DMS_NET_FTP_PARAM))
    {
        PRINT_ERR();
        return -1;
    }

    #ifdef MODULE_SUPPORT_FTP
    ret = netcam_ftp_check(ftp.csFTPIpAddress, ftp.dwFTPPort, ftp.csUserName, ftp.csPassword);
    if (0 == ret)
        ftp.byTestStatus = 0;
    else
        ftp.byTestStatus = 1;
    #else
    ftp.byTestStatus = 0;
    #endif
    PRINT_INFO(" ftp.byTestStatus:%d\n",   ftp.byTestStatus);
    PRINT_INFO(" ftp.csFTPIpAddress:%s\n", ftp.csFTPIpAddress);
    PRINT_INFO(" ftp.dwFTPPort:%d\n",      (int)ftp.dwFTPPort);
    PRINT_INFO(" ftp.csUserName:%s\n",     ftp.csUserName);
    PRINT_INFO(" ftp.csPassword:%s\n",     ftp.csPassword);

    GkCmsCmdResq(sock, (char *)&ftp, sizeof(DMS_NET_FTP_PARAM), DMS_NET_TEST_FTP_CONNECT);

    return 0;
}


int Gk_CmsGetLan(int sock)
{
    DMS_NET_NETWORK_CFG lan;
    ST_SDK_NETWORK_ATTR net_attr;


    memset(&lan,0,sizeof(DMS_NET_NETWORK_CFG));

    sprintf(net_attr.name,"eth0");
    //to do
    if(netcam_net_get(&net_attr) == 0)
    {
		lan.dwSize = sizeof(DMS_NET_NETWORK_CFG);
        lan.byEnableDHCP = net_attr.dhcp;
        lan.byMaxConnect = 1;
        lan.dwSize = sizeof(DMS_NET_NETWORK_CFG);
        lan.wHttpPort = net_attr.httpPort;


        strncpy(lan.stDnsServer1IpAddr.csIpV4,net_attr.dns1,DMS_MAX_IP_LENGTH);
        strncpy(lan.stDnsServer2IpAddr.csIpV4,net_attr.dns2,DMS_MAX_IP_LENGTH);

        strncpy((char*)lan.stGatewayIpAddr.csIpV4,(char *)net_attr.gateway,DMS_MAX_IP_LENGTH);
//        strncpy((char*)lan.stMulticastIpAddr.csIpV4,(char *)net_attr.gateway,DMS_MAX_IP_LENGTH);

        strncpy(lan.stMulticastIpAddr.csIpV4, GK_CMS_MC_GROUP_IP, DMS_MAX_IP_LENGTH);

		get_hw_array(lan.stEtherNet[0].byMACAddr,net_attr.mac);

        strncpy((char*)lan.stEtherNet[0].csNetName,(char *)net_attr.name,DMS_MAX_IP_LENGTH);
        strncpy((char*)lan.stEtherNet[0].strIPAddr.csIpV4,(char *)net_attr.ip,DMS_MAX_IP_LENGTH);
        strncpy((char*)lan.stEtherNet[0].strIPMask.csIpV4,(char *)net_attr.mask,DMS_MAX_IP_LENGTH);
		lan.stEtherNet[0].wMTU = 1500;
		lan.stEtherNet[0].dwNetInterface = 4;

		GkCmsCmdResq(sock, &lan, sizeof(DMS_NET_NETWORK_CFG), DMS_NET_GET_NETCFG);
    }
    else
    {
        PRINT_ERR("read eth0 error");
    }
    return 0;
}

int Gk_CmsSetLan(int sock)
{
    DMS_NET_NETWORK_CFG lan;
	ST_SDK_NETWORK_ATTR net_attr;

    int ret = GkSockRecv(sock, (char *)&lan, sizeof(DMS_NET_NETWORK_CFG));
    if (ret != sizeof(DMS_NET_NETWORK_CFG)) {
        PRINT_ERR();
        return -1;
    }

	sprintf(net_attr.name,"eth0");

	netcam_net_get(&net_attr) ;
	{
		net_attr.dhcp = lan.byEnableDHCP;

		//printf("mask:%s\n",net_attr.mask);

		strncpy(net_attr.dns1,lan.stDnsServer1IpAddr.csIpV4,SDK_DNSSTR_LEN);
		//printf("dns1:%s\n",lan.stDnsServer1IpAddr.csIpV4);
		strncpy(net_attr.dns2,lan.stDnsServer2IpAddr.csIpV4,SDK_DNSSTR_LEN);

		//printf("dns2:%s\n",lan.stDnsServer2IpAddr.csIpV4);

		strncpy(net_attr.gateway,lan.stGatewayIpAddr.csIpV4,SDK_GATEWAYSTR_LEN);

		//printf("gateway:%s\n",lan.stGatewayIpAddr.csIpV4);

		//strncpy((char *)net_attr.mac,(char *)lan.stEtherNet[0].byMACAddr,SDK_MACSTR_LEN);

		strncpy(net_attr.ip,lan.stEtherNet[0].strIPAddr.csIpV4,SDK_IPSTR_LEN);

		//printf("ip:%s\n",lan.stEtherNet[0].strIPAddr.csIpV4);
		//printf("mask:%s\n",net_attr.mask);

		if(netcam_net_set(&net_attr) != 0)
		{
			PRINT_ERR("set eth0 parameter error");
		}
    	//todo
	}

    return 0;
}

int Gk_CmsGetDefLan(int sock)
{
    DMS_NET_NETWORK_CFG lan;
	ST_SDK_NETWORK_ATTR net_attr;

	memset(&lan,0,sizeof(DMS_NET_NETWORK_CFG));
	lan.dwSize = sizeof(DMS_NET_NETWORK_CFG);
	sprintf(net_attr.name,"eth0");
	//to do
	if(netcam_net_get(&net_attr) == 0)
	{
	  lan.byEnableDHCP = 1;
	  lan.byMaxConnect = 1;
	  lan.wHttpPort = 80;
	  strncpy(lan.stDnsServer1IpAddr.csIpV4,net_attr.dns1,DMS_MAX_IP_LENGTH);
	  strncpy(lan.stDnsServer2IpAddr.csIpV4,net_attr.dns2,DMS_MAX_IP_LENGTH);

	  strncpy((char *)lan.stEtherNet[0].byMACAddr ,(char *)net_attr.mac,DMS_MAX_IP_LENGTH);
	  strncpy(lan.stEtherNet[0].csNetName,net_attr.name,DMS_MAX_IP_LENGTH);
	  strncpy(lan.stEtherNet[0].strIPAddr.csIpV4,net_attr.ip,DMS_MAX_IP_LENGTH);
	  strncpy(lan.stEtherNet[0].strIPMask.csIpV4,net_attr.mask,DMS_MAX_IP_LENGTH);
	  lan.stEtherNet[0].wMTU = 1500;
	  lan.stEtherNet[0].dwNetInterface = 4;
	}

    //to do
	GkCmsCmdResq(sock, &lan, sizeof(DMS_NET_NETWORK_CFG), DMS_NET_GET_DEF_NETCFG);

    //GkCmsCmdResq(sock, (char *)&lan, sizeof(DMS_NET_NETWORK_CFG), 100);
    return 0;
}

int Gk_CmsGetRtsp(int sock)
{
    DMS_NET_RTSP_CFG rtsp;

	PRINT_INFO();

    //to do
	memset(&rtsp,0,sizeof(DMS_NET_RTSP_CFG));
	rtsp.dwPort = 554;
	rtsp.dwSize = sizeof(DMS_NET_RTSP_CFG);

	GkCmsCmdResq(sock, &rtsp, sizeof(DMS_NET_RTSP_CFG), DMS_NET_GET_RTSPCFG);
    return 0;
}

int Gk_CmsSetRtsp(int sock)
{
    DMS_NET_RTSP_CFG rtsp;
    int ret = GkSockRecv(sock, (char *)&rtsp, sizeof(DMS_NET_RTSP_CFG));
    if (ret != sizeof(DMS_NET_RTSP_CFG)) {
        PRINT_ERR();
        return -1;
    }

    //todo
	PRINT_INFO();

    return 0;
}


int Gk_CmsGetEmail(int sock)
{
    DMS_NET_EMAIL_PARAM email;
	memset(&email,0,sizeof(DMS_NET_EMAIL_PARAM));
	//Get
	email.dwSize = sizeof(DMS_NET_EMAIL_PARAM);
	//PRINT_INFO("runNetworkCfg.email.attachPicture: %d\n", runNetworkCfg.email.attachPicture);
	email.bEnableEmail 		 = runNetworkCfg.email.enableEmail;
    email.byAttachPicture 	 = runNetworkCfg.email.attachPicture;
    email.bySmtpServerVerify = 0;
    email.byMailInterval 	 = 2;
    email.wEncryptionType	 = runNetworkCfg.email.encryptionType;

    strncpy(email.csEMailUser, runNetworkCfg.email.eMailUser, DMS_NAME_LEN-1);
    strncpy(email.csEmailPass, runNetworkCfg.email.eMailPass, DMS_PASSWD_LEN-1);

	//PRINT_INFO("runNetworkCfg.email.smtpServer: %s\n", runNetworkCfg.email.smtpServer);
    strncpy(email.csSmtpServer, runNetworkCfg.email.smtpServer, DMS_MAX_DOMAIN_NAME-1);
    email.wServicePort = runNetworkCfg.email.smtpPort;
    strncpy(email.csPop3Server, runNetworkCfg.email.pop3Server, DMS_MAX_DOMAIN_NAME-1);

	strncpy(email.stSendAddrList.csAddress, runNetworkCfg.email.fromAddr, DMS_MAX_DOMAIN_NAME-1);
#if 1
	strncpy(email.stToAddrList[0].csAddress, runNetworkCfg.email.toAddrList0, DMS_MAX_DOMAIN_NAME-1);
	strncpy(email.stCcAddrList[0].csAddress, runNetworkCfg.email.ccAddrList0, DMS_MAX_DOMAIN_NAME-1);
	strncpy(email.stBccAddrList[0].csAddress, runNetworkCfg.email.bccAddrList0, DMS_MAX_DOMAIN_NAME-1);
#else	// if cms set servaral email address like hw@huawei.com; apple@apple.com acrording to '";"
	int offset = 0;
	if(runNetworkCfg.email.toAddrList0[0] != '\0')
		offset += snprintf(email.stToAddrList[0].csAddress+offset, DMS_MAX_DOMAIN_NAME-offset, "%s;", runNetworkCfg.email.toAddrList0);
	if(runNetworkCfg.email.toAddrList1[0] != '\0')
		offset += snprintf(email.stToAddrList[0].csAddress+offset, DMS_MAX_DOMAIN_NAME-offset, "%s;", runNetworkCfg.email.toAddrList1);
	if(runNetworkCfg.email.toAddrList2[0] != '\0')
		offset += snprintf(email.stToAddrList[0].csAddress+offset, DMS_MAX_DOMAIN_NAME-offset, "%s;", runNetworkCfg.email.toAddrList2);
	if(runNetworkCfg.email.toAddrList3[0] != '\0')
		offset += snprintf(email.stToAddrList[0].csAddress+offset, DMS_MAX_DOMAIN_NAME-offset, "%s;", runNetworkCfg.email.toAddrList3);
	//PRINT_ERR("email.stToAddrList[0].csAddress: %s\n", email.stToAddrList[0].csAddress);
	offset = 0;
	if(runNetworkCfg.email.ccAddrList0[0] != '\0')
		offset += snprintf(email.stCcAddrList[0].csAddress+offset, DMS_MAX_DOMAIN_NAME-offset, "%s;", runNetworkCfg.email.ccAddrList0);
	if(runNetworkCfg.email.ccAddrList1[0] != '\0')
		offset += snprintf(email.stCcAddrList[0].csAddress+offset, DMS_MAX_DOMAIN_NAME-offset, "%s;", runNetworkCfg.email.ccAddrList1);
	if(runNetworkCfg.email.ccAddrList2[0] != '\0')
		offset += snprintf(email.stCcAddrList[0].csAddress+offset, DMS_MAX_DOMAIN_NAME-offset, "%s;", runNetworkCfg.email.ccAddrList2);
	if(runNetworkCfg.email.ccAddrList3[0] != '\0')
		offset += snprintf(email.stCcAddrList[0].csAddress+offset, DMS_MAX_DOMAIN_NAME-offset, "%s;", runNetworkCfg.email.ccAddrList3);
	//PRINT_ERR("email.stCcAddrList[0].csAddress: %s\n", email.stToAddrList[0].csAddress);
	offset = 0;
	if(runNetworkCfg.email.ccAddrList0[0] != '\0')
		offset += snprintf(email.stBccAddrList[0].csAddress+offset, DMS_MAX_DOMAIN_NAME-offset, "%s;", runNetworkCfg.email.bccAddrList0);
	if(runNetworkCfg.email.bccAddrList1[0] != '\0')
		offset += snprintf(email.stBccAddrList[0].csAddress+offset, DMS_MAX_DOMAIN_NAME-offset, "%s;", runNetworkCfg.email.bccAddrList1);
	if(runNetworkCfg.email.bccAddrList2[0] != '\0')
		offset += snprintf(email.stBccAddrList[0].csAddress+offset, DMS_MAX_DOMAIN_NAME-offset, "%s;", runNetworkCfg.email.bccAddrList2);
	if(runNetworkCfg.email.bccAddrList3[0] != '\0')
		offset += snprintf(email.stBccAddrList[0].csAddress+offset, DMS_MAX_DOMAIN_NAME-offset, "%s;", runNetworkCfg.email.bccAddrList3);
	//PRINT_ERR("email.stBccAddrList[0].csAddress: %s\n", email.stBccAddrList[0].csAddress);
#endif

    GkCmsCmdResq(sock, (char *)&email, sizeof(DMS_NET_EMAIL_PARAM), DMS_NET_GET_EMAILCFG);
    return 0;
}

int Gk_CmsSetEmail(int sock)
{
    DMS_NET_EMAIL_PARAM email;
    int ret = GkSockRecv(sock, (char *)&email, sizeof(DMS_NET_EMAIL_PARAM));
    if(ret != sizeof(DMS_NET_EMAIL_PARAM))
    {
        PRINT_ERR();
        return -1;
    }
	runNetworkCfg.email.enableEmail = email.bEnableEmail;

	//PRINT_INFO("runNetworkCfg.email.enableEmail: %d\n", runNetworkCfg.email.enableEmail);
    runNetworkCfg.email.encryptionType = email.wEncryptionType;
    runNetworkCfg.email.attachPicture  = email.byAttachPicture;

    strncpy(runNetworkCfg.email.eMailUser, email.csEMailUser, MAX_STR_LEN_64-1);
    strncpy(runNetworkCfg.email.eMailPass, email.csEmailPass, MAX_STR_LEN_64-1);

    strncpy(runNetworkCfg.email.smtpServer, email.csSmtpServer, MAX_STR_LEN_128-1);
	//PRINT_INFO("runNetworkCfg.email.smtpServer: %s\n", runNetworkCfg.email.smtpServer);
    runNetworkCfg.email.smtpPort = email.wServicePort;
    strncpy(runNetworkCfg.email.pop3Server, email.csPop3Server, MAX_STR_LEN_128-1);
	strncpy(runNetworkCfg.email.fromAddr, email.stSendAddrList.csAddress, MAX_STR_LEN_64-1);
#if 0
	int i = 0;
	for(i = 0; i < 3; i++)
		PRINT_INFO("email.stToAddrList[%d].csAddress: %s", i, email.stToAddrList[i].csAddress);
	for(i = 0; i < 3; i++)
		PRINT_INFO("email.stCcAddrList[%d].csAddress: %s", i, email.stCcAddrList[i].csAddress);
	for(i = 0; i < 3; i++)
		PRINT_INFO("email.stBccAddrList[%d].csAddress: %s", i, email.stBccAddrList[i].csAddress);
#endif
#if 1
	strncpy(runNetworkCfg.email.toAddrList0, email.stToAddrList[0].csAddress, MAX_STR_LEN_64-1);
	strncpy(runNetworkCfg.email.ccAddrList0, email.stCcAddrList[0].csAddress, MAX_STR_LEN_64-1);
	strncpy(runNetworkCfg.email.bccAddrList0, email.stBccAddrList[0].csAddress, MAX_STR_LEN_64-1);
#else	// if cms set servaral email address like hw@huawei.com; apple@apple.com acrording to '";"
	char *separate = NULL;
	memset(runNetworkCfg.email.toAddrList0, 0, sizeof(runNetworkCfg.email.toAddrList0));
	memset(runNetworkCfg.email.toAddrList1, 0, sizeof(runNetworkCfg.email.toAddrList1));
	memset(runNetworkCfg.email.toAddrList2, 0, sizeof(runNetworkCfg.email.toAddrList2));
	memset(runNetworkCfg.email.toAddrList3, 0, sizeof(runNetworkCfg.email.toAddrList3));
	if(email.stToAddrList[0].csAddress[0] != '\0')
	{
		separate = strtok(email.stToAddrList[0].csAddress, ";");
		if(separate == NULL)
		{
			strncpy(runNetworkCfg.email.toAddrList0, email.stToAddrList[0].csAddress, MAX_STR_LEN_64-1);
		}
		else
		{
			strncpy(runNetworkCfg.email.toAddrList0, separate, MAX_STR_LEN_64-1);
			separate = strtok(NULL, ";");
			if(separate != NULL)
			{
				strncpy(runNetworkCfg.email.toAddrList1, separate, MAX_STR_LEN_64-1);
				separate = strtok(NULL, ";");
				if(separate != NULL)
				{
					strncpy(runNetworkCfg.email.toAddrList2, separate, MAX_STR_LEN_64-1);
					separate = strtok(NULL, ";");
					if(separate != NULL)
					{
						strncpy(runNetworkCfg.email.toAddrList3, separate, MAX_STR_LEN_64-1);
					}
				}
			}
		}
	}
	memset(runNetworkCfg.email.ccAddrList0, 0, sizeof(runNetworkCfg.email.ccAddrList0));
	memset(runNetworkCfg.email.ccAddrList1, 0, sizeof(runNetworkCfg.email.ccAddrList1));
	memset(runNetworkCfg.email.ccAddrList2, 0, sizeof(runNetworkCfg.email.ccAddrList2));
	memset(runNetworkCfg.email.ccAddrList3, 0, sizeof(runNetworkCfg.email.ccAddrList3));
	if(email.stCcAddrList[0].csAddress[0] != '\0')
	{
		separate = strtok(email.stCcAddrList[0].csAddress, ";");
		if(separate == NULL)
		{
			strncpy(runNetworkCfg.email.ccAddrList0, email.stCcAddrList[0].csAddress, MAX_STR_LEN_64-1);
		}
		else
		{
			strncpy(runNetworkCfg.email.ccAddrList0, separate, MAX_STR_LEN_64-1);
			separate = strtok(NULL, ";");
			if(separate != NULL)
			{
				strncpy(runNetworkCfg.email.ccAddrList1, separate, MAX_STR_LEN_64-1);
				separate = strtok(NULL, ";");
				if(separate != NULL)
				{
					strncpy(runNetworkCfg.email.ccAddrList2, separate, MAX_STR_LEN_64-1);
					separate = strtok(NULL, ";");
					if(separate != NULL)
					{
						strncpy(runNetworkCfg.email.ccAddrList3, separate, MAX_STR_LEN_64-1);
					}
				}
			}
		}
	}
	memset(runNetworkCfg.email.bccAddrList0, 0, sizeof(runNetworkCfg.email.bccAddrList0));
	memset(runNetworkCfg.email.bccAddrList1, 0, sizeof(runNetworkCfg.email.bccAddrList1));
	memset(runNetworkCfg.email.bccAddrList2, 0, sizeof(runNetworkCfg.email.bccAddrList2));
	memset(runNetworkCfg.email.bccAddrList3, 0, sizeof(runNetworkCfg.email.bccAddrList3));
	if(email.stBccAddrList[0].csAddress[0] != '\0')
	{
		separate = strtok(email.stBccAddrList[0].csAddress, ";");
		if(separate == NULL)
		{
			strncpy(runNetworkCfg.email.bccAddrList0, email.stBccAddrList[0].csAddress, MAX_STR_LEN_64-1);
		}
		else
		{
			strncpy(runNetworkCfg.email.bccAddrList0, separate, MAX_STR_LEN_64-1);
			separate = strtok(NULL, ";");
			if(separate != NULL)
			{
				strncpy(runNetworkCfg.email.bccAddrList1, separate, MAX_STR_LEN_64-1);
				separate = strtok(NULL, ";");
				if(separate != NULL)
				{
					strncpy(runNetworkCfg.email.bccAddrList2, separate, MAX_STR_LEN_64-1);
					separate = strtok(NULL, ";");
					if(separate != NULL)
					{
						strncpy(runNetworkCfg.email.bccAddrList3, separate, MAX_STR_LEN_64-1);
					}
				}
			}
		}
	}
#endif

    return 0;
}


int Gk_CmsTestEmail(int sock)
{
    int ret = 0;
    DMS_NET_EMAIL_PARAM email;
    GK_NET_EMAIL_PARAM  gk_email;
	memset(&gk_email,0,sizeof(GK_NET_EMAIL_PARAM ));
    ret = GkSockRecv(sock, (char *)&email, sizeof(DMS_NET_EMAIL_PARAM));
    if(ret != sizeof(DMS_NET_EMAIL_PARAM))
    {
        PRINT_ERR();
        return -1;
    }

    gk_email.enableEmail    = 1;
    gk_email.attachPicture  = 0;
    gk_email.encryptionType = email.wEncryptionType;
    gk_email.smtpPort       = email.wServicePort;
    strncpy(gk_email.eMailUser,     email.csEMailUser,  MAX_STR_LEN_64-1);
    strncpy(gk_email.eMailPass,     email.csEmailPass,  MAX_STR_LEN_64-1);
    strncpy(gk_email.smtpServer,    email.csSmtpServer, MAX_STR_LEN_128-1);
    strncpy(gk_email.pop3Server,    email.csPop3Server, MAX_STR_LEN_128-1);
    strncpy(gk_email.fromAddr,      email.stSendAddrList.csAddress,     MAX_STR_LEN_64-1);
    strncpy(gk_email.toAddrList0,   email.stToAddrList[0].csAddress,    MAX_STR_LEN_64-1);
	strncpy(gk_email.ccAddrList0,   email.stCcAddrList[0].csAddress,    MAX_STR_LEN_64-1);
	strncpy(gk_email.bccAddrList0,  email.stBccAddrList[0].csAddress,   MAX_STR_LEN_64-1);

    #ifdef MODULE_SUPPORT_MAIL
    ret = netcam_mail_test_send(&gk_email);
    #else
    ret = -1;
    #endif
    if (0 == ret)
    {
        email.byTestStatus = 0;
        PRINT_INFO("Test Email send ok!\n");
    }
    else
    {
        email.byTestStatus = 1;
        PRINT_ERR("Test Email send fail!\n");
    }

    GkCmsCmdResq(sock, (char *)&email, sizeof(DMS_NET_EMAIL_PARAM), DMS_NET_TEST_SEND_EMAIL);

    return 0;
}

int Gk_CmsGetWifiSiteList(int sock)
{
	char *WifiName = NULL;
    DMS_NET_WIFI_SITE_LIST wifi_site_list;
 	WIFI_SCAN_LIST_t list[20];
	int number = 20;
	int i;
	//ST_SDK_NETWORK_ATTR net_attr;

	PRINT_INFO();
	memset(&wifi_site_list,0,sizeof(DMS_NET_WIFI_SITE_LIST));
	memset(&list,0,sizeof(WIFI_SCAN_LIST_t));

	wifi_site_list.dwSize = sizeof(DMS_NET_WIFI_SITE_LIST);

	WifiName = netcam_net_wifi_get_devname();
	if(WifiName != NULL)
	{
		netcam_net_wifi_on();

		if(netcam_net_wifi_get_scan_list(list,&number) == 0)
		{
			for(i = 0; i < number && list[i].vaild; i++)
			{
				memset(&wifi_site_list.stWifiSite[i],0,sizeof(DMS_NET_WIFI_SITE));
				if(list[i].mac[0] != '\0')
				{
					get_hw_array((unsigned char *)wifi_site_list.stWifiSite[i].byMac, (unsigned char *)list[i].mac);
				}
				if(list[i].essid[0] != '\0')
					strncpy((char *)wifi_site_list.stWifiSite[i].csEssid, (char *)list[i].essid,MAX_WIFI_NAME-1);
				wifi_site_list.stWifiSite[i].nRSSI = list[i].quality;
				wifi_site_list.stWifiSite[i].nChannel = list[i].channel;

				wifi_site_list.stWifiSite[i].bySecurity = list[i].security;
				wifi_site_list.stWifiSite[i].nType = 0;
				wifi_site_list.stWifiSite[i].dwSize = sizeof(DMS_NET_WIFI_SITE);
			}
			wifi_site_list.nCount = number;
			PRINT_INFO("scan wifi result:%d",number);
		}
		else
		{
			PRINT_INFO("scan wifi result 0");
		}
	}

    if (0 == netcam_net_wifi_isOn())
    {
        netcam_net_wifi_off();
    }

    //todo
    GkCmsCmdResq(sock, (char *)&wifi_site_list, sizeof(DMS_NET_WIFI_SITE_LIST), DMS_NET_GET_WIFI_SITE_LIST);
    return 0;
}

int Gk_CmsSetWifiWpsStart(int sock)
{
	DMS_NET_WIFI_CONFIG wifi;
	int ret = GkSockRecv(sock, (char *)&wifi, sizeof(DMS_NET_WIFI_CONFIG));
	if (ret != sizeof(DMS_NET_WIFI_CONFIG)) {
		PRINT_ERR();
		return -1;
	}

	ret = netcam_net_wifi_wps();
    //todo
	if(ret < 0) {
		PRINT_INFO("WPS Link fail");
		return -1;
	}
	PRINT_INFO("WPS Link OK");
    return 0;
}


int Gk_CmsGetWifi(int sock)
{
    int  ret       = 0;
	char *WifiName = NULL;
    DMS_NET_WIFI_CONFIG wifi;
	ST_SDK_NETWORK_ATTR net_attr;
	WIFI_LINK_INFO_t linkInfo;

	PRINT_INFO();
    //to do
    //byStatus ²ÎÕÕ WIRELESS_STATUS_E
    memset(&wifi,0,sizeof(DMS_NET_WIFI_CONFIG));
	wifi.dwSize = sizeof(DMS_NET_WIFI_CONFIG);

	WifiName = netcam_net_wifi_get_devname();
	if(WifiName == NULL) {
		PRINT_INFO("Not WiFi device.");
        wifi.byStatus = 1; // not connect
		goto exit;
	}
	snprintf(net_attr.name,sizeof(net_attr.name), WifiName);

    ret = netcam_net_get_detect("eth0");
    if(0 == ret)
    {
        wifi.byConnectStatus = 0;   /* eth0 conmunite */
    }
    else
    {
        if((ret = netcam_net_get_detect(WifiName)))
        {
            PRINT_ERR("Detect net device LAN_DEV:eth0  WIFI_DEV%s error!\n", WifiName);
            wifi.byStatus = 1; //  not connect
            goto exit;
        }
        wifi.byConnectStatus = 1;   /* ra0 conmunite */
    }
    PRINT_INFO("wifi.byConnectStatus:%d \n", wifi.byConnectStatus);

    //to do
	if(netcam_net_get(&net_attr) == 0)
	{
		memset(&linkInfo,0,sizeof(WIFI_LINK_INFO_t));
		netcam_net_wifi_get_connect_info(&linkInfo);
		{
			if(net_attr.enable == 0)
			{
				wifi.bWifiEnable = 0;
			}
			else
			{
				if( runNetworkCfg.wireless.enable == 1)
				{
					printf("net_attr.dhcp %d\n",net_attr.dhcp);
					if(net_attr.dhcp == 0)
					{
						wifi.bWifiEnable = 1;
					}
					else
					{
						wifi.bWifiEnable = 2;
					}

				}
			}

			wifi.byNetworkType = 2; //ad-hoc
			wifi.byWifiMode = linkInfo.mode; // station
			wifi.byWpsStatus = 0;
			wifi.nSecurity = linkInfo.linkScurity; // it is equal my define
			wifi.byEnableDHCP = net_attr.dhcp;

			strncpy((char*)wifi.dwGateway.csIpV4,(char *)net_attr.gateway,DMS_MAX_IP_LENGTH);
			strncpy((char*)wifi.dwNetIpAddr.csIpV4,(char *)net_attr.ip,DMS_MAX_IP_LENGTH);
			strncpy((char*)wifi.dwNetMask.csIpV4,(char *)net_attr.mask,DMS_MAX_IP_LENGTH);
			strncpy((char*)wifi.dwDNSServer.csIpV4,(char *)net_attr.dns1,DMS_MAX_IP_LENGTH);
			wifi.byStatus = linkInfo.linkStatus;
			strncpy((char*)wifi.csEssid,linkInfo.linkEssid,MAX_WIFI_NAME);

			strncpy(wifi.csWebKey,linkInfo.linkPsd,MAX_WIFI_NAME);
		}
	}
	else
	{
		wifi.byStatus = 1; // error
	}
exit:
    GkCmsCmdResq(sock, (char *)&wifi, sizeof(DMS_NET_WIFI_CONFIG), DMS_NET_GET_WIFICFG);
    return 0;
}


int Gk_CmsSetWifi(int sock)
{
	char *WifiName = NULL;
    DMS_NET_WIFI_CONFIG wifi;
	WIFI_LINK_INFO_t linkInfo;
	ST_SDK_NETWORK_ATTR net_attr;



    int ret = GkSockRecv(sock, (char *)&wifi, sizeof(DMS_NET_WIFI_CONFIG));
    if (ret != sizeof(DMS_NET_WIFI_CONFIG)) {
        PRINT_ERR();
        return -1;
    }
	PRINT_INFO();
	if(wifi.bWifiEnable == 0)
	{
		PRINT_INFO("Disable wifi function");
		netcam_net_wifi_off();
		return 0;
	}
	else
	{
		if(netcam_net_wifi_on() != 0)
		{
			PRINT_INFO("enable wifi function failed");
			return -1;
		}
	}

	memset(&linkInfo,0,sizeof(WIFI_LINK_INFO_t));
	netcam_net_wifi_get_connect_info(&linkInfo);

	WifiName = netcam_net_wifi_get_devname();
	if(WifiName == NULL) {
		PRINT_INFO("Not WiFi device.");
		return 0;
	}

	#if 1
	linkInfo.isConnect = wifi.bWifiEnable;
	//switch wifi mode
	if(wifi.byWifiMode == 0)
	{
		netcam_net_wifi_switchWorkMode(NETCAM_WIFI_STA);
		strcpy(linkInfo.linkEssid,wifi.csEssid);
		strcpy(linkInfo.linkPsd,wifi.csWebKey);
		linkInfo.linkScurity = wifi.nSecurity;
		linkInfo.mode = 0;

		snprintf(net_attr.name,sizeof(net_attr.name), WifiName);

		netcam_net_get(&net_attr);
		printf("bwifienable :%d\n",wifi.bWifiEnable);
		if(wifi.bWifiEnable == 2)
			net_attr.dhcp = 1;
		else
			net_attr.dhcp = 0;
		ret = netcam_net_wifi_set_connect_info(&linkInfo);
		if( ret == 0 &&  linkInfo.linkStatus == WIFI_CONNECT_OK )
		{

			if(net_attr.dhcp == 0)
			{
				strncpy((char *)net_attr.gateway,(char*)wifi.dwGateway.csIpV4,DMS_MAX_IP_LENGTH);
				strncpy((char *)net_attr.ip,(char*)wifi.dwNetIpAddr.csIpV4,DMS_MAX_IP_LENGTH);
				strncpy((char *)net_attr.mask,(char*)wifi.dwNetMask.csIpV4,DMS_MAX_IP_LENGTH);
				strncpy((char *)net_attr.dns1,(char*)wifi.dwDNSServer.csIpV4,DMS_MAX_IP_LENGTH);
			}

			//netcam_net_set(&net_attr);
			char *ifname = netcam_net_wifi_get_devname();
			if(ifname == NULL)
				return -1;
			netcam_net_reset_net_ip(ifname,&net_attr);
		}
		else
		{
			printf("come herrrr\n");
		}
	} else if(wifi.byWifiMode == 1) {
		netcam_net_wifi_switchWorkMode(NETCAM_WIFI_AP);
	}
		//
	#else

	if(linkInfo.isConnect != wifi.bWifiEnable)
	{
		if(wifi.bWifiEnable == 0)
		{
			  //to do
			netcam_net_get(&net_attr);
			if(net_attr.enable == 1)
			{
				net_attr.enable = 0;
				linkInfo.isConnect = 0;
				netcam_net_wifi_set_connect_info(&linkInfo);
				netcam_net_set(&net_attr);
			}
		}
		else
		{
			linkInfo.isConnect = wifi.bWifiEnable;
			linkInfo.mode = wifi.byWifiMode;
			linkInfo.isdhcp = wifi.byEnableDHCP;

			strncpy(linkInfo.linkEssid,(char*)wifi.csEssid,MAX_WIFI_NAME);
			strncpy(linkInfo.linkPsd,wifi.csWebKey,MAX_WIFI_NAME);
			linkInfo.linkScurity = wifi.nSecurity;

			if(netcam_net_wifi_set_connect_info(&linkInfo) == 0)
			{
				if(wifi.byEnableDHCP == 0 &&  linkInfo.linkStatus == WIFI_CONNECT_OK )
				{
						  //to do
					netcam_net_get(&net_attr);
					strncpy((char *)net_attr.gateway,(char*)wifi.dwGateway.csIpV4,DMS_MAX_IP_LENGTH);
					strncpy((char *)net_attr.ip,(char*)wifi.dwNetIpAddr.csIpV4,DMS_MAX_IP_LENGTH);
					strncpy((char *)net_attr.mask,(char*)wifi.dwNetMask.csIpV4,DMS_MAX_IP_LENGTH);
					strncpy((char *)net_attr.dns1,(char*)wifi.dwDNSServer.csIpV4,DMS_MAX_IP_LENGTH);
					netcam_net_set(&net_attr);
				}
			}

		}

	}
	else
	{
		if(wifi.bWifiEnable == 1)
		{
			if(linkInfo.linkStatus == WIFI_CONNECT_OK  )
			{
				if(strcmp(wifi.csEssid,linkInfo.linkEssid) == 0)
				{
					netcam_net_get(&net_attr);
					net_attr.dhcp = wifi.byEnableDHCP ;
					if(wifi.byEnableDHCP == 0)
					{
						strncpy((char *)net_attr.gateway,(char*)wifi.dwGateway.csIpV4,DMS_MAX_IP_LENGTH);
						strncpy((char *)net_attr.ip,(char*)wifi.dwNetIpAddr.csIpV4,DMS_MAX_IP_LENGTH);
						strncpy((char *)net_attr.mask,(char*)wifi.dwNetMask.csIpV4,DMS_MAX_IP_LENGTH);
						strncpy((char *)net_attr.dns1,(char*)wifi.dwDNSServer.csIpV4,DMS_MAX_IP_LENGTH);
					}
					netcam_net_set(&net_attr);
				}
				else
				{
					linkInfo.isConnect = wifi.bWifiEnable;
					linkInfo.mode = wifi.byWifiMode;
					linkInfo.isdhcp = wifi.byEnableDHCP;

					strncpy(linkInfo.linkEssid,(char*)wifi.csEssid,MAX_WIFI_NAME);
					strncpy(linkInfo.linkPsd,wifi.csWebKey,MAX_WIFI_NAME);
					linkInfo.linkScurity = wifi.nSecurity;

					if(netcam_net_wifi_set_connect_info(&linkInfo) == 0)
					{
						if(wifi.byEnableDHCP == 0 &&  linkInfo.linkStatus == WIFI_CONNECT_OK )
						{
								  //to do
							netcam_net_get(&net_attr);
							strncpy((char *)net_attr.gateway,(char*)wifi.dwGateway.csIpV4,DMS_MAX_IP_LENGTH);
							strncpy((char *)net_attr.ip,(char*)wifi.dwNetIpAddr.csIpV4,DMS_MAX_IP_LENGTH);
							strncpy((char *)net_attr.mask,(char*)wifi.dwNetMask.csIpV4,DMS_MAX_IP_LENGTH);
							strncpy((char *)net_attr.dns1,(char*)wifi.dwDNSServer.csIpV4,DMS_MAX_IP_LENGTH);
							netcam_net_set(&net_attr);
						}
					}


				}
			}
			else // ==0
			{
				netcam_net_get(&net_attr);
				if(net_attr.enable == 1)
				{
					net_attr.enable = 0;
					netcam_net_set(&net_attr);
				}

			}

		}

	}
	#endif
    //todo

    return 0;
}


int Gk_CmsTestWifiConnect(int sock)
{
    int ret = 0;
	char *WifiName = NULL;
    DMS_NET_WIFI_CONFIG wifi;
	WIFI_LINK_INFO_t linkInfo;

    ret = GkSockRecv(sock, (char *)&wifi, sizeof(DMS_NET_WIFI_CONFIG));
    if (ret != sizeof(DMS_NET_WIFI_CONFIG)) {
        PRINT_ERR("DMS_NET_WIFI_CONFIG:%d  ret:%d \n", sizeof(DMS_NET_WIFI_CONFIG), ret);
        return -1;
    }

    WifiName = netcam_net_wifi_get_devname();
	if(WifiName == NULL) {
		PRINT_INFO("Not WiFi device.");
		return 0;
	}

    PRINT_INFO("byConnectStatus :%d ", wifi.byConnectStatus);
    PRINT_INFO("wifi.csEssid    :%s ", wifi.csEssid);
    PRINT_INFO("wifi.csWebKey   :%s ", wifi.csWebKey);
    PRINT_INFO("wifi.nSecurity  :%d ", wifi.nSecurity);

	memset(&linkInfo,0, sizeof(WIFI_LINK_INFO_t));
	strcpy(linkInfo.linkEssid, wifi.csEssid);
	strcpy(linkInfo.linkPsd,   wifi.csWebKey);
	linkInfo.linkScurity = wifi.nSecurity;

	ret = netcam_net_wifi_connect_test(&linkInfo);
	if(WIFI_CONNECT_OK == linkInfo.linkStatus)
	{
	    PRINT_INFO("wifi_connect_test ssid:%s OK! ret:%d\n", wifi.csEssid, ret);
	    wifi.byStatus = 0;
	}
	else
	{
		PRINT_ERR("wifi_connect_test fail!\n");
        wifi.byStatus = 1;
	}

    GkCmsCmdResq(sock, (char *)&wifi, sizeof(DMS_NET_WIFI_CONFIG), DMS_NET_TEST_WIFI_CONNECT);
    return 0;
}


int Gk_CmsGetPPPOECFG(int sock)
{
    DMS_NET_PPPOECFG pppoe;
    memset(&pppoe, 0, sizeof(DMS_NET_PPPOECFG));
	pppoe.dwSize = sizeof(DMS_NET_PPPOECFG);


    GkCmsCmdResq(sock, (char *)&pppoe, sizeof(DMS_NET_PPPOECFG), DMS_NET_GET_PPPOECFG);

    return 0;
}


int Gk_CmsSetPPPOECFG(int sock)
{
    DMS_NET_PPPOECFG pppoe;

	int ret = GkSockRecv(sock, (char *)&pppoe, sizeof(DMS_NET_PPPOECFG));
	 if (ret != sizeof(DMS_NET_PPPOECFG)) {
		 PRINT_ERR();
		 return -1;
	 }



    //todo

    return 0;
}

int Gk_CmsGetP2PCFG(int sock)
{
    DMS_NET_P2P_CFG p2p[5];
	char deviceID[5][64] = {0};
	char mac[5][64] = {0};

    memset(&p2p,0,sizeof(DMS_NET_P2P_CFG) * 5);

	IPCsearch_get_p2p_info(deviceID, mac);

    #if 0
    // p2p type set
	p2p[0].u8p2ptype = 1;
    p2p[1].u8p2ptype = 2;
    p2p[2].u8p2ptype = 3;
    p2p[3].u8p2ptype = 5;
    // p2p id set
    for(i = 0; i < 4; i++)
    {
	    memcpy(p2p[i].csID,deviceID[i],32);
    }
    #else
	p2p[0].u8p2ptype = 5;
#ifdef MODULE_SUPPORT_GB28181
    if (strcmp(deviceID[3], "000000000") == 0)
    {
        sprintf(p2p[0].csID, "%s", deviceID[3]);
    }
    else
    {
        sprintf(p2p[0].csID, "%s,%s", deviceID[3], deviceID[4]);
    }
#else
    memcpy(p2p[0].csID,deviceID[3],32);
#endif
    #endif
    

    strcpy(p2p[0].u8mac, mac[3]);

    GkCmsCmdResq(sock, (char *)&p2p, sizeof(DMS_NET_P2P_CFG) * 1, DMS_NET_GET_P2PCFG);

    return 0;
}

int Gk_CmsSetP2PCFG(int sock)
{
    DMS_NET_P2P_CFG p2p;

    int ret = GkSockRecv(sock, (char *)&p2p, sizeof(DMS_NET_P2P_CFG));
    if (ret != sizeof(DMS_NET_P2P_CFG)) {
        PRINT_ERR();
        return -1;
    }

    #if 0
    if (0 == p2p.u8p2ptype || 0 == strlen(p2p.csID))
    {
        LOG_ERR("p2p id is invalid!\n");
        return -1;
    }
    #endif

    int type = p2p.u8p2ptype;//p2p.u8p2ptype;
    char *devId = p2p.csID;
    ret = IPCsearch_set_p2p_info(type, devId, p2p.u8mac);

    printf("Gk_CmsSetP2PCFG type:%d, ret:%d\n", type, ret);
    if (p2p.u8p2ptype == 11)//MTD_KEY_INFO
    {
        serverKeyRet = ret;
    }
    return 0;
}

int Gk_CmsGetFacCFG(int sock)
{
	DMS_NET_FAC_CFG fac;
	memset(&fac,0,sizeof(DMS_NET_FAC_CFG));

	GkCmsCmdResq(sock, (char *)&fac, sizeof(DMS_NET_FAC_CFG), CMD_GET_FAC_CFG);
    return 0;
}

int Gk_CmsSetFacCFG(int sock)
{
    DMS_NET_FAC_CFG fac;
	memset(&fac,0,sizeof(DMS_NET_FAC_CFG));

    int ret = GkSockRecv(sock, (char *)&fac, sizeof(DMS_NET_FAC_CFG));
    if (ret != sizeof(DMS_NET_FAC_CFG)) {
        LOG_ERR("recv err.\n");
        return -1;
    }

    return 0;
}

int Gk_CmsGetDDSCFG(int sock)
{
    DMS_NET_DDNSCFG dds;

    memset(&dds,0,sizeof(DMS_NET_DDNSCFG));
	dds.dwSize = sizeof(DMS_NET_DDNSCFG);


    GkCmsCmdResq(sock, (char *)&dds, sizeof(DMS_NET_DDNSCFG), DMS_NET_GET_DDNSCFG);


    //todo

    return 0;
}

int Gk_CmsSetDDSCFG(int sock)
{
    DMS_NET_DDNSCFG dds;;

    int ret = GkSockRecv(sock, (char *)&dds, sizeof(DMS_NET_DDNSCFG));
    if (ret != sizeof(DMS_NET_DDNSCFG)) {
        PRINT_ERR();
        return -1;
    }


    //todo

    return 0;
}


int Gk_CmsGetMobileCenterInfo(int sock)
{
    DMS_MOBILE_CENTER_INFO mobile;

    memset(&mobile,0,sizeof(DMS_MOBILE_CENTER_INFO));
	mobile.dwSize = sizeof(DMS_MOBILE_CENTER_INFO);


    GkCmsCmdResq(sock, (char *)&mobile, sizeof(DMS_MOBILE_CENTER_INFO), DMS_GET_MOBILE_CENTER_INFO);


    //todo

    return 0;
}

int Gk_CmsSetMobileCenterInfo(int sock)
{
	DMS_MOBILE_CENTER_INFO mobile;


    int ret = GkSockRecv(sock, (char *)&mobile, sizeof(DMS_MOBILE_CENTER_INFO));
    if (ret != sizeof(DMS_MOBILE_CENTER_INFO)) {
        PRINT_ERR();
        return -1;
    }


    //todo

    return 0;
}



