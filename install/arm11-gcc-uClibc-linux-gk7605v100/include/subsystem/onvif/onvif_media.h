/*!
*****************************************************************************
** \file        solution/software/subsystem/onvif-no-gsoap/src/onvif_media.h
**
** \brief       onvif meida
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2013 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _ONVIF_MEDIA_H_
#define _ONVIF_MEDIA_H_

#include "onvif_std.h"
#include "onvif_ptz.h"

typedef enum
{
    VideoEncoding_JPEG = 0,
    VideoEncoding_MPEG = 1,
    VideoEncoding_H264 = 2
}GONVIF_MEDIA_VideoEncoding_E;

typedef enum 
{
    Mpeg4Profile_SP  = 0,
    Mpeg4Profile_ASP = 1
}GONVIF_MEDIA_Mepg4Profile_E;

typedef enum
{
    H264Profile_BaseLine    = 0,
    H264Profile_Main        = 1,
    H264Profile_Extended    = 2,
    H264Profile_High        = 3
}GONVIF_MEDIA_H264Profile_E;

typedef enum 
{
    AudioEncoding_G711 = 0,
    AudioEncoding_G726 = 1,
    AudioEncoding_AAC  = 2
}GONVIF_MEDIA_AudioEncoding_E;

typedef enum 
{
    StreamType_RTP_Unicast = 0,
    StreamType_RTP_Multicast = 1
}GONVIF_MEDIA_StreamType_E;

typedef enum 
{
    TransportProtocol_UDP = 0,
    TransportProtocol_TCP = 1,
    TransportProtocol_RTSP = 2,
    TransportProtocol_HTTP = 3
}GONVIF_MEDIA_TransportProtocol_E;

typedef enum
{
    OSDType_Text = 0,
    OSDType_Iamge = 1
}GONVIF_MEDIA_OSDType_E;

typedef enum
{
    PositionOption_Custom = 0,
    PositionOption_UpperLeft = 1,
    PositionOption_LowerLeft = 2
}GONVIF_MEDIA_PositionOption_E;

typedef enum
{
    TextType_Plain = 0,
    TextType_Date = 1,
    TextType_Time = 2,
    TextType_DateAndTime = 3
}GONVIF_MEDIA_TextType_E;

typedef enum
{
    DateFormat_MMddyyyy = 0,  // MM/dd/yyyy
    DateFormat_ddMMyyyy = 1,  // dd/MM/yyyy
    DateFormat_yyyyMMdd = 2,  // yyyy/MM/dd
    DateFormat_yyyy_MM_dd = 3 // yyyy-MM-dd
}GONVIF_MEDIA_DateFormat_E;

typedef enum
{
    TimeFormat_HHmmss = 0,   // HH:mm:ss     12
    TimeFormat_hhmmsstt = 1   // hh:mm:ss tt 24
}GONVIF_MEDIA_TimeFormat_E; 

//*****************************************************************************
//*****************************************************************************
//** Data Structures
//*****************************************************************************
//*****************************************************************************
typedef struct 
{
    GK_S32 x;
    GK_S32 y;
    GK_S32 width;
    GK_S32 height;
}GONVIF_MEDIA_IntRectangle_S;

typedef struct 
{
    GK_S32 width;
    GK_S32 height;
}GONVIF_MEDIA_VideoResolution_S;

typedef struct 
{
    GK_S32 frameRateLimit;
    GK_S32 encodingInterval;
    GK_S32 bitrateLimit;
}GONVIF_MEDIA_VideoRateControl_S;

typedef struct
{
    GONVIF_IPType_E enType;
    GK_CHAR *pszIPv4Address;
    GK_CHAR *pszIPv6Address;
}GONVIF_MEDIA_IpAddress_S;


typedef struct 
{
    GK_S32 govLength;
    GONVIF_MEDIA_Mepg4Profile_E enMpeg4Profile;
}GONVIF_MEDIA_Mpeg4Configuration_S;

typedef struct
{
    GK_S32 govLength;
    GONVIF_MEDIA_H264Profile_E enH264Profile;
}GONVIF_MEDIA_H264Configuration_S;


typedef struct 
{
    GONVIF_MEDIA_IpAddress_S stIPAddress;
    GK_S32  port;
    GK_S32  ttl;
    GONVIF_Boolean_E enAutoStart;
}GONVIF_MEDIA_MulticastConfiguration_S;



typedef struct 
{
    GK_CHAR *pszName;
    GK_CHAR *pszValue;
}GONVIF_MEDIA_ItemList_SimpleItem_S;

typedef struct 
{
    GK_CHAR *pszName;
}GONVIF_MEDIA_ItemList_ElementItem_S;

typedef struct 
{
    GK_S32 sizeSimpleItem;
    GONVIF_MEDIA_ItemList_SimpleItem_S *pstSimpleItem;
    GK_S32 sizeElementItem;
    GONVIF_MEDIA_ItemList_ElementItem_S *pstElementItem;
}GONVIF_MEDIA_ItemList_S;

typedef struct 
{
    GONVIF_MEDIA_ItemList_S stParameters;
    GK_CHAR *pszName;
    GK_CHAR *pszType;
}GONVIF_MEDIA_Config_S;

typedef struct 
{
    GK_S32 sizeRule;
    GONVIF_MEDIA_Config_S *pstRule;
}GONVIF_MEDIA_RuleEngineCfg_S;

typedef struct 
{
    GK_S32 sizeAnalyticsModule;
    GONVIF_MEDIA_Config_S *pstAnalyticsModule;
}GONVIF_MEDIA_AnalyticsEngineConfiguration_S;

typedef struct 
{
    GONVIF_Boolean_E enStatus;
    GONVIF_Boolean_E enPosition;
}GONVIF_MEDIA_PTZFilter_S;

typedef struct 
{
    GK_CHAR *pszToken;
    GK_CHAR *pszName;
    GK_CHAR *pszSourceToken;
    GK_S32  useCount;
    GONVIF_MEDIA_IntRectangle_S stBounds;
}GONVIF_MEDIA_VideoSourceConfiguration_S;

typedef struct 
{
    GK_CHAR *pszName;
    GK_CHAR *pszToken;
    GK_CHAR *pszSourceToken;
    GK_S32  useCount;
}GONVIF_MEDIA_AudioSourceConfiguration_S;


typedef struct 
{
    GK_CHAR *pszName;
    GK_S32  useCount;
    GK_CHAR *pszToken;
    GONVIF_MEDIA_VideoEncoding_E enEncoding;
    GONVIF_MEDIA_VideoResolution_S stResolution;
    GK_FLOAT quality;
    GONVIF_MEDIA_VideoRateControl_S  stRateControl;
    GONVIF_MEDIA_Mpeg4Configuration_S stMPEG4;
    GONVIF_MEDIA_H264Configuration_S  stH264;
    GONVIF_MEDIA_MulticastConfiguration_S stMulticast;
    GK_S64 sessionTimeout;
}GONVIF_MEDIA_VideoEncoderConfiguration_S;

typedef struct 
{
    GK_CHAR *pszName;
    GK_CHAR *pszToken;
    GK_S32  useCount;
    GONVIF_MEDIA_AudioEncoding_E enEncoding;
    GK_S32 bitRate;
    GK_S32 sampleRate;
    GONVIF_MEDIA_MulticastConfiguration_S stMulticast;
    GK_S64 sessionTimeout;
}GONVIF_MEDIA_AudioEncoderConfiguration_S;

typedef struct
{
    GK_CHAR *pszName;
    GK_S32  useCount;
    GK_CHAR *pszToken;
    GONVIF_MEDIA_AnalyticsEngineConfiguration_S stAnalyticsEngineConfiguration;
    GONVIF_MEDIA_RuleEngineCfg_S stRuleEngineConfiguration;
}GONVIF_MEDIA_VideoAnalyticsConfiguration_S;

typedef struct 
{
    GK_CHAR *pszName;
    GK_S32  useCount;
    GK_CHAR *pszToken;
    GONVIF_MEDIA_PTZFilter_S  stPTZStatus;
    GONVIF_Boolean_E enAnalytics;
    GONVIF_MEDIA_MulticastConfiguration_S stMulticast;
    GK_S64 sessionTimeout;
}GONVIF_MEDIA_MetadataConfiguration_S;

typedef struct
{
  GONVIF_IntRange_S stXRange;
  GONVIF_IntRange_S stYRange;
  GONVIF_IntRange_S stWidthRange;
  GONVIF_IntRange_S stHeightRange;
}GONVIF_MEDIA_IntrectangleRange_S;

typedef struct
{
    GONVIF_MEDIA_IntrectangleRange_S stBoundsRange;
    GK_S32 sizeVideoSourceTokensAvailable;
    GK_CHAR **pszVideoSourceTokensAvailable;
}GONVIF_MEDIA_VideoSourceConfigurationOptions_S;


typedef struct 
{ 
    GK_S32 sizeResolutionsAvailable; 
    GONVIF_MEDIA_VideoResolution_S *pstResolutionsAvailable; 
    GONVIF_IntRange_S stFrameRateRange; 
    GONVIF_IntRange_S stEncodingIntervalRange; 
}GONVIF_MEDIA_JpegOptions_S;

typedef struct 
{ 
    GK_S32 sizeResolutionsAvailable; 
    GONVIF_MEDIA_VideoResolution_S *pstResolutionsAvailable; 
    GONVIF_IntRange_S stFrameRateRange; 
    GONVIF_IntRange_S stEncodingIntervalRange; 
    GONVIF_IntRange_S stBitrateRange;
}GONVIF_MEDIA_JpegOptions2_S;

typedef struct 
{ 
    GK_S32 sizeResolutionsAvailable; 
    GONVIF_MEDIA_VideoResolution_S *pstResolutionAvailable; 
    GONVIF_IntRange_S stGovLengthRange; 
    GONVIF_IntRange_S stFrameRateRange; 
    GONVIF_IntRange_S stEncodingIntervalRange; 
    GK_S32 sizeMpeg4ProfilesSupported; 
    GONVIF_MEDIA_Mepg4Profile_E *peMpeg4ProfilesSupported; 
}GONVIF_MEDIA_Mpeg4Options_S; 

typedef struct 
{ 
    GK_S32 sizeResolutionsAvailable; 
    GONVIF_MEDIA_VideoResolution_S *pstResolutionAvailable; 
    GONVIF_IntRange_S stGovLengthRange; 
    GONVIF_IntRange_S stFrameRateRange; 
    GONVIF_IntRange_S stEncodingIntervalRange; 
    GK_S32 sizeMpeg4ProfilesSupported; 
    GONVIF_MEDIA_Mepg4Profile_E *peMpeg4ProfilesSupported; 
    GONVIF_IntRange_S stBitrateRange;
}GONVIF_MEDIA_Mpeg4Options2_S; 

typedef struct 
{ 
    GK_S32 sizeResolutionsAvailable; 
    GONVIF_MEDIA_VideoResolution_S *pstResolutionsAvailable; 
    GONVIF_IntRange_S stGovLengthRange; 
    GONVIF_IntRange_S stFrameRateRange; 
    GONVIF_IntRange_S stEncodingIntervalRange; 
    GK_S32 sizeH264ProfilesSupported; 
    GONVIF_MEDIA_H264Profile_E  *penH264ProfilesSupported;
}GONVIF_MEDIA_H264Options_S;

typedef struct 
{ 
    GK_S32 sizeResolutionsAvailable; 
    GONVIF_MEDIA_VideoResolution_S *pstResolutionsAvailable; 
    GONVIF_IntRange_S stGovLengthRange; 
    GONVIF_IntRange_S stFrameRateRange; 
    GONVIF_IntRange_S stEncodingIntervalRange; 
    GK_S32 sizeH264ProfilesSupported; 
    GONVIF_MEDIA_H264Profile_E  *penH264ProfilesSupported;
    GONVIF_IntRange_S stBitrateRange;    
}GONVIF_MEDIA_H264Options2_S;

typedef struct
{
	GONVIF_MEDIA_JpegOptions2_S stJPEG;
	GONVIF_MEDIA_Mpeg4Options2_S stMPEG4;
	GONVIF_MEDIA_H264Options2_S stH264;
}GONVIF_MEDIA_VideoEncoderOptionsExtension_S;

typedef struct 
{ 
  GONVIF_IntRange_S stQualityRange; 
  GONVIF_MEDIA_JpegOptions_S stJPEG; 
  GONVIF_MEDIA_Mpeg4Options_S stMPEG4; 
  GONVIF_MEDIA_H264Options_S  stH264; 
  GONVIF_MEDIA_VideoEncoderOptionsExtension_S stExtension;
}GONVIF_MEDIA_VideoEncoderConfigurationOptions_S;


typedef struct 
{
    GK_S32 sizeInputTokensAvailable;
    GK_CHAR **pszInputTokensAvailable;
}GONVIF_MEDIA_AudioSourceConfigurationOptions_S;

typedef struct 
{
    GK_S32 sizeItems;
    GK_S32 *pItems;
}GONVIF_MEDIA_Intlist_S;


typedef struct
{
  GONVIF_MEDIA_AudioEncoding_E enEncoding;
  GONVIF_MEDIA_Intlist_S stBitrateList;
  GONVIF_MEDIA_Intlist_S stSampleRateList;
}GONVIF_MEDIA_AudioEncoderConfigurationOption_S;

typedef struct
{
    GK_S32 sizeOptions;
    GONVIF_MEDIA_AudioEncoderConfigurationOption_S *pstOptions;
}GONVIF_MEDIA_AudioEncoderConfigurationOptions_S;

typedef struct 
{
    GK_CHAR *pszName;
    GK_CHAR *pszToken;
    GONVIF_Boolean_E enFixed;
    GONVIF_MEDIA_VideoSourceConfiguration_S    *pstVideoSourceConfiguration;
    GONVIF_MEDIA_AudioSourceConfiguration_S    *pstAudioSourceConfiguration;
    GONVIF_MEDIA_VideoEncoderConfiguration_S   *pstVideoEncoderConfiguration;
    GONVIF_MEDIA_AudioEncoderConfiguration_S   *pstAudioEncoderConfiguration;
    GONVIF_MEDIA_VideoAnalyticsConfiguration_S *pstVideoAnalyticsConfiguration;
    GONVIF_PTZ_Configuration_S 		           *pstPTZConfiguration;
    GONVIF_MEDIA_MetadataConfiguration_S 	   *pstMetadataConfiguration;
}GONVIF_MEDIA_Profile_S;

typedef struct kk
{
    GONVIF_MEDIA_TransportProtocol_E enProtocol;
    struct kk *pstTunnel;
}GONVIF_MEDIA_Transport_S;

typedef struct 
{
    GONVIF_MEDIA_StreamType_E enStream;
    GONVIF_MEDIA_Transport_S stTransport;
}GONVIF_MEDIA_StreamSetup_S;

typedef struct 
{
    GK_CHAR aszUri[MAX_URI_LENGTH];
    GONVIF_Boolean_E enInvalidAfterConnect;
    GONVIF_Boolean_E enInvalidAfterReboot;
    GK_S64  timeout;
}GONVIF_MEDIA_MediaUri_S;

typedef struct
{
    GK_S32 maximumNumberOfProfiles;    
}GONVIF_MEDIA_ProfileCapabilities_S;

typedef struct
{
	GONVIF_Boolean_E enRTPMulticast;
	GONVIF_Boolean_E enRTP_USCORETCP;
	GONVIF_Boolean_E enRTP_USCORERTSP_USCORETCP;
	GONVIF_Boolean_E enNonAggregateControl;
	GONVIF_Boolean_E enNoRTSPStreaming;
}GONVIF_MEDIA_StreamingCapabilities_S;

typedef struct 
{
    GONVIF_MEDIA_ProfileCapabilities_S stProfileCapabilities;
	GONVIF_MEDIA_StreamingCapabilities_S stStreamingCapabilities;
	GONVIF_Boolean_E enSnapshotUri;
	GONVIF_Boolean_E enRotation;
	GONVIF_Boolean_E enVideoSourceMode;
	GONVIF_Boolean_E enOSD;
}GONVIF_MEDIA_Capabilities_S;


typedef struct
{
    GONVIF_MEDIA_Capabilities_S stCapabilities;
}GONVIF_MEDIA_GetServiceCapabilities_Res_S;

typedef struct 
{
    GK_CHAR aszName[MAX_NAME_LENGTH];
    GK_CHAR aszToken[MAX_TOKEN_LENGTH];
}GONVIF_MEDIA_CreateProfile_S;

typedef struct
{
    GONVIF_MEDIA_Profile_S stProfile;    
}GONVIF_MEDIA_CreateProfile_Res_S;

typedef struct 
{
    GK_CHAR aszProfileToken[MAX_TOKEN_LENGTH];
}GONVIF_MEDIA_GetProfile_S;

typedef struct
{
    GONVIF_MEDIA_Profile_S stProfile;
}GONVIF_MEDIA_GetProfile_Res_S;

typedef struct
{
    GK_S32 sizeProfiles;
    GONVIF_MEDIA_Profile_S stProfiles[MAX_PROFILE_NUM];
}GONVIF_MEDIA_GetProfiles_Res_S;

typedef struct 
{
    GK_CHAR  aszProfileToken[MAX_TOKEN_LENGTH];
}GONVIF_MEDIA_DeleteProfile_S;

typedef struct 
{
    GK_CHAR aszProfileToken[MAX_TOKEN_LENGTH];
    GK_CHAR aszConfigurationToken[MAX_TOKEN_LENGTH];
}GONVIF_MEDIA_AddVideoSourceConfiguration_S;

typedef struct 
{
    GK_CHAR aszProfileToken[MAX_TOKEN_LENGTH];
    GK_CHAR aszConfigurationToken[MAX_TOKEN_LENGTH];
}GONVIF_MEDIA_AddVideoEncoderConfiguration_S;

typedef struct 
{
    GK_CHAR aszProfileToken[MAX_TOKEN_LENGTH];
    GK_CHAR aszConfigurationToken[MAX_TOKEN_LENGTH];
}GONVIF_MEDIA_AddAudioSourceConfiguration_S;

typedef struct 
{
   GK_CHAR aszProfileToken[ MAX_TOKEN_LENGTH];
   GK_CHAR aszConfigurationToken[MAX_TOKEN_LENGTH];
}GONVIF_MEDIA_AddAudioEncoderConfiguration_S;

typedef struct 
{
    GK_CHAR aszProfileToken[MAX_TOKEN_LENGTH];
    GK_CHAR aszConfigurationToken[MAX_TOKEN_LENGTH];
}GONVIF_MEDIA_AddPTZConfiguration_S;

typedef struct 
{
    GK_CHAR aszProfileToken[MAX_TOKEN_LENGTH];
}GONVIF_MEDIA_RemoveVideoSourceConfiguration_S;

typedef struct 
{
    GK_CHAR aszProfileToken[MAX_TOKEN_LENGTH];
}GONVIF_MEDIA_RemoveVideoEncoderConfiguration_S;

typedef struct 
{
    GK_CHAR  aszProfileToken[MAX_TOKEN_LENGTH];
}GONVIF_MEDIA_RemoveAudioSourceConfiguration_S;

typedef struct
{
    GK_CHAR  aszProfileToken[MAX_TOKEN_LENGTH];
}GONVIF_MEDIA_RemoveAudioEncoderConfiguration_S;

typedef struct 
{
    GK_CHAR aszProfileToken[MAX_TOKEN_LENGTH];
}GONVIF_MEDIA_RemovePTZConfiguration_S;

typedef struct 
{
    GK_S32 sizeConfigurations;
    GONVIF_MEDIA_VideoSourceConfiguration_S stConfigurations[MAX_VIDEOSOURCE_NUM];
}GONVIF_MEDIA_GetVideoSourceConfigurations_Res_S;

typedef struct 
{
    GK_CHAR  aszConfigurationToken[MAX_TOKEN_LENGTH];
}GONVIF_MEDIA_GetVideoSourceConfiguration_S;

typedef struct
{
    GONVIF_MEDIA_VideoSourceConfiguration_S stConfiguration;
}GONVIF_MEDIA_GetVideoSourceConfiguration_Res_S;

typedef struct 
{
    GK_CHAR aszProfileToken[MAX_TOKEN_LENGTH];
}GONVIF_MEDIA_GetCompatibleVideoSourceConfigurations_S;

typedef struct
{
    GK_S32 sizeConfigurations;
    GONVIF_MEDIA_VideoSourceConfiguration_S stConfigurations[MAX_VIDEOSOURCE_NUM];
}GONVIF_MEDIA_GetCompatibleVideoSourceConfigurations_Res_S;

typedef struct
{
    GK_CHAR aszConfigurationToken[MAX_TOKEN_LENGTH];
    GK_CHAR aszProfileToken[MAX_TOKEN_LENGTH];
}GONVIF_MEDIA_GetVideoSourceConfigurationOptions_S;

typedef struct
{
    GONVIF_MEDIA_VideoSourceConfigurationOptions_S stOptions;
}GONVIF_MEDIA_GetVideoSourceConfigurationOptions_Res_S;

typedef struct 
{
    GONVIF_MEDIA_VideoSourceConfiguration_S stVideoSourceConfiguration;
    GONVIF_Boolean_E enForcePersistence;
}GONVIF_MEDIA_SetVideoSourceConfiguration_S;

typedef struct 
{
    GK_S32 sizeConfigurations;
    GONVIF_MEDIA_VideoEncoderConfiguration_S stConfigurations[MAX_VIDEOENCODE_NUM];
}GONVIF_MEDIA_GetVideoEncoderConfigurations_Res_S;

typedef struct 
{
    GK_CHAR aszConfigurationToken[MAX_TOKEN_LENGTH];
}GONVIF_MEDIA_GetVideoEncoderConfiguration_S;

typedef struct
{
    GONVIF_MEDIA_VideoEncoderConfiguration_S stConfiguration;
}GONVIF_MEDIA_GetVideoEncoderConfiguration_Res_S;

typedef struct 
{
    GK_CHAR aszProfileToken[MAX_TOKEN_LENGTH];
}GONVIF_MEDIA_GetCompatibleVideoEncoderConfigurations_S;

typedef struct 
{
    GK_S32 sizeConfigurations;
    GONVIF_MEDIA_VideoEncoderConfiguration_S stConfigurations[MAX_VIDEOENCODE_NUM];
}GONVIF_MEDIA_GetCompatibleVideoEncoderConfigurations_Res_S;

typedef struct 
{
    GK_CHAR aszConfigurationToken[MAX_TOKEN_LENGTH];
    GK_CHAR aszProfileToken[MAX_TOKEN_LENGTH];
}GONVIF_MEDIA_GetVideoEncoderConfigurationOptions_S;

typedef struct
{
    GONVIF_MEDIA_VideoEncoderConfigurationOptions_S stOptions;
}GONVIF_MEDIA_GetVideoEncoderConfigurationOptions_Res_S;

typedef struct
{
    GONVIF_MEDIA_VideoEncoderConfiguration_S stConfiguration;
    GONVIF_Boolean_E enForcePersistence;
}GONVIF_MEDIA_SetVideoEncoderConfiguration_S;

typedef struct 
{
    GK_CHAR aszConfigurationToken[MAX_TOKEN_LENGTH];
}GONVIF_MEDIA_GetGuaranteedNumberOfVideoEncoderInstances_S;

typedef struct 
{
    GK_S32 totalNumber;
    GK_S32 JPEG;
    GK_S32 H264;
    GK_S32 MPEG4;
}GONVIF_MEDIA_GetGuaranteedNumberOfVideoEncoderInstances_Res_S;

typedef struct
{
    GK_S32 sizeConfigurations;
    GONVIF_MEDIA_AudioSourceConfiguration_S stConfigurations[MAX_AUDIOSOURCE_NUM];
}GONVIF_MEDIA_GetAudioSourceConfigurations_Res_S;

typedef struct
{
    GK_CHAR aszConfigurationToken[MAX_TOKEN_LENGTH];
}GONVIF_MEDIA_GetAudioSourceConfiguration_S;

typedef struct
{
    GONVIF_MEDIA_AudioSourceConfiguration_S stConfiguration;
}GONVIF_MEDIA_GetAudioSourceConfiguration_Res_S;

typedef struct 
{
    GK_CHAR aszProfileToken[MAX_TOKEN_LENGTH];
}GONVIF_MEDIA_GetCompatibleAudioSourceConfigurations_S;

typedef struct
{
    GK_S32 sizeConfigurations;
    GONVIF_MEDIA_AudioSourceConfiguration_S stConfigurations[MAX_AUDIOSOURCE_NUM];
}GONVIF_MEDIA_GetCompatibleAudioSourceConfigurations_Res_S;

typedef struct 
{
    GK_CHAR aszConfigurationToken[MAX_TOKEN_LENGTH];
    GK_CHAR aszProfileToken[MAX_TOKEN_LENGTH];
}GONVIF_MEDIA_GetAudioSourceConfigurationOptions_S;

typedef struct
{
    GONVIF_MEDIA_AudioSourceConfigurationOptions_S stOptions;
}GONVIF_MEDIA_GetAudioSourceConfigurationOptions_Res_S;

typedef struct 
{
    GONVIF_MEDIA_AudioSourceConfiguration_S stConfiguration;
    GONVIF_Boolean_E enForcePersistence;
}GONVIF_MEDIA_SetAudioSourceConfiguration_S;

typedef struct 
{
    GK_S32 sizeConfigurations;
    GONVIF_MEDIA_AudioEncoderConfiguration_S stConfigurations[MAX_AUDIOENCODE_NUM];
}GONVIF_MEDIA_GetAudioEncoderConfigurations_Res_S;

typedef struct 
{
    GK_CHAR aszConfigurationToken[MAX_TOKEN_LENGTH];
}GONVIF_MEDIA_GetAudioEncoderConfiguration_S;

typedef struct
{
    GONVIF_MEDIA_AudioEncoderConfiguration_S stConfiguration;
}GONVIF_MEDIA_GetAudioEncoderConfiguration_Res_S;

typedef struct
{
    GK_CHAR aszProfileToken[MAX_TOKEN_LENGTH];
}GONVIF_MEDIA_GetCompatibleAudioEncoderConfigurations_S;

typedef struct 
{
    GK_S32 sizeConfigurations;
    GONVIF_MEDIA_AudioEncoderConfiguration_S stConfigurations[MAX_AUDIOENCODE_NUM];
}GONVIF_MEDIA_GetCompatibleAudioEncoderConfigurations_Res_S;

typedef struct 
{
    GK_CHAR aszConfigurationToken[MAX_TOKEN_LENGTH];
    GK_CHAR aszProfileToken[MAX_TOKEN_LENGTH];
}GONVIF_MEDIA_GetAudioEncoderConfigurationOptions_S;

typedef struct
{
    GONVIF_MEDIA_AudioEncoderConfigurationOptions_S stOptions;
}GONVIF_MEDIA_GetAudioEncoderConfigurationOptions_Res_S;

typedef struct 
{
    GONVIF_MEDIA_AudioEncoderConfiguration_S stConfiguration;
    GONVIF_Boolean_E enForcePersistence;
}GONVIF_MEDIA_SetAudioEncoderConfiguration_S;

typedef struct 
{
    GK_CHAR aszProfileToken[MAX_TOKEN_LENGTH];
}GONVIF_MEIDA_GetSnapShotUri_S;

typedef struct
{
    GONVIF_MEDIA_MediaUri_S stMediaUri;
}GONVIF_MEIDA_GetSnapShotUri_Res_S;

typedef struct 
{
    GONVIF_MEDIA_StreamSetup_S  stStreamSetup;
    GK_CHAR aszProfileToken[MAX_TOKEN_LENGTH];
}GONVIF_MEDIA_GetStreamUri_S;

typedef struct
{
    GONVIF_MEDIA_MediaUri_S stMediaUri;
}GONVIF_MEDIA_GetStreamUri_Res_S;

typedef struct 
{
    GK_CHAR aszConfigurationToken[MAX_TOKEN_LENGTH];
}GONVIF_MEDIA_GetOSDOptions_Token_S;

typedef struct
{
	GK_S32 Total;       //"10"
    GK_S32 Image;       //"4"
    GK_S32 PlainText;   //"5"
    GK_S32 Date;        //"1"
    GK_S32 Time;        //"1"
    GK_S32 DateAndTime; //"1"
}GONVIF_MEDIA_OSD_MaximumNumberOfOSDs_S;

typedef struct
{
	GONVIF_FloatRange_S stX; // "X":{"Min":"0.000000", "Max":"255.000000"},
    GONVIF_FloatRange_S stY; // "Y":{"Min":"0.000000", "Max":"255.000000"},
    GONVIF_FloatRange_S stZ; // "Y":{"Min":"0.000000", "Max":"255.000000"},
    GK_CHAR stColorspace[MAX_EVENT_DIALECT_LEN]; // "Colorspace":"http://www.onvif.org/ver10/colorspace/YCbCr"
}GONVIF_MEDIA_OSD_ColorspaceRange_S;

typedef struct
{
	GONVIF_MEDIA_OSD_ColorspaceRange_S stColorspaceRange;
}GONVIF_MEDIA_OSD_Color_S;

typedef struct
{
	GONVIF_MEDIA_OSD_Color_S stColor;
}GONVIF_MEDIA_OSD_FontColor_S;

typedef struct
{
    GK_S32 sizeType;
    GONVIF_MEDIA_TextType_E penType[MAX_OSD_Type_NUM]; // "Type":[ "Plain", "Date", "Time", "DateAndTime" ], 
    GONVIF_IntRange_S FontSizeRange; // "FontSizeRange":{ "Min":"16", "Max":"64" },
    GK_S32 sizeDateFormat;
    GONVIF_MEDIA_DateFormat_E penDateFormat[MAX_OSD_DateFormat_NUM]; // "DateFormat":[ "MM/dd/yyyy","dd/MM/yyyy","yyyy/MM/dd","yyyy-MM-dd"],
	GK_S32 sizeTimeFormat;
    GONVIF_MEDIA_TimeFormat_E penTimeFormat[MAX_OSD_TimeFormat_NUM]; // "TimeFormat":["hh:mm:ss tt", "HH:mm:ss"],
    GONVIF_MEDIA_OSD_FontColor_S stFontColor;
}GONVIF_MEDIA_OSD_TextOption_S;

typedef struct
{
	GONVIF_MEDIA_OSD_MaximumNumberOfOSDs_S stMaximumNumberOfOSDs;
	GONVIF_MEDIA_OSDType_E stType; // "Text",
	GK_S32 sizePositionOption;
    GONVIF_MEDIA_PositionOption_E penPositionOption[MAX_OSD_PositionOption_NUM]; // "PositionOption":[ "UpperLeft", "LowerLeft", "Custom"],
	GONVIF_MEDIA_OSD_TextOption_S stTextOption;
}GONVIF_MEDIA_OSD_Options_S;

typedef struct
{
    GONVIF_MEDIA_OSD_Options_S stOSDOptions;
}GONVIF_MEDIA_GetOSDOptions_S;

typedef struct 
{
    GK_CHAR aszOsdToken[MAX_TOKEN_LENGTH];
    GK_CHAR aszConfigurationToken[MAX_TOKEN_LENGTH];
}GONVIF_MEDIA_GetOSDs_Token_S;

typedef struct
{
    GK_FLOAT x;
    GK_FLOAT y;
}GONVIF_Pos_S;

typedef struct 
{
    GONVIF_MEDIA_PositionOption_E penType;
    GONVIF_Pos_S stPos;
}GONVIF_MEDIA_Position_S;

typedef struct 
{
    GONVIF_Boolean_E enChannelName;
}GONVIF_MEDIA_Extension_S;

typedef struct
{
    GK_CHAR aszPlainText[MAX_64_LENGTH];; // "DateFormat":[ "MM/dd/yyyy","dd/MM/yyyy","yyyy/MM/dd","yyyy-MM-dd"],
}GONVIF_MEDIA_OSD_TextString_Plain_S;

typedef struct
{
    GONVIF_MEDIA_DateFormat_E penDateFormat; // "DateFormat":[ "MM/dd/yyyy","dd/MM/yyyy","yyyy/MM/dd","yyyy-MM-dd"],
    GONVIF_MEDIA_TimeFormat_E penTimeFormat; // "TimeFormat":["hh:mm:ss tt", "HH:mm:ss"],
}GONVIF_MEDIA_OSD_TextString_DateAndTime_S;

typedef union
{
    GONVIF_MEDIA_OSD_TextString_Plain_S stPlain;
    GONVIF_MEDIA_OSD_TextString_DateAndTime_S stDateAndTime;
}GONVIF_MEDIA_TextType_S;

typedef struct
{
    GONVIF_MEDIA_TextType_E penType; // "Type":[ "Plain", "Date", "Time", "DateAndTime" ], 
    GONVIF_MEDIA_TextType_S stType; // "DateFormat":[ "MM/dd/yyyy","dd/MM/yyyy","yyyy/MM/dd","yyyy-MM-dd"],
    GK_S32 FontSize;
    GONVIF_MEDIA_OSD_FontColor_S stFontColor;
    GONVIF_MEDIA_Extension_S stExtension;
}GONVIF_MEDIA_OSD_TextString_S;

typedef struct
{
    GK_CHAR aszOsdToken[MAX_TOKEN_LENGTH];
	GK_CHAR aszConfigurationToken[MAX_TOKEN_LENGTH];
	GONVIF_MEDIA_OSDType_E stType; // "Text",
	GONVIF_MEDIA_Position_S stPosition;
	GONVIF_MEDIA_OSD_TextString_S stTextString;
}GONVIF_MEDIA_OSDs_S;

typedef struct
{
    GK_S32 sizeOSDs;
    GONVIF_MEDIA_OSDs_S stOSDs[2];
}GONVIF_MEDIA_GetOSDs_S;

typedef struct
{
    GONVIF_MEDIA_OSDs_S stOSD;
}GONVIF_MEDIA_SetOSD_S;

typedef struct
{
    GK_CHAR aszOsdToken[MAX_TOKEN_LENGTH];
}GONVIF_MEDIA_DeleteOSD_S;


typedef struct 
{
    GK_CHAR aszVideoSourceToken[MAX_TOKEN_LENGTH];
}GONVIF_MEDIA_GetPrivacyMaskOptions_Token_S;

typedef struct 
{
    GONVIF_FloatRange_S stXRange;
    GONVIF_FloatRange_S stYRange;
}GONVIF_MEDIA_PrivacyMaskOptionsPosition_S;

typedef struct
{
    GK_S32 MaximumNumberOfAreas;
	GONVIF_MEDIA_PrivacyMaskOptionsPosition_S stPosition;
}GONVIF_MEDIA_PrivacyMaskOptions_S;

typedef struct
{
    GONVIF_MEDIA_PrivacyMaskOptions_S stPrivacyMaskOptions;
}GONVIF_MEDIA_GetPrivacyMaskOptions_S;

typedef struct 
{
    GK_CHAR aszVideoSourceToken[MAX_TOKEN_LENGTH];
}GONVIF_MEDIA_GetPrivacyMasks_Token_S;

typedef struct 
{
    GK_FLOAT x;
    GK_FLOAT y;
}GONVIF_MEDIA_PrivacyMaskPoint_S;

typedef struct
{
    GONVIF_Boolean_E enMaskAreaPoint;
    GK_CHAR aszPrivacyMaskToken[MAX_TOKEN_LENGTH];
    GK_CHAR aszVideoSourceToken[MAX_TOKEN_LENGTH];
	GONVIF_MEDIA_PrivacyMaskPoint_S stMaskAreaPoint[4];
}GONVIF_MEDIA_PrivacyMask_S;

typedef struct
{
    GONVIF_MEDIA_PrivacyMask_S stPrivacyMasks[MAX_PM_NUM];
}GONVIF_MEDIA_GetPrivacyMasks_S;

typedef struct 
{
    GK_CHAR aszPrivacyMaskToken[MAX_TOKEN_LENGTH];
}GONVIF_MEDIA_DeletePrivacyMask_Token_S;

typedef ONVIF_RET (*GONVIF_MEDIA_GetServiceCapabilities)(GONVIF_MEDIA_GetServiceCapabilities_Res_S *pstMD_GetServiceCapabilitiesRes);
typedef ONVIF_RET (*GONVIF_MEDIA_CreateProfile)(GONVIF_MEDIA_CreateProfile_S *pstMD_CreateProfile, GONVIF_MEDIA_CreateProfile_Res_S *pstMD_CreateProfileRes);
typedef ONVIF_RET (*GONVIF_MEDIA_GetProfile)(GONVIF_MEDIA_GetProfile_S *pstMD_GetProfile, GONVIF_MEDIA_GetProfile_Res_S *pstMD_GetProfileRes);
typedef ONVIF_RET (*GONVIF_MEDIA_GetProfiles)(GONVIF_MEDIA_GetProfiles_Res_S *pstMD_GetProfilesRes);
typedef ONVIF_RET (*GONVIF_MEDIA_DeleteProfile)(GONVIF_MEDIA_DeleteProfile_S *pstMD_DeleteProfile);

typedef ONVIF_RET (*GONVIF_MEDIA_AddVideoSourceConfiguration)(GONVIF_MEDIA_AddVideoSourceConfiguration_S *pstMD_AddVideoSourceConfiguration);
typedef ONVIF_RET (*GONVIF_MEDIA_AddVideoEncoderConfiguration)(GONVIF_MEDIA_AddVideoEncoderConfiguration_S *pstMD_AddVideoEncoderConfiguration);
typedef ONVIF_RET (*GONVIF_MEDIA_AddAudioSourceConfiguration)(GONVIF_MEDIA_AddAudioSourceConfiguration_S *pstMD_AddAudioSourceConfiguration);
typedef ONVIF_RET (*GONVIF_MEDIA_AddAudioEncoderConfiguration)(GONVIF_MEDIA_AddAudioEncoderConfiguration_S *pstMD_AddAudioEncoderConfiguration);
typedef ONVIF_RET (*GONVIF_MEDIA_AddPTZConfiguration)(GONVIF_MEDIA_AddPTZConfiguration_S *pstMD_AddPTZConfiguration);

typedef ONVIF_RET (*GONVIF_MEDIA_RemoveVideoSourceConfiguration)(GONVIF_MEDIA_RemoveVideoSourceConfiguration_S *pstMD_RemoveVideoSourceConfiguration);
typedef ONVIF_RET (*GONVIF_MEDIA_RemoveVideoEncoderConfiguration)(GONVIF_MEDIA_RemoveVideoEncoderConfiguration_S *pstMD_RemoveVideoEncoderConfiguration);
typedef ONVIF_RET (*GONVIF_MEDIA_RemoveAudioSourceConfiguration)(GONVIF_MEDIA_RemoveAudioSourceConfiguration_S *pstMD_RemoveAudioSourceConfiguration);
typedef ONVIF_RET (*GONVIF_MEDIA_RemoveAudioEncoderConfiguration)(GONVIF_MEDIA_RemoveAudioEncoderConfiguration_S *pstMD_RemoveAudioEncoderConfiguration);
typedef ONVIF_RET (*GONVIF_MEDIA_RemovePTZConfiguration)(GONVIF_MEDIA_RemovePTZConfiguration_S *pstMD_RemovePTZConfiguration);


typedef ONVIF_RET (*GONVIF_MEDIA_GetVideoSourceConfigurations)(GONVIF_MEDIA_GetVideoSourceConfigurations_Res_S *pstMD_GetVideoSourceConfigurationsRes);
typedef ONVIF_RET (*GONVIF_MEDIA_GetVideoSourceConfiguration)(GONVIF_MEDIA_GetVideoSourceConfiguration_S *pstMD_GetVideoSourceConfiguration, GONVIF_MEDIA_GetVideoSourceConfiguration_Res_S *pstMD_GetVideoSourceConfigurationRes);
typedef ONVIF_RET (*GONVIF_MEDIA_GetCompatibleVideoSourceConfigurations)(GONVIF_MEDIA_GetCompatibleVideoSourceConfigurations_S *pstMD_GetCompatibleVideoSourceConfigurations, GONVIF_MEDIA_GetCompatibleVideoSourceConfigurations_Res_S *pstMD_GetCompatibleVideoSourceConfigurationsRes);
typedef ONVIF_RET (*GONVIF_MEDIA_GetVideoSourceConfigurationOptions)(GONVIF_MEDIA_GetVideoSourceConfigurationOptions_S *pstMD_GetVideoSourceConfigurationOptions, GONVIF_MEDIA_GetVideoSourceConfigurationOptions_Res_S *pstMD_GetVideoSourceConfigurationOptionsRes);
typedef ONVIF_RET (*GONVIF_MEDIA_SetVideoSourceConfiguration)(GONVIF_MEDIA_SetVideoSourceConfiguration_S *pstMD_SetVideoSourceConfiguration);

typedef ONVIF_RET (*GONVIF_MEDIA_GetVideoEncoderConfigurations)(GONVIF_MEDIA_GetVideoEncoderConfigurations_Res_S *pstMD_GetVideoEncoderConfigurationsRes);
typedef ONVIF_RET (*GONVIF_MEDIA_GetVideoEncoderConfiguration)(GONVIF_MEDIA_GetVideoEncoderConfiguration_S *pstMD_GetVideoEncoderConfiguration, GONVIF_MEDIA_GetVideoEncoderConfiguration_Res_S *pstMD_GetVideoEncoderConfigurationRes);
typedef ONVIF_RET (*GONVIF_MEDIA_GetCompatibleVideoEncoderConfigurations)(GONVIF_MEDIA_GetCompatibleVideoEncoderConfigurations_S *pstMD_GetCompatibleVideoEncoderConfigurations, GONVIF_MEDIA_GetCompatibleVideoEncoderConfigurations_Res_S *pstMD_GetCompatibleVideoEncoderConfigurationsRes);
typedef ONVIF_RET (*GONVIF_MEDIA_GetVideoEncoderConfigurationOptions)(GONVIF_MEDIA_GetVideoEncoderConfigurationOptions_S *pstMD_GetVideoEncoderConfigurationOptions, GONVIF_MEDIA_GetVideoEncoderConfigurationOptions_Res_S *pstMD_GetVideoEncoderConfigurationOptionsRes);
typedef ONVIF_RET (*GONVIF_MEDIA_SetVideoEncoderConfiguration)(GONVIF_MEDIA_SetVideoEncoderConfiguration_S *pstMD_etVideoEncoderConfiguration);
typedef ONVIF_RET (*GONVIF_MEDIA_GetGuaranteedNumberOfVideoEncoderInstances)(GONVIF_MEDIA_GetGuaranteedNumberOfVideoEncoderInstances_S *pstMD_GetGuaranteedNumberOfVideoEncoderInstances, GONVIF_MEDIA_GetGuaranteedNumberOfVideoEncoderInstances_Res_S *pstMD_GetGuaranteedNumberOfVideoEncoderInstancesRes);

typedef ONVIF_RET (*GONVIF_MEDIA_GetAudioSourceConfigurations)(GONVIF_MEDIA_GetAudioSourceConfigurations_Res_S *pstMD_GetAudioSourceConfigurationRes);
typedef ONVIF_RET (*GONVIF_MEDIA_GetAudioSourceConfiguration)(GONVIF_MEDIA_GetAudioSourceConfiguration_S *pstMD_GetAudioSourceConfiguration, GONVIF_MEDIA_GetAudioSourceConfiguration_Res_S *pstMD_GetAudioSourceConfigurationRes);
typedef ONVIF_RET (*GONVIF_MEDIA_GetCompatibleAudioSourceConfigurations)(GONVIF_MEDIA_GetCompatibleAudioSourceConfigurations_S *pstMD_GetCompatibleAudioSourceConfigurations, GONVIF_MEDIA_GetCompatibleAudioSourceConfigurations_Res_S *pstMD_GetCompatibleAudioSourceConfigurationsRes);
typedef ONVIF_RET (*GONVIF_MEDIA_GetAudioSourceConfigurationOptions)(GONVIF_MEDIA_GetAudioSourceConfigurationOptions_S *pstMD_GetAudioSourceConfigurationOptions, GONVIF_MEDIA_GetAudioSourceConfigurationOptions_Res_S *pstMD_GetAudioSourceConfigurationOptionsRes);
typedef ONVIF_RET (*GONVIF_MEDIA_SetAudioSourceConfiguration)(GONVIF_MEDIA_SetAudioSourceConfiguration_S *pstMD_SetAudioSourceConfiguration);

typedef ONVIF_RET (*GONVIF_MEDIA_GetAudioEncoderConfigurations)(GONVIF_MEDIA_GetAudioEncoderConfigurations_Res_S *pstMD_GetAudioEncoderConfigurationsRes);
typedef ONVIF_RET (*GONVIF_MEDIA_GetAudioEncoderConfiguration)(GONVIF_MEDIA_GetAudioEncoderConfiguration_S *pstMD_GetAudioEncoderConfiguration, GONVIF_MEDIA_GetAudioEncoderConfiguration_Res_S *pstMD_GetAudioEncoderConfigurationRes);
typedef ONVIF_RET (*GONVIF_MEDIA_GetCompatibleAudioEncoderConfigurations)(GONVIF_MEDIA_GetCompatibleAudioEncoderConfigurations_S *pstGetCompAECfgsReq,GONVIF_MEDIA_GetCompatibleAudioEncoderConfigurations_Res_S *pstAECfgsRes);
typedef ONVIF_RET (*GONVIF_MEDIA_GetAudioEncoderConfigurationOptions)(GONVIF_MEDIA_GetAudioEncoderConfigurationOptions_S *pstMD_GetAudioEncoderConfigurationOptions, GONVIF_MEDIA_GetAudioEncoderConfigurationOptions_Res_S *pstMD_GetAudioEncoderConfigurationOptionsRes);
typedef ONVIF_RET (*GONVIF_MEDIA_SetAudioEncoderConfiguration)(GONVIF_MEDIA_SetAudioEncoderConfiguration_S *pstMD_SetAudioEncoderConfiguration);

typedef ONVIF_RET (*GONVIF_MEDIA_GetStreamUri)(GONVIF_MEDIA_GetStreamUri_S *pstMD_GetStreamUri, GONVIF_MEDIA_GetStreamUri_Res_S *pstMMD_GetStreamUriRes);
typedef ONVIF_RET (*GONVIF_MEDIA_GetSnapshotUri)(GONVIF_MEIDA_GetSnapShotUri_S *pstMD_GetSnapShotUri, GONVIF_MEIDA_GetSnapShotUri_Res_S *pstMD_GetSnapShotUriRes);

typedef ONVIF_RET (*GONVIF_MEDIA_GetOSDOptions)(GONVIF_MEDIA_GetOSDOptions_Token_S *pstGetMediaOSDOptionsReq, GONVIF_MEDIA_GetOSDOptions_S *pstGetOSDOptionsRes);
typedef ONVIF_RET (*GONVIF_MEDIA_GetOSDs)(GONVIF_MEDIA_GetOSDs_Token_S *pstGetMediaOSDOptionsReq, GONVIF_MEDIA_GetOSDs_S *pstGetOSDsRes);
typedef ONVIF_RET (*GONVIF_MEDIA_SetOSD)(GONVIF_MEDIA_SetOSD_S *pstSetMediaSetOSDReq);
typedef ONVIF_RET (*GONVIF_MEDIA_DeleteOSD)(GONVIF_MEDIA_DeleteOSD_S *pstSetMediaDeleteOSDReq);

typedef ONVIF_RET (*GONVIF_MEDIA_GetPrivacyMaskOptions)(GONVIF_MEDIA_GetPrivacyMaskOptions_Token_S *pstGetPrivacyMaskOptionsReq, GONVIF_MEDIA_GetPrivacyMaskOptions_S *pstGetPrivacyMaskOptionsRes);
typedef ONVIF_RET (*GONVIF_MEDIA_GetPrivacyMasks)(GONVIF_MEDIA_GetPrivacyMasks_Token_S *pstGetPrivacyMasksReq, GONVIF_MEDIA_GetPrivacyMasks_S *pstGetPrivacyMasksRes);
typedef ONVIF_RET (*GONVIF_MEDIA_DeletePrivacyMask)(GONVIF_MEDIA_DeletePrivacyMask_Token_S *pstDeletePrivacyMaskReq);
typedef ONVIF_RET (*GONVIF_MEDIA_CreatePrivacyMask)(GONVIF_MEDIA_PrivacyMask_S *pstCreatePrivacyMaskReq);


typedef struct
{
    GONVIF_MEDIA_GetServiceCapabilities pfnMD_GetServiceCapabilities;
    GONVIF_MEDIA_CreateProfile pfnMD_CreateProfile;
    GONVIF_MEDIA_GetProfile    pfnMD_GetProfile;
    GONVIF_MEDIA_GetProfiles   pfnMD_GetProfiles;
    GONVIF_MEDIA_DeleteProfile pfnMD_DeleteProfile;
    
    GONVIF_MEDIA_AddVideoSourceConfiguration  pfnMD_AddVideoSourceConfiguration;    
    GONVIF_MEDIA_AddVideoEncoderConfiguration pfnMD_AddVideoEncoderConfiguration;
    GONVIF_MEDIA_AddAudioSourceConfiguration  pfnMD_AddAudioSourceConfiguration;
    GONVIF_MEDIA_AddAudioEncoderConfiguration pfnMD_AddAudioEncoderConfiguration;
    GONVIF_MEDIA_AddPTZConfiguration          pfnMD_AddPTZConfiguration;
    
    GONVIF_MEDIA_RemoveVideoSourceConfiguration  pfnMD_RemoveVideoSourceConfiguration;
    GONVIF_MEDIA_RemoveVideoEncoderConfiguration pfnMD_RemoveVideoEncoderConfiguration;
    GONVIF_MEDIA_RemoveAudioSourceConfiguration  pfnMD_RemoveAudioSourceConfiguration;
    GONVIF_MEDIA_RemoveAudioEncoderConfiguration pfnMD_RemoveAudioEncoderConfiguration;
    GONVIF_MEDIA_RemovePTZConfiguration          pfnMD_RemovePTZConfiguration;
    
    GONVIF_MEDIA_GetVideoSourceConfigurations           pfnMD_GetVideoSourceConfigurations;
    GONVIF_MEDIA_GetVideoSourceConfiguration            pfnMD_GetVideoSourceConfiguration;
    GONVIF_MEDIA_GetCompatibleVideoSourceConfigurations pfnMD_GetCompatibleVideoSourceConfigurations;
    GONVIF_MEDIA_GetVideoSourceConfigurationOptions     pfnMD_GetVideoSourceConfigurationOptions;
    GONVIF_MEDIA_SetVideoSourceConfiguration            pfnMD_SetVideoSourceConfiguration;
    
    GONVIF_MEDIA_GetVideoEncoderConfigurations              pfnMD_GetVideoEncoderConfigurations;
    GONVIF_MEDIA_GetVideoEncoderConfiguration               pfnMD_GetVideoEncoderConfiguration;
    GONVIF_MEDIA_GetCompatibleVideoEncoderConfigurations    pfnMD_GetCompatibleVideoEncoderConfigurations;
    GONVIF_MEDIA_GetVideoEncoderConfigurationOptions        pfnMD_GetVideoEncoderConfigurationOptions;
    GONVIF_MEDIA_SetVideoEncoderConfiguration               pfnMD_SetVideoEncoderConfiguration;
    GONVIF_MEDIA_GetGuaranteedNumberOfVideoEncoderInstances pfnMD_GetGuaranteedNumberOfVideoEncoderInstances;
    
    GONVIF_MEDIA_GetAudioSourceConfigurations           pfnMD_GetAudioSourceConfigurations;
    GONVIF_MEDIA_GetAudioSourceConfiguration            pfnMD_GetAudioSourceConfiguration;
    GONVIF_MEDIA_GetCompatibleAudioSourceConfigurations pfnMD_GetCompatibleAudioSourceConfigurations;
    GONVIF_MEDIA_GetAudioSourceConfigurationOptions     pfnMD_GetAudioSourceConfigurationOptions;
    GONVIF_MEDIA_SetAudioSourceConfiguration            pfnMD_SetAudioSourceConfiguration;

    GONVIF_MEDIA_GetAudioEncoderConfigurations           pfnMD_GetAudioEncoderConfigurations;
    GONVIF_MEDIA_GetAudioEncoderConfiguration            pfnMD_GetAudioEncoderConfiguration;
    GONVIF_MEDIA_GetCompatibleAudioEncoderConfigurations pfnMD_GetCompatibleAudioEncoderConfigurations;
    GONVIF_MEDIA_GetAudioEncoderConfigurationOptions     pfnMD_GetAudioEncoderConfigurationOptions;
    GONVIF_MEDIA_SetAudioEncoderConfiguration            pfnMD_SetAudioEncoderConfiguration;
    
    GONVIF_MEDIA_GetSnapshotUri pfnMD_GetSnapshotUri;
    GONVIF_MEDIA_GetStreamUri   pfnMD_GetStreamUri;

    GONVIF_MEDIA_GetOSDOptions pfnMD_GetOSDOptions;
    GONVIF_MEDIA_GetOSDs pfnMD_GetOSDs;
    GONVIF_MEDIA_SetOSD pfnMD_SetOSD;
    GONVIF_MEDIA_DeleteOSD pfnMD_DeleteOSD;
    GONVIF_MEDIA_GetPrivacyMaskOptions pfnMD_GetPrivacyMaskOptions;
    GONVIF_MEDIA_GetPrivacyMasks pfnMD_GetPrivacyMasks;
    GONVIF_MEDIA_DeletePrivacyMask pfnMD_DeletePrivacyMask;
    GONVIF_MEDIA_CreatePrivacyMask pfnMD_CreatePrivacyMask;
}GOnvif_MEDIA_CallbackFunc_S;


#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
}
#endif


#endif 

