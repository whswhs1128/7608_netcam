/*!
*****************************************************************************
** \file        imx122_reg_tbl.c
**
** \version     $Id: imx122_reg_tbl.c 11627 2017-03-10 10:58:12Z dengbiao $
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
#if 1 // for i2c
GADI_VI_SensorDrvInfoT    imx122_sensor =
{
    .magic_start        = GADI_SENSOR_HW_INFO_MAGIC_START,
    .version            = GADI_SENSOR_HW_INFO_VERSION,
    .length             = sizeof(GADI_VI_SensorDrvInfoT),
    .file_date          = 0x20161227,
    .HwInfo         =
    {
        .name               = "imx122",
        .ctl_type           = GADI_SENSOR_CTL_I2C,
        .addr_len           = sizeof(GADI_U16),
        .data_len           = sizeof(GADI_U8),
        .reset_active_level = GADI_GPIO_LOW,
        .hw_addr            = {(0x34>>1), 0, 0, 0},
        .id_reg =
        {
            {0x0000, 0x01},
            {0x0001, 0x22},
        },
        .reset_reg =
        {
            {0x3000, 0x31}, // BIT[0]STANDBY
                            // 0: Normal operation, 1:STANDBY
                            // BIT[5:4]TESTEN
                            // Register write 0: Invalid, 3:Valid, Others: Setting prohibited
            {0x0100, 0x00}, // BIT[0]MODE_SEL
                            // Standby control 0:STANDBY, 1: Normal operation
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
    .sensor_id          = GADI_ISP_SENSOR_IMX122,
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
    .max_width          = 2080,
    .max_height         = 1553,
    .def_sh_time        = GADI_VIDEO_FPS(50),
    .fmt_num            = 8,
    .auto_fmt           = GADI_VIDEO_MODE(1920, 1080,  25, 1),
    .init_reg           =
    {
        {0x302D, 0x48},// BIT[3]: 10-bit output 2-bit shift(1:right justified)
        // do not set this reg
        // {0x0200, 0x00},
        {0x301F, 0x73}, // 0x33?
        {0x3027, 0x20},
        {0x303b, 0xE0}, // E0h: CMOS output
                        // 90h: LVDS output, synccode1
                        // D0h: LVDS output, synccode2
        /*------Section 3, can't be read----------*/
        {0x3100, 0x01},
        {0x3117, 0x0D},
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .pll_table =
    {
        [0] =
        {
            // for 1080P@30, 1080P@25, 720P@60, 720P@50
            .pixclk = 74250000,
            .extclk = 37125000,
            // pixclk = extclk * FRSEL = extclk * 2
            .regs =
            {
                {0x3011, 0x00}, // BIT[2:0] FRSEL
                                // Output data rate designation
                                // 0: 2 time INCK
                                // 1: Equal to INCK
                                // 2: 1/2 of INCK (2x2 addition mode only)
                                // 3: 1/4 of INCK (2x2 addition mode only)
                                // Others: Setting prohibited
                                // BIT[4:3] OPORTSEL/M12BEN
                                // Output system selection
                                // 0: Parallel CMOS SDR output
                                // 1: Parallel CMOS DDR output
                                // 2: Serial LVDS 1ch output
                                // 3: Serial LVDS 2ch output
                                // BIT[6:5] M12BEN
                                // The output resolution is set to 10 bit (2x2 binning mode only.)
                                // 0: disable 2:enable
                                // Others: Setting prohibited
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
                {0x3011, 0x00}, // BIT[2:0] FRSEL
                                // Output data rate designation
                                // 0: 2 time INCK
                                // 1: Equal to INCK
                                // 2: 1/2 of INCK (2x2 addition mode only)
                                // 3: 1/4 of INCK (2x2 addition mode only)
                                // Others: Setting prohibited
                                // BIT[4:3] OPORTSEL/M12BEN
                                // Output system selection
                                // 0: Parallel CMOS SDR output
                                // 1: Parallel CMOS DDR output
                                // 2: Serial LVDS 1ch output
                                // 3: Serial LVDS 2ch output
                                // BIT[6:5] M12BEN
                                // The output resolution is set to 10 bit (2x2 binning mode only.)
                                // 0: disable 2:enable
                                // Others: Setting prohibited
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
                {0x3011, 0x01}, // BIT[2:0] FRSEL
                                // Output data rate designation
                                // 0: 2 time INCK
                                // 1: Equal to INCK
                                // 2: 1/2 of INCK (2x2 addition mode only)
                                // 3: 1/4 of INCK (2x2 addition mode only)
                                // Others: Setting prohibited
                                // BIT[4:3] OPORTSEL/M12BEN
                                // Output system selection
                                // 0: Parallel CMOS SDR output
                                // 1: Parallel CMOS DDR output
                                // 2: Serial LVDS 1ch output
                                // 3: Serial LVDS 2ch output
                                // BIT[6:5] M12BEN
                                // The output resolution is set to 10 bit (2x2 binning mode only.)
                                // 0: disable 2:enable
                                // Others: Setting prohibited
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
    },
    .video_fmt_table =
    {
        // 1080P IMX122_HMAX = SD(44 + 148 - 4 - 16 - 24 - 8 = 140) + Sync(4) + OB(16) + Ignored(24) + Effective(8) + x + Effective(8) + Ignored(24) + Sync(4) + HB(52)
        // 1080P IMX122_VMAX = Dummy(6) + FI(1) + OB(4) + VDE OB(8) + Ignored(4) + Effective(8) + y + Effective(9) + Ignored(4) + VB(1)
        // 720P  IMX122_HMAX = SD(40 + 220 - 4 - 16 - 24 - 8 = 208) + Sync(4) + OB(16) + Ignored(24) + Effective(8) + x + Effective(8) + Ignored(24) + Sync(4) + HB
        // 720P  IMX122_VMAX = Dummy(6) + FI(1) + OB(4) + VDE OB(6) + Ignored(2) + Effective(4) + y + Effective(5) + Ignored(2)
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
                {0x3002, 0x0F},  // BIT[3:0]MODE
                                 // Readout mode designation
                                 // 0: All pixel scan(2.3M)
                                 // 1: HD720p
                                 // 2: Window cropping
                                 // 3: 2x2 addition
                                 // 4: Vertical 1/2 sub sampling
                                 // F: 1080p
                                 // Others: Setting prohibited
                {0x0343, 0x4C},  // LINE_LENGTH_L=IMX122_HMAX_LSB   1100 = 0x044C
                {0x0342, 0x04},  // LINE_LENGTH_H=IMX122_HMAX_MSB
                {0x0341, 0x65},  // FRM_LENGTH_L=IMX122_VMAX_LSB    1125 = 0x0465
                {0x0340, 0x04},  // FRM_LENGTH_H=IMX122_VMAX_MSB
                {0x3012, 0x82},  // BIT[1] ADRES(0:10bit 1:12bit)
                {0x0345, 0x00},  // X_ADD_STA_L=IMX122_WINPH_LSB
                {0x0344, 0x00},  // X_ADD_STA_H=IMX122_WINPH_MSB
                {0x0347, 0x3C},  // Y_ADD_STA_L=IMX122_WINPV_LSB
                {0x0346, 0x00},  // Y_ADD_STA_H=IMX122_WINPV_MSB
                {0x034D, 0xC0},  // X_OUT_SIZE_L=IMX122_WINWH_LSB   1984 = 0x07C0
                {0x034C, 0x07},  // X_OUT_SIZE_H=IMX122_WINWH_MSB
                {0x034F, 0x51},  // Y_OUT_SIZE_L=IMX122_WINWV_LSB   1105 = 0x0451
                {0x034E, 0x04},  // Y_OUT_SIZE_H=IMX122_WINWV_MSB
                {0x0008, 0x00},  // I2CBLKLEVEL_H=BLKLEVEL Black level offset value setting
                {0x0009, 0xF0},  // I2CBLKLEVEL_L=BLKLEVEL AD10bit=60d = 0x3C AD12bit=240d = 0xF0
                {0x3021, 0x00},  // BIT[7] 10BITA 0:12BIT 1:10BIT
                {0x3022, 0x00},  // BIT[7] 720PMODE 0:1080PMODE 1:720PMODE
                {0x307A, 0x00},  // 10BITB Setting register for 10bit
                {0x307B, 0x00},  // 10BITC Setting register for 10bit
                {0x3098, 0x26},  // 10B1080P_LSB
                {0x3099, 0x02},  // 10B1080P_MSB
                {0x309A, 0x26},  // 12B1080P_LSB
                {0x309B, 0x02},  // 12B1080P_MSB
                {0x30CE, 0x16},  // PRES
                {0x30CF, 0x82},  // DRES_LSB
                {0x30D0, 0x00},  // DRES_MSB
                {0x302C, 0x00},  // BIT[0]XMSTA
                                 // Trigger for master mode operation start
                                 // 0: Master mode operation start
                                 // 1: Trigger standby
                {0x0112, 0x0C},  // RAW_FMT_H Output bit selection 10bit£º0x0A 12bit£º0x0C
                {0x0113, 0x0C},  // RAW_FMT_L Output bit selection 10bit£º0x0A 12bit£º0x0C
                {0x0100, 0x01},  // STANDBY 0:STANDBY, 1: Normal operation
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
                {0x3002, 0x0F},  // BIT[3:0]MODE
                                 // Readout mode designation
                                 // 0: All pixel scan(2.3M)
                                 // 1: HD720p
                                 // 2: Window cropping
                                 // 3: 2x2 addition
                                 // 4: Vertical 1/2 sub sampling
                                 // F: 1080p
                                 // Others: Setting prohibited
                {0x0343, 0x28},  // LINE_LENGTH_L=IMX122_HMAX_LSB   1320 = 0x0528
                {0x0342, 0x05},  // LINE_LENGTH_H=IMX122_HMAX_MSB
                {0x0341, 0x65},  // FRM_LENGTH_L=IMX122_VMAX_LSB    1125 = 0x0465
                {0x0340, 0x04},  // FRM_LENGTH_H=IMX122_VMAX_MSB
                {0x3012, 0x82},  // BIT[1] ADRES(0:10bit 1:12bit)
                {0x0345, 0x00},  // X_ADD_STA_L=IMX122_WINPH_LSB
                {0x0344, 0x00},  // X_ADD_STA_H=IMX122_WINPH_MSB
                {0x0347, 0x3C},  // Y_ADD_STA_L=IMX122_WINPV_LSB
                {0x0346, 0x00},  // Y_ADD_STA_H=IMX122_WINPV_MSB
                {0x034D, 0xC0},  // X_OUT_SIZE_L=IMX122_WINWH_LSB   1984 = 0x07C0
                {0x034C, 0x07},  // X_OUT_SIZE_H=IMX122_WINWH_MSB
                {0x034F, 0x51},  // Y_OUT_SIZE_L=IMX122_WINWV_LSB   1105 = 0x0451
                {0x034E, 0x04},  // Y_OUT_SIZE_H=IMX122_WINWV_MSB
                {0x0008, 0x00},  // I2CBLKLEVEL_H=BLKLEVEL Black level offset value setting
                {0x0009, 0xF0},  // I2CBLKLEVEL_L=BLKLEVEL AD10bit=60d = 0x3C AD12bit=240d = 0xF0
                {0x3021, 0x00},  // BIT[7] 10BITA 0:12BIT 1:10BIT
                {0x3022, 0x00},  // BIT[7] 720PMODE 0:1080PMODE 1:720PMODE
                {0x307A, 0x00},  // 10BITB Setting register for 10bit
                {0x307B, 0x00},  // 10BITC Setting register for 10bit
                {0x3098, 0x26},  // 10B1080P_LSB
                {0x3099, 0x02},  // 10B1080P_MSB
                {0x309A, 0x94},  // 12B1080P_LSB
                {0x309B, 0x02},  // 12B1080P_MSB
                {0x30CE, 0x16},  // PRES
                {0x30CF, 0x82},  // DRES_LSB
                {0x30D0, 0x00},  // DRES_MSB
                {0x302C, 0x00},  // BIT[0]XMSTA
                                 // Trigger for master mode operation start
                                 // 0: Master mode operation start
                                 // 1: Trigger standby
                {0x0112, 0x0C},  // RAW_FMT_H(12BIT) Output bit selection 10bit£º0x0A 12bit£º0x0C
                {0x0113, 0x0C},  // RAW_FMT_L(12BIT) Output bit selection 10bit£º0x0A 12bit£º0x0C
                {0x0100, 0x01},  // STANDBY 0:STANDBY, 1: Normal operation
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
                {0x3002, 0x0F},  // BIT[3:0]MODE
                                 // Readout mode designation
                                 // 0: All pixel scan(2.3M)
                                 // 1: HD720p
                                 // 2: Window cropping
                                 // 3: 2x2 addition
                                 // 4: Vertical 1/2 sub sampling
                                 // F: 1080p
                                 // Others: Setting prohibited
                {0x0343, 0x4C},  // LINE_LENGTH_L=IMX122_HMAX_LSB   1100 = 0x044C
                {0x0342, 0x04},  // LINE_LENGTH_H=IMX122_HMAX_MSB
                {0x0341, 0x65},  // FRM_LENGTH_L=IMX122_VMAX_LSB    1125 = 0x0465
                {0x0340, 0x04},  // FRM_LENGTH_H=IMX122_VMAX_MSB
                {0x3012, 0x82},  // BIT[1] ADRES(0:10bit 1:12bit)
                {0x0345, 0x00},  // X_ADD_STA_L=IMX122_WINPH_LSB
                {0x0344, 0x00},  // X_ADD_STA_H=IMX122_WINPH_MSB
                {0x0347, 0x3C},  // Y_ADD_STA_L=IMX122_WINPV_LSB
                {0x0346, 0x00},  // Y_ADD_STA_H=IMX122_WINPV_MSB
                {0x034D, 0xC0},  // X_OUT_SIZE_L=IMX122_WINWH_LSB   1984 = 0x07C0
                {0x034C, 0x07},  // X_OUT_SIZE_H=IMX122_WINWH_MSB
                {0x034F, 0x51},  // Y_OUT_SIZE_L=IMX122_WINWV_LSB   1105 = 0x0451
                {0x034E, 0x04},  // Y_OUT_SIZE_H=IMX122_WINWV_MSB
                {0x0008, 0x00},  // I2CBLKLEVEL_H=BLKLEVEL Black level offset value setting
                {0x0009, 0xF0},  // I2CBLKLEVEL_L=BLKLEVEL AD10bit=60d = 0x3C AD12bit=240d = 0xF0
                {0x3021, 0x00},  // BIT[7] 10BITA 0:12BIT 1:10BIT
                {0x3022, 0x00},  // BIT[7] 720PMODE 0:1080PMODE 1:720PMODE
                {0x307A, 0x00},  // 10BITB Setting register for 10bit
                {0x307B, 0x00},  // 10BITC Setting register for 10bit
                {0x3098, 0x26},  // 10B1080P_LSB
                {0x3099, 0x02},  // 10B1080P_MSB
                {0x309A, 0x26},  // 12B1080P_LSB
                {0x309B, 0x02},  // 12B1080P_MSB
                {0x30CE, 0x16},  // PRES
                {0x30CF, 0x82},  // DRES_LSB
                {0x30D0, 0x00},  // DRES_MSB
                {0x302C, 0x00},  // BIT[0]XMSTA
                                 // Trigger for master mode operation start
                                 // 0: Master mode operation start
                                 // 1: Trigger standby
                {0x0112, 0x0C},  // RAW_FMT_H(12BIT) Output bit selection 10bit£º0x0A 12bit£º0x0C
                {0x0113, 0x0C},  // RAW_FMT_L(12BIT) Output bit selection 10bit£º0x0A 12bit£º0x0C
                {0x0100, 0x01},  // STANDBY 0:STANDBY, 1: Normal operation
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
            .video_info_table =
            {
                .def_start_x    = 380,
                .def_start_y    = 60,
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
                {0x3002, 0x0F},  // BIT[3:0]MODE
                                 // Readout mode designation
                                 // 0: All pixel scan(2.3M)
                                 // 1: HD720p
                                 // 2: Window cropping
                                 // 3: 2x2 addition
                                 // 4: Vertical 1/2 sub sampling
                                 // F: 1080p
                                 // Others: Setting prohibited
                {0x0343, 0x28},  // LINE_LENGTH_L=IMX122_HMAX_LSB   1320 = 0x0528
                {0x0342, 0x05},  // LINE_LENGTH_H=IMX122_HMAX_MSB
                {0x0341, 0x65},  // FRM_LENGTH_L=IMX122_VMAX_LSB    1125 = 0x0465
                {0x0340, 0x04},  // FRM_LENGTH_H=IMX122_VMAX_MSB
                {0x3012, 0x82},  // BIT[1] ADRES(0:10bit 1:12bit)
                {0x0345, 0x00},  // X_ADD_STA_L=IMX122_WINPH_LSB
                {0x0344, 0x00},  // X_ADD_STA_H=IMX122_WINPH_MSB
                {0x0347, 0x3C},  // Y_ADD_STA_L=IMX122_WINPV_LSB
                {0x0346, 0x00},  // Y_ADD_STA_H=IMX122_WINPV_MSB
                {0x034D, 0xC0},  // X_OUT_SIZE_L=IMX122_WINWH_LSB   1984 = 0x07C0
                {0x034C, 0x07},  // X_OUT_SIZE_H=IMX122_WINWH_MSB
                {0x034F, 0x51},  // Y_OUT_SIZE_L=IMX122_WINWV_LSB   1105 = 0x0451
                {0x034E, 0x04},  // Y_OUT_SIZE_H=IMX122_WINWV_MSB
                {0x0008, 0x00},  // I2CBLKLEVEL_H=BLKLEVEL Black level offset value setting
                {0x0009, 0xF0},  // I2CBLKLEVEL_L=BLKLEVEL AD10bit=60d = 0x3C AD12bit=240d = 0xF0
                {0x3021, 0x00},  // BIT[7] 10BITA 0:12BIT 1:10BIT
                {0x3022, 0x00},  // BIT[7] 720PMODE 0:1080PMODE 1:720PMODE
                {0x307A, 0x00},  // 10BITB Setting register for 10bit
                {0x307B, 0x00},  // 10BITC Setting register for 10bit
                {0x3098, 0x26},  // 10B1080P_LSB
                {0x3099, 0x02},  // 10B1080P_MSB
                {0x309A, 0x94},  // 12B1080P_LSB
                {0x309B, 0x02},  // 12B1080P_MSB
                {0x30CE, 0x16},  // PRES
                {0x30CF, 0x82},  // DRES_LSB
                {0x30D0, 0x00},  // DRES_MSB
                {0x302C, 0x00},  // BIT[0]XMSTA
                                 // Trigger for master mode operation start
                                 // 0: Master mode operation start
                                 // 1: Trigger standby
                {0x0112, 0x0C},  // RAW_FMT_H(12BIT) Output bit selection 10bit£º0x0A 12bit£º0x0C
                {0x0113, 0x0C},  // RAW_FMT_L(12BIT) Output bit selection 10bit£º0x0A 12bit£º0x0C
                {0x0100, 0x01},  // STANDBY 0:STANDBY, 1: Normal operation
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
                .def_start_x    = 380,
                .def_start_y    = 60,
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
                {0x3002, 0x01},  // BIT[3:0]MODE
                                 // Readout mode designation
                                 // 0: All pixel scan(2.3M)
                                 // 1: HD720p
                                 // 2: Window cropping
                                 // 3: 2x2 addition
                                 // 4: Vertical 1/2 sub sampling
                                 // F: 1080p
                                 // Others: Setting prohibited
                {0x0343, 0x39},  // LINE_LENGTH_L=IMX122_HMAX_LSB    825 = 0x0339
                {0x0342, 0x03},  // LINE_LENGTH_H=IMX122_HMAX_MSB
                {0x0341, 0xEE},  // FRM_LENGTH_L=IMX122_VMAX_LSB     750 = 0x02EE
                {0x0340, 0x02},  // FRM_LENGTH_H=IMX122_VMAX_MSB
                {0x3012, 0x80},  // BIT[1] ADRES(0:10bit 1:12bit)
                {0x0345, 0x40},  // X_ADD_STA_L=IMX122_WINPH_LSB
                {0x0344, 0x01},  // X_ADD_STA_H=IMX122_WINPH_MSB
                {0x0347, 0xF0},  // Y_ADD_STA_L=IMX122_WINPV_LSB
                {0x0346, 0x00},  // Y_ADD_STA_H=IMX122_WINPV_MSB
                {0x034D, 0x40},  // X_OUT_SIZE_L=IMX122_WINWH_LSB   1344 = 0x0540
                {0x034C, 0x05},  // X_OUT_SIZE_H=IMX122_WINWH_MSB
                {0x034F, 0xE9},  // Y_OUT_SIZE_L=IMX122_WINWV_LSB    745 = 0x02E9
                {0x034E, 0x02},  // Y_OUT_SIZE_H=IMX122_WINWV_MSB
                {0x0008, 0x00},  // I2CBLKLEVEL_H=BLKLEVEL Black level offset value setting
                {0x0009, 0x3C},  // I2CBLKLEVEL_L=BLKLEVEL AD10bit=60d = 0x3C AD12bit=240d = 0xF0
                {0x3021, 0x00},  // BIT[7] 10BITA 0:12BIT 1:10BIT
                {0x3022, 0x80},  // BIT[7] 720PMODE 0:1080PMODE 1:720PMODE
                {0x307A, 0x00},  // 10BITB Setting register for 10bit
                {0x307B, 0x00},  // 10BITC Setting register for 10bit
                {0x3098, 0x26},  // 10B1080P_LSB
                {0x3099, 0x02},  // 10B1080P_MSB
                {0x309A, 0x4C},  // 12B1080P_LSB
                {0x309B, 0x04},  // 12B1080P_MSB
                {0x30CE, 0x00},  // PRES
                {0x30CF, 0x00},  // DRES_LSB
                {0x30D0, 0x00},  // DRES_MSB
                {0x302C, 0x00},  // BIT[0]XMSTA
                                 // Trigger for master mode operation start
                                 // 0: Master mode operation start
                                 // 1: Trigger standby
                {0x0112, 0x0A},  // RAW_FMT_H(12BIT) Output bit selection 10bit£º0x0A 12bit£º0x0C
                {0x0113, 0x0A},  // RAW_FMT_L(12BIT) Output bit selection 10bit£º0x0A 12bit£º0x0C
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
                .def_start_x    = (40 + 220 - 4 - 16 - 24 - 8) - 16,
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
                {0x3002, 0x01},  // BIT[3:0]MODE
                                 // Readout mode designation
                                 // 0: All pixel scan(2.3M)
                                 // 1: HD720p
                                 // 2: Window cropping
                                 // 3: 2x2 addition
                                 // 4: Vertical 1/2 sub sampling
                                 // F: 1080p
                                 // Others: Setting prohibited
                {0x0343, 0xDE},  // LINE_LENGTH_L=IMX122_HMAX_LSB    990 = 0x03DE
                {0x0342, 0x03},  // LINE_LENGTH_H=IMX122_HMAX_MSB
                {0x0341, 0xEE},  // FRM_LENGTH_L=IMX122_VMAX_LSB     750 = 0x02EE
                {0x0340, 0x02},  // FRM_LENGTH_H=IMX122_VMAX_MSB
                {0x3012, 0x80},  // BIT[1] ADRES(0:10bit 1:12bit)
                {0x0345, 0x40},  // X_ADD_STA_L=IMX122_WINPH_LSB
                {0x0344, 0x01},  // X_ADD_STA_H=IMX122_WINPH_MSB
                {0x0347, 0xF0},  // Y_ADD_STA_L=IMX122_WINPV_LSB
                {0x0346, 0x00},  // Y_ADD_STA_H=IMX122_WINPV_MSB
                {0x034D, 0x40},  // X_OUT_SIZE_L=IMX122_WINWH_LSB   1344 = 0x0540
                {0x034C, 0x05},  // X_OUT_SIZE_H=IMX122_WINWH_MSB
                {0x034F, 0xE9},  // Y_OUT_SIZE_L=IMX122_WINWV_LSB    745 = 0x02E9
                {0x034E, 0x02},  // Y_OUT_SIZE_H=IMX122_WINWV_MSB
                {0x0008, 0x00},  // I2CBLKLEVEL_H=BLKLEVEL Black level offset value setting
                {0x0009, 0x3C},  // I2CBLKLEVEL_L=BLKLEVEL AD10bit=60d = 0x3C AD12bit=240d = 0xF0
                {0x3021, 0x00},  // BIT[7] 10BITA 0:12BIT 1:10BIT
                {0x3022, 0x80},  // BIT[7] 720PMODE 0:1080PMODE 1:720PMODE
                {0x307A, 0x00},  // 10BITB Setting register for 10bit
                {0x307B, 0x00},  // 10BITC Setting register for 10bit
                {0x3098, 0x26},  // 10B1080P_LSB
                {0x3099, 0x02},  // 10B1080P_MSB
                {0x309A, 0x4C},  // 12B1080P_LSB
                {0x309B, 0x04},  // 12B1080P_MSB
                {0x30CE, 0x00},  // PRES
                {0x30CF, 0x00},  // DRES_LSB
                {0x30D0, 0x00},  // DRES_MSB
                {0x302C, 0x00},  // BIT[0]XMSTA
                                 // Trigger for master mode operation start
                                 // 0: Master mode operation start
                                 // 1: Trigger standby
                {0x0112, 0x0A},  // RAW_FMT_H(12BIT) Output bit selection 10bit£º0x0A 12bit£º0x0C
                {0x0113, 0x0A},  // RAW_FMT_L(12BIT) Output bit selection 10bit£º0x0A 12bit£º0x0C
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
                .def_start_x    = (40 + 220 - 4 - 16 - 24 - 8) - 16,
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
                {0x3002, 0x01},  // BIT[3:0]MODE
                                 // Readout mode designation
                                 // 0: All pixel scan(2.3M)
                                 // 1: HD720p
                                 // 2: Window cropping
                                 // 3: 2x2 addition
                                 // 4: Vertical 1/2 sub sampling
                                 // F: 1080p
                                 // Others: Setting prohibited
                {0x0343, 0x72},  // LINE_LENGTH_L=IMX122_HMAX_LSB   1650 = 0x0672
                {0x0342, 0x06},  // LINE_LENGTH_H=IMX122_HMAX_MSB
                {0x0341, 0xEE},  // FRM_LENGTH_L=IMX122_VMAX_LSB     750 = 0x02EE
                {0x0340, 0x02},  // FRM_LENGTH_H=IMX122_VMAX_MSB
                {0x3012, 0x82},  // BIT[1] ADRES(0:10bit 1:12bit)
                {0x0345, 0x40},  // X_ADD_STA_L=IMX122_WINPH_LSB
                {0x0344, 0x01},  // X_ADD_STA_H=IMX122_WINPH_MSB
                {0x0347, 0xF0},  // Y_ADD_STA_L=IMX122_WINPV_LSB
                {0x0346, 0x00},  // Y_ADD_STA_H=IMX122_WINPV_MSB
                {0x034D, 0x40},  // X_OUT_SIZE_L=IMX122_WINWH_LSB   1344 = 0x0540
                {0x034C, 0x05},  // X_OUT_SIZE_H=IMX122_WINWH_MSB
                {0x034F, 0xE9},  // Y_OUT_SIZE_L=IMX122_WINWV_LSB    745 = 0x02E9
                {0x034E, 0x02},  // Y_OUT_SIZE_H=IMX122_WINWV_MSB
                {0x0008, 0x00},  // I2CBLKLEVEL_H=BLKLEVEL Black level offset value setting
                {0x0009, 0xF0},  // I2CBLKLEVEL_L=BLKLEVEL AD10bit=60d = 0x3C AD12bit=240d = 0xF0
                {0x3021, 0x00},  // BIT[7] 10BITA 0:12BIT 1:10BIT
                {0x3022, 0x80},  // BIT[7] 720PMODE 0:1080PMODE 1:720PMODE
                {0x307A, 0x00},  // 10BITB Setting register for 10bit
                {0x307B, 0x00},  // 10BITC Setting register for 10bit
                {0x3098, 0x26},  // 10B1080P_LSB
                {0x3099, 0x02},  // 10B1080P_MSB
                {0x309A, 0x4C},  // 12B1080P_LSB
                {0x309B, 0x04},  // 12B1080P_MSB
                {0x30CE, 0x40},  // PRES
                {0x30CF, 0x81},  // DRES_LSB
                {0x30D0, 0x01},  // DRES_MSB
                {0x302C, 0x00},  // BIT[0]XMSTA
                                 // Trigger for master mode operation start
                                 // 0: Master mode operation start
                                 // 1: Trigger standby
                {0x0112, 0x0C},  // RAW_FMT_H(12BIT) Output bit selection 10bit£º0x0A 12bit£º0x0C
                {0x0113, 0x0C},  // RAW_FMT_L(12BIT) Output bit selection 10bit£º0x0A 12bit£º0x0C
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
                .def_start_x    = (40 + 220 - 4 - 16 - 24 - 8) - 16,
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
                {0x3002, 0x01},  // BIT[3:0]MODE
                                 // Readout mode designation
                                 // 0: All pixel scan(2.3M)
                                 // 1: HD720p
                                 // 2: Window cropping
                                 // 3: 2x2 addition
                                 // 4: Vertical 1/2 sub sampling
                                 // F: 1080p
                                 // Others: Setting prohibited
                {0x0343, 0xBC},  // LINE_LENGTH_L=IMX122_HMAX_LSB   1980 = 0x07BC
                {0x0342, 0x07},  // LINE_LENGTH_H=IMX122_HMAX_MSB
                {0x0341, 0xEE},  // FRM_LENGTH_L=IMX122_VMAX_LSB     750 = 0x02EE
                {0x0340, 0x02},  // FRM_LENGTH_H=IMX122_VMAX_MSB
                {0x3012, 0x82},  // BIT[1] ADRES(0:10bit 1:12bit)
                {0x0345, 0x40},  // X_ADD_STA_L=IMX122_WINPH_LSB
                {0x0344, 0x01},  // X_ADD_STA_H=IMX122_WINPH_MSB
                {0x0347, 0xF0},  // Y_ADD_STA_L=IMX122_WINPV_LSB
                {0x0346, 0x00},  // Y_ADD_STA_H=IMX122_WINPV_MSB
                {0x034D, 0x40},  // X_OUT_SIZE_L=IMX122_WINWH_LSB   1344 = 0x0540
                {0x034C, 0x05},  // X_OUT_SIZE_H=IMX122_WINWH_MSB
                {0x034F, 0xE9},  // Y_OUT_SIZE_L=IMX122_WINWV_LSB    745 = 0x02E9
                {0x034E, 0x02},  // Y_OUT_SIZE_H=IMX122_WINWV_MSB
                {0x0008, 0x00},  // I2CBLKLEVEL_H=BLKLEVEL Black level offset value setting
                {0x0009, 0xF0},  // I2CBLKLEVEL_L=BLKLEVEL AD10bit=60d = 0x3C AD12bit=240d = 0xF0
                {0x3021, 0x00},  // BIT[7] 10BITA 0:12BIT 1:10BIT
                {0x3022, 0x80},  // BIT[7] 720PMODE 0:1080PMODE 1:720PMODE
                {0x307A, 0x00},  // 10BITB Setting register for 10bit
                {0x307B, 0x00},  // 10BITC Setting register for 10bit
                {0x3098, 0x26},  // 10B1080P_LSB
                {0x3099, 0x02},  // 10B1080P_MSB
                {0x309A, 0x4C},  // 12B1080P_LSB
                {0x309B, 0x04},  // 12B1080P_MSB
                {0x30CE, 0x40},  // PRES
                {0x30CF, 0x81},  // DRES_LSB
                {0x30D0, 0x01},  // DRES_MSB
                {0x302C, 0x00},  // BIT[0]XMSTA
                                 // Trigger for master mode operation start
                                 // 0: Master mode operation start
                                 // 1: Trigger standby
                {0x0112, 0x0C},  // RAW_FMT_H(12BIT) Output bit selection 10bit£º0x0A 12bit£º0x0C
                {0x0113, 0x0C},  // RAW_FMT_L(12BIT) Output bit selection 10bit£º0x0A 12bit£º0x0C
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
                .max_fps    = GADI_VIDEO_FPS(25),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(25),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_720P_25,
            },
            .video_info_table =
            {
                .def_start_x    = (40 + 220 - 4 - 16 - 24 - 8) - 16,
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
#define IMX122_H_MIRROR     (1<<0)
#define IMX122_V_FLIP       (1<<1)
    .mirror_table =
    {
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY_VERTICALLY] =
        {
            {0x0101, IMX122_H_MIRROR | IMX122_V_FLIP, IMX122_H_MIRROR | IMX122_V_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY] =
        {
            {0x0101, IMX122_H_MIRROR, IMX122_H_MIRROR | IMX122_V_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_VERTICALLY] =
        {
            {0x0101, IMX122_V_FLIP, IMX122_H_MIRROR | IMX122_V_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_NONE] =
        {
            {0x0101, 0x00, IMX122_H_MIRROR | IMX122_V_FLIP},
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
    // hmax = (IMX122_HMAX_MSB & 0xFF) << 8 +
    //        (IMX122_HMAX_LSB & 0xFF) << 0
    .hmax_reg =
    {
        {0x0342, 0x00, 0xFF, 0x00, 0x08, 0}, // IMX122_HMAX_MSB
        {0x0343, 0x00, 0xFF, 0x00, 0x00, 0}, // IMX122_HMAX_LSB
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    // vmax = (IMX122_VMAX_MSB & 0xFF) << 8 +
    //        (IMX122_VMAX_LSB & 0xFF) << 0
    .vmax_reg =
    {
        {0x0340, 0x00, 0xFF, 0x00, 0x08, 0}, // IMX122_VMAX_MSB
        {0x0341, 0x00, 0xFF, 0x00, 0x00, 0}, // IMX122_VMAX_LSB
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
        {0x301E, 0x00},
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
GADI_VI_SensorDrvInfoT    imx122_sensor =
{
    .magic_start        = GADI_SENSOR_HW_INFO_MAGIC_START,
    .version            = GADI_SENSOR_HW_INFO_VERSION,
    .length             = sizeof(GADI_VI_SensorDrvInfoT),
    .file_date          = 0x20161227,
    .HwInfo         =
    {
        .name               = "imx122",
        .ctl_type           = GADI_SENSOR_CTL_SPI,
        .addr_len           = sizeof(GADI_U16),
        .data_len           = sizeof(GADI_U8),
        .reset_active_level = GADI_GPIO_LOW,
        .hw_addr            = (0x34>>1),
        .id_reg =
        {
            //{0x0000, 0x01},
            //{0x0001, 0x22},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        .reset_reg =
        {
            {0x0200, 0x31}, // BIT[0]STANDBY
                            // 0: Normal operation, 1:STANDBY
                            // BIT[5:4]TESTEN
                            // Register write 0: Invalid, 3:Valid, Others: Setting prohibited
            {GADI_VI_SENSOR_TABLE_FLAG_MSLEEP, 0x0A},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
    },
    .debug_print        = 0,
    .input_format       = GADI_VI_INPUT_FORMAT_RGB_RAW,
    .field_format       = 1,
    .adapter_id         = 0,
    .sensor_id          = GADI_ISP_SENSOR_IMX122,
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
    .max_width          = 2080,
    .max_height         = 1553,
    .def_sh_time        = GADI_VIDEO_FPS(50),
    .fmt_num            = 8,
    .auto_fmt           = GADI_VIDEO_MODE(1920, 1080,  25, 1),
    .init_reg           =
    {
        {0x022D, 0x48},// BIT[3]: 10-bit output 2-bit shift(1:right justified)
        {0x0227, 0x20},
        {0x023b, 0xE0}, // E0h: CMOS output
                        // 90h: LVDS output, synccode1
                        // D0h: LVDS output, synccode2
        /*------Section 3, can't be read----------*/
        {0x0300, 0x01},
        {0x0317, 0x0D},
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .pll_table =
    {
        [0] =
        {
            // for 1080P@30, 1080P@25, 720P@60, 720P@50
            .pixclk = 74250000,
            .extclk = 37125000,
            // pixclk = extclk * FRSEL = extclk * 2
            .regs =
            {
                {0x0211, 0x00}, // BIT[2:0] FRSEL
                                // Output data rate designation
                                // 0: 2 time INCK
                                // 1: Equal to INCK
                                // 2: 1/2 of INCK (2x2 addition mode only)
                                // 3: 1/4 of INCK (2x2 addition mode only)
                                // Others: Setting prohibited
                                // BIT[4:3] OPORTSEL/M12BEN
                                // Output system selection
                                // 0: Parallel CMOS SDR output
                                // 1: Parallel CMOS DDR output
                                // 2: Serial LVDS 1ch output
                                // 3: Serial LVDS 2ch output
                                // BIT[6:5] M12BEN
                                // The output resolution is set to 10 bit (2x2 binning mode only.)
                                // 0: disable 2:enable
                                // Others: Setting prohibited
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
                {0x0211, 0x00}, // BIT[2:0] FRSEL
                                // Output data rate designation
                                // 0: 2 time INCK
                                // 1: Equal to INCK
                                // 2: 1/2 of INCK (2x2 addition mode only)
                                // 3: 1/4 of INCK (2x2 addition mode only)
                                // Others: Setting prohibited
                                // BIT[4:3] OPORTSEL/M12BEN
                                // Output system selection
                                // 0: Parallel CMOS SDR output
                                // 1: Parallel CMOS DDR output
                                // 2: Serial LVDS 1ch output
                                // 3: Serial LVDS 2ch output
                                // BIT[6:5] M12BEN
                                // The output resolution is set to 10 bit (2x2 binning mode only.)
                                // 0: disable 2:enable
                                // Others: Setting prohibited
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
                {0x0211, 0x01}, // BIT[2:0] FRSEL
                                // Output data rate designation
                                // 0: 2 time INCK
                                // 1: Equal to INCK
                                // 2: 1/2 of INCK (2x2 addition mode only)
                                // 3: 1/4 of INCK (2x2 addition mode only)
                                // Others: Setting prohibited
                                // BIT[4:3] OPORTSEL/M12BEN
                                // Output system selection
                                // 0: Parallel CMOS SDR output
                                // 1: Parallel CMOS DDR output
                                // 2: Serial LVDS 1ch output
                                // 3: Serial LVDS 2ch output
                                // BIT[6:5] M12BEN
                                // The output resolution is set to 10 bit (2x2 binning mode only.)
                                // 0: disable 2:enable
                                // Others: Setting prohibited
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
    },
    .video_fmt_table =
    {
        // 1080P IMX122_HMAX = SD(44 + 148 - 4 - 16 - 24 - 8 = 140) + Sync(4) + OB(16) + Ignored(24) + Effective(8) + x + Effective(8) + Ignored(24) + Sync(4) + HB(52)
        // 1080P IMX122_VMAX = Dummy(6) + FI(1) + OB(4) + VDE OB(8) + Ignored(4) + Effective(8) + y + Effective(9) + Ignored(4) + VB(1)
        // 720P  IMX122_HMAX = SD(40 + 220 - 4 - 16 - 24 - 8 = 208) + Sync(4) + OB(16) + Ignored(24) + Effective(8) + x + Effective(8) + Ignored(24) + Sync(4) + HB
        // 720P  IMX122_VMAX = Dummy(6) + FI(1) + OB(4) + VDE OB(6) + Ignored(2) + Effective(4) + y + Effective(5) + Ignored(2)
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
                {0x0202, 0x0F},  // BIT[3:0]MODE
                                 // Readout mode designation
                                 // 0: All pixel scan(2.3M)
                                 // 1: HD720p
                                 // 2: Window cropping
                                 // 3: 2x2 addition
                                 // 4: Vertical 1/2 sub sampling
                                 // F: 1080p
                                 // Others: Setting prohibited
                {0x0203, 0x4C},  // LINE_LENGTH_L=IMX122_HMAX_LSB   1100 = 0x044C
                {0x0204, 0x04},  // LINE_LENGTH_H=IMX122_HMAX_MSB
                {0x0205, 0x65},  // FRM_LENGTH_L=IMX122_VMAX_LSB    1125 = 0x0465
                {0x0206, 0x04},  // FRM_LENGTH_H=IMX122_VMAX_MSB
                {0x0212, 0x82},  // BIT[1] ADRES(0:10bit 1:12bit)
                {0x0214, 0x00},  // X_ADD_STA_L=IMX122_WINPH_LSB
                {0x0215, 0x00},  // X_ADD_STA_H=IMX122_WINPH_MSB
                {0x0216, 0x3C},  // Y_ADD_STA_L=IMX122_WINPV_LSB
                {0x0217, 0x00},  // Y_ADD_STA_H=IMX122_WINPV_MSB
                {0x0218, 0xC0},  // X_OUT_SIZE_L=IMX122_WINWH_LSB   1984 = 0x07C0
                {0x0219, 0x07},  // X_OUT_SIZE_H=IMX122_WINWH_MSB
                {0x021A, 0x51},  // Y_OUT_SIZE_L=IMX122_WINWV_LSB   1105 = 0x0451
                {0x021B, 0x04},  // Y_OUT_SIZE_H=IMX122_WINWV_MSB
                {0x0220, 0xF0},  // I2CBLKLEVEL_L=BLKLEVEL AD10bit=60d = 0x3C AD12bit=240d = 0xF0
                {0x0221, 0x00},  // BIT[7] 10BITA 0:12BIT 1:10BIT
                {0x0222, 0x00},  // BIT[7] 720PMODE 0:1080PMODE 1:720PMODE
                {0x027A, 0x00},  // 10BITB Setting register for 10bit
                {0x027B, 0x00},  // 10BITC Setting register for 10bit
                {0x0298, 0x26},  // 10B1080P_LSB
                {0x0299, 0x02},  // 10B1080P_MSB
                {0x029A, 0x26},  // 12B1080P_LSB
                {0x029B, 0x02},  // 12B1080P_MSB
                {0x02CE, 0x16},  // PRES
                {0x02CF, 0x82},  // DRES_LSB
                {0x02D0, 0x00},  // DRES_MSB
                {0x022C, 0x00},  // BIT[0]XMSTA
                                 // Trigger for master mode operation start
                                 // 0: Master mode operation start
                                 // 1: Trigger standby
                {0x0112, 0x0C},  // RAW_FMT_H Output bit selection 10bit£º0x0A 12bit£º0x0C
                {0x0113, 0x0C},  // RAW_FMT_L Output bit selection 10bit£º0x0A 12bit£º0x0C
                {0x0200, 0x00, 0x01},  // STANDBY 0:Normal operation, 1:STANDBY
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
                {0x0202, 0x0F},  // BIT[3:0]MODE
                                 // Readout mode designation
                                 // 0: All pixel scan(2.3M)
                                 // 1: HD720p
                                 // 2: Window cropping
                                 // 3: 2x2 addition
                                 // 4: Vertical 1/2 sub sampling
                                 // F: 1080p
                                 // Others: Setting prohibited
                {0x0203, 0x28},  // LINE_LENGTH_L=IMX122_HMAX_LSB   1320 = 0x0528
                {0x0204, 0x05},  // LINE_LENGTH_H=IMX122_HMAX_MSB
                {0x0205, 0x65},  // FRM_LENGTH_L=IMX122_VMAX_LSB    1125 = 0x0465
                {0x0206, 0x04},  // FRM_LENGTH_H=IMX122_VMAX_MSB
                {0x0212, 0x82},  // BIT[1] ADRES(0:10bit 1:12bit)
                {0x0214, 0x00},  // X_ADD_STA_L=IMX122_WINPH_LSB
                {0x0215, 0x00},  // X_ADD_STA_H=IMX122_WINPH_MSB
                {0x0216, 0x3C},  // Y_ADD_STA_L=IMX122_WINPV_LSB
                {0x0217, 0x00},  // Y_ADD_STA_H=IMX122_WINPV_MSB
                {0x0218, 0xC0},  // X_OUT_SIZE_L=IMX122_WINWH_LSB   1984 = 0x07C0
                {0x0219, 0x07},  // X_OUT_SIZE_H=IMX122_WINWH_MSB
                {0x021A, 0x51},  // Y_OUT_SIZE_L=IMX122_WINWV_LSB   1105 = 0x0451
                {0x021B, 0x04},  // Y_OUT_SIZE_H=IMX122_WINWV_MSB
                {0x0220, 0xF0},  // I2CBLKLEVEL_L=BLKLEVEL AD10bit=60d = 0x3C AD12bit=240d = 0xF0
                {0x0221, 0x00},  // BIT[7] 10BITA 0:12BIT 1:10BIT
                {0x0222, 0x00},  // BIT[7] 720PMODE 0:1080PMODE 1:720PMODE
                {0x027A, 0x00},  // 10BITB Setting register for 10bit
                {0x027B, 0x00},  // 10BITC Setting register for 10bit
                {0x0298, 0x26},  // 10B1080P_LSB
                {0x0299, 0x02},  // 10B1080P_MSB
                {0x029A, 0x94},  // 12B1080P_LSB
                {0x029B, 0x02},  // 12B1080P_MSB
                {0x02CE, 0x16},  // PRES
                {0x02CF, 0x82},  // DRES_LSB
                {0x02D0, 0x00},  // DRES_MSB
                {0x022C, 0x00},  // BIT[0]XMSTA
                                 // Trigger for master mode operation start
                                 // 0: Master mode operation start
                                 // 1: Trigger standby
                {0x0112, 0x0C},  // RAW_FMT_H(12BIT) Output bit selection 10bit£º0x0A 12bit£º0x0C
                {0x0113, 0x0C},  // RAW_FMT_L(12BIT) Output bit selection 10bit£º0x0A 12bit£º0x0C
                {0x0200, 0x00, 0x01},  // STANDBY 0:Normal operation, 1:STANDBY
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
                {0x0202, 0x0F},  // BIT[3:0]MODE
                                 // Readout mode designation
                                 // 0: All pixel scan(2.3M)
                                 // 1: HD720p
                                 // 2: Window cropping
                                 // 3: 2x2 addition
                                 // 4: Vertical 1/2 sub sampling
                                 // F: 1080p
                                 // Others: Setting prohibited
                {0x0203, 0x84},  // LINE_LENGTH_L=IMX122_HMAX_LSB    900 = 0x0384
                {0x0204, 0x03},  // LINE_LENGTH_H=IMX122_HMAX_MSB
                {0x0205, 0xE8},  // FRM_LENGTH_L=IMX122_VMAX_LSB    1000 = 0x03E8
                {0x0206, 0x03},  // FRM_LENGTH_H=IMX122_VMAX_MSB
                {0x0212, 0x82},  // BIT[1] ADRES(0:10bit 1:12bit)
                {0x0214, 0x00},  // X_ADD_STA_L=IMX122_WINPH_LSB
                {0x0215, 0x00},  // X_ADD_STA_H=IMX122_WINPH_MSB
                {0x0216, 0x3C},  // Y_ADD_STA_L=IMX122_WINPV_LSB
                {0x0217, 0x00},  // Y_ADD_STA_H=IMX122_WINPV_MSB
                {0x0218, 0x40},  // X_OUT_SIZE_L=IMX122_WINWH_LSB   1344 = 0x0540
                {0x0219, 0x05},  // X_OUT_SIZE_H=IMX122_WINWH_MSB
                {0x021A, 0xD8},  // Y_OUT_SIZE_L=IMX122_WINWV_LSB    984 = 0x03D8
                {0x021B, 0x03},  // Y_OUT_SIZE_H=IMX122_WINWV_MSB
                {0x0220, 0xF0},  // I2CBLKLEVEL_L=BLKLEVEL AD10bit=60d = 0x3C AD12bit=240d = 0xF0
                {0x0221, 0x00},  // BIT[7] 10BITA 0:12BIT 1:10BIT
                {0x0222, 0x00},  // BIT[7] 720PMODE 0:1080PMODE 1:720PMODE
                {0x027A, 0x00},  // 10BITB Setting register for 10bit
                {0x027B, 0x00},  // 10BITC Setting register for 10bit
                {0x0298, 0x26},  // 10B1080P_LSB
                {0x0299, 0x02},  // 10B1080P_MSB
                {0x029A, 0x26},  // 12B1080P_LSB
                {0x029B, 0x02},  // 12B1080P_MSB
                {0x02CE, 0x16},  // PRES
                {0x02CF, 0x82},  // DRES_LSB
                {0x02D0, 0x00},  // DRES_MSB
                {0x022C, 0x00},  // BIT[0]XMSTA
                                 // Trigger for master mode operation start
                                 // 0: Master mode operation start
                                 // 1: Trigger standby
                {0x0200, 0x00, 0x01},  // STANDBY 0:Normal operation, 1:STANDBY
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
                {0x0202, 0x0F},  // BIT[3:0]MODE
                                 // Readout mode designation
                                 // 0: All pixel scan(2.3M)
                                 // 1: HD720p
                                 // 2: Window cropping
                                 // 3: 2x2 addition
                                 // 4: Vertical 1/2 sub sampling
                                 // F: 1080p
                                 // Others: Setting prohibited
                {0x0203, 0x38},  // LINE_LENGTH_L=IMX122_HMAX_LSB   1080 = 0x0438
                {0x0204, 0x04},  // LINE_LENGTH_H=IMX122_HMAX_MSB
                {0x0205, 0xE8},  // FRM_LENGTH_L=IMX122_VMAX_LSB    1000 = 0x03E8
                {0x0206, 0x03},  // FRM_LENGTH_H=IMX122_VMAX_MSB
                {0x0212, 0x82},  // BIT[1] ADRES(0:10bit 1:12bit)
                {0x0214, 0x00},  // X_ADD_STA_L=IMX122_WINPH_LSB
                {0x0215, 0x00},  // X_ADD_STA_H=IMX122_WINPH_MSB
                {0x0216, 0x3C},  // Y_ADD_STA_L=IMX122_WINPV_LSB
                {0x0217, 0x00},  // Y_ADD_STA_H=IMX122_WINPV_MSB
                {0x0218, 0x40},  // X_OUT_SIZE_L=IMX122_WINWH_LSB   1344 = 0x0540
                {0x0219, 0x05},  // X_OUT_SIZE_H=IMX122_WINWH_MSB
                {0x021A, 0xD8},  // Y_OUT_SIZE_L=IMX122_WINWV_LSB    984 = 0x03D8
                {0x021B, 0x03},  // Y_OUT_SIZE_H=IMX122_WINWV_MSB
                {0x0220, 0xF0},  // I2CBLKLEVEL_L=BLKLEVEL AD10bit=60d = 0x3C AD12bit=240d = 0xF0
                {0x0221, 0x00},  // BIT[7] 10BITA 0:12BIT 1:10BIT
                {0x0222, 0x00},  // BIT[7] 720PMODE 0:1080PMODE 1:720PMODE
                {0x027A, 0x00},  // 10BITB Setting register for 10bit
                {0x027B, 0x00},  // 10BITC Setting register for 10bit
                {0x0298, 0x26},  // 10B1080P_LSB
                {0x0299, 0x02},  // 10B1080P_MSB
                {0x029A, 0x26},  // 12B1080P_LSB
                {0x029B, 0x02},  // 12B1080P_MSB
                {0x02CE, 0x16},  // PRES
                {0x02CF, 0x82},  // DRES_LSB
                {0x02D0, 0x00},  // DRES_MSB
                {0x022C, 0x00},  // BIT[0]XMSTA
                                 // Trigger for master mode operation start
                                 // 0: Master mode operation start
                                 // 1: Trigger standby
                {0x0200, 0x00, 0x01},  // STANDBY 0:Normal operation, 1:STANDBY
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
                {0x0202, 0x01},  // BIT[3:0]MODE
                                 // Readout mode designation
                                 // 0: All pixel scan(2.3M)
                                 // 1: HD720p
                                 // 2: Window cropping
                                 // 3: 2x2 addition
                                 // 4: Vertical 1/2 sub sampling
                                 // F: 1080p
                                 // Others: Setting prohibited
                {0x0203, 0x39},  // LINE_LENGTH_L=IMX122_HMAX_LSB    825 = 0x0339
                {0x0204, 0x03},  // LINE_LENGTH_H=IMX122_HMAX_MSB
                {0x0205, 0xEE},  // FRM_LENGTH_L=IMX122_VMAX_LSB     750 = 0x02EE
                {0x0206, 0x02},  // FRM_LENGTH_H=IMX122_VMAX_MSB
                {0x0212, 0x80},  // BIT[1] ADRES(0:10bit 1:12bit)
                {0x0214, 0x40},  // X_ADD_STA_L=IMX122_WINPH_LSB
                {0x0215, 0x01},  // X_ADD_STA_H=IMX122_WINPH_MSB
                {0x0216, 0xF0},  // Y_ADD_STA_L=IMX122_WINPV_LSB
                {0x0217, 0x00},  // Y_ADD_STA_H=IMX122_WINPV_MSB
                {0x0218, 0x40},  // X_OUT_SIZE_L=IMX122_WINWH_LSB   1344 = 0x0540
                {0x0219, 0x05},  // X_OUT_SIZE_H=IMX122_WINWH_MSB
                {0x021A, 0xE9},  // Y_OUT_SIZE_L=IMX122_WINWV_LSB    745 = 0x02E9
                {0x021B, 0x02},  // Y_OUT_SIZE_H=IMX122_WINWV_MSB
                {0x0220, 0x3C},  // I2CBLKLEVEL_L=BLKLEVEL AD10bit=60d = 0x3C AD12bit=240d = 0xF0
                {0x0221, 0x00},  // BIT[7] 10BITA 0:12BIT 1:10BIT
                {0x0222, 0x80},  // BIT[7] 720PMODE 0:1080PMODE 1:720PMODE
                {0x027A, 0x00},  // 10BITB Setting register for 10bit
                {0x027B, 0x00},  // 10BITC Setting register for 10bit
                {0x0298, 0x26},  // 10B1080P_LSB
                {0x0299, 0x02},  // 10B1080P_MSB
                {0x029A, 0x4C},  // 12B1080P_LSB
                {0x029B, 0x04},  // 12B1080P_MSB
                {0x02CE, 0x00},  // PRES
                {0x02CF, 0x00},  // DRES_LSB
                {0x02D0, 0x00},  // DRES_MSB
                {0x022C, 0x00},  // BIT[0]XMSTA
                                 // Trigger for master mode operation start
                                 // 0: Master mode operation start
                                 // 1: Trigger standby
                {0x0200, 0x00, 0x01},  // STANDBY 0:Normal operation, 1:STANDBY
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
                .def_start_x    = (40 + 220 - 4 - 16 - 24 - 8) - 16,
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
                {0x0202, 0x01},  // BIT[3:0]MODE
                                 // Readout mode designation
                                 // 0: All pixel scan(2.3M)
                                 // 1: HD720p
                                 // 2: Window cropping
                                 // 3: 2x2 addition
                                 // 4: Vertical 1/2 sub sampling
                                 // F: 1080p
                                 // Others: Setting prohibited
                {0x0203, 0xDE},  // LINE_LENGTH_L=IMX122_HMAX_LSB    990 = 0x03DE
                {0x0204, 0x03},  // LINE_LENGTH_H=IMX122_HMAX_MSB
                {0x0205, 0xEE},  // FRM_LENGTH_L=IMX122_VMAX_LSB     750 = 0x02EE
                {0x0206, 0x02},  // FRM_LENGTH_H=IMX122_VMAX_MSB
                {0x0212, 0x80},  // BIT[1] ADRES(0:10bit 1:12bit)
                {0x0214, 0x40},  // X_ADD_STA_L=IMX122_WINPH_LSB
                {0x0215, 0x01},  // X_ADD_STA_H=IMX122_WINPH_MSB
                {0x0216, 0xF0},  // Y_ADD_STA_L=IMX122_WINPV_LSB
                {0x0217, 0x00},  // Y_ADD_STA_H=IMX122_WINPV_MSB
                {0x0218, 0x40},  // X_OUT_SIZE_L=IMX122_WINWH_LSB   1344 = 0x0540
                {0x0219, 0x05},  // X_OUT_SIZE_H=IMX122_WINWH_MSB
                {0x021A, 0xE9},  // Y_OUT_SIZE_L=IMX122_WINWV_LSB    745 = 0x02E9
                {0x021B, 0x02},  // Y_OUT_SIZE_H=IMX122_WINWV_MSB
                {0x0220, 0x3C},  // I2CBLKLEVEL_L=BLKLEVEL AD10bit=60d = 0x3C AD12bit=240d = 0xF0
                {0x0221, 0x00},  // BIT[7] 10BITA 0:12BIT 1:10BIT
                {0x0222, 0x80},  // BIT[7] 720PMODE 0:1080PMODE 1:720PMODE
                {0x027A, 0x00},  // 10BITB Setting register for 10bit
                {0x027B, 0x00},  // 10BITC Setting register for 10bit
                {0x0298, 0x26},  // 10B1080P_LSB
                {0x0299, 0x02},  // 10B1080P_MSB
                {0x029A, 0x4C},  // 12B1080P_LSB
                {0x029B, 0x04},  // 12B1080P_MSB
                {0x02CE, 0x00},  // PRES
                {0x02CF, 0x00},  // DRES_LSB
                {0x02D0, 0x00},  // DRES_MSB
                {0x022C, 0x00},  // BIT[0]XMSTA
                                 // Trigger for master mode operation start
                                 // 0: Master mode operation start
                                 // 1: Trigger standby
                {0x0200, 0x00, 0x01},  // STANDBY 0:Normal operation, 1:STANDBY
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
                .def_start_x    = (40 + 220 - 4 - 16 - 24 - 8) - 16,
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
                {0x0202, 0x01},  // BIT[3:0]MODE
                                 // Readout mode designation
                                 // 0: All pixel scan(2.3M)
                                 // 1: HD720p
                                 // 2: Window cropping
                                 // 3: 2x2 addition
                                 // 4: Vertical 1/2 sub sampling
                                 // F: 1080p
                                 // Others: Setting prohibited
                {0x0203, 0x72},  // LINE_LENGTH_L=IMX122_HMAX_LSB   1650 = 0x0672
                {0x0204, 0x06},  // LINE_LENGTH_H=IMX122_HMAX_MSB
                {0x0205, 0xEE},  // FRM_LENGTH_L=IMX122_VMAX_LSB     750 = 0x02EE
                {0x0206, 0x02},  // FRM_LENGTH_H=IMX122_VMAX_MSB
                {0x0212, 0x82},  // BIT[1] ADRES(0:10bit 1:12bit)
                {0x0214, 0x40},  // X_ADD_STA_L=IMX122_WINPH_LSB
                {0x0215, 0x01},  // X_ADD_STA_H=IMX122_WINPH_MSB
                {0x0216, 0xF0},  // Y_ADD_STA_L=IMX122_WINPV_LSB
                {0x0217, 0x00},  // Y_ADD_STA_H=IMX122_WINPV_MSB
                {0x0218, 0x40},  // X_OUT_SIZE_L=IMX122_WINWH_LSB   1344 = 0x0540
                {0x0219, 0x05},  // X_OUT_SIZE_H=IMX122_WINWH_MSB
                {0x021A, 0xE9},  // Y_OUT_SIZE_L=IMX122_WINWV_LSB    745 = 0x02E9
                {0x021B, 0x02},  // Y_OUT_SIZE_H=IMX122_WINWV_MSB
                {0x0220, 0xF0},  // I2CBLKLEVEL_L=BLKLEVEL AD10bit=60d = 0x3C AD12bit=240d = 0xF0
                {0x0221, 0x00},  // BIT[7] 10BITA 0:12BIT 1:10BIT
                {0x0222, 0x80},  // BIT[7] 720PMODE 0:1080PMODE 1:720PMODE
                {0x027A, 0x00},  // 10BITB Setting register for 10bit
                {0x027B, 0x00},  // 10BITC Setting register for 10bit
                {0x0298, 0x26},  // 10B1080P_LSB
                {0x0299, 0x02},  // 10B1080P_MSB
                {0x029A, 0x4C},  // 12B1080P_LSB
                {0x029B, 0x04},  // 12B1080P_MSB
                {0x02CE, 0x40},  // PRES
                {0x02CF, 0x81},  // DRES_LSB
                {0x02D0, 0x01},  // DRES_MSB
                {0x022C, 0x00},  // BIT[0]XMSTA
                                 // Trigger for master mode operation start
                                 // 0: Master mode operation start
                                 // 1: Trigger standby
                {0x0200, 0x00, 0x01},  // STANDBY 0:Normal operation, 1:STANDBY
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
                .def_start_x    = (40 + 220 - 4 - 16 - 24 - 8) - 16,
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
                {0x0202, 0x01},  // BIT[3:0]MODE
                                 // Readout mode designation
                                 // 0: All pixel scan(2.3M)
                                 // 1: HD720p
                                 // 2: Window cropping
                                 // 3: 2x2 addition
                                 // 4: Vertical 1/2 sub sampling
                                 // F: 1080p
                                 // Others: Setting prohibited
                {0x0203, 0xBC},  // LINE_LENGTH_L=IMX122_HMAX_LSB   1980 = 0x07BC
                {0x0204, 0x07},  // LINE_LENGTH_H=IMX122_HMAX_MSB
                {0x0205, 0xEE},  // FRM_LENGTH_L=IMX122_VMAX_LSB     750 = 0x02EE
                {0x0206, 0x02},  // FRM_LENGTH_H=IMX122_VMAX_MSB
                {0x0212, 0x82},  // BIT[1] ADRES(0:10bit 1:12bit)
                {0x0214, 0x40},  // X_ADD_STA_L=IMX122_WINPH_LSB
                {0x0215, 0x01},  // X_ADD_STA_H=IMX122_WINPH_MSB
                {0x0216, 0xF0},  // Y_ADD_STA_L=IMX122_WINPV_LSB
                {0x0217, 0x00},  // Y_ADD_STA_H=IMX122_WINPV_MSB
                {0x0218, 0x40},  // X_OUT_SIZE_L=IMX122_WINWH_LSB   1344 = 0x0540
                {0x0219, 0x05},  // X_OUT_SIZE_H=IMX122_WINWH_MSB
                {0x021A, 0xE9},  // Y_OUT_SIZE_L=IMX122_WINWV_LSB    745 = 0x02E9
                {0x021B, 0x02},  // Y_OUT_SIZE_H=IMX122_WINWV_MSB
                {0x0220, 0xF0},  // I2CBLKLEVEL_L=BLKLEVEL AD10bit=60d = 0x3C AD12bit=240d = 0xF0
                {0x0221, 0x00},  // BIT[7] 10BITA 0:12BIT 1:10BIT
                {0x0222, 0x80},  // BIT[7] 720PMODE 0:1080PMODE 1:720PMODE
                {0x027A, 0x00},  // 10BITB Setting register for 10bit
                {0x027B, 0x00},  // 10BITC Setting register for 10bit
                {0x0298, 0x26},  // 10B1080P_LSB
                {0x0299, 0x02},  // 10B1080P_MSB
                {0x029A, 0x4C},  // 12B1080P_LSB
                {0x029B, 0x04},  // 12B1080P_MSB
                {0x02CE, 0x40},  // PRES
                {0x02CF, 0x81},  // DRES_LSB
                {0x02D0, 0x01},  // DRES_MSB
                {0x022C, 0x00},  // BIT[0]XMSTA
                                 // Trigger for master mode operation start
                                 // 0: Master mode operation start
                                 // 1: Trigger standby
                {0x0200, 0x00, 0x01},  // STANDBY 0:Normal operation, 1:STANDBY
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
                .def_start_x    = (40 + 220 - 4 - 16 - 24 - 8) - 16,
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
            .update_after_vsync_start   = 37,
            .update_after_vsync_end     = 40,
        },
    },
#define IMX122_H_MIRROR     (1<<0)
#define IMX122_V_FLIP       (1<<1)
    .mirror_table =
    {
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY_VERTICALLY] =
        {
            {0x0201, IMX122_H_MIRROR | IMX122_V_FLIP, IMX122_H_MIRROR | IMX122_V_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY] =
        {
            {0x0201, IMX122_H_MIRROR, IMX122_H_MIRROR | IMX122_V_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_VERTICALLY] =
        {
            {0x0201, IMX122_V_FLIP, IMX122_H_MIRROR | IMX122_V_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_NONE] =
        {
            {0x0201, 0x00, IMX122_H_MIRROR | IMX122_V_FLIP},
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
    // hmax = (IMX122_HMAX_MSB & 0xFF) << 8 +
    //        (IMX122_HMAX_LSB & 0xFF) << 0
    .hmax_reg =
    {
        {0x0204, 0x00, 0xFF, 0x00, 0x08, 0}, // IMX122_HMAX_MSB
        {0x0203, 0x00, 0xFF, 0x00, 0x00, 0}, // IMX122_HMAX_LSB
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    // vmax = (IMX122_VMAX_MSB & 0xFF) << 8 +
    //        (IMX122_VMAX_LSB & 0xFF) << 0
    .vmax_reg =
    {
        {0x0206, 0x00, 0xFF, 0x00, 0x08, 0}, // IMX122_VMAX_MSB
        {0x0205, 0x00, 0xFF, 0x00, 0x00, 0}, // IMX122_VMAX_LSB
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .vmax_wadd_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    // shs = (INTEG_TIME_H & 0xFF) << 8 +
    //       (INTEG_TIME_L & 0xFF) << 0
    .shs_reg =
    {
        {0x0208, 0x00, 0xFF, 0x00, 0x08, 0}, // INTEG_TIME_H
        {0x0209, 0x00, 0xFF, 0x00, 0x00, 0}, // INTEG_TIME_L
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
        {0x021E, 0x00},
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

