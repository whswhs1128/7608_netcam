#ifndef __LT_WIFI_CONFIGURE__H_
#define __LT_WIFI_CONFIGURE__H_
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

//获取库的版本
char *LT_GetVoiceWifiConfigVersion();

/* =============================================
		启动声波wifi配置
		return
			1		启动成功
			0		启动失败
rate为采样频率，范围（8000--44100），超出此范围时为缺省值44100。
================================================*/
int32_t LT_VoiceWifiConfigStart(char *gid, int rate);

/* ==============================================================
		声波wifi配置查询
		return
			1		接收到wifi配置消息
			0		接收中

			当函数返回为1时，wifi的ssid和password会
			分别通过char **pSsid, char **pPasswd传出来
            注意：此函数调用是直接返回的
================================================================*/
int32_t LT_VoiceWifiConfigPoll(char **pSsid, char **pPasswd);

/* =============================================
		关闭声波wifi配置
================================================*/
void LT_VoiceWifiConfigClose();

#ifdef __cplusplus
}
#endif

#endif
