/*!
*****************************************************************************
** \file        software/subsystem/isp3a/include/gk_isp3a_img_struct.h
**
** \version     $Id: gk_isp3a_img_struct.h 6 2017-09-07 07:17:07Z dengbiao $
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
#ifndef _GK_ISP3A_IMG_STRUCT_H_
#define _GK_ISP3A_IMG_STRUCT_H_
#include "gk_isp.h"
#include <pthread.h>
#include <semaphore.h>

//*****************************************************************************
//*****************************************************************************
//** Defines and Macros
//*****************************************************************************
//*****************************************************************************
#define MAX_AWB_TILE_COL        (32)
#define MAX_AWB_TILE_ROW        (32)
#define MAX_AWB_TILE_NUM        (MAX_AWB_TILE_COL*MAX_AWB_TILE_ROW)
#define MAX_AE_TILE_COL         (12)
#define MAX_AE_TILE_ROW         (8)
#define MAX_AE_TILE_NUM         (MAX_AE_TILE_COL*MAX_AE_TILE_ROW)
#define MAX_AF_TILE_COL         (8)
#define MAX_AF_TILE_ROL         (5)
#define MAX_AF_TILE_NUM         (MAX_AF_TILE_COL*MAX_AF_TILE_ROL)

typedef signed long long s64;

#define CC_3D_SIZE              (17536) //CC 3D file size
#define CC_REG_SIZE             (18752)  // CC REG file size

#define MAX_LINE_NUMBER         10
#define MSG_QUEUE_SIZE          32
#define MSG_QUEUE_TIMEOUT       10



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
typedef struct color_correction_s
{
    u32 matrix_3d_table_addr;
}color_correction_t;

typedef struct color_correction_reg_s
{
    u32 reg_setting_addr;
}color_correction_reg_t;

typedef struct grid_point_s
{
    s16 x;
    s16 y;
} grid_point_t;

typedef struct geometry_info_s
{
    u32 start_x;    // The location in sensor of output pixel (0, 0)
    u32 start_y;
    u32 width;
    u32 height;
    u32 aspect_ratio;
    u32 downsample;    // 1, 2, 4
} geometry_info_t;

#define    WARP_VER_1_0    0x20100209
typedef struct warp_grid_info_s
{
    u32 version;        // WARP TABLE VERSION
    int grid_w;        // Horizontal grid point number
    int grid_h;        // Vertical grid point number
    int tile_w;        // Tile width in pixels. MUST be power of 2.
    int tile_h;        // Tile height in pixels. MUST be power of 2.
    int img_w;        // Image width
    int img_h;        // Image width
    geometry_info_t    img_geo;    // Image geometry
    u32 reserved[3];    // reserved
    grid_point_t *warp;    // Warp grid vector arrey.
} warp_grid_info_t;

typedef struct crop_info_s
{
    u32 left_top_x;        // 16.16 format
    u32 left_top_y;        // 16.16 format
    u32 right_bot_x;    // 16.16 format
    u32 right_bot_y;    // 16.16 format
}crop_info_t;

typedef struct warp_control_info_s
{
    // Warp table
    warp_grid_info_t* warp1;
    warp_grid_info_t* warp2;
    u32 blendRatio;
    u32 decayRatio;
    // Warp function control
    u8 ver_warp_en;
    u8 ssr_dwnsmp_h;
    u8 ssr_dwnsmp_v;
    u8 reserved;
    u8 tbl_upsmp_h;
    u8 tbl_dwnsmp_h;
    u8 tbl_upsmp_v;
    u8 tbl_dwnsmp_v;
    s32 hor_skew_phase_inc;
    // Warp window info
    geometry_info_t *dummy;
    crop_info_t *tbl_crop;
    crop_info_t *ssr_crop;
} warp_control_info_t;

typedef struct warp_output_info_s
{
    u32 width;
    u32 height;
    u32 warp_grid_width;
    u32 warp_grid_height;
    u32 cfa_output_width;
    u32 cfa_output_height;
} warp_output_info_t;

typedef struct chroma_noise_reduction_s
{
    u16 chroma_strength;
    u16 chroma_radius;
    u8  chroma_adaptation;
    u8  mode;
}chroma_noise_reduction_t;

typedef struct dump_still_bin_s
{
/*type 1: user configure parameters*/
    u8 user_num;
    u32 user_size[64];
    u32 user_addr[64];
/*type 2: icore internal data*/
    u8 internal_num;
    u32 internal_size[4];
    u32 internal_addr[4];
/*type 3: image*/
    u8 image_num;
    u32 image_size[2];
    u32 image_addr[2];
}dump_still_bin_t;

typedef struct reduce_cc_one_channel_s
{
    u16 strength;
    u16 sat0;
    u16 sat1;
    u16 padding;
} reduce_cc_one_channel_t;

typedef struct reduced_color_correction_s
{
    reduce_cc_one_channel_t reduce_cc_info[2];
}reduced_color_correction_t;

typedef struct chroma_level_based_noise_reduction_s
{
    u8 cb_start;
    u8 cb_end;
    u8 cr_start;
    u8 cr_end;
}chroma_level_based_noise_reduction_t;

typedef struct luma_control_s
{
    u16 global_strength;
}luma_control_t;

typedef struct uma_level_based_noise_reduction_s
{
    u16 strength;
}luma_level_based_noise_reduction_t;

typedef struct luma_cntl_1_band_s
{
    u8 max_change_up;
    u8 max_change_down;
    u16 noise_reduce;
    u16 sharpen_strength_up;
    u16 sharpen_strength_down;
}luma_cntl_1_band_t;

typedef struct luma_freq_seperation_s
{
    luma_cntl_1_band_t luma_cntl[3]; //low, mid, high
}luma_freq_separation_t;

typedef struct luma_very_low_freq_filter_s
{
    u16 strength;
    u16 adaptation;
}luma_very_low_freq_filter_t;

typedef struct luma_detail_preservation_s
{
    u16 strength;
    u8  max_down;
    u8  max_up;
    u8  gb_gr_mismatch;
    u8  reserved[3];
}luma_detail_preservation_t;

typedef struct chroma_first_filter_s
{
    u16 strength_bright;
    u16 adaptation_bright;
    u16 strength_dark;
    u16 adaptation_dark;
    u8  max_change_bright;
    u8  max_change_dark;
    u16 padding;
}chroma_first_filter_t;

typedef struct chroma_second_filter_s
{
    u16 strength_bright;
    u16 adaptation_bright;
    u16 edge_adap_cb_bright;
    u16 edge_adap_cr_bright;
    u16 strength_dark;
    u16 adaptation_dark;
    u16 edge_adap_cb_dark;
    u16 edge_adap_cr_dark;
    u8  max_change_bright;
    u8  max_change_dark;
    u16 padding;
}chroma_second_filter_t;

typedef struct img_lib_version_s
{
    int    major;
    int    minor;
    int    patch;
    u32    mod_time;
    char    description[64];
}  img_lib_version_t;

typedef enum
{
    IMG_VIDEO = 0,
    IMG_FAST_STILL,
    IMG_LOWISO_STILL,
    IMG_HIGHISO_STILL,
    IMG_MODE_NUMBER,
}image_mode;

typedef enum
{
    WB_ENV_INDOOR = 0,
    WB_ENV_INCANDESCENT,
    WB_ENV_D4000,
    WB_ENV_D5000,
    WB_ENV_SUNNY,
    WB_ENV_CLOUDY,
    WB_ENV_FLASH,
    WB_ENV_FLUORESCENT,
    WB_ENV_FLUORESCENT_H,
    WB_ENV_UNDERWATER,
    WB_ENV_CUSTOM,
    WB_ENV_OUTDOOR,
    WB_ENV_AUTOMATIC,
    WB_ENV_NUMBER,
}awb_environment_t;

typedef enum
{
    WB_AUTOMATIC = 0,
    WB_INCANDESCENT,    // 2800K
    WB_D4000,
    WB_D5000,
    WB_SUNNY,        // 6500K
    WB_CLOUDY,        // 7500K
    WB_FLASH,
    WB_FLUORESCENT,
    WB_FLUORESCENT_H,
    WB_UNDERWATER,
    WB_CUSTOM,
    WB_OUTDOOR,
    WB_MODE_NUMBER,
}awb_control_mode_t;

typedef enum
{
    AWB_NORMAL = 0,
    AWB_CUSTOM,
    AWB_GREY_WORLD,
    AWB_METHOD_NUMBER,
}awb_work_method_t;

typedef enum
{
    AWB_FAIL_DO_NOTHING = 0,
    AWB_FAIL_CONST_GAIN,
    AWB_FAIL_NUMBER,
}awb_failure_remedy_t;

typedef enum
{
    AE_FULL_AUTO = 0,
    AE_SHUTTER_PRIORITY,
    AE_APERTURE_PRIORITY,
    AE_PROGRAM,
    AE_MANUAL,
}ae_work_mode;

typedef enum
{
    COMP_STEP_SIZE_1BY3 = 0,
    COMP_STEP_SIZE_1BY2,
    COMP_STEP_SIZE_1,
}ae_compensation_step_size;

typedef enum
{
    AE_SPOT_METERING = 0,
    AE_CENTER_METERING,
    AE_AVERAGE_METERING,
    AE_CUSTOM_METERING,
    AE_METERING_TYPE_NUMBER,
}ae_metering_mode;

typedef enum
{
    HDR_OP_MODE_LINEAR = 0,
    HDR_OP_MODE_HDR = 1 ,
    HDR_OP_MODE_COMBI_LINEAR,
    HDR_OP_MODE_AUTO,
    HDR_OP_MODE_NUMBER
}hdr_operation_mode_t;

typedef struct ae_info_s
{
    u16    dgain_update;
    u16    shutter_update;
    u16    agc_update;
    u16    iris_update;
    u16    shutter_index;
    u16    gain_index;
    u16    iris_index;
    u16    dgain;

    u16    isp_dgain;
    u16    isp_dgain_update;
    //u16    iso_value;
    //u16    flash;
}ae_info_t;

typedef enum
{
    FLASH_OFF =0,
    FLASH_AUTO,
    FLASH_ON,
}FLASH_MODE;

typedef enum
{
    RG = 0,
    BG = 1,
    GR = 2,
    GB = 3
} bayer_pattern;

#define    AWB_UNIT_SHIFT (10)
typedef    struct wb_gain_s
{
    u32    r_gain;
    u32    g_gain;
    u32    b_gain;
} wb_gain_t;

typedef struct blc_level_s
{
    s32    r_offset;
    s32    gr_offset;
    s32    gb_offset;
    s32    b_offset;
}blc_level_t;

typedef struct rgb_to_yuv_s
{
    s16    matrix_values[9];
    s16    y_offset;
    s16    u_offset;
    s16    v_offset;
}rgb_to_yuv_t;

typedef struct awb_data_s
{
    u32 r_avg;
    u32 g_avg;
    u32 b_avg;
    u32 lin_y;
    u32 cr_avg;
    u32 cb_avg;
    u32 non_lin_y;
}awb_data_t;

typedef struct ae_data_s
{
    u32 lin_y;
    u32 non_lin_y;
	u32 lin_y_aver;
	u32 lin_y_aver_last;
}ae_data_t;

typedef struct awb_gain_s
{
    u16        video_gain_update;
    u16        still_gain_update;
    wb_gain_t    video_wb_gain;
    wb_gain_t    still_wb_gain;
}awb_gain_t;


struct af_statistics_info
{
    u16 af_tile_num_col;
    u16 af_tile_num_row;
    u16 af_tile_col_start;
    u16 af_tile_row_start;
    u16 af_tile_width;
    u16 af_tile_height;
    u16 af_tile_active_width;
    u16 af_tile_active_height;
};

typedef struct alpha_s
{
    u8 alpha_plus;
    u8 alpha_minus;
    u8 smooth_adaptation;
    u8 smooth_edge_adaptation;
    u8 t0;
    u8 t1;
    u8 max_change_up;
    u8 max_change_down;
}alpha_t;

typedef struct embed_hist_stat_s
{
    u8   valid;
    u16 frame_cnt;
    u16 frame_id;
    u32 mean;
    u32 hist_begin;
    u32 hist_end;
    u32 mean_low_end;
    u32 perc_low_end;
    u32 norm_abs_dev;
    u32 hist_bin_data[256];
}embed_hist_stat_t;

//RAW statistics for customer 3A

typedef struct awb_lut_unit_s
{
    u16 gr_min;
    u16 gr_max;
    u16 gb_min;
    u16 gb_max;
    s16 y_a_min_slope;
    s16 y_a_min;
    s16 y_a_max_slope;
    s16 y_a_max;
    s16 y_b_min_slope;
    s16 y_b_min;
    s16 y_b_max_slope;
    s16 y_b_max;
    s8  weight;
} awb_lut_unit_t;

typedef struct awb_lut_s
{
    u8        lut_no;
    awb_lut_unit_t    awb_lut[20];
} awb_lut_t;

typedef struct awb_lut_idx_s
{
    u8 start;
    u8 num;
}awb_lut_idx_t;

typedef enum
{
    ISO_AUTO = 0,
    ISO_100     = 0,    //0db
    ISO_150 = 3,
    ISO_200 = 6,    //6db
    ISO_300 = 9,
    ISO_400 = 12,    //12db
    ISO_600 = 15,
    ISO_800 = 18,    //18db
    ISO_1600 = 24,    //24db
    ISO_3200 = 30,    //30db
    ISO_6400 = 36,    //36db
    ISO_12800 = 42,    //42db
    ISO_25600 = 48,    //48db
    ISO_51200 = 54,    //54db
    ISO_102400 = 60,//60db
    ISO_204800 = 66,//66db
    ISO_409600 = 72,//72db
}ae_iso_mode;

