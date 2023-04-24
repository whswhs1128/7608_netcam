/*!
*****************************************************************************
** \file        packages/onvif/src/gk7101/gk7101_ptz.h
**
**
** \brief       Onvif PTZ Control Interface
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2013 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _GK7101_PTZ_H_
#define _GK7101_PTZ_H_


#include "onvif_std.h"

//*****************************************************************************
//*****************************************************************************
//** Defines and Macros
//*****************************************************************************
//*****************************************************************************

#define MAX_PTZ_PRESET_NUM 32

//*****************************************************************************
//*****************************************************************************
//** Enumerated types
//*****************************************************************************
//*****************************************************************************

#define PTZPRESETINFORMATION "/home/ycc.cfg"

//*****************************************************************************
//*****************************************************************************
//** Data Structures
//*****************************************************************************
//*****************************************************************************
typedef struct {
    GK_CHAR devName[32];
    GK_S32 presetsCapacity;
}ONVIF_PTZ_DevParamsT;

typedef struct
{
	GK_BOOL  zoom;
	GK_FLOAT zoomSpeed;
	GK_BOOL  panTilt;
	GK_FLOAT panSpeed;
	GK_FLOAT tiltSpeed;
} ONVIF_PTZ_ContinousMoveInfoT;

typedef struct
{
	GK_BOOL  zoom;
	GK_FLOAT zoomPosition;
	GK_FLOAT zoomSpeed;
	GK_BOOL  panTilt;
	GK_FLOAT panPosition;
	GK_FLOAT tiltPosition;
	GK_FLOAT panSpeed;
	GK_FLOAT tiltSpeed;
} ONVIF_PTZ_AbsoluteMoveInfoT;

typedef struct {
	GK_BOOL  zoom;
	GK_FLOAT zoomTranslation;
	GK_FLOAT zoomSpeed;
	
	GK_BOOL  panTilt;
	GK_FLOAT panTranslation;
	GK_FLOAT tiltTranslation;
	GK_FLOAT panSpeed;
	GK_FLOAT tiltSpeed;
} ONVIF_PTZ_RelativeMoveInfoT;

typedef struct {
    GK_BOOL stopAll;
	GK_BOOL stopPt;
	GK_BOOL stopZoom;
} Onvif_PTZ_StopInfoT;

typedef struct {
	GK_U8 presetNum;
	GK_S8 presetName[MAX_NAME_LENGTH];
	GK_S8 presetToken[MAX_TOKEN_LENGTH];
} ONVIF_PTZ_PresetInfoT;

typedef struct {
	GK_U32 presetSize;
	ONVIF_PTZ_PresetInfoT presets[MAX_PTZ_PRESET_NUM];
} ONVIF_PTZ_AllPresetsInfoT;

typedef struct  {
    GK_U8 panSpeed;
    GK_U8 tiltSpeed;
    GK_U8 zoomSpeed;
    GK_U8 presetNum;
} ONVIF_PTZ_CmdT;

typedef struct PtzAdapterConfiguration
{
    GK_S32  (*ptzContinousMove)(ONVIF_PTZ_ContinousMoveInfoT * info);
    GK_S32  (*ptzAbsoluteMove)(ONVIF_PTZ_AbsoluteMoveInfoT * info);
    GK_S32  (*ptzRelativeMove)(ONVIF_PTZ_RelativeMoveInfoT * info);
    GK_S32  (*ptzStop)(Onvif_PTZ_StopInfoT* info);
    GK_S32  (*ptzSetPreset)(ONVIF_PTZ_PresetInfoT* info);
    GK_S32  (*ptzGotoPreset)(ONVIF_PTZ_PresetInfoT* info);
    GK_S32  (*ptzGetAllPresets)(ONVIF_PTZ_AllPresetsInfoT* info);
    GK_S32  (*ptzRemovePreset)(ONVIF_PTZ_PresetInfoT* info);
    GK_BOOL (*ptzCheckPresetName)(ONVIF_PTZ_PresetInfoT *info);
    GK_U32  (*ptzAssignPresetNum)(void);
    void    (*ptzPresetListInit)();
    GK_S32  (*ptzGetDeviceInfo)(ONVIF_PTZ_DevParamsT *pstPtzDevInfo);
}PtzAdapterConfiguration_t;


//*****************************************************************************
//*****************************************************************************
//** Global Data
//*****************************************************************************
//*****************************************************************************

extern PtzAdapterConfiguration_t gPtzAdapterConfiguration;

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


#endif /* _GK7101_PTZ_H_*/

