/*!
*****************************************************************************
** \file        software/linux/kernel/drv/sensors/sc1235/sc1235.c
**
** \version     $Id: sc1235_ex.c 1280 2019-02-22 09:38:49Z dengbiao $
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
#include "sc1235_pri.h"

typedef struct
{
    int sub_id;
    //GK_Vi_SensorT api;
    int sc1235_adc_ch;
    int sc1235_reg_3631;
    int sc1235_reg_3301;
    int sc1235_reg_3633;
    int sc1235_reg_3e07;
    int sc1235_reg_3e08;
    int sc1235_reg_3e09;

    int sc1235_last_reg_3631;
    int sc1235_last_reg_3301;
    int sc1235_last_reg_3633;
    int sc1235_last_reg_3e07;
    int sc1235_last_reg_3e08;
    int sc1235_last_reg_3e09;

    int sc1235_shutter_ch;
    int sc1235_reg_exp_time_h;
    int sc1235_reg_exp_time_l;

    int sc1235_last_reg_exp_time_h;
    int sc1235_last_reg_exp_time_l;
}GK_Vi_Sensor_SC1235T;

//*****************************************************************************
//*****************************************************************************
//** Local structures
//*****************************************************************************
//*****************************************************************************
#define SC1235_GAIN_ROWS            128
typedef struct sc1235_gain_table
{
    u32 db_value;
    struct
    {
        u16 SC1235_REG_3631;
        u16 SC1235_REG_3301;
        u16 SC1235_REG_3633;
        u16 SC1235_REG_3E07;
        u16 SC1235_REG_3E08;
        u16 SC1235_REG_3E09;
    }regs;
}sc1235_gain_table_s;


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
static int sc1235_set_agc_db(int handle, GK_Vi_SensorT* sensor_extern, s32 agc_db);
static int sc1235_set_shutter_time(int handle, GK_Vi_SensorT* sensor_extern, u32 shutter_time, u32 extclk, u32 pclk);
static int sc1235_set_update_reg(int handle, GK_Vi_SensorT* sensor_extern);
static GK_Vi_SensorT sc1235_dev;
#if 0
static GK_Vi_Sensor_SC1235T sc1235 =
{
    .api =
    {
        .sensor_id                = GK_VI_SENSOR_SC1235,
        .length                   = sizeof(GK_Vi_Sensor_SC1235T),
        .sensor_get_vblank_time   = NULL,
        .sensor_set_shutter_time  = sc1235_set_shutter_time,
        .sensor_set_fps           = NULL,
        .sensor_set_agc_db        = sc1235_set_agc_db,
        .sensor_set_mirror_mode   = NULL,
        .sensor_set_update_reg    = sc1235_set_update_reg,
    },
    .sc1235_adc_ch    = 0,
    .sc1235_reg_3631  = 0,
    .sc1235_reg_3301  = 0,
    .sc1235_reg_3633  = 0,
    .sc1235_reg_3e07  = 0,
    .sc1235_reg_3e08  = 0,
    .sc1235_reg_3e09  = 0,

    .sc1235_last_reg_3631 = 0xffffffff,
    .sc1235_last_reg_3301 = 0xffffffff,
    .sc1235_last_reg_3633 = 0xffffffff,
    .sc1235_last_reg_3e07 = 0xffffffff,
    .sc1235_last_reg_3e08 = 0xffffffff,
    .sc1235_last_reg_3e09 = 0xffffffff,

    .sc1235_shutter_ch    = 0,
    .sc1235_reg_exp_time_h = 0,
    .sc1235_reg_exp_time_l = 0,

    .sc1235_last_reg_exp_time_h = 0xffffffff,
    .sc1235_last_reg_exp_time_l = 0xffffffff,
};
#endif
const sc1235_gain_table_s SC1235_GLOBAL_GAIN_TABLE[SC1235_GAIN_ROWS] =
{
    //        0x3631              0x3301              0x3633       0x3e07[0]+0x3e08[7:5]     0x3e08[4:2]               0x3e09
    //      restriction         restriction         restriction         dgain[3:0]        again coarse gain[2:0] again fine gain
    // (    gain< 2: 0x84) (    gain< 2: 0x05) (    gain< 2: 0x2F)      (0000: 1X)            (000: 1X)            ([6:4] corse_gain 000b=1X ,001b=2X, 011b=4X, 111b=8X)
    // ( 2<=gain< 4: 0x88) ( 2<=gain< 4: 0x1F) ( 2<=gain< 4: 0x23)      (0001: 2X)            (001: 2X)            ([3:0] fine_gain)
    // ( 4<=gain   : 0x88) ( 4<=gain   : 0xFF) ( 4<=gain   : 0x43)      (0011: 4X)            (011: 4X)
    //                                                                  (0111: 8X)            (111: 8X)
    {0x00000000, {0x84, 0x05, 0x2F, 0x00, 0x03, 0x10,}}, //0
    {0x0086CDDD, {0x84, 0x05, 0x2F, 0x00, 0x03, 0x11,}}, //0.526578774
    {0x0105E6A2, {0x84, 0x05, 0x2F, 0x00, 0x03, 0x12,}}, //1.023050449
    {0x017E1FC6, {0x84, 0x05, 0x2F, 0x00, 0x03, 0x13,}}, //1.492672366
    {0x01F02DE4, {0x84, 0x05, 0x2F, 0x00, 0x03, 0x14,}}, //1.93820026
    {0x025CAB21, {0x84, 0x05, 0x2F, 0x00, 0x03, 0x15,}}, //2.361986242
    {0x02C41C1C, {0x84, 0x05, 0x2F, 0x00, 0x03, 0x16,}}, //2.766053963
    {0x0326F3C3, {0x84, 0x05, 0x2F, 0x00, 0x03, 0x17,}}, //3.152157067
    {0x03859655, {0x84, 0x05, 0x2F, 0x00, 0x03, 0x18,}}, //3.521825181
    {0x03E05BC8, {0x84, 0x05, 0x2F, 0x00, 0x03, 0x19,}}, //3.87640052
    {0x043791B9, {0x84, 0x05, 0x2F, 0x00, 0x03, 0x1A,}}, //4.217067306
    {0x048B7CF8, {0x84, 0x05, 0x2F, 0x00, 0x03, 0x1B,}}, //4.54487563
    {0x04DC5AD4, {0x84, 0x05, 0x2F, 0x00, 0x03, 0x1C,}}, //4.860760974
    {0x052A6228, {0x84, 0x05, 0x2F, 0x00, 0x03, 0x1D,}}, //5.165560305
    {0x0575C43A, {0x84, 0x05, 0x2F, 0x00, 0x03, 0x1E,}}, //5.460025441
    {0x05BEAD74, {0x84, 0x05, 0x2F, 0x00, 0x03, 0x1F,}}, //5.744834224
     //totalgain=2X
    {0x06054609, {0x88, 0x1F, 0x23, 0x00, 0x07, 0x10,}}, //6.020599913
    {0x068C13E7, {0x88, 0x1F, 0x23, 0x00, 0x07, 0x11,}}, //6.547178688
    {0x070B2CAB, {0x88, 0x1F, 0x23, 0x00, 0x07, 0x12,}}, //7.043650362
    {0x078365CF, {0x88, 0x1F, 0x23, 0x00, 0x07, 0x13,}}, //7.513272279
    {0x07F573ED, {0x88, 0x1F, 0x23, 0x00, 0x07, 0x14,}}, //7.958800173
    {0x0861F12A, {0x88, 0x1F, 0x23, 0x00, 0x07, 0x15,}}, //8.382586155
    {0x08C96226, {0x88, 0x1F, 0x23, 0x00, 0x07, 0x16,}}, //8.786653877
    {0x092C39CD, {0x88, 0x1F, 0x23, 0x00, 0x07, 0x17,}}, //9.172756981
    {0x098ADC5E, {0x88, 0x1F, 0x23, 0x00, 0x07, 0x18,}}, //9.542425094
    {0x09E5A1D2, {0x88, 0x1F, 0x23, 0x00, 0x07, 0x19,}}, //9.897000434
    {0x0A3CD7C2, {0x88, 0x1F, 0x23, 0x00, 0x07, 0x1A,}}, //10.23766722
    {0x0A90C301, {0x88, 0x1F, 0x23, 0x00, 0x07, 0x1B,}}, //10.56547554
    {0x0AE1A0DD, {0x88, 0x1F, 0x23, 0x00, 0x07, 0x1C,}}, //10.88136089
    {0x0B2FA832, {0x88, 0x1F, 0x23, 0x00, 0x07, 0x1D,}}, //11.18616022
    {0x0B7B0A43, {0x88, 0x1F, 0x23, 0x00, 0x07, 0x1E,}}, //11.48062535
    {0x0BC3F37D, {0x88, 0x1F, 0x23, 0x00, 0x07, 0x1F,}}, //11.76543414
     //totalgain=4X
    {0x0C0A8C12, {0x88, 0xFF, 0x43, 0x00, 0x0F, 0x10,}}, //12.04119983
    {0x0C9159F0, {0x88, 0xFF, 0x43, 0x00, 0x0F, 0x11,}}, //12.5677786
    {0x0D1072B4, {0x88, 0xFF, 0x43, 0x00, 0x0F, 0x12,}}, //13.06425028
    {0x0D88ABD9, {0x88, 0xFF, 0x43, 0x00, 0x0F, 0x13,}}, //13.53387219
    {0x0DFAB9F6, {0x88, 0xFF, 0x43, 0x00, 0x0F, 0x14,}}, //13.97940009
    {0x0E673733, {0x88, 0xFF, 0x43, 0x00, 0x0F, 0x15,}}, //14.40318607
    {0x0ECEA82F, {0x88, 0xFF, 0x43, 0x00, 0x0F, 0x16,}}, //14.80725379
    {0x0F317FD6, {0x88, 0xFF, 0x43, 0x00, 0x0F, 0x17,}}, //15.19335689
    {0x0F902268, {0x88, 0xFF, 0x43, 0x00, 0x0F, 0x18,}}, //15.56302501
    {0x0FEAE7DB, {0x88, 0xFF, 0x43, 0x00, 0x0F, 0x19,}}, //15.91760035
    {0x10421DCB, {0x88, 0xFF, 0x43, 0x00, 0x0F, 0x1A,}}, //16.25826713
    {0x1096090A, {0x88, 0xFF, 0x43, 0x00, 0x0F, 0x1B,}}, //16.58607546
    {0x10E6E6E7, {0x88, 0xFF, 0x43, 0x00, 0x0F, 0x1C,}}, //16.9019608
    {0x1134EE3B, {0x88, 0xFF, 0x43, 0x00, 0x0F, 0x1D,}}, //17.20676013
    {0x1180504C, {0x88, 0xFF, 0x43, 0x00, 0x0F, 0x1E,}}, //17.50122527
    {0x11C93987, {0x88, 0xFF, 0x43, 0x00, 0x0F, 0x1F,}}, //17.78603405
    {0x120FD21B, {0x88, 0xFF, 0x43, 0x00, 0x1F, 0x10,}}, //18.06179974
    {0x12969FF9, {0x88, 0xFF, 0x43, 0x00, 0x1F, 0x11,}}, //18.58837851
    {0x1315B8BD, {0x88, 0xFF, 0x43, 0x00, 0x1F, 0x12,}}, //19.08485019
    {0x138DF1E2, {0x88, 0xFF, 0x43, 0x00, 0x1F, 0x13,}}, //19.55447211
    {0x14000000, {0x88, 0xFF, 0x43, 0x00, 0x1F, 0x14,}}, //20
    {0x146C7D3C, {0x88, 0xFF, 0x43, 0x00, 0x1F, 0x15,}}, //20.42378598
    {0x14D3EE38, {0x88, 0xFF, 0x43, 0x00, 0x1F, 0x16,}}, //20.8278537
    {0x1536C5DF, {0x88, 0xFF, 0x43, 0x00, 0x1F, 0x17,}}, //21.21395681
    {0x15956871, {0x88, 0xFF, 0x43, 0x00, 0x1F, 0x18,}}, //21.58362492
    {0x15F02DE4, {0x88, 0xFF, 0x43, 0x00, 0x1F, 0x19,}}, //21.93820026
    {0x164763D4, {0x88, 0xFF, 0x43, 0x00, 0x1F, 0x1A,}}, //22.27886705
    {0x169B4F13, {0x88, 0xFF, 0x43, 0x00, 0x1F, 0x1B,}}, //22.60667537
    {0x16EC2CF0, {0x88, 0xFF, 0x43, 0x00, 0x1F, 0x1C,}}, //22.92256071
    {0x173A3444, {0x88, 0xFF, 0x43, 0x00, 0x1F, 0x1D,}}, //23.22736004
    {0x17859655, {0x88, 0xFF, 0x43, 0x00, 0x1F, 0x1E,}}, //23.52182518
    {0x17CE7F90, {0x88, 0xFF, 0x43, 0x00, 0x1F, 0x1F,}}, //23.80663396
    {0x18151824, {0x88, 0xFF, 0x43, 0x00, 0x3F, 0x10,}}, //24.08239965
    {0x189BE602, {0x88, 0xFF, 0x43, 0x00, 0x3F, 0x11,}}, //24.60897843
    {0x191AFEC7, {0x88, 0xFF, 0x43, 0x00, 0x3F, 0x12,}}, //25.1054501
    {0x199337EB, {0x88, 0xFF, 0x43, 0x00, 0x3F, 0x13,}}, //25.57507202
    {0x1A054609, {0x88, 0xFF, 0x43, 0x00, 0x3F, 0x14,}}, //26.02059991
    {0x1A71C346, {0x88, 0xFF, 0x43, 0x00, 0x3F, 0x15,}}, //26.44438589
    {0x1AD93441, {0x88, 0xFF, 0x43, 0x00, 0x3F, 0x16,}}, //26.84845362
    {0x1B3C0BE8, {0x88, 0xFF, 0x43, 0x00, 0x3F, 0x17,}}, //27.23455672
    {0x1B9AAE7A, {0x88, 0xFF, 0x43, 0x00, 0x3F, 0x18,}}, //27.60422483
    {0x1BF573ED, {0x88, 0xFF, 0x43, 0x00, 0x3F, 0x19,}}, //27.95880017
    {0x1C4CA9DD, {0x88, 0xFF, 0x43, 0x00, 0x3F, 0x1A,}}, //28.29946696
    {0x1CA0951C, {0x88, 0xFF, 0x43, 0x00, 0x3F, 0x1B,}}, //28.62727528
    {0x1CF172F9, {0x88, 0xFF, 0x43, 0x00, 0x3F, 0x1C,}}, //28.94316063
    {0x1D3F7A4D, {0x88, 0xFF, 0x43, 0x00, 0x3F, 0x1D,}}, //29.24795996
    {0x1D8ADC5E, {0x88, 0xFF, 0x43, 0x00, 0x3F, 0x1E,}}, //29.54242509
    {0x1DD3C599, {0x88, 0xFF, 0x43, 0x00, 0x3F, 0x1F,}}, //29.82723388
    {0x1E1A5E2D, {0x88, 0xFF, 0x43, 0x00, 0x7F, 0x10,}}, //30.10299957
    {0x1EA12C0B, {0x88, 0xFF, 0x43, 0x00, 0x7F, 0x11,}}, //30.62957834
    {0x1F2044D0, {0x88, 0xFF, 0x43, 0x00, 0x7F, 0x12,}}, //31.12605002
    {0x1F987DF4, {0x88, 0xFF, 0x43, 0x00, 0x7F, 0x13,}}, //31.59567193
    {0x200A8C12, {0x88, 0xFF, 0x43, 0x00, 0x7F, 0x14,}}, //32.04119983
    {0x2077094F, {0x88, 0xFF, 0x43, 0x00, 0x7F, 0x15,}}, //32.46498581
    {0x20DE7A4A, {0x88, 0xFF, 0x43, 0x00, 0x7F, 0x16,}}, //32.86905353
    {0x214151F1, {0x88, 0xFF, 0x43, 0x00, 0x7F, 0x17,}}, //33.25515663
    {0x219FF483, {0x88, 0xFF, 0x43, 0x00, 0x7F, 0x18,}}, //33.62482475
    {0x21FAB9F6, {0x88, 0xFF, 0x43, 0x00, 0x7F, 0x19,}}, //33.97940009
    {0x2251EFE7, {0x88, 0xFF, 0x43, 0x00, 0x7F, 0x1A,}}, //34.32006687
    {0x22A5DB26, {0x88, 0xFF, 0x43, 0x00, 0x7F, 0x1B,}}, //34.6478752
    {0x22F6B902, {0x88, 0xFF, 0x43, 0x00, 0x7F, 0x1C,}}, //34.96376054
    {0x2344C056, {0x88, 0xFF, 0x43, 0x00, 0x7F, 0x1D,}}, //35.26855987
    {0x23902268, {0x88, 0xFF, 0x43, 0x00, 0x7F, 0x1E,}}, //35.56302501
    {0x23D90BA2, {0x88, 0xFF, 0x43, 0x00, 0x7F, 0x1F,}}, //35.84783379
    {0x241FA437, {0x88, 0xFF, 0x43, 0x00, 0xFF, 0x10,}}, //36.12359948
    {0x24A67215, {0x88, 0xFF, 0x43, 0x00, 0xFF, 0x11,}}, //36.65017825
    {0x25258AD9, {0x88, 0xFF, 0x43, 0x00, 0xFF, 0x12,}}, //37.14664993
    {0x259DC3FD, {0x88, 0xFF, 0x43, 0x00, 0xFF, 0x13,}}, //37.61627185
    {0x260FD21B, {0x88, 0xFF, 0x43, 0x00, 0xFF, 0x14,}}, //38.06179974
    {0x267C4F58, {0x88, 0xFF, 0x43, 0x00, 0xFF, 0x15,}}, //38.48558572
    {0x26E3C053, {0x88, 0xFF, 0x43, 0x00, 0xFF, 0x16,}}, //38.88965344
    {0x274697FB, {0x88, 0xFF, 0x43, 0x00, 0xFF, 0x17,}}, //39.27575655
    {0x27A53A8C, {0x88, 0xFF, 0x43, 0x00, 0xFF, 0x18,}}, //39.64542466
    {0x28000000, {0x88, 0xFF, 0x43, 0x00, 0xFF, 0x19,}}, //40
    {0x285735F0, {0x88, 0xFF, 0x43, 0x00, 0xFF, 0x1A,}}, //40.34066679
    {0x28AB212F, {0x88, 0xFF, 0x43, 0x00, 0xFF, 0x1B,}}, //40.66847511
    {0x28FBFF0B, {0x88, 0xFF, 0x43, 0x00, 0xFF, 0x1C,}}, //40.98436045
    {0x294A0660, {0x88, 0xFF, 0x43, 0x00, 0xFF, 0x1D,}}, //41.28915978
    {0x29956871, {0x88, 0xFF, 0x43, 0x00, 0xFF, 0x1E,}}, //41.58362492
    {0x29DE51AB, {0x88, 0xFF, 0x43, 0x00, 0xFF, 0x1F,}}, //41.8684337
    {0x2A24EA40, {0x88, 0xFF, 0x43, 0x01, 0xFF, 0x10,}}, //42.14419939
    {0x2AABB81E, {0x88, 0xFF, 0x43, 0x01, 0xFF, 0x11,}}, //42.67077817
    {0x2B2AD0E2, {0x88, 0xFF, 0x43, 0x01, 0xFF, 0x12,}}, //43.16724984
    {0x2BA30A07, {0x88, 0xFF, 0x43, 0x01, 0xFF, 0x13,}}, //43.63687176
    {0x2C151824, {0x88, 0xFF, 0x43, 0x01, 0xFF, 0x14,}}, //44.08239965
    {0x2C819561, {0x88, 0xFF, 0x43, 0x01, 0xFF, 0x15,}}, //44.50618563
    {0x2CE9065D, {0x88, 0xFF, 0x43, 0x01, 0xFF, 0x16,}}, //44.91025336
    {0x2D4BDE04, {0x88, 0xFF, 0x43, 0x01, 0xFF, 0x17,}}, //45.29635646
    {0x2DAA8096, {0x88, 0xFF, 0x43, 0x01, 0xFF, 0x18,}}, //45.66602457
    {0x2E054609, {0x88, 0xFF, 0x43, 0x01, 0xFF, 0x19,}}, //46.02059991
    {0x2E5C7BF9, {0x88, 0xFF, 0x43, 0x01, 0xFF, 0x1A,}}, //46.3612667
    {0x2EB06738, {0x88, 0xFF, 0x43, 0x01, 0xFF, 0x1B,}}, //46.68907502
    {0x2F014515, {0x88, 0xFF, 0x43, 0x01, 0xFF, 0x1C,}}, //47.00496037
    {0x2F4F4C69, {0x88, 0xFF, 0x43, 0x01, 0xFF, 0x1D,}}, //47.3097597
    {0x2F9AAE7A, {0x88, 0xFF, 0x43, 0x01, 0xFF, 0x1E,}}, //47.60422483
    {0x2FE397B5, {0x88, 0xFF, 0x43, 0x01, 0xFF, 0x1F,}}, //47.88903362
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
static int sc1235_set_agc_db(int handle, GK_Vi_SensorT* sensor_extern, s32 agc_db)
{
    int                     errorCode = 0;
    u32                     gain_index = SC1235_GAIN_ROWS;
    int                     i;
    GK_Vi_Sensor_SC1235T*   pdata = (GK_Vi_Sensor_SC1235T*)sensor_extern->private_data;
    //printk("[%s %d]%08x\n", __func__, __LINE__, agc_db);

    for(i = 0; i < SC1235_GAIN_ROWS; i++)
    {
        if(SC1235_GLOBAL_GAIN_TABLE[i].db_value > agc_db)
        {
            gain_index = i;
            break;
        }
    }
    if(gain_index>0)
        gain_index--;

    if (gain_index >= SC1235_GAIN_ROWS)
    {
        errorCode = -EINVAL;
        return errorCode;
    }

    pdata->sc1235_reg_3631 = SC1235_GLOBAL_GAIN_TABLE[gain_index].regs.SC1235_REG_3631;
    pdata->sc1235_reg_3301 = SC1235_GLOBAL_GAIN_TABLE[gain_index].regs.SC1235_REG_3301;
    pdata->sc1235_reg_3633 = SC1235_GLOBAL_GAIN_TABLE[gain_index].regs.SC1235_REG_3633;
    pdata->sc1235_reg_3e07 = SC1235_GLOBAL_GAIN_TABLE[gain_index].regs.SC1235_REG_3E07;
    pdata->sc1235_reg_3e08 = SC1235_GLOBAL_GAIN_TABLE[gain_index].regs.SC1235_REG_3E08;
    pdata->sc1235_reg_3e09 = SC1235_GLOBAL_GAIN_TABLE[gain_index].regs.SC1235_REG_3E09;
    pdata->sc1235_adc_ch = 1;

    return errorCode;
}

static int sc1235_set_shutter_time(int handle, GK_Vi_SensorT* sensor_extern, u32 shutter_time, u32 extclk, u32 pclk)
{
    int     errorCode = 0;
    u64     exposure_time_q9;
    u32     frame_length, line_length;
    u32     shr_width_m, shr_width_l;
    GK_Vi_Sensor_SC1235T*   pdata = (GK_Vi_Sensor_SC1235T*)sensor_extern->private_data;
    //printk("[%s %d]%08x\n", __func__, __LINE__, shutter_time);

    exposure_time_q9 = shutter_time;

    errorCode |= sensor_read_reg_ex(handle, SC1235_TIMING_VTS_H, &shr_width_m, 1);
    errorCode |= sensor_read_reg_ex(handle, SC1235_TIMING_VTS_L, &shr_width_l, 1);
    frame_length = (shr_width_m<<8) + shr_width_l;
    errorCode |= sensor_read_reg_ex(handle, SC1235_TIMING_HTS_H, &shr_width_m, 1);
    errorCode |= sensor_read_reg_ex(handle, SC1235_TIMING_HTS_L, &shr_width_l, 1);
    line_length = ((shr_width_m)<<8) + shr_width_l;

    /* t(frame)/t(per line) = t(exposure, in lines)*/
    exposure_time_q9 = exposure_time_q9 * (u64)pclk;// sc1235 spec: Hmax unit : INCK
    DO_DIV_ROUND(exposure_time_q9, line_length);
    DO_DIV_ROUND(exposure_time_q9, 512000000);

    if((frame_length - 4) < exposure_time_q9)
    {
        exposure_time_q9 = frame_length - 4;
    }

    shr_width_m = (exposure_time_q9 >> 4) & 0xff;
    shr_width_l = exposure_time_q9 & 0xf;


    pdata->sc1235_reg_exp_time_h = shr_width_m;
    pdata->sc1235_reg_exp_time_l = ((shr_width_l << 4) & 0xf0);

    pdata->sc1235_shutter_ch = 1;
    return errorCode;
}

