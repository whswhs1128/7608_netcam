/*!
*****************************************************************************
** \file        c2392_reg_tbl.c
**
** \version     $Id: c2392_reg_tbl.c 11449 2017-02-10 03:09:18Z yulindeng $
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
GADI_VI_SensorDrvInfoT    c2392_sensor =
{
    .magic_start        = GADI_SENSOR_HW_INFO_MAGIC_START,
    .version            = GADI_SENSOR_HW_INFO_VERSION,
    .length             = sizeof(GADI_VI_SensorDrvInfoT),
    .file_date          = 0x20161014,
    .HwInfo         =
    {
        .name               = "c2392",
        .ctl_type           = GADI_SENSOR_CTL_I2C,
        .addr_len           = sizeof(GADI_U16),
        .data_len           = sizeof(GADI_U8),
        .reset_active_level = GADI_GPIO_LOW,
        .hw_addr            = {(0x68>>1), (0x6C>>1), 0, 0},
        .id_reg =
        {
            {0x0000, 0x02},
            {0x0001, 0x03},
        },
        .reset_reg =
        {
            {0x0103, 0x01, 0x01}, // BIT[0]soft reset
            {0x0100, 0x00, 0x01}, // BIT[0]MODE_SEL 0:STANDBY, 1: Normal operation
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
    .sensor_id          = GADI_ISP_SENSOR_C2392,
    .sensor_double_step = 20,
    .typeofsensor       = 2,
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
    .max_height         = 1096,
    .def_sh_time        = GADI_VIDEO_FPS(50),
    .fmt_num            = 8,
    .auto_fmt           = GADI_VIDEO_MODE(1920, 1080,  25, 1),
    .init_reg           =
    {
        {0x0003, 0x00},
        {0x3000, 0x80}, // [7] frame1off en [6] fsynout re [5] sread addr bin
                        // [4] sread clk re [3:2] test out2gpio [0] autoframe en
        // {0x3001, 0x80}, // [7] fsyn select tmreg6, tmreg7 [6] fsyn en at rst
                        // [5] fsyn en at gpio [4] fsyn gpio input re [3:0] frame rst adj
        // {0x3002, 0x00}, // fsyn output delay Hi
        // {0x3003, 0x00}, // fsyn output delay Lo
        // {0x3004, 0x00}, // fsyn reset horizontal Hi
        // {0x3005, 0x00}, // fsyn reset horizontal Lo
        // {0x3006, 0x00}, // fsyn reset vertical Hi
        // {0x3007, 0x00}, // fsyn reset vertical Lo
        // {0x3008, 0x00}, // horizontal window offset Hi
        {0x3009, 0x08}, // horizontal window offset Lo
        // {0x300A, 0x00}, // vertical window offset Hi
        {0x300B, 0x08}, // vertical window offset Lo
        {0x3021, 0x11}, // [6:4] hbin factor [3] not used [2:0] vbin factor
        {0x3022, 0x22}, // [7] frame 2 off [6] bayer_hen [4] arrvbin en [3:0] bin weight
        {0x3023, 0x00}, // [6] expo non zero [5:4] expo low 2bit
        // {0x3084, 0x40}, // aec target
        // {0x3085, 0x86}, // [7:4] stable range out [3:0] stable range in
        // {0x3086, 0x40}, // [3:0] add frame
        {0x3087, 0xD0}, // [7] aec manual [6] gain delay 1f off [5:4] gain ceiling
                        // [2] setting change en [1] 50/60 auto band [0] band60 manual
        // {0x3088, 0xF0}, // [7] gain fast off [6] vsync toggle clear [5] expo full trigger
                        // [4] sinc/sdec/s2x/s4x change every frame [3] band1 stop off
                        // [2] 4x dec off [1] target dec adj [0] target inc adj
        {0x3089, 0x10}, // [7] band init en
        {0x308A, 0x00},
        {0x308B, 0x00},
        {0x308C, 0x20},
        {0x308D, 0x31},
        {0x3108, 0x89}, // [7:1] reserved [0] blc en
        {0x3109, 0xBF}, // [7:6] reserved [5:0] black level target
        {0x310A, 0x82},
        {0x310B, 0x42},
        {0x3112, 0x3C}, // [7:0] k coef
        {0x3113, 0x00},
        {0x3114, 0x80},
        {0x3115, 0x20},
        {0x3180, 0x10},
        {0x3182, 0x30},
        {0x3183, 0x10},
        {0x3184, 0x20},
        {0x3185, 0xC0},
        {0x3189, 0x50},
        {0x3205, 0x40},
        {0x3208, 0x8D},
        {0x3209, 0x04},
        {0x320F, 0x48},
        {0x3210, 0x12},
        {0x3211, 0x40},
        {0x3212, 0x50},
        {0x3215, 0x90},
        {0x3216, 0x65},
        {0x3217, 0x09},
        {0x3218, 0x20},
        {0x321A, 0x80},
        {0x321B, 0x00},
        {0x321C, 0x1A},
        {0x321E, 0x00},
        {0x3223, 0x10},
        {0x3224, 0x80},
        {0x3225, 0x00},
        {0x3226, 0x08},
        {0x3227, 0x00},
        {0x3228, 0x00},
        {0x3229, 0x08},
        {0x322A, 0x00},
        {0x322B, 0x44},
        {0x3280, 0x06},
        {0x3281, 0x30},
        {0x3282, 0x08},
        {0x3283, 0x51},
        {0x3284, 0x0D},
        {0x3285, 0x48},
        {0x3286, 0x3B},
        {0x3287, 0x07},
        {0x3288, 0x00},
        {0x3289, 0x00},
        {0x328A, 0x08},
        {0x328D, 0x01},
        {0x328E, 0x20},
        {0x328F, 0x0D},
        {0x3290, 0x10}, // [7] Current cut for RGDB, 0: larger normal current, 1: small current for sleep.
                        // [6] Current cut for RGDT 0: larger normal current, 1: small current for sleep.
                        // [4] TM_pd
        {0x3291, 0x00},
        {0x3293, 0x00},
        {0x3403, 0x00}, // group3 start
        {0x3407, 0x01}, // group3 len
        {0x3410, 0x04}, // group4 start
        {0x3414, 0x01}, // group4 len
        {0x3500, 0x00}, // clk0
        {0x3881, 0x04}, // [7:0] read start Lo
        {0x3882, 0x01}, // [5:4] lane num [2] sync 1ch [1] sram cen [0] use sram
        {0x3905, 0x00}, // [7] clip en [6] emb en [5:4] emb num
                        // [3:0] format 0: raw8 hi8 8: raw8 lo8 4: solid data 1: raw10 5: smia pn9
        {0x3980, 0x00}, // [0] : 0 : turn on dvp 1 : turn off dvp
        {0x3981, 0x57}, // [7] efuse enable [6:5] not used [4] pwd force [3] auto load en [0] I2C mode
        {0x3A80, 0x88}, // [7:4] not used [3] I2C Write mode new [1] rd enable [0] pgm enable
        {0x3A81, 0x02},
        {0x3A87, 0x04}, // [7:0] TH1
        {0x3A88, 0x10}, // [7:0] TH2
        {0x3A89, 0x60}, // [7:0] TH3
        {0x3A8A, 0x02}, // [7:0] TH4[15:8]
        {0x3A8B, 0xA7}, // [7:0] TH4[7:0]
        {0x3B80, 0x42},
        {0x3B81, 0x10},
        {0x3B82, 0x10},
        {0x3B83, 0x10},
        {0x3B84, 0x04},
        {0x3B85, 0x04},
        {0x3B86, 0x80},
        {0x3C03, 0x00}, // ISP_03 [7:4] not used [3] window rstb [2:1] not used [0] diggain en
        {0x3C9D, 0x01}, // [7] blc offset sel 0: from blc 1: use r3C9E [6] bcol sel
                        // [5] bline sel [1] para sel 0: local register 1: use otp [0] bypass lenc
        {0x3D00, 0x33}, // [7] inline type [6] not used [5] two couplet bad pixel remove enable
                        // [4] two connected same channel bad pixel remove enable [3] not used
                        // [2] black bad pixel remove enable [1] white bad pixel remove enable [0] bpc enable
        {0x3F08, 0x00}, // [7:5] not used [4] roi enable [3:2] not used
                        // [1:0] avg input selection 2'b00: sensor 2'b01: bpc
        {0x3F8C, 0x00}, // [5] r_en [4] r_gray_en [3:0] r_zonec
        {0xE000, 0x32},
        {0xE001, 0x85},
        {0xE002, 0x48},
        {0xE030, 0x32},
        {0xE031, 0x85},
        {0xE032, 0x42},
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .pll_table =
    {
        // sysclk = extclk / pre_div * pll_multiplier / post_div / mipi_clk_div * 2
        // pixclk = extclk / pre_div * pll_multiplier / post_div / sys_clk_div / 8
        [0] =
        {
            // for 1080P@30, 1080P@25, 720P@60, 720P@50
            .pixclk = 74250000,
            .extclk = 27000000,
            // sysclk = extclk / pre_div * pll_multiplier / post_div / sys_clk_div * 2
            //        = 27 / 3 * 66 / 1 / 8 * 2 = 148.5
            // pixclk = extclk / pre_div * pll_multiplier / post_div / mipi_clk_div / 8
            //        = 27 / 3 * 66 / 1 / 1 / 8 = 74.25
            .regs =
            {
                // {0x0300, 0x08}, // [3:0] frac[19:16]
                // {0x0301, 0x8C}, // frac[15:8]
                // {0x0302, 0x01}, // frac[7:0]
                {0x0303, 0x00}, // [1:0] sys_clk_div /8/10/16
                {0x0304, 0x00}, // [1:0] mipi_clk_div 1/2/4/8
                {0x0305, 0x12}, // [1:0] pre_div /(1~4)
                {0x0307, 0x42}, // [7:0] pll_multiplier * (2~255)
                {0x0309, 0x26}, // [3] pll_bypass [2] bpmode [1] bpsscq [0] aclk_div
                {0x3292, 0x28}, // [7]: r_post[1] post_div /1/2/4/8
                {0x3508, 0x00}, // [7]: r_post[0] post_div /1/2/4/8
                {0x3509, 0xC8},
                {0x350A, 0x60},
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
        [1] =
        {
            // for 960P@30, 960P@25
            .pixclk = 54000000,
            .extclk = 27000000,
            // pixclk = extclk * FRSEL = extclk * 2
            .regs =
            {
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
        [2] =
        {
            // for 720P@30, 720P@25
            .pixclk = 37125000,
            .extclk = 37125000,
            // pixclk = extclk * FRSEL = extclk * 1
            .regs =
            {
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
    },
    .video_fmt_table =
    {
        [0] = // 1920x1080P@30fps   (4400 / 2) * 1125 * 30 = 74250000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1920, 1080,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x0100, 0x00, 0x01}, // BIT[0]MODE_SEL 0:STANDBY, 1: Normal operation
                {0x0342, 0x11}, // HMAX_MSB
                {0x0343, 0x30}, // HMAX_LSB
                {0x0344, 0x00}, // X-START_MSB
                {0x0345, 0x00}, // X-START_LSB
                {0x0346, 0x00}, // Y-START_MSB
                {0x0347, 0x00}, // Y-START_LSB
                {0x0348, 0x07}, // X-END_MSB
                {0x0349, 0x8F}, // X-END_LSB
                {0x034A, 0x04}, // Y-END_MSB
                {0x034B, 0x47}, // Y-END_LSB
                {0x034C, 0x07}, // X-SIZE_MSB
                {0x034D, 0x80}, // X-SIZE_LSB
                {0x034E, 0x04}, // Y-SIZE_MSB
                {0x034F, 0x38}, // Y-SIZE_LSB
                {0x0381, 0x01}, // x_even_inc
                {0x0383, 0x01}, // x_odd_inc
                {0x0385, 0x01}, // y_even_inc
                {0x0387, 0x01}, // y_odd_inc
                {0x3080, 0x01}, // [1:0] base band60 Hi = (1/120)s * System Clock / [0x0342, 0x0343]
                {0x3081, 0x14}, // [7:0] base band60 Lo = (1/120)* 148500000 / 4400 = 281.25 = 0x0119
                {0x3082, 0x01}, // [1:0] base band50 Hi = (1/100)s * System Clock / [0x0342, 0x0343]
                {0x3083, 0x4B}, // [7:0] base band50 Lo = (1/100)* 148500000 / 4400 = 337.5 = 0x0151
                {0x0100, 0x01, 0x01}, // BIT[0]MODE_SEL 0:STANDBY, 1: Normal operation
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
                .def_start_x    = (44 + 148 - 4 - 16 - 24 - 8),// 140<=def_start_x<=200
                .def_start_y    = (6 + 1 + 4 + 8 + 4 + 9),
                .def_width      = 1920,
                .def_height     = 1080,
                .sync_start     = (0 + 0),
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_RG,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_RG,
                    GADI_VI_BAYER_PATTERN_RG,
                    GADI_VI_BAYER_PATTERN_RG,
                    GADI_VI_BAYER_PATTERN_RG,
                },
            },
            .update_after_vsync_start   = 30,
            .update_after_vsync_end     = 33,
        },
        [1] = // 1920x1080P@25fps   (5280 / 2) * 1125 * 25 = 74250000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1920, 1080,  25, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x0100, 0x00, 0x01}, // BIT[0]MODE_SEL 0:STANDBY, 1: Normal operation
                {0x0342, 0x14}, // HMAX_MSB
                {0x0343, 0xA0}, // HMAX_LSB
                {0x0344, 0x00}, // X-START_MSB
                {0x0345, 0x00}, // X-START_LSB
                {0x0346, 0x00}, // Y-START_MSB
                {0x0347, 0x00}, // Y-START_LSB
                {0x0348, 0x07}, // X-END_MSB
                {0x0349, 0x8F}, // X-END_LSB
                {0x034A, 0x04}, // Y-END_MSB
                {0x034B, 0x47}, // Y-END_LSB
                {0x034C, 0x07}, // X-SIZE_MSB
                {0x034D, 0x80}, // X-SIZE_LSB
                {0x034E, 0x04}, // Y-SIZE_MSB
                {0x034F, 0x38}, // Y-SIZE_LSB
                {0x0381, 0x01}, // x_even_inc
                {0x0383, 0x01}, // x_odd_inc
                {0x0385, 0x01}, // y_even_inc
                {0x0387, 0x01}, // y_odd_inc
                {0x3080, 0x00}, // [1:0] base band60 Hi = (1/120)s * System Clock / [0x0342, 0x0343]
                {0x3081, 0xEA}, // [7:0] base band60 Lo = (1/120)* 148500000 / 5280 = 234.375 = 0x00EA
                {0x3082, 0x01}, // [1:0] base band50 Hi = (1/100)s * System Clock / [0x0342, 0x0343]
                {0x3083, 0x19}, // [7:0] base band50 Lo = (1/100)* 148500000 / 5280 = 281.25 = 0x0119
                {0x0100, 0x01, 0x01}, // BIT[0]MODE_SEL 0:STANDBY, 1: Normal operation
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
                .def_start_x    = (44 + 148 - 4 - 16 - 24 - 8),// 140<=def_start_x<=200
                .def_start_y    = (6 + 1 + 4 + 8 + 4 + 9),
                .def_width      = 1920,
                .def_height     = 1080,
                .sync_start     = (0 + 0),
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
                .def_start_x    = (44 + 148 - 4 - 16 - 24 - 8),
                .def_start_y    = (6 + 1 + 4 + 8 + 4 + 4 + 8),
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
                .def_start_x    = (44 + 148 - 4 - 16 - 24 - 8),
                .def_start_y    = (6 + 1 + 4 + 8 + 4 + 4 + 8),
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
            .update_after_vsync_start   = 37,
            .update_after_vsync_end     = 40,
        },
        [4] = // 1280x720P@60fps    (825 * 2) * 750 * 60 = 74250000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  60, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
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
                .def_start_x    = (40 + 220 - 4 - 16 - 24 - 8 - 10) * 2 / 3,
                .def_start_y    = (6 + 1 + 4 + 6 + 2 + 4),
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
                .def_start_x    = (40 + 220 - 4 - 16 - 24 - 8),
                .def_start_y    = (6 + 1 + 4 + 6 + 2 + 4),
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
            .update_after_vsync_start   = 17,
            .update_after_vsync_end     = 20,
        },
        [6] = // 1280x720P@30fps    (1650 * 1) * 750 * 30 = 37125000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x0100, 0x01},  // STANDBY 0:STANDBY, 1: Normal operation
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
                .def_start_x    = (40 + 220 - 4 - 16 - 24 - 8),
                .def_start_y    = (6 + 1 + 4 + 6 + 2 + 4),
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
        [7] = // 1280x720P@25fps    (1980 * 1) * 750 * 25 = 37125000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  25, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
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
                .def_start_x    = (40 + 220 - 4 - 16 - 24 - 8),
                .def_start_y    = (6 + 1 + 4 + 6 + 2 + 4),
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
#define C2392_H_MIRROR     (1<<0)
#define C2392_V_FLIP       (1<<1)
    .mirror_table =
    {
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY_VERTICALLY] =
        {
            {0x0101, C2392_H_MIRROR | C2392_V_FLIP, C2392_H_MIRROR | C2392_V_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY] =
        {
            {0x0101, C2392_H_MIRROR, C2392_H_MIRROR | C2392_V_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_VERTICALLY] =
        {
            {0x0101, C2392_V_FLIP, C2392_H_MIRROR | C2392_V_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_NONE] =
        {
            {0x0101, 0x00, C2392_H_MIRROR | C2392_V_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
    },
    .version_reg =
    {
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .reg_ctl_by_ver_num = 0,
    .control_by_version_reg = {0},
    .vmax_clk_ref   = GADI_SENSOR_CAL_CLK_EXTCLK,
    .shs_clk_ref    = GADI_SENSOR_CAL_CLK_EXTCLK,
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
        {0x0342, 0x00, 0xFF, 0x00, 0x08, 0}, // HMAX_MSB
        {0x0343, 0x00, 0xFF, 0x00, 0x00, 0}, // HMAX_LSB
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    // vmax = (VMAX_MSB & 0xFF) << 8 +
    //        (VMAX_LSB & 0xFF) << 0
    .vmax_reg =
    {
        {0x0340, 0x00, 0xFF, 0x00, 0x08, 0}, // VMAX_MSB
        {0x0341, 0x00, 0xFF, 0x00, 0x00, 0}, // VMAX_LSB
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .vmax_wadd_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    // shs = (INTEG_TIME_H & 0xFF) << 8 +
    //       (INTEG_TIME_L & 0xFF) << 0
    .shs_reg =
    {
        {0x0202, 0x00, 0xFF, 0x00, 0x08, 0}, // INTEG_TIME_H
        {0x0203, 0x00, 0xFF, 0x00, 0x00, 0}, // INTEG_TIME_L
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .shs_additional_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    .cal_gain_mode              = GADI_SENSOR_CAL_GAIN_MODE0,
    .max_agc_index = 121,
    .max_again_index = 0,
    .cur_again_index = 0,
    .agc_info =
    {
        .db_max     = 0x2A000000,    // 42dB
        .db_min     = 0x00000000,    // 0dB
        .db_step    = 0x004ccccd,    // 0.3dB
    },
    .gain_reg =
    {
        {0x0205, 0x00}, // analog gain
        // {0x0210, 0x04}, // R gain[11:8]
        // {0x0211, 0x00}, // R gain[7:0]
        // {0x0212, 0x04}, // G gain[11:8]
        // {0x0213, 0x00}, // G gain[7:0]
        // {0x0214, 0x04}, // B gain[11:8]
        // {0x0215, 0x00}, // B gain[7:0]
        // {0x0216, 0x01}, // D_gain[10:8], format: 3.8
        // {0x0217, 0x00}, // D_gain[7:0]
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
        .temperatue_reg                 =
        {
            // {0x0401, 0x00}, // [0] 1: latch tm_cal, 0: tm_cal will be update each calculate cycle [1] 1: tm_cal >=0, 0: -127<=tm_cal<=+127
            // {0x0405, 0x00}, // Read only temp_cal output
            GADI_VI_SENSOR_TABLE_FLAG_END,
        },
        .gain_reg                       = {GADI_VI_SENSOR_TABLE_FLAG_END,},
        .temperatue_gain                = {0},
        .temperatue_control             = {0},
    },
    .magic_end  = GADI_SENSOR_HW_INFO_MAGIC_END,
};

