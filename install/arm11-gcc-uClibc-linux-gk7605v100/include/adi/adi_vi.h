/*!
*****************************************************************************
** \file        adi/include/adi_vi.h
**
** \brief       ADI video vi module header file
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2015-2019 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _ADI_VI_H_
#define _ADI_VI_H_

#include "stdio.h"
#include "adi_types.h"

//*****************************************************************************
//*****************************************************************************
//** Defines and Macros
//*****************************************************************************
//*****************************************************************************

/*
*******************************************************************************
** Defines for general error codes of the module.
*******************************************************************************
*/
/*! Bad parameter passed. */
#define GADI_VI_ERR_BAD_PARAMETER                                          \
                              (GADI_VI_MODULE_BASE + GADI_ERR_BAD_PARAMETER)
/*! Memory allocation failed. */
#define GADI_VI_ERR_OUT_OF_MEMORY                                          \
                              (GADI_VI_MODULE_BASE + GADI_ERR_OUT_OF_MEMORY)
/*! Device already initialised. */
#define GADI_VI_ERR_ALREADY_INITIALIZED                                    \
                              (GADI_VI_MODULE_BASE + GADI_ERR_ALREADY_INITIALIZED)
/*! Device not initialised. */
#define GADI_VI_ERR_NOT_INITIALIZED                                        \
                              (GADI_VI_MODULE_BASE + GADI_ERR_NOT_INITIALIZED)
/*! Feature or function is not available. */
#define GADI_VI_ERR_FEATURE_NOT_SUPPORTED                                  \
                              (GADI_VI_MODULE_BASE + GADI_ERR_FEATURE_NOT_SUPPORTED)
/*! Timeout occured. */
#define GADI_VI_ERR_TIMEOUT                                                \
                              (GADI_VI_MODULE_BASE + GADI_ERR_TIMEOUT)
/*! The device is busy, try again later. */
#define GADI_VI_ERR_DEVICE_BUSY                                            \
                              (GADI_VI_MODULE_BASE + GADI_ERR_DEVICE_BUSY)
/*! Invalid handle was passed. */
#define GADI_VI_ERR_INVALID_HANDLE                                         \
                              (GADI_VI_MODULE_BASE + GADI_ERR_INVALID_HANDLE)
/*! Semaphore could not be created. */
#define GADI_VI_ERR_SEMAPHORE_CREATE                                       \
                               (GADI_VI_MODULE_BASE + GADI_ERR_SEMAPHORE_CREATE)
/*! The driver's used version is not supported. */
#define GADI_VI_ERR_UNSUPPORTED_VERSION                                    \
                               (GADI_VI_MODULE_BASE + GADI_ERR_UNSUPPORTED_VERSION)
/*! The driver's used version is not supported. */
#define GADI_VI_ERR_FROM_DRIVER                                            \
                               (GADI_VI_MODULE_BASE + GADI_ERR_FROM_DRIVER)
/*! The device/handle is not open.. */
#define GADI_VI_ERR_NOT_OPEN                                               \
                               (GADI_VI_MODULE_BASE + GADI_ERR_NOT_OPEN)
/*! The file is read failed. */
#define GADI_VI_ERR_READ_FAILED                                             \
                                (GADI_VI_MODULE_BASE + GADI_ERR_READ_FAILED)


#define GADI_VI_SENSOR_TABLE_FLAG_END                (0xFFFFFFFF)
#define GADI_VI_SENSOR_TABLE_FLAG_NULL               (0xFFFFFFFF)
#define GADI_VI_SENSOR_TABLE_FLAG_SLEEP              (0xFFFFFFFE)
#define GADI_VI_SENSOR_TABLE_FLAG_MSLEEP             (0xFFFFFFFE)
#define GADI_VI_SENSOR_TABLE_FLAG_MDELAY             (0xFFFFFFFD)
#define GADI_VI_SENSOR_TABLE_FLAG_UDELAY             (0xFFFFFFFC)
//*****************************************************************************
//*****************************************************************************
//** Enumerated types
//*****************************************************************************
//*****************************************************************************
/*!
********************************************************************************
** \brief video input channel enum.
*******************************************************************************
*/
typedef enum {
    /*video input device channel A.*/
	GADI_VI_A = 0,
    /*video input device channel B.*/
	GADI_VI_B = 1,
	/*video input device channel number.*/
	GADI_VI_NUMBER,
} GADI_VI_ChannelEnumT;

