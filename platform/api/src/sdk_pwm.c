/*!
*****************************************************************************
** \file        adi/src/adi_pwm.c
**
** \brief       ADI pwm module(include DC iris) function
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "adi_sys.h"
#include "adi_types.h"
#include "sdk_pwm.h"

//*****************************************************************************
//*****************************************************************************
//** Local Defines


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
#define GADI_PWM_ERR_BAD_PARAMETER                                          \
                              (GADI_PWM_MODULE_BASE + GADI_ERR_BAD_PARAMETER)
/*! Memory allocation failed. */
#define GADI_PWM_ERR_OUT_OF_MEMORY                                          \
                              (GADI_PWM_MODULE_BASE + GADI_ERR_OUT_OF_MEMORY)
/*! Device already initialised. */
#define GADI_PWM_ERR_ALREADY_INITIALIZED                                    \
                              (GADI_PWM_MODULE_BASE + GADI_ERR_ALREADY_INITIALIZED)
/*! Device not initialised. */
#define GADI_PWM_ERR_NOT_INITIALIZED                                        \
                              (GADI_PWM_MODULE_BASE + GADI_ERR_NOT_INITIALIZED)
/*! Feature or function is not available. */
#define GADI_PWM_ERR_FEATURE_NOT_SUPPORTED                                  \
                              (GADI_PWM_MODULE_BASE + GADI_ERR_FEATURE_NOT_SUPPORTED)
/*! Timeout occured. */
#define GADI_PWM_ERR_TIMEOUT                                                \
                              (GADI_PWM_MODULE_BASE + GADI_ERR_TIMEOUT)
/*! The device is busy, try again later. */
#define GADI_PWM_ERR_DEVICE_BUSY                                            \
                              (GADI_PWM_MODULE_BASE + GADI_ERR_DEVICE_BUSY)
/*! Invalid handle was passed. */
#define GADI_PWM_ERR_INVALID_HANDLE                                         \
                              (GADI_PWM_MODULE_BASE + GADI_ERR_INVALID_HANDLE)
/*! Semaphore could not be created. */
#define GADI_PWM_ERR_SEMAPHORE_CREATE                                       \
                              (GADI_PWM_MODULE_BASE + GADI_ERR_SEMAPHORE_CREATE)
/*! The driver's used version is not supported. */
#define GADI_PWM_ERR_UNSUPPORTED_VERSION                                    \
                               (GADI_PWM_MODULE_BASE + GADI_ERR_UNSUPPORTED_VERSION)
/*! The driver's used version is not supported. */
#define GADI_PWM_ERR_FROM_DRIVER                                            \
                               (GADI_PWM_MODULE_BASE + GADI_ERR_FROM_DRIVER)
/*! The device/handle is not open.. */
#define GADI_PWM_ERR_NOT_OPEN                                               \
                               (GADI_PWM_MODULE_BASE + GADI_ERR_NOT_OPEN)


//*****************************************************************************
//*****************************************************************************
//** Enumerated types
//*****************************************************************************
//*****************************************************************************
/*!
*******************************************************************************
** \brief prm iris control state enum.
*******************************************************************************
*/
typedef enum{
    GADI_PWM_IRIS_IDLE = 0,
    GADI_PWM_IRIS_FLUCTUATE,
    GADI_PWM_IRIS_STATE_NUM,
} GADI_PWM_IRIS_STATE;


//*****************************************************************************
//*****************************************************************************
//** Data Structures
//*****************************************************************************
//*****************************************************************************

typedef struct
{
    GADI_S32    pCoef;
    GADI_S32    iCoef;
    GADI_S32    dCoef;
}GADI_PWM_IrisPidCoef;


//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************

/*!
*******************************************************************************
** \brief init pwm device.
**
** \return
** - #GADI_OK
** - #GADI_PWM_ERR_ALREADY_INITIALIZED
** - #GADI_PWM_ERR_NOT_OPEN
**
** \sa gadi_pwm_init
**
*******************************************************************************
*/
GADI_ERR gadi_pwm_init(void);

/*!
*******************************************************************************
** \brief exit pwm device.
**
** \return
** - #GADI_OK
** - #GADI_PWM_ERR_NOT_INITIALIZED
**
** \sa gadi_pwm_exit
**
*******************************************************************************
*/
GADI_ERR gadi_pwm_exit(void);

/*!
*******************************************************************************
** \brief open one pwm channel.
**
** \param[in]  handle     Valid ADI pwm instance handle previously opened by
**                        #gadi_pwm_open.
** \param[in]  index     Valid pwm channel params [0~3].
**
** \return
** - #GADI_OK
** - #GADI_PWM_ERR_BAD_PARAMETER
** - #GADI_PWM_ERR_NOT_INITIALIZED
** - #GADI_PWM_ERR_OUT_OF_MEMORY
**
** \sa gadi_pwm_open
**
*******************************************************************************
*/
GADI_SYS_HandleT gadi_pwm_open(GADI_ERR *ErrorCode, GADI_U8 channel);

/*!
*******************************************************************************
** \brief close one pwm channel.
**
** \param[in]  handle     Valid ADI pwm instance handle previously opened by
**                        #gadi_pwm_open.
**
** \return
** - #GADI_OK
** - #GADI_PWM_ERR_BAD_PARAMETER
**
** \sa gadi_pwm_close
**
*******************************************************************************
*/
GADI_ERR gadi_pwm_close(GADI_SYS_HandleT handle);

/*!
*******************************************************************************
** \brief start one pwm channel.
**
** \param[in]  handle     Valid ADI pwm instance handle previously opened by
**                        #gadi_pwm_open.
**
** \return
** - #GADI_OK
** - #GADI_PWM_ERR_BAD_PARAMETER
** - #GADI_PWM_ERR_FROM_DRIVER
**
** \sa gadi_pwm_start
**
*******************************************************************************
*/
GADI_ERR gadi_pwm_start(GADI_SYS_HandleT handle);

