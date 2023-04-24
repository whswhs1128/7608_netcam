/*!
*****************************************************************************
** \file        imx238_reg_tbl.c
**
** \version     $Id: imx238_reg_tbl.c 11627 2017-03-10 10:58:12Z dengbiao $
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
#if 1 // for i2c
GADI_VI_SensorDrvInfoT    imx238_sensor =
{
    .magic_start        = GADI_SENSOR_HW_INFO_MAGIC_START,
    .version            = GADI_SENSOR_HW_INFO_VERSION,
    .length             = sizeof(GADI_VI_SensorDrvInfoT),
    .file_date          = 0x20161228,
    .HwInfo         =
    {
        .name               = "imx238",
        .ctl_type           = GADI_SENSOR_CTL_I2C,
        .addr_len           = sizeof(GADI_U16),
        .data_len           = sizeof(GADI_U8),
        .reset_active_level = GADI_GPIO_LOW,
        .hw_addr            = {(0x34>>1), 0, 0, 0},
        .id_reg =
        {
            {0x31D4, 0x02, 0x0F},
            {0x31D3, 0x38},
        },
        .reset_reg =
        {
            {0x3003, 0x01}, // BIT[0]
                            // Standby control 0:Operating, 1: reset
            {0x3000, 0x00}, // BIT[0]STANDBY 0: Normal operation, 1:STANDBY
            {GADI_VI_SENSOR_TABLE_FLAG_MSLEEP, 0x0A},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        .standby_reg =
        {
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
    },
    .debug_print        = 0,
    .input_format       = GADI_VI_INPUT_FORMAT_RGB_RAW,
    .field_format       = 1,
    .adapter_id         = 0,
    .sensor_id          = GADI_ISP_SENSOR_IMX238,
    .sensor_double_step = 20,
    .typeofsensor       = 2,
    .sync_time_delay    = 0,
    .source_type.dummy  = GADI_VI_CMOS_CHANNEL_TYPE_AUTO,
    .dev_type           = GADI_VI_DEV_TYPE_CMOS,
    .video_system       = GADI_VIDEO_SYSTEM_AUTO,
    .vs_polarity        = GADI_VI_RISING_EDGE,
    .hs_polarity        = GADI_VI_RISING_EDGE,
    .data_edge          = GADI_VI_RISING_EDGE,
    .emb_sync_switch    = GADI_VI_EMB_SYNC_ON, // 外同步720P画面抖动。
    .emb_sync_loc       = GADI_VI_EMB_SYNC_LOWER_PEL,
    .emb_sync_mode      = GADI_VI_EMB_SYNC_FULL_RANGE,
    .max_width          = 1280,
    .max_height         = 960,
    .def_sh_time        = GADI_VIDEO_FPS(50),
    .fmt_num            = 6,
    .auto_fmt           = GADI_VIDEO_MODE(1280, 960,  25, 1),
    .init_reg           =
    {
        // CHIP ID:0x02
        {0x3001, 0x00}, // [0] REGHOLD 1:Valid 0:Invalid
        {0x3002, 0x00}, // [0] XMSTA 1:Marster mode operation stop 0:Marster mode operation start
        //{0x3004, 0x10},
        {0x3005, 0x01},
        //{0x3006, 0x00},
        {0x3008, 0x00}, // 0x10
        //{0x300C, 0x00},
        //{0x300D, 0x20},
        //{0x300E, 0x01},
        //{0x300F, 0x01},
        //{0x3010, 0x39},
        //{0x3011, 0x00}, // 0x00:Higl light perfomance mode 0x14:Low
        //{0x3012, 0x50},
        //{0x3013, 0x00},
        //{0x3015, 0x00},
        //{0x3016, 0x08},
        {0x3017, 0x01},
        {0x301D, 0xFF},
        {0x301E, 0x01},
        //{0x301F, 0x00},
        //{0x3036, 0x14}, // [4:0] WINWV_OB
        //{0x3037, 0x00},
        //{0x3040, 0x00},
        //{0x3041, 0x00},
        //{0x3042, 0x00},
        //{0x3043, 0x00},
        {0x3044, 0x01}, // BIT[3:0]
                        // BIT[7:4] OPORTSEL
                        // Output system selection
                        // 0x00: Parallel CMOS SDR output
                        // 0x06: Parallel low-voltage LVDS DDR output
                        // 0x0C: Serial low-voltage LVDS 1ch output
                        // 0x0D: Serial low-voltage LVDS 2ch output
                        // 0x0E: Serial low-voltage LVDS 4ch output
                        // Others: Setting prohibited
        //{0x3045, 0x01},
        //{0x3046, 0x00}, // [5:4] XVSLNG 0:1H 1:2H 2:4H 3:8H
        //{0x3047, 0x00}, // [5:4] XHSLNG 0:64 clk 1:128 clk 2:256 clk 3:512 clk
        //{0x3048, 0x00},
        {0x3049, 0x0A}, // [1:0] 0:output high 2: VSYNC output Others: Setting prohibited
                          // [3:2] 0:output high 2: HSYNC output Others: Setting prohibited
        {0x3054, 0x63},
        //{0x305C, 0x20},
        //{0x3060, 0x00},
        //{0x3061, 0x21},
        //{0x3062, 0x08},
        {0x30BF, 0x1F},
        // CHIP ID:0x03
        {0x3112, 0x00},
        {0x311D, 0x07},
        {0x3123, 0x07},
        {0x3126, 0xDF},
        {0x3147, 0x87},
        // CHIP ID:0x04
        {0x3203, 0xCD},
        {0x3207, 0x4B},
        {0x3209, 0xE9},
        {0x3213, 0x1B},
        {0x3215, 0xED},
        {0x3216, 0x01},
        {0x3218, 0x09},
        {0x321A, 0x19},
        {0x321B, 0xA1},
        {0x321C, 0x11},
        {0x3227, 0x00},
        {0x3228, 0x05},
        {0x3229, 0xEC},
        {0x322A, 0x40},
        {0x322B, 0x11},
        {0x322D, 0x22},
        {0x322E, 0x00},
        {0x322F, 0x05},
        {0x3231, 0xEC},
        {0x3232, 0x40},
        {0x3233, 0x11},
        {0x3235, 0x23},
        {0x3236, 0xB0},
        {0x3237, 0x04},
        {0x3239, 0x24},
        {0x323A, 0x30},
        {0x323B, 0x04},
        {0x323C, 0xED},
        {0x323D, 0xC0},
        {0x323E, 0x10},
        {0x3240, 0x44},
        {0x3241, 0xA0},
        {0x3242, 0x04},
        {0x3243, 0x0D},
        {0x3244, 0x31},
        {0x3245, 0x11},
        {0x3247, 0xEC},
        {0x3248, 0xD0},
        {0x3249, 0x1D},
        {0x3252, 0xFF},
        {0x3253, 0xFF},
        {0x3254, 0xFF},
        {0x3255, 0x02},
        {0x3256, 0x54},
        {0x3257, 0x60},
        {0x3258, 0x1F},
        {0x325A, 0xA9},
        {0x325B, 0x50},
        {0x325C, 0x0A},
        {0x325D, 0x25},
        {0x325E, 0x11},
        {0x325F, 0x12},
        {0x3261, 0x9B},
        {0x3266, 0xD0},
        {0x3267, 0x08},
        {0x326A, 0x20},
        {0x326B, 0x0A},
        {0x326E, 0x20},
        {0x326F, 0x0A},
        {0x3272, 0x20},
        {0x3273, 0x0A},
        {0x3275, 0xEC},
        {0x327D, 0xA5},
        {0x327E, 0x20},
        {0x327F, 0x0A},
        {0x3281, 0xEF},
        {0x3282, 0xC0},
        {0x3283, 0x0E},
        {0x3285, 0xF6},
        {0x328A, 0x60},
        {0x328B, 0x1F},
        {0x328D, 0xBB},
        {0x328E, 0x90},
        {0x328F, 0x0D},
        {0x3290, 0x39},
        {0x3291, 0xC1},
        {0x3292, 0x1D},
        {0x3294, 0x1D},
        {0x3295, 0x70},
        {0x3296, 0x0E},
        {0x3297, 0x47},
        {0x3298, 0xA1},
        {0x3299, 0x1E},
        {0x329B, 0xC5},
        {0x329C, 0xB0},
        {0x329D, 0x0E},
        {0x329E, 0x43},
        {0x329F, 0xE1},
        {0x32A0, 0x1E},
        {0x32A2, 0xBB},
        {0x32A3, 0x10},
        {0x32A4, 0x0C},
        {0x32A6, 0xB3},
        {0x32A7, 0x30},
        {0x32A8, 0x0A},
        {0x32A9, 0x29},
        {0x32AA, 0x91},
        {0x32AB, 0x11},
        {0x32AD, 0xB4},
        {0x32AE, 0x40},
        {0x32AF, 0x0A},
        {0x32B0, 0x2A},
        {0x32B1, 0xA1},
        {0x32B2, 0x11},
        {0x32B4, 0xAB},
        {0x32B5, 0xB0},
        {0x32B6, 0x0B},
        {0x32B7, 0x21},
        {0x32B8, 0x11},
        {0x32B9, 0x13},
        {0x32BB, 0xAC},
        {0x32BC, 0xC0},
        {0x32BD, 0x0D},
        {0x32BE, 0x22},
        {0x32BF, 0x21},
        {0x32C0, 0x13},
        {0x32C2, 0xAD},
        {0x32C3, 0x10},
        {0x32C4, 0x0B},
        {0x32C5, 0x23},
        {0x32C6, 0x71},
        {0x32C7, 0x12},
        {0x32C9, 0xB5},
        {0x32CA, 0x90},
        {0x32CB, 0x0B},
        {0x32CC, 0x2B},
        {0x32CD, 0xF1},
        {0x32CE, 0x12},
        {0x32D0, 0xBB},
        {0x32D1, 0x10},
        {0x32D2, 0x0C},
        {0x32D4, 0xE7},
        {0x32D5, 0x90},
        {0x32D6, 0x0E},
        {0x32D8, 0x45},
        {0x32D9, 0x11},
        {0x32DA, 0x1F},
        {0x32EB, 0xA4},
        {0x32EC, 0x60},
        {0x32ED, 0x1F},
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .pll_table =
    {
        [0] =
        {
            // for 1080P@30, 1080P@25, 720P@60, 720P@50
            .pixclk = 74250000,
            .extclk = 37125000,
            .regs =
            {
                {0x305B, 0x00}, // INCKSEL1 BIT[0] 1:INCK=27/54 0:INCK=37.125/74.25
                {0x305D, 0x00}, // INCKSEL2 BIT[4] 1:INCK=54/74.25 0:INCK=27/37.125
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
        [1] =
        {
            // for 960P@30, 960P@25
            .pixclk = 54000000,
            .extclk = 27000000,
            .regs =
            {
                {0x305B, 0x01}, // INCKSEL1 BIT[0] 1:INCK=27/54 0:INCK=37.125/74.25
                {0x305D, 0x00}, // INCKSEL2 BIT[4] 1:INCK=54/74.25 0:INCK=27/37.125
                {0x305F, 0x00}, // INCKSEL3 BIT[4] 1:INCK=54 0:INCK=27
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
        [2] =
        {
            // for 720P@60, 720P@50, 720P@30, 720P@25
            .pixclk = 74250000, // 720P@30, 720P@25 in fact output is 37125000,
            .extclk = 37125000,
            .regs =
            {
                {0x305B, 0x00}, // INCKSEL1 BIT[0] 1:INCK=27/54 0:INCK=37.125/74.25
                {0x305D, 0x00}, // INCKSEL2 BIT[4] 1:INCK=54/74.25 0:INCK=27/37.125
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
    },
    .video_fmt_table =
    {
        [0] = // 1280x960P@30fps    (3600 / 2) * 1000 * 30 = 54000000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  960,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x3000, 0x01, 0x01}, // 0x3000 BIT[0]STANDBY 0: Normal operation, 1:STANDBY
                {0x3007, 0x00}, // BIT[0] VREVERSE, BIT[1] VREVERSE, 0:normal 1:Inverted
                                // BIT[5:4] Readout mode designation
                                // 0: All pixel scan mode
                                // 1: 720p mode
                                // 2: Window cropping from All pixel scan mode
                                // 3: Window cropping from A720p mode
                {0x3009, 0x02}, // BIT[1:0] FRSEL 2: 30fps 1: 60fps 0/3: setting prohibited
                {0x300A, 0xF0}, // BLKLEVEL_L=BLKLEVEL AD10bit=60d = 0x3C AD12bit=240d = 0xF0
                {0x300B, 0x00}, // BLKLEVEL_H=BLKLEVEL Black level offset value setting
                {0x301B, 0x10}, // HMAX_L   3600 = 0x0E10
                {0x301C, 0x0E}, // HMAX_H
                {0x3038, 0x00}, // WINPV_L
                {0x3039, 0x00}, // WINPV_H
                {0x303A, 0xC9}, // WINWV_L  969 = 0x03C9 = 4+960+5//1069 = 0x042D = 7+1+12+4+8+1024+9+4
                {0x303B, 0x03}, // WINWV_H
                {0x303C, 0x00}, // WINPH_L
                {0x303D, 0x00}, // WINPH_H
                {0x304E, 0x20}, // WINWH_L  1312 = 0x0520 = 4+4+8+1280+8+5+3
                {0x304F, 0x05}, // WINWH_H
                {0x3000, 0x00, 0x01}, // 0x3000 BIT[0]STANDBY 0: Normal operation, 1:STANDBY
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 1280,
                .height     = 960,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_DVP, 
                .bits       = GADI_VIDEO_BITS_12,
                .ratio      = GADI_VIDEO_RATIO_4_3,
                .srm        = 0,
                .pll_index  = 1,
                .max_fps    = GADI_VIDEO_FPS(30),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(30),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_960P_30,
            },
            .video_info_table =
            {
                .def_start_x    = (4 + 4 + 8) + 20,
                .def_start_y    = (7 + 1 + 12 + 4 + 8),
                .def_width      = 1280,
                .def_height     = 960,
                .sync_start     = (0 + 0),
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_GB,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                },
            },
            .update_after_vsync_start   = 0,
            .update_after_vsync_end     = 10,
        },
        [1] = // 1280x960P@25fps    (4320 / 2) * 1000 * 25 = 54000000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  960,  25, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x3000, 0x01, 0x01}, // 0x3000 BIT[0]STANDBY 0: Normal operation, 1:STANDBY
                {0x3007, 0x00}, // BIT[0] VREVERSE, BIT[1] VREVERSE, 0:normal 1:Inverted
                                // BIT[5:4] Readout mode designation
                                // 0: All pixel scan mode
                                // 1: 720p mode
                                // 2: Window cropping from All pixel scan mode
                                // 3: Window cropping from A720p mode
                {0x3009, 0x02}, // BIT[1:0] FRSEL 2: 30fps 1: 60fps 0/3: setting prohibited
                {0x300A, 0xF0}, // BLKLEVEL_L=BLKLEVEL AD10bit=60d = 0x3C AD12bit=240d = 0xF0
                {0x300B, 0x00}, // BLKLEVEL_H=BLKLEVEL Black level offset value setting
                {0x301B, 0xE0}, // HMAX_L   4320 = 0x10E0
                {0x301C, 0x10}, // HMAX_H
                {0x3038, 0x00}, // WINPV_L
                {0x3039, 0x00}, // WINPV_H
                {0x303A, 0xC9}, // WINWV_L  969 = 0x03C9 = 4+960+5//1069 = 0x042D = 7+1+12+4+8+1024+9+4
                {0x303B, 0x03}, // WINWV_H
                {0x303C, 0x00}, // WINPH_L
                {0x303D, 0x00}, // WINPH_H
                {0x304E, 0x20}, // WINWH_L  1312 = 0x0520 = 4+4+8+1280+8+5+3
                {0x304F, 0x05}, // WINWH_H
                {0x3000, 0x00, 0x01}, // 0x3000 BIT[0]STANDBY 0: Normal operation, 1:STANDBY
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 1280,
                .height     = 960,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_DVP, 
                .bits       = GADI_VIDEO_BITS_12,
                .ratio      = GADI_VIDEO_RATIO_4_3,
                .srm        = 0,
                .pll_index  = 1,
                .max_fps    = GADI_VIDEO_FPS(25),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(25),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_960P_25,
            },
            .video_info_table =
            {
                .def_start_x    = (4 + 4 + 8) + 20,
                .def_start_y    = (7 + 1 + 12 + 4 + 8),
                .def_width      = 1280,
                .def_height     = 960,
                .sync_start     = (0 + 0),
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_GB,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                },
            },
            .update_after_vsync_start   = 0,
            .update_after_vsync_end     = 10,
        },
        [2] = // 1280x720P@60fps    (3300 / 2) * 750 * 60 = 74250000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  60, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x3000, 0x01, 0x01}, // 0x3000 BIT[0]STANDBY 0: Normal operation, 1:STANDBY
                {0x3007, 0x10}, // BIT[0] VREVERSE, BIT[1] VREVERSE, 0:normal 1:Inverted
                                // BIT[5:4] Readout mode designation
                                // 0: All pixel scan mode
                                // 1: 720p mode
                                // 2: Window cropping from All pixel scan mode
                                // 3: Window cropping from A720p mode
                {0x3009, 0x01}, // BIT[1:0] FRSEL 2: 30fps 1: 60fps 0/3: setting prohibited
                {0x300A, 0xF0}, // BLKLEVEL_L=BLKLEVEL AD10bit=60d = 0x3C AD12bit=240d = 0xF0
                {0x300B, 0x00}, // BLKLEVEL_H=BLKLEVEL Black level offset value setting
                {0x301B, 0xE4}, // HMAX_L   3300 = 0x0CE4
                {0x301C, 0x0C}, // HMAX_H
                {0x3038, 0x00}, // WINPV_L
                {0x3039, 0x00}, // WINPV_H
                {0x303A, 0xD9}, // WINWV_L   729 = 0x02D9 = 4+720+5
                {0x303B, 0x02}, // WINWV_H
                {0x303C, 0x00}, // WINPH_L
                {0x303D, 0x00}, // WINPH_H
                {0x304E, 0x20}, // WINWH_L  1312 = 0x0520 = 4+4+8+1280+8+5+3
                {0x304F, 0x05}, // WINWH_H
                {0x3000, 0x00, 0x01}, // 0x3000 BIT[0]STANDBY 0: Normal operation, 1:STANDBY
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 1280,
                .height     = 720,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_DVP, 
                .bits       = GADI_VIDEO_BITS_12,
                .ratio      = GADI_VIDEO_RATIO_16_9,
                .srm        = 0,
                .pll_index  = 2,
                .max_fps    = GADI_VIDEO_FPS(60),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(60),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_720P_60,
            },
            .video_info_table =
            {
                .def_start_x    = (4 + 4 + 8) + 40, // use emb_sync
                .def_start_y    = (7 + 1 + 4 + 4 + 2 + 4),
                .def_width      = 1280,
                .def_height     = 720,
                .sync_start     = (0 + 0),
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_GB,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                },
            },
            .update_after_vsync_start   = 0,
            .update_after_vsync_end     = 10,
        },
        [3] = // 1280x720P@50fps    (3960 / 2) * 750 * 50 = 74250000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  50, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x3000, 0x01, 0x01}, // 0x3000 BIT[0]STANDBY 0: Normal operation, 1:STANDBY
                {0x3007, 0x10}, // BIT[0] VREVERSE, BIT[1] VREVERSE, 0:normal 1:Inverted
                                // BIT[5:4] Readout mode designation
                                // 0: All pixel scan mode
                                // 1: 720p mode
                                // 2: Window cropping from All pixel scan mode
                                // 3: Window cropping from A720p mode
                {0x3009, 0x01}, // BIT[1:0] FRSEL 2: 30fps 1: 60fps 0/3: setting prohibited
                {0x300A, 0xF0}, // BLKLEVEL_L=BLKLEVEL AD10bit=60d = 0x3C AD12bit=240d = 0xF0
                {0x300B, 0x00}, // BLKLEVEL_H=BLKLEVEL Black level offset value setting
                {0x301B, 0x78}, // HMAX_L   3960 = 0x0F78
                {0x301C, 0x0F}, // HMAX_H
                {0x3038, 0x00}, // WINPV_L
                {0x3039, 0x00}, // WINPV_H
                {0x303A, 0xD9}, // WINWV_L   729 = 0x02D9 = 4+720+5
                {0x303B, 0x02}, // WINWV_H
                {0x303C, 0x00}, // WINPH_L
                {0x303D, 0x00}, // WINPH_H
                {0x304E, 0x20}, // WINWH_L  1312 = 0x0520 = 4+4+8+1280+8+5+3
                {0x304F, 0x05}, // WINWH_H
                {0x3000, 0x00, 0x01}, // 0x3000 BIT[0]STANDBY 0: Normal operation, 1:STANDBY
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 1280,
                .height     = 720,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_DVP, 
                .bits       = GADI_VIDEO_BITS_12,
                .ratio      = GADI_VIDEO_RATIO_16_9,
                .srm        = 0,
                .pll_index  = 2,
                .max_fps    = GADI_VIDEO_FPS(50),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(50),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_720P_50,
            },
            .video_info_table =
            {
                .def_start_x    = (4 + 4 + 8) + 40, // use emb_sync
                .def_start_y    = (7 + 1 + 4 + 4 + 2 + 4),
                .def_width      = 1280,
                .def_height     = 720,
                .sync_start     = (0 + 0),
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_GB,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                },
            },
            .update_after_vsync_start   = 0,
            .update_after_vsync_end     = 10,
        },
        [4] = // 1280x720P@30fps    (6600 / 4) * 750 * 30 = 37125000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x3000, 0x01, 0x01}, // 0x3000 BIT[0]STANDBY 0: Normal operation, 1:STANDBY
                {0x3007, 0x10}, // BIT[0] VREVERSE, BIT[1] VREVERSE, 0:normal 1:Inverted
                                // BIT[5:4] Readout mode designation
                                // 0: All pixel scan mode
                                // 1: 720p mode
                                // 2: Window cropping from All pixel scan mode
                                // 3: Window cropping from A720p mode
                {0x3009, 0x02}, // BIT[1:0] FRSEL 2: 30fps 1: 60fps 0/3: setting prohibited
                {0x300A, 0xF0}, // BLKLEVEL_L=BLKLEVEL AD10bit=60d = 0x3C AD12bit=240d = 0xF0
                {0x300B, 0x00}, // BLKLEVEL_H=BLKLEVEL Black level offset value setting
                {0x301B, 0xC8}, // HMAX_L   6600 = 0x19C8
                {0x301C, 0x19}, // HMAX_H
                {0x3038, 0x00}, // WINPV_L
                {0x3039, 0x00}, // WINPV_H
                {0x303A, 0xD9}, // WINWV_L   729 = 0x02D9 = 4+720+5
                {0x303B, 0x02}, // WINWV_H
                {0x303C, 0x00}, // WINPH_L
                {0x303D, 0x00}, // WINPH_H
                {0x304E, 0x20}, // WINWH_L  1312 = 0x0520 = 4+4+8+1280+8+5+3
                {0x304F, 0x05}, // WINWH_H
                {0x3000, 0x00, 0x01}, // 0x3000 BIT[0]STANDBY 0: Normal operation, 1:STANDBY
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 1280,
                .height     = 720,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_DVP, 
                .bits       = GADI_VIDEO_BITS_12,
                .ratio      = GADI_VIDEO_RATIO_16_9,
                .srm        = 0,
                .pll_index  = 2,
                .max_fps    = GADI_VIDEO_FPS(30),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(30),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_720P_30,
            },
            .video_info_table =
            {
                .def_start_x    = (4 + 4 + 8) + 40, // use emb_sync
                .def_start_y    = (7 + 1 + 4 + 4 + 2 + 4),
                .def_width      = 1280,
                .def_height     = 720,
                .sync_start     = (0 + 0),
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_GB,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                },
            },
            .update_after_vsync_start   = 0,
            .update_after_vsync_end     = 10,
        },
        [5] = // 1280x720P@25fps    (7920 / 4) * 750 * 25 = 37125000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  25, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x3007, 0x10}, // BIT[0] VREVERSE, BIT[1] VREVERSE, 0:normal 1:Inverted
                                // BIT[5:4] Readout mode designation
                                // 0: All pixel scan mode
                                // 1: 720p mode
                                // 2: Window cropping from All pixel scan mode
                                // 3: Window cropping from A720p mode
                {0x3009, 0x02}, // BIT[1:0] FRSEL 2: 30fps 1: 60fps 0/3: setting prohibited
                {0x300A, 0xF0}, // BLKLEVEL_L=BLKLEVEL AD10bit=60d = 0x3C AD12bit=240d = 0xF0
                {0x300B, 0x00}, // BLKLEVEL_H=BLKLEVEL Black level offset value setting
                {0x301B, 0xF0}, // HMAX_L   7920 = 0x1EF0
                {0x301C, 0x1E}, // HMAX_H
                {0x3038, 0x00}, // WINPV_L
                {0x3039, 0x00}, // WINPV_H
                {0x303A, 0xD9}, // WINWV_L   729 = 0x02D9 = 4+720+5
                {0x303B, 0x02}, // WINWV_H
                {0x303C, 0x00}, // WINPH_L
                {0x303D, 0x00}, // WINPH_H
                {0x304E, 0x20}, // WINWH_L  1312 = 0x0520 = 4+4+8+1280+8+5+3
                {0x304F, 0x05}, // WINWH_H
                {0x3000, 0x00, 0x01}, // 0x3000 BIT[0]STANDBY 0: Normal operation, 1:STANDBY
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 1280,
                .height     = 720,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_DVP, 
                .bits       = GADI_VIDEO_BITS_12,
                .ratio      = GADI_VIDEO_RATIO_16_9,
                .srm        = 0,
                .pll_index  = 2,
                .max_fps    = GADI_VIDEO_FPS(25),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(25),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_720P_25,
            },
            .video_info_table =
            {
                .def_start_x    = (4 + 4 + 8) + 40, // use emb_sync
                .def_start_y    = (7 + 1 + 4 + 4 + 2 + 4),
                .def_width      = 1280,
                .def_height     = 720,
                .sync_start     = (0 + 0),
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_GB,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                },
            },
            .update_after_vsync_start   = 0,
            .update_after_vsync_end     = 10,
        },
    },
#define IMX238_H_MIRROR     (1<<1)
#define IMX238_V_FLIP       (1<<0)
    .mirror_table =
    {
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY_VERTICALLY] =
        {
            {0x3007, IMX238_H_MIRROR | IMX238_V_FLIP, IMX238_H_MIRROR | IMX238_V_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY] =
        {
            {0x3007, IMX238_H_MIRROR, IMX238_H_MIRROR | IMX238_V_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_VERTICALLY] =
        {
            {0x3007, IMX238_V_FLIP, IMX238_H_MIRROR | IMX238_V_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_NONE] =
        {
            {0x3007, 0x00, IMX238_H_MIRROR | IMX238_V_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
    },
    .version_reg =
    {
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .reg_ctl_by_ver_num = 0,
    .control_by_version_reg = {0},
    .vmax_clk_ref   = GADI_SENSOR_CAL_CLK_PIXCLK_X2,
    .shs_clk_ref    = GADI_SENSOR_CAL_CLK_PIXCLK_X2,
    .cal_shs_mode   = GADI_SENSOR_CAL_SHS_BLK_MODE,
    .shs_fix        = 0,
    .shs_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_SET,
    .agc_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_SET,
    .vmax_reg_update_mode       = GADI_SENSOR_UPDATE_REG_BY_SET,
    .mirror_reg_update_mode     = GADI_SENSOR_UPDATE_REG_BY_SET,
    .shs_additional_use         = 0,
    .shs_additional_percent     = 0,
    // hmax = (HMAX_MSB & 0xFF) << 8 +
    //        (HMAX_LSB & 0xFF) << 0
    .hmax_reg =
    {
        {0x301C, 0x00, 0xFF, 0x00, 0x08, 0}, // HMAX_MSB
        {0x301B, 0x00, 0xFF, 0x00, 0x00, 0}, // HMAX_LSB
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    // vmax = (VMAX_MSB & 0xFF) << 8 +
    //        (VMAX_LSB & 0xFF) << 0
    .vmax_reg =
    {
        {0x3019, 0x00, 0xFF, 0x00, 0x08, 0}, // VMAX_MSB
        {0x3018, 0x00, 0xFF, 0x00, 0x00, 0}, // VMAX_LSB
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .vmax_wadd_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    // shs = (INTEG_TIME_H & 0xFF) << 8 +
    //       (INTEG_TIME_L & 0xFF) << 0
    .shs_reg =
    {
        {0x3021, 0x00, 0xFF, 0x00, 0x08, 0}, // INTEG_TIME_H
        {0x3020, 0x00, 0xFF, 0x00, 0x00, 0}, // INTEG_TIME_L
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .shs_additional_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    .cal_gain_mode              = GADI_SENSOR_CAL_GAIN_MODE0,
    .max_agc_index = 240,
    .max_again_index = 0,
    .cur_again_index = 0,
    .agc_info =
    {
        .db_max     = 0x30000000,    // 48dB
        .db_min     = 0x00000000,    // 0dB
        .db_step    = 0x004CCCCD,    // 0.3dB
    },
    .gain_reg =
    {
        {0x3014, 0x00}, // -6dB to 42dB /0.3dB step
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .gain_table =
    {
        {0},
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
#else // for spi
GADI_VI_SensorDrvInfoT    imx238_sensor =
{
    .magic_start        = GADI_SENSOR_HW_INFO_MAGIC_START,
    .version            = GADI_SENSOR_HW_INFO_VERSION,
    .length             = sizeof(GADI_VI_SensorDrvInfoT),
    .file_date          = 0x20161228,
    .HwInfo         =
    {
        .name               = "imx238",
        .ctl_type           = GADI_SENSOR_CTL_SPI,
        .addr_len           = sizeof(GADI_U16),
        .data_len           = sizeof(GADI_U8),
        .reset_active_level = GADI_GPIO_LOW,
        .hw_addr            = (0x34>>1),
        .id_reg =
        {
            {0x03D4, 0x02, 0x0F},
            {0x03D3, 0x38},
        },
        .reset_reg =
        {
            {0x0203, 0x01}, // 0x3003 BIT[0]
                            // Standby control 0:Operating, 1: reset
            {0x0200, 0x00}, // 0x3000 BIT[0]STANDBY 0: Normal operation, 1:STANDBY
            {GADI_VI_SENSOR_TABLE_FLAG_MSLEEP, 0x0A},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
    },
    .debug_print        = 0,
    .input_format       = GADI_VI_INPUT_FORMAT_RGB_RAW,
    .field_format       = 1,
    .adapter_id         = 0,
    .sensor_id          = GADI_ISP_SENSOR_IMX238,
    .sensor_double_step = 20,
    .typeofsensor       = 2,
    .sync_time_delay    = 0,
    .source_type.dummy  = GADI_VI_CMOS_CHANNEL_TYPE_AUTO,
    .dev_type           = GADI_VI_DEV_TYPE_CMOS,
    .video_system       = GADI_VIDEO_SYSTEM_AUTO,
    .vs_polarity        = GADI_VI_RISING_EDGE,
    .hs_polarity        = GADI_VI_RISING_EDGE,
    .data_edge          = GADI_VI_RISING_EDGE,
    .emb_sync_switch    = GADI_VI_EMB_SYNC_ON, // 外同步720P画面抖动。
    .emb_sync_loc       = GADI_VI_EMB_SYNC_LOWER_PEL,
    .emb_sync_mode      = GADI_VI_EMB_SYNC_FULL_RANGE,
    .max_width          = 1280,
    .max_height         = 960,
    .def_sh_time        = GADI_VIDEO_FPS(50),
    .fmt_num            = 6,
    .auto_fmt           = GADI_VIDEO_MODE(1280, 960,  25, 1),
    .init_reg           =
    {
        // CHIP ID:0x02
        {0x0201, 0x00}, // [0] REGHOLD 1:Valid 0:Invalid
        {0x0202, 0x00}, // [0] XMSTA 1:Marster mode operation stop 0:Marster mode operation start
        //{0x0204, 0x10},
        {0x0205, 0x01},
        //{0x0206, 0x00},
        {0x0208, 0x00}, // 0x10
        //{0x020C, 0x00},
        //{0x020D, 0x20},
        //{0x020E, 0x01},
        //{0x020F, 0x01},
        //{0x0210, 0x39},
        //{0x0211, 0x00}, // 0x00:Higl light perfomance mode 0x14:Low
        //{0x0212, 0x50},
        //{0x0213, 0x00},
        //{0x0215, 0x00},
        //{0x0216, 0x08},
        {0x0217, 0x01},
        {0x021D, 0xFF},
        {0x021E, 0x01},
        //{0x021F, 0x00},
        //{0x0236, 0x14}, // [4:0] WINWV_OB
        //{0x0237, 0x00},
        //{0x0240, 0x00},
        //{0x0241, 0x00},
        //{0x0242, 0x00},
        //{0x0243, 0x00},
        {0x0244, 0x01}, // BIT[3:0]
                        // BIT[7:4] OPORTSEL
                        // Output system selection
                        // 0x00: Parallel CMOS SDR output
                        // 0x06: Parallel low-voltage LVDS DDR output
                        // 0x0C: Serial low-voltage LVDS 1ch output
                        // 0x0D: Serial low-voltage LVDS 2ch output
                        // 0x0E: Serial low-voltage LVDS 4ch output
                        // Others: Setting prohibited
        //{0x0245, 0x01},
        //{0x0246, 0x00}, // [5:4] XVSLNG 0:1H 1:2H 2:4H 3:8H
        //{0x0247, 0x00}, // [5:4] XHSLNG 0:64 clk 1:128 clk 2:256 clk 3:512 clk
        //{0x0248, 0x00},
        {0x0249, 0x0A}, // [1:0] 0:output high 2: VSYNC output Others: Setting prohibited
                          // [3:2] 0:output high 2: HSYNC output Others: Setting prohibited
        {0x0254, 0x63},
        //{0x025C, 0x20},
        //{0x0260, 0x00},
        //{0x0261, 0x21},
        //{0x0262, 0x08},
        {0x02BF, 0x1F},

        // CHIP ID:0x03
        {0x0312, 0x00},
        {0x031D, 0x07},
        {0x0323, 0x07},
        {0x0326, 0xDF},
        {0x0347, 0x87},

        // CHIP ID:0x04
        {0x0403, 0xCD},
        {0x0407, 0x4B},
        {0x0409, 0xE9},
        {0x0413, 0x1B},
        {0x0415, 0xED},
        {0x0416, 0x01},
        {0x0418, 0x09},
        {0x041A, 0x19},
        {0x041B, 0xA1},
        {0x041C, 0x11},
        {0x0427, 0x00},
        {0x0428, 0x05},
        {0x0429, 0xEC},
        {0x042A, 0x40},
        {0x042B, 0x11},
        {0x042D, 0x22},
        {0x042E, 0x00},
        {0x042F, 0x05},
        {0x0431, 0xEC},
        {0x0432, 0x40},
        {0x0433, 0x11},
        {0x0435, 0x23},
        {0x0436, 0xB0},
        {0x0437, 0x04},
        {0x0439, 0x24},
        {0x043A, 0x30},
        {0x043B, 0x04},
        {0x043C, 0xED},
        {0x043D, 0xC0},
        {0x043E, 0x10},
        {0x0440, 0x44},
        {0x0441, 0xA0},
        {0x0442, 0x04},
        {0x0443, 0x0D},
        {0x0444, 0x31},
        {0x0445, 0x11},
        {0x0447, 0xEC},
        {0x0448, 0xD0},
        {0x0449, 0x1D},
        {0x0452, 0xFF},
        {0x0453, 0xFF},
        {0x0454, 0xFF},
        {0x0455, 0x02},
        {0x0456, 0x54},
        {0x0457, 0x60},
        {0x0458, 0x1F},
        {0x045A, 0xA9},
        {0x045B, 0x50},
        {0x045C, 0x0A},
        {0x045D, 0x25},
        {0x045E, 0x11},
        {0x045F, 0x12},
        {0x0461, 0x9B},
        {0x0466, 0xD0},
        {0x0467, 0x08},
        {0x046A, 0x20},
        {0x046B, 0x0A},
        {0x046E, 0x20},
        {0x046F, 0x0A},
        {0x0472, 0x20},
        {0x0473, 0x0A},
        {0x0475, 0xEC},
        {0x047D, 0xA5},
        {0x047E, 0x20},
        {0x047F, 0x0A},
        {0x0481, 0xEF},
        {0x0482, 0xC0},
        {0x0483, 0x0E},
        {0x0485, 0xF6},
        {0x048A, 0x60},
        {0x048B, 0x1F},
        {0x048D, 0xBB},
        {0x048E, 0x90},
        {0x048F, 0x0D},
        {0x0490, 0x39},
        {0x0491, 0xC1},
        {0x0492, 0x1D},
        {0x0494, 0x1D},
        {0x0495, 0x70},
        {0x0496, 0x0E},
        {0x0497, 0x47},
        {0x0498, 0xA1},
        {0x0499, 0x1E},
        {0x049B, 0xC5},
        {0x049C, 0xB0},
        {0x049D, 0x0E},
        {0x049E, 0x43},
        {0x049F, 0xE1},
        {0x04A0, 0x1E},
        {0x04A2, 0xBB},
        {0x04A3, 0x10},
        {0x04A4, 0x0C},
        {0x04A6, 0xB3},
        {0x04A7, 0x30},
        {0x04A8, 0x0A},
        {0x04A9, 0x29},
        {0x04AA, 0x91},
        {0x04AB, 0x11},
        {0x04AD, 0xB4},
        {0x04AE, 0x40},
        {0x04AF, 0x0A},
        {0x04B0, 0x2A},
        {0x04B1, 0xA1},
        {0x04B2, 0x11},
        {0x04B4, 0xAB},
        {0x04B5, 0xB0},
        {0x04B6, 0x0B},
        {0x04B7, 0x21},
        {0x04B8, 0x11},
        {0x04B9, 0x13},
        {0x04BB, 0xAC},
        {0x04BC, 0xC0},
        {0x04BD, 0x0D},
        {0x04BE, 0x22},
        {0x04BF, 0x21},
        {0x04C0, 0x13},
        {0x04C2, 0xAD},
        {0x04C3, 0x10},
        {0x04C4, 0x0B},
        {0x04C5, 0x23},
        {0x04C6, 0x71},
        {0x04C7, 0x12},
        {0x04C9, 0xB5},
        {0x04CA, 0x90},
        {0x04CB, 0x0B},
        {0x04CC, 0x2B},
        {0x04CD, 0xF1},
        {0x04CE, 0x12},
        {0x04D0, 0xBB},
        {0x04D1, 0x10},
        {0x04D2, 0x0C},
        {0x04D4, 0xE7},
        {0x04D5, 0x90},
        {0x04D6, 0x0E},
        {0x04D8, 0x45},
        {0x04D9, 0x11},
        {0x04DA, 0x1F},
        {0x04EB, 0xA4},
        {0x04EC, 0x60},
        {0x04ED, 0x1F},
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .pll_table =
    {
        [0] =
        {
            // for 1080P@30, 1080P@25, 720P@60, 720P@50
            .pixclk = 74250000,
            .extclk = 37125000,
            .regs =
            {
                {0x025B, 0x00}, // INCKSEL1 BIT[0] 1:INCK=27/54 0:INCK=37.125/74.25
                {0x025D, 0x00}, // INCKSEL2 BIT[4] 1:INCK=54/74.25 0:INCK=27/37.125
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
        [1] =
        {
            // for 960P@30, 960P@25
            .pixclk = 54000000,
            .extclk = 27000000,
            .regs =
            {
                {0x025B, 0x01}, // INCKSEL1 BIT[0] 1:INCK=27/54 0:INCK=37.125/74.25
                {0x025D, 0x00}, // INCKSEL2 BIT[4] 1:INCK=54/74.25 0:INCK=27/37.125
                {0x025F, 0x00}, // INCKSEL3 BIT[4] 1:INCK=54 0:INCK=27
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
        [2] =
        {
            // for 720P@60, 720P@50, 720P@30, 720P@25
            .pixclk = 74250000, // 720P@30, 720P@25 in fact output is 37125000,
            .extclk = 37125000,
            .regs =
            {
                {0x025B, 0x00}, // INCKSEL1 BIT[0] 1:INCK=27/54 0:INCK=37.125/74.25
                {0x025D, 0x00}, // INCKSEL2 BIT[4] 1:INCK=54/74.25 0:INCK=27/37.125
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
    },
    .video_fmt_table =
    {
        [0] = // 1280x960P@30fps    (3600 / 2) * 1000 * 30 = 54000000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  960,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x0200, 0x01, 0x01}, // 0x3000 BIT[0]STANDBY 0: Normal operation, 1:STANDBY
                {0x0207, 0x00}, // BIT[0] VREVERSE, BIT[1] VREVERSE, 0:normal 1:Inverted
                                // BIT[5:4] Readout mode designation
                                // 0: All pixel scan mode
                                // 1: 720p mode
                                // 2: Window cropping from All pixel scan mode
                                // 3: Window cropping from A720p mode
                {0x0209, 0x02}, // BIT[1:0] FRSEL 2: 30fps 1: 60fps 0/3: setting prohibited
                {0x020A, 0xF0}, // BLKLEVEL_L=BLKLEVEL AD10bit=60d = 0x3C AD12bit=240d = 0xF0
                {0x020B, 0x00}, // BLKLEVEL_H=BLKLEVEL Black level offset value setting
                {0x021B, 0x10}, // HMAX_L   3600 = 0x0E10
                {0x021C, 0x0E}, // HMAX_H
                {0x0238, 0x00}, // WINPV_L
                {0x0239, 0x00}, // WINPV_H
                {0x023A, 0xC9}, // WINWV_L  969 = 0x03C9 = 4+960+5//1069 = 0x042D = 7+1+12+4+8+1024+9+4
                {0x023B, 0x03}, // WINWV_H
                {0x023C, 0x00}, // WINPH_L
                {0x023D, 0x00}, // WINPH_H
                {0x024E, 0x20}, // WINWH_L  1312 = 0x0520 = 4+4+8+1280+8+5+3
                {0x024F, 0x05}, // WINWH_H
                {0x0200, 0x00, 0x01}, // 0x3000 BIT[0]STANDBY 0: Normal operation, 1:STANDBY
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 1280,
                .height     = 960,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_DVP, 
                .bits       = GADI_VIDEO_BITS_12,
                .ratio      = GADI_VIDEO_RATIO_4_3,
                .srm        = 0,
                .pll_index  = 1,
                .max_fps    = GADI_VIDEO_FPS(30),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(30),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_960P_30,
            },
            .video_info_table =
            {
                .def_start_x    = (4 + 4 + 8) + 20,
                .def_start_y    = (7 + 1 + 12 + 4 + 8),
                .def_width      = 1280,
                .def_height     = 960,
                .sync_start     = (0 + 0),
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_GB,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                },
            },
            .update_after_vsync_start   = 0,
            .update_after_vsync_end     = 10,
        },
        [1] = // 1280x960P@25fps    (4320 / 2) * 1000 * 25 = 54000000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  960,  25, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x0200, 0x01, 0x01}, // 0x3000 BIT[0]STANDBY 0: Normal operation, 1:STANDBY
                {0x0207, 0x00}, // BIT[0] VREVERSE, BIT[1] VREVERSE, 0:normal 1:Inverted
                                // BIT[5:4] Readout mode designation
                                // 0: All pixel scan mode
                                // 1: 720p mode
                                // 2: Window cropping from All pixel scan mode
                                // 3: Window cropping from A720p mode
                {0x0209, 0x02}, // BIT[1:0] FRSEL 2: 30fps 1: 60fps 0/3: setting prohibited
                {0x020A, 0xF0}, // BLKLEVEL_L=BLKLEVEL AD10bit=60d = 0x3C AD12bit=240d = 0xF0
                {0x020B, 0x00}, // BLKLEVEL_H=BLKLEVEL Black level offset value setting
                {0x021B, 0xE0}, // HMAX_L   4320 = 0x10E0
                {0x021C, 0x10}, // HMAX_H
                {0x0238, 0x00}, // WINPV_L
                {0x0239, 0x00}, // WINPV_H
                {0x023A, 0xC9}, // WINWV_L  969 = 0x03C9 = 4+960+5//1069 = 0x042D = 7+1+12+4+8+1024+9+4
                {0x023B, 0x03}, // WINWV_H
                {0x023C, 0x00}, // WINPH_L
                {0x023D, 0x00}, // WINPH_H
                {0x024E, 0x20}, // WINWH_L  1312 = 0x0520 = 4+4+8+1280+8+5+3
                {0x024F, 0x05}, // WINWH_H
                {0x0200, 0x00, 0x01}, // 0x3000 BIT[0]STANDBY 0: Normal operation, 1:STANDBY
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 1280,
                .height     = 960,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_DVP, 
                .bits       = GADI_VIDEO_BITS_12,
                .ratio      = GADI_VIDEO_RATIO_4_3,
                .srm        = 0,
                .pll_index  = 1,
                .max_fps    = GADI_VIDEO_FPS(25),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(25),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_960P_25,
            },
            .video_info_table =
            {
                .def_start_x    = (4 + 4 + 8) + 20,
                .def_start_y    = (7 + 1 + 12 + 4 + 8),
                .def_width      = 1280,
                .def_height     = 960,
                .sync_start     = (0 + 0),
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_GB,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                },
            },
            .update_after_vsync_start   = 0,
            .update_after_vsync_end     = 10,
        },
        [2] = // 1280x720P@60fps    (3300 / 2) * 750 * 60 = 74250000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  60, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x0200, 0x01, 0x01}, // 0x3000 BIT[0]STANDBY 0: Normal operation, 1:STANDBY
                {0x0207, 0x10}, // BIT[0] VREVERSE, BIT[1] VREVERSE, 0:normal 1:Inverted
                                // BIT[5:4] Readout mode designation
                                // 0: All pixel scan mode
                                // 1: 720p mode
                                // 2: Window cropping from All pixel scan mode
                                // 3: Window cropping from A720p mode
                {0x0209, 0x01}, // BIT[1:0] FRSEL 2: 30fps 1: 60fps 0/3: setting prohibited
                {0x020A, 0xF0}, // BLKLEVEL_L=BLKLEVEL AD10bit=60d = 0x3C AD12bit=240d = 0xF0
                {0x020B, 0x00}, // BLKLEVEL_H=BLKLEVEL Black level offset value setting
                {0x021B, 0xE4}, // HMAX_L   3300 = 0x0CE4
                {0x021C, 0x0C}, // HMAX_H
                {0x0238, 0x00}, // WINPV_L
                {0x0239, 0x00}, // WINPV_H
                {0x023A, 0xD9}, // WINWV_L   729 = 0x02D9 = 4+720+5
                {0x023B, 0x02}, // WINWV_H
                {0x023C, 0x00}, // WINPH_L
                {0x023D, 0x00}, // WINPH_H
                {0x024E, 0x20}, // WINWH_L  1312 = 0x0520 = 4+4+8+1280+8+5+3
                {0x024F, 0x05}, // WINWH_H
                {0x0200, 0x00, 0x01}, // 0x3000 BIT[0]STANDBY 0: Normal operation, 1:STANDBY
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 1280,
                .height     = 720,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_DVP, 
                .bits       = GADI_VIDEO_BITS_12,
                .ratio      = GADI_VIDEO_RATIO_16_9,
                .srm        = 0,
                .pll_index  = 2,
                .max_fps    = GADI_VIDEO_FPS(60),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(60),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_720P_60,
            },
            .video_info_table =
            {
                .def_start_x    = (4 + 4 + 8) + 40, // use emb_sync
                .def_start_y    = (7 + 1 + 4 + 4 + 2 + 4),
                .def_width      = 1280,
                .def_height     = 720,
                .sync_start     = (0 + 0),
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_GB,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                },
            },
            .update_after_vsync_start   = 0,
            .update_after_vsync_end     = 10,
        },
        [3] = // 1280x720P@50fps    (3960 / 2) * 750 * 50 = 74250000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  50, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x0200, 0x01, 0x01}, // 0x3000 BIT[0]STANDBY 0: Normal operation, 1:STANDBY
                {0x0207, 0x10}, // BIT[0] VREVERSE, BIT[1] VREVERSE, 0:normal 1:Inverted
                                // BIT[5:4] Readout mode designation
                                // 0: All pixel scan mode
                                // 1: 720p mode
                                // 2: Window cropping from All pixel scan mode
                                // 3: Window cropping from A720p mode
                {0x0209, 0x01}, // BIT[1:0] FRSEL 2: 30fps 1: 60fps 0/3: setting prohibited
                {0x020A, 0xF0}, // BLKLEVEL_L=BLKLEVEL AD10bit=60d = 0x3C AD12bit=240d = 0xF0
                {0x020B, 0x00}, // BLKLEVEL_H=BLKLEVEL Black level offset value setting
                {0x021B, 0x78}, // HMAX_L   3960 = 0x0F78
                {0x021C, 0x0F}, // HMAX_H
                {0x0238, 0x00}, // WINPV_L
                {0x0239, 0x00}, // WINPV_H
                {0x023A, 0xD9}, // WINWV_L   729 = 0x02D9 = 4+720+5
                {0x023B, 0x02}, // WINWV_H
                {0x023C, 0x00}, // WINPH_L
                {0x023D, 0x00}, // WINPH_H
                {0x024E, 0x20}, // WINWH_L  1312 = 0x0520 = 4+4+8+1280+8+5+3
                {0x024F, 0x05}, // WINWH_H
                {0x0200, 0x00, 0x01}, // 0x3000 BIT[0]STANDBY 0: Normal operation, 1:STANDBY
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 1280,
                .height     = 720,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_DVP, 
                .bits       = GADI_VIDEO_BITS_12,
                .ratio      = GADI_VIDEO_RATIO_16_9,
                .srm        = 0,
                .pll_index  = 2,
                .max_fps    = GADI_VIDEO_FPS(50),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(50),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_720P_50,
            },
            .video_info_table =
            {
                .def_start_x    = (4 + 4 + 8) + 40, // use emb_sync
                .def_start_y    = (7 + 1 + 4 + 4 + 2 + 4),
                .def_width      = 1280,
                .def_height     = 720,
                .sync_start     = (0 + 0),
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_GB,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                },
            },
            .update_after_vsync_start   = 0,
            .update_after_vsync_end     = 10,
        },
        [4] = // 1280x720P@30fps    (6600 / 4) * 750 * 30 = 37125000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x0200, 0x01, 0x01}, // 0x3000 BIT[0]STANDBY 0: Normal operation, 1:STANDBY
                {0x0207, 0x10}, // BIT[0] VREVERSE, BIT[1] VREVERSE, 0:normal 1:Inverted
                                // BIT[5:4] Readout mode designation
                                // 0: All pixel scan mode
                                // 1: 720p mode
                                // 2: Window cropping from All pixel scan mode
                                // 3: Window cropping from A720p mode
                {0x0209, 0x02}, // BIT[1:0] FRSEL 2: 30fps 1: 60fps 0/3: setting prohibited
                {0x020A, 0xF0}, // BLKLEVEL_L=BLKLEVEL AD10bit=60d = 0x3C AD12bit=240d = 0xF0
                {0x020B, 0x00}, // BLKLEVEL_H=BLKLEVEL Black level offset value setting
                {0x021B, 0xC8}, // HMAX_L   6600 = 0x19C8
                {0x021C, 0x19}, // HMAX_H
                {0x0238, 0x00}, // WINPV_L
                {0x0239, 0x00}, // WINPV_H
                {0x023A, 0xD9}, // WINWV_L   729 = 0x02D9 = 4+720+5
                {0x023B, 0x02}, // WINWV_H
                {0x023C, 0x00}, // WINPH_L
                {0x023D, 0x00}, // WINPH_H
                {0x024E, 0x20}, // WINWH_L  1312 = 0x0520 = 4+4+8+1280+8+5+3
                {0x024F, 0x05}, // WINWH_H
                {0x0200, 0x00, 0x01}, // 0x3000 BIT[0]STANDBY 0: Normal operation, 1:STANDBY
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 1280,
                .height     = 720,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_DVP, 
                .bits       = GADI_VIDEO_BITS_12,
                .ratio      = GADI_VIDEO_RATIO_16_9,
                .srm        = 0,
                .pll_index  = 2,
                .max_fps    = GADI_VIDEO_FPS(30),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(30),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_720P_30,
            },
            .video_info_table =
            {
                .def_start_x    = (4 + 4 + 8) + 40, // use emb_sync
                .def_start_y    = (7 + 1 + 4 + 4 + 2 + 4),
                .def_width      = 1280,
                .def_height     = 720,
                .sync_start     = (0 + 0),
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_GB,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                },
            },
            .update_after_vsync_start   = 0,
            .update_after_vsync_end     = 10,
        },
        [5] = // 1280x720P@25fps    (7920 / 4) * 750 * 25 = 37125000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  25, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x0207, 0x10}, // BIT[0] VREVERSE, BIT[1] VREVERSE, 0:normal 1:Inverted
                                // BIT[5:4] Readout mode designation
                                // 0: All pixel scan mode
                                // 1: 720p mode
                                // 2: Window cropping from All pixel scan mode
                                // 3: Window cropping from A720p mode
                {0x0209, 0x02}, // BIT[1:0] FRSEL 2: 30fps 1: 60fps 0/3: setting prohibited
                {0x020A, 0xF0}, // BLKLEVEL_L=BLKLEVEL AD10bit=60d = 0x3C AD12bit=240d = 0xF0
                {0x020B, 0x00}, // BLKLEVEL_H=BLKLEVEL Black level offset value setting
                {0x021B, 0xF0}, // HMAX_L   7920 = 0x1EF0
                {0x021C, 0x1E}, // HMAX_H
                {0x0238, 0x00}, // WINPV_L
                {0x0239, 0x00}, // WINPV_H
                {0x023A, 0xD9}, // WINWV_L   729 = 0x02D9 = 4+720+5
                {0x023B, 0x02}, // WINWV_H
                {0x023C, 0x00}, // WINPH_L
                {0x023D, 0x00}, // WINPH_H
                {0x024E, 0x20}, // WINWH_L  1312 = 0x0520 = 4+4+8+1280+8+5+3
                {0x024F, 0x05}, // WINWH_H
                {0x0200, 0x00, 0x01}, // 0x3000 BIT[0]STANDBY 0: Normal operation, 1:STANDBY
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 1280,
                .height     = 720,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_DVP, 
                .bits       = GADI_VIDEO_BITS_12,
                .ratio      = GADI_VIDEO_RATIO_16_9,
                .srm        = 0,
                .pll_index  = 2,
                .max_fps    = GADI_VIDEO_FPS(25),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(25),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_720P_25,
            },
            .video_info_table =
            {
                .def_start_x    = (4 + 4 + 8) + 40, // use emb_sync
                .def_start_y    = (7 + 1 + 4 + 4 + 2 + 4),
                .def_width      = 1280,
                .def_height     = 720,
                .sync_start     = (0 + 0),
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_GB,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                },
            },
            .update_after_vsync_start   = 0,
            .update_after_vsync_end     = 10,
        },
    },
#define IMX238_H_MIRROR     (1<<1)
#define IMX238_V_FLIP       (1<<0)
    .mirror_table =
    {
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY_VERTICALLY] =
        {
            {0x0207, IMX238_H_MIRROR | IMX238_V_FLIP, IMX238_H_MIRROR | IMX238_V_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY] =
        {
            {0x0207, IMX238_H_MIRROR, IMX238_H_MIRROR | IMX238_V_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_VERTICALLY] =
        {
            {0x0207, IMX238_V_FLIP, IMX238_H_MIRROR | IMX238_V_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_NONE] =
        {
            {0x0207, 0x00, IMX238_H_MIRROR | IMX238_V_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
    },
    .version_reg =
    {
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .reg_ctl_by_ver_num = 0,
    .control_by_version_reg = {0},
    .vmax_clk_ref   = GADI_SENSOR_CAL_CLK_PIXCLK_X2,
    .shs_clk_ref    = GADI_SENSOR_CAL_CLK_PIXCLK_X2,
    .cal_shs_mode   = GADI_SENSOR_CAL_SHS_BLK_MODE,
    .shs_fix        = 0,
    .shs_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_SET,
    .agc_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_SET,
    .vmax_reg_update_mode       = GADI_SENSOR_UPDATE_REG_BY_SET,
    .mirror_reg_update_mode     = GADI_SENSOR_UPDATE_REG_BY_SET,
    .shs_additional_use         = 0,
    .shs_additional_percent     = 0,
    // hmax = (HMAX_MSB & 0xFF) << 8 +
    //        (HMAX_LSB & 0xFF) << 0
    .hmax_reg =
    {
        {0x021C, 0x00, 0xFF, 0x00, 0x08, 0}, // HMAX_MSB
        {0x021B, 0x00, 0xFF, 0x00, 0x00, 0}, // HMAX_LSB
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    // vmax = (VMAX_MSB & 0xFF) << 8 +
    //        (VMAX_LSB & 0xFF) << 0
    .vmax_reg =
    {
        {0x0219, 0x00, 0xFF, 0x00, 0x08, 0}, // VMAX_MSB
        {0x0218, 0x00, 0xFF, 0x00, 0x00, 0}, // VMAX_LSB
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .vmax_wadd_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    // shs = (INTEG_TIME_H & 0xFF) << 8 +
    //       (INTEG_TIME_L & 0xFF) << 0
    .shs_reg =
    {
        {0x0221, 0x00, 0xFF, 0x00, 0x08, 0}, // INTEG_TIME_H
        {0x0220, 0x00, 0xFF, 0x00, 0x00, 0}, // INTEG_TIME_L
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .shs_additional_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    .cal_gain_mode              = GADI_SENSOR_CAL_GAIN_MODE0,
    .max_agc_index = 240,
    .max_again_index = 0,
    .cur_again_index = 0,
    .agc_info =
    {
        .db_max     = 0x30000000,    // 48dB
        .db_min     = 0x00000000,    // 0dB
        .db_step    = 0x004CCCCD,    // 0.3dB
    },
    .gain_reg =
    {
        {0x0214, 0x00}, // -6dB to 42dB /0.3dB step
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .gain_table =
    {
        {0},
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
#endif