/*!
*******************************************************************************
** \brief video input sensor mirror mode enum.
*******************************************************************************
*/
typedef enum {
    /*video horizontal vertical mirror flip*/
    GADI_VI_MIRROR_HORRIZONTALLY_VERTICALLY    = 0,    
    /*video horizontal mirror flip*/
    GADI_VI_MIRROR_HORRIZONTALLY,
    /*video vertical mirror flip*/
    GADI_VI_MIRROR_VERTICALLY,
    /*video not flip*/    
    GADI_VI_MIRROR_NONE,
    /*video auto mirror flip*/  
    GADI_VI_MIRROR_AUTO            = 255,
}GADI_VI_MirrorPatternEnumT;

/*!
*******************************************************************************
** \brief video input bayer data patter enum.
*******************************************************************************
*/
typedef enum {
    /*sensor RG bayer mode.*/
    GADI_VI_BAYER_PATTERN_RG       = 0,
    /*sensor BG bayer mode.*/
    GADI_VI_BAYER_PATTERN_BG,
    /*sensor GR bayer mode.*/
    GADI_VI_BAYER_PATTERN_GR,
    /*sensor GR bayer mode.*/
    GADI_VI_BAYER_PATTERN_GB,
    /*sensor auto bayer mode.*/
    GADI_VI_BAYER_PATTERN_AUTO     = 255,
}GADI_VI_BayerPatternEnumT;

