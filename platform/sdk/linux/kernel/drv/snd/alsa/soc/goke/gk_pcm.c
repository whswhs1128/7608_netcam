/*!
*****************************************************************************
** \file        linux/kernel/drv/webcam/src/sound/soc/goke/gk_pcm.c
**
** \version     $Id: gk_pcm.c 13696 2018-05-31 06:55:18Z yulindeng $
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

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>

#include <mach/i2s.h>
#include <plat/dma.h>
#include <mach/io.h>
#include <mach/audio_codec.h>
#include CONFIG_GK_CHIP_INCLUDE_FILE

#include "../inc/gk_pcm.h"

#define GK_MAX_DESC_NUM         128
#define GK_MIN_DESC_NUM         2
#define GK_PERIOD_BYTES_MAX     (128 * 1024)
#define GK_PERIOD_BYTES_MIN     32
#define GK_BUFFER_BYTES_MAX     (256 * 1024)

extern struct gk_i2s_priv gk_i2s_priv_data;

struct gk_runtime_data {
    struct gk_pcm_dma_params *dma_data;

    gk_dma_req_t*   dma_desc_array;     // FIXME this
    dma_addr_t      dma_desc_array_phys;
    int             channel;            /* Physical DMA channel */
    int             ndescr;             /* Number of descriptors */
    int             last_descr;         /* Record lastest DMA done descriptor number */
    int             ncount;             /* Record buffer*/
    int             rxfifo;

    u32*            dma_rpt_buf;
    dma_addr_t      dma_rpt_phys;

    spinlock_t      lock;
};

static const struct snd_pcm_hardware gk_pcm_hardware =
{
    .info               = SNDRV_PCM_INFO_INTERLEAVED |
                          SNDRV_PCM_INFO_BLOCK_TRANSFER |
                          SNDRV_PCM_INFO_MMAP |
                          SNDRV_PCM_INFO_MMAP_VALID |
                          SNDRV_PCM_INFO_PAUSE |
                          SNDRV_PCM_INFO_RESUME |
                          SNDRV_PCM_INFO_BATCH,
#ifdef CONFIG_SND_SOC_GOKE_AUDC
    .formats            = SNDRV_PCM_FMTBIT_S16_LE,
    .rates              = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000,
    .rate_min           = 8000,
    .rate_max           = 48000,
    .channels_min       = 1,
    .channels_max       = 2,
#else
    // 因为24bit的数据处理很麻烦，因此alsa不再支持 SNDRV_PCM_FMTBIT_S24_LE
    // 8K采样率由于分频器原因，不能支持
    .formats            = SNDRV_PCM_FMTBIT_S16_LE,
    .rates              = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000,
    .rate_min           = 8000,
    .rate_max           = 192000,
    .channels_min       = 1,
    .channels_max       = 4,
#endif
    .period_bytes_min   = GK_PERIOD_BYTES_MIN,
    .period_bytes_max   = GK_PERIOD_BYTES_MAX,
    .periods_min        = GK_MIN_DESC_NUM,
    .periods_max        = GK_MAX_DESC_NUM,
    .buffer_bytes_max   = GK_BUFFER_BYTES_MAX,
};

static int gk_dai_dma_start(struct gk_runtime_data *prtd)
{

    if(gk_i2s_priv_data.use_pio)
    {
        gk_i2s_writel(I2S_RX_FIFO_GTH_REG, prtd->rxfifo / gk_i2s_priv_data.gk_i2s_intf.ch);
        gk_i2s_setbitsl(I2S_INIT_REG, I2S_FIFO_RESET_BIT);
        while(gk_i2s_tstbitsl(I2S_INIT_REG, I2S_FIFO_RESET_BIT))
        {
            printk("DAI_FIFO_RST fail!\n");
        }
        gk_i2s_writel(I2S_RX_FIFO_GTH_REG, prtd->rxfifo / gk_i2s_priv_data.gk_i2s_intf.ch);
        gk_i2s_setbitsl(I2S_RX_INT_ENABLE_REG, 0x04);
    }
    else
    {
        return gk_dma_desc_xfr(
            prtd->dma_desc_array_phys + sizeof(gk_dma_req_t) * prtd->last_descr,
            prtd->channel);
    }
    return 0;
}

