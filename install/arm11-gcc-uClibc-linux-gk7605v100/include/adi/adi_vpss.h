/*!
*****************************************************************************
** \file        adi/include/adi_vpss.h
**
** \brief       ADI video process sub-system module header file
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2015-2019 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _ADI_VPSS_H_
#define _ADI_VPSS_H_

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
#define GADI_VPSS_ERR_BAD_PARAMETER                                          \
                              (GADI_VPSS_MODULE_BASE + GADI_ERR_BAD_PARAMETER)
/*! Memory allocation failed. */
#define GADI_VPSS_ERR_OUT_OF_MEMORY                                          \
                              (GADI_VPSS_MODULE_BASE + GADI_ERR_OUT_OF_MEMORY)
/*! Device already initialised. */
#define GADI_VPSS_ERR_ALREADY_INITIALIZED                                    \
                              (GADI_VPSS_MODULE_BASE + GADI_ERR_ALREADY_INITIALIZED)
/*! Device not initialised. */
#define GADI_VPSS_ERR_NOT_INITIALIZED                                        \
                              (GADI_VPSS_MODULE_BASE + GADI_ERR_NOT_INITIALIZED)
/*! Feature or function is not available. */
#define GADI_VPSS_ERR_FEATURE_NOT_SUPPORTED                                  \
                              (GADI_VPSS_MODULE_BASE + GADI_ERR_FEATURE_NOT_SUPPORTED)
/*! Timeout occured. */
#define GADI_VPSS_ERR_TIMEOUT                                                \
                              (GADI_VPSS_MODULE_BASE + GADI_ERR_TIMEOUT)
/*! The device is busy, try again later. */
#define GADI_VPSS_ERR_DEVICE_BUSY                                            \
                              (GADI_VPSS_MODULE_BASE + GADI_ERR_DEVICE_BUSY)
/*! Invalid handle was passed. */
#define GADI_VPSS_ERR_INVALID_HANDLE                                         \
                              (GADI_VPSS_MODULE_BASE + GADI_ERR_INVALID_HANDLE)
/*! Semaphore could not be created. */
#define GADI_VPSS_ERR_SEMAPHORE_CREATE                                       \
                               (GADI_VPSS_MODULE_BASE + GADI_ERR_SEMAPHORE_CREATE)
/*! The driver's used version is not supported. */
#define GADI_VPSS_ERR_UNSUPPORTED_VERSION                                    \
                               (GADI_VPSS_MODULE_BASE + GADI_ERR_UNSUPPORTED_VERSION)
/*! The driver's used version is not supported. */
#define GADI_VPSS_ERR_FROM_DRIVER                                            \
                               (GADI_VPSS_MODULE_BASE + GADI_ERR_FROM_DRIVER)
/*! The device/handle is not open.. */
#define GADI_VPSS_ERR_NOT_OPEN                                               \
                               (GADI_VPSS_MODULE_BASE + GADI_ERR_NOT_OPEN)
/*! The device/map is err.. */
#define GADI_VPSS_ERR_MAP                                               \
                               (GADI_VPSS_MODULE_BASE + GADI_ERR_MAP)
/*! operation not permit.*/
#define GADI_VPSS_ERR_OPERATION_NOT_PERMIT                                  \
                               (GADI_VPSS_MODULE_BASE + GADI_ERR_OPERATION_NOT_PERMIT)

//*****************************************************************************
//*****************************************************************************
//** Enumerated types
//*****************************************************************************
//*****************************************************************************
/*!
*******************************************************************************
** \brief video process sub-system channel index enum.
*******************************************************************************
*/
typedef enum {
    /*VPSS Channel MAIN_A.*/
    GADI_VPSS_CHANNEL_MAIN_A       = 0,
    /*VPSS Channel MAIN_B.*/
    GADI_VPSS_CHANNEL_MAIN_B       = 1,
    /*VPSS Channel SUB_A.*/
    GADI_VPSS_CHANNEL_SUB_A        = 2,
    /*VPSS Channel SUB_B.*/
    GADI_VPSS_CHANNEL_SUB_B        = 3,
    /*VPSS Channel SUB_C.*/
    GADI_VPSS_CHANNEL_SUB_C        = 4,
    /*VPSS Channel number.*/
    GADI_VPSS_CHANNEL_NUM,
} GADI_VPSS_ChannelEnumT;


