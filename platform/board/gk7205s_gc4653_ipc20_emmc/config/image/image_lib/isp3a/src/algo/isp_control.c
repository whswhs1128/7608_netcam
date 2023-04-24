/*!
*****************************************************************************
** \file        src/algo/gk7101_aaa_img_main.c
**
** \brief
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2015 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <string.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <stdlib.h>

#include "basetypes.h"
#include "gk_isp.h"
#include "gk_isp3a_api.h"
#include "gk_drv.h"
#include "gk_vi.h"
#include "gk_media.h"
#include "isp3a_awb.h"
#include "isp3a_ae.h"
#include "isp3a_lens.h"
#include "isp3a_af.h"
#include "isp3a_ctl.h"
#include "isp3a_hw.h"
#include "isp3a_proc.h"
#include "isp3a_tone.h"

//*****************************************************************************
//*****************************************************************************
//** Local Defines
//*****************************************************************************
//*****************************************************************************

//*****************************************************************************
//*****************************************************************************
//** Local structures
//*****************************************************************************
//*****************************************************************************

//*****************************************************************************
//*****************************************************************************
//** Global Data
//*****************************************************************************
//*****************************************************************************

extern af_param_t imx035_TamronDF300_param;
extern af_param_t MF_af_param;

//extern af_param_t ov5653_SY6310_param;
extern af_param_t ov5650_SY3510A_param;
extern af_param_t iu072f_af_param;
extern af_param_t Foctek_D14_02812IR_af_param;
extern af_param_t YuTong_YT30031FB_af_param;
extern af_param_t YuTong_YT30021FB_af_param;
extern af_param_t YS05IR_F1F18_af_param;


extern zoom_t TAMRON_DF300_DISTANCE_MAP[];
extern zoom_t MF_DISTANCE_MAP[];

//extern zoom_t SY6310_DISTANCE_MAP[];
extern zoom_t SY3510A_DISTANCE_MAP[];
extern zoom_t iu072f_DISTANCE_MAP[];
extern zoom_t Foctek_D14_02812IR_DISTANCE_MAP[];
extern zoom_t YuTong_YT30031FB_DISTANCE_MAP[];
extern zoom_t YuTong_YT30021FB_DISTANCE_MAP[];
extern zoom_t YS05IR_F1F18_DISTANCE_MAP[];


extern lens_dev_drv_t TamronDF300_dev_drv;
extern lens_dev_drv_t MF_dev_drv;

//extern lens_dev_drv_t SY6310_dev_drv;
//extern lens_dev_drv_t SY3510A_dev_drv;
extern lens_dev_drv_t SY3510A_AD5839_dev_drv;
extern lens_dev_drv_t iu072f_dev_drv;
//static u32 img_vi_cnt = 0;

extern lens_dev_drv_t Foctek_D14_02812IR_dev_drv;
extern lens_dev_drv_t YuTong_YT30031FB_dev_drv;
extern lens_dev_drv_t YuTong_YT30021FB_dev_drv;
extern lens_dev_drv_t YS05IR_F1F18_dev_drv;
extern int gpio_id [];

//*****************************************************************************
//*****************************************************************************
//** Local Data
//*****************************************************************************
//*****************************************************************************

//*****************************************************************************
//*****************************************************************************
//** Local Functions Declaration
//*****************************************************************************
//*****************************************************************************
extern int ap_listen_socket();
extern int ap_receive_msg(int fd, int *msg);
extern u32 ae_get_luma_val(Isp3AProcessor *processor, ae_data_t * p_tile_info, u16 tile_num);
//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************
typedef struct
{
    int id;
    Isp3AProcessor *handle;
} MonitorHandleListT;
#define  MONITOR_HANDLE_LIST_MAX    2 /*only accept two ISP handle*/
static MonitorHandleListT monitorHandleList[MONITOR_HANDLE_LIST_MAX] = {{-1, NULL},{-1, NULL}};

#define for_each_handle_processer(handle) \
        for(handle = &(monitorHandleList[0]); \
            handle <  &(monitorHandleList[MONITOR_HANDLE_LIST_MAX]); handle++) \
        if (handle->handle != NULL)

int monitor_handle_register(Isp3AProcessor *processor)
{
    int i;
    if (processor == NULL)
        return -1;

    for (i = 0; i < MONITOR_HANDLE_LIST_MAX; i++)
    {
        if (monitorHandleList[i].handle == NULL)
        {
            monitorHandleList[i].id = processor->srcId;
            monitorHandleList[i].handle = processor;
            return 0;
        }
    }
    return -1;
}

int monitor_handle_unregister(Isp3AProcessor *processor)
{
    int i;
    if (processor == NULL)
        return -1;

    for (i = 0; i < MONITOR_HANDLE_LIST_MAX; i++)
    {
        if (monitorHandleList[i].handle == processor)
        {
            monitorHandleList[i].id = 0;
            monitorHandleList[i].handle = NULL;
            return 0;
        }
    }
    return -1;
}

Isp3AProcessor *monitor_handle_find_by_id(int id)
{
    int i;
    if (id < 0)
        return NULL;

    for (i = 0; i < MONITOR_HANDLE_LIST_MAX; i++)
    {
        if (monitorHandleList[i].id == id)
        {
            return monitorHandleList[i].handle;
        }
    }

    return NULL;
}

int ic_config_sensor_hdr_mode(Isp3AProcessor *processor, int hdr_mode)
{
    ae_sensor_hdr_info(processor, hdr_mode);
    awb_sensor_hdr_info(processor, hdr_mode);
    return 0;
}

int ic_config_sensor_param(Isp3AProcessor *processor)
{
    GK_VI_SRC_SensorInfoT sensor_info;
    u32 chipId;
    Isp3APublicContent *publicData;
    ImageOperate    *op;

    publicData = &(processor->content.publicData);
    op = &(processor->operate);

    if (hal_get_sensor_info(processor, &sensor_info) < 0) {
        op->printf("error: hal_get_sensor_info\n");
        return -1;
    }
    publicData->sensor_pattern = sensor_info.sensor_pattern;
    publicData->sensor_max_db = sensor_info.sensor_max_db;
    publicData->sensor_double_step = sensor_info.sensor_double_step;
    publicData->typeofsensor = sensor_info.typeofsensor;
    publicData->sync_time_delay = sensor_info.sync_time_delay;
    // printf("hal_get_sensor_info %d %d %d %d %d\n",sensor_pattern, sensor_max_db,sensor_double_step,typeofsensor,sync_time_delay);

    if (hal_get_chip_id(processor, &chipId) < 0){
        op->printf("error: hal_get_chip_id\n");
        return -1;
    }
    switch(chipId){
        case GK_CHIP_ID_GK7101S:
            publicData->isp_capability.chip_id          = ISPCHIP_GK7101S;
            publicData->isp_capability.vps_enable       = 1;
            publicData->isp_capability.contrast_enable  = 1;
            break;
        case GK_CHIP_ID_GK7102S:
            publicData->isp_capability.chip_id          = ISPCHIP_GK7102S;
            publicData->isp_capability.vps_enable       = 1;
            publicData->isp_capability.contrast_enable  = 1;
            break;
        case GK_CHIP_ID_GK7102C:
            publicData->isp_capability.chip_id          = ISPCHIP_GK7102C;
            publicData->isp_capability.vps_enable       = 1;
            publicData->isp_capability.contrast_enable  = 1;
            break;
        case GK_CHIP_ID_GK7202:
            publicData->isp_capability.chip_id          = ISPCHIP_GK7202;
            publicData->isp_capability.vps_enable       = 0;
            publicData->isp_capability.contrast_enable  = 0;
            break;
        case GK_CHIP_ID_GK7101:
            publicData->isp_capability.chip_id          = ISPCHIP_GK7101;
            publicData->isp_capability.vps_enable       = 0;
            publicData->isp_capability.contrast_enable  = 0;
            break;
        case GK_CHIP_ID_GK7102:
            publicData->isp_capability.chip_id          = ISPCHIP_GK7102;
            publicData->isp_capability.vps_enable       = 0;
            publicData->isp_capability.contrast_enable  = 0;
            break;
        default:
            publicData->isp_capability.chip_id          = ISPCHIP_GK7101;
            publicData->isp_capability.vps_enable       = 0;
            publicData->isp_capability.contrast_enable  = 0;
            break;
    }

    return 0;
}

int config_isp_capability(Isp3AProcessor *processor, int mode)
{
    Isp3APublicContent *publicData;
    ImageOperate    *op;
    u32 chipId;

    publicData = &(processor->content.publicData);
    op = &(processor->operate);

    switch(mode){
        case GK_VPS_MERGE_MODE:
            publicData->isp_capability.vps_enable      = 1;
            publicData->isp_capability.contrast_enable = 1;
            break;

        case GK_VPS_SPLIT_MODE:
            publicData->isp_capability.vps_enable      = 1;
            publicData->isp_capability.contrast_enable = 0;
            break;

        case GK_DNR3DCONSTRACT_MODE:
            publicData->isp_capability.vps_enable      = 0;
            publicData->isp_capability.contrast_enable = 1;
            break;
        case GK_DNR3D_MODE:
        default:
            publicData->isp_capability.vps_enable      = 0;
            publicData->isp_capability.contrast_enable = 0;
            break;
    }


    if (hal_get_chip_id(processor, &chipId) < 0){
        op->printf("error: hal_get_chip_id\n");
        return -1;
    }

    switch(chipId){
        case GK_CHIP_ID_GK7101S:
            publicData->isp_capability.chip_id          = ISPCHIP_GK7101S;
            publicData->isp_capability.vps_enable       &= 1;
            publicData->isp_capability.contrast_enable  &= 1;
            break;
        case GK_CHIP_ID_GK7102S:
            publicData->isp_capability.chip_id          = ISPCHIP_GK7102S;
            publicData->isp_capability.vps_enable       &= 1;
            publicData->isp_capability.contrast_enable  &= 1;
            break;
        case GK_CHIP_ID_GK7102C:
            publicData->isp_capability.chip_id          = ISPCHIP_GK7102C;
            publicData->isp_capability.vps_enable       &= 1;
            publicData->isp_capability.contrast_enable  &= 1;
            break;
        case GK_CHIP_ID_GK7202:
            publicData->isp_capability.chip_id          = ISPCHIP_GK7202;
            publicData->isp_capability.vps_enable       &= 0;
            publicData->isp_capability.contrast_enable  &= 0;
            break;
        case GK_CHIP_ID_GK7101:
            publicData->isp_capability.chip_id          = ISPCHIP_GK7101;
            publicData->isp_capability.vps_enable       &= 0;
            publicData->isp_capability.contrast_enable  &= 0;
            break;
        case GK_CHIP_ID_GK7102:
            publicData->isp_capability.chip_id          = ISPCHIP_GK7102;
            publicData->isp_capability.vps_enable       &= 0;
            publicData->isp_capability.contrast_enable  &= 0;
            break;
        default:
            publicData->isp_capability.chip_id          = ISPCHIP_GK7101;
            publicData->isp_capability.vps_enable       &= 0;
            publicData->isp_capability.contrast_enable  &= 0;
            break;
    }

    return 0;

}

int isp3a_proc_get_capability(Isp3AProcessor *processor, img_isp_capability_t *capability)
{
    Isp3APublicContent *publicData;

    publicData = &(processor->content.publicData);

    if(capability == NULL || processor == NULL)
    {
        return -1;
    }

    *capability = publicData->isp_capability;

    return 0;
}


int ic_param_load_isp(Isp3AProcessor *processor, image_sensor_param_t * p_app_param_image_sensor)
{
    ImageOperate *op;
    Isp3APublicContent *publicData;

    op = &(processor->operate);
    publicData = &(processor->content.publicData);
    //printf("%x %x %x %x %x %x %x\n",p_app_param_image_sensor->p_chroma_scale,p_app_param_image_sensor->p_awb_param,p_app_param_image_sensor->p_50hz_lines,p_app_param_image_sensor->p_60hz_lines,p_app_param_image_sensor->p_dlight_range,p_app_param_image_sensor->p_ae_agc_dgain,p_app_param_image_sensor->p_ae_sht_dgain);
    if ((p_app_param_image_sensor->p_adj_param == NULL) ||
        (p_app_param_image_sensor->p_rgb2yuv == NULL) ||
        (p_app_param_image_sensor->p_chroma_scale == NULL) ||
        (p_app_param_image_sensor->p_awb_param == NULL) ||
        (p_app_param_image_sensor->p_50hz_lines == NULL) ||
        (p_app_param_image_sensor->p_60hz_lines == NULL) ||
        (p_app_param_image_sensor->p_dlight_range == NULL) ||
        (p_app_param_image_sensor->p_ae_agc_dgain == NULL) ||
        (p_app_param_image_sensor->p_ae_sht_dgain == NULL)) {
        return -1;
    }
    op->memcpy(&(publicData->gk_apc_param), p_app_param_image_sensor->p_adj_param,
        sizeof(publicData->gk_apc_param));
    op->memcpy(publicData->gk_rgb2yuv, p_app_param_image_sensor->p_rgb2yuv,
        sizeof(publicData->gk_rgb2yuv));
    op->memcpy(&(publicData->gk_uv_scale), p_app_param_image_sensor->p_chroma_scale,
        sizeof(publicData->gk_uv_scale));
    op->memcpy(&(publicData->gk_lens_shading), p_app_param_image_sensor->p_lens_shading,
        sizeof(publicData->gk_lens_shading));
    op->memcpy(&(publicData->gk_warp_config), p_app_param_image_sensor->p_warp_config,
        sizeof(publicData->gk_warp_config));
    op->memcpy(&(publicData->gk_awb_param), p_app_param_image_sensor->p_awb_param,
        sizeof(publicData->gk_awb_param));
    op->memcpy(&(publicData->gk_50hz_lines), p_app_param_image_sensor->p_50hz_lines,
        sizeof(publicData->gk_50hz_lines));
    op->memcpy(&(publicData->gk_60hz_lines), p_app_param_image_sensor->p_60hz_lines,
        sizeof(publicData->gk_60hz_lines));
    op->memcpy(&(publicData->gk_tile_config), p_app_param_image_sensor->p_tile_config,
        sizeof(publicData->gk_tile_config));

    if (p_app_param_image_sensor->p_manual_LE != NULL)
        op->memcpy(publicData->is_manual_LE, p_app_param_image_sensor->p_manual_LE,
            sizeof(publicData->is_manual_LE));
    else
        op->memset(publicData->is_manual_LE, 0, sizeof(publicData->is_manual_LE));

    //gk_h_dlight_range = p_app_param_image_sensor->p_dlight_range;
    //gk_p_ae_gain_dgain = p_app_param_image_sensor->p_ae_agc_dgain;
    //gk_p_ae_sht_dgain = p_app_param_image_sensor->p_ae_sht_dgain;

    op->memcpy(&(publicData->gk_h_dlight_range), p_app_param_image_sensor->p_dlight_range,
        sizeof(publicData->gk_h_dlight_range));
    op->memcpy(publicData->gk_p_ae_gain_dgain, p_app_param_image_sensor->p_ae_agc_dgain,
        sizeof(publicData->gk_p_ae_gain_dgain));
    op->memcpy(&(publicData->gk_p_ae_sht_dgain), p_app_param_image_sensor->p_ae_sht_dgain,
        sizeof(publicData->gk_p_ae_sht_dgain));

    op->memcpy(&(publicData->gk_apc_param_ir), p_app_param_image_sensor->p_adj_param,
        sizeof(publicData->gk_apc_param));
    op->memcpy(&(publicData->gk_apc_param_ir.sharp_info.rmf),
        &(publicData->gk_apc_param.sharp_info.rmf_night),
        sizeof(publicData->gk_apc_param.sharp_info.rmf_night));
    op->memcpy(&(publicData->gk_apc_param_ir.sharp_info.spatial_filter),
        &(publicData->gk_apc_param.sharp_info.spatial_filter_night),
        sizeof(publicData->gk_apc_param.sharp_info.spatial_filter));
    op->memcpy(&(publicData->gk_apc_param_ir.sharp_info.coring),
        &(publicData->gk_apc_param.sharp_info.coring_nigth),
        sizeof(publicData->gk_apc_param.sharp_info.coring));
    op->memcpy(&(publicData->gk_apc_param_ir.def.low_cfa_table),
        &(publicData->gk_apc_param.def.low_cfa_table_night),
        sizeof(publicData->gk_apc_param.def.low_cfa_table));
    op->memcpy(&(publicData->gk_apc_param_ir.def.high_cfa_table),
        &(publicData->gk_apc_param.def.high_cfa_table_night),
        sizeof(publicData->gk_apc_param.def.high_cfa_table));

    op->memcpy(&(publicData->gk_apc_param_ir.awbae),
        &(publicData->gk_apc_param.awbae_night),
        sizeof(publicData->gk_apc_param.awbae_night));
    op->memcpy(&(publicData->gk_apc_param_ir.nlm_info),
            &(publicData->gk_apc_param.nlm_info_night),
            sizeof(publicData->gk_apc_param.nlm_info_night));
    op->memcpy(&(publicData->gk_apc_param_ir.lce_info),
            &(publicData->gk_apc_param.lce_info_night),
            sizeof(publicData->gk_apc_param.lce_info_night));
    op->memcpy(&(publicData->gk_apc_param_ir.blf_info),
            &(publicData->gk_apc_param.blf_info_night),
            sizeof(publicData->gk_apc_param.blf_info_night));
    op->memcpy(&(publicData->gk_apc_param_ir.gmd_info),
            &(publicData->gk_apc_param.gmd_info_night),
            sizeof(publicData->gk_apc_param.gmd_info_night));
    op->memcpy(&(publicData->gk_apc_param_ir.pmv_info),
            &(publicData->gk_apc_param.pmv_info_night),
            sizeof(publicData->gk_apc_param.pmv_info_night));
    op->memcpy(&(publicData->gk_apc_param_ir.gmf_info),
             &(publicData->gk_apc_param.gmf_info_night),
             sizeof(publicData->gk_apc_param.gmf_info_night));
    op->memcpy(&(publicData->gk_apc_param_ir.vgf_info),
             &(publicData->gk_apc_param.vgf_info_night),
             sizeof(publicData->gk_apc_param.vgf_info_night));

    op->memcpy(&(publicData->gk_apc_param_weak_move), p_app_param_image_sensor->p_adj_param,
        sizeof(publicData->gk_apc_param));
    op->memcpy(&(publicData->gk_apc_param_weak_move.sharp_info.rmf),
        &(publicData->gk_apc_param.sharp_info.rmf_weak_move),
        sizeof(publicData->gk_apc_param.sharp_info.rmf_night));
    op->memcpy(&(publicData->gk_apc_param_weak_move.sharp_info.spatial_filter),
        &(publicData->gk_apc_param.sharp_info.spatial_filter_weak_move),
        sizeof(publicData->gk_apc_param.sharp_info.spatial_filter));
     op->memcpy(&(publicData->gk_apc_param_weak_move.nlm_info),
            &(publicData->gk_apc_param.nlm_info_weak_move),
            sizeof(publicData->gk_apc_param.nlm_info_night));
    op->memcpy(&(publicData->gk_apc_param_weak_move.blf_info),
            &(publicData->gk_apc_param.blf_info_weak_move),
            sizeof(publicData->gk_apc_param.blf_info_night));
    op->memcpy(&(publicData->gk_apc_param_weak_move.def.low_cfa_table),
        &(publicData->gk_apc_param.def.low_cfa_table_weak_move),
        sizeof(publicData->gk_apc_param.def.low_cfa_table));
    op->memcpy(&(publicData->gk_apc_param_weak_move.def.high_cfa_table),
        &(publicData->gk_apc_param.def.high_cfa_table_weak_move),
        sizeof(publicData->gk_apc_param.def.high_cfa_table));

    op->memcpy(&(publicData->gk_apc_param_interp), p_app_param_image_sensor->p_adj_param,
        sizeof(publicData->gk_apc_param));
    op->memcpy(&(publicData->gk_apc_param_interp.sharp_info.rmf),
        &(publicData->gk_apc_param.sharp_info.rmf_weak_move),
        sizeof(publicData->gk_apc_param.sharp_info.rmf_night));
    op->memcpy(&(publicData->gk_apc_param_interp.sharp_info.spatial_filter),
        &(publicData->gk_apc_param.sharp_info.spatial_filter_weak_move),
        sizeof(publicData->gk_apc_param.sharp_info.spatial_filter));
     op->memcpy(&(publicData->gk_apc_param_interp.nlm_info),
            &(publicData->gk_apc_param.nlm_info_weak_move),
            sizeof(publicData->gk_apc_param.nlm_info_night));
    op->memcpy(&(publicData->gk_apc_param_interp.blf_info),
            &(publicData->gk_apc_param.blf_info_weak_move),
            sizeof(publicData->gk_apc_param.blf_info_night));
    op->memcpy(&(publicData->gk_apc_param_interp.def.low_cfa_table),
        &(publicData->gk_apc_param.def.low_cfa_table_weak_move),
        sizeof(publicData->gk_apc_param.def.low_cfa_table));
    op->memcpy(&(publicData->gk_apc_param_interp.def.high_cfa_table),
        &(publicData->gk_apc_param.def.high_cfa_table_weak_move),
        sizeof(publicData->gk_apc_param.def.high_cfa_table));

    apc_video_init(processor, &(publicData->gk_apc_param_ir), &(publicData->apc_filters_ctl));
    apc_video_init(processor, &(publicData->gk_apc_param), &(publicData->apc_filters_ctl));
    apc_video_init(processor, &(publicData->gk_apc_param_weak_move), &(publicData->apc_filters_ctl));
    apc_video_init(processor, &(publicData->gk_apc_param_interp), &(publicData->apc_filters_ctl));
    return 0;
};