struct I2S_IRQ_InfoT
{
    unsigned int    irq;
    unsigned long   flags;
    char            name[0x20];
    struct snd_pcm_substream *substream;
};
static struct I2S_IRQ_InfoT i2s_irq =
{
    .irq = I2SRX_IRQ,
    .name = "i2s_rx",
};
static struct gk_runtime_data *Playprtd   = NULL;
static struct gk_runtime_data *Recordprtd = NULL;

static int gk_dai_dma_stop(struct gk_runtime_data *prtd)
{
    if(gk_i2s_priv_data.use_pio)
    {
        gk_i2s_clrbitsl(I2S_RX_INT_ENABLE_REG, 0x04);
    }
    else
    {
        return gk_dma_desc_stop(prtd->channel);
    }
    return 0;
}

#define PRT_CNT 256
static void dai_dma_handler(void *dev_id)
{
    u32 *rpt;
    int cur_descr;
    struct snd_pcm_substream *substream = dev_id;
    struct snd_pcm_runtime *runtime = substream->runtime;
    struct gk_runtime_data *prtd = runtime->private_data;
    static unsigned int cnt_irq = 0;

    cur_descr = prtd->last_descr;

    if(cnt_irq++ % PRT_CNT == 0)
    {
        if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
            printk("[%s %s %d]%d PLAYBACK stream, cur_descr:%d\n", __FILE__, __func__, __LINE__, cnt_irq, cur_descr);
    }

    prtd->last_descr++;
    if (prtd->last_descr >= prtd->ndescr) {
        prtd->last_descr = 0;
    }
    //snd_pcm_period_elapsed(substream);

    /* Check if stop dma chain */
    rpt = &prtd->dma_rpt_buf[cur_descr];
    if (*rpt & 0x10000000)
    {
        /* Descriptor chain done */
        *rpt &= (~0x10000000);
        if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
        {

#ifdef CONFIG_SND_SOC_GOKE_AUDC
            gk_aud_clrbitsl(AUDC_DIGITAL_FIFO_CTRL_REG, 0x10);
            gk_aud_setbitsl(AUDC_DIGITAL_FIFO_CTRL_REG, 0x80);
#else
            /* dai_tx_disable() */
            gk_i2s_clrbitsl(I2S_INIT_REG, 0x4);

            /* dai_fifo_rst() */
            if(!gk_i2s_tstbitsl(I2S_INIT_REG, 0x06))
            {
                gk_i2s_setbitsl(I2S_INIT_REG, 0x11);
            }
#endif
        }
        else
        {
#ifdef CONFIG_SND_SOC_GOKE_AUDC
            gk_aud_clrbitsl(AUDC_DIGITAL_FIFO_CTRL_REG, 0x1000);
            gk_aud_setbitsl(AUDC_DIGITAL_FIFO_CTRL_REG, 0x8000);
#else
            /* dai_rx_disable() */
            gk_i2s_clrbitsl(I2S_INIT_REG, 0x2);

            /* dai_fifo_rst() */
            if(!gk_i2s_tstbitsl(I2S_INIT_REG, 0x06))
            {
                gk_i2s_setbitsl(I2S_INIT_REG, 0x11);
            }
#endif
        }
    }
    if(*rpt & 0x8000000)
    {
        *rpt &= (~0x8000000);
    }
}

static u8 g_elapsed = 1;

void gk_snd_pcm_period_mode(u8 elapsed)
{
    g_elapsed = elapsed;
}
EXPORT_SYMBOL(gk_snd_pcm_period_mode);

static void dai_rx_dma_handler(void *dev_id, u32 status)
{
    struct snd_pcm_substream *substream = dev_id;
    // 2dot/sec
    // 0x2000*32bytes=262144
    // 16K*32bits*4ch=256000
    // StevenYu: when used as usb mic, cannot call this func.
    // StevenYu: when used as arecord, must call this func.
    if(g_elapsed)
    {
        snd_pcm_period_elapsed(substream);
    }
    dai_dma_handler(dev_id);
}

