/*!
*****************************************************************************
** \file        src/algo/gk7101_aaa_af_algo.c
**
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
//#include <assert.h>
#include <memory.h>
#include "basetypes.h"
#include "gk_isp3a_api.h"
#include "isp3a_lens.h"
#include "isp3a_af.h"

//*****************************************************************************
//*****************************************************************************
//** Local Defines
//*****************************************************************************
//*****************************************************************************
#ifndef ABS
#define ABS(a)       (((a) < 0) ? -(a) : (a))
#endif
#ifndef MAX
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#endif
#ifndef bool
#define bool u8
#endif

#define INIT_MIN_VALUE        99999999
#define INIT_MAX_VALUE        0

#define FV1_SHIFT    3
#define FV2_SHIFT    3

#define AF_KP_Q            0
#define AF_CLR_Q        1

#define af_result_printf(format, args...)  //printf("[IMAGE INFO] " format, ##args)
#define stm_printf(format, args...) //printf("[IMAGE INFO] " format, ##args)
#define fv_printf(format, args...) //printf("[IMAGE INFO] " format, ##args)
//#define scence_printf(format, args...) // printf("[IMAGE INFO] " format, ##args)
#define scence_printf(...) //printf(__VA_ARGS__)

#define cal_lens_printf(format, args...)//printf("[IMAGE INFO] " format, ##args)
#define zt_printf(format, args...) //printf("[IMAGE INFO] " format, ##args)
#define af_printf(format, args...) printf("[IMAGE INFO] AF---" format, ##args)


typedef signed long long s64;

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

//*****************************************************************************
//*****************************************************************************
//** Local Data
//*****************************************************************************
//*****************************************************************************
static af_control_t af_ctrl_def_param = {
    CAF,
    {{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 3, 3, 3, 3, 1, 0,
    0, 1, 3, 50, 50, 3, 1, 0,
    0, 1, 4, 50, 50, 4, 1, 0,
    0, 1, 4, 50, 50, 4, 1, 0
    }},
    0,
    0,
};

static int af_count = 0;
extern af_param_t imx035_TamronDF300_param;
extern af_param_t MF_af_param;
extern af_param_t Foctek_D14_02812IR_af_param;
extern af_param_t YuTong_YT30031FB_af_param;
extern af_param_t YuTong_YT30021FB_af_param;
extern af_param_t YS05IR_F1F18_af_param;


//extern af_param_t ov5653_SY6310_param;
extern af_param_t ov5650_SY3510A_param;
extern af_param_t iu072f_af_param;

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
extern lens_dev_drv_t Foctek_D14_02812IR_dev_drv;
extern lens_dev_drv_t YuTong_YT30031FB_dev_drv;
extern lens_dev_drv_t YuTong_YT30021FB_dev_drv;
extern lens_dev_drv_t YS05IR_F1F18_dev_drv;

s32 g_focus_reverse_err = 0;
s32 g_zoom_reverse_err = 0;
int gpio_id[3] = {28, 23, 36};

//*****************************************************************************
//*****************************************************************************
//** Local Functions Declaration
//*****************************************************************************
//*****************************************************************************
static int pass_2D_fir(u8 col_num, u8 row_num, u32 * array,
    const af_fir_t * fir_coeff);
static void feed_dragon(Isp3AProcessor *processor, af_control_t * p_af_control, u16 fv_tile_count,
    ISP_CfaAfStatT * p_af_info, u16 awb_tile_count, awb_data_t * p_awb_info,
    af_dragon_t * p_dragon);
static s32 sgk_af_fpt_est(af_dragon_t * p_dragon);
static int init_saf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result);
static int coarse_saf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result);
static int init_fine_saf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_ctrl);
static int fine_saf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result);
static int done_saf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result);
static int fail_saf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result);
static int idle_saf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result);
static int wait_vd_saf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result);
static int init_caf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result);
static int init_coarse_caf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_ctrl);
static int coarse_caf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result);
static int init_fine_caf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_ctrl);
static int fine_caf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result);
static int init_watch_caf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result);
static int watch_caf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result);
static int wait_vd_caf(Isp3AProcessor *processor, af_dragon_t * p_dragon, af_result_t * p_af_result);
static int ztrack_caf(Isp3AProcessor *processor, af_result_t * p_af_result, u8 target_zm_idx);
static void Watch_runing_CAF(void);
static int af_CAF_control(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    lens_control_t * p_lens_ctrl, u8 lens_runing_flag);
static int af_SAF_control(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    lens_control_t * p_af_ctrl, u8 lens_runing_flag);
static int af_manual_control(Isp3AProcessor *processor, af_control_t * p_af_control,
    af_dragon_t * p_dragon, lens_control_t * p_lens_ctrl, u8 lens_runing_flag);
static int af_debug_control(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    lens_control_t * p_af_ctrl, u8 lens_runing_flag);

static int init_calib_saf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result, lens_control_t * p_lens_ctrl);
static int do_calib_saf_zoom_reverse(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result, lens_control_t * p_lens_ctrl);
static int do_calib_saf_focus_reverse(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result, lens_control_t * p_lens_ctrl);
static int do_calib_saf_reverse(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result, lens_control_t * p_lens_ctrl);
static int after_calib_saf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result, lens_control_t * p_lens_ctrl);
static int wait_vd_calib_saf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result);
static int af_calib_of_saf_control(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    lens_control_t * p_af_ctrl, u8 lens_runing_flag);
static int af_calib_of_zoom_length_control(Isp3AProcessor *processor, af_control_t * p_af_control,
    af_dragon_t * p_dragon, lens_control_t * p_lens_ctrl, u8 lens_runing_flag);
static int af_calib_of_focus_length_control(Isp3AProcessor *processor, af_control_t * p_af_control,
    af_dragon_t * p_dragon, lens_control_t * p_lens_ctrl, u8 lens_runing_flag);

static int init_calib(Isp3AProcessor *processor, af_dragon_t * p_dragon, lens_control_t * p_af_ctrl,
    af_calib_t * p_af_calib_info);
static int do_calib(Isp3AProcessor *processor, af_calib_t * p_af_calib_info, af_dragon_t * p_dragon,
    lens_control_t * p_af_ctrl);
static int after_calib(Isp3AProcessor *processor, af_calib_t * p_calib_result);
static int fail_calib(Isp3AProcessor *processor, af_calib_t * p_calib_result);
static int af_calib_control(Isp3AProcessor *processor, af_control_t * mode, af_dragon_t * p_dragon,
    lens_control_t * p_af_ctrl, u8 lens_runing_flag);
static void clean_dragon(Isp3AProcessor *processor, af_dragon_t * p_dragon);
static int param_adj(Isp3AProcessor *processor, af_algo_param_t * p_af_algo_param, af_dragon_t * p_dragon,
    const af_param_t * gk_af_param);
static inline s32 get_pulse(Isp3AProcessor *processor, const u8 zoom_idx, const s32 dist_idx);
static inline s32 get_dist_idx(Isp3AProcessor *processor, const u32 zoom_idx, const s32 pulse);
static inline s32 get_dist_index(s32 b, s32 pulse);
static inline s32 af_interpolate(s32 lv, s32 hv, u8 li, u8 hi, float look);
static int af_decide_pulse(Isp3AProcessor *processor, af_result_t * p_af_result,
    lens_control_t * p_af_ctrl);
static int af_rt_inf(Isp3AProcessor *processor, af_control_t * p_af_control, lens_control_t * p_lens_ctrl);
static void af_mode_switch(Isp3AProcessor *processor, af_control_t * p_af_control);
//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************
int Isp3AAFContentInit(Isp3AContentAF *content)
{
    int i;
    /* max/min value inside a af_drg_params*/
    content->fv1_data_max = 0;
    content->fv1_data_min = 0;
    content->fv2_data_max = 0;
    content->fv2_data_min = 0;
    content->fvg_data_avg_max = 0;
    content->fvg_data_avg_min = 0;

    content->pre_fvg_data_avg = 0;
    content->fvg_down_count = 0;

    content->fvg_enable = 0;
   
    content->gain_shift = 1;
    content->gain_shift_pre = 1;
    content->first_sign = 1;
    content->af_zoom_param = NULL;

    /*input*/
    content->cali_out.dist_idx[0] = 0;
    content->cali_out.dist_idx[1] = 0;
    content->cali_out.zoom_idx = 0; //PI init zoom_position
    /*internal use*/
    for (i = 0; i < 32; i++)
        content->cali_out.top[0][i] = 0;

    for (i = 0; i < 32; i++)
        content->cali_out.top[1][i] = 0;

    content->cali_out.pulse_curr = 0;
    content->cali_out.min_pulse = 0;
    content->cali_out.max_pulse = 0;
    /*output*/
    for (i = 0; i < 32; i++)
        content->cali_out.a[i] = 0;

    for (i = 0; i < 32; i++)
        content->cali_out.b[i] = 0;

    for (i = 0; i < 32; i++)
    {
       content->calib_saf_out.focus_reverse_err[i] = 0;
       content->calib_saf_out.zoom_reverse_err[i] = 0;
       content->calib_saf_out.c[i]= 0;
    }
       
    
    content->af_statistic_config.data1.af_horizontal_filter_mode = 0;
    content->af_statistic_config.data1.af_filter_select = 0;
    content->af_statistic_config.data1.af_horizontal_filter_stage1_enb = 0;
    content->af_statistic_config.data1.af_horizontal_filter_stage2_enb = 1;
    content->af_statistic_config.data1.af_horizontal_filter_stage3_enb = 0;
    content->af_statistic_config.data1.af_horizontal_filter_gain[0] = 200;
    content->af_statistic_config.data1.af_horizontal_filter_gain[1] = 0;
    content->af_statistic_config.data1.af_horizontal_filter_gain[2] = 0;
    content->af_statistic_config.data1.af_horizontal_filter_gain[3] = 0;
    content->af_statistic_config.data1.af_horizontal_filter_gain[4] = -55;
    content->af_statistic_config.data1.af_horizontal_filter_gain[5] = 0;
    content->af_statistic_config.data1.af_horizontal_filter_gain[6] = 0;
    content->af_statistic_config.data1.af_horizontal_filter_shift[0] = 6;
    content->af_statistic_config.data1.af_horizontal_filter_shift[1] = 0;
    content->af_statistic_config.data1.af_horizontal_filter_shift[2] = 0;
    content->af_statistic_config.data1.af_horizontal_filter_shift[3] = 0;
    content->af_statistic_config.data1.af_horizontal_filter_bias_off = 0;
    content->af_statistic_config.data1.af_horizontal_filter_thresh = 0;
    content->af_statistic_config.data1.af_vertical_filter_thresh = 0;
    content->af_statistic_config.data1.af_tile_fv_horizontal_shift = 8;
    content->af_statistic_config.data1.af_tile_fv_vertical_shift = 8;
    content->af_statistic_config.data1.af_tile_fv_horizontal_weight = 168;
    content->af_statistic_config.data1.af_tile_fv_vertical_weight = 87;

    content->af_statistic_config.data2.af_horizontal_filter_mode = 0;
    content->af_statistic_config.data2.af_filter_select = 0;
    content->af_statistic_config.data2.af_horizontal_filter_stage1_enb = 1;
    content->af_statistic_config.data2.af_horizontal_filter_stage2_enb = 1;
    content->af_statistic_config.data2.af_horizontal_filter_stage3_enb = 1;
    content->af_statistic_config.data2.af_horizontal_filter_gain[0] = 188;
    content->af_statistic_config.data2.af_horizontal_filter_gain[1] = 476;
    content->af_statistic_config.data2.af_horizontal_filter_gain[2] = -235;
    content->af_statistic_config.data2.af_horizontal_filter_gain[3] = 375;
    content->af_statistic_config.data2.af_horizontal_filter_gain[4] = -184;
    content->af_statistic_config.data2.af_horizontal_filter_gain[5] = 276;
    content->af_statistic_config.data2.af_horizontal_filter_gain[6] = -206;
    content->af_statistic_config.data2.af_horizontal_filter_shift[0] = 7;
    content->af_statistic_config.data2.af_horizontal_filter_shift[1] = 2;
    content->af_statistic_config.data2.af_horizontal_filter_shift[2] = 2;
    content->af_statistic_config.data2.af_horizontal_filter_shift[3] = 0;
    content->af_statistic_config.data2.af_horizontal_filter_bias_off = 0;
    content->af_statistic_config.data2.af_horizontal_filter_thresh = 0;
    content->af_statistic_config.data2.af_vertical_filter_thresh = 0;
    content->af_statistic_config.data2.af_tile_fv_horizontal_shift = 8;
    content->af_statistic_config.data2.af_tile_fv_vertical_shift = 8;
    content->af_statistic_config.data2.af_tile_fv_horizontal_weight = 123;
    content->af_statistic_config.data2.af_tile_fv_vertical_weight = 132;

    content->af_cali_sign = 0;

    content->direction = 5;
    content->pulse_curr = 0;
    content->skip_frame = 5;

    content->direction_for_do_calib = 1;
    content->direction_pre_for_do_calib = 1;
    content->skip_frame_for_do_calib = 0;
    content->min_flag_for_do_calib = 0;
    content->max_flag_for_do_calib = 0;
    content->down_cnt_for_do_calib = 0;
	content->lensID = 0;
    content->calib_mode = 0;
    content->af_ctrl_param = af_ctrl_def_param;
    content->af_G_param = MF_af_param;
    content->af_zoom_param = &(MF_DISTANCE_MAP[0]);
    content->done_saf_set_mode = 255;
	content->af_result.zoom_idx_dest = 0;
    return 0;
}

int af_control_init(void *data, af_control_t * p_af_control, void *gk_af_param,
    void *gk_zoom_map, lens_control_t * p_lens_ctrl)
{
    Isp3AProcessor *processor;
    //ImageOperate *op;
    Isp3AContentAF *content;
    int i;
    s32 pulse;
    s32 idx;
    s32 min_f_nbd = 2147483647;
    s32 max_f_fbd = -2147483648;
    processor = (Isp3AProcessor *)data;
    //op = &(processor->operate);
    content = &(processor->content.AFData);

    content->first_sign = 1;

    p_af_control->zoom_idx = content->af_G_param.init_zoom_idx;
    content->lens_ctrl.pps = content->af_G_param.init_pps;
    content->lens_ctrl.focus_update = 0;
    content->lens_ctrl.zoom_update = 0;
    content->lens_ctrl.focus_pulse = 0;
    content->lens_ctrl.af_stat_config_update = 0;
    content->af_drg_params.pps_max =
        content->af_G_param.zoom_param[content->af_G_param.init_zoom_idx].pps_max;
    content->af_result.pps = content->af_G_param.init_pps;
    content->af_result.dist_idx_dest = 0;
    content->af_result.dist_idx_src = 0;
    content->af_result.zoom_idx_src = content->af_G_param.init_zoom_idx;
    content->af_result.zoom_idx_dest = content->af_G_param.init_zoom_idx;
    content->f_nbd = content->af_G_param.f_nbd;
    content->f_fbd = content->af_G_param.f_fbd;

    content->af_drg_params.store_len = 48;
    
    if(content->calib_mode != 0)
    {
        p_af_control->workingMode = CALIB_SAF;
        for(i = 0; i< 32; i++)
        {
            if(content->af_G_param.calib_saf_b != 0)
                content->af_zoom_param[i].b = content->af_G_param.calib_saf_b;
            if(content->af_G_param.calib_saf_step != 0)
            {
                content->af_G_param.zoom_param[i].Csf =  content->af_G_param.calib_saf_step;
                content->af_G_param.zoom_param[i].Fsf =  content->af_G_param.calib_saf_step;
            }
                
        }
    }
    else
    {
        
        af_printf("zoom_param is :\n");
        for(i = content->af_G_param.start_zoom_idx; i< content->af_G_param.end_zoom_idx + 1; i++)
        {
            content->af_zoom_param[i].c =content->calib_saf_out.c[i];
            af_printf("zoom_param[%d]=%d\n", i - content->af_G_param.start_zoom_idx, content->af_zoom_param[i].c);
        }
		if (content->lensID != 0)
		{
			g_zoom_reverse_err = content->calib_saf_out.c[32];
        	g_focus_reverse_err = content->calib_saf_out.c[33];
		}
        p_af_control->workingMode = SAF;
        af_printf("Zoom reverse is %d, Focus reverse is %d\n", g_zoom_reverse_err, g_focus_reverse_err);
        content->f_nbd = content->af_G_param.saf_f_nbd;
        for(i = content->af_G_param.start_zoom_idx; i< content->af_G_param.end_zoom_idx + 1; i++)
        {
            pulse = (content->af_zoom_param[i].c -(s32) g_focus_reverse_err * 1.8);
            idx = get_dist_index(content->af_zoom_param[i].b, pulse);
            if(-idx < min_f_nbd)
                min_f_nbd = -idx;
            
        }
        if (min_f_nbd < content->af_G_param.saf_f_nbd)
        {
           content->f_nbd = min_f_nbd;
        }
      
        content->f_fbd = content->af_G_param.saf_f_fbd;
        for(i = content->af_G_param.start_zoom_idx; i< content->af_G_param.end_zoom_idx + 1; i++)
        {
            pulse = (content->af_G_param.optical_focus_length - content->af_zoom_param[i].c);
            idx = get_dist_index(content->af_zoom_param[i].b, pulse);
            if(idx > max_f_fbd)
                max_f_fbd = idx; 
        }
        if (max_f_fbd > content->af_G_param.saf_f_fbd)
        {
           content->f_fbd = max_f_fbd;
        }
        af_printf("Focus f_nbd is %d, f_fbd is %d\n", content->f_nbd, content->f_fbd);
        content->af_G_param.dist_index_reverse_err_max = content->af_G_param.saf_dist_index_reverse_err_max;
        if(content->af_G_param.zoom_enhance_enable != 0)
        {
            if(content->af_G_param.dist_idx_init > content->f_nbd)
                content->af_G_param.dist_idx_init = content->f_nbd;
            else if(content->af_G_param.dist_idx_init < content->f_fbd)
                content->af_G_param.dist_idx_init = content->f_fbd;
            af_printf("zoom_enhance_enable %d, dist_idx_init is %d, zoom_dir %d, zoom_break_idx %d\n", 
            content->af_G_param.zoom_enhance_enable, content->af_G_param.dist_idx_init, 
            content->af_G_param.zoom_dir, content->af_G_param.zoom_break_idx);
        }
        else
            af_printf("zoom_enhance_enable %d", content->af_G_param.zoom_enhance_enable);
        
    }
    content->af_ctrl_param.zoom_status = 0;
   
    switch (p_af_control->workingMode) {
        case CAF:
            content->af_control_param.af_stm = INIT_CAF;
            content->af_control_param.af_next_stm = INIT_CAF;
            content->af_control_param.direction = 1;
            break;
        case SAF:
            content->af_control_param.af_stm = INIT_SAF;
            content->af_control_param.af_next_stm = INIT_SAF;
            content->af_control_param.direction = 1;
            break;
        case MANUAL:
            content->af_control_param.af_stm = 0;
            content->af_control_param.af_next_stm = 0;
            content->af_control_param.direction = 1;
            content->af_ctrl_param.manual_mode = MANUALIDLE;
            break;
        case CALIB:
            content->af_control_param.af_stm = INIT_CALIB;
            content->af_control_param.af_next_stm = INIT_CALIB;
            content->af_control_param.direction = 1;
            break;
        case DEBUGS:
            content->af_control_param.af_stm = INIT_CAF;
            content->af_control_param.af_next_stm = INIT_CAF;
            content->af_control_param.direction = 1;
            break;
        case CALIB_SAF:
            content->af_control_param.af_stm = INIT_CALIB_SAF;
            content->af_control_param.af_next_stm = INIT_CALIB_SAF;
            content->af_control_param.direction = 1;
            break;
        case CALIB_FOCUS_LENGTH:
            break;
        case CALIB_ZOOM_LENGTH:
            break;
        default:
            return -1;
    }
    content->lens_ctrl.af_stat_config_update = 1;
    content->lens_ctrl.af_stat_config = &(content->af_statistic_config);
    return 0;
}

int af_control(void *data, af_control_t* p_af_control, u16 af_tile_count,
    ISP_CfaAfStatT* p_af_info, u16 awb_tile_count, awb_data_t* p_awb_info,
    lens_control_t * p_lens_ctrl, ae_info_t* p_ae_info, u8 lens_runing_flag,
    ISP_CfaAfStatT* p_af_info_rgb)

