/*!
*****************************************************************************
** \file        software/linux/kernel/drv/sensors/gc0328/gc0328.c
**
** \version     $Id: gc0328_ex.c 1280 2019-02-22 09:38:49Z dengbiao $
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
#include "gc0328_pri.h"

#define SET_REG_BY_VSYNC
#ifdef SET_REG_BY_VSYNC
#define SET_SHUTTER_BY_VSYNC
#endif


typedef struct
{
    int sub_id;
    //GK_Vi_SensorT api;
#ifdef SET_REG_BY_VSYNC
    int gc0328_adc_ch;
    int gc0328_reg_fe;
    int gc0328_reg_70;
    int gc0328_reg_71;
    int gc0328_reg_72;

    int gc0328_last_reg_fe;
    int gc0328_last_reg_70;
    int gc0328_last_reg_71;
    int gc0328_last_reg_72;

#ifdef SET_SHUTTER_BY_VSYNC

    int gc0328_shutter_ch;
    int gc0328_reg_exp_time_h;
    int gc0328_reg_exp_time_l;

    int gc0328_last_reg_exp_time_h;
    int gc0328_last_reg_exp_time_l;
#endif
#endif
}GK_Vi_Sensor_GC0328T;
//*****************************************************************************
//*****************************************************************************
//** Local structures
//*****************************************************************************
//*****************************************************************************
#define GC0328_GAIN_ROWS            145
typedef struct gc0328_gain_table
{
    u8 GC0328_REG_FE;
    u8 GC0328_REG_70;
    u8 GC0328_REG_71;
    u8 GC0328_REG_72;
    u32 db_value;
}gc0328_gain_table_s;

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
static int gc0328_get_vblank_time(int handle, GK_Vi_SensorT* sensor_extern, u32* ptime, u32 extclk, u32 pclk);
static int gc0328_set_agc_db(int handle, GK_Vi_SensorT* sensor_extern, s32 agc_db);
static int gc0328_set_shutter_time(int handle, GK_Vi_SensorT* sensor_extern, u32 shutter_time, u32 extclk, u32 pclk);
static int gc0328_set_fps(int handle, GK_Vi_SensorT* sensor_extern, u32 fps, u32 extclk, u32 pclk);
static int gc0328_set_update_reg(int handle, GK_Vi_SensorT* sensor_extern);
#if 0
static GK_Vi_Sensor_GC0328T gc0328 =
{
    .api =
    {
        .sensor_id                = GK_VI_SENSOR_GC0328,
        .length                   = sizeof(GK_Vi_Sensor_GC0328T),
        .sensor_get_vblank_time   = gc0328_get_vblank_time,
        .sensor_set_shutter_time  = gc0328_set_shutter_time,
        .sensor_set_fps           = gc0328_set_fps,
        .sensor_set_agc_db        = gc0328_set_agc_db,
        .sensor_set_update_reg    = gc0328_set_update_reg,
    },
#ifdef SET_REG_BY_VSYNC
    .gc0328_adc_ch    = 0,
    .gc0328_reg_fe    = 0x0,
    .gc0328_reg_70    = 0x40,
    .gc0328_reg_71    = 0x40,
    .gc0328_reg_72    = 0x40,

    .gc0328_last_reg_fe   = 0xffffffff,
    .gc0328_last_reg_70   = 0xffffffff,
    .gc0328_last_reg_71   = 0xffffffff,
    .gc0328_last_reg_72   = 0xffffffff,

#ifdef SET_SHUTTER_BY_VSYNC
    .gc0328_shutter_ch     = 0,
    .gc0328_reg_exp_time_h = 0,
    .gc0328_reg_exp_time_l = 0,

    .gc0328_last_reg_exp_time_h = 0xffffffff,
    .gc0328_last_reg_exp_time_l = 0xffffffff,
#endif
#endif
};
#endif
static GK_Vi_SensorT gc0328_dev;

const gc0328_gain_table_s GC0328_GLOBAL_GAIN_TABLE[GC0328_GAIN_ROWS] =
{
    {0x00, 0x40, 0x40, 0x40, 0x00000000,},   //1.0000     //0.000000
    {0x00, 0x40, 0x40, 0x44, 0x008657c9,},   //1.0625     //0.524777
    {0x00, 0x40, 0x40, 0x48, 0x0105013a,},   //1.1250     //1.019550
    {0x00, 0x40, 0x40, 0x4c, 0x017cd110,},   //1.1875     //1.487565
    {0x00, 0x40, 0x40, 0x50, 0x01ee7b47,},   //1.2500     //1.931569
    {0x00, 0x40, 0x40, 0x54, 0x025a997c,},   //1.3125     //2.353905
    {0x00, 0x40, 0x40, 0x58, 0x02c1afdd,},   //1.3750     //2.756590
    {0x00, 0x40, 0x40, 0x5c, 0x032430f0,},   //1.4375     //3.141372
    {0x00, 0x40, 0x40, 0x60, 0x0382809d,},   //1.5000     //3.509775
    {0x00, 0x40, 0x40, 0x64, 0x03dcf68e,},   //1.5625     //3.863137
    {0x00, 0x40, 0x40, 0x68, 0x0433e01a,},   //1.6250     //4.202638
    {0x00, 0x40, 0x40, 0x6c, 0x048781d8,},   //1.6875     //4.529325
    {0x00, 0x40, 0x40, 0x70, 0x04d818df,},   //1.7500     //4.844130
    {0x00, 0x40, 0x40, 0x74, 0x0525dbda,},   //1.8125     //5.147886
    {0x00, 0x40, 0x40, 0x78, 0x0570fbe4,},   //1.8750     //5.441344
    {0x00, 0x40, 0x40, 0x7c, 0x05b9a541,},   //1.9375     //5.725178
    {0x00, 0x40, 0x40, 0x80, 0x06000000,},   //2.0000     //6.000000
    {0x00, 0x40, 0x40, 0x84, 0x0644307a,},   //2.0625     //6.266365
    {0x00, 0x40, 0x40, 0x88, 0x068657c9,},   //2.1250     //6.524777
    {0x00, 0x40, 0x40, 0x8c, 0x06c69426,},   //2.1875     //6.775698
    {0x00, 0x40, 0x40, 0x90, 0x0705013a,},   //2.2500     //7.019550
    {0x00, 0x40, 0x40, 0x94, 0x0741b86a,},   //2.3125     //7.256720
    {0x00, 0x40, 0x40, 0x98, 0x077cd110,},   //2.3750     //7.487565
    {0x00, 0x40, 0x40, 0x9c, 0x07b660b8,},   //2.4375     //7.712413
    {0x00, 0x40, 0x40, 0xa0, 0x07ee7b47,},   //2.5000     //7.931569
    {0x00, 0x40, 0x40, 0xa4, 0x0825332b,},   //2.5625     //8.145312
    {0x00, 0x40, 0x40, 0xa8, 0x085a997c,},   //2.6250     //8.353905
    {0x00, 0x40, 0x40, 0xac, 0x088ebe1f,},   //2.6875     //8.557589
    {0x00, 0x40, 0x40, 0xb0, 0x08c1afdd,},   //2.7500     //8.756590
    {0x00, 0x40, 0x40, 0xb4, 0x08f37c81,},   //2.8125     //8.951119
    {0x00, 0x40, 0x40, 0xb8, 0x092430f0,},   //2.8750     //9.141372
    {0x00, 0x40, 0x40, 0xbc, 0x0953d935,},   //2.9375     //9.327533
    {0x00, 0x40, 0x40, 0xc0, 0x0982809d,},   //3.0000     //9.509775
    {0x00, 0x40, 0x40, 0xc4, 0x09b031be,},   //3.0625     //9.688259
    {0x00, 0x40, 0x40, 0xc8, 0x09dcf68e,},   //3.1250     //9.863137
    {0x00, 0x40, 0x40, 0xcc, 0x0a08d867,},   //3.1875     //10.034552
    {0x00, 0x40, 0x40, 0xd0, 0x0a33e01a,},   //3.2500     //10.202638
    {0x00, 0x40, 0x40, 0xd4, 0x0a5e15f8,},   //3.3125     //10.367523
    {0x00, 0x40, 0x40, 0xd8, 0x0a8781d8,},   //3.3750     //10.529325
    {0x00, 0x40, 0x40, 0xdc, 0x0ab02b24,},   //3.4375     //10.688158
    {0x00, 0x40, 0x40, 0xe0, 0x0ad818df,},   //3.5000     //10.844130
    {0x00, 0x40, 0x40, 0xe4, 0x0aff51ae,},   //3.5625     //10.997340
    {0x00, 0x40, 0x40, 0xe8, 0x0b25dbda,},   //3.6250     //11.147886
    {0x00, 0x40, 0x40, 0xec, 0x0b4bbd5e,},   //3.6875     //11.295858
    {0x00, 0x40, 0x40, 0xf0, 0x0b70fbe4,},   //3.7500     //11.441344
    {0x00, 0x40, 0x40, 0xf4, 0x0b959cd0,},   //3.8125     //11.584424
    {0x00, 0x40, 0x40, 0xf8, 0x0bb9a541,},   //3.8750     //11.725178
    {0x00, 0x40, 0x40, 0xfc, 0x0bdd1a1a,},   //3.9375     //11.863680
    {0x00, 0x40, 0x40, 0xff, 0x0bf7573d,},   //3.984400,  //11.966175
    {0x00, 0x40, 0x44, 0xff, 0x0c7daf07,},   //4.233425,  //12.490952
    {0x00, 0x40, 0x48, 0xff, 0x0cfc5878,},   //4.482450,  //12.985725
    {0x00, 0x40, 0x4c, 0xff, 0x0d74284e,},   //4.731475,  //13.453740
    {0x00, 0x40, 0x50, 0xff, 0x0de5d284,},   //4.980500,  //13.897744
    {0x00, 0x40, 0x54, 0xff, 0x0e51f0ba,},   //5.229525,  //14.320079
    {0x00, 0x40, 0x58, 0xff, 0x0eb9071a,},   //5.478550,  //14.722765
    {0x00, 0x40, 0x5c, 0xff, 0x0f1b882d,},   //5.727575,  //15.107547
    {0x00, 0x40, 0x60, 0xff, 0x0f79d7da,},   //5.976600,  //15.475950
    {0x00, 0x40, 0x64, 0xff, 0x0fd44dcb,},   //6.225625,  //15.829312
    {0x00, 0x40, 0x68, 0xff, 0x102b3758,},   //6.474650,  //16.168813
    {0x00, 0x40, 0x6c, 0xff, 0x107ed915,},   //6.723675,  //16.495500
    {0x00, 0x40, 0x70, 0xff, 0x10cf701d,},   //6.972700,  //16.810304
    {0x00, 0x40, 0x74, 0xff, 0x111d3318,},   //7.221725,  //17.114061
    {0x00, 0x40, 0x78, 0xff, 0x11685322,},   //7.470750,  //17.407519
    {0x00, 0x40, 0x7c, 0xff, 0x11b0fc7f,},   //7.719775,  //17.691353
    {0x00, 0x40, 0x80, 0xff, 0x11f7573d,},   //7.968800,  //17.966175
    {0x00, 0x40, 0x84, 0xff, 0x123b87b7,},   //8.217825,  //18.232540
    {0x00, 0x40, 0x88, 0xff, 0x127daf07,},   //8.466850,  //18.490952
    {0x00, 0x40, 0x8c, 0xff, 0x12bdeb64,},   //8.715875,  //18.741873
    {0x00, 0x40, 0x90, 0xff, 0x12fc5878,},   //8.964900,  //18.985725
    {0x00, 0x40, 0x94, 0xff, 0x13390fa7,},   //9.213925,  //19.222895
    {0x00, 0x40, 0x98, 0xff, 0x1374284e,},   //9.462950,  //19.453740
    {0x00, 0x40, 0x9c, 0xff, 0x13adb7f5,},   //9.711975,  //19.678588
    {0x00, 0x40, 0xa0, 0xff, 0x13e5d284,},   //9.961000,  //19.897744
    {0x00, 0x40, 0xa4, 0xff, 0x141c8a68,},   //10.210025, //20.111487
    {0x00, 0x40, 0xa8, 0xff, 0x1451f0ba,},   //10.459050, //20.320079
    {0x00, 0x40, 0xac, 0xff, 0x1486155c,},   //10.708075, //20.523763
    {0x00, 0x40, 0xb0, 0xff, 0x14b9071a,},   //10.957100, //20.722765
    {0x00, 0x40, 0xb4, 0xff, 0x14ead3bf,},   //11.206125, //20.917294
    {0x00, 0x40, 0xb8, 0xff, 0x151b882d,},   //11.455150, //21.107547
    {0x00, 0x40, 0xbc, 0xff, 0x154b3073,},   //11.704175, //21.293708
    {0x00, 0x40, 0xc0, 0xff, 0x1579d7da,},   //11.953200, //21.475950
    {0x00, 0x40, 0xc4, 0xff, 0x15a788fc,},   //12.202225, //21.654434
    {0x00, 0x40, 0xc8, 0xff, 0x15d44dcb,},   //12.451250, //21.829312
    {0x00, 0x40, 0xcc, 0xff, 0x16002fa4,},   //12.700275, //22.000727
    {0x00, 0x40, 0xc0, 0xff, 0x162b3758,},   //12.949300, //22.168813
    {0x00, 0x40, 0xc4, 0xff, 0x16556d35,},   //13.198325, //22.333698
    {0x00, 0x40, 0xc8, 0xff, 0x167ed915,},   //13.447350, //22.495500
    {0x00, 0x40, 0xcc, 0xff, 0x16a78261,},   //13.696375, //22.654333
    {0x00, 0x40, 0xe0, 0xff, 0x16cf701d,},   //13.945400, //22.810304
    {0x00, 0x40, 0xe4, 0xff, 0x16f6a8eb,},   //14.194425, //22.963515
    {0x00, 0x40, 0xe8, 0xff, 0x171d3318,},   //14.443450, //23.114061
    {0x00, 0x40, 0xec, 0xff, 0x1743149c,},   //14.692475, //23.262033
    {0x00, 0x40, 0xf0, 0xff, 0x17685322,},   //14.941500, //23.407519
    {0x00, 0x40, 0xf4, 0xff, 0x178cf40d,},   //15.190525, //23.550599
    {0x00, 0x40, 0xf8, 0xff, 0x17b0fc7f,},   //15.439550, //23.691353
    {0x00, 0x40, 0xfc, 0xff, 0x17d47157,},   //15.688575, //23.829854
    {0x00, 0x40, 0xff, 0xff, 0x17eeae77,},   //15.875443, //23.932350
    {0x00, 0x44, 0xff, 0xff, 0x18750645,},   //16.867659, //24.457127
    {0x00, 0x48, 0xff, 0xff, 0x18f3afb5,},   //17.859874, //24.951900
    {0x00, 0x4c, 0xff, 0xff, 0x196b7f8b,},   //18.852089, //25.419915
    {0x00, 0x50, 0xff, 0xff, 0x19dd29c2,},   //19.844304, //25.863918
    {0x00, 0x54, 0xff, 0xff, 0x1a4947f7,},   //20.836519, //26.286254
    {0x00, 0x58, 0xff, 0xff, 0x1ab05e58,},   //21.828735, //26.688940
    {0x00, 0x5c, 0xff, 0xff, 0x1b12df6b,},   //22.820950, //27.073722
    {0x00, 0x60, 0xff, 0xff, 0x1b712f18,},   //23.813165, //27.442125
    {0x00, 0x64, 0xff, 0xff, 0x1bcba509,},   //24.805380, //27.795487
    {0x00, 0x68, 0xff, 0xff, 0x1c228e95,},   //25.797595, //28.134988
    {0x00, 0x6c, 0xff, 0xff, 0x1c763053,},   //26.789811, //28.461675
    {0x00, 0x70, 0xff, 0xff, 0x1cc6c75a,},   //27.782026, //28.776479
    {0x00, 0x74, 0xff, 0xff, 0x1d148a56,},   //28.774241, //29.080236
    {0x00, 0x78, 0xff, 0xff, 0x1d5faa5f,},   //29.766456, //29.373693
    {0x00, 0x7c, 0xff, 0xff, 0x1da853bc,},   //30.758672, //29.657528
    {0x00, 0x80, 0xff, 0xff, 0x1deeae7b,},   //31.750887, //29.932350
    {0x00, 0x84, 0xff, 0xff, 0x1e32def5,},   //32.743102, //30.198715
    {0x00, 0x88, 0xff, 0xff, 0x1e750645,},   //33.735317, //30.457127
    {0x00, 0x8c, 0xff, 0xff, 0x1eb542a1,},   //34.727532, //30.708048
    {0x00, 0x90, 0xff, 0xff, 0x1ef3afb5,},   //35.719748, //30.951900
    {0x00, 0x94, 0xff, 0xff, 0x1f3066e5,},   //36.711963, //31.189070
    {0x00, 0x98, 0xff, 0xff, 0x1f6b7f8b,},   //37.704178, //31.419915
    {0x00, 0x9c, 0xff, 0xff, 0x1fa50f33,},   //38.696393, //31.644763
    {0x00, 0xa0, 0xff, 0xff, 0x1fdd29c2,},   //39.688608, //31.863918
    {0x00, 0xa4, 0xff, 0xff, 0x2013e1a6,},   //40.680824, //32.077662
    {0x00, 0xa8, 0xff, 0xff, 0x204947f7,},   //41.673039, //32.286254
    {0x00, 0xac, 0xff, 0xff, 0x207d6c9a,},   //42.665254, //32.489938
    {0x00, 0xb0, 0xff, 0xff, 0x20b05e58,},   //43.657469, //32.688940
    {0x00, 0xb4, 0xff, 0xff, 0x20e22afc,},   //44.649684, //32.883468
    {0x00, 0xb8, 0xff, 0xff, 0x2112df6b,},   //45.641900, //33.073722
    {0x00, 0xbc, 0xff, 0xff, 0x214287b0,},   //46.634115, //33.259883
    {0x00, 0xc0, 0xff, 0xff, 0x21712f18,},   //47.626330, //33.442125
    {0x00, 0xc4, 0xff, 0xff, 0x219ee03a,},   //48.618545, //33.620609
    {0x00, 0xc8, 0xff, 0xff, 0x21cba509,},   //49.610760, //33.795487
    {0x00, 0xcc, 0xff, 0xff, 0x21f786e2,},   //50.602976, //33.966902
    {0x00, 0xd0, 0xff, 0xff, 0x22228e95,},   //51.595191, //34.134988
    {0x00, 0xd4, 0xff, 0xff, 0x224cc473,},   //52.587406, //34.299873
    {0x00, 0xd8, 0xff, 0xff, 0x22763053,},   //53.579621, //34.461675
    {0x00, 0xdc, 0xff, 0xff, 0x229ed99f,},   //54.571837, //34.620508
    {0x00, 0xe0, 0xff, 0xff, 0x22c6c75a,},   //55.564052, //34.776479
    {0x00, 0xe4, 0xff, 0xff, 0x22ee0029,},   //56.556267, //34.929690
    {0x00, 0xe8, 0xff, 0xff, 0x23148a56,},   //57.548482, //35.080236
    {0x00, 0xec, 0xff, 0xff, 0x233a6bd9,},   //58.540697, //35.228208
    {0x00, 0xf0, 0xff, 0xff, 0x235faa5f,},   //59.532913, //35.373693
    {0x00, 0xf4, 0xff, 0xff, 0x23844b4b,},   //60.525128, //35.516774
    {0x00, 0xf8, 0xff, 0xff, 0x23a853bc,},   //61.517343, //35.657528
    {0x00, 0xfc, 0xff, 0xff, 0x23cbc895,},   //62.509558, //35.796029
    {0x00, 0xff, 0xff, 0xff, 0x23e605b8,},   //63.254117, //35.898525
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
static int gc0328_get_vblank_time(int handle, GK_Vi_SensorT* sensor_extern, u32* ptime, u32 extclk, u32 pclk)
{
    int                     errorCode       = 0;
    u64                     v_btime;
    //u32                     vertical_lines  = 0;
    u32                     line_length     = 0;

    u32     hb_h, hb_l, sh_delay, win_width_h, win_width_l;
    u32     vb_h, vb_l;  //win_height_h, win_height_l;
    u32     vb;

    //page set to 0.
    sensor_write_reg_ex(handle, GC0328_REG_PAGE_SET, 0x0,1);  //page set to 0.
    errorCode |= sensor_read_reg_ex(handle, GC0328_REG_VB_H, &vb_h, 1);
    errorCode |= sensor_read_reg_ex(handle, GC0328_REG_VB_L, &vb_l, 1);
    //errorCode |= sensor_read_reg_ex(handle, GC0328_REG_WIN_HEIGHT_H, &win_height_h, 1);
    //errorCode |= sensor_read_reg_ex(handle, GC0328_REG_WIN_HEIGHT_L, &win_height_l, 1);
    vb = (vb_h<<8) + vb_l;
    //vertical_lines = (vb_h<<8) + vb_l + (win_height_h<<8) + win_height_l;
    //BUG_ON(vertical_lines == 0);
    errorCode |= sensor_read_reg_ex(handle, GC0328_REG_HB_H, &hb_h, 1);
    errorCode |= sensor_read_reg_ex(handle, GC0328_REG_HB_L, &hb_l, 1);
    errorCode |= sensor_read_reg_ex(handle, GC0328_REG_SH_DELAY, &sh_delay, 1);
    errorCode |= sensor_read_reg_ex(handle, GC0328_REG_WIN_WIDTH_H, &win_width_h, 1);
    errorCode |= sensor_read_reg_ex(handle, GC0328_REG_WIN_WIDTH_L, &win_width_l, 1);
    line_length = ((hb_h)<<8) + hb_l + sh_delay + ((win_width_h)<<8) + win_width_l + 4;
    BUG_ON(line_length == 0);

    v_btime = (u64)line_length * (u64)vb * 1000000000;
    // ns
    //if(sensor.vmax_clk_ref == GK_SENSOR_CAL_CLK_PIXCLK_DIV2)
    DO_DIV_ROUND(v_btime, (u64)pclk / 2);

    *ptime = v_btime;

    return errorCode;
}

static int gc0328_set_agc_db(int handle, GK_Vi_SensorT* sensor_extern, s32 agc_db)
{
    int                     errorCode = 0;
    u32                     idc_reg0, idc_reg1, idc_reg2, idc_reg3;
    u32                     gain_index = GC0328_GAIN_ROWS;
    int                     i;
    GK_Vi_Sensor_GC0328T*   pdata = (GK_Vi_Sensor_GC0328T*)sensor_extern->private_data;
    //printk("[%s %d]%08x\n", __func__, __LINE__, agc_db);

    for(i = 0; i < GC0328_GAIN_ROWS; i++)
    {
        if(GC0328_GLOBAL_GAIN_TABLE[i].db_value > agc_db)
        {
            gain_index = i;
            break;
        }
    }
    if(gain_index>0)
        gain_index--;

    if (gain_index >= GC0328_GAIN_ROWS)
    {
        errorCode = -EINVAL;
        return errorCode;
    }
    idc_reg0 = GC0328_GLOBAL_GAIN_TABLE[gain_index].GC0328_REG_FE;
    idc_reg1 = GC0328_GLOBAL_GAIN_TABLE[gain_index].GC0328_REG_70;
    idc_reg2 = GC0328_GLOBAL_GAIN_TABLE[gain_index].GC0328_REG_71;
    idc_reg3 = GC0328_GLOBAL_GAIN_TABLE[gain_index].GC0328_REG_72;
#ifdef SET_REG_BY_VSYNC
    pdata->gc0328_reg_fe = idc_reg0;
    pdata->gc0328_reg_70 = idc_reg1;
    pdata->gc0328_reg_71 = idc_reg2;
    pdata->gc0328_reg_72 = idc_reg3;
    pdata->gc0328_adc_ch = 1;
#else
    sensor_write_reg_ex(handle, 0xFE, idc_reg0,1);  //page set
    sensor_write_reg_ex(handle, GC0328_REG_GLOBAL_GIAN, idc_reg1,1);  //global_gain
    sensor_write_reg_ex(handle, GC0328_REG_PRE_GIAN, idc_reg2,1);     //pre_gain
    sensor_write_reg_ex(handle, GC0328_REG_POST_GIAN, idc_reg3,1);    //post_gain
#endif

    return errorCode;
}

static int gc0328_set_shutter_time(int handle, GK_Vi_SensorT* sensor_extern, u32 shutter_time, u32 extclk, u32 pclk)
{
    int     errorCode = 0;
    u64     exposure_time_q9;
    u32     frame_length, line_length;
    u32     hb_h, hb_l, sh_delay, win_width_h, win_width_l;
    u32     vb_h, vb_l, win_height_h, win_height_l;
    u32     exp_h, exp_l;
    GK_Vi_Sensor_GC0328T*   pdata = (GK_Vi_Sensor_GC0328T*)sensor_extern->private_data;
    //u8     vts1, vts2;
    //printk("[%s %d]%08x\n", __func__, __LINE__, shutter_time);

    exposure_time_q9 = shutter_time;

    //page set to 0.
    sensor_write_reg_ex(handle, 0xFE, 0x0,1);  //page set to 0.
    errorCode |= sensor_read_reg_ex(handle, GC0328_REG_VB_H, &vb_h, 1);
    errorCode |= sensor_read_reg_ex(handle, GC0328_REG_VB_L, &vb_l, 1);
    errorCode |= sensor_read_reg_ex(handle, GC0328_REG_WIN_HEIGHT_H, &win_height_h, 1);
    errorCode |= sensor_read_reg_ex(handle, GC0328_REG_WIN_HEIGHT_L, &win_height_l, 1);
    frame_length = (vb_h<<8) + vb_l + (win_height_h<<8) + win_height_l;
    errorCode |= sensor_read_reg_ex(handle, GC0328_REG_HB_H, &hb_h, 1);
    errorCode |= sensor_read_reg_ex(handle, GC0328_REG_HB_L, &hb_l, 1);
    errorCode |= sensor_read_reg_ex(handle, GC0328_REG_SH_DELAY, &sh_delay, 1);
    errorCode |= sensor_read_reg_ex(handle, GC0328_REG_WIN_WIDTH_H, &win_width_h, 1);
    errorCode |= sensor_read_reg_ex(handle, GC0328_REG_WIN_WIDTH_L, &win_width_l, 1);
    line_length = ((hb_h)<<8) + hb_l + sh_delay + ((win_width_h)<<8) + win_width_l + 4;

    /* t(frame)/t(per line) = t(exposure, in lines)*/
    exposure_time_q9 = exposure_time_q9 * (u64)pclk / 2;// gc0328 spec: Hmax unit : pclk/2
    DO_DIV_ROUND(exposure_time_q9, line_length);
    DO_DIV_ROUND(exposure_time_q9, 512000000);

    if((frame_length - 4) < exposure_time_q9)
    {
        exposure_time_q9 = frame_length - 4;
    }

    exp_h = (exposure_time_q9 >> 8) & 0x0f;
    exp_l = exposure_time_q9 & 0xff;

