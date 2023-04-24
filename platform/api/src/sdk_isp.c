/*!
*****************************************************************************
** \file        gk7101_isp.c
**
** \version
**
** \brief       sdk ISP control
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2014-2015 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#define GK_DEBUG 1
#include<math.h>

#include "sdk_isp.h"
#include "sdk_debug.h"
#include "sdk_gpio.h"
#include "sdk.h"
#include "sdk_cfg.h"

#ifdef MODULE_SUPPORT_PWM_IRCUT_LED
#include "sdk_pwm.h"
#endif

#define LINUX

//*****************************************************************************
//*****************************************************************************
//** Local Defines
//*****************************************************************************
//*****************************************************************************
#define ISP_DAY         0
#define ISP_LIGHT       1

// ircut check number
#define ISP_IRCUT_AUTO_CHECK_NUMBER  2
#define IRCUT_CHANGE_LIMITE     50

#define ISP_LOWLIGHT_MODE_CLOSE (0)
#define ISP_LOWLIGHT_MODE_ALLDAY (1)
#define ISP_LOWLIGHT_MODE_NIGHT (2)
#define ISP_LOWLIGHT_MODE_AUTO (3)
//*****************************************************************************
//*****************************************************************************
//** Local structures
//*****************************************************************************
//*****************************************************************************

typedef struct
{
	uint8_t ircut_auto_switch_enable;
	uint8_t ircut_control_mode;
    uint8_t ircut_status;
    uint8_t gpio_status_old;
}GK_IrCutAttr;

typedef struct
{
    int16_t     brightness;
    int16_t     staturation;
    int8_t      brig_offset;
    int8_t      stau_offset;
}GK_ImageStyle;

typedef struct
{
    int8_t isLowFrame;
    int8_t curMode;
}GK_slowlight_mode;

typedef int (*IMAGE_FLIP_FUNC)(uint8_t);
typedef struct
{
    GADI_S32 saturation;
    GADI_S32 brightness;
    GADI_S32 hug;
    GADI_S32 contrast;
    GADI_S32 sharpen;
}GK_ImageDefParamVal;

//*****************************************************************************
//*****************************************************************************
//** Global Data
//*****************************************************************************
//*****************************************************************************


extern GADI_SYS_HandleT vencHandle;
extern GADI_SYS_HandleT viHandle;

GADI_SYS_HandleT ispHandle        = NULL;

GK_ImageStyle imageSytle = {0,};
GK_slowlight_mode slowLight = {-1,-1};

//*****************************************************************************
//*****************************************************************************
//** Local Data
//*****************************************************************************
//*****************************************************************************


static GADI_U8          ispIsInitialized = 0;
static GADI_U8          ispIsStarted     = 0;
static GK_IrCutAttr     irCutAttr = {0,0,0,0xff};

static GK_ImageDefParamVal ispDefParams = {0x40, 0x80, 0x80, 0x80, 0x80};
static time_t isp_change_time = 0;

#if 0
#define DAY_INDEX       0
#define NIGHT_INDEX     1
#define ALL_INDEX       2

static int reserve_fps[ALL_INDEX][GADI_VENC_STREAM_NUM] =
{
    {-1, -1, -1, -1},
    {-1, -1, -1, -1}
};
#endif

//*****************************************************************************
//*****************************************************************************
//** Local Functions
//*****************************************************************************
//*****************************************************************************

//1: use adc value to check day and night, 0: use gain and adc value to check day and night
//如果光敏电阻外添加了红外滤光片，则将该值设为1，直接根据光敏值来判断白天和黑夜
#define IRCUT_ADC_USE_VALUE     0

//#define GOKE_TUNNING
#ifdef GOKE_TUNNING
pthread_t       retTuning_threadId;
extern void * tuning_process(void * argv);
#endif

static GADI_U32 ispTargetGain = 0;
static GADI_U32 ispTargetViFramerate = 0;
static GADI_U32 ispOrginalViFramerate = 0;
static GADI_U32 ispOrginalEncFramerate = 0;
static GADI_BOOL ispAutoSlowShutterRunFlag = GADI_FALSE;
static GADI_BOOL ispAutoSlowShutterEnableFlag = GADI_FALSE;
static ISP_BULB_ModeEnumT ispBulbMode = ISP_BULB_MODE_IR;
static GADI_U8 ispBulbLevel = 0;
static GADI_U8 ircutFlag = 0;
static pthread_mutex_t bulbMutex;
GADI_SYS_ThreadHandleT ispAutoSlowShutterThread;

//Add by Dengbiao 2019-8-14 10:43:49
static double exposure_tolerate = 0.0;
static double exposure_invert_tolerate = 0.0;
static double exposure_swicth_before = 0.0;
static double exposure_swicth_after = 0.0;
#define ALL_COLOR_MODE_CLOSE    (0)
#define ALL_COLOR_MODE_OPEN     (1)
static int all_color_mode_status = ALL_COLOR_MODE_CLOSE;
static int ispMdLight = 0;
static int isLedTest = 0;

#ifdef MODULE_SUPPORT_PWM_IRCUT_LED
#define IRCUT_PWM_CHANNEL   0
#define LIGHT_PWM_CHANNEL   1
int pwmOnDutyVaule[MAX_PWM_CHANNEL] = {50, 50, 50, 50, 50};
#endif


static int isp_set_slow_framerate(int setSlow);
static double isp_get_current_exposure(void)
{
    GADI_U32 gain;//gain/6 to increase or multiply
    double shutter_time_xms;
#if 0
    if (gadi_isp_get_current_gain(ispHandle, &gain) < GADI_OK)
    {
        return 0.0;
    }

    if (gadi_isp_get_current_shutter(ispHandle, &shutter_time_xms) < GADI_OK)
    {
        return 0.0;
    }
#endif		//xqq
    return shutter_time_xms * pow(2,gain/6.0);
}

static double isp_get_current_exposure_by_xframe(int xframe)
{
    GADI_U32 gain;//gain/6 to increase or multiply
    double shutter_time_xms;
    double shutter_time_xframe= 0.0;
    int xframe_valid = 0;
    int i;

    if (xframe <= 0)
        xframe = 0;

    for (i = 0; i < xframe; i++)
    {
#if 0
        if (gadi_isp_get_current_gain(ispHandle, &gain) < GADI_OK)
        {
            continue;
        }

        if (gadi_isp_get_current_shutter(ispHandle, &shutter_time_xms) < GADI_OK)
        {
            continue;
        }
#endif		//xqq
        shutter_time_xframe += shutter_time_xms * pow(2,gain/6.0);
        xframe_valid++;

       // gadi_sys_thread_sleep(shutter_time_xms+1);	xqq
    }

    if (xframe_valid == 0)
        return 0.0;
    else
        return shutter_time_xframe/xframe_valid;
}


static double isp_calculate_exposure_tolerate(double exp_before,
    double exp_after, double exp_threshold, int tolerate)
{
    double exp_delta = exp_before - exp_after;
    double exp_offset = exp_before - exp_threshold;
    double exp_tolerate = 0;

    if (exp_delta < 0)exp_delta = -exp_delta;

    if (exp_delta == 0.0)
        exp_tolerate = 1;
    else
        exp_tolerate = abs(exp_after)*100/abs(exp_threshold - exp_after);

    exp_tolerate = exp_tolerate*tolerate/100;
    if (exp_tolerate > abs(exp_after)/2)
        exp_tolerate = abs(exp_after)/2;

    if (exp_tolerate < 1)
        exp_tolerate = 1.0;

    return exp_tolerate;
}

static void isp_restore_default_mode(void)
{
    GADI_U32 viFramerate;
    GADI_ISP_AeAttrT    aeAttr;
    goke_encode_stream_params* strParams;
    GADI_VENC_FrameRateT stFrameRate;
    int cnt;

//    gadi_isp_set_day_night_mode(ispHandle, 1);
//
//    gadi_vi_set_ircut_control(viHandle, GADI_VI_IRCUT_DAY);
//    gadi_sys_thread_sleep(300);
//    gadi_vi_set_ircut_control(viHandle, GADI_VI_IRCUT_CLEAR);	xqq
#if 0	//xqq
    if(gadi_vi_get_framerate(viHandle, &viFramerate) == GADI_OK)
    {
        LOG_INFO("auto slowshutter current vi_framerate:%d  original:%d \n", viFramerate,ispOrginalViFramerate);
        if(viFramerate < ispOrginalViFramerate)
        {
            //gadi_vi_set_framerate(viHandle, ispOrginalViFramerate);	xqq

//            gadi_isp_get_ae_attr(ispHandle, &aeAttr);	xqq

            aeAttr.shutterTimeMax = ispOrginalViFramerate;
            aeAttr.gainMax        = 30;

//            gadi_isp_set_ae_attr(ispHandle, &aeAttr);	xqq
        }
    }
#endif	//xqq
    for (cnt = 0; cnt < gk_get_max_stream_nums(); cnt++)
    {
        strParams = gk_get_enc_stream_par(cnt);
        if(strParams != NULL)
        {
            LOG_INFO("stream:%d, stFrameRate.fps:%d->%d\n",cnt, strParams->streamFormat.fps);
            stFrameRate.streamId = cnt;
            stFrameRate.fps = strParams->streamFormat.fps;
            //gadi_venc_set_framerate(vencHandle, &stFrameRate);	xqq
        }
    }

    gk_isp_led_off(); // if define gpio control ircut led, this code can valid
    irCutAttr.gpio_status_old = ISP_DAY;
    
    LOG_INFO("@@@@@@@@@@restore default mode\n");

    //save status
    all_color_mode_status = ALL_COLOR_MODE_CLOSE;
    sdk_isp_changed();
}
static void isp_restore_default_mode_use_adc_gpio(void)
{
    GADI_S32 slowFramerate = 0;
    GADI_ISP_ContrastAttrT contrastAttr;
    //EM_SENSOR_MODEL sensor_id;
    GADI_ISP_SensorModelEnumT sensorModel;
    GADI_ISP_AeAttrT    aeAttr;
    int ret;
    GADI_ISP_LowBitrateInfoT lowbitAttrPtr;

    sdk_isp_changed();
//    gadi_isp_get_ae_attr(ispHandle, &aeAttr);
//    gadi_isp_get_contrast_attr(ispHandle, &contrastAttr);
//    LOG_INFO("IRCUT:day light\n");
//    //gk_isp_led_off(); // if define gpio control ircut led, this code can valid
//    gadi_isp_set_day_night_mode(ispHandle, 1);
//    gadi_isp_set_meter_mode(ispHandle, 2);	xqq
#if 0
    if(slowLight.curMode == ISP_LOWLIGHT_MODE_ALLDAY) //must slow framerate
    {
        slowFramerate = isp_set_slow_framerate(1);
    }
    else
    {
        slowFramerate = isp_set_slow_framerate(0);
    }
    LOG_INFO("slow framerate:%d\n", slowFramerate);
    
    irCutAttr.gpio_status_old = ISP_DAY;
    
    if(slowFramerate < 0)
    {
        LOG_ERR("set slow framerate failed.\n");
        return ;
    }
#endif 		//xqq
    irCutAttr.gpio_status_old = ISP_DAY;	//xqq
    //gadi_isp_get_sensor_model(ispHandle, &sensorModel);	xqq
    LOG_INFO("day SENSOR: 0x%x\n", sensorModel);
    if (sensorModel == GADI_ISP_SENSOR_JXH42)  //jxh42
    {
        LOG_INFO("day jxh42\n");
        contrastAttr.enableAuto     = 1;
        contrastAttr.manualStrength = 128;
        contrastAttr.autoStrength   = ispDefParams.contrast;//96;
        //gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);	xqq
    
        aeAttr.shutterTimeMin = 8000;
        aeAttr.shutterTimeMax = slowFramerate;
        aeAttr.gainMax        = 30;
        aeAttr.gainMin        = 1;
    
        //gadi_isp_set_ae_attr(ispHandle, &aeAttr);	xqq
    }
    else if (sensorModel == GADI_ISP_SENSOR_JXH61)  //jxh61
    {
        LOG_INFO("day jxh61\n");
        contrastAttr.enableAuto     = 1;
        contrastAttr.manualStrength = 128;
        contrastAttr.autoStrength   = ispDefParams.contrast;//96;
        //gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);		xqq
    
        aeAttr.shutterTimeMin = 8000;
        aeAttr.shutterTimeMax = slowFramerate;
        aeAttr.gainMax        = 30;
        aeAttr.gainMin        = 1;
        //gadi_isp_set_ae_attr(ispHandle, &aeAttr);
    
        //gadi_set_auto_local_wdr_mode(ispHandle, 112);		xqq
    }
    else if (sensorModel == GADI_ISP_SENSOR_JXF22)  //jxf22
    {
        LOG_INFO("day jxf22\n");
        contrastAttr.enableAuto     = 1;
        contrastAttr.manualStrength = 128;
        contrastAttr.autoStrength   = 64;//96;
        //gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);	xqq
    
        aeAttr.shutterTimeMin = 8000;//1024;
        aeAttr.shutterTimeMax = slowFramerate;
        aeAttr.gainMax        = 30;
        aeAttr.gainMin        = 1;
    
        ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);
//        gadi_set_auto_local_wdr_mode(ispHandle, 0);		xqq
    }
    else if(sensorModel == GADI_ISP_SENSOR_IMX222)   //imx222
    {
        LOG_INFO("day imx222\n");
        aeAttr.shutterTimeMin = 8000;//1024;
        aeAttr.shutterTimeMax = slowFramerate;
        aeAttr.gainMax        = 36;
        aeAttr.gainMin        = 1;
        contrastAttr.enableAuto     = 1;
        contrastAttr.manualStrength = 128;
        contrastAttr.autoStrength   = ispDefParams.contrast;//96;
       // gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);
       // gadi_isp_set_ae_attr(ispHandle, &aeAttr);	xqq
    
    }
    else if(sensorModel == GADI_ISP_SENSOR_IMX291)   //imx291
    {
        LOG_INFO("day imx291\n");
        aeAttr.shutterTimeMin = 8000;//1024;
        aeAttr.shutterTimeMax = slowFramerate;
        aeAttr.gainMax        = 42;
        aeAttr.gainMin        = 1;
        contrastAttr.enableAuto     = 1;
        contrastAttr.manualStrength = 128;
        contrastAttr.autoStrength   = ispDefParams.contrast;//96;
       // gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);
       // gadi_isp_set_ae_attr(ispHandle, &aeAttr);	xqq
    
    }
    else if(sensorModel == GADI_ISP_SENSOR_OV2710)   //ov2710
    {
        LOG_INFO("day ov2710\n");
        aeAttr.shutterTimeMin = 8000;//1024;
        aeAttr.shutterTimeMax = slowFramerate;
        aeAttr.gainMax        = 36;
        aeAttr.gainMin        = 1;
        contrastAttr.enableAuto     = 1;
        contrastAttr.manualStrength = 128;
        contrastAttr.autoStrength   = ispDefParams.contrast;//96;
       // gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);
       // gadi_isp_set_ae_attr(ispHandle, &aeAttr);	xqq
    }
    else if(sensorModel == GADI_ISP_SENSOR_OV9750)   //ov9750
    {
        LOG_INFO("day ov9750\n");
        aeAttr.shutterTimeMin = 8000;//1024;
        aeAttr.shutterTimeMax = slowFramerate;
        aeAttr.gainMax        = 36;
        aeAttr.gainMin        = 1;
        contrastAttr.enableAuto     = 1;
        contrastAttr.manualStrength = 128;
        contrastAttr.autoStrength   = 128;//96;
       // gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);
       // gadi_isp_set_ae_attr(ispHandle, &aeAttr);
       // gadi_set_auto_local_wdr_mode(ispHandle, 96);		xqq
    }
    else if(sensorModel == GADI_ISP_SENSOR_SC1035)   //sc1035
    {
        LOG_INFO("day sc1035\n");
        aeAttr.shutterTimeMin = 8000;//1024;
        aeAttr.shutterTimeMax = slowFramerate;
        aeAttr.gainMax        = 33;
        aeAttr.gainMin        = 1;
        contrastAttr.enableAuto     = 1;
        contrastAttr.manualStrength = 128;
        contrastAttr.autoStrength   = ispDefParams.contrast;//96;
        //gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);	xqq
       // ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);
       // if (ret != GADI_OK)
       // {
       //     LOG_ERR("set_ae_attr error day %d\n", ret);
       // }
       // gadi_set_auto_local_wdr_mode(ispHandle, 90);		xqq
    }
    else if(sensorModel == GADI_ISP_SENSOR_SC1135)   //sc1135
    {
        LOG_INFO("day sc1135\n");
        aeAttr.shutterTimeMin = 16000;//1024;
        aeAttr.shutterTimeMax = slowFramerate;
        aeAttr.gainMax        = 28;
        aeAttr.gainMin        = 1;
        contrastAttr.enableAuto     = 1;
        contrastAttr.manualStrength = 128;
        contrastAttr.autoStrength   = 64;//96;
       // gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);
       // gadi_isp_set_ae_attr(ispHandle, &aeAttr);
       // gadi_set_auto_local_wdr_mode(ispHandle, 96);	xqq
    }
    else if(sensorModel == GADI_ISP_SENSOR_SC1045) //sc1045
    {
        LOG_INFO("day, sensor -- sc1045\n");
        contrastAttr.enableAuto     = 1;
        //contrastAttr.manualStrength = 128;
        //contrastAttr.autoStrength   = ispDefParams.contrast;//96;
        contrastAttr.manualStrength = 114;
        contrastAttr.autoStrength   = 114;//96;
        //gapp_image_set_saturation(76);
        //gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);	xqq
    
        aeAttr.shutterTimeMin = 16000;//1024;
        aeAttr.shutterTimeMax = slowFramerate;//(aNtiFlickerParam.freq == GADI_ISP_ANTIFLICKER_FREQ_50HZ) ? 25 : 30;
        aeAttr.gainMax        = 27;
        aeAttr.gainMin        = 1;
    
       // ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);
       // gadi_set_auto_local_wdr_mode(ispHandle, 0);	xqq
    
    }
    else if(sensorModel == GADI_ISP_SENSOR_SC1145) //sc1045
    {
        LOG_INFO("day, sensor -- sc1145\n");
        contrastAttr.enableAuto     = 1;
        contrastAttr.manualStrength = 128;
        //contrastAttr.autoStrength   = ispDefParams.contrast;//96;
        contrastAttr.autoStrength   = 72;
        //gapp_image_set_saturation(76);
        //gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);	xqq
    
        aeAttr.shutterTimeMin = 16000;//1024;
        aeAttr.shutterTimeMax = slowFramerate;//(aNtiFlickerParam.freq == GADI_ISP_ANTIFLICKER_FREQ_50HZ) ? 25 : 30;
        aeAttr.gainMax        = 27;
        aeAttr.gainMin        = 1;
    
//        ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);
//        gadi_set_auto_local_wdr_mode(ispHandle, 0);	xqq
    }
    else if(sensorModel == GADI_ISP_SENSOR_GC1024) //gc1024
    {
        LOG_INFO("day, sensor -- gc1024\n");
        contrastAttr.enableAuto     = 1;
        contrastAttr.manualStrength = 128;
        contrastAttr.autoStrength   = ispDefParams.contrast;//96;
        //gapp_image_set_saturation(76);
        //gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);	xqq
    
        aeAttr.shutterTimeMin = 16000;//1024;
        aeAttr.shutterTimeMax = slowFramerate;//(aNtiFlickerParam.freq == GADI_ISP_ANTIFLICKER_FREQ_50HZ) ? 25 : 30;
        aeAttr.gainMax        = 27;
        aeAttr.gainMin        = 1;
    
        //ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);	xqq
        //gadi_set_auto_local_wdr_mode(0);
    }
    else if(sensorModel == GADI_ISP_SENSOR_GC1034) //gc1034
    {
        LOG_INFO("day, sensor -- gc1034\n");
        contrastAttr.enableAuto     = 1;
        contrastAttr.manualStrength = 128;
        contrastAttr.autoStrength   = ispDefParams.contrast;
        //gapp_image_set_saturation(76);
        //gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);		xqq
    
        aeAttr.shutterTimeMin = 16000;//1024;
        aeAttr.shutterTimeMax = slowFramerate;//(aNtiFlickerParam.freq == GADI_ISP_ANTIFLICKER_FREQ_50HZ) ? 25 : 30;
        aeAttr.gainMax        = 27;
        aeAttr.gainMin        = 1;
    
        //ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);	xqq
        //gadi_set_auto_local_wdr_mode(0);
    }
    //#ifdef SDKV20
    else if(sensorModel == GADI_ISP_SENSOR_GC2023) //gc2023
    {
        LOG_INFO("day, sensor -- gc2023\n");
        contrastAttr.enableAuto     = 1;
        contrastAttr.manualStrength = 128;
        contrastAttr.autoStrength   = ispDefParams.contrast;//96;
        //gapp_image_set_saturation(76);
        //gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);	xqq
    
        aeAttr.shutterTimeMin = 16000;//1024;
        aeAttr.shutterTimeMax = slowFramerate;//(aNtiFlickerParam.freq == GADI_ISP_ANTIFLICKER_FREQ_50HZ) ? 25 : 30;
        aeAttr.gainMax        = 27;
        aeAttr.gainMin        = 1;
    
//        ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);	xqq
        //gadi_set_auto_local_wdr_mode(0);
    }
    //#endif
    else if(sensorModel == GADI_ISP_SENSOR_GC2033) //gc2033
    {
        LOG_INFO("day, sensor -- gc2033\n");
        contrastAttr.enableAuto     = 1;
        contrastAttr.manualStrength = 128;
        contrastAttr.autoStrength   = ispDefParams.contrast;//96;
        //gapp_image_set_saturation(76);
        //gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);		xqq
    
        aeAttr.shutterTimeMin = 16000;//1024;
        aeAttr.shutterTimeMax = slowFramerate;//(aNtiFlickerParam.freq == GADI_ISP_ANTIFLICKER_FREQ_50HZ) ? 25 : 30;
        aeAttr.gainMax        = 27;
        aeAttr.gainMin        = 1;
    
//        ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);	xqq
        //gadi_set_auto_local_wdr_mode(0);
    }
    else if(sensorModel == GADI_ISP_SENSOR_GC2053) //gc2053
    {
        LOG_INFO("day, sensor -- gc2053\n");
        contrastAttr.enableAuto     = 0;
        contrastAttr.manualStrength = 64;
        contrastAttr.autoStrength   = 64;//72;
        //gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);	xqq
        contrastAttr.enableAuto     = 1;
        contrastAttr.manualStrength = 128;
        contrastAttr.autoStrength   = 128;//72;
        //gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);	xqq
    
        aeAttr.shutterTimeMin = 8000;//1024;
        aeAttr.shutterTimeMax = slowFramerate;
        aeAttr.gainMax        = 30;
        aeAttr.gainMin        = 1;
    
//       ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);	xqq
        //gadi_set_auto_local_wdr_mode(0);
    }
    else if(sensorModel == GADI_ISP_SENSOR_GC4623) //gc4623
    {
        LOG_INFO("day, sensor -- gc4623\n");
        contrastAttr.enableAuto     = 1;
        contrastAttr.manualStrength = 128;
        contrastAttr.autoStrength   = 128;
        //gapp_image_set_saturation(76);
        //gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);	xqq
    
        aeAttr.shutterTimeMin = 8000;//1024;
        aeAttr.shutterTimeMax = slowFramerate;//(aNtiFlickerParam.freq == GADI_ISP_ANTIFLICKER_FREQ_50HZ) ? 25 : 30;
        aeAttr.gainMax        = 30;
        aeAttr.gainMin        = 1;
    
//        ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);	xqq
        //gadi_set_auto_local_wdr_mode(0);
    }
    else if(sensorModel == GADI_ISP_SENSOR_AR0130)   //ar0130
    {
        LOG_INFO("day ar0130\n");
        aeAttr.shutterTimeMin = 8000;//1024;
        aeAttr.shutterTimeMax = slowFramerate;
        aeAttr.gainMax          = 36;
        aeAttr.gainMin          = 1;
        //gadi_isp_set_ae_attr(ispHandle, &aeAttr);	xqq
        contrastAttr.enableAuto     = 1;
        contrastAttr.manualStrength = 128;
        contrastAttr.autoStrength   = ispDefParams.contrast;
        //gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);	xqq
    }
    else if(sensorModel == GADI_ISP_SENSOR_MIS1002)   //ar0130
    {
        LOG_INFO("day mis1002\n");
        contrastAttr.enableAuto     = 1;
        contrastAttr.manualStrength = 128;
        contrastAttr.autoStrength   = ispDefParams.contrast;
        //gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);	xqq
    
        aeAttr.shutterTimeMin = 8000;//1024;
        aeAttr.shutterTimeMax = slowFramerate;
        aeAttr.gainMax          = 31;
        aeAttr.gainMin          = 1;
        //gadi_isp_set_ae_attr(ispHandle, &aeAttr);	xqq
    }
    else if(sensorModel == GADI_ISP_SENSOR_AR0237) //ar0237
    {
        LOG_INFO("day ar0237\n");
        contrastAttr.enableAuto     = 1;
        contrastAttr.manualStrength = 128;
        contrastAttr.autoStrength   = ispDefParams.contrast;
        //gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);		xqq
    
        aeAttr.shutterTimeMin = 8000;//1024;
        aeAttr.shutterTimeMax = slowFramerate;
        aeAttr.gainMax          = 36;
        aeAttr.gainMin          = 1;
//        ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);	xqq
    }
    else if(sensorModel == GADI_ISP_SENSOR_SC2035) //sc2035
    {
        LOG_INFO("day sc2035\n");
        contrastAttr.enableAuto     = 1;
        contrastAttr.manualStrength = 128;
        contrastAttr.autoStrength   = ispDefParams.contrast;
        //gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);		xqq
    
        aeAttr.shutterTimeMin = 16000;//1024;
        aeAttr.shutterTimeMax = slowFramerate;
        aeAttr.gainMax          = 35;
        aeAttr.gainMin          = 1;
       // ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);
       // gadi_set_auto_local_wdr_mode(ispHandle, 80);		xqq
    }
    else if(sensorModel == GADI_ISP_SENSOR_SC2135) //sc2135
    {
        LOG_INFO("day sc2135\n");
        contrastAttr.enableAuto     = 1;
        contrastAttr.manualStrength = 128;
        //contrastAttr.autoStrength   = ispDefParams.contrast;
        contrastAttr.autoStrength   = 32;
        //gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);		xqq
    
        aeAttr.shutterTimeMin = 16000;//1024;
        aeAttr.shutterTimeMax = slowFramerate;
        aeAttr.gainMax          = 23;
        aeAttr.gainMin          = 1;
       // ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);
       // gadi_set_auto_local_wdr_mode(ispHandle, 80);	xqq
    }
    else if(sensorModel == GADI_ISP_SENSOR_OV9710) //ov9710
    {
        LOG_INFO("day ov9710\n");
        contrastAttr.enableAuto     = 1;
        contrastAttr.manualStrength = 128;
        contrastAttr.autoStrength   = ispDefParams.contrast;
        //gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);	xqqq
    
        aeAttr.shutterTimeMin = 8000;//1024;
        aeAttr.shutterTimeMax = slowFramerate;
        aeAttr.gainMax          = 28;
        aeAttr.gainMin          = 1;
       // ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);
       // gadi_set_auto_local_wdr_mode(ispHandle, 0);	xqq
    }
    else if(sensorModel == GADI_ISP_SENSOR_OV9732)
    {
        LOG_INFO("day ov9732\n");
        contrastAttr.enableAuto     = 1;
        contrastAttr.manualStrength = 128;
        contrastAttr.autoStrength    = 48;//96;
        //gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);	xqq
        aeAttr.shutterTimeMin = 8000;//1024;
        aeAttr.shutterTimeMax = slowFramerate;
        aeAttr.gainMax          = 24;
        aeAttr.gainMin          = 1;
//        ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);	xqq
    }
    else if (sensorModel == GADI_ISP_SENSOR_SC2232) //SC2232
    {
        aeAttr.shutterTimeMin = 8000;//1024;
        aeAttr.shutterTimeMax = slowFramerate;
        aeAttr.gainMax        = 30;
        aeAttr.gainMin        = 1;
        contrastAttr.enableAuto     = 1;
        contrastAttr.manualStrength = 128;
        contrastAttr.autoStrength   = 128;//
       // gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);
    
       // gadi_isp_set_ae_attr(ispHandle, &aeAttr);
       // gadi_set_auto_local_wdr_mode(ispHandle, 96);	xqq
    }
    else if (sensorModel == GADI_ISP_SENSOR_SC4236) //SC4236
    {
        aeAttr.shutterTimeMin = 8000;//1024;
        aeAttr.shutterTimeMax = slowFramerate;
        aeAttr.gainMax        = 30;
        aeAttr.gainMin        = 1;
        contrastAttr.enableAuto     = 1;
        contrastAttr.manualStrength = 128;
        contrastAttr.autoStrength   = 128;//
       // gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);
    
       // gadi_isp_set_ae_attr(ispHandle, &aeAttr);	xqq
    }
    else if (sensorModel == GADI_ISP_SENSOR_OV4689) //OV4689
    {
        aeAttr.shutterTimeMin = 8000;//1024;
        aeAttr.shutterTimeMax = slowFramerate;
        aeAttr.gainMax        = 30;
        aeAttr.gainMin        = 1;
        contrastAttr.enableAuto     = 1;
        contrastAttr.manualStrength = 128;
        contrastAttr.autoStrength   = 128;//
       // gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);
       // gadi_isp_set_ae_attr(ispHandle, &aeAttr);	xqq
    }
    else if (sensorModel == GADI_ISP_SENSOR_OS04B10)    //OS04b10
    {
        LOG_INFO("day OS04b10\n");
        aeAttr.shutterTimeMin = 20000;//1024;
        aeAttr.shutterTimeMax = slowFramerate;
        aeAttr.gainMax        = 30;
        aeAttr.gainMin        = 1;
        contrastAttr.enableAuto     = 1;
        contrastAttr.manualStrength = 128;
        contrastAttr.autoStrength   = 72;//
       // gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);
       // gadi_isp_set_ae_attr(ispHandle, &aeAttr);
       // gadi_isp_set_wb_type(ispHandle, GADI_ISP_WB_TYPE_AUTO);	xqq
    }    
    else if (sensorModel == GADI_ISP_SENSOR_JXQ03)    //jxq03
    {
        aeAttr.shutterTimeMin = 8000;//1024;
        aeAttr.shutterTimeMax = slowFramerate;
        aeAttr.gainMax        = 30;
        aeAttr.gainMin        = 1;
        contrastAttr.enableAuto     = 1;
        contrastAttr.manualStrength = 128;
        contrastAttr.autoStrength   = 128;//
       // gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);
       // gadi_isp_set_ae_attr(ispHandle, &aeAttr);	xqq
    }
    else if (sensorModel == GADI_ISP_SENSOR_GC4653)    //gc4653
    {
        LOG_INFO("day gc4653\n");
        aeAttr.shutterTimeMin = 24000;//1024;
        aeAttr.shutterTimeMax = slowFramerate;
        aeAttr.gainMax        = 30;
        aeAttr.gainMin        = 1;
        contrastAttr.enableAuto     = 0;
        contrastAttr.manualStrength = 128;
        contrastAttr.autoStrength   = 128;//
       // gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);
    
       // gadi_isp_set_ae_attr(ispHandle, &aeAttr);	xqq
    }
    else    //other sensors
    {
       LOG_ERR("day, sensorModel:0x%x\n", sensorModel);
       aeAttr.shutterTimeMin = 8000;//1024;
       aeAttr.shutterTimeMax = slowFramerate;
       aeAttr.gainMax        = 30;
       aeAttr.gainMin        = 1;
       contrastAttr.enableAuto     = 1;
       contrastAttr.manualStrength = 128;
       contrastAttr.autoStrength   = 128;//
      // gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);
      // 
      // gadi_isp_set_ae_attr(ispHandle, &aeAttr);	xqq
    }
    
//    gadi_isp_get_lowbitrate_attr(ispHandle, &lowbitAttrPtr);	xqq
    lowbitAttrPtr.enable = 1;
#if 0
    lowbitAttrPtr.sharpness_ratio = 25;
    lowbitAttrPtr.cfa_denoise_property_down_ratio = 25;
    lowbitAttrPtr.cfa_denoise_property_up_ratio = 120;
#endif
    //lowbitAttrPtr.sharpness_ratio = 0;
    //lowbitAttrPtr.cfa_denoise_property_down_ratio = 0;
    //lowbitAttrPtr.cfa_denoise_property_up_ratio = 0;
    lowbitAttrPtr.tolerance = 8;
    lowbitAttrPtr.delay_frames = 40;
//    gadi_isp_set_lowbitrate_attr(ispHandle, &lowbitAttrPtr);
//    
//    // ir cut
//    gadi_vi_set_ircut_control(viHandle, GADI_VI_IRCUT_DAY);
//    gadi_sys_thread_sleep(300);
//    gadi_vi_set_ircut_control(viHandle, GADI_VI_IRCUT_CLEAR);		xqq
}

static void isp_all_color_mode(int swicth_db, int tolerate/*0~100*/)
{
    double exposure_threshold;
    double exposure_current;
    double shutter_time_xms;
    GADI_U32 viFramerate;
    GADI_ISP_AeAttrT    aeAttr;
    goke_encode_stream_params* strParams;
    GADI_VENC_FrameRateT stFrameRate;
    int cnt;

    exposure_current = isp_get_current_exposure_by_xframe(60);
    exposure_threshold = 1000/25*pow(2,swicth_db/6.0);

    if (exposure_current == 0.0)
    {
        printf("Get current exposure error!\n");
        return;
    }

    if (all_color_mode_status == ALL_COLOR_MODE_CLOSE)
    {

        //exposure time change to long, over exposure_threshold.
        if (exposure_current > exposure_threshold)
        {

			LOG_INFO("@@@@@@@@@isp_all_color_mode############%f,%f## 1\n",exposure_current,exposure_threshold);
            if (ispBulbMode == ISP_BULB_MODE_IR)
            {
               // gadi_isp_set_day_night_mode(ispHandle, 0);

               // gadi_vi_set_ircut_control(viHandle, GADI_VI_IRCUT_NIGHT);
               // gadi_sys_thread_sleep(500);
               // gadi_vi_set_ircut_control(viHandle, GADI_VI_IRCUT_CLEAR);	xqq

                /* disable framerate operation for hc project */
#if 0		//xqq change 1 to 0
                if(gadi_vi_get_framerate(viHandle, &viFramerate) == GADI_OK)
                {
                    LOG_INFO("auto slowshutter current vi_framerate:%d  target %d\n", viFramerate, ispTargetViFramerate);
                    if(viFramerate > ispTargetViFramerate)
                    {

                        ispOrginalViFramerate = viFramerate;

                        gadi_vi_set_framerate(viHandle, ispTargetViFramerate);

                        gadi_isp_get_ae_attr(ispHandle, &aeAttr);

                        aeAttr.shutterTimeMax = ispTargetViFramerate;
                        aeAttr.gainMax        = 36;

                        gadi_isp_set_ae_attr(ispHandle, &aeAttr);
                    }
                }

                for (cnt = 0; cnt < gk_get_max_stream_nums(); cnt++)
                {
                    strParams = gk_get_enc_stream_par(cnt);
                    if(strParams != NULL)
                    {
                        LOG_INFO("stFrameRate.fps:%d->%d\n",strParams->streamFormat.fps ,ispTargetViFramerate);
                        stFrameRate.streamId = cnt;
                        stFrameRate.fps = ispTargetViFramerate;
                        //gadi_venc_set_framerate(vencHandle, &stFrameRate);	xqq

                        //strParams->streamFormat.fps = ispTargetViFramerate;
                    }
                }
#endif
            }

            LOG_INFO("@@@@@@@@@@set ircut NIGHT\n");
            gk_isp_led_on();
            //save status
            all_color_mode_status = ALL_COLOR_MODE_OPEN;
            exposure_swicth_before = exposure_current;

            /* wait for isp3a ae stable */
            //gadi_sys_thread_sleep(2000);	xqq

//            if (gadi_isp_get_current_shutter(ispHandle, &shutter_time_xms) < GADI_OK)
//            {
//                shutter_time_xms = 40;//ms
//            }
//            //delay 5 frame.
//            gadi_sys_thread_sleep(shutter_time_xms*25*5);	xqq
            exposure_swicth_after = isp_get_current_exposure_by_xframe(30);
            if (exposure_swicth_after == 0.0)
                exposure_swicth_after = exposure_swicth_before;

            exposure_tolerate = isp_calculate_exposure_tolerate(
                exposure_swicth_before, exposure_swicth_after,
                exposure_threshold, tolerate);
            exposure_invert_tolerate = exposure_tolerate*exposure_tolerate;
#if 1
            printf("#########################################1\n");
            printf("exposure_swicth_before:%f\n", exposure_swicth_before);
            printf("exposure_swicth_after:%f\n", exposure_swicth_after);
            printf("exposure_tolerate:%f\n", exposure_tolerate);
            printf("exposure_invert_tolerate:%f\n", exposure_invert_tolerate);
            printf("#########################################\n");
#endif
            if (exposure_current < exposure_swicth_after)
            {
                printf("exposure is not normal work status!\n");
                all_color_mode_status = ALL_COLOR_MODE_CLOSE;
                if (ispBulbMode == ISP_BULB_MODE_IR)
                {
                   // gadi_isp_set_day_night_mode(ispHandle, 1);

                   // gadi_vi_set_ircut_control(viHandle, GADI_VI_IRCUT_DAY);
                   // gadi_sys_thread_sleep(500);
                   // gadi_vi_set_ircut_control(viHandle, GADI_VI_IRCUT_CLEAR);

                   // gadi_isp_set_day_night_mode(ispHandle, 1);	xqq

                    /* disable framerate operation for hc project */
#if 0			//xqq change 1 to 0
                    if(gadi_vi_get_framerate(viHandle, &viFramerate) == GADI_OK)
                    {
                        LOG_INFO("auto slowshutter current vi_framerate:%d  original:%d \n", viFramerate,ispOrginalViFramerate);
                        if(viFramerate < ispOrginalViFramerate)
                        {
                            gadi_vi_set_framerate(viHandle, ispOrginalViFramerate);

                            gadi_isp_get_ae_attr(ispHandle, &aeAttr);

                            aeAttr.shutterTimeMax = ispOrginalViFramerate;
                            aeAttr.gainMax        = 36;

                            gadi_isp_set_ae_attr(ispHandle, &aeAttr);
                        }
                    }

                    for (cnt = 0; cnt < gk_get_max_stream_nums(); cnt++)
                    {
                        strParams = gk_get_enc_stream_par(cnt);
                        if(strParams != NULL)
                        {
                            LOG_INFO("stream:%d, stFrameRate.fps:%d->%d\n",cnt, strParams->streamFormat.fps);
                            stFrameRate.streamId = cnt;
                            stFrameRate.fps = strParams->streamFormat.fps;
                            //gadi_venc_set_framerate(vencHandle, &stFrameRate);	xqq
                        }
                    }
#endif
                }
                LOG_INFO("@@@@@@@@@@set ircut DAY\n");

                gk_isp_led_off();
            }
        }
    }
    else
    {
        if (exposure_current < exposure_swicth_after-exposure_tolerate ||
            exposure_current > exposure_swicth_after+exposure_invert_tolerate)
        {
            if (ispBulbMode == ISP_BULB_MODE_IR)
            {
               // gadi_isp_set_day_night_mode(ispHandle, 1);

               // gadi_vi_set_ircut_control(viHandle, GADI_VI_IRCUT_DAY);
               // gadi_sys_thread_sleep(500);
               // gadi_vi_set_ircut_control(viHandle, GADI_VI_IRCUT_CLEAR);

               // gadi_isp_set_day_night_mode(ispHandle, 1);	xqq

                /* disable framerate operation for hc project */
#if 0		//change 1 to 0 xqq
                if(gadi_vi_get_framerate(viHandle, &viFramerate) == GADI_OK)
                {
                    LOG_INFO("auto slowshutter current vi_framerate:%d  original:%d \n", viFramerate,ispOrginalViFramerate);
                    if(viFramerate < ispOrginalViFramerate)
                    {
                        gadi_vi_set_framerate(viHandle, ispOrginalViFramerate);

                        gadi_isp_get_ae_attr(ispHandle, &aeAttr);

                        aeAttr.shutterTimeMax = ispOrginalViFramerate;
                        aeAttr.gainMax        = 36;

                        gadi_isp_set_ae_attr(ispHandle, &aeAttr);
                    }
                }

                for (cnt = 0; cnt < gk_get_max_stream_nums(); cnt++)
                {
                    strParams = gk_get_enc_stream_par(cnt);
                    if(strParams != NULL)
                    {
                        LOG_INFO("stream:%d, stFrameRate.fps:%d->%d\n",cnt, strParams->streamFormat.fps);
                        stFrameRate.streamId = cnt;
                        stFrameRate.fps = strParams->streamFormat.fps;
                        //gadi_venc_set_framerate(vencHandle, &stFrameRate);	xqq
                    }
                }
#endif
            }
            LOG_INFO("@@@@@@@@@@set ircut DAY\n");

            gk_isp_led_off();
            //save status
            all_color_mode_status = ALL_COLOR_MODE_CLOSE;

#if 0
            printf("#########################################2\n");
            printf("exposure_current:%f\n", exposure_current);
            printf("exposure_swicth_after:%f\n", exposure_swicth_after);
            printf("exposure_tolerate:%f\n", exposure_tolerate);
            printf("exposure_invert_tolerate:%f\n", exposure_invert_tolerate);
            printf("#########################################\n");
#endif
        }
        else
        {
#if 0
            printf("#########################################4\n");
            printf("exposure_current:%f\n", exposure_current);
            printf("exposure_swicth_before:%f\n", exposure_swicth_before);
            printf("exposure_swicth_after:%f\n", exposure_swicth_after);
            printf("exposure_tolerate:%f\n", exposure_tolerate);
            printf("exposure_invert_tolerate:%f\n", exposure_invert_tolerate);
            printf("#########################################\n");
#endif
        }
    }
}

