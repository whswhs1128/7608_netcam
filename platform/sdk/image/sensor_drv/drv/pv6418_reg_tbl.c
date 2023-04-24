/*!
*****************************************************************************
** \file        pv6418_reg_tbl.c
**
** \version     $Id: pv6418_reg_tbl.c 10045 2016-09-03 04:58:45Z yulindeng $
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
GADI_VI_SensorDrvInfoT    pv6418_mach =
{
    .magic_start        = GADI_SENSOR_HW_INFO_MAGIC_START,
    .version            = GADI_SENSOR_HW_INFO_VERSION,
    .length             = sizeof(GADI_VI_SensorDrvInfoT),
    .file_date          = 0x20171123,
    .HwInfo         =
    {
        .name               = "pv6418",
        .ctl_type           = GADI_SENSOR_CTL_I2C,
        .addr_len           = sizeof(GADI_U16),
        .data_len           = sizeof(GADI_U8),
        .reset_active_level = GADI_GPIO_LOW,
        .hw_addr            = {(0x6C>>1), 0, 0, 0},
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
    .sensor_id          = GADI_ISP_MACH_PV6418,
    .sensor_double_step = 16,
    .typeofsensor       = 1,
    .sync_time_delay    = 0,
    .source_type.dummy  = GADI_VI_CMOS_CHANNEL_TYPE_AUTO,
    .dev_type           = GADI_VI_DEV_TYPE_CMOS,
    .video_system       = GADI_VIDEO_SYSTEM_AUTO,
    .vs_polarity        = GADI_VI_RISING_EDGE,
    .hs_polarity        = GADI_VI_RISING_EDGE,
    .data_edge          = GADI_VI_RISING_EDGE,
    .emb_sync_switch    = GADI_VI_EMB_SYNC_ON,
    .emb_sync_loc       = GADI_VI_EMB_SYNC_UPPER_PEL,
    .emb_sync_mode      = GADI_VI_EMB_SYNC_ITU_656,
    .max_width          = 1952,
    .max_height         = 1092,
    .def_sh_time        = GADI_VIDEO_FPS(50),
    .fmt_num            = 6,
    .auto_fmt           = GADI_VIDEO_MODE(1920, 1080,  30, 1),
    .init_reg           =
    {
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .pll_table =
    {
        [0] =
        {
            .pixclk = 74250000,
            .extclk = 27000000,
            .regs =
            {
                {GADI_VI_SENSOR_TABLE_FLAG_END,}
            },
        },
    },
    .video_fmt_table =
    {
        [0] = //720P@60
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
                .type       = GADI_VIDEO_TYPE_YUV_BT1120,
                .interface  = GADI_VIDEO_IN_BT1120, 
                .bits       = GADI_VIDEO_BITS_8,
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
                .bayer_pattern  = GADI_VI_YUV_Y0_CB_Y1_CR,
                .mirror_bayer_pattern =
                {
                    GADI_VI_YUV_Y0_CB_Y1_CR,
                    GADI_VI_YUV_Y0_CB_Y1_CR,
                    GADI_VI_YUV_Y0_CB_Y1_CR,
                    GADI_VI_YUV_Y0_CB_Y1_CR,
                },
            },
            .update_after_vsync_start   = 13,
            .update_after_vsync_end     = 16,
        },
        [1] = //720P@50
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
                .type       = GADI_VIDEO_TYPE_YUV_BT1120,
                .interface  = GADI_VIDEO_IN_BT1120, 
                .bits       = GADI_VIDEO_BITS_8,
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
            .update_after_vsync_start   = 17,
            .update_after_vsync_end     = 20,
        },
        [2] = //720P@30
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  30, 1),
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
                .type       = GADI_VIDEO_TYPE_YUV_BT1120,
                .interface  = GADI_VIDEO_IN_BT1120, 
                .bits       = GADI_VIDEO_BITS_8,
                .ratio      = GADI_VIDEO_RATIO_16_9,
                .srm        = 0,
                .pll_index  = 0,
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
        [3] = //720P@25
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
                .type       = GADI_VIDEO_TYPE_YUV_BT1120,
                .interface  = GADI_VIDEO_IN_BT1120, 
                .bits       = GADI_VIDEO_BITS_8,
                .ratio      = GADI_VIDEO_RATIO_16_9,
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
                .def_start_y    = 0,
                .def_width      = 1280,
                .def_height     = 720,
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
        [4] = //1080P@30
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1920, 1080,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 1920,
                .height     = 1080,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_YUV_BT1120,
                .interface  = GADI_VIDEO_IN_BT1120, 
                .bits       = GADI_VIDEO_BITS_8,
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
        [5] = //1080P@25
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1920, 1080,  25, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 1920,
                .height     = 1080,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_YUV_BT1120,
                .interface  = GADI_VIDEO_IN_BT1120, 
                .bits       = GADI_VIDEO_BITS_8,
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
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .gain_table =
    {
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
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



