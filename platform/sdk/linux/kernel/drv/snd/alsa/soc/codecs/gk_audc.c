/*!
*****************************************************************************
** \file        linux/kernel/drv/webcam/src/sound/soc/codecs/gk_audc.c
**
** \version     $Id: gk_audc.c 13658 2018-05-14 03:07:22Z yulindeng $
**
** \brief
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2018 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/delay.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/initval.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>

#include <mach/hardware.h>
#include <mach/audio_codec.h>
#include <plat/audio.h>

#include "gk_audc.h"


gk_audioctrl_reg_t gk_audio_reg;

static u32 gk_audio_pga_gain_table[AUD_AO_GAIN_NUM] =
{
    /* -15*(-3)=45dB */
    (AUC_DAC_GAIN_PGA1_1|AUC_DAC_GAIN_PGA2_F),
    (AUC_DAC_GAIN_PGA1_1|AUC_DAC_GAIN_PGA2_E),
    (AUC_DAC_GAIN_PGA1_1|AUC_DAC_GAIN_PGA2_D),
    (AUC_DAC_GAIN_PGA1_1|AUC_DAC_GAIN_PGA2_C),
    (AUC_DAC_GAIN_PGA1_1|AUC_DAC_GAIN_PGA2_B),
    (AUC_DAC_GAIN_PGA1_1|AUC_DAC_GAIN_PGA2_A),
    (AUC_DAC_GAIN_PGA1_1|AUC_DAC_GAIN_PGA2_9),
    (AUC_DAC_GAIN_PGA1_1|AUC_DAC_GAIN_PGA2_8),
    (AUC_DAC_GAIN_PGA1_1|AUC_DAC_GAIN_PGA2_7),
    (AUC_DAC_GAIN_PGA1_1|AUC_DAC_GAIN_PGA2_6),
    (AUC_DAC_GAIN_PGA1_1|AUC_DAC_GAIN_PGA2_5),
    (AUC_DAC_GAIN_PGA1_1|AUC_DAC_GAIN_PGA2_4),
    (AUC_DAC_GAIN_PGA1_1|AUC_DAC_GAIN_PGA2_3),
    (AUC_DAC_GAIN_PGA1_1|AUC_DAC_GAIN_PGA2_2),
    /* -1*(-3)=3dB */
    (AUC_DAC_GAIN_PGA1_1|AUC_DAC_GAIN_PGA2_1),
    /* 0*(-3)=0dB */
    (AUC_DAC_GAIN_PGA1_0|AUC_DAC_GAIN_PGA2_0),
    (AUC_DAC_GAIN_PGA1_0|AUC_DAC_GAIN_PGA2_1),
    (AUC_DAC_GAIN_PGA1_0|AUC_DAC_GAIN_PGA2_2),
    (AUC_DAC_GAIN_PGA1_0|AUC_DAC_GAIN_PGA2_3),
    (AUC_DAC_GAIN_PGA1_0|AUC_DAC_GAIN_PGA2_4),
    (AUC_DAC_GAIN_PGA1_0|AUC_DAC_GAIN_PGA2_5),
    (AUC_DAC_GAIN_PGA1_0|AUC_DAC_GAIN_PGA2_6),
    (AUC_DAC_GAIN_PGA1_0|AUC_DAC_GAIN_PGA2_7),
    (AUC_DAC_GAIN_PGA1_0|AUC_DAC_GAIN_PGA2_8),
    (AUC_DAC_GAIN_PGA1_0|AUC_DAC_GAIN_PGA2_9),
    (AUC_DAC_GAIN_PGA1_0|AUC_DAC_GAIN_PGA2_A),
    (AUC_DAC_GAIN_PGA1_0|AUC_DAC_GAIN_PGA2_B),
    (AUC_DAC_GAIN_PGA1_0|AUC_DAC_GAIN_PGA2_C),
    (AUC_DAC_GAIN_PGA1_0|AUC_DAC_GAIN_PGA2_D),
    /* 14*(-3)=-42dB */
    (AUC_DAC_GAIN_PGA1_0|AUC_DAC_GAIN_PGA2_E),
    // ÉùÒô¾ÓÈ»±ÈAUC_DAC_GAIN_PGA2_EÐ¡
    /* 15*(-3)=-45dB */
    //(AUC_DAC_GAIN_PGA1_0|AUC_DAC_GAIN_PGA2_F),
};

static u32 gk_audio_ai_gain_table[AUD_AI_GAIN_NUM] =
{
    // 0,3,6,9,12,15,-3,-6dB
    /* 23+15=38DB */
    (AUC_DAC_GAIN_PGA1_1|AUC_DAC_GAIN_PGA2_5),
    /* 23+12=35DB */
    (AUC_DAC_GAIN_PGA1_1|AUC_DAC_GAIN_PGA2_4),
    /* 23+9=32DB */
    (AUC_DAC_GAIN_PGA1_1|AUC_DAC_GAIN_PGA2_3),
    /* 23+6=29DB */
    (AUC_DAC_GAIN_PGA1_1|AUC_DAC_GAIN_PGA2_2),
    /* 13+15=28DB */
    (AUC_DAC_GAIN_PGA1_0|AUC_DAC_GAIN_PGA2_5),
    /* 23+3=26DB */
    (AUC_DAC_GAIN_PGA1_1|AUC_DAC_GAIN_PGA2_1),
    /* 13+12=25DB */
    (AUC_DAC_GAIN_PGA1_0|AUC_DAC_GAIN_PGA2_4),
    /* 23+0=23DB */
    (AUC_DAC_GAIN_PGA1_1|AUC_DAC_GAIN_PGA2_0),
    /* 13+9=22DB */
    (AUC_DAC_GAIN_PGA1_0|AUC_DAC_GAIN_PGA2_3),
    /* 23-3=20DB */
    (AUC_DAC_GAIN_PGA1_1|AUC_DAC_GAIN_PGA2_6),
    /* 13+6=19DB */
    (AUC_DAC_GAIN_PGA1_0|AUC_DAC_GAIN_PGA2_2),
    /* 23-6=17DB */
    (AUC_DAC_GAIN_PGA1_1|AUC_DAC_GAIN_PGA2_7),
    /* 13+3=16DB */
    (AUC_DAC_GAIN_PGA1_0|AUC_DAC_GAIN_PGA2_1),
    /* 13+0=13DB */
    (AUC_DAC_GAIN_PGA1_0|AUC_DAC_GAIN_PGA2_0),
    /* 13-3=10DB */
    (AUC_DAC_GAIN_PGA1_0|AUC_DAC_GAIN_PGA2_6),
    /* 13-6=7DB */
    (AUC_DAC_GAIN_PGA1_0|AUC_DAC_GAIN_PGA2_7),
};

