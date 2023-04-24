/*!
*****************************************************************************
** \file        software/linux/kernel/drv/sensors/sc3035/sc3035.c
**
** \version     $Id: sc3035_ex.c 12569 2017-08-15 03:48:54Z wenruidan $
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
#include "sc3035_pri.h"

typedef struct
{
    int sub_id;
    //GK_Vi_SensorT api;
    int sc3035_adc_ch;
    int sc3035_reg_3630;
    int sc3035_reg_3627;
    int sc3035_reg_3620;
    int sc3035_reg_3e08;  //default: 1x gain total.
    int sc3035_reg_3e09;

    int sc3035_last_reg_3630;
    int sc3035_last_reg_3627;
    int sc3035_last_reg_3620;
    int sc3035_last_reg_3e08;
    int sc3035_last_reg_3e09;

    int sc3035_shutter_ch;
    int sc3035_reg_exp_time_h;
    int sc3035_reg_exp_time_l;

    int sc3035_last_reg_exp_time_h;
    int sc3035_last_reg_exp_time_l;
}GK_Vi_Sensor_SC3035T;

//*****************************************************************************
//*****************************************************************************
//** Local structures
//*****************************************************************************
//*****************************************************************************
#define sc3035_GAIN_ROWS            112
typedef struct sc3035_gain_table
{
    u32 db_value;
    struct
    {
        u16 sc3035_REG_3630;
        u16 sc3035_REG_3627;
        u16 sc3035_REG_3620;
        u16 sc3035_REG_3E08;
        u16 sc3035_REG_3E09;
    }regs;
}sc3035_gain_table_s;


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
static int sc3035_set_update_reg(int handle, GK_Vi_SensorT* sensor_extern);
static int sc3035_set_fps(int handle, GK_Vi_SensorT* sensor_extern, u32 fps, u32 extclk, u32 pclk);
static int sc3035_set_shutter_time(int handle, GK_Vi_SensorT* sensor_extern, u32 shutter_time, u32 extclk, u32 pclk);
static int sc3035_set_agc_db(int handle, GK_Vi_SensorT* sensor_extern, s32 agc_db);
static GK_Vi_SensorT sc3035_dev;
#if 0
static GK_Vi_Sensor_SC3035T sc3035 =
{
    .api =
    {
        .sensor_id                = GK_VI_SENSOR_SC3035,
        .length                   = sizeof(GK_Vi_Sensor_SC3035T),
        .sensor_get_vblank_time   = NULL,
        .sensor_set_shutter_time  = sc3035_set_shutter_time,
        .sensor_set_fps           = sc3035_set_fps,
        .sensor_set_agc_db        = sc3035_set_agc_db,
        .sensor_set_mirror_mode   = NULL,
        .sensor_set_update_reg    = sc3035_set_update_reg,
    },
    .sc3035_adc_ch = 0,
    .sc3035_reg_3630 = 0xa9,
    .sc3035_reg_3627 = 0x02,
    .sc3035_reg_3620 = 0xd2,
    .sc3035_reg_3e08 = 0x00,    //default: 1x gain total.
    .sc3035_reg_3e09 = 0x10,

    .sc3035_last_reg_3630 = 0xffffffff,
    .sc3035_last_reg_3627 = 0xffffffff,
    .sc3035_last_reg_3620 = 0xffffffff,
    .sc3035_last_reg_3e08 = 0xffffffff,
    .sc3035_last_reg_3e09 = 0xffffffff,

    .sc3035_shutter_ch     = 0,
    .sc3035_reg_exp_time_h = 0,
    .sc3035_reg_exp_time_l = 0,

    .sc3035_last_reg_exp_time_h = 0xffffffff,
    .sc3035_last_reg_exp_time_l = 0xffffffff,
};
#endif
const sc3035_gain_table_s sc3035_GLOBAL_GAIN_TABLE[sc3035_GAIN_ROWS] =
{
      //        0x3630              0x3627              0x3620           0x3e08[15:0]   0x3e09[15:0]
      //      restriction         restriction         restriction
      // (  gain < 2: 0xa9 ) ( gain < 16: 0x02 ) (  gain < 2: 0xd2 )    //when 0x3e03=0x03
      // (  gain >= 2: 0x67) ( gain >= 16: 0x06) (  gain >= 2: 0x62)    //gain=(0x3e08,0x3e09)/0x10.  max:0x7c0 for 124x.
      { 0x00000000,  {0xa9,  0x02,  0xd2,  0x00,  0x10, }},//  1.000000,  //0.000000   //
      { 0x008657c9,  {0xa9,  0x02,  0xd2,  0x00,  0x11, }},//  1.062500,  //0.524777   //
      { 0x0105013a,  {0xa9,  0x02,  0xd2,  0x00,  0x12, }},//  1.125000,  //1.019550   //
      { 0x017cd110,  {0xa9,  0x02,  0xd2,  0x00,  0x13, }},//  1.187500,  //1.487565   //
      { 0x01ee7b47,  {0xa9,  0x02,  0xd2,  0x00,  0x14, }},//  1.250000,  //1.931569   //
      { 0x025a997c,  {0xa9,  0x02,  0xd2,  0x00,  0x15, }},//  1.312500,  //2.353905   //
      { 0x02c1afdd,  {0xa9,  0x02,  0xd2,  0x00,  0x16, }},//  1.375000,  //2.756590   //
      { 0x032430f0,  {0xa9,  0x02,  0xd2,  0x00,  0x17, }},//  1.437500,  //3.141372   //
      { 0x0382809d,  {0xa9,  0x02,  0xd2,  0x00,  0x18, }},//  1.500000,  //3.509775   //
      { 0x03dcf68e,  {0xa9,  0x02,  0xd2,  0x00,  0x19, }},//  1.562500,  //3.863137   //
      { 0x0433e01a,  {0xa9,  0x02,  0xd2,  0x00,  0x1a, }},//  1.625000,  //4.202638   //
      { 0x048781d8,  {0xa9,  0x02,  0xd2,  0x00,  0x1b, }},//  1.687500,  //4.529325   //
      { 0x04d818df,  {0xa9,  0x02,  0xd2,  0x00,  0x1c, }},//  1.750000,  //4.844130   //
      { 0x0525dbda,  {0xa9,  0x02,  0xd2,  0x00,  0x1d, }},//  1.812500,  //5.147886   //
      { 0x0570fbe4,  {0xa9,  0x02,  0xd2,  0x00,  0x1e, }},//  1.875000,  //5.441344   //
      { 0x05b9a541,  {0xa9,  0x02,  0xd2,  0x00,  0x1f, }},//  1.937500,  //5.725178   //
      { 0x06000000,  {0x67,  0x02,  0x62,  0x00,  0x20, }},//  2.000000,  //6.000000   //
      { 0x068657c9,  {0x67,  0x02,  0x62,  0x00,  0x22, }},//  2.125000,  //6.524777   //
      { 0x0705013a,  {0x67,  0x02,  0x62,  0x00,  0x24, }},//  2.250000,  //7.019550   //
      { 0x077cd110,  {0x67,  0x02,  0x62,  0x00,  0x26, }},//  2.375000,  //7.487565   //
      { 0x07ee7b47,  {0x67,  0x02,  0x62,  0x00,  0x28, }},//  2.500000,  //7.931569   //
      { 0x085a997c,  {0x67,  0x02,  0x62,  0x00,  0x2a, }},//  2.625000,  //8.353905   //
      { 0x08c1afdd,  {0x67,  0x02,  0x62,  0x00,  0x2c, }},//  2.750000,  //8.756590   //
      { 0x092430f0,  {0x67,  0x02,  0x62,  0x00,  0x2e, }},//  2.875000,  //9.141372   //
      { 0x0982809d,  {0x67,  0x02,  0x62,  0x00,  0x30, }},//  3.000000,  //9.509775   //
      { 0x09dcf68e,  {0x67,  0x02,  0x62,  0x00,  0x32, }},//  3.125000,  //9.863137   //
      { 0x0a33e01a,  {0x67,  0x02,  0x62,  0x00,  0x34, }},//  3.250000,  //10.202638  //
      { 0x0a8781d8,  {0x67,  0x02,  0x62,  0x00,  0x36, }},//  3.375000,  //10.529325  //
      { 0x0ad818df,  {0x67,  0x02,  0x62,  0x00,  0x38, }},//  3.500000,  //10.844130  //
      { 0x0b25dbda,  {0x67,  0x02,  0x62,  0x00,  0x3a, }},//  3.625000,  //11.147886  //
      { 0x0b70fbe4,  {0x67,  0x02,  0x62,  0x00,  0x3c, }},//  3.750000,  //11.441344  //
      { 0x0bb9a541,  {0x67,  0x02,  0x62,  0x00,  0x3e, }},//  3.875000,  //11.725178  //
      { 0x0c000000,  {0x67,  0x02,  0x62,  0x00,  0x40, }},//  4.000000,  //12.000000  //
      { 0x0c8657c9,  {0x67,  0x02,  0x62,  0x00,  0x44, }},//  4.250000,  //12.524777  //
      { 0x0d05013a,  {0x67,  0x02,  0x62,  0x00,  0x48, }},//  4.500000,  //13.019550  //
      { 0x0d7cd110,  {0x67,  0x02,  0x62,  0x00,  0x4c, }},//  4.750000,  //13.487565  //
      { 0x0dee7b47,  {0x67,  0x02,  0x62,  0x00,  0x50, }},//  5.000000,  //13.931569  //
      { 0x0e5a997c,  {0x67,  0x02,  0x62,  0x00,  0x54, }},//  5.250000,  //14.353905  //
      { 0x0ec1afdd,  {0x67,  0x02,  0x62,  0x00,  0x58, }},//  5.500000,  //14.756590  //
      { 0x0f2430f0,  {0x67,  0x02,  0x62,  0x00,  0x5c, }},//  5.750000,  //15.141372  //
      { 0x0f82809d,  {0x67,  0x02,  0x62,  0x00,  0x60, }},//  6.000000,  //15.509775  //
      { 0x0fdcf68e,  {0x67,  0x02,  0x62,  0x00,  0x64, }},//  6.250000,  //15.863137  //
      { 0x1033e01a,  {0x67,  0x02,  0x62,  0x00,  0x68, }},//  6.500000,  //16.202638  //
      { 0x108781d8,  {0x67,  0x02,  0x62,  0x00,  0x6c, }},//  6.750000,  //16.529325  //
      { 0x10d818df,  {0x67,  0x02,  0x62,  0x00,  0x70, }},//  7.000000,  //16.844130  //
      { 0x1125dbda,  {0x67,  0x02,  0x62,  0x00,  0x74, }},//  7.250000,  //17.147886  //
      { 0x1170fbe4,  {0x67,  0x02,  0x62,  0x00,  0x78, }},//  7.500000,  //17.441344  //
      { 0x11b9a541,  {0x67,  0x02,  0x62,  0x00,  0x7c, }},//  7.750000,  //17.725178  //
      { 0x12000000,  {0x67,  0x02,  0x62,  0x00,  0x80, }},//  8.000000,  //18.000000  //
      { 0x128657c9,  {0x67,  0x02,  0x62,  0x00,  0x88, }},//  8.500000,  //18.524777  //
      { 0x1305013a,  {0x67,  0x02,  0x62,  0x00,  0x90, }},//  9.000000,  //19.019550  //
      { 0x137cd110,  {0x67,  0x02,  0x62,  0x00,  0x98, }},//  9.500000,  //19.487565  //
      { 0x13ee7b47,  {0x67,  0x02,  0x62,  0x00,  0xa0, }},// 10.000000,  //19.931569  //
      { 0x145a997c,  {0x67,  0x02,  0x62,  0x00,  0xa8, }},// 10.500000,  //20.353905  //
      { 0x14c1afdd,  {0x67,  0x02,  0x62,  0x00,  0xb0, }},// 11.000000,  //20.756590  //
      { 0x152430f0,  {0x67,  0x02,  0x62,  0x00,  0xb8, }},// 11.500000,  //21.141372  //
      { 0x1582809d,  {0x67,  0x02,  0x62,  0x00,  0xc0, }},// 12.000000,  //21.509775  //
      { 0x15dcf68e,  {0x67,  0x02,  0x62,  0x00,  0xc8, }},// 12.500000,  //21.863137  //
      { 0x1633e01a,  {0x67,  0x02,  0x62,  0x00,  0xd0, }},// 13.000000,  //22.202638  //
      { 0x168781d8,  {0x67,  0x02,  0x62,  0x00,  0xd8, }},// 13.500000,  //22.529325  //
      { 0x16d818df,  {0x67,  0x02,  0x62,  0x00,  0xe0, }},// 14.000000,  //22.844130  //
      { 0x1725dbda,  {0x67,  0x02,  0x62,  0x00,  0xe8, }},// 14.500000,  //23.147886  //
      { 0x1770fbe4,  {0x67,  0x02,  0x62,  0x00,  0xf0, }},// 15.000000,  //23.441344  //
      { 0x17b9a541,  {0x67,  0x02,  0x62,  0x00,  0xf8, }},// 15.500000,  //23.725178  //
      { 0x18000000,  {0x67,  0x06,  0x62,  0x01,  0x00, }},// 16.000000,  //24.000000  //
      { 0x188657c9,  {0x67,  0x06,  0x62,  0x01,  0x10, }},// 17.000000,  //24.524777  //
      { 0x1905013a,  {0x67,  0x06,  0x62,  0x01,  0x20, }},// 18.000000,  //25.019550  //
      { 0x197cd110,  {0x67,  0x06,  0x62,  0x01,  0x30, }},// 19.000000,  //25.487565  //
      { 0x19ee7b47,  {0x67,  0x06,  0x62,  0x01,  0x40, }},// 20.000000,  //25.931569  //
      { 0x1a5a997c,  {0x67,  0x06,  0x62,  0x01,  0x50, }},// 21.000000,  //26.353905  //
      { 0x1ac1afdd,  {0x67,  0x06,  0x62,  0x01,  0x60, }},// 22.000000,  //26.756590  //
      { 0x1b2430f0,  {0x67,  0x06,  0x62,  0x01,  0x70, }},// 23.000000,  //27.141372  //
      { 0x1b82809d,  {0x67,  0x06,  0x62,  0x01,  0x80, }},// 24.000000,  //27.509775  //
      { 0x1bdcf68e,  {0x67,  0x06,  0x62,  0x01,  0x90, }},// 25.000000,  //27.863137  //
      { 0x1c33e01a,  {0x67,  0x06,  0x62,  0x01,  0xa0, }},// 26.000000,  //28.202638  //
      { 0x1c8781d8,  {0x67,  0x06,  0x62,  0x01,  0xb0, }},// 27.000000,  //28.529325  //
      { 0x1cd818df,  {0x67,  0x06,  0x62,  0x01,  0xc0, }},// 28.000000,  //28.844130  //
      { 0x1d25dbda,  {0x67,  0x06,  0x62,  0x01,  0xd0, }},// 29.000000,  //29.147886  //
      { 0x1d70fbe4,  {0x67,  0x06,  0x62,  0x01,  0xe0, }},// 30.000000,  //29.441344  //
      { 0x1db9a541,  {0x67,  0x06,  0x62,  0x01,  0xf0, }},// 31.000000,  //29.725178  //
      { 0x1e000000,  {0x67,  0x06,  0x62,  0x02,  0x00, }},// 32.000000,  //30.000000  //
      { 0x1e8657c9,  {0x67,  0x06,  0x62,  0x02,  0x20, }},// 34.000000,  //30.524777  //
      { 0x1f05013a,  {0x67,  0x06,  0x62,  0x02,  0x40, }},// 36.000000,  //31.019550  //
      { 0x1f7cd110,  {0x67,  0x06,  0x62,  0x02,  0x60, }},// 38.000000,  //31.487565  //
      { 0x1fee7b47,  {0x67,  0x06,  0x62,  0x02,  0x80, }},// 40.000000,  //31.931569  //
      { 0x205a997c,  {0x67,  0x06,  0x62,  0x02,  0xa0, }},// 42.000000,  //32.353905  //
      { 0x20c1afdd,  {0x67,  0x06,  0x62,  0x02,  0xc0, }},// 44.000000,  //32.756590  //
      { 0x212430f0,  {0x67,  0x06,  0x62,  0x02,  0xe0, }},// 46.000000,  //33.141372  //
      { 0x2182809d,  {0x67,  0x06,  0x62,  0x03,  0x00, }},// 48.000000,  //33.509775  //
      { 0x21dcf68e,  {0x67,  0x06,  0x62,  0x03,  0x20, }},// 50.000000,  //33.863137  //
      { 0x2233e01a,  {0x67,  0x06,  0x62,  0x03,  0x40, }},// 52.000000,  //34.202638  //
      { 0x228781d8,  {0x67,  0x06,  0x62,  0x03,  0x60, }},// 54.000000,  //34.529325  //
      { 0x22d818df,  {0x67,  0x06,  0x62,  0x03,  0x80, }},// 56.000000,  //34.844130  //
      { 0x2325dbda,  {0x67,  0x06,  0x62,  0x03,  0xa0, }},// 58.000000,  //35.147886  //
      { 0x2370fbe4,  {0x67,  0x06,  0x62,  0x03,  0xc0, }},// 60.000000,  //35.441344  //
      { 0x23b9a541,  {0x67,  0x06,  0x62,  0x03,  0xe0, }},// 62.000000,  //35.725178  //
      { 0x24000000,  {0x67,  0x06,  0x62,  0x04,  0x00, }},// 64.000000,  //36.000000  //
      { 0x248657c9,  {0x67,  0x06,  0x62,  0x04,  0x40, }},// 68.000000,  //36.524777  //
      { 0x2505013a,  {0x67,  0x06,  0x62,  0x04,  0x80, }},// 72.000000,  //37.019550  //
      { 0x257cd110,  {0x67,  0x06,  0x62,  0x04,  0xc0, }},// 76.000000,  //37.487565  //
      { 0x25ee7b47,  {0x67,  0x06,  0x62,  0x05,  0x00, }},// 80.000000,  //37.931569  //
      { 0x265a997c,  {0x67,  0x06,  0x62,  0x05,  0x40, }},// 84.000000,  //38.353905  //
      { 0x26c1afdd,  {0x67,  0x06,  0x62,  0x05,  0x80, }},// 88.000000,  //38.756590  //
      { 0x272430f0,  {0x67,  0x06,  0x62,  0x05,  0xc0, }},// 92.000000,  //39.141372  //
      { 0x2782809d,  {0x67,  0x06,  0x62,  0x06,  0x00, }},// 96.000000,  //39.509775  //
      { 0x27dcf68e,  {0x67,  0x06,  0x62,  0x06,  0x40, }},//100.000000,  //39.863137  //
      { 0x2833e01a,  {0x67,  0x06,  0x62,  0x06,  0x80, }},//104.000000,  //40.202638  //
      { 0x288781d8,  {0x67,  0x06,  0x62,  0x06,  0xc0, }},//108.000000,  //40.529325  //
      { 0x28d818df,  {0x67,  0x06,  0x62,  0x07,  0x00, }},//112.000000,  //40.844130  //
      { 0x2925dbda,  {0x67,  0x06,  0x62,  0x07,  0x40, }},//116.000000,  //41.147886  //
      { 0x2970fbe4,  {0x67,  0x06,  0x62,  0x07,  0x80, }},//120.000000,  //41.441344  //
      { 0x29b9a541,  {0x67,  0x06,  0x62,  0x07,  0xc0, }},//124.000000,  //41.725178  //
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
static int sc3035_set_agc_db(int handle, GK_Vi_SensorT* sensor_extern, s32 agc_db)
{
    int                     errorCode = 0;
    u32                     gain_index = sc3035_GAIN_ROWS;
    int                     i;
    GK_Vi_Sensor_SC3035T*   pdata = (GK_Vi_Sensor_SC3035T*)sensor_extern->private_data;
    //printk("[%s %d]%08x\n", __func__, __LINE__, agc_db);

    for(i = 0; i < sc3035_GAIN_ROWS; i++)
    {
        if(sc3035_GLOBAL_GAIN_TABLE[i].db_value > agc_db)
        {
            gain_index = i;
            break;
        }
    }
    if(gain_index>0)
        gain_index--;

    if (gain_index >= sc3035_GAIN_ROWS)
    {
        errorCode = -EINVAL;
        return errorCode;
    }

    pdata->sc3035_reg_3630 = sc3035_GLOBAL_GAIN_TABLE[gain_index].regs.sc3035_REG_3630;
    pdata->sc3035_reg_3627 = sc3035_GLOBAL_GAIN_TABLE[gain_index].regs.sc3035_REG_3627;
    pdata->sc3035_reg_3620 = sc3035_GLOBAL_GAIN_TABLE[gain_index].regs.sc3035_REG_3620;
    pdata->sc3035_reg_3e08 = sc3035_GLOBAL_GAIN_TABLE[gain_index].regs.sc3035_REG_3E08;
    pdata->sc3035_reg_3e09 = sc3035_GLOBAL_GAIN_TABLE[gain_index].regs.sc3035_REG_3E09;
    pdata->sc3035_adc_ch = 1;

    return errorCode;
}

static int sc3035_set_shutter_time(int handle, GK_Vi_SensorT* sensor_extern, u32 shutter_time, u32 extclk, u32 pclk)
{
    int     errorCode = 0;
    u64     exposure_time_q9;
    u32     frame_length, line_length;
    u32     shr_width_m, shr_width_l;
    //printk("[%s %d]%08x\n", __func__, __LINE__, shutter_time);
    GK_Vi_Sensor_SC3035T*   pdata = (GK_Vi_Sensor_SC3035T*)sensor_extern->private_data;

    exposure_time_q9 = shutter_time;

    errorCode |= sensor_read_reg_ex(handle, sc3035_TIMING_VTS_H, &shr_width_m, 1);
    errorCode |= sensor_read_reg_ex(handle, sc3035_TIMING_VTS_L, &shr_width_l, 1);
    frame_length = (shr_width_m<<8) + shr_width_l;
    errorCode |= sensor_read_reg_ex(handle, sc3035_TIMING_HTS_H, &shr_width_m, 1);
    errorCode |= sensor_read_reg_ex(handle, sc3035_TIMING_HTS_L, &shr_width_l, 1);
    line_length = ((shr_width_m)<<8) + shr_width_l;

    /* t(frame)/t(per line) = t(exposure, in lines)*/
    exposure_time_q9 = exposure_time_q9 * (u64)pclk;// sc3035 spec: Hmax unit : INCK
    DO_DIV_ROUND(exposure_time_q9, line_length);
    DO_DIV_ROUND(exposure_time_q9, 512000000);

    //vts & frame_rate protect.
    /*********protect(head)********/
    #if 0
    if(fps_set_flag == 1)
    {
        if((fps_frame_len_bak - 4) < exposure_time_q9)
        {
            exposure_time_q9 = fps_frame_len_bak - 4;
        }
        fps_set_flag = 0;
    }
    else
    {
        if((frame_length - 4) < exposure_time_q9)
        {
            exposure_time_q9 = frame_length - 4;
        }
    }
    #endif
    /*********protect(end)********/

    if((frame_length - 4) < exposure_time_q9)
    {
        exposure_time_q9 = frame_length - 4;
    }

    shr_width_m = (exposure_time_q9 >> 4) & 0xff;
    shr_width_l = exposure_time_q9 & 0xf;


    pdata->sc3035_reg_exp_time_h = shr_width_m;
    pdata->sc3035_reg_exp_time_l = ((shr_width_l << 4) & 0xf0);

    pdata->sc3035_shutter_ch = 1;
    return errorCode;
}

