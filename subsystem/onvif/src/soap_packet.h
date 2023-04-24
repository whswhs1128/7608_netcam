/*!
*****************************************************************************
** \file        solution/software/subsystem/onvif-no-gsoap/src/soap_packet.h
**
** \brief       soap packet
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2013 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _SOAP_PACKET_H_
#define _SOAP_PACKET_H_

#include "onvif_std.h"

#ifdef __cplusplus
extern "C" {
#endif

//common
GK_S32 soap_response_error(GK_CHAR *p_buf, GK_S32 mlen, const GK_CHAR *code, const GK_CHAR *subcode, const GK_CHAR *subcode_ex, const GK_CHAR *reason, const void *header);

//device manage
GK_S32 soap_response_Device_GetServiceCapabilities(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetWsdlUrl(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetServices(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetCapabilities(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_SystemReboot(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetHostname(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_SetHostname(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetDNS(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_SetDNS(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetNTP(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_SetNTP(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetNetworkInterfaces(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_SetNetworkInterfaces(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetNetworkProtocols(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_SetNetworkProtocols(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetNetworkDefaultGateway(GK_CHAR * p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_SetNetworkDefaultGateway(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetSystemDateAndTime(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_SetSystemDateAndTime(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetDeviceInformation(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetScopes(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_SetScopes(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_AddScopes(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_RemoveScopes(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetDiscoveryMode(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_SetDiscoveryMode(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetUsers(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_CreateUsers(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_DeleteUsers(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_SetUser(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_SetSystemFactoryDefault(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
//device manage end


//deviceIO
GK_S32 soap_packet_response_GetVideoSources(GK_CHAR * p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetAudioSources(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetRelayOutputs(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_SetRelayOutputSettings(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_SetRelayOutputState(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
//deviceIO end

//media
GK_S32 soap_response_Media_GetServiceCapabilities(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_CreateProfile(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetProfile(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetProfiles(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_DeleteProfile(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);

GK_S32 soap_packet_response_AddVideoSourceConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_AddVideoEncoderConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_AddAudioSourceConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_AddAudioEncoderConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_AddPTZConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);

GK_S32 soap_packet_response_RemoveVideoSourceConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_RemoveVideoEncoderConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_RemoveAudioSourceConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_RemoveAudioEncoderConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_RemovePTZConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);

GK_S32 soap_packet_response_GetVideoSourceConfigurations(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetVideoSourceConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetCompatibleVideoSourceConfigurations(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetVideoSourceConfigurationOptions(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_SetVideoSourceConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);

GK_S32 soap_packet_response_GetVideoEncoderConfigurations(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetVideoEncoderConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetCompatibleVideoEncoderConfigurations(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetVideoEncoderConfigurationOptions(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_SetVideoEncoderConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetGuaranteedNumberOfVideoEncoderInstances(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);

GK_S32 soap_packet_response_GetAudioSourceConfigurations(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetAudioSourceConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetCompatibleAudioSourceConfigurations(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetAudioSourceConfigurationOptions(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_SetAudioSourceConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);

GK_S32 soap_packet_response_GetAudioEncoderConfigurations(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetAudioEncoderConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetCompatibleAudioEncoderConfigurations(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetAudioEncoderConfigurationOptions(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_SetAudioEncoderConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);

GK_S32 soap_packet_response_GetMetadataConfigurations(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetMetadataConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetCompatibleMetadataConfigurations(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetMetadataConfigurationOptions(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);

GK_S32 soap_packet_response_GetStreamUri(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetSnapshotUri(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);

GK_S32 soap_packet_response_StartMulticastStreaming(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_StopMulticastStreaming(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);

GK_S32 soap_packet_response_GetOSDOptions(GK_CHAR * p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetOSDs(GK_CHAR * p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_SetOSD(GK_CHAR * p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_DeleteOSD(GK_CHAR * p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetPrivacyMaskOptions(GK_CHAR * p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetPrivacyMasks(GK_CHAR * p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_DeletePrivacyMask(GK_CHAR * p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_CreatePrivacyMask(GK_CHAR * p_buf, GK_S32 mlen, const void *argv, const void *header);

//media end

//event
GK_S32 soap_response_Event_GetServiceCapabilities(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetEventProperties(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_Subscribe(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_Unsubscribe(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_Renew(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_CreatePullPointSubscription(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_PullMessages(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_SetSynchronizationPoint(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);

//event end

/***************************************** image ***************************************/
GK_S32 soap_response_Imaging_GetServiceCapabilities(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetImagingSettings(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_SetImagingSettings(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetOptions(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetMoveOptions(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_Move(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_response_Imaging_GetStatus(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_response_Imaging_Stop(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);

/***************************************** ptz ***************************************/
GK_S32 soap_response_PTZ_GetServiceCapabilities(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetNodes(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetNode(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetConfigurations(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetConfigurationOptions(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_SetConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_ContinuousMove(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_response_PTZ_Stop(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetPresets(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_SetPreset(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GotoPreset(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_RemovePreset(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);





GK_S32 soap_response_PTZ_GetStatus(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_AbsoluteMove(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_RelativeMove(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);

/********************************* analytics *******************************************/
GK_S32 soap_response_VideoAnalytics_GetServiceCapabilities(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetSupportedAnalyticsModules(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetAnalyticsModules(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetSupportedRules(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);
GK_S32 soap_packet_response_GetRules(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header);


#ifdef __cplusplus
}
#endif

#endif


