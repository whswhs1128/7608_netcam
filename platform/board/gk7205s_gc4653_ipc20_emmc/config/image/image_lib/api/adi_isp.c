/*!
*****************************************************************************
** \file        adi/src/adi_isp.c
**
** \brief       ADI implementation for ISP3A.
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2015 by GOKE MICROELECTRONICS CO.,LTD
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
#include <assert.h>
#include <sys/prctl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "adi_types.h"
#include "basetypes.h"
#include "adi_sys.h"
#include "adi_isp.h"

#include "gk_isp3a_api.h"
#include "param_load.h"
#include "tuning.h"
#include "adi_isp.h"

#include "gk_drv.h"
#include "gk_vi.h"
#include "gk_media.h"

#include "isp3a_ae.h"
#include "image_version.h"

// Added by GK7101S
#include "isp3a_proc.h"
// End
//*****************************************************************************
//*****************************************************************************
//** TO BE REMOVED HEADERS !!
//*****************************************************************************
//*****************************************************************************

#include "math.h"
//#define IMAGEDEBUG
//*****************************************************************************
//*****************************************************************************
//** Local Defines
//*****************************************************************************
//*****************************************************************************
#if defined(IMAGEDEBUG)         /* DEBUG is only valid in the debug-mode make process */
#define IMAGE_ERROR(format, args...) printf("[IMAGE ERROR] [%s: %d]" format "\n", __FILE__, __LINE__, ##args)
#define IMAGE_DEBUG(format, args...) printf("[IMAGE DEBUG] " format, ##args)
#define IMAGE_INFO(format, args...)  printf("[IMAGE INFO] " format, ##args)
#else
#define IMAGE_ERROR(format, args...)
#define IMAGE_DEBUG(format, args...)
#define IMAGE_INFO(format, args...)
#endif

#define GADI_MEDIA_DEV              "/dev/gk_video"

/* Only one ISP instance for GK7101 */
#define ISP_HANDLE_MAX_NUM              2

#define ISP_SLOW_SHUTTER_TASK_PRIORITY  3

#define ISP_SLOW_SHUTTER_TASK_STACKSIZE 2048
#define ISP_SLOW_SHUTTER_TASK_NAME      "GADI:ISP_SLOWSHUTTER"

#define ISP_TOTAL_AE_LINES            10

#define GADI_ISP_METERING_TABLE_SIZE            96

#define GADI_ISP_IMAGE_CONTRAST_MIN             0
#define GADI_ISP_IMAGE_CONTRAST_MAX             255

#define GADI_ISP_IMAGE_SATURATION_MIN           0
#define GADI_ISP_IMAGE_SATURATION_MAX           255

#define GADI_ISP_IMAGE_BRIGHTNESS_MIN          -128
#define GADI_ISP_IMAGE_BRIGHTNESS_MAX           127

#define GADI_ISP_IMAGE_SHARPNESS_MIN            0
#define GADI_ISP_IMAGE_SHARPNESS_MAX            255

#define GADI_ISP_IMAGE_HUE_MIN                  0
#define GADI_ISP_IMAGE_HUE_MAX                  255

#define GADI_ISP_IMAGE_DNR3D_STRENGTH_MIN        0
#define GADI_ISP_IMAGE_DNR3D_STRENGTH_MAX        512

#define GADI_ISP_EXPOSURE_CURVE_TABLE_SIZE      256

#define GADI_ISP_BLC_MIN                       -100
#define GADI_ISP_BLC_MAX                        100

#define SHT_TIME(SHT_Q9)    (SHT_Q9 ? DIV_ROUND(512000000, (SHT_Q9)):SHT_Q9)

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

//*****************************************************************************
//*****************************************************************************
//** Local structures
//*****************************************************************************
//*****************************************************************************

/*!
*******************************************************************************
** \brief Image effect attributes.
*******************************************************************************
*/
typedef struct {

    GADI_S32 sharpness;

    GADI_S32 saturation;

    GADI_S32 brightness;

    GADI_S32 hue;

    GADI_U16 localExpCurveTable[GADI_ISP_EXPOSURE_CURVE_TABLE_SIZE];

    GADI_U32 backLightCompEnable;

    GADI_U32 dayNightEnable;

    GADI_S32 extraBlc;

    GADI_ISP_ContrastAttrT contrast;

} GADI_ISP_P_ImageAttrT;

typedef struct {

    /* MEDIA device file descriptor */
    GADI_S32 mediaFd;

    /* whether this ISP handle is in use. */
    GADI_U32 inUse;

    /* whether this ISP engine was started before. */
    GADI_U32 isRunning;

    /* whether ISP3A first started. */
    GADI_U32 isFirstStarted;

    /* denoise mode.GADI_ISP_DenoiseModeEnumT */
    GADI_U32 denoiseMode;

    /* the video width retrieve from the input device associate with this ISP */
    GADI_U32 inputVideoWidth;

    /* the video height retrieve from the input device associate with this ISP */
    GADI_U32 inputVideoHeight;

    /* the factor of sensor slow framerate. */
    GADI_U8 frameRateFactor;

    /* the information of the connected sensor (see also: G_mw_sensor_model) */
    GADI_SENSORS_InfoT sensorInfo;

    GADI_U32 currentviFps;

    /* the antiflicker parameter. */
    GADI_ISP_AntiFlickerParamT antiFlickerParam;

    GADI_ISP_ExpTypeEnumT expType;

    /* current AE attributes. */
    GADI_ISP_AeAttrT aeAttrCur;

    /* local exposure mode. */
    GADI_U32 autoLocalExposureMode;

    /* exposure level value. */
    GADI_U32 exposureLevel;

    GADI_U32 aeSlowShutterEnable;

    pthread_t aeSlowShutterThread;

    /* slow shutter control thread exit flag */
    GADI_U32 aeSlowShutterThreadExit;

    GADI_ISP_MeAttrT meAttrCur;

    GADI_CHAR viIntArray[8];

    GADI_S32 viTickFd;

    line_t aeLines[ISP_TOTAL_AE_LINES];

    GADI_S32 aeLineNum;

    GADI_S32 aeLineBeltDefault;

    GADI_S32 aeLineBeltCurrent;

    GADI_ISP_WbTypeEnumT wbType;

    /* current AWB attributes. */
    GADI_ISP_AwbAttrT awbAttrCur;

    /* current MWB attributes. */
    GADI_ISP_MwbAttrT mwbAttrCur;

    /* current 3A algo API in use. */
    aaa_api_t aaaLibCur;

    /* current image attributes. */
    GADI_ISP_P_ImageAttrT imageAttrCur;

    /* current sharpen attributes. */
    GADI_ISP_SharpenAttrT sharpenAttr;

    /* current cfa denoise attributes. */
    GADI_ISP_DenoiseAttrT denoiseAttr;

     /* current cfa denoise attributes. */
    GADI_ISP_SpatialFilterAttrT SpatialFilterAttr;

    /* current backlight attributes. */
    GADI_ISP_BacklightAttrT backlightAttr;

    /*current dewarp attributes*/
    GADI_ISP_WarpInfoT dewarpAttr;
//added by Hu Yin 08-03-2017
    /*current lowbitrate attributes*/
    GADI_ISP_LowBitrateInfoT  lowbitrateAttr;
//end

    /* ISP control handle */
    GADI_VOID           *isp3AHandle;
} GADI_ISP_P_HandleT;

//*****************************************************************************
//*****************************************************************************
//** Global Data
//*****************************************************************************
//*****************************************************************************

extern u32 SHUT_DATA_TABLE[SHUTTER_TIME_TABLE_LENGTH];

//*****************************************************************************
//*****************************************************************************
//** Local Data
//*****************************************************************************
//*****************************************************************************
static GADI_U32 ispInitialized = 0;
static GADI_ISP_P_HandleT *ispHandleArray[ISP_HANDLE_MAX_NUM] = {NULL, NULL};

static GADI_BOOL gIsDay = 1;
static GADI_BOOL gReload_cfg_flag = 0;
static GADI_INT  gFdMedia = -1;
//*****************************************************************************
//*****************************************************************************
//** Local Functions Declaration
//*****************************************************************************
//*****************************************************************************
static GADI_ISP_P_HandleT *isp_p_allocate_handle(GADI_ERR * errCodePtr);
static void isp_p_release_handle(GADI_ISP_P_HandleT * handlePtr);
static GADI_S32 isp_p_is_rgb_sensor_connect(GADI_ISP_P_HandleT * handlePtr);
static int isp_load_ccm_data (void *isp3AHandle, GADI_ISP_P_HandleT * handlePtr);
static int isp_p_load_adj_cc_table(void *handle, GADI_SENSORS_InfoT * sensorInfoPtr);

static int isp_p_load_dsp_vignette_table(GADI_ISP_P_HandleT * handlePtr);
#if 0
static void isp_ae_slow_shutter_thread(void *optDataPtr);
static GADI_ERR isp_p_ae_create_slowshutter_thread(GADI_ISP_P_HandleT *
    ispHandlePtr);
static GADI_ERR isp_p_ae_destroy_slowshutter_thread(GADI_ISP_P_HandleT *
    ispHandlePtr);
#endif

//*****************************************************************************
//*****************************************************************************
//** inline Functions
//*****************************************************************************
//*****************************************************************************

static inline GADI_ERR isp_p_ioc_get_frame_rate(GADI_ISP_P_HandleT * handlePtr,
    GADI_U32 * frameTime)
{
    GK_VI_SRC_FrameRateT fmtime;

    fmtime.srcid = gk_get_3a_srcid(handlePtr->isp3AHandle);// TODO: Add src ID
    if (ioctl(handlePtr->mediaFd, GK_MEDIA_IOC_VI_SRC_GET_FRAME_RATE,
            &fmtime) < 0) {
        IMAGE_ERROR("get vi source frame rate failed.\n");
        return GADI_ISP_ERR_FROM_DRIVER;
    }

    *frameTime = fmtime.fps;

    return GADI_OK;
}

static inline GADI_ERR isp_p_ioc_update_frame_rate(GADI_ISP_P_HandleT *
    handlePtr)
{
    if (ioctl(handlePtr->mediaFd, GK_MEDIA_IOC_UPDATE_VI_FRAMERATE_EX, 0) < 0) {
        IMAGE_ERROR("update vi frame rate failed.\n");
        return GADI_ISP_ERR_FROM_DRIVER;
    }

    return GADI_OK;
}

static inline GADI_ERR isp_p_ioc_set_framerate(GADI_ISP_P_HandleT * handlePtr,
    u32 frameTime)
{
    GK_VI_SRC_FrameRateT frameRate;

    assert(handlePtr->viTickFd > 0);
    read(handlePtr->viTickFd, handlePtr->viIntArray, 8);

    frameRate.srcid = gk_get_3a_srcid(handlePtr->isp3AHandle);// TODO: Add src ID
    frameRate.fps = frameTime;
    if (ioctl(handlePtr->mediaFd, GK_MEDIA_IOC_VI_SRC_SET_FRAME_RATE,
            &frameRate) < 0) {
        IMAGE_ERROR("set vi source frame rate filed.\n");
        return GADI_ISP_ERR_FROM_DRIVER;
    }

    return GADI_OK;
}

static inline int isp_i_print_ae_path(line_t * lines,
    int line_num, int line_belt, int enable_convert)
{
    int i;

    IMAGE_INFO("Generated AE paths\n");
    for (i = 0; i < line_num; ++i) {
        if (i == line_belt) {
            IMAGE_INFO("  Display the path belt:\n");
        }
        if (enable_convert) {
            IMAGE_INFO(" [%d] begin (1/%d, %d, %d) == end (1/%d, %d, %d)\n",
                (i + 1), SHT_TIME(lines[i].start.factor[CHG_SHUTTER]),
                lines[i].start.factor[CHG_GAIN], lines[i].start.factor[CHG_APERTURE],
                SHT_TIME(lines[i].end.factor[CHG_SHUTTER]),
                lines[i].end.factor[CHG_GAIN], lines[i].end.factor[CHG_APERTURE]);
        } else {
            IMAGE_INFO(" [%d] begin (%d, %d, %d) == end (%d, %d, %d)\n",
                (i + 1), lines[i].start.factor[CHG_SHUTTER],
                lines[i].start.factor[CHG_GAIN], lines[i].start.factor[CHG_APERTURE],
                lines[i].end.factor[CHG_SHUTTER], lines[i].end.factor[CHG_GAIN],
                lines[i].end.factor[CHG_APERTURE]);
        }
    }
    IMAGE_INFO("PathCount [%d] PathBelt [%d]\n\n", line_num, line_belt);
    return 0;
}

static inline s32 af_pulse_interpolate(s32 lv, s32 hv, u8 li, u8 hi, float look)
{
    float rval;

    rval = ((hi - look) * lv + (look - li) * hv) / (hi - li);
    return (s32) rval;
}


//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************

GADI_ERR gadi_isp_init(void)
{
    SYS_DRV_MmapInfoT mapDspInfo;

    /* print version info */
    printf("\r%s\n", IMAGE_BANNER);

    if (ispInitialized) {
        return GADI_ISP_ERR_ALREADY_INITIALIZED;
    }

    /*open media dev */
    if ((gFdMedia = open(GADI_MEDIA_DEV, O_RDWR, 0)) < 0) {
        GADI_ERROR("open  driver failed.\n");
        goto exit;
    }

    if (ioctl(gFdMedia, GK_MEDIA_IOC_MAP_DSP,
        &mapDspInfo) < 0) {
        GADI_ERROR("Map DSP memroy failed.\n");
        goto exit_closefd;
    }

    if (gk_lib_init() < 0) {
        GADI_ERROR("gk_lib_init failed.\n");
        goto exit_undsp;
    }

    ispInitialized = 1;

    return GADI_OK;

exit_undsp:
    ioctl(gFdMedia, GK_MEDIA_IOC_UNMAP_DSP);
exit_closefd:
    close(gFdMedia);
    gFdMedia = -1;
exit:
    return GADI_ISP_ERR_NOT_OPEN;
}

GADI_ERR gadi_isp_exit(void)
{
    /* stop instance. */

    /* close instance. */
    if (ispInitialized == 0)
        return GADI_ISP_ERR_NOT_INITIALIZED;

    gk_lib_deinit();

    ioctl(gFdMedia, GK_MEDIA_IOC_UNMAP_DSP);
    close(gFdMedia);
    gFdMedia = -1;

    ispInitialized = 0;

    return GADI_OK;
}