static irqreturn_t gk_dai_rx_irq(int irqno, void *dev_id)
{
    u32 i;
    u32 src;
    u16* dst;
    unsigned long flags;

    struct I2S_IRQ_InfoT* dev = (struct I2S_IRQ_InfoT*)dev_id;
    struct snd_pcm_substream *substream = dev->substream;
    struct snd_pcm_runtime *runtime = substream->runtime;
    struct gk_runtime_data *prtd = runtime->private_data;
    gk_dma_req_t *dma_desc;
    spin_lock_irqsave(&prtd->lock, flags);
    // 2dot/sec
    // 0x2000*32bytes=262144
    // 16K*32bits*4ch=256000
    {
        dma_desc = &prtd->dma_desc_array[prtd->last_descr];
        if(prtd->ncount >= dma_desc->xfr_count)
        {
            prtd->last_descr++;
            if (prtd->last_descr >= prtd->ndescr) {
                prtd->last_descr = 0;
            }
            prtd->ncount = 0;
            dma_desc = &prtd->dma_desc_array[prtd->last_descr];
            // StevenYu: when used as usb mic, cannot call this func.
            // StevenYu: when used as arecord, must call this func.
            if(g_elapsed)
            {
                snd_pcm_period_elapsed(substream);
            }
        }
        src = gk_phys_to_virt(dma_desc->src + gk_aud_get_dma_offset());
        dst = gk_phys_to_virt(dma_desc->dst + prtd->ncount);
        //printk("[%s %s %d]0x%08x(0x%08x) 0x%08x(0x%08x) 0x%08x 0x%08x 0x%08x\n", __FILE__, __func__, __LINE__,
        //    dst, dma_desc->dst, src, dma_desc->src, dma_desc->xfr_count, prtd->rxfifo, prtd->ncount);
        prtd->ncount += prtd->rxfifo / 2;
        if(gk_i2s_priv_data.capture.channels_max == 0x04)
        {
            if(gk_i2s_priv_data.gk_i2s_intf.ch == 0x04)
            {
                for(i=0;i<(prtd->rxfifo / 4);i++)
                {
                    *dst++ = gk_i2s_readl(src);
                }
            }
            else if(gk_i2s_priv_data.gk_i2s_intf.ch == 0x02)
            {
                for(i=0;i<(prtd->rxfifo / 4);i++)
                {
                    *dst++ = gk_i2s_readl(src);
                    gk_i2s_readl(src);
                }
            }
            else if(gk_i2s_priv_data.gk_i2s_intf.ch == 0x01)
            {
                for(i=0;i<(prtd->rxfifo / 4);i++)
                {
                    *dst++ = gk_i2s_readl(src);
                    gk_i2s_readl(src);
                    gk_i2s_readl(src);
                    gk_i2s_readl(src);
                }
            }
        }
        else if(gk_i2s_priv_data.capture.channels_max == 0x02)
        {
            if(gk_i2s_priv_data.gk_i2s_intf.ch == 0x02)
            {
                for(i=0;i<(prtd->rxfifo / 4);i++)
                {
                    *dst++ = gk_i2s_readl(src);
                }
            }
            else if(gk_i2s_priv_data.gk_i2s_intf.ch == 0x01)
            {
                for(i=0;i<(prtd->rxfifo / 4);i++)
                {
                    *dst++ = gk_i2s_readl(src);
                    gk_i2s_readl(src);
                }
            }
        }
        else if(gk_i2s_priv_data.capture.channels_max == 0x01)
        {
            for(i=0;i<(prtd->rxfifo / 4);i++)
            {
                *dst++ = gk_i2s_readl(src);
            }
        }
    }
    spin_unlock_irqrestore(&prtd->lock, flags);
    return 0;
}

static void dai_tx_dma_handler(void *dev_id, u32 status)
{
    struct snd_pcm_substream *substream = dev_id;
    snd_pcm_period_elapsed(substream);
    dai_dma_handler(dev_id);
}