int ic_dynamic_param_load_isp(Isp3AProcessor *processor, image_sensor_param_t * p_app_param_image_sensor)
{
    ImageOperate *op;
    Isp3APublicContent *publicData;

    op = &(processor->operate);
    publicData = &(processor->content.publicData);
    if (p_app_param_image_sensor->p_adj_param == NULL) {
        return -1;
    }

    if (publicData->gk_apc_param_tmp == NULL)
        publicData->gk_apc_param_tmp = op->malloc(sizeof(adj_param_t));
    if (publicData->gk_apc_param_ir_tmp == NULL)
        publicData->gk_apc_param_ir_tmp = op->malloc(sizeof(adj_param_t));
    if (publicData->gk_rgb2yuv_tmp == NULL)
        publicData->gk_rgb2yuv_tmp = op->malloc(sizeof(rgb_to_yuv_t)*4);
    if (publicData->gk_uv_scale_tmp == NULL)
        publicData->gk_uv_scale_tmp = op->malloc(sizeof(chroma_scale_filter_t));
    if (publicData->is_manual_LE_tmp == NULL)
        publicData->is_manual_LE_tmp = op->malloc(sizeof(local_wdr_t)*3);

    op->memcpy(&(publicData->gk_apc_param_tmp), p_app_param_image_sensor->p_adj_param,
        sizeof(adj_param_t));
    op->memcpy(&(publicData->gk_apc_param_weak_move), p_app_param_image_sensor->p_adj_param,
        sizeof(publicData->gk_apc_param));
    if (p_app_param_image_sensor->p_adj_param_ir != NULL) {
        op->memcpy(&(publicData->gk_apc_param_ir_tmp), p_app_param_image_sensor->p_adj_param_ir,
            sizeof(adj_param_t));
    } else {
        op->memcpy(&(publicData->gk_apc_param_ir_tmp), p_app_param_image_sensor->p_adj_param,
            sizeof(adj_param_t));
    }
    op->memcpy(publicData->gk_rgb2yuv_tmp, p_app_param_image_sensor->p_rgb2yuv, sizeof(rgb_to_yuv_t)*4);
    op->memcpy(&(publicData->gk_uv_scale_tmp), p_app_param_image_sensor->p_chroma_scale,
        sizeof(chroma_scale_filter_t));
     if (p_app_param_image_sensor->p_manual_LE != NULL)
        op->memcpy(publicData->is_manual_LE_tmp, p_app_param_image_sensor->p_manual_LE,
            sizeof(local_wdr_t)*3);
    else
        op->memset(publicData->is_manual_LE_tmp, 0, sizeof(local_wdr_t)*3);

    op->memcpy(&(publicData->gk_apc_param_ir_tmp->sharp_info.rmf),
        &(publicData->gk_apc_param_tmp->sharp_info.rmf_night),
        sizeof(publicData->gk_apc_param_tmp->sharp_info.rmf_night));
    op->memcpy(&publicData->gk_apc_param_ir_tmp->sharp_info.spatial_filter,
        &publicData->gk_apc_param_tmp->sharp_info.spatial_filter_night,
        sizeof(publicData->gk_apc_param_tmp->sharp_info.spatial_filter));
    op->memcpy(&publicData->gk_apc_param_ir_tmp->sharp_info.coring,
        &publicData->gk_apc_param_tmp->sharp_info.coring_nigth,
        sizeof(publicData->gk_apc_param_tmp->sharp_info.coring));
    op->memcpy(&publicData->gk_apc_param_ir_tmp->def.low_cfa_table,
        &publicData->gk_apc_param_tmp->def.low_cfa_table_night,
        sizeof(publicData->gk_apc_param_tmp->def.low_cfa_table));
    op->memcpy(&publicData->gk_apc_param_ir_tmp->def.high_cfa_table,
        &publicData->gk_apc_param_tmp->def.high_cfa_table_night,
        sizeof(publicData->gk_apc_param_tmp->def.high_cfa_table));
    op->memcpy(&(publicData->gk_apc_param_ir.awbae),
        &(publicData->gk_apc_param.awbae_night),
        sizeof(publicData->gk_apc_param.awbae_night));
    op->memcpy(&(publicData->gk_apc_param_ir.nlm_info),
            &(publicData->gk_apc_param.nlm_info_night),
            sizeof(publicData->gk_apc_param.nlm_info_night));
    op->memcpy(&(publicData->gk_apc_param_ir.lce_info),
            &(publicData->gk_apc_param.lce_info_night),
            sizeof(publicData->gk_apc_param.lce_info_night));
    op->memcpy(&(publicData->gk_apc_param_ir.blf_info),
            &(publicData->gk_apc_param.blf_info_night),
            sizeof(publicData->gk_apc_param.blf_info_night));
    op->memcpy(&(publicData->gk_apc_param_ir.gmd_info),
            &(publicData->gk_apc_param.gmd_info_night),
            sizeof(publicData->gk_apc_param.gmd_info_night));
    op->memcpy(&(publicData->gk_apc_param_ir.pmv_info),
            &(publicData->gk_apc_param.pmv_info_night),
            sizeof(publicData->gk_apc_param.pmv_info_night));
    op->memcpy(&(publicData->gk_apc_param_ir.gmf_info),
             &(publicData->gk_apc_param.gmf_info_night),
             sizeof(publicData->gk_apc_param.gmf_info_night));
    op->memcpy(&(publicData->gk_apc_param_ir.vgf_info),
             &(publicData->gk_apc_param.vgf_info_night),
             sizeof(publicData->gk_apc_param.vgf_info_night));


    op->memcpy(&(publicData->gk_apc_param_weak_move.sharp_info.rmf),
        &(publicData->gk_apc_param.sharp_info.rmf_weak_move),
        sizeof(publicData->gk_apc_param.sharp_info.rmf_night));
    op->memcpy(&(publicData->gk_apc_param_weak_move.sharp_info.spatial_filter),
        &(publicData->gk_apc_param.sharp_info.spatial_filter_weak_move),
        sizeof(publicData->gk_apc_param.sharp_info.spatial_filter));
     op->memcpy(&(publicData->gk_apc_param_weak_move.nlm_info),
            &(publicData->gk_apc_param.nlm_info_weak_move),
            sizeof(publicData->gk_apc_param.nlm_info_night));
    op->memcpy(&(publicData->gk_apc_param_weak_move.blf_info),
            &(publicData->gk_apc_param.blf_info_weak_move),
            sizeof(publicData->gk_apc_param.blf_info_night));
    op->memcpy(&(publicData->gk_apc_param_weak_move.def.low_cfa_table),
        &(publicData->gk_apc_param.def.low_cfa_table_weak_move),
        sizeof(publicData->gk_apc_param.def.low_cfa_table));
    op->memcpy(&(publicData->gk_apc_param_weak_move.def.high_cfa_table),
        &(publicData->gk_apc_param.def.high_cfa_table_weak_move),
        sizeof(publicData->gk_apc_param.def.high_cfa_table));

    apc_video_init(processor, publicData->gk_apc_param_tmp, &(publicData->apc_filters_ctl));
    apc_video_init(processor, publicData->gk_apc_param_ir_tmp, &(publicData->apc_filters_ctl));
    apc_video_init(processor, publicData->gk_apc_param_weak_move_tmp, &(publicData->apc_filters_ctl));
    return 0;
};
#if 0
int ic_dynamic_param_interp( Isp3AProcessor *processor,  int count)
{
    int weight_weak;
    int weight_s;
    int i, j;

    ImageOperate *op;
    Isp3APublicContent *publicData;

    op = &(processor->operate);
    publicData = &(processor->content.publicData);
    weight_s = count/10;
    weight_weak = 10 - count/10;
    for(i = 0; i<25; i++)
    {
        for(j = 0; j < 20; j++)
        {
            publicData->gk_apc_param_interp.blf_info.blf_table[i].value[j] =
                (publicData->gk_apc_param_weak_move.blf_info.blf_table[i].value[j]*weight_weak +
                publicData->gk_apc_param_tmp.blf_info.blf_table[i].value[j]*weight_s + 5)/10;

            publicData->gk_apc_param_interp.nlm_info.nlm_table[i].value[j] =
                (publicData->gk_apc_param_weak_move.nlm_info.nlm_table[i].value[j]*weight_weak +
                publicData->gk_apc_param_tmp.nlm_info.nlm_table[i].value[j]*weight_s + 5)/10;

            publicData->gk_apc_param_interp.def.low_cfa_table[i].value[j] =
                (publicData->gk_apc_param_weak_move.def.low_cfa_table[i].value[j]*weight_weak +
                publicData->gk_apc_param_tmp.def.low_cfa_table[i].value[j]*weight_s + 5)/10;

            publicData->gk_apc_param_interp.def.high_cfa_table[i].value[j] =
                            (publicData->gk_apc_param_weak_move.def.high_cfa_table[i].value[j]*weight_weak +
                            publicData->gk_apc_param_tmp.def.high_cfa_table[i].value[j]*weight_s + 5)/10;

            publicData->gk_apc_param_interp.sharp_info.spatial_filter[i].value[j] =
                            (publicData->gk_apc_param_weak_move.sharp_info.spatial_filter[i].value[j]*weight_weak +
                            publicData->gk_apc_param_tmp.sharp_info.spatial_filter[i].value[j]*weight_s + 5)/10;

        }

        for(j = 0; j < 35; j++)
            {
                publicData->gk_apc_param_interp.sharp_info.rmf[i].value[j] =
                            (publicData->gk_apc_param_weak_move.sharp_info.rmf[i].value[j]*weight_weak +
                            publicData->gk_apc_param_tmp.sharp_info.rmf[i].value[j]*weight_s + 5)/10;
            }
    }



}
#else
int ic_dynamic_param_interp( Isp3AProcessor *processor,  int mv_blocks)
{
    int weight_weak;
    int weight_s;
    int i, j;
    int num;
    ImageOperate *op;
    Isp3APublicContent *publicData;
    int th1;
    int th3;

    op = &(processor->operate);
    publicData = &(processor->content.publicData);

    th1 = (publicData->apc_filters_ctl.globle_motion_filter.mv_percent_th1*publicData->mv_info_blks_total)>>14;
    th3 = (publicData->apc_filters_ctl.globle_motion_filter.mv_percent_th3*publicData->mv_info_blks_total)>>14;

    weight_s = mv_blocks - th1;
    if(weight_s < 0)
        weight_s = 0;
    weight_weak = th3 - mv_blocks;
    if(weight_weak < 0)
        weight_weak = 0;

    num = th3 - th1;

    //printf("ic_dynamic_param_interp %d %d %d \n",weight_s,weight_weak,num);
    if(num > 0)
    {
        for(i = 0; i<25; i++)
        {
            for(j = 0; j < 20; j++)
            {
                publicData->gk_apc_param_interp.blf_info.blf_table[i].value[j] =
                    (publicData->gk_apc_param_weak_move.blf_info.blf_table[i].value[j]*weight_weak +
                    publicData->gk_apc_param.blf_info.blf_table[i].value[j]*weight_s + num/2)/num;

                publicData->gk_apc_param_interp.nlm_info.nlm_table[i].value[j] =
                    (publicData->gk_apc_param_weak_move.nlm_info.nlm_table[i].value[j]*weight_weak +
                    publicData->gk_apc_param.nlm_info.nlm_table[i].value[j]*weight_s + num/2)/num;

                publicData->gk_apc_param_interp.def.low_cfa_table[i].value[j] =
                    (publicData->gk_apc_param_weak_move.def.low_cfa_table[i].value[j]*weight_weak +
                    publicData->gk_apc_param.def.low_cfa_table[i].value[j]*weight_s + num/2)/num;

                publicData->gk_apc_param_interp.def.high_cfa_table[i].value[j] =
                                (publicData->gk_apc_param_weak_move.def.high_cfa_table[i].value[j]*weight_weak +
                                publicData->gk_apc_param.def.high_cfa_table[i].value[j]*weight_s + num/2)/num;

                publicData->gk_apc_param_interp.sharp_info.spatial_filter[i].value[j] =
                                (publicData->gk_apc_param_weak_move.sharp_info.spatial_filter[i].value[j]*weight_weak +
                                publicData->gk_apc_param.sharp_info.spatial_filter[i].value[j]*weight_s + num/2)/num;

        }

            for(j = 0; j < 35; j++)
                {
                    publicData->gk_apc_param_interp.sharp_info.rmf[i].value[j] =
                                (publicData->gk_apc_param_weak_move.sharp_info.rmf[i].value[j]*weight_weak +
                                publicData->gk_apc_param.sharp_info.rmf[i].value[j]*weight_s + num/2)/num;
                }
            }
    }



}