/*!
*******************************************************************************
** \brief video process sub-system channel type enum.
*******************************************************************************
*/
typedef enum {
    /*channel disabled*/
    GADI_VPSS_CHANNEL_TYPE_OFF     = 0,
    /*channel for encoding*/
    GADI_VPSS_CHANNEL_TYPE_ENCODE  = 1,
    /*channel for video output*/
    GADI_VPSS_CHANNEL_TYPE_VOUT    = 2,
} GADI_VPSS_ChannelTypeEnumT;

/*!
*******************************************************************************
** \brief encoder buffer mode enum.
*******************************************************************************
*/
typedef enum
{
    /*real time mode: width <= 1920,donot split screen.*/
    GADI_VPSS_OP_MODE_REAL_TIME         = 0,
    /*non real time mode: width > 1920, split screen for width < 3000.*/
    GADI_VPSS_OP_MODE_NON_REAL_TIME0    = 1,
    /*non real time mode: width > 1920, split screen for width > 3000.*/
    GADI_VPSS_OP_MODE_NON_REAL_TIME1    = 2,
    /*double vin mode: width <= 1920.*/
    GADI_VPSS_OP_MODE_DUAL_VI           = 3,
    /*non real time mode: width > 1920, split screen for width < 3000. */
    /*for higher performance h265 encoder.*/
    GADI_VPSS_OP_MODE_NON_REAL_TIME2    = 4,
    /*for higher performance h264/h265 encoder,dsp will use more ram.*/
    GADI_VPSS_OP_MODE_NON_REAL_TIME3    = 5,
    /*VPSS operation mode number.*/
    GADI_VPSS_OP_MODE_NUM               = 6,
}GADI_VPSS_OpModeEnumT;

//*****************************************************************************
//*****************************************************************************
//** Data Structures
//*****************************************************************************
//*****************************************************************************
/*!
*******************************************************************************
** \brief channel parameters struct.
*******************************************************************************
*/
typedef struct
{
    /*channel mainA type 0:disable, 1:encoding, 2:vout(invalid for main channel).*/
    GADI_U16            chanMainAType;
    /*channel mainA output width.*/
    GADI_U16            chanMainAWidth;
    /*channel mainA output height.*/
    GADI_U16            chanMainAHeight;
    /*channel keep aspect ratio.*/
    /*vi width scale to video width ratio not equal video height ratio.*/
    /*keep aspect ration means use the small aspect ratio.*/
    /* 1: means keep aspect ration, 0: means do not keep.*/
    GADI_U16            chanMainAKeepAspRat;
    /*channel mainB type 0:disable, 1:encoding, 2:vout(invalid for main channel).*/
    GADI_U16            chanMainBType;
    /*channel mainB output width.*/
    GADI_U16            chanMainBWidth;
    /*channel mainB output height.*/
    GADI_U16            chanMainBHeight;
    /*channel keep aspect ratio.*/
    /*vi width scale to video width ratio not equal video height ratio.*/
    /*keep aspect ration means use the small aspect ratio.*/
    /* 1: means keep aspect ration, 0: means do not keep.*/
    GADI_U16            chanMainBKeepAspRat;
    /*channel subA type 0:disable, 1:encoding, 2:vout(invalid for subA channel).*/
    GADI_U16            chanSubAType;
    /*channel subA output width.*/
    GADI_U16            chanSubAWidth;
    /*channel subA output height.*/
    GADI_U16            chanSubAHeight;
    /*channel subA source from(mainA or mainB).*/
    GADI_U16            chanSubASourceFrom;
    /*channel keep aspect ratio.*/
    /*vi width scale to video width ratio not equal video height ratio.*/
    /*keep aspect ration means use the small aspect ratio.*/
    /* 1: means keep aspect ration, 0: means do not keep.*/
    GADI_U16            chanSubAKeepAspRat;
    /*channel subB type 0:disable, 1:encoding, 2:vout.*/
    GADI_U16            chanSubBType;
    /*channel subB output width.*/
    GADI_U16            chanSubBWidth;
    /*channel subB output height.*/
    GADI_U16            chanSubBHeight;
    /*channel subB source from(mainA or mainB).*/
    GADI_U16            chanSubBSourceFrom;
    /*channel keep aspect ratio.*/
    /*vi width scale to video width ratio not equal video height ratio.*/
    /*keep aspect ration means use the small aspect ratio.*/
    /* 1: means keep aspect ration, 0: means do not keep.*/
    GADI_U16            chanSubBKeepAspRat;
    /*channel subC type 0:disable, 1:encoding, 2:vout(invalid for subB channel).*/
    GADI_U16            chanSubCType;
    /*channel subC output width.*/
    GADI_U16            chanSubCWidth;
    /*channel subC output height.*/
    GADI_U16            chanSubCHeight;
    /*channel subC source from(mainA or mainB).*/
    GADI_U16            chanSubCSourceFrom;
    /*channel keep aspect ratio.*/
    /*vi width scale to video width ratio not equal video height ratio.*/
    /*keep aspect ration means use the small aspect ratio.*/
    /* 1: means keep aspect ration, 0: means do not keep.*/
    GADI_U16            chanSubCKeepAspRat;
}GADI_VPSS_ChannelsParamsT;


