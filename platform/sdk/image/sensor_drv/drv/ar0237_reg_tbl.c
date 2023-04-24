/*!
*****************************************************************************
** \file        ar0237_reg_tbl.c
**
** \version     $Id: ar0237_reg_tbl.c 11627 2017-03-10 10:58:12Z dengbiao $
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
GADI_VI_SensorDrvInfoT    ar0237_sensor =
{
    .magic_start        = GADI_SENSOR_HW_INFO_MAGIC_START,
    .version            = GADI_SENSOR_HW_INFO_VERSION,
    .length             = sizeof(GADI_VI_SensorDrvInfoT),
    .file_date          = 0x20161014,
    .HwInfo         =
    {
        .name               = "ar0237",
        .ctl_type           = GADI_SENSOR_CTL_I2C,
        .addr_len           = sizeof(GADI_U16),
        .data_len           = sizeof(GADI_U16),
        .reset_active_level = GADI_GPIO_LOW,
        .hw_addr            = {(0x20>>1), (0x30>>1), 0, 0},
        .id_reg =
        {
            {0x3000, 0x0256},   // AR0237_CHIP_VERSION_REG
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        .reset_reg =
        {
            {0x301A, 0x0001}, // soft reset
            {GADI_VI_SENSOR_TABLE_FLAG_MSLEEP, 200},    // msleep(200);
            {0x301A, 0x10D8}, // soft reset
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
    .sensor_id          = GADI_ISP_SENSOR_AR0237,
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
    .max_width          = 1412,
    .max_height         = 1016,
    .def_sh_time        = GADI_VIDEO_FPS(50),
    .fmt_num            = 8,
    .auto_fmt           = GADI_VIDEO_MODE(1920, 1080,  25, 1),
    .init_reg           =
    {
        {0x301A, 0x0001},
        {GADI_VI_SENSOR_TABLE_FLAG_SLEEP, 100},
        {0x301A, 0x10D8},
        {0x3088, 0x8000},
        {0x3086, 0x4558},
        {0x3086, 0x72A6},
        {0x3086, 0x4A31},
        {0x3086, 0x4342},
        {0x3086, 0x8E03},
        {0x3086, 0x2A14},
        {0x3086, 0x4578},
        {0x3086, 0x7B3D},
        {0x3086, 0xFF3D},
        {0x3086, 0xFF3D},
        {0x3086, 0xEA2A},
        {0x3086, 0x043D},
        {0x3086, 0x102A},
        {0x3086, 0x052A},
        {0x3086, 0x1535},
        {0x3086, 0x2A05},
        {0x3086, 0x3D10},
        {0x3086, 0x4558},
        {0x3086, 0x2A04},
        {0x3086, 0x2A14},
        {0x3086, 0x3DFF},
        {0x3086, 0x3DFF},
        {0x3086, 0x3DEA},
        {0x3086, 0x2A04},
        {0x3086, 0x622A},
        {0x3086, 0x288E},
        {0x3086, 0x0036},
        {0x3086, 0x2A08},
        {0x3086, 0x3D64},
        {0x3086, 0x7A3D},
        {0x3086, 0x0444},
        {0x3086, 0x2C4B},
        {0x3086, 0xA403},
        {0x3086, 0x430D},
        {0x3086, 0x2D46},
        {0x3086, 0x4316},
        {0x3086, 0x2A90},
        {0x3086, 0x3E06},
        {0x3086, 0x2A98},
        {0x3086, 0x5F16},
        {0x3086, 0x530D},
        {0x3086, 0x1660},
        {0x3086, 0x3E4C},
        {0x3086, 0x2904},
        {0x3086, 0x2984},
        {0x3086, 0x8E03},
        {0x3086, 0x2AFC},
        {0x3086, 0x5C1D},
        {0x3086, 0x5754},
        {0x3086, 0x495F},
        {0x3086, 0x5305},
        {0x3086, 0x5307},
        {0x3086, 0x4D2B},
        {0x3086, 0xF810},
        {0x3086, 0x164C},
        {0x3086, 0x0955},
        {0x3086, 0x562B},
        {0x3086, 0xB82B},
        {0x3086, 0x984E},
        {0x3086, 0x1129},
        {0x3086, 0x9460},
        {0x3086, 0x5C19},
        {0x3086, 0x5C1B},
        {0x3086, 0x4548},
        {0x3086, 0x4508},
        {0x3086, 0x4588},
        {0x3086, 0x29B6},
        {0x3086, 0x8E01},
        {0x3086, 0x2AF8},
        {0x3086, 0x3E02},
        {0x3086, 0x2AFA},
        {0x3086, 0x3F09},
        {0x3086, 0x5C1B},
        {0x3086, 0x29B2},
        {0x3086, 0x3F0C},
        {0x3086, 0x3E03},
        {0x3086, 0x3E15},
        {0x3086, 0x5C13},
        {0x3086, 0x3F11},
        {0x3086, 0x3E0F},
        {0x3086, 0x5F2B},
        {0x3086, 0x902B},
        {0x3086, 0x803E},
        {0x3086, 0x062A},
        {0x3086, 0xF23F},
        {0x3086, 0x103E},
        {0x3086, 0x0160},
        {0x3086, 0x29A2},
        {0x3086, 0x29A3},
        {0x3086, 0x5F4D},
        {0x3086, 0x1C2A},
        {0x3086, 0xFA29},
        {0x3086, 0x8345},
        {0x3086, 0xA83E},
        {0x3086, 0x072A},
        {0x3086, 0xFB3E},
        {0x3086, 0x2945},
        {0x3086, 0x8824},
        {0x3086, 0x3E08},
        {0x3086, 0x2AFA},
        {0x3086, 0x5D29},
        {0x3086, 0x9288},
        {0x3086, 0x102B},
        {0x3086, 0x048B},
        {0x3086, 0x1686},
        {0x3086, 0x8D48},
        {0x3086, 0x4D4E},
        {0x3086, 0x2B80},
        {0x3086, 0x4C0B},
        {0x3086, 0x3F36},
        {0x3086, 0x2AF2},
        {0x3086, 0x3F10},
        {0x3086, 0x3E01},
        {0x3086, 0x6029},
        {0x3086, 0x8229},
        {0x3086, 0x8329},
        {0x3086, 0x435C},
        {0x3086, 0x155F},
        {0x3086, 0x4D1C},
        {0x3086, 0x2AFA},
        {0x3086, 0x4558},
        {0x3086, 0x8E00},
        {0x3086, 0x2A98},
        {0x3086, 0x3F0A},
        {0x3086, 0x4A0A},
        {0x3086, 0x4316},
        {0x3086, 0x0B43},
        {0x3086, 0x168E},
        {0x3086, 0x032A},
        {0x3086, 0x9C45},
        {0x3086, 0x783F},
        {0x3086, 0x072A},
        {0x3086, 0x9D3E},
        {0x3086, 0x305D},
        {0x3086, 0x2944},
        {0x3086, 0x8810},
        {0x3086, 0x2B04},
        {0x3086, 0x530D},
        {0x3086, 0x4558},
        {0x3086, 0x3E08},
        {0x3086, 0x8E01},
        {0x3086, 0x2A98},
        {0x3086, 0x8E00},
        {0x3086, 0x76A7},
        {0x3086, 0x77A7},
        {0x3086, 0x4644},
        {0x3086, 0x1616},
        {0x3086, 0xA57A},
        {0x3086, 0x1244},
        {0x3086, 0x4B18},
        {0x3086, 0x4A04},
        {0x3086, 0x4316},
        {0x3086, 0x0643},
        {0x3086, 0x1605},
        {0x3086, 0x4316},
        {0x3086, 0x0743},
        {0x3086, 0x1658},
        {0x3086, 0x4316},
        {0x3086, 0x5A43},
        {0x3086, 0x1645},
        {0x3086, 0x588E},
        {0x3086, 0x032A},
        {0x3086, 0x9C45},
        {0x3086, 0x787B},
        {0x3086, 0x3F07},
        {0x3086, 0x2A9D},
        {0x3086, 0x530D},
        {0x3086, 0x8B16},
        {0x3086, 0x863E},
        {0x3086, 0x2345},
        {0x3086, 0x5825},
        {0x3086, 0x3E10},
        {0x3086, 0x8E01},
        {0x3086, 0x2A98},
        {0x3086, 0x8E00},
        {0x3086, 0x3E10},
        {0x3086, 0x8D60},
        {0x3086, 0x1244},
        {0x3086, 0x4BB9},
        {0x3086, 0x2C2C},
        {0x3086, 0x2C2C},
        {0x3086, 0x2C2C},
        {0x301A, 0x10D8},
        {0x30B0, 0x1A38},
        {0x31AC, 0x0C0C},
        {0x3036, 0x000C},
        {0x3038, 0x0001},
        {0x30A2, 0x0001},
        {0x30AE, 0x0001},
        {0x30A8, 0x0001},
        {0x31AE, 0x0301},
        {0x3082, 0x0009},
        {0x30BA, 0x760C},
        {0x31D0, 0x0000},
        {0x3064, 0x1982},
        {0x3064, 0x1802},
        {0x3EEE, 0xA0AA},
        {0x30BA, 0x762C},
        {0x3F4A, 0x0F70},
        {0x309E, 0x016C},
        {0x3092, 0x006F},
        {0x3EE4, 0x9937},
        {0x3EE6, 0x3863},
        {0x3EEC, 0x3B0C},
        {0x30B0, 0x1A3A},
        {0x30B0, 0x1A3A},
        {0x30BA, 0x762C},
        {0x30B0, 0x1A3A},
        {0x30B0, 0x0A3A},
        {0x3EEA, 0x2838},
        {0x3ECC, 0x4E2D},
        {0x3ED2, 0xFEA6},
        {0x3ED6, 0x2CB3},
        {0x3EEA, 0x2819},
        {0x30B0, 0x0A3A},
        {0x306E, 0x2418},
        {0x301A, 0x10DC},
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .pll_table =
    {
        [0] =
        {
            // for 1080P@30, 1080P@25, 720P@60, 720P@50
            .pixclk = 74250000, // pixclk = ((extclk / AR0237_PRE_PLL_CLK_DIV / AR0237_VT_PIX_CLK_DIV) * AR0237_PLL_MULTIPLIER / AR0237_VT_SYS_CLK_DIV
            .extclk = 27000000, //        = (27000000 / 2 / 8) * 44 / 1 = 74250000
            .regs =
            {
                {0x302A, 0x08}, // AR0237_VT_PIX_CLK_DIV(4-16)
                {0x302C, 0x01}, // AR0237_VT_SYS_CLK_DIV(1,2,4,6,8,10,12,14,16)
                {0x302E, 0x02}, // AR0237_PRE_PLL_CLK_DIV(1-64)
                {0x3030, 0x2C}, // AR0237_PLL_MULTIPLIER(32-384)
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
        [1] =
        {
            // for 960P@30, 960P@25
            .pixclk = 54000000, // pixclk = ((extclk / AR0237_PRE_PLL_CLK_DIV / AR0237_VT_PIX_CLK_DIV) * AR0237_PLL_MULTIPLIER / AR0237_VT_SYS_CLK_DIV
            .extclk = 27000000, //        = (27000000 / 2 / 10) * 40 / 1 = 54000000
            .regs =
            {
                {0x302A, 0x0A}, // AR0237_VT_PIX_CLK_DIV(4-16)
                {0x302C, 0x01}, // AR0237_VT_SYS_CLK_DIV(1,2,4,6,8,10,12,14,16)
                {0x302E, 0x02}, // AR0237_PRE_PLL_CLK_DIV(1-64)
                {0x3030, 0x28}, // AR0237_PLL_MULTIPLIER(32-384)
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
        [2] =
        {
            // for 720P@30, 720P@25
            .pixclk = 37125000, // pixclk = ((extclk / AR0237_PRE_PLL_CLK_DIV / AR0237_VT_PIX_CLK_DIV) * AR0237_PLL_MULTIPLIER / AR0237_VT_SYS_CLK_DIV
            .extclk = 27000000, //        = (27000000 / 4 / 8) * 44 / 1 = 37125000
            .regs =
            {
                {0x302A, 0x08}, // AR0237_VT_PIX_CLK_DIV(4-16)
                {0x302C, 0x01}, // AR0237_VT_SYS_CLK_DIV(1,2,4,6,8,10,12,14,16)
                {0x302E, 0x04}, // AR0237_PRE_PLL_CLK_DIV(1-64)
                {0x3030, 0x2C}, // AR0237_PLL_MULTIPLIER(32-384)
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
    },
    .video_fmt_table =
    {
        [0] = // 1920x1080P@30fps   (1100 * 2) * 1125 * 30 = 74250000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1920, 1080,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x300C, 0x044C},   //AR0237_LINE_LENGTH_PCK        1100 = 0x044C
                {0x300A, 0x0465},   //AR0237_FRAME_LENGTH_LINES     1125 = 0x0465
                {0x3002, 0x0004},   //AR0237_Y_ADDR_START,          4
                {0x3004, 0x000C},   //AR0237_X_ADDR_START,          12
                {0x3006, 0x043B},   //AR0237_Y_ADDR_END,            0x043B = 1083 -  4 + 1 = 1080
                {0x3008, 0x078B},   //AR0237_X_ADDR_END,            0x078B = 1931 - 12 + 1 = 1920
                {0x30A6, 0x0001},   //AR0237_Y_ODD_INC,
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 1920,
                .height     = 1080,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_DVP,
                .bits       = GADI_VIDEO_BITS_12,
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
                .sync_start     = (0 + 0),
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_GR,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_GR,
                    GADI_VI_BAYER_PATTERN_GR,
                    GADI_VI_BAYER_PATTERN_GR,
                    GADI_VI_BAYER_PATTERN_GR,
                },
            },
            .update_after_vsync_start   = 30,
            .update_after_vsync_end     = 33,
        },
        [1] = // 1920x1080P@25fps   (1320 * 2) * 1125 * 25 = 74250000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1920, 1080,  25, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x300C, 0x0528},   //AR0237_LINE_LENGTH_PCK        1320 = 0x0528
                {0x300A, 0x0465},   //AR0237_FRAME_LENGTH_LINES     1125 = 0x0465
                {0x3002, 0x0004},   //AR0237_Y_ADDR_START,          4
                {0x3004, 0x000C},   //AR0237_X_ADDR_START,          12
                {0x3006, 0x043B},   //AR0237_Y_ADDR_END,            0x043B = 1083 -  4 + 1 = 1080
                {0x3008, 0x078B},   //AR0237_X_ADDR_END,            0x078B = 1931 - 12 + 1 = 1920
                {0x30A6, 0x0001},   //AR0237_Y_ODD_INC,
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 1920,
                .height     = 1080,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_DVP,
                .bits       = GADI_VIDEO_BITS_12,
                .ratio      = GADI_VIDEO_RATIO_16_9,
                .srm        = 0,
                .pll_index  = 0,
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
                .sync_start     = (0 + 0),
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_GR,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_GR,
                    GADI_VI_BAYER_PATTERN_GR,
                    GADI_VI_BAYER_PATTERN_GR,
                    GADI_VI_BAYER_PATTERN_GR,
                },
            },
            .update_after_vsync_start   = 37,
            .update_after_vsync_end     = 40,
        },
        [2] = // 1280x960P@30fps    (900 * 2) * 1000 * 30 = 54000000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  960,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x300C, 0x0384},   //AR0237_LINE_LENGTH_PCK         900 = 0x0384
                {0x300A, 0x03E8},   //AR0237_FRAME_LENGTH_LINES     1000 = 0x03E8
                {0x3002, 0x0004},   //AR0237_Y_ADDR_START,          4
                {0x3004, 0x000C},   //AR0237_X_ADDR_START,          12
                {0x3006, 0x03C3},   //AR0237_Y_ADDR_END,            0x03C3 =  963 -  4 + 1 = 960
                {0x3008, 0x050B},   //AR0237_X_ADDR_END,            0x050B = 1291 - 12 + 1 = 1280
                {0x30A6, 0x0001},   //AR0237_Y_ODD_INC,
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
                .def_start_x    = 0,
                .def_start_y    = 0,
                .def_width      = 1280,
                .def_height     = 960,
                .sync_start     = (0 + 0),
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_GR,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_GR,
                    GADI_VI_BAYER_PATTERN_GR,
                    GADI_VI_BAYER_PATTERN_GR,
                    GADI_VI_BAYER_PATTERN_GR,
                },
            },
            .update_after_vsync_start   = 30,
            .update_after_vsync_end     = 33,
        },
        [3] = // 1280x960P@25fps    (1080 * 2) * 1000 * 25 = 54000000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  960,  25, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x300C, 0x0438},   //AR0237_LINE_LENGTH_PCK        1080 = 0x0438
                {0x300A, 0x03E8},   //AR0237_FRAME_LENGTH_LINES     1000 = 0x03E8
                {0x3002, 0x0004},   //AR0237_Y_ADDR_START,          4
                {0x3004, 0x000C},   //AR0237_X_ADDR_START,          12
                {0x3006, 0x03C3},   //AR0237_Y_ADDR_END,            0x03C3 =  963 -  4 + 1 = 960
                {0x3008, 0x050B},   //AR0237_X_ADDR_END,            0x050B = 1291 - 12 + 1 = 1280
                {0x30A6, 0x0001},   //AR0237_Y_ODD_INC,
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
                .def_start_x    = 0,
                .def_start_y    = 0,
                .def_width      = 1280,
                .def_height     = 960,
                .sync_start     = (0 + 0),
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_GR,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_GR,
                    GADI_VI_BAYER_PATTERN_GR,
                    GADI_VI_BAYER_PATTERN_GR,
                    GADI_VI_BAYER_PATTERN_GR,
                },
            },
            .update_after_vsync_start   = 37,
            .update_after_vsync_end     = 40,
        },
        [4] = // 1280x720P@60fps    (824 * 2) * 751 * 60 = 74258880 // 74250000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  60, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                // 0x300C Only even values are allowed.
                {0x300C, 0x0338},   //AR0237_LINE_LENGTH_PCK         825 = 0x0339 --> 0x0338
                {0x300A, 0x02EF},   //AR0237_FRAME_LENGTH_LINES      750 = 0x02EE --> 0x02EF
                {0x3002, 0x0004},   //AR0237_Y_ADDR_START,          4
                {0x3004, 0x000C},   //AR0237_X_ADDR_START,          12
                {0x3006, 0x02D3},   //AR0237_Y_ADDR_END,            0x02D3 =  723 -  4 + 1 = 720
                {0x3008, 0x050B},   //AR0237_X_ADDR_END,            0x050B = 1291 - 12 + 1 = 1280
                {0x30A6, 0x0001},   //AR0237_Y_ODD_INC,
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
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_GR,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_GR,
                    GADI_VI_BAYER_PATTERN_GR,
                    GADI_VI_BAYER_PATTERN_GR,
                    GADI_VI_BAYER_PATTERN_GR,
                },
            },
            .update_after_vsync_start   = 13,
            .update_after_vsync_end     = 16,
        },
        [5] = // 1280x720P@50fps    (990 * 2) * 750 * 50 = 74250000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  50, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x300C, 0x03DE},   //AR0237_LINE_LENGTH_PCK         990 = 0x03DE
                {0x300A, 0x02EE},   //AR0237_FRAME_LENGTH_LINES      750 = 0x02EE
                {0x3002, 0x0004},   //AR0237_Y_ADDR_START,          4
                {0x3004, 0x000C},   //AR0237_X_ADDR_START,          12
                {0x3006, 0x02D3},   //AR0237_Y_ADDR_END,            0x02D3 =  723 -  4 + 1 = 720
                {0x3008, 0x050B},   //AR0237_X_ADDR_END,            0x050B = 1291 - 12 + 1 = 1280
                {0x30A6, 0x0001},   //AR0237_Y_ODD_INC,
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
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_GR,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_GR,
                    GADI_VI_BAYER_PATTERN_GR,
                    GADI_VI_BAYER_PATTERN_GR,
                    GADI_VI_BAYER_PATTERN_GR,
                },
            },
            .update_after_vsync_start   = 17,
            .update_after_vsync_end     = 20,
        },
        [6] = // 1280x720P@30fps    (824 * 2) * 751 * 30 = 37129440 // 37125000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                // 0x300C Only even values are allowed.
                {0x300C, 0x0338},   //AR0237_LINE_LENGTH_PCK         825 = 0x0339 --> 0x0338
                {0x300A, 0x02EF},   //AR0237_FRAME_LENGTH_LINES      750 = 0x02EE --> 0x02EF
                {0x3002, 0x0004},   //AR0237_Y_ADDR_START,          4
                {0x3004, 0x000C},   //AR0237_X_ADDR_START,          12
                {0x3006, 0x02D3},   //AR0237_Y_ADDR_END,            0x02D3 =  723 -  4 + 1 = 720
                {0x3008, 0x050B},   //AR0237_X_ADDR_END,            0x050B = 1291 - 12 + 1 = 1280
                {0x30A6, 0x0001},   //AR0237_Y_ODD_INC,
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
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_GR,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_GR,
                    GADI_VI_BAYER_PATTERN_GR,
                    GADI_VI_BAYER_PATTERN_GR,
                    GADI_VI_BAYER_PATTERN_GR,
                },
            },
            .update_after_vsync_start   = 30,
            .update_after_vsync_end     = 33,
        },
        [7] = // 1280x720P@25fps    (990 * 2) * 750 * 25 = 37125000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  25, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x300C, 0x03DE},   //AR0237_LINE_LENGTH_PCK         990 = 0x03DE
                {0x300A, 0x02EE},   //AR0237_FRAME_LENGTH_LINES      750 = 0x02EE
                {0x3002, 0x0004},   //AR0237_Y_ADDR_START,          4
                {0x3004, 0x000C},   //AR0237_X_ADDR_START,          12
                {0x3006, 0x02D3},   //AR0237_Y_ADDR_END,            0x02D3 =  723 -  4 + 1 = 720
                {0x3008, 0x050B},   //AR0237_X_ADDR_END,            0x050B = 1291 - 12 + 1 = 1280
                {0x30A6, 0x0001},   //AR0237_Y_ODD_INC,
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
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_GR,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_GR,
                    GADI_VI_BAYER_PATTERN_GR,
                    GADI_VI_BAYER_PATTERN_GR,
                    GADI_VI_BAYER_PATTERN_GR,
                },
            },
            .update_after_vsync_start   = 37,
            .update_after_vsync_end     = 40,
        },
    },
    .mirror_table =
    {
#define AR0237_HORIZ_MIRROR     0x4000
#define AR0237_VERT_FLIP        0x8000
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY_VERTICALLY] =
        {
            {0x3040, AR0237_HORIZ_MIRROR, AR0237_HORIZ_MIRROR},
            {0x301A, 0x10D8},
            {0x30D4, 0x6007},
            {0x301A, 0x10DC},
            {GADI_VI_SENSOR_TABLE_FLAG_MSLEEP, 400},
            {0x301A, 0x10D8},
            {0x30D4, 0xE007},
            {0x301A, 0x10DC},
            {0x3040, AR0237_VERT_FLIP, AR0237_VERT_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY] =
        {
            {0x3040, AR0237_HORIZ_MIRROR, AR0237_HORIZ_MIRROR},
            {0x301A, 0x10D8},
            {0x30D4, 0x6007},
            {0x301A, 0x10DC},
            {GADI_VI_SENSOR_TABLE_FLAG_MSLEEP, 400},
            {0x301A, 0x10D8},
            {0x30D4, 0xE007},
            {0x301A, 0x10DC},
            {0x3040, 0x00, AR0237_VERT_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_VERTICALLY] =
        {
            {0x3040, 0x00, AR0237_HORIZ_MIRROR},
            {0x301A, 0x10D8},
            {0x30D4, 0x6007},
            {0x301A, 0x10DC},
            {GADI_VI_SENSOR_TABLE_FLAG_MSLEEP, 400},
            {0x301A, 0x10D8},
            {0x30D4, 0xE007},
            {0x301A, 0x10DC},
            {0x3040, AR0237_VERT_FLIP, AR0237_VERT_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_NONE] =
        {
            {0x3040, 0x00, AR0237_HORIZ_MIRROR},
            {0x301A, 0x10D8},
            {0x30D4, 0x6007},
            {0x301A, 0x10DC},
            {GADI_VI_SENSOR_TABLE_FLAG_MSLEEP, 400},
            {0x301A, 0x10D8},
            {0x30D4, 0xE007},
            {0x301A, 0x10DC},
            {0x3040, 0x00, AR0237_VERT_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
    },
    .version_reg =
    {
        {0x3000, 0x00}, // AR0237_CHIP_VERSION_REG
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .reg_ctl_by_ver_num = 0,
    .control_by_version_reg = {0},
    .vmax_clk_ref   = GADI_SENSOR_CAL_CLK_PIXCLK_DIV2,
    .shs_clk_ref    = GADI_SENSOR_CAL_CLK_PIXCLK_DIV2,
    .cal_shs_mode   = GADI_SENSOR_CAL_SHS_VMAX_MODE,
    .shs_fix        = 0,
    .shs_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_SET,
    .agc_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_SET,
    .vmax_reg_update_mode       = GADI_SENSOR_UPDATE_REG_BY_SET,
    .mirror_reg_update_mode     = GADI_SENSOR_UPDATE_REG_BY_SET,
    .shs_additional_use         = 0,
    .shs_additional_percent     = 0,
    .hmax_reg =
    {
        {0x300C, 0x0000, 0xFFFF, 0x00, 0x00, 0}, // AR0237_LINE_LENGTH_PCK
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .vmax_reg =
    {
        {0x300A, 0x0000, 0xFFFF, 0x00, 0x00, 0}, // AR0237_FRAME_LENGTH_LINES
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .vmax_wadd_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    .shs_reg =
    {
        {0x3012, 0x0000, 0xFFFF, 0x00, 0x00, 0}, // AR0237_COARSE_INTEGRATION_TIME
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .shs_additional_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    .cal_gain_mode              = GADI_SENSOR_CAL_GAIN_MODE1,
    .max_agc_index = 51,
    .max_again_index = 0,
    .cur_again_index = 0,
    .agc_info =
    {
        .db_max     = 0x21000000,    // 33dB
        .db_min     = 0x00000000,    // 0dB
        .db_step    = 0x00300000,    // 0.1875dB
    },
    .gain_reg =
    {
        {0x3100, 0x00}, // AR0237_AE_CTRL_REG
        {0x3060, 0x00}, // AR0237_DIGITAL_TEST
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .gain_table =
    {
        {0x03A30A07, {0x0000, 0x000B,},},
        {0x04151824, {0x0000, 0x000C,},},
        {0x04819561, {0x0000, 0x000D,},},
        {0x05022681, {0x0000, 0x000E,},},
        {0x057BB02D, {0x0000, 0x000F,},},
        {0x06054609, {0x0000, 0x0010,},},
        {0x06914DB6, {0x0000, 0x0012,},},
        {0x07325B90, {0x0000, 0x0014,},},
        {0x07D19676, {0x0000, 0x0016,},},
        {0x0887BCD7, {0x0000, 0x0018,},},
        {0x08A0951C, {0x0004, 0x0000,},},
        {0x08E97E57, {0x0004, 0x0001,},},
        {0x093016EB, {0x0004, 0x0002,},},
        {0x097BFCBD, {0x0004, 0x0003,},},
        {0x09CC9660, {0x0004, 0x0004,},},
        {0x0A1A5E2D, {0x0004, 0x0005,},},
        {0x0A6C3A1B, {0x0004, 0x0006,},},
        {0x0AC811D2, {0x0004, 0x0007,},},
        {0x0B2044D0, {0x0004, 0x0008,},},
        {0x0B80F636, {0x0004, 0x0009,},},
        {0x0BE34A34, {0x0004, 0x000A,},},
        {0x0C46DEC0, {0x0004, 0x000B,},},
        {0x0CB5AD41, {0x0004, 0x000C,},},
        {0x0D2904B2, {0x0004, 0x000D,},},
        {0x0D9FF483, {0x0004, 0x000E,},},
        {0x0E1E05AF, {0x0004, 0x000F,},},
        {0x0EA5DB26, {0x0004, 0x0010,},},
        {0x0F355CF5, {0x0004, 0x0012,},},
        {0x0FCE4291, {0x0004, 0x0014,},},
        {0x1074D8C9, {0x0004, 0x0016,},},
        {0x11258AD9, {0x0004, 0x0018,},},
        {0x11E8903D, {0x0004, 0x001A,},},
        {0x12BAF34A, {0x0004, 0x001C,},},
        {0x13A53A8C, {0x0004, 0x001E,},},
        {0x14AB212F, {0x0004, 0x0020,},},
        {0x153AA2FE, {0x0004, 0x0022,},},
        {0x15D3889B, {0x0004, 0x0024,},},
        {0x167872A9, {0x0004, 0x0026,},},
        {0x172AD0E2, {0x0004, 0x0028,},},
        {0x17EC6C0C, {0x0004, 0x002A,},},
        {0x18C03954, {0x0004, 0x002C,},},
        {0x19AA8096, {0x0004, 0x002E,},},
        {0x1AB06738, {0x0004, 0x0030,},},
        {0x1B40E00A, {0x0004, 0x0032,},},
        {0x1BD9B53E, {0x0004, 0x0034,},},
        {0x1C7E8ED1, {0x0004, 0x0036,},},
        {0x1D2F513C, {0x0004, 0x0038,},},
        {0x1DF2673A, {0x0004, 0x003A,},},
        {0x1EC57F5D, {0x0004, 0x003C,},},
        {0x1FAFC69F, {0x0004, 0x003E,},},
        {0x20B5AD41, {0x0004, 0x0040,},},
    },
    .temperatue_gain_table =
    {
        .agc_control_by_temperatue      = 0,
        .Aptina_temperatue_mode         = 1,
        .temperatue_control_reg_index   = 0,
        .temperatue_gain_num            = 0,
        .temperatue_reg                 = {0x30B2, 0x0000, 0x3FFF},// must to be fix
        .gain_reg                       = {GADI_VI_SENSOR_TABLE_FLAG_END,},
        .temperatue_gain                = {0},
        .temperatue_control             = {0},
    },
    .magic_end  = GADI_SENSOR_HW_INFO_MAGIC_END,
};