void sdk_isp_set_md_light_mode(int mode)
{
    ispMdLight = mode;
}

static void isp_auto_slowshutter0(void)
{
	GADI_U32 gain;
	GADI_U32 viFramerate;
	GADI_ISP_AeAttrT aeAttr;
	double shutter;
	int exposure;
	int down_exposure;
	int up_exposure;
	int last_exposure;
	int equal_exposure_down_cnt = 0;
	int equal_exposure_up_cnt = 0;
	GADI_VI_SettingParamsT viParams;
	GADI_ERR retVal = GADI_OK;
	int up_gain;

	while(ispAutoSlowShutterRunFlag)
	{
#if 0
		if (gadi_isp_get_current_gain(ispHandle, &gain) < GADI_OK)
		{
			continue;
		}

		if (gadi_isp_get_current_shutter(ispHandle, &shutter) < GADI_OK)
		{
			continue;
		}
#endif		//xqq
		down_exposure = pow(10,ispTargetGain/20.0) * 50;
		up_gain = ispTargetGain*5/8;
		up_exposure = 4*down_exposure/5;
		exposure = shutter * pow(10,gain/20.0);

#if 0
		if(last_exposure != exposure)
		{
			equal_exposure_cnt = 0;
		}
		else
		{
			equal_exposure_cnt++;
		}
		last_exposure = exposure;

		if(equal_exposure_cnt <= 5)
		{
			gadi_sys_thread_sleep(500);
			continue;
		}
#else
		if(ispAutoSlowShutterEnableFlag ==GADI_FALSE)
		{
			if(exposure > down_exposure)
			{
				//gadi_sys_thread_sleep(50);	xqq
				equal_exposure_down_cnt++;
			}
			else
			{
//				gadi_sys_thread_sleep(50);	xqq
				equal_exposure_down_cnt = 0;
			}
			//last_exposure = exposure;

			if(equal_exposure_down_cnt <= 5)
			{
				//gadi_sys_thread_sleep(50);	xqq
				continue;
			}
		}
		else
		{
			if(exposure < up_exposure)
			{
				equal_exposure_up_cnt++;
				//gadi_sys_thread_sleep(100);	xqq
			}
			else
			{
				equal_exposure_up_cnt = 0;
				//gadi_sys_thread_sleep(100);	xqq
			}
			//last_exposure = exposure;

			if(equal_exposure_up_cnt <= 5)
			{
				//gadi_sys_thread_sleep(40);	xqq
				continue;
			}
		}
#endif
		LOG_INFO("gain %d, shutter %f, exposure %d(down:%4d up:%4d) [%d, %d]\n", gain, shutter, exposure, equal_exposure_down_cnt, equal_exposure_up_cnt, up_exposure, down_exposure);
#if 0		//xqq
		if((exposure > down_exposure && equal_exposure_down_cnt >= 5) && (ispAutoSlowShutterEnableFlag ==GADI_FALSE))
		{
			if(gadi_vi_get_framerate(viHandle, &viFramerate) == GADI_OK)
			{
				if(viFramerate > ispTargetViFramerate)
				{
					LOG_INFO("auto slowshutter current vi_framerate:%d->%d\n", viFramerate, ispTargetViFramerate);
					ispOrginalViFramerate = viFramerate;

					gadi_vi_set_framerate(viHandle, ispTargetViFramerate);

					gadi_sys_thread_sleep(200);
					gadi_isp_get_ae_attr(ispHandle, &aeAttr);
					//aeAttr.shutterTimeMin = 8000;//1024;
					aeAttr.shutterTimeMax = ispTargetViFramerate;
					//aeAttr.gainMax		= 30;
					//aeAttr.gainMin		= 1;
					gadi_isp_set_ae_attr(ispHandle, &aeAttr);
					equal_exposure_down_cnt = 0;
					ispAutoSlowShutterEnableFlag = GADI_TRUE;
					gadi_sys_thread_sleep(200);
				}
			}
		}
		else if((exposure < up_exposure && equal_exposure_up_cnt >= 5) && (ispAutoSlowShutterEnableFlag == GADI_TRUE))
		{
			if(gadi_vi_get_framerate(viHandle, &viFramerate) == GADI_OK)
			{
				if(viFramerate < ispOrginalViFramerate)
				{
					LOG_INFO("auto slowshutter current vi_framerate:%d->%d\n", viFramerate, ispOrginalViFramerate);
					//gdm_vi_set_framerate(0, ispOrginalViFramerate);
					gadi_isp_get_ae_attr(ispHandle, &aeAttr);
					//aeAttr.shutterTimeMin = 8000;//1024;
					aeAttr.shutterTimeMax = ispOrginalViFramerate;
					//aeAttr.gainMax		= 30;
					//aeAttr.gainMin		= 1;
					gadi_isp_set_ae_attr(ispHandle, &aeAttr);
					gadi_sys_thread_sleep(200);
					//printf("set fps\n");
					gadi_vi_set_framerate(viHandle, ispOrginalViFramerate);
					equal_exposure_up_cnt = 0;
					ispAutoSlowShutterEnableFlag = GADI_FALSE;
					gadi_sys_thread_sleep(200);
				}
			}
		}

		//gadi_sys_thread_sleep(2000);
	}
#endif		//xqq
	}
}

