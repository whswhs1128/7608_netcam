/*!
*****************************************************************************
** \file        adi/include/adi_vout.h
**
** \brief       ADI video out module header file
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2015-2019 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _ADI_VOUT_H_
#define _ADI_VOUT_H_

#include "stdio.h"
#include "adi_types.h"


//*****************************************************************************
//*****************************************************************************
//** Defines and Macros
//*****************************************************************************
//*****************************************************************************
#define I80COMMAND          0
#define I80PARA             1
#define LCD_WRITE_CMD(x)    ((I80COMMAND<<17)   |   (1<<16)  |   (x))
#define LCD_WRITE_PARA(x)   ((I80PARA<<17)      |   (1<<16)  |   (x))
#define LCD_READ_CMD(x)     ((I80COMMAND<<17)   |   (1<<16)  |   (x))
#define LCD_READ_PARA       ((I80PARA<<17)      |   (0<<16)  |   (0))
#define LCD_CMD_END_FLAG    0xA5A5A5A5
/*
*******************************************************************************
** Defines for general error codes of the module.
*******************************************************************************
*/
/*! Bad parameter passed. */
#define GADI_VO_ERR_BAD_PARAMETER                                          \
                              (GADI_VO_MODULE_BASE + GADI_ERR_BAD_PARAMETER)
/*! Memory allocation failed. */
#define GADI_VO_ERR_OUT_OF_MEMORY                                          \
                              (GADI_VO_MODULE_BASE + GADI_ERR_OUT_OF_MEMORY)
/*! Device already initialised. */
#define GADI_VO_ERR_ALREADY_INITIALIZED                                    \
                              (GADI_VO_MODULE_BASE + GADI_ERR_ALREADY_INITIALIZED)
/*! Device not initialised. */
#define GADI_VO_ERR_NOT_INITIALIZED                                        \
                              (GADI_VO_MODULE_BASE + GADI_ERR_NOT_INITIALIZED)
/*! Feature or function is not available. */
#define GADI_VO_ERR_FEATURE_NOT_SUPPORTED                                  \
                              (GADI_VO_MODULE_BASE + GADI_ERR_FEATURE_NOT_SUPPORTED)
/*! Timeout occured. */
#define GADI_VO_ERR_TIMEOUT                                                \
                              (GADI_VO_MODULE_BASE + GADI_ERR_TIMEOUT)
/*! The device is busy, try again later. */
#define GADI_VO_ERR_DEVICE_BUSY                                            \
                              (GADI_VO_MODULE_BASE + GADI_ERR_DEVICE_BUSY)
/*! Invalid handle was passed. */
#define GADI_VO_ERR_INVALID_HANDLE                                         \
                              (GADI_VO_MODULE_BASE + GADI_ERR_INVALID_HANDLE)
/*! Semaphore could not be created. */
#define GADI_VO_ERR_SEMAPHORE_CREATE                                       \
                               (GADI_VO_MODULE_BASE + GADI_ERR_SEMAPHORE_CREATE)
/*! The driver's used version is not supported. */
#define GADI_VO_ERR_UNSUPPORTED_VERSION                                    \
                               (GADI_VO_MODULE_BASE + GADI_ERR_UNSUPPORTED_VERSION)
/*! The driver's used version is not supported. */
#define GADI_VO_ERR_FROM_DRIVER                                            \
                               (GADI_VO_MODULE_BASE + GADI_ERR_FROM_DRIVER)
/*! The device/handle is not open.. */
#define GADI_VO_ERR_NOT_OPEN                                               \
                               (GADI_VO_MODULE_BASE + GADI_ERR_NOT_OPEN)


//*****************************************************************************
//*****************************************************************************
//** Enumerated types
//*****************************************************************************
//*****************************************************************************
/*!
*******************************************************************************
** \brief video output support resolution enum.
*******************************************************************************
*/
typedef enum {
    /*video output resolution: 480I.*/
    GADI_VOUT_RESOLUTION_480I = 0,
    /*video output resolution: 576I.*/
    GADI_VOUT_RESOLUTION_576I     = 1,
    /*video output resolution: 720p50.*/
    GADI_VOUT_RESOLUTION_720P_PAL = 2,
    /*video output resolution: 720p30.*/
    GADI_VOUT_RESOLUTION_720P30   = 3,
    /*video output resolution: 800x480.*/
    GADI_VOUT_RESOLUTION_WVGA     = 4,
    /*video output resolution: 480p60.*/
    GADI_VOUT_RESOLUTION_D1_NTSC  = 5,
    /*video output resolution: 576p50.*/
    GADI_VOUT_RESOLUTION_D1_PAL   = 6,
    /*video output resolution: 720p60.*/
    GADI_VOUT_RESOLUTION_720P60   = 7,
    /*video output resolution: 1080i50.*/
    GADI_VOUT_RESOLUTION_1080I50  = 8,
    /*video output resolution: 1080i60.*/
    GADI_VOUT_RESOLUTION_1080I60  = 9,
    /*video output resolution: 1080p25.*/
    GADI_VOUT_RESOLUTION_HDTV     = 10,
    /*video output resolution: 1080p30.*/
    GADI_VOUT_RESOLUTION_1080P30  = 11,
    /*video output resolution: 720p25.*/
    GADI_VOUT_RESOLUTION_720P25   = 12,
    /*video output resolution: max number*/
    GADI_VOUT_RESOLUTION_NUMBER,
} GADI_VOUT_ResolutionModeEnumT;

