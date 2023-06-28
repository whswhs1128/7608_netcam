
#include "cfg_com.h"
#include "media_fifo.h"
#include "common.h"
#include "sdk_isp.h"
#include "ot_common_isp.h"
#include "ss_mpi_isp.h"


pthread_rwlock_t image_lock;

static inline int image_enter_lock(void)
{
	return pthread_rwlock_wrlock(&image_lock);
}

static int image_leave_lock(void)
{
	return pthread_rwlock_unlock(&image_lock);
}

static void image_set_all(void)
{
    sdk_isp_changed();
    sdk_isp_set_wb_mode(runImageCfg.wbMode);
    sdk_isp_set_vi_flicker(runImageCfg.antiFlickerFreq);
    sdk_isp_set_scene_mode(runImageCfg.sceneMode);
    sdk_isp_set_color_mode(runImageCfg.imageStyle);
    sdk_isp_set_contrast(runImageCfg.contrast);
    sdk_isp_set_brightness(runImageCfg.brightness);
    sdk_isp_set_saturation(runImageCfg.saturation);
    sdk_isp_set_sharpen(runImageCfg.sharpness, 1);
    sdk_isp_set_hue(runImageCfg.hue);

    ot_isp_csc_attr csc_attr;
    ot_isp_sharpen_attr shp_attr;
    ss_mpi_isp_get_csc_attr(0, &csc_attr);
    ss_mpi_isp_get_sharpen_attr(0, &shp_attr);
    csc_attr.contr = runImageCfg.contrast;
    csc_attr.luma = runImageCfg.brightness;
    csc_attr.satu = runImageCfg.saturation;
    csc_attr.hue = runImageCfg.hue;
    shp_attr.op_type = OT_OP_MODE_MANUAL;
        shp_attr.manual_attr.detail_ctrl = runImageCfg.sharpness;
        ss_mpi_isp_set_sharpen_attr(0, &shp_attr);
        ss_mpi_isp_set_csc_attr(0, &csc_attr);
    #if 0
    if(runImageCfg.enabledWDR== 1)
    {
        ispApi->WDR_MODE_ENABLE((SDK_U8)runImageCfg.enabledWDR);
        ispApi->WDR_STRENGTH_SET((SDK_U8)runImageCfg.strengthWDR);
    }
    else
    {
        ispApi->WDR_MODE_ENABLE((SDK_U8)runImageCfg.enabledWDR);
    }

    if(runImageCfg.enableDenoise3d== 1)
    {

        ispApi->WDR_MODE_ENABLE((SDK_U8)runImageCfg.enableDenoise3d);
        ispApi->DENOISE_STRENGTH_SET((SDK_U8)runImageCfg.enableDenoise3d);
    }
    else
    {
        ispApi->WDR_MODE_ENABLE((SDK_U8)runImageCfg.enableDenoise3d);
    }
    #endif
    sdk_isp_set_lowlight_mode(runImageCfg.lowlightMode);

    //sdk_isp_set_mirro_flip(runImageCfg.mirrorEnabled, runImageCfg.flipEnabled);	xqq

	//sdk_isp_set_ircut_control_mode(runImageCfg.irCutControlMode);	xqq
//	sdk_isp_set_ircut_mode(runImageCfg.irCutMode);	xqq
	sdk_isp_set_bulb_mode(runImageCfg.lightMode);
	sdk_isp_set_bulb_level(runImageCfg.lightLevel);
}

void netcam_image_init(void)
{
    pthread_rwlock_init(&image_lock, NULL);
    ImageCfgLoadDefValue();
    sdk_isp_set_ImageDefParam(runImageCfg.saturation, runImageCfg.brightness, runImageCfg.hue, runImageCfg.contrast, runImageCfg.sharpness);
    ImageCfgLoad();
    image_set_all();
    //and so on
}

void netcam_iamge_ircut_auto_check(void)
{
	image_enter_lock();
    sdk_isp_ircut_auto_switch();
	image_leave_lock();
}

int netcam_image_get_map(GK_CFG_MAP *imageArr, int mapSize)
{
	int i = 0;
	int num = 0;
	if(imageArr == NULL)
	{
		PRINT_ERR("Invalid paramters.\n");
		return -1;
	}

    image_enter_lock();
	while(imageMap[i].stringName != NULL)i++;

	if(mapSize > i)
		num = i;
	else
		num = mapSize;
	memcpy(imageArr, imageMap, num*sizeof(GK_CFG_MAP));
    image_leave_lock();
	return 0;

}

