/*!
*****************************************************************************
** FileName     : cfg_network.c
**
** Description  : network infomation config api.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create Date  : 2015-8-4
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#if 1
#include "cfg_network.h"
#include <sys/time.h>
#include "flash_ctl.h"
#include <time.h>
//#include "ipcsearch.h"
#include "netcam_api.h"

static void random_produce_mac(char *randomMac);

GK_NET_NETWORK_CFG runNetworkCfg;

#if USE_DEFAULT_PARAM
GK_CFG_MAP lanMap[] = {
    {"enable",   &(runNetworkCfg.lan.enable),     GK_CFG_DATA_TYPE_S32,    "1",                 1, 0, 1,             NULL},
    {"netName",   &(runNetworkCfg.lan.netName),   GK_CFG_DATA_TYPE_STRING, "eth0",              1, 1, MAX_STR_LEN_64, NULL},
    {"ipVersion", &(runNetworkCfg.lan.ipVersion), GK_CFG_DATA_TYPE_S32,    "0",                 1, 0, 1,              "0 v4, 1 v6"},
    {"mac",       &(runNetworkCfg.lan.mac),       GK_CFG_DATA_TYPE_STRING, "28:E3:83:F6:40:12", 1, 1, MAX_STR_LEN_20, NULL},
    {"dhcpEnable",    &(runNetworkCfg.lan.dhcpIp),    GK_CFG_DATA_TYPE_S32, "1",                1, 0, 1,              NULL},
    {"upnpEnable",&(runNetworkCfg.lan.upnpEnable),GK_CFG_DATA_TYPE_S32,    "0",                 1, 0, 1,              NULL},
    {"ip",        &(runNetworkCfg.lan.ip),        GK_CFG_DATA_TYPE_STRING, "192.168.10.25",     1, 1, MAX_STR_LEN_16, NULL},
    {"netmask",   &(runNetworkCfg.lan.netmask),   GK_CFG_DATA_TYPE_STRING, "255.255.255.0",     1, 1, MAX_STR_LEN_16, NULL},
    {"gateway",   &(runNetworkCfg.lan.gateway),   GK_CFG_DATA_TYPE_STRING, "192.168.10.1",      1, 1, MAX_STR_LEN_16, NULL},
    {"multicast", &(runNetworkCfg.lan.multicast), GK_CFG_DATA_TYPE_STRING, "192.168.10.255",    1, 1, MAX_STR_LEN_16, NULL},
    {"dhcpDns",   &(runNetworkCfg.lan.dhcpDns),   GK_CFG_DATA_TYPE_S32,    "1",                 1, 0, 1,              NULL},
    {"dns1",      &(runNetworkCfg.lan.dns1),      GK_CFG_DATA_TYPE_STRING, "114.114.114.114",   1, 1, MAX_STR_LEN_16, NULL},
    {"dns2",      &(runNetworkCfg.lan.dns2),      GK_CFG_DATA_TYPE_STRING, "8.8.8.8",           1, 1, MAX_STR_LEN_16, NULL},
    {NULL,},
};

GK_CFG_MAP wifiMap[] = {
    {"enable",    &(runNetworkCfg.wifi.enable),     GK_CFG_DATA_TYPE_S32,     "1",                1, 0, 1,             NULL},
    {"netName",   &(runNetworkCfg.wifi.netName),   GK_CFG_DATA_TYPE_STRING, "wlan0",               1, 1, MAX_STR_LEN_64, NULL},
    {"ipVersion", &(runNetworkCfg.wifi.ipVersion), GK_CFG_DATA_TYPE_S32,    "0",                 1, 0, 1,              "0 v4, 1 v6"},
    {"mac",       &(runNetworkCfg.wifi.mac),       GK_CFG_DATA_TYPE_STRING, "28:E3:83:F6:40:13", 1, 1, MAX_STR_LEN_20, NULL},
    {"dhcpEnable",&(runNetworkCfg.wifi.dhcpIp),GK_CFG_DATA_TYPE_S32,    	"1",                 1, 0, 1,              NULL},
    {"upnpEnable",&(runNetworkCfg.wifi.upnpEnable),GK_CFG_DATA_TYPE_S32,    "0",                 1, 0, 1,              NULL},
    {"ip",        &(runNetworkCfg.wifi.ip),        GK_CFG_DATA_TYPE_STRING, "192.168.10.26",     1, 1, MAX_STR_LEN_16, NULL},
    {"netmask",   &(runNetworkCfg.wifi.netmask),   GK_CFG_DATA_TYPE_STRING, "255.255.255.0",     1, 1, MAX_STR_LEN_16, NULL},
    {"gateway",   &(runNetworkCfg.wifi.gateway),   GK_CFG_DATA_TYPE_STRING, "192.168.10.1",      1, 1, MAX_STR_LEN_16, NULL},
    {"multicast", &(runNetworkCfg.wifi.multicast), GK_CFG_DATA_TYPE_STRING, "5.255.255.255",    1, 1, MAX_STR_LEN_16, NULL},
    {"dhcpDns",   &(runNetworkCfg.wifi.dhcpDns),   GK_CFG_DATA_TYPE_S32,    "1",                 1, 0, 1,              NULL},
    {"dns1",      &(runNetworkCfg.wifi.dns1),      GK_CFG_DATA_TYPE_STRING, "114.114.114.114",   1, 1, MAX_STR_LEN_16, NULL},
    {"dns2",      &(runNetworkCfg.wifi.dns2),      GK_CFG_DATA_TYPE_STRING, "8.8.8.8",           1, 1, MAX_STR_LEN_16, NULL},
    {NULL,},
};

GK_CFG_MAP emailMap[] = {
    {"enable",    	 &(runNetworkCfg.email.enableEmail),    GK_CFG_DATA_TYPE_S32,     "1",                1, 0, 1,      NULL},
	{"attachPicture",&(runNetworkCfg.email.attachPicture),	GK_CFG_DATA_TYPE_S32,	  "1",				  1, 0, 1,	   NULL},
    {"smtpServer",   &(runNetworkCfg.email.smtpServer),     GK_CFG_DATA_TYPE_STRING,  "", 	   1, 1, MAX_STR_LEN_64, NULL},
	{"smtpPort",	 &(runNetworkCfg.email.smtpPort), 		GK_CFG_DATA_TYPE_S32,	  "25",    1, 1, 65535, NULL},
	{"pop3Server",	 &(runNetworkCfg.email.pop3Server), 	GK_CFG_DATA_TYPE_STRING,  "", 	   1, 1, MAX_STR_LEN_64, NULL},
	{"pop3Port",	 &(runNetworkCfg.email.pop3Port),		GK_CFG_DATA_TYPE_S32,	  "110",   1, 1, 65535, NULL},
    {"cryptionType", &(runNetworkCfg.email.encryptionType), GK_CFG_DATA_TYPE_S32,     "0",                1,  0, 2,	   NULL},
    {"user",       	 &(runNetworkCfg.email.eMailUser),      GK_CFG_DATA_TYPE_STRING,  "", 	   1, 1, MAX_STR_LEN_64,    NULL},
    {"password",	 &(runNetworkCfg.email.eMailPass),		GK_CFG_DATA_TYPE_STRING,  "", 	   1, 1, MAX_STR_LEN_64,	   NULL},
	{"fromAddrList", &(runNetworkCfg.email.fromAddr),		GK_CFG_DATA_TYPE_STRING,  "", 	   1, 1, MAX_STR_LEN_64,    NULL},

	{"toAddrList0", &(runNetworkCfg.email.toAddrList0),		GK_CFG_DATA_TYPE_STRING,  "", 	   1, 1, MAX_STR_LEN_64,    NULL},
	{"toAddrList1", &(runNetworkCfg.email.toAddrList1), 	GK_CFG_DATA_TYPE_STRING,  "",	   1, 1, MAX_STR_LEN_64,    NULL},
	{"toAddrList2", &(runNetworkCfg.email.toAddrList2), 	GK_CFG_DATA_TYPE_STRING,  "",	   1, 1, MAX_STR_LEN_64,    NULL},
	{"toAddrList3", &(runNetworkCfg.email.toAddrList3), 	GK_CFG_DATA_TYPE_STRING,  "",	   1, 1, MAX_STR_LEN_64,    NULL},
	{"ccAddrList0", &(runNetworkCfg.email.ccAddrList0), 	GK_CFG_DATA_TYPE_STRING,  "",	   1, 1, MAX_STR_LEN_64,    NULL},
	{"ccAddrList1", &(runNetworkCfg.email.ccAddrList1), 	GK_CFG_DATA_TYPE_STRING,  "",	   1, 1, MAX_STR_LEN_64,    NULL},
	{"ccAddrList2", &(runNetworkCfg.email.ccAddrList2), 	GK_CFG_DATA_TYPE_STRING,  "",	   1, 1, MAX_STR_LEN_64,    NULL},
	{"ccAddrList3", &(runNetworkCfg.email.ccAddrList3), 	GK_CFG_DATA_TYPE_STRING,  "",	   1, 1, MAX_STR_LEN_64,    NULL},
	{"bccAddrList0",&(runNetworkCfg.email.bccAddrList0), 	GK_CFG_DATA_TYPE_STRING,  "",	   1, 1, MAX_STR_LEN_64,    NULL},
	{"bccAddrList1",&(runNetworkCfg.email.bccAddrList1), 	GK_CFG_DATA_TYPE_STRING,  "",	   1, 1, MAX_STR_LEN_64,    NULL},
	{"bccAddrList2",&(runNetworkCfg.email.bccAddrList2), 	GK_CFG_DATA_TYPE_STRING,  "",	   1, 1, MAX_STR_LEN_64,    NULL},
	{"bccAddrList3",&(runNetworkCfg.email.bccAddrList3), 	GK_CFG_DATA_TYPE_STRING,  "",	   1, 1, MAX_STR_LEN_64,    NULL},
    {NULL,},
};

GK_CFG_MAP ftpMap[] = {
    {"enableFTP", &(runNetworkCfg.ftp.enableFTP), GK_CFG_DATA_TYPE_S32,     "0",       1, 0, 1,               NULL},
    {"address",   &(runNetworkCfg.ftp.address),   GK_CFG_DATA_TYPE_STRING,  "",  1, 1, MAX_STR_LEN_128, NULL},
    {"port",      &(runNetworkCfg.ftp.port),      GK_CFG_DATA_TYPE_S32,     "",      1, 0, 65536,           NULL},
    {"userName",  &(runNetworkCfg.ftp.userName),  GK_CFG_DATA_TYPE_STRING,  "",    1, 1, MAX_STR_LEN_64,  NULL},
    {"password",  &(runNetworkCfg.ftp.password),  GK_CFG_DATA_TYPE_STRING,  "",    1, 1, MAX_STR_LEN_64,  NULL},
    {"datapath",  &(runNetworkCfg.ftp.datapath),  GK_CFG_DATA_TYPE_STRING,  "MDSnapshot",   1, 1, MAX_STR_LEN_128,  NULL},
    {"filename",  &(runNetworkCfg.ftp.filename),  GK_CFG_DATA_TYPE_STRING,  "image.jpg",   1, 1, MAX_STR_LEN_128,  NULL},
    {"interval",  &(runNetworkCfg.ftp.interval),  GK_CFG_DATA_TYPE_S32,     "30",   1, 1, MAX_STR_LEN_128,  NULL},
    {NULL,},
};

GK_CFG_MAP portMap[] = {
    {"httpPort", &(runNetworkCfg.port.httpPort), GK_CFG_DATA_TYPE_S32,  "80",   1, 0, 65536, NULL},
    {"rtspPort", &(runNetworkCfg.port.rtspPort), GK_CFG_DATA_TYPE_S32,  "554",  1, 0, 65536, NULL},
    {NULL,},
};

GK_CFG_MAP wifiLinkMap[] ={

	{"ssid",	  runNetworkCfg.wifilink[0].essid,GK_CFG_DATA_TYPE_STRING,  "",    1, 1, MAX_STR_LEN_32,  NULL},
	{"passd",	  runNetworkCfg.wifilink[0].passd,GK_CFG_DATA_TYPE_STRING,	"",    1, 1, MAX_STR_LEN_32,  NULL},
	{"encMode",   &runNetworkCfg.wifilink[0].encMode,GK_CFG_DATA_TYPE_S32,	"0",    1, 1, 88,  NULL},
	{NULL,}
};


GK_CFG_MAP wirelessMap[] ={
	{"enable",	  &runNetworkCfg.wireless.enable,GK_CFG_DATA_TYPE_S32,  "1",    1, 0, 1,  NULL},
	{"mode",	  &runNetworkCfg.wireless.mode,	GK_CFG_DATA_TYPE_S32,    "1",	  1, 0, 1,  NULL},
	{"apBssId",	  &runNetworkCfg.wireless.apBssId,GK_CFG_DATA_TYPE_S32, "0",	  1, 0, 1,  NULL},
	{"apPsk",     &runNetworkCfg.wireless.apPsk,GK_CFG_DATA_TYPE_S32, "0",   1, 0, 1,  NULL},
	{NULL,}

};


#else
GK_CFG_MAP lanMap[] = {
    {"enable",   &(runNetworkCfg.lan.enable),     },
    {"netName",   &(runNetworkCfg.lan.netName),   },
    {"ipVersion", &(runNetworkCfg.lan.ipVersion), },
    {"mac",       &(runNetworkCfg.lan.mac),       },
    {"dhcpEnable",    &(runNetworkCfg.lan.dhcpIp),},
    {"upnpEnable",&(runNetworkCfg.lan.upnpEnable),},
    {"ip",        &(runNetworkCfg.lan.ip),        },
    {"netmask",   &(runNetworkCfg.lan.netmask),   },
    {"gateway",   &(runNetworkCfg.lan.gateway),   },
    {"multicast", &(runNetworkCfg.lan.multicast), },
    {"dhcpDns",   &(runNetworkCfg.lan.dhcpDns),   },
    {"dns1",      &(runNetworkCfg.lan.dns1),      },
    {"dns2",      &(runNetworkCfg.lan.dns2),      },
    {"autotrack", &(runNetworkCfg.lan.autotrack), },
    {NULL,},
};

GK_CFG_MAP wifiMap[] = {
    {"enable",    &(runNetworkCfg.wifi.enable),    },
    {"netName",   &(runNetworkCfg.wifi.netName),   },
    {"ipVersion", &(runNetworkCfg.wifi.ipVersion), },
    {"mac",       &(runNetworkCfg.wifi.mac),       },
    {"dhcpEnable",&(runNetworkCfg.wifi.dhcpIp),    },
    {"upnpEnable",&(runNetworkCfg.wifi.upnpEnable),},
    {"ip",        &(runNetworkCfg.wifi.ip),        },
    {"netmask",   &(runNetworkCfg.wifi.netmask),   },
    {"gateway",   &(runNetworkCfg.wifi.gateway),   },
    {"multicast", &(runNetworkCfg.wifi.multicast), },
    {"dhcpDns",   &(runNetworkCfg.wifi.dhcpDns),   },
    {"dns1",      &(runNetworkCfg.wifi.dns1),      },
    {"dns2",      &(runNetworkCfg.wifi.dns2),      },
    {NULL,},
};

GK_CFG_MAP emailMap[] = {
    {"enable",    	 &(runNetworkCfg.email.enableEmail),    },
	{"attachPicture",&(runNetworkCfg.email.attachPicture),	},
    {"smtpServer",   &(runNetworkCfg.email.smtpServer),     },
	{"smtpPort",	 &(runNetworkCfg.email.smtpPort), 		},
	{"pop3Server",	 &(runNetworkCfg.email.pop3Server), 	},
	{"pop3Port",	 &(runNetworkCfg.email.pop3Port),		},
    {"cryptionType", &(runNetworkCfg.email.encryptionType), },
    {"user",       	 &(runNetworkCfg.email.eMailUser),      },
    {"password",	 &(runNetworkCfg.email.eMailPass),		},
	{"fromAddrList", &(runNetworkCfg.email.fromAddr),		},

	{"toAddrList0", &(runNetworkCfg.email.toAddrList0),		},
	{"toAddrList1", &(runNetworkCfg.email.toAddrList1), 	},
	{"toAddrList2", &(runNetworkCfg.email.toAddrList2), 	},
	{"toAddrList3", &(runNetworkCfg.email.toAddrList3), 	},
	{"ccAddrList0", &(runNetworkCfg.email.ccAddrList0), 	},
	{"ccAddrList1", &(runNetworkCfg.email.ccAddrList1), 	},
	{"ccAddrList2", &(runNetworkCfg.email.ccAddrList2), 	},
	{"ccAddrList3", &(runNetworkCfg.email.ccAddrList3), 	},
	{"bccAddrList0",&(runNetworkCfg.email.bccAddrList0), 	},
	{"bccAddrList1",&(runNetworkCfg.email.bccAddrList1), 	},
	{"bccAddrList2",&(runNetworkCfg.email.bccAddrList2), 	},
	{"bccAddrList3",&(runNetworkCfg.email.bccAddrList3), 	},
    {NULL,},
};

GK_CFG_MAP ftpMap[] = {
    {"enableFTP", &(runNetworkCfg.ftp.enableFTP),   },
    {"address",   &(runNetworkCfg.ftp.address),     },
    {"port",      &(runNetworkCfg.ftp.port),        },
    {"userName",  &(runNetworkCfg.ftp.userName),    },
    {"password",  &(runNetworkCfg.ftp.password),    },
    {"datapath",  &(runNetworkCfg.ftp.datapath),    },
    {"filename",  &(runNetworkCfg.ftp.filename),    },
    {"interval",  &(runNetworkCfg.ftp.interval),    },
    {NULL,},
};

GK_CFG_MAP portMap[] = {
    {"httpPort", &(runNetworkCfg.port.httpPort),    },
    {"rtspPort", &(runNetworkCfg.port.rtspPort),    },
    {NULL,},
};

GK_CFG_MAP wifiLinkMap[] ={

	{"ssid",	  runNetworkCfg.wifilink[0].essid,      },
	{"passd",	  runNetworkCfg.wifilink[0].passd,      },
	{"encMode",   &runNetworkCfg.wifilink[0].encMode,   },
	{NULL,}
};


GK_CFG_MAP wirelessMap[] ={
	{"enable",	  &runNetworkCfg.wireless.enable,       },
	{"mode",	  &runNetworkCfg.wireless.mode,	        },
	{"apBssId",	  &runNetworkCfg.wireless.apBssId,      },
	{"apPsk",     &runNetworkCfg.wireless.apPsk,        },
	{NULL,}

};
#endif
//use default ip address for dhcp failed
static void reset_net_default_dhcp(void)
{
    int i = 0;
    if(runNetworkCfg.lan.dhcpIp == 1)
    {
        for(i = 0;  lanMap[i].stringName != NULL;i++)
        {
            if(strcmp("ip",lanMap[i].stringName) == 0)
                strcpy(runNetworkCfg.lan.ip,lanMap[i].defaultValue);
            else if(strcmp("netmask",lanMap[i].stringName) == 0)
                strcpy(runNetworkCfg.lan.netmask,lanMap[i].defaultValue);
            else if(strcmp("gateway",lanMap[i].stringName) == 0)
                strcpy(runNetworkCfg.lan.gateway,lanMap[i].defaultValue);
            else if(strcmp("dns1",lanMap[i].stringName) == 0)
                strcpy(runNetworkCfg.lan.dns1,lanMap[i].defaultValue);
            else if(strcmp("dns2",lanMap[i].stringName) == 0)
                strcpy(runNetworkCfg.lan.dns2,lanMap[i].defaultValue);
        }
    }
    if(runNetworkCfg.wifi.dhcpIp == 1)
    {
        for(i = 0;  wifiMap[i].stringName != NULL;i++)
        {
            if(strcmp("ip",wifiMap[i].stringName) == 0)
                strcpy(runNetworkCfg.wifi.ip,wifiMap[i].defaultValue);
            else if(strcmp("netmask",wifiMap[i].stringName) == 0)
                strcpy(runNetworkCfg.wifi.netmask,wifiMap[i].defaultValue);
            else if(strcmp("gateway",wifiMap[i].stringName) == 0)
                strcpy(runNetworkCfg.wifi.gateway,wifiMap[i].defaultValue);
            else if(strcmp("dns1",wifiMap[i].stringName) == 0)
                strcpy(runNetworkCfg.wifi.dns1,wifiMap[i].defaultValue);
            else if(strcmp("dns2",wifiMap[i].stringName) == 0)
                strcpy(runNetworkCfg.wifi.dns2,wifiMap[i].defaultValue);
        }
    }


}
void NetworkCfgPrint()
{
    printf("*************** Network **************\n");

    printf("lan:\n");
    CfgPrintMap(lanMap);
    printf("\n");

    printf("wifi:\n");
    CfgPrintMap(wifiMap);
    printf("\n");

    printf("wifi:\n");
    CfgPrintMap(emailMap);
    printf("\n");

    printf("ftp:\n");
    CfgPrintMap(ftpMap);
    printf("\n");

    printf("port:\n");
    CfgPrintMap(portMap);
    printf("\n");

    printf("*************** Network **************\n\n");
}


void wifiLinkParse(cJSON * item, char *link)
{
	int i = 0;
	cJSON *obj;
	memset(runNetworkCfg.wifilink,0,sizeof(runNetworkCfg.wifilink));
	item = cJSON_GetObjectItem(item, link);
    if (!item) {
        //PRINT_ERR("Error before: [%s]\n", cJSON_GetErrorPtr());
        return ;
    }
	for( i = 0; i < cJSON_GetArraySize(item) && i < WIFI_CONNECT_LIST_MAX_NUMBER ; i++)
	{
		obj = cJSON_GetArrayItem(item,i);
		strcpy((char *)runNetworkCfg.wifilink[i].essid, (char *)cJosn_Read_String(obj,(char *)"ssid"));
		strcpy((char *)runNetworkCfg.wifilink[i].passd, (char *)cJosn_Read_String(obj,(char *)"passd"));
		runNetworkCfg.wifilink[i].encMode = cJosn_Read_Int(obj,(char *)"encMode");
		runNetworkCfg.wifilink[i].valid = 1;
	}
}

cJSON *wifiLinkCreate()
{
	int i = 0;
    cJSON *root;

	cJSON *obj;

    root = cJSON_CreateArray();//创建项目


	for(i = 0; i < WIFI_CONNECT_LIST_MAX_NUMBER; i++)
	{
		if(runNetworkCfg.wifilink[i].valid)
		{
			obj = cJSON_CreateObject();
			cJSON_AddStringToObject(obj, "ssid",(char *)runNetworkCfg.wifilink[i].essid);
			cJSON_AddStringToObject(obj, "passd",(char *)runNetworkCfg.wifilink[i].passd);
			cJSON_AddNumberToObject(obj, "encMode",runNetworkCfg.wifilink[i].encMode);
			cJSON_AddItemToArray(root,obj);
		}
	}

	return root;

}


int NetworkCfgSave()
{
    cJSON *root;
	cJSON *wifiLink = NULL;
    char *out;

    root = cJSON_CreateObject();//创建项目

    CfgAddCjson(root, "lan", lanMap);
    CfgAddCjson(root, "wifi", wifiMap);
    CfgAddCjson(root, "email", emailMap);
    CfgAddCjson(root, "ftp", ftpMap);
    CfgAddCjson(root, "port", portMap);
    CfgAddCjson(root, "wireless", wirelessMap);

	wifiLink = wifiLinkCreate();
	cJSON_AddItemToObject(root,"wifiLink",wifiLink);

    out = cJSON_Print(root);

    int ret = CfgWriteToFile(NETWORK_CFG_FILE, out);
    if (ret != 0) {
        PRINT_ERR("CfgWriteToFile %s error.", NETWORK_CFG_FILE);
        return -1;
    }

    free(out);
    cJSON_Delete(root);

    return 0;
}

int NetworkCfgLoadDefValue()
{
    CfgLoadDefValue(lanMap);
    CfgLoadDefValue(wifiMap);
    CfgLoadDefValue(emailMap);
    CfgLoadDefValue(ftpMap);
    CfgLoadDefValue(portMap);
	CfgLoadDefValue(wirelessMap);
	memset(runNetworkCfg.wifilink,0,sizeof(runNetworkCfg.wifilink));
    random_produce_mac(runNetworkCfg.lan.mac);
    reset_net_default_dhcp();
    return 0;
}


int NetworkCfgLoad()
{
    int load_def_config = 0;
    char *data = NULL;
    int i;
	int save_flag = 0;

start_load:
    data = CfgReadFromFile(NETWORK_CFG_FILE);
    if (data == NULL) {
        //从配置文件读取失败，则使用默认参数
        PRINT_INFO("load %s error, so to load default cfg param.\n", NETWORK_CFG_FILE);
        goto err;
    }

    cJSON *json = NULL;
    json = cJSON_Parse(data);
    if (!json){
        //从配置文件解析cjson失败，则使用默认参数
        PRINT_ERR("Error before: [%s]\n", cJSON_GetErrorPtr());
        free(data);
        goto err;
    }


    CfgParseCjson(json, "lan", lanMap);
    CfgParseCjson(json, "wifi", wifiMap);
    CfgParseCjson(json, "email", emailMap);
    CfgParseCjson(json, "ftp", ftpMap);
    CfgParseCjson(json, "port", portMap);
	CfgParseCjson(json, "wireless", wirelessMap);
	wifiLinkParse(json,"wifiLink");

    for(i = 0;  lanMap[i].stringName != NULL;i++)
    {
    	if(strcmp(lanMap[i].stringName,"mac") == 0)
    	{

            #ifdef MODULE_SUPPORT_MOJING
            int ret = -1;
            char mac[MAX_STR_LEN_20]={0};
            struct device_info_mtd factory;
            memset(&factory, 0, sizeof(struct device_info_mtd));
            ret = load_info_to_mtd_reserve(MTD_TUTK_P2P, &factory, sizeof(struct device_info_mtd));
    		if(ret == 0 && strcmp(factory.eth_mac_addr, "00:00:00:00:00:00") != 0 && strlen(factory.eth_mac_addr) > 0)
            {
                if (strncmp(factory.device_id, "000000000", 9) != 0)
                {
                    int j = 0,k = 0;
                    for(j=5; j<17; j++)
                    {
                        mac[k++] = factory.device_id[j];

                        if((j-4)%2 == 0 && k<17)
                            mac[k++] = ':';
                    }
                    //魔镜id才需要与mac同步
                    if (strcmp(factory.eth_mac_addr, mac) == 0 ||
                        (factory.device_id[0] != '0' && factory.device_id[1] != '8'))
                    {
                        memset(runNetworkCfg.lan.mac, 0, sizeof(runNetworkCfg.lan.mac));
                        strncpy(runNetworkCfg.lan.mac, factory.eth_mac_addr, sizeof(runNetworkCfg.lan.mac));
        				PRINT_ERR("load mac from mtd1:[%s]\n", factory.eth_mac_addr);
        				break;
                    }
                    else
                    {
                        PRINT_ERR("mac change:%s->%s\n", factory.eth_mac_addr, mac);
                    }
                }
                else
                {
                    strcpy(mac, factory.eth_mac_addr);
                }
            }

            if (ret != 0)
            {
                //set 000000000 to factory tool to burn id
                strcpy(factory.device_id, "000000000");
            }
            //printf("fac mac:%s,len:%d mac:%s\n", factory.eth_mac_addr, strlen(factory.eth_mac_addr),  mac);
            if (strcmp(factory.eth_mac_addr, "00:00:00:00:00:00") == 0 
                || strlen(factory.eth_mac_addr) == 0
                || (strcmp(mac, "00:00:00:00:00:00") != 0 && strcmp(factory.eth_mac_addr, mac) != 0))
            {
    			//first random a mac
                random_produce_mac(runNetworkCfg.lan.mac);
                //read mac form mtd
                //if(!load_info_to_mtd_reserve(MTD_GOKE_INFO, &factory, sizeof(struct goke_factory)) &&
                //        strcmp(factory.mac, "00:00:00:00:00:00") != 0)
                //{
                    //memset(runNetworkCfg.lan.mac, 0, sizeof(runNetworkCfg.lan.mac));
                    //strncpy(runNetworkCfg.lan.mac, factory.mac, sizeof(runNetworkCfg.lan.mac));
                //}
                //else
                //{
                //save mac to mtd
    			strncpy(factory.eth_mac_addr, runNetworkCfg.lan.mac, sizeof(runNetworkCfg.lan.mac));
    			save_info_to_mtd_reserve(MTD_TUTK_P2P, &factory, sizeof(struct device_info_mtd));
    			save_flag = 1;
                //}
            }
			PRINT_ERR("random mac and save to mtd:[%s], flag:%d\n", factory.eth_mac_addr, save_flag);
            #else
    		struct goke_factory factory;
    		if(!load_info_to_mtd_reserve(MTD_GOKE_INFO, &factory, sizeof(struct goke_factory)) &&
                    strcmp(factory.mac, "00:00:00:00:00:00") != 0)
            {
                memset(runNetworkCfg.lan.mac, 0, sizeof(runNetworkCfg.lan.mac));
                strncpy(runNetworkCfg.lan.mac, factory.mac, sizeof(runNetworkCfg.lan.mac));
				PRINT_ERR("load mac from mtd2:[%s]\n", factory.mac);
				break;
            }

			//first random a mac
            random_produce_mac(runNetworkCfg.lan.mac);
            //read mac form mtd
            memset(&factory, 0, sizeof(struct goke_factory));
            //if(!load_info_to_mtd_reserve(MTD_GOKE_INFO, &factory, sizeof(struct goke_factory)) &&
            //        strcmp(factory.mac, "00:00:00:00:00:00") != 0)
            //{
                //memset(runNetworkCfg.lan.mac, 0, sizeof(runNetworkCfg.lan.mac));
                //strncpy(runNetworkCfg.lan.mac, factory.mac, sizeof(runNetworkCfg.lan.mac));
            //}
            //else
            //{
            //save mac to mtd
			strncpy(factory.mac, runNetworkCfg.lan.mac, sizeof(runNetworkCfg.lan.mac));
			save_info_to_mtd_reserve(MTD_GOKE_INFO, &factory, sizeof(struct goke_factory));
			save_flag = 1;
            //}
			PRINT_ERR("random mac and save to mtd:[%s]\n", factory.mac);
            #endif
            break;
    	}
    }
	// 如果默认的配置文件无wifi信息，则从reserve分区中还原wifi信息
	if(strlen((char*)runNetworkCfg.wifilink[0].essid) == 0)
	{
		struct device_info_mtd info ;
		if(!load_info_to_mtd_reserve(MTD_TUTK_P2P, &info, sizeof(struct device_info_mtd)) )
		{
			if( strlen(info.wifi_ssid) != 0 )
			{
				strncpy((char*)runNetworkCfg.wifilink[0].essid, info.wifi_ssid,32);
				strncpy((char*)runNetworkCfg.wifilink[0].passd, info.wifi_psd,32);
				runNetworkCfg.wifilink[0].valid = 1;
				runNetworkCfg.wifilink[0].encMode = info.wifi_enc_type;
                runNetworkCfg.wireless.mode = 1;
				PRINT_INFO("Use reserve wifi info [ssid][psd] [%s]:[%s]:[%d]",
					info.wifi_psd,info.wifi_ssid,info.wifi_enc_type);
				save_flag = 1;
			}
		}

	}
	if(save_flag == 1)
		NetworkCfgSave();//save default ip address

    cJSON_Delete(json);
    reset_net_default_dhcp();

	free(data);
    return 0;

err:
    if(!load_def_config)
    {
        load_def_config = 1;
        char src_name[64], des_name[64];
        memset(src_name, 0, sizeof(src_name));
        snprintf(src_name,sizeof(src_name), "%s%s", DEFCFG_DIR, NETWORK_CFG_FILE);
        memset(des_name, 0, sizeof(des_name));
        snprintf(des_name,sizeof(des_name), "%s%s", CFG_DIR, NETWORK_CFG_FILE);
        if(!copy_file(src_name, des_name))
            goto start_load;
    }
    NetworkCfgLoadDefValue();
    NetworkCfgSave();
    return 0;
}

#endif


char *NetworkCfgGetJosnString(int type)
{

    char *out;
    cJSON *root;
    switch(type)
    {
        case 0:
        root = CfgDataToCjsonByMap(lanMap);
        break;
        case 1:
        root = CfgDataToCjsonByMap(wifiMap);
        break;
        default:
            return NULL;
        break;
    }

    if(root == NULL)
    {
        PRINT_ERR("Network Map to json error.");
        return NULL;
    }
    out = cJSON_Print(root);
    PRINT_INFO("Network type:%d,json:%s\n",type,out);
    cJSON_Delete(root);
    return out;

}


static void random_produce_mac(char *randomMac)
{
	char HEXCHAR[16]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	char fmtMACAddr[]={'B','E',':','2','7',':','0','0',':','0','0',':','0','0',':','0','0',0};
	int i = 0, n = 0;
	unsigned int seed = 0;
	struct timespec tv;
	seed = (unsigned int)HEXCHAR+(unsigned int)fmtMACAddr \
		+(unsigned int)getpid()+(unsigned int)&i +(unsigned int)&n;
	usleep(500000);
	clock_gettime(CLOCK_REALTIME, &tv);
	seed += tv.tv_sec + tv.tv_nsec;
	PRINT_INFO("seed: %u\n", seed);
	srand(seed);
	for(i = 6; i < strlen(fmtMACAddr); i++)
	{
		n = (rand()% 16);
		if(((i+1)% 3) !=0)
		{
			fmtMACAddr[i] = HEXCHAR[n];
		}
	}

#ifdef MODULE_SUPPORT_MOJING
	struct device_info_mtd gb28181_info;
	char mac[MAX_STR_LEN_20]={0};
	int j = 0,k = 0;
    PRINT_INFO("start to load mtd\n");
	if(!load_info_to_mtd_reserve(MTD_TUTK_P2P, &gb28181_info, sizeof(struct device_info_mtd)))
	{
        //printf("gb28181_info.eth_mac_addr:%s\n", gb28181_info.eth_mac_addr);
        if (strcmp(gb28181_info.device_id, "000000000") != NULL 
            && strncmp(gb28181_info.device_id, "08", 2) == 0)
        {
    	    PRINT_INFO("cfg_network id:%s\n", gb28181_info.device_id);
    		for(j=5; j<17; j++)
    		{
    			mac[k++] = gb28181_info.device_id[j];

    			if((j-4)%2 == 0 && k<17)
    				mac[k++] = ':';
    		}

    		PRINT_INFO("MAC = %s\n",mac);
    		strcpy(randomMac,mac);
        }
        else
        {
    		strcpy(randomMac,fmtMACAddr);
        }
	}
	else
    {
        PRINT_INFO("--->load addr error:%s\n", gb28181_info.eth_mac_addr);
		strcpy(randomMac, fmtMACAddr);
    }
#else
    strcpy(randomMac, fmtMACAddr);
#endif

	PRINT_INFO("Random mac: %s\n",randomMac);
}