typedef enum
{
    APERTURE_AUTO = 0,
    APERTURE_F16,
    APERTURE_F11,
    APERTURE_F8,
    APERTURE_F5P6,
    APERTURE_F4,
    APERTURE_F2P8,
    APERTURE_F2,
    APERTURE_F1P4,
    APERTURE_MANUAL,
}ae_aperture_mode;
#define    SHUTTER_TIME_TABLE_LENGTH    (2304)
#define    SHUTTER_TIME_TABLE_LENGTH_1    (SHUTTER_TIME_TABLE_LENGTH-1)
#define    SHUTTER_DGAIN_TABLE_LENGTH    (1292)//1012=>2303
#define    AGC_DGAIN_TABLE_LENGTH    (769)//0=>768
typedef enum{
    SHUTTER_AUTO = 0,
    SHUTTER_2S = SHUTTER_TIME_TABLE_LENGTH_1-256,
    SHUTTER_1S = SHUTTER_TIME_TABLE_LENGTH_1-384,
    SHUTTER_1BY2 = SHUTTER_TIME_TABLE_LENGTH_1-512,
    SHUTTER_1BY4 = SHUTTER_TIME_TABLE_LENGTH_1-640,
    SHUTTER_1BY5 = SHUTTER_TIME_TABLE_LENGTH_1-672,
    SHUTTER_1BY7P5 = SHUTTER_TIME_TABLE_LENGTH_1 -756,
    SHUTTER_1BY8 = SHUTTER_TIME_TABLE_LENGTH_1-768,
    SHUTTER_1BY10 = SHUTTER_TIME_TABLE_LENGTH_1-809,
    SHUTTER_1BY12 = SHUTTER_TIME_TABLE_LENGTH_1-845,
    SHUTTER_1BY12P5 = SHUTTER_TIME_TABLE_LENGTH_1-850,
    SHUTTER_1BY15 = SHUTTER_TIME_TABLE_LENGTH_1-884,
    SHUTTER_1BY18 = SHUTTER_TIME_TABLE_LENGTH_1-914,
    SHUTTER_1BY20 = SHUTTER_TIME_TABLE_LENGTH_1-937,
    SHUTTER_1BY25 = SHUTTER_TIME_TABLE_LENGTH_1-978,
    SHUTTER_1BY30 = SHUTTER_TIME_TABLE_LENGTH_1-1012,
    SHUTTER_1BY50 = SHUTTER_TIME_TABLE_LENGTH_1-1106,
    SHUTTER_1BY60 = SHUTTER_TIME_TABLE_LENGTH_1-1140,
    SHUTTER_1BY100 = SHUTTER_TIME_TABLE_LENGTH_1-1234,
    SHUTTER_1BY120 = SHUTTER_TIME_TABLE_LENGTH_1-1268,
    SHUTTER_1BY240 = SHUTTER_TIME_TABLE_LENGTH_1-1396,
    SHUTTER_1BY480 = SHUTTER_TIME_TABLE_LENGTH_1-1524,
    SHUTTER_1BY960 = SHUTTER_TIME_TABLE_LENGTH_1-1652,
    SHUTTER_1BY1024 = SHUTTER_TIME_TABLE_LENGTH_1-1664,
    SHUTTER_1BY8000 = SHUTTER_TIME_TABLE_LENGTH_1-2046,
    SHUTTER_1BY16000 = SHUTTER_TIME_TABLE_LENGTH_1 -2174,
    SHUTTER_1BY32000 =SHUTTER_TIME_TABLE_LENGTH_1 -2302,
}ae_shutter_mode;

typedef struct joint_s
{
/*    s16    shutter;        // shall be values in ae_shutter_mode
    s16    gain;        //shall be values in ae_iso_mode
    s16    aperture;    //shall be values in ae_aperture_mode
*/
    s32    factor[3];    //0-shutter, 1-gain, 2-iris
}joint_t;

typedef struct line_s
{
    joint_t    start;
    joint_t    end;
}line_t;

typedef  struct adj_black_level_control_s
{
    s16    low_temperature_r;
    s16    low_temperature_g;
    s16    low_temperature_b;
    s16    high_temperature_r;
    s16    high_temperature_g;
    s16    high_temperature_b;
}adj_black_level_control_t;

typedef struct adj_awb_control_s
{
    u8    low_temp_r_target;
    u8    low_temp_b_target;
    u8    d50_r_target;
    u8    d50_b_target;
    u8    high_temp_r_target;
    u8    high_temp_b_target;
}adj_awb_control_t;

typedef  struct adj_ev_color_s
{
    u8    y_offset;
    u8    uv_saturation_ratio;
    u8    color_ratio;
    u8    gamma_ratio;
    u8    local_wdr_ratio;
    u8    chroma_scale_ratio;
}adj_ev_color_t;

typedef struct adj_ev_img_s {
    u8        y_offset_enable;
    u8        uv_saturation_ratio_enable;
    u8        color_ratio_enable;
    u8        gamma_ratio_enable;
    u8        local_wdr_ratio_enable;
    u8        chroma_scale_ratio_enable;
    u8        max_table_count;
    adj_ev_color_t    ev_color[20];
}adj_ev_img_t;

typedef struct gamma_curve_info_s
{
    u16 tone_curve_red[256];
    u16 tone_curve_green[256];
    u16 tone_curve_blue[256];
}gamma_curve_info_t;

typedef  struct adj_noise_enable_s
{
    u8    bad_pixel_enable;
    u8    chroma_median_filter_enable;
}adj_noise_enable_t;

typedef  struct adj_noise_control_s
{
    u8    hot_pixel_strength;
    u8    dark_pixel_strength;
    u16    cb_mfilter_str;
    u16    cr_mfilter_str;
}adj_noise_control_t;

typedef  struct adj_cfa_filter_control_s
{
    u8    dir_center_weight_red;
    u8    dir_center_weight_green;
    u8    dir_center_weight_blue;
    u16    dir_threshold_k0_red;
    u16    dir_threshold_k0_green;
    u16    dir_threshold_k0_blue;
    u8    non_dir_center_weight_red;
    u8    non_dir_center_weight_green;
    u8    non_dir_center_weight_blue;
    u16    non_dir_threshold_k0_red;
    u16    non_dir_threshold_k0_green;
    u16    non_dir_threshold_k0_blue;
    u16    non_dir_threshold_k0_close;
}adj_cfa_filter_control_t;

typedef struct dnr3d_s
{
    u8 alpha;
    u8 threshold_1;
    u8 threshold_2;
    u8 y_max_change;
    u8 u_max_change;
    u8 v_max_change;
}dnr3d_t;

typedef struct spatial_filter_s
{
    u8  isotropic_strength;
    u8  directional_strength;
    u16 edge_threshold; //11 bits only
    u16 max_change;
}spatial_filter_t;

typedef struct sharpen_level_s
{
    u8 low;
    u8 low_delta;
    u8 low_strength;
    u8 mid_strength;
    u8 high;
    u8 high_delta;
    u8 high_strength;
}sharpen_level_t;

typedef struct max_change_s
{
    u8 max_change_up;
    u8 max_change_down;
    //added by Hu Yin 15-08-2017 for GK7202
    u8 max_change_sel;
    u8 max_change_up_fir1s;
    u8 max_change_down_fir1s;
    u8 max_change_up_fir1w;
    u8 max_change_down_fir1w;
    u8 max_change_up_mv;
    u8 max_change_down_mv;
    //end
} max_change_t;

typedef struct fir_s
{
    int fir_strength;
    //added by Hu Yin 15-08-2017 for GK7202
    int fir1_s_str;
    int fir1_s_mode;
    int fir1_w_str;
    int fir1_w_mode;
    int fir1_mode;
    int fir2_str;
    int fir3_str_iso;
    int fir3_str_dir;
    int fir1_s_enable;
    int fir1_w_enable;
    int fir3_enable;
    int fir_th1;
    int fir_th2;
    int fir_th3;
    int fir1_mv_str;
    int fir2_mv_str;

    u8  edge_disp_enable;
    u8  edge_dir_sel;
    u8  edge_dir_shift;
    u8  edge_dir_info;
    u8  mv_enable;
//end
    s16 fir_coeff[10];
}fir_t;

typedef struct retain_max_fir_s
{
    u8                edge_shap_level;
    max_change_t        max_change;
    fir_t            fir;
}retain_max_fir_t;
/*
typedef struct def_sharp_info_s
{
    u8        max_table_count;
    u8                        high_freq_noise_enable;
    u8                high_freq_noise_reduc[15];

    u8                              spatial_enable;
    icore_spatial_filter_t        spatial_filter[15];

    u8                              sharpen_enable;
    icore_level_t            level_min[15];
    icore_level_t            level_overall[15];
    retain_max_fir_t        rmf[15];
    coring_table_t        coring[15];
}def_sharp_info_t;

typedef struct adj_def_s
{
    u8            max_table_count;
    color_3d_t        color;
    u8            black_level_enable;
    adj_black_level_control_t    black_level[15];
    adj_noise_enable_t              noise_enable;
    adj_noise_control_t    noise_table[15];
    u8                                  cfa_filter_enable;
    adj_cfa_filter_control_t    cfa_table[15];

    gamma_curve_info_t    ratio_255_gamma;
    gamma_curve_info_t    ratio_0_gamma;

}adj_def_t;
*/

typedef struct cfa_leakage_filter_s
{
    u8 enable;
    s8 alpha_rr;
    s8 alpha_rb;
    s8 alpha_br;
    s8 alpha_bb;
    u16 saturation_level;
}cfa_leakage_filter_t;


typedef struct chroma_median_filter_s
{
    int    enable;
    u16    cb_str;
    u16    cr_str;
}chroma_median_filter_t;

#define NUM_CHROMA_GAIN_CURVE    (128)
typedef struct chroma_scale_filter_s
{
    u8 enable; //0:disable 1:PC style VO 2:HDTV VO
    u16 gain_curve[NUM_CHROMA_GAIN_CURVE];
} chroma_scale_filter_t;




#define    TONE_CURVE_SIZE    (256)
typedef struct tone_curve_s_h42
{
    u16 tone_curve[TONE_CURVE_SIZE];

}tone_curve_h42;


typedef struct tone_curve_s
{
    u16 tone_curve_red[TONE_CURVE_SIZE];
    u16 tone_curve_green[TONE_CURVE_SIZE];
    u16 tone_curve_blue[TONE_CURVE_SIZE];
}tone_curve_t;

typedef struct digital_sat_level_s
{
    u32 level_red;
    u32 level_green_even;
    u32 level_green_odd;
    u32 level_blue;
}digital_sat_level_t;


typedef struct dbp_correction_s
{
    u8  enable;
    u8  hot_pixel_strength;
    u8  dark_pixel_strength;
}dbp_correction_t;

#define    NUM_EXPOSURE_CURVE    256
typedef struct local_wdr_s
{
    u8 enable;
    u8 radius;
    u8 luma_weight_red;
    u8 luma_weight_green;
    u8 luma_weight_blue;
    u8 luma_weight_shift;
    u16 gain_curve_table[NUM_EXPOSURE_CURVE];
}local_wdr_t;

typedef struct coring_table_s
{
    u8 coring[256];
}coring_table_t;

#define    NUM_BAD_PIXEL_THD    (384)

typedef struct vignette_info_s
{
    u8 enable;
    u8 gain_shift;
    u32 vignette_red_gain_addr;
    u32 vignette_green_even_gain_addr;
    u32 vignette_green_odd_gain_addr;
    u32 vignette_blue_gain_addr;
}vignette_info_t;

typedef struct sensor_info_s
{
    u8 sensor_id;
     u8 field_format;          /**< Field */
     u8 sensor_resolution;     /**< Number of bits for data representation */
     u8 sensor_pattern;        /**< Bayer patterns RG, BG, GR, GB */
}sensor_info_t;

typedef struct fpn_correction_s
{
    u8    enable;
    u16    pixel_map_width;
    u16    pixel_map_height;
    u16    fpn_pitch;
    u32    pixel_map_addr;
    u32    pixel_map_size;
}fpn_correction_t;

typedef struct img_adj_param_s
{
    blc_level_t    blc[3*6];//(FA+D50+D75)*6
    u16        ae_target[6];
    rgb_to_yuv_t    rgb2yuv_matrix;
    tone_curve_t    tone_curve;
}img_adj_param_t;

typedef struct still_cap_info_s
{
    u32 jpeg_w;
    u32 jpeg_h;
    u32 thumb_w;
    u32 thumb_h;
    u32 capture_num;
    u32 need_raw : 1;
    u32 keep_AR_flag : 1;    //keep correct aspect ratio flag
    u32 reserved : 30;
}still_cap_info_t;

typedef struct still_proc_mem_init_info_s
{
    u32 width;
    u32 height;
    u8* __user_raw_addr;
}still_proc_mem_init_info_t;

typedef struct still_proc_mem_info_s
{
    u32 width;
    u32 height;
}still_proc_mem_info_t;

/********************************************************
*    Auto-Focus related data structs                            *
*********************************************************/

/**
 * The supported Lens ID
 */
typedef enum
{
/* fixed lens ID list */
    LENS_CMOUNT_ID = 0,
/*single focus lens ID list*/
    LENS_CM8137_ID = 10,
    LENS_SY3510A_ID = 11,
    LENS_IU072F_ID = 12,
/*zoom lens ID list*/
    LENS_TAMRON18X_ID = 101,
    LENS_JCD661_ID = 102,
    LENS_SY6310_ID = 103,
    LENS_YuTong_YT30031FB_ID = 104,//3.5-9.5mm
    LENS_Foctek_D14_02812IR_ID = 105,//2.8-12mm
    LENS_YuTong_YT30021FB_ID = 106,//6-22mm
    LENS_YS05IR_F1F18_ID = 107,//4.9-47mm
/*custom lens ID list*/
    LENS_CUSTOM_ID = 900,
}lens_ID;

typedef enum
{
    CAF = 0,
    SAF = 1,
    MANUAL = 3,
    CALIB = 4,
    DEBUGS = 5,
    CALIB_SAF = 6,
    CALIB_FOCUS_LENGTH = 7,
    CALIB_ZOOM_LENGTH = 8,
}af_mode;

typedef struct af_range_s
{
    s32 far_bd;
    s32 near_bd;
}af_range_t;

typedef struct af_ROI_config_s
{
    u32    tiles[40];
}af_ROI_config_t;

typedef enum
{
    MANUALIDLE = 0,
    ZOOMIN = 1,
    ZOOMOUT = 2,
    ZOOMSTOP = 3,
    FOCUSN = 4,
    FOCUSF = 5,
    FOCUSSTOP = 6,
}af_manual_mode;

typedef struct af_control_s
{
    af_mode workingMode;
    af_ROI_config_t af_window;
    u8 zoom_idx;
    s32 focus_idx; //only avilable in MANUAL mode
    u8 zoom_status;
    af_manual_mode manual_mode;
}af_control_t;

typedef struct lens_control_s
{ // for algo internal use
    u8 focus_update;
    u8 zoom_update;
    s32 focus_pulse;
    s32 focus_pulse_pre;
    s32 zoom_pulse;
    s32 zoom_pulse_pre;
    s16 pps;
    ISP_StatisticsSetupT *af_stat_config;
    u8 af_stat_config_update;
}lens_control_t;

/* GOKE type begin */
typedef int (*aaa_awb_control_init_cb)(void *data, awb_control_mode_t mode, wb_gain_t * menu_gain, awb_lut_t * lut_table, awb_lut_idx_t * lut_table_idx);
typedef void (*aaa_awb_control_cb)(void *data, awb_work_method_t method, awb_control_mode_t mode, u16 tile_count, awb_data_t * p_tile_info, awb_gain_t * p_awb_gain);
/* GOKE type end */

typedef struct aaa_api_s
{
    int (*p_ae_control_init)(void *data, u8 Stype, line_t* line, u16 line_num, u16 max_sensor_gain, u16 double_inc, u32* agc_dgain,u32* sht_dgain, u8* dlight_range);
    int (*p_ae_control)(void *data, ae_data_t *p_tile_info, u16 tile_num, ae_info_t *p_video_ae_info, ae_info_t *p_still_ae_info);
    int (*p_awb_control_init)(void *data, awb_control_mode_t mode, wb_gain_t * menu_gain, awb_lut_t * lut_table, awb_lut_idx_t * lut_table_idx);
    void (*p_awb_control)(void *data, awb_work_method_t method, awb_control_mode_t mode, u16 tile_count, awb_data_t * p_tile_info, awb_gain_t * p_awb_gain);
    int (*p_af_control_init)(void *data, af_control_t* p_af_control, void* gk_af_param, void* gk_zoom_map, lens_control_t* p_lens_ctrl);
    int (*p_af_control)(void *data, af_control_t* p_af_control, u16 af_tile_count, ISP_CfaAfStatT* p_af_info, u16 awb_tile_count, awb_data_t* p_awb_info, lens_control_t * p_lens_ctrl, ae_info_t* p_ae_info, u8 lens_runing_flag,ISP_CfaAfStatT* p_af_info_rgb);
    void (*p_af_set_range)(void *data, af_range_t* p_af_range);
    void (*p_af_set_calib_param)(void *data, void* p_calib_param);
    u32 (*p_ae_get_system_gain)(void *data);
}aaa_api_t;

