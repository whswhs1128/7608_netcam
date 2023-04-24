/*!
*****************************************************************************
** \file        solution/software/subsystem/onvif-no-gsoap/src/onvif_media.c
**
** \brief       onvif media
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2013 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#include "onvif_media.h"
#include "onvif_adapter.h"
#include "config.h"

#define RTSP_SERVER_PORT 1998
#define MAX_MUTIINFOR_LENGTH 48
#define CLIENT_RTSP_IP_PORT_REPORT    0x01000001
#define SERVER_RTSP_IP_PORT_ACK       0x02000001

#define PACKAGE_HEAD    1
#define PACKAGE_CMD     2
#define PACKAGE_LEN     3
#define PACKAGE_TAIL    4

GK_S32 media_GetVideoEncoderConfigurations(void *argv);
GK_S32 media_GetAudioEncoderConfigurations(void *argv);
GK_S32 media_GetVideoEncoderConfigurationOptions(void *argv, GK_S32 sizeOptions);
GK_S32 media_GetAudioEncoderConfigurationOptions(void *argv, GK_S32 sizeOptions);

static GK_S32 media_GetRtspAck(GK_U8 *ack, GK_U8 *pkg, GK_U32 pkg_len);
static void   media_PacketContent(GK_U8 *buffer, void *content, GK_U32 content_type);
static GK_S32 media_PacketCommand(GK_U32 cmd, GK_U32 ipaddr, GK_U16 port, GK_U8 media_type, GK_U8 media_switch,GK_U8 *pdata);
static void   media_PrintPackage(GK_U8 *data, GK_U32 data_len);
static GK_S32 media_ParseAckMsg(GK_U8 *rcv_buffer, GK_U32 len);
static GK_S32 media_SendMuticastInforToRtsp(GK_U32 ipaddr, GK_U16 port, GK_U8 media_type, GK_U8 media_switch);

//*****************************************************************************
//*****************************************************************************
//** API Functions : Capabilities
//*****************************************************************************
//*****************************************************************************

ONVIF_RET GK_NVT_Media_GetServiceCapabilities(GONVIF_MEDIA_GetServiceCapabilities_Res_S *pstMD_GetServiceCapabilitiesRes)
{
    pstMD_GetServiceCapabilitiesRes->stCapabilities.stProfileCapabilities.maximumNumberOfProfiles = MAX_PROFILE_NUM;
    pstMD_GetServiceCapabilitiesRes->stCapabilities.stStreamingCapabilities.enRTPMulticast = Boolean_TRUE;
    pstMD_GetServiceCapabilitiesRes->stCapabilities.stStreamingCapabilities.enRTP_USCORETCP= Boolean_TRUE;
    pstMD_GetServiceCapabilitiesRes->stCapabilities.stStreamingCapabilities.enRTP_USCORERTSP_USCORETCP= Boolean_TRUE;
    pstMD_GetServiceCapabilitiesRes->stCapabilities.stStreamingCapabilities.enNonAggregateControl = Boolean_FALSE;
    pstMD_GetServiceCapabilitiesRes->stCapabilities.stStreamingCapabilities.enNoRTSPStreaming= Boolean_FALSE;
    pstMD_GetServiceCapabilitiesRes->stCapabilities.enSnapshotUri = Boolean_TRUE;
    pstMD_GetServiceCapabilitiesRes->stCapabilities.enRotation = Boolean_FALSE;
    pstMD_GetServiceCapabilitiesRes->stCapabilities.enVideoSourceMode = Boolean_FALSE;
    pstMD_GetServiceCapabilitiesRes->stCapabilities.enOSD = Boolean_FALSE;

    return ONVIF_OK;
}

/***************************************************************************
函数描述: 创建Profile
入口参数：pstMD_CreateProfile : 需要创建的Profile信息
				pstMD_CreateProfileRes : 生成的Profile信息				
返回值     ：ONVIF_OK: 成功
         		      非零: 失败
***************************************************************************/
ONVIF_RET GK_NVT_Media_CreateProfile(GONVIF_MEDIA_CreateProfile_S *pstMD_CreateProfile, GONVIF_MEDIA_CreateProfile_Res_S *pstMD_CreateProfileRes)
{
    GK_S32 i = 0;
    if (pstMD_CreateProfile->aszName[0] == '\0' && pstMD_CreateProfile->aszToken[0] == '\0')
    {
        goto invalid_arg;
    }
    
    for(i = 0; i < g_GkIpc_OnvifInf.ProfileNum; i++)
    {
        if(strcmp(pstMD_CreateProfile->aszName, g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszName) == 0)
        {
            goto profile_exist;
        }
        if(strcmp(pstMD_CreateProfile->aszToken, g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken) == 0)
        {
            goto profile_exist;
        }
    }

    for(i = 0; i < MAX_PROFILE_NUM; i++)
    {
        if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken == NULL && g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszName == NULL)
        {
            g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszName = (GK_CHAR *)malloc(sizeof(GK_CHAR) * MAX_NAME_LENGTH);
            memset(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszName, 0, MAX_TOKEN_LENGTH);
            if(pstMD_CreateProfile->aszName[0] == '\0')
            {
                sprintf(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszName, "Profile%d", i);
            }
            else
            {
                strncpy(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszName, pstMD_CreateProfile->aszName, MAX_NAME_LENGTH - 1);
            }
            pstMD_CreateProfileRes->stProfile.pszName = g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszName;

            g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken= (GK_CHAR *)malloc(sizeof(GK_CHAR) * MAX_TOKEN_LENGTH);
            memset(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken, 0, MAX_TOKEN_LENGTH);
            if(pstMD_CreateProfile->aszToken[0] == '\0')
            {
                sprintf(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken, "ProfileToken%d", i);
            }
            else
            {
                strncpy(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken, pstMD_CreateProfile->aszToken, MAX_TOKEN_LENGTH -1);
            }
            pstMD_CreateProfileRes->stProfile.pszToken = g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken;

            g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].enFixed = Boolean_FALSE;
            pstMD_CreateProfileRes->stProfile.enFixed = g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].enFixed;

            g_GkIpc_OnvifInf.ProfileNum++;
            break;
        }
    }

    if(i == MAX_PROFILE_NUM)
    {
        goto max_NVT_profiles;
    }
    
    GK_NVT_SaveXmlConfig(XML_MEIDA);
    return ONVIF_OK;
    
invalid_arg:
    return ONVIF_ERR_MEDIA_INVAILD_ARG;
profile_exist:
    return ONVIF_ERR_MEDIA_PROFILE_EXIST;
max_NVT_profiles:
    return ONVIF_ERR_MEDIA_MAX_NVT_PROFILES;

}

/***************************************************************************
函数描述: 获取单个Profile信息
入口参数：pstMD_GetProfile : 需要创建的Profile信息
				pstMD_GetProfileRes : 生成的Profile信息				
返回值     ：ONVIF_OK: 成功
         		      非零: 失败
***************************************************************************/
ONVIF_RET GK_NVT_Media_GetProfile(GONVIF_MEDIA_GetProfile_S *pstMD_GetProfile, GONVIF_MEDIA_GetProfile_Res_S *pstMD_GetProfileRes)
{
    GK_S32 ret;
    GK_S32 i;
    if(pstMD_GetProfile->aszProfileToken[0] == '\0')
    {
        goto invalid_arg;
    }
    
    ret = media_GetVideoEncoderConfigurations(g_GkIpc_OnvifInf.gokeIPCameraVideoEncode);
    if(ret != 0)
    {
        ONVIF_DBG("Get Video Encoder Configuration failed.\n");
        goto run_error;
    }
    ret = media_GetAudioEncoderConfigurations(g_GkIpc_OnvifInf.gokeIPCameraAudioEncode);
    if(ret != 0)
    {
        ONVIF_DBG("Get Audio Encoder Configuration failed.\n");
        goto run_error;
    }
    for(i = 0; i < g_GkIpc_OnvifInf.ProfileNum; i++)
    {
        if(strcmp(pstMD_GetProfile->aszProfileToken, g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken) == 0)
        {
            memcpy(&pstMD_GetProfileRes->stProfile, &g_GkIpc_OnvifInf.gokeIPCameraProfiles[i], sizeof(GONVIF_MEDIA_Profile_S));
            break;
        }
    }

    if(i == g_GkIpc_OnvifInf.ProfileNum)
    {
        goto no_such_profile;
    }
    
    return ONVIF_OK;
    
run_error:
    return ONVIF_ERR_DEVMNG_SERVER_RUN_ERROR;
invalid_arg:
    return ONVIF_ERR_MEDIA_INVAILD_ARG;
no_such_profile:
    return ONVIF_ERR_MEDIA_NO_SUCH_PROFILE_EXIST;
    
}

/***************************************************************************
函数描述:  获取全部Profile信息
入口参数：pstMD_GetProfilesRes : 获取的Profile信息			
返回值     ：ONVIF_OK: 成功
         		      ONVIF_ERR_MEDIA_SERVER_RUN_ERROR : 失败
***************************************************************************/
ONVIF_RET GK_NVT_Media_GetProfiles(GONVIF_MEDIA_GetProfiles_Res_S *pstMD_GetProfilesRes)
{
    GK_S32 ret;
    GK_S32 i = 0;
    
    ret = media_GetVideoEncoderConfigurations(g_GkIpc_OnvifInf.gokeIPCameraVideoEncode);
    if (ret != 0)
    {
        ONVIF_DBG("Get Video Encoder Configuration failed.\n");
        goto run_error;
    }
    ret = media_GetAudioEncoderConfigurations(g_GkIpc_OnvifInf.gokeIPCameraAudioEncode);
    if (ret != 0)
    {
        ONVIF_DBG("Get Audio Encoder Configuration failed.\n");
        goto run_error;
    }
    pstMD_GetProfilesRes->sizeProfiles = g_GkIpc_OnvifInf.ProfileNum;
    
    for (i = 0; i < pstMD_GetProfilesRes->sizeProfiles; i++)
    {
        memcpy(&pstMD_GetProfilesRes->stProfiles[i], &g_GkIpc_OnvifInf.gokeIPCameraProfiles[i], sizeof(GONVIF_MEDIA_Profile_S));
    }

    return ONVIF_OK;
    
run_error:
    return ONVIF_ERR_MEDIA_SERVER_RUN_ERROR;
}

/***************************************************************************
函数描述:    删除全部Profile信息
入口参数：pstMD_DeleteProfile : 删除的Profile Token值			
返回值     ：ONVIF_OK: 成功
         		      非零 : 失败
***************************************************************************/
ONVIF_RET GK_NVT_Media_DeleteProfile(GONVIF_MEDIA_DeleteProfile_S *pstMD_DeleteProfile)
{
    GK_S32 i = 0, j = 0;
    if (pstMD_DeleteProfile->aszProfileToken[0] == '\0')
    {
        goto invalid_arg;
    }
    for(i = 0; i < g_GkIpc_OnvifInf.ProfileNum; i++)
    {
        if(strcmp(pstMD_DeleteProfile->aszProfileToken,g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken) == 0)
        {
            break;
        }
    }
    if(i == g_GkIpc_OnvifInf.ProfileNum)
    {
        goto no_such_profile;

    }

    if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].enFixed == Boolean_TRUE)
    {
        goto fixed_profile;
    }

    if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstVideoSourceConfiguration != NULL)
    {
        for(j = 0; j < g_GkIpc_OnvifInf.VideoSourceNum; j++)
        {
            if(strcmp(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstVideoSourceConfiguration->pszToken, g_GkIpc_OnvifInf.gokeIPCameraVideoSource[j].pszToken) == 0)
            {
                g_GkIpc_OnvifInf.gokeIPCameraVideoSource[j].useCount--;
                break;
            }
        }
        g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstVideoSourceConfiguration = NULL;
    }
    
    if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstAudioSourceConfiguration != NULL)
    {
        for(j = 0; j < g_GkIpc_OnvifInf.AudioSourceNum; j++)
        {
            if(strcmp(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstAudioSourceConfiguration->pszToken, g_GkIpc_OnvifInf.gokeIPCameraAudioSource[j].pszToken) == 0)
            {
                g_GkIpc_OnvifInf.gokeIPCameraAudioSource[j].useCount--;
                break;
            }
        }
        g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstAudioSourceConfiguration = NULL;
    }
    
    if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstVideoEncoderConfiguration != NULL)
    {
        for(j = 0; j < g_GkIpc_OnvifInf.VideoEncodeNum; j++)
        {
            if(strcmp(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstVideoEncoderConfiguration->pszToken, g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[j].pszToken) == 0)
            {
                g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[j].useCount--;
                break;
            }
        }
        g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstVideoEncoderConfiguration = NULL;
    }

    if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstAudioEncoderConfiguration != NULL)
    {
        for(j = 0; j < g_GkIpc_OnvifInf.AudioEncodeNum; j++)
        {
            if(strcmp(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstAudioEncoderConfiguration->pszToken, g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[j].pszToken) == 0)
            {
                g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[j].useCount--;
                break;
            }
        }
        g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstAudioEncoderConfiguration = NULL;
    }

    if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstVideoAnalyticsConfiguration != NULL)
    {
        for(j = 0; j < g_GkIpc_OnvifInf.VideoAnalyticNum; j++)
        {
            if(strcmp(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstVideoAnalyticsConfiguration->pszToken, g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[j].pszToken) == 0)
            {
                g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[j].useCount--;
                break;
            }
        }
        g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstVideoAnalyticsConfiguration = NULL;
    }

    if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstPTZConfiguration != NULL)
    {
        for(j = 0; j < g_GkIpc_OnvifInf.PTZConfigurationNum; j++)
        {
            if(strcmp(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstPTZConfiguration->token, g_GkIpc_OnvifInf.gokePTZConfiguration[j].token) == 0)
            {
                g_GkIpc_OnvifInf.gokePTZConfiguration[j].useCount--;
                break;
            }
        }
        g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstPTZConfiguration = NULL;
    }
    #if 0
    //detele Metadata configuration
    if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstMetadataConfiguration != NULL)
    {
        for(j = 0; j < g_GkIpc_OnvifInf.; j++)
        {
            if(strcmp(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstMetadataConfiguration->pszToken, g_GkIpc_OnvifInf.) == 0)
            {
                g_GkIpc_OnvifInf.;
                break;
            }
        }
        g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstMetadataConfiguration = NULL;
    }
    #endif
    free(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszName);
    g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszName = NULL;
    free(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken);
    g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken = NULL;
    
    memset(&g_GkIpc_OnvifInf.gokeIPCameraProfiles[i], 0, sizeof(GONVIF_MEDIA_Profile_S));

    GK_S32 num = 0;
    for(num = i+1; num < MAX_PROFILE_NUM; num++)
    {
        memcpy(&g_GkIpc_OnvifInf.gokeIPCameraProfiles[i], &g_GkIpc_OnvifInf.gokeIPCameraProfiles[num], sizeof(GONVIF_MEDIA_Profile_S));
        memset(&g_GkIpc_OnvifInf.gokeIPCameraProfiles[num], 0, sizeof(GONVIF_MEDIA_Profile_S));
        i++;
    }
    g_GkIpc_OnvifInf.ProfileNum--;
    
    GK_NVT_SaveXmlConfig(XML_MEIDA);
    return ONVIF_OK;
    
invalid_arg:
    return ONVIF_ERR_MEDIA_INVAILD_ARG;
no_such_profile:
    return ONVIF_ERR_MEDIA_NO_SUCH_PROFILE_EXIST;
fixed_profile:
    return ONVIF_ERR_MEDIA_DELETE_FIXED_PROFILE;
}


/***************************************************************************
函数描述: 获取视频源配置信息
入口参数：pstMD_AddVideoSourceConfiguration : 添加的视频源配置信息
返回值     ：ONVIF_OK: 成功
         		      非零: 失败
***************************************************************************/
ONVIF_RET GK_NVT_Media_AddVideoSourceConfiguration(GONVIF_MEDIA_AddVideoSourceConfiguration_S *pstMD_AddVideoSourceConfiguration)
{
    GK_S32 i = 0;
    GK_S32 j = 0;
    GK_S32 k = 0;

    if(pstMD_AddVideoSourceConfiguration->aszProfileToken[0] == '\0' || pstMD_AddVideoSourceConfiguration->aszConfigurationToken[0] == '\0')
    {
        goto invalid_arg;
    }

    for(i = 0; i < g_GkIpc_OnvifInf.ProfileNum; i++)
    {
        if(strcmp(pstMD_AddVideoSourceConfiguration->aszProfileToken, g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken) == 0)
        {
            break;
        }
    }
    if(i == g_GkIpc_OnvifInf.ProfileNum)
    {
        goto no_such_profile;
    }

    for(j = 0; j < g_GkIpc_OnvifInf.VideoSourceNum; j++)
    {
        if(strcmp(pstMD_AddVideoSourceConfiguration->aszConfigurationToken, g_GkIpc_OnvifInf.gokeIPCameraVideoSource[j].pszToken) == 0)
        {
            break;
        }
    }
    if(j == g_GkIpc_OnvifInf.VideoSourceNum)
    {
        goto no_config_exist;
    }

    if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstVideoSourceConfiguration != NULL)
    {
        for(k = 0; k < g_GkIpc_OnvifInf.VideoSourceNum; k++)
        {
            if(strcmp(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstVideoSourceConfiguration->pszToken, g_GkIpc_OnvifInf.gokeIPCameraVideoSource[k].pszToken) == 0)
            {
                g_GkIpc_OnvifInf.gokeIPCameraVideoSource[k].useCount--;
                break;
            }
        }
        g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstVideoSourceConfiguration = &g_GkIpc_OnvifInf.gokeIPCameraVideoSource[j];
        g_GkIpc_OnvifInf.gokeIPCameraVideoSource[j].useCount++;
    }
    else
    {
        g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstVideoSourceConfiguration = &g_GkIpc_OnvifInf.gokeIPCameraVideoSource[j];
        g_GkIpc_OnvifInf.gokeIPCameraVideoSource[j].useCount++;

    }
    
    GK_NVT_SaveXmlConfig(XML_MEIDA);
    return ONVIF_OK;
    
invalid_arg:
    return ONVIF_ERR_MEDIA_INVAILD_ARG;  
no_such_profile:
    return ONVIF_ERR_MEDIA_NO_SUCH_PROFILE_EXIST; 
no_config_exist:
    return ONVIF_ERR_MEDIA_NO_SUCH_CONFIG_EXIST;
//config_conflict:
    //return ONVIF_ERR_MEDIA_CONFIGURATION_CONFLICT;
}