/*!
*******************************************************************************
** \brief stop one pwm channel.
**
** \param[in]  handle     Valid ADI pwm instance handle previously opened by
**                        #gadi_pwm_open.
**
** \return
** - #GADI_OK
** - #GADI_PWM_ERR_BAD_PARAMETER
** - #GADI_PWM_ERR_FROM_DRIVER
**
** \sa gadi_pwm_stop
**
*******************************************************************************
*/
GADI_ERR gadi_pwm_stop(GADI_SYS_HandleT handle);

/*!
*******************************************************************************
** \brief get speed of one pwm channel.
**
** \param[in]  handle     Valid ADI pwm instance handle previously opened by
**                        #gadi_pwm_open.
** \param[in]  *speed     Valid ADI pwm speed(Hz).
**
** \return
** - #GADI_OK
** - #GADI_PWM_ERR_BAD_PARAMETER
** - #GADI_PWM_ERR_FROM_DRIVER
**
** \sa gadi_pwm_get_speed
**
*******************************************************************************
*/
GADI_ERR gadi_pwm_get_speed(GADI_SYS_HandleT handle, GADI_U32 *speed);

/*!
*******************************************************************************
** \brief set speed of one pwm channel.
**
** \param[in]  handle     Valid ADI pwm instance handle previously opened by
**                        #gadi_pwm_open.
** \param[in]  speed     Valid ADI pwm speed(Hz).
**
** \return
** - #GADI_OK
** - #GADI_PWM_ERR_BAD_PARAMETER
** - #GADI_PWM_ERR_FROM_DRIVER
**
** \sa gadi_pwm_set_speed
**
*******************************************************************************
*/
GADI_ERR gadi_pwm_set_speed(GADI_SYS_HandleT handle, GADI_U32 speed);

/*!
*******************************************************************************
** \brief get mode of one pwm channel.
**
** \param[in]  handle     Valid ADI pwm instance handle previously opened by
**                        #gadi_pwm_open.
** \param[in]  *mode     Valid ADI pwm work mode [0~1].
**
** \return
** - #GADI_OK
** - #GADI_PWM_ERR_BAD_PARAMETER
** - #GADI_PWM_ERR_FROM_DRIVER
**
** \sa gadi_pwm_get_mode
**
*******************************************************************************
*/
GADI_ERR gadi_pwm_get_mode(GADI_SYS_HandleT handle, GADI_U8 *mode);

/*!
*******************************************************************************
** \brief set mode of one pwm channel.
**
** \param[in]  handle     Valid ADI pwm instance handle previously opened by
**                        #gadi_pwm_open.
** \param[in]  mode     Valid ADI pwm work mode [0~1].
**
** \return
** - #GADI_OK
** - #GADI_PWM_ERR_BAD_PARAMETER
** - #GADI_PWM_ERR_FROM_DRIVER
**
** \sa gadi_pwm_set_mode
**
*******************************************************************************
*/
GADI_ERR gadi_pwm_set_mode(GADI_SYS_HandleT handle, GADI_U8 mode);

/*!
*******************************************************************************
** \brief get the pwm device duty rate.
**
** \param[in]  handle     Valid ADI pwm instance handle previously opened by
**                        #gadi_pwm_open.
** \param[in]  *duty       get duty rate.
**
** \return
** - #GADI_OK
** - #GADI_PWM_ERR_FROM_DRIVER
** - #GADI_PWM_ERR_BAD_PARAMETER
**
** \sa gadi_pwm_get_duty
**
*******************************************************************************
*/
GADI_ERR gadi_pwm_get_duty(GADI_SYS_HandleT handle, GADI_U32 *duty);

/*!
*******************************************************************************
** \brief set the pwm device duty rate.
**
** \param[in]  handle     Valid ADI pwm instance handle previously opened by
**                        #gadi_pwm_open.
** \param[in]  duty       duty rate [1 ~ 999].
**
** \return
** - #GADI_OK
** - #GADI_PWM_ERR_FROM_DRIVER
** - #GADI_PWM_ERR_BAD_PARAMETER
**
** \sa gadi_pwm_set_duty
**
*******************************************************************************
*/
GADI_ERR gadi_pwm_set_duty(GADI_SYS_HandleT handle, GADI_U32 duty);

#if 0
/*!
*******************************************************************************
** \brief enable the dc iris.
**
** \param[in]  handle     Valid ADI pwm instance handle previously opened by
**                        #gadi_pwm_open.
** \param[in]  enable     flag of disable or enable.
**
** \return
** - #GADI_OK
** - #GADI_PWM_ERR_FROM_DRIVER
** - #GADI_PWM_ERR_BAD_PARAMETER
**
** \sa gadi_pwm_open
**
*******************************************************************************
*/
GADI_ERR gadi_pwm_dc_iris_enable(GADI_SYS_HandleT handle, GADI_U8 enable);

/*!
*******************************************************************************
** \brief get the dc iris pid data.
**
** \param[in]  pPidCoef     date pointer of struct GADI_PWM_IrisPidCoef.
**
** \return
** - #GADI_OK
** - #GADI_PWM_ERR_FROM_DRIVER
** - #GADI_PWM_ERR_BAD_PARAMETER
**
** \sa gadi_pwm_open
**
*******************************************************************************
*/
GADI_ERR gadi_pwm_dc_iris_get_pid_coef(GADI_PWM_IrisPidCoef * pPidCoef);

/*!
*******************************************************************************
** \brief set the dc iris pid data.
**
** \param[in]  pPidCoef     date pointer of struct GADI_PWM_IrisPidCoef.
**
** \return
** - #GADI_OK
** - #GADI_PWM_ERR_FROM_DRIVER
** - #GADI_PWM_ERR_BAD_PARAMETER
**
** \sa gadi_pwm_open
**
*******************************************************************************
*/
GADI_ERR gadi_pwm_dc_iris_set_pid_coef(GADI_PWM_IrisPidCoef * pPidCoef);
#endif