typedef struct lens_dev_drv_s
{
    int (*af_set_IRCut)(u8 enable);
    int (*set_shutter_speed)(u8 ex_mode, u16 ex_time);
    int (*ic_lens_init)(void);
    int (*af_lens_park)(void);
    int (*zoom_stop)(void);
    int (*af_focus_n)(u16 pps, u32 distance);
    int (*af_focus_f)(u16 pps, u32 distance);
    int (*zoom_in)(u16 pps, u32 distance);
    int (*zoom_out)(u16 pps, u32 distance);
    int (*af_focus_stop)(void);
    int (*af_set_aperture)(u16 aperture_idx);
    int (*set_mechanical_shutter)(u8 me_shutter);
    void (*af_set_zoom_pi)(u8 pi_n);
    void (*af_set_focus_pi)(u8 pi_n);
    int (*af_lens_standby)(u8 en);
    int (*check_isFocusRuning)(void);
    int (*check_isZoomRuning)(void);
}lens_dev_drv_t;
/********************************************************
*    End of Auto-Focus related data structs                        *
*********************************************************/
typedef struct wb_cali_s
{
    wb_gain_t org_gain[2];
    wb_gain_t ref_gain[2];
    wb_gain_t comp_gain[2];
}wb_cali_t;

typedef struct img_awb_param_s
{
    wb_gain_t    menu_gain[WB_MODE_NUMBER];
    awb_lut_t        wr_table;
    awb_lut_idx_t awb_param_table_idx[20];
}img_awb_param_t;

/*******adjust***********/
typedef struct adj_color_control_ev_s
{
    u16 ev_thre_low;
    u16 ev_thre_hi;
}adj_color_control_ev_t;

typedef struct adj_color_control_s
{
    u16 r_gain;
    u16 b_gain;
    u32 matrix_3d_table_addr;
    s16 uv_matrix[4];
    s16 cc_matrix[9];
}adj_color_control_t;

typedef struct adj_lut_s
{
    s16        value[20];
}adj_lut_t;

//added by Hu Yin 15-08-2017 for GK7202
typedef struct adj_l_lut_s
{
    s16        value[35];
}adj_l_lut_t;
//end

typedef struct adj_u_lut_s
{
    u16        value[20];
}adj_u_lut_t;


typedef struct color_3d_s
{
    u8            type;
    u8            control;
    u8            table_count;
    adj_color_control_t       table[5];
    adj_color_control_ev_t    cc_interpo_ev;
}color_3d_t;

typedef struct adj_ae_awb_s
{
    u8                max_table_count;
    adj_lut_t            table[25];
}adj_awb_ae_t;

typedef struct dnr3d_info_s
{
    u8        max_table_count;
    u8        enable;
    adj_lut_t        dnr3d_weak[15];
    adj_lut_t        dnr3d_strong[15];
    adj_lut_t        dnr3d_weak_night[15];
    adj_lut_t        dnr3d_strong_night[15];
}dnr3d_info_t;

//added by heqi 11-07_2016
typedef struct tuning_CFA_denoise_s{
    u8  dir_center_weight_red;
    u8  dir_center_weight_green;
    u8  dir_center_weight_blue;
    u16 dir_threshold;
    u8  non_dir_center_weight_red;
    u8  non_dir_center_weight_green;
    u8  non_dir_center_weight_blue;
    u16 non_dir_threshold;
    u16 direct_grad_thresh;
} tuning_CFA_denoise_t;

typedef struct tuning_sharp_str_s{
    s16  fir_strength;
    //added by Hu Yin 15-08-2017 for GK7202
    s16  fir1_s_strength;
    s16  fir1_w_strength;
    s16  fir2_strength;
    s16  fir3_strength_iso;
    s16  fir3_strength_dir;
    u8  fir1_s_enable;
    u8  fir1_w_enable;
    u8  fir3_enable;
    u8  fir_th1;
    u8  fir_th2;
    //end

    u8  isotropic_strength;
    u8  directional_strength;
    u16 edge_threshold; //11 bits only
    u8  max_change_up;
    u8  max_change_down;
} tuning_sharp_str_t;

typedef struct tuning_LE_str_s{
    u8 str;
    u8 low_or_high;
    u8 enable;
    u8 radius;
    u8 luma_weight_red;
    u8 luma_weight_green;
    u8 luma_weight_blue;
    u8 luma_weight_shift;
} tuning_LE_str_t;
//end by heqi 11-07-2016
typedef struct adj_filter_info_s {
    u8            table_count;
    adj_lut_t        enable;
    adj_lut_t        table[25];
}adj_filter_info_t;

typedef struct adj_def_s
{
    u8                  max_table_count;
    color_3d_t          color;
    u8                  black_level_enable;
    adj_lut_t           black_level[25];
    //u8                bad_pixel_enable;
    //u8                chroma_median_filter_enable;
    adj_lut_t           noise_enable;
    adj_lut_t           noise_table[25];
    u8                  cfa_filter_enable;
    adj_lut_t           low_cfa_table[25];
    adj_lut_t           high_cfa_table[25];
    adj_lut_t           low_cfa_table_weak_move[25];
    adj_lut_t           high_cfa_table_weak_move[25];
    adj_lut_t            low_cfa_table_night[25];
    adj_lut_t            high_cfa_table_night[25];
    tone_curve_h42      tone_curve[25];
    gamma_curve_info_t  ratio_255_gamma;
    gamma_curve_info_t  ratio_0_gamma;
    u16 ratio_255_local_wdr_table[NUM_EXPOSURE_CURVE];
    u16 ratio_0_local_wdr_table[NUM_EXPOSURE_CURVE];
}adj_def_t;

typedef struct def_sharp_info_s
{
    u8                  max_table_count;
    u8                  spatial_enable;
    adj_lut_t           spatial_filter[25];
    adj_lut_t           spatial_filter_weak_move[25];
    adj_lut_t           spatial_filter_night[25];
    u8                  sharpen_enable;
    adj_lut_t           level_min[25];
    adj_lut_t           level_overall[25];
    //added by Hu Yin 15-08-2017 for GK7202
    adj_l_lut_t         rmf[25];
    adj_l_lut_t         rmf_weak_move[25];
    adj_l_lut_t         rmf_night[25];
    //end
    coring_table_t      coring[25];
    coring_table_t      coring_nigth[25];
 //added by Hu Yin 15-08-2017 for GK7202
    coring_table_t      coring_mv[25];
    coring_table_t      coring_mv_nigth[25];
//end
}def_sharp_info_t;

typedef struct str_info_s
{
    s16             str;
    u16             offset;
    u16             adapt;
}str_info_t;

#define STR_MAX_VALUE        19
typedef struct str_lut_s
{
    str_info_t    info[20];
}str_lut_t;

typedef struct adj_str_control_s
{
    u8          table_count;
    u8          enable;
    s16         start;
    s16         end;
    str_lut_t   lut[12];
}adj_str_control_t;

//Added by Hu Yin 20-04-2016
typedef struct adj_vps_info_s {
    u8                         max_table_count;
    u8                         yuv2d_enable;
    adj_lut_t                  yuv2d_s[25];
    adj_lut_t                  yuv2d_w[25];
    //yuv2d_table_info_t       yuv2d_table[25];
    u8                         yuv3d_enable;
    adj_u_lut_t                yuv3d[25];
    //yuv3d_table_info_t       yuv3d_table[25];
    adj_lut_t                  yuv3d_table_0[25];
    adj_lut_t                  yuv3d_table_1[25];
    adj_lut_t                  yuv3d_table_2[25];
    adj_lut_t                  yuv3d_table_3[25];
    adj_lut_t                  yuv3d_table_4[25];
    adj_lut_t                  yuv3d_table_5[25];
    adj_lut_t                  yuv3d_table_6[25];
    adj_lut_t                  yuv3d_table_7[25];
//added for tcnt patch 24-02-2017
    adj_lut_t                  yuv3d_table_option_0[25];
    adj_lut_t                  yuv3d_table_option_1[25];
    adj_lut_t                  yuv3d_table_option_2[25];
    adj_lut_t                  yuv3d_table_option_3[25];
    adj_lut_t                  yuv3d_table_option_4[25];
    adj_lut_t                  yuv3d_table_option_5[25];
    adj_lut_t                  yuv3d_table_option_6[25];
    adj_lut_t                  yuv3d_table_option_7[25];
//end
//add by heqi
    adj_lut_t                  yuv3d_night[25];
    //yuv3d_table_info_t       yuv3d_table[25];
    adj_lut_t                  yuv3d_table_0_night[25];
    adj_lut_t                  yuv3d_table_1_night[25];
    adj_lut_t                  yuv3d_table_2_night[25];
    adj_lut_t                  yuv3d_table_3_night[25];
    adj_lut_t                  yuv3d_table_4_night[25];
    adj_lut_t                  yuv3d_table_5_night[25];
    adj_lut_t                  yuv3d_table_6_night[25];
    adj_lut_t                  yuv3d_table_7_night[25];
//end add by heqi

    u8                         contrast_enable;
    adj_lut_t                  contrast[25];
}adj_vps_info_t;
//end

//added by Hu Yin 15-08-2017 for GK7202
typedef struct adj_dpf_info_s{
    u8              max_table_count;
    u8              enable;
    adj_lut_t       dpf_table[25];
}adj_dpf_info_t;

typedef struct adj_nlm_info_s{
    u8              max_table_count;
    u8              enable;
    adj_lut_t       nlm_table[25];
}adj_nlm_info_t;

typedef struct vgf_table_s{
    u16 lookup_table[256];
}vgf_lookup_table_t;

typedef struct adj_vgf_info_s{
    u8                   max_table_count;
    u8                   enable;
    adj_lut_t            vgf_table[25];
    vgf_lookup_table_t   lookup_table[25];
}adj_vgf_info_t;

typedef struct adj_dms_info_s{
    u8              max_table_count;
    u8              enable;
    adj_lut_t       dms_table[25];
}adj_dms_info_t;

typedef struct adj_cmn_ex_info_s{
    u8              max_table_count;
    u8              enable;
    adj_lut_t       cmn_ex_table[25];
}adj_cmn_ex_info_t;

typedef struct adj_lce_info_s{
    u8              max_table_count;
    u8              enable;
    adj_lut_t       lce_table[25];
}adj_lce_info_t;

typedef struct adj_blf_info_s{
    u8              max_table_count;
    u8              enable;
    adj_lut_t       blf_table[25];
}adj_blf_info_t;

typedef struct adj_gmd_info_s{
    u8              max_table_count;
    u8              enable;
    adj_lut_t       gmd_table[25];
}adj_gmd_info_t;

typedef struct adj_pmv_info_s{
    u8              max_table_count;
    u8              enable;
    adj_lut_t       pmv_table[25];
}adj_pmv_info_t;

typedef struct adj_gmf_info_s{
    u8              max_table_count;
    u8              enable;
    adj_u_lut_t     gmf_table[25];
    adj_lut_t       gmf_static_table[25];
    adj_lut_t       gmf_move_table[25];
    adj_lut_t       gmf_static_table_2[25];
    adj_lut_t       gmf_move_table_2[25];
    adj_lut_t       gmf_static_table_3[25];
    adj_lut_t       gmf_move_table_3[25];
}adj_gmf_info_t;

//end

//add by heqi
typedef struct warp_config_s{
    u32 warp_control;
    u8 warp_multiplier;
    u8 vert_warp_enable;
    u8 vert_warp_multiplier;
}warp_config_t;
//end

typedef struct adj_encode_info_s{
    u8                max_table_count;
    u8                enable;
    adj_lut_t           encode_cntl[25];
}adj_encode_info_t;


typedef struct adj_param_s
{
    adj_awb_ae_t        awbae;
    adj_awb_ae_t        awbae_night;
    adj_filter_info_t   ev_img;
    adj_def_t           def;
    def_sharp_info_t    sharp_info;
    //adj_str_control_t   dzoom_control;

//added by Hu Yin 20-04-2016
    //adj_vps_info_t      vps_info;
    //adj_encode_info_t   encode_info;
//end

//added by Hu Yin 15-08-2017 for GK7202
    adj_dpf_info_t      dpf_info;
    adj_nlm_info_t      nlm_info;
    adj_nlm_info_t      nlm_info_weak_move;
    adj_nlm_info_t      nlm_info_night;
    adj_dms_info_t      dms_info;
    adj_cmn_ex_info_t   cmn_ex_info;
    adj_lce_info_t      lce_info;
    adj_lce_info_t      lce_info_night;
    adj_blf_info_t      blf_info;
    adj_blf_info_t      blf_info_weak_move;
    adj_blf_info_t      blf_info_night;
    adj_gmd_info_t      gmd_info;
    adj_gmd_info_t      gmd_info_night;
    adj_pmv_info_t      pmv_info;
    adj_pmv_info_t      pmv_info_night;
    adj_gmf_info_t      gmf_info;
    adj_gmf_info_t      gmf_info_night;
    adj_vgf_info_t      vgf_info;
    adj_vgf_info_t      vgf_info_night;
//end
} adj_param_t;

typedef struct adj_aeawb_control_s
{
    u8    low_temp_r_target;
    u8    low_temp_b_target;
    u8    d50_r_target;
    u8    d50_b_target;
    u8    high_temp_r_target;
    u8    high_temp_b_target;
    u16   ae_target;
    u8     aeawb_update;
} adj_aeawb_control_t;

/*******adjust***********/

typedef struct aaa_cntl_s
{
    u8    awb_enable;
    u8    ae_enable;
    u8    af_enable;
    u8    adj_enable;
}aaa_cntl_t;

typedef   struct image_property_s
{
    int    brightness;
    int    contrast;
    int    saturation;
    int    hue;
    int    sharpness;
    int    local_exp;
    int    vps_3d;
    int    vps_contrast;
}image_property_t;

typedef struct sharpen_property_s
{
    int fir_ratio;
    int spatial_filter_ratio;
    int dir_filter_ratio;
    int max_change_up_ratio;
    int max_change_down_ratio;
    int edge_ratio;

}sharpen_property_t;

typedef struct cfa_denoise_property_s
{
    int cfa_flat_weight_ratio;
    int cfa_dir_weight_ratio;
    int cfa_flat_thr_ratio;
    int cfa_dir_thr_ratio;
    int edge_ratio;
    //int cfa_threshold_ratio;
}cfa_denoise_property_t;


typedef struct dnr3d_property_s
{
    int dnr3d_alpha_ratio;
    int dnr3d_threshold_ratio;
    int dnr3d_threshold_limit;
    int dnr3d_max_change_ratio;
}dnr3d_property_t;