{
    Isp3AProcessor *processor;
    //ImageOperate *op;
    Isp3AContentAF *content;

    processor = (Isp3AProcessor *)data;
    //op = &(processor->operate);
    content = &(processor->content.AFData);

    content->gain_shift = p_ae_info->gain_index;
    content->gain_shift = ((content->gain_shift / 128) + 1);
    //printf("enter af_control,working mode is %d", content->af_ctrl_param.workingMode);
    af_mode_switch(processor, &(content->af_ctrl_param));

    if (content->af_ctrl_param.workingMode == MANUAL)
    {
        af_manual_control(processor, &(content->af_ctrl_param), &(content->af_drg_params), &(content->lens_ctrl),
                lens_runing_flag);
        return 0;
    }
    feed_dragon(processor, &(content->af_ctrl_param), af_tile_count, p_af_info, awb_tile_count,
        p_awb_info, &(content->af_drg_params));
    param_adj(processor, &(content->af_control_param), &(content->af_drg_params), &(content->af_G_param));
    switch (content->af_ctrl_param.workingMode) {
        case CAF:
            af_CAF_control(processor, &(content->af_ctrl_param), &(content->af_drg_params), &(content->lens_ctrl),
                lens_runing_flag);
            break;
        case SAF:
            af_SAF_control(processor, &(content->af_ctrl_param), &(content->af_drg_params), &(content->lens_ctrl),
                lens_runing_flag);
            break;
        case MANUAL:
            af_manual_control(processor, &(content->af_ctrl_param), &(content->af_drg_params), &(content->lens_ctrl),
                lens_runing_flag);
            break;
        case CALIB:
            af_calib_control(processor, &(content->af_ctrl_param), &(content->af_drg_params), &(content->lens_ctrl),
                lens_runing_flag);
            break;
        case DEBUGS:
            af_debug_control(processor, &(content->af_ctrl_param), &(content->af_drg_params), &(content->lens_ctrl),
                lens_runing_flag);
            break;
        case CALIB_SAF:
            af_calib_of_saf_control(processor, &(content->af_ctrl_param), &(content->af_drg_params), &(content->lens_ctrl),
                lens_runing_flag);
            break;
        case CALIB_ZOOM_LENGTH:
            af_calib_of_zoom_length_control(processor, &(content->af_ctrl_param), &(content->af_drg_params), &(content->lens_ctrl),
                lens_runing_flag);
            break;
        case CALIB_FOCUS_LENGTH:
            af_calib_of_focus_length_control(processor, &(content->af_ctrl_param), &(content->af_drg_params), &(content->lens_ctrl),
                lens_runing_flag);
            break;

        default:
            return -1;
    }
    content->gain_shift_pre = content->gain_shift;
    return 0;
}

void af_set_dis_range(void *data, af_range_t * p_af_range)
{
    Isp3AProcessor *processor;
    //ImageOperate *op;
    Isp3AContentAF *content;

    processor = (Isp3AProcessor *)data;
    //op = &(processor->operate);
    content = &(processor->content.AFData);

    content->f_nbd = p_af_range->near_bd;
    content->f_fbd = p_af_range->far_bd - 50;
}

void af_set_cali_param(void *data, void *p_calib_param)
{
    Isp3AProcessor *processor;
    ImageOperate *op;
    Isp3AContentAF *content;

    processor = (Isp3AProcessor *)data;
    op = &(processor->operate);
    content = &(processor->content.AFData);

    op->memcpy(&(content->cali_out), p_calib_param, sizeof(content->cali_out));
}

//*****************************************************************************
//*****************************************************************************
//** Local Functions
//*****************************************************************************
//*****************************************************************************
/*
* impliment for 3x3 fir, output the larger of horizental value and vertical value
* support smaller than 32x32 array.
*/
static int pass_2D_fir(u8 col_num, u8 row_num, u32 * array,
    const af_fir_t * fir_coeff)
{
    u32 array_src[384];
    s32 kernel[9];

//    s32 TH = 16;
//    u32 high_TH = 0;
    int i, j;
    s32 result;

    /* copy orginal array to be processed */
    for (i = 0; i < col_num; i++) {
        for (j = 0; j < row_num; j++) {
            *(array_src + i * col_num + j) = *(array + i * col_num + j);
            *(array + i * col_num + j) = 0;
        }
    }
    /*process fir */
    for (i = 1; i < col_num - 1; i++) {
        for (j = 1; j < row_num - 1; j++) {
            kernel[0] = *(array_src + (i - 1) * col_num + j - 1);
            kernel[1] = *(array_src + (i - 1) * col_num + j);
            kernel[2] = *(array_src + (i - 1) * col_num + j + 1);

            kernel[3] = *(array_src + i * col_num + j - 1);
            kernel[4] = *(array_src + i * col_num + j);
            kernel[5] = *(array_src + i * col_num + j + 1);

            kernel[6] = *(array_src + (i + 1) * col_num + j - 1);
            kernel[7] = *(array_src + (i + 1) * col_num + j);
            kernel[8] = *(array_src + (i + 1) * col_num + j + 1);

            result = kernel[0] * fir_coeff->coff[0] +
                kernel[1] * fir_coeff->coff[1] +
                kernel[2] * fir_coeff->coff[2] +
                kernel[3] * fir_coeff->coff[3] +
                kernel[4] * fir_coeff->coff[4] +
                kernel[5] * fir_coeff->coff[5] +
                kernel[6] * fir_coeff->coff[6] +
                kernel[7] * fir_coeff->coff[7] +
                kernel[8] * fir_coeff->coff[8];
            result = ABS(result);
/*
            result = (result*128)/(kernel[4]+1);
            if (((result*128)/(kernel[2]+1)) < TH){
                result = 0;
            }

            if((high_TH < kernel[0])||(high_TH < kernel[1])||(high_TH < kernel[2])||(high_TH < kernel[3])||(high_TH < kernel[4])){
                result = 0;
            }
*/
            /*record the max filter result */
            *(array + i * col_num + j) = result;

        }
    }
    return 0;
}

static void feed_dragon(Isp3AProcessor *processor, af_control_t * p_af_control, u16 fv_tile_count,
    ISP_CfaAfStatT * p_af_info, u16 awb_tile_count, awb_data_t * p_awb_info,
    af_dragon_t * p_dragon)
{
    int i;
    u32 fv_temp = 0;
    u32 weight_count = 0;
    u8 zoom_idx = 0;
    u32 af_tile_weight[40];

    static awb_weight_window_t af_luma_weight_window =
        { {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                1,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                1, 1, 1,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                1, 1, 1,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                1, 1, 1,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                1, 1, 1,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                1, 1, 1,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                1, 1, 1,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                1, 1, 1,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                1, 1, 1,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                1, 1, 1,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                1, 1, 1,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                1, 1, 1,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                1, 1, 1,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                1, 1, 1,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                1, 1, 1}
    };

    s16 temp1;
    s32 j;
    u32 fv2_eng_avg = 0;
    u32 fv1_avg = 0;
    u32 fv2_avg = 0;
    u32 lum_dif = 0;
    u32 luma = 0;
    u32 fv1_dif = 0;
    u32 fv2_dif = 0;

    u32 fv1_total = 0;
    u32 fv2_total = 0;
    u32 fv3_total = 0;
    u32 lum_total = 0;

    ImageOperate *op;
    Isp3AContentAF *content;

    op = &(processor->operate);
    content = &(processor->content.AFData);

    content->af_sta_obj_GK.fv1_shift = FV1_SHIFT;
    content->af_sta_obj_GK.fv2_shift = FV2_SHIFT;
    content->af_sta_obj_GK.fv3_shift = FV1_SHIFT;

    /* calculate focus and luma value */
    content->af_sta_obj_GK.fv1_data = 0;
    content->af_sta_obj_GK.fv2_data = 0;
    content->af_sta_obj_GK.fv3_eng = 0;
    content->af_sta_obj_GK.fv1_tile_diff = 0;
    content->af_sta_obj_GK.fv2_tile_diff = 0;
    content->af_sta_obj_GK.fv3_tile_diff = 0;
    content->af_sta_obj_GK.lum_tile_diff = 0;

    /*feed luma */
    fv_temp = 0;
    for (i = 0; i < fv_tile_count; i++) {
        lum_total += (p_af_info[i].sum_fy) >> 4;
        content->af_sta_obj_GK.lum_tile_diff +=
            ABS((signed) p_af_info[i].sum_fy - (signed) content->lum_tile_cum[i]);
        content->lum_tile_cum[i] = p_af_info[i].sum_fy;
    }
    content->af_sta_obj_GK.lum_avg = (lum_total / fv_tile_count);
    content->af_sta_obj_GK.lum_tile_diff = (content->af_sta_obj_GK.lum_tile_diff / fv_tile_count);

    /*feed fv1 */
    fv_temp = 0;
    weight_count = 0;
    op->memcpy(af_tile_weight, p_af_control->af_window.tiles,
        sizeof(af_tile_weight));

    for (i = 0; i < fv_tile_count; i++) {
        fv_temp = (p_af_info[i].sum_fv1) * af_tile_weight[i] / (content->gain_shift + 1);
        weight_count += af_tile_weight[i];
        fv1_total += fv_temp;
        content->af_sta_obj_GK.fv1_data += (fv_temp * fv_temp) >> 3;
        content->af_sta_obj_GK.fv1_tile_diff +=
            ABS((signed) p_af_info[i].sum_fv1 - (signed) content->fv1_tile_cum[i]);
        content->fv1_tile_cum[i] = p_af_info[i].sum_fv1;
    }

    content->af_sta_obj_GK.fv1_avg = (fv1_total / weight_count);
    content->af_sta_obj_GK.fv1_data =
        (content->af_sta_obj_GK.fv1_data >> content->af_sta_obj_GK.fv1_shift) / weight_count;
    content->af_sta_obj_GK.fv1_tile_diff /= weight_count;

    /*feed fv2 */
    fv_temp = 0;
    weight_count = 0;

    for (i = 0; i < fv_tile_count; i++) {
        fv_temp = (p_af_info[i].sum_fv2) * af_tile_weight[i] / (content->gain_shift + 1);
        weight_count += af_tile_weight[i];
        fv2_total += fv_temp;
        content->af_sta_obj_GK.fv2_data += ((u64)fv_temp * (u64)fv_temp) >> 3;
        content->af_sta_obj_GK.fv2_tile_diff +=
            ABS((signed) p_af_info[i].sum_fv2 - (signed) content->fv2_tile_cum[i]);
        content->fv2_tile_cum[i] = p_af_info[i].sum_fv2;
    }

    content->af_sta_obj_GK.fv2_avg = (fv2_total / weight_count);
    content->af_sta_obj_GK.fv2_data = (content->af_sta_obj_GK.fv2_data >> content->af_sta_obj_GK.fv2_shift);
    //content->af_sta_obj_GK.fv2_data = (fv2_total / weight_count);
    content->af_sta_obj_GK.fv2_tile_diff /= weight_count;

    /*feed fv3 */

    fv_temp = 0;
    weight_count = 0;
    zoom_idx = content->af_result.zoom_idx_dest;
    if (content->af_G_param.zoom_param[zoom_idx].fvg_enable) {
        for (i = 0; i < awb_tile_count; i++) {
            content->af_luma_fir.tiles[i] = p_awb_info[i].g_avg;
        }
        pass_2D_fir(16, 24, (u32 *) (&(content->af_luma_fir.tiles[0])),
            &(content->af_G_param.spacial_fir));
        for (i = 0; i < awb_tile_count; i++) {
            fv_temp = (content->af_luma_fir.tiles[i]) * af_luma_weight_window.tiles[i];
            weight_count += af_luma_weight_window.tiles[i];
            fv3_total += fv_temp;
            content->af_sta_obj_GK.fv3_eng += (fv_temp * fv_temp) >> 3;
            content->af_sta_obj_GK.fv3_tile_diff +=
                ABS((signed) (content->af_luma_fir.tiles[i]) - (signed) content->fvg_tile_cum[i]);
            content->fvg_tile_cum[i] = content->af_luma_fir.tiles[i];
        }
        content->af_sta_obj_GK.fvg_data_avg = (fv3_total / weight_count);
        content->af_sta_obj_GK.fv3_eng = (content->af_sta_obj_GK.fv3_eng >> content->af_sta_obj_GK.fv3_shift);
        content->af_sta_obj_GK.fv3_tile_diff /= weight_count;
    }

    i = p_dragon->f_step;       // >_<
    if (i == 0)
        i = 1;
    p_dragon->store_cnt++;
    p_dragon->fv_store_idx++;
    if (p_dragon->fv_store_idx >= p_dragon->store_len)
        p_dragon->fv_store_idx = 0;
    p_dragon->fv_store[p_dragon->fv_store_idx].occ_flag = 1;
    p_dragon->dist_idx[p_dragon->fv_store_idx] = content->af_result.dist_idx_src;
    p_dragon->fv_store[p_dragon->fv_store_idx].fv1_avg = content->af_sta_obj_GK.fv1_avg;
    p_dragon->fv_store[p_dragon->fv_store_idx].fv2_avg = content->af_sta_obj_GK.fv2_avg;
    p_dragon->fv_store[p_dragon->fv_store_idx].fvg_data_avg = content->af_sta_obj_GK.fvg_data_avg;
    p_dragon->fv_store[p_dragon->fv_store_idx].fv1_data = content->af_sta_obj_GK.fv1_data;
    p_dragon->fv_store[p_dragon->fv_store_idx].fv2_data = content->af_sta_obj_GK.fv2_data;
    p_dragon->fv_store[p_dragon->fv_store_idx].fv3_eng = content->af_sta_obj_GK.fv3_eng;
    p_dragon->fv_store[p_dragon->fv_store_idx].luma_avg = content->af_sta_obj_GK.lum_avg;
    p_dragon->fv_store[p_dragon->fv_store_idx].luma_tile_diff =
        content->af_sta_obj_GK.lum_tile_diff / i;
    p_dragon->fv_store[p_dragon->fv_store_idx].fv1_tile_diff =
        content->af_sta_obj_GK.fv1_tile_diff / i;
    p_dragon->fv_store[p_dragon->fv_store_idx].fv2_tile_diff =
        content->af_sta_obj_GK.fv2_tile_diff / i;
    p_dragon->fv_store[p_dragon->fv_store_idx].fv3_tile_diff =
        content->af_sta_obj_GK.fv3_tile_diff / i;

    if (content->af_sta_obj_GK.fv1_data > content->fv1_data_max) {
        content->fv1_data_max = content->af_sta_obj_GK.fv1_data;
        temp1 = p_dragon->fv_store_idx - p_dragon->store_delay;
        if (temp1 < 0)
            temp1 += p_dragon->store_len;
        if (p_dragon->fv_store[temp1].occ_flag == 1) {
            content->af_fv1_top = p_dragon->dist_idx[temp1];
        } else {
            content->af_fv1_top = content->af_result.dist_idx_src;
        }
    }
    if (content->af_sta_obj_GK.fv1_data < content->fv1_data_min) {
        content->fv1_data_min = content->af_sta_obj_GK.fv1_data;
        temp1 = p_dragon->fv_store_idx - p_dragon->store_delay;
        if (temp1 < 0)
            temp1 += p_dragon->store_len;
        if (p_dragon->fv_store[temp1].occ_flag == 1) {
            content->af_fv1_bot = p_dragon->dist_idx[temp1];
        } else {
            content->af_fv1_bot = p_dragon->dist_idx[p_dragon->fv_store_idx];
        }
    }
    
    if (content->af_sta_obj_GK.fv2_data > content->fv2_data_max) {
        content->fv2_data_max = content->af_sta_obj_GK.fv2_data;
        temp1 = p_dragon->fv_store_idx - p_dragon->store_delay;
        if (temp1 < 0)
            temp1 += p_dragon->store_len;
        if (p_dragon->fv_store[temp1].occ_flag == 1) {
            content->af_fv2_top = p_dragon->dist_idx[temp1];
        } else {
            content->af_fv2_top = content->af_result.dist_idx_src;
        }
    }

    if (content->af_sta_obj_GK.fv2_data > content->fv2_data_max_direct) {
        content->fv2_data_max_direct = content->af_sta_obj_GK.fv2_data;      
        content->af_fv2_top_direct = content->af_result.dist_idx_src;
    }
    if (content->af_sta_obj_GK.fv2_data < content->fv2_data_min) {
        content->fv2_data_min = content->af_sta_obj_GK.fv2_data;
        temp1 = p_dragon->fv_store_idx - p_dragon->store_delay;
        if (temp1 < 0)
            temp1 += p_dragon->store_len;
        if (p_dragon->fv_store[temp1].occ_flag == 1) {
            content->af_fv2_bot = p_dragon->dist_idx[temp1];
        } else {
            content->af_fv2_bot = p_dragon->dist_idx[p_dragon->fv_store_idx];
        }
    }

    if (content->af_sta_obj_GK.fvg_data_avg > content->fvg_data_avg_max) {
        content->fvg_data_avg_max = content->af_sta_obj_GK.fvg_data_avg;
        temp1 = p_dragon->fv_store_idx - p_dragon->store_delay;
        if (temp1 < 0)
            temp1 += p_dragon->store_len;
        if (p_dragon->fv_store[temp1].occ_flag == 1) {
            content->af_fvg_top = p_dragon->dist_idx[temp1];
        } else {
            content->af_fvg_top = content->af_result.dist_idx_src;
        }
    }
    if (content->af_sta_obj_GK.fvg_data_avg < content->fvg_data_avg_min) {
        content->fvg_data_avg_min = content->af_sta_obj_GK.fvg_data_avg;
        temp1 = p_dragon->fv_store_idx - p_dragon->store_delay;
        if (temp1 < 0)
            temp1 += p_dragon->store_len;
        if (p_dragon->fv_store[temp1].occ_flag == 1) {
            content->af_fvg_bot = p_dragon->dist_idx[temp1];
        } else {
            content->af_fvg_bot = p_dragon->dist_idx[p_dragon->fv_store_idx];
        }
    }

    if (content->af_sta_obj_GK.fvg_data_avg < content->pre_fvg_data_avg) {
        if (content->fvg_down_count >= 0)
            content->fvg_down_count++;
    } else {
        if (content->fvg_down_count > 0)
            content->fvg_down_count = 0;
    }
    content->pre_fvg_data_avg = content->af_sta_obj_GK.fvg_data_avg;
//    printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>fvg_down_count is %d\n",fvg_down_count);

    for (i = 0; i < p_dragon->store_len; i++) {
        if (p_dragon->fv_store[i].occ_flag == 1) {
            if (p_dragon->fv_store[i].fv1_data > content->fv1_data_max) {
                content->fv1_data_max = p_dragon->fv_store[i].fv1_data;
                temp1 = i - p_dragon->store_delay;
                if (temp1 < 0)
                    temp1 += p_dragon->store_len;
                if (p_dragon->fv_store[temp1].occ_flag == 1) {
                    content->af_fv1_top = p_dragon->dist_idx[temp1];
                } else {
                    content->af_fv1_top = p_dragon->dist_idx[i];
                }
            } else if (p_dragon->fv_store[i].fv1_data < content->fv1_data_min) {
                content->fv1_data_min = p_dragon->fv_store[i].fv1_data;
                temp1 = i - p_dragon->store_delay;
                if (temp1 < 0)
                    temp1 += p_dragon->store_len;
                if (p_dragon->fv_store[temp1].occ_flag == 1) {
                    content->af_fv1_bot = p_dragon->dist_idx[temp1];
                } else {
                    content->af_fv1_bot = p_dragon->dist_idx[i];
                }
            }
            if (p_dragon->fv_store[i].fv2_data > content->fv2_data_max) {
                content->fv2_data_max = p_dragon->fv_store[i].fv2_data;
                temp1 = i - p_dragon->store_delay;
                if (temp1 < 0)
                    temp1 += p_dragon->store_len;
                if (p_dragon->fv_store[temp1].occ_flag == 1) {
                    content->af_fv2_top = p_dragon->dist_idx[temp1];
                } else {
                    content->af_fv2_top = p_dragon->dist_idx[i];
                }
            } else if (p_dragon->fv_store[i].fv2_data < content->fv2_data_min) {
                content->fv2_data_min = p_dragon->fv_store[i].fv2_data;
                temp1 = i - p_dragon->store_delay;
                if (temp1 < 0)
                    temp1 += p_dragon->store_len;
                if (p_dragon->fv_store[temp1].occ_flag == 1) {
                    content->af_fv2_bot = p_dragon->dist_idx[temp1];
                } else {
                    content->af_fv2_bot = p_dragon->dist_idx[i];
                }
            }
            if (p_dragon->fv_store[i].fvg_data_avg > content->fvg_data_avg_max) {
                content->fvg_data_avg_max = p_dragon->fv_store[i].fvg_data_avg;
                temp1 = i - p_dragon->store_delay;
                if (temp1 < 0)
                    temp1 += p_dragon->store_len;
                if (p_dragon->fv_store[temp1].occ_flag == 1) {
                    content->af_fvg_top = p_dragon->dist_idx[temp1];
                } else {
                    content->af_fvg_top = p_dragon->dist_idx[i];
                }
            } else if (p_dragon->fv_store[i].fvg_data_avg < content->fvg_data_avg_min) {
                content->fvg_data_avg_min = p_dragon->fv_store[i].fvg_data_avg;
                temp1 = i - p_dragon->store_delay;
                if (temp1 < 0)
                    temp1 += p_dragon->store_len;
                if (p_dragon->fv_store[temp1].occ_flag == 1) {
                    content->af_fvg_bot = p_dragon->dist_idx[temp1];
                } else {
                    content->af_fvg_bot = p_dragon->dist_idx[i];
                }
            }
        }
    }
    content->fv1_data = content->af_sta_obj_GK.fv1_data;
    content->fv2_data = content->af_sta_obj_GK.fv2_data;
	content->fv2_data_all[af_count] = content->fv2_data;
    content->fvg_data_avg = content->af_sta_obj_GK.fvg_data_avg;

    //printf("fv2_data_max is %d,fv2_data_min is %d\n",fv2_data_max,fv2_data_min);
    //printf("af_fv2_top is %d,af_fv2_bot is %d,fpos_max is %d, fpos_min is %d\n",af_fv2_top,af_fv2_bot,fpos_max,fpos_min);

    //fv_printf("fv1_avg is %d, fv1_data is %d, fv1_tile_diff is %d\n",content->af_sta_obj_GK.fv1_avg,content->af_sta_obj_GK.fv1_data,content->af_sta_obj_GK.fv1_tile_diff);
    //printf("fv2_avg is %d, fv2_data is %llu, fv2_tile_diff is %d\n",content->af_sta_obj_GK.fv2_avg,content->af_sta_obj_GK.fv2_data,content->af_sta_obj_GK.fv2_tile_diff);
   // fv_printf("fvg_data_avg is %d, fv3_eng is %d, fv3_tile_diff is %d\n",
   //     content->af_sta_obj_GK.fvg_data_avg, content->af_sta_obj_GK.fv3_eng, content->af_sta_obj_GK.fv3_tile_diff);

    j = ((p_dragon->store_cnt >=
            p_dragon->store_len) ? p_dragon->store_len : p_dragon->store_cnt);
    for (i = 0; i < j; i++) {
        if (p_dragon->fv_store[i].occ_flag == 1) {
            fv2_eng_avg += p_dragon->fv_store[i].fv2_data;
            lum_dif += p_dragon->fv_store[i].luma_tile_diff;
            luma += p_dragon->fv_store[i].luma_avg;
            fv1_avg += p_dragon->fv_store[i].fv1_avg;
            fv1_dif += p_dragon->fv_store[i].fv1_tile_diff;
            fv2_avg += p_dragon->fv_store[i].fv2_avg;
            fv2_dif += p_dragon->fv_store[i].fv2_tile_diff;
        }
    }
    fv2_eng_avg /= j;
    content->video_lum_diff = lum_dif * 100 / (luma + 1);
//    video_lum_diff = stable:10, move:50
    //scence_printf("video_lum_diff is %d\n", content->video_lum_diff);
    content->video_fv2_eng =
        ABS((s32) content->af_sta_obj_GK.fv2_data -
        (s32) fv2_eng_avg) * 100 / (MIN(fv2_eng_avg, content->af_sta_obj_GK.fv2_data) + 1);
//    video_fv2_eng = stable:<10, move:>20
    //scence_printf("video_fv2_eng is %d\n", content->video_fv2_eng);
    content->video_fv1_diff = fv1_dif * 100 / (fv1_avg + 1);
//    video_fv1_diff = stable:<10, move:>20
   // scence_printf("video_fv1_diff is %d,fv1_dif is %d,fv1a is %d\n", content->video_fv1_diff,
     //   fv1_dif, fv1_avg);
    p_dragon->sec_chg_val = ((content->video_lum_diff + content->video_fv2_eng + content->video_fv1_diff) >> 1);
//    sec_chg_val = stable:<20, move:>50
    //scence_printf("sec_chg_val is %d\n", p_dragon->sec_chg_val);
}