//*****************************************************************************
//*****************************************************************************
#define GADI_PWM_CHANNEL_NUMBERS        (4)

#define GADI_PWM_OPEN_DUTY              (200)
#define GADI_PWM_MAX_DUTY               (900)
#define GADI_PWM_MIN_DUTY               (100)

#define GADI_PWM_IGNORE_FULL_OPEN_CHECK (-10)
#define GADI_PWM_FULL_OPEN_CHECK        (20)

#define Kac 1000

#define GK_PWM_IOCTL_MAGIC      'k'

#define _PWM_IO(IOCTL)              _IO(GK_PWM_IOCTL_MAGIC, IOCTL)
#define _PWM_IOR(IOCTL, param)      _IOR(GK_PWM_IOCTL_MAGIC, IOCTL, param)
#define _PWM_IOW(IOCTL, param)      _IOW(GK_PWM_IOCTL_MAGIC, IOCTL, param)
#define _PWM_IOWR(IOCTL, param)     _IOWR(GK_PWM_IOCTL_MAGIC, IOCTL, param)

#define GADI_PWM_DEVICE_NAME "/dev/pwm"

//*****************************************************************************
//*****************************************************************************
//** Local structures
//*****************************************************************************
//*****************************************************************************
typedef struct {
    /*file descriptor of pwm device. */
    GADI_S32 fdPwm;
    /*PWM channel index. */
    GADI_U8 pwmIndex;
} GADI_PWM_HandleT;

typedef struct {
    GADI_S32 enable;
    GADI_S32 pData;
    GADI_S32 iData;
    GADI_S32 dData;
} GADI_PWM_PidData;

typedef struct {
    GADI_PWM_HandleT *pwmHandle;
    GADI_PWM_IRIS_STATE state;
    GADI_PWM_PidData pidData;
    GADI_S32 duty;
    GADI_S32 lumaData[2];
    GADI_U8 enable;
} GADI_PWM_IrisContHandleT;

typedef struct {
    GADI_U8 channel;            /* 0 ~ 3 */
    GADI_U8 enable;             /* 0: enable  1: disable */
} GADI_PWM_StatusT;

typedef struct {
    GADI_U8 channel;            /* 0 ~ 3 */
    GADI_U8 mode;               /* 0: normal speed  1: sync speed */
} GADI_PWM_ModeT;

typedef struct {
    GADI_U8 channel;
    GADI_U16 xon;               /* 0x00 ~ 0xffff */
    GADI_U16 xoff;              /* 0x00 ~ 0xffff */
} GADI_PWM_DutyT;

typedef struct {
    u8 channel;
    u32 speed;
} GADI_PWM_SpeedT;

enum {
    IOCTL_ENABLE = 0x10,
    IOCTL_DISABLE,
    IOCTL_SET_DUTY,
    IOCTL_GET_DUTY,
    IOCTL_SET_MODE,
    IOCTL_GET_MODE,
    IOCTL_SET_SPEED,
    IOCTL_GET_SPEED,
    IOCTL_GET_STATUS,
};

#define GK_PWM_IOCTL_ENABLE     _PWM_IO(IOCTL_ENABLE)
#define GK_PWM_IOCTL_DISABLE    _PWM_IO(IOCTL_DISABLE)
#define GK_PWM_IOCTL_SET_DUTY   _PWM_IOW(IOCTL_SET_DUTY, GADI_PWM_DutyT*)
#define GK_PWM_IOCTL_GET_DUTY   _PWM_IOR(IOCTL_GET_DUTY, GADI_PWM_DutyT*)
#define GK_PWM_IOCTL_SET_MODE   _PWM_IOW(IOCTL_SET_MODE, GADI_PWM_ModeT*)
#define GK_PWM_IOCTL_GET_MODE   _PWM_IOR(IOCTL_GET_MODE, GADI_PWM_ModeT*)
#define GK_PWM_IOCTL_SET_SPEED  _PWM_IOW(IOCTL_SET_SPEED, GADI_PWM_SpeedT*)
#define GK_PWM_IOCTL_GET_SPEED  _PWM_IOR(IOCTL_GET_SPEED, GADI_PWM_SpeedT*)
#define GK_PWM_IOCTL_GET_STATUS _PWM_IOR(IOCTL_GET_STATUS, GADI_PWM_StatusT*)

//*****************************************************************************
//*****************************************************************************
//** Global Data
//*****************************************************************************
//*****************************************************************************

//*****************************************************************************
//*****************************************************************************
//** Local Data
//*****************************************************************************
//*****************************************************************************
#if 0
static GADI_PWM_IrisContHandleT pwmIrisHandle = {
    .duty = 200,
    .lumaData[0] = 0,
    .lumaData[1] = 0,
};

static GADI_PWM_IrisPidCoef pwmIrisPidCoef = {
    .pCoef = 1500,
    .iCoef = 1,
    .dCoef = 1000,
};
#endif

/* pwm device file discrabiton.  */
static GADI_S32 pwmFd = -1;

//*****************************************************************************
//*****************************************************************************
//** Local Functions Declaration
//*****************************************************************************
//*****************************************************************************
static GADI_ERR pwm_enable(GADI_SYS_HandleT handle);
static GADI_ERR pwm_disable(GADI_SYS_HandleT handle);
static GADI_ERR pwm_get_status(GADI_SYS_HandleT handle, GADI_U8 * status);
static GADI_ERR pwm_set_duty(GADI_S32 fdPwm, GADI_U8 channel, GADI_U32 duty);
static GADI_ERR pwm_get_duty(GADI_S32 fdPwm, GADI_U8 channel, GADI_U32 * duty);