//added by Hu Yin 08-03-2017
typedef struct image_low_bitrate_property_s
{
    u32 sharpness_low_bitrate_ratio;
    u32 sharpness_low_bitrate_param;
    //u32 vps_3d_low_bitrate_ratio;
    //u32 vps_3d_low_bitrate_param;
    //u32 vps_contrast_low_bitrate_ratio;
    //u32 vps_contrast_low_bitrate_param;
    u32 sharpen_property_low_bitrate_up_ratio;
    u32 sharpen_property_low_bitrate_up_param;
    u32 sharpen_property_low_bitrate_down_ratio;
    u32 sharpen_property_low_bitrate_down_param;
    u32 cfa_denoise_property_low_bitrate_up_ratio;
    u32 cfa_denoise_property_low_bitrate_up_param;
    u32 cfa_denoise_property_low_bitrate_down_ratio;
    u32 cfa_denoise_property_low_bitrate_down_param;
}image_low_bitrate_property_t;
//end

//added by Hu Yin 06-01-2016
typedef struct lens_shading_property_s
{
     u8  enable;
     //u8  padding1;
}lens_shading_property_t;
//end

typedef struct image_sensor_param_s
{
    adj_param_t *               p_adj_param;
    adj_param_t *               p_adj_param_ir;
    rgb_to_yuv_t *              p_rgb2yuv;
    chroma_scale_filter_t*      p_chroma_scale;
 //added by Hu Yin 06-01-2016
    lens_shading_property_t*    p_lens_shading;
    warp_config_t*              p_warp_config;
//end
    img_awb_param_t *           p_awb_param;
    line_t *                    p_50hz_lines;
    line_t *                    p_60hz_lines;
    ISP_StatisticsConfigT*      p_tile_config;
    u32*                        p_ae_agc_dgain;
    u32*                        p_ae_sht_dgain;
    u8*                         p_dlight_range;
    local_wdr_t*                p_manual_LE;
}image_sensor_param_t;

typedef int (*dc_iris_cntl)(int, int, int*);

typedef struct cali_badpix_setup_s
{
    u8* badpixmap_buf;
    u32 cap_width;
    u32 cap_height;
    u32 cali_mode; // video or still
    u32 badpix_type; // hot pixel for cold pixel
    u32 block_h;
    u32 block_w;
    u32 upper_thres;
    u32 lower_thres;
    u32 detect_times;
    u32 shutter_idx;
    u32 agc_idx;
    u8  save_raw_flag;
} cali_badpix_setup_t;

typedef struct vignette_cal_s
{
#define VIGNETTE_MAX_WIDTH (33)
#define VIGNETTE_MAX_HEIGHT (33)
#define VIGNETTE_MAX_SIZE (VIGNETTE_MAX_WIDTH*VIGNETTE_MAX_HEIGHT)
/*input*/
    u16* raw_addr;
    u16 raw_w;
    u16 raw_h;
    bayer_pattern bp;
    u32 threshold; // if ((max/min)<<10 > threshold) => NG
    u16 compensate_ratio;  //1024 for fully compensated , 0 for no compensation
    u8  lookup_shift; // 0 for 3.7, 1 for 2.8, 2 for 1.9, 3 for 0.10, 255 for auto select
    blc_level_t blc;
/*output*/
    u16* r_tab;
    u16* ge_tab;
    u16* go_tab;
    u16* b_tab;
}vignette_cal_t;

typedef struct cali_gyro_setup_s
{
    //gyro_cal_info_s
    u8 mode;
#define GYRO_FUNC_CALIB    (0)
#define GYRO_BIAS_CALIB        (1)
    u32 time; //how many seconds to do calibration
#define SAMPLE_NUM_MAX (10240) // 10s, 1 sample/ms
    u32 freq; //the freq of shaker used
    u32 amp; // the full-range angle of shaker and multiply 10
    u16 buf_size;
    u16* buf_x;
    u16* buf_y;
    u8 sampling_rate;
}cali_gyro_setup_t;
//gyro_cal_info_t;

typedef struct gyro_calib_info_s
{
    u32 mean_x;
    u32 mean_y;
    u32 sense_x;
    u32 sense_y;
}gyro_calib_info_t;

typedef enum
{
    IMG_COLOR_TV = 0,
    IMG_COLOR_PC,
    IMG_COLOR_STILL,
    IMG_COLOR_CUSTOM,
    IMG_COLOR_NUMBER,
}img_color_style;

typedef struct still_hdr_proc_s
{
    u32 height;
    u32 width;
    u16* short_exp_raw;
    u16* long_exp_raw;
    u32* raw_buff;
    u32 short_shutter_idx;
    u32 long_shutter_idx;
    u32 r_gain;
    u32 b_gain;
}still_hdr_proc_t;

typedef struct aaa_tile_report_s
{
    u16 awb_tile;
    u16 ae_tile;
    u16 af_tile;
}aaa_tile_report_t;

typedef struct img_snapshot_config_s
{
    u8 cap_mode; //0- fast, 1- high iso , 2 - low iso, 4 auto
    u8 cap_burst_num;
    //u8 art_effect;
    u8 save_raw;
    u8 flash_cntl;
    u8 hdr_cntl;
    u16 vidcap_w;
    u16 vidcap_h;
    u16 main_w;
    u16 main_h;
    u16 encode_w;
    u16 encode_h;
    u16 preview_w_A;
    u16 preview_h_A;
    u16 preview_w_B;
    u16 preview_h_B;
    u16 thumbnail_w;
    u16 thumbnail_h;
    u16 thumbnail_dram_w;
    u16 thumbnail_dram_h;
    u8 qp;
}img_snapshot_config_t;

//#ifdef CUSTOMER_MAINLOOP

typedef struct cc_addr_s
{
    u32 matrix_3d_table_addr;
}cc_addr_t;

//added by Hu Yin 15-08-2017 for GK7202
typedef struct depurple_fringe_s{
    u8 depurple_fringe_enable;
    u8 nlm_dpf_enable;
    //u8 dpf_exphor1_enable;
    //u8 dpf_exphor2_enable;
    //u8 dpf_expver_enable;
    u8 depurple_fringe_strength;
    //u8 dpf_expver_range;
    u8 dpf_enhance_enable;

    u8 bpattern;                            //reg 1
    //u8 dpf_exphor1_range;
    //u8 dpf_exphor2_range;

    u16 depurple_fringe_th_max_a;            //reg 2
    u16 depurple_fringe_th_max_b;            //reg 3
    u16 depurple_fringe_th_max_c;            //reg 4
    u16 depurple_fringe_th_max_d;            //reg 5
    u16 depurple_fringe_th_max_e;            //reg 6
    u16 depurple_fringe_th_max_f;            //reg 7
    u16 depurple_fringe_th_max;              //reg 8
    u16 depurple_fringe_green_threshold;
    u16 depurple_fringe_rb_threshold;

    u16 de_purple_fringe_red_a[6];
    u16 de_purple_fringe_red_b[3];
    u16 de_purple_fringe_red_c[6];
    u16 de_purple_fringe_green_a[6];
    u16 de_purple_fringe_green_b[3];
    u16 de_purple_fringe_green_c[6];
    u16 de_purple_fringe_blue_a[6];
    u16 de_purple_fringe_blue_b[3];
    u16 de_purple_fringe_blue_c[6];

    u16 dpf_minA_threshold;
    u16 dpf_minB_threshold;
    u16 dpf_minC_threshold;
    u16 dpf_grad_threshold;
    u16 dpf_flag_threshold;

    u16 dpf_max_change;
    u16 dpf_alpha;
}depurple_fringe_t;

typedef struct nlm_noise_filter_s{
    u8 nlm_noise_filter_enable;
    u8 nlm_noise_filter_mv_enable;

    u16 nlm_noise_filter_luma_offset_red;     // reg 0xC
    u16 nlm_noise_filter_luma_offset_green;   // reg 0xD
    u16 nlm_noise_filter_luma_offset_blue;    // reg 0xE
    u16 nlm_noise_filter_strength_red;        // reg 0xF
    u16 nlm_noise_filter_strength_green;      // reg 0x10
    u16 nlm_noise_filter_strength_blue;       // reg 0x11
    u16 nlm_noise_filter_mv_luma_offset_red;  // reg 0x12
    u16 nlm_noise_filter_mv_luma_offset_green;// reg 0x13
    u16 nlm_noise_filter_mv_luma_offset_blue; // reg 0x14
    u16 nlm_noise_filter_mv_strength_red;     // reg 0x15
    u16 nlm_noise_filter_mv_strength_green;   // reg 0x16
    u16 nlm_noise_filter_mv_strength_blue;    // reg 0x17
    u16 nlm_noise_filter_factor;              // reg 0x18
    u16 nlm_max_change;
    u16 nlm_noise_filter_exp_lut[64];
}nlm_noise_filter_t;

typedef struct variance_guided_filter_s{
    u8  vgf_enable;             //reg 0
    u8  vgf_bpc_enable;
    u8  vgf_mv_enable;
    u16 vgf_mv_offset;

    u16 vgf_cutoff_th1;            // reg 1
    u16 vgf_cutoff_value1;         // reg 2
    u16 vgf_cutoff_th2;            // reg 3
    u16 vgf_cutoff_value2;         // reg 4
    u16 vgf_bpc_vgf_th;            // reg 5
    u16 vgf_bpc_w_th;              // reg 6
    u16 vgf_bpc_b_th;              // reg 7

    u16 vgf_edge_th;
    u8  vgf_edge_wide_weight;
    u8  vgf_edge_narrow_weight;
    u8  vgf_grad_th;

    u8  vgf_disp_info_en;
    u16 vgf_disp_var_th1;
    u16 vgf_disp_var_th2;

    u8  vgf_disp_grad_th1;
    u8  vgf_disp_grad_th2;

    u16 vgf_edge_var_th;
    u8  vgf_edge_str_th1;
    u16 edge_offset;

    u16 lut_offset_r;
    u16 lut_offset_g;
    u16 lut_offset_b;

    u16 strength;
} variance_guided_filter_t;

typedef struct demosaic_noise_reduction_s{
    u16 thresh_grad_clipping;  // Reg 0x3

    u16 thresh_grad_noise;     // Reg 0x4 // A5M or ealier
    u8  thresh_activity;        // Reg 0x5
    u16 thresh_activity_diff;  // Reg 0x6
    u16 threshedge;

    u8 shift;
    u8 demoire_A_Y;
    u8 demoire_B_Y;
    u8 demoire_C_Y;
    u8 demoire_D_Y;
    u8 demoire_A_C;
    u8 demoire_B_C;
    u8 demoire_C_C;
    u8 demoire_D_C;
    u16 conti_th1;
    u16 conti_th2;
    u16 conti_th3;
    u16 conti_th4;

    u16 reverse;
}demosaic_noise_reduction_t;

typedef struct chroma_median_filter_ex_s{
    u8 ex_blur_en;
    u8 blur_nolimit_en;

    u8 ex_blur_delta_1;
    u8 ex_blur_delta_2;
    u8 ex_blur_delta_3;
    u8 ex_blur_mul_1;
    u8 ex_blur_sft_1;
    u8 ex_blur_mul_2;
    u8 ex_blur_sft_2;
    u8 ex_blur_mul_3;
    u8 ex_blur_sft_3;

    u8 ex_blur_data_u;
    u8 ex_blur_data_v;

    u8 reverse;
    u16 reverse1;
}chroma_median_filter_ex_t;

typedef struct local_contrast_enhancement_s{
    u8 lce_enable;
    u8 pre_lut_en;
    u8 pm_en;
    u8 mv_scan_en;
    u8 disp_mv_en;
    u8 tile;

    u8 pre_hist_scan_en;
    u8 pre_hist_scan_frame;
    u8 pre_hist_scan_id;
    u8 post_hist_scan_en;
    u8 post_hist_scan_frame;
    u8 post_hist_scan_id;
    u8 hist_scan_sft;

    u16 res_width;
    u16 res_height;

    u8 lce_range;
    u8 lce_guard;

    u8 lce_range_th1;
    u8 lce_range_th2;

    u8 lce_min_th;
    u8 lce_max_th;

    u8 lce_minb_range;
    u8 lce_maxb_range;

    u8 lce_contrast;
    u8 lce_delta_th;

    u16 lce_cutlimit;
    u16 lce_alpha;

    u8 lce_sud_th;
    u8 lce_sud_sum_th;

    u8 lce_delta_disable;
    u8 lce_index;
    u8 lce_search_th;

    u16 pm_y_gain;
    u8 pm_y_shift;
    u16 pm_u_gain;
    u8  pm_u_shift;
    u16 pm_v_gain;
    u8  pm_v_shift;

    u16 pm_y_offset;

    u16 pm_u_offset;

    u16 pm_v_offset;

    u8 mv_statis_th;

    u16 ram_lut[256];
    u16 ram_pm_mask[684];
}local_contrast_enhancement_t;

typedef struct bilateral_filter_s{
    u8 y_enable;
    u8 c_enable;
    u8 use_mv;
    u8 gmd_sel;
	// add by heqi 2018/08/09
    u8 adj_enable;
	u8 th_enable;
	u16 multi_coff;
	s16 min_mv_str;
	//end
    //u16 blf_simS_w[5];
    //u16 blf_simS_s[5];
    //u16 blf_simR_w[24];
    //u16 blf_simR_s[24];
    s16 simr_str;
    s16 sims_str;
    s16 simr_mv_str;
    s16 sims_mv_str;
}bilateral_filter_t;

typedef struct gmd_cfa_s{
  u8 en;
  u8 gamma_en;
  u8 blur_en_cfa;
  u8 single_sad_en;
  u8 cfa_interp_en;
  u8 reverse;

  u32 noise_cfa;
  s16 black_level_globe_offset_green;

  u16 digital_gain_multiplier_green;
  u16 digital_gain_shift_green;

  u16 gamma_lookup_green[352];
} gmd_cfa_t;

typedef struct pmv_s{
  u8 bpc_cor_enable;
  u8 bpc_cor_range;
  u8 bpc_cor_thresh;

  u8 psc_cor_enable;
  u8 psc_cor_range;
  u8 psc_feedthrough;

  u8 lea_expand_enable;
  u8 lea_expand_range;

  u8 dpf_exphor1_enable;
  u8 dpf_exphor2_enable;
  u8 dpf_expver_enable;

  u8 dpf_expver_range;
  u8 dpf_exphor1_range;
  u8 dpf_exphor2_range;

} pmv_t;