/* this may get called several times by oss emulation */
static int gk_pcm_hw_params(struct snd_pcm_substream *substream,
    struct snd_pcm_hw_params *params)
{
    struct snd_pcm_runtime *runtime = substream->runtime;
    struct gk_runtime_data *prtd = runtime->private_data;
    struct snd_soc_pcm_runtime *rtd = substream->private_data;
    struct gk_pcm_dma_params *dma_data;
    size_t totsize = params_buffer_bytes(params);
    size_t period = params_period_bytes(params);
    gk_dma_req_t *dma_desc;
    dma_addr_t dma_buff_phys, next_desc_phys, next_rpt;
    unsigned long flags;
    int ret, i;
    unsigned char *pVm_data = NULL;

    printk("[%s %s %d]totsize:%d, dma_buffer.bytes:%d\n", __FILE__, __func__, __LINE__, totsize, substream->dma_buffer.bytes);

    dma_data = snd_soc_dai_get_dma_data(rtd->cpu_dai, substream);
    if (!dma_data)
        return -ENODEV;

    snd_pcm_set_runtime_buffer(substream, &substream->dma_buffer);
    runtime->dma_bytes = totsize;
    pVm_data = runtime->dma_area;  // by yke

    if (prtd->dma_data)
        return 0;

    prtd->dma_data = dma_data;

    if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
    {
        printk("[%s %s %d] SNDRV_PCM_STREAM_PLAYBACK Tx\n", __FILE__, __func__, __LINE__);
        prtd->channel = I2S_TX_DMA_CHAN;
        ret = gk_dma_request_irq(I2S_TX_DMA_CHAN,
                dai_tx_dma_handler, substream);
        if (ret < 0)
            return ret;
        ret = gk_dma_enable_irq(I2S_TX_DMA_CHAN,
                dai_tx_dma_handler);
        if (ret < 0)
            return ret;
    }
    else
    {
        printk("[%s %s %d] SNDRV_PCM_STREAM_CAPTURE Rx\n", __FILE__, __func__, __LINE__);
        prtd->channel = I2S_RX_DMA_CHAN;
        if(gk_i2s_priv_data.use_pio)//gk_i2s_priv_data.capture.channels_max != params_channels(params)
        {
            i2s_irq.substream = substream;
            ret = request_irq(I2SRX_IRQ, gk_dai_rx_irq, IRQF_SHARED | IRQF_TRIGGER_HIGH, i2s_irq.name, &i2s_irq);
            if (ret < 0)
                return ret;
        }
        else
        {
            ret = gk_dma_request_irq(I2S_RX_DMA_CHAN,
                    dai_rx_dma_handler, substream);
            if (ret < 0)
                return ret;
            ret = gk_dma_enable_irq(I2S_RX_DMA_CHAN,
                    dai_rx_dma_handler);
            if (ret < 0)
                return ret;
        }
    }

    spin_lock_irqsave(&prtd->lock, flags);

    dma_desc = prtd->dma_desc_array;
    next_desc_phys = prtd->dma_desc_array_phys;
    next_rpt = prtd->dma_rpt_phys;
    dma_buff_phys = runtime->dma_addr;

