/*!
*****************************************************************************
** \file        sc1235_reg_tbl.c
**
** \version     $Id: sc1235_reg_tbl.c 12538 2017-08-10 07:26:35Z yulindeng $
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
GADI_VI_SensorDrvInfoT    sc1235_sensor =
{
    .magic_start        = GADI_SENSOR_HW_INFO_MAGIC_START,
    .version            = GADI_SENSOR_HW_INFO_VERSION,
    .length             = sizeof(GADI_VI_SensorDrvInfoT),
    .file_date          = 0x20170810,
    .HwInfo         =
    {
        .name               = "sc1235",
        .ctl_type           = GADI_SENSOR_CTL_I2C,
        .addr_len           = sizeof(GADI_U16),
        .data_len           = sizeof(GADI_U8),
        .reset_active_level = GADI_GPIO_LOW,
        .hw_addr            = {(0x60>>1), 0, 0, 0},
        .id_reg =
        {
            {0x3107, 0x12},
            {0x3108, 0x35},
        },
        .reset_reg =
        {
            {0x0103, 0x01}, // soft reset
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
    .sensor_id          = GADI_ISP_SENSOR_SC1235,
    .sensor_double_step = 32,
    .typeofsensor       = 2,
    .sync_time_delay    = 40,
    .source_type.dummy  = GADI_VI_CMOS_CHANNEL_TYPE_AUTO,
    .dev_type           = GADI_VI_DEV_TYPE_CMOS,
    .video_system       = GADI_VIDEO_SYSTEM_AUTO,
    .vs_polarity        = GADI_VI_FALLING_EDGE,
    .hs_polarity        = GADI_VI_RISING_EDGE,
    .data_edge          = GADI_VI_FALLING_EDGE,
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
        {0x0100, 0x00},
        {0x3034, 0x05},
        {0x3038, 0xFF},  //light
        {0x3235, 0x03},
        {0x3236, 0xe6}, // vts-2
        //low power
        {0x3302, 0xFF},
        {0x3303, 0x28},
        {0x3305, 0x00},
        {0x3306, 0x68},
        {0x3309, 0xA8},
        {0x330A, 0x01},
        {0x330B, 0x6C},
        {0x330E, 0x30},
        {0x331D, 0x0A}, //value
        {0x331F, 0x8D},
        {0x3321, 0x8F},
        //low power
        {0x333A, 0x0A}, //[7:4] falling edge of stg1 samp MSB
        {0x333B, 0x00}, //[3:0] msb
        {0x3357, 0x5A}, //[3:0] lsb
        {0x335C, 0x57}, //[3:0] falling edge of stg1 samp LSB
        {0x335E, 0x01},
        {0x335F, 0x03},
        {0x3364, 0x05},// [2] 1: write at sampling ending
        {0x3366, 0x7C}, // div_rst gap
        {0x3367, 0x08},
        {0x3368, 0x02},
        {0x3369, 0x00},
        {0x336A, 0x00},
        {0x336B, 0x00},
        {0x337C, 0x04},
        {0x337D, 0x06},
        {0x337F, 0x03}, // [1:0] 11:new auto precharge  330e in 3372    [7:6] 11: close div_rst 00:open div_rstnew auto precharge  330e in 3372
        {0x33A0, 0x05},
        {0x33B5, 0x10},
        {0x3621, 0x28},
        {0x3622, 0x02},
        {0x3625, 0x01},
        {0x3630, 0xA8},
        {0x3634, 0x21},
        {0x3635, 0xA0}, //bypass txvdd to AVDD
        {0x3636, 0x25},
        {0x3637, 0x0E},
        {0x3638, 0x0F},
        {0x3639, 0x0C},
        {0x363A, 0x1F},
        {0x363B, 0x09}, //hvdd
        {0x363C, 0x06}, //NVDD -0.8V
        {0x366E, 0x08},  // ofs auto en [3]
        {0x366F, 0x2F},  // ofs+finegain  real ofs in 0x3687[4:0]
        //{0x3670, 0x08},  //[3]:3633 logic ctrl  real value in 3682
        //{0x3677, 0x2F},  //<gain0
        //{0x3678, 0x23},  //gain0 - gain1
        //{0x3679, 0x22},  //>gain1
        //{0x367E, 0x07},  //gain0
        //{0x367F, 0x0F},  //gain1
        {0x3802, 0x01}, //digital ctrl
        {0x3908, 0x11},
        {0x391B, 0x4D}, //high temp
        {0x391E, 0x00}, //[2] 1:enable
        {0x3D08, 0x00},
        {0x3E23, 0x07}, //gain set
        {0x3E24, 0x10},
        {0x3f00, 0x07}, //digital ctrl// bit[2] = 1
        {0x4500, 0x59},
        {0x0100, 0x01},
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .pll_table =
    {
        // PreDIV[2:0] = 1/1.5/2/3/4/5/6/7
        // SYSEL[2:0]  = 0/1/2/3/4/5/6/7
        [0] =
        {
            // for 720P@60, 720P@50  1650 * 750 * 60 = 74250000
            .pixclk = 74250000, // pixclk = (extclk * (64 - PLLDIV[5:0])) / (PreDIV[2:0] * (SYSEL[2:0] + 1))
            .extclk = 27000000, //        = 27 * (64 - 42) / (2 * (3 + 1)) = 74.25MHz
            .regs =
            {
                {0x3039, 0x35}, // 0 011 010 1 SYSEL[2:0] = b011, PreDIV[2:0] = b010 2x, PLLDIV[5] = b1
                {0x303A, 0x56}, // 01010 110 PLLDIV[4:0] = b101010
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
        [1] =
        {
            // for 960P@30, 960@25
            .pixclk = 54000000, // pixclk = (extclk * (64 - PLLDIV[5:0])) / (PreDIV[2:0] * (SYSEL[2:0] + 1))
            .extclk = 27000000, //        = 27 * (64 - 56) / (1 * (3 + 1)) = 54MHz
            .regs =
            {
                {0x3039, 0x31}, // 0 011 000 1 SYSEL[2:0] = b011, PreDIV[2:0] = b000 1x, PLLDIV[5] = b1
                {0x303A, 0xC6}, // 11000 110 PLLDIV[4:0] = b111000
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
        [2] =
        {
            // for 720P@30, 720P@25
            .pixclk = 37125000, // pixclk = (extclk * (64 - PLLDIV[5:0])) / (PreDIV[2:0] * (SYSEL[2:0] + 1))
            .extclk = 27000000, //        = 27 * (64 - 42) / (4 * (3 + 1)) = 37.125MHz
            .regs =
            {
                {0x3039, 0x39}, // 0 011 100 1 SYSEL[2:0] = b011, PreDIV[2:0] = b100 4x, PLLDIV[5] = b1
                {0x303A, 0x56}, // 01010 110 PLLDIV[4:0] = b101010
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
                //{0x3202, 0x00}, // TIMING_Y_START_H
                //{0x3203, 0x08}, // TIMING_Y_START_L
                //{0x3206, 0x03}, // TIMING_Y_END_H
                //{0x3207, 0xCF}, // TIMING_Y_END_L
                // config Output image size
                {0x3208, 0x05}, // TIMING_X_SIZE_H   1280 = 0x0500
                {0x3209, 0x00}, // TIMING_X_SIZE_L
                {0x320A, 0x03}, // TIMING_Y_SIZE_H    960 = 0x03C0
                {0x320B, 0xC0}, // TIMING_Y_SIZE_L
                // config Frame length and width
                {0x320C, 0x07}, // TIMING_HTS_H      1800 = 0x0708
                {0x320D, 0x08}, // TIMING_HTS_L
                {0x3F04, 0x06},
                {0x3F05, 0xE4}, // hts - 0x24
                //{0x320E, 0x03}, // TIMING_VTS_H      1000 = 0x03E8
                //{0x320F, 0xE8}, // TIMING_VTS_L
                // config Output window position
                {0x3210, 0x00}, // TIMING_HOFFS_H
                {0x3211, 0x08}, // TIMING_HOFFS_L
                {0x3212, 0x00}, // TIMING_VOFFS_H
                {0x3213, 0x04}, // TIMING_VOFFS_L
                // power consumption reduction & timing
                //{0x3300, 0x33},
                //{0x3301, 0x38}, // cancel aec shanshuo
                //{0x3303, 0x80},
                //{0x3304, 0x18},
                //{0x3308, 0x40}, // def: 0x00
                //{0x330D, 0x40}, // def: 0x00
                //{0x331E, 0x30},
                //{0x3320, 0x06}, // {0x3320, 0x3321} = HTS - 0x30 = 0x0708 - 0x30 = 0x06D8
                //{0x3321, 0xD8}, //
                //{0x3322, 0x01}, // def: 0x00
                //{0x3323, 0x80}, // power reduct def: 0x00
                //{0x3330, 0x0D}, // def: 0x00
                //{0x3335, 0x06}, // def: 0x1A
                //{0x3600, 0x64}, // def: 0x54
                //{0x3620, 0x42}, // def: 0x88
                //{0x3636, 0x0D}, // def: 0x00 power reduct
                //{0x5785, 0x0C}, // def: 0x40 black spot threshold*/
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
                //{0x3202, 0x00}, // TIMING_Y_START_H
                //{0x3203, 0x08}, // TIMING_Y_START_L
                //{0x3206, 0x03}, // TIMING_Y_END_H
                //{0x3207, 0xCF}, // TIMING_Y_END_L
                // config Output image size
                {0x3208, 0x05}, // TIMING_X_SIZE_H   1280 = 0x0500
                {0x3209, 0x00}, // TIMING_X_SIZE_L
                {0x320A, 0x03}, // TIMING_Y_SIZE_H    960 = 0x03C0
                {0x320B, 0xC0}, // TIMING_Y_SIZE_L
                // config Frame length and width
                {0x320C, 0x08}, // TIMING_HTS_H      2160 = 0x0870
                {0x320D, 0x70}, // TIMING_HTS_L
                {0x3F04, 0x08},
                {0x3F05, 0x4C}, // hts - 0x24
                //{0x320E, 0x03}, // TIMING_VTS_H      1000 = 0x03E8
                //{0x320F, 0xE8}, // TIMING_VTS_L
                // config Output window position
                {0x3210, 0x00}, // TIMING_HOFFS_H
                {0x3211, 0x08}, // TIMING_HOFFS_L
                {0x3212, 0x00}, // TIMING_VOFFS_H
                {0x3213, 0x04}, // TIMING_VOFFS_L
                // power consumption reduction & timing
                //{0x3300, 0x33},
                //{0x3301, 0x38}, // cancel aec shanshuo
                //{0x3303, 0x80},
                //{0x3304, 0x18},
                //{0x3308, 0x40}, // def: 0x00
                //{0x330D, 0x40}, // def: 0x00
                //{0x331E, 0x30},
                //{0x3320, 0x08}, // {0x3320, 0x3321} = HTS - 0x30 = 0x0870 - 0x30 = 0x0840
                //{0x3321, 0x40}, //
                //{0x3322, 0x01}, // def: 0x00
                //{0x3323, 0x80}, // power reduct def: 0x00
                //{0x3330, 0x0D}, // def: 0x00
                //{0x3335, 0x06}, // def: 0x1A
                //{0x3600, 0x64}, // def: 0x54
                //{0x3620, 0x42}, // def: 0x88
                //{0x3636, 0x0D}, // def: 0x00 power reduct
                //{0x5785, 0x0C}, // def: 0x40 black spot threshold
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
                {0x3F04, 0x06},
                {0x3F05, 0x4E}, // hts - 0x24
                //{0x320E, 0x02}, // TIMING_VTS_H       750 = 0x02EE
                //{0x320F, 0xEE}, // TIMING_VTS_L
                // config Output window position
                {0x3210, 0x00}, // TIMING_HOFFS_H
                {0x3211, 0x08}, // TIMING_HOFFS_L
                {0x3212, 0x00}, // TIMING_VOFFS_H
                {0x3213, 0x04}, // TIMING_VOFFS_L
                // power consumption reduction & timing
                //{0x3300, 0x22},
                //{0x3301, 0x22}, // cancel aec shanshuo
                //{0x3303, 0x60},
                //{0x3304, 0x10},
                //{0x3308, 0x40}, // def: 0x00
                //{0x330D, 0x40}, // def: 0x00
                //{0x331E, 0x50},
                //{0x3320, 0x06}, // {0x3320, 0x3321} = HTS - 0x30 = 0x0672 - 0x30 = 0x0642
                //{0x3321, 0x42}, //
                //{0x3322, 0x01}, // def: 0x00
                //{0x3323, 0x30}, // power reduct def: 0x00
                //{0x3330, 0x0D}, // def: 0x00
                //{0x3335, 0x04}, // def: 0x1A
                //{0x3600, 0x7C}, // def: 0x54
                //{0x3620, 0x64}, // def: 0x88
                //{0x3636, 0x0D}, // def: 0x00 power reduct
                //{0x5785, 0x08}, // def: 0x40 black spot threshold
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
                {0x3F04, 0x07},
                {0x3F05, 0x98}, // hts - 0x24
                //{0x320E, 0x02}, // TIMING_VTS_H       750 = 0x02EE
                //{0x320F, 0xEE}, // TIMING_VTS_L
                // config Output window position
                {0x3210, 0x00}, // TIMING_HOFFS_H
                {0x3211, 0x08}, // TIMING_HOFFS_L
                {0x3212, 0x00}, // TIMING_VOFFS_H
                {0x3213, 0x04}, // TIMING_VOFFS_L
                // power consumption reduction & timing
                //{0x3300, 0x33},
                //{0x3301, 0x38}, // cancel aec shanshuo
                //{0x3303, 0x86},
                //{0x3304, 0x18},
                //{0x3308, 0x40}, // def: 0x00
                //{0x330D, 0x40}, // def: 0x00
                //{0x331E, 0x50},
                //{0x3320, 0x07}, // {0x3320, 0x3321} = HTS - 0x30 = 0x07BC - 0x30 = 0x078C
                //{0x3321, 0x8C}, //
                //{0x3322, 0x01}, // def: 0x00
                //{0x3323, 0xC0}, // power reduct def: 0x00
                //{0x3330, 0x0D}, // def: 0x00
                //{0x3335, 0x05}, // def: 0x1A
                //{0x3600, 0x7C}, // def: 0x54
                //{0x3620, 0x44}, // def: 0x88
                //{0x3636, 0x0D}, // def: 0x00 power reduct
                //{0x5785, 0x08}, // def: 0x40 black spot threshold
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
                {0x3F04, 0x06},
                {0x3F05, 0x4E}, // hts - 0x24
                //{0x320E, 0x02}, // TIMING_VTS_H       750 = 0x02EE
                //{0x320F, 0xEE}, // TIMING_VTS_L
                // config Output window position
                {0x3210, 0x00}, // TIMING_HOFFS_H
                {0x3211, 0x08}, // TIMING_HOFFS_L
                {0x3212, 0x00}, // TIMING_VOFFS_H
                {0x3213, 0x04}, // TIMING_VOFFS_L
                // power consumption reduction & timing
                //{0x3300, 0x22},
                //{0x3301, 0x22}, // cancel aec shanshuo
                //{0x3303, 0x50},
                //{0x3304, 0x18},
                //{0x3308, 0x40}, // def: 0x00
                //{0x330D, 0x40}, // def: 0x00
                //{0x331E, 0x50},
                //{0x3320, 0x06}, // {0x3320, 0x3321} = HTS - 0x30 = 0x0672 - 0x30 = 0x0642
                //{0x3321, 0x42}, //
                //{0x3322, 0x01}, // def: 0x00
                //{0x3323, 0x30}, // power reduct def: 0x00
                //{0x3330, 0x0D}, // def: 0x00
                //{0x3335, 0x05}, // def: 0x1A
                //{0x3600, 0x7C}, // def: 0x54
                //{0x3620, 0x44}, // def: 0x88
                //{0x3636, 0x0D}, // def: 0x00 power reduct
                //{0x5785, 0x08}, // def: 0x40 black spot threshold
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
                {0x3F04, 0x07},
                {0x3F05, 0x98}, // hts - 0x24
                //{0x320E, 0x02}, // TIMING_VTS_H       750 = 0x02EE
                //{0x320F, 0xEE}, // TIMING_VTS_L
                // config Output window position
                {0x3210, 0x00}, // TIMING_HOFFS_H
                {0x3211, 0x08}, // TIMING_HOFFS_L
                {0x3212, 0x00}, // TIMING_VOFFS_H
                {0x3213, 0x04}, // TIMING_VOFFS_L
                // power consumption reduction & timing
                //{0x3300, 0x33},
                //{0x3301, 0x38}, // cancel aec shanshuo
                //{0x3303, 0x66},
                //{0x3304, 0x18},
                //{0x3308, 0x40}, // def: 0x00
                //{0x330D, 0x40}, // def: 0x00
                //{0x331E, 0x50},
                //{0x3320, 0x07}, // {0x3320, 0x3321} = HTS - 0x30 = 0x07BC - 0x30 = 0x078C
                //{0x3321, 0x8C}, //
                //{0x3322, 0x01}, // def: 0x00
                //{0x3323, 0xC0}, // power reduct def: 0x00
                //{0x3330, 0x0D}, // def: 0x00
                //{0x3335, 0x05}, // def: 0x1A
                //{0x3600, 0x7C}, // def: 0x54
                //{0x3620, 0x44}, // def: 0x88
                //{0x3636, 0x0D}, // def: 0x00 power reduct
                //{0x5785, 0x08}, // def: 0x40 black spot threshold
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
#define SC1235_H_MIRROR     (3<<1)
#define SC1235_V_FLIP       (3<<5)
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY_VERTICALLY] =
        {
            {0x3221, SC1235_V_FLIP | SC1235_H_MIRROR, SC1235_V_FLIP | SC1235_H_MIRROR},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY] =
        {
            {0x3221, SC1235_H_MIRROR, SC1235_V_FLIP | SC1235_H_MIRROR},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_VERTICALLY] =
        {
            {0x3221, SC1235_V_FLIP, SC1235_V_FLIP | SC1235_H_MIRROR},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_NONE] =
        {
            {0x3221, 0x00, SC1235_V_FLIP | SC1235_H_MIRROR},
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
    .shs_additional_use         = 0,
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
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
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
    .max_agc_index      = 128,
    .max_again_index    = 0,
    .cur_again_index    = 0,
    .agc_info =
    {
        .db_max     = 0x2FE397B5,   // 42db     //bug bigger than 42db.
        .db_min     = 0x01000000,   // 1dB
        .db_step    = 0x00100000,   // 0.0625dB
    },
    .gain_reg =
    {
        {0x3E07, 0x00,},
        {0x3E08, 0x00,},
        {0x3E09, 0x00,},
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .gain_table =
    {
        //        0x3631              0x3301              0x3633       0x3e07[0]+0x3e08[7:5]     0x3e08[4:2]               0x3e09
        //      restriction         restriction         restriction         dgain[3:0]        again coarse gain[2:0] again fine gain
        // (    gain< 2: 0x84) (    gain< 2: 0x05) (    gain< 2: 0x2F)      (0000: 1X)            (000: 1X)            ([6:4] corse_gain 000b=1X ,001b=2X, 011b=4X, 111b=8X)
        // ( 2<=gain< 4: 0x88) ( 2<=gain< 4: 0x1F) ( 2<=gain< 4: 0x23)      (0001: 2X)            (001: 2X)            ([3:0] fine_gain)
        // ( 4<=gain   : 0x88) ( 4<=gain   : 0xFF) ( 4<=gain   : 0x43)      (0011: 4X)            (011: 4X)
        //                                                                  (0111: 8X)            (111: 8X)
        {0x00000000, {0x00, 0x00, 0x10,}}, //0
        {0x0086CDDD, {0x00, 0x00, 0x11,}}, //0.526578774
        {0x0105E6A2, {0x00, 0x00, 0x12,}}, //1.023050449
        {0x017E1FC6, {0x00, 0x00, 0x13,}}, //1.492672366
        {0x01F02DE4, {0x00, 0x00, 0x14,}}, //1.93820026
        {0x025CAB21, {0x00, 0x00, 0x15,}}, //2.361986242
        {0x02C41C1C, {0x00, 0x00, 0x16,}}, //2.766053963
        {0x0326F3C3, {0x00, 0x00, 0x17,}}, //3.152157067
        {0x03859655, {0x00, 0x00, 0x18,}}, //3.521825181
        {0x03E05BC8, {0x00, 0x00, 0x19,}}, //3.87640052
        {0x043791B9, {0x00, 0x00, 0x1A,}}, //4.217067306
        {0x048B7CF8, {0x00, 0x00, 0x1B,}}, //4.54487563
        {0x04DC5AD4, {0x00, 0x00, 0x1C,}}, //4.860760974
        {0x052A6228, {0x00, 0x00, 0x1D,}}, //5.165560305
        {0x0575C43A, {0x00, 0x00, 0x1E,}}, //5.460025441
        {0x05BEAD74, {0x00, 0x00, 0x1F,}}, //5.744834224
        {0x06054609, {0x00, 0x04, 0x10,}}, //6.020599913
        {0x068C13E7, {0x00, 0x04, 0x11,}}, //6.547178688
        {0x070B2CAB, {0x00, 0x04, 0x12,}}, //7.043650362
        {0x078365CF, {0x00, 0x04, 0x13,}}, //7.513272279
        {0x07F573ED, {0x00, 0x04, 0x14,}}, //7.958800173
        {0x0861F12A, {0x00, 0x04, 0x15,}}, //8.382586155
        {0x08C96226, {0x00, 0x04, 0x16,}}, //8.786653877
        {0x092C39CD, {0x00, 0x04, 0x17,}}, //9.172756981
        {0x098ADC5E, {0x00, 0x04, 0x18,}}, //9.542425094
        {0x09E5A1D2, {0x00, 0x04, 0x19,}}, //9.897000434
        {0x0A3CD7C2, {0x00, 0x04, 0x1A,}}, //10.23766722
        {0x0A90C301, {0x00, 0x04, 0x1B,}}, //10.56547554
        {0x0AE1A0DD, {0x00, 0x04, 0x1C,}}, //10.88136089
        {0x0B2FA832, {0x00, 0x04, 0x1D,}}, //11.18616022
        {0x0B7B0A43, {0x00, 0x04, 0x1E,}}, //11.48062535
        {0x0BC3F37D, {0x00, 0x04, 0x1F,}}, //11.76543414
        {0x0C0A8C12, {0x00, 0x0C, 0x10,}}, //12.04119983
        {0x0C9159F0, {0x00, 0x0C, 0x11,}}, //12.5677786
        {0x0D1072B4, {0x00, 0x0C, 0x12,}}, //13.06425028
        {0x0D88ABD9, {0x00, 0x0C, 0x13,}}, //13.53387219
        {0x0DFAB9F6, {0x00, 0x0C, 0x14,}}, //13.97940009
        {0x0E673733, {0x00, 0x0C, 0x15,}}, //14.40318607
        {0x0ECEA82F, {0x00, 0x0C, 0x16,}}, //14.80725379
        {0x0F317FD6, {0x00, 0x0C, 0x17,}}, //15.19335689
        {0x0F902268, {0x00, 0x0C, 0x18,}}, //15.56302501
        {0x0FEAE7DB, {0x00, 0x0C, 0x19,}}, //15.91760035
        {0x10421DCB, {0x00, 0x0C, 0x1A,}}, //16.25826713
        {0x1096090A, {0x00, 0x0C, 0x1B,}}, //16.58607546
        {0x10E6E6E7, {0x00, 0x0C, 0x1C,}}, //16.9019608
        {0x1134EE3B, {0x00, 0x0C, 0x1D,}}, //17.20676013
        {0x1180504C, {0x00, 0x0C, 0x1E,}}, //17.50122527
        {0x11C93987, {0x00, 0x0C, 0x1F,}}, //17.78603405
        {0x120FD21B, {0x00, 0x1C, 0x10,}}, //18.06179974
        {0x12969FF9, {0x00, 0x1C, 0x11,}}, //18.58837851
        {0x1315B8BD, {0x00, 0x1C, 0x12,}}, //19.08485019
        {0x138DF1E2, {0x00, 0x1C, 0x13,}}, //19.55447211
        {0x14000000, {0x00, 0x1C, 0x14,}}, //20
        {0x146C7D3C, {0x00, 0x1C, 0x15,}}, //20.42378598
        {0x14D3EE38, {0x00, 0x1C, 0x16,}}, //20.8278537
        {0x1536C5DF, {0x00, 0x1C, 0x17,}}, //21.21395681
        {0x15956871, {0x00, 0x1C, 0x18,}}, //21.58362492
        {0x15F02DE4, {0x00, 0x1C, 0x19,}}, //21.93820026
        {0x164763D4, {0x00, 0x1C, 0x1A,}}, //22.27886705
        {0x169B4F13, {0x00, 0x1C, 0x1B,}}, //22.60667537
        {0x16EC2CF0, {0x00, 0x1C, 0x1C,}}, //22.92256071
        {0x173A3444, {0x00, 0x1C, 0x1D,}}, //23.22736004
        {0x17859655, {0x00, 0x1C, 0x1E,}}, //23.52182518
        {0x17CE7F90, {0x00, 0x1C, 0x1F,}}, //23.80663396
        {0x18151824, {0x00, 0x3C, 0x10,}}, //24.08239965
        {0x189BE602, {0x00, 0x3C, 0x11,}}, //24.60897843
        {0x191AFEC7, {0x00, 0x3C, 0x12,}}, //25.1054501
        {0x199337EB, {0x00, 0x3C, 0x13,}}, //25.57507202
        {0x1A054609, {0x00, 0x3C, 0x14,}}, //26.02059991
        {0x1A71C346, {0x00, 0x3C, 0x15,}}, //26.44438589
        {0x1AD93441, {0x00, 0x3C, 0x16,}}, //26.84845362
        {0x1B3C0BE8, {0x00, 0x3C, 0x17,}}, //27.23455672
        {0x1B9AAE7A, {0x00, 0x3C, 0x18,}}, //27.60422483
        {0x1BF573ED, {0x00, 0x3C, 0x19,}}, //27.95880017
        {0x1C4CA9DD, {0x00, 0x3C, 0x1A,}}, //28.29946696
        {0x1CA0951C, {0x00, 0x3C, 0x1B,}}, //28.62727528
        {0x1CF172F9, {0x00, 0x3C, 0x1C,}}, //28.94316063
        {0x1D3F7A4D, {0x00, 0x3C, 0x1D,}}, //29.24795996
        {0x1D8ADC5E, {0x00, 0x3C, 0x1E,}}, //29.54242509
        {0x1DD3C599, {0x00, 0x3C, 0x1F,}}, //29.82723388
        {0x1E1A5E2D, {0x00, 0x7C, 0x10,}}, //30.10299957
        {0x1EA12C0B, {0x00, 0x7C, 0x11,}}, //30.62957834
        {0x1F2044D0, {0x00, 0x7C, 0x12,}}, //31.12605002
        {0x1F987DF4, {0x00, 0x7C, 0x13,}}, //31.59567193
        {0x200A8C12, {0x00, 0x7C, 0x14,}}, //32.04119983
        {0x2077094F, {0x00, 0x7C, 0x15,}}, //32.46498581
        {0x20DE7A4A, {0x00, 0x7C, 0x16,}}, //32.86905353
        {0x214151F1, {0x00, 0x7C, 0x17,}}, //33.25515663
        {0x219FF483, {0x00, 0x7C, 0x18,}}, //33.62482475
        {0x21FAB9F6, {0x00, 0x7C, 0x19,}}, //33.97940009
        {0x2251EFE7, {0x00, 0x7C, 0x1A,}}, //34.32006687
        {0x22A5DB26, {0x00, 0x7C, 0x1B,}}, //34.6478752
        {0x22F6B902, {0x00, 0x7C, 0x1C,}}, //34.96376054
        {0x2344C056, {0x00, 0x7C, 0x1D,}}, //35.26855987
        {0x23902268, {0x00, 0x7C, 0x1E,}}, //35.56302501
        {0x23D90BA2, {0x00, 0x7C, 0x1F,}}, //35.84783379
        {0x241FA437, {0x00, 0xFC, 0x10,}}, //36.12359948
        {0x24A67215, {0x00, 0xFC, 0x11,}}, //36.65017825
        {0x25258AD9, {0x00, 0xFC, 0x12,}}, //37.14664993
        {0x259DC3FD, {0x00, 0xFC, 0x13,}}, //37.61627185
        {0x260FD21B, {0x00, 0xFC, 0x14,}}, //38.06179974
        {0x267C4F58, {0x00, 0xFC, 0x15,}}, //38.48558572
        {0x26E3C053, {0x00, 0xFC, 0x16,}}, //38.88965344
        {0x274697FB, {0x00, 0xFC, 0x17,}}, //39.27575655
        {0x27A53A8C, {0x00, 0xFC, 0x18,}}, //39.64542466
        {0x28000000, {0x00, 0xFC, 0x19,}}, //40
        {0x285735F0, {0x00, 0xFC, 0x1A,}}, //40.34066679
        {0x28AB212F, {0x00, 0xFC, 0x1B,}}, //40.66847511
        {0x28FBFF0B, {0x00, 0xFC, 0x1C,}}, //40.98436045
        {0x294A0660, {0x00, 0xFC, 0x1D,}}, //41.28915978
        {0x29956871, {0x00, 0xFC, 0x1E,}}, //41.58362492
        {0x29DE51AB, {0x00, 0xFC, 0x1F,}}, //41.8684337
        {0x2A24EA40, {0x01, 0xFC, 0x10,}}, //42.14419939
        {0x2AABB81E, {0x01, 0xFC, 0x11,}}, //42.67077817
        {0x2B2AD0E2, {0x01, 0xFC, 0x12,}}, //43.16724984
        {0x2BA30A07, {0x01, 0xFC, 0x13,}}, //43.63687176
        {0x2C151824, {0x01, 0xFC, 0x14,}}, //44.08239965
        {0x2C819561, {0x01, 0xFC, 0x15,}}, //44.50618563
        {0x2CE9065D, {0x01, 0xFC, 0x16,}}, //44.91025336
        {0x2D4BDE04, {0x01, 0xFC, 0x17,}}, //45.29635646
        {0x2DAA8096, {0x01, 0xFC, 0x18,}}, //45.66602457
        {0x2E054609, {0x01, 0xFC, 0x19,}}, //46.02059991
        {0x2E5C7BF9, {0x01, 0xFC, 0x1A,}}, //46.3612667
        {0x2EB06738, {0x01, 0xFC, 0x1B,}}, //46.68907502
        {0x2F014515, {0x01, 0xFC, 0x1C,}}, //47.00496037
        {0x2F4F4C69, {0x01, 0xFC, 0x1D,}}, //47.3097597
        {0x2F9AAE7A, {0x01, 0xFC, 0x1E,}}, //47.60422483
        {0x2FE397B5, {0x01, 0xFC, 0x1F,}}, //47.88903362
    },
    .temperatue_gain_table =
    {
        .agc_control_by_temperatue      = 0,
        .Aptina_temperatue_mode         = 0,
        .temperatue_control_reg_index   = 3,
        .temperatue_gain_num            = 10,
        .temperatue_reg                 = {GADI_VI_SENSOR_TABLE_FLAG_END,},
        .gain_reg                       =
        {
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        .temperatue_gain                =
        {
                0x00,
        },
        .temperatue_control             =
        {
            0x00,
        },
    },
    .magic_end  = GADI_SENSOR_HW_INFO_MAGIC_END,
};

