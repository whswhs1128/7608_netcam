/*!
*****************************************************************************
** \file        sc1135_reg_tbl.c
**
** \version     $Id: sc1135_reg_tbl.c 11627 2017-03-10 10:58:12Z dengbiao $
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
GADI_VI_SensorDrvInfoT    sc1135_sensor =
{
    .magic_start        = GADI_SENSOR_HW_INFO_MAGIC_START,
    .version            = GADI_SENSOR_HW_INFO_VERSION,
    .length             = sizeof(GADI_VI_SensorDrvInfoT),
    .file_date          = 0x20161014,
    .HwInfo         =
    {
        .name               = "sc1135",
        .ctl_type           = GADI_SENSOR_CTL_I2C,
        .addr_len           = sizeof(GADI_U16),
        .data_len           = sizeof(GADI_U8),
        .reset_active_level = GADI_GPIO_LOW,
        .hw_addr            = {(0x60>>1), 0, 0, 0},
        .id_reg =
        {
            {0x3107, 0x00},
            {0x2148, 0x35},
        },
        .reset_reg =
        {
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
    .sensor_id          = GADI_ISP_SENSOR_SC1135,
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
        {0x3000, 0x00}, // manual stream disable
        //{0x3115, 0x0A}, // def: 0x44
        {0x3216, 0x0A},
        {0x321E, 0x00},
        {0x321F, 0x0A},
        {0x3223, 0x22}, // vsync mode
        {0x3302, 0x30},
        {0x3305, 0x72},
        {0x3310, 0x83}, // prechg tx auto ctrl [5] 0825
        {0x3315, 0x44}, // bl_en all high£¬cancel column fpn
        {0x3332, 0x38},
        {0x3416, 0xC0},
        //{0x3500, 0x42}, // def: 0x00
        //{0x3600, 0x64}, // power reduct def: 0x54
        {0x3601, 0x1A},
        {0x3610, 0x03}, // sa2 current
        {0x3612, 0x00},
        {0x3622, 0x26},
        {0x3632, 0x41},
        {0x3633, 0x7F},
        {0x3635, 0x00},
        {0x3780, 0x0B},
        {0x3880, 0x00},
        {0x3928, 0x00},
        {0x3D08, 0x00},
        {0x3E03, 0x03}, // to close aec/agc :03
        {0x3E0E, 0x50}, // 12bit exp
        {0x3E0F, 0x90}, // gain[1:0] 00b=1X, 01b=2X, 11b=4X,
        {0x5000, 0x66}, // 0x00:close DPC, 0x66:open DPC
        {0x5054, 0x82},
        {0x5780, 0xFF},
        {0x5781, 0x04}, // hhl note: bright spot threshold
        {0x3000, 0x01}, // manual stream enable
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .pll_table =
    {
        // PreDIV[2:0] = 1/1.5/2/3/4/5/6/7
        // SYSEL[2:0]  = 0/1/2/3/4/5/6/7
        [0] =
        {
            // for 720P@60, 720P@50
            .pixclk = 74250000, // pixclk = (extclk * (64 - PLLDIV[5:0])) / (PreDIV[2:0] * (SYSEL[2:0] + 1) * SCLK_DIV)
            .extclk = 27000000, //        = 27 * (64 - 53) / (1 * (1 + 0) * 4) = 74.25MHz
            .regs =
            {
                {0x3010, 0x01}, // PLL_PRE_CLK_DIV 0 000 000 1 SYSEL[2:0] = b000, PreDIV[2:0] = b000 1x, PLLDIV[5] = b1
                {0x3011, 0xAE}, // PLL_SYS_CLK_DIV 10101 110 PLLDIV[4:0] = b10101
                {0x3004, 0x04}, // SCLK_DIV[3:0]
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
        [1] =
        {
             // for 960P@30, 960@25
            .pixclk = 54000000, // pixclk = (extclk * (64 - PLLDIV[5:0])) / (PreDIV[2:0] * (SYSEL[2:0] + 1) * [0x3004])
            .extclk = 27000000, //        = 27 * (64 - 40) / (3 * (0 + 1) * 4) = 54MHz
            .regs =
            {
                {0x3010, 0x07}, // PLL_PRE_CLK_DIV 0 000 011 1 SYSEL[2:0] = b000, PreDIV[2:0] = b011 3x, PLLDIV[5] = b1
                {0x3011, 0x46}, // PLL_SYS_CLK_DIV 01000 110 PLLDIV[4:0] = b01000
                {0x3004, 0x04}, // SCLK_DIV[3:0]
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
        [2] =
        {
            // for 720P@30, 720P@25
            .pixclk = 37125000, // pixclk = (extclk * (64 - PLLDIV[5:0])) / (PreDIV[2:0] * (SYSEL[2:0] + 1) * SCLK_DIV)
            .extclk = 27000000, //        = 27 * (64 - 42) / (1 * (3 + 1) * 4) = 37.125MHz
            .regs =
            {
                {0x3010, 0x01}, // PLL_PRE_CLK_DIV 0 000 000 1 SYSEL[2:0] = b000, PreDIV[2:0] = b000 1x, PLLDIV[5] = b1
                {0x3011, 0xAE}, // PLL_SYS_CLK_DIV 10101 110 PLLDIV[4:0] = b10101
                {0x3004, 0x08}, // SCLK_DIV[3:0]
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
                // config Frame start physical position
                {0x3202, 0x00}, // TIMING_Y_START_H
                {0x3203, 0x08}, // TIMING_Y_START_L
                {0x3206, 0x03}, // TIMING_Y_END_H
                {0x3207, 0xCF}, // TIMING_Y_END_L
                // config Output image size
                {0x3208, 0x05}, // TIMING_X_SIZE_H   1280 = 0x0500
                {0x3209, 0x00}, // TIMING_X_SIZE_L
                {0x320A, 0x03}, // TIMING_Y_SIZE_H    960 = 0x03C0
                {0x320B, 0xC0}, // TIMING_Y_SIZE_L
                // config Frame length and width
                {0x320C, 0x07}, // TIMING_HTS_H      1800 = 0x0708
                {0x320D, 0x08}, // TIMING_HTS_L
                {0x320E, 0x03}, // TIMING_VTS_H      1000 = 0x03E8
                {0x320F, 0xE8}, // TIMING_VTS_L
                // config Output window position
                {0x3210, 0x00}, // TIMING_HOFFS_H
                {0x3211, 0x60}, // TIMING_HOFFS_L
                {0x3212, 0x00}, // TIMING_VOFFS_H
                {0x3213, 0x04}, // TIMING_VOFFS_L
                // power consumption reduction & timing
                {0x3300, 0x33},
                {0x3301, 0x38}, // cancel aec shanshuo
                {0x3303, 0x80},
                {0x3304, 0x18},
                {0x3308, 0x40}, // def: 0x00
                //{0x330D, 0x40}, // def: 0x00
                {0x331E, 0x30},
                {0x3320, 0x06}, // {0x3320, 0x3321} = HTS - 0x30 = 0x0708 - 0x30 = 0x06D8
                {0x3321, 0xD8}, //
                {0x3322, 0x01}, // def: 0x00
                {0x3323, 0x80}, // power reduct def: 0x00
                {0x3330, 0x0D}, // def: 0x00
                {0x3335, 0x06}, // def: 0x1A
                {0x3600, 0x64}, // def: 0x54
                {0x3620, 0x42}, // def: 0x88
                {0x3636, 0x0D}, // def: 0x00 power reduct
                {0x5785, 0x0C}, // def: 0x40 black spot threshold
                //{GADI_VI_SENSOR_TABLE_FLAG_END,},
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
                .def_start_x    = 0,
                .def_start_y    = 0,
                .def_width      = 1280,
                .def_height     = 960,
                .sync_start     = (0 + 0),
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_BG,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_BG,
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
                // config Frame start physical position
                {0x3202, 0x00}, // TIMING_Y_START_H
                {0x3203, 0x08}, // TIMING_Y_START_L
                {0x3206, 0x03}, // TIMING_Y_END_H
                {0x3207, 0xCF}, // TIMING_Y_END_L
                // config Output image size
                {0x3208, 0x05}, // TIMING_X_SIZE_H   1280 = 0x0500
                {0x3209, 0x00}, // TIMING_X_SIZE_L
                {0x320A, 0x03}, // TIMING_Y_SIZE_H    960 = 0x03C0
                {0x320B, 0xC0}, // TIMING_Y_SIZE_L
                // config Frame length and width
                {0x320C, 0x08}, // TIMING_HTS_H      2160 = 0x0870
                {0x320D, 0x70}, // TIMING_HTS_L
                {0x320E, 0x03}, // TIMING_VTS_H      1000 = 0x03E8
                {0x320F, 0xE8}, // TIMING_VTS_L
                // config Output window position
                {0x3210, 0x00}, // TIMING_HOFFS_H
                {0x3211, 0x60}, // TIMING_HOFFS_L
                {0x3212, 0x00}, // TIMING_VOFFS_H
                {0x3213, 0x04}, // TIMING_VOFFS_L
                // power consumption reduction & timing
                {0x3300, 0x33},
                {0x3301, 0x38}, // cancel aec shanshuo
                {0x3303, 0x80},
                {0x3304, 0x18},
                {0x3308, 0x40}, // def: 0x00
                //{0x330D, 0x40}, // def: 0x00
                {0x331E, 0x30},
                {0x3320, 0x08}, // {0x3320, 0x3321} = HTS - 0x30 = 0x0870 - 0x30 = 0x0840
                {0x3321, 0x40}, //
                {0x3322, 0x01}, // def: 0x00
                {0x3323, 0x80}, // power reduct def: 0x00
                {0x3330, 0x0D}, // def: 0x00
                {0x3335, 0x06}, // def: 0x1A
                {0x3600, 0x64}, // def: 0x54
                {0x3620, 0x42}, // def: 0x88
                {0x3636, 0x0D}, // def: 0x00 power reduct
                {0x5785, 0x0C}, // def: 0x40 black spot threshold
                //{GADI_VI_SENSOR_TABLE_FLAG_END,},
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
                .def_start_x    = 0,
                .def_start_y    = 0,
                .def_width      = 1280,
                .def_height     = 960,
                .sync_start     = (0 + 0),
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_BG,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_BG,
                },
            },
            .update_after_vsync_start   = 3,
            .update_after_vsync_end     = 10,
        },
        [2] = // 1280x720P@60fps    1650 * 750 * 60 = 74250000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  60, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                // config Frame start physical position
                {0x3202, 0x00}, // TIMING_Y_START_H
                {0x3203, 0x78}, // TIMING_Y_START_L
                {0x3206, 0x03}, // TIMING_Y_END_H
                {0x3207, 0x4F}, // TIMING_Y_END_L
                // config Output image size
                {0x3208, 0x05}, // TIMING_X_SIZE_H   1280 = 0x0500
                {0x3209, 0x00}, // TIMING_X_SIZE_L
                {0x320A, 0x02}, // TIMING_Y_SIZE_H    720 = 0x02D0
                {0x320B, 0xD0}, // TIMING_Y_SIZE_L
                // config Frame length and width
                {0x320C, 0x06}, // TIMING_HTS_H      1650 = 0x0672
                {0x320D, 0x72}, // TIMING_HTS_L
                {0x320E, 0x02}, // TIMING_VTS_H       750 = 0x02EE
                {0x320F, 0xEE}, // TIMING_VTS_L
                // config Output window position
                {0x3210, 0x00}, // TIMING_HOFFS_H
                {0x3211, 0x60}, // TIMING_HOFFS_L
                {0x3212, 0x00}, // TIMING_VOFFS_H
                {0x3213, 0x04}, // TIMING_VOFFS_L
                // power consumption reduction & timing
                {0x3300, 0x22},
                {0x3301, 0x22}, // cancel aec shanshuo
                {0x3303, 0x60},
                {0x3304, 0x10},
                //{0x3308, 0x40}, // def: 0x00
                {0x330D, 0x40}, // def: 0x00
                {0x331E, 0x50},
                {0x3320, 0x06}, // {0x3320, 0x3321} = HTS - 0x30 = 0x0672 - 0x30 = 0x0642
                {0x3321, 0x42}, //
                {0x3322, 0x01}, // def: 0x00
                {0x3323, 0x30}, // power reduct def: 0x00
                {0x3330, 0x0D}, // def: 0x00
                {0x3335, 0x04}, // def: 0x1A
                {0x3600, 0x7C}, // def: 0x54
                {0x3620, 0x64}, // def: 0x88
                //{0x3636, 0x0D}, // def: 0x00 power reduct
                {0x5785, 0x08}, // def: 0x40 black spot threshold
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
                .pll_index  = 0,
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
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_BG,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_BG,
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
                // config Frame start physical position
                {0x3202, 0x00}, // TIMING_Y_START_H
                {0x3203, 0x78}, // TIMING_Y_START_L
                {0x3206, 0x03}, // TIMING_Y_END_H
                {0x3207, 0x4F}, // TIMING_Y_END_L
                // config Output image size
                {0x3208, 0x05}, // TIMING_X_SIZE_H   1280 = 0x0500
                {0x3209, 0x00}, // TIMING_X_SIZE_L
                {0x320A, 0x02}, // TIMING_Y_SIZE_H    720 = 0x02D0
                {0x320B, 0xD0}, // TIMING_Y_SIZE_L
                // config Frame length and width
                {0x320C, 0x07}, // TIMING_HTS_H      1980 = 0x07BC
                {0x320D, 0xBC}, // TIMING_HTS_L
                {0x320E, 0x02}, // TIMING_VTS_H       750 = 0x02EE
                {0x320F, 0xEE}, // TIMING_VTS_L
                // config Output window position
                {0x3210, 0x00}, // TIMING_HOFFS_H
                {0x3211, 0x60}, // TIMING_HOFFS_L
                {0x3212, 0x00}, // TIMING_VOFFS_H
                {0x3213, 0x04}, // TIMING_VOFFS_L
                // power consumption reduction & timing
                {0x3300, 0x33},
                {0x3301, 0x38}, // cancel aec shanshuo
                {0x3303, 0x86},
                {0x3304, 0x18},
                //{0x3308, 0x40}, // def: 0x00
                {0x330D, 0x40}, // def: 0x00
                {0x331E, 0x50},
                {0x3320, 0x07}, // {0x3320, 0x3321} = HTS - 0x30 = 0x07BC - 0x30 = 0x078C
                {0x3321, 0x8C}, //
                {0x3322, 0x01}, // def: 0x00
                {0x3323, 0xC0}, // power reduct def: 0x00
                {0x3330, 0x0D}, // def: 0x00
                {0x3335, 0x05}, // def: 0x1A
                {0x3600, 0x7C}, // def: 0x54
                {0x3620, 0x44}, // def: 0x88
                //{0x3636, 0x0D}, // def: 0x00 power reduct
                {0x5785, 0x08}, // def: 0x40 black spot threshold
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
                .pll_index  = 0,
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
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_BG,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_BG,
                },
            },
            .update_after_vsync_start   = 3,
            .update_after_vsync_end     = 10,
        },
        [4] = // 1280x720P@30fps    1650 * 750 * 30 = 37125000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                // config Frame start physical position
                {0x3202, 0x00}, // TIMING_Y_START_H
                {0x3203, 0x78}, // TIMING_Y_START_L
                {0x3206, 0x03}, // TIMING_Y_END_H
                {0x3207, 0x4F}, // TIMING_Y_END_L
                // config Output image size
                {0x3208, 0x05}, // TIMING_X_SIZE_H   1280 = 0x0500
                {0x3209, 0x00}, // TIMING_X_SIZE_L
                {0x320A, 0x02}, // TIMING_Y_SIZE_H    720 = 0x02D0
                {0x320B, 0xD0}, // TIMING_Y_SIZE_L
                // config Frame length and width
                {0x320C, 0x06}, // TIMING_HTS_H      1650 = 0x0672
                {0x320D, 0x72}, // TIMING_HTS_L
                {0x320E, 0x02}, // TIMING_VTS_H       750 = 0x02EE
                {0x320F, 0xEE}, // TIMING_VTS_L
                // config Output window position
                {0x3210, 0x00}, // TIMING_HOFFS_H
                {0x3211, 0x60}, // TIMING_HOFFS_L
                {0x3212, 0x00}, // TIMING_VOFFS_H
                {0x3213, 0x04}, // TIMING_VOFFS_L
                // power consumption reduction & timing
                {0x3300, 0x22},
                {0x3301, 0x22}, // cancel aec shanshuo
                {0x3303, 0x50},
                {0x3304, 0x18},
                //{0x3308, 0x40}, // def: 0x00
                {0x330D, 0x40}, // def: 0x00
                {0x331E, 0x50},
                {0x3320, 0x06}, // {0x3320, 0x3321} = HTS - 0x30 = 0x0672 - 0x30 = 0x0642
                {0x3321, 0x42}, //
                {0x3322, 0x01}, // def: 0x00
                {0x3323, 0x30}, // power reduct def: 0x00
                {0x3330, 0x0D}, // def: 0x00
                {0x3335, 0x05}, // def: 0x1A
                {0x3600, 0x7C}, // def: 0x54
                {0x3620, 0x44}, // def: 0x88
                //{0x3636, 0x0D}, // def: 0x00 power reduct
                {0x5785, 0x08}, // def: 0x40 black spot threshold
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
                .def_start_x    = 0,
                .def_start_y    = 0,
                .def_width      = 1280,
                .def_height     = 720,
                .sync_start     = (0 + 0),
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_BG,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_BG,
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
                // config Frame start physical position
                {0x3202, 0x00}, // TIMING_Y_START_H
                {0x3203, 0x78}, // TIMING_Y_START_L
                {0x3206, 0x03}, // TIMING_Y_END_H
                {0x3207, 0x4F}, // TIMING_Y_END_L
                // config Output image size
                {0x3208, 0x05}, // TIMING_X_SIZE_H   1280 = 0x0500
                {0x3209, 0x00}, // TIMING_X_SIZE_L
                {0x320A, 0x02}, // TIMING_Y_SIZE_H    720 = 0x02D0
                {0x320B, 0xD0}, // TIMING_Y_SIZE_L
                // config Frame length and width
                {0x320C, 0x07}, // TIMING_HTS_H      1980 = 0x07BC
                {0x320D, 0xBC}, // TIMING_HTS_L
                {0x320E, 0x02}, // TIMING_VTS_H       750 = 0x02EE
                {0x320F, 0xEE}, // TIMING_VTS_L
                // config Output window position
                {0x3210, 0x00}, // TIMING_HOFFS_H
                {0x3211, 0x60}, // TIMING_HOFFS_L
                {0x3212, 0x00}, // TIMING_VOFFS_H
                {0x3213, 0x04}, // TIMING_VOFFS_L
                // power consumption reduction & timing
                {0x3300, 0x33},
                {0x3301, 0x38}, // cancel aec shanshuo
                {0x3303, 0x66},
                {0x3304, 0x18},
                //{0x3308, 0x40}, // def: 0x00
                {0x330D, 0x40}, // def: 0x00
                {0x331E, 0x50},
                {0x3320, 0x07}, // {0x3320, 0x3321} = HTS - 0x30 = 0x07BC - 0x30 = 0x078C
                {0x3321, 0x8C}, //
                {0x3322, 0x01}, // def: 0x00
                {0x3323, 0xC0}, // power reduct def: 0x00
                {0x3330, 0x0D}, // def: 0x00
                {0x3335, 0x05}, // def: 0x1A
                {0x3600, 0x7C}, // def: 0x54
                {0x3620, 0x44}, // def: 0x88
                //{0x3636, 0x0D}, // def: 0x00 power reduct
                {0x5785, 0x08}, // def: 0x40 black spot threshold
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
                .def_start_x    = 0,
                .def_start_y    = 0,
                .def_width      = 1280,
                .def_height     = 720,
                .sync_start     = (0 + 0),
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_BG,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_BG,
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
#define SC1135_H_MIRROR     (1<<0)
#define SC1135_V_FLIP       (1<<6)
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY_VERTICALLY] =
        {
            {0x3211, 0x11},         // TIMING_HOFFS_L
            {0x3213, 0x01, 0x01},   // TIMING_VOFFS_L
            {0x3400, 0x52},         // RNC_ENABLE
            {0x3907, 0x00},         // BLC_TARGET_H
            {0x3908, 0xC0},         // BLC_TARGET_L
            {0x3781, 0x08},         // BLC_SYNC01
            {0x321C, SC1135_V_FLIP, SC1135_V_FLIP},
            {0x321D, SC1135_H_MIRROR, SC1135_H_MIRROR},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY] =
        {
            {0x3211, 0x11},         // TIMING_HOFFS_L
            {0x3213, 0x00, 0x01},   // TIMING_VOFFS_L
            {0x3400, 0x52},         // RNC_ENABLE
            {0x3907, 0x00},         // BLC_TARGET_H
            {0x3908, 0xC0},         // BLC_TARGET_L
            {0x3781, 0x08},         // BLC_SYNC01
            {0x321C, 0x00, SC1135_V_FLIP},
            {0x321D, SC1135_H_MIRROR, SC1135_H_MIRROR},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_VERTICALLY] =
        {
            {0x3211, 0x60},         // TIMING_HOFFS_L
            {0x3213, 0x01, 0x01},   // TIMING_VOFFS_L
            {0x3400, 0x53},         // RNC_ENABLE
            {0x3907, 0x02},         // BLC_TARGET_H
            {0x3908, 0x00},         // BLC_TARGET_L
            {0x3781, 0x10},         // BLC_SYNC01
            {0x321C, SC1135_V_FLIP, SC1135_V_FLIP},
            {0x321D, 0x00, SC1135_H_MIRROR},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_NONE] =
        {
            {0x3211, 0x60},         // TIMING_HOFFS_L
            {0x3213, 0x00, 0x01},   // TIMING_VOFFS_L
            {0x3400, 0x53},         // RNC_ENABLE
            {0x3907, 0x02},         // BLC_TARGET_H
            {0x3908, 0x00},         // BLC_TARGET_L
            {0x3781, 0x10},         // BLC_SYNC01
            {0x321C, 0x00, SC1135_V_FLIP},
            {0x321D, 0x00, SC1135_H_MIRROR},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
    },
    .version_reg =
    {
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .reg_ctl_by_ver_num = 0,
    .control_by_version_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
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
    // hmax = (TIMING_HTS_H & 0xFF) << 8 +
    //        (TIMING_HTS_L & 0xFF) << 0
    .hmax_reg =
    {
        {0x320C, 0x00, 0xFF, 0x00, 0x08, 0}, // TIMING_HTS_H
        {0x320D, 0x00, 0xFF, 0x00, 0x00, 0}, // TIMING_HTS_L
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    // vmax = (TIMING_VTS_H & 0xFF) << 8 +
    //        (TIMING_VTS_L & 0xFF) << 0
    .vmax_reg =
    {
        {0x320E, 0x00, 0xFF, 0x00, 0x08, 0}, // TIMING_VTS_H
        {0x320F, 0x00, 0xFF, 0x00, 0x00, 0}, // TIMING_VTS_L
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .vmax_wadd_reg =
    {
        {0x3338, 0x00, 0xFF, 0x00, 0x08, 0},
        {0x3339, 0x00, 0xFF, 0x00, 0x00, 0},
        {0x3336, 0x00, 0xFF, 0x00, 0x08, 0x02E8},
        {0x3337, 0x00, 0xFF, 0x00, 0x00, 0x02E8},
    },
    // shs = (AEC_EXP_TIME_H & 0xFF) << 4 +
    //       (AEC_EXP_TIME_L & 0xF0) >> 4
    .shs_reg =
    {
        {0x3E01, 0x00, 0xFF, 0x00, 0x04, 0}, // AEC_EXP_TIME_H
        {0x3E02, 0x00, 0xF0, 0x01, 0x04, 0}, // AEC_EXP_TIME_L
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .shs_additional_reg = {0x331E, 0x00, 0x00},
    .cal_gain_mode      = GADI_SENSOR_CAL_GAIN_MODE1,
    .max_agc_index      = 96,
    .max_again_index    = 48,
    .cur_again_index    = 0,
    .agc_info =
    {
        .db_max     = 0x2379b664,    // 35.48dB
        .db_min     = 0x00000000,    // 0dB
        .db_step    = 0x004ccccd,    // useless
    },
    .gain_reg =
    {
        {0x3630, 0x00,},
        {0x3631, 0x00,},
        {0x3E08, 0x00,},
        {0x3E09, 0x00,},
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .gain_table =
    {
        //        0x3630              0x3631        0x3e08          0x3e09
        //     restriction          restriction    dgain[1:0]       again
        // (    gain< 2: 0xB8) (    gain< 2: 0x82) (00: 1X)     ([6:4] corse_gain 000b=1X ,001b=2X, 011b=4X, 111b=8X)
        // ( 2<=gain   : 0x70) ( 2<=gain<16: 0x8E) (01: 2X)     ([3:0] fine_gain)
        //                     (16<=gain   : 0x8C) (11: 4X)
        {0x00000000, {0xB8, 0x82, 0x00, 0x00,},}, //  1.0000,
        {0x007e9cd5, {0xB8, 0x82, 0x00, 0x01,},}, //  1.0588,
        {0x00f66163, {0xB8, 0x82, 0x00, 0x02,},}, //  1.1176,
        {0x016831ab, {0xB8, 0x82, 0x00, 0x03,},}, //  1.1765,
        {0x01d44466, {0xB8, 0x82, 0x00, 0x04,},}, //  1.2353,
        {0x023b5056, {0xB8, 0x82, 0x00, 0x05,},}, //  1.2941,
        {0x029dc7e2, {0xB8, 0x82, 0x00, 0x06,},}, //  1.3529,
        {0x02fc3702, {0xB8, 0x82, 0x00, 0x07,},}, //  1.4118,
        {0x0356a34e, {0xB8, 0x82, 0x00, 0x08,},}, //  1.4706,
        {0x03ad83f3, {0xB8, 0x82, 0x00, 0x09,},}, //  1.5294,
        {0x04011d72, {0xB8, 0x82, 0x00, 0x0a,},}, //  1.5882,
        {0x0451cf44, {0xB8, 0x82, 0x00, 0x0b,},}, //  1.6471,
        {0x049f89ef, {0xB8, 0x82, 0x00, 0x0c,},}, //  1.7059,
        {0x04eaa236, {0xB8, 0x82, 0x00, 0x0d,},}, //  1.7647,
        {0x05334451, {0xB8, 0x82, 0x00, 0x0e,},}, //  1.8235,
        {0x0579b664, {0xB8, 0x82, 0x00, 0x0f,},}, //  1.8824,
        {0x06000000, {0x70, 0x8E, 0x00, 0x10,},}, //  2.0000,
        {0x067e9cd5, {0x70, 0x8E, 0x00, 0x11,},}, //  2.1176,
        {0x06f66163, {0x70, 0x8E, 0x00, 0x12,},}, //  2.2352,
        {0x076831ab, {0x70, 0x8E, 0x00, 0x13,},}, //  2.3530,
        {0x07d44466, {0x70, 0x8E, 0x00, 0x14,},}, //  2.4706,
        {0x083b5056, {0x70, 0x8E, 0x00, 0x15,},}, //  2.5882,
        {0x089dc7e2, {0x70, 0x8E, 0x00, 0x16,},}, //  2.7058,
        {0x08fc3702, {0x70, 0x8E, 0x00, 0x17,},}, //  2.8236,
        {0x0932e12d, {0x70, 0x8E, 0x00, 0x18,},}, //  2.89412,
        {0x09ad83f3, {0x70, 0x8E, 0x00, 0x19,},}, //  3.0588,
        {0x0a011d72, {0x70, 0x8E, 0x00, 0x1a,},}, //  3.1764,
        {0x0a51cf44, {0x70, 0x8E, 0x00, 0x1b,},}, //  3.2942,
        {0x0a9f89ef, {0x70, 0x8E, 0x00, 0x1c,},}, //  3.4118,
        {0x0aeaa236, {0x70, 0x8E, 0x00, 0x1d,},}, //  3.5294,
        {0x0b334451, {0x70, 0x8E, 0x00, 0x1e,},}, //  3.6470,
        {0x0b79b664, {0x70, 0x8E, 0x00, 0x1f,},}, //  3.7648,
        {0x0c000000, {0x70, 0x8E, 0x00, 0x30,},}, //  4.0000,
        {0x0c7e9cd5, {0x70, 0x8E, 0x00, 0x31,},}, //  4.2352,
        {0x0cf66163, {0x70, 0x8E, 0x00, 0x32,},}, //  4.4704,
        {0x0d6831ab, {0x70, 0x8E, 0x00, 0x33,},}, //  4.7060,
        {0x0dd44466, {0x70, 0x8E, 0x00, 0x34,},}, //  4.9412,
        {0x0e3b5056, {0x70, 0x8E, 0x00, 0x35,},}, //  5.1764,
        {0x0e9dc7e2, {0x70, 0x8E, 0x00, 0x36,},}, //  5.4116,
        {0x0efc3702, {0x70, 0x8E, 0x00, 0x37,},}, //  5.6472,
        {0x0f56a34e, {0x70, 0x8E, 0x00, 0x38,},}, //  5.8824,
        {0x0fad83f3, {0x70, 0x8E, 0x00, 0x39,},}, //  6.1176,
        {0x10011d72, {0x70, 0x8E, 0x00, 0x3a,},}, //  6.3528,
        {0x1051cf44, {0x70, 0x8E, 0x00, 0x3b,},}, //  6.5884,
        {0x109f89ef, {0x70, 0x8E, 0x00, 0x3c,},}, //  6.8236,
        {0x10eaa236, {0x70, 0x8E, 0x00, 0x3d,},}, //  7.0588,
        {0x11334451, {0x70, 0x8E, 0x00, 0x3e,},}, //  7.2940,
        {0x1179b664, {0x70, 0x8E, 0x00, 0x3f,},}, //  7.5296,
        {0x12000000, {0x70, 0x8E, 0x00, 0x70,},}, //  8.0000,
        {0x127e9cd5, {0x70, 0x8E, 0x00, 0x71,},}, //  8.4704,
        {0x12f66163, {0x70, 0x8E, 0x00, 0x72,},}, //  8.9408,
        {0x136831ab, {0x70, 0x8E, 0x00, 0x73,},}, //  9.412,
        {0x13d44466, {0x70, 0x8E, 0x00, 0x74,},}, //  9.8824,
        {0x143b5056, {0x70, 0x8E, 0x00, 0x75,},}, // 10.3528,
        {0x149dc7e2, {0x70, 0x8E, 0x00, 0x76,},}, // 10.8232,
        {0x14fc3702, {0x70, 0x8E, 0x00, 0x77,},}, // 11.2944,
        {0x1556a34e, {0x70, 0x8E, 0x00, 0x78,},}, // 11.7648,
        {0x15ad83f3, {0x70, 0x8E, 0x00, 0x79,},}, // 12.2352,
        {0x16011d72, {0x70, 0x8E, 0x00, 0x7a,},}, // 12.7056,
        {0x1651cf44, {0x70, 0x8E, 0x00, 0x7b,},}, // 13.1768,
        {0x15f6e7f1, {0x70, 0x8E, 0x00, 0x7c,},}, // 12.6472,
        {0x16eaa236, {0x70, 0x8E, 0x00, 0x7d,},}, // 14.1176,
        {0x17334451, {0x70, 0x8E, 0x00, 0x7e,},}, // 14.5880,
        {0x1779b664, {0x70, 0x8E, 0x00, 0x7f,},}, // 15.0592,
        {0x18000000, {0x70, 0x8C, 0x01, 0x70,},}, // 16.0000, dgain=2X
        {0x187e9cd5, {0x70, 0x8C, 0x01, 0x71,},}, // 16.9408,
        {0x18f66163, {0x70, 0x8C, 0x01, 0x72,},}, // 17.8816,
        {0x196831ab, {0x70, 0x8C, 0x01, 0x73,},}, // 18.8240,
        {0x19d44466, {0x70, 0x8C, 0x01, 0x74,},}, // 19.7648,
        {0x1a3b5056, {0x70, 0x8C, 0x01, 0x75,},}, // 20.7056,
        {0x1a9dc7e2, {0x70, 0x8C, 0x01, 0x76,},}, // 21.6464,
        {0x1afc3702, {0x70, 0x8C, 0x01, 0x77,},}, // 22.5888,
        {0x1b56a34e, {0x70, 0x8C, 0x01, 0x78,},}, // 23.5296,
        {0x1bad83f3, {0x70, 0x8C, 0x01, 0x79,},}, // 24.4704,
        {0x1c011d72, {0x70, 0x8C, 0x01, 0x7a,},}, // 25.4112,
        {0x1c51cf44, {0x70, 0x8C, 0x01, 0x7b,},}, // 26.3536,
        {0x1bf6e7f1, {0x70, 0x8C, 0x01, 0x7c,},}, // 25.2944,
        {0x1ceaa236, {0x70, 0x8C, 0x01, 0x7d,},}, // 28.2352,
        {0x1d334451, {0x70, 0x8C, 0x01, 0x7e,},}, // 29.1760,
        {0x1d79b664, {0x70, 0x8C, 0x01, 0x7f,},}, // 30.1184,
        {0x1e000000, {0x70, 0x8C, 0x03, 0x70,},}, // 32.0000, dgain=4X
        {0x1e7e9cd5, {0x70, 0x8C, 0x03, 0x71,},}, // 33.8816,
        {0x1ef66163, {0x70, 0x8C, 0x03, 0x72,},}, // 35.7632,
        {0x1f6831ab, {0x70, 0x8C, 0x03, 0x73,},}, // 37.6480,
        {0x1fd44466, {0x70, 0x8C, 0x03, 0x74,},}, // 39.5296,
        {0x203b5056, {0x70, 0x8C, 0x03, 0x75,},}, // 41.4112,
        {0x209dc7e2, {0x70, 0x8C, 0x03, 0x76,},}, // 43.2928,
        {0x20fc3702, {0x70, 0x8C, 0x03, 0x77,},}, // 45.1776,
        {0x2156a34e, {0x70, 0x8C, 0x03, 0x78,},}, // 47.0592,
        {0x21ad83f3, {0x70, 0x8C, 0x03, 0x79,},}, // 48.9408,
        {0x22011d72, {0x70, 0x8C, 0x03, 0x7a,},}, // 50.8224,
        {0x2251cf44, {0x70, 0x8C, 0x03, 0x7b,},}, // 52.7072,
        {0x21f6e7f1, {0x70, 0x8C, 0x03, 0x7c,},}, // 50.5888,
        {0x22eaa236, {0x70, 0x8C, 0x03, 0x7d,},}, // 56.4704,
        {0x23334451, {0x70, 0x8C, 0x03, 0x7e,},}, // 58.3520,
        {0x2379b664, {0x70, 0x8C, 0x03, 0x7f,},}, // 60.2368,
    },
    .temperatue_gain_table =
    {
        .agc_control_by_temperatue      = 1,
        .Aptina_temperatue_mode         = 0,
        .temperatue_control_reg_index   = 3,
        .temperatue_gain_num            = 10,
        .temperatue_reg                 = {0x3911, 0x00, 0x00,},
        .gain_reg                       =
        {
            {0x3E09,},
            {0x3606,},
            {0x333f,},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        .temperatue_gain                =
        {
            // 0x57 < {0x3911}
            {0x57, {0x00, 0x20, 0xA0,},},
            // 0x54 < {0x3911} <= 0x57
            {0x54, {0x00, 0x20, 0xA0,},
                   {{.opt = GADI_GAIN_OPT_SKIP},{.opt = GADI_GAIN_OPT_SKIP},{.opt = GADI_GAIN_OPT_SKIP},},},
            // 0x52 < {0x3911} <= 0x54
            {0x52, {0x00, 0x20, 0x90,},},
            // 0x51 < {0x3911} <= 0x52
            {0x51, {0x00, 0x20, 0x90,},
                   {{.opt = GADI_GAIN_OPT_SKIP},{.opt = GADI_GAIN_OPT_SKIP},{.opt = GADI_GAIN_OPT_SKIP},},},
            // 0x4B < {0x3911} <= 0x51
            {0x4B, {0x00, 0x20, 0x8C,},},
            // 0x4A < {0x3911} <= 0x4B
            {0x4A, {0x00, 0x20, 0x8C,},
                   {{.opt = GADI_GAIN_OPT_SKIP},{.opt = GADI_GAIN_OPT_SKIP},{.opt = GADI_GAIN_OPT_SKIP},},},
            // 0x47 < {0x3911} <= 0x4A
            {0x47, {0x00, 0x20, 0x87,},},
            // 0x46 < {0x3911} <= 0x47
            {0x46, {0x00, 0x00, 0x00,},
                   {{.opt = GADI_GAIN_OPT_SKIP},{.opt = GADI_GAIN_OPT_SKIP},{.opt = GADI_GAIN_OPT_SKIP},},},
            // 0x45 < {0x3911} <= 0x46
            {0x45, {0x00, 0x00, 0x00,},
                   {{.opt = GADI_GAIN_OPT_SKIP},{.opt = GADI_GAIN_OPT_SKIP},{.opt = GADI_GAIN_OPT_SKIP},},},
            // {0x3911} <= 0x45
            {0x44, {0x00, 0x00, 0x00,},},
        },
        .temperatue_control             =
        {
            0x7f,0x7e,0x7d,0x7c,0x7b,0x7a,0x79,0x78,0x77,0x76,0x75,0x74,0x73,0x72,0x71,0x70,
            0x3f,0x3e,0x3d,0x3c,0x3b,0x3a,0x39,0x38,0x37,0x36,0x35,0x34,0x33,0x32,0x31,0x30,
            0x1f,0x1e,0x1d,0x1c,0x1b,0x1a,0x19,0x18,0x17,0x16,0x15,0x14,0x13,0x12,0x11,0x10,
        },
    },
    .magic_end  = GADI_SENSOR_HW_INFO_MAGIC_END,
};