/***************************************************************************
函数描述: 获取视频源配置信息
入口参数：pstMD_AddVideoEncoderConfiguration : 添加视频编码配置信息
返回值     ：ONVIF_OK: 成功
         		      非零: 失败
***************************************************************************/
ONVIF_RET GK_NVT_Media_AddVideoEncoderConfiguration(GONVIF_MEDIA_AddVideoEncoderConfiguration_S *pstMD_AddVideoEncoderConfiguration)
{
    GK_S32 i = 0;
    GK_S32 j = 0;
    GK_S32 k = 0;
    if(pstMD_AddVideoEncoderConfiguration->aszProfileToken[0] == '\0' || pstMD_AddVideoEncoderConfiguration->aszConfigurationToken[0] == '\0')
    {
        goto invalid_arg;
    }

    for(i = 0; i < g_GkIpc_OnvifInf.ProfileNum; i++)
    {
        if(strcmp(pstMD_AddVideoEncoderConfiguration->aszProfileToken, g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken) == 0)
        {
            break;
        }
    }

    if(i == g_GkIpc_OnvifInf.ProfileNum)
    {
        goto no_such_profile;
    }

    for(j = 0; j < g_GkIpc_OnvifInf.VideoEncodeNum; j++)
    {
        if(strcmp(pstMD_AddVideoEncoderConfiguration->aszConfigurationToken, g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[j].pszToken) == 0)
        {
            break;
        }
    }
    if(j == g_GkIpc_OnvifInf.VideoEncodeNum)
    {
        goto no_config_exist;
    }
    
    if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstVideoSourceConfiguration != NULL)
    {
        if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstVideoEncoderConfiguration != NULL)
        {
            for(k = 0; k < g_GkIpc_OnvifInf.VideoEncodeNum; k++)
            {
                if(strcmp(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstVideoEncoderConfiguration->pszToken, g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[k].pszToken) == 0)
                {
                    g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[k].useCount--;
                    break;
                }
            }
            g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstVideoEncoderConfiguration = &g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[j];
            g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[j].useCount++;
        }
        else
        {
            g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstVideoEncoderConfiguration = &g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[j];
            g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[j].useCount++;
        }
    }
    
    GK_NVT_SaveXmlConfig(XML_MEIDA);
    return ONVIF_OK;
    
invalid_arg:
    return ONVIF_ERR_MEDIA_INVAILD_ARG;
no_such_profile:
    return ONVIF_ERR_MEDIA_NO_SUCH_PROFILE_EXIST;
no_config_exist:
    return ONVIF_ERR_MEDIA_NO_SUCH_CONFIG_EXIST;
//config_conflict:
    //return ONVIF_ERR_MEDIA_CONFIGURATION_CONFLICT;
}

ONVIF_RET GK_NVT_Media_AddAudioSourceConfiguration(GONVIF_MEDIA_AddAudioSourceConfiguration_S *pstMD_AddAudioSourceConfiguration)
{
    GK_S32 i = 0;
    GK_S32 j = 0;
    GK_S32 k = 0;

	if(g_GkIpc_OnvifInf.AudioSourceNum == 0)
	{
		goto no_audio_supported;
	}

    if(pstMD_AddAudioSourceConfiguration->aszProfileToken[0] == '\0' || pstMD_AddAudioSourceConfiguration->aszConfigurationToken[0] == '\0')
    {
        goto invalid_arg;
    }

    for(i = 0; i < g_GkIpc_OnvifInf.ProfileNum; i++)
	{
	    if(strcmp(pstMD_AddAudioSourceConfiguration->aszProfileToken, g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken) == 0)
	    {
    		break;
	    }
	}
	if(i == g_GkIpc_OnvifInf.ProfileNum)
	{
		goto no_such_profile;
	}

    for(j = 0; j < g_GkIpc_OnvifInf.AudioSourceNum; j++)
	{
	    if(strcmp(pstMD_AddAudioSourceConfiguration->aszConfigurationToken, g_GkIpc_OnvifInf.gokeIPCameraAudioSource[j].pszToken) == 0)
	    {
    		break;
	    }
	}
	if(j == g_GkIpc_OnvifInf.AudioSourceNum)
	{
		goto no_config_exist;
	}
	if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstAudioSourceConfiguration != NULL)
	{
	    for(k = 0; k < g_GkIpc_OnvifInf.AudioSourceNum; k++)
		{
		    if(strcmp(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstAudioSourceConfiguration->pszToken,g_GkIpc_OnvifInf.gokeIPCameraAudioSource[k].pszToken) == 0)
		    {
				g_GkIpc_OnvifInf.gokeIPCameraAudioSource[k].useCount--;
	    		break;
		    }
		}
		g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstAudioSourceConfiguration = &g_GkIpc_OnvifInf.gokeIPCameraAudioSource[j];
		g_GkIpc_OnvifInf.gokeIPCameraAudioSource[j].useCount++;
	}
	else
	{
		g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstAudioSourceConfiguration = &g_GkIpc_OnvifInf.gokeIPCameraAudioSource[j];
		g_GkIpc_OnvifInf.gokeIPCameraAudioSource[j].useCount++;
	}
	
    GK_NVT_SaveXmlConfig(XML_MEIDA);

    return ONVIF_OK;
    

invalid_arg:
    return ONVIF_ERR_MEDIA_INVAILD_ARG;
no_such_profile:
    return ONVIF_ERR_MEDIA_NO_SUCH_PROFILE_EXIST;
no_config_exist:
    return ONVIF_ERR_MEDIA_NO_SUCH_CONFIG_EXIST;
no_audio_supported:
    return ONVIF_ERR_MEDIA_AUDIO_NOT_SUPPORTED;
//config_conflict:
    //return ONVIF_ERR_MEDIA_CONFIGURATION_CONFLICT;
}

ONVIF_RET GK_NVT_Media_AddAudioEncoderConfiguration(GONVIF_MEDIA_AddAudioEncoderConfiguration_S *pstMD_AddAudioEncoderConfiguration)
{
    GK_S32 i = 0;
    GK_S32 j = 0;
    GK_S32 k = 0;
    if(pstMD_AddAudioEncoderConfiguration->aszProfileToken[0] == '\0' || pstMD_AddAudioEncoderConfiguration->aszConfigurationToken[0] == '\0')
    {
        goto invalid_arg;
    }

	if(g_GkIpc_OnvifInf.AudioSourceNum == 0)
	{
		goto no_audio_supported;
	}
    for(i = 0; i < g_GkIpc_OnvifInf.ProfileNum; i++)
	{
	    if(strcmp(pstMD_AddAudioEncoderConfiguration->aszProfileToken, g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken) == 0)
	    {
    		break;
	    }
	}

	if(i == g_GkIpc_OnvifInf.ProfileNum)
	{
		goto no_such_profile;
	}

    for(j = 0; j < g_GkIpc_OnvifInf.AudioEncodeNum; j++)
	{
	    if(strcmp(pstMD_AddAudioEncoderConfiguration->aszConfigurationToken, g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[j].pszToken) == 0)
	    {
    		break;
	    }
	}
	if(j == g_GkIpc_OnvifInf.AudioEncodeNum)
	{
        goto no_config_exist;
	}

    if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstAudioSourceConfiguration != NULL)
    {
    	if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstAudioEncoderConfiguration != NULL)
    	{
    	    for(k = 0; k < g_GkIpc_OnvifInf.AudioEncodeNum; k++)
    		{
    		    if(strcmp(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstAudioEncoderConfiguration->pszToken,g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[k].pszToken) == 0)
    		    {
    				g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[k].useCount--;
    	    		break;
    		    }
    		}
    		g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstAudioEncoderConfiguration = &g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[j];
    		g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[j].useCount++;
    	}
    	else
    	{
    		g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstAudioEncoderConfiguration = &g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[j];
    		g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[j].useCount++;

    	}
    }
	
    GK_NVT_SaveXmlConfig(XML_MEIDA);
    return ONVIF_OK;
    
invalid_arg:
    return ONVIF_ERR_MEDIA_INVAILD_ARG;
no_such_profile:
    return ONVIF_ERR_MEDIA_NO_SUCH_PROFILE_EXIST;
no_config_exist:
    return ONVIF_ERR_MEDIA_NO_SUCH_CONFIG_EXIST;
no_audio_supported:
    return ONVIF_ERR_MEDIA_AUDIO_NOT_SUPPORTED;
//config_conflict:
    //return ONVIF_ERR_MEDIA_CONFIGURATION_CONFLICT;
}

/***************************************************************************
函数描述: 添加云台配置信息
入口参数：pstMD_AddPTZConfiguration : 添加视频编码配置信息
返回值     ：ONVIF_OK: 成功
         		      非零: 失败
***************************************************************************/
ONVIF_RET GK_NVT_Media_AddPTZConfiguration(GONVIF_MEDIA_AddPTZConfiguration_S *pstMD_AddPTZConfiguration)
{
    GK_S32 i = 0;
    GK_S32 j = 0;
    GK_S32 k = 0;
    if(g_GkIpc_OnvifInf.PTZConfigurationNum == 0)
    {
        goto no_PTZ_supported;
    }

    if(pstMD_AddPTZConfiguration->aszProfileToken[0] == '\0' || pstMD_AddPTZConfiguration->aszConfigurationToken[0] == '\0')
    {
        goto invalid_arg;
    }

    for(i = 0; i < g_GkIpc_OnvifInf.ProfileNum; i++)
    {
        if(strcmp(pstMD_AddPTZConfiguration->aszProfileToken, g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken) == 0)
        {
            break;
        }
    }
    if(i == g_GkIpc_OnvifInf.ProfileNum)
    {
        goto no_such_profile;
    }

    for(j = 0; j < g_GkIpc_OnvifInf.PTZConfigurationNum; j++)
    {
        if(strcmp(pstMD_AddPTZConfiguration->aszConfigurationToken, g_GkIpc_OnvifInf.gokePTZConfiguration[j].token) == 0)
        {
            break;
        }
    }
    if(j == g_GkIpc_OnvifInf.PTZConfigurationNum)
    {
        goto no_config_exist;
    }


    if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstPTZConfiguration != NULL)
    {
        for(k = 0; k < g_GkIpc_OnvifInf.PTZConfigurationNum; k++)
        {
            if(strcmp(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstPTZConfiguration->token, g_GkIpc_OnvifInf.gokePTZConfiguration[k].token) == 0)
            {
                g_GkIpc_OnvifInf.gokePTZConfiguration[k].useCount--;
                break;
            }
        }
        g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstPTZConfiguration = &g_GkIpc_OnvifInf.gokePTZConfiguration[j];
        g_GkIpc_OnvifInf.gokePTZConfiguration[j].useCount++;
    }
    else
    {
        g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstPTZConfiguration = &g_GkIpc_OnvifInf.gokePTZConfiguration[j];
        g_GkIpc_OnvifInf.gokePTZConfiguration[j].useCount++;
    }
    
    GK_NVT_SaveXmlConfig(XML_MEIDA);
    return ONVIF_OK;
    
invalid_arg:
    return ONVIF_ERR_MEDIA_INVAILD_ARG;
no_such_profile:
    return ONVIF_ERR_MEDIA_NO_SUCH_PROFILE_EXIST;
no_config_exist:
    return ONVIF_ERR_MEDIA_NO_SUCH_CONFIG_EXIST;
no_PTZ_supported:
    return ONVIF_ERR_MEDIA_NO_PTZ_SUPPORTED;
//config_conflict:
    //return ONVIF_ERR_MEDIA_CONFIGURATION_CONFLICT;
}



/***************************************************************************
函数描述: 删除视频源配置信息
入口参数：pstMD_RemoveVideoSourceConfiguration : 删除的视频源配置信息
返回值     ：ONVIF_OK: 成功
         		      非零: 失败
***************************************************************************/
ONVIF_RET GK_NVT_Media_RemoveVideoSourceConfiguration(GONVIF_MEDIA_RemoveVideoSourceConfiguration_S *pstMD_RemoveVideoSourceConfiguration)
{
    GK_S32 i = 0;
    GK_S32 j = 0;

    if(pstMD_RemoveVideoSourceConfiguration->aszProfileToken[0] == '\0')
    {
        goto invalid_arg;
    }

    for(i = 0; i < g_GkIpc_OnvifInf.ProfileNum; i++)
    {
        if(strcmp(pstMD_RemoveVideoSourceConfiguration->aszProfileToken, g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken) == 0)
        {
            break;
        }
    }
    if(i == g_GkIpc_OnvifInf.ProfileNum)
    {
        goto no_such_profile;
    }

    if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstVideoEncoderConfiguration == NULL)
    {
        if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstVideoSourceConfiguration != NULL)
        {
            for(j = 0; j < g_GkIpc_OnvifInf.VideoSourceNum; j++)
            {
                if(strcmp(g_GkIpc_OnvifInf.gokeIPCameraVideoSource[j].pszToken,g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstVideoSourceConfiguration->pszToken) == 0)
                {
                    g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstVideoSourceConfiguration = NULL;
                    g_GkIpc_OnvifInf.gokeIPCameraVideoSource[j].useCount--;
                    break;
                }
            }
        }
        else
        {
            goto no_config_exist;

        }
    }
    GK_NVT_SaveXmlConfig(XML_MEIDA);
    return ONVIF_OK;
    
invalid_arg:
    return ONVIF_ERR_MEDIA_INVAILD_ARG;
no_such_profile:
    return ONVIF_ERR_MEDIA_NO_SUCH_PROFILE_EXIST;
no_config_exist:
    return ONVIF_ERR_MEDIA_NO_SUCH_CONFIG_EXIST;
//config_conflict:
    //return ONVIF_ERR_MEDIA_CONFIGURATION_CONFLICT;
}

/***************************************************************************
函数描述: 删除视频编码配置信息
入口参数：pstMD_RemoveVideoEncoderConfiguration : 删除的视频编码配置信息
返回值     ：ONVIF_OK: 成功
         		      非零: 失败
***************************************************************************/
ONVIF_RET GK_NVT_Media_RemoveVideoEncoderConfiguration(GONVIF_MEDIA_RemoveVideoEncoderConfiguration_S *pstMD_RemoveVideoEncoderConfiguration)
{
    GK_S32 i = 0;
    GK_S32 j = 0;
    
    if(pstMD_RemoveVideoEncoderConfiguration->aszProfileToken[0] == '\0')
    {
        goto invalid_arg;
    }

    for(i = 0; i < g_GkIpc_OnvifInf.ProfileNum; i++)
    {
        if(strcmp(pstMD_RemoveVideoEncoderConfiguration->aszProfileToken, g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken) == 0)
        {
            break;
        }
    }
    if(i == g_GkIpc_OnvifInf.ProfileNum)
    {
        goto no_such_profile;
    }



    if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstVideoEncoderConfiguration != NULL)
    {
        for(j = 0; j < g_GkIpc_OnvifInf.VideoEncodeNum; j++)
        {
            if(strcmp(g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[j].pszToken,g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstVideoEncoderConfiguration->pszToken) == 0)
            {
                g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstVideoEncoderConfiguration = NULL;
                g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[j].useCount--;
                break;
            }
        }
    }
    else
    {
        goto no_config_exist;

    }
    
    GK_NVT_SaveXmlConfig(XML_MEIDA);
    return ONVIF_OK;
    
invalid_arg:
    return ONVIF_ERR_MEDIA_INVAILD_ARG;
no_such_profile:
    return ONVIF_ERR_MEDIA_NO_SUCH_PROFILE_EXIST;
no_config_exist:
    return ONVIF_ERR_MEDIA_NO_SUCH_CONFIG_EXIST;
//config_conflict:
    //return ONVIF_ERR_MEDIA_CONFIGURATION_CONFLICT;
}

ONVIF_RET GK_NVT_Media_RemoveAudioSourceConfiguration(GONVIF_MEDIA_RemoveAudioSourceConfiguration_S *pstMD_RemoveAudioSourceConfiguration)
{
    GK_S32 i = 0;
    GK_S32 j = 0;
    if(pstMD_RemoveAudioSourceConfiguration->aszProfileToken[0] == '\0')
    {
        goto invalid_arg;
    }
	if(g_GkIpc_OnvifInf.AudioSourceNum == 0)
	{
		goto no_audio_supported;
	}
    for(i = 0; i < g_GkIpc_OnvifInf.ProfileNum; i++)
	{
	    if(strcmp(pstMD_RemoveAudioSourceConfiguration->aszProfileToken, g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken) == 0)
	    {
    		break;
	    }
	}
	if(i == g_GkIpc_OnvifInf.ProfileNum)
	{
		goto no_such_profile;
	}

    if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstAudioEncoderConfiguration == NULL)
    {
    	if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstAudioSourceConfiguration != NULL)
    	{
    	    for(j = 0; j < g_GkIpc_OnvifInf.AudioSourceNum; j++)
    		{
    		    if(strcmp(g_GkIpc_OnvifInf.gokeIPCameraAudioSource[j].pszToken, g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstAudioSourceConfiguration->pszToken) == 0)
    		    {
    				g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstAudioSourceConfiguration = NULL;
    				g_GkIpc_OnvifInf.gokeIPCameraAudioSource[j].useCount--;
    	    		break;
    		    }
    		}
    	}
    	else
    	{
    		goto no_config_exist;

    	}
    }
    
    GK_NVT_SaveXmlConfig(XML_MEIDA);
    return ONVIF_OK;
    
invalid_arg:
    return ONVIF_ERR_MEDIA_INVAILD_ARG;
no_such_profile:
    return ONVIF_ERR_MEDIA_NO_SUCH_PROFILE_EXIST;
no_config_exist:
    return ONVIF_ERR_MEDIA_NO_SUCH_CONFIG_EXIST;
no_audio_supported:
    return ONVIF_ERR_MEDIA_AUDIO_NOT_SUPPORTED;
//config_conflict:
    //return ONVIF_ERR_MEDIA_CONFIGURATION_CONFLICT;

}

ONVIF_RET GK_NVT_Media_RemoveAudioEncoderConfiguration(GONVIF_MEDIA_RemoveAudioEncoderConfiguration_S *pstMD_RemoveAudioEncoderConfiguration)
{
    GK_S32 i = 0;
    GK_S32 j = 0;

    if(pstMD_RemoveAudioEncoderConfiguration->aszProfileToken[0] == '\0')
    {
        goto invalid_arg;
    }
	if(g_GkIpc_OnvifInf.AudioSourceNum == 0)
	{
		goto no_audio_supported;
	}
    for(i = 0; i < g_GkIpc_OnvifInf.ProfileNum; i++)
    {
        if(strcmp(pstMD_RemoveAudioEncoderConfiguration->aszProfileToken, g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken) == 0)
        {
            break;
        }
    }
    if(i == g_GkIpc_OnvifInf.ProfileNum)
    {
        goto no_such_profile;
    }

	if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstAudioEncoderConfiguration != NULL)
	{
	    for(j = 0; j < g_GkIpc_OnvifInf.AudioEncodeNum; j++)
		{
		    if(strcmp(g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[j].pszToken, g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstAudioEncoderConfiguration->pszToken) == 0){
				g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstAudioEncoderConfiguration = NULL;
				g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[j].useCount--;
	    		break;
		    }
		}
	}
	else
	{
		goto no_config_exist;

	}

    GK_NVT_SaveXmlConfig(XML_MEIDA);
    return ONVIF_OK;
    
