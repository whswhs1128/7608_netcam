/*!
*****************************************************************************
** \file        subsystem/onvif-no-gsoap/src/onvif_deviceio.c
**
** \version     $Id$
**
** \brief       onvif deviceio
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2013 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#include "onvif_deviceio.h"
#include "onvif_priv.h"

/***************************************************************************/
ONVIF_RET GK_NVT_DeviceIO_GetVideoSources(GONVIF_DEVICEIO_GetVideoSources_Res_S *pstIO_GetVideoSourcesRes)
{
    GK_S32 i;
    if(g_GkIpc_OnvifInf.VideoSourceNum <= MAX_VIDEOSOURCE_NUM)
    {
        pstIO_GetVideoSourcesRes->sizeVideoSources = g_GkIpc_OnvifInf.VideoSourceNum;
    }
    else
    {
        pstIO_GetVideoSourcesRes->sizeVideoSources = MAX_VIDEOSOURCE_NUM;
    }
    
    for(i = 0; i < pstIO_GetVideoSourcesRes->sizeVideoSources; i++)
    {
        strncpy(pstIO_GetVideoSourcesRes->stVideoSources[i].aszToken, g_GkIpc_OnvifInf.gokeIPCameraVideoSource[i].pszSourceToken, MAX_TOKEN_LENGTH-1);
        pstIO_GetVideoSourcesRes->stVideoSources[i].aszToken[MAX_TOKEN_LENGTH-1] = '\0';
        pstIO_GetVideoSourcesRes->stVideoSources[i].framerate = 30;
        
        pstIO_GetVideoSourcesRes->stVideoSources[i].stResolution.width = g_GkIpc_OnvifInf.gokeIPCameraVideoSource[i].stBounds.width;
        pstIO_GetVideoSourcesRes->stVideoSources[i].stResolution.height = g_GkIpc_OnvifInf.gokeIPCameraVideoSource[i].stBounds.height;
    }
        
    return ONVIF_OK;
}
ONVIF_RET GK_NVT_DeviceIO_GetAudioSources(GONVIF_DEVICEIO_GetAudioSources_Res_S *pstIO_GetAudioSourcesRes)
{
    GK_S32 i;
    
    if(g_GkIpc_OnvifInf.AudioSourceNum <= MAX_AUDIOSOURCE_NUM)
    {
        pstIO_GetAudioSourcesRes->sizeAudioSources = g_GkIpc_OnvifInf.AudioSourceNum;
    }
    else
    {
        pstIO_GetAudioSourcesRes->sizeAudioSources = MAX_AUDIOSOURCE_NUM;
    }

    for(i = 0; i < pstIO_GetAudioSourcesRes->sizeAudioSources; i++)
    {
        pstIO_GetAudioSourcesRes->stAudioSources[i].channels = 1;
        strncpy(pstIO_GetAudioSourcesRes->stAudioSources[i].aszToken, g_GkIpc_OnvifInf.gokeIPCameraAudioSource[i].pszSourceToken, MAX_TOKEN_LENGTH-1);
    }
    
    return ONVIF_OK;

}

ONVIF_RET GK_NVT_DeviceIO_GetRelayOutputs(GONVIF_DEVICEIO_GetRelayOutputs_Res_S *pstIO_GetRelayOutputsRes)
{
    GK_S32 i;
    pstIO_GetRelayOutputsRes->sizeRelayOutputs = g_GkIpc_OnvifInf.relayOutputsNum;
    
    for(i = 0; i < pstIO_GetRelayOutputsRes->sizeRelayOutputs; i++)
	{
        strncpy(pstIO_GetRelayOutputsRes->stRelayOutputs[i].aszToken, g_GkIpc_OnvifInf.pstGokeIPCameraRelayOutputs[i].aszToken, MAX_TOKEN_LENGTH-1);
        pstIO_GetRelayOutputsRes->stRelayOutputs[i].aszToken[MAX_TOKEN_LENGTH-1] = '\0';
        pstIO_GetRelayOutputsRes->stRelayOutputs[i].stProperties.enMode = g_GkIpc_OnvifInf.pstGokeIPCameraRelayOutputs[i].stProperties.enMode;
        pstIO_GetRelayOutputsRes->stRelayOutputs[i].stProperties.delayTime = g_GkIpc_OnvifInf.pstGokeIPCameraRelayOutputs[i].stProperties.delayTime;
        pstIO_GetRelayOutputsRes->stRelayOutputs[i].stProperties.enIdleState = g_GkIpc_OnvifInf.pstGokeIPCameraRelayOutputs[i].stProperties.enIdleState;
    }
        
    return ONVIF_OK;
}

ONVIF_RET GK_NVT_DeviceIO_SetRelayOutputSettings(GONVIF_DEVICEIO_SetRelayOutputSettings_S *pstIO_SetRelayOutputSettings)
{
    GK_S32 i;
    for(i = 0;i < g_GkIpc_OnvifInf.relayOutputsNum; i++)
    {
        if(0 == strcmp(pstIO_SetRelayOutputSettings->aszRelayOutputToken, g_GkIpc_OnvifInf.pstGokeIPCameraRelayOutputs[i].aszToken))
            break;
    }
    if(i == g_GkIpc_OnvifInf.relayOutputsNum)
    {
        ONVIF_ERR("Unknown relay token reference");
        return ONVIF_ERR_DEVICEIO_INVALID_RELAY_TOKEN;
    }
    else
    {
            g_GkIpc_OnvifInf.pstGokeIPCameraRelayOutputs[i].stProperties.enMode = pstIO_SetRelayOutputSettings->stProperties.enMode;
            g_GkIpc_OnvifInf.pstGokeIPCameraRelayOutputs[i].stProperties.delayTime = pstIO_SetRelayOutputSettings->stProperties.delayTime;
            g_GkIpc_OnvifInf.pstGokeIPCameraRelayOutputs[i].stProperties.enIdleState = pstIO_SetRelayOutputSettings->stProperties.enIdleState;
    }
    GK_NVT_SaveXmlConfig(XML_MEIDA);    
    return ONVIF_OK;
}

ONVIF_RET GK_NVT_DeviceIO_SetRelayOutputState(GONVIF_DEVICEIO_SetRelayOutputState_S *pstIO_SetRelayOutputState)
{
    GK_S32 i;
    for(i = 0;i < g_GkIpc_OnvifInf.relayOutputsNum; i++)
    {
        if(0 == strcmp(pstIO_SetRelayOutputState->aszRelayOutputToken, g_GkIpc_OnvifInf.pstGokeIPCameraRelayOutputs[i].aszToken))
            break;
    }
    if(i == g_GkIpc_OnvifInf.relayOutputsNum)
    {
        ONVIF_ERR("Unknown relay token reference");
        return ONVIF_ERR_DEVICEIO_INVALID_RELAY_TOKEN;
    }
    else
    {
    }
        
    return ONVIF_OK;
}