static void isp_auto_slowshutter1(void)
{
    while(ispAutoSlowShutterRunFlag)
    {
        if ((ispBulbMode == ISP_BULB_MODE_IR) || (ispBulbMode == ISP_BULB_MODE_MIX))
        {
            isp_all_color_mode(18, 50);
        }
        else if(ispBulbMode == ISP_BULB_MODE_ALL_COLOR)
        {
            if (0 == ircutFlag)
            {
               // gadi_isp_set_day_night_mode(ispHandle, 1);

               // gadi_vi_set_ircut_control(viHandle, GADI_VI_IRCUT_DAY);
               // gadi_sys_thread_sleep(500);
               // gadi_vi_set_ircut_control(viHandle, GADI_VI_IRCUT_CLEAR);	xqq

                ircutFlag = 1;
            }
            isp_all_color_mode(18, 50);
        }
        else if(ispBulbMode == ISP_BULB_MODE_MANUAL)
        {
            if (0 == ircutFlag)
            {
               // gadi_isp_set_day_night_mode(ispHandle, 1);

               // gadi_vi_set_ircut_control(viHandle, GADI_VI_IRCUT_DAY);
               // gadi_sys_thread_sleep(500);
               // gadi_vi_set_ircut_control(viHandle, GADI_VI_IRCUT_CLEAR);	xqq

                ircutFlag = 1;
            }

            if (ispBulbLevel)
            {
				if (0 == gk_isp_get_led_state())
				{
					gk_isp_led_on();
				}
            }
            else
            {
 				if (1 == gk_isp_get_led_state())
				{
					gk_isp_led_off();
				}
            }
        }
       // else
       // {
       //     gadi_sys_thread_sleep(200);
       // }	xqq
    }
}

static void isp_auto_slowshutter_thread(void *arg)
{
    isp_auto_slowshutter1();
}

GADI_ERR isp_register_auto_slowshutter(u32 targetGain, u32 targetViFramerate)
{
    if(ispAutoSlowShutterThread)
    {
        return GADI_OK;
    }

    ispAutoSlowShutterRunFlag = GADI_TRUE;

    ispTargetGain = targetGain;
    ispTargetViFramerate = targetViFramerate;

    LOG_INFO("isp_register_auto_slowshutterXXX\n");
    /* running auto IR-cut*/
    //gadi_sys_thread_create(isp_auto_slowshutter_thread, NULL, GADI_SYS_THREAD_PRIO_DEFAULT,
      //  GADI_SYS_THREAD_STATCK_SIZE_DEFAULT, "auto_slowshutter", &ispAutoSlowShutterThread);	xqq

    return 0;
}

static int gpio_is_valid(int gpioNum)
{
    if(gpioNum <= 0)
    {
        return 0;
    }
    else
    {
        return 1;
    }

}

