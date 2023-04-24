/*******************************************************************************
 *
 * Copyright (c)
 *
 *  \file search_ipc.h
 *
 *  \brief search ipc for product test tool.
 *
 *   create version  v 1.0.0  wangguixing
 *
 ******************************************************************************/

#ifndef _SEARCH_IPC_H_
#define _SEARCH_IPC_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

/* None */

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define NET_DEVICE      "eth0"

#define RECV_BUF_SIZE   1024

#define MAX_LISTEN_NUM          5
#define SEARCH_PORT             8600
#define SEARCH_BROADCAST_ADDR   "255.255.255.255"
#define SEARCH_BROADCAST_PORT   20909

#define START_CODE          0x4844
#define CMD_GET             0x0101
#define CMD_GET_RET         0x0801
#define CMD_SET             0x0102
#define CMD_SET_RET         0x0802

#define STATUS_OK           0x8000
#define STATUS_NAME_ERR     0x8001
#define STATUS_RIGHT_ERR    0x8002


/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

#pragma pack(1)
typedef struct _stBcastParam{
	char		   szHostIpAddr[16];	
    char           szIpAddr[16];     //IP地址，可以修改
    char           szMask[16];       //子网掩码，可以修改
    char           szGateway[16];    //网关，可以修改
    char           szDns1[16];       //dns1，可以修改
    char           szDns2[16];       //dns2，可以修改
    char           szMacAddr[6];     //设备MAC地址
    char           szWifiMacAddr[6];     //设备MAC地址
    unsigned short nPort;            //设备端口
    char           dwDeviceID[32];   //deviceid
    char           dwDeviceID1[32];   //deviceid `1
    char           dwDeviceID2[32];   // deviceid 2
    char           dwDeviceID3[32];   //deviceid 3
    char           szDevName[128];    //设备名称
    char           szManufacturerId[128];    //设备名称
    char           dwAeskey[32];   //platform deviceid
    char           sysver[16];       //固件版本
    char           appver[16];       //软件版本
    char           szUserName[32];   //修改时会对用户认证
    char           szPassword[32];   //修改时会对用户认证
    char           sysmode;          //0->baby 1->HDIPCAM
    char           dhcp;             //0->禁止dhcp,1->充许DHCP
    char           other[2];         //other
    char           other1[20];       //other1
}BCASTPARAM, *PBCASTPARAM;
#pragma pack()


#pragma pack(1)
typedef struct _stBcastDDNS{
    char    szProxySvr[64];          //服务名字
    char    szDdnsName[64];          //ddns名字
    char    szUserName[32];          //用户名
    char    szPassword[32];          //密码
}BCASTDDNS,*PBCASTDDNS;
#pragma pack()



/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/******************************************************************************
 * 函数介绍:  用于监听并处理生成测试工具和批量生产工具
 * 输入参数:  无
 * 输出参数:  无
 * 返回值      : 0   : success
 *                           <0 : error
 *                               v1.0.0    2015.09.09  wangguixing
 *****************************************************************************/
int ppcs_search_daemon_init(void);


/******************************************************************************
 * 函数介绍:  退出生产测试程序和生成批量工具
 * 输入参数:  无
 * 输出参数:  无
 * 返回值      : 0   : success
 *                           <0 : error
 *                               v1.0.0    2015.09.09  wangguixing
 *****************************************************************************/
int ppcs_search_daemon_exit(void);


#ifdef __cplusplus
}
#endif

#endif /* #ifndef _SEARCH_IPC_H_ */