typedef struct globle_motion_filter_s{
    u8 enable;
    u8 gmd_sel;
    u8  lpf_enable;
    u8 reserved;

    u16 noise_l;
    u16 noise_h;
    u8  lpf3x3;
    u8  lpf_str;
    u16 reserved1;

    u8  maxchange_y_static;
    u8  maxchange_c_static;
    u8  maxchange_y_move;
    u8  maxchange_c_move;
    u16  mv_percent_th1;
    u16  mv_percent_th2;
    u16  mv_percent_th3;
    u16  mv_delta;
    u8  mv_count;
    u16 luma_diff_th1;
    u16 luma_diff_th2;

    u8 static_alpha_y_max;
    u8 static_alpha_y_min;
    u8 static_threshold_y_1;
    u8 static_threshold_y_2;
    u8 static_alpha_uv_max;
    u8 static_alpha_uv_min;
    u8 static_threshold_uv_1;
    u8 static_threshold_uv_2;

    u8 move_alpha_y_max;
    u8 move_alpha_y_min;
    u8 move_threshold_y_1;
    u8 move_threshold_y_2;
    u8 move_alpha_uv_max;
    u8 move_alpha_uv_min;
    u8 move_threshold_uv_1;
    u8 move_threshold_uv_2;

    u8 static_alpha_y_max_2;
    u8 static_alpha_y_min_2;
    u8 static_threshold_y_12;
    u8 static_threshold_y_22;
    u8 static_alpha_uv_max_2;
    u8 static_alpha_uv_min_2;
    u8 static_threshold_uv_12;
    u8 static_threshold_uv_22;

    u8 move_alpha_y_max_2;
    u8 move_alpha_y_min_2;
    u8 move_threshold_y_12;
    u8 move_threshold_y_22;
    u8 move_alpha_uv_max_2;
    u8 move_alpha_uv_min_2;
    u8 move_threshold_uv_12;
    u8 move_threshold_uv_22;

    u8 static_alpha_y_max_3;
    u8 static_alpha_y_min_3;
    u8 static_threshold_y_13;
    u8 static_threshold_y_23;
    u8 static_alpha_uv_max_3;
    u8 static_alpha_uv_min_3;
    u8 static_threshold_uv_13;
    u8 static_threshold_uv_23;

    u8 move_alpha_y_max_3;
    u8 move_alpha_y_min_3;
    u8 move_threshold_y_13;
    u8 move_threshold_y_23;
    u8 move_alpha_uv_max_3;
    u8 move_alpha_uv_min_3;
    u8 move_threshold_uv_13;
    u8 move_threshold_uv_23;

	// add by heqi 2018/08/09
	u8 adj_enable;
	u8 th_enable;
	u16 alpha_th;
	u16 max_change_th;
	u16 alpha_multi_coff;
    u16 max_change_multi_coff;
    //u16 gmf_mv_percent[3];
    //u16 gmf_range;
    //u16 gmf_count;
    u8 exp_str_uv;
    u8 exp_str_y;
    // end
    //u16 k_y_static_coeff[32];
    //u16 k_y_move_coeff[32];
    //u16 k_c_static_coeff[32];
    //u16 k_c_move_coeff[32];
}globle_motion_filter_t;


//end

typedef struct encode_realtime_cntl_s{
  u8  stream_id;      //[0,3]
  u8  qp_min_on_I_3a; //[0,51]
  u8  qp_max_on_I_3a; //[0,51]
  u8  qp_min_on_p_3a; //[0,51]
  u8  qp_max_on_p_3a; //[0,51]
  u8  aqp_3a;         //[0,2]
  u8  i_qp_reduce_3a; //[1,10]
  u8  p_qp_reduce_3a; //[1,5]
  u16 p_intraBiasAdd_3a; //[0,500]
  u8  intra_16_16_bias_3a; //[0,128]
  u8  intra_4_4_bias_3a;   //[0,128]
  u8  inter_16_16_bias_3a; //[0,128]
  u8  inter_8_8_bias_3a;   //[0,128]
  u8  me_lamda_qp_offset_3a; //[0,12]
  u8  alpha_3a;              //[0,12]
  u8  beta_3a;               //[0,12]
  u8  zmv_thresh_3a;         //[0,255]
  u8  uv_qp_offset_3a;       //[0,24]
  u8  IsizeCtlThresh_3a;     //[0,5]
  u8  IsizeCtlStrength_3a;   //[0,5]
  u8  reenc_threshStC_3a;    //[0,7]
  u8  reenc_strengthStC_3a;  //[0,4]
  u8  reenc_threshCtS_3a;    //[0,7]
  u8  reenc_strengthCtS_3a;  //[0,4]
}encode_realtime_cntl_t;


//end 09-05-2016
typedef struct bin_interpolation_s
{
    float A[100];
    float B[100];
    float Y[100];
    float reA[100];
    float reB[100];
    float Z[100];
}bin_interpolation_t; // CHENGBo 2016-10-12


typedef struct pipeline_control_s {
    u8                black_corr_update;
    blc_level_t            black_corr;

    u8                      rgb_yuv_matrix_update;
    rgb_to_yuv_t            rgb_yuv_matrix;

    u8                      gamma_update;
    tone_curve_t            gamma_table;

    u8                      local_wdr_update;
    local_wdr_t        local_wdr;

    u8                      chroma_scale_update;
    chroma_scale_filter_t   uv_scale;

    u8                      anti_aliasing_update;
    u8                      anti_aliasing;

    u8                      chroma_median_filter_update;
    chroma_median_filter_t  chroma_median_filter;

    u8                      high_freq_noise_reduc_update;
    u8                      high_freq_noise_reduc;

    u8                      cc_matrix_update;
    color_correction_t      color_corr;

    u8                      badpix_corr_update;
    dbp_correction_t        badpix_corr_strength;

    u8                      cfa_filter_update;
    ISP_FILTER_CFA_NoiseSetupT cfa_filter;

    u8                      spatial_filter_update;
    spatial_filter_t        spatial_filter;

    u8                      sharp_coring_update;
    coring_table_t          sharp_coring;
//added by Hu Yin 15-08-2017 for GK7202
    coring_table_t          sharp_coring_mv;
//end

    u8                      sharp_retain_update;
    u8                      sharp_retain;

    u8                      sharp_max_change_update;
    max_change_t            sharp_max_change;

    u8                      sharp_fir_update;
    fir_t                   sharp_fir;

    u8                      sharp_level_minimum_update;
    sharpen_level_t         sharp_level_minimum;

    u8                      sharp_level_overall_update;
    sharpen_level_t         sharp_level_overall;
//added by Hu Yin 22-04-2016
    //u8                      vps_stg1_yuv2d_update;
    //vps_stg1_yuv2d_t        vps_stg1_yuv2d;

    //u8                      vps_stg2_yuv3d_update;
    //vps_stg2_yuv3d_t        vps_stg2_yuv3d;

    //u8                      vps_stg3_contrast_update;
    //vps_stg3_contrast_t     vps_stg3_contrast;
    //u8                      encode_update;
    //encode_realtime_cntl_t  ecode_param;
//end
//added by Hu Yin 15-08-2017 for GK7202
    u8                            depurple_fringe_update;
    depurple_fringe_t             depurple_fringe;

    u8                            nlm_noise_filter_update;
    nlm_noise_filter_t            nlm_noise_filter;

    u8                            variance_guided_filter_update;
    variance_guided_filter_t      variance_guided_filter;

    u8                            demosaic_noise_reduction_update;
    demosaic_noise_reduction_t    demosaic_noise_reduction;

    u8                            chroma_median_filter_ex_update;
    chroma_median_filter_ex_t     chroma_median_filter_ex;

    u8                            local_contrast_enhancement_update;
    local_contrast_enhancement_t  local_contrast_enhancement;

    u8                            bilateral_filter_update;
    bilateral_filter_t            bilateral_filter;

    u8                            gmd_cfa_update;
    gmd_cfa_t                     gmd_cfa;

    u8                            pmv_update;
    pmv_t                         pmv;

    u8                            globle_motion_filter_update;
    globle_motion_filter_t        globle_motion_filter;
//end
}pipeline_control_t;

typedef struct
{
    int chip_id;
    u8 vps_enable;
    u8 contrast_enable;
}img_isp_capability_t;

typedef struct icore_cfa_leakage_filter_s
{
    u8 enable;
    s8 alpha_rr;
    s8 alpha_rb;
    s8 alpha_br;
    s8 alpha_bb;
    u16 saturation_level;
}icore_cfa_leakage_filter_t;

typedef struct icore_alpha_s
{
    u8 alpha_plus;
    u8 alpha_minus;
    u8 smooth_adaptation;
    u8 smooth_edge_adaptation;
    u8 t0;
    u8 t1;
    u8 reserved1; //reserved for backward compatible, to be removed
    u8 reserved2; //reserved for backward compatible, to be removed
} icore_alpha_t;

typedef struct icore_dir_shp_s
{
    u8  enable;
    u16 dir_strength[9];
    u16 iso_strength[9];
    u16 dir_amounts[9];
}icore_dir_shp_t;

typedef unsigned char word_8_byte[8];

typedef enum
{
    LOW_PASS,
    HIGH_PASS,
    HIGH_PASS_INVERTED,
} single_fir_type;

/*
typedef struct
{
    u16 width;
    u16 height;
    u8  bayer_pattern;
    u8  debug_mode;
    u8  max_up;
    u8  max_down;
    u8  use_CC_output_table_A;
    u8  use_CC_output_table_C;
    u16 edge_thresh_A;
    u16 smooth_chroma_width;
    u16 smooth_chroma_height;
    u16 main_phase_inc_chroma_x_A;
    u16 main_phase_inc_chroma_y_A;
    u16 prevB_phase_inc_chroma_x_A;
    u16 prevB_phase_inc_chroma_y_A;
    u32 cc_reg_A; // 2304 byte  , calculate with lowISO API
    u32 cc_3d_A;  // 16384 byte , calculate with lowISO API
    u32 cc_reg_C; // 2304 byte  , = normal API input cc
    u32 cc_3d_C;  // 16384 byte , = normal API input cc
    u32 chroma_median_table_A; // 48 byte, all 31
    u32 chroma_median_table_C; // 48 byte, = user API input
    u32 coring_A; // 256 byte    , = user API input
    u32 fir1_A;   // 256 byte    , calculate from user API input
    u32 fir1_C;   // 256 byte    , calculate from user API input
    u32 alpha_A;  // 512 byte alpha , calculate from user API input
    u32 dnr3d_C;   // dnr3d binary size 516 or 520 , calculate from low ISO still dnr3d API
    u32 zero;     // pointer to 512 bytes of zeros in dram
    ISP_LUMA_SharpLevelCtrlT coring1_multiplier_min_A;
    ISP_LUMA_SharpLevelCtrlT coring_index_mul_A;
    u32 low_iso_data_addr; //point to self
    // for fw internal use only
    u32 debug_icore_config_addr[3];
    u32 debug_addr[2];
    u16 debug_pitch[2];
    u16 debug_width[2];
    u16 debug_height[2];
} low_iso_param_t;
*/
typedef struct
{
    int version_major;                          // 4 bytes
    int version_minor;                          // 4 bytes
    int reg_file_size;                          // 4 bytes
    u8 reserved0[116];                          // 116 bytes

    u16 hiiso_inp0[384*3];                      // 2304 bytes
    u16 hiiso_inp1[384*3];                      // 2304 bytes
    u16 hiiso_inp2[384*3];                      // 2304 bytes
    u16 hiiso_inp3[384*3];                      // 2304 bytes
    u16 lowiso_inp0[384*3];                     // 2304 bytes
    u16 lowiso_inp1[384*3];                     // 2304 bytes
    u16 lowiso_inp2[384*3];                     // 2304 bytes
    u16 lowiso_inp3[384*3];                     // 2304 bytes

    u16 hiiso_input_curve_inverse_rgb[16];      // 32 bytes
    u16 lowiso_input_curve_inverse_rgb[16];     // 32 bytes
    u16 hiiso_input_curve_inverse_yuv[2][16];   // 64 bytes
    u16 hiiso_input_curve_inverse_alpha[2][16]; // 64 bytes
} cc_reg_file;

typedef struct
{
    int version_major;                          // 4 bytes
    int version_minor;                          // 4 bytes
    int is_high_iso;                            // 4 bytes
    u8  reserved0[116];                         // 116 bytes
    u32 threeD[4096];                           // 16KB
    u16 base_tone_curve[256];                   // 512 bytes
    u16 inverse_of_tone_curve[256];             // 512 bytes
} cc_3D_File;

typedef struct
{
    int version_major;                          // 4 bytes
    int version_minor;                          // 4 bytes
    int is_high_iso;                            // 4 bytes
    u8  reserved0[116];                         // 116 bytes
    u16 base_tone_curve[256];                   // 512 bytes
    u16 inverse_of_tone_curve[256];             // 512 bytes
} cc_3D_expansion_t;

typedef struct dnr3d_hw_config_s
{
    u16 curves[4][64]; //Note legal range is only 8bits not 16bits.
    u32 thres0   : 6;
    u32 thres1   : 6;
    u32 thres2   : 6;
    u32 thres3   : 6;
    u32 mv_coher : 3;
    u32 mv_max   : 5;
    u32 padding1;
    u8  y_max_change;
    u8  u_max_change;
    u8  v_max_change;
    u8  still_dnr3d_mode : 1;
    u8  reserved1       : 3;
    u8  blend           : 1;
    u8  reserved2       : 3;
    u32 padding2;
} dnr3d_hw_config_t;






typedef struct
{
    u16 width;
    u16 height;
    u8  bayer_pattern;
    u8  debug_mode;
    u8  max_up;
    u8  max_down;
    u8  use_CC_output_table_A;
    u8  use_CC_output_table_C;
    u16 edge_thresh_A;
    u16 smooth_chroma_width;
    u16 smooth_chroma_height;
    u16 main_phase_inc_chroma_x_A;
    u16 main_phase_inc_chroma_y_A;
    u16 prevB_phase_inc_chroma_x_A;
    u16 prevB_phase_inc_chroma_y_A;
    u32 cc_reg_A; // 2304 byte  , calculate with lowISO API
    u32 cc_3d_A;  // 16384 byte , calculate with lowISO API
    u32 cc_reg_C; // 2304 byte  , = normal API input cc
    u32 cc_3d_C;  // 16384 byte , = normal API input cc
    u32 chroma_median_table_A; // 48 byte, all 31
    u32 chroma_median_table_C; // 48 byte, = user API input
    u32 coring_A; // 256 byte    , = user API input
    u32 fir1_A;   // 256 byte    , calculate from user API input
    u32 fir1_C;   // 256 byte    , calculate from user API input
    u32 alpha_A;  // 512 byte alpha , calculate from user API input
    u32 alpha_C;
    u32 dnr3d_C;   // dnr3d binary size 516 or 520 , calculate from low ISO still dnr3d API
    u32 zero;     // pointer to 512 bytes of zeros in dram
    ISP_LumaSharpLevelCtrlT coring1_multiplier_min_A;
    ISP_LumaSharpLevelCtrlT coring_index_mul_A;
    u32 low_iso_data_addr; //point to self
    // for fw internal use only
    u32 debug_icore_config_addr[3];
    u32 debug_addr[2];
    u16 debug_pitch[2];
    u16 debug_width[2];
    u16 debug_height[2];
}low_iso_param_t;

typedef struct fir_coef_one_dir_s
{
    s32 coef24 : 10;
    s32 reserved : 22;
    s8  coef0to23[24];
}fir_coef_one_dir_t;


typedef struct fir_shift_s
{
    u32 shift0 : 3;
    u32 shift1 : 3;
    u32 shift2 : 3;
    u32 shift3 : 3;
    u32 shift4 : 3;
    u32 shift5 : 3;
    u32 shift6 : 3;
    u32 shift7 : 3;
    u32 shift8 : 3;
    u32 reserved : 5;
}fir_shift_t;