static int isp_set_slow_framerate(int setSlow)
{
    GADI_VI_FrameRateEnumT fps;
    unsigned int cnt;
    goke_encode_stream_params* strParams;
    GADI_VENC_FrameRateT stFrameRate;

    if((slowLight.isLowFrame == 1 || slowLight.isLowFrame == -1) && setSlow == 0)
    {
        slowLight.isLowFrame = 0;
        /*change vin(sensor) & capture.*/
        fps = GADI_VI_FPS_AUTO;
#if 0
        if(gadi_vi_set_framerate(viHandle, fps) != GADI_OK)
        {
            LOG_ERR("resume normal mode frome slowshutter failed.\n");
            return -1;
        }
        if(gadi_vi_get_framerate(viHandle, &fps) != GADI_OK)
        {
            LOG_ERR("resume normal mode frome slowshutter failed.\n");
            return -1;
        }
	LOG_INFO("daylight mode: slow shutter: framerate=%d!\n", fps);
#endif		//xqq
        /*change venc framerate.*/
        /*only change the stream 0 & stream 1 framerate.*/
        int stream_num = gk_get_max_stream_nums();
        for (cnt = 0; cnt < stream_num; cnt++)
        {
            strParams = gk_get_enc_stream_par(cnt);
            if(strParams != NULL)
            {
                stFrameRate.streamId = cnt;
                stFrameRate.fps = strParams->streamFormat.fps;

                LOG_INFO("day, stream%d set fps=%d\n", cnt, stFrameRate.fps);
                //gadi_venc_set_framerate(vencHandle, &stFrameRate);	xqq
            }
        }
    }
    else  if((slowLight.isLowFrame == 0  || slowLight.isLowFrame == -1) && setSlow == 1)
    {
        slowLight.isLowFrame = 1;
        /*change vin(sensor) & capture. 2: means 15fps. 3: means 10fps.*/
        fps = GADI_VI_FPS_15;
#if 0
        if(gadi_vi_set_framerate(viHandle, fps) != GADI_OK)
        {
            LOG_ERR("enter slowshutter mode[0] failed.\n");
            return -1;
        }
    	LOG_INFO("night mode: slow shutter: framerate=%d!\n", fps);
#endif		//xqq
        /*change venc framerate.*/
        /*only change the stream 0 & stream 1 framerate.*/
        int stream_num = gk_get_max_stream_nums();
        for (cnt = 0; cnt < stream_num; cnt++)
        {
            strParams = gk_get_enc_stream_par(cnt);
            if(strParams != NULL)
            {
                stFrameRate.streamId = cnt;
                stFrameRate.fps = fps;

                LOG_INFO("night, stream%d set fps=%d\n", cnt, stFrameRate.fps);
                //gadi_venc_set_framerate(vencHandle, &stFrameRate);	xqq
            }
        }
    }
    else
    {
        return -1;
    }

    return fps;
}

static int vi_set_mirror(uint8_t enable)
{
    GADI_VI_MirrorModeParamsT mirrorMode;
    GADI_ERR errorCode = GADI_OK;

//    errorCode = gadi_vi_get_mirror_mode(viHandle, &mirrorMode);	xqq
    if (errorCode != GADI_OK){
        LOG_ERR("get vi parameters failed %d\n", errorCode);
    }
    if (enable) {
        switch (mirrorMode.mirrorPattern) {
            case GADI_VI_MIRROR_HORRIZONTALLY:
            case GADI_VI_MIRROR_HORRIZONTALLY_VERTICALLY:
                break;

            case GADI_VI_MIRROR_NONE:
                mirrorMode.mirrorPattern = GADI_VI_MIRROR_HORRIZONTALLY;
                break;

            case GADI_VI_MIRROR_VERTICALLY:
                mirrorMode.mirrorPattern = GADI_VI_MIRROR_HORRIZONTALLY_VERTICALLY;
                break;

            default:
                break;
        }
    }
    else {
        switch (mirrorMode.mirrorPattern) {
            case GADI_VI_MIRROR_HORRIZONTALLY:
                mirrorMode.mirrorPattern = GADI_VI_MIRROR_NONE;
                break;

            case GADI_VI_MIRROR_HORRIZONTALLY_VERTICALLY:
                mirrorMode.mirrorPattern = GADI_VI_MIRROR_VERTICALLY;
                break;

            case GADI_VI_MIRROR_NONE:
            case GADI_VI_MIRROR_VERTICALLY:
                break;

            default:
                break;
        }
    }

//    errorCode = gadi_vi_set_mirror_mode(viHandle, &mirrorMode);	xqq
    if (errorCode != GADI_OK) {
        LOG_ERR("set mirror mode parameters failed %d\n", errorCode);

	}
    return errorCode;
}

static int vi_set_flip(uint8_t enable)
{
    GADI_VI_MirrorModeParamsT mirrorMode;
    GADI_ERR               errorCode = GADI_OK;

//    errorCode = gadi_vi_get_mirror_mode(viHandle, &mirrorMode);	xqq
    if (errorCode != GADI_OK){
        LOG_ERR("get vi parameters failed %d\n", errorCode);
    }

    if (enable) {
        switch (mirrorMode.mirrorPattern) {
            case GADI_VI_MIRROR_VERTICALLY:
            case GADI_VI_MIRROR_HORRIZONTALLY_VERTICALLY:
                break;

            case GADI_VI_MIRROR_NONE:
                mirrorMode.mirrorPattern = GADI_VI_MIRROR_VERTICALLY;
                break;

            case GADI_VI_MIRROR_HORRIZONTALLY:
                mirrorMode.mirrorPattern = GADI_VI_MIRROR_HORRIZONTALLY_VERTICALLY;
                break;

            default:
                break;
        }
    }
    else {
        switch (mirrorMode.mirrorPattern) {
            case GADI_VI_MIRROR_VERTICALLY:
                mirrorMode.mirrorPattern = GADI_VI_MIRROR_NONE;
                break;

            case GADI_VI_MIRROR_HORRIZONTALLY_VERTICALLY:
                mirrorMode.mirrorPattern = GADI_VI_MIRROR_HORRIZONTALLY;
                break;

            case GADI_VI_MIRROR_NONE:
            case GADI_VI_MIRROR_HORRIZONTALLY:
                break;

            default:
                break;
        }
    }

//    errorCode = gadi_vi_set_mirror_mode(viHandle, &mirrorMode);	xqq
    if (errorCode != GADI_OK) {
        LOG_ERR("set mirror mode parameters failed %d\n", errorCode);
    }

    return errorCode;
}

static int isp_set_shutter(uint8_t frequency)
{
    GADI_ISP_AntiFlickerParamT    antiFlickerParams;
    GADI_ERR                      errorCode = GADI_OK;

    #if 0 //  the shutter is auto control by 3A.
    GADI_VI_SettingParamsT        viParams;
    errorCode = gadi_vi_get_params(viHandle, &viParams);
    if (errorCode != GADI_OK) {
        LOG_ERR("get vi param failed %d\n", errorCode);
    }

    viParams.frameRate = fps;

    errorCode = gadi_vi_set_params(viHandle, &viParams);
    if (errorCode != GADI_OK) {
        LOG_ERR("set vi param failed %d\n", errorCode);
    }
    #endif

//    errorCode = gadi_isp_get_antiflicker(ispHandle, &antiFlickerParams);	xqq
    if (errorCode != GADI_OK) {
        LOG_ERR("get antiflicker param failed %d\n", errorCode);
    }
    antiFlickerParams.enableDisable = 1;

    if (frequency == 50) {
        antiFlickerParams.freq = GADI_ISP_ANTIFLICKER_FREQ_50HZ;
    }
    else {
        antiFlickerParams.freq = GADI_ISP_ANTIFLICKER_FREQ_60HZ;
    }

//    errorCode = gadi_isp_set_antiflicker(ispHandle, &antiFlickerParams);	xqq
    if (errorCode != GADI_OK) {
        LOG_ERR("set antiflicker param failed %d\n", errorCode);
    }

    /* FIXME: adjus the ISP3A parameters if needed */

    return errorCode;
}