GADI_SYS_HandleT gadi_isp_open(GADI_ISP_OpenParamsT * openParamPtr,
    GADI_ERR * errorCodePtr)
{
    GADI_ERR errorCode = GADI_OK;
    GADI_ISP_P_HandleT *ispHandlePtr = NULL;

    /*check input parameters.*/
    if((errorCodePtr == NULL) || (openParamPtr == NULL))
    {
        return NULL;
    }

    /*check open parameters.*/
    if(openParamPtr->ispChnID >= GADI_ISP_CHN_NUMBER){
        *errorCodePtr = GADI_ISP_ERR_BAD_PARAMETER;
        return NULL;
    }

    ispHandlePtr = isp_p_allocate_handle(&errorCode);
    if (ispHandlePtr == NULL || errorCode != GADI_OK) {
        *errorCodePtr = errorCode;
        return NULL;
    }

    ispHandlePtr->isFirstStarted = 1;

    /* enable slow shutter by default. */
    ispHandlePtr->aeSlowShutterEnable = 1;

    /* set default value */
    ispHandlePtr->awbAttrCur.speed = GADI_AWB_AUTO_SPEED;
    ispHandlePtr->awbAttrCur.colorTemp = GADI_ISP_AWB_COLOR_TEMP_AUTO;
    ispHandlePtr->awbAttrCur.algo = GADI_ISP_AWB_ALGO_AUTO;

    ispHandlePtr->imageAttrCur.contrast.enableAuto = 0;

    /*set default value. */
    ispHandlePtr->imageAttrCur.saturation = 64;
    ispHandlePtr->imageAttrCur.brightness = 0;
    ispHandlePtr->imageAttrCur.hue = 128;

    ispHandlePtr->aeAttrCur.tagetRatio = 0x80;  // default value is 0x80

    /*set default value. */
    ispHandlePtr->SpatialFilterAttr.enableDisable= 1;
    ispHandlePtr->SpatialFilterAttr.spatial_thr_dir= 64;
    ispHandlePtr->SpatialFilterAttr.spatial_thr_flat= 64;
    ispHandlePtr->SpatialFilterAttr.spatial_weight_dir= 64;
    ispHandlePtr->SpatialFilterAttr.sptial_weight_flat= 64;
    ispHandlePtr->SpatialFilterAttr.edge_thr= 64;


    /*set default value. */
    ispHandlePtr->sharpenAttr.enableDisable= 1;
    ispHandlePtr->sharpenAttr.enableManual= 0;
    ispHandlePtr->sharpenAttr.level= 0x80;
    ispHandlePtr->sharpenAttr.flat_strength= 64;
    ispHandlePtr->sharpenAttr.dir_strength= 64;
    ispHandlePtr->sharpenAttr.edge_th= 64;
    ispHandlePtr->sharpenAttr.max_change_down= 64;
    ispHandlePtr->sharpenAttr.max_change_up= 64;


    /*set default value. */
    ispHandlePtr->denoiseAttr.enableDisable= 1;
    ispHandlePtr->denoiseAttr.level= 64;
    ispHandlePtr->denoiseAttr.thr_level= 64;
    ispHandlePtr->denoiseAttr.max_change_level= 64;

    /*set default value. */
    ispHandlePtr->imageAttrCur.contrast.enableAuto= 1;
    ispHandlePtr->imageAttrCur.contrast.autoStrength= 64;
    ispHandlePtr->imageAttrCur.contrast.manualStrength= 128;

    ispHandlePtr->isp3AHandle = gk_lib_create_ISP3AProcesser(ispHandlePtr->mediaFd,
                                    openParamPtr->ispChnID);
    if (ispHandlePtr->isp3AHandle == NULL) {
        *errorCodePtr = errorCode;
        GADI_ERROR("gk_lib_create_ISP3AProcesser failed.\n");
        isp_p_release_handle(ispHandlePtr);
        return NULL;
    }

    IMAGE_INFO("isp: isp3a library init done\n");

    return (GADI_SYS_HandleT) ispHandlePtr;
}

GADI_ERR gadi_isp_close(GADI_SYS_HandleT ispHandle)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;

    if (ispHandlePtr == NULL) {
        return GADI_ISP_ERR_NOT_OPEN;
    }

    gk_lib_destroy_ISP3AProcesser(ispHandlePtr->isp3AHandle);

    isp_p_release_handle(ispHandlePtr);

    return GADI_OK;
}

GADI_ERR gadi_isp_load_param(GADI_SYS_HandleT ispHandle, const char *paramFile)
{
    GADI_ERR errorCode = GADI_OK;
    GADI_U32 driverState;
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    warp_config_t warp_config;

    if (ispHandlePtr == NULL) {
        return GADI_ISP_ERR_NOT_OPEN;
    }

    if (ispHandlePtr->isRunning) {
        IMAGE_INFO("%s, %d, isRunning:%d\n", __func__, __LINE__, ispHandlePtr->isRunning);
        return GADI_OK;
    }

    errorCode = ioctl(ispHandlePtr->mediaFd,
        GK_MEDIA_IOC_GET_STATE, &driverState);
    if (errorCode != GADI_OK) {
        return errorCode;
    }

    if (driverState == MEDIA_STATE_IDLE || driverState == MEDIA_STATE_INIT) {
        IMAGE_ERROR("isp: state wrong, current state %d\n", driverState);
        return GADI_ISP_ERR_DEVICE_BUSY;

    }

    errorCode = isp_p_is_rgb_sensor_connect(ispHandlePtr);
    if (errorCode < 0) {
        IMAGE_ERROR("no sensor connected \n");
        return errorCode;
    }

    if (errorCode == 0) {
        IMAGE_INFO("This Sensor Not Support ISP.\n");
        return GADI_ISP_ERR_FEATURE_NOT_SUPPORTED;
    }

    if (gadi_sensors_get_3a_parameters(ispHandlePtr->isp3AHandle, &ispHandlePtr->sensorInfo) < 0) {
        IMAGE_ERROR("get sensor's 3A parameters error\n");
        return GADI_ISP_ERR_FROM_DRIVER;
    }

    IMAGE_INFO
        ("isp: sensor id[0x%08X] name[%s] def_fps[%d] cur_fps[%d] op_mode[%d]\n",
        ispHandlePtr->sensorInfo.sensor_id,
        ispHandlePtr->sensorInfo.sensor_name,
        ispHandlePtr->sensorInfo.default_fps,
        ispHandlePtr->sensorInfo.current_fps,
        ispHandlePtr->sensorInfo.sensor_op_mode);

    /* in unit of GK_VIDEO_FPS_30 like */
    ispHandlePtr->currentviFps = ispHandlePtr->sensorInfo.current_fps;

    if (gk_config_sensor_info(ispHandlePtr->isp3AHandle) < 0) {
        IMAGE_ERROR("isp: gk_config_sensor_info error!\n");
        return GADI_ISP_ERR_FROM_DRIVER;
    }





    /*if (ispHandlePtr->sensorInfo.sensor_id == AR_0331) {
        img_config_sensor_hdr_mode(ispHandlePtr->sensorInfo.sensor_op_mode);
    }*/

    /* load bad pixel correction data. */

    /* load lens shading data. */
    //added by Hu Yin 06-01-2016
    warp_config.warp_control = ispHandlePtr->sensorInfo.sensor_param.p_warp_config->warp_control;
    warp_config.warp_multiplier = ispHandlePtr->sensorInfo.sensor_param.p_warp_config->warp_multiplier;

    img_dsp_set_dewarp(ispHandlePtr->isp3AHandle, &warp_config);
    //end

    /* load color correction registers value. */
    /* load color correction 3D matrix table. */
    if (isp_load_ccm_data (ispHandlePtr->isp3AHandle, ispHandlePtr) < 0) {
        return GADI_ISP_ERR_FROM_DRIVER;
    }

    /* load color correction adjust table. */
    if (isp_p_load_adj_cc_table(ispHandlePtr->isp3AHandle, &ispHandlePtr->sensorInfo) < 0) {
        return GADI_ISP_ERR_FROM_DRIVER;
    }

    IMAGE_INFO("isp: load cc table done\n");

    /* load vig-to-awb calibration data. */
    if (ispHandlePtr->sensorInfo.sensor_param.p_lens_shading->enable != 0) {
        if (isp_p_load_dsp_vignette_table(ispHandlePtr) < 0) {
            return GADI_ISP_ERR_FROM_DRIVER;
        }
    }

    IMAGE_INFO("isp: load lens shading done\n");

    /*  release sensor parameter if allocate it */
    gadi_sensor_free_parameter_buf(ispHandlePtr->isp3AHandle);

    return GADI_OK;
}

GADI_ERR gadi_isp_config_lens(GADI_SYS_HandleT ispHandle, u32 lensID)
{
	Isp3AContentAF *content;
    Isp3AProcessor *processor;
	GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    processor = ispHandlePtr->isp3AHandle;
    content = &(processor->content.AFData);
	content->lensID = lensID;
	if (gk_config_lens_info(ispHandlePtr->isp3AHandle) < 0) {
        IMAGE_ERROR("isp: gk_config_lens_info error!\n");
        return GADI_ISP_ERR_FROM_DRIVER;
    }

    if(gk_lens_init(ispHandlePtr->isp3AHandle)<0)
    {
        IMAGE_ERROR("isp: gk_lens_init error!\n");
        return GADI_ISP_ERR_FROM_DRIVER;
    }
	return GADI_OK;
}

GADI_ERR gadi_isp_set_af_mode(GADI_SYS_HandleT ispHandle, u8 mode, void(*pgetLensParams)(s32* pLensParams, u16 length))
{
    Isp3AContentAF *content;
    Isp3AProcessor *processor;
	GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    processor = ispHandlePtr->isp3AHandle;
    content = &(processor->content.AFData);
	content->calib_mode = mode;
    content->pgetLensParams = pgetLensParams;
	return GADI_OK;
}

GADI_ERR gadi_isp_set_lens_params(GADI_SYS_HandleT ispHandle, s32* pLensParams, u16 length)
{
    int i = 0;
    Isp3AContentAF *content;
    Isp3AProcessor *processor;
	GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    processor = ispHandlePtr->isp3AHandle;
    content = &(processor->content.AFData);
	if (pLensParams != NULL && length != 0)
    {
        memcpy(content->calib_saf_out.c, pLensParams, sizeof(s32) * length);
        content->calib_saf_out.length = length;
    }
	return GADI_OK;
}



GADI_ERR gadi_isp_start(GADI_SYS_HandleT ispHandle)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    GADI_ISP_ContrastAttrT contrastAttr;
    //image_low_bitrate_property_t image_low_bitrate;
    //u8  tolerance_low_bitrate;
    //int delay_frames_low_bitrate;
    GADI_ISP_LowBitrateInfoT lowbitrateAttr;

    if (ispHandlePtr->isRunning == 1) {
        IMAGE_INFO("%s, %d, isRunning:%d\n", __func__, __LINE__, ispHandlePtr->isRunning);
        return GADI_OK;
    }

    /* start 3A. */
    if (gk_start_3A(ispHandlePtr->isp3AHandle) < 0) {
        IMAGE_ERROR("isp: img_start_aaa error\n");
        return GADI_ISP_ERR_FROM_DRIVER;
    }

    //gk_set_auto_contrast(1);
    //gk_set_auto_contrast_strength(48);
    //aeTagetRatio = ae_get_target_ratio();
    //IMAGE_INFO("isp: aeTagetRatio = %d\n", aeTagetRatio);

    //ae_set_target_ratio(96);

    //gk_set_auto_contrast(1);
    //gk_set_auto_contrast_strength(64);
    //gk_set_auto_contrast(1);
    //gk_set_auto_contrast_strength(64);

    /*enalbe auto contrast defaultly.*/
    contrastAttr.enableAuto     = 1;
    contrastAttr.manualStrength = 128;
    contrastAttr.autoStrength   = 128;
    gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);

#if 0                           /*do not create slowshutter thread. */
    /* TODO: create slowshutter task. */
    if (ispHandlePtr->isFirstStarted) {
        errorCode = isp_p_ae_create_slowshutter_thread(ispHandlePtr);
        if (errorCode != GADI_OK) {
            IMAGE_ERROR("isp: create ss thread failed %d\n", errorCode);
            return errorCode;
        }
        ispHandlePtr->isFirstStarted = 0;
    }
#endif

//added by Hu Yin 08-03-2017
    gadi_isp_get_lowbitrate_attr(ispHandle, &lowbitrateAttr);

    IMAGE_INFO("isp: low bitrate mode %d\n", lowbitrateAttr.enable);
    IMAGE_INFO("isp: tolerance %d, delay_frames %d\n",
                lowbitrateAttr.tolerance, lowbitrateAttr.delay_frames);
#if 0
    lowbitrateAttr.enable = 1;
    lowbitrateAttr.tolerance = 8;
    lowbitrateAttr.delay_frames  = 15;
    gadi_isp_set_lowbitrate_attr(ispHandle, &lowbitrateAttr);
#endif
//end
    ispHandlePtr->isRunning = 1;

    IMAGE_INFO("isp: start isp3a done\n");

    return GADI_OK;
}

GADI_ERR gadi_isp_stop(GADI_SYS_HandleT ispHandle)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;

    if (!ispHandlePtr->isRunning) {
        return GADI_OK;
    }

    /* disable AE */
    gk_enable_ae(ispHandlePtr->isp3AHandle, 0);

    /* destroy slow shutter */
    //isp_p_ae_destroy_slowshutter_thread(ispHandlePtr);

    /* stop 3A */
    gk_stop_3A(ispHandlePtr->isp3AHandle);

    ispHandlePtr->isRunning = 0;

    return GADI_OK;
}

GADI_ERR gadi_isp_set_exposure_type(GADI_SYS_HandleT ispHandle,
    GADI_ISP_ExpTypeEnumT type)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    GADI_ERR errorCode = GADI_OK;

    if (ispHandlePtr->expType != type) {
        /* TODO: change the exposure type. */
        if (type == GADI_ISP_EXPOSURE_TYPE_AUTO) {
            errorCode = gk_enable_ae(ispHandlePtr->isp3AHandle, 1);
        } else {
            errorCode = gk_enable_ae(ispHandlePtr->isp3AHandle, 0);

        }
        if (errorCode != GADI_OK) {
            IMAGE_ERROR("ISP: set exposure type to %s failed\n",
                type ? "auto" : "manual");
            return GADI_ISP_ERR_FROM_DRIVER;
        }
    } else {
        return GADI_OK;
    }

    ispHandlePtr->expType = type;

    return errorCode;
}

static inline int generate_manual_shutter_lines(GADI_ISP_AeAttrT * p,
    line_t * lines, int *line_num, int *line_belt)
{
    int total_lines = 0;

    lines[total_lines].start.factor[CHG_SHUTTER] = p->shutterTimeMax;
    lines[total_lines].start.factor[CHG_GAIN] = 0;
    lines[total_lines].start.factor[CHG_APERTURE] = 0;
    lines[total_lines].end.factor[CHG_SHUTTER] = p->shutterTimeMax;
    lines[total_lines].end.factor[CHG_GAIN] = p->gainMax;
    lines[total_lines].end.factor[CHG_APERTURE] = 0;
    ++total_lines;
    *line_num = total_lines;
    *line_belt = total_lines;

#if 0
    isp_i_print_ae_path(lines, total_lines, total_lines, 1);
#endif
    return 0;
}

static int generate_normal_ae_lines(GADI_ISP_P_HandleT *ispHandlePtr, GADI_ISP_AeAttrT * p, line_t * lines,
    int *line_num, int *line_belt)
{
    int dst, src;
    int s_max, s_min, g_max;
    line_t p_src[10];
    int flicker_off_shutter_time = 0;
    int longest_possible_shutter = 0, curr_belt = 0;

    s_max = p->shutterTimeMax;
    s_min = p->shutterTimeMin;
    g_max = p->gainMax;

    memset(p_src, 0, sizeof(p_src));
    gadi_get_sensor_exposure_lines(ispHandlePtr->isp3AHandle, gadi_get_sensors_id(ispHandlePtr->isp3AHandle),
        ispHandlePtr->antiFlickerParam.freq, p_src);

    flicker_off_shutter_time = (ispHandlePtr->antiFlickerParam.freq ==
        GADI_ISP_ANTIFLICKER_FREQ_60HZ) ? SHUTTER_1BY120_SEC :
        SHUTTER_1BY100_SEC;
    dst = src = curr_belt = 0;
    if (s_min < flicker_off_shutter_time) {
        // create shutter line
        lines[dst] = p_src[src];
        lines[dst].start.factor[CHG_SHUTTER] = s_min;

        if (s_max < flicker_off_shutter_time) {
            lines[dst].end.factor[CHG_SHUTTER] = s_max;
            ++dst;
            lines[dst].start.factor[CHG_SHUTTER] = s_max;
            lines[dst].end.factor[CHG_SHUTTER] = s_max;
            lines[dst].end.factor[CHG_GAIN] = g_max;
            ++dst;
            curr_belt = dst;
            goto GENERATE_LINES_EXIT;
        }
        ++dst;
        ++src;
    } else {
        // create digital gain line
        while (s_min > p_src[src].start.factor[CHG_SHUTTER])
            ++src;
        lines[dst] = p_src[src];

        ++dst;
        ++src;
    }

    // create other lines - digital gain line
    while (s_max >= p_src[src].start.factor[CHG_SHUTTER]) {
        if (p_src[src].start.factor[CHG_SHUTTER] == SHUTTER_INVALID)
            break;

        lines[dst] = p_src[src];
        ++dst;
        ++src;
    }
    lines[dst - 1].end.factor[CHG_GAIN] = g_max;

    // calculate line belt of current and default
    curr_belt = dst;
    while (1) {
        longest_possible_shutter = lines[curr_belt - 1].end.factor[CHG_SHUTTER];
        if (longest_possible_shutter <= ispHandlePtr->sensorInfo.default_fps) {
            ispHandlePtr->aeLineBeltDefault = curr_belt;
        }
        if (ispHandlePtr->aeSlowShutterEnable) {
            if (longest_possible_shutter <=
                ispHandlePtr->sensorInfo.current_fps) {
                IMAGE_INFO("\tSlow shutter is enabled, set curr_belt [%d] to"
                    " current frame rate [%d].\n", curr_belt,
                    SHT_TIME(ispHandlePtr->sensorInfo.current_fps));
                break;
            }
        } else {
            if (longest_possible_shutter <=
                ispHandlePtr->sensorInfo.default_fps) {
                lines[curr_belt - 1].end.factor[CHG_GAIN] = g_max;
                IMAGE_INFO
                    ("\tSlow shutter is disabled, restore curr_belt [%d] to"
                    " default frame rate [%d].\n", curr_belt,
                    SHT_TIME(ispHandlePtr->sensorInfo.default_fps));
                break;
            }
        }
        --curr_belt;
        IMAGE_INFO
            ("\t\t\t=== curr_belt [%d] def_belt [%d] == VI [%d fps] == \n",
            curr_belt, ispHandlePtr->aeLineBeltDefault,
            SHT_TIME(ispHandlePtr->sensorInfo.current_fps));
    }

  GENERATE_LINES_EXIT:
    *line_num = dst;
    *line_belt = curr_belt;

#if 0
    isp_i_print_ae_path(lines, dst, curr_belt, 1);
#endif
    return 0;
}

