/*!
*****************************************************************************
** \file        /include/adi_pda.h
**
** \brief       Picture data analysis by luma buffer
**              It analysis luma variation and return SAD value of each micro block.
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2015 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#ifndef ADI_PDA_H
#define ADI_PDA_H

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
#define GADI_PDA_ERR_BAD_PARAMETER                                          \
                              (GADI_PDA_MODULE_BASE + GADI_ERR_BAD_PARAMETER)
/*! Memory allocation failed. */
#define GADI_PDA_ERR_OUT_OF_MEMORY                                          \
                              (GADI_PDA_MODULE_BASE + GADI_ERR_OUT_OF_MEMORY)
/*! Device already initialised. */
#define GADI_PDA_ERR_ALREADY_INITIALIZED                                    \
                              (GADI_PDA_MODULE_BASE + GADI_ERR_ALREADY_INITIALIZED)
/*! Device not initialised. */
#define GADI_PDA_ERR_NOT_INITIALIZED                                        \
                              (GADI_PDA_MODULE_BASE + GADI_ERR_NOT_INITIALIZED)
/*! Feature or function is not available. */
#define GADI_PDA_ERR_FEATURE_NOT_SUPPORTED                                  \
                              (GADI_PDA_MODULE_BASE + GADI_ERR_FEATURE_NOT_SUPPORTED)
/*! Timeout occured. */
#define GADI_PDA_ERR_TIMEOUT                                                \
                              (GADI_PDA_MODULE_BASE + GADI_ERR_TIMEOUT)
/*! The device is busy, try again later. */
#define GADI_PDA_ERR_DEVICE_BUSY                                            \
                              (GADI_PDA_MODULE_BASE + GADI_ERR_DEVICE_BUSY)
/*! Invalid handle was passed. */
#define GADI_PDA_ERR_INVALID_HANDLE                                         \
                              (GADI_PDA_MODULE_BASE + GADI_ERR_INVALID_HANDLE)
/*! Semaphore could not be created. */
#define GADI_PDA_ERR_SEMAPHORE_CREATE                                       \
                              (GADI_PDA_MODULE_BASE + GADI_ERR_SEMAPHORE_CREATE)
/*! The driver's used version is not supported. */
#define GADI_PDA_ERR_UNSUPPORTED_VERSION                                    \
                              (GADI_PDA_MODULE_BASE + GADI_ERR_UNSUPPORTED_VERSION)
/*! The driver's used version is not supported. */
#define GADI_PDA_ERR_FROM_DRIVER                                            \
                              (GADI_PDA_MODULE_BASE + GADI_ERR_FROM_DRIVER)
/*! The device/handle is not open.. */
#define GADI_PDA_ERR_NOT_OPEN                                               \
                              (GADI_PDA_MODULE_BASE + GADI_ERR_NOT_OPEN)
/*! The file is written failed. */
#define GADI_PDA_ERR_WRITE_FAILED                                           \
                                  (GADI_PDA_MODULE_BASE + GADI_ERR_WRITE_FAILED)
/*! The file is read failed. */
#define GADI_PDA_ERR_READ_FAILED                                            \
                                  (GADI_PDA_MODULE_BASE + GADI_ERR_READ_FAILED)
/*! The file is read failed. */
#define GADI_PDA_ERR_MAP                                            \
                                  (GADI_PDA_MODULE_BASE + GADI_ERR_MAP)

#define GADI_MD_MAX_CHANNEL (2)

/*! Select monitor buffer  */
typedef enum {
    /*use main A buffer*/
    GADI_PDA_CAP_MAIN_A_BUFFER = 0,
    /*use main B buffer*/
    GADI_PDA_CAP_MAIN_B_BUFFER,
    /*use sub A vpss buffer:yuv 420 only*/
    GADI_PDA_CAP_SUB_A_BUFFER,
    /*use sub B vpss buffer:yuv 420 only*/
    GADI_PDA_CAP_SUB_B_BUFFER,
    /*use sub C vpss buffer:encode yuv 420/vout yuv 422*/
    GADI_PDA_CAP_SUB_C_BUFFER,
    /*PDA capture source numbers.*/
    GADI_PDA_CAP_NUM_BUFFER
} GADI_PDA_CAP_Buffer_TypeT;