invalid_arg:
    return ONVIF_ERR_MEDIA_INVAILD_ARG;
no_such_profile:
    return ONVIF_ERR_MEDIA_NO_SUCH_PROFILE_EXIST;
no_config_exist:
    return ONVIF_ERR_MEDIA_NO_SUCH_CONFIG_EXIST;
no_audio_supported:
    return ONVIF_ERR_MEDIA_AUDIO_NOT_SUPPORTED;
//config_conflict:
    //return ONVIF_ERR_MEDIA_CONFIGURATION_CONFLICT;
}


/***************************************************************************
函数描述: 删除云台配置信息
入口参数：pstMD_RemovePTZConfiguration : 删除的云台配置信息
返回值     ：ONVIF_OK: 成功
         		      非零: 失败
***************************************************************************/
ONVIF_RET GK_NVT_Media_RemovePTZConfiguration(GONVIF_MEDIA_RemovePTZConfiguration_S *pstMD_RemovePTZConfiguration)
{
    GK_S32 i = 0;
    GK_S32 j = 0;
    
    if(g_GkIpc_OnvifInf.PTZConfigurationNum == 0)
    {
        goto no_PTZ_supported;
    }

    if(pstMD_RemovePTZConfiguration->aszProfileToken[0] == '\0')
    {
        goto invalid_arg;
    }

    for(i = 0; i < g_GkIpc_OnvifInf.ProfileNum; i++)
    {
        if(strcmp(pstMD_RemovePTZConfiguration->aszProfileToken, g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken) == 0)
        {
            break;
        }
    }
    if(i == g_GkIpc_OnvifInf.ProfileNum)
    {
        goto no_such_profile;
    }

    if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstPTZConfiguration != NULL)
    {
        for(j = 0; j < g_GkIpc_OnvifInf.PTZConfigurationNum; j++)
        {
            if(strcmp(g_GkIpc_OnvifInf.gokePTZConfiguration[j].token, g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstPTZConfiguration->token) == 0)
            {
                g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstPTZConfiguration = NULL;
                g_GkIpc_OnvifInf.gokePTZConfiguration[j].useCount--;
                break;
            }
        }
    }
    else
    {
        goto no_config_exist;

    }
    
    GK_NVT_SaveXmlConfig(XML_MEIDA);
    return ONVIF_OK;
    
invalid_arg:
    return ONVIF_ERR_MEDIA_INVAILD_ARG;
no_such_profile:
    return ONVIF_ERR_MEDIA_NO_SUCH_PROFILE_EXIST;
no_config_exist:
    return ONVIF_ERR_MEDIA_NO_SUCH_CONFIG_EXIST;
no_PTZ_supported:
    return ONVIF_ERR_MEDIA_NO_PTZ_SUPPORTED;
//config_conflict:
    //return ONVIF_ERR_MEDIA_CONFIGURATION_CONFLICT;
}


/***************************************************************************
函数描述: 获取全部视频源配置信息
入口参数：pstMD_SetVideoSourceConfiguration : 	返回的全部视频源
				配置信息	
返回值     ：ONVIF_OK: 成功
         		      非零: 失败
***************************************************************************/
ONVIF_RET GK_NVT_Media_GetVideoSourceConfigurations(GONVIF_MEDIA_GetVideoSourceConfigurations_Res_S *pstMD_GetVideoSourceConfigurationsRes)
{
    GK_S32 i = 0;
    
    pstMD_GetVideoSourceConfigurationsRes->sizeConfigurations = g_GkIpc_OnvifInf.VideoSourceNum;
    for(i = 0; i < g_GkIpc_OnvifInf.VideoSourceNum; i++)
    {
        memcpy(&pstMD_GetVideoSourceConfigurationsRes->stConfigurations[i], &g_GkIpc_OnvifInf.gokeIPCameraVideoSource[i], sizeof(GONVIF_MEDIA_VideoSourceConfiguration_S));
    }
    
	return ONVIF_OK;

}

/***************************************************************************
函数描述: 获取视频源配置信息
入口参数：pstMD_GetVideoSourceConfiguration : 	传入的视频源配置信息
				pstMD_GetVideoSourceConfigurationRes : 返回的视频源配置信息
返回值     ：ONVIF_OK: 成功
         		      非零: 失败
***************************************************************************/
ONVIF_RET GK_NVT_Media_GetVideoSourceConfiguration(GONVIF_MEDIA_GetVideoSourceConfiguration_S *pstMD_GetVideoSourceConfiguration, GONVIF_MEDIA_GetVideoSourceConfiguration_Res_S *pstMD_GetVideoSourceConfigurationRes)
{
    GK_S32 i = 0;
    if(pstMD_GetVideoSourceConfiguration->aszConfigurationToken[0] == '\0')
    {
        goto invalid_arg;
    }
    for(i = 0; i < g_GkIpc_OnvifInf.VideoSourceNum; i++)
    {
        if(strcmp(pstMD_GetVideoSourceConfiguration->aszConfigurationToken, g_GkIpc_OnvifInf.gokeIPCameraVideoSource[i].pszToken) == 0)
        {
            memcpy(&pstMD_GetVideoSourceConfigurationRes->stConfiguration, &g_GkIpc_OnvifInf.gokeIPCameraVideoSource[i], sizeof(GONVIF_MEDIA_VideoSourceConfiguration_S));
            break;
        }
    }
    if(i == g_GkIpc_OnvifInf.VideoSourceNum)
    {
        goto no_config_exist;
    }
    return ONVIF_OK;
    
invalid_arg:
    return ONVIF_ERR_MEDIA_INVAILD_ARG;
no_config_exist:
    return ONVIF_ERR_MEDIA_NO_SUCH_CONFIG_EXIST;

}


/***************************************************************************
函数描述: 获取全部视频源兼容的配置信息
入口参数：pstMD_GetCompatibleVideoSourceConfigurations : 	传入的全部
				视频源兼容配置信息
				pstMD_GetCompatibleVideoSourceConfigurationsRes : 返回的全部
				视频源兼容配置信息	
返回值     ：ONVIF_OK: 成功
         		      非零: 失败
***************************************************************************/
ONVIF_RET GK_NVT_Media_GetCompatibleVideoSourceConfigurations(GONVIF_MEDIA_GetCompatibleVideoSourceConfigurations_S *pstMD_GetCompatibleVideoSourceConfigurations, GONVIF_MEDIA_GetCompatibleVideoSourceConfigurations_Res_S *pstMD_GetCompatibleVideoSourceConfigurationsRes)
{
    GK_S32 i = 0;
    
    if(g_GkIpc_OnvifInf.VideoSourceNum == 0)
    {
        goto no_video_source;
    }

    for(i = 0; i <g_GkIpc_OnvifInf.ProfileNum; i++)
    {
        if(strcmp(pstMD_GetCompatibleVideoSourceConfigurations->aszProfileToken, g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken) == 0)
        {
            break;
        }
    }
    if(i == g_GkIpc_OnvifInf.ProfileNum)
    {
        goto no_such_profile;
    }
    pstMD_GetCompatibleVideoSourceConfigurationsRes->sizeConfigurations = g_GkIpc_OnvifInf.VideoSourceNum;
    for(i = 0; i < g_GkIpc_OnvifInf.VideoSourceNum; i++)
    {
        memcpy(&pstMD_GetCompatibleVideoSourceConfigurationsRes->stConfigurations[i], &g_GkIpc_OnvifInf.gokeIPCameraVideoSource[i], sizeof(GONVIF_MEDIA_VideoSourceConfiguration_S));
    }
    
    return ONVIF_OK;
    
no_such_profile:
    return ONVIF_ERR_MEDIA_NO_SUCH_PROFILE_EXIST;
no_video_source:
    return ONVIF_ERR_MEDIA_NO_VIDEO_SOURCE;
}


/***************************************************************************
函数描述: 获取全部视频源兼容的配置条件信息
入口参数：pstMD_GetVideoSourceConfigurationOptions : 	传入的全部视频
				源配置条件信息
				pstMD_GetVideoSourceConfigurationOptionsRes : 返回的全部视频
				源配置条件信息
返回值     ：ONVIF_OK: 成功
         		      非零: 失败
***************************************************************************/
ONVIF_RET GK_NVT_Media_GetVideoSourceConfigurationOptions(GONVIF_MEDIA_GetVideoSourceConfigurationOptions_S *pstMD_GetVideoSourceConfigurationOptions, GONVIF_MEDIA_GetVideoSourceConfigurationOptions_Res_S *pstMD_GetVideoSourceConfigurationOptionsRes)
{
    GK_S32 i, j;

    if(pstMD_GetVideoSourceConfigurationOptions->aszProfileToken[0] != '\0')
    {
        for(i = 0; i < g_GkIpc_OnvifInf.ProfileNum; i++)
        {
            if(strcmp(pstMD_GetVideoSourceConfigurationOptions->aszProfileToken, g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken) == 0)
            {
                break;
            }
        }
        if(i == g_GkIpc_OnvifInf.ProfileNum)
        {
            goto no_such_profile;
        }
    }
    if(pstMD_GetVideoSourceConfigurationOptions->aszConfigurationToken[0] != '\0')
    {
        for(j = 0; j < g_GkIpc_OnvifInf.VideoSourceNum; j++)
        {
            if(strcmp(pstMD_GetVideoSourceConfigurationOptions->aszConfigurationToken, g_GkIpc_OnvifInf.gokeIPCameraVideoSource[j].pszToken) == 0)
            {
                memcpy(&pstMD_GetVideoSourceConfigurationOptionsRes->stOptions, &g_GkIpc_OnvifInf.gokeVideoSourceConfigurationOption[j], sizeof(GONVIF_MEDIA_VideoSourceConfigurationOptions_S));
                break;
            }
        }
        if(j == g_GkIpc_OnvifInf.VideoSourceNum)
        {
            goto no_config_exist;
        }
    }
    else
    {
        memcpy(&pstMD_GetVideoSourceConfigurationOptionsRes->stOptions, &g_GkIpc_OnvifInf.gokeVideoSourceConfigurationOption[0], sizeof(GONVIF_MEDIA_VideoSourceConfigurationOptions_S));
    }

    return ONVIF_OK;
    
no_such_profile:
    return ONVIF_ERR_MEDIA_NO_SUCH_PROFILE_EXIST;
no_config_exist:
    return ONVIF_ERR_MEDIA_NO_SUCH_CONFIG_EXIST;
}


/***************************************************************************
函数描述: 设置视频源配置信息
入口参数：pstMD_SetVideoSourceConfiguration : 设置的视频源配置信息
返回值     ：ONVIF_OK: 成功
         		      非零: 失败
***************************************************************************/
ONVIF_RET GK_NVT_Media_SetVideoSourceConfiguration(GONVIF_MEDIA_SetVideoSourceConfiguration_S *pstMD_SetVideoSourceConfiguration)
{
    GK_S32 i = 0;
    for(i = 0; i < g_GkIpc_OnvifInf.VideoSourceNum; i++)
    {
        if(strcmp(pstMD_SetVideoSourceConfiguration->stVideoSourceConfiguration.pszToken, g_GkIpc_OnvifInf.gokeIPCameraVideoSource[i].pszToken) == 0 &&
            strcmp(pstMD_SetVideoSourceConfiguration->stVideoSourceConfiguration.pszSourceToken, g_GkIpc_OnvifInf.gokeIPCameraVideoSource[i].pszSourceToken) == 0)
        {
            if(pstMD_SetVideoSourceConfiguration->stVideoSourceConfiguration.stBounds.height < g_GkIpc_OnvifInf.gokeVideoSourceConfigurationOption[i].stBoundsRange.stHeightRange.min ||
               pstMD_SetVideoSourceConfiguration->stVideoSourceConfiguration.stBounds.height >  g_GkIpc_OnvifInf.gokeVideoSourceConfigurationOption[i].stBoundsRange.stHeightRange.max ||
               pstMD_SetVideoSourceConfiguration->stVideoSourceConfiguration.stBounds.width < g_GkIpc_OnvifInf.gokeVideoSourceConfigurationOption[i].stBoundsRange.stWidthRange.min ||
               pstMD_SetVideoSourceConfiguration->stVideoSourceConfiguration.stBounds.width > g_GkIpc_OnvifInf.gokeVideoSourceConfigurationOption[i].stBoundsRange.stWidthRange.max ||
               pstMD_SetVideoSourceConfiguration->stVideoSourceConfiguration.stBounds.x < g_GkIpc_OnvifInf.gokeVideoSourceConfigurationOption[i].stBoundsRange.stXRange.min ||
               pstMD_SetVideoSourceConfiguration->stVideoSourceConfiguration.stBounds.x > g_GkIpc_OnvifInf.gokeVideoSourceConfigurationOption[i].stBoundsRange.stXRange.max ||
               pstMD_SetVideoSourceConfiguration->stVideoSourceConfiguration.stBounds.y < g_GkIpc_OnvifInf.gokeVideoSourceConfigurationOption[i].stBoundsRange.stYRange.min ||
               pstMD_SetVideoSourceConfiguration->stVideoSourceConfiguration.stBounds.y > g_GkIpc_OnvifInf.gokeVideoSourceConfigurationOption[i].stBoundsRange.stYRange.max)
            {
                goto can_not_modify;
            }
			
            g_GkIpc_OnvifInf.gokeIPCameraVideoSource[i].stBounds.height = pstMD_SetVideoSourceConfiguration->stVideoSourceConfiguration.stBounds.height;
            g_GkIpc_OnvifInf.gokeIPCameraVideoSource[i].stBounds.width = pstMD_SetVideoSourceConfiguration->stVideoSourceConfiguration.stBounds.width;
            g_GkIpc_OnvifInf.gokeIPCameraVideoSource[i].stBounds.x = pstMD_SetVideoSourceConfiguration->stVideoSourceConfiguration.stBounds.x;
            g_GkIpc_OnvifInf.gokeIPCameraVideoSource[i].stBounds.y = pstMD_SetVideoSourceConfiguration->stVideoSourceConfiguration.stBounds.y;
            break;
        }
    }

    if(i == g_GkIpc_OnvifInf.VideoSourceNum)
    {
        goto no_config_exist;
    }
    
    GK_NVT_SaveXmlConfig(XML_MEIDA);
    return ONVIF_OK;
    
can_not_modify:
    return ONVIF_ERR_MEDIA_CONFIG_CANNOT_MODIFY;
no_config_exist:
    return ONVIF_ERR_MEDIA_NO_SUCH_CONFIG_EXIST;
//config_conflict:
    //return ONVIF_ERR_MEDIA_CONFIGURATION_CONFLICT;
}


/***************************************************************************
函数描述:  获取全部视频编码配置信息
入口参数：pstMD_GetVideoEncoderConfigurationsRes : 全部视频编码配置信息
返回值     ：ONVIF_OK: 成功
         		      ONVIF_ERR_MEDIA_SERVER_RUN_ERROR: 失败
***************************************************************************/
ONVIF_RET GK_NVT_Media_GetVideoEncoderConfigurations(GONVIF_MEDIA_GetVideoEncoderConfigurations_Res_S *pstMD_GetVideoEncoderConfigurationsRes)
{
    GK_S32 ret;
    GK_S32 i = 0;
    ret = media_GetVideoEncoderConfigurations(g_GkIpc_OnvifInf.gokeIPCameraVideoEncode);
    if(ret != 0)
    {
        goto run_error;
    }

    pstMD_GetVideoEncoderConfigurationsRes->sizeConfigurations = g_GkIpc_OnvifInf.VideoEncodeNum;
    for(i = 0; i < g_GkIpc_OnvifInf.VideoEncodeNum; i++)
    {
       memcpy(&pstMD_GetVideoEncoderConfigurationsRes->stConfigurations[i], &g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i], sizeof(GONVIF_MEDIA_VideoEncoderConfiguration_S));
    }

    ONVIF_DBG("GetVideoEncoderConfigurations Name:%s,Token:%s,UseCount:%d,Encoding:%d,Width:%d,Height:%d,Quality:%f,BitrateLimit:%d,EncodingInterval:%d,FrameRateLimit:%d\n",
        pstMD_GetVideoEncoderConfigurationsRes->stConfigurations[i].pszName,
        pstMD_GetVideoEncoderConfigurationsRes->stConfigurations[i].pszToken,
        pstMD_GetVideoEncoderConfigurationsRes->stConfigurations[i].useCount,
        pstMD_GetVideoEncoderConfigurationsRes->stConfigurations[i].enEncoding,
        pstMD_GetVideoEncoderConfigurationsRes->stConfigurations[i].stResolution.width,
        pstMD_GetVideoEncoderConfigurationsRes->stConfigurations[i].stResolution.height,
        pstMD_GetVideoEncoderConfigurationsRes->stConfigurations[i].quality,
        pstMD_GetVideoEncoderConfigurationsRes->stConfigurations[i].stRateControl.bitrateLimit,
        pstMD_GetVideoEncoderConfigurationsRes->stConfigurations[i].stRateControl.encodingInterval,
        pstMD_GetVideoEncoderConfigurationsRes->stConfigurations[i].stRateControl.frameRateLimit);

    ONVIF_DBG("GovLength:%d,H264Profile:%d,Port:%d,TTL:%d,AutoStart:%d,SessionTimeout:%lld\n",
        pstMD_GetVideoEncoderConfigurationsRes->stConfigurations[i].stH264.govLength,
        pstMD_GetVideoEncoderConfigurationsRes->stConfigurations[i].stH264.enH264Profile,
        pstMD_GetVideoEncoderConfigurationsRes->stConfigurations[i].stMulticast.port,
        pstMD_GetVideoEncoderConfigurationsRes->stConfigurations[i].stMulticast.ttl,
        pstMD_GetVideoEncoderConfigurationsRes->stConfigurations[i].stMulticast.enAutoStart,
        pstMD_GetVideoEncoderConfigurationsRes->stConfigurations[i].sessionTimeout);
        
    return ONVIF_OK;
    