/* the arr is in reverse order */
static int search_nearest(u32 key, u32 arr[], int size)
{
    int l = 0;
    int r = size - 1;
    int m = (l + r) / 2;

    while (1) {
        if (l == r)
            return l;
        if (key > arr[m]) {
            r = m;
        } else if (key < arr[m]) {
            l = m + 1;
        } else {
            return m;
        }
        m = (l + r) / 2;
    }
    return -1;
}

static int generate_ae_lines(GADI_ISP_P_HandleT *ispHandle, GADI_ISP_AeAttrT * aeAttrPtr,
    line_t * lines, int *line_num, int *line_belt)
{
    int retv = 0;

    if (aeAttrPtr->shutterTimeMax != aeAttrPtr->shutterTimeMin)
        retv = generate_normal_ae_lines(ispHandle, aeAttrPtr, lines, line_num, line_belt);
    else
        retv =
            generate_manual_shutter_lines(aeAttrPtr, lines, line_num,
            line_belt);

    return retv;
}

static int shutter_q9_to_index(GADI_U32 shutter_time)
{
    int tmp_idx;

    tmp_idx = search_nearest(shutter_time,
        SHUT_DATA_TABLE, SHUTTER_TIME_TABLE_LENGTH);
    return (SHUTTER_TIME_TABLE_LENGTH - tmp_idx);
}

static FINLINE double shutter_index_to_shutter(GADI_U32 shutter_index)
{
    if (shutter_index >=
        sizeof(SHUT_DATA_TABLE) / sizeof(SHUT_DATA_TABLE[0])) {
        return 0;
    }
    //shutter (ms) = (SHUT_DATA_TABLE[shutter_index] / 512000000) * 1000;
    return (SHUT_DATA_TABLE[shutter_index] / 512000.0);
}

static FINLINE GADI_U32 gain_index_to_gain(GADI_U32 gain_index)
{
    return (gain_index * 6 / 128);
}

static int load_ae_lines(void *isphandle, line_t * line, u16 line_num, u16 line_belt)
{
    int i;
    static line_t img_ae_lines[ISP_TOTAL_AE_LINES];

    memcpy(&img_ae_lines[0], line, sizeof(line_t) * line_num);

    //transfer q9 format to shutter index format
    for (i = 0; i < line_num; i++) {
        img_ae_lines[i].start.factor[CHG_SHUTTER]
            = shutter_q9_to_index(line[i].start.factor[CHG_SHUTTER]);

        img_ae_lines[i].end.factor[CHG_SHUTTER]
            = shutter_q9_to_index(line[i].end.factor[CHG_SHUTTER]);
    }

#if 0
    IMAGE_INFO("=== Convert shutter time to shutter index ===\n");
    isp_i_print_ae_path(img_ae_lines, line_num, line_belt, 0);
#endif

    if (gk_ae_load_exp_params(isphandle, img_ae_lines, line_num, line_belt) < 0) {
        IMAGE_ERROR
            ("[gk_ae_load_exp_params error] : line_num [%d] line_belt [%d].\n",
            line_num, line_belt);
        return -1;
    }

    return 0;
}

static int load_ae_exp_lines(GADI_ISP_P_HandleT * ispHandlePtr,
    GADI_ISP_AeAttrT * aeAttrPtr)
{
    memset(ispHandlePtr->aeLines, 0, sizeof(ispHandlePtr->aeLines));

    if (aeAttrPtr->shutterTimeMax < aeAttrPtr->shutterTimeMin) {
        IMAGE_INFO
            ("shutter limit max [%d] is less than shutter min [%d]. Tie them to shutter min\n",
            aeAttrPtr->shutterTimeMax, aeAttrPtr->shutterTimeMin);
        aeAttrPtr->shutterTimeMax = aeAttrPtr->shutterTimeMin;
    }

    if (generate_ae_lines(ispHandlePtr, aeAttrPtr, ispHandlePtr->aeLines,
            &ispHandlePtr->aeLineNum, &ispHandlePtr->aeLineBeltCurrent) < 0) {
        IMAGE_ERROR("generate_ae_lines error\n");
        return -1;
    }

    if (load_ae_lines(ispHandlePtr->isp3AHandle, ispHandlePtr->aeLines, ispHandlePtr->aeLineNum,
            ispHandlePtr->aeLineBeltCurrent) < 0) {
        IMAGE_ERROR("load_ae_lines error! line_num [%d], line_belt [%d]\n",
            ispHandlePtr->aeLineNum, ispHandlePtr->aeLineBeltCurrent);
        return -1;
    }

    return 0;
}

GADI_ERR isp_p_set_ae_attr(GADI_ISP_P_HandleT * ispHandlePtr,
    GADI_ISP_AeAttrT * attrPtr)
{
    GK_VI_SRC_FrameRateT frameReate;
    GADI_ISP_AeAttrT *curAeAttrPtr = &ispHandlePtr->aeAttrCur;

    GADI_SENSORS_InfoT *sensorInfoPtr = &ispHandlePtr->sensorInfo;

    /* get current vi fps */
    frameReate.srcid = gk_get_3a_srcid(ispHandlePtr->isp3AHandle);// TODO: Add src ID
    if (ioctl(ispHandlePtr->mediaFd, GK_MEDIA_IOC_VI_SRC_GET_FRAME_RATE,
            &frameReate) < 0) {
        IMAGE_ERROR("get vi source frame rate failed.\n");
        return GADI_ERR_FROM_DRIVER;
    }

    if (ispHandlePtr->aeSlowShutterEnable) {
        if (frameReate.fps != 0) {
            if(attrPtr->shutterTimeMax >= frameReate.fps){
                attrPtr->shutterTimeMax = frameReate.fps;
            }
            sensorInfoPtr->current_fps = frameReate.fps;
        } else {
            attrPtr->shutterTimeMax = sensorInfoPtr->current_fps;
            /* attrPtr->current_vi_fps  = ispHandlePtr.currentviFps; */
        }
    } else {
        /* If slow shutter task is disabled, sensor frame rate will keep
         * the "initial" value when 3A process starts. Any attempt to
         * change sensor frame rate will be restored to "initial" value.
         */
        /*
         * pAe_param->shutter_time_max =  ispHandlePtr.current_vi_fps;
         * pAe_param->current_vi_fps  = ispHandlePtr.current_vi_fps;
         */
    }

    if (attrPtr->shutterTimeMax > frameReate.fps) {
        IMAGE_ERROR("Shutter time min [%d] is longer than VI frametime [%d]."
            " Ignore this change!\n", attrPtr->shutterTimeMax, frameReate.fps);
        return GADI_ISP_ERR_FROM_DRIVER;
    }

    /* Disable and re-enable dc iris when loading ae line. */
#if 0                           /* T.B.D */
    if (G_dc_iris_control_enable) {
        if (enable_dc_iris(0) < 0) {
            MW_ERROR("dc_iris_enable error\n");
            return -1;
        }
    }
#endif

    if (load_ae_exp_lines(ispHandlePtr, attrPtr) < 0) {
        IMAGE_ERROR("load_ae_exp_lines error\n");
        return GADI_ISP_ERR_FROM_DRIVER;
    }
    *curAeAttrPtr = *attrPtr;

    /* re-enable dc iris. */
#if 0                           /* T.B.D */
    if (G_dc_iris_control_enable) {
        if (enable_dc_iris(1) < 0) {
            MW_ERROR("dc_iris_enable error\n");
            return -1;
        }
    }
#endif

    return GADI_OK;
}

GADI_U16 gadi_isp_get_y_result(GADI_SYS_HandleT * ispHandle)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    //return ae_get_rgb_luma_value(void);
    return gk_get_ae_cfa_luma_value(ispHandlePtr->isp3AHandle);
}

/*!
*******************************************************************************
** \brief
**
** (addition description is here)
**
** \param[in]  parameter_name   (parameter description)
** \param[out] parameter_name   (parameter description)
**
**
** \return
** - #RETURN_VALUE_A (return value description)
** - #RETURN_VALUE_B (return value description)
**
** \sa
** - see_also_symbol
*******************************************************************************
*/

//add by heqi
GADI_ERR gadi_isp_set_auto_luma_control(GADI_SYS_HandleT ispHandle, GADI_U8 auto_luma_cntl)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    gk_set_ae_auto_luma_control(ispHandlePtr->isp3AHandle, auto_luma_cntl);
    return GADI_OK;
}

GADI_ERR gadi_isp_set_ae_speed(GADI_SYS_HandleT ispHandle, u8 ae_speed_level)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    int ae_mode;

    ae_mode = 1;
    return (gk_set_ae_speed(ispHandlePtr->isp3AHandle, ae_mode, ae_speed_level));

}

GADI_ERR gadi_isp_set_awb_speed(GADI_SYS_HandleT ispHandle, u8 awb_speed)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    return (gk_awb_set_speed(ispHandlePtr->isp3AHandle, awb_speed));

}

