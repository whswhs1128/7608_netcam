/*!
*****************************************************************************
** \file        linux/kernel/3.4.43/sound/soc/codecs/gk_audc.h
**
** \version     $Id: gk_audc.h 13619 2018-04-28 07:28:10Z yulindeng $
**
** \brief
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2016 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#ifndef _GK_AUDC_H_
#define _GK_AUDC_H_

//*****************************************************************************
//*****************************************************************************
//** Defines and Macros
//*****************************************************************************
//*****************************************************************************

#define DroopCompensationFilter_Size     6
#define ImageSuppressionFilter_Size      9
#define PeriodicFilter_Size             21
#define DCRemovalFilter_Size            20
#define WindNoiseStep                   30
#define WindNoiseFilter_Size            20
#define DeEmphasisFilter_Size            6
#define AUD_AO_GAIN_NUM                 0x1E
#define AUD_AI_GAIN_NUM                 0x10
#define ADC_CIC_MULT_LEVEL              MAX_AIN_VOLUME_LEVLE
#define ADC_CIC_MULT_STEP               0x2000  //(0x80000/ADC_CIC_MULT_LEVEL)

#define GK_AUDIO_DAC_LINE_OUT           AOUT_DAC_LINE_OUT
#define GK_AUDIO_DAC_HP_OUT             AOUT_DAC_HP_OUT
#define GK_AUDIO_DAC_SP_OUT             AOUT_DAC_SP_OUT
#define GK_AUDIO_ADC_LINE_IN            AIN_ADC_LINE_IN
#define GK_AUDIO_ADC_MIC_IN             AIN_ADC_MIC_IN
/* ---------------------------------------------------------------------- */

/********/
/* AUC */
/********/
#define gk_audio_write(p, v)        gka_writel(p, v)
#define gk_audio_read(p)            gka_readl(p)

#define AUC_ENABLE_REG_VAL          0x00000000    //Default Audio Codec Enable Reg Value
#define AUC_DP_RESET_REG_VAL        0x00000000    //Default Audio Codec Digital Path Reset Reg Value
#define AUC_OSR_REG_VAL             0x0000001F    //Default Over Sample Rate Reg Value
#define AUC_CONTROL_REG_VAL         0x00000080    //Default Audio Codec Control Bits Reg Value
#define AUC_STATUS_REG_VAL          0x00000000    //Default Audio Codec Status Value
#define AUC_CIC_REG_VAL             0x00080000    //Default CIC Multiplicand Data Reg Value
#define AUC_I2S_REG_VAL             0x00020000    //Default I2S Multiplicand Data Reg Value
//#define AUC_CLOCK_REG_VAL           0x00000000    //Default Clock Latch Reg Value
#define AUC_DSM_GAIN_REG_VAL        0x00000000    //Default DSM Gain Reg Value
#define AUC_EMA_REG_VAL             0x00000000    //Default Audio Codec EMA Reg Value
//#define AUC_BG_CONTROL_REG_VAL      0x00000000    //Default BG Control Reg Value
#define AUC_ADC_CONTROL_REG_VAL     0x0000001A    //Default ADC Control Reg Value
#define AUC_ADC_VGA_GAIN_REG_VAL    0x00000000    //Default ADC VGA Gain Reg Value
#define AUC_DAC_CONTROL_REG_VAL     0x0000043F    //Default DAC Control Reg Value
#define AUC_DAC_GAIN_REG_VAL        0x0000000E    //Default DAC Gain Control Reg Value

#define AUC_NORMAL_STATE            0x00000000    //AUC in normal state
#define AUC_IDLE_STATE              0x00000001    //AUC in idle state
#define AUC_RESET_STATE             0xffffffff    //AUC in reset state

#if 0
#define AUC_ADC_CONTROL_REG_OPT     0x08000000
#define AUC_ADC_VGA_GAIN_REG_OPT    0x0000000C
#else
#define AUC_ADC_CONTROL_REG_OPT     0x140C0000

#if (defined(CONFIG_BSP_HD747) || defined(CONFIG_BSP_HD787))
#define AUC_ADC_VGA_GAIN_REG_OPT    0x00000019
#else
#define AUC_ADC_VGA_GAIN_REG_OPT    0x0000000E
#endif

#endif
/* ---------------------------------------------------------------------- */

#define AUC_ADC_ENABLE              1
#define AUC_DAC_ENABLE              (1<<1)

#define AUC_DP_RESET_ADC            1
#define AUC_DP_RESET_DAC            (1<<1)

#define AUC_OSR_512x                0x3f
#define AUC_OSR_384x                0x2f
#define AUC_OSR_256x                0x1f
#define AUC_OSR_192x                0x17
#define AUC_OSR_128x                0x0f

#define AUC_CONTROL_LOOPBACK_ON     (1<<8)
#define AUC_CONTROL_DITHER_GAIN8    (0<<6)
#define AUC_CONTROL_DITHER_GAIN4    (1<<6)
#define AUC_CONTROL_DITHER_GAIN2    (2<<6)
#define AUC_CONTROL_DITHER_GAIN1    (3<<6)
#define AUC_CONTROL_DITHER_ON       (1<<5)
#define AUC_CONTROL_DWA_ON          (1<<4)
#define AUC_CONTROL_DEEMPHASIS_ON   (1<<3)
#define AUC_CONTROL_PRD_ADC         0
#define AUC_CONTROL_IIR_ADC         1
#define AUC_CONTROL_IMG_ADC         2
#define AUC_CONTROL_IIR_DAC         3
#define AUC_CONTROL_PRD_DAC         4

