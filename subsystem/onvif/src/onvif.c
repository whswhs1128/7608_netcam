#include "onvif.h"
#include "config.h"

int onvif_start(int isFromWeb, int webFd)
{
	if (NVT_NO_ERROR != GK_NVT_Env_Init())
	{
		ONVIF_ERR("Fail to init Env.");
		return NVT_SYS_ERROR;
	}
	if (NVT_NO_ERROR != GK_NVT_Device_Init(DEVICES))
	{
		ONVIF_ERR("Fail to init device.");
		return NVT_SYS_ERROR;
	}
	if (NVT_NO_ERROR != GK_NVT_Discover_Init(DISCOVER))
	{
		ONVIF_ERR("Fail to init discover.");
		return NVT_SYS_ERROR;
	}
	if (NVT_NO_ERROR != GK_NVT_Media_Init(MEDIASTATIC))
	{
		ONVIF_ERR("Fail to init media.");
		return NVT_SYS_ERROR;
	}
	if (NVT_NO_ERROR != GK_NVT_Imaging_Init(IMAGING))
	{
		ONVIF_ERR("Fail to init imaging.");
		return NVT_SYS_ERROR;
	}
	if (NVT_NO_ERROR != GK_NVT_Event_Init())
	{
		ONVIF_ERR("Fail to init event.");
		return NVT_SYS_ERROR;
	}
	if (NVT_NO_ERROR != GK_NVT_Ptz_Init(PTZ))
	{
		ONVIF_ERR("Fail to init ptz.");
		return NVT_SYS_ERROR;
	}
	if (NVT_NO_ERROR != GK_NVT_Start_UdpSoapService())
	{
		ONVIF_ERR("Fail to init UDP soap service.");
		return NVT_SYS_ERROR;
	}
	if (NVT_NO_ERROR != GK_NVT_Start_HttpSoapService(isFromWeb, webFd))
	{
		ONVIF_ERR("Fail to init HTTP soap service.");
		return NVT_SYS_ERROR;
	}

   return 0;
}

int onvif_stop(void)
{
    return 0;
}