//end
GADI_ERR gadi_isp_set_ae_attr(GADI_SYS_HandleT ispHandle,
    GADI_ISP_AeAttrT * attrPtr)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    GADI_ISP_AeAttrT *curAeAttrPtr = &ispHandlePtr->aeAttrCur;
    GADI_ERR errorCode = GADI_OK;

    /* default AE mode defined in isp3a_ae.c ae_mode. */
    GADI_U8 aeMode = IMBA_MODE;
    GADI_ISP_AeAttrT realAeAttr;
    GADI_U32 aeTarget;

    if (ispHandlePtr == NULL || attrPtr == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    if (ispHandlePtr->expType != GADI_ISP_EXPOSURE_TYPE_AUTO) {
        IMAGE_ERROR("change exposure type before set AE attributes\n");
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    memset(&realAeAttr, 0, sizeof(GADI_ISP_AeAttrT));

    IMAGE_INFO
        ("gadi_isp_set_ae_attr: Customer: shutterMax[%09d] shutterMin[%09d] gainMax[%02d] gainMin[%02d] speed[%d]\n",
        attrPtr->shutterTimeMax, attrPtr->shutterTimeMin, attrPtr->gainMax,
        attrPtr->gainMin, attrPtr->speed);

    realAeAttr = *attrPtr;

    if (realAeAttr.speed != curAeAttrPtr->speed) {
        /* for IMBA_MODE, the speed counts from 0-7.
         * for NORMAL_MODE, the speed does not work.
         */
        if (realAeAttr.speed > 10) {    // 20160725 LYX add 3 AE change step level
            IMAGE_ERROR("AE speed[%d] exceed the limits\n", realAeAttr.speed);
            return GADI_ISP_ERR_BAD_PARAMETER;
        }
        errorCode = gk_set_ae_speed(ispHandlePtr->isp3AHandle, aeMode, attrPtr->speed);
        if (errorCode != GADI_OK) {
            IMAGE_ERROR("set AE speed failed\n");
            return GADI_ISP_ERR_FROM_DRIVER;
        }

        curAeAttrPtr->speed = realAeAttr.speed;
    }
    /*
     * set ae target range form 1024-255 to 1024+255 ;
     */

    aeTarget = 1024 + (((int) attrPtr->tagetRatio - 0x80) * 2);

    //printf("aetarget:%d,radio:%d\n",aeTarget,attrPtr->tagetRatio);
    if (curAeAttrPtr->tagetRatio != attrPtr->tagetRatio) {
        errorCode = gk_ae_set_target_param(ispHandlePtr->isp3AHandle, aeTarget);
        if (errorCode == GADI_OK) {
            curAeAttrPtr->tagetRatio = attrPtr->tagetRatio;
        } else {
            IMAGE_ERROR("set AE target ratio failed\n");
            return GADI_ISP_ERR_FROM_DRIVER;
        }
    }

    if (attrPtr->gainMax != curAeAttrPtr->gainMax ||
        attrPtr->gainMin != curAeAttrPtr->gainMin ||
        attrPtr->shutterTimeMax != curAeAttrPtr->shutterTimeMax ||
        attrPtr->shutterTimeMin != curAeAttrPtr->shutterTimeMin) {

        realAeAttr.shutterTimeMax =
            (realAeAttr.shutterTimeMax != 0) ?
            DIV_ROUND(512000000, realAeAttr.shutterTimeMax) : 0;
        realAeAttr.shutterTimeMin =
            (realAeAttr.shutterTimeMin != 0) ?
            DIV_ROUND(512000000, realAeAttr.shutterTimeMin) : 0;

        IMAGE_INFO
            ("gadi_isp_set_ae_attr: Real    : shutterMax[%09d] shutterMin[%09d] gainMax[%02d] gainMin[%02d] speed[%d]\n",
            SHT_TIME(realAeAttr.shutterTimeMax),
            SHT_TIME(realAeAttr.shutterTimeMin), realAeAttr.gainMax,
            realAeAttr.gainMin, realAeAttr.speed);

        IMAGE_INFO
            ("gadi_isp_set_ae_attr: Prev    : shutterMax[%09d] shutterMin[%09d] gainMax[%02d] gainMin[%02d] speed[%d]\n",
            SHT_TIME(curAeAttrPtr->shutterTimeMax),
            SHT_TIME(curAeAttrPtr->shutterTimeMin), curAeAttrPtr->gainMax,
            curAeAttrPtr->gainMin, curAeAttrPtr->speed);
        IMAGE_INFO("call isp_p_set_ae_attr\n");
        errorCode = isp_p_set_ae_attr(ispHandlePtr, &realAeAttr);
        if (errorCode == GADI_OK) {
            *curAeAttrPtr = *attrPtr;
        }
    }

    IMAGE_INFO("gadi_isp_set_ae_attr: done[%d]\n", errorCode);

    return errorCode;
}

GADI_ERR gadi_isp_get_ae_attr(GADI_SYS_HandleT ispHandle,
    GADI_ISP_AeAttrT * attrPtr)
{

    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;

    if (ispHandlePtr == NULL || attrPtr == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    *attrPtr = ispHandlePtr->aeAttrCur;

    return GADI_OK;
}

GADI_ERR gadi_isp_set_ae_lib(GADI_SYS_HandleT ispHandle,
    GADI_ISP_AeLibT customAeLib)
{

#if 0
    aaa_api_t 3 a_lib;

    gk_register_3A_algorithm(custom_aaa_api);
#endif

    /* TODO: suspend until isp3a lib refactored. */

    return GADI_OK;
}

GADI_ERR gadi_isp_set_me_attr(GADI_SYS_HandleT ispHandle,
    GADI_ISP_MeAttrT * attrPtr)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    GADI_ISP_MeAttrT *curMeAttrPtr = &ispHandlePtr->meAttrCur;
    GADI_ERR errorCode = GADI_OK;
    GK_VI_SRC_ShutterTimeT shutterTime;
    GK_VI_SRC_AgcDBT agc;

    if (ispHandlePtr == NULL || attrPtr == NULL ||
        attrPtr->shutterTime == 0) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    if (ispHandlePtr->expType == GADI_ISP_EXPOSURE_TYPE_AUTO) {
        IMAGE_ERROR("change exposure type before set ME attributes\n");
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    if (curMeAttrPtr->shutterTime != attrPtr->shutterTime) {
        shutterTime.srcid = gk_get_3a_srcid(ispHandlePtr->isp3AHandle);// TODO: Add src ID
        shutterTime.shutter_time = (512000000) / attrPtr->shutterTime;
        errorCode =
            ioctl(ispHandlePtr->mediaFd, GK_MEDIA_IOC_VI_SRC_SET_SHUTTER_TIME,
            &shutterTime);
        if (errorCode < 0) {
            IMAGE_ERROR("set shutter time failed\n");
            return GADI_ISP_ERR_FROM_DRIVER;
        }
        curMeAttrPtr->shutterTime = attrPtr->shutterTime;
    }

    if (curMeAttrPtr->gain != attrPtr->gain) {
        agc.srcid = gk_get_3a_srcid(ispHandlePtr->isp3AHandle);// TODO: Add src ID
        agc.agc_db = attrPtr->gain << 24;
        errorCode =
            ioctl(ispHandlePtr->mediaFd, GK_MEDIA_IOC_VI_SRC_SET_AGC_DB,
            &agc);
        if (errorCode < 0) {
            IMAGE_ERROR("set gain failed\n");
            return GADI_ISP_ERR_FROM_DRIVER;
        }
        IMAGE_INFO("set agc db[%d]\n", attrPtr->gain);
        curMeAttrPtr->gain = attrPtr->gain;
    }

    return GADI_OK;
}

GADI_ERR gadi_isp_get_me_attr(GADI_SYS_HandleT ispHandle,
    GADI_ISP_MeAttrT * attrPtr)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    GADI_ISP_MeAttrT *curMeAttrPtr = &ispHandlePtr->meAttrCur;

    if (ispHandlePtr == NULL || attrPtr == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    *attrPtr = *curMeAttrPtr;

    return GADI_OK;
}

#if 0
GADI_ERR gadi_isp_set_sensor_shuttle_index(GADI_SYS_HandleT ispHandle,
    GADI_U32 shutterIdx)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;

    if (ispHandlePtr == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    if (gk_set_sensor_shutter_time_index(ispHandlePtr->mediaFd, shutterIdx) < 0) {
        IMAGE_ERROR("gadi_isp_set_sensor_shuttle_index failed\n");
        return GADI_ISP_ERR_FROM_DRIVER;
    }

    return GADI_OK;
}

GADI_ERR gadi_isp_set_sensor_agc_index(GADI_SYS_HandleT ispHandle,
    GADI_U32 agcIdx, GADI_U32 step)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;

    if (ispHandlePtr == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    if (gk_set_sensor_gain_index(ispHandlePtr->mediaFd, agcIdx, step) < 0) {
        IMAGE_ERROR("gadi_isp_set_sensor_agc_index failed\n");
        return GADI_ISP_ERR_FROM_DRIVER;
    }

    return GADI_OK;
}
#endif

/*!
*******************************************************************************
** \brief In our platform, we can change the framerate at sensor side and
**        encoding stage, not sure if we can change the framerate at isp stage.
*******************************************************************************
*/
/*!
*******************************************************************************
** \brief this API will only change the senosr framerate.
**        target sensor framerate = current sensor framerate / (factor >> 4).
**        so the MIN value of factor is 0x10;
*******************************************************************************
*/
GADI_ERR gadi_isp_set_slow_framerate(GADI_SYS_HandleT ispHandle, GADI_U8 factor)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    GK_VI_SRC_FrameRateT frameTime;

    if (ispHandlePtr == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    /*check input framerate factor. */
    if (factor < 0x10) {
        IMAGE_ERROR("input invalid frame rate factor:%d\n", factor);
        return GADI_ISP_ERR_FROM_DRIVER;
    }

    frameTime.srcid = gk_get_3a_srcid(ispHandlePtr->isp3AHandle);// TODO: add src ID
    if (ioctl(ispHandlePtr->mediaFd, GK_MEDIA_IOC_VI_SRC_GET_FRAME_RATE,
            &frameTime) < 0) {
        IMAGE_ERROR("get sensor frame rate failed.\n");
        return GADI_ISP_ERR_FROM_DRIVER;
    }

    if (ispHandlePtr->frameRateFactor == 0) {
        frameTime.fps = frameTime.fps * factor / 0x10;
    } else {
        frameTime.fps = frameTime.fps * factor / ispHandlePtr->frameRateFactor;
    }

    if (ioctl(ispHandlePtr->mediaFd, GK_MEDIA_IOC_VI_SRC_SET_FRAME_RATE,
            &frameTime) < 0) {
        IMAGE_ERROR("set sensor frame rate failed.\n");
        return GADI_ISP_ERR_FROM_DRIVER;
    }

    ispHandlePtr->frameRateFactor = factor;

    ispHandlePtr->sensorInfo.current_fps = GK_VIDEO_FPS_15;
    if (gadi_sensors_get_3a_parameters(ispHandlePtr->isp3AHandle, &ispHandlePtr->sensorInfo) < 0) {
        IMAGE_ERROR("get sensor's 3A parameters error\n");
        return GADI_ISP_ERR_FROM_DRIVER;
    }

    IMAGE_INFO
        ("isp: sensor id[0x%08X] name[%s] def_fps[%d] cur_fps[%d] op_mode[%d]\n",
        ispHandlePtr->sensorInfo.sensor_id,
        ispHandlePtr->sensorInfo.sensor_name,
        ispHandlePtr->sensorInfo.default_fps,
        ispHandlePtr->sensorInfo.current_fps,
        ispHandlePtr->sensorInfo.sensor_op_mode);

    //ispHandlePtr->sensor_param.p_ae_sht_dgain = imx122_ae_sht_dgain;
    //if (gk_config_sensor_info(ispHandlePtr->mediaFd, ispHandlePtr->sensorInfo.sensor_id) < 0)
    //{
    //    IMAGE_ERROR("isp: gk_config_sensor_info error!\n");
    //    return GADI_ISP_ERR_FROM_DRIVER;

    //}

    //if (gk_config_lens_info(LENS_CMOUNT_ID) < 0)
    //{
    //    IMAGE_ERROR("isp: gk_config_lens_info error!\n");
    //    return GADI_ISP_ERR_FROM_DRIVER;
    //}

    //if (ispHandlePtr->sensorInfo.sensor_id == AR_0331)
    //{
    //    img_config_sensor_hdr_mode(ispHandlePtr->sensorInfo.sensor_op_mode);
    //}

    return GADI_OK;
}

GADI_ERR gadi_isp_get_slow_framerate(GADI_SYS_HandleT ispHandle,
    GADI_U8 * factor)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;

    if ((ispHandlePtr == NULL) || (factor == NULL)) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    *factor = ispHandlePtr->frameRateFactor;

    return GADI_OK;
}

GADI_ERR gadi_isp_get_current_shutter(GADI_SYS_HandleT ispHandle,
    double *shutter)
{
    GADI_U32 shutter_index = 0;
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;

    if ((ispHandle == NULL) || (shutter == NULL)) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    shutter_index = (GADI_U32) gk_get_sensor_shutter_time_index(ispHandlePtr->isp3AHandle);
    *shutter = shutter_index_to_shutter(shutter_index);

    return GADI_OK;
}

GADI_ERR gadi_isp_get_current_gain(GADI_SYS_HandleT ispHandle, GADI_U32 * gain)
{
    GADI_U32 gain_index = 0;
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;

    if ((ispHandle == NULL) || (gain == NULL)) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    gain_index = (GADI_U32) gk_get_sensor_gain_index(ispHandlePtr->isp3AHandle);
    *gain = gain_index_to_gain(gain_index);

    return GADI_OK;
}

/***************0715 hhl add (head)**************/
GADI_ERR gadi_isp_get_current_awb_gain(GADI_SYS_HandleT ispHandle,
    u32 * r_gain, u32 * g_gain, u32 * b_gain)
{
    wb_gain_t curWbGain = { 1024, 1024, 1024 };
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    GADI_U16 dgain;

    if ((gk_isp_get_rgb_gain(ispHandlePtr->isp3AHandle, &curWbGain, &dgain)) != GADI_OK)
        return (-1);

    *r_gain = curWbGain.r_gain;
    *g_gain = curWbGain.g_gain;
    *b_gain = curWbGain.b_gain;

    return GADI_OK;
}

/***************0715 hhl add (end)**************/
GADI_ERR gadi_isp_set_antiflicker(GADI_SYS_HandleT ispHandle,
    GADI_ISP_AntiFlickerParamT * paramPtr)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    GADI_ERR errorCode = GADI_OK;

    if (ispHandlePtr == NULL || paramPtr == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    if (ispHandlePtr->antiFlickerParam.freq == paramPtr->freq &&
        ispHandlePtr->antiFlickerParam.enableDisable == paramPtr->enableDisable)
    {
        return GADI_OK;
    }

    if (paramPtr->enableDisable == 0) {
        /*  TODO: disable antiFlicker.
         * We may implement this in isp3a lib first.
         */
        ispHandlePtr->antiFlickerParam.enableDisable = paramPtr->enableDisable;
        ispHandlePtr->antiFlickerParam.freq = paramPtr->freq;
        return GADI_OK;
    }

    if (ispHandlePtr->antiFlickerParam.freq != paramPtr->freq) {
        if (paramPtr->freq == GADI_ISP_ANTIFLICKER_FREQ_50HZ) {
            errorCode = gk_ae_set_antiflicker(ispHandlePtr->isp3AHandle, 0);
            ispHandlePtr->antiFlickerParam.freq = paramPtr->freq;
        } else if (paramPtr->freq == GADI_ISP_ANTIFLICKER_FREQ_60HZ) {
            errorCode = gk_ae_set_antiflicker(ispHandlePtr->isp3AHandle, 1);

        } else {
            errorCode = GADI_ISP_ERR_BAD_PARAMETER;
        }
    }

    if (errorCode == GADI_OK) {
        ispHandlePtr->antiFlickerParam.enableDisable = paramPtr->enableDisable;
        ispHandlePtr->antiFlickerParam.freq = paramPtr->freq;
    }

    return errorCode;
}

GADI_ERR gadi_isp_get_antiflicker(GADI_SYS_HandleT ispHandle,
    GADI_ISP_AntiFlickerParamT * paramPtr)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;

    if (ispHandlePtr == NULL || paramPtr == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    paramPtr->enableDisable = ispHandlePtr->antiFlickerParam.enableDisable;
    paramPtr->freq = ispHandlePtr->antiFlickerParam.freq;

    return GADI_OK;
}

/*!
*******************************************************************************
** \note
**    This API is used to set the color correction matrix under different
**    color temperatures (high,mid,low).
**
**    In Hisilicon implementation, the API could specify the value of the high,
**    mid, low temp and the according CCM.
**
**    In GOKE implementation by now, we offer the fixed and tuned ccm to isp3a
**    library using _adj_param.c with (2700K, 5000K, 7500K) sets of ccm value.
**
**    Acutally, GOKE implementation does not use ccm values in _adj_param.c but
**    use xxx_01/2/3/4_3D.bin instead.
**
*******************************************************************************
*/
GADI_ERR gadi_isp_set_ccm(GADI_SYS_HandleT ispHandle,
    GADI_ISP_ColorMatrixT * paramPtr)
{
    return GADI_OK;
}

GADI_ERR gadi_isp_get_ccm(GADI_SYS_HandleT ispHandle,
    GADI_ISP_ColorMatrixT * paramPtr)
{
    return GADI_OK;
}

/*!
*******************************************************************************
** \brief Currently our isp3a library gives below API to set gamma curve:
**           gk_isp_set_tone_curve()
**        And the xxx_adj.c offers the default ratio0 and ratio255 gamma table
**        Our platform use three separated gamma tables for R/G/B but share the
**        same values.
**
**        On our platform, the gamma has relationship with contrast.
**        One more thing, do not use img_dsp_xxx direclty, abstract according
**        img_xxx API to send message to main loop.
**
**        Hisilico API offers serveral predefined gamma tables internally (1.6 sRGB/
**        1.8 sRGB/2.0 sRGB/2.2 sRGB/standard sRGB) and also support customnized table.
*******************************************************************************
*/
GADI_ERR gadi_isp_set_gamma_attr(GADI_SYS_HandleT ispHandle,
    GADI_ISP_GammaAttrT * attrPtr)
{
    tone_curve_t toneCurve;
    GADI_ERR errorCode = GADI_OK;
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;

    /* TODO: enable/disable functionality. */

    memcpy(toneCurve.tone_curve_blue, attrPtr->gammaTable,
        GADI_ISP_GAMMA_LUT_SIZE);
    memcpy(toneCurve.tone_curve_red, attrPtr->gammaTable,
        GADI_ISP_GAMMA_LUT_SIZE);
    memcpy(toneCurve.tone_curve_green, attrPtr->gammaTable,
        GADI_ISP_GAMMA_LUT_SIZE);

    errorCode = gk_isp_set_tone_curve(ispHandlePtr->isp3AHandle, &toneCurve);
    if (errorCode != 0) {
        IMAGE_ERROR("isp: set gamma error %d\n", errorCode);
        errorCode = GADI_ISP_ERR_FROM_DRIVER;
    }

    return errorCode;
}

GADI_ERR gadi_isp_get_gamma_attr(GADI_SYS_HandleT ispHandle,
    GADI_ISP_GammaAttrT * attrPtr)
{
    tone_curve_t tone_curve;
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;

    /* TODO: get enable/disable functionality. */

    (void) gk_isp_get_tone_curve(ispHandlePtr->isp3AHandle, &tone_curve);
    memcpy(attrPtr->gammaTable, tone_curve.tone_curve_green,
        GADI_ISP_GAMMA_LUT_SIZE);

    return GADI_OK;
}

GADI_ERR gadi_isp_set_wb_type(GADI_SYS_HandleT ispHandle,
    GADI_ISP_WbTypeEnumT type)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    GADI_ERR errorCode = GADI_OK;
    wb_gain_t initWbGain = { 1024, 1024, 1024 };
    GADI_U16 dgain;

    if (ispHandlePtr->wbType != type) {
        /* TODO: change the white balance type. */
        if (type == GADI_ISP_WB_TYPE_AUTO) {
            errorCode = gk_enable_awb(ispHandlePtr->isp3AHandle, 1);
            gk_isp_get_rgb_gain(ispHandlePtr->isp3AHandle, &initWbGain, &dgain);
            awb_set_wb_init_state(ispHandlePtr->isp3AHandle, &initWbGain);
        } else {
            errorCode = gk_enable_awb(ispHandlePtr->isp3AHandle, 0);

        }
        if (errorCode != GADI_OK) {
            IMAGE_ERROR("ISP: set white balance type to %s failed\n",
                type ? "auto" : "manual");
            return GADI_ISP_ERR_FROM_DRIVER;
        }
    } else {
        return GADI_OK;
    }

    ispHandlePtr->wbType = type;

    return errorCode;
}

GADI_ERR gadi_isp_set_awb_attr(GADI_SYS_HandleT ispHandle,
    GADI_ISP_AwbAttrT * attrPtr)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    GADI_ERR errorCode = GADI_OK;
    awb_control_mode_t mode = WB_AUTOMATIC;
    awb_work_method_t method = AWB_NORMAL;

    if (ispHandlePtr == NULL) {
        return GADI_ISP_ERR_NOT_OPEN;
    }

    if (attrPtr == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    errorCode = gk_awb_set_speed(ispHandlePtr->isp3AHandle, attrPtr->speed);
    ispHandlePtr->awbAttrCur.speed = attrPtr->speed;

    switch (attrPtr->colorTemp) {
        case GADI_ISP_AWB_COLOR_TEMP_2800:
            mode = WB_INCANDESCENT;
            break;

        case GADI_ISP_AWB_COLOR_TEMP_3200:
            mode = WB_FLUORESCENT;
            break;

        case GADI_ISP_AWB_COLOR_TEMP_4500:
            mode = WB_D4000;
            break;

        case GADI_ISP_AWB_COLOR_TEMP_5500:
            mode = WB_D5000;
            break;

        case GADI_ISP_AWB_COLOR_TEMP_6500:
            mode = WB_SUNNY;
            break;

        case GADI_ISP_AWB_COLOR_TEMP_7500:
            mode = WB_CLOUDY;
            break;

        case GADI_ISP_AWB_COLOR_TEMP_AUTO:
        default:
            break;
    }
    errorCode = gk_awb_set_cntl_mode(ispHandlePtr->isp3AHandle, mode);
    ispHandlePtr->awbAttrCur.colorTemp = attrPtr->colorTemp;

    switch (attrPtr->algo) {
        case GADI_ISP_AWB_ALGO_AUTO:
        default:
            method = AWB_NORMAL;
            break;

        case GADI_ISP_AWB_ALGO_GRAYWORLD:
            method = AWB_GREY_WORLD;
            break;
    }
    errorCode = gk_awb_set_algo(ispHandlePtr->isp3AHandle, method);
    ispHandlePtr->awbAttrCur.algo = attrPtr->algo;

    return errorCode;
}