static void isp_ircut_switch(uint8_t bEnable)//0:daytime   1:night
{
    GADI_S32 slowFramerate = 0;
    GADI_ISP_ContrastAttrT contrastAttr;
    //EM_SENSOR_MODEL sensor_id;
    GADI_ISP_SensorModelEnumT sensorModel;
    GADI_ISP_AeAttrT    aeAttr;
    int ret;
    GADI_ISP_LowBitrateInfoT lowbitAttrPtr;

    if (isLedTest)
    {
        return;
    }

    sdk_isp_changed();
#if 0		//xqq
   // gadi_isp_get_ae_attr(ispHandle, &aeAttr);
   // gadi_isp_get_contrast_attr(ispHandle, &contrastAttr);	xqq
    if(!bEnable)
    {
        LOG_INFO("IRCUT:day light\n");
        gk_isp_led_off(); // if define gpio control ircut led, this code can valid
        irCutAttr.gpio_status_old = ISP_DAY;
        if ((ispBulbMode == ISP_BULB_MODE_IR) || \
            (ispBulbMode == ISP_BULB_MODE_MIX))
        {
           // gadi_isp_set_day_night_mode(ispHandle, 1);	xqq
           // gadi_isp_set_meter_mode(ispHandle, 2);		xqq
            if(slowLight.curMode == ISP_LOWLIGHT_MODE_ALLDAY) //must slow framerate
            {
                slowFramerate = isp_set_slow_framerate(1);
            }
            else
            {
                slowFramerate = isp_set_slow_framerate(0);
            }
            LOG_INFO("slow framerate:%d\n", slowFramerate);

            if(slowFramerate < 0)
            {
                LOG_ERR("set slow framerate failed.\n");
                return ;
            }

//            gadi_isp_get_sensor_model(ispHandle, &sensorModel);	xqq
            LOG_INFO("day SENSOR: 0x%x\n", sensorModel);
            if (sensorModel == GADI_ISP_SENSOR_JXH42)  //jxh42
            {
                LOG_INFO("day jxh42\n");
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = ispDefParams.contrast;//96;
//                gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);	xqq

                aeAttr.shutterTimeMin = 8000;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax        = 30;
                aeAttr.gainMin        = 1;

                //gadi_isp_set_ae_attr(ispHandle, &aeAttr);	xqq
            }
            else if (sensorModel == GADI_ISP_SENSOR_JXH61)  //jxh61
            {
                LOG_INFO("day jxh61\n");
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = ispDefParams.contrast;//96;
                //gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);	xqq

                aeAttr.shutterTimeMin = 8000;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax        = 30;
                aeAttr.gainMin        = 1;
               // gadi_isp_set_ae_attr(ispHandle, &aeAttr);

               // gadi_set_auto_local_wdr_mode(ispHandle, 112);	xqq
            }
            else if (sensorModel == GADI_ISP_SENSOR_JXF22)  //jxf22
            {
                LOG_INFO("day jxf22\n");
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = 64;//96;
                //gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);	xqq

                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax        = 30;
                aeAttr.gainMin        = 1;

               // ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);
               // gadi_set_auto_local_wdr_mode(ispHandle, 0);	xqq
            }
            else if(sensorModel == GADI_ISP_SENSOR_IMX222)   //imx222
            {
                LOG_INFO("day imx222\n");
                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax        = 36;
                aeAttr.gainMin        = 1;
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = ispDefParams.contrast;//96;
               // gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);
               // gadi_isp_set_ae_attr(ispHandle, &aeAttr);	xqq

            }
            else if(sensorModel == GADI_ISP_SENSOR_IMX291)   //imx291
            {
                LOG_INFO("day imx291\n");
                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax        = 42;
                aeAttr.gainMin        = 1;
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = ispDefParams.contrast;//96;
               // gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);
               // gadi_isp_set_ae_attr(ispHandle, &aeAttr);	xqq
            
            }
            else if(sensorModel == GADI_ISP_SENSOR_OV2710)   //ov2710
            {
                LOG_INFO("day ov2710\n");
                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax        = 36;
                aeAttr.gainMin        = 1;
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = ispDefParams.contrast;//96;
               // gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);
               // gadi_isp_set_ae_attr(ispHandle, &aeAttr);	xqq
            }
            else if(sensorModel == GADI_ISP_SENSOR_OV9750)   //ov9750
            {
                LOG_INFO("day ov9750\n");
                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax        = 36;
                aeAttr.gainMin        = 1;
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = 128;//96;
               // gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);
               // gadi_isp_set_ae_attr(ispHandle, &aeAttr);
               // gadi_set_auto_local_wdr_mode(ispHandle, 96);	xqqgadi_vi_set_framerate
            }
            else if(sensorModel == GADI_ISP_SENSOR_SC1035)   //sc1035
            {
                LOG_INFO("day sc1035\n");
                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax        = 33;
                aeAttr.gainMin        = 1;
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = ispDefParams.contrast;//96;
               // gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);
               // ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);	xqq
                if (ret != GADI_OK)
                {
                    LOG_ERR("set_ae_attr error day %d\n", ret);
                }
                //gadi_set_auto_local_wdr_mode(ispHandle, 90);	xqq
            }
            else if(sensorModel == GADI_ISP_SENSOR_SC1135)   //sc1135
            {
                LOG_INFO("day sc1135\n");
                aeAttr.shutterTimeMin = 16000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax        = 28;
                aeAttr.gainMin        = 1;
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = 64;//96;
               // gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);
               // gadi_isp_set_ae_attr(ispHandle, &aeAttr);
               // gadi_set_auto_local_wdr_mode(ispHandle, 96);	xqq
            }
            else if(sensorModel == GADI_ISP_SENSOR_SC1045) //sc1045
            {
                LOG_INFO("day, sensor -- sc1045\n");
                contrastAttr.enableAuto     = 1;
                //contrastAttr.manualStrength = 128;
                //contrastAttr.autoStrength   = ispDefParams.contrast;//96;
                contrastAttr.manualStrength = 114;
                contrastAttr.autoStrength   = 114;//96;
                //gapp_image_set_saturation(76);
                //gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);	xqq

                aeAttr.shutterTimeMin = 16000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;//(aNtiFlickerParam.freq == GADI_ISP_ANTIFLICKER_FREQ_50HZ) ? 25 : 30;
                aeAttr.gainMax        = 27;
                aeAttr.gainMin        = 1;

               // ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);
               // gadi_set_auto_local_wdr_mode(ispHandle, 0);	xqq

            }
            else if(sensorModel == GADI_ISP_SENSOR_SC1145) //sc1045
            {
                LOG_INFO("day, sensor -- sc1145\n");
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                //contrastAttr.autoStrength   = ispDefParams.contrast;//96;
                contrastAttr.autoStrength   = 72;
                //gapp_image_set_saturation(76);
                //gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);	xqq

                aeAttr.shutterTimeMin = 16000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;//(aNtiFlickerParam.freq == GADI_ISP_ANTIFLICKER_FREQ_50HZ) ? 25 : 30;
                aeAttr.gainMax        = 27;
                aeAttr.gainMin        = 1;

               // ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);
               // gadi_set_auto_local_wdr_mode(ispHandle, 0);	xqq
            }
            else if(sensorModel == GADI_ISP_SENSOR_GC1024) //gc1024
            {
                LOG_INFO("day, sensor -- gc1024\n");
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = ispDefParams.contrast;//96;
                //gapp_image_set_saturation(76);
                //gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);	xqq

                aeAttr.shutterTimeMin = 16000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;//(aNtiFlickerParam.freq == GADI_ISP_ANTIFLICKER_FREQ_50HZ) ? 25 : 30;
                aeAttr.gainMax        = 27;
                aeAttr.gainMin        = 1;

                //ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);	xqq
                //gadi_set_auto_local_wdr_mode(0);
            }
            else if(sensorModel == GADI_ISP_SENSOR_GC1034) //gc1034
            {
                LOG_INFO("day, sensor -- gc1034\n");
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = ispDefParams.contrast;
                //gapp_image_set_saturation(76);
                //gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);	xqq

                aeAttr.shutterTimeMin = 16000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;//(aNtiFlickerParam.freq == GADI_ISP_ANTIFLICKER_FREQ_50HZ) ? 25 : 30;
                aeAttr.gainMax        = 27;
                aeAttr.gainMin        = 1;

                //ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);	xqq
                //gadi_set_auto_local_wdr_mode(0);
            }
            //#ifdef SDKV20
            else if(sensorModel == GADI_ISP_SENSOR_GC2023) //gc2023
            {
                LOG_INFO("day, sensor -- gc2023\n");
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = ispDefParams.contrast;//96;
                //gapp_image_set_saturation(76);	not
                //gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);	xqq

                aeAttr.shutterTimeMin = 16000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;//(aNtiFlickerParam.freq == GADI_ISP_ANTIFLICKER_FREQ_50HZ) ? 25 : 30;
                aeAttr.gainMax        = 27;
                aeAttr.gainMin        = 1;

                //ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);	xqq
                //gadi_set_auto_local_wdr_mode(0);
            }
            //#endif
            else if(sensorModel == GADI_ISP_SENSOR_GC2033) //gc2033
            {
                LOG_INFO("day, sensor -- gc2033\n");
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = ispDefParams.contrast;//96;
                //gapp_image_set_saturation(76);
                //gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);	xqq

                aeAttr.shutterTimeMin = 16000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;//(aNtiFlickerParam.freq == GADI_ISP_ANTIFLICKER_FREQ_50HZ) ? 25 : 30;
                aeAttr.gainMax        = 27;
                aeAttr.gainMin        = 1;

//                ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);	xqq
                //gadi_set_auto_local_wdr_mode(0);
            }
            else if(sensorModel == GADI_ISP_SENSOR_GC2053) //gc2053
            {
                LOG_INFO("day, sensor -- gc2053\n");
                contrastAttr.enableAuto     = 0;
                contrastAttr.manualStrength = 64;
                contrastAttr.autoStrength   = 64;//72;
                //gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);	xqq
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = 128;//72;
                gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);

                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax        = 30;
                aeAttr.gainMin        = 1;

                ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);
                //gadi_set_auto_local_wdr_mode(0);
            }
            else if(sensorModel == GADI_ISP_SENSOR_GC4623) //gc4623
            {
                LOG_INFO("day, sensor -- gc4623\n");
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = 128;
                //gapp_image_set_saturation(76);
               // gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);		xqq

                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;//(aNtiFlickerParam.freq == GADI_ISP_ANTIFLICKER_FREQ_50HZ) ? 25 : 30;
                aeAttr.gainMax        = 30;
                aeAttr.gainMin        = 1;

//                ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);	xqq
                //gadi_set_auto_local_wdr_mode(0);
            }
            else if(sensorModel == GADI_ISP_SENSOR_AR0130)   //ar0130
            {
                LOG_INFO("day ar0130\n");
                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax          = 36;
                aeAttr.gainMin          = 1;
                //gadi_isp_set_ae_attr(ispHandle, &aeAttr);	xqq
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = ispDefParams.contrast;
                //gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);	xqq
            }
            else if(sensorModel == GADI_ISP_SENSOR_MIS1002)   //ar0130
            {
                LOG_INFO("day mis1002\n");
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = ispDefParams.contrast;
                //gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);	xqq

                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax          = 31;
                aeAttr.gainMin          = 1;
                //gadi_isp_set_ae_attr(ispHandle, &aeAttr);	xqq
            }
            else if(sensorModel == GADI_ISP_SENSOR_AR0237) //ar0237
            {
                LOG_INFO("day ar0237\n");
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = ispDefParams.contrast;
                //gadi_isp_set_contrast_attr(ispHandle, &contrastAttr); xqq

                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax          = 36;
                aeAttr.gainMin          = 1;
//                ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);	xqq
            }
            else if(sensorModel == GADI_ISP_SENSOR_SC2035) //sc2035
            {
                LOG_INFO("day sc2035\n");
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = ispDefParams.contrast;
                //gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);	xqq

                aeAttr.shutterTimeMin = 16000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax          = 35;
                aeAttr.gainMin          = 1;
               // ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);	xqq
               // gadi_set_auto_local_wdr_mode(ispHandle, 80);		xqq
            }
            else if(sensorModel == GADI_ISP_SENSOR_SC2135) //sc2135
            {
                LOG_INFO("day sc2135\n");
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                //contrastAttr.autoStrength   = ispDefParams.contrast;
                contrastAttr.autoStrength   = 32;
                //gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);	xqq

                aeAttr.shutterTimeMin = 16000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax          = 23;
                aeAttr.gainMin          = 1;
               // ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);
               // gadi_set_auto_local_wdr_mode(ispHandle, 80);	xqq
            }
            else if(sensorModel == GADI_ISP_SENSOR_OV9710) //ov9710
            {
                LOG_INFO("day ov9710\n");
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = ispDefParams.contrast;
                //gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);	xqq

                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax          = 28;
                aeAttr.gainMin          = 1;
               // ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);
               // gadi_set_auto_local_wdr_mode(ispHandle, 0);	xqq
            }
            else if(sensorModel == GADI_ISP_SENSOR_OV9732)
            {
                LOG_INFO("day ov9732\n");
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength    = 48;//96;
                //gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);	xqq
                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax          = 24;
                aeAttr.gainMin          = 1;
                //ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);	xqq
            }
            else if (sensorModel == GADI_ISP_SENSOR_SC2232) //SC2232
            {
                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax        = 30;
                aeAttr.gainMin        = 1;
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = 128;//
                gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);

                gadi_isp_set_ae_attr(ispHandle, &aeAttr);
                gadi_set_auto_local_wdr_mode(ispHandle, 96);
            }
            else if (sensorModel == GADI_ISP_SENSOR_SC4236) //SC4236
            {
                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax        = 30;
                aeAttr.gainMin        = 1;
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = 128;//
                gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);

                gadi_isp_set_ae_attr(ispHandle, &aeAttr);
            }
            else if (sensorModel == GADI_ISP_SENSOR_OV4689) //OV4689
            {
                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax        = 30;
                aeAttr.gainMin        = 1;
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = 128;//
                gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);
                gadi_isp_set_ae_attr(ispHandle, &aeAttr);
            }
            else if (sensorModel == GADI_ISP_SENSOR_JXQ03)    //jxq03
            {
                LOG_INFO("day jxq03\n");
                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax        = 30;
                aeAttr.gainMin        = 1;
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = 128;//
                gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);

                gadi_isp_set_ae_attr(ispHandle, &aeAttr);
            }
            else if (sensorModel == GADI_ISP_SENSOR_GC4653)    //gc4653
            {
                LOG_INFO("day GC4653\n");
                aeAttr.shutterTimeMin = 24000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax        = 30;
                aeAttr.gainMin        = 1;
                contrastAttr.enableAuto     = 0;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = 128;//
                gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);

                gadi_isp_set_ae_attr(ispHandle, &aeAttr);
            }
            else    //other sensors
            {
               LOG_ERR("day, sensorModel:0x%x\n", sensorModel);
               contrastAttr.enableAuto     = 1;
               contrastAttr.manualStrength = 128;
               contrastAttr.autoStrength   = 128;//
               gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);
            }

            gadi_isp_get_lowbitrate_attr(ispHandle, &lowbitAttrPtr);
            lowbitAttrPtr.enable = 1;
        #if 0
            lowbitAttrPtr.sharpness_ratio = 25;
            lowbitAttrPtr.cfa_denoise_property_down_ratio = 25;
            lowbitAttrPtr.cfa_denoise_property_up_ratio = 120;
        #endif
            //lowbitAttrPtr.sharpness_ratio = 0;
            //lowbitAttrPtr.cfa_denoise_property_down_ratio = 0;
            //lowbitAttrPtr.cfa_denoise_property_up_ratio = 0;
            lowbitAttrPtr.tolerance = 8;
            lowbitAttrPtr.delay_frames = 40;
            gadi_isp_set_lowbitrate_attr(ispHandle, &lowbitAttrPtr);

            // ir cut
            gadi_vi_set_ircut_control(viHandle, GADI_VI_IRCUT_DAY);
            gadi_sys_thread_sleep(500);
            gadi_vi_set_ircut_control(viHandle, GADI_VI_IRCUT_CLEAR);
            //irCutAttr.gpio_status_old = ISP_DAY;
        }
    }
    else
    {
        LOG_INFO("IRCUT: night\n");
        gk_isp_led_on(); // if define gpio control ircut led, this code can valid
        irCutAttr.gpio_status_old = ISP_LIGHT;
        if ((ispBulbMode == ISP_BULB_MODE_IR) || \
            (ispBulbMode == ISP_BULB_MODE_MIX))
        {
            gadi_isp_set_day_night_mode(ispHandle, 0);
            gadi_isp_set_meter_mode(ispHandle, 0);//0:偏中
            /* if the current gain is ver small,we can also support high framerate,
                we can also use slow framerate  simplely.
            */
            slowFramerate = isp_set_slow_framerate(1); // slow frameate
            LOG_INFO("slow framerate:%d\n", slowFramerate);

            if(slowFramerate < 0)
            {
                LOG_ERR("set slow framerate failed.\n");
                return ;
            }

            //sensor_id = sdk_isp_get_sensor_model();
            //LOG_INFO("night SENSOR: %d\n", sensor_id);
            gadi_isp_get_sensor_model(ispHandle, &sensorModel);
            LOG_INFO("night SENSOR: %d\n", sensorModel);

            if(sensorModel == GADI_ISP_SENSOR_IMX222)   //imx222
            {
                LOG_INFO("night, sensor -- imx222\n");
                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax        = 30;
                aeAttr.gainMin        = 1;
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = 96;//96;
                gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);
                gadi_isp_set_ae_attr(ispHandle, &aeAttr);
            }
            else if(sensorModel == GADI_ISP_SENSOR_IMX291)   //imx291
            {
                    LOG_INFO("night, sensor -- imx291\n");
                    aeAttr.shutterTimeMin = 8000;//1024;
                    aeAttr.shutterTimeMax = slowFramerate;
                    aeAttr.gainMax        = 42;
                    aeAttr.gainMin        = 1;
                    contrastAttr.enableAuto     = 1;
                    contrastAttr.manualStrength = 128;
                    contrastAttr.autoStrength   = 96;//96;
                    gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);
                    gadi_isp_set_ae_attr(ispHandle, &aeAttr);
                
            }
            else if(sensorModel == GADI_ISP_SENSOR_OV2710)   //ov2710
            {
                LOG_INFO("night, sensor -- ov2710\n");
                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax        = 30;
                aeAttr.gainMin        = 1;
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = 80;//96;
                gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);
                gadi_isp_set_ae_attr(ispHandle, &aeAttr);

            }
            else if (sensorModel == GADI_ISP_SENSOR_JXH42)  //jxh42
            {
                LOG_INFO("night, sensor -- jxh42\n");
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = 36;//96;
                gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);

                aeAttr.shutterTimeMin = 1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax        = 30;
                aeAttr.gainMin        = 1;

                ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);
            }
            else if (sensorModel == GADI_ISP_SENSOR_JXH61)  //jxh61
            {
                LOG_INFO("night, sensor -- jxh61\n");
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = 36;//96;
                gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);

                aeAttr.shutterTimeMin = 1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax        = 30;
                aeAttr.gainMin        = 1;

                ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);
                gadi_set_auto_local_wdr_mode(ispHandle, 0);
            }
            else if (sensorModel == GADI_ISP_SENSOR_JXF22)  //jxf22
            {
                LOG_INFO("night, sensor -- jxf22\n");
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = 36;//96;
                gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);

                aeAttr.shutterTimeMin = 1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax        = 30;
                aeAttr.gainMin        = 1;

                ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);
                gadi_set_auto_local_wdr_mode(ispHandle, 0);
            }
            else if(sensorModel == GADI_ISP_SENSOR_SC1035)   //sc1035
            {
                LOG_INFO("night, sensor -- sc1035\n");
                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax        = 30;
                aeAttr.gainMin        = 1;
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = 64;//96;

                gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);
                gadi_isp_set_ae_attr(ispHandle, &aeAttr);
            }
            else if(sensorModel == GADI_ISP_SENSOR_SC1135)   //sc1135
            {
                LOG_INFO("night, sensor -- sc1135\n");
                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax        = 28;
                aeAttr.gainMin        = 1;
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = 64;//96;
                gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);
                gadi_isp_set_ae_attr(ispHandle, &aeAttr);
                gadi_set_auto_local_wdr_mode(ispHandle, 0);
            }
            else if(sensorModel == GADI_ISP_SENSOR_SC1045) //sc1045
            {
                LOG_INFO("night, sensor -- sc1045\n");
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = 64;//96;
                gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);

                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax        = 26;
                aeAttr.gainMin        = 1;

                ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);
                gadi_set_auto_local_wdr_mode(ispHandle, 0);
            }
            else if(sensorModel == GADI_ISP_SENSOR_SC1145) //sc1145
            {
                LOG_INFO("night, sensor -- sc1145\n");
                contrastAttr.enableAuto     = 0;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = 64;//96;
                gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);

                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax        = 26;
                aeAttr.gainMin        = 1;

                ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);
                gadi_set_auto_local_wdr_mode(ispHandle, 0);
            }
            else if(sensorModel == GADI_ISP_SENSOR_GC1024) //gc1024
            {
                LOG_INFO("night, sensor -- gc1024\n");
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength    = 64;//96;
                gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);

                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax          = 31;
                aeAttr.gainMin          = 1;

                ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);
                gadi_set_auto_local_wdr_mode(ispHandle, 0);
            }
            else if(sensorModel == GADI_ISP_SENSOR_GC1034) //gc1034
            {
                LOG_INFO("night, sensor -- gc1034\n");
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength    = 64;//96;
                gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);

                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax          = 31;
                aeAttr.gainMin          = 1;

                ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);
                //gadi_set_auto_local_wdr_mode(0);
            }
            //#ifdef SDKV20
            else if(sensorModel == GADI_ISP_SENSOR_GC2023) //gc2023
            {
                LOG_INFO("night, sensor -- gc2023\n");
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength    = 64;//96;
                gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);

                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax          = 31;
                aeAttr.gainMin          = 1;

                ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);
                gadi_set_auto_local_wdr_mode(ispHandle, 0);
            }
            else if(sensorModel == GADI_ISP_SENSOR_GC2033) //gc2033
            {
                LOG_INFO("night, sensor -- gc2033\n");
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength    = 64;//96;
                gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);

                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax          = 31;
                aeAttr.gainMin          = 1;

                ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);
                gadi_set_auto_local_wdr_mode(ispHandle, 0);
            }
            else if(sensorModel == GADI_ISP_SENSOR_GC2053) //gc2053
            {
                LOG_INFO("night, sensor -- gc2053\n");
                contrastAttr.enableAuto     = 0;
                contrastAttr.manualStrength = 64;
                contrastAttr.autoStrength    = 64;//96;
                gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength    = 128;//96;
                gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);

                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax          = 30;
                aeAttr.gainMin          = 1;

                ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);
                gadi_set_auto_local_wdr_mode(ispHandle, 0);
            }
            else if(sensorModel == GADI_ISP_SENSOR_GC4623) //gc4623
            {
                LOG_INFO("night, sensor -- gc4623\n");
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = 128;
                gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);

                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax        = 30;
                aeAttr.gainMin        = 1;

                ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);
                //gadi_set_auto_local_wdr_mode(0);
            }
            //#endif
            else if(sensorModel == GADI_ISP_SENSOR_AR0130) //ar0130
            {
                LOG_INFO("night, sensor -- ar0130\n");
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength    = 48;//96;
                gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);

                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax          = 36;
                aeAttr.gainMin          = 1;

                ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);
            }
            else if(sensorModel == GADI_ISP_SENSOR_MIS1002) //mis1002
            {
                LOG_INFO("night, sensor -- mis1002\n");
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength    = 48;//96;
                gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);

                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax          = 31;
                aeAttr.gainMin          = 1;

                ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);
            }
            else if(sensorModel == GADI_ISP_SENSOR_SC2035) //sc2035
            {
                LOG_INFO("night, sensor -- sc2035\n");
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength    = 48;//96;
                gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);
                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax          = 35;
                aeAttr.gainMin          = 1;
                ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);
                gadi_set_auto_local_wdr_mode(ispHandle, 0);
            }
            else if(sensorModel == GADI_ISP_SENSOR_SC2135) //sc2135
            {
                LOG_INFO("night, sensor -- sc2135\n");
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength    = 48;//96;
                gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);
                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax          = 23;
                aeAttr.gainMin          = 1;
                ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);
                gadi_set_auto_local_wdr_mode(ispHandle, 0);
            }
            else if(sensorModel == GADI_ISP_SENSOR_OV9710) //ov9710
            {
                LOG_INFO("night, sensor -- ov9710\n");
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength    = 64;//96;
                gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);
                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax          = 28;
                aeAttr.gainMin          = 1;
                ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);
                gadi_set_auto_local_wdr_mode(ispHandle, 0);
            }
            else if(sensorModel == GADI_ISP_SENSOR_OV9732)
            {
                LOG_INFO("night, sensor -- ov9732\n");
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength    = 48;//96;
                gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);

                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax          = 24;
                aeAttr.gainMin          = 1;

                ret = gadi_isp_set_ae_attr(ispHandle, &aeAttr);
            }
            else if(sensorModel == GADI_ISP_SENSOR_OV9750)   //ov9750
            {
                LOG_INFO("night, sensor -- ov9750\n");
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = 128;//96;
                gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);

                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax        = 36;
                aeAttr.gainMin        = 1;
                gadi_isp_set_ae_attr(ispHandle, &aeAttr);
                gadi_set_auto_local_wdr_mode(ispHandle, 90);
            }
            else if(sensorModel == GADI_ISP_SENSOR_SC2232)   //SC2232
            {
                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax        = 30;
                aeAttr.gainMin        = 1;
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = 128;//
                gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);
                gadi_isp_set_ae_attr(ispHandle, &aeAttr);
            }
            else if (sensorModel == GADI_ISP_SENSOR_OV4689) //OV4689
            {
                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax        = 30;
                aeAttr.gainMin        = 1;
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = 128;//
                gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);
                gadi_isp_set_ae_attr(ispHandle, &aeAttr);
            }
            else if (sensorModel == GADI_ISP_SENSOR_JXQ03)    //jxq03
            {
                LOG_INFO("night jxq03\n");
                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax        = 30;
                aeAttr.gainMin        = 1;
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = 128;//
                gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);

                gadi_isp_set_ae_attr(ispHandle, &aeAttr);
            }
            else if (sensorModel == GADI_ISP_SENSOR_GC4653)    //gc4653
            {
                LOG_INFO("night gc4653\n");
                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax        = 30;
                aeAttr.gainMin        = 1;
                contrastAttr.enableAuto     = 0;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = 128;//
                gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);

                gadi_isp_set_ae_attr(ispHandle, &aeAttr);
            }
            else    //other sensors
            {
                LOG_WARNING("night, sensorModel -- %d\n", sensorModel);
            #if 0
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = 128;
                gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);
            #else
                aeAttr.shutterTimeMin = 8000;//1024;
                aeAttr.shutterTimeMax = slowFramerate;
                aeAttr.gainMax        = 30;
                aeAttr.gainMin        = 1;
                contrastAttr.enableAuto     = 1;
                contrastAttr.manualStrength = 128;
                contrastAttr.autoStrength   = 128;//
                gadi_isp_set_contrast_attr(ispHandle, &contrastAttr);
                gadi_isp_set_ae_attr(ispHandle, &aeAttr);
            #endif
            }

            gadi_isp_get_lowbitrate_attr(ispHandle, &lowbitAttrPtr);
            lowbitAttrPtr.enable = 1;
        #if 0
            lowbitAttrPtr.sharpness_ratio = 25;
            lowbitAttrPtr.cfa_denoise_property_down_ratio = 25;
            lowbitAttrPtr.cfa_denoise_property_up_ratio = 120;
        #endif
            //lowbitAttrPtr.sharpness_ratio = 0;
            //lowbitAttrPtr.cfa_denoise_property_down_ratio = 0;
            //lowbitAttrPtr.cfa_denoise_property_up_ratio = 0;
            lowbitAttrPtr.tolerance = 8;
            lowbitAttrPtr.delay_frames = 20;
            gadi_isp_set_lowbitrate_attr(ispHandle, &lowbitAttrPtr);

            // ir cut
            gadi_sys_thread_sleep(500);
            gadi_vi_set_ircut_control(viHandle, GADI_VI_IRCUT_NIGHT);
            gadi_sys_thread_sleep(500);
            gadi_vi_set_ircut_control(viHandle, GADI_VI_IRCUT_CLEAR);

            //irCutAttr.gpio_status_old = ISP_LIGHT;
        }
    }