#endif
int ic_config_lens_info(Isp3AProcessor *processor)
{
    Isp3AContentAF *content;
    int i;
    content = &(processor->content.AFData);
    switch (content->lensID) {
        case LENS_CMOUNT_ID:
            content->af_G_param = MF_af_param;
            content->lens_drv = MF_dev_drv;
            content->af_zoom_param = MF_DISTANCE_MAP;
            break;
        case LENS_CM8137_ID:
            return -1;
        case LENS_CUSTOM_ID:
            break;
        case LENS_TAMRON18X_ID:
            content->af_G_param = imx035_TamronDF300_param;
            content->lens_drv = TamronDF300_dev_drv;
            content->af_zoom_param = TAMRON_DF300_DISTANCE_MAP;
            break;
        case LENS_SY3510A_ID:
            content->af_G_param = ov5650_SY3510A_param;
            content->lens_drv = SY3510A_AD5839_dev_drv;
            content->af_zoom_param = SY3510A_DISTANCE_MAP;
            break;
        case LENS_IU072F_ID:
            content->af_G_param = iu072f_af_param;
            content->lens_drv = iu072f_dev_drv;
            content->af_zoom_param = iu072f_DISTANCE_MAP;
            break;
	   case LENS_YuTong_YT30031FB_ID:
            content->af_G_param = YuTong_YT30031FB_af_param;
            content->lens_drv = YuTong_YT30031FB_dev_drv;
            content->af_zoom_param = YuTong_YT30031FB_DISTANCE_MAP;
            break;
	   case LENS_Foctek_D14_02812IR_ID:
            content->af_G_param = Foctek_D14_02812IR_af_param;
            content->lens_drv = Foctek_D14_02812IR_dev_drv;
            content->af_zoom_param = Foctek_D14_02812IR_DISTANCE_MAP;
            //printf("lenID is LENS_Foctek_D14_02812IR_ID\n");
            break;
		case LENS_YuTong_YT30021FB_ID:
            content->af_G_param = YuTong_YT30021FB_af_param;
            content->lens_drv = YuTong_YT30021FB_dev_drv;
            content->af_zoom_param = YuTong_YT30021FB_DISTANCE_MAP;
			break;
		case LENS_YS05IR_F1F18_ID:
            content->af_G_param = YS05IR_F1F18_af_param;
            content->lens_drv = YS05IR_F1F18_dev_drv;
            content->af_zoom_param = YS05IR_F1F18_DISTANCE_MAP;
            for(i = 0; i < 3; i++)
            {
                gpio_id[i] = content->gpioArray[i];
            }
            break;
        default:
            content->af_G_param = MF_af_param;
            content->lens_drv = MF_dev_drv;
            content->af_zoom_param = MF_DISTANCE_MAP;
            break;
    }
    return 0;
}

void ic_param_load_af(Isp3AProcessor *processor, void *p_custom_af_param, void *p_custom_zoom_map)
{
    Isp3AContentAF *content;

    content = &(processor->content.AFData);
    content->af_G_param = *(af_param_t*)(p_custom_af_param);
    content->af_zoom_param = p_custom_zoom_map;
    return;
}

int ic_lens_init(Isp3AProcessor *processor)
{
    Isp3AContentAF *content;

    content = &(processor->content.AFData);
    if (content->lens_drv.ic_lens_init) {
        return content->lens_drv.ic_lens_init();
    }
    return 0;
}

int ic_register_custom_3a_fun(Isp3AProcessor *processor, aaa_api_t custom_aaa_api)
{
    aaa_api_t *ispOp = &(processor->isp3A);

    if (custom_aaa_api.p_ae_control_init) {
        ispOp->p_ae_control_init = custom_aaa_api.p_ae_control_init;
    }
    if (custom_aaa_api.p_ae_control) {
        ispOp->p_ae_control = custom_aaa_api.p_ae_control;
    }
    if (custom_aaa_api.p_awb_control_init) {
        ispOp->p_awb_control_init = custom_aaa_api.p_awb_control_init;
    }
    if (custom_aaa_api.p_awb_control) {
        ispOp->p_awb_control = custom_aaa_api.p_awb_control;
    }
    if (custom_aaa_api.p_af_control_init) {
        ispOp->p_af_control_init = custom_aaa_api.p_af_control_init;
    }
    if (custom_aaa_api.p_af_control) {
        ispOp->p_af_control = custom_aaa_api.p_af_control;
    }
    if (custom_aaa_api.p_af_set_range) {
        ispOp->p_af_set_range = custom_aaa_api.p_af_set_range;
    }
    if (custom_aaa_api.p_af_set_calib_param) {
        ispOp->p_af_set_calib_param = custom_aaa_api.p_af_set_calib_param;
    }
    if (custom_aaa_api.p_ae_get_system_gain) {
        ispOp->p_ae_get_system_gain = custom_aaa_api.p_ae_get_system_gain;
    }
    return 0;
}

int isp3a_proc_get_3a_lib(Isp3AProcessor *processor, aaa_api_t * aaa_ptr)
{
    ImageOperate *op = &(processor->operate);

    op->memcpy(aaa_ptr, &(processor->isp3A), sizeof(aaa_api_t));

    return 0;
}

int ic_register_lens_drv(Isp3AProcessor *processor, lens_dev_drv_t custom_lens_drv)
{
    Isp3AContentAF *content;

    content = &(processor->content.AFData);
    content->lens_drv = custom_lens_drv;
    return 0;
}

void ls_set_awb_statis_gain(Isp3AProcessor *processor, u8 enable, u16 * p_tab, u16 tile_count, u32 shift)
{
    int tab_index;
    Isp3AContentAWB *awbData = &(processor->content.AWBData);
    u16 *p_tab_r = p_tab + (0 * tile_count);
    u16 *p_tab_gr = p_tab + (1 * tile_count);
    u16 *p_tab_gb = p_tab + (2 * tile_count);
    u16 *p_tab_b = p_tab + (3 * tile_count);

    for (tab_index = 0; tab_index < tile_count; tab_index++) {
        awbData->ls_awb_gain_r[tab_index] = *(p_tab_r + tab_index);
        awbData->ls_awb_gain_gr[tab_index] = *(p_tab_gr + tab_index);
        awbData->ls_awb_gain_gb[tab_index] = *(p_tab_gb + tab_index);
        awbData->ls_awb_gain_b[tab_index] = *(p_tab_b + tab_index);
    }
    awbData->ls_awb_table_shift = shift;
    awbData->apply_ls_awb_enable = enable;
}

void apply_ls_gain_to_awb_statis(Isp3AProcessor *processor)
{
    u16 tile_num = 0;
    u16 temp_gain_r = 0, temp_gain_g = 0, temp_gain_b = 0;
    u16 unit_gain;
    u16 tile_count;
    awb_data_t * awb_statis_info;
    Isp3AContentAWB *awbData = &(processor->content.AWBData);
    Isp3APublicContent *publicData = &(processor->content.publicData);

    awb_statis_info = publicData->st_awb_info;
    tile_count = publicData->act_tile_num.awb_tile;
    unit_gain = 2 << (7 + awbData->ls_awb_table_shift);

    for (tile_num = 0; tile_num < tile_count; tile_num++) {
        temp_gain_r = awbData->ls_awb_gain_r[tile_num];
        temp_gain_g =
            (awbData->ls_awb_gain_gr[tile_num] + awbData->ls_awb_gain_gb[tile_num]) / 2;
        temp_gain_b = awbData->ls_awb_gain_b[tile_num];

        awb_statis_info[tile_num].r_avg =
            awb_statis_info[tile_num].r_avg * temp_gain_r / unit_gain;
        awb_statis_info[tile_num].g_avg =
            awb_statis_info[tile_num].g_avg * temp_gain_g / unit_gain;
        awb_statis_info[tile_num].b_avg =
            awb_statis_info[tile_num].b_avg * temp_gain_b / unit_gain;
        awb_statis_info[tile_num].lin_y =
            awb_statis_info[tile_num].lin_y * temp_gain_g / unit_gain;
    }
}

int gk_isp_msg_handler(Isp3AProcessor *processor, img_msg_t * p_msg, pipeline_control_t * p_adj_video_pipe)
{
    wb_gain_t wb_org_gain[2], wb_ref_gain[2];
    Isp3APublicContent *publicData;
    Isp3AContentAWB *awbData;
    Isp3AContentAF *afData;

    awbData = &(processor->content.AWBData);
    afData = &(processor->content.AFData);
    publicData = &(processor->content.publicData);

    //printf("msg %d %d\n", p_msg->msg_id, p_msg->param[0]);
    switch (p_msg->msg_id) {
        case IMG_MSG_ANTIFLICKER:
            if (p_msg->param[0] == 0)
                ae_load_antiflicker_params(processor, publicData->gk_50hz_lines, 4, p_msg->param[0]);
            else if (p_msg->param[0] == 1)
                ae_load_antiflicker_params(processor, publicData->gk_60hz_lines, 4, p_msg->param[0]);
            break;
        case IMG_MSG_METERMODE:
            ae_set_meter_mode(processor, p_msg->param[0]);
            break;
        case IMG_MSG_BACKLIGHT:
            ae_enable_backlight(processor, p_msg->param[0]);
            break;
        case IMG_MSG_SHUTTER_LIMIT:
            break;
        case IMG_MSG_SATURATION:
            if (!apc_set_saturation(processor, p_msg->param[0]))
                p_adj_video_pipe->rgb_yuv_matrix_update = 1;
            break;
        case IMG_MSG_CONTRAST:
            if (!apc_set_contrast(processor, p_msg->param[0]))
                p_adj_video_pipe->gamma_update = 1;
            break;

        case IMG_MSG_VPS_CONTRAST:
            break;
        case IMG_MSG_AUTO_CONTRAST:
            d_tone_curve_set_enable(processor, p_msg->param[0]);
            p_adj_video_pipe->gamma_update = 1;
            break;
        case IMG_MSG_AUTO_CONTRAST_STR:
            {
                float dynamic_alpha = (float) (p_msg->param[0]) / 128.0;

                d_tone_curve_set_apram(processor, dynamic_alpha);
                //if(d_tone_curve_get_status(processor))
                    apc_set_contrast(processor, p_msg->param[0]);
                p_adj_video_pipe->gamma_update = 1;
            }
            break;
        case IMG_MSG_HUE:
            if (!apc_set_hue(processor, p_msg->param[0]))
                p_adj_video_pipe->rgb_yuv_matrix_update = 1;
            break;
        case IMG_MSG_BRIGHTNESS:
            if (!apc_set_brightness(processor, p_msg->param[0]))
                p_adj_video_pipe->rgb_yuv_matrix_update = 1;
            break;
        case IMG_MSG_AE_HLC_ENABLE:
            ae_enable_high_luma_ctl(processor, p_msg->param[0]);
            break;
        case IMG_MSG_SHARPNESS:
            apc_set_sharpness_level(processor, p_msg->param[0]);
            break;
        case IMG_MSG_SHARPEN_LEVEL_EX:
            //    adj_set_sharpeness_property_fir_ratio(p_msg->param[0]);
            //    adj_set_sharpeness_property_spatial_filter_ratio(p_msg->param[1]);
            //    adj_set_sharpeness_property_max_change_ratio(p_msg->param[2]);
            //    adj_set_sharpeness_property_overall_ratio(p_msg->param[3]);
            //    adj_set_sharpeness_property_cfa_threshold_ratio(p_msg->param[4]);
            //    adj_set_sharpeness_property_cfa_weight_ratio(p_msg->param[5]);
            apc_set_sharpeness_property_ratio(processor, p_msg->param[0], p_msg->param[1],
                p_msg->param[2], p_msg->param[3], p_msg->param[4],
                p_msg->param[5]);
            break;
        case IMG_MSG_SET_CFA_EX:
            adj_set_cfa(processor, p_msg->param[0], p_msg->param[1],
                p_msg->param[2], p_msg->param[3], p_msg->param[4]);
            break;
        case IMG_MSG_DNR3D_LEVEL_EX:
            apc_set_dnr3d_property_ratio(processor, p_msg->param[0], p_msg->param[1],
                p_msg->param[2], p_msg->param[3]);
            break;
        case IMG_MSG_AE_LOAD_EXP_LINE:
            ae_load_exp_params(processor, (void *) p_msg->param[0], p_msg->param[1],
                p_msg->param[2]);
            break;
        case IMG_MSG_AE_SET_BELT:
            ae_set_line_belt(processor, p_msg->param[0]);
            break;
        case IMG_MSG_WB_ENV:
            awb_set_control_env(processor, (awb_environment_t) p_msg->param[0]);
            break;
        case IMG_MSG_WB_MODE:
            awbData->awb_mode = p_msg->param[0];
            break;
        case IMG_MSG_WB_METHOD:
            awbData->awb_algo = p_msg->param[0];
            break;
        case IMG_MSG_AE_MODE:
            break;
        case IMG_MSG_AF_MODE:
            afData->af_ctrl_param.workingMode = (af_mode) p_msg->param[0];
            break;
        case IMG_MSG_AF_SET_RANGE:
            if (processor->isp3A.p_af_set_range)
                processor->isp3A.p_af_set_range(processor, (af_range_t *) p_msg->param[0]);
            break;
        case IMG_MSG_AF_SET_ROI:
            afData->af_ctrl_param.af_window = *(af_ROI_config_t *) p_msg->param[0];
            break;
        case IMG_MSG_AF_SET_ZOOM:
            afData->af_ctrl_param.zoom_idx = ((u16) p_msg->param[0]);
            break;
        case IMG_MSG_AF_SET_FOCUS:
            if (afData->af_ctrl_param.workingMode == MANUAL)
                afData->af_ctrl_param.focus_idx = ((s32) p_msg->param[0]);
            break;
        case IMG_MSG_AF_SET_MANUAL_MODE:
            afData->af_ctrl_param.workingMode = MANUAL;
            afData->af_ctrl_param.manual_mode = (af_manual_mode) p_msg->param[0];
            break;
        case IMG_MSG_ENABLE_ADJ:
            publicData->cntl_3a.adj_enable = p_msg->param[0];
            break;
        case IMG_MSG_ENABLE_AE:
            publicData->cntl_3a.ae_enable = p_msg->param[0];
            break;
        case IMG_MSG_ENABLE_AF:
            publicData->cntl_3a.af_enable = p_msg->param[0];
            break;
        case IMG_MSG_ENABLE_AWB:
            publicData->cntl_3a.awb_enable = p_msg->param[0];
            break;
        case IMG_MSG_ENABLE_VPS_3D:
            adj_set_vps_3d(processor, p_msg->param[0]);
            break;
        case IMG_MSG_ENABLE_LOCAL_EXP:
            apc_set_local_exp(processor, p_msg->param[0]);
            break;
        case IMG_MSG_LENS_SET_IRCUT:
            break;
        case IMG_MSG_AF_RESET:
            break;
        case IMG_MSG_AE_SET_TARGET_RATIO:
            ae_set_target_param(processor, p_msg->param[0]);
            break;
        case IMG_MSG_SET_COLOR_STYLE:
            publicData->rgb2yuv_style = p_msg->param[0];
            publicData->apc_filters_ctl.rgb_yuv_matrix_update = 1;
            break;
        case IMG_MSG_SET_BW_MODE:
            apc_set_bw_mode(processor, p_msg->param[0]);
            publicData->bw_mode_enable = p_msg->param[0];
            break;
//added by Hu Yin 08-03-2017
        case IMG_MSG_SET_LOW_BITRATE_MODE:
            apc_set_low_bitrate_mode(processor, p_msg->param[0]);
            //low_bitrate_mode_enable = p_msg->param[0];
            break;
//end
        case IMG_MSG_SET_EXTRA_BLC:
            apc_set_blc_ext(processor, p_msg->param[0]);
            break;
        case IMG_MSG_EXIT:
            apc_recover_image_property(processor);
            break;
        case IMG_MSG_SET_AE_ROI:
            ae_set_meter_table((int *) &p_msg->param[0]);
            break;
        case IMG_MSG_AE_SET_SPEED:
            ae_set_speed_ctl(processor, p_msg->param[0], p_msg->param[1]);
            break;
        case IMG_MSG_AE_SET_KNEE:
            ae_set_auto_luma_ctl(processor, p_msg->param[0]);
            break;
        case IMG_MSG_AWB_SET_WB_SHIFT:
            wb_org_gain[0].r_gain = (u32) p_msg->param[0];
            wb_org_gain[0].g_gain = (u32) p_msg->param[1];
            wb_org_gain[0].b_gain = (u32) p_msg->param[2];

            wb_org_gain[1].r_gain = (u32) p_msg->param[3];
            wb_org_gain[1].g_gain = (u32) p_msg->param[4];
            wb_org_gain[1].b_gain = (u32) p_msg->param[5];

            wb_ref_gain[0].r_gain = (u32) p_msg->param[6];
            wb_ref_gain[0].g_gain = (u32) p_msg->param[7];
            wb_ref_gain[0].b_gain = (u32) p_msg->param[8];

            wb_ref_gain[1].r_gain = (u32) p_msg->param[9];
            wb_ref_gain[1].g_gain = (u32) p_msg->param[10];
            wb_ref_gain[1].b_gain = (u32) p_msg->param[11];
            awb_set_white_tile_wb_shift(processor, wb_org_gain, wb_ref_gain);
            break;
        case IMG_MSG_AWB_SET_SPEED:
            awb_set_speed(processor, (u8) p_msg->param[0]);
            break;
        case IMG_MSG_AWB_SET_CALI_DIFF_THR:
            awb_set_diff_thr_for_cali(processor, (u16) p_msg->param[0], (u16) p_msg->param[1]);
            break;
        case IMG_MSG_AWB_VIG_GAIN_TAB:
            ls_set_awb_statis_gain(processor, (u8) p_msg->param[0],
                (u16 *) p_msg->param[1], (u16) p_msg->param[2],
                (u32) p_msg->param[3]);
            break;
        case IMG_MSG_LENS_SHADING_ENABLE:
            publicData->gk_lens_shading.enable = p_msg->param[0];
            break;
        case IMG_MSG_ADJ_YUV_EXTRA_BRIGHTNESS:
            apc_enable_extra_brightness(processor, (u8) p_msg->param[0]);
            break;
        case IMG_MSG_AWB_SET_CUSTOM_GAIN:
            {
                wb_gain_t cus_gain;

                cus_gain.r_gain = p_msg->param[0];
                cus_gain.g_gain = p_msg->param[1];
                cus_gain.b_gain = p_msg->param[2];
                awb_set_custom_gain(processor, &cus_gain);
            }
            break;
        case IMG_MSG_SET_SYNC_TIME_DELAY:
            publicData->sync_time_delay = p_msg->param[0];
            break;
        case IMG_MSG_SET_AUTO_SHARPEN_ENABLE:
            apc_set_auto_sharpen_enable(processor, p_msg->param[0]);
            break;
        case IMG_MSG_SET_AUTO_SPATIAL_NOISE_FILTER_ENABLE:
            apc_set_auto_spatial_noise_filter_enable(processor, p_msg->param[0]);
            break;
        default:
            printf("error msg\n");
            break;
    }

    return 0;
}