int netcam_image_get(GK_NET_IMAGE_CFG *imagAttr)
{
    int ret = 0;
    image_enter_lock();
    memcpy(imagAttr,&runImageCfg,sizeof(GK_NET_IMAGE_CFG));
    image_leave_lock();
    return ret;
}

int netcam_image_set(GK_NET_IMAGE_CFG imagAttr)
{
    image_enter_lock();
    sdk_isp_changed();
    ot_isp_csc_attr csc_attr;
    ot_isp_sharpen_attr shp_attr;
    ss_mpi_isp_get_csc_attr(0, &csc_attr);
    ss_mpi_isp_get_sharpen_attr(0, &shp_attr);

    if(imagAttr.contrast != runImageCfg.contrast)
    {
       csc_attr.contr = imagAttr.contrast;
       ss_mpi_isp_set_csc_attr(0, &csc_attr);

       sdk_isp_set_contrast(imagAttr.contrast);
    }
    if(imagAttr.brightness != runImageCfg.brightness)
    {
        sdk_isp_set_brightness(imagAttr.brightness);
        csc_attr.luma = imagAttr.brightness;
    }
       

    if(imagAttr.saturation != runImageCfg.saturation)
    {
        sdk_isp_set_saturation(imagAttr.saturation);
        csc_attr.satu = imagAttr.saturation;
        ss_mpi_isp_set_csc_attr(0, &csc_attr);
    }
        

    if(imagAttr.sharpness != runImageCfg.sharpness)
    {
        shp_attr.op_type = OT_OP_MODE_MANUAL;
        shp_attr.manual_attr.detail_ctrl = imagAttr.sharpness;
        ss_mpi_isp_set_sharpen_attr(0, &shp_attr);
        sdk_isp_set_sharpen(imagAttr.sharpness, 1);
    }
        


    if(imagAttr.hue != runImageCfg.hue)
    {
        sdk_isp_set_hue(imagAttr.hue);
        csc_attr.hue = imagAttr.hue;
    }
        

    if(imagAttr.wbMode != runImageCfg.wbMode)
        sdk_isp_set_wb_mode(imagAttr.wbMode);

    if(imagAttr.irCutControlMode != runImageCfg.irCutControlMode)
        sdk_isp_set_ircut_control_mode(imagAttr.irCutControlMode);

    if(imagAttr.irCutMode != runImageCfg.irCutMode)
    {
        sdk_isp_set_ircut_mode(imagAttr.irCutMode);
        imagAttr.lightMode = (imagAttr.irCutMode==3)?ISP_BULB_MODE_ALL_COLOR:ISP_BULB_MODE_IR;
    }
    if(imagAttr.antiFlickerFreq != runImageCfg.antiFlickerFreq)
        sdk_isp_set_vi_flicker(imagAttr.antiFlickerFreq);

    if(imagAttr.sceneMode != runImageCfg.sceneMode)
        sdk_isp_set_scene_mode(imagAttr.sceneMode);

    if(imagAttr.imageStyle != runImageCfg.imageStyle)
        sdk_isp_set_color_mode(imagAttr.imageStyle);

    #if 0
    if(imagAttr.enabledWDR == 1)
    {
        if(runImageCfg.enabledWDR == 0)
        {
            ispApi->WDR_MODE_ENABLE((SDK_U8)imagAttr.enabledWDR);
            ispApi->WDR_STRENGTH_SET((SDK_U8)imagAttr.strengthWDR);

        }
        else if(imagAttr.strengthWDR != runImageCfg.strengthWDR)
        {
            ispApi->WDR_STRENGTH_SET((SDK_U8)imagAttr.strengthWDR);
        }
    }
    else
    {
        if(runImageCfg.enabledWDR == 1)
        {
            ispApi->WDR_MODE_ENABLE((SDK_U8)imagAttr.enabledWDR);
        }
    }

    if(imagAttr.enableDenoise3d== 1)
    {
        if(runImageCfg.enableDenoise3d == 0)
        {
            ispApi->DENOISE_ENABLE((SDK_U8)imagAttr.enableDenoise3d);
            ispApi->DENOISE_STRENGTH_SET((SDK_U8)imagAttr.strengthDenoise3d);
        }
        else if(imagAttr.strengthDenoise3d != runImageCfg.strengthDenoise3d)
        {
            ispApi->DENOISE_STRENGTH_SET((SDK_U8)imagAttr.strengthDenoise3d);
        }
    }
    else
    {
        if(runImageCfg.enableDenoise3d == 1)
        {
            ispApi->DENOISE_ENABLE((SDK_U8)imagAttr.enableDenoise3d);
        }
    }

    if(imagAttr.lowlightMode!= runImageCfg.lowlightMode)
        ispApi->LIGHT_MODE_SET((SDK_U32)imagAttr.lowlightMode);
    #endif


    if(imagAttr.flipEnabled != runImageCfg.flipEnabled || imagAttr.mirrorEnabled != runImageCfg.mirrorEnabled)
    {
        //sdk_isp_set_mirro_flip(imagAttr.mirrorEnabled, imagAttr.flipEnabled);		xqq
	sample_comm_isp_mirror_flip_set(0,imagAttr.mirrorEnabled, imagAttr.flipEnabled);
    }
    // csc_attr.en = 1;
    ss_mpi_isp_set_csc_attr(0, &csc_attr);
    memcpy(&runImageCfg,&imagAttr,sizeof(imagAttr));
    image_leave_lock();
    return 0;
}

