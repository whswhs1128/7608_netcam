/*!
*****************************************************************************
** \file        linux/kernel/drv/include/gk_sensor.h
**
** \version     $Id: gk_sensor.h 1517 2019-07-04 12:06:28Z tangyuping $
**
** \brief
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2016 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#ifndef _GK_SENSOR_H_
#define _GK_SENSOR_H_



//*****************************************************************************
//*****************************************************************************
//** Defines and Macros
//*****************************************************************************
//*****************************************************************************
//32-bit
#ifndef DIV_ROUND
#define DIV_ROUND(divident, divider)    (((divident)+((divider)>>1))/(divider))
#endif

//64-bit
#ifndef DO_DIV_ROUND
#define DO_DIV_ROUND(divident, divider) \
    do                                  \
    {                                   \
        (divident) += ((divider)>>1);   \
        do_div((divident), (divider));  \
    } while (0)
#endif



//*****************************************************************************
//*****************************************************************************
//** Enumerated types
//*****************************************************************************
//*****************************************************************************
typedef enum
{
    GK_VI_SENSOR_UNKNOWN    = 0x00000000,
    // Sony Sensor
    GK_VI_SENSOR_IMX122     = 0x00001000,   // 1080P IMX122, IMX222, IMX322, IMX323
    GK_VI_SENSOR_IMX323,    // 1080P IMX122, IMX222, IMX322, IMX323
    GK_VI_SENSOR_IMX225,    // 960P
    GK_VI_SENSOR_IMX238,    // 720P
    GK_VI_SENSOR_IMX291,    // 1080P

    // Aptina Sensor
    GK_VI_SENSOR_AR0130     = 0x00002000,   // 960P
    GK_VI_SENSOR_AR0230,    // 1080P
    GK_VI_SENSOR_AR0237,    // 1080P
    GK_VI_SENSOR_AR0330,    // 1080P 3M
    GK_VI_SENSOR_AR0331,    // 1080P 3M

    // OmniVision Sensor
    GK_VI_SENSOR_OV2710     = 0x00003000,   // 1080P OV2715
    GK_VI_SENSOR_OV9710,    // 720P OV9712
    GK_VI_SENSOR_OV9732,    // 720P
    GK_VI_SENSOR_OV9750,    // 960P
    GK_VI_SENSOR_OV4689,
    GK_VI_SENSOR_OV2735,    //1080P
    GK_VI_SENSOR_OS04B10,   // 1440p    

    // SmartSens Sensor
    GK_VI_SENSOR_SC1015     = 0x00004000,   // 720P
    GK_VI_SENSOR_SC1020,    // 720P
    GK_VI_SENSOR_SC1035,    // 960P
    GK_VI_SENSOR_SC1135,    // 960P
    GK_VI_SENSOR_SC1042,    // 720P YUV
    GK_VI_SENSOR_SC1045,    // 720P RGB
    GK_VI_SENSOR_SC1145,    // 720P RGB
    GK_VI_SENSOR_SC2035,    // 1080P
    GK_VI_SENSOR_SC2135,    // 1080P
    GK_VI_SENSOR_SC2045,    // 1080P
    GK_VI_SENSOR_SC1235,    // 960P
    GK_VI_SENSOR_SC1245,    // 720P
    GK_VI_SENSOR_SC3035,    // 2304x1536
    GK_VI_SENSOR_SC2235,    // 1080P
    GK_VI_SENSOR_SC2232,    // 1080P
    GK_VI_SENSOR_SC4236,
    GK_VI_SENSOR_SC3235,
    GK_VI_SENSOR_SC5235,

    // Silicon Sensor
    GK_VI_SENSOR_JXH42      = 0x00005000,   // 720P JXH22
    GK_VI_SENSOR_JXH61,
    GK_VI_SENSOR_JXH62,     // 720P
    GK_VI_SENSOR_JXH65,     // 960P
    GK_VI_SENSOR_JXF22,     // 1080P
    GK_VI_SENSOR_JXF23,     // 1080P
    GK_VI_SENSOR_JXK02,     // 1080P
    GK_VI_SENSOR_JXK03,     // 1080P

    // BriGates Sensor
    GK_VI_SENSOR_BG0701     = 0x00006000,   // 720P BG0703

    // BYD Sensor
    GK_VI_SENSOR_BYD0703    = 0x00007000,   // 720P

    // GC Sensor
    GK_VI_SENSOR_GC1004     = 0x00008000,   // 720P GC1014
    GK_VI_SENSOR_GC1024,    // 720P GC2004
    GK_VI_SENSOR_GC0308,    // 640* 480 GC0308
    GK_VI_SENSOR_GC0328,    // 640* 480 GC0328
    GK_VI_SENSOR_GC1034,    // 720P GC2004
    GK_VI_SENSOR_GC2023,    // 1080P
    GK_VI_SENSOR_GC2033,    // 1080P
    GK_VI_SENSOR_GC2053,    // 1080P
    GK_VI_SENSOR_GC4623,    // 2560*1440
    GK_VI_SENSOR_GC4633,    // 2560*1440

    // PP Sensor
    GK_VI_SENSOR_PP1210     = 0x00009000,   // 1080P

    // mis sensor
    GK_VI_SENSOR_MIS1002    = 0x0000A000,
    GK_VI_SENSOR_MIS2003    = 0x0000A001,   //1080P

    GK_VI_SENSOR_TW9900     = 0x0000B000,
    GK_VI_SENSOR_TW9912,

    //bt1120 input
    GK_VI_MACH_PV6418       = 0x0000D000,
    GK_VI_MACH_ADV7611      = 0x0000D001,  // adv7611 hdmi receiver.

    //CISTA
    GK_VI_SENSOR_C1490      = 0x0000E000,   // 960P
    GK_VI_SENSOR_C2390,     // 1080P BSI
    GK_VI_SENSOR_C2392,     // 1080P FSI

    //novatek
    GK_VI_SENSOR_NT99231    = 0x0000E000,   // 960P                   // 1080P

    //0909 hhl add for tpg_test
    GK_VI_SENSOR_PSEUDO     = 0x000010000,  // pseudo sensor for chip-tpg-mode.

    //SuperPix
    GK_VI_SENSOR_SP1409     = 0x000011000,  // 720P
    GK_VI_SENSOR_SP2519,    // 1600 x 1200 UXGA @15fps ,720P@30fps
    GK_VI_SENSOR_SP2307,
    GK_VI_SENSOR_SP2508,    // 1600 x 1200 UXGA @15fps ,720P@30fps

    //ST
    GK_VI_SENSOR_MN168       = 0x00012000,   //VD6869, 1520P    
}GK_VI_SensorIdEnumT; /* please sync with GADI_ISP_SensorModelEnumT*/