run_error:
    return ONVIF_ERR_MEDIA_SERVER_RUN_ERROR;;
}

ONVIF_RET GK_NVT_Media_GetVideoEncoderConfiguration(GONVIF_MEDIA_GetVideoEncoderConfiguration_S *pstMD_GetVideoEncoderConfiguration, GONVIF_MEDIA_GetVideoEncoderConfiguration_Res_S *pstMD_GetVideoEncoderConfigurationRes)
{
    GK_S32 i = 0;
    GK_S32 ret;
    ret = media_GetVideoEncoderConfigurations(g_GkIpc_OnvifInf.gokeIPCameraVideoEncode);
    if(ret != 0)
    {
        ONVIF_DBG("Get Video Encoder Configuration failed.\n");
        goto run_error;
    }
    for(i = 0; i < g_GkIpc_OnvifInf.VideoEncodeNum; i++)
    {
        if(strcmp(pstMD_GetVideoEncoderConfiguration->aszConfigurationToken, g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].pszToken) == 0)
        {
            memcpy(&pstMD_GetVideoEncoderConfigurationRes->stConfiguration, &g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i], sizeof(GONVIF_MEDIA_VideoEncoderConfiguration_S));
            ONVIF_DBG("GetVideoEncoderConfiguration Name:%s,Token:%s,UseCount:%d,Encoding:%d,Width:%d,Height:%d,Quality:%f,BitrateLimit:%d,EncodingInterval:%d,FrameRateLimit:%d\n",
                    pstMD_GetVideoEncoderConfigurationRes->stConfiguration.pszName,
                    pstMD_GetVideoEncoderConfigurationRes->stConfiguration.pszToken,
                    pstMD_GetVideoEncoderConfigurationRes->stConfiguration.useCount,
                    pstMD_GetVideoEncoderConfigurationRes->stConfiguration.enEncoding,
                    pstMD_GetVideoEncoderConfigurationRes->stConfiguration.stResolution.width,
                    pstMD_GetVideoEncoderConfigurationRes->stConfiguration.stResolution.height,
                    pstMD_GetVideoEncoderConfigurationRes->stConfiguration.quality,
                    pstMD_GetVideoEncoderConfigurationRes->stConfiguration.stRateControl.bitrateLimit,
                    pstMD_GetVideoEncoderConfigurationRes->stConfiguration.stRateControl.encodingInterval,
                    pstMD_GetVideoEncoderConfigurationRes->stConfiguration.stRateControl.frameRateLimit);


            ONVIF_DBG("GovLength:%d,H264Profile:%d,Port:%d,TTL:%d,AutoStart:%d,SessionTimeout:%lld\n",
                    pstMD_GetVideoEncoderConfigurationRes->stConfiguration.stH264.govLength,
                    pstMD_GetVideoEncoderConfigurationRes->stConfiguration.stH264.enH264Profile,
                    pstMD_GetVideoEncoderConfigurationRes->stConfiguration.stMulticast.port,
                    pstMD_GetVideoEncoderConfigurationRes->stConfiguration.stMulticast.ttl,
                    pstMD_GetVideoEncoderConfigurationRes->stConfiguration.stMulticast.enAutoStart,
                    pstMD_GetVideoEncoderConfigurationRes->stConfiguration.sessionTimeout);
            break;

        }
    }

    if(i == g_GkIpc_OnvifInf.VideoEncodeNum)
    {
        goto no_such_config;
    }
    return ONVIF_OK;
    
run_error:
    return ONVIF_ERR_MEDIA_SERVER_RUN_ERROR;;
no_such_config:
    return ONVIF_ERR_MEDIA_NO_SUCH_CONFIG_EXIST;
}

/***************************************************************************
函数描述:  获取全部兼容的视频编码配置信息
入口参数：pstMD_GetCompatibleVideoEncoderConfigurations : 传入全部兼容
				视频编码配置的信息
				pstMD_GetCompatibleVideoEncoderConfigurationsRes : 返回全部兼
				容视频编码配置的信息	
返回值     ：ONVIF_OK: 成功
         		      非零: 失败
***************************************************************************/
ONVIF_RET GK_NVT_Media_GetCompatibleVideoEncoderConfigurations(GONVIF_MEDIA_GetCompatibleVideoEncoderConfigurations_S *pstMD_GetCompatibleVideoEncoderConfigurations, GONVIF_MEDIA_GetCompatibleVideoEncoderConfigurations_Res_S *pstMD_GetCompatibleVideoEncoderConfigurationsRes)
{
    GK_S32 i = 0;
    if(g_GkIpc_OnvifInf.VideoEncodeNum == 0)
    {
        goto no_video_encoder;
    }

    for(i = 0; i <g_GkIpc_OnvifInf.ProfileNum; i++)
    {
        if(strcmp(pstMD_GetCompatibleVideoEncoderConfigurations->aszProfileToken, g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken) == 0)
        {
            break;
        }
    }
    if(i == g_GkIpc_OnvifInf.ProfileNum)
    {
        goto no_such_profile;
    }

    pstMD_GetCompatibleVideoEncoderConfigurationsRes->sizeConfigurations = g_GkIpc_OnvifInf.VideoEncodeNum;
    for(i = 0; i < g_GkIpc_OnvifInf.VideoEncodeNum; i++)
    {
        memcpy(&pstMD_GetCompatibleVideoEncoderConfigurationsRes->stConfigurations[i], &g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i], sizeof(GONVIF_MEDIA_VideoEncoderConfiguration_S));
    }
    return ONVIF_OK;

no_video_encoder:
    return ONVIF_ERR_MEDIA_NO_VIDEO_ENCODER;
no_such_profile:
    return ONVIF_ERR_MEDIA_NO_SUCH_PROFILE_EXIST;

}

/***************************************************************************
函数描述:  获取全部视频编码配置条件信息
入口参数：pstMD_GetVideoEncoderConfigurationOptions : 传入的全部视频
				编码配置条件信息
				pstMD_GetVideoEncoderConfigurationOptionsRes : 返回的全部视
				频编码配置条件信息
返回值     ：ONVIF_OK: 成功
         		      非零: 失败
***************************************************************************/
ONVIF_RET GK_NVT_Media_GetVideoEncoderConfigurationOptions(GONVIF_MEDIA_GetVideoEncoderConfigurationOptions_S *pstMD_GetVideoEncoderConfigurationOptions, GONVIF_MEDIA_GetVideoEncoderConfigurationOptions_Res_S *pstMD_GetVideoEncoderConfigurationOptionsRes)
{
    GK_S32 i = 0;
    GK_S32 j = 0;
    
    if(pstMD_GetVideoEncoderConfigurationOptions->aszProfileToken[0] != '\0')
    {
        for(i = 0; i < g_GkIpc_OnvifInf.ProfileNum; i++)
        {
            if(strcmp(pstMD_GetVideoEncoderConfigurationOptions->aszProfileToken, g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken) == 0)
            {
                break;
            }
        }
        if(i == g_GkIpc_OnvifInf.ProfileNum)
        {
            goto no_such_profile;
        }
    }
    
    if(pstMD_GetVideoEncoderConfigurationOptions->aszConfigurationToken[0] != '\0')
    {
        for(j = 0; j < g_GkIpc_OnvifInf.VideoEncodeNum; j++)
        {
            if(strcmp(pstMD_GetVideoEncoderConfigurationOptions->aszConfigurationToken, g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[j].pszToken) == 0)
            {
                memcpy(&pstMD_GetVideoEncoderConfigurationOptionsRes->stOptions, &g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[j], sizeof(GONVIF_MEDIA_VideoEncoderConfigurationOptions_S));
                break;
            }
        }
        if(j == g_GkIpc_OnvifInf.VideoEncodeNum)
        {
            goto no_such_config;
        }
    }
    else
    {
        memcpy(&pstMD_GetVideoEncoderConfigurationOptionsRes->stOptions, &g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[0], sizeof(GONVIF_MEDIA_VideoEncoderConfigurationOptions_S));
    }
    
    return ONVIF_OK;
    
no_such_profile:
    return ONVIF_ERR_MEDIA_NO_SUCH_PROFILE_EXIST;
no_such_config:
    return ONVIF_ERR_MEDIA_NO_SUCH_CONFIG_EXIST;
}

ONVIF_RET GK_NVT_Media_SetVideoEncoderConfiguration(GONVIF_MEDIA_SetVideoEncoderConfiguration_S *pstMD_SetVideoEncoderConfiguration)
{
    GK_S32 i = 0;
    GK_S32 j = 0;
    GK_BOOL isTypeChange = GK_FALSE;
    GK_S32 retVal = 0;
    ONVIF_DBG("Name:%s,Token:%s,UseCount:%d,Quality:%f\n",
        pstMD_SetVideoEncoderConfiguration->stConfiguration.pszName,
        pstMD_SetVideoEncoderConfiguration->stConfiguration.pszToken,
        pstMD_SetVideoEncoderConfiguration->stConfiguration.useCount,
        pstMD_SetVideoEncoderConfiguration->stConfiguration.quality);

    ONVIF_DBG("GovLength:%d,H264Profile:%d,Width:%d,Height:%d,BitrateLimit:%d,FrameRateLimit:%d,EncodingInterval:%d,Encode:%d,Force:%d\n",
        pstMD_SetVideoEncoderConfiguration->stConfiguration.stH264.govLength,
        pstMD_SetVideoEncoderConfiguration->stConfiguration.stH264.enH264Profile,
        pstMD_SetVideoEncoderConfiguration->stConfiguration.stResolution.width,
        pstMD_SetVideoEncoderConfiguration->stConfiguration.stResolution.height,
        pstMD_SetVideoEncoderConfiguration->stConfiguration.stRateControl.bitrateLimit,
        pstMD_SetVideoEncoderConfiguration->stConfiguration.stRateControl.frameRateLimit,
        pstMD_SetVideoEncoderConfiguration->stConfiguration.stRateControl.encodingInterval,
        pstMD_SetVideoEncoderConfiguration->stConfiguration.enEncoding,
        pstMD_SetVideoEncoderConfiguration->enForcePersistence);

    ONVIF_DBG("AddrType:%d,Port:%d,TTL:%d,AutoStart:%d\n",
        pstMD_SetVideoEncoderConfiguration->stConfiguration.stMulticast.stIPAddress.enType,
        pstMD_SetVideoEncoderConfiguration->stConfiguration.stMulticast.port,
        pstMD_SetVideoEncoderConfiguration->stConfiguration.stMulticast.ttl,
        pstMD_SetVideoEncoderConfiguration->stConfiguration.stMulticast.enAutoStart);

    retVal = media_GetVideoEncoderConfigurations(g_GkIpc_OnvifInf.gokeIPCameraVideoEncode);
    if(retVal != 0)
    {
        ONVIF_ERR("Get video encoder configurations failed.");
        goto run_error;
    }
    for(i = 0; i < g_GkIpc_OnvifInf.VideoEncodeNum; i++)
    {
        if(strcmp(pstMD_SetVideoEncoderConfiguration->stConfiguration.pszToken, g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].pszToken) == 0)
        {  
            if(pstMD_SetVideoEncoderConfiguration->stConfiguration.stResolution.width != g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].stResolution.width ||
               pstMD_SetVideoEncoderConfiguration->stConfiguration.stResolution.height != g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].stResolution.height ||
               pstMD_SetVideoEncoderConfiguration->stConfiguration.enEncoding != g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].enEncoding)
            {
                if(pstMD_SetVideoEncoderConfiguration->stConfiguration.enEncoding == VideoEncoding_JPEG)
                {
                    for(j = 0; j < g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stJPEG.sizeResolutionsAvailable; j++)
                    {
                        if(pstMD_SetVideoEncoderConfiguration->stConfiguration.stResolution.width == g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stJPEG.pstResolutionsAvailable[j].width &&
                            pstMD_SetVideoEncoderConfiguration->stConfiguration.stResolution.height == g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stJPEG.pstResolutionsAvailable[j].height)
                        {
                            break;
                        }
                    }
                    if(j == g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stJPEG.sizeResolutionsAvailable)
                    {
                        goto can_not_modify;
                    }
                }
                else if(pstMD_SetVideoEncoderConfiguration->stConfiguration.enEncoding == VideoEncoding_MPEG)
                {
                    goto can_not_modify;
                }
                else if(pstMD_SetVideoEncoderConfiguration->stConfiguration.enEncoding == VideoEncoding_H264)
                {
                    for(j = 0; j < g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stH264.sizeResolutionsAvailable; j++)
                    {
                        if(pstMD_SetVideoEncoderConfiguration->stConfiguration.stResolution.width == g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stH264.pstResolutionsAvailable[j].width &&
                            pstMD_SetVideoEncoderConfiguration->stConfiguration.stResolution.height == g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stH264.pstResolutionsAvailable[j].height)
                        {
                            break;
                        }
                    }
                    if(j == g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stH264.sizeResolutionsAvailable)
                    {
                        goto can_not_modify;
                    }
                }
                else
                {
                    goto invalid_arg;
                }
                retVal = g_stMediaAdapter.SetResolution(i, pstMD_SetVideoEncoderConfiguration->stConfiguration.stResolution.width, pstMD_SetVideoEncoderConfiguration->stConfiguration.stResolution.height, pstMD_SetVideoEncoderConfiguration->stConfiguration.enEncoding);
                if(retVal != 0)
                {
                    ONVIF_ERR("Set resolution failed.");
                    goto run_error;
                }
                if(pstMD_SetVideoEncoderConfiguration->stConfiguration.enEncoding != g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].enEncoding)
                {
                    isTypeChange = GK_TRUE;
                }
                g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].stResolution.width = pstMD_SetVideoEncoderConfiguration->stConfiguration.stResolution.width;
                g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].stResolution.height = pstMD_SetVideoEncoderConfiguration->stConfiguration.stResolution.height;
                g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].enEncoding = pstMD_SetVideoEncoderConfiguration->stConfiguration.enEncoding;
            }

            if(pstMD_SetVideoEncoderConfiguration->stConfiguration.stRateControl.frameRateLimit != g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].stRateControl.frameRateLimit)
            {
                if(pstMD_SetVideoEncoderConfiguration->stConfiguration.enEncoding == VideoEncoding_JPEG)
                {
                    if(pstMD_SetVideoEncoderConfiguration->stConfiguration.stRateControl.frameRateLimit < g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stJPEG.stFrameRateRange.min ||
                        pstMD_SetVideoEncoderConfiguration->stConfiguration.stRateControl.frameRateLimit > g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stJPEG.stFrameRateRange.max)
                    {
                        goto can_not_modify;
                    }
                }
                else if(pstMD_SetVideoEncoderConfiguration->stConfiguration.enEncoding == VideoEncoding_MPEG)
                {
                    goto can_not_modify;
                }
                else if(pstMD_SetVideoEncoderConfiguration->stConfiguration.enEncoding == VideoEncoding_H264)
                {
                    if(pstMD_SetVideoEncoderConfiguration->stConfiguration.stRateControl.frameRateLimit < g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stH264.stFrameRateRange.min ||
                        pstMD_SetVideoEncoderConfiguration->stConfiguration.stRateControl.frameRateLimit > g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stH264.stFrameRateRange.max)
                    {
                        goto can_not_modify;
                    }
                }
                else
                {
                    goto invalid_arg;
                }
                retVal = g_stMediaAdapter.SetFrameRate(i, pstMD_SetVideoEncoderConfiguration->stConfiguration.stRateControl.frameRateLimit);
                if(retVal != 0)
                {
                    ONVIF_ERR("Set frame rate failed.");
                    goto run_error;
                }
                g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].stRateControl.frameRateLimit = pstMD_SetVideoEncoderConfiguration->stConfiguration.stRateControl.frameRateLimit;
            }

            if(pstMD_SetVideoEncoderConfiguration->stConfiguration.stRateControl.bitrateLimit != g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].stRateControl.bitrateLimit)
            {
             
                retVal = g_stMediaAdapter.SetBitRate(i,pstMD_SetVideoEncoderConfiguration->stConfiguration.stRateControl.bitrateLimit);
                if(retVal != 0)
                {                       
                    ONVIF_ERR("Set bitrate failed.");
                    goto run_error;
                }
                g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].stRateControl.bitrateLimit = pstMD_SetVideoEncoderConfiguration->stConfiguration.stRateControl.bitrateLimit;
            }

            if(pstMD_SetVideoEncoderConfiguration->stConfiguration.stRateControl.encodingInterval != g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].stRateControl.encodingInterval)
            {
                if(pstMD_SetVideoEncoderConfiguration->stConfiguration.enEncoding == VideoEncoding_JPEG)
                {
                    if(pstMD_SetVideoEncoderConfiguration->stConfiguration.stRateControl.encodingInterval < g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stJPEG.stEncodingIntervalRange.min ||
                        pstMD_SetVideoEncoderConfiguration->stConfiguration.stRateControl.encodingInterval > g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stJPEG.stEncodingIntervalRange.max)
                    {
                        goto can_not_modify;
                    }
                }
                else if(pstMD_SetVideoEncoderConfiguration->stConfiguration.enEncoding == VideoEncoding_MPEG)
                {
                    goto can_not_modify;
                }
                else if(pstMD_SetVideoEncoderConfiguration->stConfiguration.enEncoding == VideoEncoding_H264)
                {
                    if(pstMD_SetVideoEncoderConfiguration->stConfiguration.stRateControl.encodingInterval < g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stH264.stEncodingIntervalRange.min ||
                        pstMD_SetVideoEncoderConfiguration->stConfiguration.stRateControl.encodingInterval > g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stH264.stEncodingIntervalRange.max)
                    {
                        goto can_not_modify;
                    }
                }
                else
                {
                    goto invalid_arg;
                }
                retVal = g_stMediaAdapter.SetInterval(i,pstMD_SetVideoEncoderConfiguration->stConfiguration.stRateControl.encodingInterval);
                if(retVal != 0)
                {
                    goto can_not_modify;
                }
                g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].stRateControl.encodingInterval = pstMD_SetVideoEncoderConfiguration->stConfiguration.stRateControl.encodingInterval;
            }

            if(pstMD_SetVideoEncoderConfiguration->stConfiguration.quality != g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].quality)
            {
                if(pstMD_SetVideoEncoderConfiguration->stConfiguration.quality < g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stQualityRange.min ||
                    pstMD_SetVideoEncoderConfiguration->stConfiguration.quality > g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stQualityRange.max)
                {
                    goto can_not_modify;
                }
                retVal = g_stMediaAdapter.SetQuality(i,pstMD_SetVideoEncoderConfiguration->stConfiguration.quality);
             
                g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].quality = pstMD_SetVideoEncoderConfiguration->stConfiguration.quality;
            }
            if(pstMD_SetVideoEncoderConfiguration->stConfiguration.enEncoding == VideoEncoding_H264 && isTypeChange == GK_FALSE)
            {
                if(pstMD_SetVideoEncoderConfiguration->stConfiguration.stH264.govLength != g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].stH264.govLength)
                {
                    if(pstMD_SetVideoEncoderConfiguration->stConfiguration.stH264.govLength < g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stH264.stGovLengthRange.min||
                        pstMD_SetVideoEncoderConfiguration->stConfiguration.stH264.govLength > g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stH264.stGovLengthRange.max)
                    {
                        goto can_not_modify;
                    }
                
                    retVal = g_stMediaAdapter.SetGovLength(i,pstMD_SetVideoEncoderConfiguration->stConfiguration.stH264.govLength);
                    if(retVal != 0)
                    {
                        goto can_not_modify;
                    }
                    g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].stH264.govLength = pstMD_SetVideoEncoderConfiguration->stConfiguration.stH264.govLength;
                }
            }
        
            break;
        }
    }
    if(i == g_GkIpc_OnvifInf.VideoEncodeNum)
    {
        goto no_config_exist;
    }

    if(pstMD_SetVideoEncoderConfiguration->stConfiguration.stMulticast.stIPAddress.pszIPv4Address != NULL)
    {
        g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].stMulticast.port = pstMD_SetVideoEncoderConfiguration->stConfiguration.stMulticast.port;
        
    	strncpy(g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].stMulticast.stIPAddress.pszIPv4Address, 
    	    pstMD_SetVideoEncoderConfiguration->stConfiguration.stMulticast.stIPAddress.pszIPv4Address, IPV4_STR_LENGTH - 1);
    	    
        media_SendMuticastInforToRtsp((GK_U32)inet_addr(pstMD_SetVideoEncoderConfiguration->stConfiguration.stMulticast.stIPAddress.pszIPv4Address),
            pstMD_SetVideoEncoderConfiguration->stConfiguration.stMulticast.port, 1, 1);
    }
    
    GK_NVT_SaveXmlConfig(XML_MEIDA);
    g_stMediaAdapter.SaveCfg();
    return ONVIF_OK;
    