/*bufffer information */
typedef struct
{
    /* preview buffer width*/
    GADI_U32 width;
    /* preview buffer height*/
    GADI_U32 height;
    /* one line's width of preview buffer */
    GADI_U32 stride;
}GADI_PDA_Buffer_InfoT;

/*MD attribute*/
typedef struct
{
    /* PDA width*/
    u32 width;
    /* PDA height*/
    u32 height;
}GADI_PDA_AttrT;


/*PDA data*/
typedef struct
{
    /*address*/
    void            *pAddr;
    /*stride of PDA data*/
    GADI_U32        stride;
    /*PDA channle width in MB*/
    GADI_U32        u32MbWidth;
    /*PDA channle height in MB*/
    GADI_U32        u32MbHeight;
    /*PDA data length*/
    u32             total_num_byte;
}GADI_PDA_DataT;

/*format of caputre frame*/
typedef enum
{
    /*Y is planer,U V is interleaved*/
    FRAME_FORMAT_YUV_SEMIPLANAR_420,
    /*Y is planer,U V is interleaved*/
    FRAME_FORMAT_YUV_SEMIPLANAR_422,
}GADI_PDA_Frame_Format;


/*information of caputre frame*/
typedef struct
{
    /*format of caputre frame*/
    GADI_PDA_Frame_Format frameFormat;
    /*width of caputre frame*/
    GADI_U32    width;
    /*height of caputre frame*/
    GADI_U32    height;
    /*Y address of caputre frame*/
    GADI_U8     *yAddr;
    /*UV address of caputre frame, the UV data is interleaved*/
    GADI_U8     *uvAddr;
    /*stride of caputre frame*/
    GADI_U32    stride;
}GADI_PDA_Frame_InfoT;


