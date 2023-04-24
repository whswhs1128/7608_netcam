/*!
*****************************************************************************
** \file        linux/kernel/drv/webcam/src/sound/soc/goke/gk_i2s.c
**
** \version     $Id: gk_i2s.c 13693 2018-05-30 03:21:36Z yulindeng $
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
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>

#include <mach/hardware.h>
#include <plat/audio.h>
#include <mach/gpio.h>
#include <mach/io.h>
#include <mach/i2s.h>
#include <mach/audio_codec.h>

#include CONFIG_GK_CHIP_INCLUDE_FILE

#include "../inc/gk_pcm.h"
#include "../inc/gk_i2s.h"

unsigned int used_port = 1;
module_param(used_port, uint, S_IRUGO);
MODULE_PARM_DESC(used_port, "Select the I2S port.");

unsigned int default_sfreq = 0;
module_param(default_sfreq, uint, S_IRUGO);
MODULE_PARM_DESC(default_sfreq, "Default sfreq: 0. 44100, 1. 48000.");

static DEFINE_MUTEX(clock_reg_mutex);
static int enable_ext_i2s = 1;

/* FIXME HERE for PCM interface */
static struct gk_pcm_dma_params gk_i2s_pcm_stereo_out =
{
    .name       = "I2S PCM Stereo out",
    .dev_addr   = I2S_TX_LEFT_DATA_DMA_REG,
};

static struct gk_pcm_dma_params gk_i2s_pcm_stereo_in =
{
    .name       = "I2S PCM Stereo in",
    .dev_addr   = I2S_RX_DATA_DMA_REG,
};

static inline void dai_tx_enable(struct gk_i2s_priv *priv_data)
{
    if(priv_data->AudioOrI2s == AUDIO_MODE)
    {
        gk_aud_clrbitsl(AUDC_DIGITAL_FIFO_CTRL_REG, 0x80);
        gk_aud_clrbitsl(AUDC_DIGITAL_FIFO_CTRL_REG, 0x1F);
        gk_aud_setbitsl(AUDC_DIGITAL_FIFO_CTRL_REG, 0x18);
    }
    else
    {
        gk_i2s_setbitsl(I2S_INIT_REG, I2S_TX_FIFO_RESET_BIT);
        //msleep(1);
        if(gk_i2s_tstbitsl(I2S_INIT_REG, I2S_TX_FIFO_RESET_BIT))
        {
            printk("DAI_FIFO_RST fail!\n");
        }
        gk_i2s_writel(I2S_TX_FIFO_LTH_REG, 0x20);
        gk_i2s_setbitsl(I2S_INIT_REG, I2S_TX_ENABLE_BIT);
    }
    if(priv_data->gpio_SPK != -1)
    {
        gk_gpio_set_out(priv_data->gpio_SPK, 0x01);
    }
}

static inline void dai_rx_enable(struct gk_i2s_priv *priv_data)
{
    if(priv_data->AudioOrI2s == AUDIO_MODE)
    {
        gk_aud_clrbitsl(AUDC_DIGITAL_FIFO_CTRL_REG, 0x8000);
        gk_aud_clrbitsl(AUDC_DIGITAL_FIFO_CTRL_REG, 0x1F00);
        gk_aud_setbitsl(AUDC_DIGITAL_FIFO_CTRL_REG, 0x1800);
    }
    else
    {
        /*gk_i2s_setbitsl(I2S_INIT_REG, I2S_FIFO_RESET_BIT);
        gk_i2s_setbitsl(I2S_INIT_REG, I2S_FIFO_RESET_BIT);
        //dma_start(i2s_dev);
        msleep(1);
        if(gk_i2s_tstbitsl(I2S_INIT_REG, 0x03))
        {
            GK_DEBUG_PRINTK("DAI_FIFO_RST fail!\n");
        }
        gk_i2s_setbitsl(I2S_CLOCK_REG, I2S_CLK_WS_OUT_EN | I2S_CLK_BCLK_OUT_EN);
        gk_i2s_setbitsl(I2S_CLOCK_REG, I2S_CLK_BCLK_OUTPUT);*/
        gk_i2s_setbitsl(I2S_INIT_REG, I2S_RX_FIFO_RESET_BIT);
        gk_i2s_writel(I2S_RX_FIFO_GTH_REG, 0x20);
        //msleep(1);
        if(gk_i2s_tstbitsl(I2S_INIT_REG, I2S_RX_FIFO_RESET_BIT))
        {
            printk("DAI_FIFO_RST fail!\n");
        }
        udelay(1000);
        if(priv_data->gpio_LRCLK != -1)
        {
            gk_gpio_set_out(priv_data->gpio_LRCLK, 0x01);
        }
        gk_i2s_setbitsl(I2S_INIT_REG, I2S_RX_ENABLE_BIT);
    }
}

