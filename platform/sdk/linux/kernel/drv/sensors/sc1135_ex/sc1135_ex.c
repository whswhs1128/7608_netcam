/*!
*****************************************************************************
** \file        software/linux/kernel/drv/sensors/sc1135/sc1135.c
**
** \version     $Id: sc1135_ex.c 1280 2019-02-22 09:38:49Z dengbiao $
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
#include "sc1135_pri.h"

#define SET_REG_BY_VSYNC
#ifdef SET_REG_BY_VSYNC
#define SET_SHUTTER_BY_VSYNC
#endif

typedef struct
{
    int sub_id;
    //GK_Vi_SensorT api;
#ifdef SET_REG_BY_VSYNC
    int sc1135_adc_ch;
    int sc1135_reg_3630;
    int sc1135_reg_3631;
    int sc1135_reg_3e08;
    int sc1135_reg_3e09;
    //int sc1135_reg_3e0f;

    int sc1135_last_reg_3630;
    int sc1135_last_reg_3631;
    int sc1135_last_reg_3e08;
    int sc1135_last_reg_3e09;
    //int sc1135_last_reg_3e0f;

#ifdef SET_SHUTTER_BY_VSYNC
    int sc1135_shutter_ch;
    int sc1135_reg_exp_time_h;
    int sc1135_reg_exp_time_l;
    int sc1135_reg_331e;

    int sc1135_last_reg_exp_time_h;
    int sc1135_last_reg_exp_time_l;
    int sc1135_last_reg_331e;
#endif
#endif
}GK_Vi_Sensor_SC1135T;

//*****************************************************************************
//*****************************************************************************
//** Local structures
//*****************************************************************************
//*****************************************************************************
#define SC1135_GAIN_ROWS            96   //109
#define AGAIN_LIST_NUM              48   //61
typedef struct sc1135_gain_table
{
    u16 SC1135_REG_3630;
    u16 SC1135_REG_3631;
    u16 SC1135_REG_3E08;
    u16 SC1135_REG_3E09;
    u32 db_value;
}sc1135_gain_table_s;



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
static int sc1135_set_agc_db(int handle, GK_Vi_SensorT* sensor_extern, s32 agc_db);
static int sc1135_set_shutter_time(int handle, GK_Vi_SensorT* sensor_extern, u32 shutter_time, u32 extclk, u32 pclk);
static int sc1135_set_fps(int handle, GK_Vi_SensorT* sensor_extern, u32 fps, u32 extclk, u32 pclk);
static int sc1135_get_again_from_offset(int reg_3e09_value,int offset);
static int sc1135_set_update_reg(int handle, GK_Vi_SensorT* sensor_extern);
static GK_Vi_SensorT sc1135_dev;
#if 0
static GK_Vi_Sensor_SC1135T sc1135 =
{
    .api =
    {
        .sensor_id                = GK_VI_SENSOR_SC1135,
        .length                   = sizeof(GK_Vi_Sensor_SC1135T),
        .sensor_get_vblank_time   = NULL,  //hhl add.
        .sensor_set_shutter_time  = sc1135_set_shutter_time,
        .sensor_set_fps           = NULL,  //sc1135_set_fps,
        .sensor_set_agc_db        = sc1135_set_agc_db,
        .sensor_set_mirror_mode   = NULL,
        .sensor_set_update_reg    = sc1135_set_update_reg,
    },
#ifdef SET_REG_BY_VSYNC
    .sc1135_adc_ch    = 0,
    .sc1135_reg_3630  = 0,
    .sc1135_reg_3631  = 0,
    .sc1135_reg_3e08  = 0,
    .sc1135_reg_3e09  = 0,
    //.sc1135_reg_3e0f  = 0,

    .sc1135_last_reg_3630     = 0xffffffff,
    .sc1135_last_reg_3631     = 0xffffffff,
    .sc1135_last_reg_3e08     = 0xffffffff,
    .sc1135_last_reg_3e09     = 0xffffffff,
    //.sc1135_last_reg_3e0f     = 0xffffffff,

#ifdef SET_SHUTTER_BY_VSYNC
    .sc1135_shutter_ch        = 0,
    .sc1135_reg_exp_time_h    = 0,
    .sc1135_reg_exp_time_l    = 0,
    .sc1135_reg_331e          = 0,

    .sc1135_last_reg_exp_time_h   = 0xffffffff,
    .sc1135_last_reg_exp_time_l   = 0xffffffff,
    .sc1135_last_reg_331e         = 0xffffffff,
#endif
#endif
};
#endif

const sc1135_gain_table_s   SC1135_GLOBAL_GAIN_TABLE[SC1135_GAIN_ROWS] =
{
    //        0x3630                      0x3631                   0x3e08                0x3e09
    //        restriction                 restriction                 dgain                  again
    //     (<2X: 0xD0)          (<2X:      0x80)                (00: 1X)         (6:4 corse_gain)
    //     (>2X: 0x60)          (2X~16X: 0x8E)                (01: 2X)         (3:0 fine_gain)
    //                                (>16X:     0x8C)               (11: 4X)
    {0xD0, 0x80, 0x00, 0x00, 0x00000000,  },  //      1.0000,
    {0xD0, 0x80, 0x00, 0x01, 0x007e9cd5,  },  //      1.0588,
    {0xD0, 0x80, 0x00, 0x02, 0x00f66163,  },  //      1.1176,
    {0xD0, 0x80, 0x00, 0x03, 0x016831ab,  },  //      1.1765,
    {0xD0, 0x80, 0x00, 0x04, 0x01d44466,  },  //      1.2353,
    {0xD0, 0x80, 0x00, 0x05, 0x023b5056,  },  //      1.2941,
    {0xD0, 0x80, 0x00, 0x06, 0x029dc7e2,  },  //      1.3529,
    {0xD0, 0x80, 0x00, 0x07, 0x02fc3702,  },  //      1.4118,
    {0xD0, 0x80, 0x00, 0x08, 0x0356a34e,  },  //      1.4706,
    {0xD0, 0x80, 0x00, 0x09, 0x03ad83f3,  },  //      1.5294,
    {0xD0, 0x80, 0x00, 0x0a, 0x04011d72,  },  //      1.5882,
    {0xD0, 0x80, 0x00, 0x0b, 0x0451cf44,  },  //      1.6471,
    {0xD0, 0x80, 0x00, 0x0c, 0x049f89ef,  },  //      1.7059,
    {0xD0, 0x80, 0x00, 0x0d, 0x04eaa236,  },  //      1.7647,
    {0xD0, 0x80, 0x00, 0x0e, 0x05334451,  },  //      1.8235,
    {0xD0, 0x80, 0x00, 0x0f, 0x0579b664,  },  //      1.8824,
    {0x60, 0x8E, 0x00, 0x10, 0x06000000,  },  //      2.0000,
    {0x60, 0x8E, 0x00, 0x11, 0x067e9cd5,  },  //      2.1176,
    {0x60, 0x8E, 0x00, 0x12, 0x06f66163,  },  //      2.2352,
    {0x60, 0x8E, 0x00, 0x13, 0x076831ab,  },  //      2.3530,
    {0x60, 0x8E, 0x00, 0x14, 0x07d44466,  },  //      2.4706,
    {0x60, 0x8E, 0x00, 0x15, 0x083b5056,  },  //      2.5882,
    {0x60, 0x8E, 0x00, 0x16, 0x089dc7e2,  },  //      2.7058,
    {0x60, 0x8E, 0x00, 0x17, 0x08fc3702,  },  //      2.8236,
    {0x60, 0x8E, 0x00, 0x18, 0x0932e12d,  },  //      2.89412,
    {0x60, 0x8E, 0x00, 0x19, 0x09ad83f3,  },  //      3.0588,
    {0x60, 0x8E, 0x00, 0x1a, 0x0a011d72,  },  //      3.1764,
    {0x60, 0x8E, 0x00, 0x1b, 0x0a51cf44,  },  //      3.2942,
    {0x60, 0x8E, 0x00, 0x1c, 0x0a9f89ef,  },  //      3.4118,
    {0x60, 0x8E, 0x00, 0x1d, 0x0aeaa236,  },  //      3.5294,
    {0x60, 0x8E, 0x00, 0x1e, 0x0b334451,  },  //      3.6470,
    {0x60, 0x8E, 0x00, 0x1f, 0x0b79b664,  },  //      3.7648,
    {0x60, 0x8E, 0x00, 0x30, 0x0c000000,  },  //      4.0000,
    {0x60, 0x8E, 0x00, 0x31, 0x0c7e9cd5,  },  //      4.2352,
    {0x60, 0x8E, 0x00, 0x32, 0x0cf66163,  },  //      4.4704,
    {0x60, 0x8E, 0x00, 0x33, 0x0d6831ab,  },  //      4.7060,
    {0x60, 0x8E, 0x00, 0x34, 0x0dd44466,  },  //      4.9412,
    {0x60, 0x8E, 0x00, 0x35, 0x0e3b5056,  },  //      5.1764,
    {0x60, 0x8E, 0x00, 0x36, 0x0e9dc7e2,  },  //      5.4116,
    {0x60, 0x8E, 0x00, 0x37, 0x0efc3702,  },  //      5.6472,
    {0x60, 0x8E, 0x00, 0x38, 0x0f56a34e,  },  //      5.8824,
    {0x60, 0x8E, 0x00, 0x39, 0x0fad83f3,  },  //      6.1176,
    {0x60, 0x8E, 0x00, 0x3a, 0x10011d72,  },  //      6.3528,
    {0x60, 0x8E, 0x00, 0x3b, 0x1051cf44,  },  //      6.5884,
    {0x60, 0x8E, 0x00, 0x3c, 0x109f89ef,  },  //      6.8236,
    {0x60, 0x8E, 0x00, 0x3d, 0x10eaa236,  },  //      7.0588,
    {0x60, 0x8E, 0x00, 0x3e, 0x11334451,  },  //      7.2940,
    {0x60, 0x8E, 0x00, 0x3f, 0x1179b664,  },  //      7.5296,
    {0x60, 0x8E, 0x00, 0x70, 0x12000000,  },  //      8.0000,
    {0x60, 0x8E, 0x00, 0x71, 0x127e9cd5,  },  //      8.4704,
    {0x60, 0x8E, 0x00, 0x72, 0x12f66163,  },  //      8.9408,
    {0x60, 0x8E, 0x00, 0x73, 0x136831ab,  },  //      9.412,
    {0x60, 0x8E, 0x00, 0x74, 0x13d44466,  },  //      9.8824,
    {0x60, 0x8E, 0x00, 0x75, 0x143b5056,  },  //      10.3528,
    {0x60, 0x8E, 0x00, 0x76, 0x149dc7e2,  },  //      10.8232,
    {0x60, 0x8E, 0x00, 0x77, 0x14fc3702,  },  //      11.2944,
    {0x60, 0x8E, 0x00, 0x78, 0x1556a34e,  },  //      11.7648,
    {0x60, 0x8E, 0x00, 0x79, 0x15ad83f3,  },  //      12.2352,
    {0x60, 0x8E, 0x00, 0x7a, 0x16011d72,  },  //      12.7056,
    {0x60, 0x8E, 0x00, 0x7b, 0x1651cf44,  },  //      13.1768,
    {0x60, 0x8E, 0x00, 0x7c, 0x15f6e7f1,  },  //      12.6472,
    {0x60, 0x8E, 0x00, 0x7d, 0x16eaa236,  },  //      14.1176,
    {0x60, 0x8E, 0x00, 0x7e, 0x17334451,  },  //      14.5880,
    {0x60, 0x8E, 0x00, 0x7f, 0x1779b664,  },  //      15.0592,
    //dgain=2X
    {0x60, 0x8C, 0x01, 0x70, 0x18000000,  },  //      16.0000,
    {0x60, 0x8C, 0x01, 0x71, 0x187e9cd5,  },  //      16.9408,
    {0x60, 0x8C, 0x01, 0x72, 0x18f66163,  },  //      17.8816,
    {0x60, 0x8C, 0x01, 0x73, 0x196831ab,  },  //      18.8240,
    {0x60, 0x8C, 0x01, 0x74, 0x19d44466,  },  //      19.7648,
    {0x60, 0x8C, 0x01, 0x75, 0x1a3b5056,  },  //      20.7056,
    {0x60, 0x8C, 0x01, 0x76, 0x1a9dc7e2,  },  //      21.6464,
    {0x60, 0x8C, 0x01, 0x77, 0x1afc3702,  },  //      22.5888,
    {0x60, 0x8C, 0x01, 0x78, 0x1b56a34e,  },  //      23.5296,
    {0x60, 0x8C, 0x01, 0x79, 0x1bad83f3,  },  //      24.4704,
    {0x60, 0x8C, 0x01, 0x7a, 0x1c011d72,  },  //      25.4112,
    {0x60, 0x8C, 0x01, 0x7b, 0x1c51cf44,  },  //      26.3536,
    {0x60, 0x8C, 0x01, 0x7c, 0x1bf6e7f1,  },  //      25.2944,
    {0x60, 0x8C, 0x01, 0x7d, 0x1ceaa236,  },  //      28.2352,
    {0x60, 0x8C, 0x01, 0x7e, 0x1d334451,  },  //      29.1760,
    {0x60, 0x8C, 0x01, 0x7f, 0x1d79b664,  },  //      30.1184,
    //dgain=4X
    {0x60, 0x8C, 0x03, 0x70, 0x1e000000,  },  //      32.0000,
    {0x60, 0x8C, 0x03, 0x71, 0x1e7e9cd5,  },  //      33.8816,
    {0x60, 0x8C, 0x03, 0x72, 0x1ef66163,  },  //      35.7632,
    {0x60, 0x8C, 0x03, 0x73, 0x1f6831ab,  },  //      37.6480,
    {0x60, 0x8C, 0x03, 0x74, 0x1fd44466,  },  //      39.5296,
    {0x60, 0x8C, 0x03, 0x75, 0x203b5056,  },  //      41.4112,
    {0x60, 0x8C, 0x03, 0x76, 0x209dc7e2,  },  //      43.2928,
    {0x60, 0x8C, 0x03, 0x77, 0x20fc3702,  },  //      45.1776,
    {0x60, 0x8C, 0x03, 0x78, 0x2156a34e,  },  //      47.0592,
    {0x60, 0x8C, 0x03, 0x79, 0x21ad83f3,  },  //      48.9408,
    {0x60, 0x8C, 0x03, 0x7a, 0x22011d72,  },  //      50.8224,
    {0x60, 0x8C, 0x03, 0x7b, 0x2251cf44,  },  //      52.7072,
    {0x60, 0x8C, 0x03, 0x7c, 0x21f6e7f1,  },  //      50.5888,
    {0x60, 0x8C, 0x03, 0x7d, 0x22eaa236,  },  //      56.4704,
    {0x60, 0x8C, 0x03, 0x7e, 0x23334451,  },  //      58.3520,
    {0x60, 0x8C, 0x03, 0x7f, 0x2379b664,  },  //      60.2368,

};

static const u8 again_list[AGAIN_LIST_NUM] =
{
    0x7f,0x7e,0x7d,0x7c,0x7b,0x7a,0x79,0x78,0x77,0x76,0x75,0x74,0x73,0x72,0x71,0x70,
    0x3f,0x3e,0x3d,0x3c,0x3b,0x3a,0x39,0x38,0x37,0x36,0x35,0x34,0x33,0x32,0x31,0x30,
    0x1f,0x1e,0x1d,0x1c,0x1b,0x1a,0x19,0x18,0x17,0x16,0x15,0x14,0x13,0x12,0x11,0x10,
};



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
static int sc1135_set_agc_db(int handle, GK_Vi_SensorT* sensor_extern, s32 agc_db)
{
    int                     errorCode = 0;
    u16                     idc_reg0, idc_reg1, idc_reg2, idc_reg3;
    u32                     gain_index = SC1135_GAIN_ROWS;
    int                     i;
    GK_Vi_Sensor_SC1135T*   pdata = (GK_Vi_Sensor_SC1135T*)sensor_extern->private_data;
    //printk("[%s %d]%08x\n", __func__, __LINE__, agc_db);

    for(i = 0; i < SC1135_GAIN_ROWS; i++)
    {
        if(SC1135_GLOBAL_GAIN_TABLE[i].db_value > agc_db)
        {
            gain_index = i;
            break;
        }
    }
    if(gain_index>0)
        gain_index--;

    if (gain_index >= SC1135_GAIN_ROWS)
    {
        errorCode = -EINVAL;
        return errorCode;
    }
    idc_reg0 = SC1135_GLOBAL_GAIN_TABLE[gain_index].SC1135_REG_3630;
    idc_reg1 = SC1135_GLOBAL_GAIN_TABLE[gain_index].SC1135_REG_3631;
    idc_reg2 = SC1135_GLOBAL_GAIN_TABLE[gain_index].SC1135_REG_3E08;
    idc_reg3 = SC1135_GLOBAL_GAIN_TABLE[gain_index].SC1135_REG_3E09;
#ifdef SET_REG_BY_VSYNC
    pdata->sc1135_reg_3630 = idc_reg0;
    pdata->sc1135_reg_3631 = idc_reg1;
    pdata->sc1135_reg_3e08 = idc_reg2;
    pdata->sc1135_reg_3e09 = idc_reg3;
    pdata->sc1135_adc_ch = 1;
#else
    sensor_write_reg_ex(handle, SC1135_3630, idc_reg0,1);
    sensor_write_reg_ex(handle, SC1135_3631, idc_reg1,1);
    sensor_write_reg_ex(handle, SC1135_AEC_DGAIN, idc_reg2,1);
    sensor_write_reg_ex(handle, SC1135_AEC_AGAIN, idc_reg3,1);
#endif

    return errorCode;
}

static int sc1135_set_shutter_time(int handle, GK_Vi_SensorT* sensor_extern, u32 shutter_time, u32 extclk, u32 pclk)
{
    int     errorCode = 0;
    u64     exposure_time_q9;
    u32     frame_length, line_length;
    u32     shr_width_m, shr_width_l, sc1135_331e_value;
    u32     vts1, vts2;
    GK_Vi_Sensor_SC1135T*   pdata = (GK_Vi_Sensor_SC1135T*)sensor_extern->private_data;
    //printk("[%s %d]%08x\n", __func__, __LINE__, shutter_time);

    exposure_time_q9 = shutter_time;

    errorCode |= sensor_read_reg_ex(handle, SC1135_TIMING_VTS_H, &shr_width_m,1);
    errorCode |= sensor_read_reg_ex(handle, SC1135_TIMING_VTS_L, &shr_width_l,1);
    frame_length = (shr_width_m<<8) + shr_width_l;
    errorCode |= sensor_read_reg_ex(handle, SC1135_TIMING_HTS_H, &shr_width_m,1);
    errorCode |= sensor_read_reg_ex(handle, SC1135_TIMING_HTS_L, &shr_width_l,1);
    line_length = ((shr_width_m)<<8) + shr_width_l;

    /* t(frame)/t(per line) = t(exposure, in lines)*/
    exposure_time_q9 = exposure_time_q9 * (u64)pclk;// sc1135 spec: Hmax unit : INCK
    DO_DIV_ROUND(exposure_time_q9, line_length);
    DO_DIV_ROUND(exposure_time_q9, 512000000);

    if((frame_length - 4) < exposure_time_q9)
    {
        exposure_time_q9 = frame_length - 4;
    }

    shr_width_m = (exposure_time_q9 >> 4) & 0xff;
    shr_width_l = exposure_time_q9 & 0xf;