static void gk_audio_volume_control(u8 volume);
static void gk_audio_capture_control(u8 volume);

int gk_audio_snd_soc_info_volsw(struct snd_kcontrol *kcontrol,
    struct snd_ctl_elem_info *uinfo)
{
    uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
    uinfo->count = 1;
    uinfo->value.integer.min = 0;
    uinfo->value.integer.max = AUD_AO_GAIN_NUM - 0x01;
    return 0;
}

int gk_audio_snd_soc_put_volsw(struct snd_kcontrol *kcontrol,
    struct snd_ctl_elem_value *ucontrol)
{
    unsigned char val, change = 0;
    if (ucontrol->value.integer.value[0] > (AUD_AO_GAIN_NUM - 1))
        ucontrol->value.integer.value[0] = AUD_AO_GAIN_NUM - 1;

    val = AUD_AO_GAIN_NUM - 1 - ucontrol->value.integer.value[0];

    if (val >= AUD_AO_GAIN_NUM)
        val = AUD_AO_GAIN_NUM - 1;

    if(gk_audio_pga_gain_table[val] != (gk_audio_reg.reg_10 & 0x0000001F)){
        change = 1;
        gk_audio_volume_control(val);
    }

    return change;
}

int gk_audio_snd_soc_get_volsw(struct snd_kcontrol *kcontrol,
    struct snd_ctl_elem_value *ucontrol)
{
    int i;

    for(i = 0; i < AUD_AO_GAIN_NUM; i++){
        if((gk_audio_reg.reg_10 & 0x0000001F) == gk_audio_pga_gain_table[i])
            break;
    }

    ucontrol->value.integer.value[0] = AUD_AO_GAIN_NUM - 1 - i;

    return 0;
}

int gk_audio_snd_soc_info_capsw(struct snd_kcontrol *kcontrol,
    struct snd_ctl_elem_info *uinfo)
{
    uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
    uinfo->count = 1;
    uinfo->value.integer.min = 0;
    uinfo->value.integer.max = AUD_AI_GAIN_NUM - 0x01;
    return 0;
}

int gk_audio_snd_soc_put_capsw(struct snd_kcontrol *kcontrol,
    struct snd_ctl_elem_value *ucontrol)
{
    unsigned char val, change = 0;
    if (ucontrol->value.integer.value[0] > (AUD_AI_GAIN_NUM - 1))
        ucontrol->value.integer.value[0] = AUD_AI_GAIN_NUM - 1;

    val = AUD_AI_GAIN_NUM - 1 - ucontrol->value.integer.value[0];

    if (val >= AUD_AI_GAIN_NUM)
        val = AUD_AI_GAIN_NUM - 1;

    if(gk_audio_ai_gain_table[val] != (gk_audio_reg.reg_0e & 0x00000017)){
        change = 1;
        gk_audio_capture_control(val);
    }

    return change;
}

int gk_audio_snd_soc_get_capsw(struct snd_kcontrol *kcontrol,
    struct snd_ctl_elem_value *ucontrol)
{
    int i;

    for(i = 0; i < AUD_AI_GAIN_NUM; i++){
        if((gk_audio_reg.reg_0e & 0x00000017) == gk_audio_ai_gain_table[i])
            break;
    }

    ucontrol->value.integer.value[0] = AUD_AI_GAIN_NUM - 1 - i;

    return 0;
}

#define GK_AUC_SOC_SINGLE(xname, mod)            \
{                                           \
    .iface  = SNDRV_CTL_ELEM_IFACE_MIXER,   \
    .name   = xname,                        \
    .info   = gk_audio_snd_soc_info_##mod##sw,  \
    .get    = gk_audio_snd_soc_get_##mod##sw,   \
    .put    = gk_audio_snd_soc_put_##mod##sw    \
}

static const struct snd_kcontrol_new gk_audio_snd_controls[] =
{
    GK_AUC_SOC_SINGLE("Capture Volume", cap),
    //SOC_SINGLE("ADC Capture Switch", 0x0C34, 2, 1, 1),
    GK_AUC_SOC_SINGLE("Speaker Playback Volume", vol),
};

static const struct snd_soc_dapm_widget gk_audio_dapm_widgets[] =
{
    //SND_SOC_DAPM_DAC("DAC", "Playback", 0x0C3C, 6, 1),
    SND_SOC_DAPM_OUTPUT("LLOUT"),
    SND_SOC_DAPM_OUTPUT("RLOUT"),
    SND_SOC_DAPM_OUTPUT("LHPOUT"),
    SND_SOC_DAPM_OUTPUT("RHPOUT"),
    SND_SOC_DAPM_OUTPUT("SPOUT"),

    //SND_SOC_DAPM_ADC("ADC", "Capture", 0x0C34, 0, 1),
    SND_SOC_DAPM_INPUT("LLIN"),
    SND_SOC_DAPM_INPUT("RLIN"),
};

