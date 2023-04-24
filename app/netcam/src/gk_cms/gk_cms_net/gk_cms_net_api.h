/*!
*****************************************************************************
** FileName     : gk_cms_net_api.h
**
** Description  : media api of cms.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Date         : 2015-9-9, create.
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/


#ifndef _GK_CMS_NET_API_H_
#define _GK_CMS_NET_API_H_

#ifdef __cplusplus
extern "C"
{
#endif

int Gk_CmsGetException(int sock);
int Gk_CmsSetException(int sock);


int Gk_CmsGetRestore(int sock);
int Gk_CmsSetRestore(int sock);
int Gk_CmsSetSave(int sock);
void Gk_CmsSave();
void Gk_CmsRestore();
void Gk_CmsTestSpeaker();


int Gk_CmsGetLog(int sock);
int Gk_CmsGetAFOffset(int sock);
int Gk_CmsSetAFOffset(void);
int Gk_CmsGetFactoryKeyTestStatus(void);
void Gk_CmsSetFactoryKeyTestResult(int keyStatus);


int Gk_GetSystime(int sock);
int Gk_SetSystime(int sock);
int Gk_CmsGetSystime(int sock);
int Gk_CmsSetSystime(int sock);


int Gk_CmsGetAlarmout(int sock);
int Gk_CmsSetAlarmout(int sock);
int Gk_CmsAlarmoutControl(int sock);
int Gk_CmsGetAlarmoutState(int sock);


int Gk_CmsGetAlarmin(int sock);
int Gk_CmsSetAlarmin(int sock);


int Gk_CmsGetAllPreset(int sock);
int Gk_CmsGetCruise(int sock);
int Gk_CmsSetCruise(int sock);
int Gk_CmsPtzControl(int sock);


int Gk_CmsGetDevice(int sock);
int Gk_CmsSetDevice(int sock);
int Gk_CmsGetDefDevice(int sock);


int Gk_CmsGetStreamSupport(int sock);
int Gk_CmsGetVideo(int sock);
int Gk_CmsSetVideo(int sock);
int Gk_CmsGetDefVideo(int sock);


int Gk_CmsGetEnhancedColorSupport(int sock);
int Gk_CmsGetEnhancedColor(int sock);


int Gk_CmsGetDayNightDetection(int sock);
int Gk_CmsSetDayNightDetection(int sock);


int Gk_CmsGetColorSupport(int sock);
int Gk_CmsGetColor(int sock);
int Gk_CmsSetColor(int sock);
int Gk_CmsSetColorSingle(int sock);
int Gk_CmsGetDefColor(int sock);


int Gk_CmsGetFtp(int sock);
int Gk_CmsSetFtp(int sock);
int Gk_CmsTestFtp(int sock);


int Gk_CmsGetOsd(int sock, int stream_no);
int Gk_CmsSetOsd(int sock, int stream_no);


int Gk_CmsGetShelter(int sock, int stream_no);
int Gk_CmsSetShelter(int sock, int stream_no);


int Gk_CmsGetLan(int sock);
int Gk_CmsSetLan(int sock);
int Gk_CmsGetDefLan(int sock);


int Gk_CmsGetRtsp(int sock);
int Gk_CmsSetRtsp(int sock);


int Gk_CmsGetEmail(int sock);
int Gk_CmsSetEmail(int sock);
int Gk_CmsTestEmail(int sock);


int Gk_CmsGetRecordStreamMode(int sock);
int Gk_CmsSetRecordStreamMode(int sock);


int Gk_CmsGetZoneAndDst(int sock);
int Gk_CmsSetZoneAndDst(int sock);


int Gk_CmsGetNtp(int sock);
int Gk_CmsSetNtp(int sock);
int Gk_CmsDefNtp(int sock);

int Gk_CmsGetSnap(int sock);
int Gk_CmsSetSnap(int sock);


int Gk_CmsGetSnapEvent(int sock);
int Gk_CmsSetSnapEvent(int sock);


int Gk_CmsGetRs232(int sock);
int Gk_CmsSetRs232(int sock);
int Gk_CmsGetDefRs232(int sock);


int Gk_CmsGetUser(int sock);
int Gk_CmsSetUser(int sock);


int Gk_CmsGetMd(int sock);
int Gk_CmsSetMd(int sock);


int Gk_CmsGetVlost(int sock);
int Gk_CmsSetVlost(int sock);


int Gk_CmsGetHideAlarm(int sock);
int Gk_CmsSetHideAlarm(int sock);


int Gk_CmsGetWifi(int sock);
int Gk_CmsSetWifi(int sock);

int Gk_CmsTestWifiConnect(int sock);

int Gk_CmsGetWifiSiteList(int sock);
int Gk_CmsSetWifiWpsStart(int sock);


int Gk_CmdUpgradeData(int sock, int ext_len);
int Gk_CmdUpgradeReq(int sock, int ext_len);
int Gk_CmdUpgradeResp(int sock, int ext_len);


int Gk_CmsGetMaintain(int sock);
int Gk_CmsSetMaintain(int sock);


int Gk_CmsSetP2PCFG(int sock);
int Gk_CmsGetP2PCFG(int sock);

int Gk_CmsSetPPPOECFG(int sock);
int Gk_CmsGetPPPOECFG(int sock);


int Gk_CmsGetMobileCenterInfo(int sock);
int Gk_CmsSetMobileCenterInfo(int sock);


int Gk_CmsGetDDSCFG(int sock);
int Gk_CmsSetDDSCFG(int sock);


int Gk_CmsDoSnapshot(int sock);

int Gk_CmsGetFilelist(int sock);

int Gk_CmsGetPtzDecode(int sock);
int Gk_CmsSetPtzDecode(int sock);
int Gk_CmsGetDefPtzDecode(int sock);

int write_audio_from_file();
int Gk_CmsWriteAudioData(char *buffer, int count, int type);

int GkInitTimeSnap(void* param);


#ifdef MODULE_SUPPORT_LOCAL_REC
int Gk_CmsGetRecDayInMonth(int sock);
int Gk_CmsGetDefRecord(int sock);
int Gk_CmsGetRecord(int sock);
int Gk_CmsSetRecord(int sock);

int Gk_CmsGetRecordMode(int sock);
int Gk_CmsSetRecordMode(int sock);

int Gk_GetNas(int sock);
int Gk_SetNas(int sock);

int Gk_CmsGetHd(int sock);
int Gk_CmsSetHd(int sock);
int Gk_CmsHdFormat(int sock);
int Gk_CmsHdGetFormatStaus(int sock);
int Gk_CmsUnloadDisk();
int Gk_CmsRecordStart(int sock);
int Gk_CmsRecordStop(int sock);

int Gk_CmdRecControl(int sock);

#endif


#ifdef __cplusplus
}
#endif

#endif