static s32 sgk_af_fpt_est(af_dragon_t * p_dragon)
{
    s32 t1;
    //s32 t2;
    s32 diff_bk = 0;
    s32 diff_fw = 0;
    u32 fv2e_max_idx = 0;       //wil  be removed!!!!! 2010-4-9

    t1 = fv2e_max_idx - 1;

    if (t1 < 0) {
        t1 += p_dragon->store_len;
    }

    if (p_dragon->fv_store[t1].occ_flag == 1) {
        diff_bk = p_dragon->fv_store[t1].fv2_data;
    } else {
        diff_bk = p_dragon->fv_store[fv2e_max_idx].fv2_data;
    }
    t1 = fv2e_max_idx + 1;
    if (t1 >= p_dragon->store_len)
        t1 = 0;
    if (p_dragon->fv_store[t1].occ_flag == 1) {
        diff_fw = p_dragon->fv_store[t1].fv2_data;
    } else {
//        diff_fw = p_dragon->fv_store[t1].fv2e_a1_max_idx;
    }
    diff_bk = ABS(diff_bk - (s32) p_dragon->fv_store[fv2e_max_idx].fv2_data);
    diff_fw = ABS(diff_fw - (s32) p_dragon->fv_store[fv2e_max_idx].fv2_data);
    t1 = ((p_dragon->f_step * (diff_bk - diff_fw) * 10) / (1 + (diff_bk +
                diff_fw) * 2));
    if (t1 >= 0)
        t1 = (t1 + 5);
    else
        t1 = (t1 - 5);
    t1 /= 10;

    #if 0
    t2 = (p_dragon->f_step * (diff_bk - diff_fw)) / (1 + (diff_bk +
            diff_fw) * 2);

    printf("BK: %d, FW: %d, F_sp: %d(%d)\n", diff_bk, diff_fw, t1, t2);
    #endif

    return t1;
}

/*
*    Single AF
*/
#if 0
//staic reverse error process(af drv modify) and delay store_delay modify 1 or 2 or 3
//af_param_t  Csf 160 Fsf 40 Cpk_th 50(wide)-60(narrow) Fpk_th40(wide)-50(narrow)
static int init_saf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result)
{
    //ImageOperate *op;
    Isp3AContentAF *content;

    //op = &(processor->operate);
    content = &(processor->content.AFData);

    content->f_nbd_sign = 0;
    content->f_fbd_sign = 0;
    content->af_peak_sign = 0;
    p_dragon->skip_frame = 0;
    p_dragon->f_step = 6;
    p_dragon->store_len = 64;
    p_dragon->fm1 = p_af_result->dist_idx_src;
    p_dragon->fm2 = p_af_result->dist_idx_src;
    clean_dragon(processor, p_dragon);
    p_af_result->dist_idx_dest =
        p_af_result->dist_idx_src + p_dragon->f_step * content->af_control_param.direction;
    if (p_af_result->dist_idx_dest >= content->f_nbd) {
        content->af_control_param.direction *= -1;
        p_af_result->dist_idx_dest = content->f_nbd;
    } else if (p_af_result->dist_idx_dest <= content->f_fbd) {
        content->af_control_param.direction *= -1;
        p_af_result->dist_idx_dest = content->f_fbd;
    }
    content->af_control_param.af_next_stm = CoarseSAF;
    stm_printf(">>>>>>>>>>>>>>>>init_saf\n");
    stm_printf("init_saf-dist_idx_dest is %d, dist_idx_src is %d\n", p_af_result->dist_idx_dest, p_af_result->dist_idx_src);

    return 0;
}

static int coarse_saf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result)
{
    u64 fv_dir_chg;
    u8 zoom_idx;
    //ImageOperate *op;
    Isp3AContentAF *content;
    stm_printf("enter coarse_saf,af_next_stm is %d\n");

    //op = &(processor->operate);
    content = &(processor->content.AFData);

    zoom_idx = p_af_control->zoom_idx;
    if (content->gain_shift_pre != content->gain_shift) {
        clean_dragon(processor, p_dragon);
    } else {
        p_dragon->fv_rev_fac = content->af_G_param.zoom_param[zoom_idx].Cpk_th * 10;
       

        p_dragon->fv_rev_TH = content->fv2_data_max * (p_dragon->fv_rev_fac / 1000.0f);
        p_dragon->f_step = content->af_G_param.zoom_param[zoom_idx].Csf;
        stm_printf("coarse_saf-fv2_data_max is %d, fv2_data is %d\n", content->fv2_data_max, content->fv2_data);
        fv_dir_chg = content->fv2_data_max - content->fv2_data;
        stm_printf("fv_dir_chg is %d,fv_rev_TH is %d\n", fv_dir_chg, p_dragon->fv_rev_TH);
        stm_printf("zoom_idx is %d,Cpk_th is %d\n", zoom_idx, content->af_G_param.zoom_param[zoom_idx].Cpk_th);

        if (fv_dir_chg > p_dragon->fv_rev_TH) {
            content->af_control_param.direction *= -1;
            p_dragon->skip_frame++;
            p_dragon->fm2 = p_dragon->fm1;
            p_dragon->fm1 = p_af_result->dist_idx_src;
            stm_printf("mark: %d, %d (Coarse-SAF)\n", p_dragon->fm1,
                p_dragon->fm2);
            if (p_dragon->skip_frame >= 2) {
                content->af_peak_sign = 1;
                content->af_control_param.af_next_stm = INIT_FineSAF;
                p_af_result->dist_idx_dest = 
                  (content->af_fv2_top -
                   ((5 * p_dragon->f_step * content->af_control_param.direction)));

                  /*  (content->af_fv2_top -
                    ((3 * p_dragon->f_step * content->af_control_param.direction) / 4));*/
                /*p_af_result->dist_idx_dest =
                    (content->af_fv2_top -
                    sgk_af_fpt_est(p_dragon) * content->af_control_param.direction -
                    ((3 * p_dragon->f_step * content->af_control_param.direction) / 4));*/

            } else {
                p_af_result->dist_idx_dest =
                    content->af_fv2_top  
                        - 4 * p_dragon->f_step * content->af_control_param.direction;
                    /*((p_dragon->f_step * content->af_control_param.direction) / 2);*/
                stm_printf("dist_idx_dest is %d\n", p_af_result->dist_idx_dest);
                content->wait_for_vd = 1;
                p_dragon->store_af_stm = CoarseSAF;
                content->af_control_param.af_next_stm = WAIT_VD_SAF;
                stm_printf("af_reverse (FSP-SAF)\n");
            }
        } else {
            p_af_result->dist_idx_dest =
                p_af_result->dist_idx_src +
                (p_dragon->f_step * content->af_control_param.direction);
            if (p_af_result->dist_idx_dest >= content->f_nbd) {
                p_dragon->fm2 = p_dragon->fm1;
                p_dragon->fm1 = content->f_nbd;
                p_dragon->skip_frame++;
                content->f_nbd_sign++;
                content->af_control_param.direction *= -1;
                p_af_result->dist_idx_dest = content->f_nbd;
                if (p_dragon->skip_frame >= 2) {
                    content->af_control_param.af_next_stm = INIT_FineSAF;
                } else {
                    content->wait_for_vd = 1;
                    p_dragon->store_af_stm = CoarseSAF;
                    content->af_control_param.af_next_stm = WAIT_VD_SAF;
                }
                stm_printf("NEAR LIM (FSP-SAF)\n");
            } else if (p_af_result->dist_idx_dest <= content->f_fbd) {
                p_dragon->fm2 = p_dragon->fm1;
                p_dragon->fm1 = content->f_fbd;
                p_dragon->skip_frame++;
                content->f_fbd_sign++;
                content->af_control_param.direction *= -1;
                p_af_result->dist_idx_dest = content->f_fbd;
                if (p_dragon->skip_frame >= 2) {
                    content->af_control_param.af_next_stm = INIT_FineSAF;
                } else {
                    content->wait_for_vd = 1;
                    p_dragon->store_af_stm = CoarseSAF;
                    content->af_control_param.af_next_stm = WAIT_VD_SAF;
                }
                stm_printf("FAR LIM (FSP-SAF)\n");
            }
            if ((content->f_nbd_sign > 1) && (content->f_fbd_sign > 1)) {
                p_af_result->dist_idx_dest = content->af_fv2_top;
                content->af_control_param.af_next_stm = FAIL_SAF;
                //printf("in-focus point: %d, current point: %d(Fail SAF)\n",af_fv2_top, p_af_result->dist_idx_src);
            }
        }
    }
    stm_printf(">>>>>>>>>>>>>>>>coarse_saf\n");
    return 0;
}

static int init_fine_saf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_ctrl)
{
    s32 temp_pos;
    //ImageOperate *op;
    Isp3AContentAF *content;

    //op = &(processor->operate);
    content = &(processor->content.AFData);

    content->f_nbd_sign = 0;
    content->f_fbd_sign = 0;
    content->af_peak_sign = 0;
    content->af_dis = 0;
    p_dragon->skip_frame = 0;
    clean_dragon(processor, p_dragon);
    p_dragon->f_step = 2;
    p_dragon->search_len_TH =
        (ABS((s32) p_dragon->fm1 - (s32) p_dragon->fm2) * 3) >> 1;
    if (p_dragon->fm1 > p_dragon->fm2) {
        temp_pos = p_dragon->fm2;
        p_dragon->fm2 = p_dragon->fm1;
        p_dragon->fm1 = temp_pos;
    }
    content->af_control_param.af_next_stm = FineSAF;
    stm_printf(">>>>>>>>>>>>>>>>enter IFS mode(SAF), m1: %d, m2: %d\n",
        p_dragon->fm1, p_dragon->fm2);
    return -1;
}

static int fine_saf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result)
{
    u64 fv_dir_chg;
    u8 zoom_idx;

    //ImageOperate *op;
    Isp3AContentAF *content;

    //op = &(processor->operate);
    content = &(processor->content.AFData);
    zoom_idx = p_af_control->zoom_idx;
    if (content->gain_shift_pre != content->gain_shift) {
        clean_dragon(processor, p_dragon);
    } else {
        if ((p_af_result->dist_idx_src >= p_dragon->fm1)
            && (p_af_result->dist_idx_src <= p_dragon->fm2)) {
            p_dragon->fv_rev_fac = content->af_G_param.zoom_param[zoom_idx].Fpk_th * 10;
            p_dragon->fv_rev_TH = content->fv2_data_max * (p_dragon->fv_rev_fac / 1000.0f);
            p_dragon->f_step = content->af_G_param.zoom_param[zoom_idx].Fsf;
            fv_dir_chg = content->fv2_data_max - content->fv2_data;
            if (fv_dir_chg > p_dragon->fv_rev_TH) {
                content->af_control_param.direction *= -1;
                p_dragon->skip_frame++;
                if (p_dragon->skip_frame == 1) {
                    content->af_peak_sign = 1;
                    content->af_control_param.af_next_stm = DONE_SAF;
                    stm_printf("find peak (IFS-SAF)\n");
                } else {
                    content->wait_for_vd = 1;
                    p_dragon->store_af_stm = FineSAF;
                    content->af_control_param.af_next_stm = WAIT_VD_SAF;
                    stm_printf("af_reverse (IFS-SAF)\n");
                }
            } else {
                p_af_result->dist_idx_dest =
                    p_af_result->dist_idx_src +
                    (p_dragon->f_step * content->af_control_param.direction);
                if (p_af_result->dist_idx_dest >= content->f_nbd) {
                    content->af_peak_sign = 1;
                    content->af_control_param.af_next_stm = DONE_SAF;
                    stm_printf("NEAR LIM (IFS-SAF)\n");
                } else if (p_af_result->dist_idx_dest <= content->f_fbd) {
                    content->af_peak_sign = 1;
                    content->af_control_param.af_next_stm = DONE_SAF;
                    stm_printf("FAR LIM (IFS-SAF)\n");
                }
            }
            if (content->af_peak_sign == 1) {
                p_af_result->dist_idx_dest = content->af_fv2_top;
                //    content->af_fv2_top -
                //    sgk_af_fpt_est(p_dragon) * content->af_control_param.direction;
               // p_af_result->dist_idx_dest += content->af_control_param.direction * p_dragon->f_step * 2;
               // content->af_control_param.af_next_stm = DONE_SAF;
            }
        } else {
            p_af_result->dist_idx_dest = content->af_fv2_top;
            content->af_control_param.af_next_stm = FAIL_SAF;
        }
    }
    printf(">>>>>>>>>>>>>>>>fine_saf,dist_idx_dest is %d\n", p_af_result->dist_idx_dest);
    printf(">>>>>>>>>>>>>>>>fine_saf,af_fv2_top is %d\n", content->af_fv2_top);
   
    return 0;
}

static int done_saf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result)
{
    //ImageOperate *op;
    Isp3AContentAF *content;

    //op = &(processor->operate);
    content = &(processor->content.AFData);

    stm_printf(">>>>>>>>>>>>>>>>done_saf\n");
    content->af_control_param.af_next_stm = IDLE_SAF;

    return 0;
}

static int fail_saf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result)
{
    //stm_printf(">>>>>>>>>>>>>>>>fail_saf\n");
    return 0;
}

static int idle_saf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result)
{
    //u32 dist_idx;


    //dist_idx = p_af_result->dist_idx_dest;
    //dist = 500.0 / (float) dist_idx;
    //printf("^_^ locked at %fm\n",dist);
    return 0;
}

static int wait_vd_saf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result)
{
    //ImageOperate *op;
    Isp3AContentAF *content;

    //op = &(processor->operate);
    content = &(processor->content.AFData);

    if (content->wait_for_vd == 0) {
        content->af_control_param.af_next_stm = p_dragon->store_af_stm;
        clean_dragon(processor, p_dragon);
    } else {
        content->wait_for_vd--;
    }
    stm_printf(">>>>>>>>>>>>>>>>wait_vd_saf\n");
    return 0;
}
#else

static int init_saf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result)
{
    //ImageOperate *op;
    Isp3AContentAF *content;

    //op = &(processor->operate);
    content = &(processor->content.AFData);

    content->f_nbd_sign = 0;
    content->f_fbd_sign = 0;
    content->af_peak_sign = 0;
    p_dragon->skip_frame = 0;
    p_dragon->f_step = 6;
    p_dragon->store_len = 64;
    p_dragon->fm1 = p_af_result->dist_idx_src;
    p_dragon->fm2 = p_af_result->dist_idx_src;
    //clean_dragon(processor, p_dragon);
    p_af_result->dist_idx_dest =
        p_af_result->dist_idx_src + p_dragon->f_step * content->af_control_param.direction;
    if (p_af_result->dist_idx_dest >= content->f_nbd) {
        content->af_control_param.direction *= -1;
        p_af_result->dist_idx_dest = content->f_nbd;
    } else if (p_af_result->dist_idx_dest <= content->f_fbd) {
        content->af_control_param.direction *= -1;
        p_af_result->dist_idx_dest = content->f_fbd;
    }
  
    content->wait_for_vd = 4;
    p_dragon->store_af_stm = CoarseSAF;
    content->af_control_param.af_next_stm = WAIT_VD_SAF;
    stm_printf(">>>>>>>>>>>>>>>>init_saf\n");
    stm_printf("init_saf-dist_idx_dest is %d, dist_idx_src is %d\n", p_af_result->dist_idx_dest, p_af_result->dist_idx_src);

    return 0;
}