/*!
*******************************************************************************
** \brief video output support output device enum.
*******************************************************************************
*/
typedef enum {
    /*video output via auto select.(currently not support).*/
    GADI_VOUT_DEVICE_AUTO = 0,
    /*video output disable, enable overlay.*/
    GADI_VOUT_DEVICE_DISABLE = 1,
    /*video output via BT1120 disable overlay.*/
    GADI_VOUT_DEVICE_BT1120 = 2,
    /*video output via LCD RGB disable overlay.*/
    GADI_VOUT_DEVICE_RGB    = 3,
    /*video output via LCD I80 disable overlay.*/
    GADI_VOUT_DEVICE_I80    = 4,
    /*video output max device number.*/
    GADI_VOUT_DEVICE_NUMBER,
} GADI_VOUT_DeviceEnumT;

/*!
*******************************************************************************
** \brief video&OSD flip enum.
*******************************************************************************
*/
typedef enum
{
    /*no flip.*/
    GADI_VOUT_FLIP_TYPE_NORMAL = 0,
    /*flip video/OSD horizontal and vertical. Rotate 180 degrees*/
    GADI_VOUT_FLIP_TYPE_HV,
    /*flip video/OSD horizontal.*/
    GADI_VOUT_FLIP_TYPE_HORIZONTAL,
    /*flip video/OSD vertical.*/
    GADI_VOUT_FLIP_TYPE_VERTICAL,
}GADI_VOUT_FlipTypeEnumT;

/*!
*******************************************************************************
** \brief video output framebuffer id.
*******************************************************************************
*/
typedef enum {
    /*video output framebuffer 0.*/
    GADI_VOUT_FB_0 = 0,
    /*video output framebuffer 1.*/
    GADI_VOUT_FB_1 = 1,
    /*video output support max framebuffer number.*/
    GADI_VOUT_FB_NUMBER,
} GADI_VOUT_FbEnumT;
/*!
*******************************************************************************
** \brief video output channel enum.
*******************************************************************************
*/

typedef enum {
    /*video output channel A.*/
	GADI_VOUT_A = 0,
	/*video output channel number.*/
	GADI_VOUT_NUMBER,
} GADI_VOUT_ChannelEnumT;
/*!
*******************************************************************************
** \Video output I80 command.
*******************************************************************************
*/
typedef enum  {
    /*I80 lcd sleep.*/
    GADI_VOUT_I80_Cmd_EnterSleep    = 0, //0x10,
    /*I80 lcd wakeup.*/
    GADI_VOUT_I80_Cmd_SleepOut        , //0x11,
    /*I80 lcd display close.*/
    GADI_VOUT_I80_Cmd_DisplayOff      , //0x28, 
    /*I80 lcd display open.*/
    GADI_VOUT_I80_Cmd_DisplayOn       , //0x29,
    /*I80 lcd show width.*/
    GADI_VOUT_I80_Cmd_DisplayWinWidth , //0x2A,
    /*I80 lcd show height.*/
    GADI_VOUT_I80_Cmd_DisplayWinHeight, //0x2B,
    /*I80 lcd write mem.*/
    GADI_VOUT_I80_Cmd_MemWrite        , //0x2C,
    /*I80 lcd write read mem control.*/
    GADI_VOUT_I80_Cmd_MemAccessCtrl   , //0x36,    
    /*I80 lcd frames rate control.*/
    GADI_VOUT_I80_Cmd_FrameRateCtrl   , //0xB1,
    /*I80 lcd proch control.*/
    GADI_VOUT_I80_Cmd_ProchCtrl       , //0xB5, 
    /*I80 lcd cmd number.*/
    GADI_VOUT_I80_Cmd_NUMBER,
}GADI_VOUT_I80_CmdT;


/*!
*******************************************************************************
** \brief video output source type.
*******************************************************************************
*/
typedef enum {
    /*video output source from input of encoder.*/
    GADI_VOUT_SOURCE_TYPE_ENCODE          = 1,
    /*video output source from background color.*/
    GADI_VOUT_SOURCE_TYPE_BACKGROUND_COLOR= 2,
    /*video output source from boot logo.*/
    GADI_VOUT_SOURCE_TYPE_BOOT_LOGO       = 3,
    /*video output source number.*/
    GADI_VOUT_SOURCE_TYPE_NUM,
} GADI_VOUT_SrcTypeEnumT;
//*****************************************************************************
//*****************************************************************************
//** Data Structures
//*****************************************************************************
//*****************************************************************************