static int sc3035_set_fps(int handle, GK_Vi_SensorT* sensor_extern, u32 fps, u32 extclk, u32 pclk)
{
    int errorCode = 0;
    u32 hb_h, hb_l;
    u32 vb_h, vb_l;
    u32 fps_v = 512000000/fps;
    u32 line_length = 800;  //default 25fps.
    u32 frame_length = 900;  //default 25fps.
    GK_Vi_Sensor_SC3035T*   pdata = (GK_Vi_Sensor_SC3035T*)sensor_extern->private_data;

    //printk("[%s %d]%08x\n", __func__, __LINE__, fps);
    //printk("debug print: fps=%d  pclk=%d \n", fps_v, pclk);

    errorCode |= sensor_read_reg_ex(handle, sc3035_TIMING_HTS_H, &hb_h, 1);
    errorCode |= sensor_read_reg_ex(handle, sc3035_TIMING_HTS_L, &hb_l, 1);

    line_length = (hb_h<<8) + hb_l;

    frame_length = pclk/fps_v/(2*line_length);

    vb_h = (frame_length>>8) & 0xff;
    vb_l = frame_length & 0xff;

    errorCode |= sensor_write_reg_ex(handle, sc3035_TIMING_VTS_H, vb_h, 1);
    errorCode |= sensor_write_reg_ex(handle, sc3035_TIMING_VTS_L, vb_l, 1);

    //restrict registers setting.
    /***********restrict control (head)*************/
    //{0x336a, 0x336b} = {0x320e, 0x320f};
    errorCode |= sensor_write_reg_ex(handle, sc3035_336A, vb_h, 1);
    errorCode |= sensor_write_reg_ex(handle, sc3035_336B, vb_l, 1);

    //{0x3368, 0x3369} = {0x320e, 0x320f} - 0x300;
    if(frame_length <= 0x300)
    {
        errorCode |= sensor_write_reg_ex(handle, sc3035_3368, 0x00, 1);
        errorCode |= sensor_write_reg_ex(handle, sc3035_3369, 0x00, 1);
    }
    else
    {
        vb_h = ((frame_length-0x300)>>8) & 0xff;
        vb_l = (frame_length-0x300) & 0xff;

        errorCode |= sensor_write_reg_ex(handle, sc3035_3368, vb_h, 1);
        errorCode |= sensor_write_reg_ex(handle, sc3035_3369, vb_l, 1);
    }
    /***********restrict control (end)*************/
    #if 0  //vts & frame_rate protect.
    fps_set_flag = 1;
    fps_frame_len_bak = frame_length;
    #endif
    //printk("frame_length: 0x%x  \n", frame_length);

    return errorCode;
}