GADI_ERR gadi_isp_get_awb_attr(GADI_SYS_HandleT ispHandle,
    GADI_ISP_AwbAttrT * attrPtr)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;

    attrPtr->speed = ispHandlePtr->awbAttrCur.speed;
    attrPtr->algo = ispHandlePtr->awbAttrCur.algo;
    attrPtr->colorTemp = ispHandlePtr->awbAttrCur.colorTemp;

    return GADI_OK;
}

/*!
*******************************************************************************
** \note
**      This API should be invocated after gadi_isp_open and before gadi_isp_start.
**
**      To give more difference between GOKE implementation and origina implementation,
**      we need adjust isp3a library: (TODO)
**          - add p_xxx_control_exit interface
**          - adjust the callback parameters
*******************************************************************************
*/
GADI_ERR gadi_isp_set_awb_lib(GADI_SYS_HandleT ispHandle,
    GADI_ISP_AwbLibT customAwbLib)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;

#ifdef ISP3A_LIBRARY_GOKE
    gk_get_3A_algorithm(ispHandlePtr->isp3AHandle, &ispHandlePtr->aaaLibCur);
#endif

    ispHandlePtr->aaaLibCur.p_awb_control_init =
        (aaa_awb_control_init_cb) customAwbLib.awbInitFunc;
    ispHandlePtr->aaaLibCur.p_awb_control =
        (aaa_awb_control_cb) customAwbLib.awbProcFunc;

    gk_register_3A_algorithm(ispHandlePtr->isp3AHandle, ispHandlePtr->aaaLibCur);

    return GADI_OK;
}

GADI_ERR gadi_isp_set_mwb_attr(GADI_SYS_HandleT ispHandle,
    GADI_ISP_MwbAttrT * attrPtr)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    GADI_ISP_MwbAttrT *curMwbAttrPtr = &ispHandlePtr->mwbAttrCur;
    wb_gain_t isp3aWbGain;
    wb_gain_t dspWbGain;
    GADI_U16 dgain;

    if (ispHandlePtr == NULL || attrPtr == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    if (ispHandlePtr->wbType == GADI_ISP_WB_TYPE_AUTO) {
        IMAGE_ERROR("change white balance type before set MWB attributes\n");
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    gk_isp_get_rgb_gain(ispHandlePtr->isp3AHandle, &dspWbGain, &dgain);

    isp3aWbGain.r_gain = attrPtr->gainRed;
    isp3aWbGain.g_gain = attrPtr->gainGreen;
    isp3aWbGain.b_gain = attrPtr->gainBlue;

    IMAGE_INFO("dsp set mwb r[%d] g[%d] b[%d] d[%d]\n",
        isp3aWbGain.r_gain, isp3aWbGain.g_gain, isp3aWbGain.b_gain, dgain);
    if (gk_isp_set_rgb_gain(ispHandlePtr->isp3AHandle, &isp3aWbGain, dgain) < 0) {
        IMAGE_ERROR("isp3a set rgb gain failed\n");
        return GADI_ISP_ERR_FROM_DRIVER;
    }

    /* update the ADI internal attributes. */
    *curMwbAttrPtr = *attrPtr;

    return GADI_OK;
}

GADI_ERR gadi_isp_get_mwb_attr(GADI_SYS_HandleT ispHandle,
    GADI_ISP_MwbAttrT * attrPtr)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    GADI_ISP_MwbAttrT *curMwbAttrPtr = &ispHandlePtr->mwbAttrCur;

    if (ispHandlePtr == NULL || attrPtr == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    *attrPtr = *curMwbAttrPtr;

    return GADI_OK;
}

GADI_ERR gadi_isp_get_systemgain(GADI_SYS_HandleT ispHandle,
    GADI_S32 * valuePtr)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;

    if (ispHandlePtr == NULL || valuePtr == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    *valuePtr = gk_ae_get_sensor_gain(ispHandlePtr->isp3AHandle);

    return GADI_OK;
}

GADI_ERR gadi_isp_set_saturation(GADI_SYS_HandleT ispHandle, GADI_S32 value)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    GADI_ERR errorCode = GADI_OK;

    if (ispHandlePtr == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    if (value < 0) {
        value = 0;
    }
    if (value > 0x7F) {
        value = 0x7F;
    }

    if (ispHandlePtr->imageAttrCur.saturation == value) {
        return GADI_OK;
    }

    errorCode = gk_set_color_saturation(ispHandlePtr->isp3AHandle, value);
    if (errorCode != GADI_OK) {
        return GADI_ISP_ERR_FROM_DRIVER;
    } else {
        ispHandlePtr->imageAttrCur.saturation = value;
    }
    gReload_cfg_flag = 0;
    if (value <= 0x10) {
        gReload_cfg_flag = 1;
    }

    return GADI_OK;
}

GADI_ERR gadi_isp_get_saturation(GADI_SYS_HandleT ispHandle,
    GADI_S32 * valuePtr)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;

    if (ispHandlePtr == NULL || valuePtr == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    *valuePtr = ispHandlePtr->imageAttrCur.saturation;

    return GADI_OK;
}

GADI_ERR gadi_isp_set_control_day_night_mode(GADI_SYS_HandleT ispHandle,u8 en)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    img_awb_set_day_night_mode(ispHandlePtr->isp3AHandle, en);
    return GADI_OK;
}

GADI_ERR gadi_isp_set_contrast_attr(GADI_SYS_HandleT ispHandle,
    GADI_ISP_ContrastAttrT * attrPtr)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    GADI_ERR errorCode = GADI_OK;

    if (ispHandlePtr == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    if (attrPtr == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    /*
     * original value from [0,128], default is 64;
     * mapping [0,255] on original and default is 128;
     */

    if (ispHandlePtr == NULL || attrPtr == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    if (attrPtr->enableAuto == 1) {
        errorCode = gk_set_auto_contrast(ispHandlePtr->isp3AHandle, 1);
        if (errorCode != GADI_OK) {
            return GADI_ISP_ERR_FROM_DRIVER;
        } else {
            ispHandlePtr->imageAttrCur.contrast.enableAuto = 1;
        }

        errorCode =
            gk_set_auto_contrast_strength(ispHandlePtr->isp3AHandle, attrPtr->autoStrength/2);
        if (errorCode != GADI_OK) {
            IMAGE_ERROR("set auto contrast strength failed %d\n",
                errorCode);
            return GADI_ISP_ERR_FROM_DRIVER;
        } else {
            ispHandlePtr->imageAttrCur.contrast.autoStrength =
                attrPtr->autoStrength;
        }
    } else {
        errorCode = gk_set_auto_contrast(ispHandlePtr->isp3AHandle, 0);
        if (errorCode != GADI_OK) {
            return GADI_ISP_ERR_FROM_DRIVER;
        } else {
            ispHandlePtr->imageAttrCur.contrast.enableAuto = 0;
        }

        errorCode = gk_set_contrast(ispHandlePtr->isp3AHandle, attrPtr->manualStrength/2 );
        IMAGE_INFO("manual strength is %d\n", attrPtr->manualStrength/2);
        if (errorCode != GADI_OK) {
            IMAGE_ERROR("set manual contrast strength failed %d\n", errorCode);
            return GADI_ISP_ERR_FROM_DRIVER;
        } else {
            ispHandlePtr->imageAttrCur.contrast.manualStrength =
                    attrPtr->manualStrength;
        }
    }

    return GADI_OK;
}

GADI_ERR gadi_isp_get_contrast_attr(GADI_SYS_HandleT ispHandle,
    GADI_ISP_ContrastAttrT * attrPtr)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;

    if (ispHandlePtr == NULL || attrPtr == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    memcpy(attrPtr, &ispHandlePtr->imageAttrCur.contrast,
        sizeof(GADI_ISP_ContrastAttrT));

    return GADI_OK;
}

GADI_ERR gadi_isp_set_hue(GADI_SYS_HandleT ispHandle, GADI_S32 value)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    GADI_ERR errorCode = GADI_OK;
    GADI_S32 valSetting = 0;

    IMAGE_INFO("[ADI] gadi_isp_set_hue[In]  value[%d]\n", value);

    if (ispHandlePtr == NULL) {
        errorCode = GADI_ISP_ERR_BAD_PARAMETER;
        return errorCode;
    }

    if (value == ispHandlePtr->imageAttrCur.hue) {
        return GADI_OK;
    }

    /*
     * 0   - 119 [-15, -1] eight value for each degree
     * 120 - 135 [0]
     * 136 - 255 [1, 15]
     */
    if (value <= 119) {
        valSetting = value / 8;
        valSetting = valSetting - 15;
    } else if (value >= 136) {
        valSetting = value / 8;
        valSetting = valSetting - 16;
    } else {
        valSetting = 0;
    }

    errorCode = gk_set_color_hue(ispHandlePtr->isp3AHandle, valSetting);
    if (errorCode != 0) {
        errorCode = GADI_ISP_ERR_FROM_DRIVER;
        return errorCode;
    }

    IMAGE_INFO("set color hue [%d]\n", valSetting);

    ispHandlePtr->imageAttrCur.hue = value;

    IMAGE_INFO("[ADI] gadi_isp_set_hue[Out] ret[%d]\n", errorCode);

    return errorCode;
}

GADI_ERR gadi_isp_get_hue(GADI_SYS_HandleT ispHandle, GADI_S32 * valuePtr)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    GADI_ERR errorCode = GADI_OK;

    IMAGE_INFO("[ADI] gadi_isp_get_hue[In]  valuePtr[0x%08X]\n",
        (GADI_U32) valuePtr);

    if (ispHandlePtr == NULL || valuePtr == NULL) {
        errorCode = GADI_ISP_ERR_BAD_PARAMETER;
        return errorCode;
    }

    *valuePtr = ispHandlePtr->imageAttrCur.hue;

    IMAGE_INFO("[ADI] gadi_isp_get_hue[Out] ret[%d]\n", GADI_OK);

    return errorCode;
}

GADI_ERR gadi_isp_set_brightness(GADI_SYS_HandleT ispHandle, GADI_S32 value)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    GADI_ERR errorCode = GADI_OK;
    GADI_S32 valueSetting = 0;

    if (ispHandlePtr == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    if (ispHandlePtr->imageAttrCur.brightness == value) {
        return GADI_OK;
    }

    if (value < 0) {
        value = 0;
    }

    if (value > 0xFF) {
        value = 0xFF;
    }

    /* input [0 - 255]
     * 0   - 127   [-255, -1]
     * 128         [0]
     * 129 - 255   [1, 255]
     */
    if (value < 128) {
        valueSetting = value * 2 - 255;
    } else if (value > 128) {
        valueSetting = value * 2 - 255;
    } else {
        valueSetting = 0;       /* real default value */
    }

    errorCode = gk_set_color_brightness(ispHandlePtr->isp3AHandle, valueSetting);
    if (errorCode != GADI_OK) {
        return GADI_ISP_ERR_FROM_DRIVER;
    } else {
        ispHandlePtr->imageAttrCur.brightness = value;
    }

    return GADI_OK;
}

GADI_ERR gadi_isp_get_brightness(GADI_SYS_HandleT ispHandle,
    GADI_S32 * valuePtr)
{

    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;

    if (ispHandlePtr == NULL || valuePtr == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    *valuePtr = ispHandlePtr->imageAttrCur.brightness;

    return GADI_OK;
}

/*!
*******************************************************************************
** \note
**      Current sharpen attributes are controlled by the adj_xxx.c where
**          def_sharp_info_t sharp_info;
**      was defined.
*******************************************************************************
*/
GADI_ERR gadi_isp_set_sharpen_attr(GADI_SYS_HandleT ispHandle,
    GADI_ISP_SharpenAttrT * attrPtr)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    GADI_ERR errorCode = GADI_OK;

    if (ispHandlePtr == NULL || attrPtr == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }
#if 0                           // not support manual mode
    if (attrPtr->enableDisable != ispHandlePtr->sharpenAttr.enableDisable) {
        /* TODO: disable or enable the sharpen */
        if (attrPtr->enableDisable) {
            /* enable the sharpen */

            /* see also p_adj_video_param->sharp_info.sharpen_enable
             ** in isp3a_ctl.c, this parameter has been decided in
             ** xxx_adj.c
             **
             ** New API needs to be added to implement this feature.
             */
        } else {
            /* disable the sharpen */
        }

        ispHandlePtr->sharpenAttr.enableDisable = attrPtr->enableDisable;
    }
#endif
//    printf("ISP: sharpen, enable:%d,level:%d\n",attrPtr->enableDisable,attrPtr->level);

    if (attrPtr->enableDisable != ispHandlePtr->sharpenAttr.enableDisable) {
        /* TODO: disable or enable the sharpen */
        if (attrPtr->enableDisable == 0) {
            /* enable the manual(disable the auto) */
            gk_set_auto_sharpen_enable(ispHandlePtr->isp3AHandle, 0);

        } else {
            /* disable the manual(enable the auto) */
            gk_set_auto_sharpen_enable(ispHandlePtr->isp3AHandle, 1);

        }

        ispHandlePtr->sharpenAttr.enableDisable = attrPtr->enableDisable;
    }

    if (attrPtr->level != ispHandlePtr->sharpenAttr.level) {

        errorCode = gk_set_sharpness(ispHandlePtr->isp3AHandle, attrPtr->level);
        if (errorCode != GADI_OK) {
            return GADI_ISP_ERR_FROM_DRIVER;
        }

        ispHandlePtr->sharpenAttr.level = attrPtr->level;
    }
   if ((attrPtr->dir_strength != ispHandlePtr->sharpenAttr.dir_strength) ||
       (attrPtr->flat_strength != ispHandlePtr->sharpenAttr.flat_strength) ||
       (attrPtr->level != ispHandlePtr->sharpenAttr.level) ||
       (attrPtr->edge_th       != ispHandlePtr->sharpenAttr.edge_th)||
       (attrPtr->max_change_up != ispHandlePtr->sharpenAttr.max_change_up) ||
       (attrPtr->max_change_down != ispHandlePtr->sharpenAttr.max_change_down))
    {
       errorCode = gk_set_sharpen_str_ex(ispHandlePtr->isp3AHandle, attrPtr->level,
        attrPtr->flat_strength,attrPtr->flat_strength,attrPtr->max_change_up,
        attrPtr->max_change_down,attrPtr->edge_th);
        if (errorCode != GADI_OK) {
            return GADI_ISP_ERR_FROM_DRIVER;
        }

        ispHandlePtr->sharpenAttr.dir_strength= attrPtr->dir_strength;
        ispHandlePtr->sharpenAttr.edge_th= attrPtr->edge_th;
        ispHandlePtr->sharpenAttr.flat_strength= attrPtr->flat_strength;
        ispHandlePtr->sharpenAttr.max_change_up = attrPtr->max_change_up;
        ispHandlePtr->sharpenAttr.max_change_down = attrPtr->max_change_down;
    }

    return GADI_OK;
}