#ifdef SET_REG_BY_VSYNC
#ifdef SET_SHUTTER_BY_VSYNC
    pdata->gc0328_reg_exp_time_h = exp_h;
    pdata->gc0328_reg_exp_time_l = exp_l;
    pdata->gc0328_shutter_ch = 1;
#else
    errorCode |= sensor_write_reg_ex(handle, GC0328_REG_EXP_TIME_H, exp_h, 1);  //
    errorCode |= sensor_write_reg_ex(handle, GC0328_REG_EXP_TIME_L, exp_l, 1);
#endif
#else
    errorCode |= sensor_write_reg_ex(handle, GC0328_REG_EXP_TIME_H, exp_h, 1);  //
    errorCode |= sensor_write_reg_ex(handle, GC0328_REG_EXP_TIME_L, exp_l, 1);
#endif

    return errorCode;
}

static int gc0328_set_fps(int handle, GK_Vi_SensorT* sensor_extern, u32 fps, u32 extclk, u32 pclk)
{
    int                             errorCode           = 0;
    //printk("[%s %d]%08x\n", __func__, __LINE__, fps);
    printk("GC0328c don't support fps auto setting! \n");

    return errorCode;
}

static int gc0328_set_update_reg(int handle, GK_Vi_SensorT* sensor_extern)
{
    u32 shutter_chnged =0;
    GK_Vi_Sensor_GC0328T*   pdata = (GK_Vi_Sensor_GC0328T*)sensor_extern->private_data;
    //printk("[%s %d]\n", __func__, __LINE__);

#ifdef SET_REG_BY_VSYNC
#ifdef SET_SHUTTER_BY_VSYNC
    if(pdata->gc0328_shutter_ch == 1)
    {
        if(pdata->gc0328_last_reg_exp_time_h!=pdata->gc0328_reg_exp_time_h)
        {
            sensor_write_reg_ex(handle, GC0328_REG_EXP_TIME_H, pdata->gc0328_reg_exp_time_h,0);
        }
        if(pdata->gc0328_last_reg_exp_time_l!=pdata->gc0328_reg_exp_time_l)
        {
            sensor_write_reg_ex(handle, GC0328_REG_EXP_TIME_L, pdata->gc0328_reg_exp_time_l,0);
        }

        pdata->gc0328_last_reg_exp_time_h = pdata->gc0328_reg_exp_time_h;
        pdata->gc0328_last_reg_exp_time_l = pdata->gc0328_reg_exp_time_l;
        pdata->gc0328_shutter_ch = 0;
        shutter_chnged = 1;
    }
#endif
    if(pdata->gc0328_adc_ch == 1)
    {
        if (pdata->gc0328_last_reg_fe != pdata->gc0328_reg_fe)
        {
            sensor_write_reg_ex(handle, GC0328_REG_PAGE_SET, pdata->gc0328_reg_fe,0);
        }
        if (pdata->gc0328_last_reg_70 != pdata->gc0328_reg_70)
        {
            sensor_write_reg_ex(handle, GC0328_REG_GLOBAL_GIAN, pdata->gc0328_reg_70,0);
        }
        if (pdata->gc0328_last_reg_71 != pdata->gc0328_reg_71)
        {
            sensor_write_reg_ex(handle, GC0328_REG_PRE_GIAN, pdata->gc0328_reg_71,0);
        }
        if (pdata->gc0328_last_reg_72 != pdata->gc0328_reg_72)
        {
            sensor_write_reg_ex(handle, GC0328_REG_POST_GIAN, pdata->gc0328_reg_72,0);
        }
        pdata->gc0328_adc_ch = 0;
        pdata->gc0328_last_reg_fe = pdata->gc0328_reg_fe;
        pdata->gc0328_last_reg_70 = pdata->gc0328_reg_70;
        pdata->gc0328_last_reg_71 = pdata->gc0328_reg_71;
        pdata->gc0328_last_reg_72 = pdata->gc0328_reg_72;
    }

#endif
    return 0;
}