static inline void dai_tx_disable(struct gk_i2s_priv *priv_data)
{
    if(priv_data->AudioOrI2s == AUDIO_MODE)
    {
        gk_aud_setbitsl(AUDC_DIGITAL_FIFO_CTRL_REG, 0x80);
        gk_aud_clrbitsl(AUDC_DIGITAL_FIFO_CTRL_REG, 0x10);
    }
    else
    {
        gk_i2s_clrbitsl(I2S_INIT_REG, DAI_TX_EN);
    }
    if(priv_data->gpio_SPK != -1)
    {
        gk_gpio_set_out(priv_data->gpio_SPK, 0x00);
    }
}

static inline void dai_rx_disable(struct gk_i2s_priv *priv_data)
{
    if(priv_data->AudioOrI2s == AUDIO_MODE)
    {
        gk_aud_setbitsl(AUDC_DIGITAL_FIFO_CTRL_REG, 0x8000);
        gk_aud_clrbitsl(AUDC_DIGITAL_FIFO_CTRL_REG, 0x1000);
    }
    else
    {
        gk_i2s_clrbitsl(I2S_INIT_REG, DAI_RX_EN);
        if(priv_data->gpio_LRCLK != -1)
        {
            gk_gpio_set_out(priv_data->gpio_LRCLK, 0x00);
        }
    }
}

static inline void dai_fifo_rx_rst(struct gk_i2s_priv *priv_data)
{
    if(priv_data->AudioOrI2s == I2S_MODE)
    {
        gk_i2s_clrbitsl(I2S_INIT_REG, I2S_RX_FIFO_RESET_BIT | I2S_RX_ENABLE_BIT);
        gk_i2s_setbitsl(I2S_INIT_REG, I2S_RX_FIFO_RESET_BIT);
        msleep(1);
        if(gk_i2s_tstbitsl(I2S_INIT_REG, I2S_RX_FIFO_RESET_BIT))
        {
            printk("dai_fifo_rx_rst fail!\n");
        }
    }
}

static inline void dai_fifo_tx_rst(struct gk_i2s_priv *priv_data)
{
    if(priv_data->AudioOrI2s == I2S_MODE)
    {
        gk_i2s_clrbitsl(I2S_INIT_REG, I2S_TX_FIFO_RESET_BIT | I2S_TX_ENABLE_BIT);
        gk_i2s_setbitsl(I2S_INIT_REG, I2S_TX_FIFO_RESET_BIT);
        msleep(1);
        if(gk_i2s_tstbitsl(I2S_INIT_REG, I2S_TX_FIFO_RESET_BIT))
        {
            printk("dai_fifo_tx_rst fail!\n");
        }
    }
}

static int gk_i2s_hw_params(struct snd_pcm_substream *substream,
                struct snd_pcm_hw_params *params,
                struct snd_soc_dai *cpu_dai)
{
    struct gk_pcm_dma_params *dma_data;
    struct gk_i2s_priv *priv_data = snd_soc_dai_get_drvdata(cpu_dai);
    u8 slots, word_pos, clksrc, mclk, oversample;
    u32 clock_divider, clock_reg, channels;
    u32 bit_num = 0;