invalid_arg:
    return ONVIF_ERR_MEDIA_INVAILD_ARG;
run_error:
    return ONVIF_ERR_MEDIA_SERVER_RUN_ERROR;    
can_not_modify:
    return ONVIF_ERR_MEDIA_CONFIG_CANNOT_MODIFY;
no_config_exist:
    return ONVIF_ERR_MEDIA_NO_SUCH_CONFIG_EXIST;
//config_conflict:
    //return ONVIF_ERR_MEDIA_CONFIGURATION_CONFLICT;

}

ONVIF_RET GK_NVT_Media_GetGuaranteedNumberOfVideoEncoderInstances(GONVIF_MEDIA_GetGuaranteedNumberOfVideoEncoderInstances_S *pstMD_GetGuaranteedNumberOfVideoEncoderInstances, GONVIF_MEDIA_GetGuaranteedNumberOfVideoEncoderInstances_Res_S *pstMD_GetGuaranteedNumberOfVideoEncoderInstancesRes)
{
    GK_S32 i = 0;
    if(pstMD_GetGuaranteedNumberOfVideoEncoderInstances->aszConfigurationToken[0] != '\0')
    {
        for(i = 0; i < g_GkIpc_OnvifInf.VideoSourceNum; i++)
        {
            if(strcmp(pstMD_GetGuaranteedNumberOfVideoEncoderInstances->aszConfigurationToken, g_GkIpc_OnvifInf.gokeIPCameraVideoSource[i].pszToken) == 0)
            {
                break;
            }
        }
    }
    if(i == g_GkIpc_OnvifInf.VideoSourceNum)
    {
        goto no_such_config;
    }
    pstMD_GetGuaranteedNumberOfVideoEncoderInstancesRes->totalNumber = g_GkIpc_OnvifInf.VideoEncodeNum;

    for(i = 0; i < g_GkIpc_OnvifInf.VideoEncodeNum; i++)
    {
        if(g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].enEncoding == VideoEncoding_JPEG)
        {
            pstMD_GetGuaranteedNumberOfVideoEncoderInstancesRes->JPEG++;
        }
        else if(g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].enEncoding == VideoEncoding_MPEG)
        {
            pstMD_GetGuaranteedNumberOfVideoEncoderInstancesRes->MPEG4++;
        }
        else if(g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].enEncoding == VideoEncoding_H264)
        {
            pstMD_GetGuaranteedNumberOfVideoEncoderInstancesRes->H264++;
        }
    }
    
    return ONVIF_OK;

no_such_config:
    return ONVIF_ERR_MEDIA_NO_SUCH_CONFIG_EXIST;

}


ONVIF_RET GK_NVT_Media_GetAudioSourceConfigurations(GONVIF_MEDIA_GetAudioSourceConfigurations_Res_S *pstMD_GetAudioSourceConfigurationRes)
{
    GK_S32 i = 0;
    pstMD_GetAudioSourceConfigurationRes->sizeConfigurations = g_GkIpc_OnvifInf.AudioSourceNum;
    for(i = 0; i < g_GkIpc_OnvifInf.AudioSourceNum; i++)
    {
        memcpy(&pstMD_GetAudioSourceConfigurationRes->stConfigurations[i], &g_GkIpc_OnvifInf.gokeIPCameraAudioSource[i], sizeof(GONVIF_MEDIA_AudioSourceConfiguration_S));
    }
	return ONVIF_OK;
}

ONVIF_RET GK_NVT_Media_GetAudioSourceConfiguration(GONVIF_MEDIA_GetAudioSourceConfiguration_S *pstMD_GetAudioSourceConfiguration, GONVIF_MEDIA_GetAudioSourceConfiguration_Res_S *pstMD_GetAudioSourceConfigurationRes)
{
    GK_S32 i = 0;
	if(g_GkIpc_OnvifInf.AudioSourceNum == 0)
	{
		goto audio_not_supported;
	}

    if(pstMD_GetAudioSourceConfiguration->aszConfigurationToken[0] == '\0')
    {
        goto invalid_arg;
    }

    for(i = 0; i < g_GkIpc_OnvifInf.AudioSourceNum; i++)
    {
        if(strcmp(pstMD_GetAudioSourceConfiguration->aszConfigurationToken, g_GkIpc_OnvifInf.gokeIPCameraAudioSource[i].pszToken) == 0)
        {
            memcpy(&pstMD_GetAudioSourceConfigurationRes->stConfiguration, &g_GkIpc_OnvifInf.gokeIPCameraAudioSource[i], sizeof(GONVIF_MEDIA_AudioSourceConfiguration_S));
            break;
        }
    }
	if(i == g_GkIpc_OnvifInf.AudioSourceNum)
	{
		goto no_config_exist;
	}
    return ONVIF_OK;

audio_not_supported:
    return ONVIF_ERR_MEDIA_AUDIO_NOT_SUPPORTED;
invalid_arg:
    return ONVIF_ERR_MEDIA_INVAILD_ARG;
no_config_exist:
    return ONVIF_ERR_MEDIA_NO_SUCH_CONFIG_EXIST;

}

ONVIF_RET GK_NVT_Media_GetCompatibleAudioSourceConfigurations(GONVIF_MEDIA_GetCompatibleAudioSourceConfigurations_S *pstMD_GetCompatibleAudioSourceConfigurations, GONVIF_MEDIA_GetCompatibleAudioSourceConfigurations_Res_S *pstMD_GetCompatibleAudioSourceConfigurationsRes)
{
    GK_S32 i = 0;
	if(g_GkIpc_OnvifInf.AudioSourceNum == 0)
	{
		goto audio_not_supported;
	}

    for(i = 0; i <g_GkIpc_OnvifInf.ProfileNum; i++)
    {
        if(strcmp(pstMD_GetCompatibleAudioSourceConfigurations->aszProfileToken, g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken) == 0)
        {
            break;
        }
    }
    if(i == g_GkIpc_OnvifInf.ProfileNum)
    {
        goto no_such_profile;
    }
    pstMD_GetCompatibleAudioSourceConfigurationsRes->sizeConfigurations = g_GkIpc_OnvifInf.AudioSourceNum;
    for(i = 0; i < g_GkIpc_OnvifInf.AudioSourceNum; i++)
    {
        memcpy(&pstMD_GetCompatibleAudioSourceConfigurationsRes->stConfigurations[i], &g_GkIpc_OnvifInf.gokeIPCameraAudioSource[i], sizeof(GONVIF_MEDIA_AudioSourceConfiguration_S));
    }
    return ONVIF_OK;

audio_not_supported:
    return ONVIF_ERR_MEDIA_AUDIO_NOT_SUPPORTED;
no_such_profile:
    return ONVIF_ERR_MEDIA_NO_SUCH_PROFILE_EXIST;
}

ONVIF_RET GK_NVT_Media_GetAudioSourceConfigurationOptions(GONVIF_MEDIA_GetAudioSourceConfigurationOptions_S *pstMD_GetAudioSourceConfigurationOptions, GONVIF_MEDIA_GetAudioSourceConfigurationOptions_Res_S *pstMD_GetAudioSourceConfigurationOptionsRes)
{
    GK_S32 i, j;

    if(pstMD_GetAudioSourceConfigurationOptions->aszProfileToken[0] != '\0')
    {
        for(i = 0; i < g_GkIpc_OnvifInf.ProfileNum; i++)
        {
            if(strcmp(pstMD_GetAudioSourceConfigurationOptions->aszProfileToken, g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken) == 0)
            {
                break;
            }
        }
        if(i == g_GkIpc_OnvifInf.ProfileNum)
        {
            goto no_such_profile;
        }
    }
    if(pstMD_GetAudioSourceConfigurationOptions->aszConfigurationToken[0] != '\0')
    {
        for(j = 0; j < g_GkIpc_OnvifInf.AudioSourceNum; j++)
        {
            if(strcmp(pstMD_GetAudioSourceConfigurationOptions->aszConfigurationToken, g_GkIpc_OnvifInf.gokeIPCameraAudioSource[j].pszToken) == 0)
            {
                memcpy(&pstMD_GetAudioSourceConfigurationOptionsRes->stOptions, &g_GkIpc_OnvifInf.gokeAudioSourceConfigurationOption[j], sizeof(GONVIF_MEDIA_AudioSourceConfigurationOptions_S));
                break;
            }
        }
        if(j == g_GkIpc_OnvifInf.AudioSourceNum)
        {
            goto no_config_exist;
        }
    }
    else
    {     
        memcpy(&pstMD_GetAudioSourceConfigurationOptionsRes->stOptions, &g_GkIpc_OnvifInf.gokeAudioSourceConfigurationOption[0], sizeof(GONVIF_MEDIA_AudioSourceConfigurationOptions_S));
    }

    return ONVIF_OK;
    
no_such_profile:
    return ONVIF_ERR_MEDIA_NO_SUCH_PROFILE_EXIST;
no_config_exist:
    return ONVIF_ERR_MEDIA_NO_SUCH_CONFIG_EXIST;
}

ONVIF_RET GK_NVT_Media_SetAudioSourceConfiguration(GONVIF_MEDIA_SetAudioSourceConfiguration_S *pstMD_SetAudioSourceConfiguration)
{
    ONVIF_DBG("SetAudioSourceConfiguration	Name:%s,Token:%s,SourceToken:%s,UseCount:%d,Force:%d\n",
        pstMD_SetAudioSourceConfiguration->stConfiguration.pszName,
        pstMD_SetAudioSourceConfiguration->stConfiguration.pszToken,
        pstMD_SetAudioSourceConfiguration->stConfiguration.pszSourceToken,
        pstMD_SetAudioSourceConfiguration->stConfiguration.useCount,
        pstMD_SetAudioSourceConfiguration->enForcePersistence);
        
	GK_S32 i = 0;
	if(g_GkIpc_OnvifInf.AudioSourceNum == 0)
	{
		goto audio_not_supported;
	}
	for(i = 0; i < g_GkIpc_OnvifInf.AudioSourceNum; i++)
	{
		if(strcmp(pstMD_SetAudioSourceConfiguration->stConfiguration.pszToken, g_GkIpc_OnvifInf.gokeIPCameraAudioSource[i].pszToken) == 0 &&
            strcmp(pstMD_SetAudioSourceConfiguration->stConfiguration.pszSourceToken, g_GkIpc_OnvifInf.gokeIPCameraAudioSource[i].pszSourceToken) == 0)
		{
		    //check value
		    //set value
		    //sava value
			break;
		}
	}

	if(i == g_GkIpc_OnvifInf.AudioSourceNum)
	{
		goto no_config_exist;
	}
	
    return ONVIF_OK;
    
audio_not_supported:
    return ONVIF_ERR_MEDIA_AUDIO_NOT_SUPPORTED;
no_config_exist:
    return ONVIF_ERR_MEDIA_NO_SUCH_CONFIG_EXIST;
    
}


ONVIF_RET GK_NVT_Media_GetAudioEncoderConfigurations(GONVIF_MEDIA_GetAudioEncoderConfigurations_Res_S *pstMD_GetAudioEncoderConfigurationsRes)
{
    GK_S32 i = 0;
    GK_S32 ret = 0;
	if(g_GkIpc_OnvifInf.AudioSourceNum == 0)
	{
		goto audio_not_supported;
	}
    ret = media_GetAudioEncoderConfigurations(g_GkIpc_OnvifInf.gokeIPCameraAudioEncode);
    if(ret != 0)
    {
        ONVIF_DBG("Get Audio Encoder Configuration failed.");
        goto run_error;
    }
    pstMD_GetAudioEncoderConfigurationsRes->sizeConfigurations = g_GkIpc_OnvifInf.AudioEncodeNum;
    for(i = 0; i < g_GkIpc_OnvifInf.AudioEncodeNum; i++)
    {
       memcpy(&pstMD_GetAudioEncoderConfigurationsRes->stConfigurations[i], &g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i], sizeof(GONVIF_MEDIA_AudioEncoderConfiguration_S));
    }
    return ONVIF_OK;
    
audio_not_supported:
    return ONVIF_ERR_MEDIA_AUDIO_NOT_SUPPORTED;
run_error:
    return ONVIF_ERR_MEDIA_SERVER_RUN_ERROR;
}

ONVIF_RET GK_NVT_Media_GetAudioEncoderConfiguration(GONVIF_MEDIA_GetAudioEncoderConfiguration_S *pstMD_GetAudioEncoderConfiguration, GONVIF_MEDIA_GetAudioEncoderConfiguration_Res_S *pstMD_GetAudioEncoderConfigurationRes)
{
    GK_S32 i = 0;
    GK_S32 ret = 0;
	if(g_GkIpc_OnvifInf.AudioEncodeNum == 0)
	{
		goto audio_not_supported;
	}
    ret = media_GetAudioEncoderConfigurations(g_GkIpc_OnvifInf.gokeIPCameraAudioEncode);
    if(ret != 0)
    {
        ONVIF_DBG("Get Audio Encoder Configuration failed.");
        goto run_error;
    }
    for(i = 0; i < g_GkIpc_OnvifInf.AudioEncodeNum; i++)
    {
        if(strcmp(pstMD_GetAudioEncoderConfiguration->aszConfigurationToken, g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i].pszToken) == 0)
        {
            memcpy(&pstMD_GetAudioEncoderConfigurationRes->stConfiguration, &g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i], sizeof(GONVIF_MEDIA_AudioEncoderConfiguration_S));
            break;
        }
    }
	if(i == g_GkIpc_OnvifInf.AudioEncodeNum)
	{
		goto no_config_exist;
	}
	
	return ONVIF_OK;
	
audio_not_supported:
    return ONVIF_ERR_MEDIA_AUDIO_NOT_SUPPORTED;
no_config_exist:
    return ONVIF_ERR_MEDIA_NO_SUCH_CONFIG_EXIST;
run_error:
    return ONVIF_ERR_MEDIA_SERVER_RUN_ERROR;
}

ONVIF_RET GK_NVT_Media_GetCompatibleAudioEncoderConfigurations(GONVIF_MEDIA_GetCompatibleAudioEncoderConfigurations_S *pstMD_GetCompatibleAudioEncoderConfigurations, GONVIF_MEDIA_GetCompatibleAudioEncoderConfigurations_Res_S *pstMD_GetCompatibleAudioEncoderConfigurationsRes)
{
    GK_S32 i = 0;
	if(g_GkIpc_OnvifInf.AudioEncodeNum == 0)
	{
		goto audio_not_supported;
	}

    for(i = 0; i <g_GkIpc_OnvifInf.ProfileNum; i++)
    {
        if(strcmp(pstMD_GetCompatibleAudioEncoderConfigurations->aszProfileToken,g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken) == 0)
        {
            break;
        }
    }
    if(i == g_GkIpc_OnvifInf.ProfileNum)
    {
        goto no_such_profile;
    }

    pstMD_GetCompatibleAudioEncoderConfigurationsRes->sizeConfigurations = g_GkIpc_OnvifInf.AudioEncodeNum;
    for(i = 0; i < g_GkIpc_OnvifInf.AudioEncodeNum; i++)
    {
        memcpy(&pstMD_GetCompatibleAudioEncoderConfigurationsRes->stConfigurations[i], &g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i], sizeof(GONVIF_MEDIA_AudioEncoderConfiguration_S));
    }
    
    return ONVIF_OK;
    
audio_not_supported:
    return ONVIF_ERR_MEDIA_AUDIO_NOT_SUPPORTED;
no_such_profile:
    return ONVIF_ERR_MEDIA_NO_SUCH_PROFILE_EXIST;
}

