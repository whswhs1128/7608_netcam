/*!
*****************************************************************************
** \file        packages/onvif/src/gk7101/gk7101_media.h
**
**
** \brief       Onvif Media Control Interface
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2013 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _GK7101_MEDIA_H_
#define _GK7101_MEDIA_H_

#include "onvif_std.h"


//*****************************************************************************
//*****************************************************************************
//** Defines and Macros
//*****************************************************************************
//*****************************************************************************
#define MSG_COMM_MEDIA_TYPE 		10
#define MSG_COMM_MEDIAACK_TYPE 		11


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
typedef struct ONVIF_Media{
	GK_ULONG streamid;
	GK_ULONG encode_type;
	GK_ULONG encode_width;
	GK_ULONG encode_height;
	GK_ULONG framerate;
	GK_ULONG bitrate;
	GK_ULONG encInterval;
	GK_DOUBLE quality;
	GK_ULONG govLength;
	GK_ULONG h264Profile;
	GK_ULONG stillCapFlag;
}ONVIF_Media_Cmd;


typedef struct MediaAdapterConfiguration
{
    GK_S32 (*GetMediaInfo)(GK_U8 datalen, GK_U8 *databuf);
    GK_S32 (*SetResolution)(GK_S32 streamid, GK_S32 width, GK_S32 height, GK_S32 encodetype);
    GK_S32 (*SetFrameRate)(GK_S32 streamid,GK_S32 value);
    GK_S32 (*SetBitRate)(GK_S32 streamid,GK_S32 value);
    GK_S32 (*SetEncodingInterval)(GK_S32 streamid,GK_S32 value);
    GK_S32 (*SetQuality)(GK_S32 streamid,GK_FLOAT value);
    GK_S32 (*SetGovLength)(GK_S32 streamid,GK_S32 value);
    GK_S32 (*SetstillCap)(GK_S32 streamid,GK_BOOL stillCapFlag);
}MediaAdapterConfiguration_t;


//*****************************************************************************
//*****************************************************************************
//** Global Data
//*****************************************************************************
//*****************************************************************************

extern MediaAdapterConfiguration_t gMediaAdapterConfiguration;


//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************



#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
}
#endif

#endif