typedef struct gk_fir_s
{
    fir_coef_one_dir_t coefs[9];
    fir_shift_t        shift;
}gk_fir_t;


typedef struct img_dsp_input_param_s
{
    icore_cfa_leakage_filter_t      cfa_lk_filter;
    u8                              anti_aliasing_enable;

    //new items
    icore_alpha_t                   alpha_info[IMG_MODE_NUMBER];
    max_change_t                    max_change_info[IMG_MODE_NUMBER];
    u8                              fir_mode; // 0:5x5 + spat, 1:7x7
    u8                              spat_filt_mode; // 0:conv, 1:iden 2:dir_shp
    fir_t                           fir_info_5x5[IMG_MODE_NUMBER];
    fir_t                           fir_info_5x5_aligned[IMG_MODE_NUMBER];
    fir_t                           fir_info_7x7[IMG_MODE_NUMBER];
    //fir_t                           fir_info[IMG_MODE_NUMBER];
    coring_table_t                  coring_info[IMG_MODE_NUMBER];
    //added by Hu Yin 15-08-2017 for GK7202
    coring_table_t                  coring_mv_info[IMG_MODE_NUMBER];
    //end

    u8                              luma_high_freq_nf_str[IMG_MODE_NUMBER];
    u8                              sharpening_signal_retain_level[IMG_MODE_NUMBER];
    sharpen_level_t                 minimum_sharpeness_level[IMG_MODE_NUMBER];
    sharpen_level_t                 overall_sharpeness_level[IMG_MODE_NUMBER];
    spatial_filter_t                spatial_filter_info[IMG_MODE_NUMBER];
    icore_dir_shp_t                 dir_shp_info;
    chroma_noise_reduction_t        chroma_noise_reduction_info;
    //u8                              color_dependent_noise_reduction_enable;
    still_cap_info_t                still_capture_info;
    u8                              def_blc_enable;
    s16                             deferred_blc_amount[3];
    u8                              art_strength;
    u8                              art_shift;
    u8                              art_cor;
    u8                              art_alpha;
    u16                             spat_filt_adaptation;
} img_dsp_input_param_t;


// Data structure
typedef struct dsp_param_s
{
    u8*                          gk_sharp_str_coeff1[IMG_MODE_NUMBER];
    u8*                          gk_sharp_str_coeff2[IMG_MODE_NUMBER];
    u8*                          gk_sharp_str_coeff1_s[IMG_MODE_NUMBER];
    u8*                          gk_sharp_str_coeff1_w[IMG_MODE_NUMBER];
    u8*                          gk_sharp_str_coeff3[IMG_MODE_NUMBER];
    u8*                          gk_sharp_str_coeff1_mv[IMG_MODE_NUMBER];
    u8*                          gk_sharp_str_coeff2_mv[IMG_MODE_NUMBER];
    u8*                          shp_edge_str[IMG_MODE_NUMBER];
    u8*                          shp_mv_edge_str[IMG_MODE_NUMBER];
    ISP_LumaSharpeningT          luma_sharpen_info[IMG_MODE_NUMBER];
    ISP_FilterLumaSharpConfigurateT        luma_sharpen_fir_info[IMG_MODE_NUMBER];
    ISP_LumaSharpMiscCtrlT       luma_sharpen_misc_info[IMG_MODE_NUMBER];

    ISP_LumaSharpEdgeCtrlT       luma_sharpen_edge_info[IMG_MODE_NUMBER];
    ISP_LumaSharpBlendCtrlT      luma_sharpen_blend_info[IMG_MODE_NUMBER];
    ISP_LumaSharpLevelCtrlT      luma_sharpen_level_min_info[IMG_MODE_NUMBER];
    ISP_LumaSharpLevelCtrlT      luma_sharpen_level_all_info[IMG_MODE_NUMBER];
    ISP_LumaSharpLevelCtrlT      luma_sharpen_level_max_num[IMG_MODE_NUMBER];
    ISP_LumaSharpLevelCtrlT      luma_sharpen_level_fir2[IMG_MODE_NUMBER];
    ISP_LumaSharpLevelCtrlT      luma_sharpen_level_fir1[IMG_MODE_NUMBER];
    ISP_FilterDemoasicSetupT     demoasic_filter_info[IMG_MODE_NUMBER];
}dsp_param_t;

/************ AF data ********************/

typedef enum
{
/* CAF stm list */
    INIT_CAF =    10,
    INIT_CoarseCAF = 11,
    CoarseCAF = 12,
    INIT_FineCAF = 13,
    FineCAF = 14,
    INIT_Watch_CAF = 15,
    Watch_CAF = 16,
    WAIT_VD_CAF = 17,
    ZTRACK_CAF = 18,

/* SAF stm list */
    INIT_SAF = 30,
    CoarseSAF = 31,
    INIT_FineSAF = 32,
    FineSAF = 33,
    DONE_SAF = 34,
    FAIL_SAF = 36,
    IDLE_SAF = 37,
    WAIT_VD_SAF = 38,

/* ZTRACK stm list */
    INIT_ZT = 50,
    LINEAR_ZT = 51,
    CHG_ZONE_ZT = 52,
    NONLINEAR_ZT = 53,
    DONE_ZT = 54,

/* CALIB stm list */
    INIT_CALIB = 61,
    DO_CALIB = 62,
    AFTER_CALIB = 63,
    FAIL_CALIB = 64,

/* CALIB SAF stm list*/
    INIT_CALIB_SAF = 75,
    DO_CALIB_SAF_ZOOM_REVERSE = 76,
    AFTER_CALIB_SAF_ZOOM_REVERSE = 77,
    DO_CALIB_SAF = 78,
    AFTER_CALIB_SAF = 79,
    WAIT_VD_CALIB_SAF = 80,
}af_stm_t;

typedef enum{
    FULL_WINDOWS = 1,
    MEDIAN_WINDOWS = 2,
    SMALL_WINDOWS = 3,
    MICRO_WINDOWS = 4,
}af_ROI_t;

typedef struct af_algo_param_s
{
    af_stm_t af_stm;
    af_stm_t af_next_stm;
    s8 direction;
    af_ROI_t ROI_size;
}af_algo_param_t;

typedef struct white_adj_s {
    s32 r_ratio;
    s32 b_ratio;
} white_adj_t;


/*
* calculate the statictis data
*/
typedef struct fv_s
{
    u32 occ_flag; // occupide flag
    u32 fv1_data;
    u64 fv2_data;
    u32 fv3_eng;
    u32 luma_avg;
    u32 luma_tile_diff;
    u32 fv1_avg;
    u32 fv2_avg;
    u32 fvg_data_avg;
    u32 fv1_tile_diff;
    u32 fv2_tile_diff;
    u32 fv3_tile_diff;
}af_fv_t;


typedef struct dragon_s
{
    af_fv_t fv_store[128];
    u32 dist_idx[128]; // distance index

    s16 fv_store_idx;
    u16 store_len;
    u32 store_cnt;
    u16 store_delay;

    u32 f_dof; // Depth of Field

    u32 fv_rev_fac; //fv reverse factor
    u64 fv_rev_TH; //fv reverse threshold

    s32 fv_max_TH;
    u8 store_af_stm;

    u16 skip_frame_TH;
    u8  clr_q_flg;
    u8  af_range_flg;

    u16 skip_frame;
    u32 search_len_TH;
    u16 ev_inx_diff_TH;
    u16 fagc_inx_diff_TH;
    u16 mk_dir_TH;
    u16 sec_chg_TH;
    u16 sec_chg_val;
    s32 fm1;
    s32 fm2;
    s32 f_step;
    u16 pps_max;
}af_dragon_t;

typedef struct af_result_s
{
    // for algo internal use
    s32 dist_idx_src; // 0~2047
    s32 dist_idx_dest; // 0~2047
    float zoom_idx_src;
    float zoom_idx_dest;

    s32 dist_idx_cur; 

    u16 pps;
}af_result_t;

typedef struct zoom_s
{
    s32 a;
    s32 b;
    s32 c;
    u32 zoom_pluse;
    u16 total_step;
} zoom_t;

typedef struct af_fir_s
{
    s8 coff[9];
}af_fir_t;

typedef struct zoom_param_s
{
    u16    pps_max;
    u8    fvg_enable;
    u16    skip_frame_TH;
    u16    ev_idx_diff_TH;
    u16    Csf;//Coarse CAF step_factor
    u16    Fsf; //Fine CAF step_factor
    u8    Cpk_th;//Coarse CAF peak down threshold
    u8    Fpk_th;//Fine CAF peak down threshold
}zoom_param_t;

typedef struct af_param_s
{
    s16    init_pps;
    u16    zoom_thread_sleep_time;
    u8     init_zoom_idx;
    u8     start_zoom_idx;
    u8     begin_zoom_idx;
    u8     end_zoom_idx;
    u8     zoom_reverse_idx;
    s16    z_inbd;
    s16    z_outbd;
    u16    zoom_reverse_after_z;
    u16    zoom_reverse_after_f;
    s16    f_nbd;
    s16    f_fbd;
    u16    calib_saf_step;
    s32    calib_saf_b;
    u8     zoom_enhance_enable;
    s16    dist_idx_init;
    u8     zoom_dir;
    u8     zoom_break_idx;
    u16    manual_focus_step;
    float  manual_zoom_step;
    af_fir_t spacial_fir;
    u16 DOF_param;
    zoom_param_t zoom_param[32];
	s16    dist_index_reverse_err_min;
	s16    dist_index_reverse_err_max;
	s16    zoom_reverse_err_min;
	s16    zoom_reverse_err_max;
    s16    saf_dist_index_reverse_err_max;
    s16    saf_f_nbd;
    s16    saf_f_fbd;
    u8     saf_optical_pos_offset;
    u16    optical_focus_length;
	double  zoom_slope;
	double  zoom_offset;
}af_param_t;

typedef struct af_calib_s
{
/*input*/
    s32 dist_idx[2];
    u8 zoom_idx; //PI init zoom_position
/*internal use*/
    s32 top[2][32];

    s32 top_all[4][32];
    //u32 top_value[4][32];
    s32 pulse_curr;
    s32 min_pulse;
    s32 max_pulse;
    u8  first_do;
/*output*/
    s32 a[32];
    s32 b[32];
}af_calib_t;

typedef struct af_calib_saf_s
{
/*inner use*/
    s32 focus_reverse_err[32];
    s32 zoom_reverse_err[32];
/*input and output*/
    s32 c[32+2];
    s32 length;
       
}af_calib_saf_t;


typedef struct af_sta_obj_s
{
    u8 caf_win_size;
    u8 saf_win_size;

    u32 af_tile_size;
    u32 af_tile_fac;
    u16 af_eng_shift;


    u8 fv1_shift;
    u8 fv2_shift;
    u8 fv3_shift;

    u32 fv1_tile_diff;
    u32 fv1_data;
    u32 fv1_avg;

    u32 fv2_tile_diff;
    u64 fv2_data;
    u32 fv2_avg;

    u32 fv3_tile_diff;
    u32 fv3_eng;
    u32 fvg_data_avg;

    u32 lum_avg;
    u32 lum_tile_diff;

} af_sta_obj_t;

typedef struct awb_weight_window_s
{
    u32    tiles[384];
}awb_weight_window_t;

typedef struct dyn_tone_proc_s
{
    pthread_cond_t dyn_tone_cond;
    pthread_mutex_t dyn_tone_mutex;
}dyn_tone_proc_t;


typedef struct gmf_drv_param_s
{
    u8                      adj_enable;
    u8                      th_enable;
    u8                      alpha_th;
    u8                      max_change_th;
    u8                      alpha_coff;
    u8                      max_change_coff;
    u16                     mv_percent[3];
	u16                     mv_percent3;
    u8                      mv_count;
    u8                      mv_range;
	u16                     luma_diff_th1;
	u16                     luma_diff_th2;
    u8                      static_alpha_y_max;
    u8                      static_alpha_y_min;
    u8                      static_threshold_y_1;
    u8                      static_threshold_y_2;
    u8                      static_alpha_uv_max;
    u8                      static_alpha_uv_min;
    u8                      static_threshold_uv_1;
    u8                      static_threshold_uv_2;

    u8                      move_alpha_y_max;
    u8                      move_alpha_y_min;
    u8                      move_threshold_y_1;
    u8                      move_threshold_y_2;
    u8                      move_alpha_uv_max;
    u8                      move_alpha_uv_min;
    u8                      move_threshold_uv_1;
    u8                      move_threshold_uv_2;

    u8                      static_alpha_y_max2;
    u8                      static_alpha_y_min2;
    u8                      static_threshold_y_12;
    u8                      static_threshold_y_22;
    u8                      static_alpha_uv_max2;
    u8                      static_alpha_uv_min2;
    u8                      static_threshold_uv_12;
    u8                      static_threshold_uv_22;

    u8                      move_alpha_y_max2;
    u8                      move_alpha_y_min2;
    u8                      move_threshold_y_12;
    u8                      move_threshold_y_22;
    u8                      move_alpha_uv_max2;
    u8                      move_alpha_uv_min2;
    u8                      move_threshold_uv_12;
    u8                      move_threshold_uv_22;

    u8                      static_alpha_y_max3;
    u8                      static_alpha_y_min3;
    u8                      static_threshold_y_13;
    u8                      static_threshold_y_23;
    u8                      static_alpha_uv_max3;
    u8                      static_alpha_uv_min3;
    u8                      static_threshold_uv_13;
    u8                      static_threshold_uv_23;

    u8                      move_alpha_y_max3;
    u8                      move_alpha_y_min3;
    u8                      move_threshold_y_13;
    u8                      move_threshold_y_23;
    u8                      move_alpha_uv_max3;
    u8                      move_alpha_uv_min3;
    u8                      move_threshold_uv_13;
    u8                      move_threshold_uv_23;
}gmf_drv_param_t;

