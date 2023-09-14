/*!
*****************************************************************************
** \file        adi/include/adi_pm.h
**
** \brief       ADI privacy mask module header file.
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2015-2019 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _ADI_PM_H_
#define _ADI_PM_H_

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
#define GADI_PM_ERR_BAD_PARAMETER                                          \
                              (GADI_PM_MODULE_BASE + GADI_ERR_BAD_PARAMETER)
/*! Memory allocation failed. */
#define GADI_PM_ERR_OUT_OF_MEMORY                                          \
                              (GADI_PM_MODULE_BASE + GADI_ERR_OUT_OF_MEMORY)
/*! Device already initialised. */
#define GADI_PM_ERR_ALREADY_INITIALIZED                                    \
                              (GADI_PM_MODULE_BASE + GADI_ERR_ALREADY_INITIALIZED)
/*! Device not initialised. */
#define GADI_PM_ERR_NOT_INITIALIZED                                        \
                              (GADI_PM_MODULE_BASE + GADI_ERR_NOT_INITIALIZED)
/*! Feature or function is not available. */
#define GADI_PM_ERR_FEATURE_NOT_SUPPORTED                                  \
                              (GADI_PM_MODULE_BASE + GADI_ERR_FEATURE_NOT_SUPPORTED)
/*! Timeout occured. */
#define GADI_PM_ERR_TIMEOUT                                                \
                              (GADI_PM_MODULE_BASE + GADI_ERR_TIMEOUT)
/*! The device is busy, try again later. */
#define GADI_PM_ERR_DEVICE_BUSY                                            \
                              (GADI_PM_MODULE_BASE + GADI_ERR_DEVICE_BUSY)
/*! Invalid handle was passed. */
#define GADI_PM_ERR_INVALID_HANDLE                                         \
                              (GADI_PM_MODULE_BASE + GADI_ERR_INVALID_HANDLE)
/*! Semaphore could not be created. */
#define GADI_PM_ERR_SEMAPHORE_CREATE                                       \
                               (GADI_PM_MODULE_BASE + GADI_ERR_SEMAPHORE_CREATE)
/*! The driver's used version is not supported. */
#define GADI_PM_ERR_UNSUPPORTED_VERSION                                    \
                               (GADI_PM_MODULE_BASE + GADI_ERR_UNSUPPORTED_VERSION)
/*! The driver's used version is not supported. */
#define GADI_PM_ERR_FROM_DRIVER                                            \
                               (GADI_PM_MODULE_BASE + GADI_ERR_FROM_DRIVER)
/*! The device/handle is not open.. */
#define GADI_PM_ERR_NOT_OPEN                                               \
                               (GADI_PM_MODULE_BASE + GADI_ERR_NOT_OPEN)


//*****************************************************************************
//*****************************************************************************
//** Enumerated types
//*****************************************************************************
//*****************************************************************************
/*!
*******************************************************************************
** \brief privacy mask colour enum.
*******************************************************************************
*/
typedef enum {
    /*! add include region.*/
    GADI_PM_ADD_INC = 0,
    /*! add exclude region.*/
    GADI_PM_ADD_EXC,
    /*! replace with new region.*/
    GADI_PM_REPLACE,
    /*! remove all regions.*/
    GADI_PM_REMOVE_ALL,
    GADI_PM_ACTIONS_NUM,
}GADI_PM_ActionEnumT;


//*****************************************************************************
//*****************************************************************************
//** Data Structures
//*****************************************************************************
//*****************************************************************************


/*!
*******************************************************************************
** \brief privacy mask malloc parameters.
*******************************************************************************
*/
typedef struct
{
    /*! vin source id.*/
    GADI_U32                srcId;
    /*! coordinate unite: 0:precent, 1-pixel.*/
    GADI_U32                unit;
    /*! privacy mask coordinate x offset.*/
    GADI_U32                offsetX;
    /*! privacy mask coordinate y offset.*/
    GADI_U32                offsetY;
    /*! privacy mask coordinate width.*/
    GADI_U32                width;
    /*! privacy mask coordinate height.*/
    GADI_U32                height;
    /*! privacy mask colour. format: RGB888*/
    GADI_U32                colour;
    /*! privacy mask action.*/
    GADI_PM_ActionEnumT     action;
} GADI_PM_MallocParamsT;

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
** \brief Initialize the ADI privacy mask module.
**
** \return
** - #GADI_OK
** - #GADI_PM_ERR_OUT_OF_MEMORY
** - #GADI_PM_ERR_ALREADY_INITIALIZED
**
** \sa gadi_pm_exit
**
*******************************************************************************
*/
GADI_ERR gadi_pm_init(void);

/*!
*******************************************************************************
** \brief Shutdown the ADI privacy mask module.
**
** \return
** - #GADI_OK
** - #GADI_PM_ERR_NOT_INITIALIZED
**
** \sa gadi_pm_init
**
*******************************************************************************
*/
GADI_ERR gadi_pm_exit(void);

/*!
*******************************************************************************
** \brief Open the ADI privacy mask module.
**
** \param[in] errorCodePtr pointer to return the error code.
**
** \return 
** - #Return an valid handle of ADI privacy mask module instance.
**
** \sa gadi_pm_close
**
*******************************************************************************
*/
GADI_SYS_HandleT gadi_pm_open(GADI_ERR * errorCodePtr, GADI_S32 viId);

/*!
*******************************************************************************
** \brief Close one ADI privacy mask module instance.
**
** \param[in]  handle     Valid ADI privacy mask instance handle previously opened by
**                        #gadi_pm_open.
**
** \return
** - #GADI_OK
** - #GADI_PM_ERR_BAD_PARAMETER
**
** \sa gadi_pm_open
**
*******************************************************************************
*/
GADI_ERR gadi_pm_close(GADI_SYS_HandleT handle);

/*!
*******************************************************************************
** \brief Malloc one privacy mask, return the privacy mask index.
**
** \param[in]  handle     Valid ADI privacy mask instance handle previously opened by
**                        #gadi_pm_open.
** \param[in]  *params    the pointer of privacy mask malloc parameters struct.
** \param[out] *pmIndex   if malloc successful, this pointer will return the
**                        privacy mask index.
**
** \return
** - #GADI_OK
** - #GADI_PM_ERR_FROM_DRIVER
** - #GADI_PM_ERR_BAD_PARAMETER
**
** \sa gadi_pm_open
**
*******************************************************************************
*/
GADI_ERR gadi_pm_malloc(GADI_SYS_HandleT handle, GADI_PM_MallocParamsT *params, GADI_U8 *pmIndex);

/*!
*******************************************************************************
** \brief Enable/disable the specified privacy mask that has been malloc.
**
** \param[in]  handle     Valid ADI privacy mask instance handle previously opened by
**                        #gadi_pm_open.
** \param[in]  pmIndex    the privacy mask index.
** \param[in]  enable     flag of enable or disable(0:disable, 1:enable).
**
** \return
** - #GADI_OK
** - #GADI_PM_ERR_FROM_DRIVER
** - #GADI_PM_ERR_BAD_PARAMETER
**
** \sa gadi_pm_malloc
**
*******************************************************************************
*/
GADI_ERR gadi_pm_enable(GADI_SYS_HandleT handle, GADI_U8 pmIndex, GADI_U32 enable);


#ifdef __cplusplus
    }
#endif


#endif /* _ADI_PM_H_ */
