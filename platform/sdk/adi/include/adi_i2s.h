/*!
*****************************************************************************
** \file          adi/include/adi_i2s.h
**
** \brief         ADI i2s module header file.
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**                 ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**                 OMMISSIONS
**
** (C) Copyright 2015-2019 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _ADI_I2S_H_
#define _ADI_I2S_H_


//*****************************************************************************
//*****************************************************************************
//** Defines and Macros
//*****************************************************************************
//*****************************************************************************

/*
**************************************************************************
** Defines for general error codes of the module.
**************************************************************************
*/
/*! Bad parameter passed. */
#define GADI_I2S_ERR_BAD_PARAMETER                                          \
                              (GADI_I2S_MODULE_BASE + GADI_ERR_BAD_PARAMETER)
/*! Memory allocation failed. */
#define GADI_I2S_ERR_OUT_OF_MEMORY                                          \
                              (GADI_I2S_MODULE_BASE + GADI_ERR_OUT_OF_MEMORY)
/*! Device already initialised. */
#define GADI_I2S_ERR_ALREADY_INITIALIZED                                    \
                              (GADI_I2S_MODULE_BASE + GADI_ERR_ALREADY_INITIALIZED)
/*! Device not initialised. */
#define GADI_I2S_ERR_NOT_INITIALIZED                                        \
                              (GADI_I2S_MODULE_BASE + GADI_ERR_NOT_INITIALIZED)
/*! Feature or function is not available. */
#define GADI_I2S_ERR_FEATURE_NOT_SUPPORTED                                  \
                              (GADI_I2S_MODULE_BASE + GADI_ERR_FEATURE_NOT_SUPPORTED)
/*! Timeout occured. */
#define GADI_I2S_ERR_TIMEOUT                                                \
                              (GADI_I2S_MODULE_BASE + GADI_ERR_TIMEOUT)
/*! The device is busy, try again later. */
#define GADI_I2S_ERR_DEVICE_BUSY                                            \
                              (GADI_I2S_MODULE_BASE + GADI_ERR_DEVICE_BUSY)
/*! Invalid handle was passed. */
#define GADI_I2S_ERR_INVALID_HANDLE                                         \
                              (GADI_I2S_MODULE_BASE + GADI_ERR_INVALID_HANDLE)
/*! Semaphore could not be created. */
#define GADI_I2S_ERR_SEMAPHORE_CREATE                                       \
                              (GADI_I2S_MODULE_BASE + GADI_ERR_SEMAPHORE_CREATE)
/*! The driver's used version is not supported. */
#define GADI_I2S_ERR_UNSUPPORTED_VERSION                                    \
                              (GADI_I2S_MODULE_BASE + GADI_ERR_UNSUPPORTED_VERSION)
/*! The driver's used version is not supported. */
#define GADI_I2S_ERR_FROM_DRIVER                                            \
                              (GADI_I2S_MODULE_BASE + GADI_ERR_FROM_DRIVER)
/*! The device/handle is not open.. */
#define GADI_I2S_ERR_NOT_OPEN                                               \
                              (GADI_I2S_MODULE_BASE + GADI_ERR_NOT_OPEN)
/*! operation not permit.*/
#define GADI_I2S_ERR_OPERATION_NOT_PERMIT                                            \
                            (GADI_I2S_MODULE_BASE + GADI_ERR_OPERATION_NOT_PERMIT)
//*****************************************************************************
//*****************************************************************************
//** Enumerated types
//*****************************************************************************
//*****************************************************************************
/*!
*******************************************************************************
** \brief I2S mode enum.
*******************************************************************************
*/
typedef enum {
    /*! left mode.*/
    GADI_I2S_MODE_LEFT  = 0,  
    /*! right mode.*/
    GADI_I2S_MODE_RIGHT = 1, 
    /*! msb mode.*/
    GADI_I2S_MODE_MSB   = 2, 
    /*! i2s mode.*/
    GADI_I2S_MODE_I2S   = 4,  
    /*! dsp mode.*/
    GADI_I2S_MODE_DSP  = 6, 
}GADI_I2S_ModeEnumT;

