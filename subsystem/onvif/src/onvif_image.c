/*!
*****************************************************************************
** \file        subsystem/onvif-no-gsoap/src/onvif_image.c
**
** \version     $Id$
**
** \brief       onvif image 
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2013 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#include "onvif_image.h"
#include "onvif_priv.h"
#include "onvif_adapter.h"


/***************************************************************************
函数描述: 获取图像处理服务能力集
入口参数：pstIMG_GetServiceCapabilitiesRes :服务能力返回信息
返回值     ：ONVIF_OK: 成功
***************************************************************************/
ONVIF_RET GK_NVT_Imaging_GetServiceCapabilities(GONVIF_IMAGING_GetServiceCapabilities_Res_S *pstIMG_GetServiceCapabilitiesRes)
{
    pstIMG_GetServiceCapabilitiesRes->stCapabilities.enImageStabilization = Boolean_FALSE;    
    return ONVIF_OK;
}

/***************************************************************************
函数描述: 获取图像设置信息
入口参数：pstImagingSettings : 获取图像设置返回信息
返回值     ：ONVIF_OK: 成功
         		      非零: 失败
***************************************************************************/
ONVIF_RET GK_NVT_Image_GetImaging(GONVIF_IMAGE_Settings_S *pstImagingSettings)
{      
    ONVIF_ImagingInfor_S stImageRevInfo;
    memset(&stImageRevInfo, 0, sizeof(ONVIF_ImagingInfor_S));
    if(g_stImagingAdapter.GetImagingInfor(&stImageRevInfo) != 0)
    {
    	ONVIF_ERR("Image server run error.");
        return ONVIF_ERR_IMAGE_SERVER_RUN_ERROR;
    }
    if(pstImagingSettings == NULL)
    {
    	ONVIF_ERR("pstImagingSettings is NULL.");
        return ONVIF_ERR_IMAGE_INVALID_VIDEO_SOURCE_TOKEN;
    }
#if 0
    ONVIF_DBG("GK_NVT_Image_GetImagingSettings():\n\
stImageRevInfo.\nblcmode = %d\n\
blcLevel = %d\nbrightness = %f\ncolorsaturation = %f\ncontrast = %f\n\
exposureMode = %d\naeShutterTimeMin = %f\naeShutterTimeMax = %f\naeGainMin = %f\naeGainMax = %f\nmeShutterTime = %f\nmeGain = %f\n\
focusMode = %d\nafNearLimit = %f\nafFarLimit = %f\nmfDefaultSpeed = %f\n\
irCutFilter = %d\nsharpness = %f\n\
wdrMode = %d\nwdrlevel = %f\n\
wbMode = %d\nwbrgain = %f\nwbbgain = %f\n",
        (GK_U32)stImageRevInfo.blcMode,
        (GK_S32)stImageRevInfo.blcLevel,
        (float)stImageRevInfo.brightness,
        (float)stImageRevInfo.colorSaturation,
        (float)stImageRevInfo.contrast,
        (int)stImageRevInfo.exposureMode,
        (float)stImageRevInfo.aeShutterTimeMin,
        (float)stImageRevInfo.aeShutterTimeMax,
        (float)stImageRevInfo.aeGainMin,
        (float)stImageRevInfo.aeGainMax,
        (float)stImageRevInfo.meShutterTime,
        (float)stImageRevInfo.meGain,
        (int)stImageRevInfo.focusMode,
        (float)stImageRevInfo.afNearLimit,
        (float)stImageRevInfo.afFarLimit,
        (float)stImageRevInfo.mfDefaultSpeed,
        (int)stImageRevInfo.irCutFilter,
        (float)stImageRevInfo.sharpness,
        (int)stImageRevInfo.wdrMode,
        (float)stImageRevInfo.wdrLevel,
        (int)stImageRevInfo.wbMode,
        (float)stImageRevInfo.mwbRGain,
        (float)stImageRevInfo.mwbBGain);
#endif
#if 0
/*check parameter of stImageInfo structure*/
    if(stImageRevInfo.blcMode != (GK_U32)g_GkIpc_OnvifInf.gokeImagingOptions->stBacklightCompensation.penMode[0] \
        && stImageRevInfo.blcMode != (GK_U32)g_GkIpc_OnvifInf.gokeImagingOptions->stBacklightCompensation.penMode[1])
    {
        ONVIF_ERR("value of stImageRevInfo.blcMode is invaild!\n");
        stImageRevInfo.blcMode = (GK_U32)g_GkIpc_OnvifInf.gokeImagingOptions->stBacklightCompensation.penMode[0];
    }
    if(stImageRevInfo.blcLevel < (GK_S32)g_GkIpc_OnvifInf.gokeImagingOptions->stBacklightCompensation.stLevel.min \
        || stImageRevInfo.blcLevel > (GK_S32)g_GkIpc_OnvifInf.gokeImagingOptions->stBacklightCompensation.stLevel.max)
    {
        ONVIF_ERR("value of stImageRevInfo.blcLevel is invaild!\n");
        stImageRevInfo.blcLevel = ((GK_S32)g_GkIpc_OnvifInf.gokeImagingOptions->stBacklightCompensation.stLevel.min \
            + (GK_S32)g_GkIpc_OnvifInf.gokeImagingOptions->stBacklightCompensation.stLevel.max)/2;
    }
#endif
    if(stImageRevInfo.brightness < (GK_S32)g_GkIpc_OnvifInf.gokeImagingOptions->stBrightness.min \
        || stImageRevInfo.brightness > (GK_S32)g_GkIpc_OnvifInf.gokeImagingOptions->stBrightness.max)
    {
        ONVIF_ERR("value of stImageRevInfo.brightness is invaild!\n");
        stImageRevInfo.brightness = ((GK_S32)g_GkIpc_OnvifInf.gokeImagingOptions->stBrightness.min \
            + (GK_S32)g_GkIpc_OnvifInf.gokeImagingOptions->stBrightness.max)/2;
    }
    if(stImageRevInfo.colorSaturation < (GK_S32)g_GkIpc_OnvifInf.gokeImagingOptions->stColorSaturation.min \
        || stImageRevInfo.colorSaturation > (GK_S32)g_GkIpc_OnvifInf.gokeImagingOptions->stColorSaturation.max)
    {
        ONVIF_ERR("value of stImageRevInfo.colorSaturation is invaild!\n");
        stImageRevInfo.colorSaturation = ((GK_S32)g_GkIpc_OnvifInf.gokeImagingOptions->stColorSaturation.min \
            + (GK_S32)g_GkIpc_OnvifInf.gokeImagingOptions->stColorSaturation.max)/2;
    }
    if(stImageRevInfo.contrast < (GK_S32)g_GkIpc_OnvifInf.gokeImagingOptions->stContrast.min \
        || stImageRevInfo.contrast > (GK_S32)g_GkIpc_OnvifInf.gokeImagingOptions->stContrast.max)
    {
        ONVIF_ERR("value of stImageRevInfo.contrast is invaild!\n");
        stImageRevInfo.contrast = ((GK_S32)g_GkIpc_OnvifInf.gokeImagingOptions->stContrast.min \
            + (GK_S32)g_GkIpc_OnvifInf.gokeImagingOptions->stContrast.max)/2;
    }
#if 0
    if(stImageRevInfo.exposureMode != (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.penMode[0] \
        && stImageRevInfo.exposureMode != (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.penMode[1])
    {
        ONVIF_ERR("value of stImageRevInfo.exposureMode is invaild!\n");
        stImageRevInfo.exposureMode = (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.penMode[0];
    }
    if(stImageRevInfo.aeShutterTimeMin < (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stMinExposureTime.min \
        || stImageRevInfo.aeShutterTimeMin > (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stMinExposureTime.max)
    {
        ONVIF_ERR("value of stImageRevInfo.aeShutterTimeMin is invaild!\n");
        stImageRevInfo.aeShutterTimeMin = ((unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stMinExposureTime.min \
            + (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stMinExposureTime.max)/2;
    }
    if(stImageRevInfo.aeShutterTimeMax < (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stMaxExposureTime.min \
        || stImageRevInfo.aeShutterTimeMax > (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stMaxExposureTime.max)
    {
        ONVIF_ERR("value of stImageRevInfo.aeShutterTimeMax is invaild!\n");
        stImageRevInfo.aeShutterTimeMax = ((unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stMaxExposureTime.min \
            + (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stMaxExposureTime.max)/2;
    }
    if(stImageRevInfo.aeGainMin < (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stMinGain.min \
        || stImageRevInfo.aeGainMin > (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stMinGain.max)
    {
        ONVIF_ERR("value of stImageRevInfo.aeGainMin is invaild!\n");
        stImageRevInfo.aeGainMin = ((unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stMinGain.min \
            + (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stMinGain.max)/2;
    }
    if(stImageRevInfo.aeGainMax < (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stMaxGain.min \
        || stImageRevInfo.aeGainMax > (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stMaxGain.max)
    {
        ONVIF_ERR("value of stImageRevInfo.aeGainMax is invaild!\n");
        stImageRevInfo.aeGainMax = ((unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stMaxGain.min \
            + (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stMaxGain.max)/2;
    }
    if(stImageRevInfo.meShutterTime < (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stExposureTime.min \
        || stImageRevInfo.meShutterTime > (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stExposureTime.max)
    {
        ONVIF_ERR("value of stImageRevInfo.meShutterTime is invaild!\n");
        stImageRevInfo.meShutterTime = ((unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stExposureTime.min \
            + (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stExposureTime.max)/2;
    }
    if(stImageRevInfo.meGain < (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stGain.min \
        || stImageRevInfo.meGain > (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stGain.max)
    {
        ONVIF_ERR("value of stImageRevInfo.meGain is invaild!\n");
        stImageRevInfo.meGain = ((unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stGain.min \
            + (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stGain.max)/2;
    }
    if(stImageRevInfo.focusMode != (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.penAutoFocusModes[0] \
        && stImageRevInfo.focusMode != (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.penAutoFocusModes[1])
    {
        ONVIF_ERR("value of stImageRevInfo.focusMode is invaild!\n");
        stImageRevInfo.focusMode = (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.penAutoFocusModes[0];
    }
    if(stImageRevInfo.afNearLimit < (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.stNearLimit.min \
        || stImageRevInfo.afNearLimit > (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.stNearLimit.max)
    {
        ONVIF_ERR("value of stImageRevInfo.afNearLimit is invaild!\n");
        stImageRevInfo.afNearLimit = ((unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.stNearLimit.min \
            + (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.stNearLimit.max)/2;
    }
    if(stImageRevInfo.afFarLimit < (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.stFarLimit.min \
        || stImageRevInfo.afFarLimit > (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.stFarLimit.max)
    {
        ONVIF_ERR("value of stImageRevInfo.afFarLimit is invaild!\n");
        stImageRevInfo.afFarLimit = ((unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.stFarLimit.min \
            + (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.stFarLimit.max)/2;
    }
    if(stImageRevInfo.mfDefaultSpeed < (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.stDefaultSpeed.min \
        || stImageRevInfo.mfDefaultSpeed > (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.stDefaultSpeed.max)
    {
        ONVIF_ERR("value of stImageRevInfo.mfDefaultSpeed is invaild!\n");
        stImageRevInfo.mfDefaultSpeed = ((unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.stDefaultSpeed.min \
            + (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.stDefaultSpeed.max)/2;
    }
#endif
    if(stImageRevInfo.irCutFilter != (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->penIrCutFilterModes[0] \
        && stImageRevInfo.irCutFilter != (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->penIrCutFilterModes[1] \
        && stImageRevInfo.irCutFilter != (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->penIrCutFilterModes[2])
    {
        ONVIF_ERR("value of stImageRevInfo.irCutFilter is invaild!\n");
        stImageRevInfo.irCutFilter = (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->penIrCutFilterModes[1];
    }
    if(stImageRevInfo.sharpness < (GK_S32)g_GkIpc_OnvifInf.gokeImagingOptions->stSharpness.min \
        || stImageRevInfo.sharpness > (GK_S32)g_GkIpc_OnvifInf.gokeImagingOptions->stSharpness.max)
    {
        ONVIF_ERR("value of stImageRevInfo.sharoness is invaild!\n");
        stImageRevInfo.sharpness = ((GK_S32)g_GkIpc_OnvifInf.gokeImagingOptions->stSharpness.min \
            + (GK_S32)g_GkIpc_OnvifInf.gokeImagingOptions->stSharpness.max)/2;
    }
#if 0
    if(stImageRevInfo.wdrMode != (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stWideDynamicRange.penMode[0] \
        && stImageRevInfo.wdrMode != (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stWideDynamicRange.penMode[1])
    {
        ONVIF_ERR("value of stImageRevInfo.wdrMode is invaild!\n");
        stImageRevInfo.wdrMode = (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stWideDynamicRange.penMode[0];
    }

    if(stImageRevInfo.wdrLevel < (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stWideDynamicRange.stLevel.min \
        || stImageRevInfo.wdrLevel > (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stWideDynamicRange.stLevel.max)
    {
        ONVIF_ERR("value of stImageRevInfo.wdrLevel is invaild!\n");
        stImageRevInfo.wdrLevel = ((unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stWideDynamicRange.stLevel.min \
            + (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stWideDynamicRange.stLevel.max)/2;
    }
    if(stImageRevInfo.wbMode != (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stWhiteBalance.penMode[0] \
        && stImageRevInfo.wbMode != (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stWhiteBalance.penMode[1])
    {
        ONVIF_ERR("value of stImageRevInfo.wbMode is invaild!\n");
        stImageRevInfo.wbMode = (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stWhiteBalance.penMode[0];
    }
    if(stImageRevInfo.mwbRGain < (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stWhiteBalance.stYrGain.min \
        || stImageRevInfo.mwbRGain > (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stWhiteBalance.stYrGain.max)
    {
        ONVIF_ERR("value of stImageRevInfo.mwbRGain is invaild!\n");
        stImageRevInfo.mwbRGain = ((unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stWhiteBalance.stYrGain.min \
            + (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stWhiteBalance.stYrGain.max)/2;
    }
    if(stImageRevInfo.mwbBGain < (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stWhiteBalance.stYbGain.min \
        || stImageRevInfo.mwbBGain > (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stWhiteBalance.stYbGain.max)
    {
        ONVIF_ERR("value of stImageRevInfo.mwbBGain is invaild!\n");
        stImageRevInfo.mwbBGain = ((unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stWhiteBalance.stYbGain.min \
            + (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stWhiteBalance.stYbGain.max)/2;
    }
#endif

#if 0
/*BLC*/
    //blc mode, on or off
    pstImagingSettings->stBacklightCompensation.enMode = (GONVIF_IMAGE_BackLightCompensation_Mode_E)stImageRevInfo.blcMode;
    //blc level
    pstImagingSettings->stBacklightCompensation.level = (float)stImageRevInfo.blcLevel;
#endif
    pstImagingSettings->brightness = (float)stImageRevInfo.brightness;
    pstImagingSettings->colorSaturation = (float)stImageRevInfo.colorSaturation;
    pstImagingSettings->contrast = (float)stImageRevInfo.contrast;
#if 0
/*Exposure*/
    //Exposure Mode, auto or manual
    pstImagingSettings->stExposure.enMode = (GONVIF_IMAGE_Exposure_Mode_E)stImageRevInfo.exposureMode;
    //Auto Exposure: Priority
    
    //Auto Exposure: Window
    
    //Auto Exposure: MinExposureTime
    pstImagingSettings->stExposure.minExposureTime = (float)stImageRevInfo.aeShutterTimeMin;
    //Auto Exposure: MaxExposureTime
    pstImagingSettings->stExposure.maxExposureTime = (float)stImageRevInfo.aeShutterTimeMax;
    //Auto Exposure: MinGain
    pstImagingSettings->stExposure.minGain = (float)stImageRevInfo.aeGainMin;
    //Auto Exposure: MaxGain
    pstImagingSettings->stExposure.maxGain = (float)stImageRevInfo.aeGainMax;
    //Auto Exposure: MinIris
    
    //Auto Exposure: MaxIris
    
    //Manual Exposure: ExposureTime
    pstImagingSettings->stExposure.exposureTime = (float)stImageRevInfo.meShutterTime;
    //Manual Exposure: Gain
    pstImagingSettings->stExposure.gain = (float)stImageRevInfo.meGain;
    //Manual Exposure: Iris
#endif

#if 0
/*Focus*/
    //Focus mode, auto or manual
    pstImagingSettings->stFocus.enAutoFocusMode = (GONVIF_IMAGE_Focus_Mode_E)stImageRevInfo.focusMode;
    //Auto Focus: NearLimit
    pstImagingSettings->stFocus.nearLimit = (float)stImageRevInfo.afNearLimit;
    //Auto Focus: FarLimit
    pstImagingSettings->stFocus.farLimit = (float)stImageRevInfo.afFarLimit;
    //Manual Focus: DefualtSpeed
    pstImagingSettings->stFocus.defaultSpeed = (float)stImageRevInfo.mfDefaultSpeed;
#endif

    pstImagingSettings->enIrCutFilter = (GONVIF_IMAGE_IrCutFilter_Mode_E)stImageRevInfo.irCutFilter;
    pstImagingSettings->sharpness = (float)stImageRevInfo.sharpness;
#if 0
/*Wide Dynamic Range*/
    //Mode, on or off
    pstImagingSettings->stWideDynamicRange.enMode = (GONVIF_IMAGE_WideDynamicRange_Mode_E)stImageRevInfo.wdrMode;
    //Level
    pstImagingSettings->stWideDynamicRange.level = (float)stImageRevInfo.wdrLevel;
#endif

    pstImagingSettings->stWhiteBalance.enMode = (GONVIF_IMAGE_WhiteBalance_Mode_E)stImageRevInfo.wbMode;
#if 0
    //Manual WhiteBalance: CrGain
    pstImagingSettings->stWhiteBalance.crGain = (float)stImageRevInfo.mwbRGain;
    //Manual WhiteBalance: CbGain
    pstImagingSettings->stWhiteBalance.cbGain = (float)stImageRevInfo.mwbBGain;
#endif
    return ONVIF_OK;
}

/***************************************************************************
函数描述: 获取图像设置信息
入口参数：pstGetImagingSettingsReq : 获取图像设置请求信息
				pstGetImagingSettingsRes : 获取图像设置返回信息
返回值     ：ONVIF_OK: 成功
         		      非零: 失败
***************************************************************************/
ONVIF_RET GK_NVT_Image_GetImagingSettings(GONVIF_IMAGE_GetSettings_Token_S *pstGetImagingSettingsReq, GONVIF_IMAGE_GetSettings_S *pstGetImagingSettingsRes)
{  
    if((pstGetImagingSettingsReq->aszVideoSourceToken == NULL) || (strcmp(pstGetImagingSettingsReq->aszVideoSourceToken, g_GkIpc_OnvifInf.gokeIPCameraVideoSource[0].pszSourceToken) != 0))
    {
    	ONVIF_ERR("Invalid video source token.");
        return ONVIF_ERR_IMAGE_INVALID_VIDEO_SOURCE_TOKEN;
    }
    
    GK_NVT_Image_GetImaging(&pstGetImagingSettingsRes->stImageSettings);

    return ONVIF_OK;
}

/***************************************************************************
函数描述: 图像设置
入口参数：pstSetImagingSettingsReq : 图像设置请求信息
返回值     ：ONVIF_OK: 成功
         		      非零: 失败
***************************************************************************/
ONVIF_RET GK_NVT_Image_SetImagingSettings(GONVIF_IMAGE_SetSettings_S *pstSetImagingSettingsReq)
{
    GK_S8 retVal = ONVIF_OK;
    if(strcmp(pstSetImagingSettingsReq->aszVideoSourceToken, g_GkIpc_OnvifInf.gokeIPCameraVideoSource[0].pszSourceToken) != 0)
    {
    	ONVIF_ERR("Invalid video source token.");
        return ONVIF_ERR_IMAGE_INVALID_VIDEO_SOURCE_TOKEN;
    } 
    ONVIF_ImagingInfor_S stImageRevInfo;
    memset(&stImageRevInfo, 0, sizeof(ONVIF_ImagingInfor_S));
    if(g_stImagingAdapter.GetImagingInfor(&stImageRevInfo) != 0)
    {
    	ONVIF_ERR("Image server run error.");
        return ONVIF_ERR_IMAGE_SERVER_RUN_ERROR;
    }
#if 0
    ONVIF_DBG("GK_NVT_Image_SetImagingSettings():\n\
stImageRevInfo.\nblcmode = %d\n\
blcLevel = %d\nbrightness = %f\ncolorsaturation = %f\ncontrast = %f\n\
exposureMode = %d\naeShutterTimeMin = %f\naeShutterTimeMax = %f\naeGainMin = %f\naeGainMax = %f\nmeShutterTime = %f\nmeGain = %f\n\
focusMode = %d\nafNearLimit = %f\nafFarLimit = %f\nmfDefaultSpeed = %f\n\
irCutFilter = %d\nsharpness = %f\n\
wdrMode = %d\nwdrlevel = %f\n\
wbMode = %d\nwbrgain = %f\nwbbgain = %f\n",
        (int)stImageRevInfo.blcMode,
        (int)stImageRevInfo.blcLevel,
        (float)stImageRevInfo.brightness,
        (float)stImageRevInfo.colorSaturation,
        (float)stImageRevInfo.contrast,
        (int)stImageRevInfo.exposureMode,
        (float)stImageRevInfo.aeShutterTimeMin,
        (float)stImageRevInfo.aeShutterTimeMax,
        (float)stImageRevInfo.aeGainMin,
        (float)stImageRevInfo.aeGainMax,
        (float)stImageRevInfo.meShutterTime,
        (float)stImageRevInfo.meGain,
        (int)stImageRevInfo.focusMode,
        (float)stImageRevInfo.afNearLimit,
        (float)stImageRevInfo.afFarLimit,
        (float)stImageRevInfo.mfDefaultSpeed,
        (int)stImageRevInfo.irCutFilter,
        (float)stImageRevInfo.sharpness,
        (int)stImageRevInfo.wdrMode,
        (float)stImageRevInfo.wdrLevel,
        (int)stImageRevInfo.wbMode,
        (float)stImageRevInfo.mwbRGain,
        (float)stImageRevInfo.mwbBGain);
#endif

#if 0
/*BLC*/
    //Mode
    if((GK_U32)(pstSetImagingSettingsReq->stImageSettings.stBacklightCompensation.enMode) != (GK_U32)(g_GkIpc_OnvifInf.gokeImagingOptions->stBacklightCompensation.penMode[0]) \
        && (GK_U32)(pstSetImagingSettingsReq->stImageSettings.stBacklightCompensation.enMode) != (GK_U32)(g_GkIpc_OnvifInf.gokeImagingOptions->stBacklightCompensation.penMode[1]))
    {
        return ONVIF_ERR_IMAGE_BLC_MODE_INVALID_ARGVAL;
    }
    if((GK_U32)(pstSetImagingSettingsReq->stImageSettings.stBacklightCompensation.enMode) != stImageRevInfo.blcMode)
    {
        retVal = gImageAdapterConfiguration.imagingSetBlcMode((GK_U32)(pstSetImagingSettingsReq->stImageSettings.stBacklightCompensation.enMode));
        if(retVal != ONVIF_OK)
        {
            return ONVIF_ERR_IMAGE_SERVER_RUN_ERROR;
        }
    }
    //Level
    if(pstSetImagingSettingsReq->stImageSettings.stBacklightCompensation.enMode == BacklightCompensationMode_ON)
    {
        if((GK_S32)(pstSetImagingSettingsReq->stImageSettings.stBacklightCompensation.level) < (GK_S32)(g_GkIpc_OnvifInf.gokeImagingOptions->stBacklightCompensation.stLevel.min) \
            || (GK_S32)(pstSetImagingSettingsReq->stImageSettings.stBacklightCompensation.level) > (GK_S32)(g_GkIpc_OnvifInf.gokeImagingOptions->stBacklightCompensation.stLevel.max))
        {
            return ONVIF_ERR_IMAGE_BLC_LEVEL_INVALID_ARGVAL;
        }
        if((GK_S32)(pstSetImagingSettingsReq->stImageSettings.stBacklightCompensation.level) != stImageRevInfo.blcLevel)
        {
            retVal = gImageAdapterConfiguration.imagingSetBlcLevel((GK_S32)(pstSetImagingSettingsReq->stImageSettings.stBacklightCompensation.level));
            if(retVal != ONVIF_OK)
            {
                return ONVIF_ERR_IMAGE_SERVER_RUN_ERROR;
            }
        }
    }
#endif

    if((GK_S32)(pstSetImagingSettingsReq->stImageSettings.brightness) < (GK_S32)(g_GkIpc_OnvifInf.gokeImagingOptions->stBrightness.min) \
        || (GK_S32)(pstSetImagingSettingsReq->stImageSettings.brightness) > (GK_S32)(g_GkIpc_OnvifInf.gokeImagingOptions->stBrightness.max))
    {
        return ONVIF_ERR_IMAGE_BRIGHTNESS_INVALID_ARGVAL;
    }
    if((GK_S32)(pstSetImagingSettingsReq->stImageSettings.brightness) != stImageRevInfo.brightness)
    {
        retVal = g_stImagingAdapter.SetBrightness(pstSetImagingSettingsReq->stImageSettings.brightness);
        if(retVal != 0)
        {
            return ONVIF_ERR_IMAGE_SERVER_RUN_ERROR;
        }
    }
    
    if((GK_S32)(pstSetImagingSettingsReq->stImageSettings.colorSaturation) < (GK_S32)(g_GkIpc_OnvifInf.gokeImagingOptions->stColorSaturation.min) \
        || (GK_S32)(pstSetImagingSettingsReq->stImageSettings.colorSaturation) > (GK_S32)(g_GkIpc_OnvifInf.gokeImagingOptions->stColorSaturation.max))
    {
        return ONVIF_ERR_IMAGE_COLORSATURETION_INVALID_ARGVAL;
    }
    if((GK_S32)(pstSetImagingSettingsReq->stImageSettings.colorSaturation) != stImageRevInfo.colorSaturation)
    {
        retVal = g_stImagingAdapter.SetColorSaturation(pstSetImagingSettingsReq->stImageSettings.colorSaturation);
        if(retVal != 0)
        {
            return ONVIF_ERR_IMAGE_SERVER_RUN_ERROR;
        }
    }

    if((GK_S32)(pstSetImagingSettingsReq->stImageSettings.contrast) < (GK_S32)(g_GkIpc_OnvifInf.gokeImagingOptions->stContrast.min) \
        || (GK_S32)(pstSetImagingSettingsReq->stImageSettings.contrast) > (GK_S32)(g_GkIpc_OnvifInf.gokeImagingOptions->stContrast.max))
    {
        return ONVIF_ERR_IMAGE_CONTRAST_INVALID_ARGVAL;
    }
    if((GK_S32)(pstSetImagingSettingsReq->stImageSettings.contrast) != stImageRevInfo.contrast)
    {
        retVal = g_stImagingAdapter.SetContrast(pstSetImagingSettingsReq->stImageSettings.contrast);
        if(retVal != 0)
        {
            return ONVIF_ERR_IMAGE_SERVER_RUN_ERROR;
        }
    }
#if 0
/*Exposure*/
    if((unsigned long)(pstSetImagingSettingsReq->stImageSettings.stExposure.enMode) != (unsigned long)(g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.penMode[0]) \
        && (unsigned long)(pstSetImagingSettingsReq->stImageSettings.stExposure.enMode) != (unsigned long)(g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.penMode[1]))
    {
        return ONVIF_ERR_IMAGE_EXPOSURE_MODE_INVALID_ARGVAL;
    }
    //AUTO
    if(pstSetImagingSettingsReq->stImageSettings.stExposure.enMode == ExposureMode_AUTO)
    {
        //Mode
        if(pstSetImagingSettingsReq->stImageSettings.stExposure.enMode != (int)stImageRevInfo.exposureMode)
        {
            retVal = gImageAdapterConfiguration.imagingSetExposureMode(pstSetImagingSettingsReq->stImageSettings.stExposure.enMode);
            //sleep(1);
            if(retVal != ONVIF_OK)
            {
                return ONVIF_ERR_IMAGE_SERVER_RUN_ERROR;
            }
        }
        
        //Auto Exposure: MinExposureTime & MaxExposureTime
        if((unsigned long)(pstSetImagingSettingsReq->stImageSettings.stExposure.minExposureTime) < (unsigned long)(g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stMinExposureTime.min) \
            || (unsigned long)(pstSetImagingSettingsReq->stImageSettings.stExposure.minExposureTime) > (unsigned long)(g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stMinExposureTime.max))
        {
            return ONVIF_ERR_IMAGE_EXPOSURE_MIN_TIME_INVALID_ARGVAL;
        }      
        if((unsigned long)(pstSetImagingSettingsReq->stImageSettings.stExposure.maxExposureTime) < (unsigned long)(g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stMaxExposureTime.min) \
            || (unsigned long)(pstSetImagingSettingsReq->stImageSettings.stExposure.maxExposureTime) > (unsigned long)(g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stMaxExposureTime.max))
        {
            return ONVIF_ERR_IMAGE_EXPOSURE_MAX_TIME_INVALID_ARGVAL;
        }  
        if((unsigned long)(pstSetImagingSettingsReq->stImageSettings.stExposure.minExposureTime) <= (unsigned long)(pstSetImagingSettingsReq->stImageSettings.stExposure.maxExposureTime))
        {
            if((int)(pstSetImagingSettingsReq->stImageSettings.stExposure.minExposureTime) != (int)stImageRevInfo.aeShutterTimeMin)
            {
                retVal = gImageAdapterConfiguration.imagingSetAeShutterTimeMin((int)(pstSetImagingSettingsReq->stImageSettings.stExposure.minExposureTime));
                if( retVal != ONVIF_OK)
                {
                    return ONVIF_ERR_IMAGE_SERVER_RUN_ERROR;
                }
            }
            if((int)(pstSetImagingSettingsReq->stImageSettings.stExposure.maxExposureTime) != (int)stImageRevInfo.aeShutterTimeMax)
            {
                retVal = gImageAdapterConfiguration.imagingSetAeShutterTimeMax((int)(pstSetImagingSettingsReq->stImageSettings.stExposure.maxExposureTime));
                if( retVal != ONVIF_OK)
                {
                    return ONVIF_ERR_IMAGE_SERVER_RUN_ERROR;
                }
            }
        }
        else
        {
            return ONVIF_ERR_IMAGE_EXPOSURE_TIME_MIN_GREATER_MAX_INVALID_ARGVAL;
        }
        
        //Auto Exposure: MinGain & MaxGain
        if((unsigned long)(pstSetImagingSettingsReq->stImageSettings.stExposure.minGain) < (unsigned long)(g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stMinGain.min) \
            || (unsigned long)(pstSetImagingSettingsReq->stImageSettings.stExposure.minGain) > (unsigned long)(g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stMinGain.max))
        {
            return ONVIF_ERR_IMAGE_EXPOSURE_MIN_GAIN_INVALID_ARGVAL;
        }
        if((unsigned long)(pstSetImagingSettingsReq->stImageSettings.stExposure.maxGain) < (unsigned long)(g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stMaxGain.min) \
            || (unsigned long)(pstSetImagingSettingsReq->stImageSettings.stExposure.maxGain) > (unsigned long)(g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stMaxGain.max))
        {
            return ONVIF_ERR_IMAGE_EXPOSURE_MAX_GAIN_INVALID_ARGVAL;
        }
        if((unsigned long)(pstSetImagingSettingsReq->stImageSettings.stExposure.minGain) <= (unsigned long)(pstSetImagingSettingsReq->stImageSettings.stExposure.maxGain) )
        {
            if((int)(pstSetImagingSettingsReq->stImageSettings.stExposure.minGain) != (int)stImageRevInfo.aeGainMin)
            {
                retVal = gImageAdapterConfiguration.imagingSetAeGainMin((int)(pstSetImagingSettingsReq->stImageSettings.stExposure.minGain));
                if( retVal != 0)
                {
                    return ONVIF_ERR_IMAGE_SERVER_RUN_ERROR;
                }
            }
            if((int)(pstSetImagingSettingsReq->stImageSettings.stExposure.maxGain) != (int)stImageRevInfo.aeGainMax)
            {
                retVal = gImageAdapterConfiguration.imagingSetAeGainMax((int)(pstSetImagingSettingsReq->stImageSettings.stExposure.maxGain));
                if( retVal != 0)
                {
                    return ONVIF_ERR_IMAGE_SERVER_RUN_ERROR;
                }
            }
        }
        else
        {
            return ONVIF_ERR_IMAGE_EXPOSURE_GAIN_MIN_GREATER_MAX_INVALID_ARGVAL;
        }

        //Auto Exposure: MinIris & MaxIris

    }
    //MANUAL
    else
    {
        //Mode
        if(pstSetImagingSettingsReq->stImageSettings.stExposure.enMode != (int)stImageRevInfo.exposureMode)
        {
            retVal = gImageAdapterConfiguration.imagingSetExposureMode(pstSetImagingSettingsReq->stImageSettings.stExposure.enMode);
            //sleep(1);
            if(retVal != 0)
            {
                return ONVIF_ERR_IMAGE_SERVER_RUN_ERROR;
            }
        }
        
        //Manual Exposure: ExposureTime
        if((unsigned long)(pstSetImagingSettingsReq->stImageSettings.stExposure.exposureTime) < (unsigned long)(g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stExposureTime.min) \
            || (unsigned long)(pstSetImagingSettingsReq->stImageSettings.stExposure.exposureTime) > (unsigned long)(g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stExposureTime.max))
        {
            return ONVIF_ERR_IMAGE_EXPOSURE_TIME_INVALID_ARGVAL;
        }
        if((int)(pstSetImagingSettingsReq->stImageSettings.stExposure.exposureTime) != (int)stImageRevInfo.meShutterTime)
        {
            retVal = gImageAdapterConfiguration.imagingSetMeShutterTime((int)(pstSetImagingSettingsReq->stImageSettings.stExposure.exposureTime));
            if( retVal != 0)
            {
                return ONVIF_ERR_IMAGE_SERVER_RUN_ERROR;
            }
        }
        
        //Manual Exposure: Gain
        if((unsigned long)(pstSetImagingSettingsReq->stImageSettings.stExposure.gain) < (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stGain.min \
            || (unsigned long)(pstSetImagingSettingsReq->stImageSettings.stExposure.gain) > (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stGain.max)
        {
            return ONVIF_ERR_IMAGE_EXPOSURE_GAIN_INVALID_ARGVAL;
        }
        if((int)(pstSetImagingSettingsReq->stImageSettings.stExposure.gain) != (int)stImageRevInfo.meGain)
        {
            retVal = gImageAdapterConfiguration.imagingSetMeGain((int)(pstSetImagingSettingsReq->stImageSettings.stExposure.gain));
            if( retVal != 0)
            {
                return ONVIF_ERR_IMAGE_SERVER_RUN_ERROR;
            }
        }
        
        //Manual Exposure: Iris
        
    }

/*Focus*/
    if((unsigned long)(pstSetImagingSettingsReq->stImageSettings.stFocus.enAutoFocusMode) != (unsigned long)g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.penAutoFocusModes[0] \
        && (unsigned long)(pstSetImagingSettingsReq->stImageSettings.stFocus.enAutoFocusMode) != (unsigned long)(g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.penAutoFocusModes[1]))
    {
        return ONVIF_ERR_IMAGE_FOCUS_MODE_INVALID_ARGVAL;
    }
    //AUTO
    if(pstSetImagingSettingsReq->stImageSettings.stFocus.enAutoFocusMode == AutoFocusMode_AUTO)
    {
        //Mode
        if(pstSetImagingSettingsReq->stImageSettings.stFocus.enAutoFocusMode != (int)stImageRevInfo.focusMode)
        {
            retVal = gImageAdapterConfiguration.imagingSetFocusMode(pstSetImagingSettingsReq->stImageSettings.stFocus.enAutoFocusMode);
            //sleep(1);
            if(retVal != 0)
            {
                return ONVIF_ERR_IMAGE_SERVER_RUN_ERROR;
            }
        }
        //Auto Focus: NearLimit & FarLimit
        if((unsigned long)(pstSetImagingSettingsReq->stImageSettings.stFocus.nearLimit) < (unsigned long)(g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.stNearLimit.min) \
            || (unsigned long)(pstSetImagingSettingsReq->stImageSettings.stFocus.nearLimit) > (unsigned long)(g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.stNearLimit.max))
        {
            return ONVIF_ERR_IMAGE_FOCUS_NEAR_LIMIT_INVALID_ARGVAL;
        }
        if((unsigned long)(pstSetImagingSettingsReq->stImageSettings.stFocus.farLimit) < (unsigned long)(g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.stFarLimit.min) \
            || (unsigned long)(pstSetImagingSettingsReq->stImageSettings.stFocus.farLimit) > (unsigned long)(g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.stFarLimit.max))
        {
            return ONVIF_ERR_IMAGE_FOCUS_FAR_LIMIT_INVALID_ARGVAL;
        }
        if((unsigned long)(pstSetImagingSettingsReq->stImageSettings.stFocus.nearLimit) <= (unsigned long)(pstSetImagingSettingsReq->stImageSettings.stFocus.farLimit))
        {
            if((int)(pstSetImagingSettingsReq->stImageSettings.stFocus.nearLimit) != (int)stImageRevInfo.afNearLimit)
            {
                retVal = gImageAdapterConfiguration.imagingSetAfNearLimit((int)(pstSetImagingSettingsReq->stImageSettings.stFocus.nearLimit));
                if( retVal != 0)
                {
                    return ONVIF_ERR_IMAGE_SERVER_RUN_ERROR;
                }
            }
            if((int)(pstSetImagingSettingsReq->stImageSettings.stFocus.farLimit) != (int)stImageRevInfo.afFarLimit)
            {
                retVal = gImageAdapterConfiguration.imagingSetAfFarLimit((int)(pstSetImagingSettingsReq->stImageSettings.stFocus.farLimit));
                if( retVal != 0)
                {
                    return ONVIF_ERR_IMAGE_SERVER_RUN_ERROR;
                }
            }
        }
        else
        {
            return ONVIF_ERR_IMAGE_FOCUS_LIMIT_NEAR_GREATER_FAR_INVALID_ARGVAL;
        }
        
    }
    //MANUAL
    else
    {
        //Mode
        if(pstSetImagingSettingsReq->stImageSettings.stFocus.enAutoFocusMode != (int)stImageRevInfo.focusMode)
        {
            retVal = gImageAdapterConfiguration.imagingSetFocusMode(pstSetImagingSettingsReq->stImageSettings.stFocus.enAutoFocusMode);
            //sleep(1);
            if(retVal != 0)
            {
                return ONVIF_ERR_IMAGE_SERVER_RUN_ERROR;
            }
        }
        //Manual Focus: DefualtSpeed
        if((unsigned long)(pstSetImagingSettingsReq->stImageSettings.stFocus.defaultSpeed) < (unsigned long)(g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.stDefaultSpeed.min) \
            || (unsigned long)(pstSetImagingSettingsReq->stImageSettings.stFocus.defaultSpeed) > (unsigned long)(g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.stDefaultSpeed.max))
        {
            return ONVIF_ERR_IMAGE_FOCUS_DEFAULT_SPEED_INVALID_ARGVAL;
        }
        if((int)(pstSetImagingSettingsReq->stImageSettings.stFocus.defaultSpeed) != (int)stImageRevInfo.mfDefaultSpeed)
        {
            retVal = gImageAdapterConfiguration.imagingSetMfDefaultSpeed((int)(pstSetImagingSettingsReq->stImageSettings.stFocus.defaultSpeed));
            if( retVal != 0)
            {
                return ONVIF_ERR_IMAGE_SERVER_RUN_ERROR;
            }
        }
    }
#endif

    if((unsigned long)(pstSetImagingSettingsReq->stImageSettings.enIrCutFilter) != (unsigned long)(g_GkIpc_OnvifInf.gokeImagingOptions->penIrCutFilterModes[0]) \
        && (unsigned long)(pstSetImagingSettingsReq->stImageSettings.enIrCutFilter) != (unsigned long)(g_GkIpc_OnvifInf.gokeImagingOptions->penIrCutFilterModes[1]) \
        && (unsigned long)(pstSetImagingSettingsReq->stImageSettings.enIrCutFilter) != (unsigned long)(g_GkIpc_OnvifInf.gokeImagingOptions->penIrCutFilterModes[2]))
    {
        return ONVIF_ERR_IMAGE_IRCUTFILTER_INVALID_ARGVAL;
    }
    if(pstSetImagingSettingsReq->stImageSettings.enIrCutFilter != (int)stImageRevInfo.irCutFilter)
    {
        retVal = g_stImagingAdapter.SetIrCutFilter(pstSetImagingSettingsReq->stImageSettings.enIrCutFilter);
        if(retVal != 0)
        {
            return ONVIF_ERR_IMAGE_SERVER_RUN_ERROR;
        }
    }

    if((GK_S32)(pstSetImagingSettingsReq->stImageSettings.sharpness) < (GK_S32)(g_GkIpc_OnvifInf.gokeImagingOptions->stSharpness.min) \
        || (GK_S32)(pstSetImagingSettingsReq->stImageSettings.sharpness) > (GK_S32)(g_GkIpc_OnvifInf.gokeImagingOptions->stSharpness.max))
    {
        return ONVIF_ERR_IMAGE_SHARPNESS_INVALID_ARGVAL;
    }
    if((GK_S32)(pstSetImagingSettingsReq->stImageSettings.sharpness) != stImageRevInfo.sharpness)
    {
        retVal = g_stImagingAdapter.SetSharpness(pstSetImagingSettingsReq->stImageSettings.sharpness);
        if(retVal != 0)
        {
            return ONVIF_ERR_IMAGE_SERVER_RUN_ERROR;
        }
    }
#if 0
/*Wide Dynamic Range*/
    //Mode
    if((unsigned long)pstSetImagingSettingsReq->stImageSettings.stWideDynamicRange.enMode != (unsigned long)(g_GkIpc_OnvifInf.gokeImagingOptions->stWideDynamicRange.penMode[0]) \
        && (unsigned long)pstSetImagingSettingsReq->stImageSettings.stWideDynamicRange.enMode != (unsigned long)(g_GkIpc_OnvifInf.gokeImagingOptions->stWideDynamicRange.penMode[1]))
    {
        return ONVIF_ERR_IMAGE_WIDEDYNAMICRANGE_MDOE_INVALID_ARGVAL;
    }
    if(pstSetImagingSettingsReq->stImageSettings.stWideDynamicRange.enMode != (int)stImageRevInfo.wdrMode)
    {
        retVal = gImageAdapterConfiguration.imagingSetWdrMode(pstSetImagingSettingsReq->stImageSettings.stWideDynamicRange.enMode);
        if(retVal != 0)
        {
            return ONVIF_ERR_IMAGE_SERVER_RUN_ERROR;
        }
    }
    //Level
    if(pstSetImagingSettingsReq->stImageSettings.stWideDynamicRange.enMode == WideDynamicMode_ON)
    {
        if((unsigned long)(pstSetImagingSettingsReq->stImageSettings.stWideDynamicRange.level) < (unsigned long)(g_GkIpc_OnvifInf.gokeImagingOptions->stWideDynamicRange.stLevel.min) \
            || (unsigned long)(pstSetImagingSettingsReq->stImageSettings.stWideDynamicRange.level) > (unsigned long)(g_GkIpc_OnvifInf.gokeImagingOptions->stWideDynamicRange.stLevel.max))
        {
            return ONVIF_ERR_IMAGE_WIDEDYNAMICRANGE_LEVEL_INVALID_ARGVAL;
        }
        if((int)(pstSetImagingSettingsReq->stImageSettings.stWideDynamicRange.level) != (int)stImageRevInfo.wdrLevel)
        {
            retVal = gImageAdapterConfiguration.imagingSetWdrLevel((int)(pstSetImagingSettingsReq->stImageSettings.stWideDynamicRange.level));
            if(retVal != 0)
            {
                return ONVIF_ERR_IMAGE_SERVER_RUN_ERROR;
            }
        }
    }

/*WhiteBalance*/
    //Mode
    if((unsigned long)pstSetImagingSettingsReq->stImageSettings.stWhiteBalance.enMode != (unsigned long)(g_GkIpc_OnvifInf.gokeImagingOptions->stWhiteBalance.penMode[0]) \
        && (unsigned long)pstSetImagingSettingsReq->stImageSettings.stWhiteBalance.enMode != (unsigned long)(g_GkIpc_OnvifInf.gokeImagingOptions->stWhiteBalance.penMode[1]))
    {
        return ONVIF_ERR_IMAGE_WHITEBALANCE_MODE_INVALID_ARGVAL;
    }
    if(pstSetImagingSettingsReq->stImageSettings.stWhiteBalance.enMode != (int)stImageRevInfo.wbMode)
    {
        retVal = g_stImagingAdapter.SetWbMode(pstSetImagingSettingsReq->stImageSettings.stWhiteBalance.enMode);
        if(retVal != 0)
        {
            return ONVIF_ERR_IMAGE_SERVER_RUN_ERROR;
        }
    }   
    //gain
    if(pstSetImagingSettingsReq->stImageSettings.stWhiteBalance.enMode == WhiteBalanceMode_MANUAL)
    {
        if((unsigned long)(pstSetImagingSettingsReq->stImageSettings.stWhiteBalance.crGain) < (unsigned long)(g_GkIpc_OnvifInf.gokeImagingOptions->stWhiteBalance.stYrGain.min) \
            || (unsigned long)(pstSetImagingSettingsReq->stImageSettings.stWhiteBalance.crGain) > (unsigned long)(g_GkIpc_OnvifInf.gokeImagingOptions->stWhiteBalance.stYrGain.max))
        {
            return ONVIF_ERR_IMAGE_WHITEBALANCE_CR_GAIN_INVALID_ARGVAL;
        }
        if((int)(pstSetImagingSettingsReq->stImageSettings.stWhiteBalance.crGain) != (int)stImageRevInfo.mwbRGain)
        {
            retVal = gImageAdapterConfiguration.imagingSetMwbRGain((int)(pstSetImagingSettingsReq->stImageSettings.stWhiteBalance.crGain));
            if(retVal != 0)
            {
                return ONVIF_ERR_IMAGE_SERVER_RUN_ERROR;
            }
        }
        if((unsigned long)(pstSetImagingSettingsReq->stImageSettings.stWhiteBalance.cbGain) < (unsigned long)(g_GkIpc_OnvifInf.gokeImagingOptions->stWhiteBalance.stYbGain.min) \
            || (unsigned long)(pstSetImagingSettingsReq->stImageSettings.stWhiteBalance.cbGain) > (unsigned long)(g_GkIpc_OnvifInf.gokeImagingOptions->stWhiteBalance.stYbGain.max))
        {
            return ONVIF_ERR_IMAGE_WHITEBALANCE_CB_GAIN_INVALID_ARGVAL;
        }
        if((int)(pstSetImagingSettingsReq->stImageSettings.stWhiteBalance.cbGain) != (int)stImageRevInfo.mwbBGain)
        {
            retVal = gImageAdapterConfiguration.imagingSetMwbBGain((int)(pstSetImagingSettingsReq->stImageSettings.stWhiteBalance.cbGain));
            if(retVal != 0)
            {
                return ONVIF_ERR_IMAGE_SERVER_RUN_ERROR;
            }
        }
    }
#endif
  
    if((pstSetImagingSettingsReq->enForcePersistence != Boolean_FALSE) && (pstSetImagingSettingsReq->enForcePersistence != Boolean_TRUE))
    {
        return ONVIF_ERR_IAMGE_FORCE_PERSISTENCE_INVALID_ARGVAL;
    }
    else
    {
        if(pstSetImagingSettingsReq->enForcePersistence == Boolean_TRUE)
        g_stImagingAdapter.SaveCfg();
    }

    return ONVIF_OK;
}

/***************************************************************************
函数描述: 获取图像设置选项
入口参数：pstGetImageOptionsReq : 获取图像设置选项请求信息
				pstGetImageOptionsRes : 获取图像设置选项返回信息
返回值     ：ONVIF_OK: 成功
         		      ONVIF_ERR_IMAGE_INVALID_VIDEO_SOURCE_TOKEN: 失败
***************************************************************************/
ONVIF_RET GK_NVT_Image_GetOptions(GONVIF_IMAGE_GetOptions_Token_S *pstGetImageOptionsReq, GONVIF_IMAGE_GetOptions_S *pstGetImageOptionsRes)
{
    if ((pstGetImageOptionsReq->aszVideoSourceToken == NULL) || (strcmp(pstGetImageOptionsReq->aszVideoSourceToken, g_GkIpc_OnvifInf.gokeIPCameraVideoSource[0].pszSourceToken) != 0))
    {
        return ONVIF_ERR_IMAGE_INVALID_VIDEO_SOURCE_TOKEN;
    }
	memcpy(&(pstGetImageOptionsRes->stGetImageOptions), g_GkIpc_OnvifInf.gokeImagingOptions, sizeof(pstGetImageOptionsRes->stGetImageOptions));

    
    return ONVIF_OK;
}

/***************************************************************************
函数描述: 焦点运动控制
入口参数：pstMoveReq : 焦点运动控制请求信息
返回值     ：ONVIF_OK: 成功
         		       非零: 失败
***************************************************************************/
ONVIF_RET GK_NVT_Image_Move(GONVIF_IMAGE_Move_S *pstMoveReq)
{    
	if (strcmp(pstMoveReq->aszVideoSourceToken, g_GkIpc_OnvifInf.gokeIPCameraVideoSource[0].pszSourceToken) != 0)
    {
        return ONVIF_ERR_IMAGE_INVALID_VIDEO_SOURCE_TOKEN;
	}
	if(pstMoveReq->stFocus.stAbsolute != NULL)
	{
        if(pstMoveReq->stFocus.stAbsolute->position != NULL)
        {
            if(((GK_S32)(*(pstMoveReq->stFocus.stAbsolute->position)) >= ((GK_S32)(g_GkIpc_OnvifInf.gokeMoveOptions->stAbsolute.stPosition.min))) \
                && ((GK_S32)(*(pstMoveReq->stFocus.stAbsolute->position)) <= ((GK_S32)(g_GkIpc_OnvifInf.gokeMoveOptions->stAbsolute.stPosition.max))))
            {
                if((GK_S32)(*(pstMoveReq->stFocus.stAbsolute->position)) != 0)
                    ONVIF_ERR("absolute position had been set!\n");
            }
            else
            {
                return ONVIF_ERR_IMAGE_FOCUS_MOVE_ABSOLUTE_POSITION_INVALID_ARGVAL;
            }            
        }
        if(pstMoveReq->stFocus.stAbsolute->speed != NULL)
        {
            if(((GK_S32)(*(pstMoveReq->stFocus.stAbsolute->speed)) >= ((GK_S32)(g_GkIpc_OnvifInf.gokeMoveOptions->stAbsolute.stSpeed.min))) \
                && ((GK_S32)(*(pstMoveReq->stFocus.stAbsolute->speed)) <= ((GK_S32)(g_GkIpc_OnvifInf.gokeMoveOptions->stAbsolute.stSpeed.max))))
            {
                if((GK_S32)(*(pstMoveReq->stFocus.stAbsolute->speed)) != 0) 
                    ONVIF_ERR("absolute speed had been set!\n");
            }
            else
            {
                return ONVIF_ERR_IMAGE_FOCUS_MOVE_ABSOLUTE_SPEED_INVALID_ARGVAL;
            } 
        }
	}	
	else if(pstMoveReq->stFocus.stRelative != NULL)
	{
        if(pstMoveReq->stFocus.stRelative->distance != NULL)
        {
            if(((GK_S32)(*(pstMoveReq->stFocus.stRelative->distance)) >= ((GK_S32)(g_GkIpc_OnvifInf.gokeMoveOptions->stRelative.stDistance.min))) \
                && ((GK_S32)(*(pstMoveReq->stFocus.stRelative->distance)) <= ((GK_S32)(g_GkIpc_OnvifInf.gokeMoveOptions->stRelative.stDistance.max))))
            {
                if((GK_S32)(*(pstMoveReq->stFocus.stRelative->distance)) != 0) 
                    ONVIF_ERR("relative distance had been set!\n");
            }
            else
            {
                if((GK_S32)(g_GkIpc_OnvifInf.gokeMoveOptions->stRelative.stDistance.min) - (GK_S32)(*(pstMoveReq->stFocus.stRelative->distance)) == 1)
                    ;
                else if((GK_S32)(*(pstMoveReq->stFocus.stRelative->distance)) - (GK_S32)(g_GkIpc_OnvifInf.gokeMoveOptions->stRelative.stDistance.max) == 1)
                    ;
                else
                    return ONVIF_ERR_IMAGE_FOCUS_MOVE_RELATIVE_DISTANCE_INVALID_ARGVAL;
            } 
        }
        if(pstMoveReq->stFocus.stRelative->speed != NULL)
        {
            if(((GK_S32)(*(pstMoveReq->stFocus.stRelative->speed)) >= ((GK_S32)(g_GkIpc_OnvifInf.gokeMoveOptions->stRelative.stSpeed.min))) \
                && ((GK_S32)(*(pstMoveReq->stFocus.stRelative->speed)) <= ((GK_S32)(g_GkIpc_OnvifInf.gokeMoveOptions->stRelative.stSpeed.max))))
            {
                if((GK_S32)(*(pstMoveReq->stFocus.stRelative->speed)) != 0) 
                    ONVIF_ERR("relative speed had been set!\n");
            }
            else
            {
                return ONVIF_ERR_IMAGE_FOCUS_MOVE_RELATIVE_DISTANCE_INVALID_ARGVAL;
            } 
        }
   	}
	else if(pstMoveReq->stFocus.stContinuous != NULL)
	{
        if(pstMoveReq->stFocus.stContinuous->speed != NULL)
        {
            if(((GK_S32)(*(pstMoveReq->stFocus.stContinuous->speed)) >= ((GK_S32)(g_GkIpc_OnvifInf.gokeMoveOptions->stContinuous.stSpeed.min))) \
                && ((GK_S32)(*(pstMoveReq->stFocus.stContinuous->speed)) <= ((GK_S32)(g_GkIpc_OnvifInf.gokeMoveOptions->stContinuous.stSpeed.max))))
            {
                if((GK_S32)(*(pstMoveReq->stFocus.stContinuous->speed)) != 0) 
                    ONVIF_ERR("continuos speed had been set!\n");
            }
            else
            {
                return ONVIF_ERR_IMAGE_FOCUS_MOVE_CONTINUOUS_SPEED_INVALID_ARGVAL;
            } 
        }
	}
	else
    {
        return ONVIF_ERR_IMAGE_FOCUS_MOVE_INVALID_ARGVAL;
    }

    
    return ONVIF_OK;
}

/***************************************************************************
函数描述: 焦点运动停止
入口参数：pstStopReq : 停止焦点运动请求信息
返回值     ：ONVIF_OK: 成功
         		       ONVIF_ERR_IMAGE_INVALID_VIDEO_SOURCE_TOKEN : 失败
***************************************************************************/
ONVIF_RET GK_NVT_Image_Stop(GONVIF_IMAGE_Stop_Token_S *pstStopReq)
{
    if(strcmp(pstStopReq->aszVideoSourceToken, g_GkIpc_OnvifInf.gokeIPCameraVideoSource[0].pszSourceToken) != 0)
    {
        return ONVIF_ERR_IMAGE_INVALID_VIDEO_SOURCE_TOKEN;
    }

	/* stop all ongoing movements of lense by ctlserver pocessing , not implement yet.*/
	
    
    return ONVIF_OK;
}

/***************************************************************************
函数描述: 获取图像状态
入口参数：pstGetStatusReq : 获取图像状态请求信息
				pstGetStatusRes : 获取图像状态返回信息
返回值     ：ONVIF_OK: 成功
         		       ONVIF_ERR_IMAGE_INVALID_VIDEO_SOURCE_TOKEN : 失败
***************************************************************************/
ONVIF_RET GK_NVT_Image_GetStatus(GONVIF_IMAGE_GetStatus_Token_S *pstGetStatusReq, GONVIF_IMAGE_GetStatus_S *pstGetStatusRes)
{
    if(strcmp(pstGetStatusReq->aszVideoSourceToken, g_GkIpc_OnvifInf.gokeIPCameraVideoSource[0].pszSourceToken) != 0)
    {
        return ONVIF_ERR_IMAGE_INVALID_VIDEO_SOURCE_TOKEN;
    }
    pstGetStatusRes->stImageStatus.stFocusStatus.position = 0;
    pstGetStatusRes->stImageStatus.stFocusStatus.enMoveStatus = MoveStatus_IDLE;
    strcpy(pstGetStatusRes->stImageStatus.stFocusStatus.aszError, "A positioning error indicated by the hardware.");
    
    return ONVIF_OK;
}

/***************************************************************************
函数描述:  获取焦点运动选项
入口参数：pstGetMoveOptionsReq : 获取焦点运动选项请求信息
				pstGetMoveOptionsRes : 获取焦点运动选项返回信息
返回值     ：ONVIF_OK: 成功
         		       ONVIF_ERR_IMAGE_INVALID_VIDEO_SOURCE_TOKEN : 失败
***************************************************************************/
ONVIF_RET GK_NVT_Image_GetMoveOptions(GONVIF_IMAGE_GetMoveOptions_Token_S *pstGetMoveOptionsReq, GONVIF_IMAGE_GetMoveOptions_S *pstGetMoveOptionsRes)
{
    if (strcmp(pstGetMoveOptionsReq->aszVideoSourceToken, g_GkIpc_OnvifInf.gokeIPCameraVideoSource[0].pszSourceToken) != 0)
    {
        return ONVIF_ERR_IMAGE_INVALID_VIDEO_SOURCE_TOKEN;
    }
    
	memcpy(&(pstGetMoveOptionsRes->stMoveOptions), g_GkIpc_OnvifInf.gokeMoveOptions, sizeof(pstGetMoveOptionsRes->stMoveOptions));

    return ONVIF_OK;
}