static int coarse_saf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result)
{
    u64 fv_dir_chg;
    u8 zoom_idx;
    //ImageOperate *op;
    Isp3AContentAF *content;
    stm_printf("enter coarse_saf,af_next_stm is %d\n");

    //op = &(processor->operate);
    content = &(processor->content.AFData);

    zoom_idx = p_af_control->zoom_idx;
    if (content->gain_shift_pre != content->gain_shift) {
        clean_dragon(processor, p_dragon);
    } else {
        p_dragon->fv_rev_fac = content->af_G_param.zoom_param[zoom_idx].Cpk_th * 10;
       

        p_dragon->fv_rev_TH = content->fv2_data_max * (p_dragon->fv_rev_fac / 1000.0f);
        p_dragon->f_step = content->af_G_param.zoom_param[zoom_idx].Csf;
        stm_printf("coarse_saf-fv2_data_max is %llu, fv2_data is %llu\n", content->fv2_data_max, content->fv2_data);
        fv_dir_chg = content->fv2_data_max - content->fv2_data;
        stm_printf("fv_dir_chg is %llu,fv_rev_TH is %llu\n", fv_dir_chg, p_dragon->fv_rev_TH);
        stm_printf("zoom_idx is %d,Cpk_th is %d\n", zoom_idx, content->af_G_param.zoom_param[zoom_idx].Cpk_th);

        if (fv_dir_chg > p_dragon->fv_rev_TH) {
            content->af_control_param.direction *= -1;
            p_dragon->skip_frame++;
            p_dragon->fm2 = p_dragon->fm1;
            p_dragon->fm1 = p_af_result->dist_idx_src;
            stm_printf("mark: %d, %d (Coarse-SAF)\n", p_dragon->fm1,
                p_dragon->fm2);
            if (p_dragon->skip_frame >= 3) {
                content->af_peak_sign = 1;
                content->af_control_param.af_next_stm = INIT_FineSAF;
                p_af_result->dist_idx_dest = 
                  (content->af_fv2_top -
                   (( content->af_G_param.saf_optical_pos_offset * p_dragon->f_step * content->af_control_param.direction)));
				if (p_af_result->dist_idx_dest >= content->f_nbd) 
                    p_af_result->dist_idx_dest =  content->f_nbd;
                else if (p_af_result->dist_idx_dest <= content->f_fbd) 
                    p_af_result->dist_idx_dest =  content->f_fbd;
                content->dist_index_reverse_err = content->af_fv2_top - content->af_fv2_top_pre;
				if(content->dist_index_reverse_err > content->af_G_param.dist_index_reverse_err_max)
					content->dist_index_reverse_err = content->af_G_param.dist_index_reverse_err_max;
				if(-(content->dist_index_reverse_err) > content->af_G_param.dist_index_reverse_err_max)
					content->dist_index_reverse_err = - content->af_G_param.dist_index_reverse_err_max;
				content->fv2_data_max_pre = content->fv2_data_max;
                  /*  (content->af_fv2_top -
                    ((3 * p_dragon->f_step * content->af_control_param.direction) / 4));*/
                /*p_af_result->dist_idx_dest =
                    (content->af_fv2_top -
                    sgk_af_fpt_est(p_dragon) * content->af_control_param.direction -
                    ((3 * p_dragon->f_step * content->af_control_param.direction) / 4));*/

            } else {
                //clean_dragon(processor, p_dragon);
                p_af_result->dist_idx_dest =
                    content->af_fv2_top  
                        - content->af_G_param.saf_optical_pos_offset * p_dragon->f_step * content->af_control_param.direction;
                    /*((p_dragon->f_step * content->af_control_param.direction) / 2);*/
                if (p_af_result->dist_idx_dest >= content->f_nbd) 
                    p_af_result->dist_idx_dest =  content->f_nbd;
                else if (p_af_result->dist_idx_dest <= content->f_fbd) 
                    p_af_result->dist_idx_dest =  content->f_fbd; 
                stm_printf("dist_idx_dest is %d\n", p_af_result->dist_idx_dest);
                content->wait_for_vd = 4;
                p_dragon->store_af_stm = CoarseSAF;
                content->af_control_param.af_next_stm = WAIT_VD_SAF;
                stm_printf("af_reverse (FSP-SAF)\n");
				if (p_dragon->skip_frame == 2) {
					content->fv2_data_max_pre = content->fv2_data_max;
					content->af_fv2_top_pre = content->af_fv2_top;
				}
            }
        } else {
            p_af_result->dist_idx_dest =
                p_af_result->dist_idx_src +
                (p_dragon->f_step * content->af_control_param.direction);
            if (p_af_result->dist_idx_dest >= content->f_nbd) {
                p_dragon->fm2 = p_dragon->fm1;
                p_dragon->fm1 = content->f_nbd;
                p_dragon->skip_frame++;
                content->f_nbd_sign++;
                content->af_control_param.direction *= -1;
                p_af_result->dist_idx_dest = content->f_nbd;
                if (p_dragon->skip_frame >= 3) {
                    content->af_control_param.af_next_stm = INIT_FineSAF;
					content->dist_index_reverse_err = content->af_fv2_top - content->af_fv2_top_pre;
					if(content->dist_index_reverse_err > content->af_G_param.dist_index_reverse_err_max)
						content->dist_index_reverse_err = content->af_G_param.dist_index_reverse_err_max;
					if(-(content->dist_index_reverse_err) > content->af_G_param.dist_index_reverse_err_max)
						content->dist_index_reverse_err = - content->af_G_param.dist_index_reverse_err_max;
					 content->fv2_data_max_pre = content->fv2_data_max;
                } else {
                    //clean_dragon(processor, p_dragon);
                    content->wait_for_vd = 4;
                    p_dragon->store_af_stm = CoarseSAF;
                    content->af_control_param.af_next_stm = WAIT_VD_SAF;
					if (p_dragon->skip_frame == 2) {
					    content->fv2_data_max_pre = content->fv2_data_max;
					    content->af_fv2_top_pre = content->af_fv2_top;
				    }
                }
                stm_printf("NEAR LIM (FSP-SAF)\n");
            } else if (p_af_result->dist_idx_dest <= content->f_fbd) {
                p_dragon->fm2 = p_dragon->fm1;
                p_dragon->fm1 = content->f_fbd;
                p_dragon->skip_frame++;
                content->f_fbd_sign++;
                content->af_control_param.direction *= -1;
                p_af_result->dist_idx_dest = content->f_fbd;
                if (p_dragon->skip_frame >= 3) {
                    content->af_control_param.af_next_stm = INIT_FineSAF;
					content->dist_index_reverse_err = content->af_fv2_top - content->af_fv2_top_pre;
					if(content->dist_index_reverse_err > content->af_G_param.dist_index_reverse_err_max)
						content->dist_index_reverse_err = content->af_G_param.dist_index_reverse_err_max;
					if(-(content->dist_index_reverse_err) > content->af_G_param.dist_index_reverse_err_max)
						content->dist_index_reverse_err = - content->af_G_param.dist_index_reverse_err_max;
					 content->fv2_data_max_pre = content->fv2_data_max;
                } else {
               		//clean_dragon(processor, p_dragon);
                    content->wait_for_vd = 4;
                    p_dragon->store_af_stm = CoarseSAF;
                    content->af_control_param.af_next_stm = WAIT_VD_SAF;
					if (p_dragon->skip_frame == 2) {
				        content->fv2_data_max_pre = content->fv2_data_max;
					    content->af_fv2_top_pre = content->af_fv2_top;
				    }
                }
                stm_printf("FAR LIM (FSP-SAF)\n");
            }
            if ((content->f_nbd_sign > 1) && (content->f_fbd_sign > 1)) {
                p_af_result->dist_idx_dest = content->af_fv2_top;
                content->af_control_param.af_next_stm = FAIL_SAF;
                //printf("in-focus point: %d, current point: %d(Fail SAF)\n",af_fv2_top, p_af_result->dist_idx_src);
            }
        }
    }
    stm_printf("dist_idx_dest is %d\n", p_af_result->dist_idx_dest);
    stm_printf(">>>>>>>>>>>>>>>>coarse_saf\n");
    return 0;
}

static int init_fine_saf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_ctrl)
{
    s32 temp_pos;
    //ImageOperate *op;
    Isp3AContentAF *content;

    //op = &(processor->operate);
    content = &(processor->content.AFData);

    content->f_nbd_sign = 0;
    content->f_fbd_sign = 0;
    content->af_peak_sign = 0;
    content->af_dis = 0;
    p_dragon->skip_frame = 0;
    //clean_dragon(processor, p_dragon);
    p_dragon->f_step = 2;
    p_dragon->search_len_TH =
        (ABS((s32) p_dragon->fm1 - (s32) p_dragon->fm2) * 3) >> 1;
    if (p_dragon->fm1 > p_dragon->fm2) {
        temp_pos = p_dragon->fm2;
        p_dragon->fm2 = p_dragon->fm1;
        p_dragon->fm1 = temp_pos;
    }
    content->wait_for_vd = 4;
    p_dragon->store_af_stm = FineSAF;
    content->af_control_param.af_next_stm = WAIT_VD_SAF;
    stm_printf(">>>>>>>>>>>>>>>>enter IFS mode(SAF), m1: %d, m2: %d\n",
        p_dragon->fm1, p_dragon->fm2);
    return -1;
}


static int fine_saf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result)
{
    u64 fv_dir_chg;
    u8 zoom_idx;

    //ImageOperate *op;
    Isp3AContentAF *content;

    //op = &(processor->operate);
    content = &(processor->content.AFData);
    zoom_idx = p_af_control->zoom_idx;
    if (content->gain_shift_pre != content->gain_shift) {
        clean_dragon(processor, p_dragon);
    } else {
        /*if ((p_af_result->dist_idx_src >= p_dragon->fm1)
            && (p_af_result->dist_idx_src <= p_dragon->fm2)) {*/
            p_dragon->fv_rev_fac = content->af_G_param.zoom_param[zoom_idx].Fpk_th * 10;
            p_dragon->fv_rev_TH = content->fv2_data_max * (p_dragon->fv_rev_fac / 1000.0f);
            p_dragon->f_step = content->af_G_param.zoom_param[zoom_idx].Fsf;
            fv_dir_chg = content->fv2_data_max - content->fv2_data;
            if (fv_dir_chg > p_dragon->fv_rev_TH) {
                content->af_control_param.direction *= -1;
                p_dragon->skip_frame++;
                if (p_dragon->skip_frame == 1) {
                    content->af_peak_sign = 1;
                    content->af_control_param.af_next_stm = DONE_SAF;
                    stm_printf("find peak (IFS-SAF)\n");
                } else {
                    content->wait_for_vd = 1;
                    p_dragon->store_af_stm = FineSAF;
                    content->af_control_param.af_next_stm = WAIT_VD_SAF;
                    stm_printf("af_reverse (IFS-SAF)\n");
                }
            } else {
                p_af_result->dist_idx_dest =
                    p_af_result->dist_idx_src +
                    (p_dragon->f_step * content->af_control_param.direction);
                if (p_af_result->dist_idx_dest >= content->f_nbd) {
                    content->af_peak_sign = 1;
                    content->af_control_param.af_next_stm = DONE_SAF;
                    stm_printf("NEAR LIM (IFS-SAF)\n");
                } else if (p_af_result->dist_idx_dest <= content->f_fbd) {
                    content->af_peak_sign = 1;
                    content->af_control_param.af_next_stm = DONE_SAF;
                    stm_printf("FAR LIM (IFS-SAF)\n");
                }
            }
            if (content->af_peak_sign == 1) {
                p_af_result->dist_idx_dest = content->af_fv2_top + content->dist_index_reverse_err;
				if (p_af_result->dist_idx_dest >= content->f_nbd) {
					p_af_result->dist_idx_dest = content->f_nbd;               
    			} else if (p_af_result->dist_idx_dest <= content->f_fbd) {
        			p_af_result->dist_idx_dest = content->f_fbd;     
    			}
                //    content->af_fv2_top -
                //    sgk_af_fpt_est(p_dragon) * content->af_control_param.direction;
               // p_af_result->dist_idx_dest += content->af_control_param.direction * p_dragon->f_step * 2;
                //content->af_control_param.af_next_stm = DONE_SAF;
            }
        /*
        } else {
            //p_af_result->dist_idx_dest = content->af_fv2_top;
			if ((p_af_result->dist_idx_dest - p_af_result->dist_idx_src > 0 &&  content->af_control_param.direction > 0)
				||(p_af_result->dist_idx_dest - p_af_result->dist_idx_src < 0 &&  content->af_control_param.direction < 0))
			{
				p_af_result->dist_idx_dest = content->af_fv2_top- content->dist_index_reverse_err;
			}
			else
			{
				p_af_result->dist_idx_dest = content->af_fv2_top + content->dist_index_reverse_err;
			}
			if (p_af_result->dist_idx_dest >= content->f_nbd) {
				p_af_result->dist_idx_dest = content->f_nbd;			   
			} else if (p_af_result->dist_idx_dest <= content->f_fbd) {
				p_af_result->dist_idx_dest = content->f_fbd; 	
			}

            content->af_control_param.af_next_stm = FAIL_SAF;
        }*/
    }
	
    stm_printf(">>>>>>>>>>>>>>>>fine_saf,dist_idx_dest is %d\n", p_af_result->dist_idx_dest);
    stm_printf(">>>>>>>>>>>>>>>>fine_saf,dist_index_reverse_err is %d\n", content->dist_index_reverse_err);
    stm_printf("coarse_saf-fv2_data_max is %llu, fv2_data is %llu\n", content->fv2_data_max, content->fv2_data);
    return 0;
}




static int done_saf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result)
{
    //ImageOperate *op;
    Isp3AContentAF *content;
    s32 dist_pulse;
    u8 zoom_idx_dest;
    s32 pulse_dest_l;
    s32 pulse_dest_h;
    s32 pulse_dest;
    s32 reverse_pulse;
    //op = &(processor->operate);
    content = &(processor->content.AFData);
    zoom_idx_dest = p_af_result->zoom_idx_dest;
    pulse_dest_l = get_pulse(processor, zoom_idx_dest, p_af_result->dist_idx_dest);
    pulse_dest_h = get_pulse(processor, (zoom_idx_dest + 1), p_af_result->dist_idx_dest);   
    dist_pulse =
        af_interpolate(pulse_dest_l, pulse_dest_h, zoom_idx_dest,
        (zoom_idx_dest + 1), p_af_result->zoom_idx_dest);
    
    //stm_printf(">>>>>>>>>>>>>>>>done_saf\n");
    zoom_idx_dest = p_af_result->zoom_idx_dest;
    pulse_dest_l = content->af_zoom_param[zoom_idx_dest].zoom_pluse;
    pulse_dest_h = content->af_zoom_param[(zoom_idx_dest + 1)].zoom_pluse;
    pulse_dest =
        af_interpolate(pulse_dest_l, pulse_dest_h, zoom_idx_dest,
        (zoom_idx_dest + 1), p_af_result->zoom_idx_dest);
    reverse_pulse = get_pulse(processor, 0, -(content->dist_index_reverse_err));
    af_printf("(done) Zoomid(%f/%d) - reverse err step is %d, dist_idx_dest is %d\n", 
        p_af_result->zoom_idx_dest,pulse_dest,
        reverse_pulse,
        dist_pulse);
    content->af_control_param.af_next_stm = IDLE_SAF;
    if(content->done_saf_set_mode == CALIB_SAF)
    {
        content->af_ctrl_param.workingMode = CALIB_SAF;
        content->af_control_param.af_next_stm = DO_CALIB_SAF;  
        
        if(content->af_control_param.direction > 0 && content->dist_index_reverse_err > 0)
        {
           reverse_pulse = get_pulse(processor, 0, -(content->dist_index_reverse_err)); 
           content->calib_saf_out.c[(s32)(p_af_result->zoom_idx_dest)] = 
            reverse_pulse + dist_pulse;
           content->calib_saf_out.focus_reverse_err[(u8)(p_af_result->zoom_idx_dest)] = 
            reverse_pulse;
        }
        else  if(content->af_control_param.direction < 0  && content->dist_index_reverse_err < 0)
        {
           reverse_pulse = get_pulse(processor, 0, content->dist_index_reverse_err);
           content->calib_saf_out.c[(s32)(p_af_result->zoom_idx_dest)] = dist_pulse;
           content->calib_saf_out.focus_reverse_err[(u8)(p_af_result->zoom_idx_dest)] = reverse_pulse;
        }
        else
        {
           content->calib_saf_out.c[(s32)(p_af_result->zoom_idx_dest)] = dist_pulse;
           content->calib_saf_out.focus_reverse_err[(u8)(p_af_result->zoom_idx_dest)] = 0;
            
        }

        
   }
   else
   {
     if (content->af_G_param.zoom_enhance_enable != 0 && content->af_G_param.zoom_dir == 1
         && content->af_result.zoom_idx_dest > content->af_G_param.zoom_break_idx)
         content->af_G_param.dist_idx_init = content->af_result.dist_idx_dest;
     if (content->af_G_param.zoom_enhance_enable != 0 && content->af_G_param.zoom_dir == 2
         && content->af_result.zoom_idx_dest < content->af_G_param.zoom_break_idx)
         content->af_G_param.dist_idx_init = content->af_result.dist_idx_dest;
   }
    return 0;
}


static int fail_saf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result)
{
    //ImageOperate *op;
    Isp3AContentAF *content;
    s32 dist_pulse;
    u8 zoom_idx_dest;
    s32 pulse_dest_l;
    s32 pulse_dest_h;
    s32 pulse_dest;
    s32 reverse_pulse;
    //op = &(processor->operate);
    content = &(processor->content.AFData);
    zoom_idx_dest = p_af_result->zoom_idx_dest;
    pulse_dest_l = get_pulse(processor, zoom_idx_dest, p_af_result->dist_idx_dest);
    pulse_dest_h = get_pulse(processor, (zoom_idx_dest + 1), p_af_result->dist_idx_dest);   
    dist_pulse =
        af_interpolate(pulse_dest_l, pulse_dest_h, zoom_idx_dest,
        (zoom_idx_dest + 1), p_af_result->zoom_idx_dest);
    
    //stm_printf(">>>>>>>>>>>>>>>>done_saf\n");
    zoom_idx_dest = p_af_result->zoom_idx_dest;
    pulse_dest_l = content->af_zoom_param[zoom_idx_dest].zoom_pluse;
    pulse_dest_h = content->af_zoom_param[(zoom_idx_dest + 1)].zoom_pluse;
    pulse_dest =
        af_interpolate(pulse_dest_l, pulse_dest_h, zoom_idx_dest,
        (zoom_idx_dest + 1), p_af_result->zoom_idx_dest);
    reverse_pulse = get_pulse(processor, 0, -(content->dist_index_reverse_err));
    af_printf("(Other) Zoomid(%f/%d) - reverse err step is %d, dist_idx_dest is %d\n", 
        p_af_result->zoom_idx_dest,pulse_dest,
        reverse_pulse,
        dist_pulse);
    content->af_control_param.af_next_stm = IDLE_SAF;
    if(content->done_saf_set_mode == CALIB_SAF)
    {
        content->af_ctrl_param.workingMode = CALIB_SAF;
        content->af_control_param.af_next_stm = DO_CALIB_SAF;  
        
        if(content->af_control_param.direction > 0 && content->dist_index_reverse_err > 0)
        {
           reverse_pulse = get_pulse(processor, 0, -(content->dist_index_reverse_err)); 
           content->calib_saf_out.c[(s32)(p_af_result->zoom_idx_dest)] = 
            reverse_pulse + dist_pulse;
           content->calib_saf_out.focus_reverse_err[(u8)(p_af_result->zoom_idx_dest)] = 
            reverse_pulse;
        }
        else  if(content->af_control_param.direction < 0  && content->dist_index_reverse_err < 0)
        {
           reverse_pulse = get_pulse(processor, 0, content->dist_index_reverse_err);
           content->calib_saf_out.c[(s32)(p_af_result->zoom_idx_dest)] = dist_pulse;
           content->calib_saf_out.focus_reverse_err[(u8)(p_af_result->zoom_idx_dest)] = reverse_pulse;
        }
        else
        {
           content->calib_saf_out.c[(s32)(p_af_result->zoom_idx_dest)] = dist_pulse;
           content->calib_saf_out.focus_reverse_err[(u8)(p_af_result->zoom_idx_dest)] = 0;
            
        }

        
    }
    return 0;
}

static int idle_saf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result)
{
    //u32 dist_idx;


    //dist_idx = p_af_result->dist_idx_dest;
    //dist = 500.0 / (float) dist_idx;
    //printf("^_^ locked at %fm\n",dist);
    return 0;
}

static int wait_vd_saf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result)
{
    //ImageOperate *op;
    Isp3AContentAF *content;

    //op = &(processor->operate);
    content = &(processor->content.AFData);

    if (content->wait_for_vd == 0) {
        content->af_control_param.af_next_stm = p_dragon->store_af_stm;
        clean_dragon(processor, p_dragon);
    } else {
        content->wait_for_vd--;
    }
    stm_printf(">>>>>>>>>>>>>>>>wait_vd_saf\n");
    return 0;
}
#endif


/*
*    Contious AF
*/

static int init_caf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result)
{
    //ImageOperate *op;
    Isp3AContentAF *content;

    //op = &(processor->operate);
    content = &(processor->content.AFData);

    stm_printf(">>>>>>>>>>>>>>>>enter INIT_CAF\n");

    content->af_control_param.ROI_size = SMALL_WINDOWS;
    content->af_control_param.direction = 1;

    content->f_nbd_sign = 0;
    content->f_fbd_sign = 0;
    p_dragon->fm1 = p_af_result->dist_idx_src;
    p_dragon->fm2 = p_af_result->dist_idx_src;
    p_dragon->af_range_flg = 0;
    content->af_peak_sign = 0;
    content->interval_cnt = 0;
    p_dragon->skip_frame = 0;
    p_dragon->skip_frame_TH = 0;
    p_dragon->store_len = 48;
    p_dragon->f_step = 1;
    clean_dragon(processor, p_dragon);
    content->af_control_param.af_next_stm = INIT_CoarseCAF;
    return 0;
}

static int init_coarse_caf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_ctrl)
{
    //ImageOperate *op;
    Isp3AContentAF *content;

    //op = &(processor->operate);
    content = &(processor->content.AFData);

    stm_printf(">>>>>>>>>>>>>>>>enter INIT_Coarse_CAF\n");
    content->af_control_param.af_next_stm = CoarseCAF;
    clean_dragon(processor, p_dragon);
    return 0;
}