    prtd->ndescr = 0;
    prtd->last_descr = 0;
    prtd->ncount = 0;
    prtd->rxfifo = 0x40;
    printk("[%s %s %d]0x%08x %d*%d*, period:%d, totsize:%d\n", __FILE__, __func__, __LINE__,
                        dma_buff_phys, params_rate(params), params_channels(params), period, totsize);
    i = 0;
    do
    {
        next_desc_phys += sizeof(gk_dma_req_t);
        dma_desc->next = (struct gk_dma_req_s *) next_desc_phys;

        if (period > totsize)
            period = totsize;

        if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
        {
            switch (params_format(params))
            {
            case SNDRV_PCM_FORMAT_S16_LE:
                if(gk_i2s_priv_data.playback.channels_max == 0x01)
                {
                    dma_desc->attr = DMA_DESC_NI | DMA_DESC_TS_2B |
                        DMA_DESC_BLK_8B | DMA_DESC_IE | DMA_DESC_ST |
                        DMA_DESC_ID | DMA_DESC_RM;
                }
                else
                {
                    dma_desc->attr = DMA_DESC_NI | DMA_DESC_TS_4B |
                        DMA_DESC_BLK_32B | DMA_DESC_IE | DMA_DESC_ST |
                        DMA_DESC_ID | DMA_DESC_RM;
                }
                break;
            case SNDRV_PCM_FORMAT_S24_LE:
                dma_desc->attr = DMA_DESC_NI | DMA_DESC_TS_4B |
                    DMA_DESC_BLK_32B | DMA_DESC_IE | DMA_DESC_ST |
                    DMA_DESC_ID | DMA_DESC_RM;
                break;
            }
            dma_desc->src = dma_buff_phys;
            dma_desc->dst = gk_virt_to_phys(prtd->dma_data->dev_addr) - gk_aud_get_dma_offset();
        }
        else
        {
            switch (params_format(params))
            {
            case SNDRV_PCM_FORMAT_S16_LE:
                if(gk_i2s_priv_data.capture.channels_max == 0x01)
                {
                    // evb OK
                    dma_desc->attr = DMA_DESC_NI | DMA_DESC_TS_2B |
                        DMA_DESC_BLK_8B | DMA_DESC_IE | DMA_DESC_ST |
                        DMA_DESC_ID | DMA_DESC_WM;
                }
                else
                {
                    dma_desc->attr = DMA_DESC_NI | DMA_DESC_TS_2B |
                        DMA_DESC_BLK_32B | DMA_DESC_IE | DMA_DESC_ST |
                        DMA_DESC_ID | DMA_DESC_WM;
                }
                break;
            case SNDRV_PCM_FORMAT_S24_LE:
                dma_desc->attr = DMA_DESC_NI | DMA_DESC_TS_4B |
                    DMA_DESC_BLK_32B | DMA_DESC_IE | DMA_DESC_ST |
                    DMA_DESC_ID | DMA_DESC_WM;
                break;
            }
            dma_desc->src = gk_virt_to_phys(prtd->dma_data->dev_addr) - gk_aud_get_dma_offset();
            dma_desc->dst = dma_buff_phys;
        }
        dma_desc->xfr_count = period;
        dma_desc->rpt = next_rpt;

        dma_buff_phys += period;
        next_rpt += sizeof(dma_addr_t);
        dma_desc++;
        prtd->ndescr++;
        i++;
        pVm_data += period;
    } while (totsize -= period);
    dma_desc[-1].next = (struct gk_dma_req_s *) prtd->dma_desc_array_phys;

    for (i = 0; i < prtd->ndescr; i++)
        prtd->dma_rpt_buf[i] = 0;

    spin_unlock_irqrestore(&prtd->lock, flags);

    return 0;
}

static int gk_pcm_hw_free(struct snd_pcm_substream *substream)
{
    struct snd_pcm_runtime *runtime = substream->runtime;
    struct gk_runtime_data *prtd = runtime->private_data;

    if (prtd->dma_data) {
        /* Disable and free DMA irq */
        if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
        {
            gk_dma_disable_irq(I2S_TX_DMA_CHAN, dai_tx_dma_handler);
            gk_dma_free_irq(I2S_TX_DMA_CHAN, dai_tx_dma_handler);
        }
        else
        {
            if(gk_i2s_priv_data.use_pio)
            {
                free_irq(I2SRX_IRQ, &i2s_irq);
            }
            else
            {
                gk_dma_disable_irq(I2S_RX_DMA_CHAN, dai_rx_dma_handler);
                gk_dma_free_irq(I2S_RX_DMA_CHAN, dai_rx_dma_handler);
            }
        }
        prtd->dma_data = NULL;
        prtd->ndescr = 0;
        prtd->last_descr = 0;
    }

    /* TODO - do we need to ensure DMA flushed */
    snd_pcm_set_runtime_buffer(substream, NULL);

    return 0;
}

static int gk_pcm_prepare(struct snd_pcm_substream *substream)
{
    struct gk_runtime_data *prtd = substream->runtime->private_data;

    /* Ensure dma is stopped */
    gk_dai_dma_stop(prtd);

    return 0;
}