/*!
*******************************************************************************
** \brief channel parameters struct.
*******************************************************************************
*/
typedef struct
{
    /*intlec mode: 0: OFF  1: use progressive vi to encode interlacing video*/
    GADI_U8             viIntlcMode;
    /*channel mainA deintlc mode, 0:disable . 1:deintlc for intlc VI.*/
    GADI_U8             chanMainADeintlcMode;
    /*channel mainB deintlc mode, 0:disable . 1:deintlc for intlc VI.*/
    GADI_U8             chanMainBDeintlcMode;
    /*channel subA deintlc mode, 0:disable . 1:deintlc for intlc VI.*/
    GADI_U8             chanSubADeintlcMode;
    /*channel subB deintlc mode, 0:disable . 1:deintlc for intlc VI.*/
    GADI_U8             chanSubBDeintlcMode;
    /*channel subC deintlc mode, 0:disable . 1:deintlc for intlc VI.*/
    GADI_U8             chanSubCDeintlcMode;
}GADI_VPSS_ChannelsDeintlcModeT;


/*!
*******************************************************************************
** \brief set one sub_channel input window croping struct.
*******************************************************************************
*/
typedef struct
{
    /*vpss channel index.*/
    GADI_U32    channelId;
    /*vpss channel output width.*/
    GADI_U16    width;
    /*vpss channel output height.*/
    GADI_U16    height;
    /*vpss channel input width.*/
    GADI_U16    inputWidth;
    /*vpss channel input height.*/
    GADI_U16    inputHeight;
    /*vpss channel input x offset.*/
    GADI_U16    inputXOffset;
    /*vpss channel input y offset.*/
    GADI_U16    inputYOffset;
}GADI_VPSS_SubChanlCropParamsT;

/*!
*******************************************************************************
** \brief set  sub_channel buf num.
*******************************************************************************
*/
typedef struct
{
    /*one buf size: (ChannelWidth * ChannelHeight) * 3 >> 1 */
    /*region:  [2,6] (set 0 to use default value)*/
    /*channel subA buf num.*/
    GADI_U8             chanSuaBufNum;
    /*channel subB buf num.*/
    GADI_U8             chanSubBufNum;
    /*channel subC buf num.*/
    GADI_U8             chanSucBufNum;
}GADI_VPSS_SubChanlBufNumT;

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
** \brief Initialize the ADI video encode module.
**
** \return
** - #GADI_OK
** - #GADI_VENC_ERR_OUT_OF_MEMORY
** - #GADI_VENC_ERR_ALREADY_INITIALIZED
**
** \sa gadi_vi_exit
**
*******************************************************************************
*/
GADI_ERR gadi_vpss_init(void);

/*!
*******************************************************************************
** \brief Shutdown the ADI video encode module.
**
** \return
** - #GADI_OK
** - #GADI_VENC_ERR_NOT_INITIALIZED
**
** \sa gadi_vi_init
**
*******************************************************************************
*/
GADI_ERR gadi_vpss_exit(void);

/*!
*******************************************************************************
** \brief Open one ADI video venc module instance.
**
** \param[in] openParams a struct pointer of open prameters.
** \param[in] errorCodePtr pointer to return the error code.
**
** \return Return an valid handle of ADI vi module instance.
**
** \sa gadi_venc_close
**
*******************************************************************************
*/
GADI_SYS_HandleT gadi_vpss_open(GADI_ERR * errorCodePtr);