/*!
*******************************************************************************
** \brief video input support frame rate.
*******************************************************************************
*/
typedef enum {
    /*use default sensor frame rate.*/
    GADI_VI_FPS_AUTO   = 0,
    /*fps:1.*/
    GADI_VI_FPS_1      = 1,
    GADI_VI_FPS_2      = 2,
    GADI_VI_FPS_3      = 3,
    GADI_VI_FPS_4      = 4,
    GADI_VI_FPS_5      = 5,
    GADI_VI_FPS_6      = 6,
    GADI_VI_FPS_7      = 7,
    GADI_VI_FPS_8      = 8,
    GADI_VI_FPS_9      = 9,
    GADI_VI_FPS_10     = 10,
    GADI_VI_FPS_11     = 11,
    GADI_VI_FPS_12     = 12,
    GADI_VI_FPS_13     = 13,
    GADI_VI_FPS_14     = 14,
    GADI_VI_FPS_15     = 15,
    GADI_VI_FPS_16     = 16,
    GADI_VI_FPS_17     = 17,
    GADI_VI_FPS_18     = 18,
    GADI_VI_FPS_19     = 19,
    GADI_VI_FPS_20     = 20,
    GADI_VI_FPS_21     = 21,
    GADI_VI_FPS_22     = 22,
    GADI_VI_FPS_23     = 23,
    GADI_VI_FPS_24     = 24,
    GADI_VI_FPS_25     = 25,
    GADI_VI_FPS_26     = 26,
    GADI_VI_FPS_27     = 27,
    GADI_VI_FPS_28     = 28,
    GADI_VI_FPS_29     = 29,
    GADI_VI_FPS_30     = 30,
    GADI_VI_FPS_31     = 31,
    GADI_VI_FPS_32     = 32,
    GADI_VI_FPS_33     = 33,
    GADI_VI_FPS_34     = 34,
    GADI_VI_FPS_35     = 35,
    GADI_VI_FPS_36     = 36,
    GADI_VI_FPS_37     = 37,
    GADI_VI_FPS_38     = 38,
    GADI_VI_FPS_39     = 39,
    GADI_VI_FPS_40     = 40,
    GADI_VI_FPS_41     = 41,
    GADI_VI_FPS_42     = 42,
    GADI_VI_FPS_43     = 43,
    GADI_VI_FPS_44     = 44,
    GADI_VI_FPS_45     = 45,
    GADI_VI_FPS_46     = 46,
    GADI_VI_FPS_47     = 47,
    GADI_VI_FPS_48     = 48,
    GADI_VI_FPS_49     = 49,
    GADI_VI_FPS_50     = 50,
    GADI_VI_FPS_51     = 51,
    GADI_VI_FPS_52     = 52,
    GADI_VI_FPS_53     = 53,
    GADI_VI_FPS_54     = 54,
    GADI_VI_FPS_55     = 55,
    GADI_VI_FPS_56     = 56,
    GADI_VI_FPS_57     = 57,
    GADI_VI_FPS_58     = 58,
    GADI_VI_FPS_59     = 59,
    GADI_VI_FPS_60     = 60,
    GADI_VI_FPS_61     = 61,
    GADI_VI_FPS_62     = 62,
    GADI_VI_FPS_63     = 63,
    GADI_VI_FPS_64     = 64,
    GADI_VI_FPS_65     = 65,
    GADI_VI_FPS_66     = 66,
    GADI_VI_FPS_67     = 67,
    GADI_VI_FPS_68     = 68,
    GADI_VI_FPS_69     = 69,
    GADI_VI_FPS_70     = 70,
    GADI_VI_FPS_71     = 71,
    GADI_VI_FPS_72     = 72,
    GADI_VI_FPS_73     = 73,
    GADI_VI_FPS_74     = 74,
    GADI_VI_FPS_75     = 75,
    GADI_VI_FPS_76     = 76,
    GADI_VI_FPS_77     = 77,
    GADI_VI_FPS_78     = 78,
    GADI_VI_FPS_79     = 79,
    GADI_VI_FPS_80     = 80,
    GADI_VI_FPS_81     = 81,
    GADI_VI_FPS_82     = 82,
    GADI_VI_FPS_83     = 83,
    GADI_VI_FPS_84     = 84,
    GADI_VI_FPS_85     = 85,
    GADI_VI_FPS_86     = 86,
    GADI_VI_FPS_87     = 87,
    GADI_VI_FPS_88     = 88,
    GADI_VI_FPS_89     = 89,
    GADI_VI_FPS_90     = 90,
    GADI_VI_FPS_91     = 91,
    GADI_VI_FPS_92     = 92,
    GADI_VI_FPS_93     = 93,
    GADI_VI_FPS_94     = 94,
    GADI_VI_FPS_95     = 95,
    GADI_VI_FPS_96     = 96,
    GADI_VI_FPS_97     = 97,
    GADI_VI_FPS_98     = 98,
    GADI_VI_FPS_99     = 99,
    GADI_VI_FPS_100    = 100,
    GADI_VI_FPS_101    = 101,
    GADI_VI_FPS_102    = 102,
    GADI_VI_FPS_103    = 103,
    GADI_VI_FPS_104    = 104,
    GADI_VI_FPS_105    = 105,
    GADI_VI_FPS_106    = 106,
    GADI_VI_FPS_107    = 107,
    GADI_VI_FPS_108    = 108,
    GADI_VI_FPS_109    = 109,
    GADI_VI_FPS_110    = 110,
    GADI_VI_FPS_111    = 111,
    GADI_VI_FPS_112    = 112,
    GADI_VI_FPS_113    = 113,
    GADI_VI_FPS_114    = 114,
    GADI_VI_FPS_115    = 115,
    GADI_VI_FPS_116    = 116,
    GADI_VI_FPS_117    = 117,
    GADI_VI_FPS_118    = 118,
    GADI_VI_FPS_119    = 119,
    GADI_VI_FPS_120    = 120,
    /*fps:3.125.*/
    GADI_VI_FPS_3_125  = 3125000,
    /*fps:3.75.*/
    GADI_VI_FPS_3_75   = 37500,
    /*fps:6.25.*/
    GADI_VI_FPS_6_25   = 62500,
    /*fps:7.5.*/
    GADI_VI_FPS_7_5    = 750,
    /*fps:12.5.*/
    GADI_VI_FPS_12_5   = 1250,
    /*fps:23.976.*/
    GADI_VI_FPS_23_976 = 23976000,
    /*fps:29.97.*/
    GADI_VI_FPS_29_97  = 299700,
    /*fps:59.94.*/
    GADI_VI_FPS_59_94  = 599400,
}GADI_VI_FrameRateEnumT;


