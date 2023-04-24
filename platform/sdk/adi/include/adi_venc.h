/*!
*****************************************************************************
** \file        adi/include/adi_venc.h
**
** \brief       ADI video encode module header file
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _ADI_VENC_H_
#define _ADI_VENC_H_

#include "stdio.h"
#include "adi_types.h"

//*****************************************************************************
//*****************************************************************************
//** Defines and Macros
//*****************************************************************************
//*****************************************************************************

/*! The num of chan .. */
#define MAX_STREAM_NUM  6 /*MAX STREAM*/

/*
*******************************************************************************
** Defines for general error codes of the module.
*******************************************************************************
*/
/*! Bad parameter passed. */
#define GADI_VENC_ERR_BAD_PARAMETER                                          \
                              (GADI_VENC_MODULE_BASE + GADI_ERR_BAD_PARAMETER)
/*! Memory allocation failed. */
#define GADI_VENC_ERR_OUT_OF_MEMORY                                          \
                              (GADI_VENC_MODULE_BASE + GADI_ERR_OUT_OF_MEMORY)
/*! Device already initialised. */
#define GADI_VENC_ERR_ALREADY_INITIALIZED                                    \
                              (GADI_VENC_MODULE_BASE + GADI_ERR_ALREADY_INITIALIZED)
/*! Device not initialised. */
#define GADI_VENC_ERR_NOT_INITIALIZED                                        \
                              (GADI_VENC_MODULE_BASE + GADI_ERR_NOT_INITIALIZED)
/*! Feature or function is not available. */
#define GADI_VENC_ERR_FEATURE_NOT_SUPPORTED                                  \
                              (GADI_VENC_MODULE_BASE + GADI_ERR_FEATURE_NOT_SUPPORTED)
/*! Timeout occured. */
#define GADI_VENC_ERR_TIMEOUT                                                \
                              (GADI_VENC_MODULE_BASE + GADI_ERR_TIMEOUT)
/*! The device is busy, try again later. */
#define GADI_VENC_ERR_DEVICE_BUSY                                            \
                              (GADI_VENC_MODULE_BASE + GADI_ERR_DEVICE_BUSY)
/*! Invalid handle was passed. */
#define GADI_VENC_ERR_INVALID_HANDLE                                         \
                              (GADI_VENC_MODULE_BASE + GADI_ERR_INVALID_HANDLE)
/*! Semaphore could not be created. */
#define GADI_VENC_ERR_SEMAPHORE_CREATE                                       \
                               (GADI_VENC_MODULE_BASE + GADI_ERR_SEMAPHORE_CREATE)
/*! The driver's used version is not supported. */
#define GADI_VENC_ERR_UNSUPPORTED_VERSION                                    \
                               (GADI_VENC_MODULE_BASE + GADI_ERR_UNSUPPORTED_VERSION)
/*! The driver's used version is not supported. */
#define GADI_VENC_ERR_FROM_DRIVER                                            \
                               (GADI_VENC_MODULE_BASE + GADI_ERR_FROM_DRIVER)
/*! The device/handle is not open.. */
#define GADI_VENC_ERR_NOT_OPEN                                               \
                               (GADI_VENC_MODULE_BASE + GADI_ERR_NOT_OPEN)
/*! The device/map is err.. */
#define GADI_VENC_ERR_MAP                                               \
                               (GADI_VENC_MODULE_BASE + GADI_ERR_MAP)
/*! operation not permit.*/
#define GADI_VENC_ERR_OPERATION_NOT_PERMIT                                            \
                               (GADI_VENC_MODULE_BASE + GADI_ERR_OPERATION_NOT_PERMIT)


//*****************************************************************************
//*****************************************************************************
//** Enumerated types
//*****************************************************************************
//*****************************************************************************
/*!
*******************************************************************************
** \brief video encode output stream enum.
*******************************************************************************
*/
typedef enum {
    /*steam index:first*/
    GADI_VENC_STREAM_FIRST = 0,
    /*steam index:second*/
    GADI_VENC_STREAM_SECOND,
    /*steam index:third*/
    GADI_VENC_STREAM_THIRD,
    /*steam index:forth*/
    GADI_VENC_STREAM_FORTH,
	/*steam index:fifth*/
    GADI_VENC_STREAM_FIFTH,
	/*steam index:sixth*/
    GADI_VENC_STREAM_SIXTH,
	/*steam number*/
    GADI_VENC_STREAM_NUM,
} GADI_VENC_StreamEnumT;

/*!
*******************************************************************************
** \brief video encode stream state enum.
*******************************************************************************
*/
typedef enum {
    /*uninitialized or unconfigured*/
    GADI_VENC_STREAM_STATE_UNKNOWN = 0,
    /*configured ready, but not started encoding yet*/
    GADI_VENC_STREAM_STATE_READY_FOR_ENCODING = 1,
    /*encoding*/
    GADI_VENC_STREAM_STATE_ENCODING = 2,
    /*transition state: starting to encode*/
    GADI_VENC_STREAM_STATE_STARTING = 3,
    /*transition state: stopping encoding*/
    GADI_VENC_STREAM_STATE_STOPPING = 4,
    /*error state: dsp encoding time out*/
    GADI_VENC_STREAM_STATE_DSP_TIMEOUT = 5,
    /*known error*/
    GADI_VENC_STREAM_STATE_ERROR    = 255,
} GADI_VENC_StreamStateEnumT;

/*!
*******************************************************************************
** \brief video encode type enum.
*******************************************************************************
*/
typedef enum {
    /*steam type:off*/
    GADI_VENC_TYPE_OFF    = 0,
    /*steam type:H264*/
    GADI_VENC_TYPE_H264   = 1,
    /*steam type:MJPEG*/
    GADI_VENC_TYPE_MJPEG  = 2,
    /*steam type:H265*/
    GADI_VENC_TYPE_H265   = 3,
    /*steam type number.*/
    GADI_VENC_TYPE_NUM,
} GADI_VENC_TypeEnumT;