/*!
*******************************************************************************
** \brief close one ADI video encode module instance.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
**
** \return
** - #GADI_OK
** - #GADI_VENC_ERR_BAD_PARAMETER
**
** \sa gadi_venc_open
**
*******************************************************************************
*/
GADI_ERR gadi_vpss_close(GADI_SYS_HandleT handle);


/*!
*******************************************************************************
** \brief set video op mode.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_vpss_open.
**
** \return
** - #GADI_OK
** - #GADI_VENC_ERR_BAD_PARAMETER
**
** \sa gadi_vpss_open
**
*******************************************************************************
*/

GADI_ERR gadi_vpss_set_op_mode(GADI_SYS_HandleT handle, GADI_VPSS_OpModeEnumT mode);


/*!
*******************************************************************************
** \brief get all(four) channels params.
**
** \param[in]   handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[out]  chansParams pointer of all channels parameters struct;
**
** \return
** - #GADI_OK
** - #GADI_VENC_ERR_FROM_DRIVER
** - #GADI_VENC_ERR_BAD_PARAMETER
**
** \sa gadi_venc_open
**
*******************************************************************************
*/
GADI_ERR gadi_vpss_get_channels_params(GADI_SYS_HandleT handle,
        GADI_VPSS_ChannelsParamsT *chansParams);

/*!
*******************************************************************************
** \brief set all(four) channels parameters.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]  bufsParams pointer of all channels parameters struct;
**
** \return
** - #GADI_OK
** - #GADI_VENC_ERR_FROM_DRIVER
** - #GADI_VENC_ERR_BAD_PARAMETER
**
** \sa gadi_venc_open
**
*******************************************************************************
*/
GADI_ERR gadi_vpss_set_channels_params(GADI_SYS_HandleT handle,
        GADI_VPSS_ChannelsParamsT *chansParams);

/*!
*******************************************************************************
** \brief get all(four) channels deintlc parameters.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]  bufsParams pointer of all channels parameters struct;
**
** \return
** - #GADI_OK
** - #GADI_VENC_ERR_FROM_DRIVER
** - #GADI_VENC_ERR_BAD_PARAMETER
**
** \sa gadi_venc_open
**
*******************************************************************************
*/
GADI_ERR gadi_vpss_get_channels_deintlc(GADI_SYS_HandleT handle,
    GADI_VPSS_ChannelsDeintlcModeT *chansParams);

/*!
*******************************************************************************
** \brief set all(four) channels deintlc parameters.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]  bufsParams pointer of all channels parameters struct;
**
** \return
** - #GADI_OK
** - #GADI_VENC_ERR_FROM_DRIVER
** - #GADI_VENC_ERR_BAD_PARAMETER
**
** \sa gadi_venc_open
**
*******************************************************************************
*/
GADI_ERR gadi_vpss_set_channels_deintlc(GADI_SYS_HandleT handle,
    GADI_VPSS_ChannelsDeintlcModeT *chansParams);

/*!
*******************************************************************************
** \brief set sub channel crop parameters.
**
** \param[in]   handle  Valid ADI vpss instance handle previously opened by
**                        #gadi_vpss_open.
**
** \param[in]  cropPar  pointer sub channel crop parameters struct;
**
** \return
** - #GADI_OK
**
** \sa gadi_vpss_open
**
*******************************************************************************
*/
GADI_ERR gadi_vpss_set_sub_channel_crop(GADI_SYS_HandleT handle,
        GADI_VPSS_SubChanlCropParamsT *cropPar);

/*!
*******************************************************************************
** \brief print out all parameters.
**
** \param[in]   handle     Valid ADI vpss instance handle previously opened by
**                        #gadi_vpss_open.
**
** \return
** - #GADI_OK
**
** \sa gadi_vpss_open
**
*******************************************************************************
*/
GADI_ERR gadi_vpss_print_params(GADI_SYS_HandleT handle);

/*!
*******************************************************************************
** \brief set vpss sub channel buffer number.
**
** \param[in]   handle     Valid ADI vpss instance handle previously opened by
**                        #gadi_vpss_open.
**
** \param[in]  chansBufNum  pointer of sub channels buffer number struct;
**
** \return
** - #GADI_OK
**
** \sa gadi_vpss_open
**
*******************************************************************************
*/
GADI_ERR gadi_vpss_set_sub_channel_buf_num(GADI_SYS_HandleT handle,
    GADI_VPSS_SubChanlBufNumT *chansBufNum);

#ifdef __cplusplus
}
#endif



#endif /* _ADI_VPSS_H_ */