// flag=0, active but not save
// flag=1, active and save
// flag=2, restore last config
int netcam_image_set2(GK_NET_IMAGE_CFG imagAttr, int flag)
{
    image_enter_lock();
    sdk_isp_changed();

    if(imagAttr.contrast != runImageCfg.contrast)
       sdk_isp_set_contrast(imagAttr.contrast);

    if(imagAttr.brightness != runImageCfg.brightness)
       sdk_isp_set_brightness(imagAttr.brightness);

    if(imagAttr.saturation != runImageCfg.saturation)
        sdk_isp_set_saturation(imagAttr.saturation);

    if(imagAttr.sharpness != runImageCfg.sharpness)
        sdk_isp_set_sharpen(imagAttr.sharpness, 1);


    if(imagAttr.hue != runImageCfg.hue)
        sdk_isp_set_hue(imagAttr.hue);

    if(imagAttr.wbMode != runImageCfg.wbMode)
        sdk_isp_set_wb_mode(imagAttr.wbMode);

    if(imagAttr.irCutControlMode != runImageCfg.irCutControlMode)
        sdk_isp_set_ircut_control_mode(imagAttr.irCutControlMode);

    if(imagAttr.irCutMode != runImageCfg.irCutMode)
    {
        sdk_isp_set_ircut_mode(imagAttr.irCutMode);
    }
    if(imagAttr.antiFlickerFreq != runImageCfg.antiFlickerFreq)
        sdk_isp_set_vi_flicker(imagAttr.antiFlickerFreq);

    if(imagAttr.sceneMode != runImageCfg.sceneMode)
        sdk_isp_set_scene_mode(imagAttr.sceneMode);

    if(imagAttr.imageStyle != runImageCfg.imageStyle)
        sdk_isp_set_color_mode(imagAttr.imageStyle);

    if(imagAttr.flipEnabled != runImageCfg.flipEnabled || imagAttr.mirrorEnabled != runImageCfg.mirrorEnabled)
    {
        //sdk_isp_set_mirro_flip(imagAttr.mirrorEnabled, imagAttr.flipEnabled);   xqq
    }

    memcpy(&runImageCfg,&imagAttr,sizeof(imagAttr));
    if (flag == 1)
    {
        ImageCfgSave();
    }

    image_leave_lock();

    return 0;
}



void netcam_image_cfg_save(void)
{
	if(0 == image_enter_lock())
    {
		PRINT_INFO("save cfg!!");
		ImageCfgSave();
		image_leave_lock();
	}
}

void netcam_image_reset_default(void)
{
    if(0 == image_enter_lock())
    {
		PRINT_INFO("reset!!");
		ImageCfgLoadDefValue();
        image_set_all();
		image_leave_lock();
	}
}