/*!
*******************************************************************************
** \brief video encode bit rate control mode enum.
*******************************************************************************
*/
typedef enum {
    /*H264/H265 encode stream bit rate control mode:center bit rate.*/
    GADI_VENC_CBR_MODE = 0,
    /*H264/H265 encode stream bit rate control mode:variable bit rate.*/
    GADI_VENC_VBR_MODE,
    /*H264/H265 encode stream bit rate control mode:center bit rate.*/
    /*for guarantee bitrate, it will skip frame.*/
    GADI_VENC_CBR_QUALITY_MODE,
    /*H264/H265 encode stream bit rate control mode:variable bit rate.*/
    /*for guarantee bitrate, it will skip frame.*/
    GADI_VENC_VBR_QUALITY_MODE,
    /*H264/H265 encode stream bit rate control mode number.*/
    GADI_VENC_BRC_MODE_NUM,
}GADI_VENC_BrcModeEnumT;

/*!
*******************************************************************************
** \brief video encode support frame rate.
*******************************************************************************
*/
typedef enum {
    /*use default sensor frame rate.*/
    GADI_VENC_FPS_AUTO   = 0,
    /*fps:1.*/
    GADI_VENC_FPS_1      = 1,
    GADI_VENC_FPS_2      = 2,
    GADI_VENC_FPS_3      = 3,
    GADI_VENC_FPS_4      = 4,
    GADI_VENC_FPS_5      = 5,
    GADI_VENC_FPS_6      = 6,
    GADI_VENC_FPS_7      = 7,
    GADI_VENC_FPS_8      = 8,
    GADI_VENC_FPS_9      = 9,
    GADI_VENC_FPS_10     = 10,
    GADI_VENC_FPS_11     = 11,
    GADI_VENC_FPS_12     = 12,
    GADI_VENC_FPS_13     = 13,
    GADI_VENC_FPS_14     = 14,
    GADI_VENC_FPS_15     = 15,
    GADI_VENC_FPS_16     = 16,
    GADI_VENC_FPS_17     = 17,
    GADI_VENC_FPS_18     = 18,
    GADI_VENC_FPS_19     = 19,
    GADI_VENC_FPS_20     = 20,
    GADI_VENC_FPS_21     = 21,
    GADI_VENC_FPS_22     = 22,
    GADI_VENC_FPS_23     = 23,
    GADI_VENC_FPS_24     = 24,
    GADI_VENC_FPS_25     = 25,
    GADI_VENC_FPS_26     = 26,
    GADI_VENC_FPS_27     = 27,
    GADI_VENC_FPS_28     = 28,
    GADI_VENC_FPS_29     = 29,
    GADI_VENC_FPS_30     = 30,
    GADI_VENC_FPS_50     = 50,
    GADI_VENC_FPS_60     = 60,
    GADI_VENC_FPS_120    = 120,
    /*fps:3.125.*/
    GADI_VENC_FPS_3_125  = 3125000,
    /*fps:3.75.*/
    GADI_VENC_FPS_3_75   = 37500,
    /*fps:6.25.*/
    GADI_VENC_FPS_6_25   = 62500,
    /*fps:7.5.*/
    GADI_VENC_FPS_7_5    = 750,
    /*fps:12.5.*/
    GADI_VENC_FPS_12_5   = 1250,
    /*fps:23.976.*/
    GADI_VENC_FPS_23_976 = 23976000,
    /*fps:29.97.*/
    GADI_VENC_FPS_29_97  = 299700,
    /*fps:59.94.*/
    GADI_VENC_FPS_59_94  = 599400,
}GADI_VENC_FrameRateEnumT;

/*!
*******************************************************************************
** \brief video encode stream frame type.
*******************************************************************************
*/
typedef enum
{
    /*encode frame type: H264/H265 IDR frame*/
    GADI_VENC_IDR_FRAME  = 1,
    /*encode frame type: H264/H265 I frame*/
    GADI_VENC_I_FRAME    = 2,
    /*encode frame type: H264/H265 P frame*/
    GADI_VENC_P_FRAME    = 3,
    /*encode frame type: H264/H265 B frame*/
    GADI_VENC_B_FRAME    = 4,
    /*encode frame type: MJPEG frame*/
    GADI_VENC_JPEG_FRAME = 5,
}GADI_VENC_FrameTypeEnumT;

/*!
*******************************************************************************
** \brief video h264 stream reencode threshold enum.
*******************************************************************************
*/
typedef enum
{
    /*H264 reencode threahold:level 0,minimum*/
    GADI_VENC_REENC_THRESH_0    = 0,
    /*H264 reencode threahold:level 1*/
    GADI_VENC_REENC_THRESH_1    = 1,
    /*H264 reencode threahold:level 2*/
    GADI_VENC_REENC_THRESH_2    = 2,
    /*H264 reencode threahold:level 3*/
    GADI_VENC_REENC_THRESH_3    = 3,
    /*H264 reencode threahold:level 4*/
    GADI_VENC_REENC_THRESH_4    = 4,
    /*H264 reencode threahold:level 5*/
    GADI_VENC_REENC_THRESH_5    = 5,
    /*H264 reencode threahold:level 6*/
    GADI_VENC_REENC_THRESH_6    = 6,
    /*H264 reencode threahold:level 7, maximum*/
    GADI_VENC_REENC_THRESH_7    = 7,
    /*H264 reencode threahold number.*/
    GADI_VENC_REENC_THRESH_NUM  = 8,
}GADI_VENC_H264ReEncThreshEnumT;

/*!
*******************************************************************************
** \brief video h264 stream strength enum.
*******************************************************************************
*/
typedef enum
{
    /*H264 reencode strength:level 0, minimum*/
    GADI_VENC_REENC_STRENGTH_0   = 0,
    /*H264 reencode strength:level 1*/
    GADI_VENC_REENC_STRENGTH_1   = 1,
    /*H264 reencode strength:level 2*/
    GADI_VENC_REENC_STRENGTH_2   = 2,
    /*H264 reencode strength:level 3*/
    GADI_VENC_REENC_STRENGTH_3   = 3,
    /*H264 reencode strength:level 4 , maximum*/
    GADI_VENC_REENC_STRENGTH_4   = 4,
    /*H264 reencode strength number*/
    GADI_VENC_REENC_STRENGTH_NUM = 5,
}GADI_VENC_H264ReEncStrengthEnumT;

