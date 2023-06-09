/*!
*****************************************************************************
** \file        src/algo/af_params/ov5653_sy6310_af_param.c
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

#include "basetypes.h"
#include "gk_isp.h"
#include "gk_isp3a_api.h"
#include "isp3a_lens.h"
#include "isp3a_af.h"

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
af_param_t ov5653_SY6310_param = {
    /*init pps */
    300,
    /*zoom_thread_sleep_time*/ 
    1000,
    /*init zoom index */
    0,
    /*start_zoom_idx*/
    2,
	 /*begin_zoom_idx*/
    2,
    /*end_zoom_idx*/
    16,
	/*zoom_reverse_idx*/
    2,
	/*z_inbd*/
    500,
    /*z_outbd*/
    242,
    /*zoom_reverse_after_z*/
    60,
    /*zoom_reverse_after_f*/
    70,
    /*f_nbd*/
    4000,
    /*f_fbd*/
    -4000,
	/*calib_saf_step*/
    0,
    /*calib_saf_b*/
    0,
    /*zoom_enhance_enable*/
    0,
    /*dist_idx_init*/
    35,
    /*zoom_dir,in 1, out 2*/
    1,
    /*zoom_break_idx*/
    13,
    /*manual_focus_step*/
    160,
    /*manual_zoom_step*/
    0.05,
    /*spacial_fir */
    {{0, -1, 0,
                    -1, 4, -1,
            0, -1, 0}
        }
    ,
    /*Depth of field parameter */
    128,
    //pps max, fv3 enable, wait interval, ev_idx_TH, CStep, FStep, CThrh, FThrh
    {
            {1500, 0, 31, 5, 50, 20, 5, 3}
            ,                   // zp0
            {1500, 0, 32, 5, 50, 20, 5, 3}
            ,                   // zp1
            {1500, 0, 32, 5, 50, 20, 5, 3}
            ,                   // zp2
            {1500, 0, 32, 5, 50, 20, 5, 3}
            ,                   // zp3
            {1500, 0, 32, 5, 50, 20, 5, 3}
            ,                   // zp4
            {1500, 0, 32, 5, 50, 20, 5, 3}
            ,                   // zp5
            {1500, 0, 32, 5, 50, 20, 5, 3}
            ,                   // zp6
            {1500, 0, 32, 5, 50, 20, 5, 3}
            ,                   // zp7
            {1500, 0, 64, 9, 15, 8, 15, 5}
            ,                   // zp8
            {1500, 0, 64, 9, 15, 8, 15, 5}
            ,                   // zp9
            {1500, 0, 64, 9, 14, 7, 20, 8}
            ,                   // zp10
            {1500, 0, 64, 9, 14, 7, 30, 8}
            ,                   // zp11
            {1500, 0, 64, 13, 12, 6, 30, 15}
            ,                   // zp12
            {1500, 0, 64, 13, 12, 6, 30, 15}
            ,                   // zp13
            {1500, 0, 64, 13, 12, 6, 30, 15}
            ,                   // zp14
            {1500, 0, 128, 13, 8, 4, 50, 50}
            ,                   // zp15
            {1500, 0, 128, 13, 8, 4, 70, 50}
            ,                   // zp16
            {1500, 0, 128, 13, 8, 4, 70, 50}
            ,                   // zp17
            {1500, 0, 128, 13, 8, 4, 70, 50}
            ,                   // zp18
            {1500, 0, 128, 13, 8, 4, 70, 50}
        }                       // zp19
    ,
	/*dist index reverse err min*/
    0, 
    /*dist index reverse err max 45/28*1000+4(统计信息延迟帧数)*4(step)*2(不同方向)/28*1000*/
    2800,
	 /*zoom_reverse_err_min*/
    0,
	/*zoom_reverse_err_max*/
    50,
    /*saf_dist_index_reverse_err_max*/
    1800,
    /*saf_f_nbd*/
    12000,
    /*saf_f_fbd*/
    -12000,
	/*saf_optical_pos_offset*/
    6,
	/*optical_focus_length*/
    2000,
    /*zoom_slope*/
	0.1315384615,
	/*zoom_offset*/
	0.7369230770
};

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

//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************

//*****************************************************************************
//*****************************************************************************
//** Local Functions
//*****************************************************************************
//*****************************************************************************