static int gk_pcm_trigger(struct snd_pcm_substream *substream, int cmd)
{
    struct snd_pcm_runtime *runtime = substream->runtime;
    struct gk_runtime_data *prtd = runtime->private_data;
    unsigned long flags;
    int ret = 0;

    spin_lock_irqsave(&prtd->lock, flags);

    switch(cmd)
    {
    case SNDRV_PCM_TRIGGER_START:
    case SNDRV_PCM_TRIGGER_RESUME:
    case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
        gk_dai_dma_start(prtd);
        break;
    case SNDRV_PCM_TRIGGER_STOP:
    case SNDRV_PCM_TRIGGER_SUSPEND:
    case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
        gk_dai_dma_stop(prtd);
        break;

    default:
        ret = -EINVAL;
        break;
    }

    spin_unlock_irqrestore(&prtd->lock, flags);

    return ret;
}

static snd_pcm_uframes_t gk_pcm_pointer(
    struct snd_pcm_substream *substream)
{
    struct snd_pcm_runtime *runtime = substream->runtime;
    struct gk_runtime_data *prtd = runtime->private_data;

    return prtd->last_descr * runtime->period_size;
}

static int gk_pcm_open(struct snd_pcm_substream *substream)
{
    struct snd_pcm_runtime *runtime = substream->runtime;
    int ret = 0;


    snd_soc_set_runtime_hwparams(substream, &gk_pcm_hardware);

    /* Add a rule to enforce the DMA buffer align. */
    ret = snd_pcm_hw_constraint_step(runtime, 0,
        SNDRV_PCM_HW_PARAM_PERIOD_BYTES, 32);
    if (ret)
        goto gk_pcm_open_exit;

    ret = snd_pcm_hw_constraint_step(runtime, 0,
        SNDRV_PCM_HW_PARAM_BUFFER_BYTES, 32);
    if (ret)
        goto gk_pcm_open_exit;

    ret = snd_pcm_hw_constraint_integer(runtime,
        SNDRV_PCM_HW_PARAM_PERIODS);
    if (ret < 0)
        goto gk_pcm_open_exit;

    if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
    {
        runtime->private_data = Playprtd;
    }
    else
    {
        runtime->private_data = Recordprtd;
    }

gk_pcm_open_exit:
    return ret;
}

static int gk_pcm_close(struct snd_pcm_substream *substream)
{
    struct snd_pcm_runtime *runtime = substream->runtime;

    runtime->private_data = NULL;
    return 0;
}

static int gk_pcm_mmap(struct snd_pcm_substream *substream,
    struct vm_area_struct *vma)
{
    struct snd_pcm_runtime *runtime = substream->runtime;

    return dma_mmap_coherent(substream->pcm->card->dev, vma,
        runtime->dma_area, runtime->dma_addr, runtime->dma_bytes);
}

static struct snd_pcm_ops gk_pcm_ops =
{
    .open       = gk_pcm_open,
    .close      = gk_pcm_close,
    .ioctl      = snd_pcm_lib_ioctl,
    .hw_params  = gk_pcm_hw_params,
    .hw_free    = gk_pcm_hw_free,
    .prepare    = gk_pcm_prepare,
    .trigger    = gk_pcm_trigger,
    .pointer    = gk_pcm_pointer,
    .mmap       = gk_pcm_mmap,
};

static int gk_pcm_preallocate_dma_buffer(struct snd_pcm *pcm, int stream)
{
    struct snd_pcm_substream *substream = pcm->streams[stream].substream;
    struct snd_dma_buffer *buf = &substream->dma_buffer;
    size_t size = gk_pcm_hardware.buffer_bytes_max;

    buf->dev.type = SNDRV_DMA_TYPE_DEV;
    buf->dev.dev = pcm->card->dev;
    buf->private_data = NULL;
    buf->area = dma_alloc_writecombine(pcm->card->dev, size,
                       &buf->addr, GFP_KERNEL);
    if (!buf->area)
        return -ENOMEM;
    buf->bytes = size;
    return 0;
}