static int coarse_caf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result)
{
    int focus_step_1, focus_step_2;
    s32 fv_dir_chg;
    u8 step_factor;
    u8 zoom_idx = 0;
    //ImageOperate *op;
    Isp3AContentAF *content;

    //op = &(processor->operate);
    content = &(processor->content.AFData);

    stm_printf(">>>>>>>>>>>>>>>>enter Coarse_CAF\n");
    zoom_idx = p_af_control->zoom_idx;
    step_factor = content->af_G_param.zoom_param[zoom_idx].Csf;
    step_factor = 80;//test

    p_dragon->fv_rev_fac = content->af_G_param.zoom_param[zoom_idx].Cpk_th;
    p_dragon->fv_rev_fac = 40;//test

    focus_step_1 = (content->video_fv1_diff * 2 + content->video_lum_diff * 6);
    focus_step_2 = (ABS((s32) p_af_result->dist_idx_src) / 100) + 1;
    p_dragon->skip_frame_TH = p_dragon->skip_frame;
    p_dragon->fv_rev_TH = content->fv2_data_max * p_dragon->fv_rev_fac / 100;
    p_dragon->f_step = focus_step_1 * focus_step_2 * step_factor;
    /*if (p_dragon->f_step < step_factor)
        p_dragon->f_step = step_factor;
    if (p_dragon->f_step > step_factor * 5)
        p_dragon->f_step = step_factor * 5;*/
    p_dragon->f_step = step_factor;
    fv_dir_chg = content->fv2_data_max - content->fv2_data;
//    s32 idea_TH;
//    idea_TH = (fv2_data_max - fv2_data)*100/fv2_data_max;
    printf("f_step1 is %d, f_step2 is %d, f_step is %d, idea_TH is %d  fv2_data_max is %d\n",focus_step_1,focus_step_2,p_dragon->f_step, p_dragon->fv_rev_TH,content->fv2_data_max);
    if (fv_dir_chg > p_dragon->fv_rev_TH) {
        content->af_control_param.direction *= -1;
        p_dragon->skip_frame++;
        p_dragon->fm2 = p_dragon->fm1;
        p_dragon->fm1 = p_af_result->dist_idx_src;
        if (p_dragon->skip_frame >= 3) {
            printf("Coarse_find_peak!\n");
            content->af_peak_sign = 1;
            p_dragon->af_range_flg = 1;
            p_af_result->dist_idx_dest = content->af_fv2_top;
            p_dragon->clr_q_flg = AF_CLR_Q;
            content->wait_for_vd = 2;
            p_dragon->store_af_stm = INIT_FineCAF;
            content->af_control_param.af_next_stm = WAIT_VD_CAF;
        } else {
            p_af_result->dist_idx_dest =
                ((3 * content->af_fv2_top + p_af_result->dist_idx_src) >> 2);
            p_dragon->clr_q_flg = AF_CLR_Q;
            content->wait_for_vd = 2;
            p_dragon->store_af_stm = CoarseCAF;
            content->af_control_param.af_next_stm = WAIT_VD_CAF;
        }
    } else {
        fv_dir_chg = content->fvg_data_avg - content->fvg_data_avg_min;
        p_dragon->fv_rev_TH = content->fvg_data_avg * p_dragon->fv_rev_fac / 100;
        if ((content->fvg_down_count >= 5) && (content->fvg_enable)) {
            content->af_control_param.direction *= -1;
            p_dragon->skip_frame++;
            p_dragon->fm2 = p_dragon->fm1;
            p_dragon->fm1 = p_af_result->dist_idx_src;
            //printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>FV3_find_peak! C\n");
            content->af_peak_sign = 1;
            p_dragon->af_range_flg = 1;
            p_af_result->dist_idx_dest = content->af_fvg_top;
            p_dragon->clr_q_flg = AF_CLR_Q;
            content->wait_for_vd = 2;
            p_dragon->store_af_stm = CoarseCAF;
            content->af_control_param.af_next_stm = WAIT_VD_CAF;
        } else {
            content->af_dis += p_dragon->f_step * content->af_control_param.direction;
            p_af_result->dist_idx_dest =
                (s32) (p_af_result->dist_idx_src +
                (p_dragon->f_step * content->af_control_param.direction));
            if (p_af_result->dist_idx_dest >= content->f_nbd) {
                p_dragon->fm2 = p_dragon->fm1;
                p_dragon->fm1 = content->f_nbd;
                content->f_nbd_sign++;
                p_dragon->skip_frame++;
                content->af_control_param.direction = -1;
                p_af_result->dist_idx_dest = content->defalut_fd_val;
                p_dragon->clr_q_flg = AF_CLR_Q;
                content->wait_for_vd = 2;
                p_dragon->store_af_stm = CoarseCAF;
                content->af_control_param.af_next_stm = WAIT_VD_CAF;
            } else if (p_af_result->dist_idx_dest <= content->f_fbd) {
                p_dragon->fm2 = p_dragon->fm1;
                p_dragon->fm1 = content->f_fbd;
                content->f_fbd_sign++;
                p_dragon->skip_frame++;
                content->af_control_param.direction = 1;
                p_af_result->dist_idx_dest = content->defalut_fd_val;
                p_dragon->clr_q_flg = AF_CLR_Q;
                content->wait_for_vd = 2;
                p_dragon->store_af_stm = CoarseCAF;
                content->af_control_param.af_next_stm = WAIT_VD_CAF;
            }
            if ((content->f_nbd_sign > 1) && (content->f_fbd_sign > 1)) {
                p_af_result->dist_idx_dest = content->defalut_fd_val;   // TODO: walk slowly to far distance;
                p_dragon->clr_q_flg = AF_CLR_Q;
                content->wait_for_vd = 2;
                p_dragon->store_af_stm = INIT_Watch_CAF;
                content->af_control_param.af_next_stm = WAIT_VD_CAF;
                content->f_nbd_sign = 0;
                content->f_fbd_sign = 0;
            }
//            assert(p_af_result->pps);
            if (!p_af_result->pps) {
                return -1;
            }
        }
    }
    return 0;
}

static int init_fine_caf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_ctrl)
{
    //ImageOperate *op;
    Isp3AContentAF *content;

    //op = &(processor->operate);
    content = &(processor->content.AFData);

    content->f_nbd_sign = 0;
    content->f_fbd_sign = 0;
    content->af_dis = 0;
    content->af_peak_sign = 0;
    content->interval_cnt = 0;
    p_dragon->skip_frame = 0;
    p_dragon->skip_frame_TH = 0;
    p_dragon->f_step = 1;
    if (p_dragon->af_range_flg == 1) {
        p_dragon->search_len_TH =
            ((ABS((s32) p_dragon->fm2 - (s32) p_dragon->fm1)) * 2 / 3);
        if (p_dragon->search_len_TH < p_dragon->f_dof)
            p_dragon->search_len_TH = p_dragon->f_dof;
    } else {
        p_dragon->search_len_TH = p_dragon->f_dof;
    }
    p_dragon->af_range_flg = 0;
    p_dragon->store_len = 48;
    clean_dragon(processor, p_dragon);
    stm_printf(">>>>>>>>>>>>>>>>enter INIT_FineCAF\n");
    content->af_control_param.af_next_stm = FineCAF;

    return -1;
}

static int fine_caf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result)
{
    s32 fv_dir_chg;

//    s32 focus_step_1;
//    s32 focus_step_2;
    u8 step_factor;
    u8 zoom_idx = 0;
    ImageOperate *op;
    Isp3AContentAF *content;

    op = &(processor->operate);
    content = &(processor->content.AFData);

    zoom_idx = p_af_control->zoom_idx;
    step_factor = content->af_G_param.zoom_param[zoom_idx].Fsf;
    step_factor = 80;
    stm_printf(">>>>>>>>>>>>>>>>enter Fine_CAF\n");
    if (ABS(content->af_dis) > p_dragon->search_len_TH) {
        content->af_control_param.direction *= -1;
        p_af_result->dist_idx_dest = content->defalut_fd_val;
        content->af_control_param.af_next_stm = INIT_CoarseCAF;
    } else {
        if (ABS((s16) (content->gain_shift_pre) - (s16) (content->gain_shift)) >
            content->af_G_param.zoom_param[p_af_control->zoom_idx].ev_idx_diff_TH) {
            content->interval_cnt = 0;
            p_dragon->skip_frame = 0;
            clean_dragon(processor, p_dragon);
            return 0;
        }
        if (content->interval_cnt == p_dragon->skip_frame_TH) {
            content->interval_cnt = 0;
            p_dragon->skip_frame_TH = 0;
            p_dragon->fv_rev_fac = content->af_G_param.zoom_param[zoom_idx].Fpk_th;
//            focus_step_2 = (ABS((s32)p_af_result->dist_idx_src) / 100) + 1;
//            focus_step_1 = (video_fv1_diff * 2 + video_lum_diff * 2);
            p_dragon->skip_frame_TH =
                ((p_dragon->skip_frame > 1) ? 1 : p_dragon->skip_frame);

            p_dragon->fv_rev_fac = 5;//test

            p_dragon->fv_rev_TH = content->fv2_data_max * p_dragon->fv_rev_fac / 1000;
            p_dragon->f_step = step_factor;     //focus_step_1 * focus_step_2*step_factor;
            fv_dir_chg = content->fv2_data_max - content->fv2_data;
//            s32 idea_TH;
//            idea_TH = (fv2_data_max - fv2_data)*100/fv2_data_max;
//            printf("f_step1 is %d, f_step2 is %d, f_step is %d, idea_TH is %d\n",focus_step_1,focus_step_2,p_dragon->f_step,idea_TH);
            if (fv_dir_chg > p_dragon->fv_rev_TH) {
                content->af_control_param.direction *= -1;
                p_dragon->skip_frame++;
                p_dragon->fm2 = p_dragon->fm1;
                p_dragon->fm1 = p_af_result->dist_idx_dest;
                stm_printf("mark: %d, %d (Fine CAF)\n", p_dragon->fm1,
                    p_dragon->fm2);
                if (p_dragon->skip_frame >= 3) {
                    op->printf("Fine_find_peak!\n");
                    content->af_peak_sign = 1;
                    p_dragon->af_range_flg = 1;
                    p_dragon->search_len_TH =
                        ABS((s32) p_dragon->fm2 - (s32) p_dragon->fm1);
                    p_af_result->dist_idx_dest = content->af_fv2_top;
                    p_dragon->clr_q_flg = AF_CLR_Q;
                    content->wait_for_vd = 2;
                    content->af_dis = 0;
                    p_dragon->store_af_stm = INIT_Watch_CAF;
                    content->af_control_param.af_next_stm = WAIT_VD_CAF;
                } else {
                    p_af_result->dist_idx_dest =
                        ((3 * content->af_fv2_top + p_af_result->dist_idx_src) >> 2);
                    p_dragon->clr_q_flg = AF_CLR_Q;
                    content->wait_for_vd = 2;
                    p_dragon->store_af_stm = FineCAF;
                    content->af_control_param.af_next_stm = WAIT_VD_CAF;
                    stm_printf("af_reverse (Fine-CAF)\n");
//                    printf("reverse to:fpos %d\n",p_af_result->dist_idx_dest);
                }
            } else {
                content->af_dis += p_dragon->f_step * content->af_control_param.direction;
                p_af_result->dist_idx_dest =
                    (s32) (p_af_result->dist_idx_src +
                    (p_dragon->f_step * content->af_control_param.direction));
                if (p_af_result->dist_idx_dest >= content->f_nbd) {
                    content->f_nbd_sign++;
                    p_dragon->fm2 = p_dragon->fm1;
                    p_dragon->fm1 = content->f_nbd;
                    content->af_control_param.direction *= -1;
                    p_af_result->dist_idx_dest = content->f_nbd;
                    p_dragon->clr_q_flg = AF_CLR_Q;
                    content->wait_for_vd = 2;
                    p_dragon->store_af_stm = FineCAF;
                    content->af_control_param.af_next_stm = WAIT_VD_CAF;
                    stm_printf("NEAR Lim (Fine-CAF)\n");
                } else if (p_af_result->dist_idx_dest <= content->f_fbd) {
                    content->f_fbd_sign++;
                    p_dragon->fm2 = p_dragon->fm1;
                    p_dragon->fm1 = content->f_fbd;
                    content->af_control_param.direction *= -1;
                    p_af_result->dist_idx_dest = content->f_fbd;
                    content->af_control_param.af_next_stm = FineCAF;
                    p_dragon->clr_q_flg = AF_CLR_Q;
                    content->wait_for_vd = 2;
                    p_dragon->store_af_stm = FineCAF;
                    content->af_control_param.af_next_stm = WAIT_VD_CAF;
                    stm_printf("FAR Lim (Fine-CAF)\n");
                }
                if ((content->f_nbd_sign > 0) && (content->f_fbd_sign > 0)) {
                    p_af_result->dist_idx_dest = content->af_fv2_top;
                    p_dragon->clr_q_flg = AF_CLR_Q;
                    content->wait_for_vd = 2;
                    p_dragon->store_af_stm = INIT_Watch_CAF;
                    content->af_control_param.af_next_stm = WAIT_VD_CAF;
                    content->f_nbd_sign = 0;
                    content->f_fbd_sign = 0;
                }
            }
        } else {
            content->interval_cnt++;
        }
    }
    return 0;
}

static int init_watch_caf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result)
{
    //ImageOperate *op;
    Isp3AContentAF *content;

    //op = &(processor->operate);
    content = &(processor->content.AFData);

    stm_printf(">>>>>>>>>>>>>>>>enter INIT_WatchCAF\n");
    content->interval_cnt = 0;
    p_dragon->skip_frame_TH = 4;
    p_dragon->store_len = 64;
    p_dragon->f_step = 1;
    clean_dragon(processor, p_dragon);
    p_dragon->fv_max_TH = content->fv2_data;
    content->af_control_param.af_next_stm = Watch_CAF;
    return 0;
}

static int watch_caf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result)
{
    s32 fv_dir_chg;
    u8 zoom = 0;

    bool agc_change;
    bool sec_change;
    bool time_out;
    bool fv_dir_chage;
    //ImageOperate *op;
    Isp3AContentAF *content;

    //op = &(processor->operate);
    content = &(processor->content.AFData);

    zoom = content->af_result.zoom_idx_dest;
    p_dragon->skip_frame_TH = content->af_G_param.zoom_param[zoom].skip_frame_TH;
    p_dragon->sec_chg_TH = 10;
    p_dragon->fv_rev_fac = 35 + content->gain_shift;
    p_dragon->fv_rev_TH = p_dragon->fv_max_TH * p_dragon->fv_rev_fac / 100;
    fv_dir_chg = ABS((s32) (p_dragon->fv_max_TH) - (s32) (content->fv2_data));
/*>_<*/
    if (ABS((s16) (content->gain_shift_pre) - (s16) (content->gain_shift)) >
        content->af_G_param.zoom_param[zoom].ev_idx_diff_TH)
        {agc_change = 1;
           printf("WatchCAF - Next is INIT_FineCAF - agc_change\n");}

    else
        agc_change = 0;

    if (p_dragon->sec_chg_val > p_dragon->sec_chg_TH)
        {sec_change = 1;
        printf("WatchCAF - Next is INIT_FineCAF - sec_change\n");}
    else
        sec_change = 0;

    if (content->interval_cnt > p_dragon->skip_frame_TH)
        {time_out = 1;
        printf("WatchCAF - Next is INIT_FineCAF - interval_cnt\n");}

    else
        time_out = 0;

    if (fv_dir_chg > p_dragon->fv_rev_TH)
        {fv_dir_chage = 1;
        printf("WatchCAF - Next is INIT_FineCAF - fv_dir_chg\n ");}

    else
        fv_dir_chage = 0;
/*>_<*/
    if (sec_change | agc_change | fv_dir_chage) {
        content->changed_sign = 1;
    }

    if (content->changed_sign) {
        if (time_out) {
            content->af_control_param.af_next_stm = INIT_FineCAF;
            content->interval_cnt = 0;
            content->changed_sign = 0;
        } else {
            content->interval_cnt++;
        }
    }

    stm_printf(">>>>>>>>>>>>>>>>enter WatchCAF %d\n",
        p_af_result->dist_idx_src);
    return 0;
}

static int wait_vd_caf(Isp3AProcessor *processor, af_dragon_t * p_dragon, af_result_t * p_af_result)
{
    //ImageOperate *op;
    Isp3AContentAF *content;

    //op = &(processor->operate);
    content = &(processor->content.AFData);

    if (content->wait_for_vd == 0) {
        content->af_control_param.af_next_stm = p_dragon->store_af_stm;
        if (p_dragon->clr_q_flg == AF_CLR_Q)
            clean_dragon(processor, p_dragon);
    } else {
        content->wait_for_vd--;
    }
    stm_printf(">>>>>>>>>>>>>>>>wait_vd_caf\n");
    return 0;
}

static int ztrack_caf(Isp3AProcessor *processor, af_result_t * p_af_result, u8 target_zm_idx)
{
    float target = target_zm_idx;

    if (p_af_result->zoom_idx_dest > target)
        //p_af_result->zoom_idx_dest += -0.1f;
        p_af_result->zoom_idx_dest += -0.05f;

    else
        //p_af_result->zoom_idx_dest += 0.1f;
        p_af_result->zoom_idx_dest += 0.05f;

//    printf("p_af_result->zoom_idx_dest is %f\n",p_af_result->zoom_idx_dest);
    stm_printf(">>>>>>>>>>>>>>>>ztrack_caf\n");
    return 0;
}

static void Watch_runing_CAF(void)
{

}

static int af_CAF_control(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    lens_control_t * p_lens_ctrl, u8 lens_runing_flag)
{
    u8 zoom = 0;
    //ImageOperate *op;
    Isp3AContentAF *content;

    //op = &(processor->operate);
    content = &(processor->content.AFData);

    zoom = content->af_result.zoom_idx_dest;
    if (lens_runing_flag == 0) {
       /* if (content->first_sign) {
            af_rt_inf(processor, p_af_control, p_lens_ctrl);
            content->first_sign = 0;
            return 0;
        }*/

        if (content->first_sign == 1) {
             // processor->content.AFData.af_ctrl_param.zoom_idx =  3;
             content->af_result.zoom_idx_src =  0;
        
             content->af_result.zoom_idx_dest =  13;
                          
        
             af_decide_pulse(processor, &(content->af_result), p_lens_ctrl);
             content->first_sign = 2;
             printf("dist_idx_dest is %d\n", content->af_result.zoom_idx_dest);
             return 0;
         }
         if (content->first_sign == 2) {
             p_af_control->zoom_idx = 13;
        
             af_rt_inf(processor, p_af_control, p_lens_ctrl);
                          
             content->first_sign = 0;
             return 0;
        }
        content->af_result.dist_idx_src = content->af_result.dist_idx_dest;
        if (ABS(p_af_control->zoom_idx - content->af_result.zoom_idx_src) > 0.1) {
            content->af_control_param.af_next_stm = ZTRACK_CAF;
            p_dragon->store_af_stm = content->af_control_param.af_stm;
        } else if (content->af_control_param.af_next_stm == ZTRACK_CAF) {
            content->af_control_param.af_next_stm = INIT_FineCAF;
        }
        content->af_control_param.af_stm = content->af_control_param.af_next_stm;
        switch (content->af_control_param.af_next_stm) {
            case INIT_CAF:
                init_caf(processor, p_af_control, p_dragon, &(content->af_result));
                break;
            case INIT_CoarseCAF:
                init_coarse_caf(processor, p_af_control, p_dragon, &(content->af_result));
                break;
            case CoarseCAF:
                coarse_caf(processor, p_af_control, p_dragon, &(content->af_result));
                break;
            case INIT_FineCAF:
                init_fine_caf(processor, p_af_control, p_dragon, &(content->af_result));
                break;
            case FineCAF:
                fine_caf(processor, p_af_control, p_dragon, &(content->af_result));
                break;
            case INIT_Watch_CAF:
                init_watch_caf(processor, p_af_control, p_dragon, &(content->af_result));
                break;
            case Watch_CAF:
                watch_caf(processor, p_af_control, p_dragon, &(content->af_result));
                break;
            case WAIT_VD_CAF:
                wait_vd_caf(processor, p_dragon, &(content->af_result));
                break;
            case ZTRACK_CAF:
                ztrack_caf(processor, &(content->af_result), p_af_control->zoom_idx);
                break;
            default:
                return -1;

        }
        af_decide_pulse(processor, &(content->af_result), p_lens_ctrl);
        p_af_control->focus_idx = content->af_result.dist_idx_src;
        if (p_lens_ctrl->pps > content->af_G_param.zoom_param[zoom].pps_max)
            p_lens_ctrl->pps = content->af_G_param.zoom_param[zoom].pps_max;
    } else {
        Watch_runing_CAF();
    }
    return 0;
}

