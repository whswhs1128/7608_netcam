/*!
g*****************************************************************************
** \file        nt99231_reg_tbl.c
**
** \version     $Id: nt99231_reg_tbl.c 11627 2017-05-23 10:58:12Z zhoushuai $
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
GADI_VI_SensorDrvInfoT    nt99231_sensor =
{
    .magic_start        = GADI_SENSOR_HW_INFO_MAGIC_START,
    .version            = GADI_SENSOR_HW_INFO_VERSION,
    .length             = sizeof(GADI_VI_SensorDrvInfoT),
    .file_date          = 0x20161014,
    .HwInfo         =
    {
        .name               = "nt99231",
        .ctl_type           = GADI_SENSOR_CTL_I2C,
        .addr_len           = sizeof(GADI_U16),
        .data_len           = sizeof(GADI_U8),
        .reset_active_level = GADI_GPIO_LOW,
        .hw_addr            = {(0x66>>1), 0, 0, 0},
        .id_reg =
        {
            {0x3000, 0x23}, // NT99231_CHIP_ID_H
            {0x3001, 0x01}, // NT99231_CHIP_ID_L
        },
        .reset_reg =
        {
            {0x3021,0x01, 0x01},   // NT99231_SYSTEM_CONTROL00
            {GADI_VI_SENSOR_TABLE_FLAG_MSLEEP, 10},    // msleep(10);
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        .standby_reg =
        {
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
    },
    .debug_print        = 1,
    .input_format       = GADI_VI_INPUT_FORMAT_RGB_RAW,
    .field_format       = 1,
    .adapter_id         = 0,
    .sensor_id          = GADI_ISP_SENSOR_NT99231,
    .sensor_double_step = 16,
    .typeofsensor       = 1,
    .sync_time_delay    = 0,
    .source_type.dummy  = GADI_VI_CMOS_CHANNEL_TYPE_AUTO,
    .dev_type           = GADI_VI_DEV_TYPE_CMOS,
    .video_system       = GADI_VIDEO_SYSTEM_AUTO,
    .vs_polarity        = GADI_VI_RISING_EDGE,
    .hs_polarity        = GADI_VI_RISING_EDGE,
    .data_edge          = GADI_VI_RISING_EDGE,
    .emb_sync_switch    = GADI_VI_EMB_SYNC_OFF,
    .emb_sync_loc       = GADI_VI_EMB_SYNC_LOWER_PEL,
    .emb_sync_mode      = GADI_VI_EMB_SYNC_ITU_656,
    .max_width          = 1936,
    .max_height         = 1084,
    .def_sh_time        = GADI_VIDEO_FPS(50),
    .fmt_num            = 2,
    .auto_fmt           = GADI_VIDEO_MODE(1920, 1080,  25, 1),
    .init_reg           =
    {
        {0x303E, 0x00},
        {0x3100, 0x03},
        {0x3101, 0x00},
        {0x3102, 0x09},
        {0x3103, 0x09},
        {0x3104, 0x00},
        {0x3105, 0x03},
        {0x3106, 0x07},
        {0x3107, 0x30},
        {0x3108, 0x50},
        {0x3109, 0x02},
        {0x310A, 0x77},
        {0x310B, 0xC0},
        {0x310C, 0x00},
        {0x310D, 0x43},
        {0x310E, 0x01},
        {0x3110, 0x88},
        {0x3111, 0xCE},
        {0x3112, 0x88},
        {0x3113, 0x66},
        {0x3114, 0x33},
        {0x3115, 0x88},
        {0x3116, 0x86},
        {0x3118, 0xAF},
        {0x3119, 0xAF},
        {0x311A, 0xAF},
        {0x303F, 0x32},
        {0x3055, 0x00},
        {0x3056, 0x24},
        {0x30A4, 0x1F},
        {0x30A0, 0x01},
        {0x3710, 0x02},
        {0x3210, 0x1D},
        {0x3211, 0x1F},
        {0x3212, 0x24},
        {0x3213, 0x1E},
        {0x3214, 0x1B},
        {0x3215, 0x1E},
        {0x3216, 0x1E},
        {0x3217, 0x19},
        {0x3218, 0x1B},
        {0x3219, 0x1E},
        {0x321A, 0x1E},
        {0x321B, 0x19},
        {0x321C, 0x1B},
        {0x321D, 0x1E},
        {0x321E, 0x1E},
        {0x321F, 0x18},
        {0x3230, 0x1D},
        {0x3231, 0x00},
        {0x3232, 0x00},
        {0x3233, 0x00},
        {0x3234, 0x00},
        {0x3235, 0x00},
        {0x3236, 0x00},
        {0x3237, 0x00},
        {0x3243, 0xC2},
        {0x3250, 0x34},
        {0x3251, 0x25},
        {0x3252, 0x36},
        {0x3253, 0x22},
        {0x3254, 0x40},
        {0x3255, 0x24},
        {0x3256, 0x2A},
        {0x3257, 0x1E},
        {0x3258, 0x4B},
        {0x3259, 0x3E},
        {0x325A, 0x27},
        {0x325B, 0x1C},
        {0x325C, 0xFF},
        {0x325D, 0x05},
        {0x3270, 0x00},
        {0x3271, 0x0C},
        {0x3272, 0x18},
        {0x3273, 0x32},
        {0x3274, 0x44},
        {0x3275, 0x54},
        {0x3276, 0x70},
        {0x3277, 0x88},
        {0x3278, 0x9D},
        {0x3279, 0xB0},
        {0x327A, 0xCF},
        {0x327B, 0xE2},
        {0x327C, 0xEF},
        {0x327D, 0xF7},
        {0x327E, 0xFF},
        {0x3700, 0x04},
        {0x3701, 0x0F},
        {0x3702, 0x1B},
        {0x3703, 0x35},
        {0x3704, 0x46},
        {0x3705, 0x56},
        {0x3706, 0x72},
        {0x3707, 0x89},
        {0x3708, 0x9E},
        {0x3709, 0xB1},
        {0x370A, 0xCF},
        {0x370B, 0xE2},
        {0x370C, 0xEF},
        {0x370D, 0xF7},
        {0x370E, 0xFF},
        {0x3326, 0x08},
        {0x3327, 0x05},
        {0x3365, 0x08},
        {0x3366, 0x0A},
        {0x3367, 0x10},
        {0x3368, 0x28},
        {0x3369, 0x24},
        {0x336B, 0x20},
        {0x3375, 0x10},
        {0x3376, 0x10},
        {0x3378, 0x10},
        {0x336D, 0x20},
        {0x336E, 0x16},
        {0x3370, 0x0E},
        {0x3371, 0x20},
        {0x3372, 0x28},
        {0x3374, 0x30},
        {0x3379, 0x08},
        {0x337A, 0x0C},
        {0x337C, 0x14},
        {0x33A9, 0x02},
        {0x33AA, 0x03},
        {0x33AC, 0x04},
        {0x33AD, 0x04},
        {0x33AE, 0x05},
        {0x33B0, 0x08},
        {0x33C0, 0x01},
        {0x33A6, 0x03},
        {0x33C9, 0x0A},
        {0x33C7, 0x23},
        {0x33C8, 0x33},
        {0x33B1, 0x00},
        {0x33B4, 0x66},
        {0x33B5, 0xA4},
        {0x33BA, 0x02},
        {0x33BB, 0x04},
        {0x308B, 0x2F},
        {0x308C, 0x28},
        {0x308D, 0x24},
        {0x308F, 0x10},
        {0x3360, 0x20},
        {0x3361, 0x28},
        {0x3362, 0x30},
        {0x3363, 0x01},
        {0x3364, 0x09},
        {0x33C0, 0x00},
        {0x33A0, 0x40},
        {0x33A1, 0x88},
        {0x33A2, 0x00},
        {0x33A3, 0x28},
        {0x33A4, 0x01},
        {0x3262, 0x00},
        {0x334B, 0x00},
        {0x3810, 0x00},
        {0x333F, 0x0F},
        {0x3514, 0x00},
        {0x301E, 0x08},
        {0x3109, 0x00},
        {0x307D, 0x02},
        {0x3531, 0x08},
        {0x336D, 0x20},
        {0x336E, 0x16},
        {0x3370, 0x0E},
        {0x3371, 0x00},
        {0x3372, 0x00},
        {0x3374, 0x00},
        {0x3379, 0x00},
        {0x337A, 0x00},
        {0x337C, 0x00},
        {0x33A9, 0x00},
        {0x33AA, 0x00},
        {0x33AC, 0x00},
        {0x33AD, 0x00},
        {0x33AE, 0x00},
        {0x33B0, 0x00},
        {0x3112, 0xA9},
        {0x3113, 0x77},
        {0x3114, 0x44},
        {0x3115, 0x99},
        {0x3116, 0x96},
        {0x32BB, 0x77},  //AE Start           
        {0x32BF, 0x60},                       
        {0x32C0, 0x60},                       
        {0x32C1, 0x60},                       
        {0x32C2, 0x60},                      
        {0x32C3, 0x00},                       
        {0x32C4, 0x2F},                    
        {0x32C5, 0x2F},                   
        {0x32C6, 0x2F},                    
        {0x32D3, 0x01},                       
        {0x32D4, 0x18},                       
        {0x32D5, 0x81},                       
        {0x32D6, 0x00},                       
        {0x32D7, 0xEA},                       
        {0x32D8, 0x7D},  //AE End             
        {0x32F0, 0x80},  //Output Format      
        {0x3200, 0x0e},  //Mode Control       
        {0x3201, 0x08},  //Mode Control  
        {GADI_VI_SENSOR_TABLE_FLAG_END,},

    },
    .pll_table =
    {
        [0] =
        {
            // for 1080P@30
            .pixclk = 81000000, // pixclk = ((extclk / 8) / NT99231_PLL_PREDIVEDER) * NT99231_PLL_CTRL02
            .extclk = 27000000, //        = (27000000/8)/1*24=81000000
            .regs =
            {
                {0x302A, 0x80}, //PLL Start
                {0x302B, 0x04},
                {0x302C, 0x04},
                {0x302D, 0x00},
                {0x302E, 0x00},
                {0x302F, 0x04}, //PLL End
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,}
            },
        },
        
        [1] =
        {
            // for 1080P@25
            .pixclk = 74250000, // pixclk = ((extclk / 8) / NT99231_PLL_PREDIVEDER) * NT99231_PLL_CTRL02
            .extclk = 27000000, //        = (27000000/8)/1*24=74250000
            .regs =
            {
                {0x302A, 0x80}, //PLL Start
                {0x302B, 0x05},
                {0x302C, 0x0A},
                {0x302D, 0x01},
                {0x302E, 0x00},
                {0x302F, 0x05}, //PLL End
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,}
            },
        },
    },
    .video_fmt_table =
    {
        // 1920x1080P: HTS must >= 2404, if HTS is to low , must fix the horizontal start before set mirror mode
        // 1280x720P : HTS must >= 1768, if HTS is to low , must fix the horizontal start before set mirror mode
        [0] =  // 1920x1080P@30fps  2400 * 1125 * 30 = 81000000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1920, 1080,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x3022, 0x24}, //Timing Start
                {0x3023, 0x24},
                {0x3002, 0x00},
                {0x3003, 0x08},
                {0x3004, 0x00},
                {0x3005, 0x02},
                {0x3006, 0x07},
                {0x3007, 0x87},
                {0x3008, 0x04},
                {0x3009, 0x39},
                {0x300A, 0x09},
                {0x300B, 0x60},
                {0x300C, 0x04},
                {0x300D, 0x65},
                {0x300E, 0x07},
                {0x300F, 0x80},
                {0x3010, 0x04},
                {0x3011, 0x38}, //Timing End
                {0x3012, 0x04},
                {0x3013, 0x60},
                {0x301D, 0x0A},
                {0x320A, 0x00},
                {0x3021, 0x02},
                {0x3060, 0x01},
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 1920,
                .height     = 1080,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_DVP, 
                .bits       = GADI_VIDEO_BITS_10,
                .ratio      = GADI_VIDEO_RATIO_16_9,
                .srm        = 0,
                .pll_index  = 0,
                .max_fps    = GADI_VIDEO_FPS(30),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(30),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_1080P_30,
            },
            .video_info_table =
            {
                .def_start_x    = 0,
                .def_start_y    = 0,
                .def_width      = 1920,
                .def_height     = 1080,
                .sync_start     = 0,
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_BG,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_BG,
                },
            },
            .update_after_vsync_start   = 30,
            .update_after_vsync_end     = 33,
        },
        [1] = // 1920x1080P@25fps   2640 * 1125 * 25 = 74250000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1920, 1080,  25, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x3022, 0x24}, //Timing Start
                {0x3023, 0x24},
                {0x3002, 0x00},
                {0x3003, 0x08},
                {0x3004, 0x00},
                {0x3005, 0x02},
                {0x3006, 0x07},
                {0x3007, 0x87},
                {0x3008, 0x04},
                {0x3009, 0x39},
                {0x300A, 0x0A},
                {0x300B, 0x50},
                {0x300C, 0x04},
                {0x300D, 0x65},
                {0x300E, 0x07},
                {0x300F, 0x80},
                {0x3010, 0x04},
                {0x3011, 0x38}, //Timing End
                {0x3012, 0x04},
                {0x3013, 0x60},
                {0x301D, 0x0A},
                {0x320A, 0x00},
                {0x3021, 0x02},
                {0x3060, 0x01},
                {GADI_VI_SENSOR_TABLE_FLAG_END,},

            },
            .vi_info =
            {
                .width      = 1920,
                .height     = 1080,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_DVP, 
                .bits       = GADI_VIDEO_BITS_10,
                .ratio      = GADI_VIDEO_RATIO_16_9,
                .srm        = 0,
                .pll_index  = 1,
                .max_fps    = GADI_VIDEO_FPS(25),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(25),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_1080P_25,
            },
            .video_info_table =
            {
                .def_start_x    = 0,
                .def_start_y    = 0,
                .def_width      = 1920,
                .def_height     = 1080,
                .sync_start     = 0,
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_RG,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_RG,
                    GADI_VI_BAYER_PATTERN_RG,
                    GADI_VI_BAYER_PATTERN_RG,
                    GADI_VI_BAYER_PATTERN_RG,
                },
            },
            .update_after_vsync_start   = 37,
            .update_after_vsync_end     = 40,
        },
    },
    .mirror_table =
    {
#define NT99231_HORIZ_MIRROR     (0x01 << 1)
#define NT99231_VERT_FLIP        (0x01 << 0)
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY_VERTICALLY] =
        {
            {0x3022, NT99231_HORIZ_MIRROR | NT99231_VERT_FLIP, NT99231_HORIZ_MIRROR | NT99231_VERT_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY] =
        {
            {0x3022, NT99231_HORIZ_MIRROR, NT99231_HORIZ_MIRROR | NT99231_VERT_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_VERTICALLY] =
        {
            {0x3022, NT99231_VERT_FLIP, NT99231_HORIZ_MIRROR | NT99231_VERT_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_NONE] =
        {
            {0x3022, 0x24, NT99231_HORIZ_MIRROR | NT99231_VERT_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
    },
    .version_reg =
    {
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .reg_ctl_by_ver_num = 0,
    .control_by_version_reg = {0},
    .vmax_clk_ref   = GADI_SENSOR_CAL_CLK_PIXCLK,
    .shs_clk_ref    = GADI_SENSOR_CAL_CLK_PIXCLK,
    .cal_shs_mode   = GADI_SENSOR_CAL_SHS_VMAX_MODE,
    .shs_fix        = 3,
    .shs_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_SET,
    .agc_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_SET,
    .vmax_reg_update_mode       = GADI_SENSOR_UPDATE_REG_BY_SET,
    .mirror_reg_update_mode     = GADI_SENSOR_UPDATE_REG_BY_SET,
    .shs_additional_use         = 0,
    .shs_additional_percent     = 0,
    // hmax = (NT99231_TIMING_CONTROL_HTS_HIGHBYTE & 0xFF) << 8 +
    //        (NT99231_TIMING_CONTROL_HTS_LOWBYTE & 0xFF) << 0
    .hmax_reg =
    {
        {0x300A, 0x00, 0xFF, 0x00, 0x08, 0}, // NT99231_TIMING_CONTROL_HTS_HIGHBYTE
        {0x300B, 0x00, 0xFF, 0x00, 0x00, 0}, // NT99231_TIMING_CONTROL_HTS_LOWBYTE
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    // vmax = (NT99231_TIMING_CONTROL_VTS_HIGHBYTE & 0xFF) << 8 +
    //        (NT99231_TIMING_CONTROL_VTS_LOWBYTE & 0xFF) << 0
    .vmax_reg =
    {
        {0x300C, 0x00, 0xFF, 0x00, 0x08, 0}, // NT99231_TIMING_CONTROL_VTS_HIGHBYTE
        {0x300D, 0x00, 0xFF, 0x00, 0x00, 0}, // NT99231_TIMING_CONTROL_VTS_LOWBYTE
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .vmax_wadd_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    // shs = (EXP_H & 0xFF) << 8 +
    //       (EXP_L & 0xFF) << 0
    .shs_reg =
    {
        {0x3012, 0x00, 0xFF, 0x00, 0x08, 0}, // NT99231_AEC_PK_EXPO_H
        {0x3013, 0x00, 0xFF, 0x00, 0x00, 0}, // NT99231_AEC_PK_EXPO_L
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .shs_additional_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    .cal_gain_mode              = GADI_SENSOR_CAL_GAIN_MODE2,
    .max_agc_index = 80,
    .max_again_index = 0,
    .cur_again_index = 0,
    .agc_info =
    {
        .db_max     = 0x24000000,    // 36dB
        .db_min     = 0x00000000,    // 0dB
        .db_step    = 0x00100000,    // 0.0625dB
    },
    .gain_reg =
    {
        {0x301C,},
        {0x301D,},  
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .gain_table =
    {
        /*addr 0x301C;   bit[6:0];  again = (PGA[6]+1)*(PGA[5]+1)*(PGA[4]+1)*(1+PGA[3:0]/16)*/
        /*addr 0x301D;   bit[1:0];  dgain: bit[1:0]=00, no digital gain;  bit[1:0]=01 or 10, 2x;  bit[1:0]=11, 4x;*/
        /*Total gain = dgain*again  */
        /*PGA   DVP2     agc_db    */ //   db
        {0x00000000, {0x00, 0x00,}}, //0                       
        {0x0086CDDD, {0x01, 0x00,}}, //0.526578774       
        {0x0105E6A2, {0x02, 0x00,}}, //1.023050449       
        {0x017E1FC6, {0x03, 0x00,}}, //1.492672366       
        {0x01F02DE4, {0x04, 0x00,}}, //1.93820026        
        {0x025CAB21, {0x05, 0x00,}}, //2.361986242       
        {0x02C41C1C, {0x06, 0x00,}}, //2.766053963       
        {0x0326F3C3, {0x07, 0x00,}}, //3.152157067       
        {0x03859655, {0x08, 0x00,}}, //3.521825181       
        {0x03E05BC8, {0x09, 0x00,}}, //3.87640052        
        {0x043791B9, {0x0A, 0x00,}}, //4.217067306       
        {0x048B7CF8, {0x0B, 0x00,}}, //4.54487563        
        {0x04DC5AD4, {0x0C, 0x00,}}, //4.860760974       
        {0x052A6228, {0x0D, 0x00,}}, //5.165560305       
        {0x0575C43A, {0x0E, 0x00,}}, //5.460025441       
        {0x05BEAD74, {0x0F, 0x00,}}, //5.744834224       
        {0x06054609, {0x10, 0x00,}}, //6.020599913      
        {0x068C13E7, {0x11, 0x00,}}, //6.547178688      
        {0x070B2CAB, {0x12, 0x00,}}, //7.043650362      
        {0x078365CF, {0x13, 0x00,}}, //7.513272279      
        {0x07F573ED, {0x14, 0x00,}}, //7.958800173      
        {0x0861F12A, {0x15, 0x00,}}, //8.382586155      
        {0x08C96226, {0x16, 0x00,}}, //8.786653877      
        {0x092C39CD, {0x17, 0x00,}}, //9.172756981      
        {0x098ADC5E, {0x18, 0x00,}}, //9.542425094      
        {0x09E5A1D2, {0x19, 0x00,}}, //9.897000434      
        {0x0A3CD7C2, {0x1A, 0x00,}}, //10.23766722      
        {0x0A90C301, {0x1B, 0x00,}}, //10.56547554      
        {0x0AE1A0DD, {0x1C, 0x00,}}, //10.88136089      
        {0x0B2FA832, {0x1D, 0x00,}}, //11.18616022      
        {0x0B7B0A43, {0x1E, 0x00,}}, //11.48062535      
        {0x0BC3F37D, {0x1F, 0x00,}}, //11.76543414      
        {0x0C0A8C12, {0x30, 0x00,}}, //12.04119983      
        {0x0C9159F0, {0x31, 0x00,}}, //12.5677786       
        {0x0D1072B4, {0x32, 0x00,}}, //13.06425028      
        {0x0D88ABD9, {0x33, 0x00,}}, //13.53387219      
        {0x0DFAB9F6, {0x34, 0x00,}}, //13.97940009      
        {0x0E673733, {0x35, 0x00,}}, //14.40318607      
        {0x0ECEA82F, {0x36, 0x00,}}, //14.80725379      
        {0x0F317FD6, {0x37, 0x00,}}, //15.19335689      
        {0x0F902268, {0x38, 0x00,}}, //15.56302501      
        {0x0FEAE7DB, {0x39, 0x00,}}, //15.91760035      
        {0x10421DCB, {0x3A, 0x00,}}, //16.25826713     
        {0x1096090A, {0x3B, 0x00,}}, //16.58607546     
        {0x10E6E6E7, {0x3C, 0x00,}}, //16.9019608      
        {0x1134EE3B, {0x3D, 0x00,}}, //17.20676013     
        {0x1180504C, {0x3E, 0x00,}}, //17.50122527     
        {0x11C93987, {0x3F, 0x00,}}, //17.78603405     
        {0x120FD21B, {0x70, 0x00,}}, //18.06179974     
        {0x12969FF9, {0x71, 0x00,}}, //18.58837851     
        {0x1315B8BD, {0x72, 0x00,}}, //19.08485019     
        {0x138DF1E2, {0x73, 0x00,}}, //19.55447211     
        {0x14000000, {0x74, 0x00,}}, //20              
        {0x146C7D3C, {0x75, 0x00,}}, //20.42378598     
        {0x14D3EE38, {0x76, 0x00,}}, //20.8278537      
        {0x1536C5DF, {0x77, 0x00,}}, //21.21395681     
        {0x15956871, {0x78, 0x00,}}, //21.58362492     
        {0x15F02DE4, {0x79, 0x00,}}, //21.93820026     
        {0x164763D4, {0x7A, 0x00,}}, //22.27886705     
        {0x169B4F13, {0x7B, 0x00,}}, //22.60667537     
        {0x16EC2CF0, {0x7C, 0x00,}}, //22.92256071     
        {0x173A3444, {0x7D, 0x00,}}, //23.22736004     
        {0x17859655, {0x7E, 0x00,}}, //23.52182518     
        {0x17CE7F90, {0x7F, 0x00,}}, //23.80663396     
        {0x18151824, {0xF0, 0x00,}}, //24.08239965     
        {0x189BE602, {0xF1, 0x00,}}, //24.60897843     
        {0x191AFEC7, {0xF2, 0x00,}}, //25.1054501      
        {0x199337EB, {0xF3, 0x00,}}, //25.57507202     
        {0x1A054609, {0xF4, 0x00,}}, //26.02059991     
        {0x1A71C346, {0xF5, 0x00,}}, //26.44438589     
        {0x1AD93441, {0xF6, 0x00,}}, //26.84845362     
        {0x1B3C0BE8, {0xF7, 0x00,}}, //27.23455672     
        {0x1B9AAE7A, {0xF8, 0x00,}}, //27.60422483     
        {0x1BF573ED, {0xF9, 0x00,}}, //27.95880017     
        {0x1C4CA9DD, {0xFA, 0x00,}}, //28.29946696     
        {0x1CA0951C, {0xFB, 0x00,}}, //28.62727528     
        {0x1CF172F9, {0xFC, 0x00,}}, //28.94316063     
        {0x1D3F7A4D, {0xFD, 0x00,}}, //29.24795996     
        {0x1D8ADC5E, {0xFE, 0x00,}}, //29.54242509     
        {0x1DD3C599, {0xFF, 0x00,}}, //29.82723388     
    },
    .temperatue_gain_table =
    {
        .agc_control_by_temperatue      = 0,
        .Aptina_temperatue_mode         = 0,
        .temperatue_control_reg_index   = 0,
        .temperatue_gain_num            = 0,
        .temperatue_reg                 = {GADI_VI_SENSOR_TABLE_FLAG_END,},
        .gain_reg                       = {GADI_VI_SENSOR_TABLE_FLAG_END,},
        .temperatue_gain                = {0},
        .temperatue_control             = {0},
    },
    .magic_end  = GADI_SENSOR_HW_INFO_MAGIC_END,
};