typedef enum
{
    IMG_MSG_ANTIFLICKER = 1,
    IMG_MSG_METERMODE,
    IMG_MSG_BACKLIGHT,
    IMG_MSG_SHUTTER_LIMIT,
    IMG_MSG_SATURATION,
    IMG_MSG_CONTRAST,
    IMG_MSG_AUTO_CONTRAST,
    IMG_MSG_AUTO_CONTRAST_STR,
    IMG_MSG_HUE,
    IMG_MSG_BRIGHTNESS,
    IMG_MSG_SHARPNESS,
    IMG_MSG_SHARPEN_LEVEL_EX,
    IMG_MSG_DNR3D_LEVEL_EX,
    IMG_MSG_WB_ENV,
    IMG_MSG_WB_MODE,
    IMG_MSG_WB_METHOD,
    IMG_MSG_AE_MODE,
    IMG_MSG_AE_HLC_ENABLE,
    IMG_MSG_AE_LOAD_EXP_LINE,
    IMG_MSG_AE_SET_BELT,
    IMG_MSG_AE_SET_TARGET_RATIO,
    IMG_MSG_AE_SET_SPEED,
    IMG_MSG_AE_SET_KNEE,
    IMG_MSG_ENABLE_LOCAL_EXP,
    IMG_MSG_ENABLE_AWB,
    IMG_MSG_ENABLE_AE,
    IMG_MSG_ENABLE_AF,
    IMG_MSG_ENABLE_ADJ,
    IMG_MSG_AF_MODE,
    IMG_MSG_AF_SET_RANGE,
    IMG_MSG_AF_SET_ROI,
    IMG_MSG_AF_SET_ZOOM,
    IMG_MSG_AF_SET_FOCUS,
    IMG_MSG_AF_RESET,
    IMG_MSG_AF_SET_MANUAL_MODE,
    IMG_MSG_LENS_SET_IRCUT,
    IMG_MSG_SET_COLOR_STYLE,
    IMG_MSG_SET_BW_MODE,
//added by Hu Yin 08-03-2017
    IMG_MSG_SET_LOW_BITRATE_MODE,
//end
    IMG_MSG_SET_EXTRA_BLC,
    IMG_MSG_SET_AE_ROI,
    IMG_MSG_AWB_SET_WB_SHIFT,
    IMG_MSG_AWB_SET_SPEED,
    IMG_MSG_AWB_SET_CALI_DIFF_THR,
    IMG_MSG_AWB_VIG_GAIN_TAB,
    IMG_MSG_ADJ_YUV_EXTRA_BRIGHTNESS,
    IMG_MSG_AWB_SET_CUSTOM_GAIN,
    IMG_MSG_SET_SYNC_TIME_DELAY,
    IMG_MSG_SET_AUTO_SPATIAL_NOISE_FILTER_ENABLE,
    IMG_MSG_SET_AUTO_SHARPEN_ENABLE,
    IMG_MSG_ENABLE_VPS_3D,
    IMG_MSG_VPS_CONTRAST,
    IMG_MSG_SET_CFA_EX,
    IMG_MSG_EXIT,
    IMG_MSG_LENS_SHADING_ENABLE,
}img_msg_id_t;

typedef struct img_msg_s
{
    img_msg_id_t msg_id;
    int param[127];
}img_msg_t;

typedef struct
{
    char        *thread_name;
    pthread_t   thread_id;
    int         thread_exit_flag;
    sem_t       thread_sync;
    void        *thread_arg;
    int         (*thread_init)(void *);
    void        (*thread_exit)(void *);
    void        (*thread_loop)(void *);
} ThreadRunInfoT;

typedef void* (*img_malloc)(u32 size);
typedef void (*img_free)(void *ptr);
typedef void* (*img_memset)(void *s, int c, u32 n);
typedef void* (*img_memcpy)(void *dest, const void *src, u32 n);
typedef int (*img_ioctl)(int handle, int request, ...);
typedef int (*img_print)(char*format, ...);

typedef struct
{
    img_malloc  malloc;
    img_free    free;
    img_memset  memset;
    img_memcpy  memcpy;
    img_ioctl   ioctl;
    img_print   printf;
} ImageOperate;

typedef struct
{
    af_algo_param_t             af_control_param;
    af_dragon_t                 af_drg_params;
    af_result_t                 af_result;
    u32                         fv1_data;
    u64                         fv2_data;
	u64                         fv2_data_all[8000];
    u32                         fvg_data_avg;

    /* max/min value inside a af_drg_params*/
    u32                         fv1_data_max;
    u32                         fv1_data_min;
    u64                         fv2_data_max;
    u64                         fv2_data_max_direct;
	u64                         fv2_data_max_pre;

    u64                         fv2_data_min;
    u32                         fvg_data_avg_max;
    u32                         fvg_data_avg_min;

    s16                         wait_for_vd;
    u16                         interval_cnt;

    s32                         af_fv1_top;
    s32                         af_fv1_bot;
    s32                         af_fv2_top;
    s32                         af_fv2_top_direct;
	s32                         af_fv2_top_pre;
	s16                         dist_index_reverse_err;

    s32                         af_fv2_bot;
    s32                         af_fvg_top;
    s32                         af_fvg_bot;
    u32                         pre_fvg_data_avg;

    u8                          fvg_down_count;
    u8                          first_sign;
    u8                          af_peak_sign;
    u8                          f_nbd_sign;
    u8                          f_fbd_sign;
    u8                          af_cali_sign;

    s32                         af_dis;               //searched distance in Fine af
    int                         video_lum_diff;
    int                         video_fv2_eng;
    int                         video_fv1_diff;
    int                         fvg_enable;
    s32                         f_nbd;
    s32                         f_fbd;
    u32                         gain_shift;
    u32                         gain_shift_pre;
    s32                         defalut_fd_val;
    s32                         defalut_nd_val;
    zoom_t                      *af_zoom_param;
    af_calib_saf_t              calib_saf_out;
    af_param_t                  af_G_param;
    af_calib_t                  cali_out;
    ISP_StatisticsSetupT        af_statistic_config;
    af_control_t                af_ctrl_param;
    lens_control_t              lens_ctrl;
    lens_dev_drv_t              lens_drv;

    /***watch_caf local data*****/
    u8                          changed_sign;

    /***af_debug_control local data*****/
    s8                          direction;
    s8                          skip_frame;
    s32                         pulse_curr;
    s32                         pulse_f;
    s32                         pulse_n;

    /***do_calib local data*****/
    s8                          direction_for_do_calib;
    s8                          direction_pre_for_do_calib;
    s8                          skip_frame_for_do_calib;
    u8                          min_flag_for_do_calib;
    u8                          max_flag_for_do_calib;
    u8                          down_cnt_for_do_calib;

    /***af_mode_switch local data*****/
    af_mode                     pre_mode;

    /***feed_dragon local data*****/
    awb_weight_window_t         af_luma_fir;
    af_sta_obj_t                af_sta_obj_GK;
    u32                         fv1_tile_cum[40];
    u32                         fv2_tile_cum[40];
    u32                         fvg_tile_cum[384];
    u32                         lum_tile_cum[40];

	u32                         lensID;
    u8                          done_saf_set_mode;
    u8                          calib_mode;
    void(*pgetLensParams)(s32* pLensParams, u16 length);
    u8                          gpioArray[3];
} Isp3AContentAF;

typedef struct
{
    awb_control_mode_t          awb_mode;
    awb_work_method_t           awb_algo;
    awb_environment_t           awb_env;
    wb_gain_t                   awb_gain_table[WB_MODE_NUMBER];
    awb_lut_t                   awb_param_table;
    awb_lut_idx_t               awb_param_table_idx[20];
    wb_cali_t                   awb_cali_params;
    u8                          light_weight_table[20];
    u16                         awb_cd_r_thr;
    u16                         awb_cd_b_thr;
    wb_gain_t                   awb_low_param_h;
    wb_gain_t                   awb_hig_param_l;
    wb_gain_t                   awb_hig_param_h;
    wb_gain_t                   awb_low_param_l;
    awb_gain_t                  awb_gain;

    u8                          awb_first_sign;
    u8                          awb_first_sign_for_wb_gain;
    u8                          awb_dec_fail_cnt;
    u8                          awb_stable;
    u8                          hdr_sensor_sign;
    u16                          awb_speed;
    u8                          init_state_sign;
    u8                          awb_day_night_mode;     //LYX 20160720 0:Lumi-ircut, 1:ADC-ircut
    u8                          apply_ls_awb_enable;     //0;
    u32                         ls_awb_table_shift;
    s16                         new_speed;
    s16                         awb_frames_cnt;
    u32                         diff_m_a;
    u32                         diff_m_a2;
    wb_gain_t                   curr_still_video_gain;
    wb_gain_t                   pre_video_gain;
    wb_gain_t                   awb_compute_wb_gain;
    wb_gain_t                   wb_gain_init_val;

    adj_aeawb_control_t         auto_param_noise_control;

    u16                         ls_awb_gain_r[MAX_AWB_TILE_NUM];
    u16                         ls_awb_gain_gr[MAX_AWB_TILE_NUM];
    u16                         ls_awb_gain_gb[MAX_AWB_TILE_NUM];
    u16                         ls_awb_gain_b[MAX_AWB_TILE_NUM];

    /****************awb_video_wb_gain_diff local data*****************/
    s32                         wb_diff_cnt;
    u32                         pre_diff;
    u32                         diff;
    u32                         pre_raw_diff;
    u32                         raw_diff;
    u32                         wa_diff_same;

    u16                         diff_a_ref;
    u16                         wb_diff_ref;
    u8                          step_a_spd;
    u8                          awb_step_speed;
    u8                          d_a_diff_sign;
    u8                          wb_diff_sign;
    u8                          awb_stable_sign;
    white_adj_t                 awb_shift;
    /*********************end**************************************/
} Isp3AContentAWB;

typedef struct
{
    u8                          apc_uv_scale_shift;
    u16                         apc_wdr_shift;
    u8                          apc_nf_reset_sign;
    u8                          apc_exp_reset_sign;
    u8                          apc_black_level_reset_sign;
    u8                          apc_sharp_ml_sign;
    u8                          auto_snf_sign;
    //static u8 auto_3d_denoise_filter_flag = 1;
    u8                          auto_sharp_filter_sign;
    u8                          apc_sharp_level_sign;
    u8                          adj_sharp_manu;
    u8                          adj_cfa_manu;
    u8                          adj_vps_3d_manu;
    u16                         apc_wdr_auto;
    u8                          apc_bw_cntl;
    u8                          apc_bw_reset_sign;
    u8                          apc_bw_reset_3dnr_sign;
    u8                          apc_en_extra_brightness;
    int                         extra_brightness;
    u8                          apc_anti_aliasing;
    u8                          pre_low_sum;
    u16                         pre_param_index;

    u8                          sharp_temp_mode;

    dbp_correction_t            apc_bad_pixel_correct;
    ISP_FILTER_CFA_NoiseSetupT  apc_cfa_noise_filter;
    chroma_median_filter_t      apc_color_denoise_filter;
    chroma_scale_filter_t       apc_uv_scale_table;
    spatial_filter_t            apc_spatial_filter;
    sharpen_level_t             apc_sharpe_lm;
    sharpen_level_t             apc_sharpe_level;
    adj_l_lut_t                 apc_dmc_sharp_str;
    //added by Hu Yin 15-08-2017 for GK7202
    depurple_fringe_t           adj_dpf_params;
    nlm_noise_filter_t          adj_nlm_params;
    variance_guided_filter_t    adj_vgf_params;
    demosaic_noise_reduction_t  adj_dms_params;
    chroma_median_filter_ex_t   adj_cmn_ex_params;
    local_contrast_enhancement_t adj_lce_params;
    bilateral_filter_t          adj_blf_params;
    gmd_cfa_t                   adj_gmd_params;
    pmv_t                       adj_pmv_params;
    globle_motion_filter_t      adj_gmf_params;
    //end
    //added by Hu Yin 22-04-2016
    //vps_stg1_yuv2d_t            adj_vps_stg1_yuv2d;
    //vps_stg2_yuv3d_t            adj_vps_stg2_yuv3d;
    //vps_stg3_contrast_t         adj_vps_stg3_contrast;
    //end
    //encode_realtime_cntl_t      adj_encode_params;
    u8                          wdr_ok;
    u8                          wdr_manual_sign;
    u16                         pre_wdr_ratio;
    s8                          wdr_frame_cnt;

    cc_3D_File                  apc_ccm[5];
    cc_3D_File                  apc_ccm_output;
    u16                         tone_curve_m[256];
    local_wdr_t                 apc_wdr_table;

    image_property_t            img_attr;
    sharpen_property_t          sharpen_attr;
    cfa_denoise_property_t      denoise_property;
    dnr3d_property_t            dnr3d_attr;
    image_low_bitrate_property_t    image_low_bitrate_property;
    blc_level_t                 apc_blc_corr;

    u8                          pre_color_shift;

    wb_gain_t                   pre_wb_gain;
    u8                          *p_ccm_data;
    u16                         pre_index;
    u16                         pre_vgf_table_index;
    u8                          ev_interpo_sign;

    u16                         pre_shap_tamplate_index;
    u16                         pre_edge_shrp_str_index;

    u8                          run_chk;
    u8                          run_chk_1;
    u8                          run_chk_2;
    //u8                          run_chk_3;
    u8                          run_chk_4;
    u8                          run_chk_5;
    int                         mv_motion_blks;
} Isp3AContentCTL;

typedef struct
{
    u16             line_number;
    u16             belt;   //all exp lines, max line number for current use
    u32             ae_sensor_gain;
    u8              typeofsensor;

    u8              hdr_sensor_sign;
    u8              isp_dgain_enable; //isp_dgain
    u32             max_isp_dgain;
    u16             ae_target;
    u16             ae_target_params;
    u8              ae_first_sign;
    ae_info_t       curr_ae_info;
    u8              hsum_thr;
    u8              lsum_thr;
    u16             his_low_luma_sum;
    u16             his_high_luma_sum;
    int             ae_rgb_lightness_val;
    s16             exp_line_site;
    int             flicker_mode;
    ae_metering_mode    meter_mode;
    u8              auto_luma_control;
    u8              auto_luma_control_cpy;
    u8              ae_backlight;
    s16             ae_target_shift;
    dc_iris_cntl    aperture_cntl;
    int             ae_lightness_value;

    int             ae_speed_ctl; //NORMAL_MODE;
    u32             ae_speed_level;

    int             ae_skip_frames;
    int             ae_curr_step;
    int             new_step;
    int             pre_step;
    u8              ae_step_ctl;

    //added by Hu Yin 08-03-2017
    int             low_bitrate_mode_enable;
    u8              target_apc_low_bitrate;
    int             ae_skip_frames_low_bitrate;
    u8              target_apc;
    //end

    int             light_ratio;
    int             prev_light_ratio;
    int             light_ratio_befor;

    u8              hlc_sign;
    u8              ae_stable_sign;
    u8              set_init_state_sign;
    joint_t         curr_exp;
    u16             curr_num;
    u16             max_gain_index;

    line_t          ae_exp_table[MAX_LINE_NUMBER];
    u32             ae_param_steps[MAX_LINE_NUMBER];
    u32             comp_for_sensor_gain[AGC_DGAIN_TABLE_LENGTH];
    u32             comp_for_sensor_shutter[SHUTTER_DGAIN_TABLE_LENGTH];
} Isp3AContentAE;