static int sc1235_set_update_reg(int handle, GK_Vi_SensorT* sensor_extern)
{
    u32 chnged =0;
    GK_Vi_Sensor_SC1235T*   pdata = (GK_Vi_Sensor_SC1235T*)sensor_extern->private_data;
    //printk("[%s %d]\n", __func__, __LINE__);

    if(pdata->sc1235_shutter_ch || pdata->sc1235_adc_ch)
    {
        chnged = 1;
        //sensor_write_reg_ex(handle, SC1235_3903, 0x84, 0);
        //sensor_write_reg_ex(handle, SC1235_3903, 0x04, 0);
        //sensor_write_reg_ex(handle, SC1235_3812, 0x00, 0);
    }
    if(pdata->sc1235_shutter_ch == 1)
    {
        if(pdata->sc1235_last_reg_exp_time_h!=pdata->sc1235_reg_exp_time_h)
        {
            sensor_write_reg_ex(handle, SC1235_AEC_EXP_TIME_H, pdata->sc1235_reg_exp_time_h, 0);
        }
        if(pdata->sc1235_last_reg_exp_time_l!=pdata->sc1235_reg_exp_time_l)
        {
            sensor_write_reg_ex(handle, SC1235_AEC_EXP_TIME_L, pdata->sc1235_reg_exp_time_l, 0);
        }

        pdata->sc1235_last_reg_exp_time_h = pdata->sc1235_reg_exp_time_h;
        pdata->sc1235_last_reg_exp_time_l = pdata->sc1235_reg_exp_time_l;
        pdata->sc1235_shutter_ch = 0;
    }
    if(pdata->sc1235_adc_ch == 1)
    {
        if (pdata->sc1235_last_reg_3631 != pdata->sc1235_reg_3631)
        {
            sensor_write_reg_ex(handle, SC1235_3631, pdata->sc1235_reg_3631, 0);
        }
        if (pdata->sc1235_last_reg_3301 != pdata->sc1235_reg_3301)
        {
            sensor_write_reg_ex(handle, SC1235_3301, pdata->sc1235_reg_3301, 0);
        }
        if (pdata->sc1235_last_reg_3633 != pdata->sc1235_reg_3633)
        {
            sensor_write_reg_ex(handle, SC1235_3633, pdata->sc1235_reg_3633, 0);
        }
        if (pdata->sc1235_last_reg_3e07 != pdata->sc1235_reg_3e07)
        {
            sensor_write_reg_ex(handle, SC1235_3e07, pdata->sc1235_reg_3e07, 0);
        }
        if (pdata->sc1235_last_reg_3e08 != pdata->sc1235_reg_3e08)
        {
            sensor_write_reg_ex(handle, SC1235_3e08, pdata->sc1235_reg_3e08, 0);
        }
        if (pdata->sc1235_last_reg_3e09 != pdata->sc1235_reg_3e09)
        {
            sensor_write_reg_ex(handle, SC1235_3e09, pdata->sc1235_reg_3e09, 0);
        }
        pdata->sc1235_adc_ch = 0;
        pdata->sc1235_last_reg_3631=pdata->sc1235_reg_3631;
        pdata->sc1235_last_reg_3301=pdata->sc1235_reg_3301;
        pdata->sc1235_last_reg_3633=pdata->sc1235_reg_3633;
        pdata->sc1235_last_reg_3e07=pdata->sc1235_reg_3e07;
        pdata->sc1235_last_reg_3e08=pdata->sc1235_reg_3e08;
        pdata->sc1235_last_reg_3e09=pdata->sc1235_reg_3e09;
    }
    if(chnged)
    {
        //sensor_write_reg_ex(handle, SC1235_3812, 0x30, 0);
    }
    return 0;
}