#if 0
static GADI_S32 pwm_iris_check_full_open(GADI_S32 lumaDiff);
static GADI_S32 pwm_iris_duty_adjust(GADI_S32 lumaDiff);
static GADI_ERR pwm_iris_control(GADI_S32 run, GADI_S32 lumaDiff,
    GADI_S32 * state);
#endif

//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************
GADI_ERR gadi_pwm_init(void)
{
    if (pwmFd > 0) {
        GADI_ERROR("pwm device already initalized!\n");
        return GADI_PWM_ERR_ALREADY_INITIALIZED;
    }
    if ((pwmFd = open(GADI_PWM_DEVICE_NAME, O_RDWR | O_TRUNC)) < 0) {
        GADI_ERROR("open pwm device:%s failed!\n", GADI_PWM_DEVICE_NAME);
        return GADI_PWM_ERR_NOT_OPEN;
    }
    return GADI_OK;
}

GADI_ERR gadi_pwm_exit(void)
{
    if (pwmFd <= 0) {
        GADI_ERROR("pwm device is not initalized!\n");
        return GADI_PWM_ERR_NOT_INITIALIZED;
    }
    close(pwmFd);
    pwmFd = -1;
    return GADI_OK;
}

GADI_SYS_HandleT gadi_pwm_open(GADI_ERR * ErrorCode, GADI_U8 channel)
{
    GADI_PWM_HandleT *pwmHandle;

    /*check input parameter. */
    if (ErrorCode == NULL) {
        return NULL;
    }
    if (channel >= GADI_PWM_CHANNEL_NUMBERS) {
        GADI_ERROR("pwm device channel number is bad!\n");
        *ErrorCode = GADI_PWM_ERR_BAD_PARAMETER;
        return NULL;
    }
    if (pwmFd < 0) {
        GADI_ERROR("pwm device not initalized!\n");
        *ErrorCode = GADI_PWM_ERR_NOT_INITIALIZED;
        return NULL;
    }
    //pwmHandle = gadi_sys_malloc(sizeof(GADI_PWM_HandleT));
    pwmHandle = malloc(sizeof(GADI_PWM_HandleT));	//xqq
    if (pwmHandle == NULL) {
        GADI_ERROR("gadi_pwm_open malloc memory failed!\n");
        *ErrorCode = GADI_PWM_ERR_OUT_OF_MEMORY;
        return NULL;
    }
    //gadi_sys_memset(pwmHandle, 0, sizeof(GADI_PWM_HandleT));
    memset(pwmHandle, 0, sizeof(GADI_PWM_HandleT));	//xqq

    pwmHandle->fdPwm = pwmFd;
    pwmHandle->pwmIndex = channel;

    *ErrorCode = GADI_OK;
    return (GADI_SYS_HandleT) pwmHandle;

}

GADI_ERR gadi_pwm_close(GADI_SYS_HandleT handle)
{
    GADI_ERR retVal = GADI_OK;
    GADI_PWM_HandleT *pwmHandleTemp = (GADI_PWM_HandleT *) handle;

    if (pwmHandleTemp == NULL) {
        return GADI_PWM_ERR_BAD_PARAMETER;
    }
    retVal = pwm_disable(handle);
    if (retVal) {
        GADI_ERROR("close pwm channel failed!\n");
        return retVal;
    }
   // gadi_sys_memset(pwmHandleTemp, 0, sizeof(GADI_PWM_HandleT));
   // gadi_sys_free(pwmHandleTemp);
   memset(pwmHandleTemp, 0, sizeof(GADI_PWM_HandleT));
   free(pwmHandleTemp);		//xqq

    return GADI_OK;

}

GADI_ERR gadi_pwm_get_speed(GADI_SYS_HandleT handle, GADI_U32 * speed)
{
    GADI_PWM_SpeedT pwmSpeed;
    GADI_PWM_HandleT *pwmHandleTemp = (GADI_PWM_HandleT *) handle;

    if (pwmHandleTemp == NULL || speed == NULL || pwmHandleTemp->pwmIndex < 0
        || pwmHandleTemp->pwmIndex >= GADI_PWM_CHANNEL_NUMBERS) {
        return GADI_PWM_ERR_BAD_PARAMETER;
    }

    pwmSpeed.channel = pwmHandleTemp->pwmIndex;
    if (ioctl(pwmHandleTemp->fdPwm, GK_PWM_IOCTL_GET_SPEED, &pwmSpeed)) {
        GADI_ERROR("get pwm speed  failed!\n");
        return GADI_PWM_ERR_FROM_DRIVER;
    }
    *speed = pwmSpeed.speed;
    return GADI_OK;
}

/*  speed: n Hz   mode:0 normal speed  1 vin sync speed*/
GADI_ERR gadi_pwm_set_speed(GADI_SYS_HandleT handle, GADI_U32 speed)
{
    GADI_ERR retVal = GADI_OK;
    GADI_U8 pwmStatus = 0;
    GADI_PWM_SpeedT pwmSpeed;
    GADI_PWM_HandleT *pwmHandleTemp = (GADI_PWM_HandleT *) handle;

    if (pwmHandleTemp == NULL || pwmHandleTemp->pwmIndex < 0
        || pwmHandleTemp->pwmIndex >= GADI_PWM_CHANNEL_NUMBERS) {
        return GADI_PWM_ERR_BAD_PARAMETER;
    }
    retVal = pwm_get_status(handle, &pwmStatus);
    if (retVal) {
        return retVal;
    }
    if (pwmStatus) {
        retVal = pwm_disable(handle);
        if (retVal)
            return retVal;
    }

    pwmSpeed.channel = pwmHandleTemp->pwmIndex;
    pwmSpeed.speed = speed;
    if (ioctl(pwmHandleTemp->fdPwm, GK_PWM_IOCTL_SET_SPEED, &pwmSpeed)) {
        GADI_ERROR("set pwm speed  failed!\n");
        return GADI_PWM_ERR_FROM_DRIVER;
    }

    if (pwmStatus) {
        retVal = pwm_enable(handle);
        if (retVal)
            return retVal;
    }
    return GADI_OK;
}

