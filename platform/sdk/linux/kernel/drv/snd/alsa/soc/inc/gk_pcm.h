/*!
*****************************************************************************
** \file        linux/kernel/drv/webcam/src/sound/soc/goke/gk_pcm.h
**
** \version     $Id: gk_pcm.h 13657 2018-05-14 02:54:03Z yulindeng $
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
#ifndef _GK_PCM_H_
#define _GK_PCM_H_

struct gk_pcm_dma_params
{
    char *name;             /* stream identifier */
    u32 dev_addr;           /* device physical address for DMA */
};

#define GOKE_CLKSRC_ON_IPC_CHIP         0
#define GOKE_CLKDIV_LRCLK               0
#define GOKE1_CLKDIV_LRCLK              1
int __init snd_gk_pcm_init(void);
void __exit snd_gk_pcm_exit(void);
int gk_ipcam_init(struct snd_soc_pcm_runtime *rtd);

#endif /* _GK_PCM_H_ */