static int af_SAF_control(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    lens_control_t * p_af_ctrl, u8 lens_runing_flag)
{
//    static u8 first_sign = 1;
    //ImageOperate *op;
    Isp3AContentAF *content;

    //op = &(processor->operate);
    content = &(processor->content.AFData);
    //printf("lens_runing_flag is %d\n", lens_runing_flag);
    if (lens_runing_flag == 0) {
        /*if (content->first_sign) {
            af_rt_inf(processor, p_af_control, p_af_ctrl);
            content->first_sign = 0;
            return 0;
        }*/
        //printf("content->first_sign is %d\n", content->first_sign);
        if (content->first_sign == 1) {
             // processor->content.AFData.af_ctrl_param.zoom_idx =  3;
             content->af_result.zoom_idx_src =  0;

             content->af_result.zoom_idx_dest =  content->af_G_param.start_zoom_idx;
                   
             af_decide_pulse(processor, &(content->af_result), p_af_ctrl);
             content->first_sign = 2;
             //printf("dist_idx_dest is %d\n", content->af_result.zoom_idx_dest);
             return 0;
         }
         if (content->first_sign == 2) {
             p_af_control->zoom_idx = content->af_G_param.start_zoom_idx;

             //af_rt_inf(processor, p_af_control, p_af_ctrl);
                   
             content->first_sign = 0;
             return 0;
         }
        
        content->af_result.dist_idx_src = content->af_result.dist_idx_dest;
        content->af_control_param.af_stm = content->af_control_param.af_next_stm;
        switch (content->af_control_param.af_next_stm) {
            case INIT_SAF:
                init_saf(processor, p_af_control, p_dragon, &(content->af_result));
                break;
            case CoarseSAF:
                coarse_saf(processor, p_af_control, p_dragon, &(content->af_result));
                break;
            case INIT_FineSAF:
                init_fine_saf(processor, p_af_control, p_dragon, &(content->af_result));
                break;
            case FineSAF:
                fine_saf(processor, p_af_control, p_dragon, &(content->af_result));
                break;
            case DONE_SAF:
                done_saf(processor, p_af_control, p_dragon, &(content->af_result));
                break;
            case FAIL_SAF:
                fail_saf(processor, p_af_control, p_dragon, &(content->af_result));
                break;
            case IDLE_SAF:
                idle_saf(processor, p_af_control, p_dragon, &(content->af_result));
                break;
            case WAIT_VD_SAF:
                wait_vd_saf(processor, p_af_control, p_dragon, &(content->af_result));
            default:
                return -1;
        }
        af_decide_pulse(processor, &(content->af_result), p_af_ctrl);
    } else {

    }
    return 0;
}
#if 0
static int af_manual_control(Isp3AProcessor *processor, af_control_t * p_af_control,
    af_dragon_t * p_dragon, lens_control_t * p_lens_ctrl, u8 lens_runing_flag)
{
    #if 1
//    static u8 first_sign = 1;
    //ImageOperate *op;
    Isp3AContentAF *content;

    //op = &(processor->operate);
    content = &(processor->content.AFData);

    if (lens_runing_flag == 0) {
        /*if (content->first_sign) {
            af_rt_inf(processor, p_af_control, p_lens_ctrl);
            content->first_sign = 0;
            return 0;
        }*/
        //content->af_result.dist_idx_dest = p_af_control->focus_idx;
        if (content->first_sign == 1) {
                         // processor->content.AFData.af_ctrl_param.zoom_idx =  3;
                          content->af_result.zoom_idx_src =  0;
        
                          content->af_result.zoom_idx_dest =  2;
                          
        
                          af_decide_pulse(processor, &(content->af_result), p_lens_ctrl);
                          content->first_sign = 2;
                          //printf("dist_idx_dest is %d\n", content->af_result.zoom_idx_dest);
                          return 0;
                  }
               if (content->first_sign == 2) {
                          p_af_control->zoom_idx = 2;
        
                          af_rt_inf(processor, p_af_control, p_lens_ctrl);
                          
        
                          content->first_sign = 0;
                          return 0;
                  }
        
        p_af_control->zoom_idx = 15;
        
        //printf("dist_idx_dest is %d\n",af_result.dist_idx_dest);
        if (ABS(p_af_control->zoom_idx - content->af_result.zoom_idx_src) > 0.05) {
            if (p_af_control->zoom_status == 0)
            {
                p_af_control->zoom_status = 1;
                return 0;
            }
            ztrack_caf(processor, &(content->af_result), p_af_control->zoom_idx);
            //content->af_result.dist_idx_dest = content->af_result.dist_idx_src = 0;
            af_decide_pulse(processor, &(content->af_result), p_lens_ctrl);
            
           
        }
        else
        {
            content->af_ctrl_param.workingMode = SAF;
            p_af_control->zoom_status = 0;
            //printf("workingMode is %d, zoom_status is %d\n", content->af_ctrl_param.workingMode, p_lens_ctrl->zoom_status);
        }
        
       
        
        
        
    } else {

    }
 #endif
    return 0;
}
#endif

#if 1
static int af_manual_control(Isp3AProcessor *processor, af_control_t * p_af_control,
    af_dragon_t * p_dragon, lens_control_t * p_lens_ctrl, u8 lens_runing_flag)
{
//    static u8 first_sign = 1;
    //ImageOperate *op;
    Isp3AContentAF *content;
    Isp3APublicContent* publicData;
    //op = &(processor->operate);
    content = &(processor->content.AFData);
    publicData = &(processor->content.publicData);
    
    if (lens_runing_flag == 0) {
        /*if (content->first_sign) {
            af_rt_inf(processor, p_af_control, p_lens_ctrl);
            content->first_sign = 0;
            return 0;
        }*/
        //content->af_result.dist_idx_dest = p_af_control->focus_idx;
        if (content->first_sign == 1) {
            // processor->content.AFData.af_ctrl_param.zoom_idx =  3;
            content->af_result.zoom_idx_src =  0;
        
            content->af_result.zoom_idx_dest =  content->af_G_param.start_zoom_idx;
                          
        
            af_decide_pulse(processor, &(content->af_result), p_lens_ctrl);
            content->first_sign = 2;
            //printf("dist_idx_dest is %d\n", content->af_result.zoom_idx_dest);
            return 0;
        }
         if (content->first_sign == 2) {
            p_af_control->zoom_idx = content->af_G_param.start_zoom_idx;
        
            //af_rt_inf(processor, p_af_control, p_lens_ctrl);
            content->first_sign = 0;
            return 0;
         }
        switch(p_af_control->manual_mode)
        {
            case ZOOMIN:
                if (p_af_control->zoom_status == 0)
                {
                    if (content->af_G_param.zoom_enhance_enable != 0 && content->af_G_param.zoom_dir == 1
                        && content->af_result.zoom_idx_dest < content->af_G_param.zoom_break_idx)
                    {
                        content->af_result.dist_idx_dest = content->af_G_param.dist_idx_init;
                        af_decide_pulse(processor, &(content->af_result), p_lens_ctrl);
       
                        if (content->lens_ctrl.focus_update ) {
                            if (content->lens_ctrl.focus_pulse > 0) {
                                if (content->lens_drv.af_focus_f)
                                    content->lens_drv.af_focus_f(content->lens_ctrl.pps,
                                    content->lens_ctrl.focus_pulse);
                            }
	                        else {
                                if (content->lens_drv.af_focus_n)
                                    content->lens_drv.af_focus_n(content->lens_ctrl.pps,
                                        (-content->lens_ctrl.focus_pulse));
                            }
                         }
                         content->lens_ctrl.focus_update = 0;
                    }
                    p_af_control->zoom_status = 1;
                    pthread_cond_signal(&(publicData->af_zoom_cond));
                    return 0;
                }
                if (content->af_result.zoom_idx_dest + content->af_G_param.manual_zoom_step <= content->af_G_param.end_zoom_idx)
                {
                    content->af_result.zoom_idx_dest += content->af_G_param.manual_zoom_step;
                    p_af_control->zoom_idx = content->af_result.zoom_idx_dest;
                    //printf("zoom_idx_dest is %f, zoom_status is %d\n", 
                    //    content->af_result.zoom_idx_dest, p_af_control->zoom_status);
                }
                break;
            case ZOOMOUT:
                if (p_af_control->zoom_status == 0)
                {
                    if (content->af_G_param.zoom_enhance_enable != 0 && content->af_G_param.zoom_dir == 2
                        && content->af_result.zoom_idx_dest > content->af_G_param.zoom_break_idx)
                    {
                        content->af_result.dist_idx_dest = content->af_G_param.dist_idx_init;
                        af_decide_pulse(processor, &(content->af_result), p_lens_ctrl);
       
                        if (content->lens_ctrl.focus_update ) {
                            if (content->lens_ctrl.focus_pulse > 0) {
                                if (content->lens_drv.af_focus_f)
                                    content->lens_drv.af_focus_f(content->lens_ctrl.pps,
                                    content->lens_ctrl.focus_pulse);
                            }
	                        else {
                                if (content->lens_drv.af_focus_n)
                                    content->lens_drv.af_focus_n(content->lens_ctrl.pps,
                                        (-content->lens_ctrl.focus_pulse));
                            }
                         }
                         content->lens_ctrl.focus_update = 0;
                    }
                    p_af_control->zoom_status = 1;
                    pthread_cond_signal(&(publicData->af_zoom_cond));
                    return 0;
                }
                if (content->af_result.zoom_idx_dest - content->af_G_param.manual_zoom_step >= content->af_G_param.begin_zoom_idx)
                {
                    content->af_result.zoom_idx_dest -= content->af_G_param.manual_zoom_step;
                    p_af_control->zoom_idx = content->af_result.zoom_idx_dest;
                }
                break;
            case FOCUSN:
                //printf("dist_idx_dest is %d\n", content->af_result.dist_idx_dest);
                if (content->af_result.dist_idx_dest + content->af_G_param.manual_focus_step <= content->f_nbd)
                {
                    content->af_result.dist_idx_dest += content->af_G_param.manual_focus_step;
                }
                break;
            case FOCUSF:
                if (content->af_result.dist_idx_dest - content->af_G_param.manual_focus_step >= content->f_fbd)
                {
                    content->af_result.dist_idx_dest -= content->af_G_param.manual_focus_step;
                }
                break;
            case ZOOMSTOP:
                p_af_control->manual_mode = MANUALIDLE;
                content->af_ctrl_param.workingMode = SAF;
                p_af_control->zoom_status = 0; 
                return 0;              
                break;
            case FOCUSSTOP:
                p_af_control->manual_mode = MANUALIDLE;
                return 0;            
                break;
            default:
               return -1;  
        }
        af_decide_pulse(processor, &(content->af_result), p_lens_ctrl);      
    } else {

    }
    return 0;
}
#endif

static int init_calib_saf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result, lens_control_t * p_lens_ctrl)
{
    Isp3AContentAF *content;
    content = &(processor->content.AFData);
    p_dragon->skip_frame = 0;
    p_dragon->store_len = 64;
    //clean_dragon(processor, p_dragon);
    p_af_result->dist_idx_cur = p_af_result->dist_idx_src;
    p_af_result->dist_idx_src = content->af_zoom_param[p_af_control->zoom_idx].zoom_pluse;
    p_af_result->dist_idx_dest = content->af_zoom_param[p_af_control->zoom_idx].zoom_pluse;
    p_dragon->store_af_stm = DO_CALIB_SAF_ZOOM_REVERSE;
    content->wait_for_vd = 4;
    content->af_control_param.af_next_stm = WAIT_VD_CALIB_SAF;
    content->af_control_param.direction = 1;
    //reverse_err_printf(">>>>>>>>>>init_calib_saf\n");

}
static int do_calib_saf_zoom_reverse(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result, lens_control_t * p_lens_ctrl)
{
    u64 fv_dir_chg;
    u8 zoom_idx;
    Isp3AContentAF *content;
    //reverse_err_printf(">>>>>>>>>>do_calib_saf_zoom_reverse\n");
    content = &(processor->content.AFData);

    zoom_idx = p_af_control->zoom_idx;
       
    p_dragon->fv_rev_TH = content->fv2_data_max * 0.6f;
    p_dragon->f_step = 4;
    fv_dir_chg = content->fv2_data_max - content->fv2_data;
     
    if (fv_dir_chg > p_dragon->fv_rev_TH) {
        content->af_control_param.direction *= -1;
        p_dragon->skip_frame++;
        if (p_dragon->skip_frame >= 3) {
            p_af_result->dist_idx_dest = content->af_G_param.zoom_reverse_after_z;
            content->af_control_param.af_next_stm = AFTER_CALIB_SAF_ZOOM_REVERSE;
            content->calib_saf_out.zoom_reverse_err[(u8)(p_af_result->zoom_idx_dest)] = content->af_fv2_top - content->af_fv2_top_pre;
            if(content->calib_saf_out.zoom_reverse_err[(u8)(p_af_result->zoom_idx_dest)] > content->af_G_param.zoom_reverse_err_max)
                content->calib_saf_out.zoom_reverse_err[(u8)(p_af_result->zoom_idx_dest)] = content->af_G_param.zoom_reverse_err_max;
            af_printf("(threshold) zoom reverse pulse is %d\n", content->calib_saf_out.zoom_reverse_err[(u8)(p_af_result->zoom_idx_dest)]);
            p_lens_ctrl->focus_pulse = content->af_zoom_param[content->af_G_param.start_zoom_idx].c 
              - content->af_zoom_param[content->af_G_param.zoom_reverse_idx].c - content->af_G_param.zoom_reverse_after_f;
            if(p_lens_ctrl->focus_pulse != 0)
            {
              p_lens_ctrl->focus_update = 1;
            }
            else
            {
              p_lens_ctrl->focus_update = 0;
            }
        } else {
            //clean_dragon(processor, p_dragon);
            p_af_result->dist_idx_dest =
               content->af_fv2_top  
               - 4 * p_dragon->f_step * content->af_control_param.direction;
				if (p_dragon->skip_frame == 2) {
					content->fv2_data_max_pre = content->fv2_data_max;
					content->af_fv2_top_pre = content->af_fv2_top;
				}
            
            p_dragon->store_af_stm = DO_CALIB_SAF_ZOOM_REVERSE;
            content->wait_for_vd = 4;
            content->af_control_param.af_next_stm = WAIT_VD_CALIB_SAF;
       }
       
   }
    else {
         p_af_result->dist_idx_dest =
         p_af_result->dist_idx_src +
            (p_dragon->f_step * content->af_control_param.direction);
         if (p_af_result->dist_idx_dest >= content->af_zoom_param[p_af_control->zoom_idx].zoom_pluse + content->af_G_param.z_inbd) {
             p_dragon->skip_frame++;
             content->af_control_param.direction *= -1;
             p_af_result->dist_idx_dest = content->af_zoom_param[p_af_control->zoom_idx].zoom_pluse + content->af_G_param.z_inbd * 0.6;
             if (p_dragon->skip_frame >= 3) {
                 p_af_result->dist_idx_dest = content->af_G_param.zoom_reverse_after_z;
                 content->af_control_param.af_next_stm = AFTER_CALIB_SAF_ZOOM_REVERSE;
                 content->calib_saf_out.zoom_reverse_err[(u8)(p_af_result->zoom_idx_dest)] = content->af_fv2_top - content->af_fv2_top_pre;
                 if(content->calib_saf_out.zoom_reverse_err[(u8)(p_af_result->zoom_idx_dest)] > content->af_G_param.zoom_reverse_err_max)
                    content->calib_saf_out.zoom_reverse_err[(u8)(p_af_result->zoom_idx_dest)] = content->af_G_param.zoom_reverse_err_max;
                 af_printf("(bound) zoom reverse pulse is %d\n", content->calib_saf_out.zoom_reverse_err[(u8)(p_af_result->zoom_idx_dest)]);
                 p_lens_ctrl->focus_pulse = content->af_zoom_param[content->af_G_param.start_zoom_idx].c 
                   - content->af_zoom_param[content->af_G_param.zoom_reverse_idx].c - content->af_G_param.zoom_reverse_after_f;  
                 if(p_lens_ctrl->focus_pulse != 0)
                 {
                  p_lens_ctrl->focus_update = 1;
                 }
                 else
                 {
                  p_lens_ctrl->focus_update = 0;
                 }                 
             } else {
                 //clean_dragon(processor, p_dragon);
			     if (p_dragon->skip_frame == 2) {
					 content->fv2_data_max_pre = content->fv2_data_max;
					 content->af_fv2_top_pre = content->af_fv2_top;
				 }
                 p_dragon->store_af_stm = DO_CALIB_SAF_ZOOM_REVERSE;
                 content->wait_for_vd = 4;
                 content->af_control_param.af_next_stm = WAIT_VD_CALIB_SAF;
             }
         }
         else if (p_af_result->dist_idx_dest <= content->af_zoom_param[p_af_control->zoom_idx].zoom_pluse - content->af_G_param.z_outbd) {
             p_dragon->skip_frame++;
             content->af_control_param.direction *= -1;
             p_af_result->dist_idx_dest = content->af_zoom_param[p_af_control->zoom_idx].zoom_pluse - content->af_G_param.z_outbd * 0.8;
             if (p_dragon->skip_frame >= 3) {
                p_af_result->dist_idx_dest = content->af_G_param.zoom_reverse_after_z;
                content->af_control_param.af_next_stm = AFTER_CALIB_SAF_ZOOM_REVERSE;               
                content->calib_saf_out.zoom_reverse_err[(u8)(p_af_result->zoom_idx_dest)] = content->af_fv2_top - content->af_fv2_top_pre;
                if(content->calib_saf_out.zoom_reverse_err[(u8)(p_af_result->zoom_idx_dest)] > content->af_G_param.zoom_reverse_err_max)
                   content->calib_saf_out.zoom_reverse_err[(u8)(p_af_result->zoom_idx_dest)] = content->af_G_param.zoom_reverse_err_max;
                af_printf("(bound) zoom reverse pulse is %d\n", content->calib_saf_out.zoom_reverse_err[(u8)(p_af_result->zoom_idx_dest)]);
                p_lens_ctrl->focus_pulse = content->af_zoom_param[content->af_G_param.start_zoom_idx].c 
                  - content->af_zoom_param[content->af_G_param.zoom_reverse_idx].c - content->af_G_param.zoom_reverse_after_f;
                if(p_lens_ctrl->focus_pulse != 0)
                {
                 p_lens_ctrl->focus_update = 1;
                }
                else
                {
                 p_lens_ctrl->focus_update = 0;
                }
             } else {
                //clean_dragon(processor, p_dragon);
				if (p_dragon->skip_frame == 2) {
				    content->fv2_data_max_pre = content->fv2_data_max;
					content->af_fv2_top_pre = content->af_fv2_top;
				}
                p_dragon->store_af_stm = DO_CALIB_SAF_ZOOM_REVERSE;
                content->wait_for_vd = 4;
                content->af_control_param.af_next_stm = WAIT_VD_CALIB_SAF;
            }
        }
    }
    if (p_af_result->dist_idx_dest -  p_af_result->dist_idx_src!= 0) {
      
        p_lens_ctrl->zoom_pulse = p_af_result->dist_idx_dest -  p_af_result->dist_idx_src; 
        p_lens_ctrl->zoom_update = 1;
    } else {
        p_lens_ctrl->zoom_pulse = 0;
        p_lens_ctrl->zoom_update = 0;
    }
    p_af_result->dist_idx_src = p_af_result->dist_idx_dest;
    p_lens_ctrl->pps = p_af_result->pps;

    return 0;
}

static int after_calib_saf_zoom_reverse(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result, lens_control_t * p_lens_ctrl)
{
    Isp3AContentAF *content;
    content = &(processor->content.AFData);
   
    content->af_result.zoom_idx_dest = content->af_G_param.start_zoom_idx;
    content->af_result.zoom_idx_src = content->af_G_param.start_zoom_idx;
    p_af_control->zoom_idx = content->af_G_param.start_zoom_idx;

    p_af_result->dist_idx_dest = content->af_zoom_param[p_af_control->zoom_idx].zoom_pluse;
    content->af_control_param.af_next_stm = DO_CALIB_SAF;
    if (p_af_result->dist_idx_dest -  p_af_result->dist_idx_src!= 0) {
        p_lens_ctrl->zoom_pulse = p_af_result->dist_idx_dest -  p_af_result->dist_idx_src; 
        p_lens_ctrl->zoom_update = 1;
    } else {
        p_lens_ctrl->zoom_pulse = 0;
        p_lens_ctrl->zoom_update = 0;
    }
    p_lens_ctrl->focus_pulse = content->af_G_param.zoom_reverse_after_f;
    if(p_lens_ctrl->focus_pulse != 0)
    {
       p_lens_ctrl->focus_update = 1;
    }
    else
    {
       p_lens_ctrl->focus_update = 0;
    }    
    p_af_result->dist_idx_src = p_af_result->dist_idx_dest;
    p_lens_ctrl->pps = p_af_result->pps;

    p_af_result->dist_idx_src = p_af_result->dist_idx_cur;
    p_af_result->dist_idx_dest = p_af_result->dist_idx_cur;
    
}