GADI_ERR gadi_pwm_get_mode(GADI_SYS_HandleT handle, GADI_U8 * mode)
{
    GADI_PWM_ModeT pwmMode;
    GADI_PWM_HandleT *pwmHandleTemp = (GADI_PWM_HandleT *) handle;

    if (pwmHandleTemp == NULL || mode == NULL || pwmHandleTemp->pwmIndex < 0
        || pwmHandleTemp->pwmIndex >= GADI_PWM_CHANNEL_NUMBERS) {
        return GADI_PWM_ERR_BAD_PARAMETER;
    }

    pwmMode.channel = pwmHandleTemp->pwmIndex;
    if (ioctl(pwmHandleTemp->fdPwm, GK_PWM_IOCTL_GET_MODE, &pwmMode)) {
        GADI_ERROR("set pwm mode  failed!\n");
        return GADI_PWM_ERR_FROM_DRIVER;
    }

    return GADI_OK;
}

GADI_ERR gadi_pwm_set_mode(GADI_SYS_HandleT handle, GADI_U8 mode)
{
    GADI_ERR retVal = GADI_OK;
    GADI_U8 pwmStatus = 0;
    GADI_PWM_ModeT pwmMode;
    GADI_PWM_HandleT *pwmHandleTemp = (GADI_PWM_HandleT *) handle;

    if (pwmHandleTemp == NULL || pwmHandleTemp->pwmIndex < 0
        || pwmHandleTemp->pwmIndex >= GADI_PWM_CHANNEL_NUMBERS) {
        return GADI_PWM_ERR_BAD_PARAMETER;
    }
    retVal = pwm_get_status(handle, &pwmStatus);
    if (retVal) {
        return retVal;
    }
    if (pwmStatus) {
        retVal = pwm_disable(handle);
        if (retVal)
            return retVal;
    }

    pwmMode.channel = pwmHandleTemp->pwmIndex;
    pwmMode.mode = mode;
    if (ioctl(pwmHandleTemp->fdPwm, GK_PWM_IOCTL_SET_MODE, &pwmMode)) {
        GADI_ERROR("set pwm mode  failed!\n");
        return GADI_PWM_ERR_FROM_DRIVER;
    }

    if (pwmStatus) {
        retVal = pwm_enable(handle);
        if (retVal)
            return retVal;
    }
    return GADI_OK;
}

GADI_ERR gadi_pwm_start(GADI_SYS_HandleT handle)
{
    if (handle == NULL) {
        return GADI_PWM_ERR_BAD_PARAMETER;
    }

    return pwm_enable(handle);
}

GADI_ERR gadi_pwm_stop(GADI_SYS_HandleT handle)
{
    if (handle == NULL) {
        return GADI_PWM_ERR_BAD_PARAMETER;
    }

    return pwm_disable(handle);
}

GADI_ERR gadi_pwm_get_duty(GADI_SYS_HandleT handle, GADI_U32 * duty)
{
    GADI_ERR retVal;
    GADI_PWM_HandleT *pwmHandleTemp = (GADI_PWM_HandleT *) handle;

    if (pwmHandleTemp == NULL || duty == NULL || pwmHandleTemp->pwmIndex < 0
        || pwmHandleTemp->pwmIndex >= GADI_PWM_CHANNEL_NUMBERS) {
        return GADI_PWM_ERR_BAD_PARAMETER;
    }

    retVal = pwm_get_duty(pwmHandleTemp->fdPwm, pwmHandleTemp->pwmIndex, duty);
    return retVal;

}

GADI_ERR gadi_pwm_set_duty(GADI_SYS_HandleT handle, GADI_U32 duty)
{
    GADI_ERR retVal;
    GADI_PWM_HandleT *pwmHandleTemp = (GADI_PWM_HandleT *) handle;

    if (pwmHandleTemp == NULL || pwmHandleTemp->pwmIndex < 0
        || pwmHandleTemp->pwmIndex >= GADI_PWM_CHANNEL_NUMBERS) {
        return GADI_PWM_ERR_BAD_PARAMETER;
    }

    retVal = pwm_set_duty(pwmHandleTemp->fdPwm, pwmHandleTemp->pwmIndex, duty);

    return retVal;
}

#if 0
GADI_ERR gadi_pwm_dc_iris_get_pid_coef(GADI_PWM_IrisPidCoef * pPidCoef)
{
    /*check input parameter. */
    if (pPidCoef == NULL) {
        return GADI_PWM_ERR_BAD_PARAMETER;
    }
    pPidCoef->pCoef = pwmIrisPidCoef.pCoef;
    pPidCoef->iCoef = pwmIrisPidCoef.iCoef;
    pPidCoef->dCoef = pwmIrisPidCoef.dCoef;

    return GADI_OK;
}

GADI_ERR gadi_pwm_dc_iris_set_pid_coef(GADI_PWM_IrisPidCoef * pPidCoef)
{
    /*check input parameter. */
    if (pPidCoef == NULL) {
        return GADI_PWM_ERR_BAD_PARAMETER;
    }
    pwmIrisPidCoef.pCoef = pPidCoef->pCoef;
    pwmIrisPidCoef.iCoef = pPidCoef->iCoef;
    pwmIrisPidCoef.dCoef = pPidCoef->dCoef;

    return GADI_OK;
}