/*!
*******************************************************************************
** \brief video h264 stream iframe size control threshold enum.
*******************************************************************************
*/
typedef enum
{
    /*H264 I size control threahold:level 0,minimum*/
    GADI_VENC_I_SIZE_CTL_THRESH_0    = 0,
    /*H264 I size control threahold:level 1*/
    GADI_VENC_I_SIZE_CTL_THRESH_1    = 1,
    /*H264 I size control threahold:level 2*/
    GADI_VENC_I_SIZE_CTL_THRESH_2    = 2,
    /*H264 I size control threahold:level 3*/
    GADI_VENC_I_SIZE_CTL_THRESH_3    = 3,
    /*H264 I size control threahold:level 4*/
    GADI_VENC_I_SIZE_CTL_THRESH_4    = 4,
    /*H264 I size control threahold:level 5, maximum*/
    GADI_VENC_I_SIZE_CTL_THRESH_5    = 5,
    /*H264 I size control threahold number.*/
    GADI_VENC_I_SIZE_CTL_THRESH_NUM  = 6,
}GADI_VENC_H264ISizeCtlThreshEnumT;

/*!
*******************************************************************************
** \brief video h264 stream iframe size control strength enum.
*******************************************************************************
*/
typedef enum
{
    /*H264 I size control strength:level 0, minimum*/
    GADI_VENC_I_SIZE_CTL_STRENGTH_0   = 0,
    /*H264 I size control strength:level 1*/
    GADI_VENC_I_SIZE_CTL_STRENGTH_1   = 1,
    /*H264 I size control strength:level 2*/
    GADI_VENC_I_SIZE_CTL_STRENGTH_2   = 2,
    /*H264 I size control strength:level 3*/
    GADI_VENC_I_SIZE_CTL_STRENGTH_3   = 3,
    /*H264 I size control strength:level 4*/
    GADI_VENC_I_SIZE_CTL_STRENGTH_4   = 4,
    /*H264 I size control strength:level 5 , maximum*/
    GADI_VENC_I_SIZE_CTL_STRENGTH_5   = 5,
    /*H264 I size control strength number*/
    GADI_VENC_I_SIZE_CTL_STRENGTH_NUM = 6,
}GADI_VENC_H264ISizeCtlStrengthEnumT;


//*****************************************************************************
//*****************************************************************************
//** Data Structures
//*****************************************************************************
//*****************************************************************************

/*!
*******************************************************************************
** \brief H264 stream attribute struct.
*******************************************************************************
*/
typedef struct
{
    /*gop M value.*/
    GADI_U8             gopM;
    /*gop N value.*/
    GADI_U8             gopN;
    /*gop model.*/
    GADI_U8             gopModel;
    /*IDR interval .*/
    GADI_U8             idrInterval;
    /*encode profile.*/
    GADI_U8             profile;
    /*rotate: 0x01:means flip horizontal,0x02:means flip vertical.*/
    GADI_U8             flip;
    /*0: CBR; 1: VBR; 2: CBR keep quality; 3: VBR keep quality: GADI_VENC_BrcModeEnumT*/
    GADI_U8             brcMode;
    /*cbr mode, bit rate.*/
    GADI_U32            cbrAvgBps;
    /*vbr mode, min bit rate.*/
    GADI_U32            vbrMinbps;
    /*vbr mode, max bit rate.*/
    GADI_U32            vbrMaxbps;
    /*reencode mode: 0:disable(default), 1:enable.*/
    GADI_U8             reEncMode;
}GADI_VENC_StrH264AttrT;

/*!
*******************************************************************************
** \brief MJPEG stream attribute struct.
*******************************************************************************
*/
typedef struct
{
    /*0: YUV 422, 1: YUV 420.*/
    GADI_U8             chromaFormat;
    /*rotate: 0x01:means flip horizontal,0x02:means flip vertical.*/
    GADI_U8             flip;
    /*1 ~ 100, 100 is best quality.*/
    GADI_U8             quality;
    /*snapshot mode: 0:disable(default), 1:enable.*/
    GADI_U8             snapshotMode;
} GADI_VENC_StrMjpegAttrT;

/*!
*******************************************************************************
** \brief H265 stream attribute struct.
*******************************************************************************
*/
typedef struct
{
    /*gop M value.*/
    GADI_U8             gopM;
    /*gop N value.*/
    GADI_U8             gopN;
    /*gop model.*/
    GADI_U8             gopModel;
    /*IDR interval .*/
    GADI_U8             idrInterval;
    /*encode profile.*/
    GADI_U8             profile;
    /*rotate: 0x01:means flip horizontal,0x02:means flip vertical, 0x04:means rotate 90.*/
    GADI_U8             flip;
    /*0: CBR; 1: VBR; 2: CBR keep quality; 3: VBR keep quality: GADI_VENC_BrcModeEnumT*/
    GADI_U8             brcMode;
    /*cbr mode, bit rate.*/
    GADI_U32            cbrAvgBps;
    /*vbr mode, min bit rate.*/
    GADI_U32            vbrMinbps;
    /*vbr mode, max bit rate.*/
    GADI_U32            vbrMaxbps;
} GADI_VENC_StrH265AttrT;