#endif		//xqq
    LOG_INFO("IRCUT: finished\n");
}


static void isp_adapted_flip(IMAGE_FLIP_FUNC *hFlip, IMAGE_FLIP_FUNC *vFlip)
{
	GADI_U8					  sensorSupportImgFlip;
    GADI_U8					  sensorSupportImgMirror;
    GADI_ISP_SensorModelEnumT sensorMode;
	//gadi_isp_get_sensor_model(ispHandle, &sensorMode);	xqq
	//probe sensor whether support flip
    switch (sensorMode) {
		case GADI_ISP_SENSOR_SC1045:
		case GADI_ISP_SENSOR_SC1035:
		//case GADI_ISP_SENSOR_SC1145:
        case GADI_ISP_SENSOR_GC1034:
        //case GADI_ISP_SENSOR_JXF22:
        //case GADI_ISP_SENSOR_SC2135:
			sensorSupportImgFlip = 0;// SC1035 & SC1045 no support sensor flip
			sensorSupportImgMirror = 0;
			break;
        case GADI_ISP_SENSOR_GC2023:
			sensorSupportImgFlip = 0;// SC1035 & SC1045 no support sensor flip
			sensorSupportImgMirror = 0;
			break;
        case GADI_ISP_SENSOR_JXF22:
        case GADI_ISP_SENSOR_GC2033:
        case GADI_ISP_SENSOR_SC2135:
            sensorSupportImgFlip = 1;// SC1035 & SC1045 no support sensor flip
            sensorSupportImgMirror = 1;
        break;
        default:
			sensorSupportImgFlip = 1;
            sensorSupportImgMirror = 1;
            break;
    }
	//adapted flip function.
	if(sensorSupportImgFlip){
		*vFlip = vi_set_flip;
	}
    else{
        *vFlip = gk_enc_set_flip;
    }

    if(sensorSupportImgMirror){
		*hFlip = vi_set_mirror;
	}
    else {
		*hFlip = gk_enc_set_mirror;
	}
}

static int isp_set_mirror(uint8_t enable)
{
	IMAGE_FLIP_FUNC hflip = NULL;
	IMAGE_FLIP_FUNC vflip = NULL;

	//adapted flip function.
	isp_adapted_flip(&hflip, &vflip);
	return hflip(enable);
}

static int isp_set_flip(uint8_t enable)
{
	IMAGE_FLIP_FUNC hflip = NULL;
	IMAGE_FLIP_FUNC vflip = NULL;

	//adapted flip function.
	isp_adapted_flip(&hflip, &vflip);
	return vflip(enable);
}

static void isp_ircut_use_gpio()
{
    static int gpio_count  = 0;
    int gpio_value = 0;
    int gpio_status_cur = ISP_DAY;

    if(sdk_cfg.ircut_gpio.use_ircut_gpio == 0)
        return;
    if ((ispBulbMode == ISP_BULB_MODE_IR) || \
        (ispBulbMode == ISP_BULB_MODE_ALL_COLOR) || \
        (ispBulbMode == ISP_BULB_MODE_MIX))
    {
        if(sdk_gpio_get_intput_value(sdk_cfg.ircut_gpio.ircut_gpio_num, &gpio_value) == 0)
        {
    //        LOG_INFO("gpio num:%d, value:%d",sdk_cfg.ircut_gpio.ircut_gpio_num,gpio_value);
            if(gpio_value != sdk_cfg.ircut_gpio.ircut_day_value)
                gpio_status_cur = ISP_LIGHT;
            else
                gpio_status_cur = ISP_DAY;
        }
        else
        {
            LOG_ERR("Read gpio %d error",sdk_cfg.ircut_gpio.ircut_gpio_num);
            return ;
        }
        gpio_count++;
        if(irCutAttr.gpio_status_old == gpio_status_cur){
            gpio_count = 0;
        }else if (gpio_count < ISP_IRCUT_AUTO_CHECK_NUMBER ) {
            return;
        }
        else{
        //statusCount = 0;
        }
    
        if(irCutAttr.gpio_status_old != gpio_status_cur && gpio_count > ISP_IRCUT_AUTO_CHECK_NUMBER) {
            LOG_INFO("ircut hareware switch:%d -> %d \r\n",
                    irCutAttr.gpio_status_old, gpio_status_cur);
    
            if(!gpio_status_cur){
                isp_ircut_switch(ISP_DAY);
                irCutAttr.gpio_status_old = gpio_status_cur;
                gpio_count = 0;
            }else{
                    isp_ircut_switch(ISP_LIGHT);
                    irCutAttr.gpio_status_old = gpio_status_cur;
                    gpio_count = 0;
            }
        }
    }
    else if(ispBulbMode == ISP_BULB_MODE_MANUAL)
    {
        if(ispMdLight == 0)
        {
            if (ispBulbLevel)
            {
                if (0 == gk_isp_get_led_state())
                {
                    gk_isp_led_on();
                }
            }
            else
            {
                if (1 == gk_isp_get_led_state())
                {
                    gk_isp_led_off();
                }
            }
        }
    }
}


static void isp_ircut_use_adc()
{
    static GADI_U32 chang_status_count = 0;
    int gpio_status_cur = -1;
    GADI_GPIO_AdcValue adcValue;
	GADI_U32 gain;
    static int extraValue = 0;
    static time_t lastChangeTime = 0;
    int oneStep = 20;
    int diff = 0;
    struct tm *ptm;
    long ts;
    struct tm tt = {0};
    static int printCnt = 0;
    static int printFlag = 0;
    
    ts = time(NULL);
    ptm = localtime_r(&ts, &tt);

    if (extraValue != 0 && (ptm->tm_hour < 20 && ptm->tm_hour > 8) 
        && (isp_change_time - lastChangeTime > 3600))
    {
        extraValue = 0;
    }

    if(sdk_cfg.ircut_adc.use_adc == 0){
        return;
    }

	adcValue.channel = 0;
    adcValue.value = 0;
#if 0
    if(gadi_gpio_read_adc(&adcValue) != GADI_OK) {
        return;
    }
#endif		//xqq
    if ((ispBulbMode == ISP_BULB_MODE_IR) || \
        (ispBulbMode == ISP_BULB_MODE_ALL_COLOR) || \
        (ispBulbMode == ISP_BULB_MODE_MIX))
    {
//        gadi_isp_get_current_gain(ispHandle, &gain);	xqq
        printCnt++;
        if (printFlag == 0 && printCnt % 10 == 0)
        {
            if (access("/tmp/adc", F_OK) == 0)
            {
                printFlag = 1;
            }
            printCnt = 0;
        }
        //printFlag = 1;
        if (printFlag)
        {
            printf("isp_ircut_use_adc vluae:%d, gain:%d,normal:%d, day:%d, night:%d, extral:%d\n",adcValue.value,gain,sdk_cfg.ircut_adc.normal,sdk_cfg.ircut_adc.value_day,sdk_cfg.ircut_adc.value_night, extraValue);
        }
        if(sdk_cfg.ircut_adc.normal == 0) // adc 越大，越亮
        {
            if(adcValue.value >= sdk_cfg.ircut_adc.value_day + extraValue)
            {
                gpio_status_cur = ISP_DAY;
            }
            else if(adcValue.value < sdk_cfg.ircut_adc.value_night)
            {
                gpio_status_cur = ISP_LIGHT;
            }
            else
            {
                chang_status_count = 0;
                return;
            }
        }
        else // adc 越小，越亮
        {
            if(adcValue.value <= sdk_cfg.ircut_adc.value_day - extraValue)
            {
                gpio_status_cur = ISP_DAY;
        
            }
            else if(adcValue.value > sdk_cfg.ircut_adc.value_night)
            {
                gpio_status_cur = ISP_LIGHT;
            }
            else
            {
                chang_status_count = 0;
                return;
            }
        }

        if(irCutAttr.gpio_status_old != gpio_status_cur)
        {
            chang_status_count++;
        }
        else
        {
            chang_status_count = 0;
        }
        
        if(chang_status_count >= 2)
        {
            chang_status_count = 0;
            time_t now_time = time(NULL);
            
            diff = now_time - isp_change_time;
            if (printFlag)
                printf("---->diff:%d, gpio:%d, mode:%d\n", diff, gpio_status_cur, ispBulbMode);
            if (diff >= 5 || gpio_status_cur == ISP_LIGHT || ispBulbMode != ISP_BULB_MODE_ALL_COLOR)
            {
                LOG_INFO("adc vluae:%u, normal:%d, day:%d, night:%d, mdLight:%d\n",
                    adcValue.value,
                    sdk_cfg.ircut_adc.normal,
                    sdk_cfg.ircut_adc.value_day,
                    sdk_cfg.ircut_adc.value_night, ispMdLight);
                //移动侦测开灯后不切回白天
                if (!(ispMdLight == 1 && gpio_status_cur == ISP_DAY))
                {
                    isp_ircut_switch(gpio_status_cur);
                }
                lastChangeTime = now_time;
            }
            else
            {
                if (isp_change_time - lastChangeTime < 60)
                //if (access("/tmp/light", F_OK) != 0)
                {
                    sdk_isp_changed();
                }
                else
                {
                    extraValue -= oneStep;
                }
                if (extraValue < 80)
                    extraValue += oneStep;
            }
        }
    }
    else if(ispBulbMode == ISP_BULB_MODE_MANUAL)
    {
        if(ispMdLight == 0)
        {
            if (ispBulbLevel)
            {
                if (0 == gk_isp_get_led_state())
                {
                    gk_isp_led_on();
                }
            }
            else
            {
                if (1 == gk_isp_get_led_state())
                {
                    gk_isp_led_off();
                }
            }
        }
    }
}


static void isp_ircut_use_time()
{
    static int lastStauts = -1;
    struct tm *ptm;
    long ts;
    struct tm tt = {0};
    
    ts = time(NULL);
    ptm = localtime_r(&ts, &tt);
    if (ptm->tm_hour >= sdk_cfg.ircut_adc.value_night || ptm->tm_hour < sdk_cfg.ircut_adc.value_day)
    {
        if (lastStauts == -1 || lastStauts == 0)
        {
            isp_ircut_switch(ISP_LIGHT);
            lastStauts = ISP_LIGHT;
            LOG_INFO("isp_ircut_use_time: %d,%d, %d-%d\n", lastStauts, ptm->tm_hour, sdk_cfg.ircut_adc.value_day, sdk_cfg.ircut_adc.value_night);
        }
    }
    else
    {
        if (lastStauts == -1 || lastStauts == 1)
        {
            isp_ircut_switch(ISP_DAY);
            lastStauts = ISP_DAY;
            LOG_INFO("isp_ircut_use_time: %d,%d, %d-%d\n", lastStauts, ptm->tm_hour, sdk_cfg.ircut_adc.value_day, sdk_cfg.ircut_adc.value_night);
        }
    }
}


static void isp_ircut_use_adc_again()
{
    static int ircut_init = 0, ircut_mode = ISP_DAY, detect_case = ISP_DAY, old_adc = -1, adc_min_of_day = 0,adc_max_of_day = 0;
    static int old_gain = 0, chang_status_count = 0;
    static int check_night_to_day = 0;
    GADI_U32 gain = 0;
    GADI_GPIO_AdcValue adcValue;

    if(sdk_cfg.ircut_adc.use_adc == 0){
        return;
    }

    adcValue.channel = GADI_GPIO_ADC_CHANNEL_ONE;
    if(!ircut_init){
        ircut_init = 1;
        isp_ircut_switch(ISP_DAY);
        ircut_mode = ISP_DAY;
        detect_case = ISP_DAY;
        chang_status_count = 0;
    }
	adcValue.channel = 0;
    adcValue.value = 0;
#if 0
    if(gadi_gpio_read_adc(&adcValue) != GADI_OK) {
        return;
    }
#endif		//xqq
    if ((ispBulbMode == ISP_BULB_MODE_IR) || \
        (ispBulbMode == ISP_BULB_MODE_ALL_COLOR) || \
        (ispBulbMode == ISP_BULB_MODE_MIX))
    {

        //gadi_isp_get_current_gain(ispHandle, &gain);	xqq
        //LOG_INFO("isp_ircut_use_adc_again, gain:%d, adc:%d, old_adc:%d, adc_min_of_day:%d, mode:%d,ircut_normal:%d\n", gain, adcValue.value, old_adc, adc_min_of_day, detect_case,sdk_cfg.ircut_adc.normal);

        // sdk_cfg.ircut_adc.normal == 0， gain越小(越亮)，adc值越大，gain越大(越暗)，adc值越小
        // sdk_cfg.ircut_adc.normal == 1， gain越小(越亮)，adc值越小，gain越大(越暗)，adc值越大
        switch(detect_case)
        {
            case ISP_DAY:
                if(gain > 18){//白天gain>18同时ADC变化超过门限就切到晚上
                    if((old_adc > 0) &&
                        ((sdk_cfg.ircut_adc.normal == 0 && old_adc > IRCUT_CHANGE_LIMITE && adcValue.value >= (old_adc - IRCUT_CHANGE_LIMITE)) ||
                        ((adcValue.value < (old_adc + IRCUT_CHANGE_LIMITE) && sdk_cfg.ircut_adc.normal == 1))) ){
                        chang_status_count = 0;
                        return;
                    }
                    if(chang_status_count < ISP_IRCUT_AUTO_CHECK_NUMBER){//增加容错判断两次再切换
                        chang_status_count ++;
                        return ;
                    }else{
                        chang_status_count = 0;
                    }

                    isp_ircut_switch(ISP_LIGHT);
                    ircut_mode = ISP_LIGHT;
                    detect_case = ISP_LIGHT;
                    check_night_to_day = 0;
                    adcValue.value = 0;
#if 0
                    if(gadi_gpio_read_adc(&adcValue) != GADI_OK) {
                        return;
                    }
#endif		//xqq
                    old_adc = adcValue.value;//
                    return;
                }else{//(gain > 0) && (gain <= 18)
                    if(-1 == old_adc)
                        old_adc = adcValue.value;//记录刚开机的AD值用于遮挡镜头时不切换到夜晚
                    if(0 == old_gain)
                        old_gain = gain;
                    if((0 == adc_min_of_day) || (0 == adc_max_of_day)){
                        adc_max_of_day = adc_min_of_day = adcValue.value;
                    }else{
                        if((adc_min_of_day > adcValue.value && sdk_cfg.ircut_adc.normal == 0)||//取出白天的时的最小ADC值,用于夜晚切到白天的最小值判断
                                (adc_max_of_day < adcValue.value && sdk_cfg.ircut_adc.normal == 1)){
                            if(abs(old_gain - gain)< 3){//为了精确使用gain变化不大的ADC值
                                adc_max_of_day = adc_min_of_day = adcValue.value;
                            }
                            old_gain = gain;
                        }
                    }
                }
                break;

            case ISP_LIGHT:
                {

                    adcValue.value = 0;
#if 0
                    if(gadi_gpio_read_adc(&adcValue) != GADI_OK) {
                        return;
                    }
#endif		//xqq
                    if((((sdk_cfg.ircut_adc.normal == 0) && (adcValue.value >= (old_adc + IRCUT_CHANGE_LIMITE))) ||
                         ((sdk_cfg.ircut_adc.normal == 1) && (old_adc > IRCUT_CHANGE_LIMITE) && (adcValue.value <  (old_adc - IRCUT_CHANGE_LIMITE)))) &&
                         (ircut_mode == ISP_LIGHT)){//夜晚模式下亮度变化达到IRCUT_CHANGE_LIMITE就切换到白天模式
                            /*增加容错判断,判断ISP_IRCUT_AUTO_CHECK_NUMBER次再条件都满足切换*/
                            if(chang_status_count < ISP_IRCUT_AUTO_CHECK_NUMBER){
                                chang_status_count ++;
                                return ;
                            }else{
                                chang_status_count = 0;
                            }

                            isp_ircut_switch(ISP_DAY);
                            ircut_mode = ISP_DAY;

                            //更新门限值
                            adcValue.value = 0;
#if 0
                            if(gadi_gpio_read_adc(&adcValue) != GADI_OK) {
                                return;
                            }
#endif		//xqq
                            check_night_to_day = 1;
                            old_adc = adcValue.value;// + IRCUT_CHANGE_LIMITE;
                            return;
                    }else{
                        chang_status_count = 0;
                    }

                    if((check_night_to_day) && (ircut_mode == ISP_DAY)){//再次确认，在夜晚模式下切换到白天模式，是否真是白天
                        //ADC小于白天统计的最小值就切回晚上
                        if((adc_min_of_day > 0 || adc_max_of_day > 0) && gain > 18 && // 如果是真正的黑夜，则gain值会大于18，否则应该是白天
                            ((adcValue.value <= (adc_min_of_day - (IRCUT_CHANGE_LIMITE/2)) && sdk_cfg.ircut_adc.normal == 0 )||  //可用白天最小值或白天的平均值,(IRCUT_CHANGE_LIMITE/2,已经是最小值了，防止偏差过大)
                            (adcValue.value >  (adc_max_of_day + (IRCUT_CHANGE_LIMITE/2)) && sdk_cfg.ircut_adc.normal == 1 ))){  //可用白天最大值或白天的平均值,(IRCUT_CHANGE_LIMITE/2,已经是最小值了，防止偏差过大)

                            if(chang_status_count < ISP_IRCUT_AUTO_CHECK_NUMBER)//增加容错判断两次再切换
                            {
                                chang_status_count ++;
                                return ;
                            }else{
                                chang_status_count = 0;
                            }

                            isp_ircut_switch(ISP_LIGHT);
                            ircut_mode = ISP_LIGHT;
                            //detect_case = ISP_LIGHT;

                            adcValue.value = 0;
#if 0
                            if(gadi_gpio_read_adc(&adcValue) != GADI_OK) {
                                return;
                            }
#endif			//xqq
                            old_adc = adcValue.value;// + IRCUT_CHANGE_LIMITE;
                            check_night_to_day = 0;
                        }else{//夜晚模式ADC大于白天的最小值,进入白天检测状态
                            check_night_to_day = 0;
                            detect_case = ISP_DAY;
                            chang_status_count = 0;
                        }
                        return;
                    }
                }

                break;
            default:
                break;

        }
    }
    else if(ispBulbMode == ISP_BULB_MODE_MANUAL)
    {
        if(ispMdLight == 0)
        {
            if (ispBulbLevel)
            {
                if (0 == gk_isp_get_led_state())
                {
                    gk_isp_led_on();
                }
            }
            else
            {
                if (1 == gk_isp_get_led_state())
                {
                    gk_isp_led_off();
                }
            }
        }
    }
}