void apc_set_init_def(Isp3AProcessor *processor, u8 enable)
{
    processor->content.publicData.apc_set_init_sign = enable;
}

void adj_get_isp_statistics_enable(Isp3AProcessor *processor, int enable)
{
    if (enable) {
        processor->content.publicData.adj_focus_statistics_enable = 1;
    } else {
        processor->content.publicData.adj_focus_statistics_enable = 0;
    }
}

int adj_get_isp_statistics(Isp3AProcessor *processor, ISP_CfaAfStatT * fh_sta,
    ISP_CfaAfStatT * fhv_sta, ae_data_t *ae_data,
    awb_data_t* awb_data, ISP_HistogramStatT* hist_data,unsigned int xms)
{
    Isp3APublicContent  *publicData;
    publicData = &(processor->content.publicData);

    if (fhv_sta == NULL || fh_sta == NULL || publicData->adj_focus_statistics_enable == 0)
        return -1;
    pthread_mutex_lock(&(publicData->statMutexLock));
    if (xms == 0)
        pthread_cond_wait(&(publicData->statCond), &(publicData->statMutexLock));
    else {
        struct timespec abstime;

        abstime.tv_sec = xms / 1000;
        abstime.tv_nsec = (xms % 1000) * 1000;
        pthread_cond_timedwait(&(publicData->statCond), &(publicData->statMutexLock), &abstime);
    }
    memcpy(fh_sta, publicData->st_af_cfa_info, sizeof(ISP_CfaAfStatT) * MAX_AF_TILE_NUM);
    memcpy(fhv_sta, publicData->st_af_rgb_info, sizeof(ISP_CfaAfStatT) * MAX_AF_TILE_NUM);
    memcpy(ae_data, publicData->st_ae_info, sizeof(ae_data_t) * MAX_AE_TILE_NUM);
    memcpy(awb_data,publicData->st_awb_info,sizeof(awb_data_t) * MAX_AWB_TILE_NUM);
    memcpy(hist_data,&(publicData->st_dsp_histo_info),sizeof(ISP_HistogramStatT));
    pthread_mutex_unlock(&(publicData->statMutexLock));
    return 0;
}

int adj_get_color_info(Isp3AProcessor *processor, color_3d_t *color_info)
{
    Isp3APublicContent  *publicData;
    publicData = &(processor->content.publicData);

    //memcpy(color_info,&(gk_apc_param.def.color),sizeof(color_info));
    color_info->cc_interpo_ev.ev_thre_hi = publicData->gk_apc_param.def.color.cc_interpo_ev.ev_thre_hi;
    color_info->cc_interpo_ev.ev_thre_low= publicData->gk_apc_param.def.color.cc_interpo_ev.ev_thre_low;
    color_info->type= publicData->gk_apc_param.def.color.type;
    color_info->table_count= publicData->gk_apc_param.def.color.table_count;
    color_info->control= publicData->gk_apc_param.def.color.control;
    color_info->table[0].r_gain= publicData->gk_apc_param.def.color.table[0].r_gain;
    color_info->table[0].b_gain= publicData->gk_apc_param.def.color.table[0].b_gain;
    color_info->table[1].r_gain= publicData->gk_apc_param.def.color.table[1].r_gain;
    color_info->table[1].b_gain= publicData->gk_apc_param.def.color.table[1].b_gain;
    color_info->table[2].r_gain= publicData->gk_apc_param.def.color.table[2].r_gain;
    color_info->table[2].b_gain= publicData->gk_apc_param.def.color.table[2].b_gain;

   return 0;

}

