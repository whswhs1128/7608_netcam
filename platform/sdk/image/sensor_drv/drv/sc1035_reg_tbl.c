/*!
*****************************************************************************
** \file        sc1035_reg_tbl.c
**
** \version     $Id: sc1035_reg_tbl.c 11627 2017-03-10 10:58:12Z dengbiao $
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
GADI_VI_SensorDrvInfoT    sc1035_sensor =
{
    .magic_start        = GADI_SENSOR_HW_INFO_MAGIC_START,
    .version            = GADI_SENSOR_HW_INFO_VERSION,
    .length             = sizeof(GADI_VI_SensorDrvInfoT),
    .file_date          = 0x20161014,
    .HwInfo         =
    {
        .name               = "sc1035",
        .ctl_type           = GADI_SENSOR_CTL_I2C,
        .addr_len           = sizeof(GADI_U16),
        .data_len           = sizeof(GADI_U8),
        .reset_active_level = GADI_GPIO_LOW,
        .hw_addr            = {(0x60>>1), 0, 0, 0},
        .id_reg =
        {
            {0x3107, 0x00},
            {0x3108, 0x10},
        },
        .reset_reg =
        {
            {0x3000, 0x01}, // manual stream enbale
            {0x3003, 0x01}, // soft reset
            {GADI_VI_SENSOR_TABLE_FLAG_MSLEEP, 0x14},   // msleep(20);
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
    .sensor_id          = GADI_ISP_SENSOR_SC1035,
    .sensor_double_step = 32,
    .typeofsensor       = 2,
    .sync_time_delay    = 40,
    .source_type.dummy  = GADI_VI_CMOS_CHANNEL_TYPE_AUTO,
    .dev_type           = GADI_VI_DEV_TYPE_CMOS,
    .video_system       = GADI_VIDEO_SYSTEM_AUTO,
    .vs_polarity        = GADI_VI_RISING_EDGE,
    .hs_polarity        = GADI_VI_RISING_EDGE,
    .data_edge          = GADI_VI_RISING_EDGE,
    .emb_sync_switch    = GADI_VI_EMB_SYNC_OFF,
    .emb_sync_loc       = GADI_VI_EMB_SYNC_LOWER_PEL,
    .emb_sync_mode      = GADI_VI_EMB_SYNC_ITU_656,
    .max_width          = 1800,
    .max_height         = 1000,
    .def_sh_time        = GADI_VIDEO_FPS(50),
    .fmt_num            = 6,
    .auto_fmt           = GADI_VIDEO_MODE(1280,  960,  25, 1),
    .init_reg           =
    {
        {0x3400, 0x53},
        {0x3416, 0xc0},
        {0x3D08, 0x00},
        {0x5000, 0x00},
        {0x3E03, 0x03},
        {0x3928, 0x00},
        {0x3630, 0x58},
        {0x3612, 0x00},
        {0x3632, 0x41},
        {0x3635, 0x04},
        {0x3500, 0x10},
        {0x3631, 0x82},
        {0x3620, 0x44},
        {0x3633, 0x7C},
        {0x3780, 0x0B},
        {0x3300, 0x33},
        {0x3301, 0x38},
        {0x3302, 0x30},
        {0x3303, 0x66},
        {0x3304, 0xA0},
        {0x3305, 0x72},
        {0x331E, 0x56},
        {0x321E, 0x00},
        {0x321F, 0x0A},
        {0x3216, 0x0A},
        {0x3115, 0x0A},
        {0x3332, 0x38},
        {0x5054, 0x82},
        {0x3622, 0x26},
        {0x3907, 0x02},
        {0x3908, 0x00},
        {0x3601, 0x18},
        {0x3315, 0x44},
        {0x3308, 0x40},
        {0x3223, 0x22},
        {0x3E0E, 0x50},
        {0x3E0F, 0x10}, //  close dgain
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .pll_table =
    {
        [0] =
        {
            // for 960P@30, 960P@25
            .pixclk = 54000000, // pixclk = (extclk * (64 - PLLDIV[5:0])) / (PreDIV[2:0] * (SYSEL[2:0] + 1) * [0x3004])
            .extclk = 27000000, //        = 27 * (64 - 40) / (3 * (0 + 1) * 4) = 54MHz
            .regs =
            {
                {0x3010, 0x07}, // 0 000 011 1 SYSEL[2:0] = b000, PreDIV[2:0] = b011 3x, PLLDIV[5] = b1
                {0x3011, 0x46}, // 01000 110 PLLDIV[4:0] = b01000
                {0x3004, 0x04},
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
        [1] =
        {
            // for 720P@60
            .pixclk = 81000000, // pixclk = (extclk * (64 - PLLDIV[5:0])) / (PreDIV[2:0] * (SYSEL[2:0] + 1))
            .extclk = 27000000, //        = 27 * (64 - 40) / (2 * (0 + 1) * 4) = 81MHz
            .regs =
            {
                {0x3010, 0x05}, // 0 000 010 1 SYSEL[2:0] = b000, PreDIV[2:0] = b010 2x,PLLDIV[5] = b1
                {0x3011, 0x46}, // 01000 110 PLLDIV[4:0] = b01000
                {0x3004, 0x04},
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
        [2] =
        {
            // for 720P@50
            .pixclk = 74250000, // pixclk = (extclk * (64 - PLLDIV[5:0])) / (PreDIV[2:0] * (SYSEL[2:0] + 1))
            .extclk = 27000000, //        = 27 * (64 - 42) / (2 * (0 + 1) * 4) = 74.25MHz
            .regs =
            {
                {0x3010, 0x05}, // 0 000 010 1 SYSEL[2:0] = b000, PreDIV[2:0] = b010 2x,PLLDIV[5] = b1
                {0x3011, 0x56}, // 01010 110 PLLDIV[4:0] = b01010
                {0x3004, 0x04},
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
        [3] =
        {
            // for 720P@30
            .pixclk = 40500000, // pixclk = (extclk * (64 - PLLDIV[5:0])) / (PreDIV[2:0] * (SYSEL[2:0] + 1))
            .extclk = 27000000, //        = 27 * (64 - 52) / (1 * (0 + 1) * 8) = 40.5MHz
            .regs =
            {
                {0x3010, 0x01}, // 0 000 000 1 SYSEL[2:0] = b000, PreDIV[2:0] = b000 1x,PLLDIV[5] = b1
                {0x3011, 0xA6}, // 10100 110 PLLDIV[4:0] = b10100
                {0x3004, 0x08},
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
        [4] =
        {
            // for 720P@25
            .pixclk = 37125000, // pixclk = (extclk * (64 - PLLDIV[5:0])) / (PreDIV[2:0] * (SYSEL[2:0] + 1))
            .extclk = 27000000, //        = 27 * (64 - 53) / (1 * (0 + 1) * 8) = 37.125MHz
            .regs =
            {
                {0x3010, 0x01}, // 0 000 000 1 SYSEL[2:0] = b000, PreDIV[2:0] = b000 1x,PLLDIV[5] = b1
                {0x3011, 0xAE}, // 10101 110 PLLDIV[4:0] = b10101
                {0x3004, 0x08},
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
    },
    .video_fmt_table =
    {
        [0] = // 1280x960P@30fps    1800 * 1000 * 30 = 54000000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  960,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                // config Frame length and width
                {0x320C,    0x07},  // SC1035_TIMING_HTS_H      1800 = 0x0708
                {0x320D,    0x08},  // SC1035_TIMING_HTS_L
                {0x320E,    0x03},  // SC1035_TIMING_VTS_H      1000 = 0x03E8
                {0x320F,    0xE8},  // SC1035_TIMING_VTS_L
                // config Output window position
                {0x3210,    0x00},  // SC1035_TIMING_HOFFS_H
                {0x3211,    0x62},  // SC1035_TIMING_HOFFS_L
                {0x3212,    0x00},  // SC1035_TIMING_VOFFS_H    968 - 8 = 960
                {0x3213,    0x08},  // SC1035_TIMING_VOFFS_L
                // config Output image size
                {0x3208,    0x05},  // SC1035_TIMING_X_SIZE_H   1280 = 0x0500
                {0x3209,    0x00},  // SC1035_TIMING_X_SIZE_L
                {0x320A,    0x03},  // SC1035_TIMING_Y_SIZE_H    960 = 0x03C0
                {0x320B,    0xC0},  // SC1035_TIMING_Y_SIZE_L
                // config Frame start physical position
                {0x3202,    0x00},  // SC1035_TIMING_Y_START_H  0x3D0 - 0x8 = 0x3C8 = 968
                {0x3203,    0x08},  // SC1035_TIMING_Y_START_L
                {0x3206,    0x03},  // SC1035_TIMING_Y_END_H
                {0x3207,    0xD0},  // SC1035_TIMING_Y_END_L
                // power consumption reduction
                {0x3330,    0x0D},  // SC1035_3330
                {0x3320,    0x06},  // SC1035_3320
                {0x3321,    0xE8},  // SC1035_3321
                {0x3322,    0x01},  // SC1035_3322
                {0x3323,    0xC0},  // SC1035_3323
                {0x3610,    0x03},  // SC1035_3610
                {0x3600,    0x54},  // SC1035_3600
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
                .pll_index  = 0,
                .max_fps    = GADI_VIDEO_FPS(30),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(30),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_960P_30,
            },
            .update_after_vsync_start   = 3,
            .update_after_vsync_end     = 10,
            .video_info_table =
            {
                .def_start_x    = 0,
                .def_start_y    = 0,
                .def_width      = 1280,
                .def_height     = 960,
                .sync_start     = (0 + 0),
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_GB,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_BG,
                },
            },
            .update_after_vsync_start   = 3,
            .update_after_vsync_end     = 10,
        },
        [1] = // 1280x960P@25fps    2160 * 1000 * 25 = 54000000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  960,  25, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                // config Frame length and width
                {0x320C,    0x07},  // SC1035_TIMING_HTS_H      2160 = 0x0870
                {0x320D,    0x08},  // SC1035_TIMING_HTS_L
                {0x320E,    0x03},  // SC1035_TIMING_VTS_H      1000 = 0x03E8
                {0x320F,    0xE8},  // SC1035_TIMING_VTS_L
                // config Output window position
                {0x3210,    0x00},  // SC1035_TIMING_HOFFS_H
                {0x3211,    0x62},  // SC1035_TIMING_HOFFS_L
                {0x3212,    0x00},  // SC1035_TIMING_VOFFS_H
                {0x3213,    0x08},  // SC1035_TIMING_VOFFS_L    968 - 8 = 960
                // config Output image size
                {0x3208,    0x05},  // SC1035_TIMING_X_SIZE_H   1280 = 0x0500
                {0x3209,    0x00},  // SC1035_TIMING_X_SIZE_L
                {0x320A,    0x03},  // SC1035_TIMING_Y_SIZE_H    960 = 0x03C0
                {0x320B,    0xC0},  // SC1035_TIMING_Y_SIZE_L
                // config Frame start physical position
                {0x3202,    0x00},  // SC1035_TIMING_Y_START_H  0x3D0 - 0x8 = 0x3C8 = 968
                {0x3203,    0x08},  // SC1035_TIMING_Y_START_L
                {0x3206,    0x03},  // SC1035_TIMING_Y_END_H
                {0x3207,    0xD0},  // SC1035_TIMING_Y_END_L
                // power consumption reduction
                {0x3330,    0x0D},  // SC1035_3330
                {0x3320,    0x06},  // SC1035_3320
                {0x3321,    0xE8},  // SC1035_3321
                {0x3322,    0x01},  // SC1035_3322
                {0x3323,    0xC0},  // SC1035_3323
                {0x3610,    0x03},  // SC1035_3610
                {0x3600,    0x54},  // SC1035_3600
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
                .pll_index  = 0,
                .max_fps    = GADI_VIDEO_FPS(25),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(25),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_960P_25,
            },
            .video_info_table =
            {
                .def_start_x    = 0,
                .def_start_y    = 0,
                .def_width      = 1280,
                .def_height     = 960,
                .sync_start     = (0 + 0),
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_GB,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_BG,
                },
            },
            .update_after_vsync_start   = 3,
            .update_after_vsync_end     = 10,
        },
        [2] = // 1280x720P@60fps    1800 * 750 * 60 = 81000000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  60, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                // config Frame length and width
                {0x320C, 0x07},  // SC1035_TIMING_HTS_H         1800 = 0x0708
                {0x320D, 0x08},  // SC1035_TIMING_HTS_L
                {0x320E, 0x02},  // SC1035_TIMING_VTS_H          750 = 0x02EE
                {0x320F, 0xEE},  // SC1035_TIMING_VTS_L
                // config Output window position
                {0x3210, 0x00},  // SC1035_TIMING_HOFFS_H
                {0x3211, 0x62},  // SC1035_TIMING_HOFFS_L
                {0x3212, 0x00},  // SC1035_TIMING_VOFFS_H        728 - 8 = 720
                {0x3213, 0x08},  // SC1035_TIMING_VOFFS_L
                // config Output image size
                {0x3208, 0x05},  // SC1035_TIMING_X_SIZE_H      1280 = 0x0500
                {0x3209, 0x00},  // SC1035_TIMING_X_SIZE_L
                {0x320A, 0x02},  // SC1035_TIMING_Y_SIZE_H       720 = 0x02D0
                {0x320B, 0xD0},  // SC1035_TIMING_Y_SIZE_L
                // config Frame start physical position
                {0x3202, 0x00},  // SC1035_TIMING_Y_START_H     0x350 - 0x78 = 0x2D8 = 728
                {0x3203, 0x78},  // SC1035_TIMING_Y_START_L
                {0x3206, 0x03},  // SC1035_TIMING_Y_END_H
                {0x3207, 0x50},  // SC1035_TIMING_Y_END_L
                // power consumption reduction
                {0x3330, 0x0D},  // SC1035_3330
                {0x3320, 0x06},  // SC1035_3320
                {0x3321, 0xE8},  // SC1035_3321
                {0x3322, 0x01},  // SC1035_3322
                {0x3323, 0xC0},  // SC1035_3323
                {0x3610, 0x03},  // SC1035_3610
                {0x3600, 0x7C},  // SC1035_3600
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
                .pll_index  = 1,
                .max_fps    = GADI_VIDEO_FPS(60),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(60),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_720P_60,
            },
            .video_info_table =
            {
                .def_start_x    = 0,
                .def_start_y    = 0,
                .def_width      = 1280,
                .def_height     = 720,
                .sync_start     = (0 + 0),
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_GB,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_BG,
                },
            },
            .update_after_vsync_start   = 3,
            .update_after_vsync_end     = 10,
        },
        [3] = // 1280x720P@50fps    1980 * 750 * 50 = 74250000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  50, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                // config Frame length and width
                {0x320C, 0x07},  // SC1035_TIMING_HTS_H         1980 = 0x07BC
                {0x320D, 0xBC},  // SC1035_TIMING_HTS_L
                {0x320E, 0x02},  // SC1035_TIMING_VTS_H          750 = 0x02EE
                {0x320F, 0xEE},  // SC1035_TIMING_VTS_L
                // config Output window position
                {0x3210, 0x00},  // SC1035_TIMING_HOFFS_H
                {0x3211, 0x62},  // SC1035_TIMING_HOFFS_L
                {0x3212, 0x00},  // SC1035_TIMING_VOFFS_H       728 - 8 = 720
                {0x3213, 0x08},  // SC1035_TIMING_VOFFS_L
                // config Output image size
                {0x3208, 0x05},  // SC1035_TIMING_X_SIZE_H      1280 = 0x0500
                {0x3209, 0x00},  // SC1035_TIMING_X_SIZE_L
                {0x320A, 0x02},  // SC1035_TIMING_Y_SIZE_H       720 = 0x02D0
                {0x320B, 0xD0},  // SC1035_TIMING_Y_SIZE_L
                // config Frame start physical position
                {0x3202, 0x00},  // SC1035_TIMING_Y_START_H     0x350 - 0x78 = 0x2D8 = 728
                {0x3203, 0x78},  // SC1035_TIMING_Y_START_L
                {0x3206, 0x03},  // SC1035_TIMING_Y_END_H
                {0x3207, 0x50},  // SC1035_TIMING_Y_END_L
                // power consumption reduction
                {0x3330, 0x0D},  // SC1035_3330
                {0x3320, 0x06},  // SC1035_3320
                {0x3321, 0xE8},  // SC1035_3321
                {0x3322, 0x01},  // SC1035_3322
                {0x3323, 0xC0},  // SC1035_3323
                {0x3610, 0x03},  // SC1035_3610
                {0x3600, 0x7C},  // SC1035_3600
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
                .def_start_x    = 0,
                .def_start_y    = 0,
                .def_width      = 1280,
                .def_height     = 720,
                .sync_start     = (0 + 0),
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_GB,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_BG,
                },
            },
            .update_after_vsync_start   = 3,
            .update_after_vsync_end     = 10,
        },
        [4] = // 1280x720P@30fps    1800 * 750 * 30 = 40500000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                // config Frame length and width
                {0x320C, 0x07},  // SC1035_TIMING_HTS_H         1800 = 0x0708
                {0x320D, 0x08},  // SC1035_TIMING_HTS_L
                {0x320E, 0x02},  // SC1035_TIMING_VTS_H          750 = 0x02EE
                {0x320F, 0xEE},  // SC1035_TIMING_VTS_L
                // config Output window position
                {0x3210, 0x00},  // SC1035_TIMING_HOFFS_H
                {0x3211, 0x62},  // SC1035_TIMING_HOFFS_L
                {0x3212, 0x00},  // SC1035_TIMING_VOFFS_H       728 - 8 = 720
                {0x3213, 0x08},  // SC1035_TIMING_VOFFS_L
                // config Output image size
                {0x3208, 0x05},  // SC1035_TIMING_X_SIZE_H      1280 = 0x0500
                {0x3209, 0x00},  // SC1035_TIMING_X_SIZE_L
                {0x320A, 0x02},  // SC1035_TIMING_Y_SIZE_H       720 = 0x02D0
                {0x320B, 0xD0},  // SC1035_TIMING_Y_SIZE_L
                // config Frame start physical position
                {0x3202, 0x00},  // SC1035_TIMING_Y_START_H     0x350 - 0x78 = 0x2D8 = 728
                {0x3203, 0x78},  // SC1035_TIMING_Y_START_L
                {0x3206, 0x03},  // SC1035_TIMING_Y_END_H
                {0x3207, 0x50},  // SC1035_TIMING_Y_END_L
                // power consumption reduction
                {0x3330, 0x0D},  // SC1035_3330
                {0x3320, 0x04},  // SC1035_3320 <=0x05 ??
                {0x3321, 0xE8},  // SC1035_3321
                {0x3322, 0x01},  // SC1035_3322
                {0x3323, 0xC0},  // SC1035_3323
                {0x3610, 0x03},  // SC1035_3610
                {0x3600, 0x7C},  // SC1035_3600
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
                .pll_index  = 3,
                .max_fps    = GADI_VIDEO_FPS(30),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(30),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_720P_30,
            },
            .video_info_table =
            {
                .def_start_x    = 0,
                .def_start_y    = 0,
                .def_width      = 1280,
                .def_height     = 720,
                .sync_start     = (0 + 0),
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_GB,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_BG,
                },
            },
            .update_after_vsync_start   = 3,
            .update_after_vsync_end     = 10,
        },
        [5] = // 1280x720P@25fps    1980 * 750 * 25 = 37125000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  25, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                // config Frame length and width
                {0x320C, 0x07},  // SC1035_TIMING_HTS_H         1980 = 0x07BC
                {0x320D, 0xBC},  // SC1035_TIMING_HTS_L
                {0x320E, 0x02},  // SC1035_TIMING_VTS_H          750 = 0x02EE
                {0x320F, 0xEE},  // SC1035_TIMING_VTS_L
                // config Output window position
                {0x3210, 0x00},  // SC1035_TIMING_HOFFS_H
                {0x3211, 0x62},  // SC1035_TIMING_HOFFS_L
                {0x3212, 0x00},  // SC1035_TIMING_VOFFS_H       728 - 8 = 720
                {0x3213, 0x08},  // SC1035_TIMING_VOFFS_L
                // config Output image size
                {0x3208, 0x05},  // SC1035_TIMING_X_SIZE_H      1280 = 0x0500
                {0x3209, 0x00},  // SC1035_TIMING_X_SIZE_L
                {0x320A, 0x02},  // SC1035_TIMING_Y_SIZE_H       720 = 0x02D0
                {0x320B, 0xD0},  // SC1035_TIMING_Y_SIZE_L
                // config Frame start physical position
                {0x3202, 0x00},  // SC1035_TIMING_Y_START_H     0x350 - 0x78 = 0x2D8 = 728
                {0x3203, 0x78},  // SC1035_TIMING_Y_START_L
                {0x3206, 0x03},  // SC1035_TIMING_Y_END_H
                {0x3207, 0x50},  // SC1035_TIMING_Y_END_L
                // power consumption reduction
                {0x3330, 0x0D},  // SC1035_3330
                {0x3320, 0x06},  // SC1035_3320
                {0x3321, 0xE8},  // SC1035_3321
                {0x3322, 0x01},  // SC1035_3322
                {0x3323, 0xC0},  // SC1035_3323
                {0x3610, 0x03},  // SC1035_3610
                {0x3600, 0x7C},  // SC1035_3600
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
                .pll_index  = 4,
                .max_fps    = GADI_VIDEO_FPS(25),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(25),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_720P_25,
            },
            .video_info_table =
            {
                .def_start_x    = 0,
                .def_start_y    = 0,
                .def_width      = 1280,
                .def_height     = 720,
                .sync_start     = (0 + 0),
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_GB,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_BG,
                },
            },
            .update_after_vsync_start   = 3,
            .update_after_vsync_end     = 10,
        },
    },
    .mirror_table =
    {
#define SC1035_FLIP     0x40
#define SC1035_MIRROR   0x01
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY_VERTICALLY] =
        {
            {0x3400, 0x52},         // SC1035_RNC_ENABLE
            {0x3907, 0x00},         // SC1035_BLC_TARGET_H
            {0x3908, 0xC0},         // SC1035_BLC_TARGET_L
            {0x3781, 0x08},         // SC1035_BLC_SYNC01
            {0x321D, SC1035_MIRROR, SC1035_MIRROR}, // SC1035_MIRROR
            {0x321C, SC1035_FLIP, SC1035_FLIP},     // SC1035_FLIP
            {0x3211, 0x10},         // SC1035_TIMING_HOFFS_L
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY] =
        {
            {0x3400, 0x52},         // SC1035_RNC_ENABLE
            {0x3907, 0x00},         // SC1035_BLC_TARGET_H
            {0x3908, 0xC0},         // SC1035_BLC_TARGET_L
            {0x3781, 0x08},         // SC1035_BLC_SYNC01
            {0x321D, SC1035_MIRROR, SC1035_MIRROR}, // SC1035_MIRROR
            {0x321C, 0x00, SC1035_FLIP},            // SC1035_FLIP
            {0x3211, 0x10},         // SC1035_TIMING_HOFFS_L
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_VERTICALLY] =
        {
            {0x3400, 0x53},         // SC1035_RNC_ENABLE
            {0x3907, 0x02},         // SC1035_BLC_TARGET_H
            {0x3908, 0x00},         // SC1035_BLC_TARGET_L
            {0x3781, 0x10},         // SC1035_BLC_SYNC01
            {0x321D, 0x00, SC1035_MIRROR},          // SC1035_MIRROR
            {0x321C, SC1035_FLIP, SC1035_FLIP},     // SC1035_FLIP
            {0x3211, 0x62},         // SC1035_TIMING_HOFFS_L
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_NONE] =
        {
            {0x3400, 0x53},         // SC1035_RNC_ENABLE
            {0x3907, 0x02},         // SC1035_BLC_TARGET_H
            {0x3908, 0x00},         // SC1035_BLC_TARGET_L
            {0x3781, 0x10},         // SC1035_BLC_SYNC01
            {0x321D, 0x00, SC1035_MIRROR},          // SC1035_MIRROR
            {0x321C, 0x00, SC1035_FLIP},            // SC1035_FLIP
            {0x3211, 0x62},         // SC1035_TIMING_HOFFS_L
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
    .shs_fix        = 4,
    .shs_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_VSYNC,
    .agc_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_VSYNC,
    .vmax_reg_update_mode       = GADI_SENSOR_UPDATE_REG_BY_SET,
    .mirror_reg_update_mode     = GADI_SENSOR_UPDATE_REG_BY_SET,
    .shs_additional_use         = 1,
    .shs_additional_percent     = 70,
    // hmax = (SC1035_TIMING_HTS_H & 0xFF) << 8 +
    //        (SC1035_TIMING_HTS_L & 0xFF) << 0
    .hmax_reg =
    {
        {0x320C, 0x00, 0xFF, 0x00, 0x08, 0}, // SC1035_TIMING_HTS_H
        {0x320D, 0x00, 0xFF, 0x00, 0x00, 0}, // SC1035_TIMING_HTS_L
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    // vmax = (SC1035_TIMING_VTS_H & 0xFF) << 8 +
    //        (SC1035_TIMING_VTS_L & 0xFF) << 0
    .vmax_reg =
    {
        {0x320E, 0x00, 0xFF, 0x00, 0x08, 0}, // SC1035_TIMING_VTS_H
        {0x320F, 0x00, 0xFF, 0x00, 0x00, 0}, // SC1035_TIMING_VTS_L
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .vmax_wadd_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    // shs = (SC1035_AEC_EXP_TIME_H & 0xFF) << 4 +
    //       (SC1035_AEC_EXP_TIME_L & 0xF0) >> 4
    .shs_reg =
    {
        {0x3E01, 0x00, 0xFF, 0x00, 0x04, 0}, // SC1035_AEC_EXP_TIME_H
        {0x3E02, 0x00, 0xF0, 0x01, 0x04, 0}, // SC1035_AEC_EXP_TIME_L
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .shs_additional_reg = {0x331E, 0x00, 0x00},
    .cal_gain_mode              = GADI_SENSOR_CAL_GAIN_MODE1,
    .max_agc_index = 109,
    .max_again_index = 61,
    .cur_again_index = 0,
    .agc_info =
    {
        .db_max     = 0x286acd35,    // 40.417194
        .db_min     = 0x00000000,    // 0dB
        .db_step    = 0x004ccccd,    // 0.3dB
    },
    .gain_reg =
    {
        {0x3E0F, 0x00},
        {0x3E0A, 0x00},
        {0x3E09, 0x00},
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .gain_table =
    {
        /*0x3e0f[1:0] 0x3e090[7:0]  db_value*100*/
        /*  dig_gain        ana_gain        dcg_gain        fine_gain
            0x3E0F          0x3E09          0x3E09          0x3E09
            bit[1:0]        bit[6:4]        bit[7]          bit[3:0]*/
        {0x00000000, {0x14, 0x00, 0x00/*dig_again 1;ana_gain 1;dcg_gain 1; fine_gain 1.0000*/,    }},//0.000000 //1.00
        {0x006c1e35, {0x14, 0x00, 0x01/*dig_again 1;ana_gain 1;dcg_gain 1; fine_gain 1.0588*/,    }},//0.422336 //1.05
        {0x00e74279, {0x14, 0x00, 0x02/*dig_again 1;ana_gain 1;dcg_gain 1; fine_gain 1.1176*/,    }},//0.903358 //1.11
        {0x015beac7, {0x14, 0x00, 0x03/*dig_again 1;ana_gain 1;dcg_gain 1; fine_gain 1.1765*/,    }},//1.359051 //1.17
        {0x01cabd3a, {0x14, 0x00, 0x04/*dig_again 1;ana_gain 1;dcg_gain 1; fine_gain 1.2353*/,    }},//1.791950 //1.23
        {0x0234482e, {0x14, 0x00, 0x05/*dig_again 1;ana_gain 1;dcg_gain 1; fine_gain 1.2941*/,    }},//2.204226 //1.29
        {0x02a9613b, {0x14, 0x00, 0x06/*dig_again 1;ana_gain 1;dcg_gain 1; fine_gain 1.3529*/,    }},//2.661640 //1.36
        {0x02f96344, {0x14, 0x00, 0x07/*dig_again 1;ana_gain 1;dcg_gain 1; fine_gain 1.4118*/,    }},//2.974171 //1.41
        {0x0355bbce, {0x14, 0x00, 0x08/*dig_again 1;ana_gain 1;dcg_gain 1; fine_gain 1.4706*/,    }},//3.334897 //1.47
        {0x039fda82, {0x14, 0x00, 0x09/*dig_again 1;ana_gain 1;dcg_gain 1; fine_gain 1.5294*/,    }},//3.624428 //1.52
        {0x03f5a4e5, {0x14, 0x00, 0x0a/*dig_again 1;ana_gain 1;dcg_gain 1; fine_gain 1.5882*/,    }},//3.959547 //1.58
        {0x04483c9d, {0x14, 0x00, 0x0b/*dig_again 1;ana_gain 1;dcg_gain 1; fine_gain 1.6471*/,    }},//4.282175 //1.64
        {0x0497dc82, {0x14, 0x00, 0x0c/*dig_again 1;ana_gain 1;dcg_gain 1; fine_gain 1.7059*/,    }},//4.593208 //1.70
        {0x04e4b94e, {0x14, 0x00, 0x0d/*dig_again 1;ana_gain 1;dcg_gain 1; fine_gain 1.7647*/,    }},//4.893453 //1.76
        {0x052f026b, {0x14, 0x00, 0x0e/*dig_again 1;ana_gain 1;dcg_gain 1; fine_gain 1.8235*/,    }},//5.183631 //1.82
        {0x0576e2a7, {0x14, 0x00, 0x0f/*dig_again 1;ana_gain 1;dcg_gain 1; fine_gain 1.8824*/,    }},//5.464396 //1.88
        {0x06000000, {0x14, 0x00, 0x10/*dig_again 1;ana_gain 2;dcg_gain 1; fine_gain 1.0000*/,    }},//6.000000 //2
        {0x067e9cd5, {0x14, 0x00, 0x11/*dig_again 1;ana_gain 2;dcg_gain 1; fine_gain 1.0588*/,    }},//6.494581 //2.1176
        {0x06f66163, {0x14, 0x00, 0x12/*dig_again 1;ana_gain 2;dcg_gain 1; fine_gain 1.1176*/,    }},//6.962424 //2.2352
        {0x076831ab, {0x14, 0x00, 0x13/*dig_again 1;ana_gain 2;dcg_gain 1; fine_gain 1.1765*/,    }},//7.407008 //2.353
        {0x07d44466, {0x14, 0x00, 0x14/*dig_again 1;ana_gain 2;dcg_gain 1; fine_gain 1.2353*/,    }},//7.829169 //2.4706
        {0x083b5056, {0x14, 0x00, 0x15/*dig_again 1;ana_gain 2;dcg_gain 1; fine_gain 1.2941*/,    }},//8.231695 //2.5882
        {0x089dc7e2, {0x14, 0x00, 0x16/*dig_again 1;ana_gain 2;dcg_gain 1; fine_gain 1.3529*/,    }},//8.616331 //2.7058
        {0x08fc3702, {0x14, 0x00, 0x17/*dig_again 1;ana_gain 2;dcg_gain 1; fine_gain 1.4118*/,    }},//8.985214 //2.8236
        {0x0956a34e, {0x14, 0x00, 0x18/*dig_again 1;ana_gain 2;dcg_gain 1; fine_gain 1.4706*/,    }},//9.338429 //2.9412
        {0x09ad83f3, {0x14, 0x00, 0x19/*dig_again 1;ana_gain 2;dcg_gain 1; fine_gain 1.5294*/,    }},//9.677795 //3.0588
        {0x0a011d72, {0x14, 0x00, 0x1a/*dig_again 1;ana_gain 2;dcg_gain 1; fine_gain 1.5882*/,    }},//10.004356//3.1764
        {0x0a51cf44, {0x14, 0x00, 0x1b/*dig_again 1;ana_gain 2;dcg_gain 1; fine_gain 1.6471*/,    }},//10.319569//3.2942
        {0x0a9f89ef, {0x14, 0x00, 0x1c/*dig_again 1;ana_gain 2;dcg_gain 1; fine_gain 1.7059*/,    }},//10.623198//3.4118
        {0x0af1476d, {0x14, 0x00, 0x90/*dig_again 1;ana_gain 2;dcg_gain 1.77; fine_gain 1.0000*/, }},//10.942496//3.54
        {0x0b6b1118, {0x14, 0x00, 0x91/*dig_again 1;ana_gain 2;dcg_gain 1.77; fine_gain 1.0588*/, }},//11.418230//3.74
        {0x0be4202c, {0x14, 0x00, 0x92/*dig_again 1;ana_gain 2;dcg_gain 1.77; fine_gain 1.1176*/, }},//11.891116//3.95
        {0x0c56e98c, {0x14, 0x00, 0x93/*dig_again 1;ana_gain 2;dcg_gain 1.77; fine_gain 1.1765*/, }},//12.339501//4.16
        {0x0cc40b72, {0x14, 0x00, 0x94/*dig_again 1;ana_gain 2;dcg_gain 1.77; fine_gain 1.2353*/, }},//12.765800//4.37
        {0x0d2c0dcd, {0x14, 0x00, 0x95/*dig_again 1;ana_gain 2;dcg_gain 1.77; fine_gain 1.2941*/, }},//13.172086//4.58
        {0x0d8ac4af, {0x14, 0x00, 0x96/*dig_again 1;ana_gain 2;dcg_gain 1.77; fine_gain 1.3529*/, }},//13.542064//4.78
        {0x0dea0b8f, {0x14, 0x00, 0x97/*dig_again 1;ana_gain 2;dcg_gain 1.77; fine_gain 1.4118*/, }},//13.914239//4.99
        {0x0e4564d3, {0x14, 0x00, 0x98/*dig_again 1;ana_gain 2;dcg_gain 1.77; fine_gain 1.4706*/, }},//14.271070//5.2
        {0x0e9d2022, {0x14, 0x00, 0x99/*dig_again 1;ana_gain 2;dcg_gain 1.77; fine_gain 1.5294*/, }},//14.613772//7.41
        {0x0ef18406, {0x14, 0x00, 0x9a/*dig_again 1;ana_gain 2;dcg_gain 1.77; fine_gain 1.5882*/, }},//14.943421//5.62
        {0x0f42cf46, {0x14, 0x00, 0x9b/*dig_again 1;ana_gain 2;dcg_gain 1.77; fine_gain 1.6471*/, }},//15.260975//5.83
        {0x0f8d8e00, {0x14, 0x00, 0x9c/*dig_again 1;ana_gain 2;dcg_gain 1.77; fine_gain 1.7059*/, }},//15.552948//6.03
        {0x0fd96a29, {0x14, 0x00, 0x9d/*dig_again 1;ana_gain 2;dcg_gain 1.77; fine_gain 1.7647*/, }},//15.849276//6.24
        {0x1022c375, {0x14, 0x00, 0x9e/*dig_again 1;ana_gain 2;dcg_gain 1.77; fine_gain 1.8235*/, }},//16.135795//6.45
        {0x1069c316, {0x14, 0x00, 0x9f/*dig_again 1;ana_gain 2;dcg_gain 1.77; fine_gain 1.8824*/, }},//16.413133//6.66
        {0x10f1476d, {0x14, 0x00, 0xb0/*dig_again 1;ana_gain 4;dcg_gain 1.77; fine_gain 1.0000*/, }},//16.942496//7.08
        {0x116e0700, {0x14, 0x00, 0xb1/*dig_again 1;ana_gain 4;dcg_gain 1.77; fine_gain 1.0588*/, }},//17.429794//7.49
        {0x11e6edce, {0x14, 0x00, 0xb2/*dig_again 1;ana_gain 4;dcg_gain 1.77; fine_gain 1.1176*/, }},//17.902066//7.91
        {0x1256e98c, {0x14, 0x00, 0xb3/*dig_again 1;ana_gain 4;dcg_gain 1.77; fine_gain 1.1765*/, }},//18.339501//8.32
        {0x12c40b72, {0x14, 0x00, 0xb4/*dig_again 1;ana_gain 4;dcg_gain 1.77; fine_gain 1.2353*/, }},//18.765800//8.74
        {0x132c0dcd, {0x14, 0x00, 0xb5/*dig_again 1;ana_gain 4;dcg_gain 1.77; fine_gain 1.2941*/, }},//19.172086//9.16
        {0x138d15c7, {0x14, 0x00, 0xb6/*dig_again 1;ana_gain 4;dcg_gain 1.77; fine_gain 1.3529*/, }},//19.551114//9.57
        {0x13ec43b4, {0x14, 0x00, 0xb7/*dig_again 1;ana_gain 4;dcg_gain 1.77; fine_gain 1.4118*/, }},//19.922908//9.99
        {0x14478609, {0x14, 0x00, 0xb8/*dig_again 1;ana_gain 4;dcg_gain 1.77; fine_gain 1.4706*/, }},//20.279389//10.41
        {0x149d2022, {0x14, 0x00, 0xb9/*dig_again 1;ana_gain 4;dcg_gain 1.77; fine_gain 1.5294*/, }},//20.613772//10.82
        {0x14f18406, {0x14, 0x00, 0xba/*dig_again 1;ana_gain 4;dcg_gain 1.77; fine_gain 1.5882*/, }},//20.943421//11.24
        {0x1542cf46, {0x14, 0x00, 0xbb/*dig_again 1;ana_gain 4;dcg_gain 1.77; fine_gain 1.6471*/, }},//21.260975//11.66
        {0x158f6432, {0x14, 0x00, 0xbc/*dig_again 1;ana_gain 4;dcg_gain 1.77; fine_gain 1.7059*/, }},//21.560123//12.07
        {0x15db308a, {0x14, 0x00, 0xbd/*dig_again 1;ana_gain 4;dcg_gain 1.77; fine_gain 1.7647*/, }},//21.856209//12.49
        {0x16247b0c, {0x14, 0x00, 0xbe/*dig_again 1;ana_gain 4;dcg_gain 1.77; fine_gain 1.8235*/, }},//22.142503//12.91
        {0x1669c316, {0x14, 0x00, 0xbf/*dig_again 1;ana_gain 4;dcg_gain 1.77; fine_gain 1.8824*/, }},//22.413133//13.32
        {0x16f1476d, {0x14, 0x00, 0xf0/*dig_again 1;ana_gain 8;dcg_gain 1.77; fine_gain 1.0000*/, }},//22.942496//14.16
        {0x176f8192, {0x14, 0x00, 0xf1/*dig_again 1;ana_gain 8;dcg_gain 1.77; fine_gain 1.0588*/, }},//23.435571//14.99
        {0x17e6edce, {0x14, 0x00, 0xf2/*dig_again 1;ana_gain 8;dcg_gain 1.77; fine_gain 1.1176*/, }},//23.902066//15.82
        {0x18583e5d, {0x14, 0x00, 0xf3/*dig_again 1;ana_gain 8;dcg_gain 1.77; fine_gain 1.1765*/, }},//24.344702//16.65
        {0x18c54fe4, {0x14, 0x00, 0xf4/*dig_again 1;ana_gain 8;dcg_gain 1.77; fine_gain 1.2353*/, }},//24.770750//17.49
        {0x192c0dcd, {0x14, 0x00, 0xf5/*dig_again 1;ana_gain 8;dcg_gain 1.77; fine_gain 1.2941*/, }},//25.172086//18.32
        {0x198e3e17, {0x14, 0x00, 0xf6/*dig_again 1;ana_gain 8;dcg_gain 1.77; fine_gain 1.3529*/, }},//25.555635//19.15
        {0x19ed5f8f, {0x14, 0x00, 0xf7/*dig_again 1;ana_gain 8;dcg_gain 1.77; fine_gain 1.4118*/, }},//25.927239//19.99
        {0x1a478609, {0x14, 0x00, 0xf8/*dig_again 1;ana_gain 8;dcg_gain 1.77; fine_gain 1.4706*/, }},//26.279389//20.82
        {0x1a9e2638, {0x14, 0x00, 0xf9/*dig_again 1;ana_gain 8;dcg_gain 1.77; fine_gain 1.5294*/, }},//26.617771//21.65
        {0x1af18406, {0x14, 0x00, 0xfa/*dig_again 1;ana_gain 8;dcg_gain 1.77; fine_gain 1.5882*/, }},//26.943421//22.48
        {0x1b42cf46, {0x14, 0x00, 0xfb/*dig_again 1;ana_gain 8;dcg_gain 1.77; fine_gain 1.6471*/, }},//27.260975//23.32
        {0x1b904f25, {0x14, 0x00, 0xfc/*dig_again 1;ana_gain 8;dcg_gain 1.77; fine_gain 1.7059*/, }},//27.563708//24.15
        {0x1bdb308a, {0x14, 0x00, 0xfd/*dig_again 1;ana_gain 8;dcg_gain 1.77; fine_gain 1.7647*/, }},//27.856209//24.98
        {0x1c247b0c, {0x14, 0x00, 0xfe/*dig_again 1;ana_gain 8;dcg_gain 1.77; fine_gain 1.8235*/, }},//28.142503//25.82
        {0x1c6a97fe, {0x14, 0x00, 0xff/*dig_again 1;ana_gain 8;dcg_gain 1.77; fine_gain 1.8824*/, }},//28.416382//26.65
        {0x1cf1476d, {0x15, 0x01, 0xf0/*dig_again 2;ana_gain 8;dcg_gain 1.77; fine_gain 1.0000*/, }},//28.942496//28.32
        {0x1d6f8192, {0x15, 0x01, 0xf1/*dig_again 2;ana_gain 8;dcg_gain 1.77; fine_gain 1.0588*/, }},//29.435571//29.98
        {0x1de7a113, {0x15, 0x01, 0xf2/*dig_again 2;ana_gain 8;dcg_gain 1.77; fine_gain 1.1176*/, }},//29.904802//31.65
        {0x1e58e8b2, {0x15, 0x01, 0xf3/*dig_again 2;ana_gain 8;dcg_gain 1.77; fine_gain 1.1765*/, }},//30.347301//33.31
        {0x1ec54fe4, {0x15, 0x01, 0xf4/*dig_again 2;ana_gain 8;dcg_gain 1.77; fine_gain 1.2353*/, }},//30.770750//34.98
        {0x1f2c0dcd, {0x15, 0x01, 0xf5/*dig_again 2;ana_gain 8;dcg_gain 1.77; fine_gain 1.2941*/, }},//31.172086//36.64
        {0x1f8ed230, {0x15, 0x01, 0xf6/*dig_again 2;ana_gain 8;dcg_gain 1.77; fine_gain 1.3529*/, }},//31.557895//38.31
        {0x1fed5f8f, {0x15, 0x01, 0xf7/*dig_again 2;ana_gain 8;dcg_gain 1.77; fine_gain 1.4118*/, }},//31.927239//39.98
        {0x20478609, {0x15, 0x01, 0xf8/*dig_again 2;ana_gain 8;dcg_gain 1.77; fine_gain 1.4706*/, }},//32.279389//41.64
        {0x209ea938, {0x15, 0x01, 0xf9/*dig_again 2;ana_gain 8;dcg_gain 1.77; fine_gain 1.5294*/, }},//32.619770//43.31
        {0x20f2022f, {0x15, 0x01, 0xfa/*dig_again 2;ana_gain 8;dcg_gain 1.77; fine_gain 1.5882*/, }},//32.945346//44.97
        {0x2142cf46, {0x15, 0x01, 0xfb/*dig_again 2;ana_gain 8;dcg_gain 1.77; fine_gain 1.6471*/, }},//33.260975//46.64
        {0x2190c496, {0x15, 0x01, 0xfc/*dig_again 2;ana_gain 8;dcg_gain 1.77; fine_gain 1.7059*/, }},//33.565500//48.31
        {0x21dba214, {0x15, 0x01, 0xfd/*dig_again 2;ana_gain 8;dcg_gain 1.77; fine_gain 1.7647*/, }},//33.857942//49.97
        {0x22247b0c, {0x15, 0x01, 0xfe/*dig_again 2;ana_gain 8;dcg_gain 1.77; fine_gain 1.8235*/, }},//34.142503//51.64
        {0x226a97fe, {0x15, 0x01, 0xff/*dig_again 2;ana_gain 8;dcg_gain 1.77; fine_gain 1.8824*/, }},//34.416382//53.30
        {0x22f1476d, {0x17, 0x03, 0xf0/*dig_again 4;ana_gain 8;dcg_gain 1.77; fine_gain 1.0000*/, }},//34.942496//56.64
        {0x236fe02d, {0x17, 0x03, 0xf1/*dig_again 4;ana_gain 8;dcg_gain 1.77; fine_gain 1.0588*/, }},//35.437014//59.97
        {0x23e7a113, {0x17, 0x03, 0xf2/*dig_again 4;ana_gain 8;dcg_gain 1.77; fine_gain 1.1176*/, }},//35.904802//63.30
        {0x24593dd8, {0x17, 0x03, 0xf3/*dig_again 4;ana_gain 8;dcg_gain 1.77; fine_gain 1.1765*/, }},//36.348600//66.63
        {0x24c54fe4, {0x17, 0x03, 0xf4/*dig_again 4;ana_gain 8;dcg_gain 1.77; fine_gain 1.2353*/, }},//36.770750//69.96
        {0x252c5b35, {0x17, 0x03, 0xf5/*dig_again 4;ana_gain 8;dcg_gain 1.77; fine_gain 1.2941*/, }},//37.173267//73.29
        {0x258ed230, {0x17, 0x03, 0xf6/*dig_again 4;ana_gain 8;dcg_gain 1.77; fine_gain 1.3529*/, }},//37.557895//76.62
        {0x25ed5f8f, {0x17, 0x03, 0xf7/*dig_again 4;ana_gain 8;dcg_gain 1.77; fine_gain 1.4118*/, }},//37.927239//79.96
        {0x2647ca26, {0x17, 0x03, 0xf8/*dig_again 4;ana_gain 8;dcg_gain 1.77; fine_gain 1.4706*/, }},//38.280428//83.29
        {0x269ea938, {0x17, 0x03, 0xf9/*dig_again 4;ana_gain 8;dcg_gain 1.77; fine_gain 1.5294*/, }},//38.619770//86.62
        {0x26f24142, {0x17, 0x03, 0xfa/*dig_again 4;ana_gain 8;dcg_gain 1.77; fine_gain 1.5882*/, }},//38.946308//89.95
        {0x27430c17, {0x17, 0x03, 0xfb/*dig_again 4;ana_gain 8;dcg_gain 1.77; fine_gain 1.6471*/, }},//39.261903//93.29
        {0x2790c496, {0x17, 0x03, 0xfc/*dig_again 4;ana_gain 8;dcg_gain 1.77; fine_gain 1.7059*/, }},//39.565500//96.62
        {0x27dbdad6, {0x17, 0x03, 0xfd/*dig_again 4;ana_gain 8;dcg_gain 1.77; fine_gain 1.7647*/, }},//39.858808//99.95
        {0x28247b0c, {0x17, 0x03, 0xfe/*dig_again 4;ana_gain 8;dcg_gain 1.77; fine_gain 1.8235*/, }},//40.142503//103.28
        {0x286acd35, {0x17, 0x03, 0xff/*dig_again 4;ana_gain 8;dcg_gain 1.77; fine_gain 1.8824*/, }},//40.417194//106.61
    },
    .temperatue_gain_table =
    {
        .agc_control_by_temperatue      = 1,
        .Aptina_temperatue_mode         = 0,
        .temperatue_control_reg_index   = 2,
        .temperatue_gain_num            = 3,
        .temperatue_reg                 = {0x3911, 0x00, 0x00},
        .gain_reg                       =
        {
            {0x3E09,},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        .temperatue_gain                =
        {
            // 0x58 < {0x3911}
            {0x58,},
            // 0x44 < {0x3911} <= 0x58
            {0x44, {0x00,},{{.opt = GADI_GAIN_OPT_SKIP},},},
            // {0x3911} <= 0x44
            {0x43,},
        },
        .temperatue_control             =
        {
            0xff,0xfe,0xfd,0xfc,0xfb,0xfa,0xf9,0xf8,0xf7,0xf6,0xf5,0xf4,0xf3,0xf2,0xf1,0xf0,
            0xbf,0xbe,0xbd,0xbc,0xbb,0xba,0xb9,0xb8,0xb7,0xb6,0xb5,0xb4,0xb3,0xb2,0xb1,0xb0,
            0x9f,0x9e,0x9d,0x9c,0x9b,0x9a,0x99,0x98,0x97,0x96,0x95,0x94,0x93,0x92,0x91,0x90,
            0x1c,0x1b,0x1a,0x19,0x18,0x17,0x16,0x15,0x14,0x13,0x12,0x11,0x10,
        },
    },
    .magic_end  = GADI_SENSOR_HW_INFO_MAGIC_END,
};