    if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
    {
        dma_data = &gk_i2s_pcm_stereo_out;
        /* Disable tx/rx before initializing */
        dai_tx_disable(priv_data);
        if(priv_data->AudioOrI2s == I2S_MODE)
        {
            if (priv_data->gk_i2s_intf.ms_mode == DAI_SLAVE)
                gk_i2s_clrbitsl(I2S_TX_CTRL_REG, I2S_TX_LOOPBACK_BIT);

            gk_i2s_writel(I2S_TX_FIFO_LTH_REG, 0x20);
        }
        /* Set channels */
        // hardware channels data
        channels = priv_data->playback.channels_max;//params_channels(params);
    }
    else
    {
        dma_data = &gk_i2s_pcm_stereo_in;
        /* Disable tx/rx before initializing */
        dai_rx_disable(priv_data);
        if(priv_data->AudioOrI2s == I2S_MODE)
        {
            if (priv_data->gk_i2s_intf.ms_mode == DAI_SLAVE)
            {
                gk_i2s_clrbitsl(I2S_RX_CTRL_REG, I2S_RX_WS_MST_BIT);
            }
            else
            {
                gk_i2s_setbitsl(I2S_RX_CTRL_REG, I2S_RX_WS_MST_BIT);
            }
            gk_i2s_clrbitsl(I2S_RX_CTRL_REG, I2S_RX_WS_INV_BIT);     // I2S_WS=0 rev first data
        }
        /* Set channels */
        // hardware channels data
        channels = priv_data->capture.channels_max;//params_channels(params);
    }

    snd_soc_dai_set_dma_data(cpu_dai, substream, dma_data);

    if (priv_data->controller_info->channel_select)
        priv_data->controller_info->channel_select(channels);
    // need channels data
    priv_data->gk_i2s_intf.ch = params_channels(params);

    /* Set format */
    switch (params_format(params))
    {
    case SNDRV_PCM_FORMAT_S16_LE:
        bit_num = 16;
        if(priv_data->AudioOrI2s == I2S_MODE)
        {
            if (priv_data->gk_i2s_intf.mode == DAI_DSP_Mode)
            {
                slots = channels - 1;
                word_pos = 0x0f;
                priv_data->gk_i2s_intf.slots = slots;
            }
            else
            {
                slots = 0;
                word_pos = 0;
                priv_data->gk_i2s_intf.slots = DAI_32slots;
            }
            priv_data->gk_i2s_intf.word_len = DAI_16bits;
            priv_data->gk_i2s_intf.word_pos = word_pos;
            priv_data->gk_i2s_intf.word_order = DAI_MSB_FIRST;

            gk_i2s_writel(I2S_MODE_REG, priv_data->gk_i2s_intf.mode);

            gk_i2s_writel(I2S_WLEN_REG, I2S_16BIT-1);

            gk_i2s_writel(I2S_WPOS_REG, word_pos);

            gk_i2s_writel(I2S_SLOT_REG, slots);

            gk_i2s_clrbitsl(I2S_24BITMUX_MODE_REG, I2S_24BITMUX_MODE_BIT);

            gk_i2s_setbitsl(I2S_TX_CTRL_REG, I2S_TX_UNISON_BIT);     // usion
        }
        break;
    case SNDRV_PCM_FORMAT_S24_LE:
        bit_num = 32;
        if(priv_data->AudioOrI2s == I2S_MODE)
        {
            if (priv_data->gk_i2s_intf.mode == DAI_DSP_Mode)
            {
                slots = channels - 1;
                word_pos = 0x0f;
                priv_data->gk_i2s_intf.slots = slots;
            }
            else
            {
                slots = 0;
                word_pos = 0x08;
                priv_data->gk_i2s_intf.slots = DAI_32slots;
            }
            priv_data->gk_i2s_intf.word_len = DAI_24bits;
            priv_data->gk_i2s_intf.word_pos = word_pos;
            priv_data->gk_i2s_intf.word_order = DAI_MSB_FIRST;

            gk_i2s_writel(I2S_MODE_REG, priv_data->gk_i2s_intf.mode);

            gk_i2s_writel(I2S_WLEN_REG, I2S_24BIT-1);

            gk_i2s_writel(I2S_WPOS_REG, word_pos);

            gk_i2s_writel(I2S_SLOT_REG, slots);

            gk_i2s_setbitsl(I2S_24BITMUX_MODE_REG, I2S_24BITMUX_MODE_BIT);
        }
        break;
    default:
        return -EINVAL;
    }