int isp_control_init(void *arg)
{
    digital_sat_level_t dgain_sat = { 16383, 16383, 16383, 16383 };
    aaa_api_t *isp3a = NULL;
    Isp3APublicContent  *publicData;
    Isp3AContentCTL *ctlData;
    Isp3AContentAWB *awbData;
    Isp3AContentAF *afData;
    Isp3AProcessor *processor = (Isp3AProcessor *)arg;

    isp3a    = &(processor->isp3A);
    publicData = &(processor->content.publicData);
    ctlData = &(processor->content.ctlData);
    awbData = &(processor->content.AWBData);
    afData = &(processor->content.AFData);

    gk_isp_set_dgain_thr(processor, &dgain_sat);
    gk_isp_config_statistics_params(processor, &(publicData->gk_tile_config));
    gk_isp_set_sharpen_template_mode(processor, ctlData->sharp_temp_mode);

    if (publicData->apc_set_init_sign == 1) {
        gk_isp_enable_cc(processor);
        gk_isp_set_rgb2yuv_matrix(processor, &(publicData->gk_rgb2yuv[publicData->rgb2yuv_style]));
    }

    if (isp3a->p_awb_control_init)
        isp3a->p_awb_control_init(processor, WB_AUTOMATIC,
            publicData->gk_awb_param.menu_gain,
            &(publicData->gk_awb_param.wr_table),
            publicData->gk_awb_param.awb_param_table_idx);

    awbData->awb_mode = WB_AUTOMATIC;

    if (isp3a->p_ae_control_init)
        isp3a->p_ae_control_init(processor, publicData->typeofsensor,
            publicData->gk_50hz_lines, 4, publicData->sensor_max_db,
            publicData->sensor_double_step, publicData->gk_p_ae_gain_dgain,
            publicData->gk_p_ae_sht_dgain, publicData->gk_h_dlight_range);

    ae_set_line_belt(processor, 4);

    if (isp3a->p_af_control_init)
        isp3a->p_af_control_init(processor, &(afData->af_ctrl_param),
             &(afData->af_G_param), afData->af_zoom_param, &(afData->lens_ctrl));

    gk_isp_set_af_statistics_ex(processor, afData->lens_ctrl.af_stat_config, 1);

    return 0;
}
#if 0
void isp_control_loop(void *arg)
{
    Isp3AProcessor *processor = NULL;
    aaa_api_t *isp3a = NULL;
    //int fd_media;
    int count;
    adj_param_t *p_apc_param_valid = NULL;
    int z_run = -1;
    int f_run = -1;
    u8 ae_stab_flag = 0;
    int ae_step = 0;
    int ae_flag = 0;
    //int vps_i = 0;
    //int ae_target_vps;
    //int ae_luma_vps;
    u8  sharp_level_mode = 0; //choose level control
    //SYS_DRV_SourceBufFormatT buffer_format;
    ImageOperate *op;
    Isp3APublicContent  *publicData;
    Isp3AContentAWB *awbData;
    Isp3AContentAF *afData;

    processor = (Isp3AProcessor *)arg;
    isp3a    = &(processor->isp3A);
    //fd_media = processor->content.publicData.fd;
    op = &(processor->operate);
    publicData = &(processor->content.publicData);
    awbData = &(processor->content.AWBData);
    afData = &(processor->content.AFData);



    //printf("loop...\n");
    // TODO: Get statistics info for ISP
    if (gk_isp_get_statistics(processor))
        return;

    if (awbData->apply_ls_awb_enable) {
        apply_ls_gain_to_awb_statis(processor);
    }

    // TODO: externel message process
    if (pthread_mutex_trylock(&(publicData->msg_mutex)) == 0)     //unlocked
    {
        count = 0;
        while (publicData->msg_update > 0) {
            gk_isp_msg_handler(processor, &(publicData->msg[count]), &(publicData->apc_filters_ctl));
            //    printf("process msg_update %d ,count %d\n", msg_update, count);
            publicData->msg_update--;
            count++;
        }
        pthread_mutex_unlock(&(publicData->msg_mutex));
    }
    if (publicData->adj_focus_statistics_enable == 1)
        pthread_cond_signal(&(publicData->statCond));

    pthread_cond_signal(&(publicData->d_tone_proc.dyn_tone_cond));
//printf("pthread cond...ok\n");
    if (pthread_mutex_trylock(&(publicData->update_mutex)) == 0)  //unlocked
    {
        // TODO: AE Control
        if (publicData->cntl_3a.ae_enable)
            if (isp3a->p_ae_control)
                isp3a->p_ae_control(processor, publicData->st_ae_info,
                        publicData->act_tile_num.ae_tile,
                        &(publicData->ae_video_info),
                         &(publicData->ae_still_info));

        // TODO: AWB Control
        if (publicData->cntl_3a.awb_enable)
            if (isp3a->p_awb_control)
                isp3a->p_awb_control(processor, awbData->awb_algo, awbData->awb_mode,
                    publicData->act_tile_num.awb_tile, publicData->st_awb_info, &awbData->awb_gain);
        pthread_mutex_unlock(&(publicData->update_mutex));
    }


    // TODO: AF Control
    if (publicData->cntl_3a.af_enable) {
        if (afData->lens_drv.check_isFocusRuning)
            f_run = afData->lens_drv.check_isFocusRuning();
        if (afData->lens_drv.check_isZoomRuning)
            z_run = afData->lens_drv.check_isZoomRuning();
        if (isp3a->p_af_control)
            isp3a->p_af_control(processor, &(afData->af_ctrl_param), publicData->act_tile_num.af_tile,
                publicData->st_af_cfa_info, publicData->act_tile_num.awb_tile, publicData->st_awb_info,
                &(afData->lens_ctrl), &(publicData->ae_video_info), (z_run || f_run), publicData->st_af_rgb_info);
    }

    // TODO: Sync ISP params.
    if (pthread_mutex_trylock(&(publicData->load_isp_param_mutex)) == 0)      //unlocked
    {
        if (publicData->dynamic_load_param_sign == 1) {
            op->memcpy(&(publicData->gk_apc_param), publicData->gk_apc_param_tmp, sizeof(adj_param_t));
            op->memcpy(&(publicData->gk_apc_param_ir), publicData->gk_apc_param_ir_tmp,sizeof(adj_param_t));
            op->memcpy(&(publicData->gk_rgb2yuv), publicData->gk_rgb2yuv_tmp, sizeof(rgb_to_yuv_t)*4);
            op->memcpy(&(publicData->gk_uv_scale), publicData->gk_uv_scale_tmp,sizeof(chroma_scale_filter_t));
            op->memcpy(publicData->is_manual_LE, publicData->is_manual_LE_tmp,sizeof(local_wdr_t)*3);
            publicData->dynamic_load_param_sign = 0;
            op->free(publicData->gk_apc_param_tmp);
            op->free(publicData->gk_apc_param_ir_tmp);
            op->free(publicData->gk_rgb2yuv_tmp);
            op->free(publicData->gk_uv_scale_tmp);
            op->free(publicData->is_manual_LE_tmp);
            publicData->gk_apc_param_tmp = NULL;
            publicData->gk_apc_param_ir_tmp = NULL;
            publicData->gk_rgb2yuv_tmp = NULL;
            publicData->gk_uv_scale_tmp = NULL;
            publicData->is_manual_LE_tmp = NULL;
        }
        pthread_mutex_unlock(&(publicData->load_isp_param_mutex));
    }

    //ae_step = ae_get_step(processor);
   // if(ae_step >= 8)
    //    ae_flag = 1;
    if (publicData->cntl_3a.adj_enable) {
        ae_stab_flag = ae_check_stable(processor);

        if (publicData->bw_mode_enable)
            p_apc_param_valid = &(publicData->gk_apc_param_ir);
        else
            p_apc_param_valid = &(publicData->gk_apc_param);

        apc_video_aeawb_control(processor, isp3a->p_ae_get_system_gain(processor),
            &(p_apc_param_valid->awbae), &(publicData->apc_aeawb_ctl));
        apc_video_blc_control(processor, isp3a->p_ae_get_system_gain(processor),
            awbData->awb_gain.video_wb_gain, p_apc_param_valid, &(publicData->apc_filters_ctl));
        apc_video_aeawb_cntl(processor, isp3a->p_ae_get_system_gain(processor),
            awbData->awb_gain.video_wb_gain, p_apc_param_valid, &(publicData->apc_filters_ctl),
            &(publicData->gk_uv_scale));
        apc_video_nf_control(processor, isp3a->p_ae_get_system_gain(processor),
            awbData->awb_gain.video_wb_gain, 0, p_apc_param_valid, &(publicData->apc_filters_ctl));
        apc_contrast_for_ll(processor, p_apc_param_valid, &(publicData->apc_filters_ctl), ae_stab_flag,
            publicData->is_manual_LE);
        apc_yuv_extra_brightness(processor, isp3a->p_ae_get_system_gain(processor),
            &(publicData->apc_filters_ctl));
        //apc_encode_realtime_control(processor, isp3a->p_ae_get_system_gain(processor),
         //    p_apc_param_valid, &(publicData->apc_filters_ctl));
    }
    if (publicData->apc_aeawb_ctl.aeawb_update) {
        ae_set_target_val(processor, publicData->apc_aeawb_ctl.ae_target);
        awb_set_wb_ratio(processor, &(publicData->apc_aeawb_ctl));
        publicData->apc_aeawb_ctl.aeawb_update = 0;
    }

    if (publicData->apc_filters_ctl.local_wdr_update) {
        gk_isp_set_wdr(processor, &(publicData->apc_filters_ctl.local_wdr));
        publicData->apc_filters_ctl.local_wdr_update = 0;
    }
    if (publicData->apc_filters_ctl.black_corr_update) {
        gk_isp_set_final_blc(processor, &(publicData->apc_filters_ctl.black_corr),
            publicData->sensor_pattern);
        publicData->apc_filters_ctl.black_corr_update = 0;
    }
    if (publicData->apc_filters_ctl.chroma_scale_update) {
        gk_isp_set_uv_scale(processor, &(publicData->apc_filters_ctl.uv_scale));
        publicData->apc_filters_ctl.chroma_scale_update = 0;
    }
    if (publicData->apc_filters_ctl.cc_matrix_update) {
        gk_isp_set_cc(processor, &(publicData->apc_filters_ctl.color_corr));
        publicData->apc_filters_ctl.cc_matrix_update = 0;
    }
    if (publicData->apc_filters_ctl.badpix_corr_update) {
        gk_isp_set_auto_bad_pixel_correction(processor,
            &(publicData->apc_filters_ctl.badpix_corr_strength));
        publicData->apc_filters_ctl.badpix_corr_update = 0;
    }
    if (publicData->apc_filters_ctl.chroma_median_filter_update) {
        gk_isp_set_color_denoise_filter(processor,
            &(publicData->apc_filters_ctl.chroma_median_filter));
        publicData->apc_filters_ctl.chroma_median_filter_update = 0;
    }
    if (publicData->apc_filters_ctl.cfa_filter_update) {
        gk_isp_set_cfa_denoise_filter(processor, &(publicData->apc_filters_ctl.cfa_filter));
        publicData->apc_filters_ctl.cfa_filter_update = 0;
    }
    if (publicData->apc_filters_ctl.spatial_filter_update) {
        gk_isp_set_spatial_filter(processor, 0,
            &(publicData->apc_filters_ctl.spatial_filter));
        publicData->apc_filters_ctl.spatial_filter_update = 0;
    }
    if (publicData->apc_filters_ctl.sharp_level_minimum_update) {
        gk_isp_set_sharpen_lm(processor, IMG_VIDEO,
            &(publicData->apc_filters_ctl.sharp_level_minimum));
        publicData->apc_filters_ctl.sharp_level_minimum_update = 0;
    }

    if (publicData->apc_filters_ctl.sharp_level_overall_update) {

        if(publicData->apc_filters_ctl.sharp_fir.fir_strength >0 &&
            publicData->apc_filters_ctl.sharp_fir.fir_coeff[4] == 1)
            sharp_level_mode = 1;


        gk_isp_set_sharpen_level(processor, IMG_VIDEO,
            sharp_level_mode,
            &(publicData->apc_filters_ctl.sharp_level_overall));
        publicData->apc_filters_ctl.sharp_level_overall_update = 0;
    }
    if (publicData->apc_filters_ctl.high_freq_noise_reduc_update) {
        gk_isp_set_demosaic_noise_reduction(processor,
            publicData->apc_filters_ctl.high_freq_noise_reduc);
        publicData->apc_filters_ctl.high_freq_noise_reduc_update = 0;
    }
    if (publicData->apc_filters_ctl.sharp_retain_update) {
        gk_isp_set_sharpen_change_str(processor, IMG_VIDEO, publicData->apc_filters_ctl.sharp_retain);
        publicData->apc_filters_ctl.sharp_retain_update = 0;
    }
    if (publicData->apc_filters_ctl.sharp_max_change_update) {
        gk_isp_set_sharp_max_change(processor, IMG_VIDEO,
            &(publicData->apc_filters_ctl.sharp_max_change));
        publicData->apc_filters_ctl.sharp_max_change_update = 0;
    }
    if (publicData->apc_filters_ctl.sharp_fir_update) {
        publicData->apc_filters_ctl.sharp_fir.edge_disp_enable = 0;
        publicData->apc_filters_ctl.sharp_fir.edge_dir_sel     = 0;
        publicData->apc_filters_ctl.sharp_fir.edge_dir_shift   = 0;
        publicData->apc_filters_ctl.sharp_fir.edge_dir_info    = 0;
        gk_isp_set_sharpen_template(processor, IMG_VIDEO,
            &(publicData->apc_filters_ctl.sharp_fir));
        publicData->apc_filters_ctl.sharp_fir_update = 0;
    }
    if (publicData->apc_filters_ctl.sharp_coring_update) {
        gk_isp_set_sharpen_edge_str(processor, IMG_VIDEO,
            &(publicData->apc_filters_ctl.sharp_coring),
            &(publicData->apc_filters_ctl.sharp_coring_mv));
        publicData->apc_filters_ctl.sharp_coring_update = 0;
    }

//added by Hu Yin 22-04-2016

    //printf("[gk_isp_cntl_loop] vps_contrast_enable: %d\n", vps_contrast_enable);
    if(publicData->isp_capability.contrast_enable)
    {
    }
    // end
    //added by Hu Yin 15-08-2017 for GK7202
    if (publicData->apc_filters_ctl.depurple_fringe_update) {
        publicData->apc_filters_ctl.depurple_fringe.depurple_fringe_enable = 1;
        publicData->apc_filters_ctl.depurple_fringe.bpattern = publicData->sensor_pattern;
        gk_isp_set_depurple_fringe(processor,
            &(publicData->apc_filters_ctl.depurple_fringe));
        publicData->apc_filters_ctl.depurple_fringe_update = 0;
    }

    if (publicData->apc_filters_ctl.nlm_noise_filter_update) {
        publicData->apc_filters_ctl.nlm_noise_filter.nlm_noise_filter_enable = 1;
        publicData->apc_filters_ctl.nlm_noise_filter.nlm_noise_filter_mv_enable = 1;
        gk_isp_set_nlm_noise_filter(processor,
            &(publicData->apc_filters_ctl.nlm_noise_filter));
        publicData->apc_filters_ctl.nlm_noise_filter_update = 0;
    }

    if (publicData->apc_filters_ctl.variance_guided_filter_update) {
        publicData->apc_filters_ctl.variance_guided_filter.vgf_enable     = 1;
        publicData->apc_filters_ctl.variance_guided_filter.vgf_bpc_enable = 1;
        publicData->apc_filters_ctl.variance_guided_filter.vgf_mv_enable  = 1;
        publicData->apc_filters_ctl.variance_guided_filter.vgf_edge_wide_weight = 0;
        publicData->apc_filters_ctl.variance_guided_filter.vgf_edge_narrow_weight = 8;
        publicData->apc_filters_ctl.variance_guided_filter.vgf_disp_info_en = 0;
        publicData->apc_filters_ctl.variance_guided_filter.vgf_disp_var_th1 = 0;
        publicData->apc_filters_ctl.variance_guided_filter.vgf_disp_var_th2 = 1023;
        publicData->apc_filters_ctl.variance_guided_filter.vgf_disp_grad_th1 = 0;
        publicData->apc_filters_ctl.variance_guided_filter.vgf_disp_grad_th2 = 255;

        gk_isp_set_variance_guided_filter(processor,
            &(publicData->apc_filters_ctl.variance_guided_filter));
        publicData->apc_filters_ctl.variance_guided_filter_update = 0;
    }

    if (publicData->apc_filters_ctl.demosaic_noise_reduction_update) {
        publicData->apc_filters_ctl.demosaic_noise_reduction.thresh_grad_clipping = 512;
        publicData->apc_filters_ctl.demosaic_noise_reduction.shift = 7;
        publicData->apc_filters_ctl.demosaic_noise_reduction.demoire_A_Y = 4;
        publicData->apc_filters_ctl.demosaic_noise_reduction.demoire_B_Y = 4;
        publicData->apc_filters_ctl.demosaic_noise_reduction.demoire_C_Y = 4;
        publicData->apc_filters_ctl.demosaic_noise_reduction.demoire_D_Y = 4;
        publicData->apc_filters_ctl.demosaic_noise_reduction.demoire_A_C = 4;
        publicData->apc_filters_ctl.demosaic_noise_reduction.demoire_B_C = 4;
        publicData->apc_filters_ctl.demosaic_noise_reduction.demoire_C_C = 4;
        publicData->apc_filters_ctl.demosaic_noise_reduction.demoire_D_C = 4;
        publicData->apc_filters_ctl.demosaic_noise_reduction.conti_th1 = 4;
        publicData->apc_filters_ctl.demosaic_noise_reduction.conti_th2 = 8;
        publicData->apc_filters_ctl.demosaic_noise_reduction.conti_th3 = 24;
        publicData->apc_filters_ctl.demosaic_noise_reduction.conti_th4 = 8;

        gk_isp_set_demosaic_noise_reduction_new(processor,
            &(publicData->apc_filters_ctl.demosaic_noise_reduction));
        publicData->apc_filters_ctl.demosaic_noise_reduction_update = 0;
    }

    if (publicData->apc_filters_ctl.chroma_median_filter_ex_update) {
        publicData->apc_filters_ctl.chroma_median_filter_ex.ex_blur_data_u = 128;
        publicData->apc_filters_ctl.chroma_median_filter_ex.ex_blur_data_v = 128;
        publicData->apc_filters_ctl.chroma_median_filter_ex.ex_blur_en = 1;
        publicData->apc_filters_ctl.chroma_median_filter_ex.blur_nolimit_en = 1;

        gk_isp_set_chroma_median_filter_ex(processor,
            &(publicData->apc_filters_ctl.chroma_median_filter_ex));
        publicData->apc_filters_ctl.chroma_median_filter_ex_update = 0;
    }

    if (publicData->apc_filters_ctl.local_contrast_enhancement_update) {
        publicData->apc_filters_ctl.local_contrast_enhancement.lce_enable = 3;
        publicData->apc_filters_ctl.local_contrast_enhancement.pre_lut_en = 0;
        publicData->apc_filters_ctl.local_contrast_enhancement.pm_en = 0;
        //publicData->apc_filters_ctl.local_contrast_enhancement.disp_mv_en = 0;

        local_contrast_enhancement_t local_contrast_enhancement_info;
        gk_isp_get_local_contrast_enhancement(processor,&local_contrast_enhancement_info);
        publicData->apc_filters_ctl.local_contrast_enhancement.disp_mv_en = local_contrast_enhancement_info.disp_mv_en;

        gk_isp_set_local_contrast_enhancement(processor,
            &(publicData->apc_filters_ctl.local_contrast_enhancement));
        publicData->apc_filters_ctl.local_contrast_enhancement_update = 0;
    }

    if (publicData->apc_filters_ctl.bilateral_filter_update || ae_flag) {
        publicData->apc_filters_ctl.bilateral_filter.y_enable = 1;
        publicData->apc_filters_ctl.bilateral_filter.c_enable = 1;
        publicData->apc_filters_ctl.bilateral_filter.use_mv   = 1;
#if 0
        publicData->apc_filters_ctl.bilateral_filter.th_enable   = 1;
        publicData->apc_filters_ctl.bilateral_filter.min_mv_str   = 400;
        publicData->apc_filters_ctl.bilateral_filter.multi_coff   = 64;
        if(ae_flag == 1)
        {
            publicData->apc_filters_ctl.bilateral_filter.adj_enable   = 1;
            publicData->apc_filters_ctl.bilateral_filter.multi_coff = 64*(ae_step>>3);
        }
#endif
        gk_isp_set_bilateral_filter(processor, &(publicData->apc_filters_ctl.bilateral_filter));
        publicData->apc_filters_ctl.bilateral_filter_update = 0;
    }

    if (publicData->apc_filters_ctl.gmd_cfa_update ) {
        publicData->apc_filters_ctl.gmd_cfa.en = 1;
        publicData->apc_filters_ctl.gmd_cfa.gamma_en = 1;
        publicData->apc_filters_ctl.gmd_cfa.blur_en_cfa = 1;
        //publicData->apc_filters_ctl.gmd_cfa.digital_gain_multiplier_green = (processor->content.ctlData.adj_gmd_params.digital_gain_multiplier_green *
            //processor->content.publicData.ae_video_info.dgain + 512) >> 10;

        //printf("gmd_cfa_update digital_gain_multiplier_green %d\n", publicData->apc_filters_ctl.gmd_cfa.digital_gain_multiplier_green);

        //publicData->apc_filters_ctl.gmd_cfa.single_sad_en = 1;
        //publicData->apc_filters_ctl.gmd_cfa.cfa_interp_en = 1;
        //publicData->apc_filters_ctl.gmd_cfa.digital_gain_multiplier_green = 128;
        //publicData->apc_filters_ctl.gmd_cfa.digital_gain_shift_green = 7;
        //gk_isp_set_gmd_cfa(processor, &(publicData->apc_filters_ctl.gmd_cfa));
        //publicData->apc_filters_ctl.gmd_cfa_update = 0;
    }

    if (publicData->apc_filters_ctl.pmv_update) {
        publicData->apc_filters_ctl.pmv.bpc_cor_enable = 1;
        publicData->apc_filters_ctl.pmv.psc_cor_enable = 1;
        publicData->apc_filters_ctl.pmv.lea_expand_enable = 1;
        publicData->apc_filters_ctl.pmv.dpf_exphor1_enable = 1;
        //publicData->apc_filters_ctl.pmv.dpf_exphor2_enable = 1;
        publicData->apc_filters_ctl.pmv.dpf_expver_enable = 1;
        publicData->apc_filters_ctl.pmv.psc_feedthrough  = 1;

        gk_isp_set_pmv(processor, &(publicData->apc_filters_ctl.pmv));
        publicData->apc_filters_ctl.pmv_update = 0;
    }
    if (publicData->apc_filters_ctl.globle_motion_filter_update) {
        publicData->apc_filters_ctl.globle_motion_filter.enable = 1;
        publicData->apc_filters_ctl.globle_motion_filter.gmd_sel = 0;

        gk_isp_set_globle_motion_filter(processor, &(publicData->apc_filters_ctl.globle_motion_filter));
        publicData->apc_filters_ctl.globle_motion_filter_update = 0;
    }
    //end

    if(publicData->apc_filters_ctl.rgb_yuv_matrix_update) {
        rgb_to_yuv_t r2y_matrix = publicData->gk_rgb2yuv[publicData->rgb2yuv_style];

        apc_set_color_conversion(processor, &r2y_matrix);
        gk_isp_set_rgb2yuv_matrix(processor, &r2y_matrix);
        publicData->apc_filters_ctl.rgb_yuv_matrix_update = 0;
    }

    if (pthread_mutex_trylock(&(publicData->d_tone_proc.dyn_tone_mutex)) == 0) {
        if (publicData->apc_filters_ctl.gamma_update) {
            gk_isp_set_tone_curve(processor, &(publicData->apc_filters_ctl.gamma_table));
            publicData->apc_filters_ctl.gamma_update = 0;
        }
        pthread_mutex_unlock(&(publicData->d_tone_proc.dyn_tone_mutex));
    }

    if (publicData->apc_filters_ctl.anti_aliasing_update) {
        gk_isp_set_anti_aliasing_filter(processor, publicData->apc_filters_ctl.anti_aliasing);
        publicData->apc_filters_ctl.anti_aliasing_update = 0;
    }

    if (afData->lens_ctrl.af_stat_config_update) {
        gk_isp_set_af_statistics_ex(processor, afData->lens_ctrl.af_stat_config,
            1);
        afData->lens_ctrl.af_stat_config_update = 0;
    }

    if (afData->lens_ctrl.focus_update) {
        if (afData->lens_ctrl.focus_pulse > 0) {
            if (afData->lens_drv.af_focus_f)
                afData->lens_drv.af_focus_f(afData->lens_ctrl.pps,
                    afData->lens_ctrl.focus_pulse);
        } else {
            if (afData->lens_drv.af_focus_n)
                afData->lens_drv.af_focus_n(afData->lens_ctrl.pps,
                    (-afData->lens_ctrl.focus_pulse));
        }
    }

    if (afData->lens_ctrl.zoom_update) {
        if (afData->lens_ctrl.zoom_pulse > 0) {
            if (afData->lens_drv.zoom_out)
                afData->lens_drv.zoom_out(afData->lens_ctrl.pps, afData->lens_ctrl.zoom_pulse);
        } else {
            if (afData->lens_drv.zoom_in)
                afData->lens_drv.zoom_in(afData->lens_ctrl.pps,
                    (-afData->lens_ctrl.zoom_pulse));
        }
    }
    if ((afData->lens_ctrl.zoom_update == 0) && (afData->lens_ctrl.focus_update == 0)
        && (z_run == 0) && (f_run == 0)) {
        if (afData->lens_drv.af_lens_standby)
            afData->lens_drv.af_lens_standby(1);
    }

    afData->lens_ctrl.focus_update = 0;
    afData->lens_ctrl.zoom_update = 0;
}
#else
void isp_control_loop(void *arg)
{
    Isp3AProcessor *processor = NULL;
    aaa_api_t *isp3a = NULL;
    //int fd_media;
    int count;
    adj_param_t *p_apc_param_valid = NULL;
    int z_run = -1;
    int f_run = -1;
    u8 ae_stab_flag = 0;
    int ae_step = 0;
    int ae_flag = 0;
    int max_gain_index, cur_gain_index;
    int cur_luma;
    //int vps_i = 0;
    //int ae_target_vps;
    //int ae_luma_vps;
    int mv_info_blks;
    u8  sharp_level_mode = 0; //choose level control
    //SYS_DRV_SourceBufFormatT buffer_format;
    ImageOperate *op;
    Isp3APublicContent  *publicData;
    Isp3AContentAWB *awbData;
    Isp3AContentAF *afData;

    processor = (Isp3AProcessor *)arg;
    isp3a    = &(processor->isp3A);
    //fd_media = processor->content.publicData.fd;
    op = &(processor->operate);
    publicData = &(processor->content.publicData);
    awbData = &(processor->content.AWBData);
    afData = &(processor->content.AFData);



    //printf("loop...\n");
    // TODO: Get statistics info for ISP
    if (gk_isp_get_statistics(processor))
        return;

    if (awbData->apply_ls_awb_enable) {
        apply_ls_gain_to_awb_statis(processor);
    }

    // TODO: externel message process
    if (pthread_mutex_trylock(&(publicData->msg_mutex)) == 0)     //unlocked
    {
        count = 0;
        while (publicData->msg_update > 0) {
            gk_isp_msg_handler(processor, &(publicData->msg[count]), &(publicData->apc_filters_ctl));
            //    printf("process msg_update %d ,count %d\n", msg_update, count);
            publicData->msg_update--;
            count++;
        }
        pthread_mutex_unlock(&(publicData->msg_mutex));
    }
    if (publicData->adj_focus_statistics_enable == 1)
        pthread_cond_signal(&(publicData->statCond));

    pthread_cond_signal(&(publicData->d_tone_proc.dyn_tone_cond));
//printf("pthread cond...ok\n");
    if (pthread_mutex_trylock(&(publicData->update_mutex)) == 0)  //unlocked
    {
        // TODO: AE Control
        if (publicData->cntl_3a.ae_enable)
        {
            if (isp3a->p_ae_control)
            {
                //printf("[%s %d](%d)start\n", __func__, __LINE__, processor->srcId);
                isp3a->p_ae_control(processor, publicData->st_ae_info,
                        publicData->act_tile_num.ae_tile,
                        &(publicData->ae_video_info),
                         &(publicData->ae_still_info));
                //printf("[%s %d](%d)end\n", __func__, __LINE__, processor->srcId);
            }
        }

        // TODO: AWB Control
        if (publicData->cntl_3a.awb_enable)
            if (isp3a->p_awb_control)
            {
                //printf("[%s %d](%d)start\n", __func__, __LINE__, processor->srcId);
                isp3a->p_awb_control(processor, awbData->awb_algo, awbData->awb_mode,
                    publicData->act_tile_num.awb_tile, publicData->st_awb_info, &awbData->awb_gain);
                //printf("[%s %d](%d)end\n", __func__, __LINE__, processor->srcId);
            }
        pthread_mutex_unlock(&(publicData->update_mutex));
    }


    // TODO: AF Control
    if (publicData->cntl_3a.af_enable) {
        if (afData->lens_drv.check_isFocusRuning)
            f_run = afData->lens_drv.check_isFocusRuning();
        if (afData->lens_drv.check_isZoomRuning)
            z_run = afData->lens_drv.check_isZoomRuning();

        if (isp3a->p_af_control && afData->af_ctrl_param.zoom_status == 0)
        //if (isp3a->p_af_control)
        {
            isp3a->p_af_control(processor, &(afData->af_ctrl_param), publicData->act_tile_num.af_tile,
               publicData->st_af_cfa_info, publicData->act_tile_num.awb_tile, publicData->st_awb_info,
                &(afData->lens_ctrl), &(publicData->ae_video_info), (z_run || f_run), publicData->st_af_rgb_info);
        }
     }


    // TODO: Sync ISP params.
    if (pthread_mutex_trylock(&(publicData->load_isp_param_mutex)) == 0)      //unlocked
    {
        if (publicData->dynamic_load_param_sign == 1) {
            op->memcpy(&(publicData->gk_apc_param), publicData->gk_apc_param_tmp, sizeof(adj_param_t));
            op->memcpy(&(publicData->gk_apc_param_ir), publicData->gk_apc_param_ir_tmp,sizeof(adj_param_t));
            op->memcpy(&(publicData->gk_rgb2yuv), publicData->gk_rgb2yuv_tmp, sizeof(rgb_to_yuv_t)*4);
            op->memcpy(&(publicData->gk_uv_scale), publicData->gk_uv_scale_tmp,sizeof(chroma_scale_filter_t));
            op->memcpy(publicData->is_manual_LE, publicData->is_manual_LE_tmp,sizeof(local_wdr_t)*3);
            publicData->dynamic_load_param_sign = 0;
            op->free(publicData->gk_apc_param_tmp);
            op->free(publicData->gk_apc_param_ir_tmp);
            op->free(publicData->gk_rgb2yuv_tmp);
            op->free(publicData->gk_uv_scale_tmp);
            op->free(publicData->is_manual_LE_tmp);
            publicData->gk_apc_param_tmp = NULL;
            publicData->gk_apc_param_ir_tmp = NULL;
            publicData->gk_rgb2yuv_tmp = NULL;
            publicData->gk_uv_scale_tmp = NULL;
            publicData->is_manual_LE_tmp = NULL;
        }
        pthread_mutex_unlock(&(publicData->load_isp_param_mutex));
    }

    max_gain_index = publicData->sensor_max_db * 128 / 6;
    cur_gain_index = isp3a->p_ae_get_system_gain(processor);
    cur_luma  =  ae_get_luma_val(processor, publicData->st_ae_info, publicData->act_tile_num.ae_tile) / 16;
    ae_step = abs(ae_get_step(processor));

   //printf("cur_gain:%d  pre_gain :%d  cur_luma: %d  pre_luma :%d  \n",cur_gain_index,pre_gain_index,cur_luma,pre_luma);
    if(((ae_check_stable(processor) == 1) && cur_gain_index != max_gain_index) ||
        (((ae_check_stable(processor) == 0) && ((cur_gain_index == max_gain_index) || (ae_step <= 1)))))
    {
        ae_flag = 0;
    }
    else
        ae_flag = 1;
    if(publicData->bw_mode_enable)
        ae_flag = 0;
    if(ae_flag == 0 && publicData->pre_ae_flag == 1)
    {
        publicData->apc_filters_ctl.pmv_update = 1;
        publicData->apc_filters_ctl.bilateral_filter_update = 1;
        publicData->apc_filters_ctl.globle_motion_filter_update = 1;
    }
    ae_flag = 0;
    publicData->pre_luma = cur_luma;
    publicData->pre_gain_index = cur_gain_index;
    publicData->pre_ae_flag = ae_flag;
    if(publicData->mv_info_blks < publicData->apc_filters_ctl.globle_motion_filter.mv_percent_th1)
    {
        publicData->weak_move_count ++;
    }
    else
    {
        publicData->weak_move_count = 0;
    }
    if(publicData->weak_move_count > publicData->apc_filters_ctl.globle_motion_filter.mv_count)
        publicData->weak_move_count = publicData->apc_filters_ctl.globle_motion_filter.mv_count + 1;

    if(publicData->weak_move_count > publicData->apc_filters_ctl.globle_motion_filter.mv_count &&
            publicData->weak_movement_mode_enable == 0)
    {
        publicData->weak_movement_mode_enable = 1;
        processor->content.ctlData.apc_nf_reset_sign = 1;
    }
    if(publicData->weak_movement_mode_enable == 1 && (publicData->mv_info_blks > (publicData->apc_filters_ctl.globle_motion_filter.mv_percent_th1
            +  publicData->apc_filters_ctl.globle_motion_filter.mv_delta)))
    {
        publicData->weak_movement_mode_enable = 0;
        publicData->move_switch_count = 1;
        processor->content.ctlData.apc_nf_reset_sign = 1;
    }

    //publicData->apc_filters_ctl.globle_motion_filter.mv_percent_th3 = (1<<14);
    if(publicData->move_switch_count >= 1 && publicData->move_switch_count < 11)
        publicData->move_switch_count++;
    else if(publicData->move_switch_count == 10)
        publicData->move_switch_count = 0;

    if (publicData->cntl_3a.adj_enable)
    {
        ae_stab_flag = ae_check_stable(processor);

        if (publicData->bw_mode_enable)
            p_apc_param_valid = &(publicData->gk_apc_param_ir);
        else
        {
            if(publicData->weak_movement_mode_enable)
                p_apc_param_valid = &(publicData->gk_apc_param_weak_move);
            else
            {
                int th1, th3;

                th1 = (publicData->apc_filters_ctl.globle_motion_filter.mv_percent_th1*publicData->mv_info_blks_total)>>14;
                th3 = (publicData->apc_filters_ctl.globle_motion_filter.mv_percent_th3*publicData->mv_info_blks_total)>>14;
                //printf("th1:%d, th3:%d\n", th1, th3);
                if((publicData->mv_info_blks < th3) && (publicData->mv_info_blks > th1))
                {

                    if(abs(publicData->mv_info_blks - publicData->mv_info_blks_pre) > 100 ||
                        abs(publicData->mv_info_blks - publicData->mv_info_blks_pre_pre) > 100)
                    {
                        ic_dynamic_param_interp(processor, publicData->mv_info_blks);
                        processor->content.ctlData.apc_nf_reset_sign = 1;
                    }
                    p_apc_param_valid = &(publicData->gk_apc_param_interp);

                    if(publicData->parameters_status == 1)
                    {
                        publicData->parameters_status = 0;
                        processor->content.ctlData.apc_nf_reset_sign = 1;
                    }
                }
                else
                {
                    if(publicData->mv_info_blks < th1)
                    {
                        if(publicData->parameters_status == 0)
                            p_apc_param_valid = &(publicData->gk_apc_param_interp);
                        else
                            p_apc_param_valid = &(publicData->gk_apc_param);
                    }
                    else
                    {
                        p_apc_param_valid = &(publicData->gk_apc_param);
                        if(publicData->parameters_status == 0)
                        {
                            publicData->parameters_status = 1;
                            processor->content.ctlData.apc_nf_reset_sign = 1;
                        }
                    }
                }
            }
        }
        publicData->mv_info_blks_pre_pre = publicData->mv_info_blks_pre;
        publicData->mv_info_blks_pre = publicData->mv_info_blks;
        apc_video_aeawb_control(processor, isp3a->p_ae_get_system_gain(processor),
            &(p_apc_param_valid->awbae), &(publicData->apc_aeawb_ctl));
        apc_video_blc_control(processor, isp3a->p_ae_get_system_gain(processor),
            awbData->awb_gain.video_wb_gain, p_apc_param_valid, &(publicData->apc_filters_ctl));
        apc_video_aeawb_cntl(processor, isp3a->p_ae_get_system_gain(processor),
            awbData->awb_gain.video_wb_gain, p_apc_param_valid, &(publicData->apc_filters_ctl),
            &(publicData->gk_uv_scale));
        apc_video_nf_control(processor, isp3a->p_ae_get_system_gain(processor),
            awbData->awb_gain.video_wb_gain, 0, p_apc_param_valid, &(publicData->apc_filters_ctl));
        apc_contrast_for_ll(processor, p_apc_param_valid, &(publicData->apc_filters_ctl), ae_stab_flag,
            publicData->is_manual_LE);
        apc_yuv_extra_brightness(processor, isp3a->p_ae_get_system_gain(processor),
            &(publicData->apc_filters_ctl));
        //apc_encode_realtime_control(processor, isp3a->p_ae_get_system_gain(processor),
         //    p_apc_param_valid, &(publicData->apc_filters_ctl));
    }
    if (publicData->apc_aeawb_ctl.aeawb_update) {
        ae_set_target_val(processor, publicData->apc_aeawb_ctl.ae_target);
        awb_set_wb_ratio(processor, &(publicData->apc_aeawb_ctl));
        publicData->apc_aeawb_ctl.aeawb_update = 0;
    }

    if (publicData->apc_filters_ctl.local_wdr_update) {
        gk_isp_set_wdr(processor, &(publicData->apc_filters_ctl.local_wdr));
        publicData->apc_filters_ctl.local_wdr_update = 0;
    }
    if (publicData->apc_filters_ctl.black_corr_update) {
        gk_isp_set_final_blc(processor, &(publicData->apc_filters_ctl.black_corr),
            publicData->sensor_pattern);
        publicData->apc_filters_ctl.black_corr_update = 0;
    }
    if (publicData->apc_filters_ctl.chroma_scale_update) {
        gk_isp_set_uv_scale(processor, &(publicData->apc_filters_ctl.uv_scale));
        publicData->apc_filters_ctl.chroma_scale_update = 0;
    }
    if (publicData->apc_filters_ctl.cc_matrix_update) {
        gk_isp_set_cc(processor, &(publicData->apc_filters_ctl.color_corr));
        publicData->apc_filters_ctl.cc_matrix_update = 0;
    }
    if (publicData->apc_filters_ctl.badpix_corr_update) {
        gk_isp_set_auto_bad_pixel_correction(processor,
            &(publicData->apc_filters_ctl.badpix_corr_strength));
        publicData->apc_filters_ctl.badpix_corr_update = 0;
    }
    if (publicData->apc_filters_ctl.chroma_median_filter_update) {
        gk_isp_set_color_denoise_filter(processor,
            &(publicData->apc_filters_ctl.chroma_median_filter));
        publicData->apc_filters_ctl.chroma_median_filter_update = 0;
    }
    if (publicData->apc_filters_ctl.cfa_filter_update) {
        gk_isp_set_cfa_denoise_filter(processor, &(publicData->apc_filters_ctl.cfa_filter));
        publicData->apc_filters_ctl.cfa_filter_update = 0;
    }
    /*
    if (publicData->apc_filters_ctl.spatial_filter_update) {
        gk_isp_set_spatial_filter(processor, 0,
            &(publicData->apc_filters_ctl.spatial_filter));
        publicData->apc_filters_ctl.spatial_filter_update = 0;
    }*/
    if (publicData->apc_filters_ctl.sharp_level_minimum_update) {
        gk_isp_set_sharpen_lm(processor, IMG_VIDEO,
            &(publicData->apc_filters_ctl.sharp_level_minimum));
        publicData->apc_filters_ctl.sharp_level_minimum_update = 0;
    }

    if (publicData->apc_filters_ctl.sharp_level_overall_update) {

        if(publicData->apc_filters_ctl.sharp_fir.fir_strength >0 &&
            publicData->apc_filters_ctl.sharp_fir.fir_coeff[4] == 1)
            sharp_level_mode = 1;


        gk_isp_set_sharpen_level(processor, IMG_VIDEO,
            sharp_level_mode,
            &(publicData->apc_filters_ctl.sharp_level_overall));
        publicData->apc_filters_ctl.sharp_level_overall_update = 0;
    }
    if (publicData->apc_filters_ctl.high_freq_noise_reduc_update) {
        gk_isp_set_demosaic_noise_reduction(processor,
            publicData->apc_filters_ctl.high_freq_noise_reduc);
        publicData->apc_filters_ctl.high_freq_noise_reduc_update = 0;
    }
    if (publicData->apc_filters_ctl.sharp_retain_update) {
        gk_isp_set_sharpen_change_str(processor, IMG_VIDEO, publicData->apc_filters_ctl.sharp_retain);
        publicData->apc_filters_ctl.sharp_retain_update = 0;
    }
    if (publicData->apc_filters_ctl.sharp_max_change_update) {
        gk_isp_set_sharp_max_change(processor, IMG_VIDEO,
            &(publicData->apc_filters_ctl.sharp_max_change));
        publicData->apc_filters_ctl.sharp_max_change_update = 0;
    }
    if (publicData->apc_filters_ctl.sharp_fir_update || publicData->apc_filters_ctl.spatial_filter_update) {
        publicData->apc_filters_ctl.sharp_fir.edge_disp_enable = 0;
        publicData->apc_filters_ctl.sharp_fir.edge_dir_sel     = 0;
        publicData->apc_filters_ctl.sharp_fir.edge_dir_shift   = 0;
        publicData->apc_filters_ctl.sharp_fir.edge_dir_info    = 0;
        gk_isp_set_sharpen_template(processor, IMG_VIDEO,
            &(publicData->apc_filters_ctl.sharp_fir),&(publicData->apc_filters_ctl.spatial_filter));
        gk_isp_set_spatial_filter(processor, 0,
            &(publicData->apc_filters_ctl.spatial_filter));
        publicData->apc_filters_ctl.sharp_fir_update = 0;
        publicData->apc_filters_ctl.spatial_filter_update = 0;
    }
    if (publicData->apc_filters_ctl.sharp_coring_update) {
        gk_isp_set_sharpen_edge_str(processor, IMG_VIDEO,
            &(publicData->apc_filters_ctl.sharp_coring),
            &(publicData->apc_filters_ctl.sharp_coring_mv));
        publicData->apc_filters_ctl.sharp_coring_update = 0;
    }

//added by Hu Yin 22-04-2016

    //printf("[gk_isp_cntl_loop] vps_contrast_enable: %d\n", vps_contrast_enable);
    if(publicData->isp_capability.contrast_enable)
    {
    }
    // end
    //added by Hu Yin 15-08-2017 for GK7202
    if (publicData->apc_filters_ctl.depurple_fringe_update) {
        publicData->apc_filters_ctl.depurple_fringe.depurple_fringe_enable = 1;
        publicData->apc_filters_ctl.depurple_fringe.bpattern = publicData->sensor_pattern;
        gk_isp_set_depurple_fringe(processor,
            &(publicData->apc_filters_ctl.depurple_fringe));
        publicData->apc_filters_ctl.depurple_fringe_update = 0;
    }

    if (publicData->apc_filters_ctl.nlm_noise_filter_update) {
        publicData->apc_filters_ctl.nlm_noise_filter.nlm_noise_filter_enable = 1;
        publicData->apc_filters_ctl.nlm_noise_filter.nlm_noise_filter_mv_enable = 1;
        gk_isp_set_nlm_noise_filter(processor,
            &(publicData->apc_filters_ctl.nlm_noise_filter));
        publicData->apc_filters_ctl.nlm_noise_filter_update = 0;
    }

    if (publicData->apc_filters_ctl.variance_guided_filter_update) {
        publicData->apc_filters_ctl.variance_guided_filter.vgf_enable     = 1;
        publicData->apc_filters_ctl.variance_guided_filter.vgf_bpc_enable = 1;
        publicData->apc_filters_ctl.variance_guided_filter.vgf_mv_enable  = 1;
        publicData->apc_filters_ctl.variance_guided_filter.vgf_edge_wide_weight = 0;
        publicData->apc_filters_ctl.variance_guided_filter.vgf_edge_narrow_weight = 8;
        publicData->apc_filters_ctl.variance_guided_filter.vgf_disp_info_en = 0;
        publicData->apc_filters_ctl.variance_guided_filter.vgf_disp_var_th1 = 0;
        publicData->apc_filters_ctl.variance_guided_filter.vgf_disp_var_th2 = 1023;
        publicData->apc_filters_ctl.variance_guided_filter.vgf_disp_grad_th1 = 0;
        publicData->apc_filters_ctl.variance_guided_filter.vgf_disp_grad_th2 = 255;

        gk_isp_set_variance_guided_filter(processor,
            &(publicData->apc_filters_ctl.variance_guided_filter));
        publicData->apc_filters_ctl.variance_guided_filter_update = 0;
    }

    if (publicData->apc_filters_ctl.demosaic_noise_reduction_update) {
        publicData->apc_filters_ctl.demosaic_noise_reduction.thresh_grad_clipping = 512;
        publicData->apc_filters_ctl.demosaic_noise_reduction.shift = 7;
        publicData->apc_filters_ctl.demosaic_noise_reduction.demoire_A_Y = 4;
        publicData->apc_filters_ctl.demosaic_noise_reduction.demoire_B_Y = 4;
        publicData->apc_filters_ctl.demosaic_noise_reduction.demoire_C_Y = 4;
        publicData->apc_filters_ctl.demosaic_noise_reduction.demoire_D_Y = 4;
        publicData->apc_filters_ctl.demosaic_noise_reduction.demoire_A_C = 4;
        publicData->apc_filters_ctl.demosaic_noise_reduction.demoire_B_C = 4;
        publicData->apc_filters_ctl.demosaic_noise_reduction.demoire_C_C = 4;
        publicData->apc_filters_ctl.demosaic_noise_reduction.demoire_D_C = 4;
        publicData->apc_filters_ctl.demosaic_noise_reduction.conti_th1 = 4;
        publicData->apc_filters_ctl.demosaic_noise_reduction.conti_th2 = 8;
        publicData->apc_filters_ctl.demosaic_noise_reduction.conti_th3 = 24;
        publicData->apc_filters_ctl.demosaic_noise_reduction.conti_th4 = 8;

        gk_isp_set_demosaic_noise_reduction_new(processor,
            &(publicData->apc_filters_ctl.demosaic_noise_reduction));
        publicData->apc_filters_ctl.demosaic_noise_reduction_update = 0;
    }

    if (publicData->apc_filters_ctl.chroma_median_filter_ex_update) {
        publicData->apc_filters_ctl.chroma_median_filter_ex.ex_blur_data_u = 128;
        publicData->apc_filters_ctl.chroma_median_filter_ex.ex_blur_data_v = 128;
        publicData->apc_filters_ctl.chroma_median_filter_ex.ex_blur_en = 1;
        publicData->apc_filters_ctl.chroma_median_filter_ex.blur_nolimit_en = 1;

        gk_isp_set_chroma_median_filter_ex(processor,
            &(publicData->apc_filters_ctl.chroma_median_filter_ex));
        publicData->apc_filters_ctl.chroma_median_filter_ex_update = 0;
    }

    if (publicData->apc_filters_ctl.local_contrast_enhancement_update) {
        publicData->apc_filters_ctl.local_contrast_enhancement.lce_enable = 3;
        publicData->apc_filters_ctl.local_contrast_enhancement.pre_lut_en = 0;
        publicData->apc_filters_ctl.local_contrast_enhancement.pm_en = 0;
        publicData->apc_filters_ctl.local_contrast_enhancement.disp_mv_en = 0;
        publicData->apc_filters_ctl.local_contrast_enhancement.mv_scan_en = 1;
        gk_isp_set_local_contrast_enhancement(processor,
            &(publicData->apc_filters_ctl.local_contrast_enhancement));
        publicData->apc_filters_ctl.local_contrast_enhancement_update = 0;
    }
    //printf("ae_step :%d   ae_flag: %d  \n",ae_step,ae_flag);
    if (publicData->apc_filters_ctl.bilateral_filter_update || ae_flag) {
        publicData->apc_filters_ctl.bilateral_filter.y_enable = 1;
        publicData->apc_filters_ctl.bilateral_filter.c_enable = 1;
        publicData->apc_filters_ctl.bilateral_filter.use_mv   = 1;

        publicData->apc_filters_ctl.bilateral_filter.th_enable   = 1;
        publicData->apc_filters_ctl.bilateral_filter.min_mv_str   = 400;
        publicData->apc_filters_ctl.bilateral_filter.multi_coff   = 64;
        /*if(ae_flag == 1)
        {
            publicData->apc_filters_ctl.bilateral_filter.adj_enable   = 1;
            if(ae_step > 200)
            {
                publicData->apc_filters_ctl.bilateral_filter.multi_coff = 256;
            }
            else
               publicData->apc_filters_ctl.bilateral_filter.multi_coff = 256+(ae_step>>2)*50;
        }
        else
            publicData->apc_filters_ctl.bilateral_filter.adj_enable   = 0;*/

        gk_isp_set_bilateral_filter(processor, &(publicData->apc_filters_ctl.bilateral_filter));
        publicData->apc_filters_ctl.bilateral_filter_update = 0;
    }


    if (publicData->apc_filters_ctl.gmd_cfa_update ) {
        publicData->apc_filters_ctl.gmd_cfa.en = 1;
        publicData->apc_filters_ctl.gmd_cfa.gamma_en = 1;
        publicData->apc_filters_ctl.gmd_cfa.blur_en_cfa = 1;
        //publicData->apc_filters_ctl.gmd_cfa.digital_gain_multiplier_green = (processor->content.ctlData.adj_gmd_params.digital_gain_multiplier_green *
            //processor->content.publicData.ae_video_info.dgain + 512) >> 10;

        //printf("gmd_cfa_update digital_gain_multiplier_green %d\n", publicData->apc_filters_ctl.gmd_cfa.digital_gain_multiplier_green);

        //publicData->apc_filters_ctl.gmd_cfa.single_sad_en = 1;
        //publicData->apc_filters_ctl.gmd_cfa.cfa_interp_en = 1;
        //publicData->apc_filters_ctl.gmd_cfa.digital_gain_multiplier_green = 128;
        //publicData->apc_filters_ctl.gmd_cfa.digital_gain_shift_green = 7;
        //gk_isp_set_gmd_cfa(processor, &(publicData->apc_filters_ctl.gmd_cfa));
        //publicData->apc_filters_ctl.gmd_cfa_update = 0;
    }

    if (publicData->apc_filters_ctl.pmv_update /*|| ae_flag == 1*/) {
        publicData->apc_filters_ctl.pmv.bpc_cor_enable = 1;
        publicData->apc_filters_ctl.pmv.psc_cor_enable = 1;
        publicData->apc_filters_ctl.pmv.lea_expand_enable = 1;
        publicData->apc_filters_ctl.pmv.dpf_exphor1_enable = 1;
        //publicData->apc_filters_ctl.pmv.dpf_exphor2_enable = 1;
        publicData->apc_filters_ctl.pmv.dpf_expver_enable = 1;
        publicData->apc_filters_ctl.pmv.psc_feedthrough  = 1;

        /*if(ae_flag == 1)
        {
            publicData->apc_filters_ctl.pmv.lea_expand_enable = 0;
            publicData->apc_filters_ctl.pmv.dpf_exphor1_enable = 0;
            publicData->apc_filters_ctl.pmv.dpf_expver_enable = 0;
        }
        else
        {
            publicData->apc_filters_ctl.pmv.lea_expand_enable = 1;
            publicData->apc_filters_ctl.pmv.dpf_exphor1_enable = 1;
            publicData->apc_filters_ctl.pmv.dpf_expver_enable = 1;
        }*/
        gk_isp_set_pmv(processor, &(publicData->apc_filters_ctl.pmv));
        publicData->apc_filters_ctl.pmv_update = 0;
    }
    if (publicData->apc_filters_ctl.globle_motion_filter_update || ae_flag == 1) {
        publicData->apc_filters_ctl.globle_motion_filter.enable = 1;
        publicData->apc_filters_ctl.globle_motion_filter.gmd_sel = 0;
        publicData->apc_filters_ctl.globle_motion_filter.exp_str_uv   = 128;
        publicData->apc_filters_ctl.globle_motion_filter.exp_str_y   = 128;
        publicData->apc_filters_ctl.globle_motion_filter.th_enable   = 1;
        publicData->apc_filters_ctl.globle_motion_filter.alpha_th   = 192;
        publicData->apc_filters_ctl.globle_motion_filter.max_change_th   = 10;
        publicData->apc_filters_ctl.globle_motion_filter.alpha_multi_coff   = 64;
        publicData->apc_filters_ctl.globle_motion_filter.max_change_multi_coff   = 64;
        /*if(ae_flag == 1)
        {
            publicData->apc_filters_ctl.globle_motion_filter.adj_enable   = 1;
            if(ae_step > 20)
            {
                publicData->apc_filters_ctl.globle_motion_filter.alpha_multi_coff = 64;
                publicData->apc_filters_ctl.globle_motion_filter.max_change_multi_coff = 64;
            }
            else
            {
                publicData->apc_filters_ctl.globle_motion_filter.alpha_multi_coff = 256 - (ae_step>>2)*40;
                publicData->apc_filters_ctl.globle_motion_filter.max_change_multi_coff = 256 - (ae_step>>2)*40;
            }
         }
        else
            publicData->apc_filters_ctl.globle_motion_filter.adj_enable   = 0;*/
        gk_isp_set_globle_motion_filter(processor, &(publicData->apc_filters_ctl.globle_motion_filter));
        publicData->apc_filters_ctl.globle_motion_filter_update = 0;
    }
    //end

    if(publicData->apc_filters_ctl.rgb_yuv_matrix_update) {
        rgb_to_yuv_t r2y_matrix = publicData->gk_rgb2yuv[publicData->rgb2yuv_style];

        apc_set_color_conversion(processor, &r2y_matrix);
        gk_isp_set_rgb2yuv_matrix(processor, &r2y_matrix);
        publicData->apc_filters_ctl.rgb_yuv_matrix_update = 0;
    }

    if (pthread_mutex_trylock(&(publicData->d_tone_proc.dyn_tone_mutex)) == 0) {
        if (publicData->apc_filters_ctl.gamma_update) {
            gk_isp_set_tone_curve(processor, &(publicData->apc_filters_ctl.gamma_table));
            publicData->apc_filters_ctl.gamma_update = 0;
        }
        pthread_mutex_unlock(&(publicData->d_tone_proc.dyn_tone_mutex));
    }

    if (publicData->apc_filters_ctl.anti_aliasing_update) {
        gk_isp_set_anti_aliasing_filter(processor, publicData->apc_filters_ctl.anti_aliasing);
        publicData->apc_filters_ctl.anti_aliasing_update = 0;
    }

    if (afData->lens_ctrl.af_stat_config_update) {
        gk_isp_set_af_statistics_ex(processor, afData->lens_ctrl.af_stat_config,
            1);
        afData->lens_ctrl.af_stat_config_update = 0;
    }


   //if (afData->lens_ctrl.focus_update) {
    if (afData->lens_ctrl.focus_update && (afData->af_ctrl_param.zoom_status == 0)) {

       if (afData->lens_ctrl.focus_pulse > 0) {
           if (afData->lens_drv.af_focus_f)
               afData->lens_drv.af_focus_f(afData->lens_ctrl.pps,
                  afData->lens_ctrl.focus_pulse);
       }
       else {
           if (afData->lens_drv.af_focus_n)
               afData->lens_drv.af_focus_n(afData->lens_ctrl.pps,
                 (-afData->lens_ctrl.focus_pulse));
       }
    }

   //if (afData->lens_ctrl.zoom_update) {
   if (afData->lens_ctrl.zoom_update && afData->af_ctrl_param.zoom_status == 0) {
        if (afData->lens_ctrl.zoom_pulse < 0) {
            if (afData->lens_drv.zoom_out)
                afData->lens_drv.zoom_out(afData->lens_ctrl.pps,
                   -afData->lens_ctrl.zoom_pulse);
        }
        else {
            if (afData->lens_drv.zoom_in)
                afData->lens_drv.zoom_in(afData->lens_ctrl.pps,
                   (afData->lens_ctrl.zoom_pulse));
        }
    }
    if ((afData->af_ctrl_param.zoom_status == 0)
		&&
		(afData->lens_ctrl.zoom_update == 0) && (afData->lens_ctrl.focus_update == 0)
        && (z_run == 0) && (f_run == 0)) {
        if (afData->lens_drv.af_lens_standby)
            afData->lens_drv.af_lens_standby(1);
    }
    if (afData->af_ctrl_param.zoom_status == 0)
    {
        afData->lens_ctrl.focus_update = 0;
        afData->lens_ctrl.zoom_update = 0;
    }
}
#endif
int getTImeMS(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec/1000;
}