//*****************************************************************************
//*****************************************************************************
//** Data Structures
//*****************************************************************************
//*****************************************************************************
typedef struct gk_vi_sensor_s
{
    GK_VI_SensorIdEnumT sensor_id;
    int                 length;
    void               *private_data;
    int (*probe)(struct gk_vi_sensor_s* sensor_extern, int id);
    int (*remove)(struct gk_vi_sensor_s* sensor_extern);
    int (*sensor_get_vblank_time)(int handle, struct gk_vi_sensor_s* sensor_extern, u32* ptime, u32 extclk, u32 pclk);
    int (*sensor_set_shutter_time)(int handle, struct gk_vi_sensor_s* sensor_extern, u32 shutter_time, u32 extclk, u32 pclk);
    int (*sensor_set_fps)(int handle, struct gk_vi_sensor_s* sensor_extern, u32 fps, u32 extclk, u32 pclk);
    int (*sensor_set_agc_db)(int handle, struct gk_vi_sensor_s* sensor_extern, s32 agc_db);
    int (*sensor_set_mirror_mode)(int handle, struct gk_vi_sensor_s* sensor_extern, u32 mirr_mode);
    int (*sensor_set_update_reg)(int handle, struct gk_vi_sensor_s* sensor_extern);
}GK_Vi_SensorT;


typedef struct
{
    u8  interface;      //interface type: dvp bt656 bt1120 mipi
    u8  type;           //data type: rgb555 rgb565 rgb666 rgb888 yuv420 yuv422 bt656 bt1120
    u8  bits;           //width of Y/U/V (R/G/B)
}GK_VI_MipiInterfaceT;


typedef struct gk_vi_mipi_drv_s
{
    int (*mipi_set_interface)(GK_VI_MipiInterfaceT *pinterface);
    int (*mipi_set_format)(u16 width, u16 height);
    int (*mipi_set_fps)(u16 fps, u16 hts, u16 vts);
    int (*mipi_set_init)(void);
    int (*mipi_get_setting_state)(void);
    int (*mipi_clr_setting_state)(void);
    int (*mipi_setting)(void);
}GK_VI_MipiDrvT;

typedef struct gk_vi_agc_info_s
{
    s32     db_max;
    s32     db_min;
    s32     db_step;
}GK_VI_AgcInfoT;

typedef struct gk_vi_video_mode_s
{
    u16     width;
    u16     height;
    u16     frame_rate;
    u8      progressive;
}GK_VI_VideoModeT;

//*****************************************************************************
//*****************************************************************************
//** Global Data
//*****************************************************************************
//*****************************************************************************



//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************



#ifdef __cplusplus
extern "C" {
#endif
extern int sensor_get_agc_info(int handle, GK_VI_AgcInfoT* p_agc_info);
extern int sensor_get_video_mode(int handle, GK_VI_VideoModeT* p_video_mode);
extern int sensor_write_reg_group_ex(int handle, u32* pdata, u32 len, u32 event_flag);
extern int sensor_write_reg_ex(int handle, u32 subaddr, u32 data, u32 event_flag);
extern int sensor_read_reg_ex(int handle, u32 subaddr, u32* pdata, u32 event_flag);
extern void sensor_registry_api(GK_Vi_SensorT* sensor_api);
extern void sensor_unregistry_api(GK_Vi_SensorT* sensor_api);
extern void sensor_mipi_registry_api(GK_VI_MipiDrvT* mipi_api);
extern void sensor_mipi_unregistry_api(GK_VI_MipiDrvT* mipi_api);

#ifdef __cplusplus
}
#endif



#endif /* _GK_SENSOR_H_ */