static int sc3035_set_update_reg(int handle, GK_Vi_SensorT* sensor_extern)
{
    GK_Vi_Sensor_SC3035T*   pdata = (GK_Vi_Sensor_SC3035T*)sensor_extern->private_data;
    //printk("[%s %d]\n", __func__, __LINE__);

    if(pdata->sc3035_shutter_ch == 1)
    {
        if(pdata->sc3035_last_reg_exp_time_h!=pdata->sc3035_reg_exp_time_h)
        {
            sensor_write_reg_ex(handle, sc3035_AEC_EXP_TIME_H, pdata->sc3035_reg_exp_time_h, 0);
        }
        if(pdata->sc3035_last_reg_exp_time_l!=pdata->sc3035_reg_exp_time_l)
        {
            sensor_write_reg_ex(handle, sc3035_AEC_EXP_TIME_L, pdata->sc3035_reg_exp_time_l, 0);
        }

        pdata->sc3035_last_reg_exp_time_h = pdata->sc3035_reg_exp_time_h;
        pdata->sc3035_last_reg_exp_time_l = pdata->sc3035_reg_exp_time_l;
        pdata->sc3035_shutter_ch = 0;
    }
    if(pdata->sc3035_adc_ch == 1)
    {
        //restrict registers.
        if (pdata->sc3035_last_reg_3630 != pdata->sc3035_reg_3630)
        {
            sensor_write_reg_ex(handle, sc3035_3630, pdata->sc3035_reg_3630, 0);
        }
        if (pdata->sc3035_last_reg_3627 != pdata->sc3035_reg_3627)
        {
            sensor_write_reg_ex(handle, sc3035_3627, pdata->sc3035_reg_3627, 0);
        }
        if (pdata->sc3035_last_reg_3620 != pdata->sc3035_reg_3620)
        {
            sensor_write_reg_ex(handle, sc3035_3620, pdata->sc3035_reg_3620, 0);
        }
        //agc registers.
        if (pdata->sc3035_last_reg_3e08 != pdata->sc3035_reg_3e08)
        {
            sensor_write_reg_ex(handle, sc3035_3e08, pdata->sc3035_reg_3e08, 0);
        }
        if (pdata->sc3035_last_reg_3e09 != pdata->sc3035_reg_3e09)
        {
            sensor_write_reg_ex(handle, sc3035_3e09, pdata->sc3035_reg_3e09, 0);
        }
        pdata->sc3035_adc_ch = 0;
        pdata->sc3035_last_reg_3630=pdata->sc3035_reg_3630;
        pdata->sc3035_last_reg_3627=pdata->sc3035_reg_3627;
        pdata->sc3035_last_reg_3620=pdata->sc3035_reg_3620;
        pdata->sc3035_last_reg_3e08=pdata->sc3035_reg_3e08;
        pdata->sc3035_last_reg_3e09=pdata->sc3035_reg_3e09;
    }
    return 0;
}