int sc1235_probe(GK_Vi_SensorT* sensor_extern, int id)
{
    GK_Vi_Sensor_SC1235T *sc1235_data = NULL;

    sensor_extern->private_data = kzalloc(sizeof(GK_Vi_Sensor_SC1235T), GFP_KERNEL);
    if (sensor_extern->private_data == NULL)
    {
        printk("kzalloc memory(0x%04x) failed\n", sizeof(GK_Vi_Sensor_SC1235T));
        return -1;
    }
    sc1235_data = sensor_extern->private_data;

    sc1235_data->sub_id = id;
    sc1235_data->sc1235_adc_ch    = 0;
    sc1235_data->sc1235_reg_3631  = 0;
    sc1235_data->sc1235_reg_3301  = 0;
    sc1235_data->sc1235_reg_3633  = 0;
    sc1235_data->sc1235_reg_3e07  = 0;
    sc1235_data->sc1235_reg_3e08  = 0;
    sc1235_data->sc1235_reg_3e09  = 0;

    sc1235_data->sc1235_last_reg_3631 = 0xffffffff;
    sc1235_data->sc1235_last_reg_3301 = 0xffffffff;
    sc1235_data->sc1235_last_reg_3633 = 0xffffffff;
    sc1235_data->sc1235_last_reg_3e07 = 0xffffffff;
    sc1235_data->sc1235_last_reg_3e08 = 0xffffffff;
    sc1235_data->sc1235_last_reg_3e09 = 0xffffffff;

    sc1235_data->sc1235_shutter_ch    = 0;
    sc1235_data->sc1235_reg_exp_time_h = 0;
    sc1235_data->sc1235_reg_exp_time_l = 0;

    sc1235_data->sc1235_last_reg_exp_time_h = 0xffffffff;
    sc1235_data->sc1235_last_reg_exp_time_l = 0xffffffff;

    return 0;
}

