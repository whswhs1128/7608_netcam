/*!
*****************************************************************************
** \file        linux/kernel/drv/webcam/src/sound/soc/goke/gk_evb_audio.c
**
** \version     $Id: gk_evb_audio.c 13657 2018-05-14 02:54:03Z yulindeng $
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
#include <plat/gk_gpio.h>

#include "plat/audio.h"
#include "../inc/gk_i2s.h"
#include "../inc/gk_pcm.h"
#include "gk_audc.h"

#define CONFIG_SND_SOC_GOKE_AUDC    1

#include "gk_audc.c"
//#define GK_DEBUG_PRINTK(fmt, args...) printk(fmt, ## args)
#define GK_DEBUG_PRINTK printk
//#define GK_DEBUG_PRINTK(fmt, args...)


#define GK_MIC_ON       0
#define GK_MIC_OFF      1
#define GK_SPK_ON       0
#define GK_SPK_OFF      1

/* mic and speaker function: default ON */
static int gka_mic_func = GK_MIC_ON;
static int gka_spk_func = GK_SPK_ON;

static void gk_ipcam_ext_control(struct snd_soc_codec *codec)
{
    struct snd_soc_dapm_context *dapm = &codec->dapm;

    /* set up mic connection */
    if (gka_mic_func == GK_MIC_ON)
    {
        snd_soc_dapm_enable_pin(dapm, "Mic Jack");
    }
    else
    {
        snd_soc_dapm_disable_pin(dapm, "Mic Jack");
    }

    if (gka_spk_func == GK_SPK_ON)
    {
        snd_soc_dapm_enable_pin(dapm, "Ext Spk");
    }
    else
    {
        snd_soc_dapm_disable_pin(dapm, "Ext Spk");
    }

    /* signal a DAPM event */
    snd_soc_dapm_sync(dapm);

    return;
}

static int gk_ipcam_board_startup(struct snd_pcm_substream *substream)
{
    struct snd_soc_pcm_runtime *rtd = substream->private_data;
    struct snd_soc_codec *codec = rtd->codec;

    gk_ipcam_ext_control(codec);

    return 0;
}

static int gk_ipcam_board_hw_params(struct snd_pcm_substream *substream,
    struct snd_pcm_hw_params *params)
{
    struct snd_soc_pcm_runtime *rtd = substream->private_data;
    struct snd_soc_dai *codec_dai = rtd->codec_dai;
    struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
    int errorCode = 0, mclk, oversample;

