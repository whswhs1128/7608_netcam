/*!
*****************************************************************************
** \file        software/linux/kernel/drv/sensors/sc1135/sc1135_pri.h
**
** \version     $Id: sc1135_pri.h 117 2018-02-26 08:16:32Z fengxuequan $
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
#ifndef _SC1135_PRI_H_
#define _SC1135_PRI_H_



//*****************************************************************************
//*****************************************************************************
//** Defines and Macros
//*****************************************************************************
//*****************************************************************************
/*         register name            address default value    R/W    description  */
#define SC1135_SLEEP_MODE           0x3000  /*  0x02 RW System Control
                                                Bit [0]: manual stream enable
                                                Sleep = ~Bit[0] or PWDN */
#define SC1135_RST_PON              0x3003  /*  0x00
                                                Bit[0] rst soft 1:reset*/
#define SC1135_SCLK_DIV             0x3004  /*  0x04 */

#define SC1135_I2C_ADDR             0x3008  /*  0x60
                                                Bit[7:1] I2C slave id
                                                Bit[0] 0:write 1:read*/
#define SC1135_PLL_PRE_CLK_DIV      0x3010  /*  0x20 PLL control register
                                                Bit[7]: BYPASS PLL
                                                Bit[6:4]: SYSEL[2:0]=S
                                                Bit[3:1]: PreDIV[2:0] 000 1x, 001 1.5x, 010 2x, 011 3x, ..., 111 7x
                                                Bit[0]: PLLDIV[5]*/
#define SC1135_PLL_SYS_CLK_DIV      0x3011  /*  0x86 PLL control register
                                                Bit[7:3]: PLLDIV[4:0] Multiplier=64-PLLdiv[5:0]
                                                Bit[2:0]: Reserved*/

#define SC1135_3115                 0x3115  /*  0x0A*/

#define SC1135_TIMING_Y_START_H     0x3202  /*  0x00*/
#define SC1135_TIMING_Y_START_L     0x3203  /*  0x78*/
#define SC1135_TIMING_Y_END_H       0x3206  /*  0x03*/
#define SC1135_TIMING_Y_END_L       0x3207  /*  0x67*/

#define SC1135_TIMING_X_SIZE_H      0x3208  /*  0x05*/
#define SC1135_TIMING_X_SIZE_L      0x3209  /*  0x00*/
#define SC1135_TIMING_Y_SIZE_H      0x320A  /*  0x05*/
#define SC1135_TIMING_Y_SIZE_L      0x320B  /*  0x00*/
#define SC1135_TIMING_HTS_H         0x320C  /*  0x07*/
#define SC1135_TIMING_HTS_L         0x320D  /*  0x08*/
#define SC1135_TIMING_VTS_H         0x320E  /*  0x03*/
#define SC1135_TIMING_VTS_L         0x320F  /*  0x20*/
#define SC1135_TIMING_HOFFS_H       0x3210  /*  0x00*/
#define SC1135_TIMING_HOFFS_L       0x3211  /*  0x62*/
#define SC1135_TIMING_VOFFS_H       0x3212  /*  0x00*/
#define SC1135_TIMING_VOFFS_L       0x3213  /*  0x0A*/

#define SC1135_3216                 0x3216  /*  0x0A*/
#define SC1135_FLIP                 0x321C  /*  0x40 flip ctrl
                                                Bit[6]: 1:flip on*/
#define SC1135_MIRROR               0x321D  /*  mirror ctrl
                                                Bit[0]: 1:mirror on*/
#define SC1135_321E                 0x321E  /*  0x00 */
#define SC1135_321F                 0x321F  /*  0x0A */
#define SC1135_VSYNC_MODE           0x3223  /*  0x22 vysnc mode[5] */

#define SC1135_3300                 0x3300  /*  0x22 */
#define SC1135_3301                 0x3301  /*  0x22 */
#define SC1135_3302                 0x3302  /*  0x30 */
#define SC1135_3303                 0x3303  /*  0x66 */
#define SC1135_3304                 0x3304  /*  0xA0 */
#define SC1135_3305                 0x3305  /*  0x72 */
#define SC1135_3308                 0x3308  /*  0x40 */
#define SC1135_3315                 0x3315  /*  0x44 */
#define SC1135_331E                 0x331E  /*  0x56 */
#define SC1135_3320                 0x3320  /*  0x06 */
#define SC1135_3321                 0x3321  /*  0xE8 */
#define SC1135_3322                 0x3322  /*  0x01 */
#define SC1135_3323                 0x3323  /*  0xC0 */
#define SC1135_3330                 0x3330  /*  0x0D */
#define SC1135_3332                 0x3332  /*  0x38 */