    switch (params_rate(params))
    {
    case 8000:
        priv_data->gk_i2s_intf.sfreq = AUDIO_SF_8000;
        break;
    case 11025:
        priv_data->gk_i2s_intf.sfreq = AUDIO_SF_11025;
        break;
    case 16000:
        priv_data->gk_i2s_intf.sfreq = AUDIO_SF_16000;
        break;
    case 22050:
        priv_data->gk_i2s_intf.sfreq = AUDIO_SF_22050;
        break;
    case 32000:
        priv_data->gk_i2s_intf.sfreq = AUDIO_SF_32000;
        break;
    case 44100:
        priv_data->gk_i2s_intf.sfreq = AUDIO_SF_44100;
        break;
    case 48000:
        priv_data->gk_i2s_intf.sfreq = AUDIO_SF_48000;
        break;
    default:
        return -EINVAL;
    }

    /* Set clock */
    clksrc = priv_data->gk_i2s_intf.clksrc;
    mclk = priv_data->gk_i2s_intf.mclk;
    mutex_lock(&clock_reg_mutex);
    oversample = priv_data->gk_i2s_intf.oversample;
    // I2S_CLK 频率=gclk_au 频率/2(clk_div+1)
    // clk_div=gclk_au 频率/(I2S_CLK 频率*2) - 1
    // clk_div=gclk_au 频率/(rate*ch*bits*2) - 1
    clock_divider = get_audio_clk_freq()/(params_rate(params)*bit_num*channels*2) - 1; /* div = (clk_au / (2*clk_i2s)) - 1 */

    if(priv_data->AudioOrI2s == I2S_MODE)
    {
        clock_reg = gk_i2s_readl(I2S_CLOCK_REG);
        /* Disable output MCLK to disable external codec */
        /* Enable  output MCLK when dma enable */
        clock_reg &= ~DAI_CLOCK_MASK;
        clock_reg &= (~I2S_CLK_MASTER_MODE);
        clock_reg |= clock_divider;
        if (priv_data->gk_i2s_intf.ms_mode == DAI_MASTER)
        {
            clock_reg |= I2S_CLK_MASTER_MODE;
        }
        else
        {
            clock_reg &= (~I2S_CLK_BCLK_OUTPUT);
        }

        gk_i2s_writel(I2S_CLOCK_REG, clock_reg);
    }

    mutex_unlock(&clock_reg_mutex);

    /*if(priv_data->AudioOrI2s == AUDIO_MODE)
    {
        dai_fifo_rst(priv_data);
    }
    else
    {
        if(!gk_i2s_tstbitsl(I2S_INIT_REG, 0x6))
        {
            dai_fifo_rst(priv_data);
        }
    }*/

    if(substream->stream == SNDRV_PCM_STREAM_CAPTURE)
        dai_rx_enable(priv_data);
    if(substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
        dai_tx_enable(priv_data);

    return 0;
}

static int gk_i2s_prepare(struct snd_pcm_substream *substream,
            struct snd_soc_dai *dai)
{
    struct gk_i2s_priv *priv_data = snd_soc_dai_get_drvdata(dai);
    if(substream->stream == SNDRV_PCM_STREAM_CAPTURE)
        dai_fifo_rx_rst(priv_data);
    if(substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
        dai_fifo_tx_rst(priv_data);
    return 0;
}

static int gk_i2s_trigger(struct snd_pcm_substream *substream, int cmd,
            struct snd_soc_dai *dai)
{
    int ret = 0;
    struct gk_i2s_priv *priv_data = snd_soc_dai_get_drvdata(dai);

    switch (cmd)
    {
    case SNDRV_PCM_TRIGGER_START:
    case SNDRV_PCM_TRIGGER_RESUME:
    case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
        if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
        {
            dai_rx_enable(priv_data);
        }
        else
        {
            //if(alsa_tx_enable_flag == 0)
                dai_tx_enable(priv_data);
        }
        break;
    case SNDRV_PCM_TRIGGER_STOP:
    case SNDRV_PCM_TRIGGER_SUSPEND:
    case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
        if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
        {
            dai_rx_disable(priv_data);
            //Stop by DMA EOC
        }
        else
        {
            dai_tx_disable(priv_data);
            //Stop by DMA EOC
        }
        break;
    default:
        ret = -EINVAL;
        break;
    }