#ifdef __cplusplus
extern "C" {
#endif

/*!
*******************************************************************************
** \brief Initialize the PDA module
**
** \param[in] type  motion buffer type.the encode must be running.
**
** \return
** - #GADI_OK
** - #GADI_PM_ERR_OUT_OF_MEMORY
** - #GADI_PM_ERR_ALREADY_INITIALIZED
**
** \sa gadi_pda_exit
**
*******************************************************************************
*/
GADI_ERR gadi_pda_init();

/*!
*******************************************************************************
** \brief get preivew buffer size
**
** \param[out] info  motion buffer information.
**
** \return
** - #GADI_OK
** - #GADI_PM_ERR_OUT_OF_MEMORY
** - #GADI_PM_ERR_ALREADY_INITIALIZED
**
** \sa
**
*******************************************************************************
*/
GADI_ERR gadi_pda_buffer_info(GADI_SYS_HandleT handle,GADI_PDA_Buffer_InfoT *info);


/*!
*******************************************************************************
** \brief capture motion buffer picture
**
** \param[out] frameInfo  frame information.
**
** \return
** - #GADI_OK
** - #GADI_PM_ERR_OUT_OF_MEMORY
** - #GADI_PM_ERR_ALREADY_INITIALIZED
**
** \sa
**
*******************************************************************************
*/
GADI_ERR gadi_pda_capture_buffer(GADI_SYS_HandleT handle, GADI_PDA_CAP_Buffer_TypeT buffer,
    GADI_PDA_Frame_InfoT * frameInfo);



/*!
*******************************************************************************
** \brief open a pda handle
**
** \param[in] pstAttr  motion attribute.
**
** \return
** - #NULL
** - #GADI_SYS_HandleT
**
** \sa
**
*******************************************************************************
*/
GADI_SYS_HandleT gadi_pda_open(GADI_PDA_AttrT *pstAttr, GADI_S32 viId);

/*!
*******************************************************************************
** \brief close pda handle
**
** \param[in] handle  pda handle.
**
** \return
** - #GADI_OK
** - #GADI_PM_ERR_OUT_OF_MEMORY
** - #GADI_PM_ERR_ALREADY_INITIALIZED
**
** \sa
**
*******************************************************************************
*/
GADI_ERR gadi_pda_close(GADI_SYS_HandleT handle);

/*!
*******************************************************************************
** \brief get pda attribute
**
** \param[in]  handle   pda handle.
** \param[out] pstAttr  attribute handle
**
** \return
** - #GADI_OK
** - #GADI_PM_ERR_OUT_OF_MEMORY
** - #GADI_PM_ERR_ALREADY_INITIALIZED
**
** \sa
**
*******************************************************************************
*/
GADI_ERR gadi_pda_get_attr(GADI_SYS_HandleT handle, GADI_PDA_AttrT *pstAttr);

/*!
*******************************************************************************
** \brief set pda attribute
**
** \param[in]  handle   pda handle.
** \param[in] pstAttr  attribute handle
**
** \return
** - #GADI_OK
** - #GADI_PM_ERR_OUT_OF_MEMORY
** - #GADI_PM_ERR_ALREADY_INITIALIZED
**
** \sa
**
*******************************************************************************
*/
GADI_ERR gadi_pda_set_attr(GADI_SYS_HandleT handle, GADI_PDA_AttrT *pstAttr);

/*!
*******************************************************************************
** \brief start picture analysis
**
** \param[in]  handle   pda handle.
**
** \return
** - #GADI_OK
** - #GADI_PM_ERR_OUT_OF_MEMORY
** - #GADI_PM_ERR_ALREADY_INITIALIZED
**
** \sa
**
*******************************************************************************
*/
GADI_ERR gadi_pda_start_recv_pic(GADI_SYS_HandleT handle);

/*!
*******************************************************************************
** \brief stop picture analysis
**
** \param[in]  handle   pda handle.
**
** \return
** - #GADI_OK
** - #GADI_PM_ERR_OUT_OF_MEMORY
** - #GADI_PM_ERR_ALREADY_INITIALIZED
**
** \sa
**
*******************************************************************************
*/
GADI_ERR gadi_pda_stop_recv_pic(GADI_SYS_HandleT handle);

/*!
*******************************************************************************
** \brief get sad information of all Micro block
**
** \param[in]   handle          pda handle.
** \param[out]  pstPdaData      pda SAD informtion.
** \param[in]   bBlock          block or not.
**
** \return
** - #GADI_OK
** - #GADI_PM_ERR_OUT_OF_MEMORY
** - #GADI_PM_ERR_ALREADY_INITIALIZED
**
** \sa
**
*******************************************************************************
*/
GADI_ERR gadi_pda_get_data(GADI_SYS_HandleT handle, GADI_PDA_DataT *pstPdaData, GADI_BOOL bBlock);

/*!
*******************************************************************************
** \brief relese sad information
**
** \param[in]   handle          pda handle.
** \param[in]  pstPdaData      pda SAD informtion.
**
** \return
** - #GADI_OK
** - #GADI_PM_ERR_OUT_OF_MEMORY
** - #GADI_PM_ERR_ALREADY_INITIALIZED
**
** \sa
**
*******************************************************************************
*/
GADI_ERR gadi_pda_release_data(GADI_SYS_HandleT handle, const GADI_PDA_DataT* pstPdaData);


/*!
*******************************************************************************
** \brief exit pda module
**
** \return
** - #GADI_OK
** - #GADI_PM_ERR_OUT_OF_MEMORY
** - #GADI_PM_ERR_ALREADY_INITIALIZED
**
** \sa
**
*******************************************************************************
*/
GADI_ERR gadi_pda_exit(void);

#ifdef __cplusplus
}
#endif


#endif