int sc3035_probe(GK_Vi_SensorT* sensor_extern, int id)
{
    GK_Vi_Sensor_SC3035T *sc3035_data = NULL;

    sensor_extern->private_data = kzalloc(sizeof(GK_Vi_Sensor_SC3035T), GFP_KERNEL);
    if (sensor_extern->private_data == NULL)
    {
        printk("kzalloc memory(0x%04x) failed\n", sizeof(GK_Vi_Sensor_SC3035T));
        return -1;
    }
    sc3035_data = sensor_extern->private_data;

    sc3035_data->sub_id = id;
    sc3035_data->sc3035_adc_ch = 0;
    sc3035_data->sc3035_reg_3630 = 0xa9;
    sc3035_data->sc3035_reg_3627 = 0x02;
    sc3035_data->sc3035_reg_3620 = 0xd2;
    sc3035_data->sc3035_reg_3e08 = 0x00;    //default: 1x gain total.
    sc3035_data->sc3035_reg_3e09 = 0x10;

    sc3035_data->sc3035_last_reg_3630 = 0xffffffff;
    sc3035_data->sc3035_last_reg_3627 = 0xffffffff;
    sc3035_data->sc3035_last_reg_3620 = 0xffffffff;
    sc3035_data->sc3035_last_reg_3e08 = 0xffffffff;
    sc3035_data->sc3035_last_reg_3e09 = 0xffffffff;

    sc3035_data->sc3035_shutter_ch     = 0;
    sc3035_data->sc3035_reg_exp_time_h = 0;
    sc3035_data->sc3035_reg_exp_time_l = 0;

    sc3035_data->sc3035_last_reg_exp_time_h = 0xffffffff;
    sc3035_data->sc3035_last_reg_exp_time_l = 0xffffffff;

    return 0;
}