    return ret;
}

/*
 * Set I2S DAI format
 */
static int gk_i2s_set_fmt(struct snd_soc_dai *cpu_dai,
        unsigned int fmt)
{
    struct gk_i2s_priv *priv_data = snd_soc_dai_get_drvdata(cpu_dai);

    switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK)
    {
    case SND_SOC_DAIFMT_LEFT_J:
        priv_data->gk_i2s_intf.mode = DAI_leftJustified_Mode;
        gk_i2s_setbitsl(I2S_RX_CTRL_REG, I2S_RX_WS_INV_BIT);     // I2S_WS=0 rev left data first
        gk_i2s_setbitsl(I2S_TX_CTRL_REG, I2S_TX_WS_INV_BIT);     // I2S_WS=0 transfer left data first
        break;
    case SND_SOC_DAIFMT_RIGHT_J:
        priv_data->gk_i2s_intf.mode = DAI_rightJustified_Mode;
        gk_i2s_setbitsl(I2S_RX_CTRL_REG, I2S_RX_WS_INV_BIT);     // I2S_WS=0 rev left data first
        gk_i2s_setbitsl(I2S_TX_CTRL_REG, I2S_TX_WS_INV_BIT);     // I2S_WS=0 transfer left data first
        break;
    case SND_SOC_DAIFMT_I2S:
        priv_data->gk_i2s_intf.mode = DAI_I2S_Mode;
        gk_i2s_clrbitsl(I2S_RX_CTRL_REG, I2S_RX_WS_INV_BIT);     // I2S_WS=0 rev left data first
        gk_i2s_clrbitsl(I2S_TX_CTRL_REG, I2S_TX_WS_INV_BIT);     // I2S_WS=0 transfer left data first
        break;
    case SND_SOC_DAIFMT_DSP_A:
        priv_data->gk_i2s_intf.mode = DAI_DSP_Mode;
        break;
    default:
        return -EINVAL;
    }
    priv_data->gk_i2s_intf.mode = SND_SOC_DAIFMT_LEFT_J;
    priv_data->gk_i2s_intf.mode = DAI_I2S_Mode;

    switch (fmt & SND_SOC_DAIFMT_MASTER_MASK)
    {
    case SND_SOC_DAIFMT_CBS_CFS:
        priv_data->gk_i2s_intf.ms_mode = DAI_MASTER;
        gk_i2s_setbitsl(I2S_RX_CTRL_REG, I2S_RX_WS_MST_BIT);     // master
        gk_i2s_setbitsl(I2S_TX_CTRL_REG, I2S_TX_WS_MST_BIT);     // master
        break;
    case SND_SOC_DAIFMT_CBM_CFM:
        if (priv_data->gk_i2s_intf.mode != DAI_I2S_Mode)
        {
            printk("DAI can't work in slave mode without standard I2S format!\n");
            return -EINVAL;
        }
        priv_data->gk_i2s_intf.ms_mode = DAI_SLAVE;
        break;
    default:
        return -EINVAL;
    }

    return 0;
}

static int gk_i2s_set_sysclk(struct snd_soc_dai *cpu_dai,
        int clk_id, unsigned int freq, int dir)
{
    struct gk_i2s_priv *priv_data = snd_soc_dai_get_drvdata(cpu_dai);

    switch (clk_id)
    {
    case GOKE_CLKSRC_ON_IPC_CHIP:
        priv_data->gk_i2s_intf.clksrc  = clk_id;
        priv_data->gk_i2s_intf.mclk    = freq;
        break;
    default:
        printk("CLK SOURCE (%d) is not supported yet\n", clk_id);
        return -EINVAL;
    }

    return 0;
}

static int gk_i2s_set_clkdiv(struct snd_soc_dai *cpu_dai,
        int div_id, int div)
{
    struct gk_i2s_priv *priv_data = snd_soc_dai_get_drvdata(cpu_dai);

    switch (div_id)
    {
    case GOKE_CLKDIV_LRCLK:
        priv_data->gk_i2s_intf.oversample = div;
        break;
    default:
        return -EINVAL;
    }

    return 0;
}

static int external_i2s_get_status(struct snd_kcontrol *kcontrol,
    struct snd_ctl_elem_value *ucontrol)
{
    mutex_lock(&clock_reg_mutex);
    ucontrol->value.integer.value[0] = enable_ext_i2s;
    mutex_unlock(&clock_reg_mutex);

    return 0;
}

static int external_i2s_set_status(struct snd_kcontrol *kcontrol,
    struct snd_ctl_elem_value *ucontrol)
{
    u32 clock_reg;

    mutex_lock(&clock_reg_mutex);

