#include "gk_cms_common.h"
#include "gk_cms_media.h"
#include "gk_cms_sock.h"
#include "gk_cms_protocol.h"
#include "gk_cms_utility.h"
#include "gk_cms_net_api.h"
#include "cfg_all.h"
#include "netcam_api.h"

#define CMS_IMAGE_MAX_ITEM_MAP 30

GK_CFG_MAP imageArr[CMS_IMAGE_MAX_ITEM_MAP];

int cms_image_get_def_map(void)
{
	int ret = 0;
	memset(imageArr, 0, CMS_IMAGE_MAX_ITEM_MAP*sizeof(GK_CFG_MAP));
   	ret = netcam_image_get_map(imageArr, CMS_IMAGE_MAX_ITEM_MAP);
	if(ret != 0)
	{
		PRINT_ERR("CMS: fail to get image map.\n");
		return -1;
	}
	
	return 0;
}

int cms_image_get_def_val(char *eleStr, int *val)
{
	int i;
	if(eleStr == NULL || val == NULL)
	{
		PRINT_ERR("CMS: Invalid parameters.\n");
		return -1;
	}
	for(i = 0; i < CMS_IMAGE_MAX_ITEM_MAP; i++)
	{
		if(imageArr[i].stringName != NULL && strcmp(eleStr, imageArr[i].stringName) == 0)
		{
			*val = atoi(imageArr[i].defaultValue);
		}
		if(i == CMS_IMAGE_MAX_ITEM_MAP-1)
		{
			return -1;
		}
	}
	
	return 0;
}


int Gk_CmsGetColorSupport(int sock)
{
	int i = 0;
	int ret = 0;
    DMS_NET_COLOR_SUPPORT color_support;
    memset(&color_support, 0, sizeof(DMS_NET_COLOR_SUPPORT));
    ret = cms_image_get_def_map();
    if(ret != 0)
    {
		return -1;
    }
    //Just add brightness, hue, saturation, contrast rignt now.
	color_support.dwSize = sizeof(DMS_NET_COLOR_SUPPORT);
	color_support.dwMask = DMS_COLOR_SET_BRIGHTNESS | DMS_COLOR_SET_HUE |\
		DMS_COLOR_SET_SATURATION | DMS_COLOR_SET_CONTRAST | DMS_COLOR_SET_DEFINITION;
	for(i = 0; i < CMS_IMAGE_MAX_ITEM_MAP; i++)
	{
		if(imageArr[i].stringName != NULL)
		{
			if(strcmp("saturation", imageArr[i].stringName) == 0)
			{
				//printf("saturation  min: %d  max: %d\n", imageArr[i].min, imageArr[i].max);
				color_support.strSaturation.nMin = imageArr[i].min;
				color_support.strSaturation.nMax = imageArr[i].max;
			}
			else if(strcmp("brightness", imageArr[i].stringName) == 0)
			{
				//printf("brightness  min: %d  max: %d\n", imageArr[i].min, imageArr[i].max);
				color_support.strBrightness.nMin = imageArr[i].min;
				color_support.strBrightness.nMax = imageArr[i].max;
			}
			else if(strcmp("contrast", imageArr[i].stringName) == 0)
			{
				//printf("contrast    min: %d  max: %d\n", imageArr[i].min, imageArr[i].max);
				color_support.strContrast.nMin = imageArr[i].min;
				color_support.strContrast.nMax = imageArr[i].max;
			}
			else if(strcmp("hue", imageArr[i].stringName) == 0)
			{
				//printf("hue         min: %d  max: %d\n", imageArr[i].min, imageArr[i].max);
				color_support.strHue.nMin = imageArr[i].min;
				color_support.strHue.nMax = imageArr[i].max;
			}
			else if(strcmp("sharpness", imageArr[i].stringName) == 0)
			{
				//printf("sharpness   min: %d  max: %d\n", imageArr[i].min, imageArr[i].max);
				color_support.strDefinition.nMin = imageArr[i].min;
				color_support.strDefinition.nMax = imageArr[i].max;
			}
		}
	}
	
    GkCmsCmdResq(sock, &color_support, sizeof(DMS_NET_COLOR_SUPPORT), DMS_NET_GET_COLOR_SUPPORT);
    return 0;
}


