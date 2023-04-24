/*!
*****************************************************************************
** \file        packages/onvif/src/gk7101/gk7101_media.c
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
#include"gk7101_media.h"
#include "gk7101_cmd.h"

//*****************************************************************************
//*****************************************************************************
//** Local Defines
//*****************************************************************************
//*****************************************************************************



//*****************************************************************************
//*****************************************************************************
//** Local structures
//*****************************************************************************
//*****************************************************************************



//*****************************************************************************
//*****************************************************************************
//** Global Data
//*****************************************************************************
//*****************************************************************************

//*****************************************************************************
//*****************************************************************************
//** Local Functions Declaration
//*****************************************************************************
//*****************************************************************************

static GK_S32 mediaGetInfo(GK_U8 datalen, GK_U8 *databuf);
static GK_S32 mediaSetResolution(GK_S32 streamid, GK_S32 width, GK_S32 height, GK_S32 encodetype);
static GK_S32 mediaSetFrameRate(GK_S32 streamid, GK_S32 value);
static GK_S32 mediaSetBitRate(GK_S32 streamid, GK_S32 value);
static GK_S32 mediaSetEncodingInterval(GK_S32 streamid, GK_S32 value);
static GK_S32 mediaSetQuality(GK_S32 streamid, GK_FLOAT value);
static GK_S32 mediaSetGovLength(GK_S32 streamid, GK_S32 value);
static GK_S32 mediaSetStillCapture(GK_S32 streamid, GK_U32 stillCapFlag);

//*****************************************************************************
//*****************************************************************************
//** Local Data
//*****************************************************************************
//*****************************************************************************
MediaAdapterConfiguration_t gMediaAdapterConfiguration =
{
	.GetMediaInfo			= mediaGetInfo,
    .SetResolution       	= mediaSetResolution,
    .SetFrameRate       	= mediaSetFrameRate,
    .SetBitRate				= mediaSetBitRate,
    .SetEncodingInterval    = mediaSetEncodingInterval,
    .SetQuality     		= mediaSetQuality,
    .SetGovLength         	= mediaSetGovLength,
    .SetstillCap            = mediaSetStillCapture
};


//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************



//*****************************************************************************
//*****************************************************************************
//** Local Functions
//*****************************************************************************
//*****************************************************************************

static GK_S32 mediaGetInfo(GK_U8 datalen, GK_U8 *databuf)
{
	GK_S32 retVal;
	retVal = gk7101_Comm_CmdSend(MEDIA_GET_INFO, 0, NULL, datalen, databuf);
	return retVal;
}



/*set encoder and resolution*/
static GK_S32 mediaSetResolution(GK_S32 streamid, GK_S32 width, GK_S32 height, GK_S32 encodetype)
{
	GK_S32 retVal;

	ONVIF_Media_Cmd stCmd;
	memset(&stCmd, 0, sizeof(stCmd));
	stCmd.streamid = streamid;
	stCmd.encode_width = width;
	stCmd.encode_height = height;
    stCmd.encode_type = encodetype;
    if(encodetype == 0)//VideoEncoding_JPEG
    {
        stCmd.encode_type = 2;//GADI_VENC_TYPE_MJPEG
    }
    else if(encodetype == 1)//VideoEncoding_MPEG
    {
        stCmd.encode_type = 0;//GADI_VENC_TYPE_OFF
    }
    else if(encodetype == 2)//VideoEncoding_H264
    {
        stCmd.encode_type = 1;//GADI_VENC_TYPE_H264
    }
    else
    {
        stCmd.encode_type = 0;//GADI_VENC_TYPE_OFF
    }
	retVal = gk7101_Comm_CmdSend(MEDIA_SET_RESOLUTION, sizeof(ONVIF_Media_Cmd), (GK_U8 *)&stCmd, 0, NULL);

	return retVal;
}


/*set Frame rate*/
static GK_S32 mediaSetFrameRate(GK_S32 streamid, GK_S32 value)
{
	GK_S32 retVal;

	ONVIF_Media_Cmd stCmd;
	memset(&stCmd, 0, sizeof(stCmd));
	stCmd.streamid = streamid;
	stCmd.framerate = value;
	retVal = gk7101_Comm_CmdSend(MEDIA_SET_FRAMERATE, sizeof(ONVIF_Media_Cmd), (GK_U8 *)&stCmd, 0, NULL);

	return retVal;
}

/*set Bit rate*/
static GK_S32 mediaSetBitRate(GK_S32 streamid, GK_S32 value)
{
	GK_S32 retVal;

	ONVIF_Media_Cmd stCmd;
	memset(&stCmd, 0, sizeof(stCmd));
	stCmd.streamid = streamid;
	stCmd.bitrate = value;

	retVal = gk7101_Comm_CmdSend(MEDIA_SET_BITRATE, sizeof(ONVIF_Media_Cmd), (GK_U8 *)&stCmd, 0, NULL);

	return retVal;
}

/*set Encoding Interval*/
static GK_S32 mediaSetEncodingInterval(GK_S32 streamid, GK_S32 value)
{
	GK_S32 retVal;

	ONVIF_Media_Cmd stCmd;
	memset(&stCmd, 0, sizeof(stCmd));
	stCmd.streamid = streamid;
	stCmd.encInterval = value;

	retVal = gk7101_Comm_CmdSend(MEDIA_SET_ENCINTERVAL, sizeof(ONVIF_Media_Cmd), (GK_U8 *)&stCmd, 0, NULL);

	return retVal;
}


/*set Quality*/
static GK_S32 mediaSetQuality(GK_S32 streamid, GK_FLOAT value)
{
	GK_S32 retVal;

	ONVIF_Media_Cmd stCmd;
	memset(&stCmd, 0, sizeof(stCmd));
	stCmd.streamid = streamid;
	stCmd.quality = value;

	retVal = gk7101_Comm_CmdSend(MEDIA_SET_QUALITY, sizeof(ONVIF_Media_Cmd), (GK_U8 *)&stCmd, 0, NULL);

	return retVal;
}

/*set QualitGovLengthy*/
static GK_S32 mediaSetGovLength(GK_S32 streamid, GK_S32 value)
{
	GK_S32 retVal;

	ONVIF_Media_Cmd stCmd;
	memset(&stCmd, 0, sizeof(stCmd));
	stCmd.streamid = streamid;
	stCmd.govLength = value;

	retVal = gk7101_Comm_CmdSend(MEDIA_SET_GOVLENGTH, sizeof(ONVIF_Media_Cmd), (GK_U8 *)&stCmd, 0, NULL);

	return retVal;

}

static GK_S32 mediaSetStillCapture(GK_S32 streamid, GK_U32 stillCapFlag)
{
	GK_S32 retVal;

	ONVIF_Media_Cmd stCmd;
	memset(&stCmd, 0, sizeof(stCmd));
	stCmd.streamid = streamid;
	stCmd.stillCapFlag = stillCapFlag;
	retVal = gk7101_Comm_CmdSend(MEDIA_SET_STILLCAP, sizeof(ONVIF_Media_Cmd), (GK_U8 *)&stCmd, 0, NULL);

	return retVal;

}

