/*!
*****************************************************************************
** \file        ov2710_reg_tbl.c
**
** \version     $Id: ov2710_reg_tbl.c 11627 2017-03-10 10:58:12Z dengbiao $
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
GADI_VI_SensorDrvInfoT    ov2710_sensor =
{
    .magic_start        = GADI_SENSOR_HW_INFO_MAGIC_START,
    .version            = GADI_SENSOR_HW_INFO_VERSION,
    .length             = sizeof(GADI_VI_SensorDrvInfoT),
    .file_date          = 0x20161014,
    .HwInfo         =
    {
        .name               = "ov2710",
        .ctl_type           = GADI_SENSOR_CTL_I2C,
        .addr_len           = sizeof(GADI_U16),
        .data_len           = sizeof(GADI_U8),
        .reset_active_level = GADI_GPIO_LOW,
        .hw_addr            = {(0x6C>>1),0,0,0},
        .id_reg =
        {
            {0x300A, 0x27}, // OV2710_CHIP_ID_H
            {0x300B, 0x10}, // OV2710_CHIP_ID_L
        },
        .reset_reg =
        {
            {0x3008, 0x80, 0x80},   // OV2710_SYSTEM_CONTROL00
            {GADI_VI_SENSOR_TABLE_FLAG_MSLEEP, 10},    // msleep(10);
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
    .sensor_id          = GADI_ISP_SENSOR_OV2710,
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
    .max_width          = 1952,
    .max_height         = 1092,
    .def_sh_time        = GADI_VIDEO_FPS(50),
    .fmt_num            = 6,
    .auto_fmt           = GADI_VIDEO_MODE(1920, 1080,  25, 1),
    .init_reg           =
    {
        // Select clock source
        {0x3103, 0x93}, // OV2710_PLL_CLOCK_SELECT  Clock from pre-divider
        // OV2710 software reset
//        {0x3008, 0x82}, // OV2710_SYSTEM_CONTROL00
//        {0x3008, 0x42}, // sleep mode
        // MIPI setting OFF
        {0x300E, 0x18}, // OV2710_MIPI_CTRL00   I set 1A
        // IO control
        {0x3016, 0x00}, // OV2710_PAD_OUTPUT_ENABLE00
        {0x3017, 0x7F}, // OV2710_PAD_OUTPUT_ENABLE01
        {0x3018, 0xFC}, // OV2710_PAD_OUTPUT_ENABLE02
        {0x301C, 0x00}, // OV2710_PAD_OUTPUT_SELECT00        // def = 0x00
        {0x301D, 0x00}, // OV2710_PAD_OUTPUT_SELECT01        // def = 0x00
        {0x301E, 0x00}, // OV2710_PAD_OUTPUT_SELECT02       // def = 0x00
        {0x302C, 0xC0}, // OV2710_PAD_OUTPUT_DRIVE_CAPABILITY       // Output drive capability selection (def = 0x02)
        {0x4700, 0x04}, // OV2710_DVP_CTRL00 use HSYNC instead of HREF
        {0x4708, 0x03}, // OV2710_DVP_CTRL02
        // ISP control
        {0x5000, 0xDF}, // OV2710_ISP_CONTROL0        // [7] LENC on, [2] Black pixel correct en, [1] white pixel correct en
        {0x5001, 0x4E}, // OV2710_ISP_CONTROL1       // [0] AWB en
        // Manual AEC/AGC
        {0x3503, 0x07}, //  OV2710_AEC_PK_MANUAL       // [2] VTS manual en, [1] AGC manual en, [0] AEC manual en
        {0x350A, 0x00}, // OV2710_AEC_AGC_ADJ_H
        {0x350B, 0x00}, // OV2710_AEC_AGC_ADJ_L
        {0x350C, 0x00}, // OV2710_AEC_PK_VTS_H
        {0x350D, 0x00}, // OV2710_AEC_PK_VTS_L
        // Black level calibration
        {0x4000, 0x01},//  OV2710_BLC_CONTROL_00       // BLC control
        {0x4006, 0x00},// Black level target [9:8]
        {0x4007, 0x00},// Black level target [7:0]
        // Reserved registers
        {0x302D, 0x90},
        {0x3600, 0x04},
        {0x3603, 0xA7},
        {0x3604, 0x60},
        {0x3605, 0x05},
        {0x3606, 0x12},
        {0x3621, 0x04}, // [7] Horizontal binning, [6] Horizontal skipping
        {0x3630, 0x6D},
        {0x3631, 0x26},
        {0x3702, 0x9E},
        {0x3703, 0x74},
        {0x3704, 0x10},
        {0x3706, 0x61},
        {0x370B, 0x40},
        {0x370D, 0x07},
        {0x3710, 0x9E},
        {0x3712, 0x0C},
        {0x3713, 0x8B},
        {0x3714, 0x74},
        {0x381a, 0x1a},
        {0x382e, 0x0f},
        {0x4301, 0xFF},
        {0x4303, 0x00},
        {0x3A1A, 0x06}, // AVG REG (to be removed?)
        {0x5688, 0x03}, // AVG REG (to be removed?)
        {0x3621, 0x14}, // OV2710_ANA_ARRAY_01
        {0x3803, 0x09}, // OV2710_TIMING_CONTROL_VS_LOWBYTE
        {0x3818, 0xE0}, // OV2710_TIMING_CONTROL18
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .pll_table =
    {
        // OV2710_PLL_CTRL00
        //  bit[7:6] not used
        //  bit[5:3] charge pump control
        //  bit[2]   not used
        //  bit[1:0] 0x:bypass 10: Divided by 4 when in 8bit mode 11: Divided by 5 when in 10bit mode
        // OV2710_PLL_CTRL01
        //  bit[7:4] PLL DIVS divider, system divider ratio
        //  bit[3:0] PLL DIVM divider, MIPI divider ratio
        // OV2710_PLL_CTRL02
        //  bit[7]   PLL bypass
        //  bit[5:0] PLL DIVP
        // OV2710_PLL_PREDIVEDER
        //  bit[2:0] 000=1, 001=1.5, 010=2, 011=2.5, 100=3, 101=4, 110=6, 111=8
        [0] =
        {
            // for 1080P@30, 720P@60
            .pixclk = 81000000, // pixclk = ((extclk / 8) / OV2710_PLL_PREDIVEDER) * OV2710_PLL_CTRL02
            .extclk = 27000000, //        = (27000000/8)/1*24=81000000
            .regs =
            {
                {0x300F, 0x88}, // OV2710_PLL_CTRL00
                {0x3010, 0x00}, // OV2710_PLL_CTRL01,
                {0x3011, 0x18}, // OV2710_PLL_CTRL02 = 24
                {0x3012, 0x00}, // OV2710_PLL_PREDIVEDER = 1
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,}
            },
        },
        [1] =
        {
            // for 1080P@25, 720P@50
            .pixclk = 74250000, // pixclk = ((extclk / 8) / OV2710_PLL_PREDIVEDER) * OV2710_PLL_CTRL02
            .extclk = 27000000, //        = (27000000/8)/1*22=74250000
            .regs =
            {
                {0x300F, 0x88}, // OV2710_PLL_CTRL00
                {0x3010, 0x00}, // OV2710_PLL_CTRL01,
                {0x3011, 0x16}, // OV2710_PLL_CTRL02 = 22
                {0x3012, 0x00}, // OV2710_PLL_PREDIVEDER = 1
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,}
            },
        },
        [2] =
        {
            // for 720P@30
            .pixclk = 40500000, // pixclk = ((extclk / 8) / OV2710_PLL_PREDIVEDER) * OV2710_PLL_CTRL02
            .extclk = 27000000, //        = (27000000/8)/1*12=40500000
            .regs =
            {
                {0x300F, 0x88}, // OV2710_PLL_CTRL00
                {0x3010, 0x00}, // OV2710_PLL_CTRL01,
                {0x3011, 0x0C}, // OV2710_PLL_CTRL02 = 12
                {0x3012, 0x00}, // OV2710_PLL_PREDIVEDER = 1
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,}
            },
        },
        [3] =
        {
            // for 720P@25
            .pixclk = 37125000, // pixclk = ((extclk / 8) / OV2710_PLL_PREDIVEDER) * OV2710_PLL_CTRL02
            .extclk = 27000000, //        = (27000000/8)/1*11=37125000
            .regs =
            {
                {0x300F, 0x88}, // OV2710_PLL_CTRL00
                {0x3010, 0x00}, // OV2710_PLL_CTRL01,
                {0x3011, 0x0B}, // OV2710_PLL_CTRL02 = 11
                {0x3012, 0x00}, // OV2710_PLL_PREDIVEDER = 1
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,}
            },
        },
    },
    .video_fmt_table =
    {
        // 1920x1080P: HTS must >= 2404, if HTS is to low , must fix the horizontal start before set mirror mode
        // 1280x720P : HTS must >= 1768, if HTS is to low , must fix the horizontal start before set mirror mode
        [0] =  // 1920x1080P@30fps  2428 * 1112 * 30 = 80998080 // 81000000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1920, 1080,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x3008, 0x40, 0x40},   // OV2710_SYSTEM_CONTROL00
                {0x3800, 0x01},   //0x3800 HREF horizontal start [11:8]
                {0x3801, 0xC4},   //0x3801 HREF horizontal start [7:0]
                {0x3802, 0x00},   //0x3802 HREF vertical start [11:8]
                {0x3803, 0x0A},   //0x3803 HREF vertical start [7:0]
                {0x3804, 0x07},   //0x3804 HREF horizontal width [11:8]     1920 = 0x0780
                {0x3805, 0x80},   //0x3805 HREF horizontal width [7:0]
                {0x3806, 0x04},   //0x3806 HREF vertical height [11:8]      1080 = 0x0438
                {0x3807, 0x38},   //0x3807 HREF vertical height [7:0]
                {0x3808, 0x07},   //OV2710_TIMING_CONTROL_DVP_HSIZE_HIGH    1920 = 0x0780
                {0x3809, 0x80},   //OV2710_TIMING_CONTROL_DVP_HSIZE_LOW
                {0x380A, 0x04},   //OV2710_TIMING_CONTROL_DVP_VSIZE_HIGH    1080 = 0x0438
                {0x380B, 0x38},   //OV2710_TIMING_CONTROL_DVP_VSIZE_LOW
                {0x380C, 0x09},   //OV2710_TIMING_CONTROL_HTS_HIGHBYTE      2428 = 0x097C
                {0x380D, 0x7C},   //OV2710_TIMING_CONTROL_HTS_LOWBYTE
                {0x380E, 0x04},   //OV2710_TIMING_CONTROL_VTS_HIGHBYTE      1112 = 0x0458
                {0x380F, 0x58},   //OV2710_TIMING_CONTROL_VTS_LOWBYTE
                {0x3811, 0x02},   //0x3811
                {0x381C, 0x21},   //0x381C
                {0x381D, 0x50},   //0x381D
                {0x381E, 0x01},   //0x381E
                {0x381F, 0x20},   //0x381F
                {0x3820, 0x00},   //0x3820
                {0x3821, 0x00},   //0x3821
                {0x3620, 0x37},   //0x3620
                {0x3623, 0x00},   //0x3623
                {0x3008, 0x00, 0x40},   // OV2710_SYSTEM_CONTROL00
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
                {0x3008, 0x40, 0x40},   // OV2710_SYSTEM_CONTROL00
                {0x3800, 0x01},   //0x3800 HREF horizontal start [11:8]
                {0x3801, 0xC4},   //0x3801 HREF horizontal start [7:0]
                {0x3802, 0x00},   //0x3802 HREF vertical start [11:8]
                {0x3803, 0x0A},   //0x3803 HREF vertical start [7:0]
                {0x3804, 0x07},   //0x3804 HREF horizontal width [11:8]     1920 = 0x0780
                {0x3805, 0x80},   //0x3805 HREF horizontal width [7:0]
                {0x3806, 0x04},   //0x3806 HREF vertical height [11:8]      1080 = 0x0438
                {0x3807, 0x38},   //0x3807 HREF vertical height [7:0]
                {0x3808, 0x07},   //OV2710_TIMING_CONTROL_DVP_HSIZE_HIGH    1920 = 0x0780
                {0x3809, 0x80},   //OV2710_TIMING_CONTROL_DVP_HSIZE_LOW
                {0x380A, 0x04},   //OV2710_TIMING_CONTROL_DVP_VSIZE_HIGH    1080 = 0x0438
                {0x380B, 0x38},   //OV2710_TIMING_CONTROL_DVP_VSIZE_LOW
                {0x380C, 0x0A},   //OV2710_TIMING_CONTROL_HTS_HIGHBYTE      2640 = 0x0A50
                {0x380D, 0x50},   //OV2710_TIMING_CONTROL_HTS_LOWBYTE
                {0x380E, 0x04},   //OV2710_TIMING_CONTROL_VTS_HIGHBYTE      1125 = 0x0465
                {0x380F, 0x65},   //OV2710_TIMING_CONTROL_VTS_LOWBYTE
                {0x3811, 0x02},   //0x3811
                {0x381C, 0x21},   //0x381C
                {0x381D, 0x50},   //0x381D
                {0x381E, 0x01},   //0x381E
                {0x381F, 0x20},   //0x381F
                {0x3820, 0x00},   //0x3820
                {0x3821, 0x00},   //0x3821
                {0x3620, 0x37},   //0x3620
                {0x3623, 0x00},   //0x3623
                {0x3008, 0x00, 0x40},   // OV2710_SYSTEM_CONTROL00
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
                {0x3008, 0x40, 0x40},   // OV2710_SYSTEM_CONTROL00
                {0x3800, 0x01},   //0x3800 HREF horizontal start [11:8]
                {0x3801, 0xC4},   //0x3801 HREF horizontal start [7:0]
                {0x3802, 0x00},   //0x3802 HREF vertical start [11:8]
                {0x3803, 0x0A},   //0x3803 HREF vertical start [7:0]
                {0x3804, 0x05},   //0x3804 HREF horizontal width [11:8]     1280 = 0x0500
                {0x3805, 0x00},   //0x3805 HREF horizontal width [7:0]
                {0x3806, 0x02},   //0x3806 HREF vertical height [11:8]       720 = 0x02D0
                {0x3807, 0xD0},   //0x3807 HREF vertical height [7:0]
                {0x3808, 0x05},   //OV2710_TIMING_CONTROL_DVP_HSIZE_HIGH    1280 = 0x0500
                {0x3809, 0x00},   //OV2710_TIMING_CONTROL_DVP_HSIZE_LOW
                {0x380A, 0x02},   //OV2710_TIMING_CONTROL_DVP_VSIZE_HIGH     720 = 0x02D0
                {0x380B, 0xD0},   //OV2710_TIMING_CONTROL_DVP_VSIZE_LOW
                {0x380C, 0x07},   //OV2710_TIMING_CONTROL_HTS_HIGHBYTE      1800 = 0x0708
                {0x380D, 0x08},   //OV2710_TIMING_CONTROL_HTS_LOWBYTE
                {0x380E, 0x02},   //OV2710_TIMING_CONTROL_VTS_HIGHBYTE       750 = 0x02EE
                {0x380F, 0xEE},   //OV2710_TIMING_CONTROL_VTS_LOWBYTE
                {0x3811, 0x00},   //0x3811
                {0x381C, 0x10},   //0x381C
                {0x381D, 0xB8},   //0x381D
                {0x381E, 0x02},   //0x381E
                {0x381F, 0xDC},   //0x381F
                {0x3820, 0x0A},   //0x3820
                {0x3821, 0x29},   //0x3821
                {0x3620, 0x07},   //0x3620
                {0x3623, 0x40},   //0x3623
                {0x3008, 0x00, 0x40},   // OV2710_SYSTEM_CONTROL00
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
            .update_after_vsync_start   = 13,
            .update_after_vsync_end     = 16,
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
                {0x3008, 0x40, 0x40},   // OV2710_SYSTEM_CONTROL00
                {0x3800, 0x01},   //0x3800 HREF horizontal start [11:8]
                {0x3801, 0xC4},   //0x3801 HREF horizontal start [7:0]
                {0x3802, 0x00},   //0x3802 HREF vertical start [11:8]
                {0x3803, 0x02},   //0x3803 HREF vertical start [7:0]
                {0x3804, 0x05},   //0x3804 HREF horizontal width [11:8]     1280 = 0x0500
                {0x3805, 0x00},   //0x3805 HREF horizontal width [7:0]
                {0x3806, 0x02},   //0x3806 HREF vertical height [11:8]       720 = 0x02D0
                {0x3807, 0xD0},   //0x3807 HREF vertical height [7:0]
                {0x3808, 0x05},   //OV2710_TIMING_CONTROL_DVP_HSIZE_HIGH    1280 = 0x0500
                {0x3809, 0x00},   //OV2710_TIMING_CONTROL_DVP_HSIZE_LOW
                {0x380A, 0x02},   //OV2710_TIMING_CONTROL_DVP_VSIZE_HIGH     720 = 0x02D0
                {0x380B, 0xD0},   //OV2710_TIMING_CONTROL_DVP_VSIZE_LOW
                {0x380C, 0x07},   //OV2710_TIMING_CONTROL_HTS_HIGHBYTE      1980 = 0x07BC
                {0x380D, 0xBC},   //OV2710_TIMING_CONTROL_HTS_LOWBYTE
                {0x380E, 0x02},   //OV2710_TIMING_CONTROL_VTS_HIGHBYTE       750 = 0x02EE
                {0x380F, 0xEE},   //OV2710_TIMING_CONTROL_VTS_LOWBYTE
                {0x3811, 0x00},   //0x3811
                {0x381C, 0x10},   //0x381C
                {0x381D, 0xB8},   //0x381D
                {0x381E, 0x02},   //0x381E
                {0x381F, 0xDC},   //0x381F
                {0x3820, 0x0A},   //0x3820
                {0x3821, 0x29},   //0x3821
                {0x3620, 0x07},   //0x3620
                {0x3623, 0x40},   //0x3623
                {0x3008, 0x00, 0x40},   // OV2710_SYSTEM_CONTROL00
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
            .update_after_vsync_start   = 17,
            .update_after_vsync_end     = 20,
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
                {0x3008, 0x40, 0x40},   // OV2710_SYSTEM_CONTROL00
                {0x3800, 0x01},   //0x3800 HREF horizontal start [11:8]
                {0x3801, 0xC4},   //0x3801 HREF horizontal start [7:0]
                {0x3802, 0x00},   //0x3802 HREF vertical start [11:8]
                {0x3803, 0x0A},   //0x3803 HREF vertical start [7:0]
                {0x3804, 0x05},   //0x3804 HREF horizontal width [11:8]     1280 = 0x0500
                {0x3805, 0x00},   //0x3805 HREF horizontal width [7:0]
                {0x3806, 0x02},   //0x3806 HREF vertical height [11:8]       720 = 0x02D0
                {0x3807, 0xD0},   //0x3807 HREF vertical height [7:0]
                {0x3808, 0x05},   //OV2710_TIMING_CONTROL_DVP_HSIZE_HIGH    1280 = 0x0500
                {0x3809, 0x00},   //OV2710_TIMING_CONTROL_DVP_HSIZE_LOW
                {0x380A, 0x02},   //OV2710_TIMING_CONTROL_DVP_VSIZE_HIGH     720 = 0x02D0
                {0x380B, 0xD0},   //OV2710_TIMING_CONTROL_DVP_VSIZE_LOW
                {0x380C, 0x07},   //OV2710_TIMING_CONTROL_HTS_HIGHBYTE      1800 = 0x0708
                {0x380D, 0x08},   //OV2710_TIMING_CONTROL_HTS_LOWBYTE
                {0x380E, 0x02},   //OV2710_TIMING_CONTROL_VTS_HIGHBYTE       750 = 0x02EE
                {0x380F, 0xEE},   //OV2710_TIMING_CONTROL_VTS_LOWBYTE
                {0x3811, 0x00},   //0x3811
                {0x381C, 0x10},   //0x381C
                {0x381D, 0xB8},   //0x381D
                {0x381E, 0x02},   //0x381E
                {0x381F, 0xDC},   //0x381F
                {0x3820, 0x0A},   //0x3820
                {0x3821, 0x29},   //0x3821
                {0x3620, 0x07},   //0x3620
                {0x3623, 0x40},   //0x3623
                {0x3008, 0x00, 0x40},   // OV2710_SYSTEM_CONTROL00
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
                {0x3008, 0x40, 0x40},   // OV2710_SYSTEM_CONTROL00
                {0x3800, 0x01},   //0x3800 HREF horizontal start [11:8]
                {0x3801, 0xC4},   //0x3801 HREF horizontal start [7:0]
                {0x3802, 0x00},   //0x3802 HREF vertical start [11:8]
                {0x3803, 0x02},   //0x3803 HREF vertical start [7:0]
                {0x3804, 0x05},   //0x3804 HREF horizontal width [11:8]
                {0x3805, 0x00},   //0x3805 HREF horizontal width [7:0]
                {0x3806, 0x02},   //0x3806 HREF vertical height [11:8]
                {0x3807, 0xD0},   //0x3807 HREF vertical height [7:0]
                {0x3808, 0x05},   //OV2710_TIMING_CONTROL_DVP_HSIZE_HIGH
                {0x3809, 0x00},   //OV2710_TIMING_CONTROL_DVP_HSIZE_LOW
                {0x380A, 0x02},   //OV2710_TIMING_CONTROL_DVP_VSIZE_HIGH
                {0x380B, 0xD0},   //OV2710_TIMING_CONTROL_DVP_VSIZE_LOW
                {0x380C, 0x07},   //OV2710_TIMING_CONTROL_HTS_HIGHBYTE
                {0x380D, 0xBC},   //OV2710_TIMING_CONTROL_HTS_LOWBYTE
                {0x380E, 0x02},   //OV2710_TIMING_CONTROL_VTS_HIGHBYTE
                {0x380F, 0xEE},   //OV2710_TIMING_CONTROL_VTS_LOWBYTE
                {0x3811, 0x00},   //0x3811
                {0x381C, 0x10},   //0x381C
                {0x381D, 0xB8},   //0x381D
                {0x381E, 0x02},   //0x381E
                {0x381F, 0xDC},   //0x381F
                {0x3820, 0x0A},   //0x3820
                {0x3821, 0x29},   //0x3821
                {0x3620, 0x07},   //0x3620
                {0x3623, 0x40},   //0x3623
                {0x3008, 0x00, 0x40},   // OV2710_SYSTEM_CONTROL00
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
                .pll_index  = 3,
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
            .update_after_vsync_start   = 37,
            .update_after_vsync_end     = 40,
        },
    },
    .mirror_table =
    {
#define OV2710_HORIZ_MIRROR     (0x01 << 6)
#define OV2710_VERT_FLIP        (0x01 << 5)
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY_VERTICALLY] =
        {
            {0x3008, 0x40, 0x40},   // OV2710_SYSTEM_CONTROL00
            {0x3621, 0x14},         // OV2710_ANA_ARRAY_01
            {0x3803, 0x09},         // OV2710_TIMING_CONTROL_VS_LOWBYTE
            {0x3818, OV2710_HORIZ_MIRROR | OV2710_VERT_FLIP, OV2710_HORIZ_MIRROR | OV2710_VERT_FLIP},
            {0x3008, 0x00, 0x40},   // OV2710_SYSTEM_CONTROL00
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY] =
        {
            {0x3008, 0x40, 0x40},   // OV2710_SYSTEM_CONTROL00
            {0x3621, 0x14},         // OV2710_ANA_ARRAY_01
            {0x3803, 0x0A},         // OV2710_TIMING_CONTROL_VS_LOWBYTE
            {0x3818, OV2710_HORIZ_MIRROR, OV2710_HORIZ_MIRROR | OV2710_VERT_FLIP},
            {0x3008, 0x00, 0x40},   // OV2710_SYSTEM_CONTROL00
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_VERTICALLY] =
        {
            {0x3008, 0x40, 0x40},   // OV2710_SYSTEM_CONTROL00
            {0x3621, 0x04},         // OV2710_ANA_ARRAY_01
            {0x3803, 0x09},         // OV2710_TIMING_CONTROL_VS_LOWBYTE
            {0x3818, OV2710_VERT_FLIP, OV2710_HORIZ_MIRROR | OV2710_VERT_FLIP},
            {0x3008, 0x00, 0x40},   // OV2710_SYSTEM_CONTROL00
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_NONE] =
        {
            {0x3008, 0x40, 0x40},   // OV2710_SYSTEM_CONTROL00
            {0x3621, 0x04},         // OV2710_ANA_ARRAY_01
            {0x3803, 0x0A},         // OV2710_TIMING_CONTROL_VS_LOWBYTE
            {0x3818, 0x00, OV2710_HORIZ_MIRROR | OV2710_VERT_FLIP},
            {0x3008, 0x00, 0x40},   // OV2710_SYSTEM_CONTROL00
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
    // hmax = (OV2710_TIMING_CONTROL_HTS_HIGHBYTE & 0xFF) << 8 +
    //        (OV2710_TIMING_CONTROL_HTS_LOWBYTE & 0xFF) << 0
    .hmax_reg =
    {
        {0x380C, 0x00, 0xFF, 0x00, 0x08, 0}, // OV2710_TIMING_CONTROL_HTS_HIGHBYTE
        {0x380D, 0x00, 0xFF, 0x00, 0x00, 0}, // OV2710_TIMING_CONTROL_HTS_LOWBYTE
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    // vmax = (OV2710_TIMING_CONTROL_VTS_HIGHBYTE & 0xFF) << 8 +
    //        (OV2710_TIMING_CONTROL_VTS_LOWBYTE & 0xFF) << 0
    .vmax_reg =
    {
        {0x380E, 0x00, 0xFF, 0x00, 0x08, 0}, // OV2710_TIMING_CONTROL_VTS_HIGHBYTE
        {0x380F, 0x00, 0xFF, 0x00, 0x00, 0}, // OV2710_TIMING_CONTROL_VTS_LOWBYTE
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .vmax_wadd_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    // shs = (OV2710_AEC_PK_EXPO_H & 0x0F) << 12 +
    //       (OV2710_AEC_PK_EXPO_M & 0xFF) << 4 +
    //       (OV2710_AEC_PK_EXPO_L & 0xF0) >> 4
    .shs_reg =
    {
        {0x3500, 0x00, 0x0F, 0x00, 0x0C, 0}, // OV2710_AEC_PK_EXPO_H
        {0x3501, 0x00, 0xFF, 0x00, 0x04, 0}, // OV2710_AEC_PK_EXPO_M
        {0x3502, 0x00, 0xF0, 0x01, 0x04, 0}, // OV2710_AEC_PK_EXPO_L
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .shs_additional_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    .cal_gain_mode              = GADI_SENSOR_CAL_GAIN_MODE2,
    .max_agc_index = 97,
    .max_again_index = 0,
    .cur_again_index = 0,
    .agc_info =
    {
        .db_max     = 0x24000000,    // 36dB
        .db_min     = 0x00000000,    // 0dB
        .db_step    = 0x00600000,    // 0.375dB
    },
    .gain_reg =
    {
        {0x3212,}, // OV2710_GROUP_ACCESS
        {0x350A,}, // OV2710_AEC_AGC_ADJ_H
        {0x350B,}, // OV2710_AEC_AGC_ADJ_L
        {0x3212,}, // OV2710_GROUP_ACCESS
        {0x3212,}, // OV2710_GROUP_ACCESS
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .gain_table =
    {
        // dcg,  col_gain,  dig_gain
        // 0x0004 is high dcg gain,
        //       0x1330 is 8X column gain,
        //                  R0x305E (Context A) or R0x30C4 (Context B).
        //                  0x00FF is digital gain. digital gain step is about 0.1875 db
        //                  xxx.yyyyy The step size for yyyyy is 0.03125 while the step size for xxx is 1
        //       R0x30B0[5:4](Context A) or R0x30B0[9:8] (Context B). 1x, 2x, 4x or 8x
        //       1x or 1.25x gain which can be set in R0x3EE4[9:8]. 0xD208 1101 0010 0000 1000
        {0x00600000 * 0x0060, {0x00, 0x01, 0xFF, 0x10, 0xA0,}},    /* index   0, gain = 36.123599 dB, actual gain = 35.847834 dB */
        {0x00600000 * 0x005F, {0x00, 0x01, 0xFE, 0x10, 0xA0,}},    /* index   1, gain = 35.747312 dB, actual gain = 35.847834 dB */
        {0x00600000 * 0x005E, {0x00, 0x01, 0xFD, 0x10, 0xA0,}},    /* index   2, gain = 35.371024 dB, actual gain = 35.268560 dB */
        {0x00600000 * 0x005D, {0x00, 0x01, 0xFC, 0x10, 0xA0,}},    /* index   3, gain = 34.994737 dB, actual gain = 34.963761 dB */
        {0x00600000 * 0x005C, {0x00, 0x01, 0xFB, 0x10, 0xA0,}},    /* index   4, gain = 34.618450 dB, actual gain = 34.647875 dB */
        {0x00600000 * 0x005B, {0x00, 0x01, 0xFA, 0x10, 0xA0,}},    /* index   5, gain = 34.242162 dB, actual gain = 34.320067 dB */
        {0x00600000 * 0x005A, {0x00, 0x01, 0xF9, 0x10, 0xA0,}},    /* index   6, gain = 33.865875 dB, actual gain = 33.979400 dB */
        {0x00600000 * 0x0059, {0x00, 0x01, 0xF8, 0x10, 0xA0,}},    /* index   7, gain = 33.489587 dB, actual gain = 33.624825 dB */
        {0x00600000 * 0x0058, {0x00, 0x01, 0xF7, 0x10, 0xA0,}},    /* index   8, gain = 33.113300 dB, actual gain = 33.255157 dB */
        {0x00600000 * 0x0057, {0x00, 0x01, 0xF6, 0x10, 0xA0,}},    /* index   9, gain = 32.737012 dB, actual gain = 32.869054 dB */
        {0x00600000 * 0x0056, {0x00, 0x01, 0xF5, 0x10, 0xA0,}},    /* index  10, gain = 32.360725 dB, actual gain = 32.464986 dB */
        {0x00600000 * 0x0055, {0x00, 0x01, 0xF4, 0x10, 0xA0,}},    /* index  11, gain = 31.984437 dB, actual gain = 32.041200 dB */
        {0x00600000 * 0x0054, {0x00, 0x01, 0xF3, 0x10, 0xA0,}},    /* index  12, gain = 31.608150 dB, actual gain = 31.595672 dB */
        {0x00600000 * 0x0053, {0x00, 0x01, 0xF2, 0x10, 0xA0,}},    /* index  13, gain = 31.231862 dB, actual gain = 31.126050 dB */
        {0x00600000 * 0x0052, {0x00, 0x01, 0xF1, 0x10, 0xA0,}},    /* index  14, gain = 30.855575 dB, actual gain = 30.629578 dB */
        {0x00600000 * 0x0051, {0x00, 0x01, 0xF1, 0x10, 0xA0,}},    /* index  15, gain = 30.479287 dB, actual gain = 30.629578 dB */
        {0x00600000 * 0x0050, {0x00, 0x00, 0xFF, 0x10, 0xA0,}},    /* index  16, gain = 30.103000 dB, actual gain = 29.827234 dB */
        {0x00600000 * 0x004F, {0x00, 0x00, 0xFE, 0x10, 0xA0,}},    /* index  17, gain = 29.726712 dB, actual gain = 29.827234 dB */
        {0x00600000 * 0x004E, {0x00, 0x00, 0xFD, 0x10, 0xA0,}},    /* index  18, gain = 29.350425 dB, actual gain = 29.247960 dB */
        {0x00600000 * 0x004D, {0x00, 0x00, 0xFC, 0x10, 0xA0,}},    /* index  19, gain = 28.974137 dB, actual gain = 28.943161 dB */
        {0x00600000 * 0x004C, {0x00, 0x00, 0xFB, 0x10, 0xA0,}},    /* index  20, gain = 28.597850 dB, actual gain = 28.627275 dB */
        {0x00600000 * 0x004B, {0x00, 0x00, 0xFA, 0x10, 0xA0,}},    /* index  21, gain = 28.221562 dB, actual gain = 28.299467 dB */
        {0x00600000 * 0x004A, {0x00, 0x00, 0xF9, 0x10, 0xA0,}},    /* index  22, gain = 27.845275 dB, actual gain = 27.958800 dB */
        {0x00600000 * 0x0049, {0x00, 0x00, 0xF8, 0x10, 0xA0,}},    /* index  23, gain = 27.468987 dB, actual gain = 27.604225 dB */
        {0x00600000 * 0x0048, {0x00, 0x00, 0xF7, 0x10, 0xA0,}},    /* index  24, gain = 27.092700 dB, actual gain = 27.234557 dB */
        {0x00600000 * 0x0047, {0x00, 0x00, 0xF6, 0x10, 0xA0,}},    /* index  25, gain = 26.716412 dB, actual gain = 26.848454 dB */
        {0x00600000 * 0x0046, {0x00, 0x00, 0xF5, 0x10, 0xA0,}},    /* index  26, gain = 26.340125 dB, actual gain = 26.444386 dB */
        {0x00600000 * 0x0045, {0x00, 0x00, 0xF4, 0x10, 0xA0,}},    /* index  27, gain = 25.963837 dB, actual gain = 26.020600 dB */
        {0x00600000 * 0x0044, {0x00, 0x00, 0xF3, 0x10, 0xA0,}},    /* index  28, gain = 25.587550 dB, actual gain = 25.575072 dB */
        {0x00600000 * 0x0043, {0x00, 0x00, 0xF2, 0x10, 0xA0,}},    /* index  29, gain = 25.211262 dB, actual gain = 25.105450 dB */
        {0x00600000 * 0x0042, {0x00, 0x00, 0xF1, 0x10, 0xA0,}},    /* index  30, gain = 24.834975 dB, actual gain = 24.608978 dB */
        {0x00600000 * 0x0041, {0x00, 0x00, 0xF1, 0x10, 0xA0,}},    /* index  31, gain = 24.458687 dB, actual gain = 24.608978 dB */
        {0x00600000 * 0x0040, {0x00, 0x00, 0x7F, 0x10, 0xA0,}},    /* index  32, gain = 24.082400 dB, actual gain = 23.806634 dB */
        {0x00600000 * 0x003F, {0x00, 0x00, 0x7E, 0x10, 0xA0,}},    /* index  33, gain = 23.706112 dB, actual gain = 23.806634 dB */
        {0x00600000 * 0x003E, {0x00, 0x00, 0x7D, 0x10, 0xA0,}},    /* index  34, gain = 23.329825 dB, actual gain = 23.227360 dB */
        {0x00600000 * 0x003D, {0x00, 0x00, 0x7C, 0x10, 0xA0,}},    /* index  35, gain = 22.953537 dB, actual gain = 22.922561 dB */
        {0x00600000 * 0x003C, {0x00, 0x00, 0x7B, 0x10, 0xA0,}},    /* index  36, gain = 22.577250 dB, actual gain = 22.606675 dB */
        {0x00600000 * 0x003B, {0x00, 0x00, 0x7A, 0x10, 0xA0,}},    /* index  37, gain = 22.200962 dB, actual gain = 22.278867 dB */
        {0x00600000 * 0x003A, {0x00, 0x00, 0x79, 0x10, 0xA0,}},    /* index  38, gain = 21.824675 dB, actual gain = 21.938200 dB */
        {0x00600000 * 0x0039, {0x00, 0x00, 0x78, 0x10, 0xA0,}},    /* index  39, gain = 21.448387 dB, actual gain = 21.583625 dB */
        {0x00600000 * 0x0038, {0x00, 0x00, 0x77, 0x10, 0xA0,}},    /* index  40, gain = 21.072100 dB, actual gain = 21.213957 dB */
        {0x00600000 * 0x0037, {0x00, 0x00, 0x76, 0x10, 0xA0,}},    /* index  41, gain = 20.695812 dB, actual gain = 20.827854 dB */
        {0x00600000 * 0x0036, {0x00, 0x00, 0x75, 0x10, 0xA0,}},    /* index  42, gain = 20.319525 dB, actual gain = 20.423786 dB */
        {0x00600000 * 0x0035, {0x00, 0x00, 0x74, 0x10, 0xA0,}},    /* index  43, gain = 19.943237 dB, actual gain = 20.000000 dB */
        {0x00600000 * 0x0034, {0x00, 0x00, 0x73, 0x10, 0xA0,}},    /* index  44, gain = 19.566950 dB, actual gain = 19.554472 dB */
        {0x00600000 * 0x0033, {0x00, 0x00, 0x72, 0x10, 0xA0,}},    /* index  45, gain = 19.190662 dB, actual gain = 19.084850 dB */
        {0x00600000 * 0x0032, {0x00, 0x00, 0x71, 0x10, 0xA0,}},    /* index  46, gain = 18.814375 dB, actual gain = 18.588379 dB */
        {0x00600000 * 0x0031, {0x00, 0x00, 0x71, 0x10, 0xA0,}},    /* index  47, gain = 18.438087 dB, actual gain = 18.588379 dB */
        {0x00600000 * 0x0030, {0x00, 0x00, 0x3F, 0x10, 0xA0,}},    /* index  48, gain = 18.061800 dB, actual gain = 17.786034 dB */
        {0x00600000 * 0x002F, {0x00, 0x00, 0x3E, 0x10, 0xA0,}},    /* index  49, gain = 17.685512 dB, actual gain = 17.786034 dB */
        {0x00600000 * 0x002E, {0x00, 0x00, 0x3D, 0x10, 0xA0,}},    /* index  50, gain = 17.309225 dB, actual gain = 17.206760 dB */
        {0x00600000 * 0x002D, {0x00, 0x00, 0x3C, 0x10, 0xA0,}},    /* index  51, gain = 16.932937 dB, actual gain = 16.901961 dB */
        {0x00600000 * 0x002C, {0x00, 0x00, 0x3B, 0x10, 0xA0,}},    /* index  52, gain = 16.556650 dB, actual gain = 16.586075 dB */
        {0x00600000 * 0x002B, {0x00, 0x00, 0x3A, 0x10, 0xA0,}},    /* index  53, gain = 16.180362 dB, actual gain = 16.258267 dB */
        {0x00600000 * 0x002A, {0x00, 0x00, 0x39, 0x10, 0xA0,}},    /* index  54, gain = 15.804075 dB, actual gain = 15.917600 dB */
        {0x00600000 * 0x0029, {0x00, 0x00, 0x38, 0x10, 0xA0,}},    /* index  55, gain = 15.427787 dB, actual gain = 15.563025 dB */
        {0x00600000 * 0x0028, {0x00, 0x00, 0x37, 0x10, 0xA0,}},    /* index  56, gain = 15.051500 dB, actual gain = 15.193357 dB */
        {0x00600000 * 0x0027, {0x00, 0x00, 0x36, 0x10, 0xA0,}},    /* index  57, gain = 14.675212 dB, actual gain = 14.807254 dB */
        {0x00600000 * 0x0026, {0x00, 0x00, 0x35, 0x10, 0xA0,}},    /* index  58, gain = 14.298925 dB, actual gain = 14.403186 dB */
        {0x00600000 * 0x0025, {0x00, 0x00, 0x34, 0x10, 0xA0,}},    /* index  59, gain = 13.922637 dB, actual gain = 13.979400 dB */
        {0x00600000 * 0x0024, {0x00, 0x00, 0x33, 0x10, 0xA0,}},    /* index  60, gain = 13.546350 dB, actual gain = 13.533872 dB */
        {0x00600000 * 0x0023, {0x00, 0x00, 0x32, 0x10, 0xA0,}},    /* index  61, gain = 13.170062 dB, actual gain = 13.064250 dB */
        {0x00600000 * 0x0022, {0x00, 0x00, 0x31, 0x10, 0xA0,}},    /* index  62, gain = 12.793775 dB, actual gain = 12.567779 dB */
        {0x00600000 * 0x0021, {0x00, 0x00, 0x31, 0x10, 0xA0,}},    /* index  63, gain = 12.417487 dB, actual gain = 12.567779 dB */
        {0x00600000 * 0x0020, {0x00, 0x00, 0x1F, 0x10, 0xA0,}},    /* index  64, gain = 12.041200 dB, actual gain = 11.765434 dB */
        {0x00600000 * 0x001F, {0x00, 0x00, 0x1E, 0x10, 0xA0,}},    /* index  65, gain = 11.664912 dB, actual gain = 11.765434 dB */
        {0x00600000 * 0x001E, {0x00, 0x00, 0x1D, 0x10, 0xA0,}},    /* index  66, gain = 11.288625 dB, actual gain = 11.186160 dB */
        {0x00600000 * 0x001D, {0x00, 0x00, 0x1C, 0x10, 0xA0,}},    /* index  67, gain = 10.912337 dB, actual gain = 10.881361 dB */
        {0x00600000 * 0x001C, {0x00, 0x00, 0x1B, 0x10, 0xA0,}},    /* index  68, gain = 10.536050 dB, actual gain = 10.565476 dB */
        {0x00600000 * 0x001B, {0x00, 0x00, 0x1A, 0x10, 0xA0,}},    /* index  69, gain = 10.159762 dB, actual gain = 10.237667 dB */
        {0x00600000 * 0x001A, {0x00, 0x00, 0x19, 0x10, 0xA0,}},    /* index  70, gain = 9.783475 dB, actual gain = 9.897000 dB */
        {0x00600000 * 0x0019, {0x00, 0x00, 0x18, 0x10, 0xA0,}},    /* index  71, gain = 9.407187 dB, actual gain = 9.542425 dB */
        {0x00600000 * 0x0018, {0x00, 0x00, 0x17, 0x10, 0xA0,}},    /* index  72, gain = 9.030900 dB, actual gain = 9.172757 dB */
        {0x00600000 * 0x0017, {0x00, 0x00, 0x16, 0x10, 0xA0,}},    /* index  73, gain = 8.654612 dB, actual gain = 8.786654 dB */
        {0x00600000 * 0x0016, {0x00, 0x00, 0x15, 0x10, 0xA0,}},    /* index  74, gain = 8.278325 dB, actual gain = 8.382586 dB */
        {0x00600000 * 0x0015, {0x00, 0x00, 0x14, 0x10, 0xA0,}},    /* index  75, gain = 7.902037 dB, actual gain = 7.958800 dB */
        {0x00600000 * 0x0014, {0x00, 0x00, 0x13, 0x10, 0xA0,}},    /* index  76, gain = 7.525750 dB, actual gain = 7.513272 dB */
        {0x00600000 * 0x0013, {0x00, 0x00, 0x12, 0x10, 0xA0,}},    /* index  77, gain = 7.149462 dB, actual gain = 7.043650 dB */
        {0x00600000 * 0x0012, {0x00, 0x00, 0x11, 0x10, 0xA0,}},    /* index  78, gain = 6.773175 dB, actual gain = 6.547179 dB */
        {0x00600000 * 0x0011, {0x00, 0x00, 0x11, 0x10, 0xA0,}},    /* index  79, gain = 6.396887 dB, actual gain = 6.547179 dB */
        {0x00600000 * 0x0010, {0x00, 0x00, 0x0F, 0x10, 0xA0,}},    /* index  80, gain = 6.020600 dB, actual gain = 5.744834 dB */
        {0x00600000 * 0x000F, {0x00, 0x00, 0x0E, 0x10, 0xA0,}},    /* index  81, gain = 5.644312 dB, actual gain = 5.744834 dB */
        {0x00600000 * 0x000E, {0x00, 0x00, 0x0D, 0x10, 0xA0,}},    /* index  82, gain = 5.268025 dB, actual gain = 5.165560 dB */
        {0x00600000 * 0x000D, {0x00, 0x00, 0x0C, 0x10, 0xA0,}},    /* index  83, gain = 4.891737 dB, actual gain = 4.860761 dB */
        {0x00600000 * 0x000C, {0x00, 0x00, 0x0B, 0x10, 0xA0,}},    /* index  84, gain = 4.515450 dB, actual gain = 4.544876 dB */
        {0x00600000 * 0x000B, {0x00, 0x00, 0x0A, 0x10, 0xA0,}},    /* index  85, gain = 4.139162 dB, actual gain = 4.217067 dB */
        {0x00600000 * 0x000A, {0x00, 0x00, 0x09, 0x10, 0xA0,}},    /* index  86, gain = 3.762875 dB, actual gain = 3.876401 dB */
        {0x00600000 * 0x0009, {0x00, 0x00, 0x08, 0x10, 0xA0,}},    /* index  87, gain = 3.386587 dB, actual gain = 3.521825 dB */
        {0x00600000 * 0x0008, {0x00, 0x00, 0x07, 0x10, 0xA0,}},    /* index  88, gain = 3.010300 dB, actual gain = 3.152157 dB */
        {0x00600000 * 0x0007, {0x00, 0x00, 0x06, 0x10, 0xA0,}},    /* index  89, gain = 2.634012 dB, actual gain = 2.766054 dB */
        {0x00600000 * 0x0006, {0x00, 0x00, 0x05, 0x10, 0xA0,}},    /* index  90, gain = 2.257725 dB, actual gain = 2.361986 dB */
        {0x00600000 * 0x0005, {0x00, 0x00, 0x04, 0x10, 0xA0,}},    /* index  91, gain = 1.881437 dB, actual gain = 1.938200 dB */
        {0x00600000 * 0x0004, {0x00, 0x00, 0x03, 0x10, 0xA0,}},    /* index  92, gain = 1.505150 dB, actual gain = 1.492672 dB */
        {0x00600000 * 0x0003, {0x00, 0x00, 0x02, 0x10, 0xA0,}},    /* index  93, gain = 1.128862 dB, actual gain = 1.023050 dB */
        {0x00600000 * 0x0002, {0x00, 0x00, 0x01, 0x10, 0xA0,}},    /* index  94, gain = 0.752575 dB, actual gain = 0.526579 dB */
        {0x00600000 * 0x0001, {0x00, 0x00, 0x01, 0x10, 0xA0,}},    /* index  95, gain = 0.376287 dB, actual gain = 0.526579 dB */
        {0x00600000 * 0x0000, {0x00, 0x00, 0x00, 0x10, 0xA0,}},    /* index  96, gain = 0.000000 dB, actual gain = 0.000000 dB */
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