/*!
*******************************************************************************
** \brief one stream attribute struct.
*******************************************************************************
*/
typedef struct
{
    /*stream index.*/
    GADI_U32                    streamId;
    /*indicate vpss channel, GADI_VPSS_ChannelEnumT.*/
    GADI_U8                     vpssChanId;
    /*encode stream type, 0: none, 1: H.264, 2: MJPEG, 3: H265*/
    GADI_U8                     encodeType;
    /*crop x offset of vpss channel output.*/
    GADI_U16                    xOffset;
    /*crop y offset of vpss channel output.*/
    GADI_U16                    yOffset;
    /*encode stream width.*/
    GADI_U16                    width;
    /*encode stream height.*/
    GADI_U16                    height;
    /*encode frame rate.*/
    GADI_VENC_FrameRateEnumT    fps;
    union
    {
        /*attributes of h264 stream.*/
        GADI_VENC_StrH264AttrT   stAttrH264;
        /*attributes of mjpeg stream.*/
        GADI_VENC_StrMjpegAttrT  stAttrMjpeg;
        /*attributes of h265 stream.*/
        GADI_VENC_StrH265AttrT   stAttrH265;
    };
}GADI_VENC_StrAttr;

/*!
*******************************************************************************
** \brief one h264 stream qp configuration struct. on the fly.
*******************************************************************************
*/
typedef struct
{
    /*stream index.*/
    GADI_U32            streamId;
    /*rate control factor. value:1~51, qpMinOnI <= qpMinOnP.*/
    GADI_U8             qpMinOnI;
    /*rate control factor.qpMinOnI <= qpMaxOnI*/
    GADI_U8             qpMaxOnI;
    /*rate control factor. value:1~51, qpMinOnI <= qpMinOnP.*/
    GADI_U8             qpMinOnP;
    /*rate control factor.qpMinOnP <=  qpMaxOnP*/
    GADI_U8             qpMaxOnP;
    /*rate control factor: I frame qp weight, range: 1~10,*/
    GADI_U8             qpIWeight;
    /*rate control factor. P frame qp weight, range: 1~5.*/
    GADI_U8             qpPWeight;
    /*picture quality consistency, range: 0~2. 2:will be best.*/
    GADI_U8             adaptQp;
}GADI_VENC_H264QpConfigT;

/*!
*******************************************************************************
** \brief one h265 stream qp configuration struct. on the fly.
*******************************************************************************
*/
typedef struct
{
    /*stream index.*/
    GADI_U32            streamId;
    /*rate control factor. value:1~51.*/
    GADI_U8             qpMin;
    /*rate control factor.qpMin <= qpMax*/
    GADI_U8             qpMax;
    /*picture quality consistency, range: 0~2. 2:will be best.*/
    GADI_U8             adaptQp;
    /*intra qp offset, range: -10~10.*/
    GADI_S8             intraQpOffset;
    /*intra qp offset, range: 0~10.*/
    GADI_S8             maxDeltaQp;
}GADI_VENC_H265QpConfigT;

/*!
*******************************************************************************
** \brief one h265 stream rc configuration struct. on the fly.
*******************************************************************************
*/
typedef struct
{
    /*stream index.*/
    GADI_U32            streamId;
    /*rate control, value:0~1.*/
    GADI_U8             rateControl;
    /*bit alloc mode, value:0~1.*/
    GADI_U8             bitAllocMode;
    /*cu rate control, value:0~1.*/
    GADI_U8             cuRateControl;
    /*ctu hvs qp control, value:0~1.*/
    GADI_U8             enHvsQp;
    /*ctu hvs qp  scale control, value:0~1.*/
    GADI_U8             enHvsQpScale;
    /*ctu hvs qp scale, value:0~4.*/
    GADI_U8             hvsQpScale;
}GADI_VENC_H265RcConfigT;

/*!
*******************************************************************************
** \brief one h264 stream gop N configuration struct.
*******************************************************************************
*/
typedef struct
{
    /*stream index.*/
    GADI_U32            streamId;
    /*gop N value.*/
    GADI_U8             gopN;
}GADI_VENC_GopConfigT;


/*!
*******************************************************************************
** \brief setting video encode bitrate range parameters struct.
*******************************************************************************
*/
typedef struct
{
    /*stream index for setting the bitrate.*/
    GADI_U32            streamId;
    /*0: CBR; 1: VBR; 2: CBR keep quality; 3: VBR keep quality: GADI_VENC_BrcModeEnumT*/
    GADI_U8             brcMode;
    /*cbr mode, bit rate.*/
    GADI_U32            cbrAvgBps;
    /*vbr mode, min bit rate.*/
    GADI_U32            vbrMinbps;
    /*vbr mode, max bit rate.*/
    GADI_U32            vbrMaxbps;
}GADI_VENC_BitRateRangeT;

/*!
*******************************************************************************
** \brief settting video encode frame rate parameters struct.
*******************************************************************************
*/
typedef struct
{
    /*stream index for setting the bitrate.*/
    GADI_U32                    streamId;
    /*encoding stream target frame rate value.*/
    GADI_VENC_FrameRateEnumT    fps;
}GADI_VENC_FrameRateT;

/*!
*******************************************************************************
** \brief getting VENC attr information.
*******************************************************************************
*/
typedef struct
{
    /*stream id.*/
    GADI_U32 id;
    /*stream type.*/
    GADI_VENC_TypeEnumT type;
    /*stream state.*/
    GADI_VENC_StreamStateEnumT state;
}GADI_CHN_AttrT;

/*!
*******************************************************************************
** \brief getting VENC Stream information.
*******************************************************************************
*/
typedef struct
{
    /*stream id.*/
    GADI_U32    stream_id;
    /*frame size.*/
    GADI_U32    size;
    /*frame number.*/
    GADI_U32    frame_num;
    /*frame PTS.*/
    GADI_U64    PTS;
    /*frame addr of user's address.*/
    GADI_U8     *addr;
    /*stream end flag.*/
    GADI_U32    stream_end : 1;
    /*picutre type:GADI_VENC_FrameTypeEnumT.*/
    GADI_U32    pic_type : 3;
}GADI_VENC_StreamT;

/*!
*******************************************************************************
** \brief DSP map memory informations.
*******************************************************************************
*/
typedef struct
{
    /*DSP mapping addr.*/
    GADI_U8    *addr;
    /*DSP mapping length.*/
    GADI_U32    length;
}GADI_VENC_DspMapInfoT;

