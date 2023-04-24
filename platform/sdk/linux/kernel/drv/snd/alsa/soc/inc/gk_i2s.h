/*!
*****************************************************************************
** \file        linux/kernel/drv/webcam/src/sound/soc/goke/gk_i2s.h
**
** \version     $Id: gk_i2s.h 13657 2018-05-14 02:54:03Z yulindeng $
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

#ifndef _GK_I2S_H_
#define _GK_I2S_H_
#include <sound/soc.h>


enum audio_i2s_mode{
    AUDIO_MODE  = 0,
    I2S_MODE    = 1,
};

enum i2s_master_mode{
    I2S_SLAVE  = 0,
    I2S_MASTER = 1,
};

enum i2s_channel{
    I2S_CHANNEL_2 = 0,     //!< I2S  2 channel.
    I2S_CHANNEL_4 = 1,     //!< I2S  4 channel.
    I2S_CHANNEL_6 = 2,     //!< I2S  6 channel.
};

enum i2s_wlen{
    I2S_16BIT = 16,     //!< 32kHz datarate.
    I2S_24BIT = 24,     //!< 48kHz datarate.
    I2S_32BIT = 32,     //!< 64kHz datarate.
};

enum i2s_event{
    I2S_EVENT_FRAME          = 1, /* received one audio frame or send finished one audio frame */
    I2S_EVENT_WILL_OVERFLOW  = 2,
    I2S_EVENT_WILL_UNDERFLOW = 3,
    I2S_EVENT_ALREADY_OVERFLOW  = 4,
    I2S_EVENT_ALREADY_UNDERFLOW = 5,
    I2S_EVENT_UNDEFINED         = 6,
};

struct gk_i2s_priv
{
    struct  gk_i2s_controller *controller_info;
    struct  gk_i2s_interface gk_i2s_intf;
    struct  snd_soc_pcm_stream capture;
    struct  snd_soc_pcm_stream playback;
    u32     clock_reg;
    u32     gpio_LRCLK;
    u32     gpio_SPK;
    u32     use_pio;
    enum audio_i2s_mode  AudioOrI2s;
};

int gk_i2s_add_controls(struct snd_soc_codec *codec);
int __init gk_i2s_init(struct gk_i2s_priv *priv_data);
void __init gk_i2s_exit(void);

#endif /* _GK_I2S_H_ */