GADI_ERR gadi_isp_get_sharpen_attr(GADI_SYS_HandleT ispHandle,
    GADI_ISP_SharpenAttrT * attrPtr)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;

    if (ispHandlePtr == NULL || attrPtr == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    attrPtr->enableDisable = ispHandlePtr->sharpenAttr.enableDisable;
    attrPtr->level = ispHandlePtr->sharpenAttr.level;
    attrPtr->dir_strength = ispHandlePtr->sharpenAttr.dir_strength;
    attrPtr->edge_th = ispHandlePtr->sharpenAttr.edge_th;
    attrPtr->flat_strength = ispHandlePtr->sharpenAttr.flat_strength;
    attrPtr->max_change_up = ispHandlePtr->sharpenAttr.max_change_up;
    attrPtr->max_change_down = ispHandlePtr->sharpenAttr.max_change_down;
    attrPtr->enableManual = ispHandlePtr->sharpenAttr.enableManual;

    return GADI_OK;
}

GADI_ERR gadi_isp_set_spatial_filter_attr(GADI_SYS_HandleT ispHandle,
    GADI_ISP_SpatialFilterAttrT * attrPtr)
{

    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    GADI_ERR errorCode = GADI_OK;

    if (ispHandlePtr == NULL || attrPtr == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    if (attrPtr->enableDisable != ispHandlePtr->SpatialFilterAttr.enableDisable) {
        /* TODO: disable or enable the sharpen */
        if (attrPtr->enableDisable == 0) {
            /* enable the manual(disable the auto) */
            gk_set_auto_spatial_filter_enable(ispHandlePtr->isp3AHandle, 0);

        } else {
            /* disable the manual(enable the auto) */
            gk_set_auto_spatial_filter_enable(ispHandlePtr->isp3AHandle, 1);

        }

        ispHandlePtr->sharpenAttr.enableDisable = attrPtr->enableDisable;
    }

    if ((attrPtr->spatial_thr_flat != ispHandlePtr->SpatialFilterAttr.spatial_thr_flat)||
       (attrPtr->spatial_thr_dir != ispHandlePtr->SpatialFilterAttr.spatial_thr_dir)||
       (attrPtr->spatial_weight_dir != ispHandlePtr->SpatialFilterAttr.spatial_weight_dir)||
       (attrPtr->sptial_weight_flat != ispHandlePtr->SpatialFilterAttr.sptial_weight_flat)||
       (attrPtr->edge_thr != ispHandlePtr->SpatialFilterAttr.edge_thr))
    {
       errorCode = img_set_spatial_filter(ispHandlePtr->isp3AHandle, attrPtr->sptial_weight_flat,
        attrPtr->spatial_weight_dir,attrPtr->spatial_thr_flat,
        attrPtr->spatial_thr_dir,attrPtr->edge_thr);
        if (errorCode != GADI_OK) {
            return GADI_ISP_ERR_FROM_DRIVER;
        }

        ispHandlePtr->SpatialFilterAttr.spatial_thr_flat= attrPtr->spatial_thr_flat;
        ispHandlePtr->SpatialFilterAttr.spatial_thr_dir= attrPtr->spatial_thr_dir;
        ispHandlePtr->SpatialFilterAttr.spatial_weight_dir= attrPtr->spatial_weight_dir;
        ispHandlePtr->SpatialFilterAttr.sptial_weight_flat= attrPtr->sptial_weight_flat;
        ispHandlePtr->SpatialFilterAttr.edge_thr= attrPtr->edge_thr;
    }

    return GADI_ISP_ERR_FEATURE_NOT_SUPPORTED;
}

GADI_ERR gadi_isp_get_spatial_filter_attr(GADI_SYS_HandleT ispHandle,
    GADI_ISP_SpatialFilterAttrT * attrPtr)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;

    if (ispHandlePtr == NULL || attrPtr == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    attrPtr->enableDisable = ispHandlePtr->SpatialFilterAttr.enableDisable;
    attrPtr->spatial_thr_flat = ispHandlePtr->SpatialFilterAttr.spatial_thr_flat;
    attrPtr->spatial_thr_dir = ispHandlePtr->SpatialFilterAttr.spatial_thr_dir;
    attrPtr->spatial_weight_dir = ispHandlePtr->SpatialFilterAttr.spatial_weight_dir;
    attrPtr->sptial_weight_flat = ispHandlePtr->SpatialFilterAttr.sptial_weight_flat;
    attrPtr->edge_thr = ispHandlePtr->SpatialFilterAttr.edge_thr;


    return GADI_OK;
}



GADI_ERR gadi_isp_set_shading_attr(GADI_SYS_HandleT ispHandle,
    GADI_ISP_ShadingAttrT * attrPtr)
{
    //gk_isp_set_lens_shading_compensation(int fd_media,vignette_info_t * vignette_info)

    return GADI_ISP_ERR_FEATURE_NOT_SUPPORTED;
}

GADI_ERR gadi_isp_get_shading_attr(GADI_SYS_HandleT ispHandle,
    GADI_ISP_ShadingAttrT * attrPtr)
{
    return GADI_ISP_ERR_FEATURE_NOT_SUPPORTED;
}

GADI_ERR gadi_isp_set_denoise_attr(GADI_SYS_HandleT ispHandle,
    GADI_ISP_DenoiseAttrT * attrPtr)
{
    img_isp_capability_t capability;
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    //GADI_S32 valSetting = 0;

    if (ispHandlePtr == NULL || attrPtr == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    isp3a_proc_get_capability(ispHandlePtr->isp3AHandle, &capability);

    if(!capability.vps_enable)
        {if (attrPtr->enableDisable != ispHandlePtr->denoiseAttr.enableDisable) {
            ispHandlePtr->denoiseAttr.enableDisable = attrPtr->enableDisable;
           }


        if ((attrPtr->level != ispHandlePtr->denoiseAttr.level) ||
            (attrPtr->thr_level!= ispHandlePtr->denoiseAttr.thr_level) ||
            (attrPtr->max_change_level!= ispHandlePtr->denoiseAttr.max_change_level)) {

            gk_set_3dnr_level_ex(ispHandlePtr->isp3AHandle, attrPtr->level,attrPtr->thr_level,
                                  attrPtr->max_change_level);
            ispHandlePtr->denoiseAttr.level = attrPtr->level;
            ispHandlePtr->denoiseAttr.thr_level= attrPtr->thr_level;
            ispHandlePtr->denoiseAttr.max_change_level= attrPtr->max_change_level;

           }
        }
    else
        {if (attrPtr->enableDisable != ispHandlePtr->denoiseAttr.enableDisable) {
            img_set_vps_3d_enable(ispHandlePtr->isp3AHandle, attrPtr->enableDisable);
            ispHandlePtr->denoiseAttr.enableDisable = attrPtr->enableDisable;
           }

         if (attrPtr->level != ispHandlePtr->denoiseAttr.level ||
            attrPtr->thr_level!= ispHandlePtr->denoiseAttr.thr_level ||
            attrPtr->max_change_level!= ispHandlePtr->denoiseAttr.max_change_level) {

            gk_set_3dnr_level_ex(ispHandlePtr->isp3AHandle, attrPtr->level,attrPtr->thr_level,
                                  attrPtr->max_change_level);
            ispHandlePtr->denoiseAttr.level = attrPtr->level;
            ispHandlePtr->denoiseAttr.thr_level= attrPtr->thr_level;
            ispHandlePtr->denoiseAttr.max_change_level= attrPtr->max_change_level;

           }


        }

    return GADI_OK;
}

GADI_ERR gadi_isp_get_denoise_attr(GADI_SYS_HandleT ispHandle,
    GADI_ISP_DenoiseAttrT * attrPtr)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;

    if (ispHandlePtr == NULL || attrPtr == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    attrPtr->enableDisable = ispHandlePtr->denoiseAttr.enableDisable;
    attrPtr->level = ispHandlePtr->denoiseAttr.level;
    attrPtr->thr_level= ispHandlePtr->denoiseAttr.thr_level;
    attrPtr->max_change_level= ispHandlePtr->denoiseAttr.max_change_level;

    return GADI_OK;
}

/*!
*******************************************************************************
** \note
**    This API is used to set the black level value.
**
**    The Hisilicon style of API directly set the black level value.
**
**    In GOKE implementation, we have internal default black level value:
**      def_blc[3] in isp3a_drv.c
**    And we also have internal API to add offset on the default black level:
**      gk_isp_set_final_blc
**
*******************************************************************************
*/
GADI_ERR gadi_isp_set_blacklevel_attr(GADI_SYS_HandleT ispHandle,
    GADI_ISP_BlackLevelAttrT * attrPtr)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    GADI_ERR errorCode = GADI_OK;

    GK_VI_SRC_MirrorModeT mirrorMode;

    if (ispHandlePtr == NULL || attrPtr == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    mirrorMode.src_id = gk_get_3a_srcid(ispHandlePtr->isp3AHandle); // TODO: add src ID
    errorCode =
        ioctl(ispHandlePtr->mediaFd, GK_MEDIA_IOC_VI_SRC_GET_MIRROR_MODE,
        &mirrorMode);

    errorCode =
        gk_isp_set_final_blc(ispHandlePtr->isp3AHandle, (blc_level_t *) attrPtr,
        mirrorMode.mirror_pattern);
    if (errorCode != GADI_OK) {
        IMAGE_ERROR("adi_isp: set black level failed\n");
        return GADI_ISP_ERR_FROM_DRIVER;
    }

    return GADI_OK;
}

GADI_ERR gadi_isp_get_blacklevel_attr(GADI_SYS_HandleT ispHandle,
    GADI_ISP_BlackLevelAttrT * attrPtr)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    GADI_ERR errorCode = GADI_OK;
    blc_level_t blcOffset;

    if (ispHandlePtr == NULL || attrPtr == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    errorCode = gk_isp_get_final_blc(ispHandlePtr->isp3AHandle, &blcOffset);

    memcpy(&attrPtr->blackLevel[0], &blcOffset, sizeof(blc_level_t));

    return errorCode;
}

GADI_ERR gadi_isp_set_backlight_attr(GADI_SYS_HandleT ispHandle,
    GADI_ISP_BacklightAttrT * attrPtr)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    GADI_ERR errorCode = GADI_OK;

    if (ispHandlePtr == NULL || attrPtr == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    if (attrPtr->enableDisable == ispHandlePtr->backlightAttr.enableDisable) {
        return GADI_OK;
    }

    errorCode = gk_ae_set_backlight(ispHandlePtr->isp3AHandle, attrPtr->enableDisable);
    if (errorCode != GADI_OK) {
        IMAGE_ERROR("adi_isp: set backlight enableDisable failed\n");
        return GADI_ISP_ERR_FROM_DRIVER;
    }

    ispHandlePtr->backlightAttr.enableDisable = attrPtr->enableDisable;

    return GADI_OK;
}

GADI_ERR gadi_isp_get_backlight_attr(GADI_SYS_HandleT ispHandle,
    GADI_ISP_BacklightAttrT * attrPtr)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;

    if (ispHandlePtr == NULL || attrPtr == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    attrPtr->enableDisable = ispHandlePtr->backlightAttr.enableDisable;

    return GADI_OK;
}

GADI_ERR gadi_isp_set_dewarp_attr(GADI_SYS_HandleT ispHandle,
    GADI_ISP_WarpInfoT * attrPtr)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;

    GADI_ERR errorCode = GADI_OK;
    if (ispHandlePtr == NULL || attrPtr == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    if ((attrPtr->h_warp_enable != ispHandlePtr->dewarpAttr.h_warp_enable) ||
        (attrPtr->h_warp_strength!= ispHandlePtr->dewarpAttr.h_warp_strength) ||
        (attrPtr->v_warp_enable!= ispHandlePtr->dewarpAttr.v_warp_enable) ||
        (attrPtr->v_warp_strength!= ispHandlePtr->dewarpAttr.v_warp_strength)) {

            ispHandlePtr->dewarpAttr.h_warp_enable = attrPtr->h_warp_enable;
            ispHandlePtr->dewarpAttr.h_warp_strength= attrPtr->h_warp_strength;
            ispHandlePtr->dewarpAttr.v_warp_enable= attrPtr->v_warp_enable;
            ispHandlePtr->dewarpAttr.v_warp_strength= attrPtr->v_warp_strength;
            errorCode = img_set_dewarp(ispHandlePtr->isp3AHandle,attrPtr->h_warp_enable,attrPtr->h_warp_strength,
                                       attrPtr->v_warp_enable,attrPtr->v_warp_strength);
            if (errorCode != GADI_OK) {
                IMAGE_ERROR("adi_isp: set dewrp attr failed\n");
                return GADI_ISP_ERR_FROM_DRIVER;
            }
    }


    return GADI_OK;
}

GADI_ERR gadi_isp_get_dewarp_attr(GADI_SYS_HandleT ispHandle,
    GADI_ISP_WarpInfoT * attrPtr)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    warp_config_t warp_config;

    if (ispHandlePtr == NULL || attrPtr == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }
    img_dsp_get_dewarp(ispHandlePtr->isp3AHandle,&warp_config);
    ispHandlePtr->dewarpAttr.h_warp_enable = (u8)warp_config.warp_control;
    ispHandlePtr->dewarpAttr.h_warp_strength = warp_config.warp_multiplier;
    ispHandlePtr->dewarpAttr.v_warp_enable = warp_config.vert_warp_enable;
    ispHandlePtr->dewarpAttr.v_warp_strength = warp_config.vert_warp_multiplier;

    attrPtr->h_warp_enable = ispHandlePtr->dewarpAttr.h_warp_enable ;
    attrPtr->h_warp_strength = ispHandlePtr->dewarpAttr.h_warp_strength;
    attrPtr->v_warp_enable = ispHandlePtr->dewarpAttr.v_warp_enable;
    attrPtr->v_warp_strength = ispHandlePtr->dewarpAttr.v_warp_strength;

    return GADI_OK;
}
//added by Hu Yin 08-03-2017

GADI_ERR gadi_isp_set_lowbitrate_attr(GADI_SYS_HandleT ispHandle,
    GADI_ISP_LowBitrateInfoT* attrPtr)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    image_low_bitrate_property_t image_low_bitrate;
    u8 tolerance_low_bitrate;
    int delay_frames_low_bitrate;

    if (ispHandlePtr == NULL || attrPtr == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    if (attrPtr->enable != ispHandlePtr->lowbitrateAttr.enable) {
        ispHandlePtr->lowbitrateAttr.enable = attrPtr->enable;
        IMAGE_INFO("gadi_isp_set_lowbitrate_attr: enable %d\n", ispHandlePtr->lowbitrateAttr.enable);
        gk_set_low_bitrate_mode(ispHandlePtr->isp3AHandle, ispHandlePtr->lowbitrateAttr.enable);
    }

    if ((attrPtr->sharpness_ratio != ispHandlePtr->lowbitrateAttr.sharpness_ratio) ||
        (attrPtr->sharpness_param != ispHandlePtr->lowbitrateAttr.sharpness_param) ||
        (attrPtr->vps_3d_ratio != ispHandlePtr->lowbitrateAttr.vps_3d_ratio) ||
        (attrPtr->vps_3d_param != ispHandlePtr->lowbitrateAttr.vps_3d_param) ||
        (attrPtr->vps_contrast_ratio != ispHandlePtr->lowbitrateAttr.vps_contrast_ratio) ||
        (attrPtr->vps_contrast_param != ispHandlePtr->lowbitrateAttr.vps_contrast_param) ||
        (attrPtr->sharpen_property_up_ratio != ispHandlePtr->lowbitrateAttr.sharpen_property_up_ratio) ||
        (attrPtr->sharpen_property_up_param != ispHandlePtr->lowbitrateAttr.sharpen_property_up_param) ||
        (attrPtr->sharpen_property_down_ratio != ispHandlePtr->lowbitrateAttr.sharpen_property_down_ratio) ||
        (attrPtr->sharpen_property_down_param != ispHandlePtr->lowbitrateAttr.sharpen_property_down_param) ||
        (attrPtr->cfa_denoise_property_up_ratio != ispHandlePtr->lowbitrateAttr.cfa_denoise_property_up_ratio) ||
        (attrPtr->cfa_denoise_property_up_param != ispHandlePtr->lowbitrateAttr.cfa_denoise_property_up_param) ||
        (attrPtr->cfa_denoise_property_down_ratio != ispHandlePtr->lowbitrateAttr.cfa_denoise_property_down_ratio) ||
        (attrPtr->cfa_denoise_property_down_param != ispHandlePtr->lowbitrateAttr.cfa_denoise_property_down_param) ||
        (attrPtr->tolerance != ispHandlePtr->lowbitrateAttr.tolerance) ||
        (attrPtr->delay_frames != ispHandlePtr->lowbitrateAttr.delay_frames)) {

         ispHandlePtr->lowbitrateAttr.sharpness_ratio = attrPtr->sharpness_ratio;
         ispHandlePtr->lowbitrateAttr.sharpness_param = attrPtr->sharpness_param;
         ispHandlePtr->lowbitrateAttr.vps_3d_ratio = attrPtr->vps_3d_ratio;
         ispHandlePtr->lowbitrateAttr.vps_3d_param = attrPtr->vps_3d_param;
         ispHandlePtr->lowbitrateAttr.sharpen_property_up_ratio = attrPtr->sharpen_property_up_ratio;
         ispHandlePtr->lowbitrateAttr.sharpen_property_up_param = attrPtr->sharpen_property_up_param;
         ispHandlePtr->lowbitrateAttr.sharpen_property_down_ratio = attrPtr->sharpen_property_down_ratio;
         ispHandlePtr->lowbitrateAttr.sharpen_property_down_param = attrPtr->sharpen_property_down_param;
         ispHandlePtr->lowbitrateAttr.cfa_denoise_property_up_ratio = attrPtr->cfa_denoise_property_up_ratio;
         ispHandlePtr->lowbitrateAttr.cfa_denoise_property_up_param = attrPtr->cfa_denoise_property_up_param;
         ispHandlePtr->lowbitrateAttr.cfa_denoise_property_down_ratio = attrPtr->cfa_denoise_property_down_ratio;
         ispHandlePtr->lowbitrateAttr.cfa_denoise_property_down_param = attrPtr->cfa_denoise_property_down_param;
         ispHandlePtr->lowbitrateAttr.tolerance = attrPtr->tolerance;
         ispHandlePtr->lowbitrateAttr.delay_frames = attrPtr->delay_frames;

         image_low_bitrate.sharpness_low_bitrate_ratio                 = ispHandlePtr->lowbitrateAttr.sharpness_ratio;
         image_low_bitrate.sharpness_low_bitrate_param                 = ispHandlePtr->lowbitrateAttr.sharpness_param;
         //image_low_bitrate.vps_3d_low_bitrate_ratio                    = ispHandlePtr->lowbitrateAttr.vps_3d_ratio;
         //image_low_bitrate.vps_3d_low_bitrate_param                    = ispHandlePtr->lowbitrateAttr.vps_3d_param;
         //image_low_bitrate.vps_contrast_low_bitrate_ratio              = ispHandlePtr->lowbitrateAttr.vps_contrast_ratio;
         //image_low_bitrate.vps_contrast_low_bitrate_param              = ispHandlePtr->lowbitrateAttr.vps_contrast_param;
         image_low_bitrate.sharpen_property_low_bitrate_up_ratio       = ispHandlePtr->lowbitrateAttr.sharpen_property_up_ratio;
         image_low_bitrate.sharpen_property_low_bitrate_up_param       = ispHandlePtr->lowbitrateAttr.sharpen_property_up_param;
         image_low_bitrate.sharpen_property_low_bitrate_down_ratio     = ispHandlePtr->lowbitrateAttr.sharpen_property_down_ratio;
         image_low_bitrate.sharpen_property_low_bitrate_down_param     = ispHandlePtr->lowbitrateAttr.sharpen_property_down_param;
         image_low_bitrate.cfa_denoise_property_low_bitrate_up_ratio   = ispHandlePtr->lowbitrateAttr.cfa_denoise_property_up_ratio;
         image_low_bitrate.cfa_denoise_property_low_bitrate_up_param   = ispHandlePtr->lowbitrateAttr.cfa_denoise_property_up_param;
         image_low_bitrate.cfa_denoise_property_low_bitrate_down_ratio = ispHandlePtr->lowbitrateAttr.cfa_denoise_property_down_ratio;
         image_low_bitrate.cfa_denoise_property_low_bitrate_down_param = ispHandlePtr->lowbitrateAttr.cfa_denoise_property_down_param;
         tolerance_low_bitrate                                         = ispHandlePtr->lowbitrateAttr.tolerance;
         delay_frames_low_bitrate                                      = ispHandlePtr->lowbitrateAttr.delay_frames;

         gk_set_low_bitrate_param(ispHandlePtr->isp3AHandle, &image_low_bitrate, &tolerance_low_bitrate, &delay_frames_low_bitrate);
    }

    return GADI_OK;
}

GADI_ERR gadi_isp_get_lowbitrate_attr(GADI_SYS_HandleT ispHandle,
    GADI_ISP_LowBitrateInfoT* attrPtr)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    image_low_bitrate_property_t image_low_bitrate;
    u8 tolerance_low_bitrate;
    int delay_frames_low_bitrate;

    if (ispHandlePtr == NULL || attrPtr == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    ispHandlePtr->lowbitrateAttr.enable = gk_get_low_bitrate_mode(ispHandlePtr->isp3AHandle);
    attrPtr->enable = ispHandlePtr->lowbitrateAttr.enable;

    gk_get_low_bitrate_param(ispHandlePtr->isp3AHandle, &image_low_bitrate, &tolerance_low_bitrate, &delay_frames_low_bitrate);

    ispHandlePtr->lowbitrateAttr.sharpness_ratio                 = image_low_bitrate.sharpness_low_bitrate_ratio;
    ispHandlePtr->lowbitrateAttr.sharpness_param                 = image_low_bitrate.sharpness_low_bitrate_param;
    //ispHandlePtr->lowbitrateAttr.vps_3d_ratio                    = image_low_bitrate.vps_3d_low_bitrate_ratio;
    //ispHandlePtr->lowbitrateAttr.vps_3d_param                    = image_low_bitrate.vps_3d_low_bitrate_param;
    //ispHandlePtr->lowbitrateAttr.vps_contrast_ratio              = image_low_bitrate.vps_contrast_low_bitrate_ratio;
    //ispHandlePtr->lowbitrateAttr.vps_contrast_param              = image_low_bitrate.vps_contrast_low_bitrate_param;
    ispHandlePtr->lowbitrateAttr.sharpen_property_up_ratio       = image_low_bitrate.sharpen_property_low_bitrate_up_ratio;
    ispHandlePtr->lowbitrateAttr.sharpen_property_up_param       = image_low_bitrate.sharpen_property_low_bitrate_up_param;
    ispHandlePtr->lowbitrateAttr.sharpen_property_down_ratio     = image_low_bitrate.sharpen_property_low_bitrate_down_ratio;
    ispHandlePtr->lowbitrateAttr.sharpen_property_down_param     = image_low_bitrate.sharpen_property_low_bitrate_down_param;
    ispHandlePtr->lowbitrateAttr.cfa_denoise_property_up_ratio   = image_low_bitrate.cfa_denoise_property_low_bitrate_up_ratio;
    ispHandlePtr->lowbitrateAttr.cfa_denoise_property_up_param   = image_low_bitrate.cfa_denoise_property_low_bitrate_up_param;
    ispHandlePtr->lowbitrateAttr.cfa_denoise_property_down_ratio = image_low_bitrate.cfa_denoise_property_low_bitrate_down_ratio;
    ispHandlePtr->lowbitrateAttr.cfa_denoise_property_down_param = image_low_bitrate.cfa_denoise_property_low_bitrate_down_param;
    ispHandlePtr->lowbitrateAttr.tolerance                       = tolerance_low_bitrate;
    ispHandlePtr->lowbitrateAttr.delay_frames                    = delay_frames_low_bitrate;

    attrPtr->sharpness_ratio                 = ispHandlePtr->lowbitrateAttr.sharpness_ratio;
    attrPtr->sharpness_param                 = ispHandlePtr->lowbitrateAttr.sharpness_param;
    //attrPtr->vps_3d_ratio                    = ispHandlePtr->lowbitrateAttr.vps_3d_ratio;
    //attrPtr->vps_3d_param                    = ispHandlePtr->lowbitrateAttr.vps_3d_param;
    //attrPtr->vps_contrast_ratio              = ispHandlePtr->lowbitrateAttr.vps_contrast_ratio;
    //attrPtr->vps_contrast_param              = ispHandlePtr->lowbitrateAttr.vps_contrast_param;
    attrPtr->sharpen_property_up_ratio       = ispHandlePtr->lowbitrateAttr.sharpen_property_up_ratio;
    attrPtr->sharpen_property_up_param       = ispHandlePtr->lowbitrateAttr.sharpen_property_up_param;
    attrPtr->sharpen_property_down_ratio     = ispHandlePtr->lowbitrateAttr.sharpen_property_down_ratio;
    attrPtr->sharpen_property_down_param     = ispHandlePtr->lowbitrateAttr.sharpen_property_down_param;
    attrPtr->cfa_denoise_property_up_ratio   = ispHandlePtr->lowbitrateAttr.cfa_denoise_property_up_ratio;
    attrPtr->cfa_denoise_property_up_param   = ispHandlePtr->lowbitrateAttr.cfa_denoise_property_up_param;
    attrPtr->cfa_denoise_property_down_ratio = ispHandlePtr->lowbitrateAttr.cfa_denoise_property_down_ratio;
    attrPtr->cfa_denoise_property_down_param = ispHandlePtr->lowbitrateAttr.cfa_denoise_property_down_param;
    attrPtr->tolerance                       = ispHandlePtr->lowbitrateAttr.tolerance;
    attrPtr->delay_frames                    = ispHandlePtr->lowbitrateAttr.delay_frames;

    return GADI_OK;
}
//end
GADI_ERR gadi_isp_get_statistics_enable(GADI_SYS_HandleT ispHandle,
    GADI_BOOL flag)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    if (ispHandle == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }
    adj_get_isp_statistics_enable(ispHandlePtr->isp3AHandle, flag);
    return GADI_OK;
}

