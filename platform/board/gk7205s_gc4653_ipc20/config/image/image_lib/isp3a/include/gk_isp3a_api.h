/*!
*****************************************************************************
** \file        software/subsystem/isp3a/include/gk_isp3a_api.h
**
** \version     $Id: gk_isp3a_api.h 3 2017-09-04 09:29:58Z fengxuequan $
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
#ifndef _GK_ISP3A_API_H_
#define _GK_ISP3A_API_H_

#include "basetypes.h"
#include "gk_isp.h"
#include "gk_isp3a_img_struct.h"
#include "gk_isp3a_dsp_api.h"


//*****************************************************************************
//*****************************************************************************
//** Defines and Macros
//*****************************************************************************
//*****************************************************************************



//*****************************************************************************
//*****************************************************************************
//** Enumerated types
//*****************************************************************************
//*****************************************************************************



//*****************************************************************************
//*****************************************************************************
//** Data Structures
//*****************************************************************************
//*****************************************************************************



//*****************************************************************************
//*****************************************************************************
//** Global Data
//*****************************************************************************
//*****************************************************************************



//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************



#ifdef __cplusplus
extern "C" {
#endif

// basic 3A related APIs

void gk_get_algo_lib_version(img_lib_version_t* p_img_algo_info );
int gk_lib_init(void);
int gk_lib_deinit(void);
void* gk_lib_create_ISP3AProcesser(int fd, int srcid);
int gk_lib_destroy_ISP3AProcesser(void *handle);

//int img_start_aaa_i1(int fd_media, char* proc_name);
int gk_start_3A(void *handle); //OK
int gk_stop_3A(void *handle);
int gk_get_3a_srcid(void *handle);
void gk_get_3A_ctl_state(Isp3AProcessor *processor, aaa_cntl_t *cntl);
int gk_enable_auto_param_ctl(Isp3AProcessor *processor, int enable);
int gk_enable_ae(Isp3AProcessor *processor, int enable);
int gk_enable_af(Isp3AProcessor *processor, int enable);
int gk_enable_awb(Isp3AProcessor *processor, int enable);

// lens and sensor related APIs
int gk_lens_init(Isp3AProcessor *processor);
int gk_config_lens_info(Isp3AProcessor *processor);
int gk_config_sensor_info(Isp3AProcessor *processor);
int img_config_denoise_info(Isp3AProcessor *processor, int mode);
int img_config_sensor_hdr_mode(Isp3AProcessor *processor, int hdr_mode);
int gk_load_sensor_param(Isp3AProcessor *processor, image_sensor_param_t* p_app_param_image_sensor);
int gk_auto_load_sensor_param(Isp3AProcessor *processor, image_sensor_param_t* p_app_param_image_sensor);
int gk_set_sensor_shutter_time_index(Isp3AProcessor *processor, u32 shutter_index);
int gk_get_sensor_shutter_time_index(Isp3AProcessor *processor);
int gk_set_sensor_gain_index(Isp3AProcessor *processor, u32 gain_index, u32 step);
int gk_get_sensor_gain_index(Isp3AProcessor *processor);

// adj related APIs
int gk_image_load_ccm(Isp3AProcessor *processor, u32 matrix_addr, u32 index);

// ae related APIs
void gk_get_iris_state(Isp3AProcessor *processor, dc_iris_cntl p_cntl);
u32 gk_ae_get_sensor_gain(Isp3AProcessor *processor);
int gk_ae_set_antiflicker(Isp3AProcessor *processor, int mode);        //0-50Hz, 1-60Hz
//added by Hu Yin 27-12-2016
int gk_ae_get_antiflicker(Isp3AProcessor *processor);        //0-50Hz, 1-60Hz
//end
int gk_ae_set_backlight(Isp3AProcessor *processor, int backlight_enable); //0-disable, 1-enable
int gk_ae_set_meter_mode(Isp3AProcessor *processor, ae_metering_mode mode);
int gk_ae_load_exp_params(Isp3AProcessor *processor, line_t* line, u16 line_num, u16 line_belt);    //lin_num <=10, line_belt <= lin_num, line_belt count from 1
int gk_ae_set_line_belt(Isp3AProcessor *processor, u16 line_belt);                    //line_belt <= line_num, line_belt count from 1
s16 gk_ae_get_line_site(Isp3AProcessor *processor);            //-1: <= start point, 0: in control, 1: >= end point
u16 gk_ae_get_target(Isp3AProcessor *processor);
u16 img_ae_set_target(Isp3AProcessor *processor, u16 target);

int gk_ae_set_target_param(Isp3AProcessor *processor, u16 ratio);        // unit = 1024
u16 gk_ae_get_target_ratio(Isp3AProcessor *processor);
int gk_set_ae_meter_table(Isp3AProcessor *processor, int * roi_table);
int gk_get_ae_cfa_luma_value(Isp3AProcessor *processor);
int gk_get_ae_rgb_luma_value(Isp3AProcessor *processor);
int gk_get_lens_shading_enable(Isp3AProcessor *processor);
int gk_set_lens_shading_enable(Isp3AProcessor *processor, int enable);
int gk_set_ae_speed(Isp3AProcessor *processor, u8 mode,u8 ae_speed_level);
int img_get_ae_speed(Isp3AProcessor *processor);

int gk_set_auto_luma_control_enable(Isp3AProcessor *processor, int enable);    //0: disable or 1: enable
int gk_set_ae_auto_luma_control(Isp3AProcessor *processor, u8 auto_luma_cntl);    // 0-255

u16 img_get_ae_auto_luma_control(Isp3AProcessor *processor);



// af related APIs
int gk_af_set_range(Isp3AProcessor *processor, af_range_t * p_af_range);
int gk_af_set_cntl_mode(Isp3AProcessor *processor, af_mode mode);
int gk_af_set_roi(Isp3AProcessor *processor, af_ROI_config_t* af_ROI);
int gk_af_set_zoom_idx(Isp3AProcessor *processor, u16 zoom_idx);
int gk_af_set_focus_idx(Isp3AProcessor *processor, s32 focus_idx);
int gk_af_set_reset(Isp3AProcessor *processor);
void gk_af_load_param(Isp3AProcessor *processor, void* gk_af_param, void* gk_zoom_map);

// awb related APIs
int gk_awb_set_env(Isp3AProcessor *processor, awb_environment_t env);
int gk_awb_set_cntl_mode(Isp3AProcessor *processor, awb_control_mode_t mode);
int gk_awb_set_algo(Isp3AProcessor *processor, awb_work_method_t method);
awb_work_method_t gk_awb_get_algo(Isp3AProcessor *processor);
awb_control_mode_t gk_awb_get_cntl_mode(Isp3AProcessor *processor);
int gk_awb_set_wb_diff(Isp3AProcessor *processor, wb_gain_t* p_wb_org_gain, wb_gain_t* p_wb_ref_gain);
void gk_awb_get_wb_cal_data(Isp3AProcessor *processor, wb_gain_t* p_wb_gain);
int gk_awb_set_speed(Isp3AProcessor *processor, u8 speed);
int img_awb_get_speed(Isp3AProcessor *processor);
int img_set_dewarp(Isp3AProcessor *processor,u8 h_enable, u8 h_str,
                        u8 v_enable, u8 v_str );
u8 img_awb_set_day_night_mode(Isp3AProcessor *processor, u8 en);
int img_awb_set_rb_ratio(Isp3AProcessor *processor, u8 mode, u8 ratio);
u8 img_awb_get_rb_ratio(Isp3AProcessor *processor, u8 *mode, u8 *ratio);

int gk_awb_set_cali_diff_thr(Isp3AProcessor *processor, u16 r_thr, u16 b_thr);
int gk_awb_set_custom_gain(Isp3AProcessor *processor, wb_gain_t* cus_gain);

// image property and color style related APIs
int gk_set_color_hue(Isp3AProcessor *processor, int hue);            //-15 - +15: -30deg - +30 deg
int gk_set_color_brightness(Isp3AProcessor *processor, int brightness);    //-255 - +255
int gk_set_vps_contrast(Isp3AProcessor *processor, int contrast); //unit = 64, 0 ~ 128

int gk_set_contrast(Isp3AProcessor *processor, int contrast);    //unit = 64, 0 ~ 128
int gk_set_auto_contrast(Isp3AProcessor *processor, int enable); // 0 = disable, 1 = auto;
int gk_set_auto_contrast_strength(Isp3AProcessor *processor, int strength); // 0~128, 0: no effect; 128: full effect
int gk_set_color_saturation(Isp3AProcessor *processor, int saturation);    //unit = 64
int gk_set_sharpness(Isp3AProcessor *processor, int level);        //unit = 128, 0 - 255
int img_set_vps_3d_strength(Isp3AProcessor *processor, int level); ////unit = 32, 0 - 512

int gk_set_auto_wdr(Isp3AProcessor *processor, int mode); //0 = disable, 1 = auto, 64 = 1X, 128 = 2X, 192 = 3X, 256 = 4X
void gk_get_img_attr(Isp3AProcessor *processor, image_property_t *prop);
int gk_set_image_style(Isp3AProcessor *processor, img_color_style mode);
img_color_style gk_get_image_style(Isp3AProcessor *processor);
int gk_set_day_night_mode(Isp3AProcessor *processor, u8 en); // 0: diable, 1: enable 2: auto
int gk_set_blc_e(Isp3AProcessor *processor, int blc_level);
int gk_get_blc_e(Isp3AProcessor *processor);
int gk_set_auto_spatial_filter_enable(Isp3AProcessor *processor, int enable);
int gk_set_auto_sharpen_enable(Isp3AProcessor *processor, int enable);
int gk_set_sharpen_str_ex(Isp3AProcessor *processor, int fir_ratio,int spatial_filter_ratio,
int dir_filter_ratio, int max_change_up_ratio, int max_change_down_ratio,
int edge_ratio);
int img_set_spatial_filter(Isp3AProcessor *processor, int cfa_flat_weight_ratio, int cfa_dir_weight_ratio,
int cfa_flat_thr_ratio, int cfa_dir_thr_ratio, int edge_thr_ratio);

int gk_set_3dnr_level_ex(Isp3AProcessor *processor, int level, int threshold_ratio, int max_change_ratio);
int img_set_vps_3d_enable(Isp3AProcessor *processor, int enable);

// still capture related APIs
int gk_init_image_capture(Isp3AProcessor *processor, int quality);
int gk_leave_image_capture(Isp3AProcessor *processor);
int gk_start_image_capture(Isp3AProcessor *processor, still_cap_info_t *still_cap_info);
int gk_stop_image_capture(Isp3AProcessor *processor);
int gk_image_capture_adv(Isp3AProcessor *processor);
//int gk_init_still_proc_from_memory(int fd_media, still_proc_mem_init_info_t* proc_info);
int gk_image_proc_from_memory(Isp3AProcessor *processor, still_proc_mem_info_t* proc_info);
int gk_stop_image_proc_mem(Isp3AProcessor *processor);
int gk_still_hdr_proc(Isp3AProcessor *processor, still_hdr_proc_t* proc_info);

// calibration related APIs
int gk_detect_bad_pixel(Isp3AProcessor *processor, cali_badpix_setup_t *pCali_badpix_setup);
int gk_cal_gyro(Isp3AProcessor *processor, cali_gyro_setup_t* in, gyro_calib_info_t* out);

// customerize 3A related APIs
int gk_register_3A_algorithm(Isp3AProcessor *processor, aaa_api_t custom_aaa_api);
int gk_get_3A_algorithm(Isp3AProcessor *processor, aaa_api_t *aaa_api_ptr);
int gk_register_lens_drv(Isp3AProcessor *processor, lens_dev_drv_t custom_lens_drv);
int gk_cali_lens_shading(vignette_cal_t *vig_detect_setup);
int img_dsp_set_aaa_statistic_frequency(Isp3AProcessor *processor, int freq);
// img work mode related APIs

int gk_set_awb_statis_gain_for_lens_shading(Isp3AProcessor *processor, u8 enable,u16* vig_awb_statis_gain,const u32 lookup_shift,const u16 tile_num_col,const u16 tile_num_row);
int gk_enable_yuv_extra_brightness(Isp3AProcessor *processor, u8 enable);

int gk_wait_image_capture(Isp3AProcessor *processor);
int gk_config_image_capture(Isp3AProcessor *processor, img_snapshot_config_t* config);

void * gk_image_tuning_proc(Isp3AProcessor *processor, void * argv);

void anti_Interpolation_bin(float M,float N,float A,float B,float Y,float Z,float X, float *resultA,float *resultB);
float interpolationY(float M,float N,float A,float B,float X);
void  binCaculate(float binA,float binB,float dBA,float dBB,float ExdB,float binZ,float *ReA,float *ReB);

//added by Hu Yin 08-03-2017
int gk_set_low_bitrate_mode(Isp3AProcessor *processor, u8 en);      // 0: diable, 1: enable
int gk_set_low_bitrate_param(Isp3AProcessor *processor, image_low_bitrate_property_t *image_low_bitrate,
                             u8 *tolerance_low_bitrate,
                             int *delay_frames_low_bitrate);

int gk_get_low_bitrate_param(Isp3AProcessor *processor, image_low_bitrate_property_t *image_low_bitrate,
                             u8 *tolerance_low_bitrate,
                             int *delay_frames_low_bitrate);
int gk_get_low_bitrate_mode(Isp3AProcessor *processor);
int gk_set_af_manual_status(Isp3AProcessor *processor, u8 manual_mode);

//end
//#endif

#ifdef __cplusplus
}
#endif

#endif    // _GK_ISP3A_API_H_