#ifdef SET_REG_BY_VSYNC
#ifdef SET_SHUTTER_BY_VSYNC
    pdata->sc1135_reg_exp_time_h = shr_width_m;
    pdata->sc1135_reg_exp_time_l =  ((shr_width_l << 4) & 0xf0);
#else
    errorCode |= sensor_write_reg_ex(handle, SC1135_AEC_EXP_TIME_H, shr_width_m,1);//
    errorCode |= sensor_write_reg_ex(handle, SC1135_AEC_EXP_TIME_L, ((shr_width_l << 4) & 0xf0),1);
#endif
#else
    errorCode |= sensor_write_reg_ex(handle, SC1135_AEC_EXP_TIME_H, shr_width_m);//
    errorCode |= sensor_write_reg_ex(handle, SC1135_AEC_EXP_TIME_L, ((shr_width_l << 4) & 0xf0),1);
#endif
    /*set 0x331e*/
    vts1 = frame_length*7;
    DO_DIV_ROUND(vts1, 10);
    errorCode |= sensor_read_reg_ex(handle, SC1135_331E, &sc1135_331e_value,1);//
    if (exposure_time_q9 <= vts1)
    {
        sc1135_331e_value =  0xe0;
    }
    else if (exposure_time_q9 <= (frame_length - 4))
    {
        vts2 = (0xe0 - 0x1a) * (exposure_time_q9 - vts1);
        DO_DIV_ROUND(vts2, ((frame_length - 4) - vts1));
        sc1135_331e_value = (0xe0 - vts2) ;
    }