/*!
*******************************************************************************
** \brief BSB map memory informations.
*******************************************************************************
*/
typedef struct
{
    /*BSB0(H264 and mjpeg) mapping addr.*/
    GADI_U8    *addr0;
    /*BSB1(H265) mapping addr.*/
    GADI_U8    *addr1;
    /*BSB0 mapping length.*/
    GADI_U32    length0;
    /*BSB1 mapping length.*/
    GADI_U32    length1;
}GADI_VENC_BsbMapInfoT;


/*!
*******************************************************************************
** \brief roi region informations.
*******************************************************************************
*/
typedef struct
{
    /*region info of the roi x offset.*/
    GADI_U32            offsetX;
    /*region info of the roi y offset.*/
    GADI_U32            offsetY;
    /*region info of the roi width.*/
    GADI_U32            width;
    /*region info of the roi height.*/
    GADI_U32            height;
}GADI_VENC_H264RoiRegionInfoT;

/*!
*******************************************************************************
** \brief roi configuration struct.
*******************************************************************************
*/
typedef struct
{
    /*stream index.*/
    GADI_U32                      streamId;
    /*index of roi, supports indexes range: 0~4.*/
    GADI_U32                      roiIndex;
    /*relative QP of I frame*/
    GADI_S8                       relativeQpI;
    /*relative QP of P frame*/
    GADI_S8                       relativeQpP;
    /*the roi enable flags*/
    GADI_U8                       roiEnable;
    /*region info of the roi.*/
    GADI_VENC_H264RoiRegionInfoT  roiRegionInfo;
}GADI_VENC_H264RoiConfigT;

/*!
*******************************************************************************
** \brief venc bias config struct.
*******************************************************************************
*/
typedef struct
{
    /*stream id.*/
    GADI_U32            streamId;
    /*RDO:intra16x16 bias,0~128 */
    GADI_U8             i16x16Cost;
    /*RDO:intra4x4 bias,0~128 */
    GADI_U8             i4x4Cost;
    /*RDO:inter16x16 bias,0~128 */
    GADI_U8             p16x16Cost;
    /*RDO:inter8x8 bias,0~128 */
    GADI_U8             p8x8Cost;
    /*RDO:skip bias,0~128 */
    GADI_U8             skipChance;
    /*RDO:uv qp offset,0~24 */
    GADI_U8             uvQpOffset;
    /*RDO:Alpha,0~12 */
    GADI_U8             dbfAlpha;
    /*RDO:Beta,0~12 */
    GADI_U8             dbfBeta;
}GADI_VENC_BiasConfigT;

/*!
*******************************************************************************
** \brief venc iConfig struct.
*******************************************************************************
*/
typedef struct
{
    /*stream index.*/
    GADI_U32            streamId;
    /*RDO:intra bias of P frame, 0~500.*/
    GADI_U16            iChance;
}GADI_VENC_IConfigT;


/*!
*******************************************************************************
** \brief h264 stream reencode config parameters struct.
*******************************************************************************
*/
typedef struct
{
    /*stream index.*/
    GADI_U32                         streamId;
    /*reencode threshold:scenes from simple to complex.*/
    GADI_VENC_H264ReEncThreshEnumT   threshStC;
    /*reencode strength:scenes from simple to complex.*/
    GADI_VENC_H264ReEncStrengthEnumT strengthStC;
    /*reencode threshold:scenes from complex to simple.*/
    GADI_VENC_H264ReEncThreshEnumT   threshCtS;
    /*reencode strength:scenes from complex to simple.*/
    GADI_VENC_H264ReEncStrengthEnumT strengthCtS;
}GADI_VENC_H264ReEncConfigT;

/*!
*******************************************************************************
** \brief h264 stream iframe size control parameters struct.
*******************************************************************************
*/
typedef struct
{
    /*stream index.*/
    GADI_U32                            streamId;
    /*iframe size control threshold.*/
    GADI_VENC_H264ISizeCtlThreshEnumT   IsizeCtlThresh;
    /*iframe size control strength.*/
    GADI_VENC_H264ISizeCtlStrengthEnumT IsizeCtlStrength;
}GADI_VENC_H264IsizeCtlT;

/*!
*******************************************************************************
** \brief h264 stream MB QP control parameters struct.
*******************************************************************************
*/
typedef struct
{
    /*stream index.*/
    GADI_U32    streamId;
    /*threshold value of MB QP control.*/
    u16         mbQpThrds[15];
    /*QP delta value of MB QP control.*/
    s8          mbQpDelta[16];
    /*enable MB QP control. 0:disable, 1:enable.*/
    u8          mbQpCtlEnalbe;
}GADI_VENC_MbQpCtlT;