int Gk_CmsGetColor(int sock)
{
	int ret = 0;
    DMS_NET_CHANNEL_COLOR color;
    memset(&color, 0, sizeof(DMS_NET_CHANNEL_COLOR));
    GK_NET_IMAGE_CFG imagAttr;
    memset(&imagAttr, 0, sizeof(GK_NET_IMAGE_CFG));
	ret = netcam_image_get(&imagAttr);
    if(ret != 0)
	{
		PRINT_ERR("CMS: fail to get image attr.\n");
		return -1;
	}
	if(imagAttr.brightness < 0 || imagAttr.brightness > 100 || imagAttr.contrast < 0 || imagAttr.contrast > 100 ||\
		imagAttr.hue < 0 || imagAttr.hue > 100 || imagAttr.saturation < 0 || imagAttr.saturation > 100 ||\
		imagAttr.sharpness < 0 || imagAttr.sharpness > 100)
	{
		PRINT_ERR("CMS: Invalid image parameters.\n");
		return -1;
	}
	
    color.dwSize = sizeof(DMS_NET_CHANNEL_COLOR);
	color.nBrightness = imagAttr.brightness;
	color.nContrast   = imagAttr.contrast;
	color.nHue		  = imagAttr.hue;
	color.nSaturation = imagAttr.saturation;
	color.nDefinition = imagAttr.sharpness;

	//printf("olor.nBrightness: %d\ncolor.nContrast: %d\n"
			//"color.nHue: %d\ncolor.nSaturation: %d\nnDefinition: %d\n", 
		//color.nBrightness, color.nContrast, color.nHue, color.nSaturation, color.nDefinition);   
		
    GkCmsCmdResq(sock, &color, sizeof(DMS_NET_CHANNEL_COLOR), DMS_NET_GET_COLORCFG);
    return 0;
}

int Gk_CmsSetColor(int sock)
{
    return 0;
}

int Gk_CmsGetDefColor(int sock)
{
	int i = 0;
	//int ret = 0;
    DMS_NET_CHANNEL_COLOR color;
	memset(&color, 0, sizeof(DMS_NET_CHANNEL_COLOR));
	color.dwSize = sizeof(DMS_NET_CHANNEL_COLOR);
#if 0	
	ret = cms_image_get_def_map();
	if(ret != 0)
	{
		PRINT_ERR("CMS: fail to get image default map.");
		return -1;
	}
#endif
	for(i = 0; i < CMS_IMAGE_MAX_ITEM_MAP; i++)
	{
		if(imageArr[i].stringName != NULL)
		{
			if(strcmp("saturation", imageArr[i].stringName) == 0)
			{
				if(imageArr[i].defaultValue != NULL)
					color.nBrightness = atoi(imageArr[i].defaultValue);
			}
			else if(strcmp("brightness", imageArr[i].stringName) == 0)
			{
				if(imageArr[i].defaultValue != NULL)
					color.nSaturation = atoi(imageArr[i].defaultValue);
			}
			else if(strcmp("contrast", imageArr[i].stringName) == 0)
			{
				if(imageArr[i].defaultValue != NULL)
					color.nContrast = atoi(imageArr[i].defaultValue);
			}
			else if(strcmp("hue", imageArr[i].stringName) == 0)
			{
				if(imageArr[i].defaultValue != NULL)
					color.nHue = atoi(imageArr[i].defaultValue);
			}
			else if(strcmp("sharpness", imageArr[i].stringName) == 0)
			{
				if(imageArr[i].defaultValue != NULL)
					color.nDefinition = atoi(imageArr[i].defaultValue);
			}
		}
	}

    GkCmsCmdResq(sock, &color, sizeof(DMS_NET_CHANNEL_COLOR), DMS_NET_GET_DEF_COLORCFG);
    return 0;
}