#ifdef SET_REG_BY_VSYNC
#ifdef SET_SHUTTER_BY_VSYNC
    pdata->sc1135_reg_331e = sc1135_331e_value;
    pdata->sc1135_shutter_ch = 1;
#else
    sensor_write_reg_ex(handle, SC1135_331E, sc1135_331e_value,1);//
#endif
#else
    sensor_write_reg_ex(handle, SC1135_331E, sc1135_331e_value,1);//
#endif
    return errorCode;
}

static int sc1135_set_fps(int handle, GK_Vi_SensorT* sensor_extern, u32 fps, u32 extclk, u32 pclk)
{
    int                             errorCode           = 0;
    u64                             frame_time_pclk;
    u32                             line_length         = 0;
    u32                             tem_data            = 0; //0401 hhl add: restiction.
    u32                             frame_length        = 0;
    u32                             data_val;
    //printk("[%s %d]%08x\n", __func__, __LINE__, fps);

    /*get line_length and frame_length */
    errorCode |= sensor_read_reg_ex(handle, SC1135_TIMING_HTS_H, &data_val,1);
    frame_length = (u32)(data_val) <<  8;
    errorCode |= sensor_read_reg_ex(handle, SC1135_TIMING_HTS_L, &data_val,1);
    frame_length += (u32)data_val;

    frame_time_pclk = fps * (u64)pclk;
    DO_DIV_ROUND(frame_time_pclk, frame_length);
    DO_DIV_ROUND(frame_time_pclk, 512000000);

    line_length = frame_time_pclk;
    errorCode |= sensor_write_reg_ex(handle, SC1135_TIMING_VTS_H, (u8)((line_length & 0x00FF00) >> 8),1);
    errorCode |= sensor_write_reg_ex(handle, SC1135_TIMING_VTS_L, (u8)(line_length & 0x0000FF),1);

    //0401 hhl add (restriction)
    /*******************0401 hhl add (head)*******************/
    #if 1
    errorCode |= sensor_write_reg_ex(handle, SC1135_3338, (u8)((line_length & 0x00FF00) >> 8),1);
    errorCode |= sensor_write_reg_ex(handle, SC1135_3339, (u8)(line_length & 0x0000FF),1);
    if(line_length > 0x2e8)
        tem_data = line_length - 0x2e8;
    else
        tem_data = 0;
    errorCode |= sensor_write_reg_ex(handle, SC1135_3336, (u8)((tem_data & 0x00FF00) >> 8),1);
    errorCode |= sensor_write_reg_ex(handle, SC1135_3337, (u8)(tem_data & 0x0000FF),1);
    #endif
    /*******************0401 hhl add (end)*******************/

    return errorCode;
}