int gc0328_probe(GK_Vi_SensorT* sensor_extern, int id)
{
    GK_Vi_Sensor_GC0328T *gc0328_data = NULL;

    sensor_extern->private_data = kzalloc(sizeof(GK_Vi_Sensor_GC0328T), GFP_KERNEL);
    if (sensor_extern->private_data == NULL)
    {
        printk("kzalloc memory(0x%04x) failed\n", sizeof(GK_Vi_Sensor_GC0328T));
        return -1;
    }

    gc0328_data = sensor_extern->private_data;

    gc0328_data->sub_id           = id;
#ifdef SET_REG_BY_VSYNC
    gc0328_data->gc0328_adc_ch    = 0;
    gc0328_data->gc0328_reg_fe    = 0x0;
    gc0328_data->gc0328_reg_70    = 0x40;
    gc0328_data->gc0328_reg_71    = 0x40;
    gc0328_data->gc0328_reg_72    = 0x40;

    gc0328_data->gc0328_last_reg_fe   = 0xffffffff;
    gc0328_data->gc0328_last_reg_70   = 0xffffffff;
    gc0328_data->gc0328_last_reg_71   = 0xffffffff;
    gc0328_data->gc0328_last_reg_72   = 0xffffffff;

#ifdef SET_SHUTTER_BY_VSYNC
    gc0328_data->gc0328_shutter_ch     = 0;
    gc0328_data->gc0328_reg_exp_time_h = 0;
    gc0328_data->gc0328_reg_exp_time_l = 0;

    gc0328_data->gc0328_last_reg_exp_time_h = 0xffffffff;
    gc0328_data->gc0328_last_reg_exp_time_l = 0xffffffff;
#endif
#endif
    return 0;
}