    enable_ext_i2s = ucontrol->value.integer.value[0];
    clock_reg = gk_i2s_readl(I2S_CLOCK_REG);
    if (enable_ext_i2s)
        clock_reg |= (I2S_CLK_WS_OUT_EN | I2S_CLK_BCLK_OUT_EN);
    else
        clock_reg &= ~(I2S_CLK_WS_OUT_EN | I2S_CLK_BCLK_OUT_EN);

    gk_i2s_writel(I2S_CLOCK_REG, clock_reg);

    mutex_unlock(&clock_reg_mutex);

    return 1;
}

static const char *i2s_status_str[] = {"Off", "On"};

static const struct soc_enum external_i2s_enum[] =
{
    SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(i2s_status_str), i2s_status_str),
};

static const struct snd_kcontrol_new external_i2s_controls[] =
{
    SOC_ENUM_EXT("External I2S Switch", external_i2s_enum[0],
            external_i2s_get_status, external_i2s_set_status),
};

int gk_i2s_add_controls(struct snd_soc_codec *codec)
{
    return snd_soc_add_codec_controls(codec, external_i2s_controls,
            ARRAY_SIZE(external_i2s_controls));
}
EXPORT_SYMBOL_GPL(gk_i2s_add_controls);


#ifdef CONFIG_PM
static int gk_i2s_dai_suspend(struct snd_soc_dai *dai)
{
    struct gk_i2s_priv *priv_data = snd_soc_dai_get_drvdata(dai);

    if(priv_data->AudioOrI2s == I2S_MODE)
    {
        priv_data->clock_reg = gk_i2s_readl(I2S_CLOCK_REG);
        gk_i2s_writel(I2S_CLOCK_REG, 0x00);
    }
    return 0;
}

static int gk_i2s_dai_resume(struct snd_soc_dai *dai)
{
    struct gk_i2s_priv *priv_data = snd_soc_dai_get_drvdata(dai);

    if(priv_data->AudioOrI2s == I2S_MODE)
    {
        gk_i2s_writel(I2S_CLOCK_REG, priv_data->clock_reg);
    }
    return 0;
}
#else /* CONFIG_PM */
#define gk_i2s_dai_suspend      NULL
#define gk_i2s_dai_resume       NULL
#endif /* CONFIG_PM */

static int gk_i2s_dai_probe(struct snd_soc_dai *dai)
{
    return 0;
}

static int gk_i2s_dai_remove(struct snd_soc_dai *dai)
{
    struct gk_i2s_priv *priv_data = snd_soc_dai_get_drvdata(dai);

    if(priv_data->AudioOrI2s == I2S_MODE)
    {
        /* Disable I2S clock output */
        gk_i2s_clrbitsl(I2S_CLOCK_REG, 0x20);
    }
    return 0;
}

static struct snd_soc_dai_ops gk_i2s_dai_ops =
{
    .prepare    = gk_i2s_prepare,
    .trigger    = gk_i2s_trigger,
    .hw_params  = gk_i2s_hw_params,
    .set_fmt    = gk_i2s_set_fmt,
    .set_sysclk = gk_i2s_set_sysclk,
    .set_clkdiv = gk_i2s_set_clkdiv,
};

// cpu dai SoC的I2S、PCM总线控制器
static struct snd_soc_dai_driver gk_i2s_dai =
{
    .probe      = gk_i2s_dai_probe,
    .remove     = gk_i2s_dai_remove,
    .suspend    = gk_i2s_dai_suspend,
    .resume     = gk_i2s_dai_resume,
    .ops        = &gk_i2s_dai_ops,
    .symmetric_rates = 1,
};

static int __devinit gk_i2s_probe(struct platform_device *pdev)
{
    struct gk_i2s_priv *priv_data = dev_get_drvdata(&pdev->dev);
    memcpy(&gk_i2s_dai.playback, &priv_data->playback, sizeof(gk_i2s_dai.playback));
    memcpy(&gk_i2s_dai.capture,  &priv_data->capture,  sizeof(gk_i2s_dai.capture));

    return snd_soc_register_dai(&pdev->dev, &gk_i2s_dai);
}

static int __devexit gk_i2s_remove(struct platform_device *pdev)
{

    snd_soc_unregister_dai(&pdev->dev);

    return 0;
}