static const struct snd_soc_dapm_route intercon[] =
{
    /* outputs */
    {"LLOUT", NULL, "DAC"},
    {"RLOUT", NULL, "DAC"},
    {"LHPOUT", NULL, "DAC"},
    {"RHPOUT", NULL, "DAC"},
    {"SPOUT", NULL, "DAC"},

    /* inputs */
    {"ADC", NULL, "LLIN"},
    {"ADC", NULL, "RLIN"},
};

static int gk_audio_add_widgets(struct snd_soc_codec *codec)
{
    struct snd_soc_dapm_context *dapm = &codec->dapm;

    snd_soc_dapm_new_controls(dapm, gk_audio_dapm_widgets,
                  ARRAY_SIZE(gk_audio_dapm_widgets));
    /* set up audio path interconnects */
    snd_soc_dapm_add_routes(dapm, intercon, ARRAY_SIZE(intercon));

    return 0;
}
#if 0
static u32 gk_audio_droopcompensationfilter_table[DroopCompensationFilter_Size] =
{
    0x000FF48B, 0x00003491, 0x000FD8CB, 0x000F2780, 0x000232A9, 0x00054811
};

static u32 gk_audio_imagesuppressionfilter_table[ImageSuppressionFilter_Size] =
{
    0x00000225, 0x00000981, 0x00001B3B, 0x00003C53, 0x00006D24,
    0x0000A94E, 0x0000E4D1, 0x000110D5, 0x00012165
};

static u32 gk_audio_periodicfilter_table[PeriodicFilter_Size] =
{
    0x000FFFE4, 0x000FFE04, 0x000FFF85, 0x000004C5, 0x00000262,
    0x000FF5B8, 0x000FF9D3, 0x000012F6, 0x00000D8E, 0x000FDFDB,
    0x000FE59E, 0x00003310, 0x0000302B, 0x000FB207, 0x000FAA5B,
    0x00007501, 0x00009C96, 0x000F4E62, 0x000EB810, 0x00010D9E,
    0x00043FF0
};
#endif

#ifdef AUC_USE_DC_REMOVAL_FILTER
static u32 gk_audio_dcremovalfilter_table[DCRemovalFilter_Size] =
{
    /* Stage 1 */
    0x00000000, 0x00000040, 0x000007f1, 0x000003c0, 0x00000000,
    0x00000000, 0x03c0d13b, 0x000003c0, 0x00000000, 0x00000000,
    /* Stage 2 */
    0x00000000, 0x00000040, 0x0000142e, 0x00000380, 0x000007f0,
    0x00000040, 0x0346d3bc, 0x00000380, 0xfcb9798f, 0x0000003f
};
#endif
#if 0
static u32 gk_audio_windnoisefilter_table[WindNoiseFilter_Size] =
{
    /* Stage 1 */
    0x00000000, 0x00000040, 0x00000010, 0x000003c0, 0x00000000,
    0x00000000, 0x4ae797e1, 0x000003c0, 0x00000000, 0x00000000,
    /* Stage 2 */
    0x00000000, 0x00000040, 0x002beeea, 0x00000380, 0x0000000f,
    0x00000040, 0x41daabd4, 0x00000380, 0xbe9ddf98, 0x0000003f
};

static u32 gk_audio_deemphasisfilter_table[DeEmphasisFilter_Size] =
{
    /* Stage 1 */
    0x00000000, 0x00000040, 0xdd7e6059, 0x000003f1, 0x6ecab8f7, 0x000003d7
};

#endif

gk_audioctrl_reg_t gk_audio_reg =
{
    AUC_ENABLE_REG_VAL,
    AUC_DP_RESET_REG_VAL,
    AUC_OSR_REG_VAL,
    AUC_CONTROL_REG_VAL,
    AUC_STATUS_REG_VAL,
    AUC_CIC_REG_VAL,
    AUC_I2S_REG_VAL,
    0x00000000,    //Clock Latch, Unused
    AUC_DSM_GAIN_REG_VAL,
    AUC_EMA_REG_VAL,
    0x00000000,    //Unused
    0x00000000,    //Unused
    0x00000000,    //BG Control, Unused
    AUC_ADC_CONTROL_REG_VAL,
    AUC_ADC_VGA_GAIN_REG_VAL,
    AUC_DAC_CONTROL_REG_VAL,
    AUC_DAC_GAIN_REG_VAL
};

static u32 gk_audio_check_state(void)
{
    u32 retval;
    retval = gk_aud_readl(AUDC_DIGITAL_SYS_RST_CTRL0_REG);
    if(retval&(u32)0xE000)
        return (AUC_RESET_STATE);
    else
        return (AUC_NORMAL_STATE);
}

static void gk_audio_set_droopcompensationfilter(void)
{
    int i;
    if(gk_audio_check_state()!=AUC_RESET_STATE)
    {
        for(i=0 ; i<DroopCompensationFilter_Size ; i++)
        {
            //gk_audio_write(AUC_COEFF_START_DC_REG+(i<<2), gk_audio_droopcompensationfilter_table[i]);
            //gk_audio_read(AUC_COEFF_START_DC_REG+(i<<2));
        }
    }
}