int Gk_CmsGetEnhancedColorSupport(int sock)
{
	int i = 0;
    DMS_NET_ENHANCED_COLOR_SUPPORT enhanced_color_support;
    memset(&enhanced_color_support, 0, sizeof(DMS_NET_ENHANCED_COLOR_SUPPORT));
    enhanced_color_support.dwSize = sizeof(DMS_NET_ENHANCED_COLOR_SUPPORT);
    enhanced_color_support.dwMask = DMS_COLOR_SET_SCENE | DMS_COLOR_SET_ADNSWITCH |\
    	DMS_COLOR_SET_DN | DMS_COLOR_SET_BAW | DMS_COLOR_SET_EWD | DMS_COLOR_SET_WD | DMS_COLOR_SET_MIRROR;
    	/*DMS_COLOR_SET_AWBSWITCH | DMS_COLOR_SET_AECSWITCH | DMS_COLOR_SET_EC | DMS_COLOR_SET_AGCSWITCH |*/
    	/*DMS_COLOR_SET_GC |*/
    	/*DMS_COLOR_SET_AIRIS | DMS_COLOR_SET_BLC*/
	//Unsupported
	for(i = 0; i < CMS_IMAGE_MAX_ITEM_MAP; i++)
	{
		if(imageArr[i].stringName != NULL)
		{
			if(strcmp("strengthDenoise3d", imageArr[i].stringName) == 0)
			{
				//printf("strDenoise  min: %d  max: %d\n", imageArr[i].min, imageArr[i].max);
				enhanced_color_support.strDenoise.nMin = imageArr[i].min;
				enhanced_color_support.strDenoise.nMax = imageArr[i].max;
			}	
			else if(strcmp("strengthWDR", imageArr[i].stringName) == 0)
			{
				enhanced_color_support.strWD.nMin = imageArr[i].min;
				enhanced_color_support.strWD.nMax = imageArr[i].max;
			}
		}
	}

	enhanced_color_support.strIrisBasic.nMin = 0;
	enhanced_color_support.strIrisBasic.nMax = 100;
	enhanced_color_support.strRed.nMin		 = 0;
	enhanced_color_support.strRed.nMax		 = 100;
	enhanced_color_support.strBlue.nMin		 = 0;
	enhanced_color_support.strBlue.nMax		 = 100;
	enhanced_color_support.strGreen.nMin	 = 0;
	enhanced_color_support.strGreen.nMax	 = 100;	
	enhanced_color_support.strGamma.nMin 	 = 0;
	enhanced_color_support.strGamma.nMax	 = 100;
	enhanced_color_support.strEC.nMin		 = 0;
	enhanced_color_support.strEC.nMax		 = 100;
	enhanced_color_support.strGC.nMin		 = 0;
	enhanced_color_support.strGC.nMax		 = 100;
	
    GkCmsCmdResq(sock, &enhanced_color_support, sizeof(DMS_NET_ENHANCED_COLOR_SUPPORT), DMS_NET_GET_ENHANCED_COLOR_SUPPORT);
    return 0;
}

int Gk_CmsGetEnhancedColor(int sock)
{
	int ret = 0;
    DMS_NET_CHANNEL_ENHANCED_COLOR enhanced_color;
	memset(&enhanced_color, 0, sizeof(DMS_NET_CHANNEL_ENHANCED_COLOR));
    GK_NET_IMAGE_CFG imagEnhancedAttr;
    memset(&imagEnhancedAttr, 0, sizeof(GK_NET_IMAGE_CFG));
	ret = netcam_image_get(&imagEnhancedAttr);
    if(ret != 0)
	{
		PRINT_ERR("CMS: fail to get image attr.\n");
		return -1;
	}
    enhanced_color.dwSize = sizeof(DMS_NET_CHANNEL_ENHANCED_COLOR);
    enhanced_color.bEnableAutoDenoise = imagEnhancedAttr.enableDenoise3d;    
	enhanced_color.nDenoise 		  = imagEnhancedAttr.strengthDenoise3d;
    enhanced_color.bWideDynamic		  = imagEnhancedAttr.enabledWDR;
   	enhanced_color.nWDLevel			  = imagEnhancedAttr.strengthWDR;
   	
	if(imagEnhancedAttr.sceneMode == 0)
		enhanced_color.nSceneMode = 3;
	else if(imagEnhancedAttr.sceneMode == 1)
		enhanced_color.nSceneMode = 1;
	else if(imagEnhancedAttr.sceneMode == 2)//outdoor and corridor
		enhanced_color.nSceneMode = 0;	 //show outdoor
		
    if(imagEnhancedAttr.flipEnabled == 1 && imagEnhancedAttr.mirrorEnabled == 1)
    	enhanced_color.nMirror = 3;
	else if(imagEnhancedAttr.flipEnabled == 0 && imagEnhancedAttr.mirrorEnabled == 1)
    	enhanced_color.nMirror = 2;
	else if(imagEnhancedAttr.flipEnabled == 1 && imagEnhancedAttr.mirrorEnabled == 0)
    	enhanced_color.nMirror = 1;
	else if(imagEnhancedAttr.flipEnabled == 0 && imagEnhancedAttr.mirrorEnabled == 0)
    	enhanced_color.nMirror = 0;
    	
	if(imagEnhancedAttr.irCutMode == 2)
    	enhanced_color.bEnableBAW = 1;
	else if(imagEnhancedAttr.irCutMode == 1)
    	enhanced_color.bEnableBAW = 2;
	else if(imagEnhancedAttr.irCutMode == 0)
    	enhanced_color.bEnableBAW = 0;

    GkCmsCmdResq(sock, &enhanced_color, sizeof(DMS_NET_CHANNEL_ENHANCED_COLOR), DMS_NET_GET_ENHANCED_COLOR);
    return 0;
}