/*!
*******************************************************************************
** \brief ir mode enum.
*******************************************************************************
*/
typedef enum
{
    /*clear ircut mode,maintain preview status.*/
    GADI_VI_IRCUT_CLEAR = 0,
    /*ircut night mode.*/
    GADI_VI_IRCUT_NIGHT,
    /*ircut day mode.*/
    GADI_VI_IRCUT_DAY,
}GADI_VI_IRModeEnumT;

/*!
*******************************************************************************
** \brief standby update time.
*******************************************************************************
*/

typedef enum
{
    /*standby mode,update data immediate.*/
    GADI_VI_STANDBY_UPDATE_IMMEDIATELY = 0x00,
    /*standby mode,second update data.*/
    GADI_VI_STANDBY_UPDATE_IN_VSYNC,
}GADI_VI_StandbyModeEnumT;

/*!
*******************************************************************************
** \brief standby update mode.
*******************************************************************************
*/
typedef enum
{
    /*control reset pin into standby mode.*/
    GADI_VI_STANDBY_SET_HW_GPIO        = 0x00,
    /*control i2c reg into standby mode.*/
    GADI_VI_STANDBY_SET_SOFT_REG,
    GADI_VI_STANDBY_TYPE_NUM,
}GADI_VI_StandbyTypeEnumT;


/*!
********************************************************************************
** \brief video input mode enum.
*******************************************************************************
*/
typedef enum {
    /*single or double vin simultaneous work.*/
	GADI_VI_NORMAL_MODE = 0,
    /*in double vin case, only one vin work, and need indicate which one work.*/
    /*need disable vi, and then used this mode.*/
	GADI_VI_SHARE_MODE = 1,
    /*TBD mode.*/
    GADI_VI_MULTY_MODE = 2,
    /*in double vin case, only one vin work, and need indicate which one work.*/
    /*no need disable vi, on the fly mode, but nedd guarantee the same same sensor*/
    /*and the same configurations.*/
    GADI_VI_SHARE_ONLINE_MODE = 3,
} GADI_VI_ModeEnumT;

//*****************************************************************************
//*****************************************************************************
//** Data Structures
//*****************************************************************************
//*****************************************************************************

/*!
*******************************************************************************
** \brief video input mirror mode struct.
*******************************************************************************
*/
typedef struct
{
    /*enum GADI_VI_MirrorPatternEnumT pattern.*/
    GADI_VI_MirrorPatternEnumT mirrorPattern;
    /*enum GADI_VI_BayerPatternEnumT bayer_pattern.*/
    GADI_VI_BayerPatternEnumT  bayerPattern;
} GADI_VI_MirrorModeParamsT;

/*!
*******************************************************************************
** \brief standby gpio struct.
*******************************************************************************
*/
typedef struct
{
    /*standby gpio id.*/
    GADI_S32 gpio_id;
    /*standby effective level.*/
    GADI_S32 active_level;
    /*standby effective level maintain time.*/
    GADI_S32 active_delay;       //ms
}GADI_VI_StandbyGPIOParamsT;