/*!
*******************************************************************************
** \brief h264 stream smart video control parameters struct.
*******************************************************************************
*/
typedef struct
{
    /*stream index.*/
    GADI_U32    streamId;
    /*smart video mode: 0:disable(default), 1:enable.*/
    GADI_U8     smartVideoEnable;
    /*dynamic I frame mode: 0:disable(default), 1:enable.*/
    GADI_U8     IDynamicEnable;
    /*dynamic I frame threshold 1, if motion area precent is more than the threshold,
            change mode from resting to motion, insert a I frame. [1,100],default:25.*/
    GADI_U8     IDynamicThr1;
    /*dynamic I frame threshold 2, if motion area precent is less than the threshold,
            change mode from motion to resting, insert a I frame.[1,100],default:5.*/
    GADI_U8     IDynamicThr2;
    /*dynamic I frame mode change limit, count of resting frame .
            [1,GOP],default:5.*/
    GADI_U8     IDynamicStableThr;
    /* minimum interval to the last I frame. [1,GOP],default:15.*/
    GADI_U8     IDynamicIntervalThr;
    /* QP delta of dynamic I frame. [1,51],default:0.*/
    GADI_U8     IDynamicQpDelta;
    /* large P frame mode: 0:disable(default), 1:enable.*/
    GADI_U8     largePEnable;
    /* large P frame interval. [1,GOP],default:0.*/
    GADI_U8     largePInterval;
    /* large P frame QP delta. [1,51],default:0.*/
    GADI_U8     largePQpDelta;
    /* background and foreground QP minimum delta. [1,51],default:0.*/
    GADI_U8     backForeQpDelta;
    /* max bit rate of smart video . [1,10],default:8(80%).*/
    GADI_U8     bitrateLevel;
    /* motion level step . [0,51],default:5(H264) 2(H265).*/
    GADI_U8     motionLevelStep;
    /* motion QP step . [0,51],default:1.*/
    GADI_U8     motionQpStep;
    /* motion level div . [0,100],default:5(H264) 6(H265).*/
    GADI_U8     motionLevelDiv;
    /* mini I frame QP . [0,51],default:25.*/
    GADI_U8     minIQp;
    /* mini P frame QP . [0,51],default:25.*/
    GADI_U8     minPQp;
    /* debug mode,all frame is I frame. 0:disable(default), 1:enable.*/
    GADI_U8     confirmEnable;
    /* debug mode: 0:disable(default), 1:enable.*/
    GADI_U8     debugMode;
    /* GOP  interval. [0,51],default:25.*/
    GADI_U8     gopInterval;
    /* mini I/P frame QP . [0,51],default:10.*/
    GADI_U8     minIpQp;
    /* H265 change mode from motion to resting, delay frame count for reduce bit rate.
            [0,15],default:3.*/
    GADI_U8     motion2StaticThr;
    /* motion area size(H264 16x16/H265 64x64 ). [0,8160],default:15(H264) 5(H265).*/
    GADI_U16    contourSizeThr;
}GADI_VENC_SmartVideoT;

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
GADI_ERR gadi_venc_init(void);

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
GADI_ERR gadi_venc_exit(void);

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
GADI_SYS_HandleT gadi_venc_open(GADI_ERR* errorCodePtr);

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
GADI_ERR gadi_venc_close(GADI_SYS_HandleT handle);


/*!
*******************************************************************************
** \brief map dsp memory.
**
** \param[in]  venHandle   Valid ADI venc instance handle previously opened by
**                          #gadi_venc_open.
** \param[out]  info       pointer of dsp map memory struct;
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

GADI_ERR gadi_venc_map_dsp(GADI_SYS_HandleT handle, GADI_VENC_DspMapInfoT *info);


/*!
*******************************************************************************
** \brief map bsb.
**
** \param[in]  venHandle     Valid ADI venc instance handle previously opened by
**                          #gadi_venc_open.
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
GADI_ERR gadi_venc_map_bsb(GADI_SYS_HandleT handle);

/*!
*******************************************************************************
** \brief get bsb map info.
**
** \param[in]  venHandle     Valid ADI venc instance handle previously opened by
**                          #gadi_venc_open.
** \param[out]  info         pointer of bsb map memory struct;
** \return
** - #GADI_OK
** - #GADI_VENC_ERR_OPERATION_NOT_PERMIT
** - #GADI_VENC_ERR_BAD_PARAMETER
**
** \sa gadi_venc_open
**
*******************************************************************************
*/
GADI_ERR gadi_venc_get_bsb_map_info(GADI_SYS_HandleT handle,
                                                GADI_VENC_BsbMapInfoT *info);

/*!
*******************************************************************************
** \brief get  encode stream attr.
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
GADI_ERR gadi_venc_get_stream_attr(GADI_SYS_HandleT handle,
    GADI_VENC_StrAttr *strAttr);

/*!
*******************************************************************************
** \brief set  encode stream attr.
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
GADI_ERR gadi_venc_set_stream_attr(GADI_SYS_HandleT handle,
    GADI_VENC_StrAttr *strAttr);

/*!
*******************************************************************************
** \brief get one stream frame rate.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]  frPar pointer of setting frame rate struct;
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
GADI_ERR gadi_venc_get_framerate(GADI_SYS_HandleT handle, GADI_VENC_FrameRateT* frPar);

/*!
*******************************************************************************
** \brief set one stream frame rate.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]  frPar pointer of setting frame rate struct;
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
GADI_ERR gadi_venc_set_framerate(GADI_SYS_HandleT handle, GADI_VENC_FrameRateT* frPar);

/*!
*******************************************************************************
** \brief get one stream(h264/h265) bit rate.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[out]  brPar pointer of setting bitrate struct;
**
** \return
** - #GADI_OK
** - #GADI_VENC_ERR_FROM_DRIVER
** - #GADI_VENC_ERR_BAD_PARAMETER
**
** \sa gadi_venc_set_bitrate
**
*******************************************************************************
*/
GADI_ERR gadi_venc_get_bitrate(GADI_SYS_HandleT handle,
    GADI_VENC_BitRateRangeT *brPar);
/*!
*******************************************************************************
** \brief set one stream(h264/h265) bit rate.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]  brPar pointer of setting bitrate struct;
**
** \return
** - #GADI_OK
** - #GADI_VENC_ERR_FROM_DRIVER
** - #GADI_VENC_ERR_BAD_PARAMETER
**
** \sa gadi_venc_get_bitrate
**
*******************************************************************************
*/
GADI_ERR gadi_venc_set_bitrate(GADI_SYS_HandleT handle,
    GADI_VENC_BitRateRangeT* brPar);

/*!
*******************************************************************************
** \brief           get H264 gop value.
**
** \param[in]   handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]   GADI_VENC_H264GopNConfigT.streamId.
**
** \param[ou]   GADI_VENC_H264GopNConfigT.gopN.
**
** \return
** - #GADI_OK
**
** \sa gadi_venc_get_gop
**
*******************************************************************************
*/
GADI_ERR gadi_venc_get_gop(GADI_SYS_HandleT handle,
        GADI_VENC_GopConfigT* configPar);

/*!
*******************************************************************************
** \brief           set H264 gop value.
**
** \param[in]   handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]   GADI_VENC_H264GopNConfigT.
**
** \return
** - #GADI_OK
**
** \sa gadi_venc_set_gop
**
*******************************************************************************
*/

GADI_ERR gadi_venc_set_gop(GADI_SYS_HandleT handle,
        GADI_VENC_GopConfigT* configPar);