/*!
*******************************************************************************
** \brief I2S work mode enum.
*******************************************************************************
*/
typedef enum {
    /*! slave mode.*/
    GADI_I2S_WORK_MODE_SLAVE  = 0,
    /*! master mode.*/
    GADI_I2S_WORK_MODE_MASTER,    
}GADI_I2S_WorkModeEnumT;

/*!
*******************************************************************************
** \brief I2S channel count.
*******************************************************************************
*/
typedef enum {
    /*! I2S  2 channel. */
    GADI_I2S_CHANNEL_COUNT_2 = 0,     
    /*! I2S  4 channel. */
    GADI_I2S_CHANNEL_COUNT_4,         
}GADI_I2S_ChannelCountEnumT;

/*!
*******************************************************************************
** \brief I2S sample rate.
*******************************************************************************
*/
typedef enum {
    /*!i2s speed: 8000.*/
    GADI_I2S_SPEED_8000 = 8000,
    /*!i2s speed: 16000.*/
    GADI_I2S_SPEED_16000 = 16000,
    /*!i2s speed: 24000.*/
	GADI_I2S_SPEED_24000 = 24000,
	/*!i2s speed: 32000.*/
    GADI_I2S_SPEED_32000 = 32000,
    /*!i2s speed: 44100.*/
    GADI_I2S_SPEED_44100 = 44100,
    /*!i2s speed: 48000.*/
    GADI_I2S_SPEED_48000 = 48000,
}GADI_I2S_SpeedEnumT;

/*!
*******************************************************************************
** \brief I2S sample precision.
*******************************************************************************
*/
typedef enum {
    /*!i2s word length: 16bit.*/
    GADI_I2S_WORD_LENGTH_16BIT = 16, 
    /*!i2s word length: 24bit.*/
    GADI_I2S_WORD_LENGTH_24BIT = 24,
}GADI_I2S_WordLengthEnumT;
/*!
*******************************************************************************
** \brief I2S volume level.
*******************************************************************************
*/
typedef enum
{
    /*! i2s volume level: 0.*/
    I2S_VLEVEL_0 = 0,
    /*! i2s volume level: 1.*/
    I2S_VLEVEL_1,
    /*! i2s volume level: 2.*/
    I2S_VLEVEL_2,
    /*! i2s volume level: 3.*/
    I2S_VLEVEL_3,
    /*! i2s volume level: 4.*/
    I2S_VLEVEL_4,
    /*! i2s volume level: 5.*/
    I2S_VLEVEL_5,
    /*! i2s volume level: 6.*/
    I2S_VLEVEL_6,
    /*! i2s volume level: 7.*/
    I2S_VLEVEL_7,
    /*! i2s volume level: 8.*/
    I2S_VLEVEL_8,
    /*! i2s volume level: 9.*/
    I2S_VLEVEL_9,
    /*! i2s volume level: 10.*/
    I2S_VLEVEL_10,
    /*! i2s volume level: 11.*/
    I2S_VLEVEL_11,
    /*! i2s volume level: 12.*/
    I2S_VLEVEL_12,
    /*! i2s volume level: 13.*/
    I2S_VLEVEL_13,
    /*! i2s volume level: 14.*/
    I2S_VLEVEL_14,
    /*! i2s volume level: 15.*/
    I2S_VLEVEL_15,
    /*! i2s volume level: 16.*/
    I2S_VLEVEL_16,
}GADI_I2S_VolumeLevelEnumT;

//*****************************************************************************
//*****************************************************************************
//** Data Structures
//*****************************************************************************
//*****************************************************************************
typedef struct
{    
    /*! I2S work mode.*/
    GADI_I2S_WorkModeEnumT    workMode; 
    /*! I2S channel count.*/
    GADI_I2S_ChannelCountEnumT    channel;
    /*! I2S mode count.*/
    GADI_I2S_ModeEnumT    i2sMode;
    /*! I2S sample rate.*/
    GADI_I2S_SpeedEnumT    speed;
    /*! I2S sample precision.*/
    GADI_I2S_WordLengthEnumT    wlen;
}GADI_I2S_devAttrT;

typedef struct
{
    /*! i2s frame size.*/
    GADI_S32    frameSize;
    /*! i2s frame buffer number.*/
    GADI_S32    frameNum;
}GADI_I2S_FrameChnParamT;

