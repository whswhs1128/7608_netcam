/*!
*****************************************************************************
** \file        packages/onvif/src/gk7101_imaging.h
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

#ifndef _GK7101_IMAGING_H_
#define _GK7101_IMAGING_H_

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



//*****************************************************************************
//*****************************************************************************
//** Data Structures
//*****************************************************************************
//*****************************************************************************
typedef struct ONVIF_Image{
	GK_U32 blcMode;          /*Backlight compensation Mode    1 - on, 0 - off   */
	GK_S32 blcLevel;         /*Backlight compensation level     0 - 20             */

	GK_S32 brightness;       /*Brightness                               0 - 100           */
	GK_S32 colorSaturation;  /*Color saturation                        0 - 100          */
	GK_S32 contrast;         /*Contrast                                  0 - 100           */

	GK_ULONG exposureMode;     /*Exposure mode                  0 - auto 1 - manual*/
	GK_ULONG aeShutterTimeMin; /*Auto exposure: Min time of shutter 120 - 8000  */
	GK_ULONG aeShutterTimeMax; /*Auto exposure: max time of shutter 0 - 60        */
	GK_ULONG aeGainMin;        /*Auto exposure: Min gain                 0               */
	GK_ULONG aeGainMax;        /*Auto exposure: max gain                 60            */
	GK_ULONG meShutterTime;    /*Manual exposure shutter time      1-8000         */
	GK_ULONG meGain;           /*Manual exporsure gain                 0-60            */

    GK_ULONG focusMode;        /*Focus Mode                   0 - auto, 1 - manual   */
    GK_ULONG afNearLimit;      /*Auto: limit of closest distance 0 - 100               */
    GK_ULONG afFarLimit;       /*Auto: limit of fatest longest distance 0 - 100     */
    GK_ULONG mfDefaultSpeed;   /*Manual default speed          0 - 100                 */

	GK_ULONG irCutFilter;      /*Infrared cutoff filter setting 0 - on 1 - off 2 - auto*/
    GK_S32 sharpness;        /*Sharpness                                0 - 100          */

	GK_ULONG wdrMode;          /*Wide dynamic range mode   1 - on 0 - off         */
	GK_ULONG wdrLevel;         /*Wide dynamic range level    0 - 100                 */

    GK_ULONG wbMode;           /*White balance mode           1 - auto 0 - manual*/
	GK_ULONG mwbRGain;          /*White balance: gain of r      1 - 16384              */
	GK_ULONG mwbBGain;          /*White balance: gain of b      1 - 16384             */
	GK_S32 settingsSave;
}ONVIF_Image_Cmd;

typedef struct ImageAdapterConfiguration
{
    GK_S32 (*imagingGetInfo)(GK_U8 dataLen, GK_U8 *dataLuf);
	/* BLC Mode: 0 off, 1 on. */
    GK_S32 (*imagingSetBlcMode)(GK_U32 mode);
	/*BLC Level*/
    GK_S32 (*imagingSetBlcLevel)(GK_S32 level);
	/*Brightness*/
    GK_S32 (*imagingSetBrightness)(GK_S32 brightness);
	/*Color Saturation*/
    GK_S32 (*imagingSetColorSaturation)(GK_S32 saturation);
	/*Contrast*/
    GK_S32 (*imagingSetContrast)(GK_S32 contrast);
	/*Auto exposure mode: 0 disable,1 enable*/
    GK_S32 (*imagingSetExposureMode)(GK_S32 mode);
	/*Auto exposure shutter time Min*/
    GK_S32 (*imagingSetAeShutterTimeMin)(GK_S32 shutterMin);
	/*Auto exposure sensor gain max*/
    GK_S32 (*imagingSetAeShutterTimeMax)(GK_S32 shutterMax);
    /*Auto exposure sensor gain Min*/
    GK_S32 (*imagingSetAeGainMin)(GK_S32 gainMin);
	/*Auto exposure sensor gain max*/
    GK_S32 (*imagingSetAeGainMax)(GK_S32 gainMax);
	/*Manual exposure exposuretime*/
    GK_S32 (*imagingSetMeShutterTime)(GK_S32 shutter);
	/*Manual exposure gain(ISO)*/
    GK_S32 (*imagingSetMeGain)(GK_S32 gain);
    /*Auto focus mode: enable/disable*/
    GK_S32 (*imagingSetFocusMode)(GK_S32 mode);
    /*Auto fucus: nearLimit */
    GK_S32 (*imagingSetAfNearLimit)(GK_S32 nearLimit);
    /*Auto focus: farLimit*/
    GK_S32 (*imagingSetAfFarLimit)(GK_S32 farLimit);
    /*Manual focus: defaultSpeed*/
    GK_S32 (*imagingSetMfDefaultSpeed)(GK_S32 defaultSpeed);
    /*Infrared cutoff filter*/
    GK_S32 (*imagingSetIrCutFilter)(GK_S32 mode);
	/*Sharpness*/
    GK_S32 (*imagingSetSharpness)(GK_S32 sharpness);
    /*Wide dynamic range mode*/
    GK_S32 (*imagingSetWdrMode)(GK_S32 mode);
    /*Wide dynamic range level*/
    GK_S32 (*imagingSetWdrLevel)(GK_S32 level);
	/*WhiteBalance mode*/
    GK_S32 (*imagingSetWbMode)(GK_S32 mode);
    /*WhiteBalance rgain*/
	GK_S32 (*imagingSetMwbRGain)(GK_S32 rGain);
    /*WhiteBalance bgain*/
	GK_S32 (*imagingSetMwbBGain)(GK_S32 bGain);
    /*ForcePersistenc*/
    GK_S32 (*imagingSetForcePersistence)(GK_S32 ForcePersistence);

}ImageAdapterConfiguration_t;


//*****************************************************************************
//*****************************************************************************
//** Global Data
//*****************************************************************************
//*****************************************************************************

extern ImageAdapterConfiguration_t gImageAdapterConfiguration;

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

