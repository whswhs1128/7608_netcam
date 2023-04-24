/*!
*****************************************************************************
** \file        ar0130_reg_tbl.c
**
** \version     $Id: ar0130_reg_tbl.c 11627 2017-03-10 10:58:12Z dengbiao $
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
GADI_VI_SensorDrvInfoT    ar0130_sensor =
{
    .magic_start        = GADI_SENSOR_HW_INFO_MAGIC_START,
    .version            = GADI_SENSOR_HW_INFO_VERSION,
    .length             = sizeof(GADI_VI_SensorDrvInfoT),
    .file_date          = 0x20161014,
    .HwInfo         =
    {
        .name               = "ar0130",
        .ctl_type           = GADI_SENSOR_CTL_I2C,
        .addr_len           = sizeof(GADI_U16),
        .data_len           = sizeof(GADI_U16),
        .reset_active_level = GADI_GPIO_LOW,
        .hw_addr            = {(0x20>>1), (0x30>>1), 0, 0},
        .id_reg =
        {
            {0x3000, 0x2402}, // AR0130_CHIP_ID
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
    .sensor_id          = GADI_ISP_SENSOR_AR0130,
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
    .fmt_num            = 6,
    .auto_fmt           = GADI_VIDEO_MODE(1280,  960,  25, 1),
    .init_reg           =
    {
        {0x301A, 0x0001},     // RESET_REGISTER - [0:08:40.338]
        {0x301A, 0x10D8},     // RESET_REGISTER - [0:08:40.345]
        {GADI_VI_SENSOR_TABLE_FLAG_MSLEEP, 100},//sleep 100ms
        {0x3088, 0x8000},     // SEQ_CTRL_PORT - [0:08:40.459]
        {0x3086, 0x0225},     // SEQ_DATA_PORT - [0:08:40.466]
        {0x3086, 0x5050},     // SEQ_DATA_PORT - [0:08:40.469]
        {0x3086, 0x2D26},     // SEQ_DATA_PORT - [0:08:40.472]
        {0x3086, 0x0828},     // SEQ_DATA_PORT - [0:08:40.475]
        {0x3086, 0x0D17},     // SEQ_DATA_PORT - [0:08:40.478]
        {0x3086, 0x0926},     // SEQ_DATA_PORT - [0:08:40.481]
        {0x3086, 0x0028},     // SEQ_DATA_PORT - [0:08:40.484]
        {0x3086, 0x0526},     // SEQ_DATA_PORT - [0:08:40.487]
        {0x3086, 0xA728},     // SEQ_DATA_PORT - [0:08:40.490]
        {0x3086, 0x0725},     // SEQ_DATA_PORT - [0:08:40.494]
        {0x3086, 0x8080},     // SEQ_DATA_PORT - [0:08:40.497]
        {0x3086, 0x2917},     // SEQ_DATA_PORT - [0:08:40.500]
        {0x3086, 0x0525},     // SEQ_DATA_PORT - [0:08:40.503]
        {0x3086, 0x0040},     // SEQ_DATA_PORT - [0:08:40.507]
        {0x3086, 0x2702},     // SEQ_DATA_PORT - [0:08:40.510]
        {0x3086, 0x1616},     // SEQ_DATA_PORT - [0:08:40.513]
        {0x3086, 0x2706},     // SEQ_DATA_PORT - [0:08:40.516]
        {0x3086, 0x1736},     // SEQ_DATA_PORT - [0:08:40.519]
        {0x3086, 0x26A6},     // SEQ_DATA_PORT - [0:08:40.522]
        {0x3086, 0x1703},     // SEQ_DATA_PORT - [0:08:40.526]
        {0x3086, 0x26A4},     // SEQ_DATA_PORT - [0:08:40.529]
        {0x3086, 0x171F},     // SEQ_DATA_PORT - [0:08:40.536]
        {0x3086, 0x2805},     // SEQ_DATA_PORT - [0:08:40.540]
        {0x3086, 0x2620},     // SEQ_DATA_PORT - [0:08:40.544]
        {0x3086, 0x2804},     // SEQ_DATA_PORT - [0:08:40.547]
        {0x3086, 0x2520},     // SEQ_DATA_PORT - [0:08:40.551]
        {0x3086, 0x2027},     // SEQ_DATA_PORT - [0:08:40.555]
        {0x3086, 0x0017},     // SEQ_DATA_PORT - [0:08:40.558]
        {0x3086, 0x1E25},     // SEQ_DATA_PORT - [0:08:40.562]
        {0x3086, 0x0020},     // SEQ_DATA_PORT - [0:08:40.566]
        {0x3086, 0x2117},     // SEQ_DATA_PORT - [0:08:40.569]
        {0x3086, 0x1028},     // SEQ_DATA_PORT - [0:08:40.573]
        {0x3086, 0x051B},     // SEQ_DATA_PORT - [0:08:40.577]
        {0x3086, 0x1703},     // SEQ_DATA_PORT - [0:08:40.580]
        {0x3086, 0x2706},     // SEQ_DATA_PORT - [0:08:40.584]
        {0x3086, 0x1703},     // SEQ_DATA_PORT - [0:08:40.588]
        {0x3086, 0x1747},     // SEQ_DATA_PORT - [0:08:40.591]
        {0x3086, 0x2660},     // SEQ_DATA_PORT - [0:08:40.595]
        {0x3086, 0x17AE},     // SEQ_DATA_PORT - [0:08:40.599]
        {0x3086, 0x2500},     // SEQ_DATA_PORT - [0:08:40.603]
        {0x3086, 0x9027},     // SEQ_DATA_PORT - [0:08:40.606]
        {0x3086, 0x0026},     // SEQ_DATA_PORT - [0:08:40.610]
        {0x3086, 0x1828},     // SEQ_DATA_PORT - [0:08:40.613]
        {0x3086, 0x002E},     // SEQ_DATA_PORT - [0:08:40.617]
        {0x3086, 0x2A28},     // SEQ_DATA_PORT - [0:08:40.621]
        {0x3086, 0x081E},     // SEQ_DATA_PORT - [0:08:40.624]
        {0x3086, 0x0831},     // SEQ_DATA_PORT - [0:08:40.628]
        {0x3086, 0x1440},     // SEQ_DATA_PORT - [0:08:40.631]
        {0x3086, 0x4014},     // SEQ_DATA_PORT - [0:08:40.636]
        {0x3086, 0x2020},     // SEQ_DATA_PORT - [0:08:40.639]
        {0x3086, 0x1410},     // SEQ_DATA_PORT - [0:08:40.643]
        {0x3086, 0x1034},     // SEQ_DATA_PORT - [0:08:40.647]
        {0x3086, 0x1400},     // SEQ_DATA_PORT - [0:08:40.651]
        {0x3086, 0x1014},     // SEQ_DATA_PORT - [0:08:40.655]
        {0x3086, 0x0020},     // SEQ_DATA_PORT - [0:08:40.658]
        {0x3086, 0x1400},     // SEQ_DATA_PORT - [0:08:40.662]
        {0x3086, 0x4013},     // SEQ_DATA_PORT - [0:08:40.666]
        {0x3086, 0x1802},     // SEQ_DATA_PORT - [0:08:40.670]
        {0x3086, 0x1470},     // SEQ_DATA_PORT - [0:08:40.673]
        {0x3086, 0x7004},     // SEQ_DATA_PORT - [0:08:40.678]
        {0x3086, 0x1470},     // SEQ_DATA_PORT - [0:08:40.681]
        {0x3086, 0x7003},     // SEQ_DATA_PORT - [0:08:40.685]
        {0x3086, 0x1470},     // SEQ_DATA_PORT - [0:08:40.688]
        {0x3086, 0x7017},     // SEQ_DATA_PORT - [0:08:40.692]
        {0x3086, 0x2002},     // SEQ_DATA_PORT - [0:08:40.696]
        {0x3086, 0x1400},     // SEQ_DATA_PORT - [0:08:40.700]
        {0x3086, 0x2002},     // SEQ_DATA_PORT - [0:08:40.703]
        {0x3086, 0x1400},     // SEQ_DATA_PORT - [0:08:40.707]
        {0x3086, 0x5004},     // SEQ_DATA_PORT - [0:08:40.711]
        {0x3086, 0x1400},     // SEQ_DATA_PORT - [0:08:40.714]
        {0x3086, 0x2004},     // SEQ_DATA_PORT - [0:08:40.718]
        {0x3086, 0x1400},     // SEQ_DATA_PORT - [0:08:40.722]
        {0x3086, 0x5022},     // SEQ_DATA_PORT - [0:08:40.726]
        {0x3086, 0x0314},     // SEQ_DATA_PORT - [0:08:40.729]
        {0x3086, 0x0020},     // SEQ_DATA_PORT - [0:08:40.733]
        {0x3086, 0x0314},     // SEQ_DATA_PORT - [0:08:40.737]
        {0x3086, 0x0050},     // SEQ_DATA_PORT - [0:08:40.740]
        {0x3086, 0x2C2C},     // SEQ_DATA_PORT - [0:08:40.744]
        {0x3086, 0x2C2C},     // SEQ_DATA_PORT - [0:08:40.747]
        {0x309E, 0x0000},     // DCDS_PROG_START_ADDR - [0:08:40.754]
        {0x30E4, 0x6372},     // ADC_BITS_6_7 - [0:08:40.761]
        {0x30E2, 0x7253},     // ADC_BITS_4_5 - [0:08:40.767]
        {0x30E0, 0x5470},     // ADC_BITS_2_3 - [0:08:40.774]
        {0x30E6, 0xC4CC},     // ADC_CONFIG1 - [0:08:40.780]
        {0x30E8, 0x8050},     // ADC_CONFIG2 - [0:08:40.787]
        {GADI_VI_SENSOR_TABLE_FLAG_MSLEEP, 200},//sleep 200ms
        {0x3082, 0x0029},     // OPERATION_MODE_CTRL - [0:08:40.988]
        {0x301E, 0x00C8},     // DATA_PEDESTAL - [0:08:41.004]
        {0x3EDA, 0x0F03},     // DAC_LD_14_15 - [0:08:41.011]
        {0x3EDE, 0xC005},     // DAC_LD_18_19 - [0:08:41.018]
        {0x3ED8, 0x09EF},     // DAC_LD_12_13 - [0:08:41.024]
        {0x3EE2, 0xA46B},     // DAC_LD_22_23 - [0:08:41.031]
        {0x3EE0, 0x047D},     // DAC_LD_20_21 - [0:08:41.038]
        {0x3EDC, 0x0070},     // DAC_LD_16_17 - [0:08:41.044]
        {0x3044, 0x0404},     // DARK_CONTROL - [0:08:41.051]
        {0x3EE6, 0x4303},     // DAC_LD_26_27 - [0:08:41.057]
        {0x3EE4, 0xD208},     // DAC_LD_24_25 - [0:08:41.064]
        {0x3ED6, 0x00BD},     // DAC_LD_10_11 - [0:08:41.071]
        {0x30B0, 0x1300},     // DIGITAL_TEST - [0:08:41.077]
        {0x30D4, 0xE007},     // COLUMN_CORRECTION - [0:08:41.084]
        {0x3012, 0x02A0},     // COARSE_INTEGRATION_TIME - [0:08:41.090]
        {0x3032, 0x0000},     // DIGITAL_BINNING - [0:08:41.103]
        {0x301A, 0x10D8},     // RESET_REGISTER - [0:08:41.177]
        {0x31D0, 0x0001},     // HDR_COMP - [0:08:41.192]
        {0x3064, 0x1802},
        {0x30B0, 0x1300},     // DIGITAL_TEST - [0:08:41.232]
        {0x30BA, 0x0008},

        {0x30C6, 0x01CF},
        {0x30C8, 0x01BB},
        {0x30B4, 0x0011, 0x0011},
        {GADI_VI_SENSOR_TABLE_FLAG_MSLEEP, 100},//sleep 100ms
        {0x301A, 0x10DC},     // RESET_REGISTER - [0:08:41.338]
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .pll_table =
    {
        [0] =
        {
            // for 720P@60, 720P@50
            .pixclk = 74250000, // pixclk = (extclk * M) / (N * P1 * P2)
            .extclk = 27000000, //        = (27 * 99) / (6 * 1 * 6) = 74.25MHz
                                // fVCO = (extclk * M) / N = (27 * 99) / 6 = 445.5MHz
                                // (384<=fVCO<=768)
            .regs =
            {
                {0x302A, 0x06}, // P2 = 6  ( 4<=P2<= 16)
                {0x302C, 0x01}, // P1 = 1  ( 1<=P1<= 16)
                {0x302E, 0x06}, // N  = 6  ( 1<= N<= 64)
                {0x3030, 0x63}, // M  = 99 (32<= M<=384)
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
        [1] =
        {
            // for 960P@30, 960P@25
            .pixclk = 54000000, // pixclk = (extclk * M) / (N * P1 * P2)
            .extclk = 27000000, //        = (27 * 84) / (6 * 1 * 7) = 52MHz
                                // fVCO = (extclk * M) / N = (27 * 84) / 6 = 378MHz
                                // (384<=fVCO<=768)
            .regs =
            {
                {0x302A, 0x07}, // P2 = 7  ( 4<=P2<= 16)
                {0x302C, 0x01}, // P1 = 1  ( 1<=P1<= 16)
                {0x302E, 0x06}, // N  = 6  ( 1<= N<= 64)
                {0x3030, 0x54}, // M  = 84 (32<= M<=384)
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
        [2] =
        {
            // for 720P@30, 720P@25
            .pixclk = 37125000, // pixclk = (extclk * M) / (N * P1 * P2)
            .extclk = 27000000, //        = (27 * 66) / (6 * 1 * 8) = 37.125MHz
                                // fVCO = (extclk * M) / N = (27 * 66) / 6 = 297MHz
                                // (384<=fVCO<=768)
            .regs =
            {
                {0x302A, 0x08}, // P2 = 8  ( 4<=P2<= 16)
                {0x302C, 0x01}, // P1 = 1  ( 1<=P1<= 16)
                {0x302E, 0x06}, // N  = 6  ( 1<= N<= 64)
                {0x3030, 0x42}, // M  = 66 (32<= M<=384)
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
                {0x300C, 0x0708},   //AR0130_LINE_LENGTH_PCK        1800 = 0x0708
                {0x300A, 0x03E8},   //AR0130_FRAME_LENGTH_LINES     1000 = 0x03E8
                {0x3002, 0x0004},   //AR0130_Y_ADDR_START,
                {0x3004, 0x0002},   //AR0130_X_ADDR_START,
                {0x3006, 0x03C3},   //AR0130_Y_ADDR_END,            0x03C3 - 4 + 1 = 0x03C0 = 960
                {0x3008, 0x0501},   //AR0130_X_ADDR_END,            0x0501 - 2 + 1 = 0x0500 = 1280
                {0x30A6, 0x0001},   //AR0130_Y_ODD_INC,
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
                {0x300C, 0x0870},   //AR0130_LINE_LENGTH_PCK        2160 = 0x0870
                {0x300A, 0x03E8},   //AR0130_FRAME_LENGTH_LINES     1000 = 0x03E8
                {0x3002, 0x0004},   //AR0130_Y_ADDR_START,
                {0x3004, 0x0002},   //AR0130_X_ADDR_START,
                {0x3006, 0x03C3},   //AR0130_Y_ADDR_END,            0x03C3 - 4 + 1 = 0x03C0 = 960
                {0x3008, 0x0501},   //AR0130_X_ADDR_END,            0x0501 - 2 + 1 = 0x0500 = 1280
                {0x30A6, 0x0001},   //AR0130_Y_ODD_INC,
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
                {0x300C, 0x0672},   //AR0130_LINE_LENGTH_PCK        1650 = 0x0672
                {0x300A, 0x02EE},   //AR0130_FRAME_LENGTH_LINES      750 = 0x02EE
                {0x3002, 0x0004},   //AR0130_Y_ADDR_START,
                {0x3004, 0x0002},   //AR0130_X_ADDR_START,
                {0x3006, 0x02D3},   //AR0130_Y_ADDR_END,            0x02D3 - 4 + 1 = 0x02D0 = 720
                {0x3008, 0x0501},   //AR0130_X_ADDR_END,            0x0501 - 2 + 1 = 0x0500 = 1280
                {0x30A6, 0x0001},   //AR0130_Y_ODD_INC,
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
                {0x300C, 0x07BC},   //AR0130_LINE_LENGTH_PCK        1980 = 0x07BC
                {0x300A, 0x02EE},   //AR0130_FRAME_LENGTH_LINES      750 = 0x02EE
                {0x3002, 0x0004},   //AR0130_Y_ADDR_START,
                {0x3004, 0x0002},   //AR0130_X_ADDR_START,
                {0x3006, 0x02D3},   //AR0130_Y_ADDR_END,            0x02D3 - 4 + 1 = 0x02D0 = 720
                {0x3008, 0x0501},   //AR0130_X_ADDR_END,            0x0501 - 2 + 1 = 0x0500 = 1280
                {0x30A6, 0x0001},   //AR0130_Y_ODD_INC,
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
                {0x300C, 0x0672},   //AR0130_LINE_LENGTH_PCK        1650 = 0x0672
                {0x300A, 0x02EE},   //AR0130_FRAME_LENGTH_LINES      750 = 0x02EE
                {0x3002, 0x0004},   //AR0130_Y_ADDR_START,
                {0x3004, 0x0002},   //AR0130_X_ADDR_START,
                {0x3006, 0x02D3},   //AR0130_Y_ADDR_END,            0x02D3 - 4 + 1 = 0x02D0 = 720
                {0x3008, 0x0501},   //AR0130_X_ADDR_END,            0x0501 - 2 + 1 = 0x0500 = 1280
                {0x30A6, 0x0001},   //AR0130_Y_ODD_INC,
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
                {0x300C, 0x07BC},   //AR0130_LINE_LENGTH_PCK        1980 = 0x07BC
                {0x300A, 0x02EE},   //AR0130_FRAME_LENGTH_LINES      750 = 0x02EE
                {0x3002, 0x0004},   //AR0130_Y_ADDR_START,
                {0x3004, 0x0002},   //AR0130_X_ADDR_START,
                {0x3006, 0x02D3},   //AR0130_Y_ADDR_END,            0x02D3 - 4 + 1 = 0x02D0 = 720
                {0x3008, 0x0501},   //AR0130_X_ADDR_END,            0x0501 - 2 + 1 = 0x0500 = 1280
                {0x30A6, 0x0001},   //AR0130_Y_ODD_INC,
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
#define AR0130_HORIZ_MIRROR     0x4000
#define AR0130_VERT_FLIP        0x8000
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY_VERTICALLY] =
        {
            {0x3040, AR0130_HORIZ_MIRROR, AR0130_HORIZ_MIRROR},
            {0x301A, 0x10D8},
            {0x30D4, 0x6007},
            {0x301A, 0x10DC},
            {GADI_VI_SENSOR_TABLE_FLAG_MSLEEP, 400},
            {0x301A, 0x10D8},
            {0x30D4, 0xE007},
            {0x301A, 0x10DC},
            {0x3040, AR0130_VERT_FLIP, AR0130_VERT_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY] =
        {
            {0x3040, AR0130_HORIZ_MIRROR, AR0130_HORIZ_MIRROR},
            {0x301A, 0x10D8},
            {0x30D4, 0x6007},
            {0x301A, 0x10DC},
            {GADI_VI_SENSOR_TABLE_FLAG_MSLEEP, 400},
            {0x301A, 0x10D8},
            {0x30D4, 0xE007},
            {0x301A, 0x10DC},
            {0x3040, 0x00, AR0130_VERT_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_VERTICALLY] =
        {
            {0x3040, 0x00, AR0130_HORIZ_MIRROR},
            {0x301A, 0x10D8},
            {0x30D4, 0x6007},
            {0x301A, 0x10DC},
            {GADI_VI_SENSOR_TABLE_FLAG_MSLEEP, 400},
            {0x301A, 0x10D8},
            {0x30D4, 0xE007},
            {0x301A, 0x10DC},
            {0x3040, AR0130_VERT_FLIP, AR0130_VERT_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_NONE] =
        {
            {0x3040, 0x00, AR0130_HORIZ_MIRROR},
            {0x301A, 0x10D8},
            {0x30D4, 0x6007},
            {0x301A, 0x10DC},
            {GADI_VI_SENSOR_TABLE_FLAG_MSLEEP, 400},
            {0x301A, 0x10D8},
            {0x30D4, 0xE007},
            {0x301A, 0x10DC},
            {0x3040, 0x00, AR0130_VERT_FLIP},
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
    .shs_fix        = 0,
    .shs_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_SET,
    .agc_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_SET,
    .vmax_reg_update_mode       = GADI_SENSOR_UPDATE_REG_BY_SET,
    .mirror_reg_update_mode     = GADI_SENSOR_UPDATE_REG_BY_SET,
    .shs_additional_use         = 0,
    .shs_additional_percent     = 0,
    .hmax_reg =
    {
        {0x300C, 0x0000, 0xFFFF, 0x00, 0x00, 0}, // AR0130_LINE_LENGTH_PCK
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .vmax_reg =
    {
        {0x300A, 0x0000, 0xFFFF, 0x00, 0x00, 0}, // AR0130_FRAME_LENGTH_LINES
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .vmax_wadd_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    .shs_reg =
    {
        {0x3012, 0x0000, 0xFFFF, 0x00, 0x00, 0}, // AR0130_COARSE_INTEGRATION_TIME
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .shs_additional_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    .cal_gain_mode              = GADI_SENSOR_CAL_GAIN_MODE2,
    .max_agc_index = 241,
    .max_again_index = 0,
    .cur_again_index = 0,
    .agc_info =
    {
        .db_max     = 0x2D000000,    // 45dB
        .db_min     = 0x00000000,    // 0dB
        .db_step    = 0x00300000,    // 0.1875dB
    },
    .gain_reg =
    {
        {0x3100, 0x00}, // AR0130_AE_CTRL_REG
        {0x30B0, 0x00}, // AR0130_ANALOG_GAIN
        {0x305E, 0x00}, // AR0130_GLOBAL_GAIN
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
        {0x00300000 * 0x00F0, {0x0004, 0x1330, 0x00FF},},  /* 45      dB */
        {0x00300000 * 0x00EF, {0x0004, 0x1330, 0x00FB},},  /* 35.8125 dB +9 dB */
        {0x00300000 * 0x00EE, {0x0004, 0x1330, 0x00F5},},  /* 35.625  dB +9 dB */
        {0x00300000 * 0x00ED, {0x0004, 0x1330, 0x00F0},},  /* 35.4375 dB +9 dB */
        {0x00300000 * 0x00EC, {0x0004, 0x1330, 0x00EB},},  /* 35.25   dB +9 dB */
        {0x00300000 * 0x00EB, {0x0004, 0x1330, 0x00E6},},  /* 35.0625 dB +9 dB */
        {0x00300000 * 0x00EA, {0x0004, 0x1330, 0x00E1},},  /* 34.875  dB +9 dB */
        {0x00300000 * 0x00E9, {0x0004, 0x1330, 0x00DC},},  /* 34.6875 dB +9 dB */
        {0x00300000 * 0x00E8, {0x0004, 0x1330, 0x00D7},},  /* 34.5    dB +9 dB */
        {0x00300000 * 0x00E7, {0x0004, 0x1330, 0x00D3},},  /* 34.3125 dB +9 dB */
        {0x00300000 * 0x00E6, {0x0004, 0x1330, 0x00CE},},  /* 34.125  dB +9 dB */
        {0x00300000 * 0x00E5, {0x0004, 0x1330, 0x00CA},},  /* 33.9375 dB +9 dB */
        {0x00300000 * 0x00E4, {0x0004, 0x1330, 0x00C5},},  /* 33.75   dB +9 dB */
        {0x00300000 * 0x00E3, {0x0004, 0x1330, 0x00C1},},  /* 33.5625 dB +9 dB */
        {0x00300000 * 0x00E2, {0x0004, 0x1330, 0x00BD},},  /* 33.375  dB +9 dB */
        {0x00300000 * 0x00E1, {0x0004, 0x1330, 0x00B9},},  /* 33.1875 dB +9 dB */
        {0x00300000 * 0x00E0, {0x0004, 0x1330, 0x00B5},},  /* 42      dB */
        {0x00300000 * 0x00DF, {0x0004, 0x1330, 0x00B1},},  /* 32.8125 dB +9 dB */
        {0x00300000 * 0x00DE, {0x0004, 0x1330, 0x00AD},},  /* 32.625  dB +9 dB */
        {0x00300000 * 0x00DD, {0x0004, 0x1330, 0x00AA},},  /* 32.4375 dB +9 dB */
        {0x00300000 * 0x00DC, {0x0004, 0x1330, 0x00A6},},  /* 32.25   dB +9 dB */
        {0x00300000 * 0x00DB, {0x0004, 0x1330, 0x00A2},},  /* 32.0625 dB +9 dB */
        {0x00300000 * 0x00DA, {0x0004, 0x1330, 0x009F},},  /* 31.875  dB +9 dB */
        {0x00300000 * 0x00D9, {0x0004, 0x1330, 0x009C},},  /* 31.6875 dB +9 dB */
        {0x00300000 * 0x00D8, {0x0004, 0x1330, 0x0098},},  /* 31.5    dB +9 dB */
        {0x00300000 * 0x00D7, {0x0004, 0x1330, 0x0095},},  /* 31.3125 dB +9 dB */
        {0x00300000 * 0x00D6, {0x0004, 0x1330, 0x0092},},  /* 31.125  dB +9 dB */
        {0x00300000 * 0x00D5, {0x0004, 0x1330, 0x008F},},  /* 30.9375 dB +9 dB */
        {0x00300000 * 0x00D4, {0x0004, 0x1330, 0x008C},},  /* 30.75   dB +9 dB */
        {0x00300000 * 0x00D3, {0x0004, 0x1330, 0x0089},},  /* 30.5625 dB +9 dB */
        {0x00300000 * 0x00D2, {0x0004, 0x1330, 0x0086},},  /* 30.375  dB +9 dB */
        {0x00300000 * 0x00D1, {0x0004, 0x1330, 0x0083},},  /* 30.1875 dB +9 dB */
        {0x00300000 * 0x00D0, {0x0004, 0x1330, 0x0080},},  /* 39      dB */
        {0x00300000 * 0x00CF, {0x0004, 0x1330, 0x007D},},  /* 29.8125 dB +9 dB */
        {0x00300000 * 0x00CE, {0x0004, 0x1330, 0x007B},},  /* 29.625  dB +9 dB */
        {0x00300000 * 0x00CD, {0x0004, 0x1330, 0x0078},},  /* 29.4375 dB +9 dB */
        {0x00300000 * 0x00CC, {0x0004, 0x1330, 0x0075},},  /* 29.25   dB +9 dB */
        {0x00300000 * 0x00CB, {0x0004, 0x1330, 0x0073},},  /* 29.0625 dB +9 dB */
        {0x00300000 * 0x00CA, {0x0004, 0x1330, 0x0070},},  /* 28.875  dB +9 dB */
        {0x00300000 * 0x00C9, {0x0004, 0x1330, 0x006E},},  /* 28.6875 dB +9 dB */
        {0x00300000 * 0x00C8, {0x0004, 0x1330, 0x006C},},  /* 28.5    dB +9 dB */
        {0x00300000 * 0x00C7, {0x0004, 0x1330, 0x0069},},  /* 28.3125 dB +9 dB */
        {0x00300000 * 0x00C6, {0x0004, 0x1330, 0x0067},},  /* 28.125  dB +9 dB */
        {0x00300000 * 0x00C5, {0x0004, 0x1330, 0x0065},},  /* 27.9375 dB +9 dB */
        {0x00300000 * 0x00C4, {0x0004, 0x1330, 0x0063},},  /* 27.75   dB +9 dB */
        {0x00300000 * 0x00C3, {0x0004, 0x1330, 0x0061},},  /* 27.5625 dB +9 dB */
        {0x00300000 * 0x00C2, {0x0004, 0x1330, 0x005F},},  /* 27.375  dB +9 dB */
        {0x00300000 * 0x00C1, {0x0004, 0x1330, 0x005C},},  /* 27.1875 dB +9 dB */
        {0x00300000 * 0x00C0, {0x0004, 0x1330, 0x005B},},  /* 36      dB */
        {0x00300000 * 0x00BF, {0x0004, 0x1330, 0x0059},},  /* 26.8125 dB +9 dB */
        {0x00300000 * 0x00BE, {0x0004, 0x1330, 0x0057},},  /* 26.625  dB +9 dB */
        {0x00300000 * 0x00BD, {0x0004, 0x1330, 0x0055},},  /* 26.4375 dB +9 dB */
        {0x00300000 * 0x00BC, {0x0004, 0x1330, 0x0053},},  /* 26.25   dB +9 dB */
        {0x00300000 * 0x00BB, {0x0004, 0x1330, 0x0051},},  /* 26.0625 dB +9 dB */
        {0x00300000 * 0x00BA, {0x0004, 0x1330, 0x004F},},  /* 25.875  dB +9 dB */
        {0x00300000 * 0x00B9, {0x0004, 0x1330, 0x004E},},  /* 25.6875 dB +9 dB */
        {0x00300000 * 0x00B8, {0x0004, 0x1330, 0x004C},},  /* 25.5    dB +9 dB */
        {0x00300000 * 0x00B7, {0x0004, 0x1330, 0x004A},},  /* 25.3125 dB +9 dB */
        {0x00300000 * 0x00B6, {0x0004, 0x1330, 0x0049},},  /* 25.125  dB +9 dB */
        {0x00300000 * 0x00B5, {0x0004, 0x1330, 0x0047},},  /* 24.9375 dB +9 dB */
        {0x00300000 * 0x00B4, {0x0004, 0x1330, 0x0046},},  /* 24.75   dB +9 dB */
        {0x00300000 * 0x00B3, {0x0004, 0x1330, 0x0044},},  /* 24.5625 dB +9 dB */
        {0x00300000 * 0x00B2, {0x0004, 0x1330, 0x0043},},  /* 24.375  dB +9 dB */
        {0x00300000 * 0x00B1, {0x0004, 0x1330, 0x0041},},  /* 24.1875 dB +9 dB */
        {0x00300000 * 0x00B0, {0x0004, 0x1330, 0x0040},},  /* 33      dB */
        {0x00300000 * 0x00AF, {0x0004, 0x1330, 0x003F},},  /* 23.8125 dB +9 dB */
        {0x00300000 * 0x00AE, {0x0004, 0x1330, 0x003D},},  /* 23.625  dB +9 dB */
        {0x00300000 * 0x00AD, {0x0004, 0x1330, 0x003C},},  /* 23.4375 dB +9 dB */
        {0x00300000 * 0x00AC, {0x0004, 0x1330, 0x003B},},  /* 23.25   dB +9 dB */
        {0x00300000 * 0x00AB, {0x0004, 0x1330, 0x0039},},  /* 23.0625 dB +9 dB */
        {0x00300000 * 0x00AA, {0x0004, 0x1330, 0x0038},},  /* 2.875   dB +9 dB */
        {0x00300000 * 0x00A9, {0x0004, 0x1330, 0x0037},},  /* 22.6875 dB +9 dB */
        {0x00300000 * 0x00A8, {0x0004, 0x1330, 0x0036},},  /* 22.5    dB +9 dB */
        {0x00300000 * 0x00A7, {0x0004, 0x1330, 0x0035},},  /* 22.3125 dB +9 dB */
        {0x00300000 * 0x00A6, {0x0004, 0x1330, 0x0034},},  /* 22.125  dB +9 dB */
        {0x00300000 * 0x00A5, {0x0004, 0x1330, 0x0032},},  /* 21.9375 dB +9 dB */
        {0x00300000 * 0x00A4, {0x0004, 0x1330, 0x0031},},  /* 21.75   dB +9 dB */
        {0x00300000 * 0x00A3, {0x0004, 0x1330, 0x0030},},  /* 21.5625 dB +9 dB */
        {0x00300000 * 0x00A2, {0x0004, 0x1330, 0x002F},},  /* 21.375  dB +9 dB */
        {0x00300000 * 0x00A1, {0x0004, 0x1330, 0x002E},},  /* 21.1875 dB +9 dB */
        {0x00300000 * 0x00A0, {0x0004, 0x1330, 0x002D},},  /* 30      dB */
        {0x00300000 * 0x009F, {0x0004, 0x1320, 0x0059},},  /* 5.0625  dB */
        {0x00300000 * 0x009E, {0x0004, 0x1320, 0x0057},},  /* 4.6875  dB */
        {0x00300000 * 0x009D, {0x0004, 0x1320, 0x0055},},  /* 4.3125  dB */
        {0x00300000 * 0x009C, {0x0004, 0x1320, 0x0054},},  /* 4.125   dB */
        {0x00300000 * 0x009B, {0x0004, 0x1320, 0x0053},},  /* 4.125   dB */
        {0x00300000 * 0x009A, {0x0004, 0x1320, 0x0051},},  /* 3.75    dB */
        {0x00300000 * 0x0099, {0x0004, 0x1320, 0x0050},},  /* 3.5625  dB */
        {0x00300000 * 0x0098, {0x0004, 0x1320, 0x004F},},  /* 0.375   dB */
        {0x00300000 * 0x0097, {0x0004, 0x1320, 0x004D},},  /* 0       dB */
        {0x00300000 * 0x0096, {0x0004, 0x1320, 0x004B},},  /* 5.625   dB *///2.8125
        {0x00300000 * 0x0095, {0x0004, 0x1320, 0x004A},},  /* 5.625   dB */
        {0x00300000 * 0x0094, {0x0004, 0x1320, 0x0048},},  /* 5.25    dB */
        {0x00300000 * 0x0093, {0x0004, 0x1320, 0x0047},},  /* 4.125   dB */
        {0x00300000 * 0x0092, {0x0004, 0x1320, 0x0046},},  /* 5.0625  dB */
        {0x00300000 * 0x0091, {0x0004, 0x1320, 0x0044},},  /* 4.6875  dB */
        {0x00300000 * 0x0090, {0x0004, 0x1320, 0x0043},},  /* 4.5     dB */
        {0x00300000 * 0x008F, {0x0004, 0x1320, 0x0041},},  /* 4.125   dB *///1.875
        {0x00300000 * 0x008E, {0x0004, 0x1320, 0x0040},},  /* 4.125   dB */
        {0x00300000 * 0x008D, {0x0004, 0x1320, 0x003F},},  /* 9+12+5.8125    dB */
        {0x00300000 * 0x008C, {0x0004, 0x1320, 0x003E},},  /* 3.75    dB */
        {0x00300000 * 0x008B, {0x0004, 0x1320, 0x003D},},  /* 3.5625  dB */
        {0x00300000 * 0x008A, {0x0004, 0x1320, 0x003B},},  /* 3.1875  dB */
        {0x00300000 * 0x0089, {0x0004, 0x1320, 0x0039},},  /* 2.8125  dB */
        {0x00300000 * 0x0088, {0x0004, 0x1320, 0x0038},},  /* 2.625   dB */
        {0x00300000 * 0x0087, {0x0004, 0x1320, 0x0036},},  /* 2.0625  dB */
        {0x00300000 * 0x0086, {0x0004, 0x1320, 0x0035},},  /* 1.875   dB */
        {0x00300000 * 0x0085, {0x0004, 0x1320, 0x0033},},  /* 1.5     dB */
        {0x00300000 * 0x0084, {0x0004, 0x1320, 0x0032},},  /* 1.3125  dB */
        {0x00300000 * 0x0083, {0x0004, 0x1320, 0x0030},},  /* 0.75    dB */
        {0x00300000 * 0x0082, {0x0004, 0x1320, 0x002F},},  /* 0.375   dB */
        {0x00300000 * 0x0081, {0x0004, 0x1320, 0x002E},},  /* 12+0.09375 db*/
        {0x00300000 * 0x0080, {0x0004, 0x1320, 0x002D},},  /* 9+12+3=24dB */
        {0x00300000 * 0x007F, {0x0004, 0x1310, 0x0059},},  /* 5.0625  dB */
        {0x00300000 * 0x007E, {0x0004, 0x1310, 0x0058},},  /* 4.875   dB */
        {0x00300000 * 0x007D, {0x0004, 0x1310, 0x0057},},  /* 4.6875  dB */
        {0x00300000 * 0x007C, {0x0004, 0x1310, 0x0056},},  /* 4.5     dB */
        {0x00300000 * 0x007B, {0x0004, 0x1310, 0x0055},},  /* 4.3125  dB */
        {0x00300000 * 0x007A, {0x0004, 0x1310, 0x0054},},  /* 4.125   dB */
        {0x00300000 * 0x0079, {0x0004, 0x1310, 0x0053},},  /* 4.125   dB */
        {0x00300000 * 0x0078, {0x0004, 0x1310, 0x0052},},  /* 3.9375  dB *///4.6875
        {0x00300000 * 0x0077, {0x0004, 0x1310, 0x0051},},  /* 3.75    dB */
        {0x00300000 * 0x0076, {0x0004, 0x1310, 0x0050},},  /* 3.5625  dB */
        {0x00300000 * 0x0075, {0x0004, 0x1310, 0x004E},},  /* 0.09375 dB */
        {0x00300000 * 0x0074, {0x0004, 0x1310, 0x004C},},  /* 5.8125  dB */
        {0x00300000 * 0x0073, {0x0004, 0x1310, 0x004B},},  /* 5.625   dB *///2.8125
        {0x00300000 * 0x0072, {0x0004, 0x1310, 0x004A},},  /* 5.625   dB */
        {0x00300000 * 0x0071, {0x0004, 0x1310, 0x0048},},  /* 5.25    dB */
        {0x00300000 * 0x0070, {0x0004, 0x1310, 0x0046},},  /* 5.0625  dB */
        {0x00300000 * 0x006F, {0x0004, 0x1310, 0x0044},},  /* 4.6875  dB */
        {0x00300000 * 0x006E, {0x0004, 0x1310, 0x0042},},  /* 4.3125  dB */
        {0x00300000 * 0x006D, {0x0004, 0x1310, 0x0040},},  /* 4.125   dB */
        {0x00300000 * 0x006C, {0x0004, 0x1310, 0x003F},},  /* 9+6+5.8125 dB */
        {0x00300000 * 0x006B, {0x0004, 0x1310, 0x003D},},  /* 5.625   dB */
        {0x00300000 * 0x006A, {0x0004, 0x1310, 0x003B},},  /* 5.25    dB */
        {0x00300000 * 0x0069, {0x0004, 0x1310, 0x003A},},  /* 4.125   dB */
        {0x00300000 * 0x0068, {0x0004, 0x1310, 0x0038},},  /* 4.875   dB */
        {0x00300000 * 0x0067, {0x0004, 0x1310, 0x0036},},  /* 4.5     dB */
        {0x00300000 * 0x0066, {0x0004, 0x1310, 0x0035},},  /* 4.3125  dB */
        {0x00300000 * 0x0065, {0x0004, 0x1310, 0x0034},},  /* 4.125   dB *///1.875
        {0x00300000 * 0x0064, {0x0004, 0x1310, 0x0032},},  /* 3.9375  dB */
        {0x00300000 * 0x0063, {0x0004, 0x1310, 0x0031},},  /* 3.75    dB */
        {0x00300000 * 0x0062, {0x0004, 0x1310, 0x0030},},  /* 3.5625  dB */
        {0x00300000 * 0x0061, {0x0004, 0x1310, 0x002E},},  /* 3.1875  dB */
        {0x00300000 * 0x0060, {0x0004, 0x1310, 0x002D},},  /* 9+6+3=18dB */
        {0x00300000 * 0x005F, {0x0004, 0x1300, 0x0059},},  /* 5.0625  dB */
        {0x00300000 * 0x005E, {0x0004, 0x1300, 0x0058},},  /* 4.875   dB */
        {0x00300000 * 0x005D, {0x0004, 0x1300, 0x0057},},  /* 4.6875  dB */
        {0x00300000 * 0x005C, {0x0004, 0x1300, 0x0055},},  /* 4.3125  dB */
        {0x00300000 * 0x005B, {0x0004, 0x1300, 0x0054},},  /* 4.125   dB */
        {0x00300000 * 0x005A, {0x0004, 0x1300, 0x0053},},  /* 4.125   dB */
        {0x00300000 * 0x0059, {0x0004, 0x1300, 0x0051},},  /* 3.75    dB */
        {0x00300000 * 0x0058, {0x0004, 0x1300, 0x0050},},  /* 3.5625  dB */
        {0x00300000 * 0x0057, {0x0004, 0x1300, 0x004F},},  /* 0.375   dB */
        {0x00300000 * 0x0056, {0x0004, 0x1300, 0x004D},},  /* 0       dB */
        {0x00300000 * 0x0055, {0x0004, 0x1300, 0x004C},},  /* 5.8125  dB */
        {0x00300000 * 0x0054, {0x0004, 0x1300, 0x004A},},  /* 5.625   dB */
        {0x00300000 * 0x0053, {0x0004, 0x1300, 0x0049},},  /* 5.4375  dB */
        {0x00300000 * 0x0052, {0x0004, 0x1300, 0x0048},},  /* 5.25    dB */
        {0x00300000 * 0x0051, {0x0004, 0x1300, 0x0047},},  /* 4.125   dB */
        {0x00300000 * 0x0050, {0x0004, 0x1300, 0x0046},},  /* 5.0625  dB */
        {0x00300000 * 0x004F, {0x0004, 0x1300, 0x0045},},  /* 4.875   dB */
        {0x00300000 * 0x004E, {0x0004, 0x1300, 0x0044},},  /* 4.6875  dB */
        {0x00300000 * 0x004D, {0x0004, 0x1300, 0x0043},},  /* 4.5     dB */
        {0x00300000 * 0x004C, {0x0004, 0x1300, 0x0042},},  /* 4.3125  dB */
        {0x00300000 * 0x004B, {0x0004, 0x1300, 0x0040},},  /* 4.125   dB */
        {0x00300000 * 0x004A, {0x0004, 0x1300, 0x003E},},  /* 5.625   dB *///2.8125
        {0x00300000 * 0x0049, {0x0004, 0x1300, 0x003C},},  /* 5.4375  dB */
        {0x00300000 * 0x0048, {0x0004, 0x1300, 0x003A},},  /* 4.125   dB */
        {0x00300000 * 0x0047, {0x0004, 0x1300, 0x0038},},  /* 4.875   dB */
        {0x00300000 * 0x0046, {0x0004, 0x1300, 0x0036},},  /* 4.5     dB */
        {0x00300000 * 0x0045, {0x0004, 0x1300, 0x0035},},  /* 4.3125  dB */
        {0x00300000 * 0x0044, {0x0004, 0x1300, 0x0033},},  /* 4.125   dB */
        {0x00300000 * 0x0043, {0x0004, 0x1300, 0x0031},},  /* 3.75    dB */
        {0x00300000 * 0x0042, {0x0004, 0x1300, 0x0030},},  /* 3.5625  dB */
        {0x00300000 * 0x0041, {0x0004, 0x1300, 0x002E},},  /* 3.1875  dB */
        {0x00300000 * 0x0040, {0x0004, 0x1300, 0x002D},},  /* 9+3=12  dB */
        {0x00300000 * 0x003F, {0x0000, 0x1310, 0x003F},},  /* 6+5.8125 =11.8125    dB */
        {0x00300000 * 0x003E, {0x0000, 0x1310, 0x003E},},  /* 5.625   dB *///2.8125
        {0x00300000 * 0x003D, {0x0000, 0x1310, 0x003D},},  /* 5.625   dB */
        {0x00300000 * 0x003C, {0x0000, 0x1310, 0x003C},},  /* 5.4375  dB */
        {0x00300000 * 0x003B, {0x0000, 0x1310, 0x003B},},  /* 5.25    dB */
        {0x00300000 * 0x003A, {0x0000, 0x1310, 0x003A},},  /* 4.125   dB */
        {0x00300000 * 0x0039, {0x0000, 0x1310, 0x0039},},  /* 5.0625  dB */
        {0x00300000 * 0x0038, {0x0000, 0x1310, 0x0038},},  /* 4.875   dB */
        {0x00300000 * 0x0037, {0x0000, 0x1310, 0x0037},},  /* 4.6875  dB */
        {0x00300000 * 0x0036, {0x0000, 0x1310, 0x0036},},  /* 4.5     dB */
        {0x00300000 * 0x0035, {0x0000, 0x1310, 0x0035},},  /* 4.3125  dB */
        {0x00300000 * 0x0034, {0x0000, 0x1310, 0x0034},},  /* 4.125   dB *///1.875
        {0x00300000 * 0x0033, {0x0000, 0x1310, 0x0033},},  /* 4.125   dB */
        {0x00300000 * 0x0032, {0x0000, 0x1310, 0x0032},},  /* 3.9375  dB */
        {0x00300000 * 0x0031, {0x0000, 0x1310, 0x0031},},  /* 3.75    dB */
        {0x00300000 * 0x0030, {0x0000, 0x1310, 0x0030},},  /* 3.5625  dB */
        {0x00300000 * 0x002F, {0x0000, 0x1310, 0x002F},},  /* 3.375   dB */
        {0x00300000 * 0x002E, {0x0000, 0x1310, 0x002E},},  /* 3.1875  dB */
        {0x00300000 * 0x002D, {0x0000, 0x1310, 0x002D},},  /* 3       dB */
        {0x00300000 * 0x002C, {0x0000, 0x1310, 0x002C},},  /* 2.8125  dB */
        {0x00300000 * 0x002B, {0x0000, 0x1310, 0x002B},},  /* 2.625   dB */
        {0x00300000 * 0x002A, {0x0000, 0x1310, 0x002A},},  /* 0.9375  dB */
        {0x00300000 * 0x0029, {0x0000, 0x1310, 0x0029},},  /* 2.0625  dB */
        {0x00300000 * 0x0028, {0x0000, 0x1310, 0x0028},},  /* 1.875   dB */
        {0x00300000 * 0x0027, {0x0000, 0x1310, 0x0027},},  /* 1.6875  dB */
        {0x00300000 * 0x0026, {0x0000, 0x1310, 0x0026},},  /* 1.5     dB */
        {0x00300000 * 0x0025, {0x0000, 0x1310, 0x0025},},  /* 1.3125  dB */
        {0x00300000 * 0x0024, {0x0000, 0x1310, 0x0024},},  /* 0.9375  dB */
        {0x00300000 * 0x0023, {0x0000, 0x1310, 0x0023},},  /* 0.75    dB */
        {0x00300000 * 0x0022, {0x0000, 0x1310, 0x0022},},  /* 0.375   dB */
        {0x00300000 * 0x0021, {0x0000, 0x1310, 0x0021},},  /* 0.09375 dB */
        {0x00300000 * 0x0020, {0x0000, 0x1310, 0x0020},},  /* 6       dB */
        {0x00300000 * 0x001F, {0x0000, 0x1300, 0x003F},},  /* 5.8125  dB */
        {0x00300000 * 0x001E, {0x0000, 0x1300, 0x003E},},  /* 5.625   dB *///2.8125
        {0x00300000 * 0x001D, {0x0000, 0x1300, 0x003D},},  /* 5.625   dB */
        {0x00300000 * 0x001C, {0x0000, 0x1300, 0x003C},},  /* 5.4375  dB */
        {0x00300000 * 0x001B, {0x0000, 0x1300, 0x003B},},  /* 5.25    dB */
        {0x00300000 * 0x001A, {0x0000, 0x1300, 0x003A},},  /* 4.125   dB */
        {0x00300000 * 0x0019, {0x0000, 0x1300, 0x0039},},  /* 5.0625  dB */
        {0x00300000 * 0x0018, {0x0000, 0x1300, 0x0038},},  /* 4.875   dB */
        {0x00300000 * 0x0017, {0x0000, 0x1300, 0x0037},},  /* 4.6875  dB */
        {0x00300000 * 0x0016, {0x0000, 0x1300, 0x0036},},  /* 4.5     dB */
        {0x00300000 * 0x0015, {0x0000, 0x1300, 0x0035},},  /* 4.3125  dB */
        {0x00300000 * 0x0014, {0x0000, 0x1300, 0x0034},},  /* 4.125   dB *///1.875
        {0x00300000 * 0x0013, {0x0000, 0x1300, 0x0033},},  /* 4.125   dB */
        {0x00300000 * 0x0012, {0x0000, 0x1300, 0x0032},},  /* 3.9375  dB */
        {0x00300000 * 0x0011, {0x0000, 0x1300, 0x0031},},  /* 3.75    dB */
        {0x00300000 * 0x0010, {0x0000, 0x1300, 0x0030},},  /* 3.5625  dB */
        {0x00300000 * 0x000F, {0x0000, 0x1300, 0x002F},},  /* 3.375   dB */
        {0x00300000 * 0x000E, {0x0000, 0x1300, 0x002E},},  /* 3.1875  dB */
        {0x00300000 * 0x000D, {0x0000, 0x1300, 0x002D},},  /* 3       dB */
        {0x00300000 * 0x000C, {0x0000, 0x1300, 0x002C},},  /* 2.8125  dB */
        {0x00300000 * 0x000B, {0x0000, 0x1300, 0x002B},},  /* 2.625   dB */
        {0x00300000 * 0x000A, {0x0000, 0x1300, 0x002A},},  /* 0.9375  dB */
        {0x00300000 * 0x0009, {0x0000, 0x1300, 0x0029},},  /* 2.0625  dB */
        {0x00300000 * 0x0008, {0x0000, 0x1300, 0x0028},},  /* 1.875   dB */
        {0x00300000 * 0x0007, {0x0000, 0x1300, 0x0027},},  /* 1.6875  dB */
        {0x00300000 * 0x0006, {0x0000, 0x1300, 0x0026},},  /* 1.5     dB */
        {0x00300000 * 0x0005, {0x0000, 0x1300, 0x0025},},  /* 1.3125  dB */
        {0x00300000 * 0x0004, {0x0000, 0x1300, 0x0024},},  /* 0.9375  dB */
        {0x00300000 * 0x0003, {0x0000, 0x1300, 0x0023},},  /* 0.75    dB */
        {0x00300000 * 0x0002, {0x0000, 0x1300, 0x0022},},  /* 0.375   dB */
        {0x00300000 * 0x0001, {0x0000, 0x1300, 0x0021},},  /* 0.09375 dB */
        {0x00300000 * 0x0000, {0x0000, 0x1300, 0x0020},},  /* 0       dB */
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