static void gk_audio_set_imagesuppressionfilter(void)
{
    int i;
    if(gk_audio_check_state()!=AUC_RESET_STATE)
    {
        for(i=0 ; i<ImageSuppressionFilter_Size ; i++)
        {
            //gk_audio_write(AUC_COEFF_START_IS_REG+(i<<2), gk_audio_imagesuppressionfilter_table[i]);
            //gk_audio_read(AUC_COEFF_START_IS_REG+(i<<2));
        }
    }
}

void gk_audio_set_periodicfilter(void)
{
    int i;
    if(gk_audio_check_state()!=AUC_RESET_STATE)
    {
        for(i=0 ; i<PeriodicFilter_Size ; i++)
        {
            //gk_audio_write(AUC_COEFF_START_PD_REG+(i<<2), gk_audio_periodicfilter_table[i]);
            //gk_audio_read(AUC_COEFF_START_PD_REG+(i<<2));
        }
    }
}

#ifdef AUC_USE_DC_REMOVAL_FILTER
static void gk_audio_set_dcremovalfilter(void)
{
    int i;
    if (gk_audio_check_state() != AUC_RESET_STATE)
    {
        for(i=0 ; i<DCRemovalFilter_Size ; i++)
        {
            //gk_audio_write(AUC_COEFF_START_HP_REG+(i<<2), gk_audio_dcremovalfilter_table[i]);
            //gk_audio_read(AUC_COEFF_START_HP_REG+(i<<2));
        }
    }
}
#endif

static void gk_audio_set_windnoisefilter(void)
{
    int i;
    if(gk_audio_check_state()!=AUC_RESET_STATE)
    {
        for(i=0 ; i<WindNoiseFilter_Size ; i++)
        {
#if 1
            //gk_audio_write(AUC_COEFF_START_WN_REG+(i<<2), gk_audio_windnoisefilter_table[i]);
#else
            //gk_audio_write(AUC_COEFF_START_WN_REG+(i<<2), gk_audio_windnoisefilter_table[19][i]);
#endif
            //gk_audio_read(AUC_COEFF_START_WN_REG+(i<<2));
        }
    }
}

static void gk_audio_set_deemphasisfilter(void)
{
    int i;
    if(gk_audio_check_state()!=AUC_RESET_STATE)
    {
        for(i=0 ; i<DeEmphasisFilter_Size ; i++)
        {
            //gk_audio_write(AUC_COEFF_START_DE_REG+(i<<2), gk_audio_deemphasisfilter_table[i]);
            //gk_audio_read(AUC_COEFF_START_DE_REG+(i<<2));
        }
    }
}


static void gk_audio_init_fiter(void)
{
    gk_audio_set_droopcompensationfilter();
    gk_audio_set_imagesuppressionfilter();
    gk_audio_set_periodicfilter();
#ifdef AUC_USE_DC_REMOVAL_FILTER
    gk_audio_set_dcremovalfilter();
#else
    gk_audio_set_windnoisefilter();
#endif
    gk_audio_set_deemphasisfilter();
}

static void gk_audio_adc_on(void)
{
    //gk_audio_reg.reg_00 = gk_audio_read(AUC_ENABLE_REG);
    gk_audio_reg.reg_00 = gk_audio_reg.reg_00|AUC_ADC_ENABLE;
    if(gk_audio_check_state()!=AUC_RESET_STATE)
    {
        //gk_audio_write(AUC_ENABLE_REG, gk_audio_reg.reg_00);
        //gk_audio_read(AUC_ENABLE_REG);
    }
}

static void gk_audio_adc_off(void)
{
    //gk_audio_reg.reg_00 = gk_audio_read(AUC_ENABLE_REG);
    gk_audio_reg.reg_00 = gk_audio_reg.reg_00&(~((u32)AUC_ADC_ENABLE));
    if(gk_audio_check_state()!=AUC_RESET_STATE)
    {
        //gk_audio_write(AUC_ENABLE_REG, gk_audio_reg.reg_00);
        //gk_audio_read(AUC_ENABLE_REG);
    }
}

static void gk_audio_adc_power_on(void)
{
    //gk_audio_reg.reg_0d = (u32) gk_audio_read(AUC_ADC_CONTROL_REG);
    gk_audio_reg.reg_0d = gk_audio_reg.reg_0d & (~((u32)AUC_ADC_CONTROL_ADC_PD)) ;
    if(gk_audio_check_state()!=AUC_RESET_STATE)
    {
        //gk_audio_write(AUC_ADC_CONTROL_REG, gk_audio_reg.reg_0d);
        //gk_audio_read(AUC_ADC_CONTROL_REG);
    }
}

static void gk_audio_adc_power_down(void)
{
    //gk_audio_reg.reg_0d = (u32) gk_audio_read(AUC_ADC_CONTROL_REG);
    gk_audio_reg.reg_0d = gk_audio_reg.reg_0d |AUC_ADC_CONTROL_ADC_PD;
    if(gk_audio_check_state()!=AUC_RESET_STATE)
    {
        //gk_audio_write(AUC_ADC_CONTROL_REG, gk_audio_reg.reg_0d);
        //gk_audio_read(AUC_ADC_CONTROL_REG);
    }
}

static void gk_audio_adc_mute_on(void)
{
    //gk_audio_reg.reg_0d = (u32) gk_audio_read(AUC_ADC_CONTROL_REG);
    gk_audio_reg.reg_0d = gk_audio_reg.reg_0d |AUC_ADC_CONTROL_ADC_MUTE;
    if(gk_audio_check_state()!=AUC_RESET_STATE)
    {
        //gk_audio_write(AUC_ADC_CONTROL_REG, gk_audio_reg.reg_0d);
        //gk_audio_read(AUC_ADC_CONTROL_REG);
    }
}