/*!
*******************************************************************************
** \brief hardware standby parameters struct.
*******************************************************************************
*/
typedef struct{
    /*vi module id.*/
    GADI_S32 src_id;
    /*vi module enable,0:normal work    1:standby.*/
    GADI_U8 standby_en;
    /*vi module update parameter struct,0:immediately    1:in vsync.*/
    GADI_VI_StandbyModeEnumT    standby_update_mode; 
    /*vi module standby mode struct,0:hardware(GPIO) 1:software(register).*/
    GADI_VI_StandbyTypeEnumT    standby_type;
    /*vi module standby gpio parameter.*/
    GADI_VI_StandbyGPIOParamsT  standby_gpio;
}GADI_VI_StandbyParamsT;


/*!
*******************************************************************************
** \brief video input setting parameters struct.
*******************************************************************************
*/
typedef struct
{
    /*use default sensor video output resolution if the width or height is 0.*/
    /*vi resolution width.*/
    GADI_U32                   width;
    /*vi resolution height.*/
    GADI_U32                   height;
    /*vi frame rate.*/
    GADI_U32                   fps;
    /*vi output row or field.*/
    GADI_U32                   pi;
    /*enum GADI_VI_MirrorPatternEnumT pattern*/
    GADI_VI_MirrorPatternEnumT mirrorPattern;
    /*enum GADI_VI_BayerPatternEnumT bayer_pattern*/
    GADI_VI_BayerPatternEnumT  bayerPattern;
} GADI_VI_SettingParamsT;


/*!
*******************************************************************************
** \brief sensor register struct.
*******************************************************************************
*/
typedef struct
{
    /*sensor register address.*/
    GADI_U32 reg;
    /*sensor register data.*/
    GADI_U32 data;
    /*sensor register data mask.*/
    GADI_U32 mask;
    GADI_U32 reserved;
}GADI_VI_SensorRegInfoT;

/*
*******************************************************************************
** \brief input source device parameter.
*******************************************************************************
*/
typedef struct
{
    /*selecet vi channel id.*/
    GADI_VI_ChannelEnumT       channelId;
    /*sensor driver parameters bin path.*/
    char*                      sensorHWPath;
} GADI_VI_OpenParamsT;

/*!
*******************************************************************************
** \brief vin mode parameters struct.
*******************************************************************************
*/
typedef struct
{
    /*selecet vin mode.*/
    GADI_VI_ModeEnumT       vinMode;
    /*indacate which vi channel work in GADI_VI_SHARE_MODE mode.*/
    GADI_VI_ChannelEnumT    channelId;
}GADI_VI_ModeParamsT;


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
** \brief Initialize the ADI video input module.
**
** \return
** - #GADI_OK
** - #GADI_DSP_ERR_ALREADY_INITIALIZED
** - #GADI_DSP_ERR_OUT_OF_MEMORY
**
** \sa gadi_vi_exit
**
*******************************************************************************
*/
GADI_ERR gadi_vi_init(void);

/*!
*******************************************************************************
** \brief Shutdown the ADI video input module.
**
** \return
** - #GADI_OK
** - #GADI_DSP_ERR_NOT_INITIALIZED
**
** \sa gadi_vi_init
**
*******************************************************************************
*/
GADI_ERR gadi_vi_exit(void);

/*!
*******************************************************************************
** \brief Open one ADI video input module instance.
**
** \param[in] errorCodePtr pointer to return the error code.
**
** \param[in] UserConfig: Contains hardware parameters(sensor or other input device).
**                                  if NULL, system will use default paramters.
**
** \return Return an valid handle of ADI vi module instance.
**
** \sa gadi_vi_close
**
*******************************************************************************
*/
GADI_SYS_HandleT gadi_vi_open(GADI_VI_OpenParamsT* openParamPtr, GADI_ERR* errorCodePtr);
/*!
*******************************************************************************
** \brief close one ADI video input module instance.
**
** \param[in]  handle     Valid ADI vi instance handle previously opened by
**                        #gadi_vi_open.
**
** \return
** - #GADI_OK
** - #GADI_VI_ERR_BAD_PARAMETER
**
** \sa gadi_vi_open
**
*******************************************************************************
*/
GADI_ERR gadi_vi_close(GADI_SYS_HandleT handle);

