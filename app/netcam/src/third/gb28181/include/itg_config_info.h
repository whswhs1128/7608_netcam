
#ifndef ITG_CONFIG_INFO_H
#define ITG_CONFIG_INFO_H

#define ITE_CONFIG_INFO_KEY 1 

typedef struct IPNC_SipIpcConfig_T
{
	char sipRegServerIP[50];
	int  sipRegServerPort;
	char sipRegServerUID[50];
	char sipRegServerPwd[50];
	char sipRegServerDomain[50];
	int  sipDevicePort;
	char sipDeviceUID[50];
	int  regExpire_s;
	int  sipDevHBCycle_s;
	int  sipDevHBOutTimes;
	char sipAlarmID[50];
	int  bUseSipDevice;
} IPNC_SipIpcConfig;

typedef struct iConfigInfo_T
{
	IPNC_SipIpcConfig SipIpcParam;
} iConfigInfo;


#endif //ITG_CONFIG_INFO_H