ONVIF_RET GK_NVT_Media_GetAudioEncoderConfigurationOptions(GONVIF_MEDIA_GetAudioEncoderConfigurationOptions_S *pstMD_GetAudioEncoderConfigurationOptions, GONVIF_MEDIA_GetAudioEncoderConfigurationOptions_Res_S *pstMD_GetAudioEncoderConfigurationOptionsRes)
{
    GK_S32 i = 0;
	if(g_GkIpc_OnvifInf.AudioEncodeNum == 0)
	{
		goto audio_not_supported;
	}

    if(pstMD_GetAudioEncoderConfigurationOptions->aszProfileToken[0] != '\0')
    {
        for(i = 0; i < g_GkIpc_OnvifInf.ProfileNum; i++)
        {
            if(strcmp(pstMD_GetAudioEncoderConfigurationOptions->aszProfileToken, g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken) == 0)
            {
                break;
            }
        }
        if(i == g_GkIpc_OnvifInf.ProfileNum)
        {
            goto no_such_profile;
        }
    }
    if(pstMD_GetAudioEncoderConfigurationOptions->aszConfigurationToken[0] != '\0')
    {
        for(i = 0; i < g_GkIpc_OnvifInf.AudioEncodeNum; i++)
        {
            if(strcmp(pstMD_GetAudioEncoderConfigurationOptions->aszConfigurationToken, g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i].pszToken) == 0)
            {
                memcpy(&pstMD_GetAudioEncoderConfigurationOptionsRes->stOptions, &g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption[i], sizeof(GONVIF_MEDIA_AudioEncoderConfigurationOptions_S));
                break;
            }
        }
        if(i == g_GkIpc_OnvifInf.AudioEncodeNum)
        {
            goto no_config_exist;
        }
    }
    else
    {
        memcpy(&pstMD_GetAudioEncoderConfigurationOptionsRes->stOptions, &g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption[0], sizeof(GONVIF_MEDIA_AudioEncoderConfigurationOptions_S));
    }
    
    return ONVIF_OK;
    
audio_not_supported:
    return ONVIF_ERR_MEDIA_AUDIO_NOT_SUPPORTED;
no_such_profile:
    return ONVIF_ERR_MEDIA_NO_SUCH_PROFILE_EXIST;
no_config_exist:
    return ONVIF_ERR_MEDIA_NO_SUCH_CONFIG_EXIST;

}

ONVIF_RET GK_NVT_Media_SetAudioEncoderConfiguration(GONVIF_MEDIA_SetAudioEncoderConfiguration_S *pstMD_SetAudioEncoderConfiguration)
{
    ONVIF_DBG("SetAudioEncoderConfiguration	Name:%s,token:%s,Encoding:%d,Bitrate:%d,SampleRate:%d,AddrType:%d,Port:%d,TTL:%d,AutoStart:%d,SessionTimeout:%lld,UseCount:%d,Force:%d\n",
            pstMD_SetAudioEncoderConfiguration->stConfiguration.pszName,
            pstMD_SetAudioEncoderConfiguration->stConfiguration.pszToken,
            pstMD_SetAudioEncoderConfiguration->stConfiguration.enEncoding,
			pstMD_SetAudioEncoderConfiguration->stConfiguration.bitRate,
			pstMD_SetAudioEncoderConfiguration->stConfiguration.sampleRate,
			pstMD_SetAudioEncoderConfiguration->stConfiguration.stMulticast.stIPAddress.enType,
			pstMD_SetAudioEncoderConfiguration->stConfiguration.stMulticast.port,
			pstMD_SetAudioEncoderConfiguration->stConfiguration.stMulticast.ttl,
			pstMD_SetAudioEncoderConfiguration->stConfiguration.stMulticast.enAutoStart,
			pstMD_SetAudioEncoderConfiguration->stConfiguration.sessionTimeout,
            pstMD_SetAudioEncoderConfiguration->stConfiguration.useCount,
            pstMD_SetAudioEncoderConfiguration->enForcePersistence);
    GK_S32 ret = 0;
	if(g_GkIpc_OnvifInf.AudioEncodeNum == 0)
	{
		goto audio_not_supported;
	}
    ret = media_GetAudioEncoderConfigurations(g_GkIpc_OnvifInf.gokeIPCameraAudioEncode);
    if(ret != 0)
    {
        ONVIF_ERR("Get Audio Encoder Configuration failed.");
        goto run_error;
    }
	GK_S32 i = 0, j = 0, k = 0;
	for(i = 0; i < g_GkIpc_OnvifInf.AudioEncodeNum; i++)
	{
		if(strcmp(pstMD_SetAudioEncoderConfiguration->stConfiguration.pszToken, g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i].pszToken) == 0)
		{
			break;
		}
	}

	if(i == g_GkIpc_OnvifInf.AudioEncodeNum)
	{
		goto no_config_exist;
	}
	
	if(pstMD_SetAudioEncoderConfiguration->stConfiguration.enEncoding != g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i].enEncoding)
	{ 
	    for(j = 0; j < g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption[i].sizeOptions; j++)
	    {
            if(pstMD_SetAudioEncoderConfiguration->stConfiguration.enEncoding == g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption[i].pstOptions[j].enEncoding)
                break;
        } 
        if(j == g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption[i].sizeOptions)
        {
            ONVIF_ERR("unsupported Audio encoding type.");
            goto can_not_modify;
        }
        ret = g_stMediaAdapter.SetAudioEncodingType(pstMD_SetAudioEncoderConfiguration->stConfiguration.enEncoding);
        if(ret != 0)
        {
            ONVIF_ERR("Set audio encoding type failed.");
            goto run_error;
        }
    
	}
	
	if(pstMD_SetAudioEncoderConfiguration->stConfiguration.bitRate != g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i].bitRate)
	{  
        for(k = 0; k < g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption[i].pstOptions[j].stBitrateList.sizeItems; k++)
        {
    		if(pstMD_SetAudioEncoderConfiguration->stConfiguration.bitRate == g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption[i].pstOptions[j].stBitrateList.pItems[k])
                break;
		}
        if(k == g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption[i].pstOptions[j].stBitrateList.sizeItems)
        {
            goto can_not_modify;
        }
        else
        {
            ret = g_stMediaAdapter.SetAudioBitrate(pstMD_SetAudioEncoderConfiguration->stConfiguration.bitRate);
            if(ret != 0)
            {
                ONVIF_ERR("Set audio bitrate failed.");
                goto run_error;
            }
        }
	}

	if(pstMD_SetAudioEncoderConfiguration->stConfiguration.sampleRate != g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i].sampleRate)
	{
        for(k = 0; k < g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption[i].pstOptions[j].stSampleRateList.sizeItems; k++)
        {
    		if(pstMD_SetAudioEncoderConfiguration->stConfiguration.sampleRate == g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption[i].pstOptions[j].stSampleRateList.pItems[k])
                break;
		}
        if(k == g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption[i].pstOptions[j].stSampleRateList.sizeItems)
        {
            goto can_not_modify;
        }
        else
        {
            ret = g_stMediaAdapter.SetAudioSampleRate(pstMD_SetAudioEncoderConfiguration->stConfiguration.sampleRate);
            if(ret != 0)
            {
                ONVIF_ERR("Set audio sample rate failed.");
                goto run_error;
            }
        }
        
	}
    if(pstMD_SetAudioEncoderConfiguration->stConfiguration.stMulticast.stIPAddress.pszIPv4Address != NULL)
    {
        g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i].stMulticast.port = pstMD_SetAudioEncoderConfiguration->stConfiguration.stMulticast.port;
    	strncpy(g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i].stMulticast.stIPAddress.pszIPv4Address,
            pstMD_SetAudioEncoderConfiguration->stConfiguration.stMulticast.stIPAddress.pszIPv4Address, IPV4_STR_LENGTH - 1);
        media_SendMuticastInforToRtsp(inet_addr(pstMD_SetAudioEncoderConfiguration->stConfiguration.stMulticast.stIPAddress.pszIPv4Address), pstMD_SetAudioEncoderConfiguration->stConfiguration.stMulticast.port, 2, GK_TRUE);
    }
    
    GK_NVT_SaveXmlConfig(XML_MEIDA);
    g_stMediaAdapter.SaveCfg();
    return ONVIF_OK;
    
audio_not_supported:
    return ONVIF_ERR_MEDIA_AUDIO_NOT_SUPPORTED;
can_not_modify:
    return ONVIF_ERR_MEDIA_CONFIG_CANNOT_MODIFY;
no_config_exist:
    return ONVIF_ERR_MEDIA_NO_SUCH_CONFIG_EXIST;
//config_conflict:
    //return ONVIF_ERR_MEDIA_CONFIGURATION_CONFLICT;
run_error:
    return ONVIF_ERR_MEDIA_SERVER_RUN_ERROR;
}

/***************************************************************************
函数描述: 获取码流服务地址
入口参数：pstMD_GetStreamUri : 传入的码流地址信息
				pstMD_GetStreamUriRes : 返回的媒体地址信息		
返回值     ：ONVIF_OK: 成功
         		      非零: 失败
***************************************************************************/
ONVIF_RET GK_NVT_Media_GetStreamUri(GONVIF_MEDIA_GetStreamUri_S *pstMD_GetStreamUri, GONVIF_MEDIA_GetStreamUri_Res_S *pstMD_GetStreamUriRes)
{
    GK_S32 i = 0,j = 0;
    GK_S8 StreamUri[MAX_URI_LENGTH];
    memset(StreamUri, 0, sizeof(StreamUri));
    if(pstMD_GetStreamUri->aszProfileToken[0] != '\0')
    {
        for(i = 0; i < g_GkIpc_OnvifInf.ProfileNum; i++)
        {
            if(strcmp(pstMD_GetStreamUri->aszProfileToken, g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken) == 0)
            {
                break;
            }
        }
        if(i == g_GkIpc_OnvifInf.ProfileNum)
        {
            goto no_such_profile;
        }
    }
    if(pstMD_GetStreamUri->stStreamSetup.enStream != StreamType_RTP_Unicast && pstMD_GetStreamUri->stStreamSetup.enStream != StreamType_RTP_Multicast &&
        pstMD_GetStreamUri->stStreamSetup.stTransport.enProtocol != TransportProtocol_UDP && pstMD_GetStreamUri->stStreamSetup.stTransport.enProtocol != TransportProtocol_TCP &&
        pstMD_GetStreamUri->stStreamSetup.stTransport.enProtocol != TransportProtocol_RTSP && pstMD_GetStreamUri->stStreamSetup.stTransport.enProtocol != TransportProtocol_HTTP)
    {
        goto invalid_StreamSetup;
    }

    if(pstMD_GetStreamUri->aszProfileToken[0] != '\0')
    {
        if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstVideoEncoderConfiguration != NULL)
        {
            for(j = 0; j < g_GkIpc_OnvifInf.VideoEncodeNum; j++)
            {
                if(strcmp(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstVideoEncoderConfiguration->pszToken, g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[j].pszToken) == 0)
                {
                    if(pstMD_GetStreamUri->stStreamSetup.enStream == StreamType_RTP_Unicast)
                    {
                        //sprintf(_IPAddr, "rtsp://%s/%s?unicast=true", g_GkIpc_OnvifInf.discov.hostip, stream[j]);//stream  url
                        g_stMediaAdapter.GetStreamUri(j, StreamType_RTP_Unicast, StreamUri, MAX_URI_LENGTH);
                    }
                    else
                    {
                        //sprintf(StreamUri, "rtsp://%s/%s?unicast=false", g_GkIpc_OnvifInf.discov.hostip, stream[j]);//stream  url
                        g_stMediaAdapter.GetStreamUri(j, StreamType_RTP_Multicast, StreamUri, MAX_URI_LENGTH);
                    }
                    break;
                }
            }
            if(j == g_GkIpc_OnvifInf.VideoEncodeNum)
            {
                goto incomplete_config;
            }
        }
        else
        {
            if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstAudioEncoderConfiguration != NULL)
            {
                for(j = 0; j < g_GkIpc_OnvifInf.AudioEncodeNum; j++)
                {
                    if(strcmp(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstAudioEncoderConfiguration->pszToken,g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[j].pszToken) == 0)
                    {
                        if(pstMD_GetStreamUri->stStreamSetup.enStream == StreamType_RTP_Unicast)
                        {
                            //sprintf(StreamUri, "rtsp://%s/%s?unicast=true", g_GkIpc_OnvifInf.discov.hostip, stream[j]);//stream  url
                            g_stMediaAdapter.GetStreamUri(j, StreamType_RTP_Unicast, StreamUri, MAX_URI_LENGTH);
                        }
                        else
                        {
                            //sprintf(StreamUri, "rtsp://%s/%s?unicast=false", g_GkIpc_OnvifInf.discov.hostip, stream[j]);//stream  url
                            g_stMediaAdapter.GetStreamUri(j ,StreamType_RTP_Multicast, StreamUri, MAX_URI_LENGTH);
                        }
                        break;
                    }
                }
                if(j == g_GkIpc_OnvifInf.AudioEncodeNum)
                {
                    goto incomplete_config;
                }
            }
        }
    }
    else
    {
        
        if(pstMD_GetStreamUri->stStreamSetup.enStream == StreamType_RTP_Unicast)
        {
            //sprintf(StreamUri, "rtsp://%s/%s?unicast=true", g_GkIpc_OnvifInf.discov.hostip, stream[0]);//stream0 channel1 url            
            g_stMediaAdapter.GetStreamUri(0, StreamType_RTP_Unicast, StreamUri, MAX_URI_LENGTH);
        }
        else
        {
            //sprintf(StreamUri, "rtsp://%s/%s?unicast=false", g_GkIpc_OnvifInf.discov.hostip, stream[0]);//stream0 channel1 url
            g_stMediaAdapter.GetStreamUri(0, StreamType_RTP_Multicast, StreamUri, MAX_URI_LENGTH);
        }
    }

    strncpy(pstMD_GetStreamUriRes->stMediaUri.aszUri, StreamUri, MAX_URI_LENGTH - 1);
    pstMD_GetStreamUriRes->stMediaUri.enInvalidAfterConnect = Boolean_TRUE;
    pstMD_GetStreamUriRes->stMediaUri.enInvalidAfterReboot = Boolean_TRUE;
    pstMD_GetStreamUriRes->stMediaUri.timeout = 5;
    
    return ONVIF_OK;
    
no_such_profile:
    return ONVIF_ERR_MEDIA_NO_SUCH_PROFILE_EXIST;
invalid_StreamSetup:
    return ONVIF_ERR_MEDIA_INVALID_STREAMSETUP;
incomplete_config:
    return ONVIF_ERR_MEDIA_INCOMPLETE_CONFIG;
//stream_conflict:
    //return ONVIF_ERR_MEDIA_STREAM_CONFLICT;
//invalid_multicast:
    //return ONVIF_ERR_MEDIA_INVALID_MULTICAST_SETTINGS;

}

/***************************************************************************
函数描述: 获取快照服务地址
入口参数：pstMD_GetSnapShotUri : 传入的快照地址信息
				pstMD_GetSnapShotUriRes : 返回的媒体地址信息
返回值     ：ONVIF_OK: 成功
         		      非零: 失败
***************************************************************************/
ONVIF_RET GK_NVT_Media_GetSnapshotUri(GONVIF_MEIDA_GetSnapShotUri_S *pstMD_GetSnapShotUri, GONVIF_MEIDA_GetSnapShotUri_Res_S *pstMD_GetSnapShotUriRes)
{
    GK_S32 ret;
    GK_S32 i;
    GK_S8 ImageIPAddr[MAX_INFO_LENGTH];
    if(pstMD_GetSnapShotUri->aszProfileToken[0] != '\0')
    {
        for(i = 0; i < g_GkIpc_OnvifInf.ProfileNum; i++)
        {
            if(strcmp(pstMD_GetSnapShotUri->aszProfileToken, g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken) == 0)
            {
                break;
            }
        }
        if(i == g_GkIpc_OnvifInf.ProfileNum)
        {
            goto no_such_profile;
        }
        if(g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstVideoSourceConfiguration == NULL || g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pstVideoEncoderConfiguration == NULL)
        {
            goto incomplete_config;
        }
    }
	ret = g_stMediaAdapter.Snapshot();
	if(ret != 0)
	{
        goto run_error;
	}
    memset(ImageIPAddr, 0, sizeof(ImageIPAddr));
    sprintf(ImageIPAddr, "http://%s/snapshot/onvif.jpg", g_GkIpc_OnvifInf.discov.hostip);
    strncpy(pstMD_GetSnapShotUriRes->stMediaUri.aszUri, ImageIPAddr, MAX_URI_LENGTH - 1);
    pstMD_GetSnapShotUriRes->stMediaUri.enInvalidAfterConnect = Boolean_FALSE;
    pstMD_GetSnapShotUriRes->stMediaUri.enInvalidAfterReboot  = Boolean_FALSE;
    pstMD_GetSnapShotUriRes->stMediaUri.timeout = 5;
    return ONVIF_OK;
    
no_such_profile:
    return ONVIF_ERR_MEDIA_NO_SUCH_PROFILE_EXIST;
run_error:
    return ONVIF_ERR_MEDIA_SERVER_RUN_ERROR;
incomplete_config:
    return ONVIF_ERR_MEDIA_INCOMPLETE_CONFIG;
}

ONVIF_RET GK_NVT_Media_GetOSDOptions(GONVIF_MEDIA_GetOSDOptions_Token_S *pstGetOSDOptionsReq, GONVIF_MEDIA_GetOSDOptions_S *pstGetOSDOptionsRes)
{
    if ((pstGetOSDOptionsReq->aszConfigurationToken == NULL) || (strcmp(pstGetOSDOptionsReq->aszConfigurationToken, g_GkIpc_OnvifInf.gokeIPCameraVideoSource[0].pszToken) != 0))
    {
        return ONVIF_ERR_IMAGE_INVALID_VIDEO_SOURCE_TOKEN;
    }
	//memcpy(&(pstGetOSDOptionsRes->stOSDOptions), g_GkIpc_OnvifInf.gokeOSDOptions, sizeof(pstGetOSDOptionsRes->stOSDOptions));
    pstGetOSDOptionsRes->stOSDOptions.stMaximumNumberOfOSDs.Total = 10;
    pstGetOSDOptionsRes->stOSDOptions.stMaximumNumberOfOSDs.Image = 4;
    pstGetOSDOptionsRes->stOSDOptions.stMaximumNumberOfOSDs.PlainText = 5;
    pstGetOSDOptionsRes->stOSDOptions.stMaximumNumberOfOSDs.Date = 1;
    pstGetOSDOptionsRes->stOSDOptions.stMaximumNumberOfOSDs.Time = 1;
    pstGetOSDOptionsRes->stOSDOptions.stMaximumNumberOfOSDs.DateAndTime = 1;

    pstGetOSDOptionsRes->stOSDOptions.stType = OSDType_Text;
    int i;
    pstGetOSDOptionsRes->stOSDOptions.sizePositionOption = 3;
    for(i = 0;i < pstGetOSDOptionsRes->stOSDOptions.sizePositionOption; i++)
    {
        pstGetOSDOptionsRes->stOSDOptions.penPositionOption[i] = PositionOption_Custom+i;
    }
    pstGetOSDOptionsRes->stOSDOptions.stTextOption.sizeType = 4;
    for(i = 0;i < pstGetOSDOptionsRes->stOSDOptions.stTextOption.sizeType; i++)
    {
        pstGetOSDOptionsRes->stOSDOptions.stTextOption.penType[i] = TextType_Plain+i;
    }
    pstGetOSDOptionsRes->stOSDOptions.stTextOption.FontSizeRange.min = 16;
    pstGetOSDOptionsRes->stOSDOptions.stTextOption.FontSizeRange.max = 64;
    
    pstGetOSDOptionsRes->stOSDOptions.stTextOption.sizeDateFormat = 4;
    for(i = 0;i < pstGetOSDOptionsRes->stOSDOptions.stTextOption.sizeDateFormat; i++)
    {
        pstGetOSDOptionsRes->stOSDOptions.stTextOption.penDateFormat[i] = DateFormat_MMddyyyy+i;
    }
    
    pstGetOSDOptionsRes->stOSDOptions.stTextOption.sizeTimeFormat = 2;
    for(i = 0;i < pstGetOSDOptionsRes->stOSDOptions.stTextOption.sizeTimeFormat; i++)
    {
        pstGetOSDOptionsRes->stOSDOptions.stTextOption.penTimeFormat[i] = TimeFormat_HHmmss+i;
    }
    pstGetOSDOptionsRes->stOSDOptions.stTextOption.stFontColor.stColor.stColorspaceRange.stX.min = 0;
    pstGetOSDOptionsRes->stOSDOptions.stTextOption.stFontColor.stColor.stColorspaceRange.stX.max = 255;
    pstGetOSDOptionsRes->stOSDOptions.stTextOption.stFontColor.stColor.stColorspaceRange.stY.min = 0;
    pstGetOSDOptionsRes->stOSDOptions.stTextOption.stFontColor.stColor.stColorspaceRange.stY.max = 255;
    pstGetOSDOptionsRes->stOSDOptions.stTextOption.stFontColor.stColor.stColorspaceRange.stZ.min = 0;
    pstGetOSDOptionsRes->stOSDOptions.stTextOption.stFontColor.stColor.stColorspaceRange.stZ.max = 255;
    strcpy(pstGetOSDOptionsRes->stOSDOptions.stTextOption.stFontColor.stColor.stColorspaceRange.stColorspace, "http://www.onvif.org/ver10/colorspace/YCbCr");

    
    return ONVIF_OK;
}