#define SC1135_3336                 0x3336  /*  0x38 */
#define SC1135_3337                 0x3337  /*  0x38 */
#define SC1135_3338                 0x3338  /*  0x38 */
#define SC1135_3339                 0x3339  /*  0x38 */



#define SC1135_RNC_ENABLE           0x3400  /*  0x52 rnc enable
                                                Bit[0]: 0:bypass blc, 1: BLC enable
                                                Bit[1]: 0:manual mode, 1:auto mode
                                                Bit[5]: channel select 0: 4 channel, 1: use one channel mode*/
#define SC1135_RNC_MANUAL00_L       0x3404  /*  Row noise for B channel[7:0]*/
#define SC1135_RNC_MANUAL00_H       0x3405  /*  Bit[4:0] Row noise for B channel[12:8]*/
#define SC1135_RNC_MANUAL01_L       0x3406  /*  Row noise for GB channel[7:0]*/
#define SC1135_RNC_MANUAL01_H       0x3407  /*  Bit[4:0] Row noise for GB channel[12:8]*/
#define SC1135_RNC_MANUAL10_L       0x340C  /*  Row noise for GR channel[7:0]*/
#define SC1135_RNC_MANUAL10_H       0x340D  /*  Bit[4:0] Row noise for GR channel[12:8]*/
#define SC1135_RNC_MANUAL11_L       0x340E  /*  Row noise for R channel[7:0]*/
#define SC1135_RNC_MANUAL11_H       0x340F  /*  Bit[4:0] Row noise for R channel[12:8]*/
#define SC1135_RNC_TARGET_H         0x3415  /*  0x00 Bit[7:4] target[11:8]*/
#define SC1135_RNC_TARGET_L         0x3416  /*  0xC0 target[7:0] high aligned*/



#define SC1135_3500                 0x3500  /*  0x10 */

#define SC1135_WPT                  0x350F
#define SC1135_BPT                  0x3510

#define SC1135_3600                 0x3600  /*  0x7C */
#define SC1135_3601                 0x3601  /*  0x18 */

#define SC1135_3603                 0x3603  /*  0x18 */

#define SC1135_3610                 0x3610  /*  0x03 */
#define SC1135_3612                 0x3612  /*  0x00 */
#define SC1135_3620                 0x3620  /*  0x44 */
#define SC1135_CHIP_ID              0x3621  /*  0x04 */
#define SC1135_3622                 0x3622  /*  0x2E */
#define SC1135_3630                 0x3630  /*  0x58 */
#define SC1135_3631                 0x3631  /*  0x80 */
#define SC1135_3632                 0x3632  /*  0x41 */
#define SC1135_3633                 0x3633  /*  0x7C */
#define SC1135_3635                 0x3635  /*  0x04 */
#define SC1135_3636                 0x3636  /*  0x10 */
#define SC1135_3640                 0x3640  /*  0x10 */

#define SC1135_BLC_SYNC00           0x3780  /*  0x10 [5:0] ASP delay cycles*/
#define SC1135_BLC_SYNC01           0x3781  /*  0x10
                                                Bit[7] incremental pattern enable
                                                Bit[4] r_raw1_swap_en
                                                Bit[3] r_raw0_swap_en
                                                Bit[2] rblue reverse
                                                Bit[1] adc_g channel first enable
                                                Bit[0] adclk_inv*/
#define SC1135_TEST_INCREMENTAL     0x3781  /*  0x90 incremental pattern
                                                Bit[1]: 0: normal image, 1: incremental pattern*/