int gc0328_remove(GK_Vi_SensorT* sensor_extern)
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
static int __init gc0328_init(void)
{
    gc0328_dev.sensor_id                = GK_VI_SENSOR_GC0328;
    gc0328_dev.length                   = sizeof(gc0328_dev);
    gc0328_dev.sensor_get_vblank_time   = gc0328_get_vblank_time;
    gc0328_dev.sensor_set_shutter_time  = gc0328_set_shutter_time;
    gc0328_dev.sensor_set_fps           = gc0328_set_fps;
    gc0328_dev.sensor_set_agc_db        = gc0328_set_agc_db;
    gc0328_dev.sensor_set_update_reg    = gc0328_set_update_reg;

    gc0328_dev.private_data             = NULL;
    gc0328_dev.probe                    = gc0328_probe;
    gc0328_dev.remove                   = gc0328_remove;

    sensor_registry_api((GK_Vi_SensorT*)&gc0328_dev);
    return 0;
}

static void __exit gc0328_exit(void)
{
    sensor_unregistry_api((GK_Vi_SensorT*)&gc0328_dev);
}

module_init(gc0328_init);
module_exit(gc0328_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("GOKE MICROELECTRONICS Inc.");
MODULE_DESCRIPTION("GC0328 1/6.5-Inch CMOS Digital Image Sensor");