typedef struct
{
    int                 fd;
    ThreadRunInfoT      vi_thread;
    ThreadRunInfoT      control_thread;
    ThreadRunInfoT      contrast_thread;
    ThreadRunInfoT      zoom_thread;

    pthread_mutex_t     load_isp_param_mutex;
    pthread_mutex_t     wait_sync_mutex;
    pthread_cond_t      wait_sync_cond;
    dyn_tone_proc_t     d_tone_proc;

    char                curSensorName[32];
    char                *sensorBinBuf;
    u8                  dynamic_load_param_sign;
    u8                  apc_set_init_sign;
    aaa_cntl_t          cntl_3a;

    u8                  auto_tone_cntl_enable;
    float               auto_tone_param_shift;
    float               auto_tone_shift;
    u16                 tone_change_speed;
    embed_hist_stat_t   sensor_hist_data_hdr;

    int                     bw_mode_enable;
    int                     mv_info_blks;
    int                     luma_delta_abs;
    int                     table_index_old;
    u8                      alpha_id;
    int                     mv_info_blks_pre;
    int                     mv_info_blks_pre_pre;
    int                     weak_movement_mode_enable;
    int                     parameters_status;
    int                     weak_move_count;
    int                     move_switch_count;
    u8                      ccm_enable;
    u8                      typeofsensor;
    u8                      def_blc_enable;
    u8                      anti_aliasing_enable;
    u8                      vps_contrast_enable;


    int                     vin_width;
    int                     vin_height;
    int                     mv_info_blks_total;
    //u8                      g_vps_pre_yuv2d_fm_table_mode[2];
    img_color_style         rgb2yuv_style;
    img_isp_capability_t    isp_capability;

    u16                     sensor_max_db;
    u16                     sensor_double_step;
    bayer_pattern           sensor_pattern;
    u16                     isp_gain;
    int                     blc_extra;
    u8                      sync_time_delay;  //ms

    img_dsp_input_param_t   GK_isp_input_param;
    dsp_param_t             Gk_isp_param;
    low_iso_param_t         Gk_low_luma_param;
    digital_sat_level_t     dgain_thr_cpy;
    ISP_StatisticsConfigT   aeawb_cfg_cpy;
    spatial_filter_t        sf_info_cpy;
    fir_t                   shp_str_cpy;
    image_mode              video_mode_cpy;
    rgb_to_yuv_t            r2y_matrix_cpy;
    ISP_StatisticsSetupT    af_cfg_cpy;
    blc_level_t             blc_cpy;
    u32                     sht_time_cpy;
    u32                     shutter_index_cpy;
    s32                     gain_cpy;
    s32                     gain_index_cpy;
    wb_gain_t               wb_gain_cpy;
    u16                     dgain_cpy;
    u8                      sensor_hdr_mode_cpy;
    u8                      anti_aaf_strength_cpy;
    dbp_correction_t        bad_pixel_c_cpy;
    cfa_leakage_filter_t    cfa_lkf_cpy;
    ISP_FILTER_CFA_NoiseSetupT  cfa_denoise_cpy;
    local_wdr_t             wdr_cpy;
    tone_curve_t            tone_curve_cpy;
    chroma_scale_filter_t   uv_scale_cpy;
    bilateral_filter_t      bilateral_filter_info_cpy;
    pmv_t                   pmv_cpy;
    chroma_median_filter_t  color_noise_cpy;
    max_change_t            max_change_cpy;
    sharpen_level_t         shp_lm_cpy;
    sharpen_level_t         shp_level_cpy;
    tuning_LE_str_t         le_str_cpy;
    ISP_ZoomFactorT         dzoom_factor_cpy;
    u8                      alpha_y_cpy_max[8];
    u8                      alpha_y_cpy_min[8];
    u8                      alpha_uv_cpy_max[8];
    u8                      alpha_uv_cpy_min[8];
    u8                      threshold_y_1_cpy[8];
    u8                      threshold_y_2_cpy[8];
    u8                      threshold_uv_1_cpy[8];
    u8                      threshold_uv_2_cpy[8];

    u8                      alpha_y_cpy_max_option[8];
    u8                      alpha_y_cpy_min_option[8];
    u8                      alpha_uv_cpy_max_option[8];
    u8                      alpha_uv_cpy_min_option[8];
    u8                      threshold_y_1_cpy_option[8];
    u8                      threshold_y_2_cpy_option[8];
    u8                      threshold_uv_1_cpy_option[8];
    u8                      threshold_uv_2_cpy_option[8];
    //encode_realtime_cntl_t  encode_param_cpy;
    u16                     vgf_str_cpy;
    gmf_drv_param_t         gmf_param_cpy;

    ISP_RgbStatT            *gk_rgb_statistics;
    ISP_CfaStatT            *gk_cfa_statistics;
    ISP_RgbStatT            *gk_hist_statistics;
    aaa_tile_report_t       act_tile_num;

    u8                      adj_focus_statistics_enable;
    pthread_mutex_t         statMutexLock;
    pthread_cond_t          statCond;
    ae_data_t               st_ae_info[MAX_AE_TILE_NUM];
    awb_data_t              st_awb_info[MAX_AWB_TILE_NUM];
    ISP_CfaAfStatT          st_af_cfa_info[MAX_AF_TILE_NUM];
    ISP_CfaAfStatT          st_af_rgb_info[MAX_AF_TILE_NUM];
    ISP_HistogramStatT      st_dsp_histo_info;

    u16                     hp_detect_thr_param[NUM_BAD_PIXEL_THD];
    u16                     dp_detect_thr_param[NUM_BAD_PIXEL_THD];
    u32                     isp_tone_curve[TONE_CURVE_SIZE];
    u16                     color_noise_k_table[24];
    u8                      edge_str_cntl_ratio[256];
    u8                      color_noise_isp_data[4][2][9];

    ae_info_t               ae_video_info;
    ae_info_t               ae_still_info;
    ISP_ColorCorrectionT    ccm_info;
    cc_reg_file             GK_ccr;

    adj_param_t             gk_apc_param;
    adj_param_t             gk_apc_param_ir;
    adj_param_t             gk_apc_param_weak_move;
	adj_param_t             gk_apc_param_interp;
    rgb_to_yuv_t            gk_rgb2yuv[4];
    chroma_scale_filter_t   gk_uv_scale;
    lens_shading_property_t gk_lens_shading;
    warp_config_t           gk_warp_config;
    line_t                  gk_50hz_lines[4];
    line_t                  gk_60hz_lines[4];
    img_awb_param_t         gk_awb_param;
    ISP_StatisticsConfigT   gk_tile_config;
    local_wdr_t             is_manual_LE[3];
    u8                      gk_h_dlight_range[2];
    u32                     gk_p_ae_gain_dgain[AGC_DGAIN_TABLE_LENGTH];
    u32                     gk_p_ae_sht_dgain[SHUTTER_DGAIN_TABLE_LENGTH];
    pipeline_control_t      apc_filters_ctl;

    /* isp control loop local data */

    //u8                      vps_alpha_down_sign;
    //u8                      vps_alpha_recover_sign;
    //u8                      vps_alpha_down_cnt;
    //u8                      vps_ae_low_light_cnt;
    //u8                      vps_stg2_yuv3d_bk_update;
    //int                     pre_ae_luma_vps;
    //vps_stg2_yuv3d_t        vps_stg2_yuv3d_bk;
    adj_aeawb_control_t     apc_aeawb_ctl;
    adj_param_t             *gk_apc_param_tmp;
    adj_param_t             *gk_apc_param_ir_tmp;
    adj_param_t             *gk_apc_param_weak_move_tmp;
    rgb_to_yuv_t            **gk_rgb2yuv_tmp;
    chroma_scale_filter_t   *gk_uv_scale_tmp;
    local_wdr_t             **is_manual_LE_tmp;

    /* monitor  loop local data */
    img_msg_t               msg[MSG_QUEUE_SIZE];
    int                     msg_update;
    pthread_mutex_t         msg_mutex;
    /* vi  loop local data */
    pthread_mutex_t         update_mutex;
    u8                      delay_sign;
    /* contrast loop data */
    u8                      dynt_frames_skip;
    u16                     pre_tone_curve_index;
    /* control loop data */
    int                     pre_luma;
    int                     pre_gain_index;
    int                     pre_ae_flag;
	u32 lin_y_aver;
	u32 lin_y_aver_last;
    pthread_mutex_t         af_zoom_mutex;
    pthread_cond_t          af_zoom_cond;
} Isp3APublicContent;

typedef struct
{
    Isp3AContentAF      AFData;
    Isp3AContentAWB     AWBData;
    Isp3AContentAE      AEData;
    Isp3AContentCTL     ctlData;
    Isp3APublicContent  publicData;
} Isp3AContent;

typedef struct
{
    int             srcId;
    ImageOperate    operate;
    aaa_api_t       isp3A;
    Isp3AContent    content;
} Isp3AProcessor;

typedef enum {
    ISPCHIP_UNKNOWN  = -1,
    ISPCHIP_GK7102   = 1,
    ISPCHIP_GK7101   = 2,
    ISPCHIP_GK7102S  = 3,
    ISPCHIP_GK7101S  = 4,
    ISPCHIP_GK7102C  = 5,
    ISPCHIP_GK7202   = 6,
    ISPCHIP_GK7102C_A = 7,
    ISPCHIP_GK7102C_T = 8,
    ISPCHIP_GK7102C_W = 9,
} ISPChipIdEnum;


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

int Isp3AAWBContentInit(Isp3AContentAWB *content);
void awb_set_wb_ratio(Isp3AProcessor *processor, adj_aeawb_control_t *p_aeawb_ctl);
void awb_control(void *data, awb_work_method_t method, awb_control_mode_t mode, u16 tile_count, awb_data_t *p_tile_info, awb_gain_t *p_awb_gain);
int awb_control_init(void *data, awb_control_mode_t mode, wb_gain_t *menu_gain, awb_lut_t *lut_table, awb_lut_idx_t *lut_table_idx);
awb_control_mode_t awb_get_wb_cntl_mode(Isp3AProcessor *processor);
awb_work_method_t awb_get_wb_algo(Isp3AProcessor *processor);

// add by heqi
int awb_set_rb_ratio(Isp3AProcessor *processor, u8 mode, u8 ratio);
u8 awb_get_rb_ratio(Isp3AProcessor *processor, u8 * mode, u8* ratio);

//end
void awb_set_diff_thr_for_cali(Isp3AProcessor *processor, u16 r_thr, u16 b_thr);
void awb_set_white_tile_wb_shift(Isp3AProcessor *processor, wb_gain_t* org_gain, wb_gain_t* ref_gain);
void awb_set_speed(Isp3AProcessor *processor, u8 speed);
int  awb_get_speed(Isp3AProcessor *processor);
void awb_get_cali_info(Isp3AProcessor *processor, wb_gain_t* get_org_gain);
void awb_set_custom_gain(Isp3AProcessor *processor, wb_gain_t* cus_gain);
int awb_set_wb_init_state(Isp3AProcessor *processor, wb_gain_t *p_awb_gain);
void awb_set_day_night_mode(Isp3AProcessor *processor, u8 en);
void awb_set_control_env(Isp3AProcessor *processor, awb_environment_t env);
awb_environment_t awb_get_control_env(Isp3AProcessor *processor);
void ae_set_first_exp(Isp3AProcessor *processor, int gain_index,int shutter_index);
//Image property adjust APIs
int apc_video_init(Isp3AProcessor *processor, adj_param_t *p_adj_video_param, pipeline_control_t *p_adj_video_pipe);
int apc_video_blc_control(Isp3AProcessor *processor, u16 table_index, wb_gain_t wb_gain,adj_param_t *p_adj_video_param, pipeline_control_t *p_adj_video_pipe);//mode = 0: video preview  1: photo preview
void apc_ccm_interpo(Isp3AProcessor *processor, wb_gain_t wb_gain, u8 color_ratio,
    color_3d_t * p_color, u32 output_3d_color_addr);
void apc_ccm_interpo_with_exp(Isp3AProcessor *processor, u16 index, color_3d_t * p_color,
    u32 cc_wb_3d_table_addr, u32 output_3d_color_addr);

int apc_video_aeawb_cntl(Isp3AProcessor *processor, u16 table_index, wb_gain_t wb_gain, adj_param_t * p_adj_video_param, pipeline_control_t * p_adj_video_pipe, chroma_scale_filter_t * p_adj_color_filter_table);
int apc_video_nf_control(Isp3AProcessor *processor,u16 table_index, wb_gain_t wb_gain, u16 dzoom_step, adj_param_t * p_adj_video_param, pipeline_control_t * p_adj_video_pipe);
int apc_video_aeawb_control(Isp3AProcessor *processor, u16 table_index, adj_awb_ae_t * p_adj_param, adj_aeawb_control_t * ouput_aeawb_param);
int apc_set_hue(Isp3AProcessor *processor, int hue);
int apc_set_brightness(Isp3AProcessor *processor, int bright);
int apc_set_contrast(Isp3AProcessor *processor, int contrast);
int adj_set_vps_contrast(Isp3AProcessor *processor, int contrast);
int apc_set_saturation(Isp3AProcessor *processor, int saturation);
int apc_get_img_property(Isp3AProcessor *processor, image_property_t * p_image_property);
void apc_set_color_conversion(Isp3AProcessor *processor, rgb_to_yuv_t* rgb_to_yuv_matrix);
void apc_set_sharpness_level(Isp3AProcessor *processor, int level);
void apc_set_dnr3d_property_ratio(Isp3AProcessor *processor, int alpha_ratio, int threshold_ratio,
    int threshold_limit, int max_change_ratio);
void adj_set_cfa(Isp3AProcessor *processor, int cfa_flat_weight_ratio, int cfa_dir_weight_ratio,
    int cfa_flat_thr_ratio, int cfa_dir_thr_ratio, int edge_thr_ratio);
void adj_set_vps_3d(Isp3AProcessor *processor, int level);
void apc_set_bw_mode(Isp3AProcessor *processor, u8 en);
//added by Hu Yin 08-03-2017
void apc_set_low_bitrate_mode(Isp3AProcessor *processor, u8 en);
//end
void apc_reset_denoise(Isp3AProcessor *processor);
void apc_set_local_exp(Isp3AProcessor *processor, int level);
int apc_contrast_for_ll(Isp3AProcessor *processor, adj_param_t *p_adj_video_param,pipeline_control_t *p_adj_video_pipe, u8 ae_stable, local_wdr_t* manual_LE);
void apc_set_blc_ext(Isp3AProcessor *processor, int blc);
int apc_get_contrast_tone(Isp3AProcessor *processor, u16 * tone_curve);
int adj_set_tone_contrast_64(Isp3AProcessor *processor, u16 *tone_curve);


int apc_enable_extra_brightness(Isp3AProcessor *processor, u8 enable);
void apc_yuv_extra_brightness(Isp3AProcessor *processor, u16 gain_index, pipeline_control_t *p_adj_video_pipe);
int apc_encode_realtime_control(Isp3AProcessor *processor, u16 table_index, adj_param_t * p_adj_video_param,
                 pipeline_control_t * p_adj_video_pipe);

void apc_recover_image_property(Isp3AProcessor *processor);
int apc_hist_tone_curve(Isp3AProcessor *processor, u16 ratio,adj_param_t *p_adj_video_param, pipeline_control_t *p_adj_video_pipe);
void apc_set_init_def(Isp3AProcessor *processor, u8 enable);
void adj_get_isp_statistics_enable(Isp3AProcessor *processor, int enable);//0:disable  another:enable
int adj_get_isp_statistics(Isp3AProcessor *processor, ISP_CfaAfStatT * fh_sta,
    ISP_CfaAfStatT * fhv_sta, ae_data_t *ae_data,
    awb_data_t* awb_data, ISP_HistogramStatT* hist_data,unsigned int xms);
int adj_get_color_info(Isp3AProcessor *processor, color_3d_t *color_info);

#ifdef __cplusplus
}
#endif



#endif /* _GK_ISP3A_IMG_STRUCT_H_ */