static GADI_ERR gadi_pwm_dc_iris_enable(GADI_SYS_HandleT handle, GADI_U8 enable)
{
    GADI_PWM_HandleT *pwmHandleTemp = (GADI_PWM_HandleT *) handle;
    dc_iris_cntl iris_cntl = NULL;

    if ((pwmHandleTemp == NULL) || (pwmHandleTemp->fdPwm <= 0)) {
        GADI_ERROR
            ("open corresponding pwm channel first, if wanting to enable dc iris\n");
        return GADI_PWM_ERR_BAD_PARAMETER;
    }

    pwmIrisHandle.enable = enable;

    if (enable) {
        pwmIrisHandle.pwmHandle = pwmHandleTemp;
        pwmIrisHandle.state = GADI_PWM_IRIS_IDLE;
        pwm_enable(pwmIrisHandle.pwmHandle);
        iris_cntl = pwm_iris_control;
        img_register_iris_cntl(iris_cntl);
        GADI_INFO("registered iris control function into 3A.\n");
    } else {
        iris_cntl = NULL;
        img_register_iris_cntl(iris_cntl);
        pwm_set_duty(pwmHandleTemp->fdPwm, pwmHandleTemp->pwmIndex, 1);
        pwm_disable(pwmIrisHandle.pwmHandle);
        GADI_INFO("unregistered iris control funtion into 3A.\n");
    }

    return GADI_OK;
}
#endif

//*****************************************************************************
//*****************************************************************************
//** Local Functions
//*****************************************************************************
//*****************************************************************************
GADI_ERR pwm_get_status(GADI_SYS_HandleT handle, GADI_U8 * status)
{
    GADI_ERR err;
    GADI_PWM_StatusT pwmStatus;
    GADI_PWM_HandleT *pwmHandleTemp = (GADI_PWM_HandleT *) handle;

    if (pwmHandleTemp == NULL || status == NULL || pwmHandleTemp->pwmIndex < 0
        || pwmHandleTemp->pwmIndex >= GADI_PWM_CHANNEL_NUMBERS) {
        return GADI_PWM_ERR_BAD_PARAMETER;
    }

    pwmStatus.channel = pwmHandleTemp->pwmIndex;
    err = ioctl(pwmHandleTemp->fdPwm, GK_PWM_IOCTL_GET_STATUS, &pwmStatus);
    if (err) {
        GADI_ERROR("get pwm status failed[%d]!\n", err);
        return GADI_PWM_ERR_FROM_DRIVER;
    }
    *status = pwmStatus.enable;

    return GADI_OK;
}

static GADI_ERR pwm_enable(GADI_SYS_HandleT handle)
{
    GADI_PWM_HandleT *pwmHandleTemp = (GADI_PWM_HandleT *) handle;

    if (pwmHandleTemp == NULL || pwmHandleTemp->pwmIndex < 0
        || pwmHandleTemp->pwmIndex >= GADI_PWM_CHANNEL_NUMBERS) {
        return GADI_PWM_ERR_BAD_PARAMETER;
    }

    if (ioctl(pwmHandleTemp->fdPwm, GK_PWM_IOCTL_ENABLE,
            pwmHandleTemp->pwmIndex)) {
        GADI_ERROR("enable pwm device failed!\n");
        return GADI_PWM_ERR_FROM_DRIVER;
    }
    return GADI_OK;
}

static GADI_ERR pwm_disable(GADI_SYS_HandleT handle)
{
    GADI_PWM_HandleT *pwmHandleTemp = (GADI_PWM_HandleT *) handle;

    if (pwmHandleTemp == NULL || pwmHandleTemp->pwmIndex < 0
        || pwmHandleTemp->pwmIndex >= GADI_PWM_CHANNEL_NUMBERS) {
        return GADI_PWM_ERR_BAD_PARAMETER;
    }

    if (ioctl(pwmHandleTemp->fdPwm, GK_PWM_IOCTL_DISABLE,
            pwmHandleTemp->pwmIndex)) {
        GADI_ERROR("disable pwm device failed!\n");
        return GADI_PWM_ERR_FROM_DRIVER;
    }
    return GADI_OK;
}

static GADI_ERR pwm_get_duty(GADI_S32 fdPwm, GADI_U8 channel, GADI_U32 * duty)
{
    GADI_PWM_DutyT pwmDuty;

    if (fdPwm < 0) {
        return GADI_PWM_ERR_BAD_PARAMETER;
    }
    pwmDuty.channel = channel;

    if (ioctl(fdPwm, GK_PWM_IOCTL_GET_DUTY, &pwmDuty)) {
        GADI_ERROR("pwm_get_duty: set error [%d].\n", GADI_PWM_ERR_FROM_DRIVER);
        return GADI_PWM_ERR_FROM_DRIVER;
    }
    //*duty = (pwmDuty.xoff + 1) * 1000 / (pwmDuty.xoff + pwmDuty.xon + 2);
    *duty = pwmDuty.xon + 1;
    //printf("pwm_get_duty:%d,%d--%d\n", pwmDuty.xoff, pwmDuty.xon, *duty);

    return GADI_OK;
}

static GADI_ERR pwm_set_duty(GADI_S32 fdPwm, GADI_U8 channel, GADI_U32 duty)
{
    GADI_PWM_DutyT pwmDuty;

    if (fdPwm < 0 || duty == 0 || duty >= 1000) {
        return GADI_PWM_ERR_BAD_PARAMETER;
    }
    pwmDuty.channel = channel;
    pwmDuty.xon = duty - 1;
    pwmDuty.xoff = (1000 - duty) - 1;

    if (ioctl(fdPwm, GK_PWM_IOCTL_SET_DUTY, &pwmDuty)) {
        GADI_ERROR("pwm_set_duty:[%d.%d %%] set error [%d].\n", duty / 10,
            duty % 10, GADI_PWM_ERR_FROM_DRIVER);
        return GADI_PWM_ERR_FROM_DRIVER;
    }

    return GADI_OK;
}