ONVIF_RET GK_NVT_Media_GetOSDs(GONVIF_MEDIA_GetOSDs_Token_S *pstGetOSDsReq, GONVIF_MEDIA_GetOSDs_S *pstGetOSDsRes)
{
    int i=0, osds_flag = 0x3;
    if ((pstGetOSDsReq->aszConfigurationToken == NULL) || (strcmp(pstGetOSDsReq->aszConfigurationToken, g_GkIpc_OnvifInf.gokeIPCameraVideoSource[0].pszToken) != 0))
    {
        if(strcmp(pstGetOSDsReq->aszConfigurationToken, "OsdTokenName")==0)
            osds_flag = 0x2;
        else if(strcmp(pstGetOSDsReq->aszConfigurationToken, "OsdTokenTime")==0)
            osds_flag = 0x1;
        else
            return ONVIF_ERR_IMAGE_INVALID_VIDEO_SOURCE_TOKEN;
    }
    GONVIF_MEDIA_OSDs_S *pstOSD = NULL;
	ONVIF_OsdInfor_S stOsdRevInfo;
    memset(&stOsdRevInfo, 0, sizeof(ONVIF_OsdInfor_S));
    if(g_stMediaAdapter.GetOsdInfor(&stOsdRevInfo) != 0)
    {
    	ONVIF_ERR("Osd server run error.");
        return ONVIF_ERR_IMAGE_SERVER_RUN_ERROR;
    }
    pstGetOSDsRes->sizeOSDs = 0;
    if(osds_flag&0x1)
    {
        pstOSD = &(pstGetOSDsRes->stOSDs[pstGetOSDsRes->sizeOSDs]);
        strcpy(pstOSD->aszOsdToken, "OsdTokenTime");
        strcpy(pstOSD->aszConfigurationToken, pstGetOSDsReq->aszConfigurationToken);
        //strcpy(pstGetOSDsRes->stOSDs[0].aszConfigurationToken, g_GkIpc_OnvifInf.gokeIPCameraVideoSource[0].pszSourceToken);
        pstOSD->stType = OSDType_Text;
        pstOSD->stPosition.penType = PositionOption_Custom;
        pstOSD->stPosition.stPos.x = 2*stOsdRevInfo.x-1;
        pstOSD->stPosition.stPos.y = 1-2*stOsdRevInfo.y;
        pstOSD->stTextString.penType = TextType_DateAndTime;
        GONVIF_MEDIA_DateFormat_E DateFormat;
        if(0==stOsdRevInfo.dateFormat)
            DateFormat = DateFormat_yyyy_MM_dd;
        else if(3==stOsdRevInfo.dateFormat||1==stOsdRevInfo.dateFormat)
            DateFormat = DateFormat_MMddyyyy;
        else if(5==stOsdRevInfo.dateFormat||4==stOsdRevInfo.dateFormat)
            DateFormat = DateFormat_ddMMyyyy;
        else if(2==stOsdRevInfo.dateFormat)
            DateFormat = DateFormat_yyyyMMdd;
        else
            DateFormat = DateFormat_yyyy_MM_dd;
        pstOSD->stTextString.stType.stDateAndTime.penDateFormat = DateFormat;
        pstOSD->stTextString.stType.stDateAndTime.penTimeFormat = stOsdRevInfo.timeFmt?TimeFormat_hhmmsstt: TimeFormat_HHmmss;
        pstOSD->stTextString.FontSize = 32;
        pstOSD->stTextString.stExtension.enChannelName = Boolean_FALSE;
        i = 1;
        pstGetOSDsRes->sizeOSDs++;
    }
    if(osds_flag&0x2)
    {
        pstOSD = &(pstGetOSDsRes->stOSDs[pstGetOSDsRes->sizeOSDs]);
        strcpy(pstOSD->aszOsdToken, "OsdTokenName");
        strcpy(pstOSD->aszConfigurationToken, pstGetOSDsReq->aszConfigurationToken);
        //strcpy(pstGetOSDsRes->stOSDs[1].aszConfigurationToken, g_GkIpc_OnvifInf.gokeIPCameraVideoSource[0].pszSourceToken);
        pstOSD->stType = OSDType_Text;
        pstOSD->stPosition.penType = PositionOption_Custom;
        pstOSD->stPosition.stPos.x = 2*stOsdRevInfo.chx-1;
        pstOSD->stPosition.stPos.y = 1-2*stOsdRevInfo.chy;
        pstOSD->stTextString.penType = TextType_Plain;
        strcpy(pstOSD->stTextString.stType.stPlain.aszPlainText, stOsdRevInfo.chName);
        pstOSD->stTextString.FontSize = 32;
        pstOSD->stTextString.stExtension.enChannelName = stOsdRevInfo.chEnable?Boolean_TRUE : Boolean_FALSE;
        pstGetOSDsRes->sizeOSDs++;
    }
    return ONVIF_OK;
}

ONVIF_RET GK_NVT_Media_SetOSD(GONVIF_MEDIA_SetOSD_S *pstSetOSDReq)
{
    if ((pstSetOSDReq->stOSD.aszConfigurationToken == NULL) || (strcmp(pstSetOSDReq->stOSD.aszConfigurationToken, g_GkIpc_OnvifInf.gokeIPCameraVideoSource[0].pszToken) != 0))
    {
        return ONVIF_ERR_IMAGE_INVALID_VIDEO_SOURCE_TOKEN;
    }
    float x=0, y=0;
	ONVIF_OsdInfor_S stOsdRevInfo;
    memset(&stOsdRevInfo, 0, sizeof(ONVIF_OsdInfor_S));
    if(g_stMediaAdapter.GetOsdInfor(&stOsdRevInfo) != 0)
    {
    	ONVIF_ERR("Osd server run error.");
        return ONVIF_ERR_IMAGE_SERVER_RUN_ERROR;
    }
    if(OSDType_Text == pstSetOSDReq->stOSD.stType)
    {
        if(PositionOption_Custom == pstSetOSDReq->stOSD.stPosition.penType)
        {
            x = (1+pstSetOSDReq->stOSD.stPosition.stPos.x)/2;
            y = (1-pstSetOSDReq->stOSD.stPosition.stPos.y)/2;
        }
        else if(PositionOption_UpperLeft == pstSetOSDReq->stOSD.stPosition.penType)
        {
            x = 0;
            y = 0;
        }
        else if(PositionOption_LowerLeft == pstSetOSDReq->stOSD.stPosition.penType)
        {
            x = 0;
            y = 0.7;
        }

        if(TextType_Plain == pstSetOSDReq->stOSD.stTextString.penType)
        {
            stOsdRevInfo.chx = x;
            stOsdRevInfo.chy = y;
            stOsdRevInfo.chEnable = pstSetOSDReq->stOSD.stTextString.stExtension.enChannelName;
            strcpy(stOsdRevInfo.chName, pstSetOSDReq->stOSD.stTextString.stType.stPlain.aszPlainText);
        }
        else
        {
            stOsdRevInfo.x = x;
            stOsdRevInfo.y = y;
            stOsdRevInfo.enable = 1;
            if(TextType_Date & pstSetOSDReq->stOSD.stTextString.penType)
            {
                switch(pstSetOSDReq->stOSD.stTextString.stType.stDateAndTime.penDateFormat)
                {
                    case DateFormat_yyyy_MM_dd:
                        stOsdRevInfo.dateFormat = 0;
                        break;
                    case DateFormat_MMddyyyy:
                        stOsdRevInfo.dateFormat = 3; // 1
                        break;
                    case DateFormat_ddMMyyyy:
                        stOsdRevInfo.dateFormat = 5; // 4
                        break;
                    case DateFormat_yyyyMMdd:
                        stOsdRevInfo.dateFormat = 2;
                        break;
                }
            }
            if(TextType_Time & pstSetOSDReq->stOSD.stTextString.penType)
                stOsdRevInfo.timeFmt = pstSetOSDReq->stOSD.stTextString.stType.stDateAndTime.penTimeFormat?1:0;
        }
    }
    else
    {

    }
    if(g_stMediaAdapter.SetOsdInfor(&stOsdRevInfo) != 0)
    {
        ONVIF_ERR("Osd server run error.");
        return ONVIF_ERR_IMAGE_SERVER_RUN_ERROR;
    }    
    return ONVIF_OK;
}

ONVIF_RET GK_NVT_Media_DeleteOSD(GONVIF_MEDIA_DeleteOSD_S *pstDeleteOSDReq)
{
	ONVIF_OsdInfor_S stOsdRevInfo;
    memset(&stOsdRevInfo, 0, sizeof(ONVIF_OsdInfor_S));
    if(g_stMediaAdapter.GetOsdInfor(&stOsdRevInfo) != 0)
    {
    	ONVIF_ERR("Osd server run error.");
        return ONVIF_ERR_IMAGE_SERVER_RUN_ERROR;
    }
    if(0 == strcmp(pstDeleteOSDReq->aszOsdToken, "OsdTokenName"))
    {
        stOsdRevInfo.chEnable = 0;
    }
    else if(0 == strcmp(pstDeleteOSDReq->aszOsdToken, "OsdTokenTime"))
    {
        stOsdRevInfo.enable = 0;
    }
    if(g_stMediaAdapter.SetOsdInfor(&stOsdRevInfo) != 0)
    {
        ONVIF_ERR("Osd server run error.");
        return ONVIF_ERR_IMAGE_SERVER_RUN_ERROR;
    }    
    return ONVIF_OK;
}

ONVIF_RET GK_NVT_Media_GetPrivacyMaskOptions(GONVIF_MEDIA_GetPrivacyMaskOptions_Token_S *pstGetPrivacyMaskOptionsReq, GONVIF_MEDIA_GetPrivacyMaskOptions_S *pstGetPrivacyMaskOptionsRes)
{
    if ((pstGetPrivacyMaskOptionsReq->aszVideoSourceToken == NULL) || (strcmp(pstGetPrivacyMaskOptionsReq->aszVideoSourceToken, g_GkIpc_OnvifInf.gokeIPCameraVideoSource[0].pszSourceToken) != 0))
    {
        return ONVIF_ERR_IMAGE_INVALID_VIDEO_SOURCE_TOKEN;
    }
	//memcpy(&(pstGetPrivacyMaskOptionsRes->stPrivacyMaskOptions), g_GkIpc_OnvifInf.gokeOSDOptions, sizeof(pstGetPrivacyMaskOptionsRes->stPrivacyMaskOptions));
    pstGetPrivacyMaskOptionsRes->stPrivacyMaskOptions.MaximumNumberOfAreas = MAX_PM_NUM;
    pstGetPrivacyMaskOptionsRes->stPrivacyMaskOptions.stPosition.stXRange.min = -1;
    pstGetPrivacyMaskOptionsRes->stPrivacyMaskOptions.stPosition.stXRange.max= 1;
    pstGetPrivacyMaskOptionsRes->stPrivacyMaskOptions.stPosition.stYRange.min = -1;
    pstGetPrivacyMaskOptionsRes->stPrivacyMaskOptions.stPosition.stYRange.max= 1;
    return ONVIF_OK;
}

ONVIF_RET GK_NVT_Media_GetPrivacyMasks(GONVIF_MEDIA_GetPrivacyMasks_Token_S *pstGetPrivacyMasksReq, GONVIF_MEDIA_GetPrivacyMasks_S *pstGetPrivacyMasksRes)
{
    if ((pstGetPrivacyMasksReq->aszVideoSourceToken == NULL) || (strcmp(pstGetPrivacyMasksReq->aszVideoSourceToken, g_GkIpc_OnvifInf.gokeIPCameraVideoSource[0].pszSourceToken) != 0))
    {
        return ONVIF_ERR_IMAGE_INVALID_VIDEO_SOURCE_TOKEN;
    }
    ONVIF_PMInfor_S stPMRevInfo[MAX_PM_NUM];
    memset(&stPMRevInfo, 0, sizeof(stPMRevInfo));
    if(g_stMediaAdapter.GetPMInfor(stPMRevInfo) != 0)
    {
        ONVIF_ERR("PM server run error.");
        return ONVIF_ERR_IMAGE_SERVER_RUN_ERROR;
    }
    int i;
    for(i=0; i<MAX_PM_NUM; i++)
    {
        pstGetPrivacyMasksRes->stPrivacyMasks[i].enMaskAreaPoint = stPMRevInfo[i].enable;
        sprintf(pstGetPrivacyMasksRes->stPrivacyMasks[i].aszPrivacyMaskToken, "Onvif_ext_token_%d", i+1);
        strcpy(pstGetPrivacyMasksRes->stPrivacyMasks[i].aszVideoSourceToken, pstGetPrivacyMasksReq->aszVideoSourceToken);
        pstGetPrivacyMasksRes->stPrivacyMasks[i].stMaskAreaPoint[0].x = 2*stPMRevInfo[i].x-1;
        pstGetPrivacyMasksRes->stPrivacyMasks[i].stMaskAreaPoint[0].y = 1-2*stPMRevInfo[i].y;
        pstGetPrivacyMasksRes->stPrivacyMasks[i].stMaskAreaPoint[1].x = 2*stPMRevInfo[i].x-1;
        pstGetPrivacyMasksRes->stPrivacyMasks[i].stMaskAreaPoint[1].y = 1-2*(stPMRevInfo[i].y+stPMRevInfo[i].height);
        pstGetPrivacyMasksRes->stPrivacyMasks[i].stMaskAreaPoint[2].x = 2*(stPMRevInfo[i].x+stPMRevInfo[i].width)-1;
        pstGetPrivacyMasksRes->stPrivacyMasks[i].stMaskAreaPoint[2].y = 1-2*(stPMRevInfo[i].y+stPMRevInfo[i].height);
        pstGetPrivacyMasksRes->stPrivacyMasks[i].stMaskAreaPoint[3].x = 2*(stPMRevInfo[i].x+stPMRevInfo[i].width)-1;
        pstGetPrivacyMasksRes->stPrivacyMasks[i].stMaskAreaPoint[3].y = 1-2*stPMRevInfo[i].y;
    }
    return ONVIF_OK;
}

ONVIF_RET GK_NVT_Media_DeletePrivacyMask(GONVIF_MEDIA_DeletePrivacyMask_Token_S *pstDeletePrivacyMaskReq)
{
    ONVIF_PMInfor_S stPMRevInfo[MAX_PM_NUM];
    memset(&stPMRevInfo, 0, sizeof(stPMRevInfo));
    if(g_stMediaAdapter.GetPMInfor(stPMRevInfo) != 0)
    {
        ONVIF_ERR("PM server run error.");
        return ONVIF_ERR_IMAGE_SERVER_RUN_ERROR;
    }
    if(strstr(pstDeletePrivacyMaskReq->aszPrivacyMaskToken, "Onvif_ext_token_"))
    {
        int i=MAX_PM_NUM;
        sscanf(pstDeletePrivacyMaskReq->aszPrivacyMaskToken, "Onvif_ext_token_%d", &i);
        i -= 1;
        if(i>=0&&i<MAX_PM_NUM)
        {
            stPMRevInfo[i].enable = 0;
            if(g_stMediaAdapter.SetPMInfor(&stPMRevInfo) != 0)
            {
                ONVIF_ERR("PM server run error.");
                return ONVIF_ERR_IMAGE_SERVER_RUN_ERROR;
            }
        }
    }
    return ONVIF_OK;
}

ONVIF_RET GK_NVT_Media_CreatePrivacyMask(GONVIF_MEDIA_PrivacyMask_S *pstCreatePrivacyMaskReq)
{
    if ((pstCreatePrivacyMaskReq->aszVideoSourceToken== NULL) || (strcmp(pstCreatePrivacyMaskReq->aszVideoSourceToken, g_GkIpc_OnvifInf.gokeIPCameraVideoSource[0].pszSourceToken) != 0))
    {
        return ONVIF_ERR_IMAGE_INVALID_VIDEO_SOURCE_TOKEN;
    }
    static int i=0;
    ONVIF_PMInfor_S stPMRevInfo[MAX_PM_NUM];
    memset(&stPMRevInfo, 0, sizeof(stPMRevInfo));
    if(g_stMediaAdapter.GetPMInfor(stPMRevInfo) != 0)
    {
        ONVIF_ERR("PM server run error.");
        return ONVIF_ERR_IMAGE_SERVER_RUN_ERROR;
    }
    stPMRevInfo[i].enable = pstCreatePrivacyMaskReq->enMaskAreaPoint;
    stPMRevInfo[i].x = (pstCreatePrivacyMaskReq->stMaskAreaPoint[0].x+1)/2;
    stPMRevInfo[i].y = (1-pstCreatePrivacyMaskReq->stMaskAreaPoint[0].y)/2;
    stPMRevInfo[i].width = (pstCreatePrivacyMaskReq->stMaskAreaPoint[2].x-pstCreatePrivacyMaskReq->stMaskAreaPoint[0].x)/2;
    stPMRevInfo[i].height = (pstCreatePrivacyMaskReq->stMaskAreaPoint[0].y-pstCreatePrivacyMaskReq->stMaskAreaPoint[2].y)/2;
    if(g_stMediaAdapter.SetPMInfor(stPMRevInfo) != 0)
    {
        ONVIF_ERR("PM server run error.");
        return ONVIF_ERR_IMAGE_SERVER_RUN_ERROR;
    }
    sprintf(pstCreatePrivacyMaskReq->aszPrivacyMaskToken, "Onvif_ext_token_%d", ++i);
    if(i >= MAX_PM_NUM)
        i=0;
    return ONVIF_OK;
}