/*!
*******************************************************************************
** \brief video output setting parameters struct.
*******************************************************************************
*/
typedef struct
{
    /*specify vout channel.*/
    GADI_VOUT_ChannelEnumT          voutChannel;
    /*video output resolution mode.*/
    GADI_VOUT_ResolutionModeEnumT   resoluMode;
    /*video output device type.*/
    GADI_VOUT_DeviceEnumT           deviceType;
    /*enable or disable rotate 90*/
    GADI_BOOL                       rotateEn;
    /*video filp type*/
    GADI_VOUT_FlipTypeEnumT         videoFlip;
    /*osd filp type*/
    GADI_VOUT_FlipTypeEnumT         osdFlip;
	/*video output source type when booting dsp.*/
	GADI_VOUT_SrcTypeEnumT          srcType;
     /*enable or disable OSD rotate 90*/
    GADI_BOOL                       osdrotateEn;
} GADI_VOUT_SettingParamsT;

/*!
*******************************************************************************
** \brief video output setting parameters struct.
*******************************************************************************
*/
typedef struct
{
    /*specify vout channel.*/
    GADI_VOUT_ChannelEnumT          voutChannel;
    /*specify framebuffer channel.*/
    GADI_VOUT_FbEnumT               fbChannel;
} GADI_VOUT_SelectFbParamsT;

/*!
*******************************************************************************
** \brief video output(I80) setting parameters struct.
*******************************************************************************
*/

typedef struct {
    /*vout channel id*/
    GADI_U32         voId;  
    /*i80 cmd total numbers*/
    GADI_U32         cmdParaNum;
    /*i80 read cmd total numbers*/
    GADI_U32         rdParaNum;
    /*i80 cmd array*/
    GADI_U32         cmdPara[128];
} GADI_VOUT_I80_CmdParaT;


/*
*******************************************************************************
** \brief output sink device parameter.
*******************************************************************************
*/

typedef struct
{
    GADI_VOID* lcdHwParamsPtr;
}GADI_VO_OpenParamsT;


typedef struct
{
    /*magic number,show file type*/
    GADI_U32 magic_start;
    /*version id*/
    GADI_U32 version;
    /*lcd configuration file length*/
    GADI_U32 length; 
    /*lcd type*/
    GADI_U32 type;//4--RGB 5--I80 6--RGB with SPI config    others--Reserved
    GADI_U32 reserved;
}GADI_VO_LcdHwInfoTagT;


/*!
*******************************************************************************
** \brief video rotate enable parameters struct.
*******************************************************************************
*/
typedef struct
{
    /*specify vout channel.*/
    GADI_VOUT_ChannelEnumT  voutChannel;
    /*enable or disable rotate 90*/
    GADI_BOOL               rotateEn;
}GADI_VO_RotateEnT;
/*!
*******************************************************************************
** \brief video output source struct.
*******************************************************************************
*/
typedef struct
{
    /*specify vout channel.*/
    GADI_VOUT_ChannelEnumT          voutChannel;
    /*specify vout source type.*/
    GADI_VOUT_SrcTypeEnumT          srcType;
} GADI_VOUT_SelectSrcTypeT;

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
** \brief Initialize the ADI video out module.
**
** \return
** - #GADI_OK
** - #GADI_VOUT_ERR_ALREADY_INITIALIZED
** - #GADI_VOUT_ERR_OUT_OF_MEMORY
**
** \sa gadi_vout_exit
**
*******************************************************************************
*/
GADI_ERR gadi_vout_init(void);

/*!
*******************************************************************************
** \brief Shutdown the ADI video output module.
**
** \return
** - #GADI_OK
** - #GADI_VOUT_ERR_NOT_INITIALIZED
**
** \sa gadi_vout_init
**
*******************************************************************************
*/
GADI_ERR gadi_vout_exit(void);

/*!
*******************************************************************************
** \brief Open one ADI video out module instance.
**
** \param[in] openParams a struct pointer of open prameters.
** \param[in] errorCodePtr pointer to return the error code.
**
** \return Return an valid handle of ADI vout module instance.
**
** \sa gadi_vout_close
**
*******************************************************************************
*/
GADI_SYS_HandleT gadi_vout_open(GADI_ERR* errorCodePtr);

/*!
*******************************************************************************
** \brief close one ADI video output module instance.
**
** \param[in]  handle     Valid ADI vout instance handle previously opened by
**                        #gadi_vout_open.
**
** \return
** - #GADI_OK
** - #GADI_VOUT_ERR_BAD_PARAMETER
**
** \sa gadi_vout_open
**
*******************************************************************************
*/
GADI_ERR gadi_vout_close(GADI_SYS_HandleT handle);