static void gk_audio_adc_mute_off(void)
{
    //gk_audio_reg.reg_0d = (u32) gk_audio_read(AUC_ADC_CONTROL_REG);
    gk_audio_reg.reg_0d = gk_audio_reg.reg_0d & (~((u32)AUC_ADC_CONTROL_ADC_MUTE)) ;
    if(gk_audio_check_state()!=AUC_RESET_STATE)
    {
        //gk_audio_write(AUC_ADC_CONTROL_REG, gk_audio_reg.reg_0d);
        //gk_audio_read(AUC_ADC_CONTROL_REG);
    }
}

static void gk_audio_dac_on(void)
{
    //gk_audio_reg.reg_00 = gk_audio_read(AUC_ENABLE_REG);
    gk_audio_reg.reg_00 = gk_audio_reg.reg_00|AUC_DAC_ENABLE;
    if(gk_audio_check_state()!=AUC_RESET_STATE)
    {
        //gk_audio_write(AUC_ENABLE_REG, gk_audio_reg.reg_00);
        //gk_audio_read(AUC_ENABLE_REG);
    }
}

static void gk_audio_dac_off(void)
{
    //gk_audio_reg.reg_00 = gk_audio_read(AUC_ENABLE_REG);
    gk_audio_reg.reg_00 = gk_audio_reg.reg_00&(~((u32)AUC_DAC_ENABLE));
    if(gk_audio_check_state()!=AUC_RESET_STATE)
    {
        //gk_audio_write(AUC_ENABLE_REG, gk_audio_reg.reg_00);
        //gk_audio_read(AUC_ENABLE_REG);
    }
}

static void gk_audio_dac_power_on(void)
{
    //gk_audio_reg.reg_0f = (u32) gk_audio_read(AUC_DAC_CONTROL_REG);
    gk_audio_reg.reg_0f = gk_audio_reg.reg_0f & (~((u32)AUC_DAC_CONTROL_DAC_PD_ALL)) ;
    if(gk_audio_check_state()!=AUC_RESET_STATE)
    {
        //gk_audio_write(AUC_DAC_CONTROL_REG, gk_audio_reg.reg_0f);
        //gk_audio_read(AUC_DAC_CONTROL_REG);
    }
}

static void gk_audio_dac_power_down(void)
{
    //gk_audio_reg.reg_0f = (u32) gk_audio_read(AUC_DAC_CONTROL_REG);
    gk_audio_reg.reg_0f= gk_audio_reg.reg_0f | AUC_DAC_CONTROL_DAC_PD_ALL ;
    if(gk_audio_check_state()!=AUC_RESET_STATE)
    {
        //gk_audio_write(AUC_DAC_CONTROL_REG, gk_audio_reg.reg_0f);
        //gk_audio_read(AUC_DAC_CONTROL_REG);
    }
}

static void gk_audio_dac_mute_on(void)
{
    gk_audio_reg.reg_06 = AUC_I2S_GAIN_0_1;
    if(gk_audio_check_state()!=AUC_RESET_STATE)
    {
        //gk_audio_write(AUC_I2S_REG, gk_audio_reg.reg_06);
        //gk_audio_read(AUC_I2S_REG);
    }
}

static void gk_audio_dac_mute_off(void)
{
    if(gk_audio_check_state()!=AUC_RESET_STATE)
    {
        //gk_audio_write(AUC_DP_RESET_REG, AUC_DP_RESET_DAC);
        //gk_audio_read(AUC_DP_RESET_REG);
    }
}

static void gk_audio_volume_control(u8 volume)
{
    if (volume >= AUD_AO_GAIN_NUM)
        volume = AUD_AO_GAIN_NUM - 1;

    gk_audio_reg.reg_10 = (u32) gk_aud_readl(AUDC_DIGITAL_PGA1_DPGA_CFG1_REG);
    gk_audio_reg.reg_10 = gk_audio_reg.reg_10 & (0xFFFFE0E0);
    gk_audio_reg.reg_10 = gk_audio_reg.reg_10 |
        gk_audio_pga_gain_table[volume] |
        (gk_audio_pga_gain_table[volume] << 0x08);

    if (gk_audio_check_state() != AUC_RESET_STATE) {
        gk_aud_writel(AUDC_DIGITAL_PGA1_DPGA_CFG1_REG, gk_audio_reg.reg_10);
    }
}

static void gk_audio_capture_control(u8 volume)
{
    if (volume >= AUD_AI_GAIN_NUM)
        volume = AUD_AI_GAIN_NUM - 1;

    gk_audio_reg.reg_0e = (u32) gk_aud_readl(AUDC_ANALOG_CTRL04_REG);
    gk_audio_reg.reg_0e = gk_audio_reg.reg_0e & (0xFFFFFFE8);
    gk_audio_reg.reg_0e = gk_audio_reg.reg_0e | gk_audio_ai_gain_table[volume];

    if (gk_audio_check_state() != AUC_RESET_STATE) {
        gk_aud_writel(AUDC_ANALOG_CTRL04_REG, gk_audio_reg.reg_0e);
    }
}

static void gk_audio_set_adc_pga(u8 adc_pga)
{
    if (adc_pga > 0x1f )
    {
        printk("ADC PGA Setting Overflow, Maximum value is selected.");
        adc_pga = 0x1f;
    }

    if(gk_audio_check_state()!=AUC_RESET_STATE)
    {
        //gk_audio_reg.reg_0e = (gk_audio_reg.reg_0e&0xffe0) | adc_pga;
        //gk_audio_write(AUC_ADC_VGA_GAIN_REG, gk_audio_reg.reg_0e);
        //gk_audio_read(AUC_ADC_VGA_GAIN_REG);
    }
}

