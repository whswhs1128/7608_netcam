/*!
*****************************************************************************
** \file        nvision/software/packages/onvif/src/gk7101/gk7101_cmd.h
**
**
** \brief       Cmd
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2013 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#ifndef _GK7101_CMD_H_
#define _GK7101_CMD_H_

#include "onvif_std.h"


//*****************************************************************************
//*****************************************************************************
//** Defines and Macros
//*****************************************************************************
//*****************************************************************************



//*****************************************************************************
//*****************************************************************************
//** Enumerated types
//*****************************************************************************
//*****************************************************************************
typedef enum ONVIF_CMDTYPE{

	MEDIA_SET_RESOLUTION = 1,
	MEDIA_SET_FRAMERATE,
	MEDIA_SET_BITRATE,
	MEDIA_SET_ENCINTERVAL,
	MEDIA_SET_QUALITY,
	MEDIA_SET_GOVLENGTH,
	MEDIA_SET_STILLCAP,
	MEDIA_GET_INFO,
	IMAGE_SET_BLCMODE,
	IMAGE_SET_BLCLEVEL,
	IMAGE_SET_BRIGHTNESS,
	IMAGE_SET_COLOR_SATURATION,
	IMAGE_SET_CONTRAST,
	IMAGE_SET_EXPOSUREMODE,
	IMAGE_SET_AESHUTTERTIME_MIN,
	IMAGE_SET_AESHUTTERTIME_MAX,
	IMAGE_SET_AEGAIN_MIN,
	IMAGE_SET_AEGAIN_MAX,
	IMAGE_SET_MESHUTTERTIME,
	IMAGE_SET_MEGAIN,
    IMAGE_SET_FOCUSMODE,
	IMAGE_SET_AFNEARLIMIT,
	IMAGE_SET_AFFARLIMIT,
	IMAGE_SET_MFDEFAULTSPEED,
	IMAGE_SET_IRCUTFILTER,
	IMAGE_SET_SHARPNESS,
	IMAGE_SET_WDRMODE,
	IMAGE_SET_WDRLEVEL,
	IMAGE_SET_WBMODE,
	IMAGE_SET_WBRGAIN,
	IMAGE_SET_WBBGAIN,
	IMAGE_GET_INFO,
	IMAGE_FORCE_PERSISTENCE,
    DEVICE_SET_FACTORY_DEFAULT = 35,
    PTZ_GET_DEVICE_INFO = 50,
    PTZ_STOP,
    PTZ_PAN_LEFT,
    PTZ_PAN_RIGHT,
    PTZ_TILT_UP,
    PTZ_TILT_DOWN,
    PTZ_PANTILT_RIGHTUP,
    PTZ_PANTILT_RIGHTDOWN,
    PTZ_PANTILT_LEFTUP,
    PTZ_PANTILT_LEFTDOWN,
    PTZ_ZOOM_TELE,
    PTZ_ZOOM_WIDE,
    PTZ_FOCUS_FAR,
    PTZ_FOCUS_NEAR,
    PTZ_IRIS_OPEN,
    PTZ_IRIS_CLOSE,
    PTZ_AUTO_SCAN,
    PTZ_SET_PRESET,
    PTZ_CLEAR_PRESET,
    PTZ_GOTO_PRESET
}IPC_CMDTYPE;


//*****************************************************************************
//*****************************************************************************
//** Data Structures
//*****************************************************************************
//*****************************************************************************



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
/*!
*******************************************************************************
** \brief (brief description)
**
** (addition description is here)
**
** \param[in]  parameter_name   (parameter description)
** \param[out] parameter_name   (parameter description)
**
**
** \return
** - #RETURN_VALUE_A (return value description)
** - #RETURN_VALUE_B (return value description)
**
** \sa
** - see_also_symbol
*******************************************************************************
*/
GK_S32 gk7101_Comm_Init();
/*!
*******************************************************************************
** \brief (brief description)
**
** (addition description is here)
**
** \param[in]  parameter_name   (parameter description)
** \param[out] parameter_name   (parameter description)
**
**
** \return
** - #RETURN_VALUE_A (return value description)
** - #RETURN_VALUE_B (return value description)
**
** \sa
** - see_also_symbol
*******************************************************************************
*/

GK_S32 gk7101_Comm_CmdSend(const GK_S8 cmdtype, const GK_U8 datalen,GK_U8 *pCmdData, GK_U8 revdatalen, GK_U8 *pRevData);


#ifdef __cplusplus
}
#endif








#endif