static int do_calib_saf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result, lens_control_t * p_lens_ctrl)
{
     Isp3AContentAF *content;
     u8 i= 0;
     content = &(processor->content.AFData);
       
       //reverse_err_printf(">>>>>>>>>>do_calib_saf\n");
     if(p_af_control->zoom_idx <= content->af_G_param.end_zoom_idx)
     {
         content->af_result.zoom_idx_dest = (p_af_control->zoom_idx)++;
         af_decide_pulse(processor, &(content->af_result), p_lens_ctrl);
         content->af_ctrl_param.workingMode = SAF;  
         content->af_control_param.af_next_stm = INIT_SAF;
         content->done_saf_set_mode = CALIB_SAF;
         //p_dragon->store_af_stm = INIT_SAF;
         //content->wait_for_vd = 4;
         //content->af_control_param.af_next_stm = WAIT_VD_SAF;    
     }
     else
     {    
         content->af_control_param.af_next_stm = AFTER_CALIB_SAF;
         if (content->calib_saf_out.zoom_reverse_err[content->af_G_param.zoom_reverse_idx]<0)
            content->calib_saf_out.c[32] = 0;
         else
            content->calib_saf_out.c[32] = content->calib_saf_out.zoom_reverse_err[content->af_G_param.zoom_reverse_idx];
         for(i = content->af_G_param.start_zoom_idx + 1; i< content->af_G_param.end_zoom_idx; i++)
         {
            if(content->calib_saf_out.focus_reverse_err[i] > 0)
                content->calib_saf_out.c[33] +=  content->calib_saf_out.focus_reverse_err[i];
            else
                content->calib_saf_out.c[33] +=  -(content->calib_saf_out.focus_reverse_err[i]);
         }
         content->calib_saf_out.c[33] /= content->af_G_param.end_zoom_idx - (content->af_G_param.start_zoom_idx + 1);
         content->calib_saf_out.c[33] -= 4;
         if( content->calib_saf_out.c[33] < 0)
             content->calib_saf_out.c[33] = 0;
         (content->pgetLensParams)(content->calib_saf_out.c, 34);
     }   
    
}

static int after_calib_saf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result, lens_control_t * p_lens_ctrl)
{
    ;
}

static int wait_vd_calib_saf(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    af_result_t * p_af_result)
{
    //ImageOperate *op;
    Isp3AContentAF *content;

    //op = &(processor->operate);
    content = &(processor->content.AFData);

    if (content->wait_for_vd == 0) {
        content->af_control_param.af_next_stm = p_dragon->store_af_stm;
        clean_dragon(processor, p_dragon);
    } else {
        content->wait_for_vd--;
    }
    stm_printf(">>>>>>>>>>>>>>>>wait_vd_calib_saf\n");
    return 0;
}


static int af_calib_of_saf_control(Isp3AProcessor *processor, af_control_t * p_af_control,
    af_dragon_t * p_dragon, lens_control_t * p_lens_ctrl, u8 lens_runing_flag)
{
   
    Isp3AContentAF *content;

    content = &(processor->content.AFData);
    
    if (lens_runing_flag == 0) {
        if (content->first_sign == 1) {
                      
           content->af_result.zoom_idx_src =  0;
           content->af_result.zoom_idx_dest =  content->af_G_param.zoom_reverse_idx;
           af_decide_pulse(processor, &(content->af_result), p_lens_ctrl);
           content->first_sign = 2;
                          
       }
       if (content->first_sign == 2) {
           p_af_control->zoom_idx = content->af_G_param.zoom_reverse_idx;
           //af_rt_inf(processor, p_af_control, p_lens_ctrl);                
           content->first_sign = 0;
          
           return 0;
       }
       switch (content->af_control_param.af_next_stm) 
       {
           case INIT_CALIB_SAF:
                init_calib_saf(processor, p_af_control, p_dragon, &(content->af_result), p_lens_ctrl);
                break;
           case DO_CALIB_SAF_ZOOM_REVERSE:
                do_calib_saf_zoom_reverse(processor, p_af_control, p_dragon, &(content->af_result), p_lens_ctrl);
                break;
           case AFTER_CALIB_SAF_ZOOM_REVERSE:
                after_calib_saf_zoom_reverse(processor, p_af_control, p_dragon, &(content->af_result), p_lens_ctrl);
                break;
           case DO_CALIB_SAF:
                do_calib_saf(processor, p_af_control, p_dragon, &(content->af_result), p_lens_ctrl);
                break;
           case AFTER_CALIB_SAF:
                after_calib_saf(processor, p_af_control, p_dragon, &(content->af_result), p_lens_ctrl);
                break;
           case WAIT_VD_CALIB_SAF:
                wait_vd_calib_saf(processor, p_af_control, p_dragon, &(content->af_result));
       }
        
    } 
 
    return 0;
}

static int af_calib_of_zoom_length_control(Isp3AProcessor *processor, af_control_t * p_af_control,
    af_dragon_t * p_dragon, lens_control_t * p_lens_ctrl, u8 lens_runing_flag)
{
    Isp3AContentAF *content;
    u64 fv_dir_chg;
    s32 zoom_length;
    content = &(processor->content.AFData);
    printf("first_sign is %d, dist_idx_src is %d, wait_for_vd is %d\n",
      content->first_sign, content->af_result.dist_idx_src, content->wait_for_vd);
    if (lens_runing_flag == 0) {
       if (content->first_sign == 1) {          
          p_lens_ctrl->focus_update = 1;
          p_lens_ctrl->focus_pulse = 500;//update small
          p_lens_ctrl->focus_pulse_pre = 0;
          p_lens_ctrl->zoom_pulse_pre = 0;
          content->first_sign = 2;  
          content->af_result.dist_idx_src = 0; 
          content->af_result.dist_idx_dest = 0; 
          content->wait_for_vd = 4;  
          return 1;             
       } 
      
    if (content->first_sign == 2)
    {
       if (content->wait_for_vd == 0) {
        clean_dragon(processor, p_dragon);
        content->wait_for_vd = -1;
        return 1;
        } else if (content->wait_for_vd > 0){
        content->wait_for_vd--;
        return 1;
        }
       if ( content->af_result.dist_idx_src + 4 <= 3000) //update
       { 
          content->af_result.dist_idx_src += 4;
          p_lens_ctrl->zoom_update = 1;
          p_lens_ctrl->zoom_pulse = 4;
       }
       else
       {
          content->first_sign = 3;
          content->af_fv2_top_pre = content->af_fv2_top;
          content->wait_for_vd = 4;  
       }
    }
    
    if (content->first_sign == 3)
    {  
       if (content->wait_for_vd == 0) {
        clean_dragon(processor, p_dragon);
        content->wait_for_vd = -1;
        return 1;
        } else if (content->wait_for_vd > 0){
        content->wait_for_vd--;
        return 1;
        }
        
       p_dragon->fv_rev_TH = content->fv2_data_max * 0.3f; //update  
       fv_dir_chg = content->fv2_data_max - content->fv2_data;
       if (fv_dir_chg <= p_dragon->fv_rev_TH) 
       { 
          content->af_result.dist_idx_src -= 4;
          p_lens_ctrl->zoom_update = 1;
          p_lens_ctrl->zoom_pulse = -4;
       }
       else
       {
          content->first_sign = 4;
          content->wait_for_vd = 4; 
       }
    }  
 
    if (content->first_sign == 4)
    {
       if (content->wait_for_vd == 0) {
        clean_dragon(processor, p_dragon);
        content->wait_for_vd = -1;
        return 1;
        } else if (content->wait_for_vd > 0){
        content->wait_for_vd--;
        return 1;
        }
       p_dragon->fv_rev_TH = content->fv2_data_max * 0.2f; //update  
       fv_dir_chg = content->fv2_data_max - content->fv2_data;
        //printf("fv2_data_max is%llu, fv2_data is %llu\n", content->fv2_data_max, content->fv2_data);
       if (fv_dir_chg <= p_dragon->fv_rev_TH) 
       { 
          content->af_result.dist_idx_src += 4;
          p_lens_ctrl->zoom_update = 1;
          p_lens_ctrl->zoom_pulse = 4;
       }
       else
       {
          content->first_sign = 5;
          zoom_length = 3000 - (content->af_fv2_top - content->af_fv2_top_pre);//update
          printf("zoom_length is %d\n", zoom_length);
       }
    }    
   }
   return 1;
}
static int af_calib_of_focus_length_control(Isp3AProcessor *processor, af_control_t * p_af_control,
    af_dragon_t * p_dragon, lens_control_t * p_lens_ctrl, u8 lens_runing_flag)
{
   
    Isp3AContentAF *content;
    u64 fv_dir_chg;
    s32 focus_length;
    content = &(processor->content.AFData);
    //printf("first_sign is %d, dist_idx_src is %d, wait_for_vd is %d\n",
    //  content->first_sign, content->af_result.dist_idx_src, content->wait_for_vd);
    if (lens_runing_flag == 0) {
       if (content->first_sign == 1) {          
          p_lens_ctrl->zoom_update = 1;
          p_lens_ctrl->zoom_pulse = 1000;//update big
          p_lens_ctrl->focus_pulse_pre = 0;
          p_lens_ctrl->zoom_pulse_pre = 0;
          content->first_sign = 2;  
          content->af_result.dist_idx_src = 0; 
          content->af_result.dist_idx_dest = 0; 
          content->wait_for_vd = 4;  
          return 1;             
       } 
      
    if (content->first_sign == 2)
    {
       if (content->wait_for_vd == 0) {
        clean_dragon(processor, p_dragon);
        content->wait_for_vd = -1;
        return 1;
        } else if (content->wait_for_vd > 0){
        content->wait_for_vd--;
        return 1;
        }
       if ( content->af_result.dist_idx_src + 4 <= 3000) //update
       { 
          content->af_result.dist_idx_src += 4;
          p_lens_ctrl->focus_update = 1;
          p_lens_ctrl->focus_pulse = 4;
       }
       else
       {
          content->first_sign = 3;
          content->af_fv2_top_pre = content->af_fv2_top;
          content->wait_for_vd = 4;  
       }
    }
    
    if (content->first_sign == 3)
    {  
       if (content->wait_for_vd == 0) {
        clean_dragon(processor, p_dragon);
        content->wait_for_vd = -1;
        return 1;
        } else if (content->wait_for_vd > 0){
        content->wait_for_vd--;
        return 1;
        }
        
       p_dragon->fv_rev_TH = content->fv2_data_max * 0.6f; //update  
       fv_dir_chg = content->fv2_data_max - content->fv2_data;
       if (fv_dir_chg <= p_dragon->fv_rev_TH) 
       { 
          content->af_result.dist_idx_src -= 4;
          p_lens_ctrl->focus_update = 1;
          p_lens_ctrl->focus_pulse = -4;
       }
       else
       {
          content->first_sign = 4;
          content->wait_for_vd = 4; 
       }
    }  
 
    if (content->first_sign == 4)
    {
       if (content->wait_for_vd == 0) {
        clean_dragon(processor, p_dragon);
        content->wait_for_vd = -1;
        return 1;
        } else if (content->wait_for_vd > 0){
        content->wait_for_vd--;
        return 1;
        }
       p_dragon->fv_rev_TH = content->fv2_data_max * 0.3f; //update  
       fv_dir_chg = content->fv2_data_max - content->fv2_data;
        //printf("fv2_data_max is%llu, fv2_data is %llu\n", content->fv2_data_max, content->fv2_data);
       if (fv_dir_chg <= p_dragon->fv_rev_TH) 
       { 
          content->af_result.dist_idx_src += 4;
          p_lens_ctrl->focus_update = 1;
          p_lens_ctrl->focus_pulse = 4;
       }
       else
       {
          content->first_sign = 5;
          focus_length = 3000 - (content->af_fv2_top - content->af_fv2_top_pre);//update
          printf("focus_length is %d\n", focus_length);
       }
    }   
   
   }
   return 1;
}


#if 0
static int af_calib_of_saf_control(Isp3AProcessor *processor, af_control_t * p_af_control,
    af_dragon_t * p_dragon, lens_control_t * p_lens_ctrl, u8 lens_runing_flag)
{
   
    Isp3AContentAF *content;

    content = &(processor->content.AFData);
    
    if (lens_runing_flag == 0) {
        if (content->first_sign == 1) {
                      
           content->af_result.zoom_idx_src =  0;
           content->af_result.zoom_idx_dest =  content->af_G_param.start_zoom_idx;
           af_decide_pulse(processor, &(content->af_result), p_lens_ctrl);
           content->first_sign = 2;
                          
       }
        if (content->first_sign == 2) {
           p_af_control->zoom_idx = content->af_G_param.start_zoom_idx;
           af_rt_inf(processor, p_af_control, p_lens_ctrl);                
           content->first_sign = 0;
           return 0;
       }
        
       if(p_af_control->zoom_idx <= content->af_G_param.end_zoom_idx)
       {
            content->af_result.zoom_idx_dest = (p_af_control->zoom_idx)++;
            af_decide_pulse(processor, &(content->af_result), p_lens_ctrl);
            content->af_ctrl_param.workingMode = SAF;  
            content->af_control_param.af_next_stm = INIT_SAF;
            content->done_saf_set_mode = CALIB_SAF;    
       }
       /*if(p_af_control->zoom_idx >= content->af_G_param.begin_zoom_idx)
       {
            content->af_result.zoom_idx_dest = (p_af_control->zoom_idx)--;
            af_decide_pulse(processor, &(content->af_result), p_lens_ctrl);
            content->af_ctrl_param.workingMode = SAF;  
            content->af_control_param.af_next_stm = INIT_SAF;
            content->done_saf_set_mode = CALIB_SAF;    
       }*/
       else
       {    content->af_ctrl_param.workingMode = SAF;
            content->done_saf_set_mode = SAF;    
       }

    } 
 
    return 0;
}
#endif

static int af_debug_control(Isp3AProcessor *processor, af_control_t * p_af_control, af_dragon_t * p_dragon,
    lens_control_t * p_af_ctrl, u8 lens_runing_flag)
{
//    static u8 first_sign = 1;
    s32 temp;
    //s32 top;
    //ImageOperate *op;
    Isp3AContentAF *content;

    //op = &(processor->operate);
    content = &(processor->content.AFData);

    if (!lens_runing_flag) {
        if (content->skip_frame > 0) {
            content->skip_frame--;
            return 0;
        }
        content->pulse_f = get_pulse(processor, p_af_control->zoom_idx, content->f_fbd);
        content->pulse_n = get_pulse(processor, p_af_control->zoom_idx, content->f_nbd);

        if (content->pulse_f > content->pulse_n) {
            temp = content->pulse_n;
            content->pulse_n = content->pulse_f;
            content->pulse_f = temp;
        }
        if ((content->pulse_curr < content->pulse_f) || (content->pulse_curr > content->pulse_n)) {
            content->direction *= -1;
        }
        content->af_result.dist_idx_src =
            get_dist_idx(processor, p_af_control->zoom_idx, content->pulse_curr);
        content->pulse_curr += content->direction;
        content->af_result.dist_idx_dest =
            get_dist_idx(processor, p_af_control->zoom_idx, content->pulse_curr);
        #if 0
        top = get_pulse(processor, p_af_control->zoom_idx, (u32) af_fv2_top);
        printf("dist_idx_src is %d, dist_idx_src is %d\n",af_result.dist_idx_src,af_result.dist_idx_dest);
        #endif
        p_af_ctrl->focus_update = 1;
        p_af_ctrl->focus_pulse = content->direction;
        content->skip_frame = 1;
        //printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>pulse_curr is %d, af_fv2_top is %d\n",(pulse_curr),top);
    } else {

    }
    return 0;
}

static int init_calib(Isp3AProcessor *processor, af_dragon_t * p_dragon, lens_control_t * p_af_ctrl,
    af_calib_t * p_af_calib_info)
{
    s32 pulse_curr;
    s32 pulse_f, pulse_n, min_pulse, max_pulse;
    s32 pulse_src, pulse_dest, pulse_delta;
    u8 zoom_idx;
    //ImageOperate *op;
    Isp3AContentAF *content;

    //op = &(processor->operate);
    content = &(processor->content.AFData);
    printf("enter init_calib");
    clean_dragon(processor, p_dragon);
    zoom_idx = p_af_calib_info->zoom_idx;
    pulse_src = content->af_zoom_param[zoom_idx].zoom_pluse;
    pulse_dest = content->af_zoom_param[1].zoom_pluse;    // >_< todo
    pulse_delta = pulse_dest - pulse_src;
    p_af_calib_info->zoom_idx = 1;      // >_< will be updated
    //pulse_f = 500;
    //pulse_n = 700;
    
    pulse_f =
        get_pulse(processor, p_af_calib_info->zoom_idx,
        (p_af_calib_info->dist_idx[content->af_cali_sign] - 400));
    pulse_n =
        get_pulse(processor, p_af_calib_info->zoom_idx,
        (p_af_calib_info->dist_idx[content->af_cali_sign] + 400));
    
    if (pulse_f > pulse_n) {
        min_pulse = pulse_n;
        max_pulse = pulse_f;
    } else {
        min_pulse = pulse_f;
        max_pulse = pulse_n;
    }
    pulse_curr = get_pulse(processor, p_af_calib_info->zoom_idx, p_af_calib_info->dist_idx[content->af_cali_sign]); // return to the first calib distance index
    //printf("min_pulse is %d,max_pulse is %d\n",min_pulse,max_pulse);
    content->af_cali_sign++;
    p_af_ctrl->focus_update = 1;
    p_af_ctrl->focus_pulse = pulse_curr;
    p_af_ctrl->zoom_pulse = pulse_delta;
    p_af_ctrl->zoom_update = 1;

    p_af_calib_info->pulse_curr = pulse_curr;
    p_af_calib_info->min_pulse = min_pulse;
    p_af_calib_info->max_pulse = max_pulse;

    content->af_control_param.af_next_stm = DO_CALIB;
    content->af_fv2_top = 0;
    p_af_calib_info->first_do = 1;

    //printf("af_fv2_top is %d in init_calib\n");
    //printf("focus_pulse is %d in init_calib\n", p_af_ctrl->focus_pulse);

    return 0;
}