GADI_ERR gadi_isp_get_statistics(GADI_SYS_HandleT ispHandle,
    GADI_ISP_StatisticsT * ispStatics, GADI_U32 milliSec)
{
    GADI_ERR retVal = GADI_OK;
    GADI_U32 i;
    ISP_CfaAfStatT fhSta[MAX_AF_TILE_NUM];
    ISP_CfaAfStatT fhvSta[MAX_AF_TILE_NUM];
    awb_data_t    awb_data[MAX_AWB_TILE_NUM];
    ae_data_t     ae_data[MAX_AE_TILE_NUM];
    ISP_HistogramStatT hist_data;


    GADI_ISP_FocusStatisticsBlockT *pBlockInfo = NULL;
    GADI_ISP_AWBStatisticsBlockT *pBlockInfoAWB = NULL;
    GADI_ISP_AEStatisticsBlockT *pBlockInfoAE = NULL;

    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;

    if (ispHandlePtr == NULL || ispStatics == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }
    /* get focus statistics  info */
    retVal = adj_get_isp_statistics(ispHandlePtr->isp3AHandle, fhSta, fhvSta, ae_data, awb_data, &hist_data, milliSec);

    if (retVal != 0) {
        IMAGE_ERROR("adi_isp: get isp focus statistics failed, or timeout!\n");
        return GADI_ISP_ERR_FROM_DRIVER;
    }

    /* add focus statistics  info to struct */
    pBlockInfo =
        (GADI_ISP_FocusStatisticsBlockT *)ispStatics->focusStat.zoneMetrics;
    for (i = 0;
        i <
        sizeof(ispStatics->focusStat.zoneMetrics) /
        sizeof(GADI_ISP_FocusStatisticsBlockT); i++) {
        pBlockInfo[i].sum_cfa_fy = fhSta[i].sum_fy;
        pBlockInfo[i].sum_cfa_fv1 = fhSta[i].sum_fv1;
        pBlockInfo[i].sum_cfa_fv2 = fhSta[i].sum_fv2;
        pBlockInfo[i].sum_rgb_fy = fhvSta[i].sum_fy;
        pBlockInfo[i].sum_rgb_fv1 = fhvSta[i].sum_fv1;
        pBlockInfo[i].sum_rgb_fv2 = fhvSta[i].sum_fv2;
    }

    pBlockInfoAWB = (GADI_ISP_AWBStatisticsBlockT*)ispStatics->wbStat.zoneMetricsAWB;
    for (i = 0;
        i <
        sizeof(ispStatics->wbStat.zoneMetricsAWB) /
        sizeof(GADI_ISP_AWBStatisticsBlockT); i++) {
        pBlockInfoAWB[i].R_avg = awb_data[i].r_avg;
        pBlockInfoAWB[i].G_avg = awb_data[i].g_avg;
        pBlockInfoAWB[i].B_avg = awb_data[i].b_avg;
        pBlockInfoAWB[i].cfa_y = awb_data[i].lin_y;
        pBlockInfoAWB[i].CR_avg = awb_data[i].cr_avg;
        pBlockInfoAWB[i].CB_avg = awb_data[i].cb_avg;
        pBlockInfoAWB[i].Yuv_y = awb_data[i].non_lin_y;
    }

    pBlockInfoAE = ispStatics->exStat.zoneMetricsAE;
    for (i = 0; i < MAX_AE_TILE_NUM; i++) {
        pBlockInfoAE[i].cfa_y = ae_data[i].lin_y;
        pBlockInfoAE[i].rgb_y= ae_data[i].non_lin_y;

    }



    for (i = 0; i < HIST_BIN_NUM; i++) {
        ispStatics->HistStat.cfa_his_y[i]= hist_data.cfa_histogram.his_bin_y[i];
        ispStatics->HistStat.cfa_his_r[i]= hist_data.cfa_histogram.his_bin_r[i];
        ispStatics->HistStat.cfa_his_g[i]= hist_data.cfa_histogram.his_bin_g[i];
        ispStatics->HistStat.cfa_his_b[i]= hist_data.cfa_histogram.his_bin_b[i];
        ispStatics->HistStat.rgb_his_y[i]= hist_data.rgb_histogram.his_bin_y[i];
        ispStatics->HistStat.rgb_his_r[i]= hist_data.rgb_histogram.his_bin_r[i];
        ispStatics->HistStat.rgb_his_g[i]= hist_data.rgb_histogram.his_bin_g[i];
        ispStatics->HistStat.rgb_his_b[i]= hist_data.rgb_histogram.his_bin_b[i];

    }

    return GADI_OK;
}

GADI_ERR gadi_isp_get_sensor_model(GADI_SYS_HandleT ispHandle,
    GADI_ISP_SensorModelEnumT * modelPtr)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;

    GK_VI_SRC_InfoT vi_info;

    if (ispHandlePtr == NULL || modelPtr == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    vi_info.src_id = gk_get_3a_srcid(ispHandlePtr->isp3AHandle); // TODO: add src ID
    if (ioctl(ispHandlePtr->mediaFd, GK_MEDIA_IOC_VI_SRC_GET_INFO,
        &vi_info) < 0) {
        IMAGE_ERROR("get vi source info failed.\n");
        return GADI_ISP_ERR_FROM_DRIVER;
    }

    *modelPtr = vi_info.sensor_id;

    return GADI_OK;
}

static pthread_t retTuning_threadId = 0;

GADI_ERR gadi_isp_tuning_start(void)
{
#ifdef TUNING_FUNC_CLOSE
    IMAGE_ERROR("tuning tool can't support.\n");
#else
    int retTuning;

    if (0 == retTuning_threadId) {
        IMAGE_INFO("tuning server start.\n");
    } else {
        //IMAGE_INFO("tuning server already start:%d.\n", retTuning_threadId);
        //return GADI_ERR_ALREADY_OPEN;
        return GADI_OK;
    }

    retTuning = pthread_create(&retTuning_threadId, NULL, tuning_process, NULL);
    if (retTuning != 0) {
        return GADI_ERR_THREAD_CREATE;
    }
#endif

    return GADI_OK;
}

GADI_ERR gadi_isp_tuning_stop(void)
{
#ifdef TUNING_FUNC_CLOSE
    IMAGE_ERROR("tuning tool can't support.\n");
#else
    IMAGE_INFO("tuning server stop.\n");
    if (retTuning_threadId == 0) {
        return GADI_ERR_NOT_OPEN;
    }
    //pthread_kill(retTuning_threadId, SIGUSR1);

    //retTuning_threadId = 0;
#endif

    return GADI_OK;
}

