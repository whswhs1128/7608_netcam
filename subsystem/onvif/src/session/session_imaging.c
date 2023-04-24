/*!
*****************************************************************************
** \file        packages/onvif/src/gk7101_imaging.c
**
**
** \brief       Helper function for onvif.
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#include"gk7101_imaging.h"
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
static GK_S32 imagingGetInfo(GK_U8 dataLen, GK_U8 *dataBuf);
static GK_S32 imagingSetBlcMode(GK_U32 mode);
static GK_S32 imagingSetBlcLevel(GK_S32 level);
static GK_S32 imagingSetBrightness(GK_S32 brightness);
static GK_S32 imagingSetColorSaturation(GK_S32 saturation);
static GK_S32 imagingSetContrast(GK_S32 contrast);
static GK_S32 imagingSetExposureMode(GK_S32 mode);
static GK_S32 imagingSetAeShutterTimeMin(GK_S32 shutterMin);
static GK_S32 imagingSetAeShutterTimeMax(GK_S32 shutterMax);
static GK_S32 imagingSetAeGainMin(GK_S32 gainMin);
static GK_S32 imagingSetAeGainMax(GK_S32 gainMax);
static GK_S32 imagingSetMeShutterTime(GK_S32 shutter);
static GK_S32 imagingSetMeGain(GK_S32 gain);
static GK_S32 imagingSetFocusMode(GK_S32 mode);
static GK_S32 imagingSetAfNearLimit(GK_S32 nearLimit);
static GK_S32 imagingSetAfFarLimit(GK_S32 farLimit);
static GK_S32 imagingSetMfDefaultSpeed(GK_S32 defaultSpeed);
static GK_S32 imagingSetIrCutFilter(GK_S32 mode);
static GK_S32 imagingSetSharpness(GK_S32 sharpness);
static GK_S32 imagingSetWdrMode(GK_S32 mode);
static GK_S32 imagingSetWdrLevel(GK_S32 level);
static GK_S32 imagingSetWbMode(GK_S32 mode);
static GK_S32 imagingSetMwbRGain(GK_S32 rGain);
static GK_S32 imagingSetMwbBGain(GK_S32 bGain);
static GK_S32 imagingSetForcePersistence(GK_S32 ForcePersistence);


//*****************************************************************************
//*****************************************************************************
//** Local Data
//*****************************************************************************
//*****************************************************************************
ImageAdapterConfiguration_t gImageAdapterConfiguration =
{
	.imagingGetInfo					= imagingGetInfo,

    .imagingSetBlcMode       		= imagingSetBlcMode,
    .imagingSetBlcLevel       		= imagingSetBlcLevel,

    .imagingSetBrightness       	= imagingSetBrightness,
    .imagingSetColorSaturation		= imagingSetColorSaturation,
    .imagingSetContrast             = imagingSetContrast,

    .imagingSetExposureMode    		= imagingSetExposureMode,
    .imagingSetAeShutterTimeMin		= imagingSetAeShutterTimeMin,
    .imagingSetAeShutterTimeMax		= imagingSetAeShutterTimeMax,
    .imagingSetAeGainMin            = imagingSetAeGainMin,
    .imagingSetAeGainMax     		= imagingSetAeGainMax,
    .imagingSetMeShutterTime		= imagingSetMeShutterTime,
	.imagingSetMeGain				= imagingSetMeGain,

    .imagingSetFocusMode            = imagingSetFocusMode,
    .imagingSetAfNearLimit          = imagingSetAfNearLimit,
    .imagingSetAfFarLimit           = imagingSetAfFarLimit,
    .imagingSetMfDefaultSpeed       = imagingSetMfDefaultSpeed,

    .imagingSetIrCutFilter          = imagingSetIrCutFilter,
    .imagingSetSharpness            = imagingSetSharpness,

    .imagingSetWdrMode              = imagingSetWdrMode,
    .imagingSetWdrLevel             = imagingSetWdrLevel,

    .imagingSetWbMode		        = imagingSetWbMode,
    .imagingSetMwbRGain   	        = imagingSetMwbRGain,
    .imagingSetMwbBGain             = imagingSetMwbBGain,
    .imagingSetForcePersistence     = imagingSetForcePersistence,
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
/* Get imging setting information. */
static GK_S32 imagingGetInfo(GK_U8 dataLen, GK_U8 *dataBuf)
{
	GK_S32 retVal;
	retVal = gk7101_Comm_CmdSend(IMAGE_GET_INFO, 0, NULL, dataLen, dataBuf);
	return retVal;

}
static GK_S32 imagingSetForcePersistence(GK_S32 ForcePersistence)
{
	GK_S32 retVal;

	ONVIF_Image_Cmd stCmd;
	memset(&stCmd, 0, sizeof(stCmd));
	stCmd.settingsSave = ForcePersistence;

	retVal = gk7101_Comm_CmdSend(IMAGE_FORCE_PERSISTENCE, sizeof(ONVIF_Image_Cmd), (GK_U8 *)&stCmd, 0, NULL);

	return retVal;
}

