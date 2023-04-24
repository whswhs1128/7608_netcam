/*!
*****************************************************************************
** \file        solution/software/subsystem/onvif-no-gsoap/src/soap_parse.h
**
** \brief       soap parse
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2013 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/



#ifndef _SOAP_PARSE_H_
#define _SOAP_PARSE_H_

/***************************************************************************************/
#include "onvif_std.h"
#include "onvif_priv.h"
#include "xml.h"



#ifdef __cplusplus
extern "C" {
#endif
const char *soap_get_tag_child_name(const char *tag);
GK_S32 soap_match_tag(const char *str1, const char *str2);
XMLN  *soap_get_child_node(XMLN *parent, const char *nodeName);
XMLN  *soap_get_next_node(XMLN *curNode, const char *nodeName);

//device manage
ONVIF_RET soap_parse_request_GetCapabilities(XMLN *p_node, GONVIF_DEVMNG_GetCapabilities_S *pstDM_GetCapabilities);
ONVIF_RET soap_parse_request_SetHostname(XMLN *p_node, GONVIF_DEVMNG_SetHostname_S *pstDM_SetHostname);
ONVIF_RET soap_parse_request_SetDNS(XMLN *p_node, GONVIF_DEVMNG_SetDNS_S *pstDM_SetDNS);
ONVIF_RET soap_parse_request_SetNTP(XMLN *p_node, GONVIF_DEVMNG_SetNTP_S *pstDM_SetNTP);
ONVIF_RET soap_parse_request_SetNetworkInterfaces(XMLN *p_node, GONVIF_DEVMNG_SetNetworkInterfaces_S *pstDM_SetNetworkInterfaces);
ONVIF_RET soap_parse_request_SetNetworkProtocols(XMLN *p_node, GONVIF_DEVMNG_SetNetworkProtocols_S *pstDM_SetNetworkProtocols);
ONVIF_RET soap_parse_request_SetNetworkDefaultGateway(XMLN *p_node, GONVIF_DEVMNG_SetNetworkDefaultGateway_S *pstDM_SetNetworkDefaultGateway);
ONVIF_RET soap_parse_request_SetSystemDateAndTime(XMLN *p_node, GONVIF_DEVMNG_SetSystemDateAndTime_S *pstDM_SetSystemDateAndTime);
ONVIF_RET soap_parse_request_SetScopes(XMLN *p_SetScopes, GONVIF_DEVMNG_SetScopes_S *pstDM_SetScopes);
ONVIF_RET soap_parse_request_AddScopes(XMLN *p_AddScopes, GONVIF_DEVMNG_AddScopes_S *pstDM_AddScopes);
ONVIF_RET soap_parse_request_RemoveScopes(XMLN *p_RemoveScopes, GONVIF_DEVMNG_RemoveScopes_S *pstDM_RemoveScopes);
ONVIF_RET soap_parse_request_SetDiscoveryMode(XMLN *p_node, GONIVF_DEVMNG_SetDiscoveryMode_S *pstDM_SetDiscoveryMode);

ONVIF_RET soap_parse_request_CreateUsers(XMLN *p_node, GONVIF_DEVMNG_CreateUsers_S *pstDM_CreateUsers);
ONVIF_RET soap_parse_request_DeleteUsers(XMLN *p_node, GONVIF_DEVMNG_DeleteUsers_S *pstDM_DeleteUsers);
ONVIF_RET soap_parse_request_SetUser(XMLN *p_node, GONVIF_DEVMNG_SetUser_S *pstDM_SetUser);
ONVIF_RET soap_parse_request_SetSystemFactoryDefault(XMLN *p_node, GONVIF_DEVMNG_SetSystemFactoryDefault_S *pstDM_SetSystemFactoryDefault);
//device manage end


//deviceIO
ONVIF_RET soap_parse_request_SetRelayOutputSettings(XMLN *p_body, GONVIF_DEVICEIO_SetRelayOutputSettings_S *pstIO_SetRelayOutputSettings);
ONVIF_RET soap_parse_request_SetRelayOutputState(XMLN *p_body, GONVIF_DEVICEIO_SetRelayOutputState_S *pstIO_SetRelayOutputState);
//deviceIO end



//media
ONVIF_RET soap_parse_request_CreateProfile(XMLN *p_node, GONVIF_MEDIA_CreateProfile_S *pstMD_CreateProfile);
ONVIF_RET soap_parse_request_SetVideoSourceConfiguration(XMLN *p_node, GONVIF_MEDIA_SetVideoSourceConfiguration_S *pstMD_SetVideoSourceConfiguration);
ONVIF_RET soap_parse_request_SetVideoEncoderConfiguration(XMLN *p_node, GONVIF_MEDIA_SetVideoEncoderConfiguration_S *pstMD_SetVideoEncoderConfiguration);
ONVIF_RET soap_parse_request_SetAudioSourceConfiguration(XMLN * p_node, GONVIF_MEDIA_SetAudioSourceConfiguration_S *pstMD_SetAudioSourceConfiguration);
ONVIF_RET soap_parse_request_SetAudioEncoderConfiguration(XMLN *p_node, GONVIF_MEDIA_SetAudioEncoderConfiguration_S *pstMD_SetAudioEncoderConfiguration);
ONVIF_RET soap_parse_request_GetStreamUri(XMLN *p_node, GONVIF_MEDIA_GetStreamUri_S *pstMD_GetStreamUri);

ONVIF_RET soap_parse_request_GetOSDOptions(XMLN *p_body, GONVIF_MEDIA_GetOSDOptions_Token_S *pstGetOSDOptionsReq);
ONVIF_RET soap_parse_request_GetOSDs(XMLN *p_body, GONVIF_MEDIA_GetOSDs_Token_S *pstGetOSDsRes);
ONVIF_RET soap_parse_request_SetOSD(XMLN *p_body, GONVIF_MEDIA_SetOSD_S *pstSetOSDRes);
ONVIF_RET soap_parse_request_DeleteOSD(XMLN *p_body, GONVIF_MEDIA_DeleteOSD_S *pstDeleteOSDRes);

ONVIF_RET soap_parse_request_GetPrivacyMaskOptions(XMLN *p_body, GONVIF_MEDIA_GetPrivacyMaskOptions_Token_S *pstGetPrivacyMaskOptionsRes);
ONVIF_RET soap_parse_request_GetPrivacyMasks(XMLN *p_body, GONVIF_MEDIA_GetPrivacyMasks_Token_S *pstPrivacyMasksRes);
ONVIF_RET soap_parse_request_DeletePrivacyMask(XMLN *p_body, GONVIF_MEDIA_DeletePrivacyMask_Token_S *pstPrivacyMasksRes);
ONVIF_RET soap_parse_request_CreatePrivacyMask(XMLN *p_body, GONVIF_MEDIA_PrivacyMask_S *pstCreatePrivacyMaskRes);

//media end


//event
ONVIF_RET soap_parse_request_Subscribe(XMLN *p_node, GONVIF_EVENT_Subscribe_S *pstEV_Subscribe);
ONVIF_RET soap_parse_request_CreatePullPointSubscription(XMLN *p_node, GONVIF_EVENT_CreatePullPointSubscription_S *pstEV_CreatePullPointSubscription);
//event end


//image
ONVIF_RET soap_parse_request_GetImagingSettings(XMLN *p_body, GONVIF_IMAGE_GetSettings_Token_S *pstGetSettingsReq);
ONVIF_RET soap_parse_request_SetImagingSettings(XMLN *p_body, GONVIF_IMAGE_SetSettings_S *pstSetImagingSettingsReq);
ONVIF_RET soap_parse_request_GetOptions(XMLN *p_body, GONVIF_IMAGE_GetOptions_Token_S *pstGetImageOptionsReq);
ONVIF_RET soap_parse_request_GetMoveOptions(XMLN *p_body, GONVIF_IMAGE_GetMoveOptions_Token_S *pstGetMoveOptionsReq);
ONVIF_RET soap_parse_request_Move(XMLN *p_body, GONVIF_IMAGE_Move_S *pstMoveReq);
ONVIF_RET soap_parse_request_GetStatus(XMLN *p_body, GONVIF_IMAGE_GetStatus_Token_S *pstImageGetStatusReq);
ONVIF_RET soap_parse_request_ImageStop(XMLN *p_body, GONVIF_IMAGE_Stop_Token_S *pstStopReq);

//ptz
ONVIF_RET soap_parse_request_GetNodes(XMLN *p_body);
ONVIF_RET soap_parse_request_GetNode(XMLN *p_body, GONVIF_PTZ_GetNode_S *pstPTZ_GetNode);
ONVIF_RET soap_parse_request_GetConfiguration(XMLN *p_body, GONVIF_PTZ_GetConfiguration_S *pstPTZ_GetConfiguration);
ONVIF_RET soap_parse_request_GetConfigurationOptions(XMLN *p_body, GONVIF_PTZ_GetConfigurationOptions_S *pstPTZ_GetConfigurationOptions);
ONVIF_RET soap_parse_request_SetConfiguration(XMLN *p_body, GONVIF_PTZ_SetConfiguration_S *pstPTZ_SetConfiguration);
ONVIF_RET soap_parse_request_ContinuousMove(XMLN *p_body, GONVIF_PTZ_ContinuousMove_S *pstPTZ_ContinuousMove);
ONVIF_RET soap_parse_request_PtzStop(XMLN *p_node, GONVIF_PTZ_Stop_S *stPTZ_Stop);




ONVIF_RET soap_parse_request_AbsoluteMove(XMLN *p_body, GONVIF_PTZ_AbsoluteMove_S *pstPTZ_AbsoluteMove);
ONVIF_RET soap_parse_request_RelativeMove(XMLN *p_body, GONVIF_PTZ_RelativeMove_S *pstPTZ_RelativeMove);
ONVIF_RET soap_parse_request_GotoPreset(XMLN *p_node, GONVIF_PTZ_GotoPreset_S *pstGotoPreset);

//analytics
ONVIF_RET soap_parse_request_GetSupportedAnalyticsModules(XMLN *p_body, GONVIF_ANALYTICS_GetSupportedAnalyticsModules_S *pstGetSupportedAnalyticsModulesReq);
ONVIF_RET soap_parse_request_GetAnalyticsModules(XMLN *p_body, GONVIF_ANALYTICS_GetAnalyticsModules_S *pstGetAnalyticsModulesReq);
ONVIF_RET soap_parse_request_GetSupportedRules(XMLN *p_body, GONVIF_ANALYTICS_GetSupportedRules_S *pstGetSupportedRulesReq);
ONVIF_RET soap_parse_request_GetRules(XMLN *p_body, GONVIF_ANALYTICS_GetRules_S *pstGetRulesReq);


#ifdef __cplusplus
}
#endif

#endif