static void gk_audio_pwr_on(void)
{
    gk_audio_adc_power_on();
    gk_audio_adc_on();
    gk_audio_dac_power_on();
    gk_audio_dac_on();
}

static void gk_audio_pwr_down(void)
{
    gk_audio_adc_off();
    gk_audio_adc_power_down();
    gk_audio_dac_off();
    gk_audio_dac_power_down();
}

static void gk_audio_codec_init(void)
{
    gk_aud_writel(AHB_GENNERNAL1_REG, 0x00000001);

    gk_aud_writel(AUDC_ANALOG_CTRL00_REG, 0x8040);
    gk_aud_writel(AUDC_ANALOG_CTRL03_REG, 0x4004);
    gk_aud_writel(AUDC_ANALOG_CTRL04_REG, 0x2000);
    gk_aud_writel(AUDC_ANALOG_CTRL06_REG, 0x4);
    gk_aud_writel(AUDC_ANALOG_CTRL11_REG, 0x0);
    gk_aud_writel(AUDC_ANALOG_CTRL13_REG, 0x6000);
    gk_aud_writel(AUDC_ANALOG_CTRL00_REG, 0x8040);
    gk_aud_writel(AUDC_ANALOG_CTRL03_REG, 0x401c);
    gk_aud_writel(AUDC_ANALOG_CTRL04_REG, 0x233a);
    gk_aud_writel(AUDC_ANALOG_CTRL05_REG, 0x280c);
    gk_aud_writel(AUDC_ANALOG_CTRL06_REG, 0x8407);
    gk_aud_writel(AUDC_ANALOG_CTRL07_REG, 0x3802);
    gk_aud_writel(AUDC_ANALOG_CTRL08_REG, 0xf8);
    gk_aud_writel(AUDC_ANALOG_CTRL09_REG, 0x0);
    gk_aud_writel(AUDC_ANALOG_CTRL10_REG, 0xc0c0);
    gk_aud_writel(AUDC_ANALOG_CTRL11_REG, 0x9080);
    gk_aud_writel(AUDC_ANALOG_CTRL12_REG, 0x0);
    gk_aud_writel(AUDC_ANALOG_CTRL13_REG, 0x6000);

    gk_aud_writel(AUDC_DIGITAL_MMP1_DPGA_CFG1_REG, 0x5);
    gk_aud_writel(AUDC_DIGITAL_MMP2_DPGA_CFG1_REG, 0x5);
    gk_aud_writel(AUDC_DIGITAL_SYS_RST_CTRL0_REG, 0xe000);
    gk_aud_writel(AUDC_DIGITAL_SYS_RST_CTRL0_REG, 0x0);
    gk_aud_writel(AUDC_DIGITAL_AUDIOBAND_CTRL0_REG, 0x2);
    gk_aud_writel(AUDC_DIGITAL_CKG_CTRL0_REG, 0x3);
    gk_aud_writel(AUDC_DIGITAL_TIMING_CTRL0_REG, 0x33f);
    gk_aud_writel(AUDC_DIGITAL_TIMING_CTRL1_REG, 0xf3e);
    gk_aud_writel(AUDC_DIGITAL_AUDIOBAND_CTRL2_REG, 0xfaaf);
    gk_aud_writel(AUDC_DIGITAL_SDM_CTRL0_REG, 0x15);
    gk_aud_writel(AUDC_DIGITAL_MMP1_DPGA_CFG1_REG, 0x0);
    gk_aud_writel(AUDC_DIGITAL_MMP2_DPGA_CFG1_REG, 0x0);
    gk_aud_writel(AUDC_DIGITAL_TIMING_CTRL0_REG, 0x23f);
    gk_aud_writel(AUDC_DIGITAL_MIX_CTRL0_REG, 0x2500);
    gk_aud_writel(AUDC_DIGITAL_TIMING_CTRL1_REG, 0x23e);
    gk_aud_writel(AUDC_DIGITAL_AUDIOBAND_CTRL1_REG, 0xea82);
    gk_aud_writel(AUDC_DIGITAL_AUDIOBAND_CTRL2_REG, 0xaaaf);
    gk_aud_writel(AUDC_ANALOG_CTRL02_REG, 0x8);
    gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_1_NF_H_REG, 0x000000BB);
    gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_1_NF_L_REG, 0x00008000);
    mdelay(1);
    gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_1_NF_H_REG, 0x0000C0BB);
    mdelay(1);
    gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_1_NF_H_REG, 0x000000BB);
    gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_2_NF_H_REG, 0x000000c0);
    gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_2_NF_L_REG, 0x00000000);
    mdelay(1);
    gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_2_NF_H_REG, 0x0000c0c0);
    mdelay(1);
    gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_2_NF_H_REG, 0x0000c0);
    gk_aud_writel(AUDC_DIGITAL_FIFO_TH_CTRL0_REG, 0x402);
    gk_aud_writel(AUDC_DIGITAL_FIFO_CTRL_REG, 0x8808);
    gk_aud_writel(AUDC_DIGITAL_FIFO_CTRL_REG, 0x8888);
    gk_aud_writel(AHB_GENNERNAL1_REG, 0x00000001);

    //gk_aud_setbitsl(AUDC_DIGITAL_PGA1_DPGA_CFG1_REG, 0x4040);
    // mute_disable
    gk_aud_clrbitsl(AUDC_DIGITAL_PGA1_DPGA_CFG1_REG, 0x4040);
    // aio_ao_set_volume(V_LEVEL_8);
    gk_aud_writel(AUDC_DIGITAL_PGA1_DPGA_CFG1_REG, 0xA0A0);
    // aio_ao_unbind_ai
    gk_aud_writel(AUDC_DIGITAL_MIX_CTRL0_REG, 0x2500);
    // MODE_SINGLE
    gk_aud_clrbitsw(AUDC_DIGITAL_MIX_CTRL0_REG, 0x4000);
    // MODE_MONO /MODE_STEREO
    gk_aud_clrbitsl(AUDC_DIGITAL_MIX_CTRL0_REG, 0x30);
    gk_aud_clrbitsl(AUDC_DIGITAL_MIX_CTRL0_REG, 0xc);

    // SAMPLE_RATE_16000
    gk_aud_writel(AUDC_DIGITAL_TIMING_CTRL0_REG, 0x23f);
    gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_1_NF_H_REG, 0x000000BB);
    gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_1_NF_L_REG, 0x00008000);
    mdelay(1);
    gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_1_NF_H_REG, 0x0000C0BB);
    mdelay(1);
    gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_1_NF_H_REG, 0x000000BB);
    gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_2_NF_H_REG, 0x000000C0);
    gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_2_NF_L_REG, 0x00000000);
    mdelay(1);
    gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_2_NF_H_REG, 0x0000C0C0);
    mdelay(1);
    gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_2_NF_H_REG, 0x000000C0);

    /* Enable De-Emphasis Filter */

    gk_audio_adc_off();
    gk_audio_adc_power_down();
    gk_audio_dac_off();
    gk_audio_dac_power_down();
    gk_audio_init_fiter();

    gk_audio_set_adc_pga(AUC_ADC_VGA_GAIN_REG_OPT);

    gk_audio_volume_control(DAC_PGA_GAIN_0db);
    gk_audio_dac_mute_on();
    gk_audio_adc_mute_on();

    gk_audio_dac_mute_off();
    gk_audio_adc_mute_off();

    if(gk_aud_readl(AUDC_ANALOG_CTRL00_REG) != 0x8040)
    {
        printk("ERROR: AUC Initial Fail, Audio PLL Configuration Error\n");
    }

    // MODE_LEFT:
    //gk_aud_clrbitsl(AUDC_DIGITAL_MIX_CTRL0_REG, 0x30);
    //gk_aud_setbitsl(AUDC_DIGITAL_MIX_CTRL0_REG, 0xc);
    // MODE_RIGHT:
    //gk_aud_setbitsl(AUDC_DIGITAL_MIX_CTRL0_REG, 0x30);
    //gk_aud_clrbitsl(AUDC_DIGITAL_MIX_CTRL0_REG, 0xc);
    //MODE_SINGLE:
    //gk_aud_clrbitsl(AUDC_DIGITAL_MIX_CTRL0_REG, 0x30);
    //gk_aud_setbitsl(AUDC_DIGITAL_MIX_CTRL0_REG, 0xc);
    // DMA
    //gk_aud_clrbitsl(AUDC_DIGITAL_FIFO_CTRL_REG, 0x1000);
    //gk_aud_setbitsl(AUDC_DIGITAL_FIFO_CTRL_REG, 0x8000);
    // GAIN
    //aio_ai_set_gain(G_LEVEL_5);
    gk_aud_clrbitsl(AUDC_ANALOG_CTRL04_REG, 0xFF);
    gk_aud_setbitsl(AUDC_ANALOG_CTRL04_REG, 0x38);
}