static int isp_Image_percent2value(uint8_t percent, int max_value)
{
    return (max_value*percent)/100;
}

int sdk_isp_get_iso()
{
	int iso;
	double shutter = 0;
    GADI_U32 gain = 0;
//	gadi_isp_get_current_shutter(ispHandle, &shutter);	xqq
//    gadi_isp_get_current_gain(ispHandle, &gain); // gain越小(越亮)，gain越大(越暗)	xqq

	iso = (int)(pow(10, (double)gain/20)*shutter);
    //LOG_INFO("iso:%d, gain:%d, shutter:%f\n", iso, gain, shutter);
    return iso;
}

void sdk_isp_set_flip(uint8_t mode)
{
   int val = mode;

   if(sdk_cfg.flip_revert) // HZD sensor board is oppsoited by factory, so we use this sepcial case
   {
        val = (mode == 0?1:0);
   }

   isp_set_flip(val);
}

void sdk_isp_set_mirror(uint8_t mode)
{
   int val = mode;

   if(sdk_cfg.mirror_revert) // HZD sensor board is oppsoited by factory, so we use this sepcial case
   {
        val = (mode == 0?1:0);
   }

   isp_set_mirror(val);
}

void sdk_isp_set_ImageDefParam(uint8_t saturation, uint8_t brightness, uint8_t hue, uint8_t contrast, uint8_t sharpen)
{
    ispDefParams.saturation = isp_Image_percent2value(saturation, 0x7F+1);
    ispDefParams.brightness = isp_Image_percent2value(brightness, 0xFF+1);
    ispDefParams.hug        = isp_Image_percent2value(hue, 0xFF+1);
    ispDefParams.contrast   = isp_Image_percent2value(contrast, 0xFF+1);
    ispDefParams.sharpen    = isp_Image_percent2value(sharpen, 0xFF+1);
    return;
}
#define NOT_VALID 0

void sdk_isp_set_hue(uint8_t hue)
{
    GADI_ERR errorCode = GADI_OK;
    int val;

    if (hue == 50) {
        val = ispDefParams.hug;
    }
    else {
        val = isp_Image_percent2value(hue, 0xFF);
    }

    //errorCode = gadi_isp_set_hue(ispHandle, val);	xqq
    if (errorCode != GADI_OK) {
        LOG_ERR("set hue failed %d\n", errorCode);
        return;
    }

    LOG_INFO("set hue success %d\n", val);
    // if we donn't set this value, hue maybe not valid
    #if NOT_VALID
//	gadi_isp_set_brightness(ispHandle, imageSytle.brightness  + imageSytle.brig_offset);
//	gadi_isp_set_saturation(ispHandle, imageSytle.staturation + imageSytle.stau_offset);	xqq
    #endif

    return;
}

void sdk_isp_set_saturation(uint8_t saturation)
{
    GADI_ERR errorCode = GADI_OK;
    GADI_S32 val;

    if (saturation == 50) {
        val = ispDefParams.saturation;
    }
    else {
        val = isp_Image_percent2value(saturation, 0x7F);
    }
//    errorCode = gadi_isp_set_saturation(ispHandle, val);	xqq
    if (errorCode != GADI_OK) {
        LOG_ERR("set saturation failed %d\n", errorCode);
        return;
    }

    LOG_INFO("set saturation success %d\n", val);
    #if NOT_VALID
    imageSytle.staturation = val;

//    errorCode = gadi_isp_set_saturation(ispHandle, val + imageSytle.stau_offset);	xqq
    if (errorCode != GADI_OK) {
        LOG_ERR("set saturation failed %d\n", errorCode);
        return;
    }

    LOG_INFO("set saturation success %d\n", val);
//	gadi_isp_set_brightness(ispHandle, imageSytle.brightness + imageSytle.brig_offset);	xqq
    #endif

    return;
}

void sdk_isp_set_brightness(uint8_t brightness)
{
    GADI_ERR errorCode = GADI_OK;
    GADI_S32 val;

    if (brightness == 50) {
        val = ispDefParams.brightness;
    }
    else {
        val = isp_Image_percent2value(brightness, 0xFF);
    }
//    errorCode = gadi_isp_set_brightness(ispHandle, val);	xqq
    if (errorCode != GADI_OK) {
        LOG_ERR("set brightness failed %d\n", errorCode);
        return;
    }

    LOG_INFO("set brightness success %d\n", val);
    #if NOT_VALID
    imageSytle.brightness = val;

//    errorCode = gadi_isp_set_brightness(ispHandle, val + imageSytle.brig_offset);	xqq
    if (errorCode != GADI_OK) {
        LOG_ERR("set brightness failed %d\n", errorCode);
        return;
    }

    LOG_INFO("set brightness success %d\n", val);

//	gadi_isp_set_saturation(ispHandle, imageSytle.staturation+ imageSytle.stau_offset);	xqq
    #endif
    return;
}

void sdk_isp_set_contrast(uint8_t  contrast)
{
    GADI_ERR errorCode = GADI_OK;
    GADI_ISP_ContrastAttrT contrastAttr;
    static int lastVal = -1;
    GADI_S32 val;

    LOG_INFO("set contrast enter\n");

    if (contrast == 50) {
        val = ispDefParams.contrast;
    }
    else {
        val = isp_Image_percent2value(contrast, 0xFF);
    }

//    gadi_isp_get_contrast_attr(ispHandle, &contrastAttr);	xqq

    if(contrastAttr.enableAuto == 1) {
        contrastAttr.autoStrength = val;
    }
    else {
        contrastAttr.manualStrength = val;
    }

    LOG_INFO("set contrast auto [%d]\n", contrastAttr.enableAuto);

//    errorCode = gadi_isp_set_contrast_attr(ispHandle,  &contrastAttr);	xqq
    if (errorCode != GADI_OK) {
        LOG_ERR("set contrast failed %d\n", errorCode);
        return;
    }

    if(lastVal != val){
        if(irCutAttr.gpio_status_old == ISP_DAY){
            //gadi_isp_set_day_night_mode(1); // use this code, the manual contrast can valid
        }
    }

    lastVal = val;
    #if NOT_VALID
//	gadi_isp_set_brightness(ispHandle, imageSytle.brightness + imageSytle.brig_offset);	xqq
    #endif
    LOG_INFO("set contrast success %d\n", val);

    return;
}

void sdk_isp_set_color_mode(uint8_t color_mode)
{
    LOG_INFO("%s: mode:%d\n", __FUNCTION__,color_mode);
    GADI_ERR errorCode = GADI_OK;
    GADI_S32 val;

    if(color_mode == 0){ //standard
//        errorCode = gadi_isp_set_brightness(ispHandle, ispDefParams.brightness);	xqq
        if (errorCode != GADI_OK) {
            LOG_ERR("set brightness failed %d\n", errorCode);
            return;
        }
//        errorCode = gadi_isp_set_saturation(ispHandle, ispDefParams.saturation);	xqq
        if (errorCode != GADI_OK) {
            LOG_ERR("set brightness failed %d\n", errorCode);
            return;
        }
    }
    else if(color_mode == 1){ // more brigthness
//        errorCode = gadi_isp_set_saturation(ispHandle, ispDefParams.saturation); // reset to normal	xqq
        if (errorCode != GADI_OK) {
            LOG_ERR("set saturation failed %d\n", errorCode);
            return;
        }

//        errorCode = gadi_isp_get_brightness(ispHandle, &val);	xqq
        if (errorCode != GADI_OK) {
            LOG_ERR("get brightness failed %d\n", errorCode);
            return;
        }

        val += 10; // add some brightness
//        errorCode = gadi_isp_set_brightness(ispHandle, val);	xqq
        if (errorCode != GADI_OK) {
            LOG_ERR("set brightness failed %d\n", errorCode);
            return;
        }

    }
    else{  //mode = 2 // more saturation
//        errorCode = gadi_isp_set_brightness(ispHandle, ispDefParams.brightness); // reset to normal	xqq
        if (errorCode != GADI_OK) {
            LOG_ERR("set brightness failed %d\n", errorCode);
            return;
        }

//        errorCode = gadi_isp_get_saturation(ispHandle, &val);	xqq
        if (errorCode != GADI_OK) {
            LOG_ERR("get saturation failed %d\n", errorCode);
            return;
        }

        val += 30; // add some saturation
//        errorCode = gadi_isp_set_saturation(ispHandle, val);	xqq
        if (errorCode != GADI_OK) {
            LOG_ERR("set saturation failed %d\n", errorCode);
            return;
        }

    }
    LOG_INFO("set color mode success %d\n", color_mode);
}

void sdk_isp_set_vi_flicker(uint8_t frequency)
{
    switch (frequency) {
        default:
        case 50:
            frequency = 50;
            break;
        case 60:
            frequency = 60;
            break;
    }
    isp_set_shutter(frequency);
    return;
}

void sdk_isp_set_sharpen(uint8_t val, uint8_t manual)
{
    GADI_ISP_SharpenAttrT sharpenAttr;
	GADI_ERR              errorCode = GADI_OK;

	if (val == 50) {
        val = ispDefParams.sharpen;
    }
    else {
        val = (val*255) /100;
    }

//    errorCode = gadi_isp_get_sharpen_attr(ispHandle, &sharpenAttr);	xqq
    if (errorCode != GADI_OK) {
        LOG_ERR("get sharpen attr error %d\n", errorCode);
        return;
    }

    sharpenAttr.level        = val;
    sharpenAttr.enableDisable = 1; // only support auto sharpen

//    errorCode = gadi_isp_set_sharpen_attr(ispHandle, &sharpenAttr);	xqq
    if (errorCode != GADI_OK) {
        LOG_ERR("set sharpen attr error %d\n", errorCode);
    }

	LOG_INFO("gk_isp_api_set_sharpen val[%d] bManual[%d]\n", val, manual);
    #if NOT_VALID
//	gadi_isp_set_saturation(ispHandle, imageSytle.staturation+ imageSytle.stau_offset);
//	gadi_isp_set_brightness(ispHandle, imageSytle.brightness + imageSytle.brig_offset);	xqq
    #endif
	return;
}

void sdk_isp_set_warp(uint8_t h_enable, uint8_t h_val, uint8_t v_enable, uint8_t v_val)
{
    GADI_ISP_WarpInfoT warpAttr;
	GADI_ERR           errorCode = GADI_OK;

//    errorCode = gadi_isp_get_dewarp_attr(ispHandle, &warpAttr);	xqq
    if (errorCode != GADI_OK) {
        LOG_ERR("get dewarp attr error %d\n", errorCode);
        return;
    }

    warpAttr.h_warp_enable   = h_enable;
    warpAttr.h_warp_strength = (h_val*48) /100;
    warpAttr.v_warp_enable   = v_enable;
    warpAttr.v_warp_strength = (v_val*48) /100;

//    errorCode = gadi_isp_set_dewarp_attr(ispHandle, &warpAttr);	xqq
    if (errorCode != GADI_OK) {
        LOG_ERR("set dewarp attr error %d\n", errorCode);
    }

	LOG_INFO("sdk_isp_set_warp, h_enable[%d] h_strength[%d], v_enable[%d] v_strength[%d]\n", h_enable, h_val, v_enable, v_val);
	return;
}

void sdk_isp_set_fps(int fps)
{
    GADI_ISP_AeAttrT aeAttr;
    GADI_ERR         errorCode = GADI_OK;
//    errorCode = gadi_isp_get_ae_attr(ispHandle, &aeAttr);	xqq
    if (errorCode != GADI_OK)
    {
        LOG_ERR("get ae attr error %d\n", errorCode);
        return;
    }
    LOG_INFO("sdk_isp_set_fps:%d -> %d\n", aeAttr.shutterTimeMax, fps);
    aeAttr.shutterTimeMax = fps;
//    errorCode = gadi_isp_set_ae_attr(ispHandle, &aeAttr);	xqq
    if (errorCode != GADI_OK)
    {
        LOG_ERR("set ae attr error %d\n", errorCode);
        return;
    }
}

void sdk_isp_set_scene_mode(uint8_t mode)
{
#if 0
	HI_GK_SDK_ISP_set_scene_mode(mode);
#endif
    GADI_ISP_AeAttrT attrPtr;
    GADI_ERR         errorCode = GADI_OK;
//    errorCode = gadi_isp_get_ae_attr(ispHandle, &attrPtr);	xqq
    if (errorCode != GADI_OK) {
        LOG_ERR("get ae attr error %d\n", errorCode);
        return;
    }

    if(mode == ISP_SCENE_MODE_AUTO){  // auto
        attrPtr.tagetRatio = 0x80;
    }
    else if(mode == ISP_SCENE_MODE_INDOOR){  // indoor
        attrPtr.tagetRatio = 0xf0;
    }else { // mode == 2 //out door
        attrPtr.tagetRatio = 0x40;
    }
//    errorCode = gadi_isp_set_ae_attr(ispHandle, &attrPtr);	xqq
    if (errorCode != GADI_OK) {
        LOG_ERR("set ae attr error %d\n", errorCode);
        return;
    }

    LOG_INFO("set_scene_mode,mode:%d\n",mode);
    #if NOT_VALID
//	errorCode = gadi_isp_set_brightness(ispHandle, imageSytle.brightness + imageSytle.brig_offset);	xqq
    if (errorCode != GADI_OK) {
        LOG_ERR("gadi_isp_set_brightness %d\n", errorCode);
        return;
    }
//	errorCode = gadi_isp_set_saturation(ispHandle, imageSytle.staturation + imageSytle.stau_offset);	xqq
    if (errorCode != GADI_OK) {
        LOG_ERR("gadi_isp_set_saturation %d\n", errorCode);
        return;
    }
    #endif
}

void sdk_isp_set_wb_mode(uint8_t wb_mode)
{
    GADI_ISP_AwbAttrT awbAttr;
    GADI_ERR          errorCode = GADI_OK;

//    errorCode = gadi_isp_get_awb_attr(ispHandle, &awbAttr);	xqq
    if (errorCode != GADI_OK) {
        LOG_ERR("get awb att fail %d\n", errorCode);
        return;
    }

    switch(wb_mode){
        default:
        case ISP_SCENE_MODE_AUTO:
            awbAttr.colorTemp = GADI_ISP_AWB_COLOR_TEMP_AUTO;
            break;
        case ISP_SCENE_MODE_INDOOR:
            awbAttr.colorTemp = GADI_ISP_AWB_COLOR_TEMP_4500;
            break;
        case ISP_SCENE_MODE_OUTDOOR:
            awbAttr.colorTemp = GADI_ISP_AWB_COLOR_TEMP_6500;
            break;
    }

//    errorCode = gadi_isp_set_awb_attr(ispHandle, &awbAttr);	xqq
    if (errorCode != GADI_OK) {
        LOG_ERR("set awb att fail %d\n", errorCode);
        return;
    }

    LOG_INFO("gk_isp_api_set_wb_mode: mode[%d]\n", wb_mode);
    #if NOT_VALID
//	gadi_isp_set_brightness(ispHandle, imageSytle.brightness + imageSytle.brig_offset);
//	gadi_isp_set_saturation(ispHandle, imageSytle.staturation + imageSytle.stau_offset);	xqq
    #endif
    return;
}