GK_S32 media_GetVideoEncoderConfigurations(void *argv)
{
    GONVIF_MEDIA_VideoEncoderConfiguration_S *pstGetVideoEncoderConfiguration = (GONVIF_MEDIA_VideoEncoderConfiguration_S *)argv;
    GK_U8 i;
	ONVIF_MediaVideoEncoderInfor_S stRevMediaInfo[MAX_VIDEOENCODE_NUM];
	memset(stRevMediaInfo, 0, sizeof(ONVIF_MediaVideoEncoderInfor_S) * MAX_VIDEOENCODE_NUM);
	
	if(g_stMediaAdapter.GetVideoEncoderInfor(stRevMediaInfo, MAX_VIDEOENCODE_NUM) != 0)
	{
		ONVIF_ERR("Fail to get video encoder infor.");
		return -1;
	}
	for(i = 0; i < g_GkIpc_OnvifInf.VideoEncodeNum; i++)
	{
		GK_U8 encodeType = (GONVIF_MEDIA_VideoEncoding_E)stRevMediaInfo[i].encode_type;
  		if(encodeType == 1)
            pstGetVideoEncoderConfiguration[i].enEncoding = VideoEncoding_H264;
		else if(encodeType == 2)
            pstGetVideoEncoderConfiguration[i].enEncoding = VideoEncoding_JPEG;
        else if(encodeType == 3)
            pstGetVideoEncoderConfiguration[i].enEncoding = VideoEncoding_H264;
		else if(encodeType == 0)
            pstGetVideoEncoderConfiguration[i].enEncoding = VideoEncoding_MPEG;
            
		pstGetVideoEncoderConfiguration[i].stResolution.width 			  = stRevMediaInfo[i].encode_width;
		pstGetVideoEncoderConfiguration[i].stResolution.height 			  = stRevMediaInfo[i].encode_height;
		pstGetVideoEncoderConfiguration[i].stRateControl.bitrateLimit 	  = stRevMediaInfo[i].bitrate;
		pstGetVideoEncoderConfiguration[i].stRateControl.frameRateLimit   = stRevMediaInfo[i].framerate;
		pstGetVideoEncoderConfiguration[i].stRateControl.encodingInterval = stRevMediaInfo[i].encInterval;
		pstGetVideoEncoderConfiguration[i].quality 						  = (float)stRevMediaInfo[i].quality;
		pstGetVideoEncoderConfiguration[i].stH264.govLength				  = stRevMediaInfo[i].govLength;
		pstGetVideoEncoderConfiguration[i].stH264.enH264Profile 		  = stRevMediaInfo[i].h264Profile;

		if(i >= g_GkIpc_OnvifInf.VideoEncodeOptionNum)
        {
			continue;
		}

		if(pstGetVideoEncoderConfiguration[i].enEncoding == VideoEncoding_H264)
		{
    		if(pstGetVideoEncoderConfiguration[i].stRateControl.frameRateLimit < g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stH264.stFrameRateRange.min ||
    		    pstGetVideoEncoderConfiguration[i].stRateControl.frameRateLimit > g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stH264.stFrameRateRange.max)
            {
    			pstGetVideoEncoderConfiguration[i].stRateControl.frameRateLimit = g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stH264.stFrameRateRange.min;
    		}
		}
		else if(pstGetVideoEncoderConfiguration[i].enEncoding == VideoEncoding_JPEG)
		{
    		if(pstGetVideoEncoderConfiguration[i].stRateControl.frameRateLimit < g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stJPEG.stFrameRateRange.min ||
    		    pstGetVideoEncoderConfiguration[i].stRateControl.frameRateLimit > g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stJPEG.stFrameRateRange.max)
            {
    			pstGetVideoEncoderConfiguration[i].stRateControl.frameRateLimit = g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stJPEG.stFrameRateRange.min;
    		}
		}
		else if(pstGetVideoEncoderConfiguration[i].enEncoding ==VideoEncoding_MPEG)
		{
    		if(pstGetVideoEncoderConfiguration[i].stRateControl.frameRateLimit < g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stMPEG4.stFrameRateRange.min ||
    		    pstGetVideoEncoderConfiguration[i].stRateControl.frameRateLimit > g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stMPEG4.stFrameRateRange.max)
            {
    			pstGetVideoEncoderConfiguration[i].stRateControl.frameRateLimit = g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stMPEG4.stFrameRateRange.min;
    		}
		}

		if(pstGetVideoEncoderConfiguration[i].enEncoding == VideoEncoding_H264)
		{
            if(pstGetVideoEncoderConfiguration[i].stRateControl.encodingInterval < g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stH264.stEncodingIntervalRange.min ||
                pstGetVideoEncoderConfiguration[i].stRateControl.encodingInterval > g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stH264.stEncodingIntervalRange.max)
            {
                pstGetVideoEncoderConfiguration[i].stRateControl.encodingInterval = g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stH264.stEncodingIntervalRange.min;
            }
		}
		else if(pstGetVideoEncoderConfiguration[i].enEncoding == VideoEncoding_JPEG)
		{
            if(pstGetVideoEncoderConfiguration[i].stRateControl.encodingInterval < g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stJPEG.stEncodingIntervalRange.min ||
                pstGetVideoEncoderConfiguration[i].stRateControl.encodingInterval > g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stJPEG.stEncodingIntervalRange.max)
            {
                pstGetVideoEncoderConfiguration[i].stRateControl.encodingInterval = g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stH264.stEncodingIntervalRange.min;
            }
		}
		else if(pstGetVideoEncoderConfiguration[i].enEncoding == VideoEncoding_MPEG)
		{
            if(pstGetVideoEncoderConfiguration[i].stRateControl.encodingInterval < g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stMPEG4.stEncodingIntervalRange.min ||
                pstGetVideoEncoderConfiguration[i].stRateControl.encodingInterval > g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stMPEG4.stEncodingIntervalRange.max)
            {
                pstGetVideoEncoderConfiguration[i].stRateControl.encodingInterval = g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stMPEG4.stEncodingIntervalRange.min;
            }
		}
		
		if(pstGetVideoEncoderConfiguration[i].enEncoding == VideoEncoding_H264)
        {
            if(pstGetVideoEncoderConfiguration[i].stH264.govLength < g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stH264.stGovLengthRange.min ||
                pstGetVideoEncoderConfiguration[i].stH264.govLength > g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stH264.stGovLengthRange.max)
            {
                pstGetVideoEncoderConfiguration[i].stH264.govLength = g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stH264.stGovLengthRange.min;
            }
		}

        if(pstGetVideoEncoderConfiguration[i].quality < g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stQualityRange.min ||
            pstGetVideoEncoderConfiguration[i].quality > g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stQualityRange.max)
        {
            pstGetVideoEncoderConfiguration[i].quality = g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stQualityRange.min;
        }


	}
	
    return 0;

}

GK_S32 media_GetAudioEncoderConfigurations(void *argv)
{
    GONVIF_MEDIA_AudioEncoderConfiguration_S *pstGetAudioEncoderConfiguration = (GONVIF_MEDIA_AudioEncoderConfiguration_S *)argv;
	ONVIF_MediaAudioEncoderInfor_S stRevMediaInfo[MAX_AUDIOENCODE_NUM];
	memset(stRevMediaInfo, 0, sizeof(ONVIF_MediaAudioEncoderInfor_S) * MAX_AUDIOENCODE_NUM);

	if(g_stMediaAdapter.GetAudioEncoderInfor(stRevMediaInfo, MAX_AUDIOENCODE_NUM) != 0)
	{
		ONVIF_ERR("Fail to get audio encoder infor.");
		return -1;
	}
    GK_U8 i;
	for(i = 0; i < g_GkIpc_OnvifInf.AudioEncodeNum; i++)
	{
  		if(stRevMediaInfo[i].encode_type == 0)
		{
            pstGetAudioEncoderConfiguration[i].enEncoding = AudioEncoding_G711;
		}
		else if(stRevMediaInfo[i].encode_type == 1)
		{
            pstGetAudioEncoderConfiguration[i].enEncoding = AudioEncoding_G726;
		}
		else if(stRevMediaInfo[i].encode_type == 2)
		{
            pstGetAudioEncoderConfiguration[i].enEncoding = AudioEncoding_AAC;
		}
		pstGetAudioEncoderConfiguration[i].bitRate    = stRevMediaInfo[i].bitrate;
		pstGetAudioEncoderConfiguration[i].sampleRate = stRevMediaInfo[i].sampleRate;
    }
    
    return 0;

}

GK_S32 media_GetVideoEncoderConfigurationOptions(void *argv, GK_S32 sizeOptions)
{
	if(argv == NULL || sizeOptions > MAX_VIDEOENCODEOPTION_NUM || sizeOptions <= 0)
	{
		ONVIF_ERR("Invalid parameters.");
		return -1;
	}
	GK_S32 ret = 0;
	GONVIF_MEDIA_VideoEncoderConfigurationOptions_S *pstGetVideoEncoderConfigurationOptions = (GONVIF_MEDIA_VideoEncoderConfigurationOptions_S *)argv;
	if(g_stMediaAdapter.SynVideoEncoderOptions != NULL)
	{
		ONVIF_INFO("Video configuration options use exterior.");
		ret = g_stMediaAdapter.SynVideoEncoderOptions(pstGetVideoEncoderConfigurationOptions, sizeOptions);
		if(ret != 0)
		{
			ONVIF_ERR("Fail to synchronise video configuration options.");
			return -1;
		}
	}
	else
	{
		ONVIF_INFO("Video configuration options use XML.");
	}
	
	return 0;
}

GK_S32 media_GetAudioEncoderConfigurationOptions(void *argv, GK_S32 sizeOptions)
{
	GK_S32 ret = 0;
	/* TODO: Get static audio configuration options  which can be set by customer from exterior of private XML */
	return ret;
}

static void media_PrintPackage(GK_U8 *data, GK_U32 data_len)
{
    GK_U32 i;

    for(i = 0; i < data_len; i++)
    {
        ONVIF_DBG("0x%02x ", data[i]);
    }
    ONVIF_DBG("\n");
}

static void media_PacketContent(GK_U8 *buffer, void *content, GK_U32 content_type)
{
    if(NULL == buffer)
    {
        return ;
    }
    switch(content_type)
    {
        case PACKAGE_HEAD:
            buffer[0] = 0x47;
            buffer[1] |= 0x4B;
            buffer[2] |= 0x07;
            buffer[3] |= 0x0A;
            break;
        case PACKAGE_CMD:
            {
                GK_U32 cmd_value = *((int*)content);

                if(NULL == content)
                {
                    return ;
                }
                buffer[0] = (cmd_value >> 24) & 0xFF;
                buffer[1] |= (cmd_value >> 16) & 0xFF;
                buffer[2] |= (cmd_value >> 8) & 0xFF;
                buffer[3] |= cmd_value & 0xFF;
                break;
            }
        case PACKAGE_LEN:
            {
                GK_U32 len = *((int*)content);

                if(NULL == content)
                {
                    return ;
                }
                buffer[0] = (len >> 24) & 0xFF;
                buffer[1] |= (len >> 16) & 0xFF;
                buffer[2] |= (len >> 8) & 0xFF;
                buffer[3] |= len & 0xFF;
                break;
            }
        case PACKAGE_TAIL:
            {
                buffer[0] = 0x0D;
                buffer[1] = 0x0A;
                break;
            }
        default:
            ONVIF_ERR("unknow content type:%d", content_type);
            break;
    }

}

static GK_S32 media_PacketCommand(GK_U32 cmd, GK_U32 ipaddr, GK_U16 port, GK_U8 media_type, GK_U8 media_switch,GK_U8 *pdata)
{
    GK_S32 contentLen = 0;
    switch(cmd)
    {
        case CLIENT_RTSP_IP_PORT_REPORT:
            {
                GK_U8 *ptr = pdata;
                GK_U32 len = 8;
                media_PacketContent(ptr, NULL, PACKAGE_HEAD);
                ptr += 4;
                contentLen += 4;
                media_PacketContent(ptr, &cmd, PACKAGE_CMD);
                ptr += 4;
                contentLen += 4;
                media_PacketContent(ptr, &len, PACKAGE_LEN);
                ptr += 4;
                contentLen += 4;
                *ptr = (ipaddr >> 24) & 0xff;
                *(ptr + 1) = (ipaddr >> 16) & 0xff;
                *(ptr + 2) = (ipaddr >> 8) & 0xff;
                *(ptr + 3) = ipaddr & 0xff;
                ptr += 4;
                contentLen += 4;
                *ptr = (port >> 8) & 0xff;
                *(ptr + 1) = port&0xff;
                ptr += 2;
                contentLen += 2;

                *ptr = media_type;
                *(ptr + 1) = media_switch;
                ptr += 2;
                contentLen += 2;

                media_PacketContent(ptr, NULL, PACKAGE_TAIL);
                contentLen += 2;
            }
            break;
        default:
            ONVIF_ERR("unknow cmd");
            contentLen = -1;
            break;
    }
    return contentLen;
}

static GK_S32 media_GetRtspAck(GK_U8 *ack, GK_U8 *pkg, GK_U32 pkg_len)
{
    GK_U8 ack_tmp;

    if(NULL == ack)
    {
        return -1;
    }
    ack_tmp = pkg[0];

    *ack = ack_tmp;
    return 0;
}

static GK_S32 media_ParseAckMsg(GK_U8 *rcv_buffer, GK_U32 len)
{
    GK_U32 i, offset;
    GK_U32 cmd,datalen;
    GK_S32 retval = -1;
    for(i = 0; i < (len - 4); i++)
    {
        if((rcv_buffer[i] == 0x47) && (rcv_buffer[i + 1] == 0x4B) &&
                (rcv_buffer[i + 2] == 0x07) && (rcv_buffer[i + 3] == 0x0A))
        {
            offset = i + 4;
            cmd = rcv_buffer[offset] << 24;
            offset ++;
            cmd |= rcv_buffer[offset] << 16;
            offset ++;
            cmd |= rcv_buffer[offset] << 8;
            offset ++;
            cmd |= rcv_buffer[offset] << 0;
            offset ++;
            ONVIF_DBG("rtsp package cmd:%x\n", cmd);

            datalen = rcv_buffer[offset] << 24;
            offset ++;
            datalen |= rcv_buffer[offset] << 16;
            offset ++;
            datalen |= rcv_buffer[offset] << 8;
            offset ++;
            datalen |= rcv_buffer[offset] << 0;
            offset ++;
            ONVIF_DBG("rtsp package len:%x\n", datalen);


			if((rcv_buffer[offset + datalen] != 0x0D) || (rcv_buffer[offset + datalen + 1] != 0x0A))
			{
                ONVIF_ERR("package tailer is error id1:%d,id2:%d",rcv_buffer[offset + datalen],rcv_buffer[offset + datalen+1]);
                return -1;
			}

            switch(cmd)
            {
                case SERVER_RTSP_IP_PORT_ACK:
                    {
                        GK_U8 ack = 0;
                        if(media_GetRtspAck(&ack,&rcv_buffer[offset], len - offset) < 0)
                        {
                            ONVIF_ERR("ack error!!!");
                            media_PrintPackage(&rcv_buffer[offset], len - offset);
                        }
                        if(ack == 1)
                        {
                            retval = 0;
                        }
                        else
                        {
                            ONVIF_ERR("ack error!!!");
                            media_PrintPackage(&rcv_buffer[offset], len - offset);
                        }
                        break;
                    }
                default:
                    media_PrintPackage(&rcv_buffer[offset], len - offset);
                    retval = -1;
                    break;
            }
        }
    }
    
    return retval;
}

static GK_S32 media_SendMuticastInforToRtsp(GK_U32 ipaddr, GK_U16 port, GK_U8 media_type, GK_U8 media_switch)
{
    GK_S32 retVal;
    GK_S32 contentLen = 0;
    GK_U8 pMutidata[MAX_MUTIINFOR_LENGTH];
    memset(pMutidata, 0, MAX_MUTIINFOR_LENGTH);
    contentLen = media_PacketCommand(CLIENT_RTSP_IP_PORT_REPORT, ipaddr, port, media_type, media_switch, pMutidata);
    media_PrintPackage(pMutidata, contentLen);
    GK_S32 cltSockFd = -1;
	struct sockaddr_in client_addr;
	memset(&client_addr, 0, sizeof(client_addr));
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(RTSP_SERVER_PORT);
	client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    cltSockFd = socket(AF_INET, SOCK_STREAM, 0);
	if (cltSockFd == -1)
    {
		return -1;
	}
    retVal = connect(cltSockFd, (struct sockaddr *)&client_addr, sizeof(struct sockaddr));
	if (retVal == -1)
    {
        ONVIF_ERR("connect() socket connect faild!\n");
        close(cltSockFd);
		return -1;
	}
    retVal = send(cltSockFd, pMutidata, contentLen, 0);
	if(retVal == -1)
    {
		ONVIF_ERR("send() failed!\n");
        close(cltSockFd);
		return -1;
	}

    memset(pMutidata, 0, MAX_MUTIINFOR_LENGTH);
    GK_S32 recvLen = 0;
    recvLen = recv(cltSockFd, pMutidata, MAX_MUTIINFOR_LENGTH, 0);
	if(recvLen == -1)
    {
		ONVIF_ERR("recv() failed!\n");
        close(cltSockFd);
		return -1;
	}
    close(cltSockFd);

    media_PrintPackage(pMutidata, recvLen);
    retVal = media_ParseAckMsg(pMutidata, recvLen);
    if(retVal != 0)
    {
        ONVIF_ERR();
        return -2;
    }

  	return 0;
}