int sc1235_remove(GK_Vi_SensorT* sensor_extern)
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
static int __init sc1235_init(void)
{
    sc1235_dev.sensor_id                = GK_VI_SENSOR_SC1235;
    sc1235_dev.length                   = sizeof(sc1235_dev);

    sc1235_dev.private_data             = NULL;
    sc1235_dev.probe                    = sc1235_probe;
    sc1235_dev.remove                   = sc1235_remove;
    sc1235_dev.sensor_get_vblank_time   = NULL;
    sc1235_dev.sensor_set_shutter_time  = sc1235_set_shutter_time;
    sc1235_dev.sensor_set_fps           = NULL;
    sc1235_dev.sensor_set_agc_db        = sc1235_set_agc_db;
    sc1235_dev.sensor_set_mirror_mode   = NULL;
    sc1235_dev.sensor_set_update_reg    = sc1235_set_update_reg;
    sensor_registry_api(&sc1235_dev);
    return 0;
}

static void __exit sc1235_exit(void)
{
    sensor_unregistry_api(&sc1235_dev);
}

module_init(sc1235_init);
module_exit(sc1235_exit);
MODULE_LICENSE("Proprietary");
MODULE_AUTHOR("GOKE MICROELECTRONICS Inc.");
MODULE_DESCRIPTION("SC1235 1/3-Inch 1.3-Megapixel CMOS Digital Image Sensor");