#define AUC_CIC_GAIN_2_1            0xFFFFF
#define AUC_CIC_GAIN_1_1            0x80000
#define AUC_CIC_GAIN_1_2            0x40000
#define AUC_CIC_GAIN_1_4            0x20000
#define AUC_CIC_GAIN_1_8            0x10000

#define AUC_I2S_GAIN_4_1            0x80000
#define AUC_I2S_GAIN_2_1            0x40000
#define AUC_I2S_GAIN_1_1            0x20000
#define AUC_I2S_GAIN_1_2            0x10000
#define AUC_I2S_GAIN_0_1            0x00000

//#define AUC_DSM_GAIN_02LEVEL        (0<<3)
//#define AUC_DSM_GAIN_15LEVEL        (1<<3)

#define AUC_ADC_CONTROL_ADC_BYPASS_VGA  (1<<6)
#define AUC_ADC_CONTROL_ADC_LOWP1_ON    (1<<5)
#define AUC_ADC_CONTROL_ADC_LOWP0_ON    (1<<4)
#define AUC_ADC_CONTROL_ADC_DEMEN       (1<<3)
#define AUC_ADC_CONTROL_ADC_MUTE        (1<<2)
#define AUC_ADC_CONTROL_ADC_EN_MA1      (1<<1)
#define AUC_ADC_CONTROL_ADC_PD          (1<<0)

//#define AUC_DAC_CONTROL_PGA_PD          (1<<13)
#define AUC_DAC_CONTROL_LPF_ON          (1<<10)
#define AUC_DAC_CONTROL_1B_SEL          (1<<9)
//#define AUC_DAC_CONTROL_SP_MUTE         (1<<8)
//#define AUC_DAC_CONTROL_HP_MUTE         (1<<7)
#define AUC_DAC_CONTROL_DAC_PD          (1<<6)
#define AUC_DAC_CONTROL_RF_PD           (1<<5)    //RF: Reconstruct Filter
#define AUC_DAC_CONTROL_SP_PD           (1<<4)
#define AUC_DAC_CONTROL_LINE_PD         (1<<3)
#define AUC_DAC_CONTROL_LINE_BIAS_PD    (1<<2)
#define AUC_DAC_CONTROL_HP_PD           (1<<1)
#define AUC_DAC_CONTROL_BIAS_PD         (1<<0)
#define AUC_DAC_CONTROL_DAC_PD_ALL      (AUC_DAC_CONTROL_DAC_PD|AUC_DAC_CONTROL_RF_PD|AUC_DAC_CONTROL_LINE_BIAS_PD|AUC_DAC_CONTROL_BIAS_PD)

//#define AUC_DAC_GAIN_LINE_0             (0<<16)
//#define AUC_DAC_GAIN_LINE_1             (1<<16)
//#define AUC_DAC_GAIN_LINE_2             (2<<16)
//#define AUC_DAC_GAIN_LINE_3             (3<<16)
//#define AUC_DAC_GAIN_LINE_4             (4<<16)
//#define AUC_DAC_GAIN_LINE_5             (5<<16)
//#define AUC_DAC_GAIN_LINE_6             (6<<16)

#define AUC_DAC_GAIN_PGA1_0         0x00
#define AUC_DAC_GAIN_PGA1_1         0x10

#define AUC_DAC_GAIN_PGA2_0         0x0
#define AUC_DAC_GAIN_PGA2_1         0x1
#define AUC_DAC_GAIN_PGA2_2         0x2
#define AUC_DAC_GAIN_PGA2_3         0x3
#define AUC_DAC_GAIN_PGA2_4         0x4
#define AUC_DAC_GAIN_PGA2_5         0x5
#define AUC_DAC_GAIN_PGA2_6         0x6
#define AUC_DAC_GAIN_PGA2_7         0x7
#define AUC_DAC_GAIN_PGA2_8         0x8
#define AUC_DAC_GAIN_PGA2_9         0x9
#define AUC_DAC_GAIN_PGA2_A         0xA
#define AUC_DAC_GAIN_PGA2_B         0xB
#define AUC_DAC_GAIN_PGA2_C         0xC
#define AUC_DAC_GAIN_PGA2_D         0xD
#define AUC_DAC_GAIN_PGA2_E         0xE
#define AUC_DAC_GAIN_PGA2_F         0xF

#define DAC_PGA_GAIN_6db             0
#define DAC_PGA_GAIN_0db             6
#define DAC_PGA_GAIN_Mute           90

#define GK_AUDIO_SYSCLK_MCLK         0
#define GK_AUDIO_CLKDIV_LRCLK        0



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
typedef struct gk_audioctrl_reg
{
    u32 reg_00;
    u32 reg_01;
    u32 reg_02;
    u32 reg_03;
    u32 reg_04;
    u32 reg_05;
    u32 reg_06;
    u32 reg_07;
    u32 reg_08;
    u32 reg_09;
    u32 reg_0a;
    u32 reg_0b;
    u32 reg_0c;
    u32 reg_0d;
    u32 reg_0e;
    u32 reg_0f;
    u32 reg_10;
} gk_audioctrl_reg_t;



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

int __init gk_audio_init(void);
void __exit gk_audio_exit(void);


#ifdef __cplusplus
}
#endif



#endif /* _GK_AUDC_H_ */


