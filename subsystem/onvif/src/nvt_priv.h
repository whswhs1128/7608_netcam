/*!
*****************************************************************************
** \file        solution/software/subsystem/onvif/src/nvt_priv.h
**
** \brief       onvif private header
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2013 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef  _NVT_PRIV_H_
#define  _NVT_PRIV_H_

#include "onvif_priv.h"

//*****************************************************************************
//*****************************************************************************
//** Data Structures
//*****************************************************************************
//*****************************************************************************


//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************

//common
ONVIF_RET GK_NVT_DevMng_GetServiceCapabilities(GONVIF_DEVMGN_GetServiceCapabilities_Res_S *pstDM_GetServiceCapabilitiesRes);
//common end



//device manage
ONVIF_RET GK_NVT_DevMng_GetWsdlUrl(GONVIF_DEVMNG_GetWsdlUrl_Res_S *pstDM_GetWsdlUrlRes);
ONVIF_RET GK_NVT_DevMng_GetServices(GONVIF_DEVMGN_GetServices_S *pstDM_GetServices, GONVIF_DEVMGN_GetServices_Res_S *pstDM_GetServicesRes);
ONVIF_RET GK_NVT_DevMng_GetCapabilities(GONVIF_DEVMNG_GetCapabilities_S *pstDM_GetCapabilities, GONVIF_DEVMNG_GetCapabilities_Res_S *pstDM_GetCapabilitiesRes);
ONVIF_RET GK_NVT_DevMng_SystemReboot(GONVIF_DEVMGN_SystemReboot_Res_S *pstDM_SystemRebootRes);
ONVIF_RET GK_NVT_DevMng_GetHostname(GONVIF_DEVMNG_GetHostname_Res_S *pstDM_GetHostnameRes);
ONVIF_RET GK_NVT_DevMng_SetHostname(GONVIF_DEVMNG_SetHostname_S *pstDM_SetHostnaame);
ONVIF_RET GK_NVT_DevMng_GetDNS(GONVIF_DEVMNG_GetDNS_Res_S *pstDM_GetDNSRes);
ONVIF_RET GK_NVT_DevMng_SetDNS(GONVIF_DEVMNG_SetDNS_S *pstDM_SetDNS);
ONVIF_RET GK_NVT_DevMng_GetNTP(GONVIF_DEVMNG_GetNTP_Res_S *pstDM_GetNTPRes);
ONVIF_RET GK_NVT_DevMng_SetNTP(GONVIF_DEVMNG_SetNTP_S *pstDM_SetNTP);
ONVIF_RET GK_NVT_DevMng_GetNetworkInterfaces(GONVIF_DEVMNG_GetNetworkInterfaces_Res_S *pstDM_GetNetworkInterfacesRes);
ONVIF_RET GK_NVT_DevMng_SetNetworkInterfaces(GONVIF_DEVMNG_SetNetworkInterfaces_S *pstDM_SetNetworkInterfaces, GONVIF_DEVMNG_SetNetworkInterfaces_Res_S *pstDM_SetNetworkInterfacesRes);
ONVIF_RET GK_NVT_DevMng_GetNetworkProtocols(GONVIF_DEVMNG_GetNetworkProtocols_Res_S *pstDM_GetNetworkProtocolRes);
ONVIF_RET GK_NVT_DevMng_SetNetworkProtocols(GONVIF_DEVMNG_SetNetworkProtocols_S *pstDM_SetNetworkProtocols);
ONVIF_RET GK_NVT_DevMng_GetNetworkDefaultGateway(GONVIF_DEVMNG_GetNetworkDefaultGateway_Res_S *pstDM_GetNetworkDefaultGatewayRes);
ONVIF_RET GK_NVT_DevMng_SetNetworkDefaultGateway(GONVIF_DEVMNG_SetNetworkDefaultGateway_S *pstDM_SetNetworkDefaultGateway);
ONVIF_RET GK_NVT_DevMng_GetSystemDateAndTime(GONVIF_DEVMNG_GetSystemDateAndTime_Res_S *pstDM_GetSystemDateAndTimeRes);
ONVIF_RET GK_NVT_DevMng_SetSystemDateAndTime(GONVIF_DEVMNG_SetSystemDateAndTime_S *pstDM_SetSystemDateAndTime);
ONVIF_RET GK_NVT_DevMng_GetDeviceInformation(GONVIF_DEVMNG_GetDeviceInformation_Res_S* pstDM_GetDeviceInformationRes);
ONVIF_RET GK_NVT_DevMng_GetScopes(GONVIF_DEVMNG_GetScopes_Res_S *pstDM_GetScopesRes);
ONVIF_RET GK_NVT_DevMng_SetScopes(GONVIF_DEVMNG_SetScopes_S *pstDM_SetScopes);
ONVIF_RET GK_NVT_DevMng_AddScopes(GONVIF_DEVMNG_AddScopes_S *pstDM_AddScopes);
ONVIF_RET GK_NVT_DevMng_RemoveScopes(GONVIF_DEVMNG_RemoveScopes_S *pstDM_RemoveScopes, GONVIF_DEVMNG_RemoveScopes_Res_S *pstDM_RemoveScopesRes);
ONVIF_RET GK_NVT_DevMng_GetDiscoveryMode(GONIVF_DEVMNG_GetDiscoveryMode_Res_S *pstDM_GetDiscoveryModeRes);
ONVIF_RET GK_NVT_DevMng_SetDiscoveryMode(GONIVF_DEVMNG_SetDiscoveryMode_S *pstDM_SetDiscoveryMode);
ONVIF_RET GK_NVT_DevMng_GetUsers(GONVIF_DEVMNG_GetUsers_Res_S *pstDM_GetUsersRes);
ONVIF_RET GK_NVT_DevMng_CreateUsers(GONVIF_DEVMNG_CreateUsers_S *pstDM_CreateUsers);
ONVIF_RET GK_NVT_DevMng_DeleteUsers(GONVIF_DEVMNG_DeleteUsers_S *pstDM_DeleteUsers);
ONVIF_RET GK_NVT_DevMng_SetUser(GONVIF_DEVMNG_SetUser_S *pstDM_SetUser);
ONVIF_RET GK_NVT_DevMng_SetSystemFactoryDefault(GONVIF_DEVMNG_SetSystemFactoryDefault_S *pstDM_SetSystemFactoryDefault);
#ifdef MODULE_SUPPORT_ZK_WAPI
void GK_NVT_DevMng_GetScopes_name_location(char *name, char *location);
void GK_NVT_DevMng_SetScopes_name_location(char *name, char *location);
#endif
//device manage end





//deviceIO 
ONVIF_RET GK_NVT_DeviceIO_GetVideoSources(GONVIF_DEVICEIO_GetVideoSources_Res_S *pstIO_GetVideoSourcesRes);
ONVIF_RET GK_NVT_DeviceIO_GetAudioSources(GONVIF_DEVICEIO_GetAudioSources_Res_S *pstIO_GetAudioSourcesRes);
ONVIF_RET GK_NVT_DeviceIO_GetRelayOutputs(GONVIF_DEVICEIO_GetRelayOutputs_Res_S *pstIO_GetRelayOutputsRes);
ONVIF_RET GK_NVT_DeviceIO_SetRelayOutputSettings(GONVIF_DEVICEIO_SetRelayOutputSettings_S *pstIO_SetRelayOutputSettings);
ONVIF_RET GK_NVT_DeviceIO_SetRelayOutputState(GONVIF_DEVICEIO_SetRelayOutputState_S *pstIO_GSetRelayOutputState);
//deviceIO end





//media
ONVIF_RET GK_NVT_Media_GetServiceCapabilities(GONVIF_MEDIA_GetServiceCapabilities_Res_S *pstMD_GetServiceCapabilitiesRes);
ONVIF_RET GK_NVT_Media_CreateProfile(GONVIF_MEDIA_CreateProfile_S *pstMD_CreateProfile, GONVIF_MEDIA_CreateProfile_Res_S *pstMD_CreateProfileRes);
ONVIF_RET GK_NVT_Media_GetProfile(GONVIF_MEDIA_GetProfile_S *pstMD_GetProfile, GONVIF_MEDIA_GetProfile_Res_S *pstMD_GetProfileRes);
ONVIF_RET GK_NVT_Media_GetProfiles(GONVIF_MEDIA_GetProfiles_Res_S *pstMD_GetProfilesRes);
ONVIF_RET GK_NVT_Media_DeleteProfile(GONVIF_MEDIA_DeleteProfile_S *pstMD_DeleteProfile);

ONVIF_RET GK_NVT_Media_AddVideoSourceConfiguration(GONVIF_MEDIA_AddVideoSourceConfiguration_S *pstMD_AddVideoSourceConfiguration);
ONVIF_RET GK_NVT_Media_AddVideoEncoderConfiguration(GONVIF_MEDIA_AddVideoEncoderConfiguration_S *pstMD_AddVideoEncoderConfiguration);
ONVIF_RET GK_NVT_Media_AddAudioSourceConfiguration(GONVIF_MEDIA_AddAudioSourceConfiguration_S *pstMD_AddAudioSourceConfiguration);
ONVIF_RET GK_NVT_Media_AddAudioEncoderConfiguration(GONVIF_MEDIA_AddAudioEncoderConfiguration_S *pstMD_AddAudioEncoderConfiguration);
ONVIF_RET GK_NVT_Media_AddPTZConfiguration(GONVIF_MEDIA_AddPTZConfiguration_S *pstMD_AddPTZConfiguration);

ONVIF_RET GK_NVT_Media_RemoveVideoSourceConfiguration(GONVIF_MEDIA_RemoveVideoSourceConfiguration_S *pstMD_RemoveVideoSourceConfiguration);
ONVIF_RET GK_NVT_Media_RemoveVideoEncoderConfiguration(GONVIF_MEDIA_RemoveVideoEncoderConfiguration_S *pstMD_RemoveVideoEncoderConfiguration);
ONVIF_RET GK_NVT_Media_RemoveAudioSourceConfiguration(GONVIF_MEDIA_RemoveAudioSourceConfiguration_S *pstMD_RemoveAudioSourceConfiguration);
ONVIF_RET GK_NVT_Media_RemoveAudioEncoderConfiguration(GONVIF_MEDIA_RemoveAudioEncoderConfiguration_S *pstMD_RemoveAudioEncoderConfiguration);
ONVIF_RET GK_NVT_Media_RemovePTZConfiguration(GONVIF_MEDIA_RemovePTZConfiguration_S *pstMD_RemovePTZConfiguration);

ONVIF_RET GK_NVT_Media_GetVideoSourceConfigurations(GONVIF_MEDIA_GetVideoSourceConfigurations_Res_S *pstMD_GetVideoSourceConfigurationsRes);
ONVIF_RET GK_NVT_Media_GetVideoSourceConfiguration(GONVIF_MEDIA_GetVideoSourceConfiguration_S *pstMD_GetVideoSourceConfiguration, GONVIF_MEDIA_GetVideoSourceConfiguration_Res_S *pstMD_GetVideoSourceConfigurationRes);
ONVIF_RET GK_NVT_Media_GetCompatibleVideoSourceConfigurations(GONVIF_MEDIA_GetCompatibleVideoSourceConfigurations_S *pstMD_GetCompatibleVideoSourceConfigurations, GONVIF_MEDIA_GetCompatibleVideoSourceConfigurations_Res_S *pstMD_GetCompatibleVideoSourceConfigurationsRes);
ONVIF_RET GK_NVT_Media_GetVideoSourceConfigurationOptions(GONVIF_MEDIA_GetVideoSourceConfigurationOptions_S *pstMD_GetVideoSourceConfigurationOptions, GONVIF_MEDIA_GetVideoSourceConfigurationOptions_Res_S *pstMD_GetVideoSourceConfigurationOptionsRes);
ONVIF_RET GK_NVT_Media_SetVideoSourceConfiguration(GONVIF_MEDIA_SetVideoSourceConfiguration_S *pstMD_SetVideoSourceConfiguration);

ONVIF_RET GK_NVT_Media_GetVideoEncoderConfigurations(GONVIF_MEDIA_GetVideoEncoderConfigurations_Res_S *pstMD_GetVideoEncoderConfigurationsRes);
ONVIF_RET GK_NVT_Media_GetVideoEncoderConfiguration(GONVIF_MEDIA_GetVideoEncoderConfiguration_S *pstMD_GetVideoEncoderConfiguration, GONVIF_MEDIA_GetVideoEncoderConfiguration_Res_S *pstMD_GetVideoEncoderConfigurationRes);
ONVIF_RET GK_NVT_Media_GetCompatibleVideoEncoderConfigurations(GONVIF_MEDIA_GetCompatibleVideoEncoderConfigurations_S *pstMD_GetCompatibleVideoEncoderConfigurations, GONVIF_MEDIA_GetCompatibleVideoEncoderConfigurations_Res_S *pstMD_GetCompatibleVideoEncoderConfigurationsRes);
ONVIF_RET GK_NVT_Media_GetVideoEncoderConfigurationOptions(GONVIF_MEDIA_GetVideoEncoderConfigurationOptions_S *pstMD_GetVideoEncoderConfigurationOptions, GONVIF_MEDIA_GetVideoEncoderConfigurationOptions_Res_S *pstMD_GetVideoEncoderConfigurationOptionsRes);
ONVIF_RET GK_NVT_Media_SetVideoEncoderConfiguration(GONVIF_MEDIA_SetVideoEncoderConfiguration_S *pstMD_SetVideoEncoderConfiguration);
ONVIF_RET GK_NVT_Media_GetGuaranteedNumberOfVideoEncoderInstances(GONVIF_MEDIA_GetGuaranteedNumberOfVideoEncoderInstances_S *pstMD_GetGuaranteedNumberOfVideoEncoderInstances, GONVIF_MEDIA_GetGuaranteedNumberOfVideoEncoderInstances_Res_S *pstMD_GetGuaranteedNumberOfVideoEncoderInstancesRes);

ONVIF_RET GK_NVT_Media_GetAudioSourceConfigurations(GONVIF_MEDIA_GetAudioSourceConfigurations_Res_S *pstMD_GetAudioSourceConfigurationRes);
ONVIF_RET GK_NVT_Media_GetAudioSourceConfiguration(GONVIF_MEDIA_GetAudioSourceConfiguration_S *pstMD_GetAudioSourceConfiguration, GONVIF_MEDIA_GetAudioSourceConfiguration_Res_S *pstMD_GetAudioSourceConfigurationRes);
ONVIF_RET GK_NVT_Media_GetCompatibleAudioSourceConfigurations(GONVIF_MEDIA_GetCompatibleAudioSourceConfigurations_S *pstMD_GetCompatibleAudioSourceConfigurations, GONVIF_MEDIA_GetCompatibleAudioSourceConfigurations_Res_S *pstMD_GetCompatibleAudioSourceConfigurationsRes);
ONVIF_RET GK_NVT_Media_GetAudioSourceConfigurationOptions(GONVIF_MEDIA_GetAudioSourceConfigurationOptions_S *pstMD_GetAudioSourceConfigurationOptions, GONVIF_MEDIA_GetAudioSourceConfigurationOptions_Res_S *pstMD_GetAudioSourceConfigurationOptionsRes);
ONVIF_RET GK_NVT_Media_SetAudioSourceConfiguration(GONVIF_MEDIA_SetAudioSourceConfiguration_S *pstMD_SetAudioSourceConfiguration);

ONVIF_RET GK_NVT_Media_GetAudioEncoderConfigurations(GONVIF_MEDIA_GetAudioEncoderConfigurations_Res_S *pstMD_GetAudioEncoderConfigurationsRes);
ONVIF_RET GK_NVT_Media_GetAudioEncoderConfiguration(GONVIF_MEDIA_GetAudioEncoderConfiguration_S *pstMD_GetAudioEncoderConfiguration, GONVIF_MEDIA_GetAudioEncoderConfiguration_Res_S *pstMD_GetAudioEncoderConfigurationRes);
ONVIF_RET GK_NVT_Media_GetCompatibleAudioEncoderConfigurations(GONVIF_MEDIA_GetCompatibleAudioEncoderConfigurations_S *pstGetCompAECfgsReq,GONVIF_MEDIA_GetCompatibleAudioEncoderConfigurations_Res_S *pstAECfgsRes);
ONVIF_RET GK_NVT_Media_GetAudioEncoderConfigurationOptions(GONVIF_MEDIA_GetAudioEncoderConfigurationOptions_S *pstMD_GetAudioEncoderConfigurationOptions, GONVIF_MEDIA_GetAudioEncoderConfigurationOptions_Res_S *pstMD_GetAudioEncoderConfigurationOptionsRes);
ONVIF_RET GK_NVT_Media_SetAudioEncoderConfiguration(GONVIF_MEDIA_SetAudioEncoderConfiguration_S *pstMD_SetAudioEncoderConfiguration);

ONVIF_RET GK_NVT_Media_GetStreamUri(GONVIF_MEDIA_GetStreamUri_S *pstMD_GetStreamUri, GONVIF_MEDIA_GetStreamUri_Res_S *pstMD_GetStreamUriRes);
ONVIF_RET GK_NVT_Media_GetSnapshotUri(GONVIF_MEIDA_GetSnapShotUri_S *pstMD_GetSnapShotUri, GONVIF_MEIDA_GetSnapShotUri_Res_S *pstMD_GetSnapShotUriRes);

ONVIF_RET GK_NVT_Media_GetOSDOptions(GONVIF_MEDIA_GetOSDOptions_Token_S *pstGetOSDOptionsReq, GONVIF_MEDIA_GetOSDOptions_S *pstGetOSDOptionsRes);
ONVIF_RET GK_NVT_Media_GetOSDs(GONVIF_MEDIA_GetOSDs_Token_S *pstGetOSDsReq, GONVIF_MEDIA_GetOSDs_S *pstGetOSDsRes);
ONVIF_RET GK_NVT_Media_SetOSD(GONVIF_MEDIA_SetOSD_S *pstSetOSDReq);
ONVIF_RET GK_NVT_Media_DeleteOSD(GONVIF_MEDIA_DeleteOSD_S *pstDeleteOSDReq);

ONVIF_RET GK_NVT_Media_GetPrivacyMaskOptions(GONVIF_MEDIA_GetPrivacyMaskOptions_Token_S *pstGetPrivacyMaskOptionsReq, GONVIF_MEDIA_GetPrivacyMaskOptions_S *pstGetPrivacyMaskOptionsRes);
ONVIF_RET GK_NVT_Media_GetPrivacyMasks(GONVIF_MEDIA_GetPrivacyMasks_Token_S *pstGetPrivacyMasksReq, GONVIF_MEDIA_GetPrivacyMasks_S *pstGetPrivacyMasksRes);
ONVIF_RET GK_NVT_Media_DeletePrivacyMask(GONVIF_MEDIA_DeletePrivacyMask_Token_S *pstDeletePrivacyMaskReq);
ONVIF_RET GK_NVT_Media_CreatePrivacyMask(GONVIF_MEDIA_PrivacyMask_S *pstCreatePrivacyMaskReq);

//media end




//imaging
ONVIF_RET GK_NVT_Imaging_GetServiceCapabilities(GONVIF_IMAGING_GetServiceCapabilities_Res_S *pstIMG_GetServiceCapabilitiesRes);
ONVIF_RET GK_NVT_Image_GetImaging(GONVIF_IMAGE_Settings_S *pstImagingSettings);
ONVIF_RET GK_NVT_Image_GetImagingSettings(GONVIF_IMAGE_GetSettings_Token_S *pstGetImagingSettingsReq, GONVIF_IMAGE_GetSettings_S *pstGetImagingSettingsRes);
ONVIF_RET GK_NVT_Image_SetImagingSettings(GONVIF_IMAGE_SetSettings_S *pstSetImagingSettingsReq);
ONVIF_RET GK_NVT_Image_GetOptions(GONVIF_IMAGE_GetOptions_Token_S *pstGetImageOptionsReq, GONVIF_IMAGE_GetOptions_S *pstGetImageOptionsRes);
ONVIF_RET GK_NVT_Image_Move(GONVIF_IMAGE_Move_S *pstMoveReq);
ONVIF_RET GK_NVT_Image_Stop(GONVIF_IMAGE_Stop_Token_S *pstStopReq);
ONVIF_RET GK_NVT_Image_GetStatus(GONVIF_IMAGE_GetStatus_Token_S *pstGetStatusReq, GONVIF_IMAGE_GetStatus_S *pstGetStatusRes);
ONVIF_RET GK_NVT_Image_GetMoveOptions(GONVIF_IMAGE_GetMoveOptions_Token_S *pstGetMoveOptionsReq, GONVIF_IMAGE_GetMoveOptions_S *pstGetMoveOptionsRes);
//imaging end




//event
ONVIF_RET GK_NVT_Event_GetServiceCapabilities(GONVIF_Soap_S *pstSoap, GONVIF_EVENT_GetServiceCapabilities_Res_S *pstEV_GetServiceCapabilitiesRes);
ONVIF_RET GK_NVT_Event_GetEventProperties(GONVIF_Soap_S *pstSoap, GONVIF_EVENT_GetEventProperties_Res_S *pstEV_GetEventPropertiesRes);
ONVIF_RET GK_NVT_Event_Subscribe(GONVIF_Soap_S *pstSoap, GONVIF_EVENT_Subscribe_S *pstEV_Subscribe, GONVIF_EVENT_Subscribe_Res_S *pstEV_SubscribeRes);
ONVIF_RET GK_NVT_Event_Unsubscribe(GONVIF_Soap_S *pstSoap, GONVIF_EVENT_Unsubscribe_S *pstEV_Unsubscribe);
ONVIF_RET GK_NVT_Event_Renew(GONVIF_Soap_S *pstSoap, GONVIF_EVENT_Renew_S *pstEV_Renew, GONVIF_EVENT_Renew_Res_S *pstEV_RenewRes);
ONVIF_RET GK_NVT_Event_SetSynchronizationPoint(GONVIF_Soap_S *pstSoap, GONVIF_EVENT_SetSynchronizationPoint_S *pstEV_SetSynchronizationPoint);
ONVIF_RET GK_NVT_Event_CreatePullPointSubscription(GONVIF_Soap_S *pstSoap, GONVIF_EVENT_CreatePullPointSubscription_S *pstEV_CreatePullPointSubscription, GONVIF_EVENT_CreatePullPointSubscription_Res_S *pstEV_CreatePullPointSubscriptionRes);
ONVIF_RET GK_NVT_Event_PullMessages(GONVIF_Soap_S *pstSoap, GONVIF_EVENT_PullMessages_S *pstEV_PullMessages, GONVIF_EVENT_PullMessages_Res_S *pstEV_PullMessagesRes);
//event end





//PTZ
ONVIF_RET GK_NVT_PTZ_GetServiceCapabilities(GONVIF_PTZ_GetServiceCapabilities_Res_S *pstPTZ_GetServiceCapabilitiesRes);
ONVIF_RET GK_NVT_PTZ_GetNodes(GONVIF_PTZ_GetNodes_Res_S *pstPTZ_GetNodesRes);
ONVIF_RET GK_NVT_PTZ_GetNode(GONVIF_PTZ_GetNode_S *pstPTZ_GetNode, GONVIF_PTZ_GetNode_Res_S *pstPTZ_GetNodeRes);
ONVIF_RET GK_NVT_PTZ_GetConfigurations(GONVIF_PTZ_GetConfigutations_Res_S *pstPTZ_GetConfigurationsRes);
ONVIF_RET GK_NVT_PTZ_GetConfiguration(GONVIF_PTZ_GetConfiguration_S *pstPTZ_GetConfiguration, GONVIF_PTZ_GetConfiguration_Res_S *pstPTZ_GetConfigurationRes);
ONVIF_RET GK_NVT_PTZ_GetConfigurationOptions(GONVIF_PTZ_GetConfigurationOptions_S *pstPTZ_GetConfigurationOptions, GONVIF_PTZ_GetConfigurationOptions_Res_S *pstPTZ_GetConfigurationOptionsRes);
ONVIF_RET GK_NVT_PTZ_SetConfiguration(GONVIF_PTZ_SetConfiguration_S *pstPTZ_SetConfiguration);
ONVIF_RET GK_NVT_PTZ_ContinuousMove(GONVIF_PTZ_ContinuousMove_S *pstPTZ_ContinuousMove);
ONVIF_RET GK_NVT_PTZ_Stop(GONVIF_PTZ_Stop_S *pstPTZ_Stop);
ONVIF_RET GK_NVT_PTZ_GetPresets(GONVIF_PTZ_GetPresets_S *pstPTZ_GetPresets, GONVIF_PTZ_GetPresets_Res_S *pstPTZ_GetPresetsRes);
ONVIF_RET GK_NVT_PTZ_SetPreset(GONVIF_PTZ_SetPreset_S *pstPTZ_SetPreset, GONVIF_PTZ_SetPreset_Res_S *pstPTZ_SetPresetRes);
ONVIF_RET GK_NVT_PTZ_GotoPreset(GONVIF_PTZ_GotoPreset_S *pstPTZ_GotoPreset);
ONVIF_RET GK_NVT_PTZ_RemovePreset(GONVIF_PTZ_RemovePreset_S *pstPTZ_RemovePreset);




ONVIF_RET GK_NVT_PTZ_AbsoluteMove(GONVIF_PTZ_AbsoluteMove_S *pstPTZ_AbsoluteMove);
ONVIF_RET GK_NVT_PTZ_RelativeMove(GONVIF_PTZ_RelativeMove_S *pstPTZ_RelativeMove);
ONVIF_RET GK_NVT_PTZ_GetStatus(GONVIF_PTZ_GetStatus_Token_S *pstGetStatusReq, GONVIF_PTZ_GetStatus_S *pstGetStatusRes);
//PTZ end



//Video Analytics
ONVIF_RET GK_NVT_Analytics_GetServiceCapabilities(GONVIF_ANALYTICS_GetServiceCapabilities_Res_S *pstAN_GetServiceCapabilitiesRes);
ONVIF_RET GK_NVT_Analytics_GetSupportedAnalyticsModules(GONVIF_ANALYTICS_GetSupportedAnalyticsModules_S *pstGetSupportedAnalyticsModulesReq, GONVIF_ANALYTICS_GetSupportedAnalyticsModulesRes_S *pstGetSupportedAnalyticsModulesRes);
ONVIF_RET GK_NVT_Analytics_GetAnalyticsModules(GONVIF_ANALYTICS_GetAnalyticsModules_S *pstGetAnalyticsModulesReq, GONVIF_ANALYTICS_GetAnalyticsModulesRes_S *pstpstGetAnalyticsModulesRes);
ONVIF_RET GK_NVT_Analytics_GetRules(GONVIF_ANALYTICS_GetRules_S *pstGetRulesReq, GONVIF_ANALYTICS_GetRulesRes_s *pstGetRulesRes);
ONVIF_RET GK_NVT_Analytics_GetSupportedRules(GONVIF_ANALYTICS_GetSupportedRules_S *pstGetSupportedRulesReq, GONVIF_ANALYITICS_GetSupportedRulesRes_S *pstGetSupportedRulesRes);
//Video Analytics end


#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif
#endif     /* _NVT_PRIV_H_ */
