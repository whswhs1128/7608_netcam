/*!
*****************************************************************************
** \file        software/linux/kernel/drv/sensors/sp2508/sp2508.c
**
** \version     $Id: sp2508_ex.c 13054 2017-11-28 04:00:55Z yulindeng $
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
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include "gk_sensor.h"
#include "sp2508_pri.h"

typedef struct
{
    int sub_id;
    //GK_Vi_SensorT api;
}GK_Vi_Sensor_SP2508T;
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

//*****************************************************************************
//*****************************************************************************
//** Local Data
//*****************************************************************************
//*****************************************************************************
static int sp2508_get_vblank_time(int handle, GK_Vi_SensorT* sensor_extern, u32* ptime, u32 extclk, u32 pclk);
static int sp2508_set_shutter_time(int handle, GK_Vi_SensorT* sensor_extern, u32 shutter_time, u32 extclk, u32 pclk);
static int sp2508_set_fps(int handle, GK_Vi_SensorT* sensor_extern, u32 fps, u32 extclk, u32 pclk);
static int sp2508_set_agc_db(int handle, GK_Vi_SensorT* sensor_extern, s32 agc_db);
static GK_Vi_SensorT sp2508_dev;
#if 0
static GK_Vi_Sensor_SP2508T sp2508 =
{
    .api =
    {
        .sensor_id                = GK_VI_SENSOR_SP2508,
        .length                   = sizeof(GK_Vi_Sensor_SP2508T),
        .sensor_get_vblank_time   = sp2508_get_vblank_time,
        .sensor_set_shutter_time  = sp2508_set_shutter_time,
        .sensor_set_fps           = sp2508_set_fps,
        .sensor_set_agc_db        = sp2508_set_agc_db,
        .sensor_set_mirror_mode   = NULL,
        .sensor_set_update_reg    = NULL,
    },
};
#endif

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
static int sp2508_set_agc_db(int handle, GK_Vi_SensorT* sensor_extern, s32 agc_db)
{
    int errorCode = 0;
    s32 db_max,db_set;
    s32 db_step;
    u32 data;
    GK_VI_AgcInfoT agc_info;
    errorCode = sensor_get_agc_info(handle, &agc_info);
    if(errorCode < 0)
    {
        printk("get agc info error!\n");
        return errorCode;
    }

    db_max  = agc_info.db_max;
    db_step = agc_info.db_step;
    db_set  = agc_db;
    //printk("[%s %d]%08x\n", __func__, __LINE__, agc_db);

    if (db_set > db_max)
        db_set = db_max;
    data = db_set / db_step;
    if(data<=0x10)
    {
        data = 0x10;
    }
    else if(data>= 0xA0)//gpw
    {
        data = 0xA0;
    }
    //printk("[%s %d]%08x\n", __func__, __LINE__, data);
    //page set to 1.
    sensor_write_reg_ex(handle, SP2508_REG_PAGE_SET, 0x1,1);  //page set to 1.
    errorCode |= sensor_write_reg_ex(handle, SP2508_REG_AGC, data, 1);
    errorCode |= sensor_write_reg_ex(handle, SP2508_REG_EN, 0x01, 1);

    return errorCode;
}

static int sp2508_get_vblank_time(int handle, GK_Vi_SensorT* sensor_extern, u32* ptime, u32 extclk, u32 pclk)
{
    int                     errorCode       = 0;
    u64                     v_btime;
    u32                     line_length     = 0;

    u32     hb_h, hb_l, sh_delay, win_width_h, win_width_l;
    u32     vb_h, vb_l;  //win_height_h, win_height_l;
    u32     vb;

    //page set to 0.
    sensor_write_reg_ex(handle, SP2508_REG_PAGE_SET, 0x1, 1);  //page set to 0.
    errorCode |= sensor_read_reg_ex(handle, SP2508_REG_VB_H, &vb_h, 1);
    errorCode |= sensor_read_reg_ex(handle, SP2508_REG_VB_L, &vb_l, 1);
    vb = (vb_h<<8) + vb_l;
    errorCode |= sensor_read_reg_ex(handle, SP2508_REG_HB_H, &hb_h, 1);
    errorCode |= sensor_read_reg_ex(handle, SP2508_REG_HB_L, &hb_l, 1);
    line_length = (hb_h<<8) + hb_l + ((sh_delay + (win_width_h<<8) + win_width_l)>>2) + 16;
    BUG_ON(line_length == 0);

    v_btime = (u64)line_length * (u64)vb * 1000000000;
    // ns
    //if(sensor.vmax_clk_ref == GK_SENSOR_CAL_CLK_PIXCLK_DIV2)
    DO_DIV_ROUND(v_btime, (u64)pclk / 2);

    *ptime = v_btime;
    //printk("[%s %d]%08x\n", __func__, __LINE__, v_btime);

    return errorCode;
}

static int sp2508_set_shutter_time(int handle, GK_Vi_SensorT* sensor_extern, u32 shutter_time, u32 extclk, u32 pclk)
{
    int     errorCode = 0;
    u64     exposure_time_q9;
    u32     line_length;
    u32     hb_h, hb_l, sh_delay, win_width_h, win_width_l;
    u32     vb_h, vb_l, win_height_h, win_height_l;
    u32     exp_h, exp_l;
    GK_Vi_Sensor_SP2508T*   pdata = (GK_Vi_Sensor_SP2508T*)sensor_extern->private_data;
    //u8     vts1, vts2;
    //printk("[%s %d]%08x\n", __func__, __LINE__, shutter_time);

    exposure_time_q9 = shutter_time;

    //page set to 1.
    sensor_write_reg_ex(handle, SP2508_REG_PAGE_SET, 0x1,1);  //page set to 1.
    line_length = 1160;  //Always HTS:1160. VTS:1225

    /* t(frame)/t(per line) = t(exposure, in lines)*/
    exposure_time_q9 = exposure_time_q9 * (u64)pclk / 2;// sp2508 spec: Hmax unit : pclk/2
    DO_DIV_ROUND(exposure_time_q9, line_length);
    DO_DIV_ROUND(exposure_time_q9, 512000000);

    if(exposure_time_q9 > 1125)
    {
        exposure_time_q9 = 1125 - 4;
    }
    if(exposure_time_q9 < 7)
    {
        exposure_time_q9 = 7;
    }
    exposure_time_q9 /= 4;
    exp_h = (exposure_time_q9 >> 8) & 0xFF;
    exp_l = exposure_time_q9 & 0xFF;

    //printk("[%s %d]%lld\n", __func__, __LINE__, exposure_time_q9);
    errorCode |= sensor_write_reg_ex(handle, SP2508_REG_EXP_TIME_H, exp_h, 1);  //
    errorCode |= sensor_write_reg_ex(handle, SP2508_REG_EXP_TIME_L, exp_l, 1);
    errorCode |= sensor_write_reg_ex(handle, SP2508_REG_EN, 0x01, 1);

    return errorCode;
}