static void gk_pcm_free_dma_buffers(struct snd_pcm *pcm)
{
    struct snd_pcm_substream *substream;
    struct snd_dma_buffer *buf;
    int stream;

    for (stream = 0; stream <= SNDRV_PCM_STREAM_LAST; stream++)
    {
        substream = pcm->streams[stream].substream;
        if (!substream)
            continue;

        buf = &substream->dma_buffer;
        if (!buf->area)
            continue;

        dma_free_writecombine(pcm->card->dev, buf->bytes,
            buf->area, buf->addr);
        buf->area = NULL;
        buf->addr = (dma_addr_t)NULL;
    }
    if(Recordprtd && Recordprtd->dma_rpt_buf)
    {
        dma_free_coherent(pcm->card->dev,
            GK_MAX_DESC_NUM * sizeof(Recordprtd->dma_rpt_buf),
            Recordprtd->dma_rpt_buf, Recordprtd->dma_rpt_phys);
    }
    if(Recordprtd && Recordprtd->dma_desc_array)
    {
        dma_free_coherent(pcm->card->dev,
            GK_MAX_DESC_NUM * sizeof(gk_dma_req_t),
            Recordprtd->dma_desc_array, Recordprtd->dma_desc_array_phys);
    }
    if(Recordprtd)
    {
        kfree(Recordprtd);
        Recordprtd = NULL;
    }
    if(Playprtd && Playprtd->dma_rpt_buf)
    {
        dma_free_coherent(pcm->card->dev,
            GK_MAX_DESC_NUM * sizeof(Playprtd->dma_rpt_buf),
            Playprtd->dma_rpt_buf, Playprtd->dma_rpt_phys);
    }
    if(Playprtd && Playprtd->dma_desc_array)
    {
        dma_free_coherent(pcm->card->dev,
            GK_MAX_DESC_NUM * sizeof(gk_dma_req_t),
            Playprtd->dma_desc_array, Playprtd->dma_desc_array_phys);
    }
    if(Playprtd)
    {
        kfree(Playprtd);
        Playprtd = NULL;
    }
}