static int sc1135_get_again_from_offset(int reg_3e09_value,int offset)
{
    int i;
    for(i=0;i<AGAIN_LIST_NUM;i++)
    {
        if(again_list[i]==(u8)reg_3e09_value)
        {
            i+=offset;
            if(i<AGAIN_LIST_NUM)
            {
                return again_list[i];
            }
            else
            {
                return again_list[AGAIN_LIST_NUM-1];
            }
        }
    }
    return reg_3e09_value;
}

static int sc1135_set_update_reg(int handle, GK_Vi_SensorT* sensor_extern)
{
    u32 shutter_chnged =0;
    static u8 agc_offset = 0;
    u32 temp_reg;
    GK_Vi_Sensor_SC1135T*   pdata = (GK_Vi_Sensor_SC1135T*)sensor_extern->private_data;
    //printk("[%s %d]\n", __func__, __LINE__);

#ifdef SET_REG_BY_VSYNC
#ifdef SET_SHUTTER_BY_VSYNC
    if(pdata->sc1135_shutter_ch == 1)
    {
        if(pdata->sc1135_last_reg_exp_time_h!=pdata->sc1135_reg_exp_time_h)
        {
            sensor_write_reg_ex(handle, SC1135_AEC_EXP_TIME_H, pdata->sc1135_reg_exp_time_h,0);
        }
        if(pdata->sc1135_last_reg_exp_time_l!=pdata->sc1135_reg_exp_time_l)
        {
            sensor_write_reg_ex(handle, SC1135_AEC_EXP_TIME_L, pdata->sc1135_reg_exp_time_l,0);
        }
        if(pdata->sc1135_last_reg_331e!=pdata->sc1135_reg_331e)
        {
            sensor_write_reg_ex(handle, SC1135_331E, pdata->sc1135_reg_331e,0);//
        }

        pdata->sc1135_last_reg_exp_time_h = pdata->sc1135_reg_exp_time_h;
        pdata->sc1135_last_reg_exp_time_l = pdata->sc1135_reg_exp_time_l;
        pdata->sc1135_last_reg_331e =pdata->sc1135_reg_331e;
        pdata->sc1135_shutter_ch = 0;
        shutter_chnged = 1;
    }
#endif
    if(pdata->sc1135_adc_ch == 1)
    {
        if(agc_offset!=0)
        {
            pdata->sc1135_reg_3e09=sc1135_get_again_from_offset(pdata->sc1135_reg_3e09, agc_offset);
        }
        if(pdata->sc1135_last_reg_3e09!=pdata->sc1135_reg_3e09)
        {
            if(shutter_chnged==0)
            {
            }
            sensor_write_reg_ex(handle, SC1135_AEC_AGAIN, pdata->sc1135_reg_3e09,0);
        }
        if(pdata->sc1135_last_reg_3e08!=pdata->sc1135_reg_3e08)
        {
#if 0
            if(sc1135_reg_3e08==0x00)
            {
                temp =0;
            }
            else if(sc1135_reg_3e0f==0x15)
            {
                temp =0x1;
            }
            else if(sc1135_reg_3e0f==0x17)
            {
                temp =0x3;
            }
#endif
            sensor_write_reg_ex(handle, SC1135_AEC_DGAIN, pdata->sc1135_reg_3e08,0);  //0401 hhl note: what is 0x3E0A ?
        }
        //0401 hhl add
        /*****************0401 hhl add (head)******************/
        if (pdata->sc1135_last_reg_3630 != pdata->sc1135_reg_3630)
        {
            sensor_write_reg_ex(handle, SC1135_3630, pdata->sc1135_reg_3630,0);
        }
        if (pdata->sc1135_last_reg_3631 != pdata->sc1135_reg_3631)
        {
            sensor_write_reg_ex(handle, SC1135_3631, pdata->sc1135_reg_3631,0);
        }
        /*****************0401 hhl add (end)******************/
        pdata->sc1135_adc_ch = 0;
        pdata->sc1135_last_reg_3630=pdata->sc1135_reg_3630;
        pdata->sc1135_last_reg_3631=pdata->sc1135_reg_3631;
        pdata->sc1135_last_reg_3e08=pdata->sc1135_reg_3e08;
        pdata->sc1135_last_reg_3e09=pdata->sc1135_reg_3e09;
    }

    sensor_read_reg_ex(handle, SC1135_3911, &temp_reg,0);
    if(temp_reg>0x58)
    {
        agc_offset++;
        pdata->sc1135_last_reg_3e09=sc1135_get_again_from_offset(pdata->sc1135_last_reg_3e09,1);
        sensor_write_reg_ex(handle, SC1135_AEC_AGAIN, pdata->sc1135_last_reg_3e09,0);
    }
    else if(temp_reg<0x45)
    {
        if(agc_offset>0)
        {
            agc_offset--;
            pdata->sc1135_last_reg_3e09=sc1135_get_again_from_offset(pdata->sc1135_last_reg_3e09,-1);
            sensor_write_reg_ex(handle, SC1135_AEC_AGAIN, pdata->sc1135_last_reg_3e09,0);
        }
    }
#endif
    return 0;
}