/*BLC:0-disable, 1-enable*/
static GK_S32 imagingSetBlcMode(GK_U32 mode)
{
	GK_S32 retVal;

	ONVIF_Image_Cmd stCmd;
	memset(&stCmd, 0, sizeof(stCmd));
	stCmd.blcMode = mode;

	retVal = gk7101_Comm_CmdSend(IMAGE_SET_BLCMODE, sizeof(ONVIF_Image_Cmd), (GK_U8 *)&stCmd, 0, NULL);

	return retVal;
}

/* BLC level setting. */
static GK_S32 imagingSetBlcLevel(GK_S32 level)
{
	GK_S32 retVal;

	ONVIF_Image_Cmd stCmd;
	memset(&stCmd, 0, sizeof(stCmd));
	stCmd.blcLevel = level;

	retVal = gk7101_Comm_CmdSend(IMAGE_SET_BLCLEVEL, sizeof(ONVIF_Image_Cmd), (GK_U8 *)&stCmd, 0, NULL);

	return retVal;
}

/*Brightnes: valid range is: -255 - +255. */
static GK_S32 imagingSetBrightness(GK_S32 brightness)
{
	GK_S32 retVal;

	ONVIF_Image_Cmd stCmd;
	memset(&stCmd, 0, sizeof(stCmd));
	stCmd.brightness = brightness;

	retVal = gk7101_Comm_CmdSend(IMAGE_SET_BRIGHTNESS, sizeof(ONVIF_Image_Cmd), (GK_U8 *)&stCmd, 0, NULL);

	return retVal;
}

/*ColorSaturation: unit = 64,valid range is: 0 ~ +255. */

static GK_S32 imagingSetColorSaturation(GK_S32 saturation)
{
	GK_S32 retVal;

	ONVIF_Image_Cmd stCmd;
	memset(&stCmd, 0, sizeof(stCmd));
	stCmd.colorSaturation = saturation;

	retVal = gk7101_Comm_CmdSend(IMAGE_SET_COLOR_SATURATION, sizeof(ONVIF_Image_Cmd), (GK_U8 *)&stCmd, 0, NULL);

	return retVal;
}

/*Contrast: unit = 64, valid range is:  0 ~ +128. */
static GK_S32 imagingSetContrast(GK_S32 contrast)
{
	GK_S32 retVal;

	ONVIF_Image_Cmd stCmd;
	memset(&stCmd, 0, sizeof(stCmd));
	stCmd.contrast = contrast;

	retVal = gk7101_Comm_CmdSend(IMAGE_SET_CONTRAST, sizeof(ONVIF_Image_Cmd), (GK_U8 *)&stCmd, 0, NULL);

	return retVal;
}

/*Exposure setting. */
/*Mode:0-disable, 1-enable. */
static GK_S32 imagingSetExposureMode(GK_S32 mode)
{
	GK_S32 retVal;

	ONVIF_Image_Cmd stCmd;
	memset(&stCmd, 0, sizeof(stCmd));
	stCmd.exposureMode = mode;

	retVal = gk7101_Comm_CmdSend(IMAGE_SET_EXPOSUREMODE, sizeof(ONVIF_Image_Cmd), (GK_U8 *)&stCmd, 0, NULL);

	return retVal;
}

/*Auto exposure shutter time Min. */
/*new AE shutter time Min in 1/n sec format: range 120 ~ 8000. */
static GK_S32 imagingSetAeShutterTimeMin(GK_S32 shutterMin)
{
	GK_S32 retVal;

	ONVIF_Image_Cmd stCmd;
	memset(&stCmd, 0, sizeof(stCmd));
	stCmd.aeShutterTimeMin = shutterMin;

	retVal = gk7101_Comm_CmdSend(IMAGE_SET_AESHUTTERTIME_MIN, sizeof(ONVIF_Image_Cmd), (GK_U8 *)&stCmd, 0, NULL);

	return retVal;
}