static int gk_pcm_new(struct snd_soc_pcm_runtime *rtd)
{
    int ret = 0;
    struct snd_card *card;
    struct snd_soc_dai *dai;
    struct snd_pcm *pcm;

    card = rtd->card->snd_card;
    dai = rtd->codec_dai;
    pcm=rtd->pcm;

    card->dev->dma_mask = &gk_dmamask;

    card->dev->coherent_dma_mask = gk_dmamask;

    if (dai->driver->playback.channels_min)
    {
        ret = gk_pcm_preallocate_dma_buffer(pcm, SNDRV_PCM_STREAM_PLAYBACK);
        if (ret)
            goto gk_pcm_new_exit;

        Playprtd = kzalloc(sizeof(struct gk_runtime_data), GFP_KERNEL);
        if (Playprtd == NULL)
        {
            ret = -ENOMEM;
            goto gk_pcm_new_exit;
        }

        Playprtd->dma_desc_array = dma_alloc_coherent(pcm->card->dev,
            GK_MAX_DESC_NUM * sizeof(gk_dma_req_t),
            &Playprtd->dma_desc_array_phys, GFP_KERNEL);
        if (!Playprtd->dma_desc_array)
        {
            ret = -ENOMEM;
            goto gk_pcm_new_play_free_prtd;
        }

        Playprtd->dma_rpt_buf = dma_alloc_coherent(pcm->card->dev,
                        GK_MAX_DESC_NUM * sizeof(Playprtd->dma_rpt_buf),
                        &Playprtd->dma_rpt_phys, GFP_KERNEL);
        if(Playprtd->dma_rpt_buf == NULL)
        {
            dev_err(pcm->card->dev,
                "No memory for dma_rpt_buf\n");
            goto gk_pcm_new_play_free_dma_desc_array;
        }

        spin_lock_init(&Playprtd->lock);
    }

    if (dai->driver->capture.channels_min)
    {
        ret = gk_pcm_preallocate_dma_buffer(pcm, SNDRV_PCM_STREAM_CAPTURE);
        if (ret)
            goto gk_pcm_new_play_exit;

        Recordprtd = kzalloc(sizeof(struct gk_runtime_data), GFP_KERNEL);
        if (Recordprtd == NULL)
        {
            ret = -ENOMEM;
            goto gk_pcm_new_play_free_dma_desc_array;
        }

        Recordprtd->dma_desc_array = dma_alloc_coherent(pcm->card->dev,
            GK_MAX_DESC_NUM * sizeof(gk_dma_req_t),
            &Recordprtd->dma_desc_array_phys, GFP_KERNEL);
        if (!Recordprtd->dma_desc_array)
        {
            ret = -ENOMEM;
            goto gk_pcm_new_record_free_prtd;
        }

        Recordprtd->dma_rpt_buf = dma_alloc_coherent(pcm->card->dev,
                        GK_MAX_DESC_NUM * sizeof(Recordprtd->dma_rpt_buf),
                        &Recordprtd->dma_rpt_phys, GFP_KERNEL);
        if(Recordprtd->dma_rpt_buf == NULL)
        {
            dev_err(pcm->card->dev,
                "No memory for dma_rpt_buf\n");
            goto gk_pcm_new_record_free_dma_desc_array;
        }
        spin_lock_init(&Recordprtd->lock);
    }
    goto gk_pcm_new_exit;

gk_pcm_new_record_free_dma_desc_array:
    if(Recordprtd)
    {
        dma_free_coherent(pcm->card->dev,
            GK_MAX_DESC_NUM * sizeof(gk_dma_req_t),
            Recordprtd->dma_desc_array, Recordprtd->dma_desc_array_phys);
    }
gk_pcm_new_record_free_prtd:
    if(Recordprtd)
    {
        kfree(Recordprtd);
        Recordprtd = NULL;
    }
gk_pcm_new_play_exit:
    if(Playprtd && Playprtd->dma_rpt_buf)
    {
        dma_free_coherent(pcm->card->dev,
            GK_MAX_DESC_NUM * sizeof(Playprtd->dma_rpt_buf),
            Playprtd->dma_rpt_buf, Playprtd->dma_rpt_phys);
    }
gk_pcm_new_play_free_dma_desc_array:
    if(Playprtd)
    {
        dma_free_coherent(pcm->card->dev,
            GK_MAX_DESC_NUM * sizeof(gk_dma_req_t),
            Playprtd->dma_desc_array, Playprtd->dma_desc_array_phys);
    }
gk_pcm_new_play_free_prtd:
    if(Playprtd)
    {
        kfree(Playprtd);
        Playprtd = NULL;
    }

gk_pcm_new_exit:
    return ret;
}

static struct snd_soc_platform_driver gk_soc_platform =
{
    .pcm_new    = gk_pcm_new,
    .pcm_free   = gk_pcm_free_dma_buffers,
    .ops        = &gk_pcm_ops,
};

static int __devinit gk_soc_platform_probe(struct platform_device *pdev)
{
    return snd_soc_register_platform(&pdev->dev, &gk_soc_platform);
}

static int __devexit gk_soc_platform_remove(struct platform_device *pdev)
{
    snd_soc_unregister_platform(&pdev->dev);
    return 0;
}

static struct platform_driver gk_pcm_driver =
{
    .driver =
    {
        .name = "gk-pcm",
        .owner = THIS_MODULE,
    },

    .probe  = gk_soc_platform_probe,
    .remove = __devexit_p(gk_soc_platform_remove),
};

struct platform_device gk_pcm0 =
{
    .name       = "gk-pcm",
    .id         = -1,
};

int __init snd_gk_pcm_init(void)
{
    platform_device_register(&gk_pcm0);
    device_set_wakeup_capable(&gk_pcm0.dev, 1);
    device_set_wakeup_enable(&gk_pcm0.dev, 0);

    return platform_driver_register(&gk_pcm_driver);
}

void __exit snd_gk_pcm_exit(void)
{
    platform_driver_unregister(&gk_pcm_driver);
}

MODULE_AUTHOR("<yulindeng@goke.com>");
MODULE_DESCRIPTION("GK IPC Soc PCM DMA module");
MODULE_LICENSE("GPL");