int sc1135_probe(GK_Vi_SensorT* sensor_extern, int id)
{
    GK_Vi_Sensor_SC1135T *sc1135_data = NULL;

    sensor_extern->private_data = kzalloc(sizeof(GK_Vi_Sensor_SC1135T), GFP_KERNEL);
    if (sensor_extern->private_data == NULL)
    {
        printk("kzalloc memory(0x%04x) failed\n", sizeof(GK_Vi_Sensor_SC1135T));
        return -1;
    }
    sc1135_data = sensor_extern->private_data;

    sc1135_data->sub_id = id;

#ifdef SET_REG_BY_VSYNC
    sc1135_data->sc1135_adc_ch    = 0;
    sc1135_data->sc1135_reg_3630  = 0;
    sc1135_data->sc1135_reg_3631  = 0;
    sc1135_data->sc1135_reg_3e08  = 0;
    sc1135_data->sc1135_reg_3e09  = 0;
    //.sc1135_reg_3e0f  = 0;

    sc1135_data->sc1135_last_reg_3630     = 0xffffffff;
    sc1135_data->sc1135_last_reg_3631     = 0xffffffff;
    sc1135_data->sc1135_last_reg_3e08     = 0xffffffff;
    sc1135_data->sc1135_last_reg_3e09     = 0xffffffff;
    //.sc1135_last_reg_3e0f     = 0xffffffff;

#ifdef SET_SHUTTER_BY_VSYNC
    sc1135_data->sc1135_shutter_ch        = 0;
    sc1135_data->sc1135_reg_exp_time_h    = 0;
    sc1135_data->sc1135_reg_exp_time_l    = 0;
    sc1135_data->sc1135_reg_331e          = 0;

    sc1135_data->sc1135_last_reg_exp_time_h   = 0xffffffff;
    sc1135_data->sc1135_last_reg_exp_time_l   = 0xffffffff;
    sc1135_data->sc1135_last_reg_331e         = 0xffffffff;
#endif
#endif
    return 0;
}