/*Auto exposure shutter time Min. */
/*new AE shutter time max in 1/n sec format: range 1 ~ 100. */
static GK_S32 imagingSetAeShutterTimeMax(GK_S32 shutterMax)
{
	GK_S32 retVal;

	ONVIF_Image_Cmd stCmd;
	memset(&stCmd, 0, sizeof(stCmd));
	stCmd.aeShutterTimeMax = shutterMax;

	retVal = gk7101_Comm_CmdSend(IMAGE_SET_AESHUTTERTIME_MAX, sizeof(ONVIF_Image_Cmd), (GK_U8 *)&stCmd, 0, NULL);

	return retVal;
}


/*Auto exposure sensor gain Min. */
/*new AE sensor gain(db): range 0 ~ 60. */
static GK_S32 imagingSetAeGainMin(GK_S32 gainMin)
{
	GK_S32 retVal;

	ONVIF_Image_Cmd stCmd;
	memset(&stCmd, 0, sizeof(stCmd));
	stCmd.aeGainMin = gainMin;

	retVal = gk7101_Comm_CmdSend(IMAGE_SET_AEGAIN_MIN, sizeof(ONVIF_Image_Cmd), (GK_U8 *)&stCmd, 0, NULL);

	return retVal;
}



/*Auto exposure sensor gain max. */
/*new AE sensor gain(db): range 0 ~ 60. */
static GK_S32 imagingSetAeGainMax(GK_S32 gainMax)
{
	GK_S32 retVal;

	ONVIF_Image_Cmd stCmd;
	memset(&stCmd, 0, sizeof(stCmd));
	stCmd.aeGainMax = gainMax;

	retVal = gk7101_Comm_CmdSend(IMAGE_SET_AEGAIN_MAX, sizeof(ONVIF_Image_Cmd), (GK_U8 *)&stCmd, 0, NULL);

	return retVal;
}


/*Manual exposure exposuretime. */
/*new shutter time in 1/n sec format: range 1 ~ 8000. */
static GK_S32 imagingSetMeShutterTime(GK_S32 shutter)
{
	GK_S32 retVal;

	ONVIF_Image_Cmd stCmd;
	memset(&stCmd, 0, sizeof(stCmd));
	stCmd.meShutterTime = shutter;

	retVal = gk7101_Comm_CmdSend(IMAGE_SET_MESHUTTERTIME, sizeof(ONVIF_Image_Cmd), (GK_U8 *)&stCmd, 0, NULL);

	return retVal;
}

/*Manual exposure gain(ISO). */
static GK_S32 imagingSetMeGain(GK_S32 gain)
{
	GK_S32 retVal;

	ONVIF_Image_Cmd stCmd;
	memset(&stCmd, 0, sizeof(stCmd));
	stCmd.meGain = gain;

	retVal = gk7101_Comm_CmdSend(IMAGE_SET_MEGAIN, sizeof(ONVIF_Image_Cmd), (GK_U8 *)&stCmd, 0, NULL);

	return retVal;
}

/* Focus mode: 0 auto,1 manual. */
static GK_S32 imagingSetFocusMode(GK_S32 mode)
{
	GK_S32 retVal;

	ONVIF_Image_Cmd stCmd;
	memset(&stCmd, 0, sizeof(stCmd));
	stCmd.focusMode = mode;

	retVal = gk7101_Comm_CmdSend(IMAGE_SET_FOCUSMODE, sizeof(ONVIF_Image_Cmd), (GK_U8 *)&stCmd, 0, NULL);

	return retVal;

}

/* auto focus: NearLimit setting. */
static GK_S32 imagingSetAfNearLimit(GK_S32 nearLimit)
{
	GK_S32 retVal;

	ONVIF_Image_Cmd stCmd;
	memset(&stCmd, 0, sizeof(stCmd));
	stCmd.afNearLimit = nearLimit;

	retVal = gk7101_Comm_CmdSend(IMAGE_SET_AFNEARLIMIT, sizeof(ONVIF_Image_Cmd), (GK_U8 *)&stCmd, 0, NULL);

	return retVal;
}

/* auto focus: FarLimit setting. */
static GK_S32 imagingSetAfFarLimit(GK_S32 farLimit)
{
	GK_S32 retVal;

	ONVIF_Image_Cmd stCmd;
	memset(&stCmd, 0, sizeof(stCmd));
	stCmd.afFarLimit = farLimit;

	retVal = gk7101_Comm_CmdSend(IMAGE_SET_AFFARLIMIT, sizeof(ONVIF_Image_Cmd), (GK_U8 *)&stCmd, 0, NULL);

	return retVal;
}