/*!
*******************************************************************************
** \brief get video output parameters.
**
** \param[in]  handle     Valid dsp ADI instance handle previously opened by
**                        #gadi_vout_open.
** \param[in]  voutParams  pointer to return the video output parameters struct.
**
** \return
** - #GADI_OK
** - #GADI_VO_ERR_BAD_PARAMETER
**
** \sa gadi_vout_set_params
**
*******************************************************************************
*/
GADI_ERR gadi_vout_get_params(GADI_SYS_HandleT handle, GADI_VOUT_SettingParamsT* voutParams);

/*!
*******************************************************************************
** \brief set video output parameters.
**
** \param[in]  handle     Valid dsp ADI instance handle previously opened by
**                        #gadi_vout_open.
** \param[in]  voutParams input pointer of video output parameters struct.
**
** \return
** - #GADI_OK
** - #GADI_VO_ERR_BAD_PARAMETER
**
** \sa gadi_vout_get_params
**
*******************************************************************************
*/

GADI_ERR gadi_vout_set_params(GADI_SYS_HandleT handle, GADI_VOUT_SettingParamsT* voutParams);

/*!
*******************************************************************************
** \brief select vout framebuffer.
**
** \param[in]  handle     Valid dsp ADI instance handle previously opened by
**                        #gadi_out_open.
** \param[in]  fbPar      pointer of framebuffer select struct.
**
** \return
** - #GADI_OK
** - #GADI_VO_ERR_BAD_PARAMETER
** - #GADI_VO_ERR_FROM_DRIVER
**
** \sa gadi_vout_open
**
*******************************************************************************
*/

GADI_ERR gadi_vout_select_fb(GADI_SYS_HandleT handle, GADI_VOUT_SelectFbParamsT *fbPar);

/*!
*******************************************************************************
** \brief set i80 output parameters.
**
** \param[in]  handle     Valid dsp ADI instance handle previously opened by
**                        #gadi_vout_open.
** \param[in]  i80Cmd     command to driver ic of i80.
**
** \return
** - #GADI_OK
** - #GADI_VO_ERR_BAD_PARAMETER
**
** \sa gadi_vout_set_i80_params
**
*******************************************************************************
*/

GADI_ERR gadi_vout_set_i80_params(GADI_SYS_HandleT handle, GADI_VOUT_I80_CmdParaT * pi80Cmd);

/*!
*******************************************************************************
** \brief set pwm parameters for backlight.
**
** \param[in]  handle     Valid dsp ADI instance handle previously opened by
**                        #gadi_vout_open.
** \param[in]  duty       pointer of pwm for backlight.
**
** \return
** - #GADI_OK
** - #GADI_VO_ERR_BAD_PARAMETER
**
** \sa gadi_vout_set_backlight
**
*******************************************************************************
*/

GADI_ERR gadi_vout_set_backlight(GADI_SYS_HandleT handle, GADI_U16 duty);

/*!
*******************************************************************************
** \brief print vout params.
**
** \param[in]   handle     Valid ADI vout instance handle previously opened by
**                        #gadi_vout_open.
** \return
** - #GADI_OK
** - #GADI_VO_ERR_NOT_INITIALIZED
**
** \sa gadi_vout_set_params
**
*******************************************************************************
*/
GADI_ERR gadi_vout_print_params(GADI_SYS_HandleT handle);

/*!
*******************************************************************************
** \brief select vout source type.
**
** \param[in]   handle     Valid ADI vout instance handle previously opened by
**                        #gadi_vout_open.
** \param[in]  srcType       source type struct of this channel.
** \return
** - #GADI_OK
** - #GADI_VO_ERR_BAD_PARAMETER
** - #GADI_VO_ERR_FROM_DRIVER
**
** \sa gadi_vout_init
**
*******************************************************************************
*/

GADI_ERR gadi_vout_select_source_type(GADI_SYS_HandleT handle, GADI_VOUT_SelectSrcTypeT *srcType);

/*!
*******************************************************************************
** \brief set vout background type.
**
** \param[in]   handle     Valid ADI vout instance handle previously opened by
**                        #gadi_vout_open.
** \param[in]  voutBackGround       background struct of this channel.
** \return
** - #GADI_OK
** - #GADI_VO_ERR_BAD_PARAMETER
**
** \sa gadi_vout_init
**
*******************************************************************************
*/

GADI_ERR gadi_vout_init_video_src_type(GADI_SYS_HandleT handle, GADI_VOUT_SelectSrcTypeT *voutBackGround);


#ifdef __cplusplus
    }
#endif


#endif /* _ADI_VOUT_H_ */