static int do_calib(Isp3AProcessor *processor, af_calib_t * p_af_calib_info, af_dragon_t * p_dragon,
    lens_control_t * p_af_ctrl)
{
    #if 1
    s32 pulse_curr = 0;
    s32 top;
    u32 pre_fv, fv;
    s32 pulse_src, pulse_dest, pulse_delta;
    s32 pulse_f, pulse_n, min_pulse, max_pulse;
    //ImageOperate *op;
    Isp3AContentAF *content;
    //printf("enter do_calib, skip_frame_for_do_calib is %dn");
    //op = &(processor->operate);
    content = &(processor->content.AFData);

    if (content->skip_frame_for_do_calib > 0) {
       content->skip_frame_for_do_calib--;
        return 0;
    }
    //printf("zoom_idx is %d\n", p_af_calib_info->zoom_idx);
    pulse_curr = p_af_calib_info->pulse_curr;
    min_pulse = p_af_calib_info->min_pulse;
    max_pulse = p_af_calib_info->max_pulse;

    if (pulse_curr < min_pulse) {
        content->min_flag_for_do_calib += 1;
        content->direction_pre_for_do_calib = content->direction_for_do_calib;
        content->direction_for_do_calib *= -1;
        top = get_pulse(processor, p_af_calib_info->zoom_idx, (u32) content->af_fv2_top);
        p_af_calib_info->top_all[(content->min_flag_for_do_calib + content->max_flag_for_do_calib)-1][p_af_calib_info->zoom_idx] = get_pulse(processor, p_af_calib_info->zoom_idx, content->af_fv2_top_direct);
        //printf("af_fv2_top_direct is %d\n", content->af_fv2_top_direct);
        //p_af_calib_info->top_value[(content->min_flag_for_do_calib + content->max_flag_for_do_calib)][p_af_calib_info->zoom_idx] = top;
        content->fv2_data_max_direct = INIT_MAX_VALUE;
    }
    if (pulse_curr > max_pulse) {
        content->max_flag_for_do_calib += 1;
        content->direction_pre_for_do_calib = content->direction_for_do_calib;
        content->direction_for_do_calib *= -1;
        top = get_pulse(processor, p_af_calib_info->zoom_idx, (u32) content->af_fv2_top);
        p_af_calib_info->top_all[(content->min_flag_for_do_calib + content->max_flag_for_do_calib)-1][p_af_calib_info->zoom_idx] = get_pulse(processor, p_af_calib_info->zoom_idx, content->af_fv2_top_direct);
       // printf("af_fv2_top_directis %d\n", content->af_fv2_top_direct);
        content->fv2_data_max_direct = INIT_MAX_VALUE;
    }

    if(p_af_calib_info->first_do == 1)
    {
        clean_dragon(processor, p_dragon);
        p_af_calib_info->first_do = 0;
        content->fv2_data_max_direct = INIT_MAX_VALUE;

        return 0;
    }

    if ((content->min_flag_for_do_calib > 1) && (content->max_flag_for_do_calib > 1)) {
//        if(down_cnt<3){
        p_af_calib_info->first_do = 1;
        if (0) {
            content->af_control_param.af_next_stm = FAIL_CALIB;
            return 0;
        } else {
            if (p_af_calib_info->zoom_idx == 15) {      // >_< will be updated
                content->af_control_param.af_next_stm = AFTER_CALIB;
                return 0;
            }
            content->af_control_param.af_next_stm = DO_CALIB;
            pulse_src = content->af_zoom_param[p_af_calib_info->zoom_idx].zoom_pluse;
            pulse_dest = content->af_zoom_param[p_af_calib_info->zoom_idx + 1].zoom_pluse;        //coming to the next zoom position
            p_af_calib_info->zoom_idx++;
            pulse_delta = pulse_dest - pulse_src;
            p_af_ctrl->zoom_pulse = pulse_delta;
            p_af_ctrl->zoom_update = 1;

            pulse_src = pulse_curr;
            pulse_dest = get_pulse(processor, p_af_calib_info->zoom_idx, p_af_calib_info->dist_idx[content->af_cali_sign]);        // focus to the next zoom position
            pulse_delta = pulse_dest - pulse_src;
            p_af_ctrl->focus_pulse = pulse_delta;
            p_af_ctrl->focus_update = 1;
            pulse_f =
                get_pulse(processor, p_af_calib_info->zoom_idx,
                (p_af_calib_info->dist_idx[content->af_cali_sign] - 400));
            pulse_n =
                get_pulse(processor, p_af_calib_info->zoom_idx,
                (p_af_calib_info->dist_idx[content->af_cali_sign] + 400));
            if (pulse_f > pulse_n) {
                min_pulse = pulse_n;
                max_pulse = pulse_f;
            } else {
                min_pulse = pulse_f;
                max_pulse = pulse_n;
            }
            p_af_calib_info->pulse_curr = pulse_dest;
            p_af_calib_info->min_pulse = min_pulse;
            p_af_calib_info->max_pulse = max_pulse;

            content->direction_for_do_calib = 1;
            content->direction_pre_for_do_calib = 1;
            content->skip_frame_for_do_calib = 0;
            content->min_flag_for_do_calib = 0;
            content->max_flag_for_do_calib = 0;
            content->down_cnt_for_do_calib = 0;
            content->af_fv2_top = 0;
			af_count = 0;

            return 0;
        }
    }

    fv = content->af_drg_params.fv_store[content->af_drg_params.fv_store_idx].fv2_data;
    pre_fv = content->af_drg_params.fv_store[content->af_drg_params.fv_store_idx - 1].fv2_data;
    if (content->direction_pre_for_do_calib != content->direction_for_do_calib) {
        if (fv < pre_fv) {
            content->down_cnt_for_do_calib++;
//            printf("down_cnt is %d\n",down_cnt);
        } else {
            content->down_cnt_for_do_calib = 0;
        }
    } else {
        content->down_cnt_for_do_calib = 0;
        content->direction_pre_for_do_calib = content->direction_for_do_calib;
    }
    
    content->af_result.dist_idx_src = get_dist_idx(processor, p_af_calib_info->zoom_idx, pulse_curr);
    pulse_curr += content->direction_for_do_calib*4;//test
    content->af_result.dist_idx_dest =
        get_dist_idx(processor, p_af_calib_info->zoom_idx, pulse_curr);
    //printf("dist_idx_dest is %d, pulse_curr is %d\n", content->af_result.dist_idx_dest, pulse_curr);
    top = get_pulse(processor, p_af_calib_info->zoom_idx, (u32) content->af_fv2_top);
    //printf("af_fv2_top is %d!\n", content->af_fv2_top);
    p_af_ctrl->focus_update = 1;
    p_af_ctrl->focus_pulse = content->direction_for_do_calib*4;//test
    content->skip_frame_for_do_calib = 0;

    p_af_calib_info->pulse_curr = pulse_curr;
    p_af_calib_info->top[content->af_cali_sign][p_af_calib_info->zoom_idx] = top;

    //printf(">>>>>>>>>>>>>>>>>>>>pulse_curr is %d, af_fv2_top pulse is %d,  af_fv2_top is %d\n",(pulse_curr),top, content->af_fv2_top);
    printf("zoom idx is %d, pulse_curr is %d \n", p_af_calib_info->zoom_idx, pulse_curr);
    return 0;
    #else
    return 0;
    #endif
    
}

static int after_calib(Isp3AProcessor *processor, af_calib_t * p_calib_result)
{
#if 1
    static u8 first_sign = 1;   //>_< will be removed!
    u8 i = 0;
    ImageOperate *op;
    Isp3AContentAF *content;
    FILE * fp,*fp2;
    char fn[300];
    sprintf(fn, "calib_result_index_15_3000.txt");
    fp = fopen(fn, "wt");
	fp2 = fopen("fv2_data.txt","wt");
    if(fp == NULL)
    {
       
       printf("open file fail\n");
       //return 0;

    }
    
    op = &(processor->operate);
    content = &(processor->content.AFData);
    if (first_sign) {
        printf("calibration successful ^_^\n");
        for (i = 0; i < 17; i++) {      // >_< will be updated
            printf("top[%d] is %d\n",i,p_calib_result->top[content->af_cali_sign][i]);
            printf("top_all[%d] is %d  %d  %d  %d \n",p_calib_result->top_all[0][i],p_calib_result->top_all[1][i],
            p_calib_result->top_all[2][i],p_calib_result->top_all[3][i]);
            //fprintf(fp, "%d ", p_calib_result->top[content->af_cali_sign][i]);
        }
        for (i = 0; i < 17; i++) {      // >_< will be updated
            //printf("top[%d] is %d\n",content->af_cali_sign,p_calib_result->top[content->af_cali_sign][i]);
            fprintf(fp, "top[%d] is %d   \n", i,p_calib_result->top[content->af_cali_sign][i]);
            fprintf(fp,"top_all[%d] is %d  %d  %d  %d \n",i,p_calib_result->top_all[0][i],p_calib_result->top_all[1][i],
            p_calib_result->top_all[2][i],p_calib_result->top_all[3][i]);
        }
		for (i = 0; i < af_count; i++) {      // >_< will be updated
            //printf("top[%d] is %d\n",content->af_cali_sign,p_calib_result->top[content->af_cali_sign][i]);
            fprintf(fp2, " %d\n", content->fv2_data_all[i]);
            
            
        }
        first_sign = 0;
    }
    fclose(fp);
	fclose(fp2);
    content->af_control_param.af_next_stm = FAIL_CALIB;
#endif
    return 0;
}

static int fail_calib(Isp3AProcessor *processor, af_calib_t * p_calib_result)
{
    //printf("calibration failed o_o\n");
    return 0;
}

static int af_calib_control(Isp3AProcessor *processor, af_control_t * mode, af_dragon_t * p_dragon,
    lens_control_t * p_af_ctrl, u8 lens_runing_flag)
{
    //ImageOperate *op;
    Isp3AContentAF *content;

    //op = &(processor->operate);
    content = &(processor->content.AFData);
    //printf("enter af_calib,lens_runing_flag is %d\n", lens_runing_flag);
    if (lens_runing_flag) {

    } else {
        //printf("af_control_param af_next_stm is %d", content->af_control_param.af_next_stm);
        switch (content->af_control_param.af_next_stm) {
            case INIT_CALIB:
                init_calib(processor, p_dragon, p_af_ctrl, &(content->cali_out));
                break;
            case DO_CALIB:
                do_calib(processor, &(content->cali_out), p_dragon, p_af_ctrl);
                break;
            case AFTER_CALIB:
                after_calib(processor, &(content->cali_out));
                break;
            case FAIL_CALIB:
                fail_calib(processor, &(content->cali_out));
            default:
                return -1;
        }
    }
    return 0;
}



static void clean_dragon(Isp3AProcessor *processor, af_dragon_t * p_dragon)
{
    //ImageOperate *op;
    Isp3AContentAF *content;

    //op = &(processor->operate);
    content = &(processor->content.AFData);

    for (p_dragon->fv_store_idx = 0;
        p_dragon->fv_store_idx < p_dragon->store_len; p_dragon->fv_store_idx++)
        p_dragon->fv_store[p_dragon->fv_store_idx].occ_flag = 0;
    p_dragon->fv_store_idx = -1;
    p_dragon->store_cnt = 0;
    content->fv2_data_max = INIT_MAX_VALUE;
    content->fv2_data_min = INIT_MIN_VALUE;
}

static int param_adj(Isp3AProcessor *processor, af_algo_param_t * p_af_algo_param, af_dragon_t * p_dragon,
    const af_param_t * gk_af_param)
{
//    u8 zoom_idx_dest;
//    zoom_idx_dest = af_result.zoom_idx_dest;
    //ImageOperate *op;
    Isp3AContentAF *content;

    //op = &(processor->operate);
    content = &(processor->content.AFData);

    content->defalut_nd_val = content->f_nbd;//(content->f_nbd * 4 + content->f_fbd) / 5;
    content->defalut_fd_val = content->f_fbd;//(content->f_nbd + content->f_fbd * 4) / 5;
    p_dragon->f_dof = gk_af_param->DOF_param;
    p_dragon->store_delay = 2;  // 1 >_<
    return 0;
}

/*
* convert dist_idx to lens-dependent pulse
*/
static inline s32 get_pulse(Isp3AProcessor *processor, const u8 zoom_idx, const s32 dist_idx)
{
    s64 pulse = 0;
    s32 result;
    s32 b, c;
    //ImageOperate *op;
    Isp3AContentAF *content;

    //op = &(processor->operate);
    content = &(processor->content.AFData);

//    s32 a,b,c;
//    a = af_zoom_param[zoom_idx].a;
    b = content->af_zoom_param[zoom_idx].b;
    c = content->af_zoom_param[zoom_idx].c;
//    pulse =  ((s64)(a * (s64)dist_idx*(s64)dist_idx)>>20);
    pulse += ((s64) (b * (s64) dist_idx) >> 15);
    //pulse += ((s64) (b * (s64) dist_idx));
    //pulse += (c >> 2);
    pulse += c;

    result = (s32) pulse;
//    printf("dist_idx is %d, zoom_idx is %d, b is %d, c is %d, result is %d, \n",dist_idx,zoom_idx,b,c, result);
    return result;
}

/*
* convert lens-dependent pulse to dist_dix
*/
static inline s32 get_dist_idx(Isp3AProcessor *processor, const u32 zoom_idx, const s32 pulse)
{
    s64 result;

//    s32 a,b,c;
    s32 b, c;
    //ImageOperate *op;
    Isp3AContentAF *content;

    //op = &(processor->operate);
    content = &(processor->content.AFData);


//    a = af_zoom_param[zoom_idx].a;
    b = content->af_zoom_param[zoom_idx].b;
    c = content->af_zoom_param[zoom_idx].c;
    //result = ((s64) (pulse * 4 - c)) << 13;
    result = ((s64) (pulse  - c))<<15;
	
	if(b == 0)
		return (s32)result;
	
    result = result / b;
    return (s32) result;
}
static inline s32 get_dist_index(s32 b, s32 pulse)
{

    s64 result;
    result = ((s64) (pulse))<<15; 

    if(b == 0)
      return  result;    
    result = result / b;
    return (s32) result;
}

static inline s32 af_interpolate(s32 lv, s32 hv, u8 li, u8 hi, float look)
{
    float rval;

    rval = ((hi - look) * lv + (look - li) * hv) / (hi - li);
    return (s32) rval;
}

static int af_decide_pulse(Isp3AProcessor *processor, af_result_t * p_af_result,
    lens_control_t * p_af_ctrl)
{
    s32 pulse_src_l, pulse_src_h, pulse_dest_l, pulse_dest_h;
    s32 pulse_src, pulse_dest;
    u8 zoom_idx_src, zoom_idx_dest;
    s32 pulse_delta = -1;
    //ImageOperate *op;
    Isp3AContentAF *content;

    //op = &(processor->operate);
    content = &(processor->content.AFData);

    /*
     * if index exceed table, error
     */
    if ((p_af_result->zoom_idx_dest < 0)
        || (p_af_result->zoom_idx_dest > (ZOOM_STEP - 1))) {
        //printf("zoom_move error!\n");
//        assert(0);
        return -1;
    }
    zoom_idx_src = p_af_result->zoom_idx_src;
    zoom_idx_dest = p_af_result->zoom_idx_dest;
    //printf("zoom_idx_src is %f, zoom_idx_src is %d, zoom_idx_dest is %f, zoom_idx_dest is %d\n",p_af_result->zoom_idx_src,zoom_idx_src,p_af_result->zoom_idx_dest,zoom_idx_dest);
    pulse_src_l = get_pulse(processor, zoom_idx_src, p_af_result->dist_idx_src);
    pulse_dest_l = get_pulse(processor, zoom_idx_dest, p_af_result->dist_idx_dest);
    pulse_src_h = get_pulse(processor, (zoom_idx_src + 1), p_af_result->dist_idx_src);
    pulse_dest_h = get_pulse(processor, (zoom_idx_dest + 1), p_af_result->dist_idx_dest);
    pulse_src =
        af_interpolate(pulse_src_l, pulse_src_h, zoom_idx_src,
        (zoom_idx_src + 1), p_af_result->zoom_idx_src);
    pulse_dest =
        af_interpolate(pulse_dest_l, pulse_dest_h, zoom_idx_dest,
        (zoom_idx_dest + 1), p_af_result->zoom_idx_dest);
    pulse_delta = pulse_dest - pulse_src;
   // printf("af_decide_pulse - dist_idx_src is %d,dist_idx_dest is %d\n",p_af_result->dist_idx_src, p_af_result->dist_idx_dest);
    //printf("af_decide_pulse - pulse_src_l is %d,pulse_dest_l is %d\n", pulse_src_l, pulse_dest_l);
    //printf("af_decide_pulse - pulse_src_h is %d,pulse_dest_h is %d\n", pulse_src_h, pulse_dest_h);
    //printf("af_decide_pulse - pulse_src is %d,pulse_dest is %d\n", pulse_src, pulse_dest);


    if (pulse_delta != 0) {
       /*if(pulse_delta  + p_af_ctrl->focus_pulse_pre >= 4 
            || pulse_delta  + p_af_ctrl->focus_pulse_pre <= -4)
        {
            p_af_ctrl->focus_pulse = (pulse_delta + p_af_ctrl->focus_pulse_pre) / 4 * 4;
            p_af_ctrl->focus_pulse_pre =  (pulse_delta + p_af_ctrl->focus_pulse_pre) % 4;
            p_af_ctrl->focus_update = 1;
           
        }
        else
        {
            p_af_ctrl->focus_pulse_pre += pulse_delta;
            p_af_ctrl->focus_pulse = 0;
            p_af_ctrl->focus_update = 0;
        }
        af_result_printf
                (">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>src is %d, dest is %d, delta is %d\n",
                p_af_result->dist_idx_src, p_af_result->dist_idx_dest,
                p_af_ctrl->focus_pulse);*/
        
        p_af_ctrl->focus_pulse = pulse_delta;
        p_af_ctrl->focus_update = 1;
        /*af_result_printf
                (">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>focus src is %d, dest is %d, delta is %d\n",
                p_af_result->dist_idx_src, p_af_result->dist_idx_dest,
                p_af_ctrl->focus_pulse);*/
           
    } else {
        p_af_ctrl->focus_pulse = 0;
        p_af_ctrl->focus_update = 0;
    }
    pulse_src_l = content->af_zoom_param[zoom_idx_src].zoom_pluse;
    pulse_dest_l = content->af_zoom_param[zoom_idx_dest].zoom_pluse;
    pulse_src_h = content->af_zoom_param[(zoom_idx_src + 1)].zoom_pluse;
    pulse_dest_h = content->af_zoom_param[(zoom_idx_dest + 1)].zoom_pluse;
    pulse_src =
        af_interpolate(pulse_src_l, pulse_src_h, zoom_idx_src,
        (zoom_idx_src + 1), p_af_result->zoom_idx_src);
    pulse_dest =
        af_interpolate(pulse_dest_l, pulse_dest_h, zoom_idx_dest,
        (zoom_idx_dest + 1), p_af_result->zoom_idx_dest);
    pulse_delta = pulse_dest - pulse_src;
    //printf("index %f, pulse_src_l %d, pulse_src_h %d,pulse_src %d, pulse_dest %d,pulse_dest_l %d, pulse_dest_h %d\n",p_af_result->zoom_idx_dest,pulse_src_l,pulse_src_h,pulse_src,pulse_dest,pulse_dest_l,pulse_dest_h);
    if (pulse_delta != 0) {
        /* if(pulse_delta  + p_af_ctrl->zoom_pulse_pre >= 4 
            || pulse_delta  + p_af_ctrl->zoom_pulse_pre <= -4)
        {
            p_af_ctrl->zoom_pulse = (pulse_delta + p_af_ctrl->zoom_pulse_pre) / 4 * 4;
            p_af_ctrl->zoom_pulse_pre =  (pulse_delta + p_af_ctrl->zoom_pulse_pre) % 4;
            p_af_ctrl->zoom_update = 1;
           
        }
        else
        {
            p_af_ctrl->zoom_pulse_pre += pulse_delta;
            p_af_ctrl->zoom_pulse = 0;
            p_af_ctrl->zoom_update = 0;
        }*/
        p_af_ctrl->zoom_pulse = pulse_delta; 
        p_af_ctrl->zoom_update = 1;
        /*af_result_printf
                (">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>zoom src is %f, dest is %f, delta is %d\n",
                p_af_result->zoom_idx_src, p_af_result->zoom_idx_dest,
                p_af_ctrl->zoom_pulse);*/
       
    } else {
        p_af_ctrl->zoom_pulse = 0;
        p_af_ctrl->zoom_update = 0;
    }
    p_af_result->zoom_idx_src = p_af_result->zoom_idx_dest;
    p_af_result->dist_idx_src = p_af_result->dist_idx_dest;

    p_af_ctrl->pps = p_af_result->pps;
//    assert(p_af_ctrl->pps);
    if (!p_af_ctrl->pps) {
        return -1;
    }
    return 0;
}

/*
return to infinate far
*/
static int af_rt_inf(Isp3AProcessor *processor, af_control_t * p_af_control, lens_control_t * p_lens_ctrl)
{
    p_lens_ctrl->focus_update = 1;
    p_lens_ctrl->focus_pulse = get_pulse(processor, p_af_control->zoom_idx, 0);
    p_lens_ctrl->focus_pulse_pre = 0;
    p_lens_ctrl->zoom_pulse_pre = 0;

    return 0;
}

static void af_mode_switch(Isp3AProcessor *processor, af_control_t * p_af_control)
{
    af_mode next_mode;
    //ImageOperate *op;
    Isp3AContentAF *content;

    //op = &(processor->operate);
    content = &(processor->content.AFData);

    next_mode = p_af_control->workingMode;
    if (next_mode == content->pre_mode) {
        //NOP
    } else {
        content->af_drg_params.store_len = 48;
        switch (next_mode) {
            case CAF:
                content->af_control_param.af_stm = INIT_FineCAF;
                content->af_control_param.af_next_stm = INIT_FineCAF;
                content->af_control_param.direction = 1;
                break;
            case SAF:
                content->af_control_param.af_stm = INIT_SAF;
                content->af_control_param.af_next_stm = INIT_SAF;
                content->af_control_param.direction = 1;
                break;
            case MANUAL:
                content->af_control_param.af_stm = 0;
                content->af_control_param.af_next_stm = 0;
                content->af_control_param.direction = 1;
                break;
            case CALIB:
                content->af_control_param.af_stm = INIT_CALIB;
                content->af_control_param.af_next_stm = INIT_CALIB;
                content->af_control_param.direction = 1;
                break;
            case DEBUGS:
                content->af_control_param.af_stm = INIT_CAF;
                content->af_control_param.af_next_stm = INIT_CAF;
                content->af_control_param.direction = 1;
                break;
            case CALIB_SAF:
                //content->af_control_param.af_stm = WAIT_VD_CALIB_SAF;
                //content->af_control_param.af_next_stm = WAIT_VD_CALIB_SAF;
                //content->wait_for_vd = 4;
                //content->af_drg_params.store_af_stm = INIT_CALIB_SAF;
                //content->af_control_param.direction = 1;
                break;
            default:
                return;
        }
    }
    content->pre_mode = next_mode;
};