int Gk_CmsGetDayNightDetection(int sock)
{
    DMS_NET_DAY_NIGHT_DETECTION_EX detection;
	memset(&detection, 0, sizeof(DMS_NET_DAY_NIGHT_DETECTION_EX));
	
    detection.dwSize = sizeof(DMS_NET_DAY_NIGHT_DETECTION_EX);
	detection.byMode 		   = 1;
	detection.byTrigger 	   = 0;
	detection.byAGCSensitivity = 3;
	detection.byDelay		   = 5;
	detection.byIRCutLevel	   = 0;
	detection.byLedLevel	   = 0;
	detection.stColorTime.byStartHour = 7;
	detection.stColorTime.byStartMin  =	0;
	detection.stColorTime.byStopHour  =	18;
	detection.stColorTime.byStopMin   = 0;

    GkCmsCmdResq(sock, &detection, sizeof(DMS_NET_DAY_NIGHT_DETECTION_EX), DMS_NET_GET_COLOR_BLACK_DETECTION);
    return 0;
}

int Gk_CmsSetDayNightDetection(int sock)
{
	int ret = 0;
    DMS_NET_DAY_NIGHT_DETECTION_EX det;
    memset(&det, 0, sizeof(DMS_NET_DAY_NIGHT_DETECTION_EX));
    ret = GkSockRecv(sock, (char *)&det, sizeof(DMS_NET_DAY_NIGHT_DETECTION_EX));
    if(ret != sizeof(DMS_NET_DAY_NIGHT_DETECTION_EX)) 
    {
        PRINT_ERR();
        return -1;
    }
	if(det.byMode != 1)
	{
		PRINT_ERR("CMS: invalid day/night detection mode.");
		return -1;
	}

	printf("dwSize: %d\ndwChannel: %d\nbyMode: %d\nbyTrigger: %d\n"
		   "byAGCSensitivity: %d\nbyDelay:%d\nbyIRCutLevel:%d\nbyLedLevel: %d\n"
		   "stColorTime: \nbyStartHour:%d\nbyStartMin: %d\nbyStopHour:%d\nbyStopMin:%d\n",
		(int)det.dwSize, (int)det.dwChannel, (int)det.byMode, (int)det.byTrigger,
		(int)det.byAGCSensitivity, (int)det.byDelay, (int)det.byIRCutLevel, (int)det.byLedLevel,
		(int)det.stColorTime.byStartHour, (int)det.stColorTime.byStartMin,
		(int)det.stColorTime.byStopHour, (int)det.stColorTime.byStopMin);
    return 0;
}