/*!
*******************************************************************************
** \brief get video input resolution.
**
** \param[in]  handle     Valid dsp ADI instance handle previously opened by
**                        #gadi_vi_open.
** \param[in]  viWidth   pointer to return the video input width.
** \param[in]  viHeight  pointer to return the video input height.
**
** \return
** - #GADI_OK
** - #GADI_VI_ERR_NOT_OPEN
** - #GADI_VI_ERR_FROM_DRIVER
**
** \sa gadi_vi_open
**
*******************************************************************************
*/
GADI_ERR gadi_vi_get_resolution(GADI_SYS_HandleT handle,
    GADI_U32* viWidth, GADI_U32* viHeight);


/*!
*******************************************************************************
** \brief get video input information.
**
** \param[in]  handle     Valid dsp ADI instance handle previously opened by
**                        #gadi_vi_open.
** \param[in]  viSettingParams   pointer to return the video input information struct.
**
** \return
** - #GADI_OK
** - #GADI_VI_ERR_NOT_OPEN
** - #GADI_VI_ERR_FROM_DRIVER
**
** \sa gadi_vi_open
**
*******************************************************************************
*/
GADI_ERR gadi_vi_get_params(GADI_SYS_HandleT handle, GADI_VI_SettingParamsT* viSettingParams);


/*!
*******************************************************************************
** \brief set video input information.
**
** \param[in]  handle     Valid dsp ADI instance handle previously opened by
**                        #gadi_vi_open.
** \param[in]  viSettingParams   input pointer of video input information struct.
**
** \return
** - #GADI_OK
** - #GADI_VI_ERR_NOT_OPEN
** - #GADI_VI_ERR_FROM_DRIVER
** - #GADI_VI_ERR_BAD_PARAMETER
**
** \sa gadi_vi_open
**
*******************************************************************************
*/
GADI_ERR gadi_vi_set_params(GADI_SYS_HandleT handle, GADI_VI_SettingParamsT* viSettingParams);

/*!
*******************************************************************************
** \brief enable or disable VI. Enable VI means start to receive sensor data.
**
** \param[in]  handle    Valid dsp ADI instance handle previously opened by
**                        #gadi_vi_open.
** \param[in]  enable    1: enable, 0: disable.
**
** \return
** - #GADI_OK
** - #GADI_VI_ERR_NOT_OPEN
** - #GADI_VI_ERR_FROM_DRIVER
** - #GADI_VI_ERR_BAD_PARAMETER
**
** \sa gadi_vi_open
**
*******************************************************************************
*/
GADI_ERR gadi_vi_enable(GADI_SYS_HandleT handle, GADI_U32 enable);

/*!
*******************************************************************************
** \brief set sensor output mirror/rotate mode.
**
** \param[in]  handle    Valid dsp ADI instance handle previously opened by
**                        #gadi_vi_open.
** \param[in]  mirrorModePtr  input pointer of VI data mirror struct.
**
** \return
** - #GADI_OK
** - #GADI_VI_ERR_NOT_OPEN
** - #GADI_VI_ERR_FROM_DRIVER
** - #GADI_VI_ERR_BAD_PARAMETER
**
** \sa gadi_vi_open
**
*******************************************************************************
*/
GADI_ERR gadi_vi_set_mirror_mode(GADI_SYS_HandleT handle,
                        GADI_VI_MirrorModeParamsT  *mirrorModePtr);

/*!
*******************************************************************************
** \brief get sensor output mirror/rotate mode.
**
** \param[in]  handle    Valid dsp ADI instance handle previously opened by
**                        #gadi_vi_open.
** \param[in]  mirrorModePtr  input pointer of VI data mirror struct.
**
** \return
** - #GADI_OK
** - #GADI_VI_ERR_NOT_OPEN
** - #GADI_VI_ERR_FROM_DRIVER
** - #GADI_VI_ERR_BAD_PARAMETER
**
** \sa gadi_vi_open
**
*******************************************************************************
*/
GADI_ERR gadi_vi_get_mirror_mode(GADI_SYS_HandleT handle,
                        GADI_VI_MirrorModeParamsT  *mirrorModePtr);