void sdk_isp_set_ircut_control_mode(uint8_t mode)
{

    irCutAttr.ircut_control_mode = mode;
    LOG_INFO("gk_isp_api_set_ircut_control_mode: mode[%d]\n", mode);
}

void sdk_isp_ircut_auto_switch(void)
{
    if (irCutAttr.ircut_auto_switch_enable) {
        if(sdk_cfg.ircut_adc.use_adc == 1){
            isp_ircut_use_adc_again();
        }
        else if(sdk_cfg.ircut_adc.use_adc == 2){
            isp_ircut_use_adc();
        }
        else if (sdk_cfg.ircut_adc.use_adc == 3)
        {
            isp_ircut_use_time();
        }

        else if(sdk_cfg.ircut_gpio.use_ircut_gpio ){
            isp_ircut_use_gpio();
        }
        else
        {
            LOG_ERR("IRcut control error,not gpio or adc ");
        }
    }
}

void sdk_isp_set_ircut_mode(uint8_t mode)
{
    LOG_INFO("gk_isp_api_set_ircut_mode: mode[%d]\n", mode);
    switch(mode){
        default:
        case ISP_IRCUT_MODE_AUTO:
            sdk_isp_set_bulb_mode(ISP_BULB_MODE_IR);
            irCutAttr.ircut_auto_switch_enable = GADI_TRUE;
            isp_ircut_switch(ISP_DAY);
            break;
        case ISP_IRCUT_MODE_DAYLIGHT:
            sdk_isp_set_bulb_mode(ISP_BULB_MODE_IR);
            irCutAttr.ircut_auto_switch_enable = GADI_FALSE;
            isp_ircut_switch(ISP_DAY);
            break;
        case ISP_IRCUT_MODE_NIGHT:
            sdk_isp_set_bulb_mode(ISP_BULB_MODE_IR);
            irCutAttr.ircut_auto_switch_enable = GADI_FALSE;
            isp_ircut_switch(ISP_LIGHT);
            break;
        case ISP_IRCUT_MODE_AUTOCOLOR:
            irCutAttr.ircut_auto_switch_enable = GADI_TRUE;
            sdk_isp_set_bulb_mode(ISP_BULB_MODE_ALL_COLOR);
            break;
    }

    return;
}

void sdk_isp_set_lowlight_mode(uint8_t mode)
{
    signed int  slowFramerate = 0;
    slowLight.curMode = mode;
    switch(mode)
    {
        case ISP_LOWLIGHT_MODE_CLOSE:
            if(irCutAttr.gpio_status_old == ISP_DAY && slowLight.isLowFrame == 1){
                slowFramerate = isp_set_slow_framerate(0);
            }
            break;
        case ISP_LOWLIGHT_MODE_ALLDAY:
            slowFramerate = isp_set_slow_framerate(1);
            break;
        case ISP_LOWLIGHT_MODE_NIGHT:
            break;
        case ISP_LOWLIGHT_MODE_AUTO:
            break;
    }

    LOG_INFO("val: %d, current framerate: %d\n",mode, slowFramerate);
}

//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************

int sdk_isp_init()
{
    GADI_ERR                errorCode = GADI_OK;
    GADI_ISP_OpenParamsT    openParams;
    LOG_INFO("gk_isp_init In\n");

     do {
        if (!ispIsInitialized) {
//            errorCode = gadi_isp_init();	xqq
            if (errorCode != GADI_OK) {
                LOG_ERR("gadi_isp_init: error %d\n", errorCode);
                break;
            }

            memset(&openParams, 0, sizeof(GADI_ISP_OpenParamsT));
            //openParams.bEnhance3D = 1;
            openParams.ispChnID = GADI_ISP_CHN0;// enable new 3d alg
//            ispHandle = gadi_isp_open(&openParams, &errorCode);	xqq
            if (ispHandle == NULL || errorCode != GADI_OK) {
                LOG_ERR("gadi_isp_open: error %d\n", errorCode);
                break;
            }
            ispIsInitialized = 1;
        }
    } while(0);

    pthread_mutex_init(&bulbMutex, NULL);

    LOG_INFO("gk_isp_init success\n");
	return 0;
}

int sdk_isp_destroy()
{
    GADI_ERR errorCode = GADI_OK;

    if (ispIsStarted) {
//        errorCode = gadi_isp_stop(ispHandle);	xqq
        if (errorCode != GADI_OK) {
            LOG_ERR("gadi_isp_stop: error %d\n", errorCode);
            return errorCode;
        }
        ispIsStarted = 0;
    }

    if (ispIsInitialized) {
//        errorCode = gadi_isp_close(ispHandle);	xqq
        if (errorCode != GADI_OK) {
            LOG_ERR("gadi_isp_close: error %d\n", errorCode);
            return errorCode;
        }
        ispHandle = NULL;
    }

    //gadi_isp_exit();	xqq
	return 0;
}

int gk_isp_init_start(void)
{
    GADI_ERR                errorCode = GADI_OK;
    GADI_ISP_ContrastAttrT  contrastAttr;
    GADI_ISP_SensorModelEnumT modelPtr = GADI_ISP_SENSOR_UNKNOWN;
    LOG_INFO("gk_isp_init_start enter\n");

    do {
        if (!ispIsStarted) {
//            errorCode = gadi_isp_load_param(ispHandle, NULL);	xqq
            if (errorCode != GADI_OK){
                LOG_ERR("gadi_isp_load_param: error %d\n", errorCode);
                break;
            }

//            errorCode = gadi_isp_start(ispHandle);	xqq
            if (errorCode != GADI_OK){
                LOG_ERR("gadi_isp_start: error %d\n", errorCode);
                break;
            }
            ispIsStarted = 1;
            /*set default isp parameters.*/
#if 0	//xqq
            if(gadi_isp_get_sensor_model(ispHandle,&modelPtr) == GADI_OK )
            {
                if(modelPtr == GADI_ISP_SENSOR_GC4623) //gc4623
        		{
        		    LOG_INFO("sensor gc4623, isp_register_auto_slowshutter\n");
        			isp_register_auto_slowshutter(24, 10);
        		}
                else if(modelPtr == GADI_ISP_SENSOR_IMX291) //gc4623
        		{
        		    LOG_INFO("sensor imx291, gk_set_auto_luma_control_enable\n");
        			gk_set_auto_luma_control_enable(ispHandle,1);
        		}
				else if(strstr(sdk_cfg.name, "CUS_HC_GK7202_GC2053_V10") != NULL)
				{
					gadi_isp_set_auto_luma_control(ispHandle, 128);
        			isp_register_auto_slowshutter(24, 10);
				}
            }
#endif		//xqq
            sdk_isp_set_warp(0, 0, 0, 0); // close by default
           // gadi_isp_tuning_start();	xqq
        }
    } while(0);

    LOG_INFO("gk_isp_init_start leave\n");
    return errorCode;
}


int gk_isp_led_on(void)
{
   switch(ispBulbMode)
    {
        case ISP_BULB_MODE_IR:
        case ISP_BULB_MODE_MIX:
            if( gpio_is_valid(sdk_cfg.gpio_ircut_led) )
            {
                return sdk_isp_ircut_led_set(1);
            }
            else
            {
                return -1;
            }
            break;

        case ISP_BULB_MODE_ALL_COLOR:
        case ISP_BULB_MODE_MANUAL:
            if( gpio_is_valid(sdk_cfg.gpio_light_led) )
            {
                return sdk_isp_light_led_set(1);
            }
            else
            {
                return -1;
            }
            break;

        default:
            break;
    }

    return 0;
}

int gk_isp_led_off(void)
{
    switch(ispBulbMode)
    {
        case ISP_BULB_MODE_IR:
        case ISP_BULB_MODE_MIX:
            if(gpio_is_valid(sdk_cfg.gpio_ircut_led))
            {
                return sdk_isp_ircut_led_set(0);
            }
            else
            {
                return -1;
            }
            break;

        case ISP_BULB_MODE_ALL_COLOR:
        case ISP_BULB_MODE_MANUAL:
            if(gpio_is_valid(sdk_cfg.gpio_light_led))
            {
                return sdk_isp_light_led_set(0);
            }
            else
            {
                return -1;
            }
            break;

        default:
            break;
    }

    return 0;
}

int gk_isp_get_led_state(void)
{
    int value = 0;
    int gpio_num = 0;
    int led_on_value = 0;

    if ((ISP_BULB_MODE_IR == ispBulbMode) || (ISP_BULB_MODE_MIX == ispBulbMode))
    {
        gpio_num = sdk_cfg.gpio_ircut_led;
        led_on_value = sdk_cfg.gpio_ircut_led_on_value;
    }
    else if ((ISP_BULB_MODE_ALL_COLOR == ispBulbMode) || (ISP_BULB_MODE_MANUAL == ispBulbMode))
    {
        gpio_num = sdk_cfg.gpio_light_led;
        led_on_value = sdk_cfg.gpio_light_led_on_value;
    }

    if(gpio_is_valid(gpio_num))
    {
        sdk_gpio_get_output_value(gpio_num, &value);
        if (value == led_on_value)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return -1;
    }
}
int sdk_isp_changed(void)
{
    isp_change_time = time(NULL);
    return 0;
}

int sdk_isp_change_timeout(void)
{
#define ISP_SWITCH_MD_TIMEOUT 10 //2S
    int diff = 0;
    time_t now_time = time(NULL);

    diff = now_time - isp_change_time;
    if(diff < 0)
        isp_change_time = now_time;
    if(diff > ISP_SWITCH_MD_TIMEOUT){
        return 1;
    }

    return 0;
}

void sdk_ircu_detect(void)
{
#if 0
	gadi_vi_set_ircut_control(viHandle, GADI_VI_IRCUT_DAY);
	gadi_sys_thread_sleep(100);
	gadi_vi_set_ircut_control(viHandle, GADI_VI_IRCUT_CLEAR);
	sleep(1);
	gadi_vi_set_ircut_control(viHandle, GADI_VI_IRCUT_NIGHT);
	gadi_sys_thread_sleep(100);
	gadi_vi_set_ircut_control(viHandle, GADI_VI_IRCUT_CLEAR);
	sleep(1);
	gadi_vi_set_ircut_control(viHandle, GADI_VI_IRCUT_DAY);
	gadi_sys_thread_sleep(100);
	gadi_vi_set_ircut_control(viHandle, GADI_VI_IRCUT_CLEAR);
#endif		//xqq
	return;
}


void sdk_isp_set_bulb_mode(ISP_BULB_ModeEnumT mode)
{
    pthread_mutex_lock(&bulbMutex);

    #if 0
    if (ispBulbMode == mode)
    {
        return;
    }
    #endif

    printf("sdk_isp_set_bulb_mode:%d\n", mode);
    if (mode != ISP_BULB_MODE_MANUAL)
    {
        if (1 == sdk_isp_ircut_led_get_status())
        {
            sdk_isp_ircut_led_set(0);
        }

        if (1 == sdk_isp_light_led_get_status())
        {
            sdk_isp_light_led_set(0);
        }
    }

    ispBulbMode = mode;
    
    if ((0 == sdk_cfg.ircut_adc.use_adc) && (0 == sdk_cfg.ircut_gpio.use_ircut_gpio))
        isp_restore_default_mode();
	else
        isp_restore_default_mode_use_adc_gpio();
    

    
    pthread_mutex_unlock(&bulbMutex);
}

ISP_BULB_ModeEnumT sdk_isp_get_bulb_mode(void)
{
    return ispBulbMode;
}

void sdk_isp_set_bulb_level(uint8_t level)
{
    pthread_mutex_lock(&bulbMutex);
    ispBulbLevel = level;
    pthread_mutex_unlock(&bulbMutex);
}

uint8_t sdk_isp_get_bulb_level(void)
{
    return ispBulbLevel;
}

int sdk_isp_ircut_led_set(int isOn)
{
    int ret = 0;
    #ifdef MODULE_SUPPORT_PWM_IRCUT_LED
    if (sdk_pwm_get_channel_status(IRCUT_PWM_CHANNEL) == 0)
    {
        sdk_pwm_start(IRCUT_PWM_CHANNEL);
    }

    if (isOn)
    {
        printf("sdk_isp_ircut_led_set:%d\n", pwmOnDutyVaule[IRCUT_PWM_CHANNEL]);
        sdk_set_pwm_duty(IRCUT_PWM_CHANNEL, pwmOnDutyVaule[IRCUT_PWM_CHANNEL] * 10);
    }
    else
    {
        sdk_set_pwm_duty(IRCUT_PWM_CHANNEL, 1);
    }
    
    #else
    if (isOn)
    {
        ret = sdk_gpio_set_output_value(sdk_cfg.gpio_ircut_led, sdk_cfg.gpio_ircut_led_on_value==1?1:0);
    }
    else
    {
        ret = sdk_gpio_set_output_value(sdk_cfg.gpio_ircut_led, sdk_cfg.gpio_ircut_led_on_value==1?0:1);
    }
    #endif
    return ret;
}

int sdk_isp_ircut_led_get_status(void)
{
    int ret = 0;
    int value = 0;
    #ifdef MODULE_SUPPORT_PWM_IRCUT_LED
    int diff = 0;
    if (sdk_pwm_get_channel_status(IRCUT_PWM_CHANNEL) == 0)
    {
        sdk_pwm_start(IRCUT_PWM_CHANNEL);
    }

    value = sdk_get_pwm_duty(IRCUT_PWM_CHANNEL);
    //有些PWM设下去的，获取上来的不完全一致
    diff = abs(pwmOnDutyVaule[IRCUT_PWM_CHANNEL] * 10 - value);
    //printf("sdk_isp_ircut_led_get_status:%d\n", value);
    if (diff < 20)
    {
        ret = 1;
    }
    
    #else
    ret = sdk_gpio_get_output_value(sdk_cfg.gpio_ircut_led, &value);
    if (ret == 0 && value == sdk_cfg.gpio_ircut_led_on_value)
    {
        ret = 1;
    }
    #endif
    return ret;
}



int sdk_isp_light_led_set(int isOn)
{
    int ret = 0;
    #ifdef MODULE_SUPPORT_PWM_IRCUT_LED
    if (sdk_pwm_get_channel_status(LIGHT_PWM_CHANNEL) == 0)
    {
        sdk_pwm_start(LIGHT_PWM_CHANNEL);
    }

    //printf("-->sdk_isp_light_led_set:%d\n", isOn);
    if (isOn)
    {
        //printf("sdk_isp_light_led_set:%d\n", pwmOnDutyVaule[LIGHT_PWM_CHANNEL]);
        sdk_set_pwm_duty(LIGHT_PWM_CHANNEL, pwmOnDutyVaule[LIGHT_PWM_CHANNEL] * 10);
    }
    else
    {
        sdk_set_pwm_duty(LIGHT_PWM_CHANNEL, 1);
    }
    #else
    if (isOn)
    {
        ret = sdk_gpio_set_output_value(sdk_cfg.gpio_light_led, sdk_cfg.gpio_light_led_on_value==1?1:0);
    }
    else
    {
        ret = sdk_gpio_set_output_value(sdk_cfg.gpio_light_led, sdk_cfg.gpio_light_led_on_value==1?0:1);
    }
    #endif
    return ret;
}

int sdk_isp_light_led_get_status(void)
{
    int ret = 0;
    int value = 0;
    int diff = 0;
#ifdef MODULE_SUPPORT_PWM_IRCUT_LED
    if (sdk_pwm_get_channel_status(LIGHT_PWM_CHANNEL) == 0)
    {
        sdk_pwm_start(LIGHT_PWM_CHANNEL);
    }

    value = sdk_get_pwm_duty(LIGHT_PWM_CHANNEL);
    diff = abs(pwmOnDutyVaule[LIGHT_PWM_CHANNEL] * 10 - value);
    //printf("sdk_isp_light_led_get_status %d,%d, diff:%d\n", value, pwmOnDutyVaule[LIGHT_PWM_CHANNEL] * 10, diff);
    if (diff < 20)
    {
        ret = 1;
    }
    
#else
    ret = sdk_gpio_get_output_value(sdk_cfg.gpio_light_led, &value);
    if (ret == 0 && value == sdk_cfg.gpio_light_led_on_value)
    {
        ret = 1;
    }
#endif
    //printf("-->sdk_isp_light_led_get_status:%d\n", ret);
    return ret;

}

int sdk_isp_led_test_start(void)
{
    isLedTest = 1;
    return isLedTest;
}


int sdk_isp_pwm_set_on_value(int channel, int value)
{
    #ifdef MODULE_SUPPORT_PWM_IRCUT_LED
        pwmOnDutyVaule[channel] = value;
    #endif
	return value;
}

int sdk_isp_get_cur_mode(void)
{
    return irCutAttr.gpio_status_old;
}