#if 0
static GADI_S32 pwm_iris_check_full_open(GADI_S32 lumaDiff)
{
    GADI_S32 diffValue = 0;
    static GADI_S32 lastLowLumaDiff = -1;
    static GADI_S32 count = 0;

    if (pwmIrisHandle.duty < GADI_PWM_OPEN_DUTY) {
        if (lumaDiff >= GADI_PWM_IGNORE_FULL_OPEN_CHECK) {
            goto check_full_open_exit;
        }

        while (diffValue * (diffValue + 1) < ABS(lumaDiff)) {
            diffValue++;
        }

        if (ABS(lumaDiff - lastLowLumaDiff) < diffValue) {
            if (count++ > GADI_PWM_FULL_OPEN_CHECK) {
                count = 0;
                pwmIrisHandle.pidData.pData = 0;
                pwmIrisHandle.pidData.iData = 0;
                pwmIrisHandle.pidData.dData = 0;
                GADI_INFO("dc iris full open.");
                return 1;
            }
            return GADI_OK;
        }
    }
  check_full_open_exit:
    count = 0;
    lastLowLumaDiff = lumaDiff;
    return 0;
}

static GADI_S32 pwm_iris_duty_adjust(GADI_S32 lumaDiff)
{
    pwmIrisHandle.lumaData[1] = pwmIrisHandle.lumaData[0];
    pwmIrisHandle.lumaData[0] = lumaDiff;
    /*data_p */
    pwmIrisHandle.pidData.pData = pwmIrisHandle.lumaData[0];
    /*data_i */
    if (lumaDiff < 0) {         /*rectify luma_diff to increase its linearity */
        if (pwmIrisHandle.lumaData[0] > -10) {
            pwmIrisHandle.pidData.iData = pwmIrisHandle.pidData.iData +
                pwmIrisHandle.lumaData[0] * 5;
        } else if (pwmIrisHandle.lumaData[0] > -20) {
            pwmIrisHandle.pidData.iData = pwmIrisHandle.pidData.iData +
                pwmIrisHandle.lumaData[0] * 6;
        } else if (pwmIrisHandle.lumaData[0] > -40) {
            pwmIrisHandle.pidData.iData = pwmIrisHandle.pidData.iData +
                pwmIrisHandle.lumaData[0] * 7;
        } else if (pwmIrisHandle.lumaData[0] > -60) {
            pwmIrisHandle.pidData.iData = pwmIrisHandle.pidData.iData +
                pwmIrisHandle.lumaData[0] * 8;
        } else if (pwmIrisHandle.lumaData[0] > -80) {
            pwmIrisHandle.pidData.iData = pwmIrisHandle.pidData.iData +
                pwmIrisHandle.lumaData[0] * 10;
        } else if (pwmIrisHandle.lumaData[0] > -100) {
            pwmIrisHandle.pidData.iData = pwmIrisHandle.pidData.iData +
                pwmIrisHandle.lumaData[0] * 12;
        } else {
            pwmIrisHandle.pidData.iData = pwmIrisHandle.pidData.iData +
                pwmIrisHandle.lumaData[0] * 16;
        }
    } else {
        pwmIrisHandle.pidData.iData = pwmIrisHandle.pidData.iData +
            pwmIrisHandle.lumaData[0] * 4;
    }

    pwmIrisHandle.pidData.dData =
        pwmIrisHandle.lumaData[0] - pwmIrisHandle.lumaData[1];

    pwmIrisHandle.duty =
        pwmIrisPidCoef.pCoef * pwmIrisHandle.pidData.pData / Kac +
        pwmIrisPidCoef.iCoef * pwmIrisHandle.pidData.iData / Kac +
        pwmIrisPidCoef.dCoef * pwmIrisHandle.pidData.dData / Kac;

    if (pwmIrisHandle.duty > GADI_PWM_MAX_DUTY) {
        pwmIrisHandle.duty = GADI_PWM_MAX_DUTY;
    } else if (pwmIrisHandle.duty < GADI_PWM_MIN_DUTY) {
        pwmIrisHandle.duty = GADI_PWM_MIN_DUTY;
    }
    pwm_set_duty(pwmIrisHandle.pwmHandle->fdPwm,
        pwmIrisHandle.pwmHandle->pwmIndex, pwmIrisHandle.duty);

    return GADI_OK;
}

static GADI_ERR pwm_iris_control(GADI_S32 run, GADI_S32 lumaDiff,
    GADI_S32 * state)
{
    if (pwmIrisHandle.enable == 0) {
        pwm_set_duty(pwmIrisHandle.pwmHandle->fdPwm,
            pwmIrisHandle.pwmHandle->pwmIndex, GADI_PWM_MIN_DUTY);
        pwmIrisHandle.state = GADI_PWM_IRIS_IDLE;
        goto luma_control_exit;
    }

    if (!run) {
        pwm_set_duty(pwmIrisHandle.pwmHandle->fdPwm,
            pwmIrisHandle.pwmHandle->pwmIndex, GADI_PWM_MIN_DUTY);
        pwmIrisHandle.state = GADI_PWM_IRIS_IDLE;
        goto luma_control_exit;
    }
    switch (pwmIrisHandle.state) {
        case GADI_PWM_IRIS_IDLE:
            pwm_set_duty(pwmIrisHandle.pwmHandle->fdPwm,
                pwmIrisHandle.pwmHandle->pwmIndex, GADI_PWM_MAX_DUTY);
            pwmIrisHandle.state = GADI_PWM_IRIS_FLUCTUATE;
            break;

        case GADI_PWM_IRIS_FLUCTUATE:
            if (pwm_iris_check_full_open(lumaDiff)) {
                pwmIrisHandle.state = GADI_PWM_IRIS_IDLE;
            } else {
                pwm_iris_duty_adjust(lumaDiff);
                pwmIrisHandle.state = GADI_PWM_IRIS_FLUCTUATE;
            }
            break;

        default:
            break;
    }

  luma_control_exit:
    *state = (pwmIrisHandle.state != GADI_PWM_IRIS_IDLE);
    GADI_INFO("[%d] lumaDiff %d, duty %d"
        " Kp %d, Ki %d, Kd %d,"
        "data_p %d, data_i %d, data_d %d,"
        "state %d, \n\n",
        run, lumaDiff, pwmIrisHandle.duty,
        pwmIrisPidCoef.pCoef, pwmIrisPidCoef.iCoef,
        pwmIrisPidCoef.dCoef,
        pwmIrisPidCoef.pCoef * pwmIrisHandle.pidData.pData / Kac,
        pwmIrisPidCoef.iCoef * pwmIrisHandle.pidData.iData / Kac,
        pwmIrisPidCoef.dCoef * pwmIrisHandle.pidData.dData / Kac, *state);
    return GADI_OK;
}
#endif

