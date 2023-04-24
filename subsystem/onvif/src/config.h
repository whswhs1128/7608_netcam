/*!
*****************************************************************************
** \file        solution/software/subsystem/onvif-no-gsoap/src/config_parse.h
**
** \brief       config file parse
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2013 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/


#ifndef _CONFIG_PARSE_H_
#define _CONFIG_PARSE_H_

#include "onvif_priv.h"


//*****************************************************************************
//*****************************************************************************
//** Defines and Macros
//*****************************************************************************
//*****************************************************************************


#define MEDIA_TAG					    "Media"
#define VIDEOSOURCE_TAG				    "VideoSource"
#define AUDIOSOURCE_TAG				    "AudioSource"
#define VIDEOENCODER_TAG			    "VideoEncoder"
#define AUDIOENCODER_TAG			    "AudioEncoder"
#define RELAYOUTPUT_TAG                 "RelayOutput"
#define MEDIAPROFILE_TAG			    "Profile"

#define VIDEOSOURCE_OPTION_TAG		    "VideoSourceOption"
#define AUDIOSOURCE_OPTION_TAG		    "AudioSourceOption"
#define VIDEOENCODER_OPTION_TAG		    "VideoEncoderOption"
#define AUDIOENCODER_OPTION_TAG		    "AudioEncoderOption"
#define PTZ_CONFIGURATION_OPTIONS_TAG   "PTZConfigurationOptions"
#define VIDEOANALYTIC_TAG			    "VideoAnalytics"
#define PTZ_TAG						    "PTZ"

#define IMAGEOPTION_TAG				    "ImagingOptions"
#define IMAGEMOVEOPTION_TAG			    "ImagingMoveOptions"
#define IMAGESETTING_TAG			    "ImagingSetting"



//*****************************************************************************
//*****************************************************************************
//** Enumerated types
//*****************************************************************************
//*****************************************************************************


//*****************************************************************************
//*****************************************************************************
//** Data Structures
//*****************************************************************************
//*****************************************************************************
typedef struct {
	GK_U8 Num;
	GK_S8 PresetName[MAX_NAME_LENGTH];
	GK_S8 PresetToken[MAX_TOKEN_LENGTH];
}Preset;

typedef struct listNode
{
	 Preset Preset;
	 struct listNode *Next;
}Node;



//*****************************************************************************
//*****************************************************************************
//** Global Data
//*****************************************************************************
//*****************************************************************************



//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************



#ifdef __cplusplus
extern "C" {
#endif
GK_S32 config_Load_DeviceXml(ONVIF_FILE_TYPE type);
GK_S32 config_Load_DiscoverXml(ONVIF_FILE_TYPE type);
GK_S32 config_Load_ImagingXml(ONVIF_FILE_TYPE type);
GK_S32 config_Load_MediadynamicXml(ONVIF_FILE_TYPE type);
GK_S32 config_Load_MediastaticXml(ONVIF_FILE_TYPE type);
GK_S32 config_Load_PtzXml(ONVIF_FILE_TYPE type);
GK_S32 config_Load_PtzPreset(GK_CHAR *filePath);

GK_S32 config_Save_DeviceXml(ONVIF_FILE_TYPE type);
GK_S32 config_Save_DiscoverXml(ONVIF_FILE_TYPE type);
GK_S32 config_Save_MediaXml(ONVIF_FILE_TYPE type);

GK_S32 config_GetScope(const GONVIF_DEVMNG_Scopes_S *scope);
GK_S32 config_AddUser(const GONVIF_DEVMNG_User_S *user);
GK_S32 config_FindUser(const GK_CHAR *name, GONVIF_DEVMNG_User_S **User);
GK_S32 config_DeleteUser(const GK_CHAR *name);
GK_S32 config_DeleteAllUsers(void);
const GK_CHAR *config_GetPassword(const GK_CHAR *username);

#ifdef __cplusplus
}
#endif

#endif