void set_sensor_ae_params(Isp3AProcessor *processor)
{
    //static u8 delay_sign = 0;
    Isp3APublicContent *content;
    Isp3AContentAWB *awbData;

    content = &(processor->content.publicData);
    awbData = &(processor->content.AWBData);
    gmd_cfa_t gmd_cfa_mv;

    #if 0
    if (content->ae_video_info.dgain_update == 1 || content->ae_video_info.shutter_update == 1 ||
        content->ae_video_info.agc_update == 1)
    {
        printf("=========================> ae awb id:(%d)\n", processor->srcId);
        printf("dgain:(%d)\n", content->ae_video_info.dgain);
        printf("shutter_index:(%d)\n", content->ae_video_info.shutter_index);
        printf("gain_index:(%d)\n", content->ae_video_info.gain_index);
    }
    #endif
    if (content->typeofsensor == 1)       // sht n+2,agc n+1 effective
    {
        if (content->ae_video_info.dgain_update == 1 || content->apc_filters_ctl.gmd_cfa_update == 1) {
            /*if (awbData->awb_gain.video_gain_update == 0) {
                gk_isp_set_ae_dgain(processor, content->ae_video_info.dgain);
                content->ae_video_info.dgain_update = 0;
            } else {*/
                if(content->ae_video_info.dgain_update == 1)
                {
                    //delay_sign = 1;
                    gk_isp_set_rgb_gain(processor, &awbData->awb_gain.video_wb_gain,
                        content->ae_video_info.dgain);
                    content->ae_video_info.dgain_update = 0;
                }
                gk_isp_get_gmd_cfa(processor,&gmd_cfa_mv);
                //gmd_cfa_mv.digital_gain_multiplier_green = (processor->content.ctlData.adj_gmd_params.digital_gain_multiplier_green *
                                                                                                                //content->ae_video_info.dgain + 512)>>10;
                //if(content->apc_filters_ctl.gmd_cfa_update == 1)
                {
                    content->apc_filters_ctl.gmd_cfa.en = 1;
                    content->apc_filters_ctl.gmd_cfa.gamma_en = 1;
                    content->apc_filters_ctl.gmd_cfa.blur_en_cfa = 1;
                    content->apc_filters_ctl.gmd_cfa.digital_gain_multiplier_green = (processor->content.ctlData.adj_gmd_params.digital_gain_multiplier_green *
                                                                                                                content->ae_video_info.dgain + 512)>>10;
                    int ae_step = ae_get_step(processor);
                    ae_step = (ae_step > 0) ? ae_step : -ae_step;

                    //content->apc_filters_ctl.gmd_cfa.noise_cfa = processor->content.ctlData.adj_gmd_params.noise_cfa + ((processor->content.ctlData.adj_gmd_params.noise_cfa*ae_step + 64)>>7);
                    gk_isp_set_gmd_cfa(processor,&(content->apc_filters_ctl.gmd_cfa));
                    content->apc_filters_ctl.gmd_cfa_update = 0;
                    //awbData->awb_gain.video_gain_update = 0;
                }
           // }
        }
        if (content->ae_video_info.shutter_update == 1) {
            ad_set_sensor_shutter_index(processor, content->ae_video_info.shutter_index);
            content->ae_video_info.shutter_update = 0;
            //printf("t=%d,sht %d\n",ap_get_tick(),ae_video_info.shutter_index);
            return;
        }
        if (content->ae_video_info.agc_update == 1) {
            ad_set_sensor_gain_index(processor, content->ae_video_info.gain_index,
                content->sensor_double_step);
            content->ae_video_info.agc_update = 0;
            //printf("t=%d,agc %d\n",ap_get_tick(),ae_video_info.gain_index);
        }
       /* if (content->ae_video_info.dgain_update == 1) {
            if (awbData->awb_gain.video_gain_update == 0) {
                gk_isp_set_ae_dgain(processor, content->ae_video_info.dgain);
                content->ae_video_info.dgain_update = 0;
            } else {
                gk_isp_set_rgb_gain(processor, &awbData->awb_gain.video_wb_gain,
                    content->ae_video_info.dgain);
                content->ae_video_info.dgain_update = 0;
                awbData->awb_gain.video_gain_update = 0;
            }
            //printf("t=%d,dgain %d\n",ap_get_tick(),ae_video_info.dgain);
        }*/
    } else if (content->typeofsensor == 2)        //sht, agc n+2 effective
    {
        //printf("t %d, shutter_update %d agc_update %d dgain_update %d gmd_cfa_update %d\n",getTImeMS(), content->ae_video_info.shutter_update,
            //content->ae_video_info.agc_update, content->ae_video_info.dgain_update, content->apc_filters_ctl.gmd_cfa_update);
#if 0
        if (content->ae_video_info.shutter_update == 1) {
            ad_set_sensor_shutter_index(processor, content->ae_video_info.shutter_index);
            content->ae_video_info.shutter_update = 0;
            content->delay_sign = 2;
        }
        if (content->ae_video_info.agc_update == 1) {
            ad_set_sensor_gain_index(processor, content->ae_video_info.gain_index,
                content->sensor_double_step);
            content->ae_video_info.agc_update = 0;
            if (content->delay_sign == 0)
                content->delay_sign = 2;
        }
        if (content->ae_video_info.dgain_update == 1 || content->apc_filters_ctl.gmd_cfa_update == 1) {
            if (content->delay_sign != 0) {
                content->delay_sign --;
                printf("return.\n");
                //return;
            }
            /*if (awbData->awb_gain.video_gain_update == 0) {
                gk_isp_set_ae_dgain(processor, content->ae_video_info.dgain);
                content->ae_video_info.dgain_update = 0;
            } else {*/
                gk_isp_set_rgb_gain(processor, &awbData->awb_gain.video_wb_gain,
                    content->ae_video_info.dgain);
                content->ae_video_info.dgain_update = 0;

                //gk_isp_get_gmd_cfa(processor,&gmd_cfa_mv);
                //gmd_cfa_mv.digital_gain_multiplier_green = (processor->content.ctlData.adj_gmd_params.digital_gain_multiplier_green *
                                                                                                                //content->ae_video_info.dgain + 512)>>10;

                content->apc_filters_ctl.gmd_cfa.digital_gain_multiplier_green = (processor->content.ctlData.adj_gmd_params.digital_gain_multiplier_green *
                                                                                                                content->ae_video_info.dgain + 512)>>10;
                printf("t=%d, digital_gain_multiplier_green %d adj digital_gain_multiplier_green %d, dgain %d\n",getTImeMS(),content->apc_filters_ctl.gmd_cfa.digital_gain_multiplier_green,
                    processor->content.ctlData.adj_gmd_params.digital_gain_multiplier_green, content->ae_video_info.dgain);

                gk_isp_set_gmd_cfa(processor,&(content->apc_filters_ctl.gmd_cfa));
                content->apc_filters_ctl.gmd_cfa_update = 0;
                //awbData->awb_gain.video_gain_update = 0;
           // }
        }
#else
        if (content->ae_video_info.dgain_update == 1 || content->apc_filters_ctl.gmd_cfa_update == 1)
        {
            /*if (awbData->awb_gain.video_gain_update == 0) {
                gk_isp_set_ae_dgain(processor, content->ae_video_info.dgain);
                content->ae_video_info.dgain_update = 0;
            } else {*/
            if(content->ae_video_info.dgain_update == 1)
            {
                //delay_sign = 1;
                gk_isp_set_rgb_gain(processor, &awbData->awb_gain.video_wb_gain,
                    content->ae_video_info.dgain);
                content->ae_video_info.dgain_update = 0;
            }
            gk_isp_get_gmd_cfa(processor,&gmd_cfa_mv);
            //gmd_cfa_mv.digital_gain_multiplier_green = (processor->content.ctlData.adj_gmd_params.digital_gain_multiplier_green *
                                                                                                            //content->ae_video_info.dgain + 512)>>10;
            //if(content->apc_filters_ctl.gmd_cfa_update == 1)
            {
                content->apc_filters_ctl.gmd_cfa.en = 1;
                content->apc_filters_ctl.gmd_cfa.gamma_en = 1;
                content->apc_filters_ctl.gmd_cfa.blur_en_cfa = 1;
                content->apc_filters_ctl.gmd_cfa.digital_gain_multiplier_green = (processor->content.ctlData.adj_gmd_params.digital_gain_multiplier_green *
                                                                                                            content->ae_video_info.dgain + 512)>>10;
                int ae_step = ae_get_step(processor);
                ae_step = (ae_step > 0) ? ae_step : -ae_step;

                //content->apc_filters_ctl.gmd_cfa.noise_cfa = processor->content.ctlData.adj_gmd_params.noise_cfa + ((processor->content.ctlData.adj_gmd_params.noise_cfa*ae_step + 64)>>7);
                gk_isp_set_gmd_cfa(processor,&(content->apc_filters_ctl.gmd_cfa));
                content->apc_filters_ctl.gmd_cfa_update = 0;
                //awbData->awb_gain.video_gain_update = 0;
            }
           // }
        }

        if (content->ae_video_info.shutter_update == 1) {
            if (content->delay_sign != 0) {
                content->delay_sign --;
                printf("shutter_update return.\n");
                return;
            }
            ad_set_sensor_shutter_index(processor, content->ae_video_info.shutter_index);
            content->ae_video_info.shutter_update = 0;
            content->delay_sign = 0;
        }

        if (content->ae_video_info.agc_update == 1) {
            if (content->delay_sign != 0) {
                content->delay_sign --;
                printf("agc_update return.\n");
                return;
            }
            ad_set_sensor_gain_index(processor, content->ae_video_info.gain_index,
                content->sensor_double_step);
            content->ae_video_info.agc_update = 0;
            content->delay_sign = 0;
        }
#endif
    } else if (content->typeofsensor == 3)        //sht, agc n+1 effective
    {
        if (content->ae_video_info.dgain_update == 1 || content->apc_filters_ctl.gmd_cfa_update == 1) {
            /*if (awbData->awb_gain.video_gain_update == 0) {
                gk_isp_set_ae_dgain(processor, content->ae_video_info.dgain);
                content->ae_video_info.dgain_update = 0;
            } else {*/
                if(content->ae_video_info.dgain_update == 1)
                {
                    //delay_sign = 1;
                    gk_isp_set_rgb_gain(processor, &awbData->awb_gain.video_wb_gain,
                        content->ae_video_info.dgain);
                    content->ae_video_info.dgain_update = 0;
                }
                gk_isp_get_gmd_cfa(processor,&gmd_cfa_mv);
                //gmd_cfa_mv.digital_gain_multiplier_green = (processor->content.ctlData.adj_gmd_params.digital_gain_multiplier_green *
                                                                                                                //content->ae_video_info.dgain + 512)>>10;
                //if(content->apc_filters_ctl.gmd_cfa_update == 1)
                {
                    content->apc_filters_ctl.gmd_cfa.en = 1;
                    content->apc_filters_ctl.gmd_cfa.gamma_en = 1;
                    content->apc_filters_ctl.gmd_cfa.blur_en_cfa = 1;
                    content->apc_filters_ctl.gmd_cfa.digital_gain_multiplier_green = (processor->content.ctlData.adj_gmd_params.digital_gain_multiplier_green *
                                                                                                                content->ae_video_info.dgain + 512)>>10;
                    int ae_step = ae_get_step(processor);
                    ae_step = (ae_step > 0) ? ae_step : -ae_step;

                    //content->apc_filters_ctl.gmd_cfa.noise_cfa = processor->content.ctlData.adj_gmd_params.noise_cfa + ((processor->content.ctlData.adj_gmd_params.noise_cfa*ae_step + 64)>>7);
                    gk_isp_set_gmd_cfa(processor,&(content->apc_filters_ctl.gmd_cfa));
                    content->apc_filters_ctl.gmd_cfa_update = 0;
                    //awbData->awb_gain.video_gain_update = 0;
                }
           // }
        }
        if (content->ae_video_info.shutter_update == 1) {
            ad_set_sensor_shutter_index(processor, content->ae_video_info.shutter_index);
            content->ae_video_info.shutter_update = 0;
            //printf("t=%d,sht %d\n",ap_get_tick(),ae_video_info.shutter_index);
        }
        if (content->ae_video_info.agc_update == 1) {
            ad_set_sensor_gain_index(processor, content->ae_video_info.gain_index,
                content->sensor_double_step);
            content->ae_video_info.agc_update = 0;
            //printf("t=%d,agc %d\n",ap_get_tick(),ae_video_info.gain_index);
        }
        /*if (content->ae_video_info.dgain_update == 1) {
            if (awbData->awb_gain.video_gain_update == 0) {
                gk_isp_set_ae_dgain(processor, content->ae_video_info.dgain);
                content->ae_video_info.dgain_update = 0;
            } else {
                gk_isp_set_rgb_gain(processor, &awbData->awb_gain.video_wb_gain,
                    content->ae_video_info.dgain);
                content->ae_video_info.dgain_update = 0;
                awbData->awb_gain.video_gain_update = 0;
            }
            //printf("t=%d,dgain %d\n",ap_get_tick(),ae_video_info.dgain);
        }*/
    }
}

