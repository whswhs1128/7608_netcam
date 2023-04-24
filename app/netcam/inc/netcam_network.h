#ifndef _NETCAM_NETWORK_H_
#define _NETCAM_NETWORK_H_
#include "network_wifi.h"
#include "sdk_network.h"

//wifi
/*!
******************************************
** Description   @获WIFI的搜索列表
** Param[in]     @list:存放搜索到的SSID信息
                  number:list大上
**
** Return        @成功0，失败-1
******************************************
*/
int netcam_net_wifi_get_scan_list(WIFI_SCAN_LIST_t *list, int *number);
/*!
******************************************
** Description   @设置wifi连接信息
** Param[in]     @net_wifi:连接信息
**
** Return        @返回连接状态 [WIFI_CONNECT_STATUS]
******************************************
*/
int netcam_net_wifi_set_connect_info(WIFI_LINK_INFO_t *net_wifi);
/*!
******************************************
** Description   @获取wifi连接状态
** Param[in]     @net_wifi:存放连接信息
**
** Return        @查询成功：0，查询失败-1
******************************************
*/
int netcam_net_wifi_get_connect_info(WIFI_LINK_INFO_t *net_wifi);
int netcam_net_wifi_connect_test(WIFI_LINK_INFO_t *linkInfo);
/*!
******************************************
** Description   @获取wifi设备名称
** Param[in]     @NULL
**
** Return        @失败返回NULL,成功 !NULL
******************************************
*/
char* netcam_net_wifi_get_devname(void);
/*!
******************************************
** Description   @获取WIFI运行的使能开关
** Param[in]     @
**
** Return        @
******************************************
*/
int netcam_net_wifi_isenbale(void);
/*!
******************************************
** Description   @使能wifi(开启工作模式),工作模式为配置文件中指定的模式[NETCAM_WIFI_WorkModeEnumT]
** Param[in]     @NULL
**
** Return        @成功返回0，失败返回-1
******************************************
*/
int netcam_net_wifi_on(void);
/*!
******************************************
** Description   @停止当前工作模式,工作模式为配置文件中指定的模式[NETCAM_WIFI_WorkModeEnumT]
** Param[in]     @NULL
**
** Return        @成功返回0，失败返回-1
******************************************
*/
int netcam_net_wifi_off(void);
/*!
******************************************
** Description   @查询是否使能WIFI功能,wifi配置文件开关是否打开
** Param[in]     @NULL
**
** Return        @开启返回1， 关闭返回0
******************************************
*/
int netcam_net_wifi_isOn(void);
/*!
******************************************
** Description   @切换wifi工作模式
** Param[in]     @mode:工作模式
**
** Return        @切换成功返回0，失败-1
******************************************
*/
int netcam_net_wifi_switchWorkMode(NETCAM_WIFI_WorkModeEnumT mode);
/*!
******************************************
** Description   @获取wifi当前工作模式
** Param[in]     @NULL
**
** Return        @NETCAM_WIFI_WorkModeEnumT
******************************************
*/
NETCAM_WIFI_WorkModeEnumT netcam_net_wifi_getWorkMode(void);
/*!
******************************************
** Description   @设置AP工作模式对应的配置文件
** Param[in]     @apCfg:设置的配置信息
**
** Return        @成功0，失败-1
******************************************
*/
int netcam_net_wifi_setConfigHostAP(NETCAM_WIFI_HostAPConfigT *apCfg);
/*!
******************************************
** Description   @获取AP工作模式的相应配置信息
** Param[in]     @apCfg:存放配置信息
**
** Return        @成功0，失败-1
******************************************
*/
int netcam_net_wifi_getConfigHostAP(NETCAM_WIFI_HostAPConfigT *apCfg);
/*!
******************************************
** Description   @把WIFI自动连接和网卡检测添加到循环工作队列中。
** Param[in]     @NULL
**
** Return        @NULL
******************************************
*/
void netcam_net_monitor(void);
/*!
******************************************
** Description   @开始wps方式连接
** Param[in]     @NULL
**
** Return        @连接成功0，失败-1
******************************************
*/
int netcam_net_wifi_wps(void);
/*!
******************************************
** Description   @注册SMARTLINK 回调接口
** Param[in]     @func:smarmlink回调接口函数
**
** Return        @成功返回0，失败-1
******************************************
*/
int netcam_net_register_smartlink (NETCAM_SmartLinkFuncT *func);
/*!
******************************************
** Description   @注册内部SMARTLINK 回调接口
** Param[in]     @NULL
**
** Return        @成功返回0，失败-1
******************************************
*/
int netcam_net_private_smartlink(void);

unsigned char netcam_net_get_icute_number(void);

#endif

