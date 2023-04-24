/*!
*****************************************************************************
** \file        sc1145_reg_tbl.c
**
** \version     $Id: sc1145_reg_tbl.c 11627 2017-03-10 10:58:12Z dengbiao $
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
GADI_VI_SensorDrvInfoT    sc1145_sensor =
{
    .magic_start        = GADI_SENSOR_HW_INFO_MAGIC_START,
    .version            = GADI_SENSOR_HW_INFO_VERSION,
    .length             = sizeof(GADI_VI_SensorDrvInfoT),
    .file_date          = 0x20161014,
    .HwInfo         =
    {
        .name               = "sc1145",
        .ctl_type           = GADI_SENSOR_CTL_I2C,
        .addr_len           = sizeof(GADI_U16),
        .data_len           = sizeof(GADI_U8),
        .reset_active_level = GADI_GPIO_LOW,
        .hw_addr            = {(0x60>>1), 0, 0, 0},
        .id_reg =
        {
            {0x3107, 0x11},
            {0x3108, 0x45},
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
    .sensor_id          = GADI_ISP_SENSOR_SC1145,
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
    .fmt_num            = 4,
    .auto_fmt           = GADI_VIDEO_MODE(1280,  720,  25, 1),
    .init_reg           =
    {
        {0x3000, 0x00}, // manual stream disable
        {0x3308, 0x40},
        {0x3310, 0x83}, // prechg tx auto ctrl [5]
        {0x3315, 0x44},
        {0x331E, 0xA0}, // start value
        {0x3330, 0x0D}, // sal_en timing, cancel the fpn in low light
        //{0x3415, 0x00}, // RNC_TARGET_H
        //{0x3416, 0x3B}, // RNC_TARGET_L
        {0x3600, 0x94},
        {0x3610, 0x03},
        {0x3622, 0x0E},
        {0x3633, 0x2C},
        {0x3634, 0x00},
        {0x3635, 0x80},
        {0x3640, 0x02}, // pad driving ability
        {0x3780, 0x02},
        {0x3907, 0x03}, // BLC_TARGET_H
        {0x3D04, 0x04},
        {0x3D08, 0x02}, // add sensor driver capacity
        //{0x3E03, 0x03}, // AEC_ENABLE
        {0x3E09, 0x00}, // AEC_GAIN_L
        {0x3E0F, 0x84}, // AEC
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
            .extclk = 27000000, //        = 27 * (64 - 53) / (2 * (0 + 1) * 2) = 74.25MHz
            .regs =
            {
                {0x3010, 0x05}, // PLL_PRE_CLK_DIV 0 000 010 1 SYSEL[2:0] = b000, PreDIV[2:0] = b010 2x, PLLDIV[5] = b1
                {0x3011, 0xAE}, // PLL_SYS_CLK_DIV 10101 110 PLLDIV[4:0] = b10101
                {0x3004, 0x42}, // SCLK_DIV[3:0]
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
        [1] =
        {
            // for 720P@30, 720P@25
            .pixclk = 37125000, // pixclk = (extclk * (64 - PLLDIV[5:0])) / (PreDIV[2:0] * (SYSEL[2:0] + 1) * SCLK_DIV)
            .extclk = 27000000, //        = 27 * (64 - 53) / (2 * (0 + 1) * 4) = 37.125MHz
            .regs =
            {
                {0x3010, 0x05}, // PLL_PRE_CLK_DIV 0 001 000 1 SYSEL[2:0] = b001, PreDIV[2:0] = b000 1x, PLLDIV[5] = b1
                {0x3011, 0xAE}, // PLL_SYS_CLK_DIV 01010 110 PLLDIV[4:0] = b01010
                {0x3004, 0x44}, // SCLK_DIV[3:0]
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
    },
    .video_fmt_table =
    {
        [0] = // 1280x720P@60fps    1800 * 750 * 60 = 86400000  // FLIP request >= 1800
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
                {0x3203, 0x08}, // TIMING_Y_START_L
                {0x3206, 0x02}, // TIMING_Y_END_H
                {0x3207, 0xE0}, // TIMING_Y_END_L
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
                {0x3205, 0x63}, // x phy add
                {0x3300, 0x22}, // timing
                {0x3301, 0x22}, // cancel aec shanshuo
                {0x3302, 0x08},
                {0x3303, 0x4d},
                {0x3304, 0x40},
                {0x3320, 0x05}, // {0x3320, 0x3321} = HTS - ???
                {0x3321, 0x00},
                {0x3322, 0x01},
                {0x3323, 0x50},
                {0x3335, 0x0A},
                {0x3620, 0x84},
                {0x3630, 0x83},
                {0x3631, 0xC5},
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 1280,
                .height     = 720,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_DVP, 
                .bits       = GADI_VIDEO_BITS_10,
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
            .update_after_vsync_start   = 13,
            .update_after_vsync_end     = 16,
        },
        [1] = // 1280x720P@50fps    1980 * 750 * 50 = 74250000
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
                {0x3203, 0x08}, // TIMING_Y_START_L
                {0x3206, 0x02}, // TIMING_Y_END_H
                {0x3207, 0xE0}, // TIMING_Y_END_L
                // config Output image size
                {0x3208, 0x05}, // TIMING_X_SIZE_H   1280 = 0x0500
                {0x3209, 0x00}, // TIMING_X_SIZE_L
                {0x320A, 0x02}, // TIMING_Y_SIZE_H    720 = 0x02D0
                {0x320B, 0xD0}, // TIMING_Y_SIZE_L
                // config Frame length and width
                {0x320C, 0x07}, // TIMING_HTS_H     1980 = 0x07BC
                {0x320D, 0xBC}, // TIMING_HTS_L
                {0x320E, 0x02}, // TIMING_VTS_H       750 = 0x02EE
                {0x320F, 0xEE}, // TIMING_VTS_L
                // config Output window position
                {0x3210, 0x00}, // TIMING_HOFFS_H
                {0x3211, 0x60}, // TIMING_HOFFS_L
                {0x3212, 0x00}, // TIMING_VOFFS_H
                {0x3213, 0x04}, // TIMING_VOFFS_L
                // power consumption reduction & timing
                //{0x3205, 0x63}, // x phy add
                //{0x3300, 0x22}, // timing
                //{0x3301, 0x22}, // cancel aec shanshuo
                //{0x3302, 0x08},
                {0x3303, 0xA0},
                {0x3304, 0x60},
                {0x3320, 0x06}, // {0x3320, 0x3321} = HTS - ???
                {0x3321, 0xBC},
                {0x3322, 0x02},
                {0x3323, 0xC0},
                {0x3335, 0x1A},
                {0x3620, 0x84},
                {0x3630, 0x83},
                {0x3631, 0xC5},
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 1280,
                .height     = 720,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_DVP, 
                .bits       = GADI_VIDEO_BITS_10,
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
            .update_after_vsync_start   = 17,
            .update_after_vsync_end     = 20,
        },
        [2] = // 1280x720P@30fps    1920 * 750 * 30 = 43200000  // FLIP request >= 1800
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
                {0x3203, 0x08}, // TIMING_Y_START_L
                {0x3206, 0x02}, // TIMING_Y_END_H
                {0x3207, 0xE0}, // TIMING_Y_END_L
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
                {0x3205, 0x63}, // x phy add
                {0x3300, 0x22}, // timing
                {0x3301, 0x22}, // cancel aec shanshuo
                {0x3302, 0x08},
                {0x3303, 0x4d},
                {0x3304, 0x40},
                {0x3320, 0x05}, // {0x3320, 0x3321} = HTS - ???
                {0x3321, 0x00},
                {0x3322, 0x01},
                {0x3323, 0x50},
                {0x3335, 0x0A},
                {0x3620, 0x84},
                {0x3630, 0x88},
                {0x3631, 0x85},
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 1280,
                .height     = 720,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_DVP, 
                .bits       = GADI_VIDEO_BITS_10,
                .ratio      = GADI_VIDEO_RATIO_16_9,
                .srm        = 0,
                .pll_index  = 1,
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
            .update_after_vsync_start   = 30,
            .update_after_vsync_end     = 33,
        },
        [3] = // 1280x720P@25fps    1980 * 750 * 25 = 37125000
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
                {0x3203, 0x08}, // TIMING_Y_START_L
                {0x3206, 0x02}, // TIMING_Y_END_H
                {0x3207, 0xE0}, // TIMING_Y_END_L
                // config Output image size
                {0x3208, 0x05}, // TIMING_X_SIZE_H   1280 = 0x0500
                {0x3209, 0x00}, // TIMING_X_SIZE_L
                {0x320A, 0x02}, // TIMING_Y_SIZE_H    720 = 0x02D0
                {0x320B, 0xD0}, // TIMING_Y_SIZE_L
                // config Frame length and width
                {0x320C, 0x07}, // TIMING_HTS_H     1980 = 0x07BC
                {0x320D, 0xBC}, // TIMING_HTS_L
                {0x320E, 0x02}, // TIMING_VTS_H       750 = 0x02EE
                {0x320F, 0xEE}, // TIMING_VTS_L
                // config Output window position
                {0x3210, 0x00}, // TIMING_HOFFS_H
                {0x3211, 0x60}, // TIMING_HOFFS_L
                {0x3212, 0x00}, // TIMING_VOFFS_H
                {0x3213, 0x04}, // TIMING_VOFFS_L
                // power consumption reduction & timing
                //{0x3205, 0x63}, // x phy add
                //{0x3300, 0x22}, // timing
                //{0x3301, 0x22}, // cancel aec shanshuo
                //{0x3302, 0x08},
                {0x3303, 0xA0},
                {0x3304, 0x60},
                {0x3320, 0x06}, // {0x3320, 0x3321} = HTS - ???
                {0x3321, 0xBC},
                {0x3322, 0x02},
                {0x3323, 0xC0},
                {0x3335, 0x1A},
                {0x3620, 0x44},
                {0x3630, 0x88},
                {0x3631, 0x85},
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 1280,
                .height     = 720,
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
            .update_after_vsync_start   = 37,
            .update_after_vsync_end     = 40,
        },
    },
    .mirror_table =
    {
#define SC1145_H_MIRROR     (1<<0)
#define SC1145_V_FLIP       (1<<6)
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY_VERTICALLY] =
        {
            {0x3211, 0x01, 0x01},
            {0x3781, 0x08},
            {0x321C, SC1145_V_FLIP, SC1145_V_FLIP},
            {0x321D, SC1145_H_MIRROR, SC1145_H_MIRROR},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY] =
        {
            {0x3211, 0x01, 0x01},
            {0x3781, 0x08},
            {0x321C, 0x00, SC1145_V_FLIP},
            {0x321D, SC1145_H_MIRROR, SC1145_H_MIRROR},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_VERTICALLY] =
        {
            {0x3211, 0x00, 0x01},
            {0x3781, 0x10},
            {0x321C, SC1145_V_FLIP, SC1145_V_FLIP},
            {0x321D, 0x00, SC1145_H_MIRROR},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_NONE] =
        {
            {0x3211, 0x00, 0x01},
            {0x3781, 0x10},
            {0x321C, 0x00, SC1145_V_FLIP},
            {0x321D, 0x00, SC1145_H_MIRROR},
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
    .shs_additional_use         = 0,
    .shs_additional_percent     = 0,
    // hmax = (TIMING_HTS_H & 0xFF) << 8 +
    //        (TIMING_HTS_L & 0xFF) << 0
    .hmax_reg =
    {
        {0x320C, 0x00, 0xFF, 0x00, 0x08, 0}, // TIMING_HTS_H
        {0x320D, 0x00, 0xFF, 0x00, 0x00, 0}, // TIMING_HTS_L
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    // vmax = (SC1145_TIMING_VTS_H & 0xFF) << 8 +
    //        (SC1145_TIMING_VTS_L & 0xFF) << 0
    .vmax_reg =
    {
        {0x320E, 0x00, 0xFF, 0x00, 0x08, 0}, // TIMING_VTS_H
        {0x320F, 0x00, 0xFF, 0x00, 0x00, 0}, // TIMING_VTS_L
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    // data = (vmax - fix) & mask >> shift
    .vmax_wadd_reg =
    {
        {0x3338, 0x00, 0xFF, 0x00, 0x08, 0},
        {0x3339, 0x00, 0xFF, 0x00, 0x00, 0},
        {0x3336, 0x00, 0xFF, 0x00, 0x08, 0x02EE},
        {0x3337, 0x00, 0xFF, 0x00, 0x00, 0x02EE},
    },
    // shs = (AEC_EXP_TIME_H & 0xFF) << 4 +
    //       (AEC_EXP_TIME_L & 0xF0) >> 4
    .shs_reg =
    {
        {0x3E01, 0x00, 0xFF, 0x00, 0x04, 0}, // AEC_EXP_TIME_H
        {0x3E02, 0x00, 0xF0, 0x01, 0x04, 0}, // AEC_EXP_TIME_L
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .shs_additional_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    .cal_gain_mode      = GADI_SENSOR_CAL_GAIN_MODE1,
    .max_agc_index      = 96,
    .max_again_index    = 112,
    .cur_again_index    = 0,
    .agc_info =
    {
        .db_max     = 0x23000000,    // 35dB
        .db_min     = 0x00000000,    // 0dB
        .db_step    = 0x00300000,    // Useless
    },
    .gain_reg =
    {
        {0x3E05, 0x00},
        {0x3E10, 0x00},
        {0x3E09, 0x00},
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .gain_table =
    {
        /*0x3e0f[1:0] 0x3e09[7:0]  db_value*100*/
        /*  dig_gain        ana_gain        dcg_gain        fine_gain
            0x3E0F          0x3E09          0x3E09          0x3E09
            bit[1:0]        bit[6:4]        bit[7]          bit[3:0]*/
        {0x00000000, {0x0c, 0x03, 0x00, /*dig_again 1;ana_gain 1;  fine_gain 1.0000*/}, // 1.00
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x007f0c1d, {0x0c, 0x03, 0x01, /*dig_again 1;ana_gain 1;  fine_gain 1.0588*/},   // 1.05
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x00f739ef, {0x0c, 0x03, 0x02, /*dig_again 1;ana_gain 1;  fine_gain 1.1176*/},   // 1.11
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x01696e40, {0x0c, 0x03, 0x03, /*dig_again 1;ana_gain 1;  fine_gain 1.1765*/},   // 1.17
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x01d5dff7, {0x0c, 0x03, 0x04, /*dig_again 1;ana_gain 1;  fine_gain 1.2353*/},   // 1.23
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x023d4679, {0x0c, 0x03, 0x05, /*dig_again 1;ana_gain 1;  fine_gain 1.2941*/},   // 1.29
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x02a01490, {0x0c, 0x03, 0x06, /*dig_again 1;ana_gain 1;  fine_gain 1.3529*/},   // 1.36
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x02fed6b0, {0x0c, 0x03, 0x07, /*dig_again 1;ana_gain 1;  fine_gain 1.4118*/},   // 1.41
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x03599275, {0x0c, 0x03, 0x08, /*dig_again 1;ana_gain 1;  fine_gain 1.4706*/},   // 1.47
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x03b0bf76, {0x0c, 0x03, 0x09, /*dig_again 1;ana_gain 1;  fine_gain 1.5294*/},   // 1.52
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x0404a26f, {0x0c, 0x03, 0x0a, /*dig_again 1;ana_gain 1;  fine_gain 1.5882*/},   // 1.58
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x04559b2d, {0x0c, 0x03, 0x0b, /*dig_again 1;ana_gain 1;  fine_gain 1.6471*/},   // 1.64
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x04a39a2a, {0x0c, 0x03, 0x0c, /*dig_again 1;ana_gain 1;  fine_gain 1.7059*/},   // 1.70
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x04eef471, {0x0c, 0x03, 0x0d, /*dig_again 1;ana_gain 1;  fine_gain 1.7647*/},   // 1.76
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x0537d663, {0x0c, 0x03, 0x0e, /*dig_again 1;ana_gain 1;  fine_gain 1.8235*/},   // 1.82
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x057e8660, {0x0c, 0x03, 0x0f, /*dig_again 1;ana_gain 1;  fine_gain 1.8824*/},   // 1.88
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x06054603, {0x0c, 0x03, 0x10, /*dig_again 1;ana_gain 2;  fine_gain 1.0000*/},   // fine_gain*2
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x06845220, {0x0c, 0x03, 0x11, /*dig_again 1;ana_gain 2;  fine_gain 1.0588*/},   // fine_gain*2
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x06fc7ff3, {0x0c, 0x03, 0x12, /*dig_again 1;ana_gain 2;  fine_gain 1.1176*/},   // fine_gain*2
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x076eb443, {0x0c, 0x03, 0x13, /*dig_again 1;ana_gain 2;  fine_gain 1.1765*/},   // fine_gain*2
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x07db25fa, {0x0c, 0x03, 0x14, /*dig_again 1;ana_gain 2;  fine_gain 1.2353*/},   // fine_gain*2
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x08428c7c, {0x0c, 0x03, 0x15, /*dig_again 1;ana_gain 2;  fine_gain 1.2941*/},   // fine_gain*2
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x08a55a93, {0x0c, 0x03, 0x16, /*dig_again 1;ana_gain 2;  fine_gain 1.3529*/},   // fine_gain*2
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x09041cb3, {0x0c, 0x03, 0x17, /*dig_again 1;ana_gain 2;  fine_gain 1.4118*/},   // fine_gain*2
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x095ed878, {0x0c, 0x03, 0x18, /*dig_again 1;ana_gain 2;  fine_gain 1.4706*/},   // fine_gain*2
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x09b60579, {0x0c, 0x03, 0x19, /*dig_again 1;ana_gain 2;  fine_gain 1.5294*/},   // fine_gain*2
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x0a09e873, {0x0c, 0x03, 0x1a, /*dig_again 1;ana_gain 2;  fine_gain 1.5882*/},   // fine_gain*2
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x0a5ae130, {0x0c, 0x03, 0x1b, /*dig_again 1;ana_gain 2;  fine_gain 1.6471*/},   // fine_gain*2
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x0aa8e02d, {0x0c, 0x03, 0x1c, /*dig_again 1;ana_gain 2;  fine_gain 1.7059*/},   // fine_gain*2
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x0af43a74, {0x0c, 0x03, 0x1d, /*dig_again 1;ana_gain 2;  fine_gain 1.7647*/},   // fine_gain*2
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x0b3d1c66, {0x0c, 0x03, 0x1e, /*dig_again 1;ana_gain 2;  fine_gain 1.8235*/},   // fine_gain*2
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x0b83cc64, {0x0c, 0x03, 0x1f, /*dig_again 1;ana_gain 2;  fine_gain 1.8824*/},   // fine_gain*2
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x0c0a8c06, {0x0c, 0x03, 0x30, /*dig_again 1;ana_gain 4;  fine_gain 1.0000*/},   // 3.54
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x0c899824, {0x0c, 0x03, 0x31, /*dig_again 1;ana_gain 4;  fine_gain 1.0588*/},   // 3.74
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x0d01c5f6, {0x0c, 0x03, 0x32, /*dig_again 1;ana_gain 4;  fine_gain 1.1176*/},   // 3.95
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x0d73fa46, {0x0c, 0x03, 0x33, /*dig_again 1;ana_gain 4;  fine_gain 1.1765*/},   // 4.16
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x0de06bfd, {0x0c, 0x03, 0x34, /*dig_again 1;ana_gain 4;  fine_gain 1.2353*/},   // 4.37
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x0e47d27f, {0x0c, 0x03, 0x35, /*dig_again 1;ana_gain 4;  fine_gain 1.2941*/},   // 4.58
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x0eaaa096, {0x0c, 0x03, 0x36, /*dig_again 1;ana_gain 4;  fine_gain 1.3529*/},   // 4.78
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x0f0962b6, {0x0c, 0x03, 0x37, /*dig_again 1;ana_gain 4;  fine_gain 1.4118*/},   // 4.99
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x0f641e7b, {0x0c, 0x03, 0x38, /*dig_again 1;ana_gain 4;  fine_gain 1.4706*/},   // 5.2
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x0fbb4b7c, {0x0c, 0x03, 0x39, /*dig_again 1;ana_gain 4;  fine_gain 1.5294*/},   // 7.41
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x100f2e76, {0x0c, 0x03, 0x3a, /*dig_again 1;ana_gain 4;  fine_gain 1.5882*/},   // 5.62
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x10602734, {0x0c, 0x03, 0x3b, /*dig_again 1;ana_gain 4;  fine_gain 1.6471*/},   // 5.83
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x10ae2630, {0x0c, 0x03, 0x3c, /*dig_again 1;ana_gain 4;  fine_gain 1.7059*/},   // 6.03
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x10f98078, {0x0c, 0x03, 0x3d, /*dig_again 1;ana_gain 4;  fine_gain 1.7647*/},   // 6.24
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x11426269, {0x0c, 0x03, 0x3e, /*dig_again 1;ana_gain 4;  fine_gain 1.8235*/},   // 6.45
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x11891267, {0x0c, 0x03, 0x3f, /*dig_again 1;ana_gain 4;  fine_gain 1.8824*/},   // 6.66
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x120fd209, {0x0c, 0x03, 0x70, /*dig_again 1;ana_gain 8;  fine_gain 1.0000*/},   // 7.08
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x128ede27, {0x0c, 0x03, 0x71, /*dig_again 1;ana_gain 8;  fine_gain 1.0588*/},   // 7.49
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x13070bf9, {0x0c, 0x03, 0x72, /*dig_again 1;ana_gain 8;  fine_gain 1.1176*/},   // 7.91
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x13794049, {0x0c, 0x03, 0x73, /*dig_again 1;ana_gain 8;  fine_gain 1.1765*/},   // 8.32
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x13e5b201, {0x0c, 0x03, 0x74, /*dig_again 1;ana_gain 8;  fine_gain 1.2353*/},   // 8.74
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x144d1883, {0x0c, 0x03, 0x75, /*dig_again 1;ana_gain 8;  fine_gain 1.2941*/},   // 9.16
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x14afe699, {0x0c, 0x03, 0x76, /*dig_again 1;ana_gain 8;  fine_gain 1.3529*/},   // 9.57
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x150ea8b9, {0x0c, 0x03, 0x77, /*dig_again 1;ana_gain 8;  fine_gain 1.4118*/},   // 9.99
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x1569647e, {0x0c, 0x03, 0x78, /*dig_again 1;ana_gain 8;  fine_gain 1.4706*/},   // 10.41
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x15c09180, {0x0c, 0x03, 0x79, /*dig_again 1;ana_gain 8;  fine_gain 1.5294*/},   // 10.82
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x16147479, {0x0c, 0x03, 0x7a, /*dig_again 1;ana_gain 8;  fine_gain 1.5882*/},   // 11.24
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x16656d37, {0x0c, 0x03, 0x7b, /*dig_again 1;ana_gain 8;  fine_gain 1.6471*/},   // 11.66
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x16b36c33, {0x0c, 0x03, 0x7c, /*dig_again 1;ana_gain 8;  fine_gain 1.7059*/},   // 12.07
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x16fec67b, {0x0c, 0x03, 0x7d, /*dig_again 1;ana_gain 8;  fine_gain 1.7647*/},   // 12.49
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x1747a86c, {0x0c, 0x03, 0x7e, /*dig_again 1;ana_gain 8;  fine_gain 1.8235*/},   // 12.91
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x178e586a, {0x0c, 0x03, 0x7f, /*dig_again 1;ana_gain 8;  fine_gain 1.8824*/},   // 13.32
                     {{.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x1815180c, {0x0c, 0x1b, 0x70, /*dig_again 2;ana_gain 8;  fine_gain 1.0000*/},   // 14.16
                     {{.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x1894242a, {0x0c, 0x1b, 0x71, /*dig_again 2;ana_gain 8;  fine_gain 1.0588*/},   // 14.99
                     {{.opt = GADI_GAIN_OPT_READ_AND_CHECK,  .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x190c51fc, {0x0c, 0x1b, 0x72, /*dig_again 2;ana_gain 8;  fine_gain 1.1176*/},   // 15.82
                     {{.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x197e864c, {0x0c, 0x1b, 0x73, /*dig_again 2;ana_gain 8;  fine_gain 1.1765*/},   // 16.65
                     {{.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x19eaf804, {0x0c, 0x1b, 0x74, /*dig_again 2;ana_gain 8;  fine_gain 1.2353*/},   // 17.49
                     {{.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x1a525e86, {0x0c, 0x1b, 0x75, /*dig_again 2;ana_gain 8;  fine_gain 1.2941*/},   // 18.32
                     {{.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x1ab52c9d, {0x0c, 0x1b, 0x76, /*dig_again 2;ana_gain 8;  fine_gain 1.3529*/},   // 19.15
                     {{.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x1b13eebc, {0x0c, 0x1b, 0x77, /*dig_again 2;ana_gain 8;  fine_gain 1.4118*/},   // 19.99
                     {{.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x1b6eaa82, {0x0c, 0x1b, 0x78, /*dig_again 2;ana_gain 8;  fine_gain 1.4706*/},   // 20.82
                     {{.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x1bc5d783, {0x0c, 0x1b, 0x79, /*dig_again 2;ana_gain 8;  fine_gain 1.5294*/},   // 21.65
                     {{.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x1c19ba7c, {0x0c, 0x1b, 0x7a, /*dig_again 2;ana_gain 8;  fine_gain 1.5882*/},   // 22.48
                     {{.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x1c6ab33a, {0x0c, 0x1b, 0x7b, /*dig_again 2;ana_gain 8;  fine_gain 1.6471*/},   // 23.32
                     {{.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x1cb8b236, {0x0c, 0x1b, 0x7c, /*dig_again 2;ana_gain 8;  fine_gain 1.7059*/},   // 24.15
                     {{.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x1d040c7e, {0x0c, 0x1b, 0x7d, /*dig_again 2;ana_gain 8;  fine_gain 1.7647*/},   // 24.98
                     {{.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x1d4cee6f, {0x0c, 0x1b, 0x7e, /*dig_again 2;ana_gain 8;  fine_gain 1.8235*/},   // 25.82
                     {{.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x1d939e6d, {0x0c, 0x1b, 0x7f, /*dig_again 2;ana_gain 8;  fine_gain 1.8824*/},   // 26.65
                     {{.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x1e1a5e0f, {0x3c, 0x1b, 0x70, /*dig_again 4;ana_gain 8;  fine_gain 1.0000*/},   // 28.32
                     {{.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x1e9975f0, {0x3c, 0x1b, 0x71, /*dig_again 4;ana_gain 8;  fine_gain 1.0588*/},   // 29.98
                     {{.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x1f11afdf, {0x3c, 0x1b, 0x72, /*dig_again 4;ana_gain 8;  fine_gain 1.1176*/},   // 31.65
                     {{.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x1f83bd31, {0x3c, 0x1b, 0x73, /*dig_again 4;ana_gain 8;  fine_gain 1.1765*/},   // 33.31
                     {{.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x1ff03b26, {0x3c, 0x1b, 0x74, /*dig_again 4;ana_gain 8;  fine_gain 1.2353*/},   // 34.98
                     {{.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x2057acc8, {0x3c, 0x1b, 0x75, /*dig_again 4;ana_gain 8;  fine_gain 1.2941*/},   // 36.64
                     {{.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x20ba83b7, {0x3c, 0x1b, 0x76, /*dig_again 4;ana_gain 8;  fine_gain 1.3529*/},   // 38.31
                     {{.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x211926e3, {0x3c, 0x1b, 0x77, /*dig_again 4;ana_gain 8;  fine_gain 1.4118*/},   // 39.98
                     {{.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x2173ebae, {0x3c, 0x1b, 0x78, /*dig_again 4;ana_gain 8;  fine_gain 1.4706*/},   // 41.64
                     {{.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x21cb222d, {0x3c, 0x1b, 0x79, /*dig_again 4;ana_gain 8;  fine_gain 1.5294*/},   // 43.31
                     {{.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x221f0cd1, {0x3c, 0x1b, 0x7a, /*dig_again 4;ana_gain 8;  fine_gain 1.5882*/},   // 44.97
                     {{.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x226feb33, {0x3c, 0x1b, 0x7b, /*dig_again 4;ana_gain 8;  fine_gain 1.6471*/},   // 46.64
                     {{.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x22bdf1f8, {0x3c, 0x1b, 0x7c, /*dig_again 4;ana_gain 8;  fine_gain 1.7059*/},   // 48.31
                     {{.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x23095485, {0x3c, 0x1b, 0x7d, /*dig_again 4;ana_gain 8;  fine_gain 1.7647*/},   // 49.97
                     {{.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x23523d3a, {0x3c, 0x1b, 0x7e, /*dig_again 4;ana_gain 8;  fine_gain 1.8235*/},   // 51.64
                     {{.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
        {0x2398d643, {0x3c, 0x1b, 0x7f, /*dig_again 4;ana_gain 8;  fine_gain 1.8824*/},   // 53.30
                     {{.opt = GADI_GAIN_OPT_READ_AND_CHECK, .skip_frame=1,},
                      {.opt = GADI_GAIN_OPT_SKIP,},
                      {.opt = GADI_GAIN_OPT_SET_NOW,},},},
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
            // 0x1C < {0x3911}
            {0x1C,},
            // 0x16 < {0x3911} <= 0x1C
            {0x15, {0x00,},{{.opt = GADI_GAIN_OPT_SKIP},},},
            // {0x3911} <= 0x15
            {0x14,},
        },
        .temperatue_control             =
        {
            0x7f, 0x7e, 0x7d, 0x7c, 0x7b, 0x7a, 0x79, 0x78, 0x77, 0x76, 0x75, 0x74, 0x73, 0x72, 0x71, 0x70,
            0x6f, 0x6e, 0x6d, 0x6c, 0x6b, 0x6a, 0x69, 0x68, 0x67, 0x66, 0x65, 0x64, 0x63, 0x62, 0x61, 0x60,
            0x5f, 0x5e, 0x5d, 0x5c, 0x5b, 0x5a, 0x59, 0x58, 0x57, 0x56, 0x55, 0x54, 0x53, 0x52, 0x51, 0x50,
            0x4f, 0x4e, 0x4d, 0x4c, 0x4b, 0x4a, 0x49, 0x48, 0x47, 0x46, 0x45, 0x44, 0x43, 0x42, 0x41, 0x40,
            0x3f, 0x3e, 0x3d, 0x3c, 0x3b, 0x3a, 0x39, 0x38, 0x37, 0x36, 0x35, 0x34, 0x33, 0x32, 0x31, 0x30,
            0x2f, 0x2e, 0x2d, 0x2c, 0x2b, 0x2a, 0x29, 0x28, 0x27, 0x26, 0x25, 0x24, 0x23, 0x22, 0x21, 0x20,
            0x1f, 0x1e, 0x1d, 0x1c, 0x1b, 0x1a, 0x19, 0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11, 0x10,
        },
    },
    .magic_end  = GADI_SENSOR_HW_INFO_MAGIC_END,
};