typedef struct
{
    /*! i2s frame virtual address.*/
    GADI_CHAR *data_addr;
    /*! i2s frame length*/
    GADI_U32 data_length;         
}GADI_I2S_FrameT;

/*!
*******************************************************************************
** \brief frame buffer status of the i2s tx channel.
*******************************************************************************
*/
typedef struct 
{
    /*! frame channel parameter. */
    GADI_I2S_FrameChnParamT frameChnParam;
    /*! used frame buffers count. */
    GADI_U32 frameCnt;
} GADI_I2S_ChnStateT;
           
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************

#ifdef __cplusplus
extern "C" {
#endif

/*!
*******************************************************************************
** \brief Initialize the ADI I2S module.
**
** \return
** - #GADI_OK
** - #GADI_I2S_ERR_ALREADY_INITIALIZED
** - #GADI_I2S_ERR_OUT_OF_MEMORY
** - #GADI_I2S_ERR_FROM_DRIVER
**
** \sa gadi_i2s_exit
**
*******************************************************************************
*/
GADI_ERR gadi_i2s_init();

/*!
*******************************************************************************
** \brief Shutdown the ADI I2S module.
**
** \return
** - #GADI_OK
** - #GADI_I2S_ERR_NOT_INITIALIZED
**
** \sa gadi_i2s_init
**
*******************************************************************************
*/
GADI_ERR gadi_i2s_exit();

/*!
*******************************************************************************
** \brief Sets up attributes for I2S device.
**
** \param[in] pstI2sAttr Pointer to the I2S device attributes.
**
**
** \return
** - #GADI_OK
** - #GADI_I2S_ERR_FROM_DRIVER
**
** \sa gadi_i2s_get_attr
**
*******************************************************************************
*/
GADI_ERR gadi_i2s_set_attr(GADI_I2S_devAttrT *pstI2sAttr);

/*!
*******************************************************************************
** \brief Gets attributes of I2S device.
**
** \param[in]  pstI2sAttr Pointer to the I2S device attributes.
**
**
** \return
** - #GADI_OK
** - #GADI_I2S_ERR_FROM_DRIVER
**
** \sa gadi_i2s_set_attr
**
*******************************************************************************
*/
GADI_ERR gadi_i2s_get_attr(GADI_I2S_devAttrT *pstI2sAttr); 

/*!
*******************************************************************************
** \brief set i2s channel parameters.
**
** \param[in]  fd   file descriptor of i2s device.
** \param[in]  pstFrameChnParam  pointer of i2s channel parameters struct;
**
** \return
** - #GADI_I2S_ERR_BAD_PARAMETER
** - #GADI_I2S_ERR_OPERATION_NOT_PERMIT
** - #GADI_I2S_ERR_FROM_DRIVER
**
** \sa gadi_i2s_dev_get_frame_channel_param
**
*******************************************************************************
*/
GADI_ERR gadi_i2s_dev_set_frame_channel_param(GADI_S32 fd, 
                                GADI_I2S_FrameChnParamT *pstFrameChnParam);

/*!
*******************************************************************************
** \brief get i2s channel parameters.
**
** \param[in]  fd   file descriptor of i2s device.
** \param[out]  pstFrameChnParam  pointer of i2s channel parameters struct;
**
** \return
** - #GADI_I2S_ERR_BAD_PARAMETER
** - #GADI_I2S_ERR_FROM_DRIVER
**
** \sa gadi_i2s_dev_set_frame_channel_param
**
*******************************************************************************
*/
GADI_ERR gadi_i2s_dev_get_frame_channel_param(GADI_S32 fd, 
                                GADI_I2S_FrameChnParamT *pstFrameChnParam);

/*!
*******************************************************************************
** \brief Enables I2S device.
**
** \param[in]  fd   file descriptor of i2s device.
**
** \return
** - #GADI_OK
** - #GADI_I2S_ERR_BAD_PARAMETER
** - #GADI_I2S_ERR_FROM_DRIVER
**
** \sa gadi_i2s_dev_disable
**
*******************************************************************************
*/
GADI_ERR gadi_i2s_dev_enable(GADI_S32 fd);

/*!
*******************************************************************************
** \brief Disables I2S device.
**
** \param[in]  fd   file descriptor of i2s device.
**
** \return
** - #GADI_OK
** - #GADI_I2S_ERR_BAD_PARAMETER
** - #GADI_I2S_ERR_FROM_DRIVER
**
** \sa gadi_i2s_dev_enable
**
*******************************************************************************
*/
GADI_ERR gadi_i2s_dev_disable(GADI_S32 fd);

/*!
*******************************************************************************
** \brief Receives i2s pcm frame.
**
** \param[out]  pstFrame Pointer to i2s pcm frame.
** \param[in]  block block/non-block flags.
**
** \return
** - #GADI_OK
** - #GADI_I2S_ERR_FROM_DRIVER
**
** \sa gadi_i2s_send_frame
**
*******************************************************************************
*/
GADI_ERR gadi_i2s_get_frame(GADI_I2S_FrameT *pstFrame, GADI_BOOL block);

/*!
*******************************************************************************
** \brief Transmits i2s pcm frame.
**
** \param[in]  pstFrame Pointer to i2s pcm frame.
** \param[in]  block block/non-block flags.
**
** \return
** - #GADI_OK
** - #GADI_I2S_ERR_FROM_DRIVER
**
** \sa gadi_i2s_get_frame
**
*******************************************************************************
*/
GADI_ERR gadi_i2s_send_frame(GADI_I2S_FrameT *pstFrame, GADI_BOOL block);

/*!
*******************************************************************************
** \brief Gets fd of i2s rx device.
**
**
** \return
** - #Valid fd value
** - #GADI_I2S_ERR_NOT_INITIALIZED
**
** \sa
**
*******************************************************************************
*/
GADI_S32 gadi_i2s_rx_get_fd(void);

/*!
*******************************************************************************
** \brief Gets fd of tx rx device.
**
**
** \return
** - #Valid fd value
** - #GADI_I2S_ERR_NOT_INITIALIZED
**
** \sa
**
*******************************************************************************
*/
GADI_S32 gadi_i2s_tx_get_fd(void);


/*!
*******************************************************************************
** \brief Sets I2S tx volume.
**
** \param[in] level  pointer to volume level.
**
**
** \return
** - #GADI_OK
** - #GADI_I2S_ERR_FROM_DRIVER
**
** \sa gadi_i2s_tx_get_volume
**
*******************************************************************************
*/
GADI_ERR gadi_i2s_tx_set_volume(GADI_I2S_VolumeLevelEnumT * ptrvLevel);

/*!
*******************************************************************************
** \brief Gets I2S tx volume.
**
**
** \param[in] level  pointer to volume level.
**
** \return
** - #GADI_OK
** - #GADI_I2S_ERR_FROM_DRIVER
**
** \sa gadi_i2s_tx_set_volume
**
*******************************************************************************
*/
GADI_ERR gadi_i2s_tx_get_volume(GADI_I2S_VolumeLevelEnumT * ptrvLevel);

/*!
*******************************************************************************
** \brief Sets I2S rx volume.
**
** \param[in] level  pointer to volume level.
**
**
** \return
** - #GADI_OK
** - #GADI_I2S_ERR_FROM_DRIVER
**
** \sa gadi_i2s_rx_get_volume
**
*******************************************************************************
*/
GADI_ERR gadi_i2s_rx_set_volume(GADI_I2S_VolumeLevelEnumT * ptrvLevel);

/*!
*******************************************************************************
** \brief Gets I2S rx volume.
**
**
** \param[in] level  pointer to volume level.
**
** \return
** - #GADI_OK
** - #GADI_I2S_ERR_FROM_DRIVER
**
** \sa gadi_i2s_rx_set_volume
**
*******************************************************************************
*/
GADI_ERR gadi_i2s_rx_get_volume(GADI_I2S_VolumeLevelEnumT * ptrvLevel);


/*!
*******************************************************************************
** \brief Query the frame buffer status of the i2s tx channel.
**
**
** \return
** - #GADI_OK
** - #GADI_I2S_ERR_FROM_DRIVER
**
** \sa 
**
*******************************************************************************
*/
GADI_ERR gadi_i2s_tx_query(GADI_I2S_ChnStateT *ptrTXChnState);

#ifdef __cplusplus
    }
#endif
#endif /* _ADI_I2S_H_ */