    switch (params_rate(params)) {
    case 8000:
        gk_aud_writel(AUDC_DIGITAL_TIMING_CTRL0_REG, 0x3f);

        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_1_NF_H_REG, 0x000002EE);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_1_NF_L_REG, 0x00000000);
        mdelay(1);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_1_NF_H_REG, 0x0000C2EE);
        mdelay(1);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_1_NF_H_REG, 0x000002EE);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_2_NF_H_REG, 0x00000300);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_2_NF_L_REG, 0x00000000);
        mdelay(1);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_2_NF_H_REG, 0x0000C300);
        mdelay(1);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_2_NF_H_REG, 0x00000300);
        break;
    case 11025:
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_1_NF_H_REG, 0x00000220);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_1_NF_L_REG, 0x000037BA);
        mdelay(1);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_1_NF_H_REG, 0x0000C220);
        mdelay(1);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_1_NF_H_REG, 0x00000220);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_2_NF_H_REG, 0x0000022D);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_2_NF_L_REG, 0x00004766);
        mdelay(1);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_2_NF_H_REG, 0x0000C22D);
        mdelay(1);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_2_NF_H_REG, 0x0000022D);
        break;
    case 12000:
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_1_NF_H_REG, 0x000001F4);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_1_NF_L_REG, 0x00000000);
        mdelay(1);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_1_NF_H_REG, 0x0000C1F4);
        mdelay(1);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_1_NF_H_REG, 0x000001F4);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_2_NF_H_REG, 0x00000200);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_2_NF_L_REG, 0x00000000);
        mdelay(1);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_2_NF_H_REG, 0x0000C200);
        mdelay(1);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_2_NF_H_REG, 0x00000200);
        break;
    case 16000:
        gk_aud_writel(AUDC_DIGITAL_TIMING_CTRL0_REG, 0x13f);

        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_1_NF_H_REG, 0x00000177);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_1_NF_L_REG, 0x00000000);
        mdelay(1);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_1_NF_H_REG, 0x0000C177);
        mdelay(1);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_1_NF_H_REG, 0x00000177);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_2_NF_H_REG, 0x00000180);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_2_NF_L_REG, 0x00000000);
        mdelay(1);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_2_NF_H_REG, 0x0000C180);
        mdelay(1);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_2_NF_H_REG, 0x00000180);
        break;
    case 22050:
        gk_aud_writel(AUDC_DIGITAL_TIMING_CTRL0_REG, 0x23f);

        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_1_NF_H_REG, 0x00000110);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_1_NF_L_REG, 0x00001BDD);
        mdelay(1);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_1_NF_H_REG, 0x0000C110);
        mdelay(1);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_1_NF_H_REG, 0x00000110);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_2_NF_H_REG, 0x00000116);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_2_NF_L_REG, 0x0000A3B3);
        mdelay(1);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_2_NF_H_REG, 0x0000C116);
        mdelay(1);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_2_NF_H_REG, 0x00000116);
        break;
    case 24000:
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_1_NF_H_REG, 0x000000FA);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_1_NF_L_REG, 0x00000000);
        mdelay(1);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_1_NF_H_REG, 0x0000C0FA);
        mdelay(1);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_1_NF_H_REG, 0x000000FA);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_2_NF_H_REG, 0x00000100);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_2_NF_L_REG, 0x00000000);
        mdelay(1);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_2_NF_H_REG, 0x0000C100);
        mdelay(1);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_2_NF_H_REG, 0x00000100);
        break;
    case 32000:
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
        break;
    case 44100:
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_1_NF_H_REG, 0x00000088);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_1_NF_L_REG, 0x00000DEF);
        mdelay(1);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_1_NF_H_REG, 0x0000C088);
        mdelay(1);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_1_NF_H_REG, 0x00000088);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_2_NF_H_REG, 0x0000008B);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_2_NF_L_REG, 0x000051DA);
        mdelay(1);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_2_NF_H_REG, 0x0000C08B);
        mdelay(1);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_2_NF_H_REG, 0x0000008B);
        break;
    case 48000:
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_1_NF_H_REG, 0x0000007D);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_1_NF_L_REG, 0x00000000);
        mdelay(1);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_1_NF_H_REG, 0x0000C07D);
        mdelay(1);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_1_NF_H_REG, 0x0000007D);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_2_NF_H_REG, 0x00000080);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_2_NF_L_REG, 0x00000000);
        mdelay(1);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_2_NF_H_REG, 0x0000C080);
        mdelay(1);
        gk_aud_writel(AUDC_DIGITAL_NF_SYNTH_2_NF_H_REG, 0x00000080);
        break;
    default:
        errorCode = -EINVAL;
        goto hw_params_exit;
    }
    return errorCode;

    /* set the I2S system data format*/
    errorCode = snd_soc_dai_set_fmt(codec_dai, SND_SOC_DAIFMT_I2S |
        SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS);
    if (errorCode < 0)
    {
        printk(KERN_ERR "can't set codec DAI configuration\n");
        goto hw_params_exit;
    }

    errorCode = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_I2S |
        SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS);
    if (errorCode < 0)
    {
        printk(KERN_ERR "can't set cpu DAI configuration\n");
        goto hw_params_exit;
    }

    /* set the I2S system clock*/
    errorCode = snd_soc_dai_set_sysclk(cpu_dai, GOKE_CLKSRC_ON_IPC_CHIP, mclk, 0);
    if (errorCode < 0)
    {
        printk(KERN_ERR "can't set cpu MCLK configuration\n");
        goto hw_params_exit;
    }

    errorCode = snd_soc_dai_set_clkdiv(codec_dai, GOKE1_CLKDIV_LRCLK, oversample);
    if (errorCode < 0)
    {
        printk(KERN_ERR "can't set codec MCLK/SF ratio\n");
        goto hw_params_exit;
    }

    errorCode = snd_soc_dai_set_clkdiv(cpu_dai, GOKE_CLKDIV_LRCLK, oversample);
    if (errorCode < 0)
    {
        printk(KERN_ERR "can't set cpu MCLK/SF ratio\n");
        goto hw_params_exit;
    }