/*!
*******************************************************************************
** \brief force create IDR frame, only h264/h265.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]  streamId stream id enum:GADI_VENC_StreamEnumT;
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
GADI_ERR gadi_venc_force_idr(GADI_SYS_HandleT handle, GADI_U32 streamId);

/*!
*******************************************************************************
** \brief start encoding one stream.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]  streamId stream id enum:GADI_VENC_StreamEnumT;
**
** \return
** - #GADI_OK
** - #GADI_VENC_ERR_FROM_DRIVER
** - #GADI_VENC_ERR_BAD_PARAMETER
**
** \sa gadi_venc_stop_stream
**
*******************************************************************************
*/
GADI_ERR gadi_venc_start_stream(GADI_SYS_HandleT handle, GADI_U32 streamId);

/*!
*******************************************************************************
** \brief stop encoding one stream.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]  streamId stream id enum:GADI_VENC_StreamEnumT;
**
** \return
** - #GADI_OK
** - #GADI_VENC_ERR_FROM_DRIVER
** - #GADI_VENC_ERR_BAD_PARAMETER
**
** \sa gadi_venc_start_stream
**
*******************************************************************************
*/
GADI_ERR gadi_venc_stop_stream(GADI_SYS_HandleT handle, GADI_U32 streamId);

/*!
*******************************************************************************
** \brief query status of parameter.
**
** \param[in]   handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]   streamId.
** \param[out]  chn_attr pointer of GADI_CHN_AttrT parameters struct;
**
** \return
** - #GADI_OK
**
** \sa gadi_venc_open
**
*******************************************************************************
*/
GADI_ERR gadi_venc_query(GADI_SYS_HandleT handle, GADI_S32 streamId,
GADI_CHN_AttrT * chn_attr);

/*!
*******************************************************************************
** \brief get stream of current.
**
** \param[in]   handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]   streamId.
** \param[out]  stream    pointer of GADI_VENC_StreamT parameters struct;
**
** \return
** - #GADI_OK
**
** \sa gadi_venc_open
**
*******************************************************************************
*/
GADI_ERR gadi_venc_get_stream(GADI_SYS_HandleT handle, GADI_S32 streamId,
    GADI_VENC_StreamT * stream);

/*!
*******************************************************************************
** \brief check encoder state, if encoder state is iavalid, this API will return
**        GADI_VENC_ERR_FROM_DRIVER, and print state in screen.
**
** \param[in]   handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]   streamId.
**
** \return
** - #GADI_VENC_ERR_FROM_DRIVER
** - #GADI_OK
**
** \sa gadi_venc_open
**
*******************************************************************************
*/
GADI_ERR gadi_venc_check_encoder_state(GADI_SYS_HandleT handle);

/*!
*******************************************************************************
** \brief print out all parameters.
**
** \param[in]   handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
**
** \return
** - #GADI_OK
**
** \sa gadi_venc_open
**
*******************************************************************************
*/
GADI_ERR gadi_venc_print_params(GADI_SYS_HandleT handle);

/*!
*******************************************************************************
** \brief get one h264 encode stream qp configuration.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[out]  configPar pointer of h264 stream qp configuration struct;
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
GADI_ERR gadi_venc_get_h264_qp_config(GADI_SYS_HandleT handle,
                                                GADI_VENC_H264QpConfigT* configPar);


/*!
*******************************************************************************
** \brief set one h264 encode stream qp configuration.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]  configPar pointer of h264 stream qp configuration struct;
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
GADI_ERR gadi_venc_set_h264_qp_config(GADI_SYS_HandleT handle,
                                               GADI_VENC_H264QpConfigT* configPar);

/*!
*******************************************************************************
** \brief           get H264 enc value.
**
** \param[in]   handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]   GADI_VENC_EncConfigT.streamId.
**
** \param[ou]   GADI_VENC_EncConfigT.
**
** \return
** - #GADI_OK
**
** \sa gadi_venc_get_h264_enc
**
*******************************************************************************
*/
GADI_ERR gadi_venc_get_h264_iconfig(GADI_SYS_HandleT handle,
        GADI_VENC_IConfigT * configPar);

/*!
*******************************************************************************
** \brief           set H264 enc value.
**
** \param[in]   handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]   GADI_VENC_EncConfigT.
**
** \return
** - #GADI_OK
**
** \sa gadi_venc_set_h264_enc
**
*******************************************************************************
*/
GADI_ERR gadi_venc_set_h264_iconfig(GADI_SYS_HandleT handle,
        GADI_VENC_IConfigT * configPar);

/*!
*******************************************************************************
** \brief           get H264 bias value.
**
** \param[in]   handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]   GADI_VENC_BiasConfigT.streamId.
**
** \param[ou]   GADI_VENC_BiasConfigT.
**
** \return
** - #GADI_OK
**
** \sa gadi_venc_get_h264_bias
**
*******************************************************************************
*/
GADI_ERR gadi_venc_get_h264_bias(GADI_SYS_HandleT handle,
        GADI_VENC_BiasConfigT * configPar);

/*!
*******************************************************************************
** \brief           set H264 bias value.
**
** \param[in]   handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]   GADI_VENC_BiasConfigT.
**
** \return
** - #GADI_OK
**
** \sa gadi_venc_set_h264_bias
**
*******************************************************************************
*/
GADI_ERR gadi_venc_set_h264_bias(GADI_SYS_HandleT handle,
        GADI_VENC_BiasConfigT * configPar);