/*!
*******************************************************************************
** \brief set ir cut mode.
**
** \**
** \sa gadi_vi_open
**
*******************************************************************************
*/

GADI_ERR gadi_vi_set_ircut_control(GADI_SYS_HandleT handle, GADI_VI_IRModeEnumT mode);

/*!
*******************************************************************************
** \brief reset image sensor .
**
** \param[in]   handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \return
** - #GADI_OK
**
** \sa gadi_vi_open
**
*******************************************************************************
*/
GADI_ERR gadi_vi_reset_sensor(GADI_SYS_HandleT handle);

/*!
*******************************************************************************
** \brief get vi core interrupt.
**
** \param[in]   handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \return
** - #GADI_OK
**
** \sa gadi_vi_get_interrupt
**
*******************************************************************************
*/
GADI_ERR gadi_vi_get_interrupt(GADI_SYS_HandleT handle, GADI_ULONG *vi_times);


/*!
*******************************************************************************
** \brief set vi framerate.
**
** \param[in]   handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in&out]  frameRate: input vi framerate:GADI_VI_FrameRateEnumT that we want,
**                            and return actual framerate .
** \return
** - #GADI_OK
**
** \sa gadi_vi_get_interrupt
**
*******************************************************************************
*/
GADI_ERR gadi_vi_set_framerate(GADI_SYS_HandleT handle,
        GADI_VI_FrameRateEnumT frameRate);

/*!
*******************************************************************************
** \brief get vi framerate.
**
** \param[in]   handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in&out]  frameRate: input vi framerate:GADI_VI_FrameRateEnumT that we want,
**                            and return actual framerate .
** \return
** - #GADI_OK
**
** \sa gadi_vi_get_framerate
**
*******************************************************************************
*/
GADI_ERR gadi_vi_get_framerate(GADI_SYS_HandleT handle,
        GADI_VI_FrameRateEnumT *frameRate);

/*!
*******************************************************************************
** \brief set sensor registers.
**
** \param[in]   handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]   regs: pointer for sensor register array. Use the MACRO of
**                    GADI_VI_SENSOR_TABLE_FLAG_END as the end flag of array
** \return
** - #GADI_OK
**
** \sa gadi_vi_set_sensor_regs
**
*******************************************************************************
*/
GADI_ERR gadi_vi_set_sensor_reg(GADI_SYS_HandleT handle,
    GADI_VI_SensorRegInfoT *regs);

/*!
*******************************************************************************
** \brief print vi params.
**
** \param[in]   handle     Valid ADI vi instance handle previously opened by
**                        #gadi_vi_open.
** \return
** - #GADI_OK
**
** \sa gadi_vi_get_interrupt
**
*******************************************************************************
*/
GADI_ERR gadi_vi_print_params(GADI_SYS_HandleT handle);

/*!
*******************************************************************************
** \brief set sensor standby.
**
** \param[in]   handle     Valid ADI venc instance handle previously opened by
**                        #gadi_vi_open.
** \param[in&out]
**
** \return
** - #GADI_OK
**
** \sa gadi_vi_set_sensor_standby
**
*******************************************************************************
*/
GADI_ERR gadi_vi_set_sensor_standby(GADI_SYS_HandleT handle,
         GADI_VI_StandbyParamsT * standbyPtr);

/*!
*******************************************************************************
** \brief set vi mode.
**
** \param[in]   handle     Valid ADI venc instance handle previously opened by
**                        #gadi_vi_open.
** \param[in&out]
**
** \return
** - #GADI_OK
**
** \sa gadi_vi_open
**
*******************************************************************************
*/
GADI_ERR gadi_vi_set_mode(GADI_SYS_HandleT handle,
    GADI_VI_ModeParamsT * modePtr);

#ifdef __cplusplus
    }
#endif


#endif /* _ADI_VI_H_ */