hw_params_exit:
    return errorCode;
}

static int gk_ipcam_get_mic(struct snd_kcontrol *kcontrol,
    struct snd_ctl_elem_value *ucontrol)
{
    ucontrol->value.integer.value[0] = gka_mic_func;
    return 0;
}

static int gk_ipcam_set_mic(struct snd_kcontrol *kcontrol,
    struct snd_ctl_elem_value *ucontrol)
{
    struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);

    if (gka_mic_func == ucontrol->value.integer.value[0])
        return 0;

    gka_mic_func = ucontrol->value.integer.value[0];
    gk_ipcam_ext_control(codec);
    return 1;
}

static int gk_ipcam_get_spk(struct snd_kcontrol *kcontrol,
    struct snd_ctl_elem_value *ucontrol)
{
    ucontrol->value.integer.value[0] = gka_spk_func;
    return 0;
}

static int gk_ipcam_set_spk(struct snd_kcontrol *kcontrol,
    struct snd_ctl_elem_value *ucontrol)
{
    struct snd_soc_codec *codec =  snd_kcontrol_chip(kcontrol);

    if (gka_spk_func == ucontrol->value.integer.value[0])
        return 0;

    gka_spk_func = ucontrol->value.integer.value[0];
    gk_ipcam_ext_control(codec);
    return 1;
}

static int gk_ipcam_spk_event(struct snd_soc_dapm_widget *w,
    struct snd_kcontrol *k, int event)
{
    int errorCode = 0;

    if (SND_SOC_DAPM_EVENT_ON(event))
    {
    }
    else
    {
    }

    return errorCode;
}

static int gk_ipcam_mic_event(struct snd_soc_dapm_widget *w,
    struct snd_kcontrol *k, int event)
{
    int errorCode = 0;

    if (SND_SOC_DAPM_EVENT_ON(event))
    {
    }
    else
    {
    }

    return errorCode;
}

/* IPcam machine dapm widgets */
static const struct snd_soc_dapm_widget gk_ipcam_dapm_widgets[] =
{
    SND_SOC_DAPM_MIC("Mic Jack",    gk_ipcam_mic_event),
    SND_SOC_DAPM_SPK("Ext Spk",     gk_ipcam_spk_event),
    SND_SOC_DAPM_LINE("Line In",    NULL),
    SND_SOC_DAPM_LINE("Line Out",   NULL),
};

/* IPcam machine audio map (connections to the a2auc pins) */
static const struct snd_soc_dapm_route gk_ipcam_audio_map[] =
{
    /* speaker is connected to SPOUT */
    {"Ext Spk",     NULL, "SPOUT"},

    /* mic is connected to LLIN, RLIN*/
    {"LLIN",        NULL, "Mic Jack"},
    {"RLIN",        NULL, "Mic Jack"},

    /* Line Out is connected to LLOUT, RLOUT */
    {"Line Out",    NULL, "LLOUT"},
    {"Line Out",    NULL, "RLOUT"},

    /* Line In is connected to LLIN, RLIN */
    {"LLIN",        NULL, "Line In"},
    {"RLIN",        NULL, "Line In"},
};

static const char *gk_mic_function[] = {"On", "Off"};
static const char *gk_spk_function[] = {"On", "Off"};
static const struct soc_enum gk_ipcam_enum[] =
{
    SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(gk_mic_function), gk_mic_function),
    SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(gk_spk_function), gk_spk_function),
};