int isp_vi_init(void *arg)
{
    int ret = 0;
    Isp3AProcessor *processor = (Isp3AProcessor *)arg;

    ret = hal_init_vi_tick(processor);

    return ret;
}

void isp_vi_exit(void *arg)
{
    Isp3AProcessor *processor = (Isp3AProcessor *)arg;

    hal_deinit_vi_tick(processor);
}

void isp_vi_loop(void *arg)
{
    Isp3AProcessor *processor = (Isp3AProcessor *)arg;
    Isp3APublicContent *publicData = NULL;
    Isp3AContentAWB *awbData = NULL;
    //int ae_luma_stat;
    //static int gLast_rgb_luma_stat;
    //static int gLast_cfa_luma_stat;

    processor  = (Isp3AProcessor *)arg;
    publicData = &(processor->content.publicData);
    awbData    = &(processor->content.AWBData);

    hal_get_vi_tick(processor);
//#define _SENSOR_AND_ISP_GAIN_ACTIVE_TIME_test_
#if defined(_SENSOR_AND_ISP_GAIN_ACTIVE_TIME_test_)
    static int frame_cnt = 0;
    ae_info_t *ae_video_info;
    ae_video_info = &(processor->content.publicData.ae_video_info);

    //ae_luma_stat =
    //    ae_get_luma_val(processor, publicData->st_ae_info, publicData->act_tile_num.ae_tile) / 16;

    printf("timer: %8d, cfa_luma %3d, rgb_luma %8d\n", getTImeMS(),ae_luma_stat,processor->content.AEData.ae_rgb_lightness_val);

    if(!(frame_cnt++ % 20))
    {
        ae_video_info->agc_update = 1;
    }
    else
    {
    ae_video_info->gain_index ++;
       ae_video_info->agc_update = 0;
    }

    if(ae_video_info->gain_index >= 768)
       ae_video_info->gain_index = 0;

#endif
    //ae_luma_stat =
    //    ae_get_luma_val(processor, publicData->st_ae_info, publicData->act_tile_num.ae_tile) / 16;

    //if(abs(gLast_cfa_luma_stat - ae_luma_stat ) >= 4)
    //printf("t=%d, last_cfa_luma %d, cfa_luma %3d, last_rgb_luma %d, rgb_luma %8d\n", getTImeMS(),gLast_cfa_luma_stat, ae_luma_stat, gLast_rgb_luma_stat, processor->content.AEData.ae_rgb_lightness_val);

    //gLast_rgb_luma_stat = processor->content.AEData.ae_rgb_lightness_val;
    //gLast_cfa_luma_stat = ae_luma_stat;

    pthread_mutex_lock(&(publicData->update_mutex));
    //printf("[%s %d](%d)start\n", __func__, __LINE__, processor->srcId);
    set_sensor_ae_params(processor);

    #if 1
    if (awbData->awb_gain.video_gain_update == 1) {
        gk_isp_set_wb_gain(processor, &awbData->awb_gain.video_wb_gain);
        awbData->awb_gain.video_gain_update = 0;
    }
    #endif
    //printf("[%s %d](%d)start\n", __func__, __LINE__, processor->srcId);
    pthread_mutex_unlock(&(publicData->update_mutex));

    return;
}