int sc3035_remove(GK_Vi_SensorT* sensor_extern)
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
static int __init sc3035_init(void)
{
    sc3035_dev.sensor_id                = GK_VI_SENSOR_SC3035;
    sc3035_dev.length                   = sizeof(sc3035_dev);

    sc3035_dev.private_data             = NULL;
    sc3035_dev.probe                    = sc3035_probe;
    sc3035_dev.remove                   = sc3035_remove;
    sc3035_dev.sensor_get_vblank_time   = NULL;
    sc3035_dev.sensor_set_shutter_time  = sc3035_set_shutter_time;
    sc3035_dev.sensor_set_fps           = sc3035_set_fps;
    sc3035_dev.sensor_set_agc_db        = sc3035_set_agc_db;
    sc3035_dev.sensor_set_mirror_mode   = NULL;
    sc3035_dev.sensor_set_update_reg    = sc3035_set_update_reg;
    sensor_registry_api(&sc3035_dev);
    return 0;
}

static void __exit sc3035_exit(void)
{
    sensor_unregistry_api(&sc3035_dev);
}

module_init(sc3035_init);
module_exit(sc3035_exit);
MODULE_LICENSE("Proprietary");
MODULE_AUTHOR("GOKE MICROELECTRONICS Inc.");
MODULE_DESCRIPTION("sc3035 1/3-Inch 1.3-Megapixel CMOS Digital Image Sensor");