GADI_U32 gadi_isp_get_sensor_id(GADI_SYS_HandleT ispHandle)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    return gadi_get_sensors_id(ispHandlePtr->isp3AHandle);
}

GADI_ERR gadi_isp_set_day_night_mode(GADI_SYS_HandleT ispHandle, GADI_U8 isDay)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    gk_set_day_night_mode(ispHandlePtr->isp3AHandle, isDay == 0 ? 1 : 0);
    gIsDay = isDay;
    return GADI_OK;
}

GADI_BOOL gadi_isp_get_day_night_mode(GADI_SYS_HandleT ispHandle)
{
    return gIsDay;
}

//added by Hu Yin
GADI_ERR gadi_isp_set_meter_mode(GADI_SYS_HandleT ispHandle, GADI_U8 mode)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    gk_ae_set_meter_mode(ispHandlePtr->isp3AHandle, mode);
    return GADI_OK;
}

GADI_ERR gadi_isp_set_roi(GADI_SYS_HandleT ispHandle, GADI_S32 * roi)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    gk_set_ae_meter_table(ispHandlePtr->isp3AHandle, roi);
    return GADI_OK;
}

GADI_ERR gadi_isp_reset_3a_static(GADI_SYS_HandleT ispHandle)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    warp_config_t warp_config;
    globle_motion_filter_t  gmf;
    local_contrast_enhancement_t lce;

    if (ispHandlePtr == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    if (gk_isp_move_block_number_update(ispHandlePtr->isp3AHandle) != GADI_OK) {
        IMAGE_ERROR("adi_isp: update move block number failed\n");
        return GADI_ISP_ERR_FROM_DRIVER;
    }

    if (gk_isp_config_statistics_params(ispHandlePtr->isp3AHandle, NULL) != GADI_OK) {
        IMAGE_ERROR("adi_isp: reset 3a statictics failed\n");
        return GADI_ISP_ERR_FROM_DRIVER;
    }

    img_dsp_get_dewarp(ispHandlePtr->isp3AHandle,&warp_config);
    if(warp_config.warp_control) {
        if (img_set_dewarp(ispHandlePtr->isp3AHandle,warp_config.warp_control,warp_config.warp_multiplier,
               warp_config.vert_warp_enable,warp_config.vert_warp_multiplier) != GADI_OK) {
            IMAGE_ERROR("adi_isp: set dewrp attr failed\n");
            return GADI_ISP_ERR_FROM_DRIVER;
        }
    }

    if (gk_isp_get_globle_motion_filter(ispHandlePtr->isp3AHandle, &gmf) != GADI_OK) {
        IMAGE_ERROR("adi_isp: get gmf failed\n");
        return GADI_ISP_ERR_FROM_DRIVER;
    }

    if (gk_isp_set_globle_motion_filter(ispHandlePtr->isp3AHandle, &gmf) != GADI_OK) {
        IMAGE_ERROR("adi_isp: set gmf failed\n");
        return GADI_ISP_ERR_FROM_DRIVER;
    }

    if (gk_isp_get_local_contrast_enhancement(ispHandlePtr->isp3AHandle, &lce) != GADI_OK) {
        IMAGE_ERROR("adi_isp: get lce failed\n");
        return GADI_ISP_ERR_FROM_DRIVER;
    }

    if (gk_isp_set_local_contrast_enhancement(ispHandlePtr->isp3AHandle, &lce) != GADI_OK) {
        IMAGE_ERROR("adi_isp: set lce failed\n");
        return GADI_ISP_ERR_FROM_DRIVER;
    }

    return GADI_OK;
}

GADI_ERR gadi_set_auto_local_wdr_mode(GADI_SYS_HandleT ispHandle, GADI_U32 mode)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    if (gk_set_auto_wdr(ispHandlePtr->isp3AHandle, mode) != GADI_OK) {
        IMAGE_ERROR("adi_isp: set auto local exposure error.\n");
        return GADI_ISP_ERR_FROM_DRIVER;
    }
    IMAGE_DEBUG("Local exposure mode : [%d].\n", mode);

    return GADI_OK;
}

GADI_ERR gadi_isp_set_aaa_static_frequency(GADI_SYS_HandleT ispHandle, GADI_U32 freq)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;

    if (ispHandlePtr == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    img_dsp_set_aaa_statistic_frequency(ispHandlePtr->isp3AHandle, freq);


    IMAGE_DEBUG("AAA static frame frequncy : [%d].\n", freq);

    return GADI_OK;
}

//*****************************************************************************
//*****************************************************************************
//** Local Functions
//*****************************************************************************
//*****************************************************************************
static GADI_ISP_P_HandleT *isp_p_allocate_handle(GADI_ERR * errCodePtr)
{
    GADI_U32 index;
    GADI_ISP_P_HandleT *handlePtr = NULL;
    GADI_ERR error = GADI_OK;

    for (index = 0; index < ISP_HANDLE_MAX_NUM; index++) {
        if (NULL == ispHandleArray[index]) {
            ispHandleArray[index] = (GADI_ISP_P_HandleT*)malloc(sizeof(GADI_ISP_P_HandleT));
            if (NULL != ispHandleArray[index]) {
                memset(ispHandleArray[index], 0, sizeof(GADI_ISP_P_HandleT));
                handlePtr = ispHandleArray[index];
                ispHandleArray[index]->inUse = 1;
                ispHandleArray[index]->mediaFd = gFdMedia;
            }
            else
            {
                IMAGE_ERROR("%s, %d, malloc fail\n", __func__, __LINE__);
                error = GADI_ISP_ERR_OUT_OF_MEMORY;
            }
            break;
        }
    }

    if (errCodePtr) {
        *errCodePtr = error;
    }

    return handlePtr;
}

static void isp_p_release_handle(GADI_ISP_P_HandleT *handlePtr)
{
    GADI_U32 index;

    for (index = 0; index < ISP_HANDLE_MAX_NUM; index++) {
        if (handlePtr == ispHandleArray[index]) {
            free(handlePtr);
            ispHandleArray[index] = NULL;
            break;
        }
    }

    return;
}

static GADI_S32 isp_p_is_rgb_sensor_connect(GADI_ISP_P_HandleT * handlePtr)
{
    GK_VI_SRC_InfoT src_info;
    GK_DRV_VideoInfoT video_info;

    memset(&src_info, 0, sizeof(src_info));
    src_info.src_id = gk_get_3a_srcid(handlePtr->isp3AHandle); // TODO:add src ID
    if (ioctl(handlePtr->mediaFd, GK_MEDIA_IOC_VI_SRC_GET_INFO, &src_info) < 0) {
        IMAGE_ERROR("get vi source info failed.\n");
        return GADI_ISP_ERR_FROM_DRIVER;
    }

    memset(&video_info, 0, sizeof(video_info));
    video_info.srcid = gk_get_3a_srcid(handlePtr->isp3AHandle); // TODO:add src ID
    if (ioctl(handlePtr->mediaFd, GK_MEDIA_IOC_VI_SRC_GET_VIDEO_INFO,
            &video_info) < 0) {
        IMAGE_ERROR("get vi source video info failed.");
        return GADI_ISP_ERR_FROM_DRIVER;
    }
    handlePtr->inputVideoWidth = video_info.width;
    handlePtr->inputVideoHeight = video_info.height;

    IMAGE_INFO
        ("isp: -------------------------------------------------------\n");
    IMAGE_INFO("isp: input video info listed\n");
    IMAGE_INFO("isp: width  %d\n", video_info.width);
    IMAGE_INFO("isp: height %d\n", video_info.height);
    IMAGE_INFO("isp: fps %d\n",
        video_info.fps ? 512000000 / video_info.fps : video_info.fps);
    IMAGE_INFO("isp: format %d\n", video_info.format);
    IMAGE_INFO("isp: type %d\n", video_info.type);
    IMAGE_INFO("isp: bits %d\n", video_info.bits);
    IMAGE_INFO
        ("isp: -------------------------------------------------------\n");

    return ((src_info.dev_type != GK_VI_DEV_TYPE_DECODER) &&
        ((video_info.type == GK_VIDEO_TYPE_RGB_601) ||
            (video_info.type == GK_VIDEO_TYPE_RGB_RAW)));
}

static int isp_load_ccm_data (void *isp3AHandle, GADI_ISP_P_HandleT * handlePtr)
{
    int file, count;
    char filename[128];
    u8 reg[CC_REG_SIZE], matrix[CC_3D_SIZE];

    color_correction_t color_corr;
    color_correction_reg_t color_corr_reg;

    sprintf(filename, "%s/color_reg.bin", ISP_PARAM_PATH);
    if ((file = open(filename, O_RDONLY, 0)) < 0) {
        IMAGE_ERROR("Open reg.bin file error!\n");
        return -1;
    }
    if ((count = read(file, reg, CC_REG_SIZE)) != CC_REG_SIZE) {
        IMAGE_ERROR("Read reg.bin file error!\n");
        return -1;
    }
    close(file);

    sprintf(filename, "%s/color_matrix.bin", ISP_PARAM_PATH);
    if ((file = open(filename, O_RDONLY, 0)) < 0) {
        IMAGE_ERROR("Open 3D.bin file error!\n");
        return -1;
    }
    if ((count = read(file, matrix, CC_3D_SIZE)) != CC_3D_SIZE) {
        IMAGE_ERROR("Read 3D.bin file error!\n");
        return -1;
    }
    close(file);

    color_corr_reg.reg_setting_addr = (u32) reg;
    color_corr.matrix_3d_table_addr = (u32) matrix;
    if (gk_isp_set_ccr(isp3AHandle, &color_corr_reg) < 0) {
        IMAGE_ERROR("gk_isp_set_ccr error!\n");
        return -1;
    }
    if (gk_isp_set_cc(isp3AHandle, &color_corr) < 0) {
        IMAGE_ERROR("gk_isp_set_cc error!\n");
        return -1;
    }

    return 0;
}

#if 1
static int isp_p_load_dsp_vignette_table(GADI_ISP_P_HandleT * handlePtr)
{
    int file, count;
    char filename[128];
    u16 vignette_table[33 * 33 * 4] = { 0 };
    u32 gain_shift;
    vignette_info_t vignette_info;
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) handlePtr;

    sprintf(filename, "%s/lens_shading.bin", ISP_PARAM_PATH);
    //sprintf(filename, "lens_shading.bin");

    if ((file = open(filename, O_RDONLY, 0)) < 0) {
        IMAGE_ERROR("Open lens_shading.bin file error!\n");
        return -1;
    }

    count = read(file, vignette_table, 4 * VIGNETTE_MAX_SIZE * sizeof(u16));
    if (count != 4 * VIGNETTE_MAX_SIZE * sizeof(u16)) {
        IMAGE_ERROR("Read lens_shading.bin file error!\n");
        return -1;
    }

    count = read(file, &gain_shift, sizeof(u32));
    if (count != sizeof(u32)) {
        IMAGE_ERROR("Read lens_shading.bin error\n");
        return -1;
    }
    close(file);

    vignette_info.enable = handlePtr->sensorInfo.sensor_param.p_lens_shading->enable;
    vignette_info.gain_shift = (u8) gain_shift;
    vignette_info.vignette_red_gain_addr =
        (u32) (vignette_table + 0 * VIGNETTE_MAX_SIZE);
    vignette_info.vignette_green_even_gain_addr =
        (u32) (vignette_table + 1 * VIGNETTE_MAX_SIZE);
    vignette_info.vignette_green_odd_gain_addr =
        (u32) (vignette_table + 2 * VIGNETTE_MAX_SIZE);
    vignette_info.vignette_blue_gain_addr =
        (u32) (vignette_table + 3 * VIGNETTE_MAX_SIZE);

    if (gk_isp_set_lens_shading_compensation(ispHandlePtr->isp3AHandle, &vignette_info)) {
        IMAGE_ERROR("gk_isp_set_cc error!\n");
        return -1;
    }

    return 0;
}
#endif

static int isp_p_load_adj_cc_table(void *handle, GADI_SENSORS_InfoT * sensorInfoPtr)
{
    if (gk_load_sensor_param(handle, &(sensorInfoPtr->sensor_param)) < 0) {
        IMAGE_ERROR("gk_load_sensor_param error!\n");
        return -1;
    }

    return GADI_OK;
}

GADI_ERR gadi_isp_start_af_zoom(GADI_SYS_HandleT ispHandle, GADI_S8 dir)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    u8 manualMode = 0;
    if (ispHandlePtr == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }
    if (dir>0)
       manualMode = (u8)ZOOMIN;
    else
       manualMode = (u8)ZOOMOUT;
    gk_set_af_manual_status(ispHandlePtr->isp3AHandle, manualMode);

    IMAGE_DEBUG("start af zoom: [%d].\n", dir);

    return GADI_OK;
}

GADI_ERR gadi_isp_stop_af_zoom(GADI_SYS_HandleT ispHandle)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    u8 manualMode = (u8)ZOOMSTOP;
    if (ispHandlePtr == NULL) {
           return GADI_ISP_ERR_BAD_PARAMETER;
    }
    gk_set_af_manual_status(ispHandlePtr->isp3AHandle, manualMode);

    IMAGE_DEBUG("stop af zoom.\n");

    return GADI_OK;


}

GADI_ERR gadi_isp_start_af_focus(GADI_SYS_HandleT ispHandle, GADI_S8 dir)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    u8 manualMode = 0;
    if (ispHandlePtr == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }
    if (dir>0)
       manualMode = (u8)FOCUSN;
    else
       manualMode = (u8)FOCUSF;
    gk_set_af_manual_status(ispHandlePtr->isp3AHandle, manualMode);


    IMAGE_DEBUG("start af focus : [%d].\n", dir);

    return GADI_OK;
}

GADI_ERR gadi_isp_stop_af_focus(GADI_SYS_HandleT ispHandle)
{
    GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    u8 manualMode = (u8)FOCUSSTOP;
    if (ispHandlePtr == NULL) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }

    gk_set_af_manual_status(ispHandlePtr->isp3AHandle, manualMode);

    IMAGE_DEBUG("stop af focus.\n");

    return GADI_OK;
}

GADI_ERR gadi_isp_get_af_zoom(GADI_SYS_HandleT ispHandle, float* zoomOut)
{

    Isp3AContentAF *content;
    Isp3AProcessor *processor;
	GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
	s32 pulse_dest_l;
	s32 pulse_dest_h;
	s32 pulse_dest;
	float zoom_idx_dest;
	u8 zoom_idx;
	if (ispHandlePtr == NULL || zoomOut == NULL) {
		  return GADI_ISP_ERR_BAD_PARAMETER;
	}
    processor = ispHandlePtr->isp3AHandle;
    content = &(processor->content.AFData);
	zoom_idx_dest = content->af_result.zoom_idx_dest;
	zoom_idx = (u8)zoom_idx_dest;
    pulse_dest_l = content->af_zoom_param[zoom_idx].zoom_pluse;
    pulse_dest_h = content->af_zoom_param[(zoom_idx + 1)].zoom_pluse;
    pulse_dest =
        af_pulse_interpolate(pulse_dest_l, pulse_dest_h, zoom_idx,
        (zoom_idx + 1), zoom_idx_dest);
	*zoomOut =(float) (pulse_dest * content->af_G_param.zoom_slope
		+ content->af_G_param.zoom_offset);
	return GADI_OK;
}

GADI_ERR gadi_isp_set_af_pi_gpio(GADI_SYS_HandleT ispHandle, GADI_U8* gpioArray, GADI_U8 Arrlength)
{
    Isp3AContentAF *content;
    Isp3AProcessor *processor;
	GADI_ISP_P_HandleT *ispHandlePtr = (GADI_ISP_P_HandleT *) ispHandle;
    processor = ispHandlePtr->isp3AHandle;
    content = &(processor->content.AFData);
	
    if (ispHandlePtr == NULL || gpioArray == NULL || Arrlength < 3) {
        return GADI_ISP_ERR_BAD_PARAMETER;
    }
    memcpy(content->gpioArray, gpioArray, sizeof(GADI_U8) * 3);
    
    IMAGE_DEBUG("set_af_pi_gpio,GPIO is %d, Z_GPIO is %d, F_GPIO is %d.\n", 
        gpioArray[0], gpioArray[1], gpioArray[2]);

    return GADI_OK;
}