int isp_monitor_init(void *arg)
{
    int *fd = (int *)arg;

    *fd = ap_listen_socket();
    if (*fd < 0)
        return -1;

    return 0;
}

void isp_monitor_exit(void *arg)
{
    int *fd = (int *)arg;

    close(*fd);
}

void isp_monitor_loop(void *arg)
{
    int *fd = (int *)arg;
    int num;
    int i;
    int src_id;
    Isp3AProcessor *processor;
    int buffer[128];
    int timeout = MSG_QUEUE_TIMEOUT; /* 10 ms */
    Isp3APublicContent *publicData;
    ImageOperate *op;

    num = ap_receive_msg(*fd, buffer);
    if (num > 0) {
        src_id = buffer[1];
        processor = monitor_handle_find_by_id(src_id);
        if (processor != NULL) {
            op = &(processor->operate);
            publicData = &(processor->content.publicData);
            if (publicData->msg_update > MSG_QUEUE_SIZE - 1) {
                timeout = MSG_QUEUE_TIMEOUT;
                while ((timeout-- > 0) &&
                    (publicData->msg_update > MSG_QUEUE_SIZE - 1))
                {
                    usleep(50000);
                }
                if (timeout <= 0)
                {
                    printf("isp_monitor_loop 1error MSG_QUEUE_TIMEOUT\n");
                    return;
                }
            }
            pthread_mutex_lock(&(publicData->msg_mutex));
            publicData->msg_update++;
            publicData->msg[publicData->msg_update - 1].msg_id = buffer[0];
            op->memcpy(publicData->msg[publicData->msg_update - 1].param, &buffer[2], num - 8);
            //    printf("accept %d\n", msg_update);
            pthread_mutex_unlock(&(publicData->msg_mutex));
        }

        //if src_id == 0xff, all of receive message.
        if (src_id == 0xff) {
            for (i = 0; i < MONITOR_HANDLE_LIST_MAX; i++)
            {
                if (monitorHandleList[i].handle != NULL)
                {
                    processor = monitorHandleList[i].handle;
                    op = &(processor->operate);
                    publicData = &(processor->content.publicData);
                    if (publicData->msg_update > MSG_QUEUE_SIZE - 1) {
                        timeout = MSG_QUEUE_TIMEOUT;
                        while ((timeout-- > 0) &&
                            (publicData->msg_update > MSG_QUEUE_SIZE - 1))
                        {
                            usleep(50000);
                        }
                        if (timeout <= 0)
                        {
                            printf("isp_monitor_loop 2error MSG_QUEUE_TIMEOUT\n");
                            return;
                        }
                    }
                    pthread_mutex_lock(&(publicData->msg_mutex));
                    publicData->msg_update++;
                    publicData->msg[publicData->msg_update - 1].msg_id = buffer[0];
                    op->memcpy(publicData->msg[publicData->msg_update - 1].param, &buffer[2], num - 8);
                    //    printf("accept %d\n", msg_update);
                    pthread_mutex_unlock(&(publicData->msg_mutex));
                }
            }
        }
    }
}

extern int adj_tone_curve_control(Isp3AProcessor *processor, u16 table_index, tone_curve_t * p_gamma_table,
        adj_param_t * p_adj_video_param);

void isp_contrast_loop(void *arg)
{
    //static u8 dynt_frames_skip = 0;
    u8 auto_mode = 0;
    tone_curve_t *p_gamma_table;
    ISP_RgbHistogramStatT *p_rgb_hist;
    u16 table_index_new;
    adj_param_t *p_apc_param_valid = NULL;
    Isp3AProcessor *processor;
    ImageOperate *op;
    aaa_api_t *ispOp;
    Isp3APublicContent  *publicData;

    processor = (Isp3AProcessor *)arg;
    op = &(processor->operate);
    ispOp = &(processor->isp3A);
    publicData = &(processor->content.publicData);
    p_gamma_table = &(publicData->apc_filters_ctl.gamma_table);
    p_rgb_hist = &(publicData->st_dsp_histo_info.rgb_histogram);

    pthread_mutex_lock(&(publicData->d_tone_proc.dyn_tone_mutex));
    pthread_cond_wait(&(publicData->d_tone_proc.dyn_tone_cond),
        &(publicData->d_tone_proc.dyn_tone_mutex));

    if (++publicData->dynt_frames_skip >= 10)
        publicData->dynt_frames_skip = 0;

    auto_mode = d_tone_curve_get_status(processor);
    if (auto_mode == 0) {
        if (!publicData->dynt_frames_skip) {
            table_index_new = ispOp->p_ae_get_system_gain(processor);

            if (table_index_new != publicData->table_index_old) {
                if (publicData->bw_mode_enable)
                    p_apc_param_valid = &(publicData->gk_apc_param_ir);
                else
                    p_apc_param_valid = &(publicData->gk_apc_param);

                if (adj_tone_curve_control(processor,
                    table_index_new, p_gamma_table, p_apc_param_valid))
                {
                    publicData->apc_filters_ctl.gamma_update = 1;
                }
            }
            if (publicData->apc_filters_ctl.gamma_update) {
                adj_set_tone_contrast_64(processor, p_gamma_table->tone_curve_green);
                if (!apc_get_contrast_tone(processor, p_gamma_table->tone_curve_green)) {
                    op->memcpy(p_gamma_table->tone_curve_blue,
                        p_gamma_table->tone_curve_green,
                        sizeof(u16) * TONE_CURVE_SIZE);
                    op->memcpy(p_gamma_table->tone_curve_red,
                        p_gamma_table->tone_curve_green,
                        sizeof(u16) * TONE_CURVE_SIZE);
                }
                publicData->table_index_old = table_index_new;
            }
        }
    } else if (auto_mode == 1) {
        if (!publicData->dynt_frames_skip) {
            if (d_tone_curve_control(processor, p_gamma_table, p_rgb_hist))
            {
                publicData->apc_filters_ctl.gamma_update = 1;
            }
        }
    }

    pthread_mutex_unlock(&(publicData->d_tone_proc.dyn_tone_mutex));
}

void isp_zoom_loop(void *arg)
{
    Isp3AProcessor *processor = NULL;
    aaa_api_t *isp3a = NULL;
    //int fd_media;
    int count;
    adj_param_t *p_apc_param_valid = NULL;
    int z_run = -1;
    int f_run = -1;
    u8 ae_stab_flag = 0;
    int ae_step = 0;
    int ae_flag = 0;
    int max_gain_index, cur_gain_index;
    int cur_luma;
    //int vps_i = 0;
    //int ae_target_vps;
    //int ae_luma_vps;
    int mv_info_blks;
    u8  sharp_level_mode = 0; //choose level control
    //SYS_DRV_SourceBufFormatT buffer_format;
    ImageOperate *op;
    Isp3APublicContent  *publicData;
    Isp3AContentAF *afData;

    processor = (Isp3AProcessor *)arg;
    isp3a    = &(processor->isp3A);
    //fd_media = processor->content.publicData.fd;
    op = &(processor->operate);
    publicData = &(processor->content.publicData);
    afData = &(processor->content.AFData);
    if (afData->af_ctrl_param.zoom_status == 0)
    {
        pthread_mutex_lock(&(publicData->af_zoom_mutex));
        pthread_cond_wait(&(publicData->af_zoom_cond), &(publicData->af_zoom_mutex));
        //struct timespec abstime;
        //abstime.tv_sec = xms / 1000;
        //abstime.tv_nsec = (xms % 1000) * 1000;
        //pthread_cond_timedwait(&(publicData->statCond), &(publicData->statMutexLock), &abstime);
        pthread_mutex_unlock(&(publicData->af_zoom_mutex));
    }
    if (publicData->cntl_3a.af_enable) {
       if (afData->lens_drv.check_isFocusRuning)
           f_run = afData->lens_drv.check_isFocusRuning();
       if (afData->lens_drv.check_isZoomRuning)
           z_run = afData->lens_drv.check_isZoomRuning();

       if (isp3a->p_af_control && (afData->af_ctrl_param.zoom_status == 1))
       {
           isp3a->p_af_control(processor, &(afData->af_ctrl_param), publicData->act_tile_num.af_tile,
               publicData->st_af_cfa_info, publicData->act_tile_num.awb_tile, publicData->st_awb_info,
                &(afData->lens_ctrl), &(publicData->ae_video_info), (z_run || f_run), publicData->st_af_rgb_info);
       }
    }

    if (afData->lens_ctrl.focus_update && (afData->af_ctrl_param.zoom_status == 1)) {

        if (afData->lens_ctrl.focus_pulse > 0) {
             if (afData->lens_drv.af_focus_f)
                 afData->lens_drv.af_focus_f(afData->lens_ctrl.pps,
                    afData->lens_ctrl.focus_pulse);
         } else {
             if (afData->lens_drv.af_focus_n)
                afData->lens_drv.af_focus_n(afData->lens_ctrl.pps,
                   (-afData->lens_ctrl.focus_pulse));
         }
    }

    if (afData->lens_ctrl.zoom_update  && (afData->af_ctrl_param.zoom_status == 1)) {
       if (afData->lens_ctrl.zoom_pulse < 0) {
           if (afData->lens_drv.zoom_out)
              afData->lens_drv.zoom_out(afData->lens_ctrl.pps,
                 -afData->lens_ctrl.zoom_pulse);
        } else {
           if (afData->lens_drv.zoom_in)
              afData->lens_drv.zoom_in(afData->lens_ctrl.pps,
                 (afData->lens_ctrl.zoom_pulse));
        }
    }

    if (afData->af_ctrl_param.zoom_status == 1)
    {
        afData->lens_ctrl.focus_update = 0;
        afData->lens_ctrl.zoom_update = 0;
    }
    if(afData->af_G_param.zoom_thread_sleep_time != 0)
    {
        usleep(afData->af_G_param.zoom_thread_sleep_time);
    }

}


//*****************************************************************************
//*****************************************************************************
//** Local Functions
//*****************************************************************************
//*****************************************************************************