static unsigned int gk_audio_codec_read(struct snd_soc_codec *codec,
            unsigned int _reg)
{
    u32 reg = (AUDIO_CODEC_ANALOG_BASE & 0xFFFF0000) + _reg;
    return gk_aud_readl(reg);
}

static int gk_audio_codec_write(struct snd_soc_codec *codec, unsigned int _reg,
            unsigned int value)
{
    u32 reg = (AUDIO_CODEC_ANALOG_BASE & 0xFFFF0000) + _reg;
    gk_aud_writel(reg, value);

    return 0;
}

static int gk_audio_startup(struct snd_pcm_substream *substream,
            struct snd_soc_dai *dai)
{
    gk_audio_pwr_on();

    return 0;
}

static void gk_audio_shutdown(struct snd_pcm_substream *substream,
            struct snd_soc_dai *dai)
{
    if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
    {
        gk_audio_adc_off();
        gk_audio_adc_power_down();
    }
    else
    {
        gk_audio_dac_off();
        gk_audio_dac_power_down();
    }
}

static int gk_audio_digital_mute(struct snd_soc_dai *dai, int mute)
{
    if (mute)
        gk_audio_dac_mute_on();
    else
        gk_audio_dac_mute_off();

    return 0;
}

static int gk_audio_set_fmt(struct snd_soc_dai *dai, unsigned int fmt)
{
    int ret = 0;

    switch (fmt & SND_SOC_DAIFMT_MASTER_MASK)
    {
    case SND_SOC_DAIFMT_CBS_CFS:
        break;
    default:
        ret = -EINVAL;
        break;
    }

    switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK)
    {
    case SND_SOC_DAIFMT_I2S:
        break;
    default:
        ret = -EINVAL;
        break;
    }

    switch (fmt & SND_SOC_DAIFMT_INV_MASK)
    {
    case SND_SOC_DAIFMT_NB_NF:
        break;
    default:
        ret = -EINVAL;
        break;
    }

    return ret;
}