/* manual focus: DefaultSpeed setting. */
static GK_S32 imagingSetMfDefaultSpeed(GK_S32 defaultSpeed)
{
	GK_S32 retVal;

	ONVIF_Image_Cmd stCmd;
	memset(&stCmd, 0, sizeof(stCmd));
	stCmd.mfDefaultSpeed = defaultSpeed;

	retVal = gk7101_Comm_CmdSend(IMAGE_SET_MFDEFAULTSPEED, sizeof(ONVIF_Image_Cmd), (GK_U8 *)&stCmd, 0, NULL);

	return retVal;
}

/* Infrared cutoff filter setting: 0 on, 1 off, 2 auto */
static GK_S32 imagingSetIrCutFilter(GK_S32 mode)
{
	GK_S32 retVal;

	ONVIF_Image_Cmd stCmd;
	memset(&stCmd, 0, sizeof(stCmd));
	stCmd.irCutFilter = mode;

	retVal = gk7101_Comm_CmdSend(IMAGE_SET_IRCUTFILTER, sizeof(ONVIF_Image_Cmd), (GK_U8 *)&stCmd, 0, NULL);

	return retVal;
}

/*sharpness: unit in 128, 256 is 2X; valid range is 0 ~ +255*/
static GK_S32 imagingSetSharpness(GK_S32 sharpness)
{
	GK_S32 retVal;

	ONVIF_Image_Cmd stCmd;
	memset(&stCmd, 0, sizeof(stCmd));
	stCmd.sharpness = sharpness;

	retVal = gk7101_Comm_CmdSend(IMAGE_SET_SHARPNESS, sizeof(ONVIF_Image_Cmd), (GK_U8 *)&stCmd, 0, NULL);

	return retVal;
}

/* wide dynamic range mode setting : 0 off, 1 on*/
static GK_S32 imagingSetWdrMode(GK_S32 mode)
{
	GK_S32 retVal;

	ONVIF_Image_Cmd stCmd;
	memset(&stCmd, 0, sizeof(stCmd));
	stCmd.wdrMode = mode;

	retVal = gk7101_Comm_CmdSend(IMAGE_SET_WDRMODE, sizeof(ONVIF_Image_Cmd), (GK_U8 *)&stCmd, 0, NULL);

	return retVal;

}

/* wide dynamic range level setting: 0 -100*/
static GK_S32 imagingSetWdrLevel(GK_S32 level)
{
	GK_S32 retVal;

	ONVIF_Image_Cmd stCmd;
	memset(&stCmd, 0, sizeof(stCmd));
	stCmd.wdrLevel = level;

	retVal = gk7101_Comm_CmdSend(IMAGE_SET_WDRLEVEL, sizeof(ONVIF_Image_Cmd), (GK_U8 *)&stCmd, 0, NULL);

	return retVal;
}



/*WhiteBalance. */
/*Mode:0-disable, 1-enable. */
static GK_S32 imagingSetWbMode(GK_S32 mode)
{
	GK_S32 retVal;

	ONVIF_Image_Cmd stCmd;
	memset(&stCmd, 0, sizeof(stCmd));
	stCmd.wbMode = mode;

	retVal = gk7101_Comm_CmdSend(IMAGE_SET_WBMODE, sizeof(ONVIF_Image_Cmd), (GK_U8 *)&stCmd, 0, NULL);

	return retVal;
}

/* white balance gain setting : R gain. */
static GK_S32 imagingSetMwbRGain(GK_S32 rGain)
{
	GK_S32 retVal;

	ONVIF_Image_Cmd stCmd;
	memset(&stCmd, 0, sizeof(stCmd));
	stCmd.mwbRGain = rGain;
	retVal = gk7101_Comm_CmdSend(IMAGE_SET_WBRGAIN, sizeof(ONVIF_Image_Cmd), (GK_U8 *)&stCmd, 0, NULL);

	return retVal;
}

/* white balance gain setting : B gain. */
static GK_S32 imagingSetMwbBGain(GK_S32 bGain)
{
	GK_S32 retVal;

	ONVIF_Image_Cmd stCmd;
	memset(&stCmd, 0, sizeof(stCmd));
	stCmd.mwbBGain = bGain;
	retVal = gk7101_Comm_CmdSend(IMAGE_SET_WBBGAIN, sizeof(ONVIF_Image_Cmd), (GK_U8 *)&stCmd, 0, NULL);

	return retVal;
}

