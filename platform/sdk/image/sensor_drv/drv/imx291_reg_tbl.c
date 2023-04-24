/*!
*****************************************************************************
** \file        imx291_reg_tbl.c
**
** \version     $Id: imx291_reg_tbl.c 11627 2017-03-10 10:58:12Z dengbiao $
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
GADI_VI_SensorDrvInfoT    imx291_sensor =
{
    .magic_start        = GADI_SENSOR_HW_INFO_MAGIC_START,
    .version            = GADI_SENSOR_HW_INFO_VERSION,
    .length             = sizeof(GADI_VI_SensorDrvInfoT),
    .file_date          = 0x20161014,
    .HwInfo         =
    {
        .name               = "imx291",
        .ctl_type           = GADI_SENSOR_CTL_I2C,
        .addr_len           = sizeof(GADI_U16),
        .data_len           = sizeof(GADI_U8),
        .reset_active_level = GADI_GPIO_LOW,
        .hw_addr            = {(0x34>>1), 0, 0, 0},
        .id_reg =
        {
            {0x31D8, 0x01, 0x01},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        .reset_reg =
        {
            {0x3000, 0x01}, // BIT[0]STANDBY
                            // 0: Normal operation, 1:STANDBY
                            // BIT[5:4]TESTEN
                            // Register write 0: Invalid, 3:Valid, Others: Setting prohibited
            {0x3003, 0x01}, // BIT[0]
                            // Standby control 0:Operating, 1: reset
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
    .sensor_id          = GADI_ISP_SENSOR_IMX291,
    .sensor_double_step = 20,
    .typeofsensor       = 2,
    .sync_time_delay    = 0,
    .source_type.dummy  = GADI_VI_CMOS_CHANNEL_TYPE_AUTO,
    .dev_type           = GADI_VI_DEV_TYPE_CMOS,
    .video_system       = GADI_VIDEO_SYSTEM_AUTO,
    .vs_polarity        = GADI_VI_RISING_EDGE,
    .hs_polarity        = GADI_VI_RISING_EDGE,
    .data_edge          = GADI_VI_RISING_EDGE,
    .emb_sync_switch    = GADI_VI_EMB_SYNC_ON,
    .emb_sync_loc       = GADI_VI_EMB_SYNC_LOWER_PEL,
    .emb_sync_mode      = GADI_VI_EMB_SYNC_FULL_RANGE,
    .max_width          = 1920,
    .max_height         = 1080,
    .def_sh_time        = GADI_VIDEO_FPS(50),
    .fmt_num            = 8,
    .auto_fmt           = GADI_VIDEO_MODE(1920, 1080,  25, 1),
    .init_reg           =
    {
        // CHIP ID:0x02
        {0x300F, 0x00},
        {0x3010, 0x21},
        {0x3012, 0x64},
        {0x3013, 0x00},
        {0x3016, 0x09},
        {0x3017, 0x00},

        {0x304B, 0x0A},
        {0x3070, 0x02},
        {0x3071, 0x11},
        {0x3072, 0x00},
        {0x3073, 0x01},
        //{0x308C, 0x21}, // test padden
        {0x309B, 0x10},
        {0x309C, 0x22},
        {0x30A2, 0x02},
        {0x30A6, 0x20},
        {0x30A8, 0x20},
        {0x30AA, 0x20},
        {0x30AC, 0x20},
        {0x30B0, 0x43},

        // CHIP ID:0x03
        {0x3119, 0x9E},
        {0x311C, 0x1E},
        {0x311E, 0x08},
        {0x3128, 0x05},
        {0x313D, 0x83},
        {0x3150, 0x03},
        {0x317E, 0x00},
        {0x31BF, 0x00},
        {0x31CE, 0x00},
        {0x31D0, 0x00},
        {0x31D2, 0x00},
        {0x31D4, 0x00},
        {0x31D8, 0x00},
        {0x31DC, 0x00},
        {0x31DD, 0x00},
        {0x31DF, 0x00},
        {0x31E0, 0x00},
        {0x31E1, 0x00},
        {0x31E2, 0x00},
        {0x31E3, 0x00},

        // CHIP ID:0x04
        {0x32B8, 0x50},
        {0x32B9, 0x10},
        {0x32BA, 0x00},
        {0x32BB, 0x04},
        {0x32C8, 0x50},
        {0x32C8, 0x10},
        {0x32CA, 0x00},
        {0x32CB, 0x04},
        // CHIP ID:0x05
        {0x332C, 0xD3},
        {0x332D, 0x10},
        {0x332E, 0x0D},
        {0x3358, 0x06},
        {0x3359, 0xE1},
        {0x335A, 0x11},
        {0x3360, 0x1E},
        {0x3361, 0x61},
        {0x3362, 0x10},
        {0x33B0, 0x50},
        {0x33B2, 0x1A},
        {0x33B3, 0x04},

        // CHIP ID:0x06
        {0x3495, 0x0C}, // RAW_FMT_H Output bit selection 10bit£º0x0A 12bit£º0x0C
        {0x3494, 0x0C}, // RAW_FMT_L Output bit selection 10bit£º0x0A 12bit£º0x0C
        {0x348E, 0x24},
        {0x3490, 0x01},
        {0x349E, 0xA4},
        {0x349F, 0x01},
        {0x34A2, 0xF0},
        {0x34A4, 0xF0},
        {0x34A6, 0xF0},
        {0x34C9, 0x65},
        {0x34CC, 0x30},
        {0x34D4, 0x48},
        {0x34D8, 0x49},
        {0x34EC, 0x0B},
        {0x34EE, 0x0C},
        {0x3005, 0x01}, // BIT[0] ADRES(0:10bit 1:12bit)
        {0x3046, 0x01}, // BIT[2:0] ODBIT
                        // 0: 10bit, 1:12bit
                        // BIT[7:4] OPORTSEL
                        // Output system selection
                        // 0: Parallel CMOS SDR output
                        // 0x0D: LVDS 2ch output
                        // 0x0E: LVDS 4ch output
                        // 0x0F: LVDS 8ch output
                        // Others: Setting prohibited
        {0x3129, 0x00}, // ADBIT1 0x00:12bits, 0x1D:10bits
        {0x317C, 0x00}, // ADBIT2 0x00:12bits, 0x12:10bits
        {0x31EC, 0x0E}, // ADBIT3 0x0E:12bits, 0x37:10bits
        {0x3442, 0x0C}, // RAW_FMT_H Output bit selection 10bit£º0x0A 12bit£º0x0C
        {0x3441, 0x0C}, // RAW_FMT_L Output bit selection 10bit£º0x0A 12bit£º0x0C

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
                {0x305C, 0x18}, // INCKSEL1 0x18:INCK=37.125 0x0C:INCK=74.25
                {0x305D, 0x00}, // INCKSEL2 0x03:INCK=37.125 0x00:INCK=74.25
                {0x305E, 0x20}, // INCKSEL3 0x20:INCK=37.125 0x10:INCK=74.25
                {0x305F, 0x01}, // INCKSEL4 0x01:INCK=37.125 0x01:INCK=74.25
                {0x315E, 0x1A}, // INCKSEL5 0x1A:INCK=37.125 0x1B:INCK=74.25
                {0x3164, 0x1A}, // INCKSEL6 0x1A:INCK=37.125 0x1B:INCK=74.25
                {0x3444, 0x20}, // EXTCK_FREQ 0x20:INCK=37.125 0x40:INCK=74.25
                {0x3445, 0x25}, // EXTCK_FREQ 0x25:INCK=37.125 0x4A:INCK=74.25
                {0x3480, 0x49}, // INCKSEL7 0x49:INCK=37.125 0x92:INCK=74.25
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
                {0x305C, 0x18}, // INCKSEL1 0x18:INCK=37.125 0x0C:INCK=74.25
                {0x305D, 0x00}, // INCKSEL2 0x03:INCK=37.125 0x00:INCK=74.25
                {0x305E, 0x20}, // INCKSEL3 0x20:INCK=37.125 0x10:INCK=74.25
                {0x305F, 0x01}, // INCKSEL4 0x01:INCK=37.125 0x01:INCK=74.25
                {0x315E, 0x1A}, // INCKSEL5 0x1A:INCK=37.125 0x1B:INCK=74.25
                {0x3164, 0x1A}, // INCKSEL6 0x1A:INCK=37.125 0x1B:INCK=74.25
                {0x3444, 0x20}, // EXTCK_FREQ 0x20:INCK=37.125 0x40:INCK=74.25
                {0x3445, 0x25}, // EXTCK_FREQ 0x25:INCK=37.125 0x4A:INCK=74.25
                {0x3480, 0x49}, // INCKSEL7 0x49:INCK=37.125 0x92:INCK=74.25
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
                {0x305C, 0x20}, // INCKSEL1 0x18:INCK=37.125 0x0C:INCK=74.25
                {0x305D, 0x00}, // INCKSEL2 0x03:INCK=37.125 0x00:INCK=74.25
                {0x305E, 0x20}, // INCKSEL3 0x20:INCK=37.125 0x10:INCK=74.25
                {0x305F, 0x01}, // INCKSEL4 0x01:INCK=37.125 0x01:INCK=74.25
                {0x315E, 0x1A}, // INCKSEL5 0x1A:INCK=37.125 0x1B:INCK=74.25
                {0x3164, 0x1A}, // INCKSEL6 0x1A:INCK=37.125 0x1B:INCK=74.25
                {0x3444, 0x20}, // EXTCK_FREQ 0x20:INCK=37.125 0x40:INCK=74.25
                {0x3445, 0x25}, // EXTCK_FREQ 0x25:INCK=37.125 0x4A:INCK=74.25
                {0x3480, 0x49}, // INCKSEL7 0x49:INCK=37.125 0x92:INCK=74.25
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
    },
    .video_fmt_table =
    {
        // 1080P IMX291_HMAX = SD(44 + 148 - 4 - 16 - 24 - 8 = 140) + Sync(4) + Ignored(4) + Effective(8) + x + Effective(9) + Ignored(4) + dummy(3) + Sync(4) + HB
        // 1080P IMX291_VMAX = VB(8) + FI(1) + IOB(2) + WINWV(12-2) + Effective(8) + y + Effective(9) + VB(7) = 1125
        // 720P  IMX291_HMAX = SD(40 + 220 - 4 - 16 - 24 - 8 = 208) + Sync(4) + Ignored(4) + Effective(8) + x + Effective(9) + Ignored(4) + dummy(3) + Sync(4) + HB
        // 720P  IMX291_VMAX = VB(8) + FI(1) + OB(2) + VDE OB(4) + Effective(4) + y + Effective(5) + VB(6) = 750
        [0] = // 1920x1080P@30fps   2200 * 1125 * 30 = 74250000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1920, 1080,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x3002, 0x01, 0x01}, // BIT[0] 0:Master mode operation start 1:stop
                {0x3007, 0x00}, // BIT[0] VREVERSE, BIT[1] VREVERSE, 0:normal 1:Inverted
                                // BIT[6:4] Readout mode designation
                                // 0: Full HD 1080p
                                // 1: HD720p
                                // 4: Window cropping from Full HD 1080p
                                // Others: Setting prohibited
                {0x3009, 0x02}, // BIT[1:0] FRSEL 2: 30/25fps 1: 60/50fps 0: 120/100fps
                                // BIT[4] FDG_SEL, 0:LCG mode, 1:HCG mode
                {0x300A, 0xF0}, // BLKLEVEL_L=BLKLEVEL AD10bit=60d = 0x3C AD12bit=240d = 0xF0
                {0x300B, 0x00}, // BLKLEVEL_H=BLKLEVEL Black level offset value setting
                {0x301C, 0x30}, // HMAX_L   4400 = 0x1130
                {0x301D, 0x11}, // HMAX_H
                {0x303C, 0x00}, // WINPV_L
                {0x303D, 0x00}, // WINPV_H
                {0x303E, 0x49}, // WINWV_L  1097 = 0x0449 = 8+1080+9
                {0x303F, 0x04}, // WINWV_H
                {0x3418, 0x49}, // WINWV_L  1097 = 0x0449 = 8+1080+9
                {0x3419, 0x04}, // WINWV_H
                {0x3040, 0x00}, // WINPH_L
                {0x3041, 0x00}, // WINPH_H
                {0x3042, 0x9C}, // WINWH_L  1948 = 0x079C = 4+8+1920+9+4+3
                {0x3043, 0x07}, // WINWH_H
                {0x3472, 0x9C}, // WINWH_L  1948 = 0x079C = 4+8+1920+9+4+3
                {0x3473, 0x07}, // WINWH_H
                {0x3000, 0x00}, // BIT[0]STANDBY 0: Normal operation, 1:STANDBY
                {0x3002, 0x00, 0x01}, // BIT[0] 0:Master mode operation start 1:stop
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
                .def_start_x    = 4 + 8, // use emb_sync
                .def_start_y    = (8 + 1 + 2 + 12 - 2 + 8),
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
                {0x3002, 0x01, 0x01}, // BIT[0] 0:Master mode operation start 1:stop
                {0x3007, 0x00}, // BIT[0] VREVERSE, BIT[1] VREVERSE, 0:normal 1:Inverted
                                // BIT[6:4] Readout mode designation
                                // 0: Full HD 1080p
                                // 1: HD720p
                                // 4: Window cropping from Full HD 1080p
                                // Others: Setting prohibited
                {0x3009, 0x02}, // BIT[1:0] FRSEL 2: 30/25fps 1: 60/50fps 0: 120/100fps
                                // BIT[4] FDG_SEL, 0:LCG mode, 1:HCG mode
                {0x300A, 0xF0}, // BLKLEVEL_L=BLKLEVEL AD10bit=60d = 0x3C AD12bit=240d = 0xF0
                {0x300B, 0x00}, // BLKLEVEL_H=BLKLEVEL Black level offset value setting
                {0x301C, 0xA0}, // HMAX_L   5280 = 0x14A0
                {0x301D, 0x14}, // HMAX_H
                {0x303C, 0x00}, // WINPV_L
                {0x303D, 0x00}, // WINPV_H
                {0x303E, 0x49}, // WINWV_L  1097 = 0x0449 = 8+1080+9
                {0x303F, 0x04}, // WINWV_H
                {0x3418, 0x49}, // WINWV_L  1097 = 0x0449 = 8+1080+9
                {0x3419, 0x04}, // WINWV_H
                {0x3040, 0x00}, // WINPH_L
                {0x3041, 0x00}, // WINPH_H
                {0x3042, 0x9C}, // WINWH_L  1948 = 0x079C = 4+8+1920+9+4+3
                {0x3043, 0x07}, // WINWH_H
                {0x3472, 0x9C}, // WINWH_L  1948 = 0x079C = 4+8+1920+9+4+3
                {0x3473, 0x07}, // WINWH_H
                {0x3000, 0x00}, // BIT[0]STANDBY 0: Normal operation, 1:STANDBY
                {0x3002, 0x00, 0x01}, // BIT[0] 0:Master mode operation start 1:stop
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
                // emb_sync_on:  4 + 8
                // emb_sync_off: 4 + 8 + 144 + 131
                .def_start_x    = 4 + 8,
                // emb_sync_on:  8 + 1 + 2 + 12 - 2 + 8
                // emb_sync_off: 8 + 1 + 2 + 12 - 2 + 8 + 1
                .def_start_y    = (8 + 1 + 2 + 12 - 2 + 8),
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
        [2] = // 1280x960P@30fps    1800 * 1000 * 30 = 54000000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  960,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x3002, 0x01, 0x01}, // BIT[0] 0:Master mode operation start 1:stop
                {0x3007, 0x40}, // BIT[0] VREVERSE, BIT[1] VREVERSE, 0:normal 1:Inverted
                                // BIT[6:4] Readout mode designation
                                // 0: Full HD 1080p
                                // 1: HD720p
                                // 4: Window cropping from Full HD 1080p
                                // Others: Setting prohibited
                {0x3009, 0x02}, // BIT[1:0] FRSEL 2: 30/25fps 1: 60/50fps 0: 120/100fps
                                // BIT[4] FDG_SEL, 0:LCG mode, 1:HCG mode
                {0x300A, 0xF0}, // BLKLEVEL_L=BLKLEVEL AD10bit=60d = 0x3C AD12bit=240d = 0xF0
                {0x300B, 0x00}, // BLKLEVEL_H=BLKLEVEL Black level offset value setting
                {0x301C, 0x10}, // HMAX_L   3600 = 0x0E10
                {0x301D, 0x0E}, // HMAX_H
                {0x303C, 0x00}, // WINPV_L
                {0x303D, 0x00}, // WINPV_H
                {0x303E, 0xC9}, // WINWV_L  969 = 0x03C9 = 4+960+5
                {0x303F, 0x03}, // WINWV_H
                {0x3418, 0xC9}, // WINWV_L  969 = 0x03C9 = 4+960+5
                {0x3419, 0x03}, // WINWV_H
                {0x3040, 0x00}, // WINPH_L
                {0x3041, 0x00}, // WINPH_H
                {0x3042, 0x1C}, // WINWH_L  1308 = 0x051C = 4+8+1280+9+4+3
                {0x3043, 0x05}, // WINWH_H
                {0x3472, 0x1C}, // WINWH_L  1308 = 0x051C = 4+8+1280+9+4+3
                {0x3473, 0x05}, // WINWH_H
                {0x3000, 0x00}, // BIT[0]STANDBY 0: Normal operation, 1:STANDBY
                {0x3002, 0x00, 0x01}, // BIT[0] 0:Master mode operation start 1:stop
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
                .def_start_x    = 4 + 8, // use emb_sync
                .def_start_y    = (8 + 1 + 2 + 4),
                .def_width      = 1280,
                .def_height     = 960,
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
        [3] = // 1280x960P@25fps    2160 * 1000 * 25 = 54000000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  960,  25, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x3002, 0x01, 0x01}, // BIT[0] 0:Master mode operation start 1:stop
                {0x3007, 0x40}, // BIT[0] VREVERSE, BIT[1] VREVERSE, 0:normal 1:Inverted
                                // BIT[6:4] Readout mode designation
                                // 0: Full HD 1080p
                                // 1: HD720p
                                // 4: Window cropping from Full HD 1080p
                                // Others: Setting prohibited
                {0x3009, 0x02}, // BIT[1:0] FRSEL 2: 30/25fps 1: 60/50fps 0: 120/100fps
                                // BIT[4] FDG_SEL, 0:LCG mode, 1:HCG mode
                {0x300A, 0xF0}, // BLKLEVEL_L=BLKLEVEL AD10bit=60d = 0x3C AD12bit=240d = 0xF0
                {0x300B, 0x00}, // BLKLEVEL_H=BLKLEVEL Black level offset value setting
                {0x301C, 0xE0}, // HMAX_L   4320 = 0x10E0
                {0x301D, 0x10}, // HMAX_H
                {0x303C, 0x00}, // WINPV_L
                {0x303D, 0x00}, // WINPV_H
                {0x303E, 0xC9}, // WINWV_L  969 = 0x03C9 = 4+960+5
                {0x303F, 0x03}, // WINWV_H
                {0x3418, 0xC9}, // WINWV_L  969 = 0x03C9 = 4+960+5
                {0x3419, 0x03}, // WINWV_H
                {0x3040, 0x00}, // WINPH_L
                {0x3041, 0x00}, // WINPH_H
                {0x3042, 0x1C}, // WINWH_L  1308 = 0x051C = 4+8+1280+9+4+3
                {0x3043, 0x05}, // WINWH_H
                {0x3472, 0x1C}, // WINWH_L  1308 = 0x051C = 4+8+1280+9+4+3
                {0x3473, 0x05}, // WINWH_H
                {0x3000, 0x00}, // BIT[0]STANDBY 0: Normal operation, 1:STANDBY
                {0x3002, 0x00, 0x01}, // BIT[0] 0:Master mode operation start 1:stop
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
                .def_start_x    = 4 + 8, // use emb_sync
                .def_start_y    = (8 + 1 + 2 + 4),
                .def_width      = 1280,
                .def_height     = 960,
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
        [4] = // 1280x720P@60fps    1650 * 750 * 60 = 74250000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  60, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x3002, 0x01, 0x01}, // BIT[0] 0:Master mode operation start 1:stop
                {0x3007, 0x10}, // BIT[0] VREVERSE, BIT[1] VREVERSE, 0:normal 1:Inverted
                                // BIT[6:4] Readout mode designation
                                // 0: Full HD 1080p
                                // 1: HD720p
                                // 4: Window cropping from Full HD 1080p
                                // Others: Setting prohibited
                {0x3009, 0x01}, // BIT[1:0] FRSEL 2: 30/25fps 1: 60/50fps 0: 120/100fps
                                // BIT[4] FDG_SEL, 0:LCG mode, 1:HCG mode
                {0x300A, 0xF0}, // BLKLEVEL_L=BLKLEVEL AD10bit=60d = 0x3C AD12bit=240d = 0xF0
                {0x300B, 0x00}, // BLKLEVEL_H=BLKLEVEL Black level offset value setting
                {0x301C, 0xE4}, // HMAX_L   3300 = 0x0CE4
                {0x301D, 0x0C}, // HMAX_H
                {0x303C, 0x00}, // WINPV_L
                {0x303D, 0x00}, // WINPV_H
                {0x303E, 0xD9}, // WINWV_L  729 = 0x02D9 = 4+720+5
                {0x303F, 0x02}, // WINWV_H
                {0x3418, 0xD9}, // WINWV_L  729 = 0x02D9 = 4+720+5
                {0x3419, 0x02}, // WINWV_H
                {0x3040, 0x00}, // WINPH_L
                {0x3041, 0x00}, // WINPH_H
                {0x3042, 0x1C}, // WINWH_L  1308 = 0x051C = 4+8+1280+9+4+3
                {0x3043, 0x05}, // WINWH_H
                {0x3472, 0x1C}, // WINWH_L  1308 = 0x051C = 4+8+1280+9+4+3
                {0x3473, 0x05}, // WINWH_H
                {0x3000, 0x00}, // BIT[0]STANDBY 0: Normal operation, 1:STANDBY
                {0x3002, 0x00, 0x01}, // BIT[0] 0:Master mode operation start 1:stop
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
                .def_start_x    = 4 + 8, // use emb_sync
                .def_start_y    = (8 + 1 + 2 + 4),
                .def_width      = 1280,
                .def_height     = 720,
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
            .update_after_vsync_start   = 13,
            .update_after_vsync_end     = 16,
        },
        [5] = // 1280x720P@50fps    1980 * 750 * 50 = 74250000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  50, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x3002, 0x01, 0x01}, // BIT[0] 0:Master mode operation start 1:stop
                {0x3007, 0x10}, // BIT[0] VREVERSE, BIT[1] VREVERSE, 0:normal 1:Inverted
                                // BIT[6:4] Readout mode designation
                                // 0: Full HD 1080p
                                // 1: HD720p
                                // 4: Window cropping from Full HD 1080p
                                // Others: Setting prohibited
                {0x3009, 0x01}, // BIT[1:0] FRSEL 2: 30/25fps 1: 60/50fps 0: 120/100fps
                                // BIT[4] FDG_SEL, 0:LCG mode, 1:HCG mode
                {0x300A, 0xF0}, // BLKLEVEL_L=BLKLEVEL AD10bit=60d = 0x3C AD12bit=240d = 0xF0
                {0x300B, 0x00}, // BLKLEVEL_H=BLKLEVEL Black level offset value setting
                {0x301C, 0x78}, // HMAX_L   3960 = 0x0F78
                {0x301D, 0x0F}, // HMAX_H
                {0x303C, 0x00}, // WINPV_L
                {0x303D, 0x00}, // WINPV_H
                {0x303E, 0xD9}, // WINWV_L  729 = 0x02D9 = 4+720+5
                {0x303F, 0x02}, // WINWV_H
                {0x3418, 0xD9}, // WINWV_L  729 = 0x02D9 = 4+720+5
                {0x3419, 0x02}, // WINWV_H
                {0x3040, 0x00}, // WINPH_L
                {0x3041, 0x00}, // WINPH_H
                {0x3042, 0x1C}, // WINWH_L  1308 = 0x051C = 4+8+1280+9+4+3
                {0x3043, 0x05}, // WINWH_H
                {0x3472, 0x1C}, // WINWH_L  1308 = 0x051C = 4+8+1280+9+4+3
                {0x3473, 0x05}, // WINWH_H
                {0x3000, 0x00}, // BIT[0]STANDBY 0: Normal operation, 1:STANDBY
                {0x3002, 0x00, 0x01}, // BIT[0] 0:Master mode operation start 1:stop
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
                .def_start_x    = 4 + 8, // use emb_sync
                .def_start_y    = (8 + 1 + 2 + 4),
                .def_width      = 1280,
                .def_height     = 720,
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
            .update_after_vsync_start   = 17,
            .update_after_vsync_end     = 20,
        },
        [6] = // 1280x720P@30fps    1650 * 750 * 30 = 37125000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x3002, 0x01, 0x01}, // BIT[0] 0:Master mode operation start 1:stop
                {0x3007, 0x10}, // BIT[0] VREVERSE, BIT[1] VREVERSE, 0:normal 1:Inverted
                                // BIT[6:4] Readout mode designation
                                // 0: Full HD 1080p
                                // 1: HD720p
                                // 4: Window cropping from Full HD 1080p
                                // Others: Setting prohibited
                {0x3009, 0x02}, // BIT[1:0] FRSEL 2: 30/25fps 1: 60/50fps 0: 120/100fps
                                // BIT[4] FDG_SEL, 0:LCG mode, 1:HCG mode
                {0x300A, 0xF0}, // BLKLEVEL_L=BLKLEVEL AD10bit=60d = 0x3C AD12bit=240d = 0xF0
                {0x300B, 0x00}, // BLKLEVEL_H=BLKLEVEL Black level offset value setting
                {0x301C, 0xC8}, // HMAX_L   6600 = 0x19C8
                {0x301D, 0x19}, // HMAX_H
                {0x303C, 0x00}, // WINPV_L
                {0x303D, 0x00}, // WINPV_H
                {0x303E, 0xD9}, // WINWV_L  729 = 0x02D9 = 4+720+5
                {0x303F, 0x02}, // WINWV_H
                {0x3418, 0xD9}, // WINWV_L  729 = 0x02D9 = 4+720+5
                {0x3419, 0x02}, // WINWV_H
                {0x3040, 0x00}, // WINPH_L
                {0x3041, 0x00}, // WINPH_H
                {0x3042, 0x1C}, // WINWH_L  1308 = 0x051C = 4+8+1280+9+4+3
                {0x3043, 0x05}, // WINWH_H
                {0x3472, 0x1C}, // WINWH_L  1308 = 0x051C = 4+8+1280+9+4+3
                {0x3473, 0x05}, // WINWH_H
                {0x3000, 0x00}, // BIT[0]STANDBY 0: Normal operation, 1:STANDBY
                {0x3002, 0x00, 0x01}, // BIT[0] 0:Master mode operation start 1:stop
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
                .def_start_x    = 4 + 8, // use emb_sync
                .def_start_y    = (8 + 1 + 2 + 4),
                .def_width      = 1280,
                .def_height     = 720,
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
        [7] = // 1280x720P@25fps    1980 * 750 * 25 = 37125000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  25, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x3002, 0x01, 0x01}, // BIT[0] 0:Master mode operation start 1:stop
                {0x3007, 0x10}, // BIT[0] VREVERSE, BIT[1] VREVERSE, 0:normal 1:Inverted
                                // BIT[6:4] Readout mode designation
                                // 0: Full HD 1080p
                                // 1: HD720p
                                // 4: Window cropping from Full HD 1080p
                                // Others: Setting prohibited
                {0x3009, 0x02}, // BIT[1:0] FRSEL 2: 30/25fps 1: 60/50fps 0: 120/100fps
                                // BIT[4] FDG_SEL, 0:LCG mode, 1:HCG mode
                {0x300A, 0xF0}, // BLKLEVEL_L=BLKLEVEL AD10bit=60d = 0x3C AD12bit=240d = 0xF0
                {0x300B, 0x00}, // BLKLEVEL_H=BLKLEVEL Black level offset value setting
                {0x301C, 0xF0}, // HMAX_L   7920 = 0x1EF0
                {0x301D, 0x1E}, // HMAX_H
                {0x303C, 0x00}, // WINPV_L
                {0x303D, 0x00}, // WINPV_H
                {0x303E, 0xD9}, // WINWV_L  729 = 0x02D9 = 4+720+5
                {0x303F, 0x02}, // WINWV_H
                {0x3418, 0xD9}, // WINWV_L  729 = 0x02D9 = 4+720+5
                {0x3419, 0x02}, // WINWV_H
                {0x3040, 0x00}, // WINPH_L
                {0x3041, 0x00}, // WINPH_H
                {0x3042, 0x1C}, // WINWH_L  1308 = 0x051C = 4+8+1280+9+4+3
                {0x3043, 0x05}, // WINWH_H
                {0x3472, 0x1C}, // WINWH_L  1308 = 0x051C = 4+8+1280+9+4+3
                {0x3473, 0x05}, // WINWH_H
                {0x3000, 0x00}, // BIT[0]STANDBY 0: Normal operation, 1:STANDBY
                {0x3002, 0x00, 0x01}, // BIT[0] 0:Master mode operation start 1:stop
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
                .def_start_x    = 4 + 8, // use emb_sync
                .def_start_y    = (8 + 1 + 2 + 4),
                .def_width      = 1280,
                .def_height     = 720,
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
    },
#define IMX291_H_MIRROR     (1<<1)
#define IMX291_V_FLIP       (1<<0)
    .mirror_table =
    {
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY_VERTICALLY] =
        {
            {0x3007, IMX291_H_MIRROR | IMX291_V_FLIP, IMX291_H_MIRROR | IMX291_V_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY] =
        {
            {0x3007, IMX291_H_MIRROR, IMX291_H_MIRROR | IMX291_V_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_VERTICALLY] =
        {
            {0x3007, IMX291_V_FLIP, IMX291_H_MIRROR | IMX291_V_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_NONE] =
        {
            {0x3007, 0x00, IMX291_H_MIRROR | IMX291_V_FLIP},
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
        {0x301D, 0x00, 0xFF, 0x00, 0x08, 0}, // HMAX_MSB
        {0x301C, 0x00, 0xFF, 0x00, 0x00, 0}, // HMAX_LSB
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    // vmax = (VMAX_HSB & 0x03) << 16 +
    //        (VMAX_MSB & 0xFF) << 8 +
    //        (VMAX_LSB & 0xFF) << 0
    .vmax_reg =
    {
        {0x301A, 0x00, 0x03, 0x00, 0x10, 0}, // VMAX_HSB
        {0x3019, 0x00, 0xFF, 0x00, 0x08, 0}, // VMAX_MSB
        {0x3018, 0x00, 0xFF, 0x00, 0x00, 0}, // VMAX_LSB
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .vmax_wadd_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    // shs = (INTEG_TIME_H & 0x03) << 16 +
    //       (INTEG_TIME_M & 0xFF) << 8 +
    //       (INTEG_TIME_L & 0xFF) << 0
    .shs_reg =
    {
        {0x3022, 0x00, 0x03, 0x00, 0x10, 0}, // INTEG_TIME_H
        {0x3021, 0x00, 0xFF, 0x00, 0x08, 0}, // INTEG_TIME_M
        {0x3020, 0x00, 0xFF, 0x00, 0x00, 0}, // INTEG_TIME_L
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .shs_additional_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    .cal_gain_mode      = GADI_SENSOR_CAL_GAIN_MODE1,
    .max_agc_index = 261,
    .max_again_index = 0,
    .cur_again_index = 0,
    .agc_info =
    {
        .db_max     = 0x4E000000,    // 78dB
        .db_min     = 0x00000000,    // 0dB
        .db_step    = 0x004CCCCD,    // 0.3dB
    },
    .gain_reg =
    {
        {0x3009, 0x00, 0x10},//HCG
        {0x3014, 0x00},//gain
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .gain_table =
    {
        {0x00000000, {0x00, 0x00,}}, //0.0dB
        {0x004CCCCD, {0x00, 0x01,}}, //0.3dB
        {0x0099999A, {0x00, 0x02,}}, //0.6dB
        {0x00E66666, {0x00, 0x03,}}, //0.9dB
        {0x01333333, {0x00, 0x04,}}, //1.2dB
        {0x01800000, {0x00, 0x05,}}, //1.5dB
        {0x01CCCCCD, {0x00, 0x06,}}, //1.8dB
        {0x0219999A, {0x00, 0x07,}}, //2.1dB
        {0x02666666, {0x00, 0x08,}}, //2.4dB
        {0x02B33333, {0x00, 0x09,}}, //2.7dB
        {0x03000000, {0x00, 0x0A,}}, //3.0dB
        {0x034CCCCD, {0x00, 0x0B,}}, //3.3dB
        {0x0399999A, {0x00, 0x0C,}}, //3.6dB
        {0x03E66666, {0x00, 0x0D,}}, //3.9dB
        {0x04333333, {0x00, 0x0E,}}, //4.2dB
        {0x04800000, {0x00, 0x0F,}}, //4.5dB
        {0x04CCCCCD, {0x00, 0x10,}}, //4.8dB
        {0x0519999A, {0x00, 0x11,}}, //5.1dB
        {0x05666666, {0x00, 0x12,}}, //5.4dB
        {0x05B33333, {0x00, 0x13,}}, //5.7dB
        {0x06000000, {0x00, 0x14,}}, //6.0dB
        {0x064CCCCD, {0x00, 0x15,}}, //6.3dB
        {0x0699999A, {0x00, 0x16,}}, //6.6dB
        {0x06E66666, {0x00, 0x17,}}, //6.9dB
        {0x07333333, {0x00, 0x18,}}, //7.2dB
        {0x07800000, {0x00, 0x19,}}, //7.5dB
        {0x07CCCCCD, {0x00, 0x1A,}}, //7.8dB
        {0x0819999A, {0x00, 0x1B,}}, //8.1dB
        {0x08666666, {0x00, 0x1C,}}, //8.4dB
        {0x08B33333, {0x00, 0x1D,}}, //8.7dB
        {0x09000000, {0x00, 0x1E,}}, //9.0dB
        {0x094CCCCD, {0x00, 0x1F,}}, //9.3dB
        {0x0999999A, {0x00, 0x20,}}, //9.6dB
        {0x09E66666, {0x00, 0x21,}}, //9.9dB
        {0x0A333333, {0x00, 0x22,}}, //10.2dB
        {0x0A800000, {0x00, 0x23,}}, //10.5dB
        {0x0ACCCCCD, {0x00, 0x24,}}, //10.8dB
        {0x0B19999A, {0x00, 0x25,}}, //11.1dB
        {0x0B666666, {0x00, 0x26,}}, //11.4dB
        {0x0BB33333, {0x00, 0x27,}}, //11.7dB
        {0x0C000000, {0x00, 0x28,}}, //12.0dB
        {0x0C4CCCCD, {0x00, 0x29,}}, //12.3dB
        {0x0C99999A, {0x00, 0x2A,}}, //12.6dB
        {0x0CE66666, {0x00, 0x2B,}}, //12.9dB
        {0x0D333333, {0x00, 0x2C,}}, //13.2dB
        {0x0D800000, {0x00, 0x2D,}}, //13.5dB
        {0x0DCCCCCD, {0x00, 0x2E,}}, //13.8dB
        {0x0E19999A, {0x00, 0x2F,}}, //14.1dB
        {0x0E666666, {0x00, 0x30,}}, //14.4dB
        {0x0EB33333, {0x00, 0x31,}}, //14.7dB
        {0x0F000000, {0x00, 0x32,}}, //15.0dB
        {0x0F4CCCCD, {0x00, 0x33,}}, //15.3dB
        {0x0F99999A, {0x00, 0x34,}}, //15.6dB
        {0x0FE66666, {0x00, 0x35,}}, //15.9dB
        {0x10333333, {0x00, 0x36,}}, //16.2dB
        {0x10800000, {0x00, 0x37,}}, //16.5dB
        {0x10CCCCCD, {0x00, 0x38,}}, //16.8dB
        {0x1119999A, {0x00, 0x39,}}, //17.1dB
        {0x11666666, {0x00, 0x3A,}}, //17.4dB
        {0x11B33333, {0x00, 0x3B,}}, //17.7dB
        {0x12000000, {0x00, 0x3C,}}, //18.0dB
        {0x124CCCCD, {0x00, 0x3D,}}, //18.3dB
        {0x1299999A, {0x00, 0x3E,}}, //18.6dB
        {0x12E66666, {0x00, 0x3F,}}, //18.9dB
        {0x13333333, {0x00, 0x40,}}, //19.2dB
        {0x13800000, {0x00, 0x41,}}, //19.5dB
        {0x13CCCCCD, {0x00, 0x42,}}, //19.8dB
        {0x1419999A, {0x00, 0x43,}}, //20.1dB
        {0x14666666, {0x00, 0x44,}}, //20.4dB
        {0x14B33333, {0x00, 0x45,}}, //20.7dB
        {0x15000000, {0x00, 0x46,}}, //21.0dB
        {0x154CCCCD, {0x00, 0x47,}}, //21.3dB
        {0x1599999A, {0x00, 0x48,}}, //21.6dB
        {0x15E66666, {0x00, 0x49,}}, //21.9dB
        {0x16333333, {0x00, 0x4A,}}, //22.2dB
        {0x16800000, {0x00, 0x4B,}}, //22.5dB
        {0x16CCCCCD, {0x00, 0x4C,}}, //22.8dB
        {0x1719999A, {0x00, 0x4D,}}, //23.1dB
        {0x17666666, {0x00, 0x4E,}}, //23.4dB
        {0x17B33333, {0x00, 0x4F,}}, //23.7dB
        {0x18000000, {0x00, 0x50,}}, //24.0dB
        {0x184CCCCD, {0x00, 0x51,}}, //24.3dB
        {0x1899999A, {0x00, 0x52,}}, //24.6dB
        {0x18E66666, {0x00, 0x53,}}, //24.9dB
        {0x19333333, {0x00, 0x54,}}, //25.2dB
        {0x19800000, {0x00, 0x55,}}, //25.5dB
        {0x19CCCCCD, {0x00, 0x56,}}, //25.8dB
        {0x1A19999A, {0x00, 0x57,}}, //26.1dB
        {0x1A666666, {0x00, 0x58,}}, //26.4dB
        {0x1AB33333, {0x00, 0x59,}}, //26.7dB
        {0x1B000000, {0x00, 0x5A,}}, //27.0dB
        {0x1B4CCCCD, {0x00, 0x5B,}}, //27.3dB
        {0x1B99999A, {0x00, 0x5C,}}, //27.6dB
        {0x1BE66666, {0x00, 0x5D,}}, //27.9dB
        {0x1C333333, {0x00, 0x5E,}}, //28.2dB
        {0x1C800000, {0x00, 0x5F,}}, //28.5dB
        {0x1CCCCCCD, {0x00, 0x60,}}, //28.8dB
        {0x1D19999A, {0x00, 0x61,}}, //29.1dB
        {0x1D666666, {0x00, 0x62,}}, //29.4dB
        {0x1DB33333, {0x00, 0x63,}}, //29.7dB
        {0x1E000000, {0x00, 0x64,}}, //30.0dB
        {0x1E4CCCCD, {0x10, 0x51,}}, //30.3dB
        {0x1E99999A, {0x10, 0x52,}}, //30.6dB
        {0x1EE66666, {0x10, 0x53,}}, //30.9dB
        {0x1F333333, {0x10, 0x54,}}, //31.2dB
        {0x1F800000, {0x10, 0x55,}}, //31.5dB
        {0x1FCCCCCD, {0x10, 0x56,}}, //31.8dB
        {0x2019999A, {0x10, 0x57,}}, //32.1dB
        {0x20666666, {0x10, 0x58,}}, //32.4dB
        {0x20B33333, {0x10, 0x59,}}, //32.7dB
        {0x21000000, {0x10, 0x5A,}}, //33.0dB
        {0x214CCCCD, {0x10, 0x5B,}}, //33.3dB
        {0x2199999A, {0x10, 0x5C,}}, //33.6dB
        {0x21E66666, {0x10, 0x5D,}}, //33.9dB
        {0x22333333, {0x10, 0x5E,}}, //34.2dB
        {0x22800000, {0x10, 0x5F,}}, //34.5dB
        {0x22CCCCCD, {0x10, 0x60,}}, //34.8dB
        {0x2319999A, {0x10, 0x61,}}, //35.1dB
        {0x23666666, {0x10, 0x62,}}, //35.4dB
        {0x23B33333, {0x10, 0x63,}}, //35.7dB
        {0x24000000, {0x10, 0x64,}}, //36.0dB
        {0x244CCCCD, {0x10, 0x65,}}, //36.3dB
        {0x2499999A, {0x10, 0x66,}}, //36.6dB
        {0x24E66666, {0x10, 0x67,}}, //36.9dB
        {0x25333333, {0x10, 0x68,}}, //37.2dB
        {0x25800000, {0x10, 0x69,}}, //37.5dB
        {0x25CCCCCD, {0x10, 0x6A,}}, //37.8dB
        {0x2619999A, {0x10, 0x6B,}}, //38.1dB
        {0x26666666, {0x10, 0x6C,}}, //38.4dB
        {0x26B33333, {0x10, 0x6D,}}, //38.7dB
        {0x27000000, {0x10, 0x6E,}}, //39.0dB
        {0x274CCCCD, {0x10, 0x6F,}}, //39.3dB
        {0x2799999A, {0x10, 0x70,}}, //39.6dB
        {0x27E66666, {0x10, 0x71,}}, //39.9dB
        {0x28333333, {0x10, 0x72,}}, //40.2dB
        {0x28800000, {0x10, 0x73,}}, //40.5dB
        {0x28CCCCCD, {0x10, 0x74,}}, //40.8dB
        {0x2919999A, {0x10, 0x75,}}, //41.1dB
        {0x29666666, {0x10, 0x76,}}, //41.4dB
        {0x29B33333, {0x10, 0x77,}}, //41.7dB
        {0x2A000000, {0x10, 0x78,}}, //42.0dB
        {0x2A4CCCCD, {0x10, 0x79,}}, //42.3dB
        {0x2A99999A, {0x10, 0x7A,}}, //42.6dB
        {0x2AE66666, {0x10, 0x7B,}}, //42.9dB
        {0x2B333333, {0x10, 0x7C,}}, //43.2dB
        {0x2B800000, {0x10, 0x7D,}}, //43.5dB
        {0x2BCCCCCD, {0x10, 0x7E,}}, //43.8dB
        {0x2C19999A, {0x10, 0x7F,}}, //44.1dB
        {0x2C666666, {0x10, 0x80,}}, //44.4dB
        {0x2CB33333, {0x10, 0x81,}}, //44.7dB
        {0x2D000000, {0x10, 0x82,}}, //45.0dB
        {0x2D4CCCCD, {0x10, 0x83,}}, //45.3dB
        {0x2D99999A, {0x10, 0x84,}}, //45.6dB
        {0x2DE66666, {0x10, 0x85,}}, //45.9dB
        {0x2E333333, {0x10, 0x86,}}, //46.2dB
        {0x2E800000, {0x10, 0x87,}}, //46.5dB
        {0x2ECCCCCD, {0x10, 0x88,}}, //46.8dB
        {0x2F19999A, {0x10, 0x89,}}, //47.1dB
        {0x2F666666, {0x10, 0x8A,}}, //47.4dB
        {0x2FB33333, {0x10, 0x8B,}}, //47.7dB
        {0x30000000, {0x10, 0x8C,}}, //48.0dB
        {0x304CCCCD, {0x10, 0x8D,}}, //48.3dB
        {0x3099999A, {0x10, 0x8E,}}, //48.6dB
        {0x30E66666, {0x10, 0x8F,}}, //48.9dB
        {0x31333333, {0x10, 0x90,}}, //49.2dB
        {0x31800000, {0x10, 0x91,}}, //49.5dB
        {0x31CCCCCD, {0x10, 0x92,}}, //49.8dB
        {0x3219999A, {0x10, 0x93,}}, //50.1dB
        {0x32666666, {0x10, 0x94,}}, //50.4dB
        {0x32B33333, {0x10, 0x95,}}, //50.7dB
        {0x33000000, {0x10, 0x96,}}, //51.0dB
        {0x334CCCCD, {0x10, 0x97,}}, //51.3dB
        {0x3399999A, {0x10, 0x98,}}, //51.6dB
        {0x33E66666, {0x10, 0x99,}}, //51.9dB
        {0x34333333, {0x10, 0x9A,}}, //52.2dB
        {0x34800000, {0x10, 0x9B,}}, //52.5dB
        {0x34CCCCCD, {0x10, 0x9C,}}, //52.8dB
        {0x3519999A, {0x10, 0x9D,}}, //53.1dB
        {0x35666666, {0x10, 0x9E,}}, //53.4dB
        {0x35B33333, {0x10, 0x9F,}}, //53.7dB
        {0x36000000, {0x10, 0xA0,}}, //54.0dB
        {0x364CCCCD, {0x10, 0xA1,}}, //54.3dB
        {0x3699999A, {0x10, 0xA2,}}, //54.6dB
        {0x36E66666, {0x10, 0xA3,}}, //54.9dB
        {0x37333333, {0x10, 0xA4,}}, //55.2dB
        {0x37800000, {0x10, 0xA5,}}, //55.5dB
        {0x37CCCCCD, {0x10, 0xA6,}}, //55.8dB
        {0x3819999A, {0x10, 0xA7,}}, //56.1dB
        {0x38666666, {0x10, 0xA8,}}, //56.4dB
        {0x38B33333, {0x10, 0xA9,}}, //56.7dB
        {0x39000000, {0x10, 0xAA,}}, //57.0dB
        {0x394CCCCD, {0x10, 0xAB,}}, //57.3dB
        {0x3999999A, {0x10, 0xAC,}}, //57.6dB
        {0x39E66666, {0x10, 0xAD,}}, //57.9dB
        {0x3A333333, {0x10, 0xAE,}}, //58.2dB
        {0x3A800000, {0x10, 0xAF,}}, //58.5dB
        {0x3ACCCCCD, {0x10, 0xB0,}}, //58.8dB
        {0x3B19999A, {0x10, 0xB1,}}, //59.1dB
        {0x3B666666, {0x10, 0xB2,}}, //59.4dB
        {0x3BB33333, {0x10, 0xB3,}}, //59.7dB
        {0x3C000000, {0x10, 0xB4,}}, //60.0dB
        {0x3C4CCCCD, {0x10, 0xB5,}}, //60.3dB
        {0x3C99999A, {0x10, 0xB6,}}, //60.6dB
        {0x3CE66666, {0x10, 0xB7,}}, //60.9dB
        {0x3D333333, {0x10, 0xB8,}}, //61.2dB
        {0x3D800000, {0x10, 0xB9,}}, //61.5dB
        {0x3DCCCCCD, {0x10, 0xBA,}}, //61.8dB
        {0x3E19999A, {0x10, 0xBB,}}, //62.1dB
        {0x3E666666, {0x10, 0xBC,}}, //62.4dB
        {0x3EB33333, {0x10, 0xBD,}}, //62.7dB
        {0x3F000000, {0x10, 0xBE,}}, //63.0dB
        {0x3F4CCCCD, {0x10, 0xBF,}}, //63.3dB
        {0x3F99999A, {0x10, 0xC0,}}, //63.6dB
        {0x3FE66666, {0x10, 0xC1,}}, //63.9dB
        {0x40333333, {0x10, 0xC2,}}, //64.2dB
        {0x40800000, {0x10, 0xC3,}}, //64.5dB
        {0x40CCCCCD, {0x10, 0xC4,}}, //64.8dB
        {0x4119999A, {0x10, 0xC5,}}, //65.1dB
        {0x41666666, {0x10, 0xC6,}}, //65.4dB
        {0x41B33333, {0x10, 0xC7,}}, //65.7dB
        {0x42000000, {0x10, 0xC8,}}, //66.0dB
        {0x424CCCCD, {0x10, 0xC9,}}, //66.3dB
        {0x4299999A, {0x10, 0xCA,}}, //66.6dB
        {0x42E66666, {0x10, 0xCB,}}, //66.9dB
        {0x43333333, {0x10, 0xCC,}}, //67.2dB
        {0x43800000, {0x10, 0xCD,}}, //67.5dB
        {0x43CCCCCD, {0x10, 0xCE,}}, //67.8dB
        {0x4419999A, {0x10, 0xCF,}}, //68.1dB
        {0x44666666, {0x10, 0xD0,}}, //68.4dB
        {0x44B33333, {0x10, 0xD1,}}, //68.7dB
        {0x45000000, {0x10, 0xD2,}}, //69.0dB
        {0x454CCCCD, {0x10, 0xD3,}}, //69.3dB
        {0x4599999A, {0x10, 0xD4,}}, //69.6dB
        {0x45E66666, {0x10, 0xD5,}}, //69.9dB
        {0x46333333, {0x10, 0xD6,}}, //70.2dB
        {0x46800000, {0x10, 0xD7,}}, //70.5dB
        {0x46CCCCCD, {0x10, 0xD8,}}, //70.8dB
        {0x4719999A, {0x10, 0xD9,}}, //71.1dB
        {0x47666666, {0x10, 0xDA,}}, //71.4dB
        {0x47B33333, {0x10, 0xDB,}}, //71.7dB
        {0x48000000, {0x10, 0xDC,}}, //72.0dB
        {0x484CCCCD, {0x10, 0xDD,}}, //72.3dB
        {0x4899999A, {0x10, 0xDE,}}, //72.6dB
        {0x48E66666, {0x10, 0xDF,}}, //72.9dB
        {0x49333333, {0x10, 0xE0,}}, //73.2dB
        {0x49800000, {0x10, 0xE1,}}, //73.5dB
        {0x49CCCCCD, {0x10, 0xE2,}}, //73.8dB
        {0x4A19999A, {0x10, 0xE3,}}, //74.1dB
        {0x4A666666, {0x10, 0xE4,}}, //74.4dB
        {0x4AB33333, {0x10, 0xE5,}}, //74.7dB
        {0x4B000000, {0x10, 0xE6,}}, //75.0dB
        {0x4B4CCCCD, {0x10, 0xE7,}}, //75.3dB
        {0x4B99999A, {0x10, 0xE8,}}, //75.6dB
        {0x4BE66666, {0x10, 0xE9,}}, //75.9dB
        {0x4C333333, {0x10, 0xEA,}}, //76.2dB
        {0x4C800000, {0x10, 0xEB,}}, //76.5dB
        {0x4CCCCCCD, {0x10, 0xEC,}}, //76.8dB
        {0x4D19999A, {0x10, 0xED,}}, //77.1dB
        {0x4D666666, {0x10, 0xEE,}}, //77.4dB
        {0x4DB33333, {0x10, 0xEF,}}, //77.7dB
        {0x4E000000, {0x10, 0xF0,}}, //78.0dB
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