#define MOTOR_PWM_CHANNEL 0 //内核配置使用PWM0通道
#define MOTOR_PWM_MODE    0 //0 普通模式， 1 和视频同步

static GADI_SYS_HandleT pwmHandle[MAX_PWM_CHANNEL] = {NULL};

static GADI_ERR _motor_pwm_init(void)
{
    GADI_ERR retVal;
    retVal = gadi_pwm_init();
    if(retVal != GADI_OK)
	{
        GADI_ERROR("gadi_pwm_init() failed!\n");
		return retVal;
	}

    return retVal;
}

static GADI_ERR _motor_pwm_exit(void)
{
    GADI_ERR retVal;

    retVal = gadi_pwm_exit();
    if(retVal != GADI_OK)
        GADI_ERROR("gadi_pwm_exit() failed!\n");

    return GADI_OK;
}

static GADI_ERR _motor_pwm_start(int channel)
{
    GADI_ERR retVal = GADI_OK;    

    if (pwmHandle[channel] != NULL)
    {
        return GADI_ERR_ALREADY_INITIALIZED;
    }
    
    pwmHandle[channel] = gadi_pwm_open(&retVal, channel);
    if(retVal != GADI_OK || pwmHandle[channel] == NULL)
    {
        GADI_ERROR("gadi_pwm_open() failed\n");
		return retVal;
    }
    retVal = gadi_pwm_set_mode(pwmHandle[channel], MOTOR_PWM_MODE);
    if(retVal != GADI_OK)
    {
        GADI_ERROR("gadi_pwm_get_mode() failed\n");
		return retVal;
    }

    retVal = gadi_pwm_set_duty(pwmHandle[channel], 1);
    if(retVal != GADI_OK)
    {
        GADI_ERROR("gadi_pwm_set_duty() failed\n");
		return retVal;
    }

    retVal = gadi_pwm_start(pwmHandle[channel]);
    if(retVal != GADI_OK)
    {
        GADI_ERROR("gadi_pwm_start() failed\n");
    }

    return retVal;
}

static GADI_ERR _motor_pwm_stop(int channel)
{
    GADI_ERR retVal = GADI_OK;

    if (pwmHandle[channel] == NULL)
    {
        return GADI_ERR_NOT_INITIALIZED;
    }
    
    retVal = gadi_pwm_stop(pwmHandle[channel]);
    if(retVal != GADI_OK)
    {
        GADI_ERROR("gadi_pwm_stop() failed\n");
        return retVal;
    }
    
    retVal = gadi_pwm_close(pwmHandle[channel]);
    if(retVal != GADI_OK)
    {
        GADI_ERROR("gadi_pwm_close() failed\n");
    }
    else
    {
        pwmHandle[channel] = NULL;
    }

    return retVal;
}

int sdk_set_pwm_speed(int channel, unsigned int speed)
{
    GADI_ERR retVal = GADI_OK;

    if (pwmHandle[channel] == NULL)
    {
        return -1;
    }

    retVal = gadi_pwm_set_speed(pwmHandle[channel], speed);
    if(retVal != GADI_OK)
    {
        GADI_ERROR("gadi_pwm_set_speed() failed\n");
    }
    return retVal;
}

unsigned int sdk_get_pwm_speed(int channel)
{
    GADI_ERR retVal = GADI_OK;
    unsigned int speed = 0;

    if (pwmHandle[channel] == NULL)
    {
        return -1;
    }

    retVal = gadi_pwm_get_speed(pwmHandle[channel], &speed);
    if(retVal != GADI_OK)
    {
        GADI_ERROR("gadi_pwm_get_speed() failed\n");
        return speed;
    }

    GADI_INFO("pwm speed : %d\n", speed);
    return speed;
}

int sdk_set_pwm_duty(int channel, unsigned int duty)
{
    GADI_ERR retVal = GADI_OK;

    if (pwmHandle[channel] == NULL)
    {
        return -1;
    }

    retVal = gadi_pwm_set_duty(pwmHandle[channel], duty);
    if(retVal != GADI_OK)
    {
        GADI_ERROR("gadi_pwm_set_duty() failed\n");
    }
    return retVal;
}

unsigned int sdk_get_pwm_duty(int channel)
{
    GADI_ERR retVal = GADI_OK;
    GADI_U32 duty = 0;

    if (pwmHandle[channel] == NULL)
    {
        return -1;
    }

    retVal = gadi_pwm_get_duty(pwmHandle[channel], &duty);
    if(retVal != GADI_OK)
    {
        GADI_ERROR("gadi_pwm_get_duty() failed\n");
        return duty;
    }
    GADI_INFO("pwm duty : %d.%01d %%\n",duty/10, duty%10);
    return duty;
}

int sdk_pwm_init()
{
    return _motor_pwm_init();
}

int sdk_pwm_exit()
{
    int i; 
    for (i = 0 ;i < MAX_PWM_CHANNEL; i++)
    {
        _motor_pwm_stop(i);
    }
    _motor_pwm_exit();
    return 0;
}

int sdk_pwm_start(int channel)
{
    if (pwmFd == -1)
    {
        sdk_pwm_init();
    }
    
    _motor_pwm_start(channel);
    return 0;
}

int sdk_pwm_stop(int channel)
{
    _motor_pwm_stop(channel);
    return 0;
}

int sdk_pwm_get_channel_status(int channel)
{
    if (pwmHandle[channel] != NULL)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