static struct platform_driver gk_i2s_driver = {
    .probe  = gk_i2s_probe,
    .remove = __devexit_p(gk_i2s_remove),

    .driver =
    {
        .name = "gk-i2s",
        .owner = THIS_MODULE,
    },
};

static struct resource gk_i2s0_resources[] = {
    [0] = {
        .start  = I2S_BASE,
        .end    = I2S_BASE + 0x0FFF,
        .flags  = IORESOURCE_MEM,
    },
    [1] = {
        .start  = I2STX_IRQ,
        .end    = I2SRX_IRQ,
        .flags  = IORESOURCE_IRQ,
    },
};

static struct gk_i2s_controller gk_i2s_controller0 = {
    .aucodec_digitalio_0    = NULL,
    .aucodec_digitalio_1    = NULL,
    .aucodec_digitalio_2    = NULL,
    .channel_select         = NULL,
    .set_audio_pll          = NULL,
};

struct platform_device gk_i2s0 = {
    .name               = "gk-i2s",
    .id                 = 0,
    .resource           = gk_i2s0_resources,
    .num_resources      = ARRAY_SIZE(gk_i2s0_resources),
    .dev                =
    {
        .platform_data      = &gk_i2s_controller0,
        .dma_mask           = &gk_dmamask,
        .coherent_dma_mask  = DMA_BIT_MASK(32),
    }
};

static int i2s_set_master(u32 master)
{
    u32 reg_val = 0;
    u32 set_val = 0;

    if(master == I2S_MASTER)
    {
        set_val = I2S_CLK_MASTER_MODE;
    }
    reg_val = gk_i2s_readl(I2S_CLOCK_REG);
    reg_val = (reg_val&(~I2S_CLK_DIV_MASK))|set_val;
    gk_i2s_writel(I2S_CLOCK_REG, reg_val);

    return 0;
}

int __init gk_i2s_init(struct gk_i2s_priv *priv_data)
{
    if(priv_data->AudioOrI2s == I2S_MODE);
    {
        // selsect I2S module
        gk_gpio_setbitsl(GPIO0_BASE + REG_GPIO_PER_SEL_OFFSET, 0x02); //I2S_SEL = 1, slect extern AD/DA.
        // set i2s mclk=12.288MHz
        gk_rct_writel(GK_VA_RCT + 0x024, 0x00124020);
        gk_rct_writel(GK_VA_RCT + 0x078, 0x00c49ba5);
        gk_rct_writel(GK_VA_RCT + 0x084, 0x00000008);
        gk_rct_writel(GK_VA_RCT + 0x080, 0x00000001);

        gk_i2s_writel(I2S_CHANNEL_SELECT_REG, I2S_CHANNEL_2);
        gk_i2s_clrbitsl(I2S_TX_CTRL_REG, I2S_TX_LOOPBACK_BIT);
        gk_i2s_clrbitsl(I2S_RX_CTRL_REG, I2S_RX_WS_INV_BIT);     // I2S_WS=0 rev left data first
        gk_i2s_setbitsl(I2S_RX_CTRL_REG, I2S_RX_WS_MST_BIT);     // master
        gk_i2s_clrbitsl(I2S_TX_CTRL_REG, I2S_TX_WS_INV_BIT);     // I2S_WS=0 transfer left data first
        gk_i2s_setbitsl(I2S_TX_CTRL_REG, I2S_TX_WS_MST_BIT);     // master
        gk_i2s_setbitsl(I2S_TX_CTRL_REG, I2S_TX_UNISON_BIT);     // usion
        i2s_set_master(I2S_MASTER);
    }
    platform_device_register(&gk_i2s0);
    device_set_wakeup_capable(&gk_i2s0.dev, 1);
    device_set_wakeup_enable(&gk_i2s0.dev, 0);

    priv_data->controller_info = gk_i2s0.dev.platform_data;

    dev_set_drvdata(&gk_i2s0.dev, priv_data);

    return platform_driver_register(&gk_i2s_driver);
}

void __exit gk_i2s_exit(void)
{
    platform_driver_unregister(&gk_i2s_driver);
}

MODULE_AUTHOR("<yulindeng@goke.com>");
MODULE_DESCRIPTION("GK IPC Board with Soc I2S Interface for ALSA");
MODULE_LICENSE("GPL");
MODULE_ALIAS("snd-soc-gkipc");