static int sp2508_set_fps(int handle, GK_Vi_SensorT* sensor_extern, u32 fps, u32 extclk, u32 pclk)
{
    int errorCode = 0;
    //u32 hb_h, hb_l, sh_delay, win_width_h, win_width_l;
    u32 vb_h, vb_l, win_height_h, win_height_l;
    u32 fps_v = 512000000 / fps;
    u32 line_length;
    u32 frame_length;
    u32 sp2508_vb;
    GK_Vi_Sensor_SP2508T*   pdata = (GK_Vi_Sensor_SP2508T*)sensor_extern->private_data;
    //printk("[%s %d]%08x\n", __func__, __LINE__, fps);
    //printk("debug print: fps=%d  pclk=%d \n", fps_v, pclk);

    //page set to 1.
    sensor_write_reg_ex(handle, SP2508_REG_PAGE_SET, 0x1,1);  //page set to 1.

    line_length = 1160;  //Always HTS:1160. VTS:1225
    frame_length = pclk / fps_v /(2 * line_length);

    if(frame_length > 1225)
    {
        sp2508_vb = frame_length - 1225;
    }
    else
    {
        sp2508_vb = 0;
    }
    vb_h = (sp2508_vb>>8) & 0xff;
    vb_l = sp2508_vb & 0xff;

    errorCode |= sensor_write_reg_ex(handle, SP2508_REG_VB_H, vb_h, 1);  //
    errorCode |= sensor_write_reg_ex(handle, SP2508_REG_VB_L, vb_l, 1);
    errorCode |= sensor_write_reg_ex(handle, SP2508_REG_EN, 0x01, 1);

    return errorCode;
}


int sp2508_probe(GK_Vi_SensorT* sensor_extern, int id)
{
    GK_Vi_Sensor_SP2508T * sp2508_data = NULL;

    sensor_extern->private_data = kzalloc(sizeof(GK_Vi_Sensor_SP2508T), GFP_KERNEL);
    if (sensor_extern->private_data == NULL)
    {
        printk("kzalloc memory(0x%04x) failed\n", sizeof(GK_Vi_Sensor_SP2508T));
        return -1;
    }
    sp2508_data = sensor_extern->private_data;

    sp2508_data->sub_id = id;

    return 0;
}

int sp2508_remove(GK_Vi_SensorT* sensor_extern)
{
    if (sensor_extern->private_data)
    {
        kfree(sensor_extern->private_data);
        sensor_extern->private_data = NULL;
    }
    return 0;
}

//*****************************************************************************
//*****************************************************************************
//** Local Functions
//*****************************************************************************
//*****************************************************************************
static int __init sp2508_init(void)
{
    sp2508_dev.sensor_id                = GK_VI_SENSOR_SP2508;
    sp2508_dev.length                   = sizeof(sp2508_dev);

    sp2508_dev.private_data             = NULL;
    sp2508_dev.probe                    = sp2508_probe;
    sp2508_dev.remove                   = sp2508_remove;
    sp2508_dev.sensor_get_vblank_time   = sp2508_get_vblank_time;
    sp2508_dev.sensor_set_shutter_time  = sp2508_set_shutter_time;
    sp2508_dev.sensor_set_fps           = sp2508_set_fps;
    sp2508_dev.sensor_set_agc_db        = sp2508_set_agc_db;
    sp2508_dev.sensor_set_mirror_mode   = NULL;
    sp2508_dev.sensor_set_update_reg    = NULL;
    sensor_registry_api(&sp2508_dev);
    return 0;
}

static void __exit sp2508_exit(void)
{
    sensor_unregistry_api(&sp2508_dev);
}

module_init(sp2508_init);
module_exit(sp2508_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("GOKE MICROELECTRONICS Inc.");
MODULE_DESCRIPTION("SP2508 1/5-Inch CMOS Digital Image Sensor");