#define SC1135_BLC_ENABLE           0x3900  /*  0x04 blc enable Bit[0] 0:bypass blc, 1: BLC enable*/
#define SC1135_BLC_AUTO_ENABLE      0x3902  /*  blc auto enable Bit[6] 0:manual mode 1:auto mode*/
#define SC1135_BLC_CHANNEL_SELECT1  0x3905  /*  BLC channel select Bit[6] 0:use 8 or 4 channel offset, 1: use one channel mode*/
#define SC1135_BLC_TARGET_H         0x3907  /*  0x00 Bit[4:0] BLC target[12:8]*/
#define SC1135_BLC_TARGET_L         0x3908  /*  0xc0 BLC target[7:0]*/
#define SC1135_BLC_MANUAL00_H       0x3909  /*  Bit[4:0] BLC offset for B0 channel[12:8]*/
#define SC1135_BLC_MANUAL00_L       0x390A  /*  BLC offset for B0 channel[7:0]*/
#define SC1135_BLC_MANUAL01_H       0x390B  /*  Bit[4:0] BLC offset for GB0 channel[12:8]*/
#define SC1135_BLC_MANUAL01_L       0x390C  /*  BLC offset for GB0 channel[7:0]*/
#define SC1135_BLC_MANUAL02_H       0x390D  /*  Bit[4:0] BLC offset for B1 channel[12:8]*/
#define SC1135_BLC_MANUAL02_L       0x390E  /*  BLC offset for B1 channel[7:0]*/
#define SC1135_BLC_MANUAL03_H       0x390F  /*  Bit[4:0] BLC offset for BG1 channel[12:8]*/
#define SC1135_BLC_MANUAL03_L       0x3910  /*  BLC offset for BG1 channel[7:0]*/
#define SC1135_3911                 0x3911  /*  high temperature logic*/
#define SC1135_BLC_MANUAL10_H       0x3920  /*  Bit[4:0] BLC offset for GR0 channel[12:8]*/
#define SC1135_BLC_MANUAL10_L       0x3921  /*  BLC offset for GR0 channel[7:0]*/
#define SC1135_BLC_MANUAL11_H       0x3922  /*  Bit[4:0] BLC offset for R0 channel[12:8]*/
#define SC1135_BLC_MANUAL11_L       0x3923  /*  BLC offset for R0 channel[7:0]*/
#define SC1135_BLC_MANUAL12_H       0x3924  /*  Bit[4:0] BLC offset for GR1 channel[12:8]*/
#define SC1135_BLC_MANUAL12_L       0x3925  /*  BLC offset for GR1 channel[7:0]*/
#define SC1135_BLC_MANUAL13_H       0x3926  /*  Bit[4:0] BLC offset for R1 channel[12:8]*/
#define SC1135_BLC_MANUAL13_L       0x3927  /*  BLC offset for R1 channel[7:0]*/
#define SC1135_BLC_CHANNEL_SELECT2  0x3928  /*  0x00 BLC channel select Bit[0] 0:use 8 channel offset mode, 1:use 4 channel offset mode*/

#define SC1135_3D08                 0x3D08  /*  0x00 */

#define SC1135_AEC_EXP_TIME_H       0x3E01  /*  AEC[15:8]*/
#define SC1135_AEC_EXP_TIME_L       0x3E02  /*  AEC[7:0]*/
#define SC1135_AEC_ENABLE           0x3E03  /*  0x00 Bit[0]: AEC manual 0:auto enable 1:manual enable
                                                Bit[1] :AGC manual 0:auto enable 1:manual enable*/
#define SC1135_AEC_DGAIN           0x3E08  /*  Bit[0]*/
#define SC1135_AEC_AGAIN           0x3E09  /*  Bit[7] 0x3E08[0]0x3E09[7] 01,10:2x dig gain, 11: 4x dig gain*/
#define SC1135_AEC_3E0A             0x3e0a  /**/
#define SC1135_AEC_3E0E             0x3E0E  /*  0x50 AEC use extern isp*/
#define SC1135_AEC_3E0F             0x3E0F  /*  0x14 colse dgain*/

#define SC1135_ISP_CTRL             0x5000  /*  0x99
                                                Bit[7]: LENC correction enable 0: Disable, 1: enable
                                                Bit[4]: var_en
                                                Bit[3] Bit[1]: awb_gain_en
                                                Bit[0]: auto white balance enable 0:disable, 1:enable*/
#define SC1135_TEST_COLOR_BAR       0x503D  /*  0x80 Color bar test pattern
                                                Bit[7]: 0: disable 1: enable*/
#define SC1135_5054                 0x5054  /*  0x82 */

#define SC1135_AWB_CTRL00           0x5180  /*  0x04 Bit[7:0]:stable_range*/
#define SC1135_AWB_CTRL01           0x5181  /*  0x08 Bit[7:6]:stable_range*/
#define SC1135_AWB_CTRL02           0x5182  /*  0x0 Bit[0]: gain_man_en*/

#define SC1135_AVG_XSTART_H         0x5680  /*  0x00 Horizontal start position for average window
                                                Bit[7:4]:not used
                                                Bit[3:0]:r_avg x start[11:8]*/
#define SC1135_AVG_XSTART_L         0x5681  /*  0x00 Horizontal start position for average window
                                                Bit[7:0]:r_avg x start[7:0]*/
#define SC1135_AVG_YSTART_H         0x5682  /*  0x00 Vertical start position for average window
                                                Bit[7:4]:not used
                                                Bit[3:0]:r_avg y start[11:8]*/
#define SC1135_AVG_YSTART_L         0x5683  /*  0x00 Vertical start position for average window
                                                Bit[7:0]:r_avg y start[7:0]*/
#define SC1135_AVG_WIN_WIDTH_H      0x5684  /*  0x10 the width for average window
                                                Bit[7:4]:not used
                                                Bit[3:0]:r_win_windth[11:8]*/