static const struct snd_kcontrol_new gk_ipcam_controls[] =
{
    SOC_ENUM_EXT("Mic Function", gk_ipcam_enum[0], gk_ipcam_get_mic, gk_ipcam_set_mic),
    SOC_ENUM_EXT("Spk Function", gk_ipcam_enum[1], gk_ipcam_get_spk, gk_ipcam_set_spk),
};

int gk_ipcam_init(struct snd_soc_pcm_runtime *rtd)
{
    int errorCode = 0;
    struct snd_soc_codec *codec = rtd->codec;
    struct snd_soc_dapm_context *dapm = &codec->dapm;

    snd_soc_dapm_nc_pin(dapm, "LHPOUT");
    snd_soc_dapm_nc_pin(dapm, "RHPOUT");
    /* mask this control ,beacase not implementation */
#if 0
    /* Add IPcam specific controls */
    snd_soc_add_codec_controls(codec, gk_ipcam_controls,
                ARRAY_SIZE(gk_ipcam_controls));
#endif

    /* Add IPcam specific widgets */
    errorCode = snd_soc_dapm_new_controls(dapm,
        gk_ipcam_dapm_widgets,
        ARRAY_SIZE(gk_ipcam_dapm_widgets));
    if (errorCode)
    {
        goto init_exit;
    }

    /* Set up IPcam specific audio path gk_ipcam_audio_map */
    errorCode = snd_soc_dapm_add_routes(dapm,
        gk_ipcam_audio_map,
        ARRAY_SIZE(gk_ipcam_audio_map));
    if (errorCode)
    {
        goto init_exit;
    }

    errorCode = snd_soc_dapm_sync(dapm);

init_exit:
    return errorCode;
}

static struct snd_soc_ops gk_ipcam_board_ops = {
    .startup = gk_ipcam_board_startup,
    .hw_params = gk_ipcam_board_hw_params,
};

static struct snd_soc_dai_link gk_ipcam_dai_link =
{
    .name           = "evb_audio",
    .stream_name    = "gk-audio",
    .cpu_dai_name   = "gk-i2s.0",
    .platform_name  = "gk-pcm",
    .codec_dai_name = "gk_audio_dai",
    .codec_name     = "gk_audio-codec",
    .init           = gk_ipcam_init,
    .ops            = &gk_ipcam_board_ops,
};

struct snd_soc_card snd_soc_card_ipcam =
{
    .name       = "GPIPcam",
    .dai_link   = &gk_ipcam_dai_link,
    .num_links  = 1,
};
EXPORT_SYMBOL(snd_soc_card_ipcam);

struct gk_i2s_priv gk_i2s_priv_data =
{
    .gk_i2s_intf =
    {
        .mode         = DAI_I2S_Mode,
        .clksrc       = GOKE_CLKSRC_ON_IPC_CHIP,
        .ms_mode      = DAI_MASTER,
        .mclk         = AudioCodec_12_288M,
        .oversample   = AudioCodec_256xfs,
        .word_order   = DAI_MSB_FIRST,
        .sfreq        = AUDIO_SF_16000,
        .word_len     = DAI_16bits,
        .word_pos     = 0,
        .slots        = DAI_32slots,
        .ch           = 2,  /* dynamic */
    },
    .playback   =
    {
        .channels_min   = 1,
        .channels_max   = 1,
        .rates          = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_32000,
        .formats        = SNDRV_PCM_FMTBIT_S16_LE,
    },
    .capture =
    {
        .channels_min   = 1,
        .channels_max   = 1,
        .rates          = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_32000,
        .formats        = SNDRV_PCM_FMTBIT_S16_LE,
    },
    //.clock_reg,
    .gpio_LRCLK = -1,
    .gpio_SPK = -1,
    .AudioOrI2s = AUDIO_MODE,
    .use_pio = 0,
};
EXPORT_SYMBOL(gk_i2s_priv_data);