/*!
*******************************************************************************
** \brief get one h264 encode stream re-encode configuration.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[out]  reEncConfig pointer of h264 stream qp configuration struct;
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
GADI_ERR gadi_venc_get_h264_reenc(GADI_SYS_HandleT handle,
    GADI_VENC_H264ReEncConfigT * reEncConfig);

/*!
*******************************************************************************
** \brief set one h264 encode stream re-encode configuration.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]  reEncConfig pointer of h264 stream qp configuration struct;
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
GADI_ERR gadi_venc_set_h264_reenc(GADI_SYS_HandleT handle,
    GADI_VENC_H264ReEncConfigT * reEncConfig);

/*!
*******************************************************************************
** \brief get one h264 encode stream I-frame control parameters.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[out]  reEncConfig pointer of h264 stream I-frame control struct;
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
GADI_ERR gadi_venc_get_h264_iframe_size(GADI_SYS_HandleT handle,
    GADI_VENC_H264IsizeCtlT * IsizeCtl);

/*!
*******************************************************************************
** \brief set one h264 encode stream I-frame control parameters.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]  reEncConfig pointer of h264 stream I-frame control struct;
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
GADI_ERR gadi_venc_set_h264_iframe_size(GADI_SYS_HandleT handle,
    GADI_VENC_H264IsizeCtlT * IsizeCtl);

/*!
*******************************************************************************
** \brief   Gets h264 stream MB QP control parameters.
**
** \param[in]   handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]   mbQpCtlPara  pointer of MB QP control parameters struct;
**
** \return
** - #GADI_OK
**
** \sa gadi_venc_set_h264_mb_qp_ctl
**
*******************************************************************************
*/
GADI_ERR gadi_venc_get_h264_mb_qp_ctl(GADI_SYS_HandleT handle,
    GADI_VENC_MbQpCtlT *mbQpCtlPara);

/*!
*******************************************************************************
** \brief   Sets h264 stream MB QP control parameters.
**
** \param[in]   handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]   mbQpCtlPara  pointer of MB QP control parameters struct;
**
** \return
** - #GADI_OK
**
** \sa gadi_venc_get_h264_mb_qp_ctl
**
*******************************************************************************
*/
GADI_ERR gadi_venc_set_h264_mb_qp_ctl(GADI_SYS_HandleT handle,
    GADI_VENC_MbQpCtlT *mbQpCtlPara);

/*!
*******************************************************************************
** \brief   Sets h264 or h265 stream smart video parameters.
**
** \param[in]   handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]   smartVideo  pointer of smart video control parameters struct;
**
** \return
** - #GADI_OK
**
** \sa gadi_venc_set_h26x_smart_video
**
*******************************************************************************
*/
GADI_ERR gadi_venc_get_h26x_smart_video(GADI_SYS_HandleT handle,
    GADI_VENC_SmartVideoT * smartVideo);


/*!
*******************************************************************************
** \brief   Gets  h264 or h265 stream smart video parameters.
**
** \param[in]   handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]   smartVideo  pointer of smart video control parameters struct;
**
** \return
** - #GADI_OK
**
** \sa gadi_venc_get_h26x_smart_video
**
*******************************************************************************
*/
GADI_ERR gadi_venc_set_h26x_smart_video(GADI_SYS_HandleT handle,
    GADI_VENC_SmartVideoT * smartVideo);

/*!
*******************************************************************************
** \brief Gets a ROI config of a h264 stream.
**
** \param[in]   handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]   roiIndex     ROI index
** \param[ou]   configPar    pointer of a roi configuration struct;
**
** \return
** - #GADI_OK
**
** \sa gadi_venc_set_qp_roi_config
**
*******************************************************************************
*/
GADI_ERR gadi_venc_get_h264_roi_config(GADI_SYS_HandleT handle,
    GADI_U32 roiIndex, GADI_VENC_H264RoiConfigT *configPar);

/*!
*******************************************************************************
** \brief Sets a ROI config of a h264 stream.
**
** \param[in]   handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]   configPar  pointer of a roi configuration struct;
**
** \return
** - #GADI_OK
**
** \sa gadi_venc_get_qp_roi_config
**
*******************************************************************************
*/
GADI_ERR gadi_venc_set_h264_roi_config(GADI_SYS_HandleT handle,
    GADI_VENC_H264RoiConfigT *configPar);


/*!
*******************************************************************************
** \brief Gets h265 stream qp configuration.
**
** \param[in]   handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[out]   configPar  pointer of h265 qp configuration struct.
**
** \return
** - #GADI_OK
**
** \sa gadi_venc_get_qp_roi_config
**
*******************************************************************************
*/
GADI_ERR gadi_venc_get_h265_qp_config(GADI_SYS_HandleT handle,
    GADI_VENC_H265QpConfigT * configPar);

/*!
*******************************************************************************
** \brief Sets h265 stream qp configuration.
**
** \param[in]   handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]   configPar  pointer of h265 qp configuration struct.
**
** \return
** - #GADI_OK
**
** \sa gadi_venc_get_qp_roi_config
**
*******************************************************************************
*/
GADI_ERR gadi_venc_set_h265_qp_config(GADI_SYS_HandleT handle,
    GADI_VENC_H265QpConfigT * configPar);

/*!
*******************************************************************************
** \brief reset video encoder dsp.
**
** \param[in]   handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.**
** \return
** - #GADI_OK
**
** \sa gadi_venc_open
**
*******************************************************************************
*/
GADI_ERR gadi_venc_reset_dsp(GADI_SYS_HandleT handle);

/*!
*******************************************************************************
** \brief Gets h265 stream rate control configuration.
**
** \param[in]   handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]   configPar  pointer of h265 rate control configuration struct.
**
** \return
** - #GADI_OK
**
** \sa gadi_venc_set_h265_rc_config
**
*******************************************************************************
*/
GADI_ERR gadi_venc_get_h265_rc_config(GADI_SYS_HandleT handle,
    GADI_VENC_H265RcConfigT * configPar);

/*!
*******************************************************************************
** \brief Sets h265 stream rate control configuration.
**
** \param[in]   handle Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]   configPar  pointer of h265 rate control configuration struct.
**
** \return
** - #GADI_OK
**
** \sa gadi_venc_get_h265_rc_config
**
*******************************************************************************
*/
GADI_ERR gadi_venc_set_h265_rc_config(GADI_SYS_HandleT handle,
    GADI_VENC_H265RcConfigT * configPar);


#ifdef __cplusplus
}
#endif



#endif /* _ADI_VENC_H_ */

