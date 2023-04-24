/*!
*****************************************************************************
** \file        linux/kernel/drv/webcam/src/sound/soc/goke/gk_xf_es8388.c
**
** \version     $Id: snd-gk_pcm.c 13657 2018-05-14 02:54:03Z yulindeng $
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

#include "gk_i2s.c"
#include "gk_pcm.c"

#define GK_DEBUG_PRINTK         printk

extern struct gk_i2s_priv gk_i2s_priv_data;
extern struct snd_soc_card snd_soc_card_ipcam;
static struct platform_device *gk_ipcam_snd_device;

static int __init gk_ipcam_board_init(void)
{
    int errorCode = 0;
    GK_DEBUG_PRINTK("[%s %s %d] %d\n", __FILE__, __func__, __LINE__, errorCode);
    if(gk_i2s_priv_data.gpio_LRCLK)
    {
        gk_gpio_set_out(gk_i2s_priv_data.gpio_LRCLK, 0x00);
    }
    GK_DEBUG_PRINTK("[%s %s %d] %d\n", __FILE__, __func__, __LINE__, errorCode);
    errorCode = snd_gk_pcm_init();
    if (errorCode < 0)
    {
        GK_DEBUG_PRINTK(KERN_ERR "Could not init snd_gk_pcm_init\n");
        goto ipcam_board_init_exit2;
    }
    GK_DEBUG_PRINTK("[%s %s %d] %d\n", __FILE__, __func__, __LINE__, errorCode);
    errorCode = gk_i2s_init(&gk_i2s_priv_data);
    if (errorCode < 0)
    {
        GK_DEBUG_PRINTK(KERN_ERR "Could not init gk_i2s_init\n");
        goto ipcam_board_init_exit1;
    }

    GK_DEBUG_PRINTK("[%s %s %d] %d\n", __FILE__, __func__, __LINE__, errorCode);
    gk_ipcam_snd_device = platform_device_alloc("soc-audio", -1);
    if (!gk_ipcam_snd_device)
    {
        errorCode = -ENOMEM;
        goto ipcam_board_init_exit0;
    }

    GK_DEBUG_PRINTK("[%s %s %d] %d\n", __FILE__, __func__, __LINE__, errorCode);
    platform_set_drvdata(gk_ipcam_snd_device, &snd_soc_card_ipcam);

    GK_DEBUG_PRINTK("[%s %s %d] %d\n", __FILE__, __func__, __LINE__, errorCode);

    // soc-audio soc-audio: ASoC machine GPIPcam should use snd_soc_register_card()
    // asoc: es8388 <-> gk-i2s.0 mapping ok
    errorCode = platform_device_add(gk_ipcam_snd_device);
    // snd_soc_register_card would auto called by soc_probe
    // snd_soc_register_card(&snd_soc_card_ipcam);

    GK_DEBUG_PRINTK("[%s %s %d] %d\n", __FILE__, __func__, __LINE__, errorCode);
    if (errorCode)
    {
        platform_device_put(gk_ipcam_snd_device);
        goto ipcam_board_init_exit0;
    }

    return 0;

ipcam_board_init_exit0:
    gk_i2s_exit();
ipcam_board_init_exit1:
    snd_gk_pcm_exit();
ipcam_board_init_exit2:
    return errorCode;
}

static void __exit gk_ipcam_board_exit(void)
{
    GK_DEBUG_PRINTK("[%s %s %d]\n", __FILE__, __func__, __LINE__);
    gk_i2s_exit();
    snd_gk_pcm_exit();
    platform_device_unregister(gk_ipcam_snd_device);
}

module_init(gk_ipcam_board_init);
module_exit(gk_ipcam_board_exit);

MODULE_AUTHOR("<yulindeng@goke.com>");
MODULE_DESCRIPTION("GK IPC snd-gk_pcm with Soc I2S Interface for ALSA");
MODULE_LICENSE("GPL");
MODULE_ALIAS("snd-gk_pcm");

