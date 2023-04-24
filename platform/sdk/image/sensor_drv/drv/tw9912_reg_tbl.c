/*!
*****************************************************************************
** \file        tw9912_reg_tbl.c
**
** \version     $Id: tw9912_reg_tbl.c 11627 2017-03-10 10:58:12Z dengbiao $
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
GADI_VI_SensorDrvInfoT    tw9912_sensor =
{
    .magic_start        = GADI_SENSOR_HW_INFO_MAGIC_START,
    .version            = GADI_SENSOR_HW_INFO_VERSION,
    .length             = sizeof(GADI_VI_SensorDrvInfoT),
    .file_date          = 0x20171123,
    .HwInfo         =
    {
        .name               = "tw9912",
        .ctl_type           = GADI_SENSOR_CTL_I2C,
        .addr_len           = sizeof(GADI_U8),
        .data_len           = sizeof(GADI_U8),
        .reset_active_level = GADI_GPIO_LOW,
        .hw_addr            = {(0x88>>1), (0x8A>>1), 0, 0},
        .id_reg =
        {
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        .reset_reg =
        {
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        .standby_reg =
        {
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
    },
    .debug_print        = 0,
    .input_format       = GADI_VI_INPUT_FORMAT_YUV_422_PROG,
    .field_format       = 1,
    .adapter_id         = 0,
    .sensor_id          = GADI_ISP_SENSOR_TW9912,
    .sensor_double_step = 20,
    .typeofsensor       = 1,
    .sync_time_delay    = 40,
    .source_type.dummy  = GADI_VI_CMOS_CHANNEL_TYPE_AUTO,
    .dev_type           = GADI_VI_DEV_TYPE_DECODER,
    .video_system       = GADI_VIDEO_SYSTEM_AUTO,
    .vs_polarity        = GADI_VI_RISING_EDGE,
    .hs_polarity        = GADI_VI_RISING_EDGE,
    .data_edge          = GADI_VI_RISING_EDGE,
    .emb_sync_switch    = GADI_VI_EMB_SYNC_ON,
    .emb_sync_loc       = GADI_VI_EMB_SYNC_LOWER_PEL,
    .emb_sync_mode      = GADI_VI_EMB_SYNC_ITU_656,
    .max_width          = 864,  // 720+144
    .max_height         = 625,
    .def_sh_time        = GADI_VIDEO_FPS(50),
    .fmt_num            = 2,
    .auto_fmt           = GADI_VIDEO_MODE( 720,  576,  25, 1),
    .init_reg           =
    {
        {0xFF, 0x00},  /*Page 00*/
        {0x01, 0x79},
        {0x03, 0x20},
        {0x06, 0x03},
        {0x07, 0x12},
        {0x08, 0x14},
        {0x09, 0x20},
        {0x0A, 0x0E},
        {0x1A, 0x10},
        {0x1B, 0x00},
        {0x1C, 0x1F},
        {0x2F, 0x26},
        {0x30, 0x00},
        {0x31, 0x10},
        {0x32, 0x00},
        {0x36, 0xE2},
        {0x40, 0x00},
        {0x41, 0x80},
        {0x42, 0x00},
        {0xC0, 0x01},
        {0xC1, 0x07},
        {0xE1, 0x49},
        {0xE8, 0x0F},
        {0xE9, 0x61},
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .pll_table =
    {
        [0] =
        {
            .pixclk = 27000000, // Useless
            .extclk = 27000000,
            .regs =
            {
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
    },
    .video_fmt_table =
    {
        [0] = // D1 576P  (720 + 144) * 2 * 625 * 50 = 54000000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE( 720,  576,  25, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x05, 0x1E,},
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 720,
                .height     = 576,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_YUV_656,
                .interface  = GADI_VIDEO_IN_BT656,
                .bits       = GADI_VIDEO_BITS_8,
                .ratio      = GADI_VIDEO_RATIO_4_3,
                .srm        = 0,
                .pll_index  = 0,
                .max_fps    = GADI_VIDEO_FPS(25),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(25),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_720P_25,
            },
            .video_info_table =
            {
                .def_start_x    = 0,
                .def_start_y    = 44,
                .def_width      = 640,
                .def_height     = 480,
                .sync_start     = 0,
                .bayer_pattern  = GADI_VI_YUV_Y0_CB_Y1_CR,
                .mirror_bayer_pattern =
                {
                    GADI_VI_YUV_Y0_CB_Y1_CR,
                    GADI_VI_YUV_Y0_CB_Y1_CR,
                    GADI_VI_YUV_Y0_CB_Y1_CR,
                    GADI_VI_YUV_Y0_CB_Y1_CR,
                },
            },
            .update_after_vsync_start   = 37,
            .update_after_vsync_end     = 40,
        },
        [1] = //  D1 576I
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE( 720,  480,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x05, 0x0E},
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 720,
                .height     = 480,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_YUV_656,
                .interface  = GADI_VIDEO_IN_BT656,
                .bits       = GADI_VIDEO_BITS_8,
                .ratio      = GADI_VIDEO_RATIO_4_3,
                .srm        = 0,
                .pll_index  = 0,
                .max_fps    = GADI_VIDEO_FPS(30),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(30),
            },
            .video_info_table =
            {
                .def_start_x    = 0,
                .def_start_y    = 44,
                .def_width      = 640,
                .def_height     = 480,
                .sync_start     = 0,
                .bayer_pattern  = GADI_VI_YUV_Y0_CB_Y1_CR,
                .mirror_bayer_pattern =
                {
                    GADI_VI_YUV_Y0_CB_Y1_CR,
                    GADI_VI_YUV_Y0_CB_Y1_CR,
                    GADI_VI_YUV_Y0_CB_Y1_CR,
                    GADI_VI_YUV_Y0_CB_Y1_CR,
                },
            },
            .update_after_vsync_start   = 30,
            .update_after_vsync_end     = 33,
        },
    },
    .mirror_table =
    {
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY_VERTICALLY] =
        {
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY] =
        {
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_VERTICALLY] =
        {
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_NONE] =
        {
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
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .vmax_reg =
    {
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .vmax_wadd_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    .shs_reg =
    {
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .shs_additional_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    .cal_gain_mode      = GADI_SENSOR_CAL_GAIN_MODE0,
    .max_agc_index = 100,
    .max_again_index = 0,
    .cur_again_index = 0,
    .agc_info =
    {
        .db_max     = 0x2A000000,    // Useless
        .db_min     = 0x00000000,    // Useless
        .db_step    = 0x004ccccd,    // Useless
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