static int gk_audio_set_clkdiv(struct snd_soc_dai *codec_dai, int div_id, int div)
{
    /* gk_audio_sfreq_conf */
    /*if(likely(div_id == A2AUC_CLKDIV_LRCLK)) {
        switch (div) {
        case AudioCodec_128xfs:
            gk_audio_reg.reg_02 = AUC_OSR_128x;
            break;
        case AudioCodec_256xfs:
            gk_audio_reg.reg_02 = AUC_OSR_256x;
            break;
        case AudioCodec_384xfs:
            gk_audio_reg.reg_02 = AUC_OSR_384x;
            break;
        case AudioCodec_512xfs:
            gk_audio_reg.reg_02 = AUC_OSR_512x;
            break;
        default:
            return -EINVAL;
        }
    } else {
        return -EINVAL;
    }

    if(gk_audio_check_state()!=AUC_RESET_STATE)
    {
        gk_aud_writel(AUC_OSR_REG, gk_audio_reg.reg_02);
        gk_aud_readl(AUC_OSR_REG);
    }*/

    return 0;
}

static int gk_audio_set_bias_level(struct snd_soc_codec *codec,
                 enum snd_soc_bias_level level)
{
    switch (level)
    {
    case SND_SOC_BIAS_ON: /* full On */
        gk_audio_pwr_on();
        gk_audio_adc_mute_off();
        gk_audio_dac_mute_off();
        break;
    case SND_SOC_BIAS_PREPARE:
        break;
    case SND_SOC_BIAS_STANDBY: /* Off, with power */
        gk_audio_pwr_on();
        break;
    case SND_SOC_BIAS_OFF: /* Off, without power */
        /* everything off, dac mute, inactive */
        gk_audio_dac_mute_on();
        gk_audio_adc_mute_on();
        gk_audio_pwr_down();
        break;
    }
    codec->dapm.bias_level = level;
    return 0;
}

static struct snd_soc_dai_ops gk_audio_dai_ops =
{
    .startup        = gk_audio_startup,
    .shutdown       = gk_audio_shutdown,
    .digital_mute   = gk_audio_digital_mute,
    .set_fmt        = gk_audio_set_fmt,
    .set_clkdiv     = gk_audio_set_clkdiv,
};

static struct snd_soc_dai_driver gk_audio_dai =
{
    .name = "gk_audio_dai",
    .playback =
    {
        .stream_name = "Playback",
        .channels_min = 1,
        .channels_max = 1,
        .rates          = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_32000,
        .formats        = SNDRV_PCM_FMTBIT_S16_LE,
    },
    .capture =
    {
        .stream_name = "Capture",
        .channels_min = 1,
        .channels_max = 1,
        .rates          = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_32000,
        .formats        = SNDRV_PCM_FMTBIT_S16_LE,
    },
    .ops = &gk_audio_dai_ops,
};

static int gk_audio_probe(struct snd_soc_codec *codec)
{
    dev_info(codec->dev, "GKIPC Audio Codec");

    gk_audio_codec_init();

    /* power on device */
    gk_audio_set_bias_level(codec, SND_SOC_BIAS_STANDBY);

    snd_soc_add_codec_controls(codec, gk_audio_snd_controls,
        ARRAY_SIZE(gk_audio_snd_controls));
    gk_audio_add_widgets(codec);

    return 0;
}

static int gk_audio_remove(struct snd_soc_codec *codec)
{
    gk_audio_set_bias_level(codec, SND_SOC_BIAS_OFF);

    return 0;
}

#ifdef CONFIG_PM
static int gk_audio_suspend(struct snd_soc_codec *codec, pm_message_t state)
{
    gk_audio_set_bias_level(codec, SND_SOC_BIAS_OFF);

    return 0;
}

static int gk_audio_resume(struct snd_soc_codec *codec)
{
    gk_audio_set_bias_level(codec, SND_SOC_BIAS_STANDBY);

    return 0;
}

#else
#define gk_audio_suspend    NULL
#define gk_audio_resume     NULL
#endif

static struct snd_soc_codec_driver soc_codec_dev_gk_audio =
{
    .probe          = gk_audio_probe,
    .remove         = gk_audio_remove,
    .suspend        = gk_audio_suspend,
    .resume         = gk_audio_resume,
    .read           = gk_audio_codec_read,
    .write          = gk_audio_codec_write,
    .set_bias_level = gk_audio_set_bias_level,
    .reg_cache_step = 4,
};

static int __devinit gk_audio_codec_probe(struct platform_device *pdev)
{
    return snd_soc_register_codec(&pdev->dev,
        &soc_codec_dev_gk_audio, &gk_audio_dai, 1);
}

static int __devexit gk_audio_codec_remove(struct platform_device *pdev)
{
    snd_soc_unregister_codec(&pdev->dev);
    return 0;
}

static struct platform_driver gk_audio_codec_driver =
{
    .probe      = gk_audio_codec_probe,
    .remove     = __devexit_p(gk_audio_codec_remove),
    .driver     =
    {
        .name   = "gk_audio-codec",
        .owner  = THIS_MODULE,
    },
};

static struct platform_device gk_auc_codec0 = {
    .name   = "gk_audio-codec",
    .id     = -1,
};

int __init gk_audio_init(void)
{
    platform_device_register(&gk_auc_codec0);
    device_set_wakeup_capable(&gk_auc_codec0.dev, 1);
    device_set_wakeup_enable(&gk_auc_codec0.dev, 0);

    return platform_driver_register(&gk_audio_codec_driver);
}

module_init(gk_audio_init);
void __exit gk_audio_exit(void)
{
    platform_driver_unregister(&gk_audio_codec_driver);
}
module_exit(gk_audio_exit);

MODULE_AUTHOR("<yulindeng@goke.com>");
MODULE_DESCRIPTION("GK IPC Soc Audio Driver");
MODULE_LICENSE("GPL");