#define SC1135_AVG_WIN_WIDTH_L      0x5685  /*  0xa0 the width for average window
                                                Bit[7:0]:r_win_windth[7:0]*/
#define SC1135_AVG_WIN_HEIGTH_H     0x5686  /*  0x0c the height for average window
                                                Bit[7:4]:not used
                                                Bit[3:0]:r_win_height[11:8]*/
#define SC1135_AVG_WIN_HEIGTH_L     0x5687  /*  0xa0 the height for average window
                                                Bit[7:0]:r_win_height[7:0]*/
#define SC1135_AVG_CTRL08           0x5688  /*  0x2
                                                Bit[1]; avg_opt
                                                Bit[0]: win_man*/
#define SC1135_AVG_AVERAGE          0x568A  /*  RO*/


#define SC1135_YAVG                 0x568A
#define SC1135_YAVG_WINDOW_ENABLE   0x5708  /* Bit[0]: win_man_en 0:auto enable 1:manual enable*/
#define SC1135_AVG_X_START_H        0x5700  /* Bit[3:0] avg_x_start[11:8]*/
#define SC1135_AVG_X_START_L        0x5701  /* avg_x_start[7:0]*/
#define SC1135_AVG_Y_START_H        0x5702  /* Bit[3:0] avg_y_start[11:8]*/
#define SC1135_AVG_Y_START_L        0x5703  /* avg_y_start[7:0]*/
#define SC1135_AVG_X_WIDTH_H        0x5704  /* Bit[3:0] avg_x_width[11:8]*/
#define SC1135_AVG_X_WIDTH_L        0x5705  /* avg_x_width[7:0]*/
#define SC1135_AVG_Y_HEIGTH_H       0x5706  /* Bit[3:0] avg_y_heigth[11:8]*/
#define SC1135_AVG_Y_HEIGTH_L       0x5707  /* avg_y_heigth[7:0]*/


#define SC1135_LENC_CTRL00          0x5800  /*  0x90
                                                Bit[6]:no_delay 1:sel data_i do lenc, 0:sel data_3d do lenc
                                                Bit[5]:debug mode
                                                Bit[4]:lenc_bias_en 1:sub bias before do lenc, 0: use data_i do lenc
                                                Bit[1]:sel_deltagain
                                                Bit[0]: sel_ra ,if lenc enable select test mode*/
#define SC1135_LENC_RADIRUS         0x5801  /*  0x20 Bit[7:0]: lc_radius*/
#define SC1135_LENC_XOFFSET         0x5802  /*  0x20 Bit[7:0]: the horizontal start size*/
#define SC1135_LENC_YOFFSET         0x5803  /*  0x10 Bit[7:0]: the vertical start size*/
#define SC1135_LENC_RGAIN           0x5804  /*  0x80 Bit[7:0]: red color coefficient*/
#define SC1135_LENC_GGAIN           0x5805  /*  0x80 Bit[7:0]: green color coefficient*/
#define SC1135_LENC_BGAIN           0x5806  /*  0x80 Bit[7:0]: blue color coefficient*/
#define SC1135_LENC_CTRL07          0x5807  /*  0x00
                                                Bit[7]: xy_offset_man_en
                                                Bit[5]: hskip_man_en
                                                Bit[4]: vskip_man_en
                                                Bit[3:2]: r_hskip
                                                Bit[1:0]: r_vskip*/
#define SC1135_LENC_XCNT_H          0x5808  /*  0x2
                                                Bit[7:2]: not used
                                                Bit[1:0]: xcnt[9:8] horizontal center*/
#define SC1135_LENC_XCNT_L          0x5809  /*  0x8A xcnt[7:0]*/
#define SC1135_LENC_YCNT_H          0x580A  /*  0x1
                                                Bit[7:2]: not used
                                                Bit[1:0]: ycnt[9:8] vertical center*/
#define SC1135_LENC_YCNT_L          0x580B  /*  0xF0 ycnt[7:0]*/

#define SC1135_VAR_CTRL00           0x5900  /*  0x01
                                                Bit[7]:b_avg_en
                                                Bit[6]:gb_avg_en
                                                Bit[5]:gr_avg_en
                                                Bit[4]:r_avg_en
                                                Bit[3]:debug_en
                                                Bit[2]:single_channel bypass
                                                Bit[1:0]: addopt*/
#define SC1135_VAR_CTRL01           0x5901  /*  0x00
                                                Bit[3:2]: hskip
                                                Bit[1:0]: vskip*/


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



#ifdef __cplusplus
}
#endif



#endif /* _SC1135_PRI_H_ */

