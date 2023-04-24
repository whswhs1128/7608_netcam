/*!
*****************************************************************************
** \file        bg0701_reg_tbl.c
**
** \version     $Id: bg0701_reg_tbl.c 11627 2017-03-10 10:58:12Z dengbiao $
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
GADI_VI_SensorDrvInfoT    bg0701_sensor =
{
    .magic_start        = GADI_SENSOR_HW_INFO_MAGIC_START,
    .version            = GADI_SENSOR_HW_INFO_VERSION,
    .length             = sizeof(GADI_VI_SensorDrvInfoT),
    .file_date          = 0x20161014,
    .HwInfo         =
    {
        .name               = "bg0701",
        .ctl_type           = GADI_SENSOR_CTL_I2C,
        .addr_len           = sizeof(GADI_U16),
        .data_len           = sizeof(GADI_U8),
        .reset_active_level = GADI_GPIO_LOW,
        .hw_addr            = {(0x64>>1), 0, 0, 0},
        .id_reg =
        {
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        .reset_reg =
        {
            {0x001C, 0x01},
            {GADI_VI_SENSOR_TABLE_FLAG_MSLEEP, 0x0A},
            {0x001C, 0x00},
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
    .sensor_id          = GADI_ISP_SENSOR_BG0701,
    .sensor_double_step = 20,
    .typeofsensor       = 1,
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
    .max_width          = 1650,
    .max_height         = 750,
    .def_sh_time        = GADI_VIDEO_FPS(60),
    .fmt_num            = 4,
    .auto_fmt           = GADI_VIDEO_MODE(1280,  720,  25, 1),
    .init_reg           =
    {
        {0x0089, 0x21},     //internal vddpix off
        {0x00B9, 0x22},     //Auto Gain

        {0x0014, 0x03},     //TXB ON *
        {0x001E, 0x0F},     //VTH 3.8V please check the voltage
        {0x0020, 0x01},     //mirror

        {0x0028, 0x00},     //RAMP1 ONLY
        {0x0029, 0x18},     //RSTB =1us
        {0x002A, 0x18},     //TXB = 1us
        {0x002D, 0x00},
        {0x002E, 0x01},     //ibias_cnten_gap
        {0x0030, 0x18},     //rstb_cmprst_gap=1u
        {0x0034, 0x20},     //tx_ramp2=32 CLKIN cycle*

        {0x0038, 0x03},
        {0x0039, 0xFD},
        {0x003A, 0x03},
        {0x003B, 0xFA},

        {0x0060, 0x00},     //row refresh mode
        {0x006D, 0x01},     //pll=288M pclk=72M    {when clkin=24M)
        {0x0064, 0x02},
        {0x0065, 0x00},     //RAMP1 length=200
        {0x0067, 0x05},
        {0x0068, 0xFF},     //RAMP1 length=5ff
        {0x0087, 0xAF},     //votlgate of vbg-i
        {0x001D, 0x01},     //restart

        // select page1, don't use, page is included in reg addr
        //(BG0701_REG_PAGE, 0x01},
        {0x01C8, 0x04},
        {0x01C7, 0x55},     // FD Gain = 1X
        {0x01E0, 0x01},
        {0x01E1, 0x04},
        {0x01E2, 0x03},
        {0x01E3, 0x02},
        {0x01E4, 0x01},
        {0x01E5, 0x01},     //vcm_comp =2.56V
        {0x01B4, 0x01},     //row noise remove on*
        {0x0120, 0x00},     //blcc off
        {0x0131, 0x00},     //blcc target upper high
        {0x0132, 0x38},
        {0x0133, 0x00},     //blcc target upper low
        {0x0134, 0x35},
        {0x0135, 0x00},     //blcc target lower high
        {0x0136, 0x33},
        {0x0137, 0x00},     //blcc target lower low
        {0x0138, 0x30},
        {0x0139, 0x04},     //frame count to ave

        {0x013E, 0x07},
        {0x013F, 0xFF},     // Upper Limit

        {0x0140, 0xFF},
        {0x0141, 0xC0},     // Lower Limit

        {0x0120, 0x00},     //blcc on

        {0x014E, 0x00},
        {0x014F, 0x00},     //digital offset

        {0x01F1, 0x07},     //dpc on

        // select page0, don't use, page is included in reg addr
        //(BG0701_REG_PAGE, 0x00},
        {0x007F, 0x00},     //cmp current
        {0x0081, 0x09},     //dot_en=1,vrst=vth,vtx=vth
        {0x0082, 0x11},     //bandgap current & ramp current
        {0x0083, 0x01},     //pixel current
        {0x0084, 0x07},     //check rst voltage
        {0x0088, 0x05},     //pclk phase
        {0x008A, 0x01},     //pclk drv
        {0x008C, 0x01},     //data drv
        {0x00B0, 0x01},
        {0x00B1, 0x7F},
        {0x00B2, 0x01},
        {0x00B3, 0x7F},     //analog gain=1X
        {0x00B4, 0x11},
        {0x00B5, 0x11},
        {0x00B6, 0x11},
        {0x00B7, 0x01},
        {0x00B8, 0x00},     //digital gain=1X
        {0x00BF, 0x0C},
        {0x008E, 0x00},     //OEN
        {0x008D, 0x00},     //OEN
        {0x001D, 0x02},
        {GADI_VI_SENSOR_TABLE_FLAG_END, 0x00}//end flag
    },
    .pll_table =
    {
        [0] =
        {
            //for 60fps
            .pixclk = 74250000,
            .extclk = 27000000,
            .regs =
            {
                {0x0050, 0x00},
                {0x0052, 0xDD},
                {0x0053, 0x6C},
                {0x0054, 0x03},
                {0x001D, 0x02},
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
        [1] =
        {
            //for 50fps
            .pixclk = 54000000,
            .extclk = 27000000,
            .regs =
            {
                {0x0050, 0x00},
                {0x0052, 0xDD},
                {0x0053, 0x4E},
                {0x0054, 0x03},
                {0x001D, 0x02},
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
        [2] =
        {
            //for 30fps
            .pixclk = 37125000,
            .extclk = 27000000,
            .regs =
            {
                {0x0050, 0x00},
                {0x0052, 0xDD},
                {0x0053, 0x6C},
                {0x0054, 0x08},
                {0x001D, 0x02},
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
        [3] =
        {
            //for 25fps
            .pixclk = 27000000,
            .extclk = 27000000,
            .regs =
            {
                {0x0050, 0x00},
                {0x0052, 0xDD},
                {0x0053, 0x4E},
                {0x0054, 0x08},
                {0x001D, 0x02},
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
    },
    .video_fmt_table =
    {
        [0] = //1280x720@60fps    1650*750*60=74.25MHz
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  60, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x0002,    0x00},  // BG0701_HSTARTH
                {0x0003,    0x47},  // BG0701_HSTARTL
                {0x0004,    0x00},  // BG0701_VSTARTH
                {0x0005,    0x0E},  // BG0701_VSTARTL
                {0x0006,    0x05},  // BG0701_HSIZEH
                {0x0007,    0x08},  // BG0701_HSIZEL
                {0x0008,    0x02},  // BG0701_VSIZEH
                {0x0009,    0xD8},  // BG0701_VSIZEL
                {0x000E,    0x02},  // BG0701_ROWTIMEH
                {0x000F,    0x58},  // BG0701_ROWTIMEL
                {0x0021,    0x00},  // BG0701_VBLANKH
                {0x0022,    0x0E},  // BG0701_VBLANKL
                {0x0020,    0x00},  // BG0701_READC
                {0x001D,    0x02},  // BG0701_RESTART
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
        [1] = //1280x720@50fps    1440*750*50=54MHz
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  50, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x0002,    0x00},  // BG0701_HSTARTH
                {0x0003,    0x47},  // BG0701_HSTARTL
                {0x0004,    0x00},  // BG0701_VSTARTH
                {0x0005,    0x0E},  // BG0701_VSTARTL
                {0x0006,    0x05},  // BG0701_HSIZEH
                {0x0007,    0x08},  // BG0701_HSIZEL
                {0x0008,    0x02},  // BG0701_VSIZEH
                {0x0009,    0xD8},  // BG0701_VSIZEL
                {0x000E,    0x02},  // BG0701_ROWTIMEH
                {0x000F,    0x58},  // BG0701_ROWTIMEL
                {0x0021,    0x00},  // BG0701_VBLANKH
                {0x0022,    0x0E},  // BG0701_VBLANKL
                {0x0020,    0x00},  // BG0701_READC
                {0x001D,    0x02},  // BG0701_RESTART
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
        [2] = //1280x720@30fps    1650*750*30=37.125MHz
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x0002,    0x00},  // BG0701_HSTARTH
                {0x0003,    0x47},  // BG0701_HSTARTL
                {0x0004,    0x00},  // BG0701_VSTARTH
                {0x0005,    0x0E},  // BG0701_VSTARTL
                {0x0006,    0x05},  // BG0701_HSIZEH
                {0x0007,    0x08},  // BG0701_HSIZEL
                {0x0008,    0x02},  // BG0701_VSIZEH
                {0x0009,    0xD8},  // BG0701_VSIZEL
                {0x000E,    0x02},  // BG0701_ROWTIMEH
                {0x000F,    0x58},  // BG0701_ROWTIMEL
                {0x0021,    0x00},  // BG0701_VBLANKH
                {0x0022,    0x0E},  // BG0701_VBLANKL
                {0x0020,    0x00},  // BG0701_READC
                {0x001D,    0x02},  // BG0701_RESTART
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
        [3] = //1280x720@25fps    1440*750*25=27MHz
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  25, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x0002,    0x00},  // BG0701_HSTARTH
                {0x0003,    0x47},  // BG0701_HSTARTL
                {0x0004,    0x00},  // BG0701_VSTARTH
                {0x0005,    0x0E},  // BG0701_VSTARTL
                {0x0006,    0x05},  // BG0701_HSIZEH
                {0x0007,    0x08},  // BG0701_HSIZEL
                {0x0008,    0x02},  // BG0701_VSIZEH
                {0x0009,    0xD8},  // BG0701_VSIZEL
                {0x000E,    0x02},  // BG0701_ROWTIMEH
                {0x000F,    0x58},  // BG0701_ROWTIMEL
                {0x0021,    0x00},  // BG0701_VBLANKH
                {0x0022,    0x0E},  // BG0701_VBLANKL
                {0x0020,    0x00},  // BG0701_READC
                {0x001D,    0x02},  // BG0701_RESTART
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
#define BG0701_V_FLIP       (1<<1)
#define BG0701_H_MIRROR     (1<<0)

    .mirror_table =
    {
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY_VERTICALLY] =
        {
            {0x0020, BG0701_H_MIRROR | BG0701_V_FLIP, BG0701_H_MIRROR | BG0701_V_FLIP},
            {0x001D, 0x02},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY] =
        {
            {0x0020, BG0701_H_MIRROR, BG0701_H_MIRROR | BG0701_V_FLIP},
            {0x001D, 0x02},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_VERTICALLY] =
        {
            {0x0020, BG0701_V_FLIP, BG0701_H_MIRROR | BG0701_V_FLIP},
            {0x001D, 0x02},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_NONE] =
        {
            {0x0020, 0x00, BG0701_H_MIRROR | BG0701_V_FLIP},
            {0x001D, 0x02},
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
    // hmax = (BG0701_VSIZEH & 0x3F) << 8 +
    //        (BG0701_VSIZEL & 0xFF) << 0
    .hmax_reg =
    {
        {0x0008, 0x00, 0x3F, 0x00, 0x08, 0}, // BG0701_VSIZEH
        {0x0009, 0x00, 0xFF, 0x00, 0x00, 0}, // BG0701_VSIZEL
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    // vmax = (BG0701_VBLANKH & 0xFF) << 8 +
    //        (BG0701_VBLANKL & 0xFF) << 0
    .vmax_reg =
    {
        {0x0021, 0x00, 0x3F, 0x00, 0x08, 0}, // BG0701_VBLANKH
        {0x0022, 0x00, 0xFF, 0x00, 0x00, 0}, // BG0701_VBLANKL
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .vmax_wadd_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    // shs = (BG0701_ROWTIMEH & 0x3F) << 8 +
    //       (BG0701_ROWTIMEL & 0xFF) << 0
    .shs_reg =
    {
        {0x000E, 0x00, 0x3F, 0x00, 0x08, 0}, // BG0701_ROWTIMEH
        {0x000F, 0x00, 0xFF, 0x00, 0x00, 0}, // BG0701_ROWTIMEL
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .shs_additional_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    .cal_gain_mode              = GADI_SENSOR_CAL_GAIN_MODE0,
    .max_agc_index = 0,
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