int sc1135_remove(GK_Vi_SensorT* sensor_extern)
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
static int __init sc1135_init(void)
{
    sc1135_dev.sensor_id                = GK_VI_SENSOR_SC1135;
    sc1135_dev.length                   = sizeof(GK_Vi_Sensor_SC1135T);

    sc1135_dev.private_data             = NULL;
    sc1135_dev.probe                    = sc1135_probe;
    sc1135_dev.remove                   = sc1135_remove;
    sc1135_dev.sensor_get_vblank_time   = NULL;  //hhl add.
    sc1135_dev.sensor_set_shutter_time  = sc1135_set_shutter_time;
    sc1135_dev.sensor_set_fps           = NULL;  //sc1135_set_fps;
    sc1135_dev.sensor_set_agc_db        = sc1135_set_agc_db;
    sc1135_dev.sensor_set_mirror_mode   = NULL;
    sc1135_dev.sensor_set_update_reg    = sc1135_set_update_reg;
    sensor_registry_api(&sc1135_dev);
    return 0;
}

static void __exit sc1135_exit(void)
{
    sensor_unregistry_api(&sc1135_dev);
}

module_init(sc1135_init);
module_exit(sc1135_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("GOKE MICROELECTRONICS Inc.");
MODULE_DESCRIPTION("SC1135 1/3-Inch 1.2-Megapixel CMOS Digital Image Sensor");