int Gk_CmsSetColorSingle(int sock)
{
	int ret = 0;
    DMS_NET_CHANNEL_COLOR_SINGLE color;
    ret = GkSockRecv(sock, (char *)&color, sizeof(DMS_NET_CHANNEL_COLOR_SINGLE));
    if(ret != sizeof(DMS_NET_CHANNEL_COLOR_SINGLE))
    {
        PRINT_ERR();
        return -1;
    }
	PRINT_INFO("cmd:0x%x, value:%d, saveFlag: 0x%x\n", (int)color.dwSetFlag, color.nValue, (int)color.dwSaveFlag);
	
	if(color.dwSaveFlag == DMS_COLOR_SET_DEF || color.dwSetFlag != 0)
	{
		int defval = 0;
		GK_NET_IMAGE_CFG stImagingConfig;
		memset(&stImagingConfig, 0, sizeof(GK_NET_IMAGE_CFG));
		ret = netcam_image_get(&stImagingConfig);
		if(ret != 0)
		{
			PRINT_ERR("CMS:fail to get imaging parameters.\n");
			return -1;
		}
		if(color.dwSaveFlag == DMS_COLOR_SET_DEF)
		{
			PRINT_INFO("CMS: Revert to default parameters.");
			cms_image_get_def_val("brightness", &defval);
			stImagingConfig.brightness = defval;
			cms_image_get_def_val("hue", &defval);
			stImagingConfig.hue = defval;
			cms_image_get_def_val("saturation", &defval);
			stImagingConfig.saturation = defval;
			cms_image_get_def_val("contrast", &defval);
			stImagingConfig.contrast = defval;
			cms_image_get_def_val("sharpness", &defval);
			stImagingConfig.sharpness = defval;
			cms_image_get_def_val("enableDenoise3d", &defval);
			stImagingConfig.enableDenoise3d = defval;
			cms_image_get_def_val("strengthDenoise3d", &defval);
			stImagingConfig.strengthDenoise3d = defval;
			cms_image_get_def_val("enabledWDR", &defval);
			stImagingConfig.enabledWDR = defval;
			cms_image_get_def_val("strengthWDR", &defval);
			stImagingConfig.strengthWDR = defval;
			cms_image_get_def_val("sceneMode", &defval);
			stImagingConfig.sceneMode = defval;
			cms_image_get_def_val("flipEnabled", &defval);
			stImagingConfig.flipEnabled = defval;
			cms_image_get_def_val("mirrorEnabled", &defval);
			stImagingConfig.mirrorEnabled = defval;
			cms_image_get_def_val("irCutMode", &defval);
			stImagingConfig.irCutMode = defval;
		}
		else if(color.dwSetFlag != 0)
		{
			PRINT_INFO("CMS: Set new parameter.");
			switch(color.dwSetFlag)
			{
				case DMS_COLOR_SET_BRIGHTNESS:
					stImagingConfig.brightness = color.nValue;
					break;
				case DMS_COLOR_SET_HUE:
					stImagingConfig.hue = color.nValue;
					break;
				case DMS_COLOR_SET_SATURATION:
					stImagingConfig.saturation = color.nValue;
					break;
				case DMS_COLOR_SET_CONTRAST:
					stImagingConfig.contrast = color.nValue;
					break;
				case DMS_COLOR_SET_DEFINITION:
					stImagingConfig.sharpness = color.nValue;
					break;
				case DMS_COLOR_SET_ADNSWITCH:
					stImagingConfig.enableDenoise3d = color.nValue;
					break;
				case DMS_COLOR_SET_DN:
					stImagingConfig.strengthDenoise3d = color.nValue;
					break;
				case DMS_COLOR_SET_EWD:
					stImagingConfig.enabledWDR = color.nValue;
					break;
				case DMS_COLOR_SET_WD:
					stImagingConfig.strengthWDR = color.nValue;
					break;
				case DMS_COLOR_SET_SCENE:
					if(color.nValue == 3)//»§Íâ¡£
						stImagingConfig.sceneMode = 0;
					else if(color.nValue == 1)
						stImagingConfig.sceneMode = 1;
					else
						stImagingConfig.sceneMode = 2;
					break;
				case DMS_COLOR_SET_MIRROR:
					if(color.nValue == 0)
					{			
						stImagingConfig.flipEnabled	  = 0;
						stImagingConfig.mirrorEnabled = 0;
					}
					else if(color.nValue == 1)
					{
						stImagingConfig.flipEnabled	  = 1;
						stImagingConfig.mirrorEnabled = 0;
					}
					else if(color.nValue == 2)
					{
						stImagingConfig.flipEnabled   = 0;
						stImagingConfig.mirrorEnabled = 1;
					}
					else if(color.nValue == 3)
					{
						stImagingConfig.flipEnabled   = 1;
						stImagingConfig.mirrorEnabled = 1;
					}
					break;
				case DMS_COLOR_SET_BAW:
					if(color.nValue == 2)
						stImagingConfig.irCutMode = 1;
					else if(color.nValue == 1)
						stImagingConfig.irCutMode = 2;
					else if(color.nValue == 0)
						stImagingConfig.irCutMode = 0;
					break;
				default:
					PRINT_INFO("CMS: Unsupported image attr.\n");
					break;
			}
		}
		ret = netcam_image_set(stImagingConfig);
		if(ret != 0)
		{
			PRINT_ERR("CMS: fail to set image attr.\n");
			return -1;
		}
	}
    if(color.dwSaveFlag == DMS_COLOR_SAVE)
	{
		netcam_image_cfg_save();
	}
	
    return 0;
}

