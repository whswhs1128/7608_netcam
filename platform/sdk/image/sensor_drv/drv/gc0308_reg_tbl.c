/*!
*****************************************************************************
** \file        gc0308_reg_tbl.c
**
** \version     $Id: gc0308_reg_tbl.c 11627 2017-03-10 10:58:12Z dengbiao $
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
GADI_VI_SensorDrvInfoT    gc0308_sensor =
{
    .magic_start        = GADI_SENSOR_HW_INFO_MAGIC_START,
    .version            = GADI_SENSOR_HW_INFO_VERSION,
    .length             = sizeof(GADI_VI_SensorDrvInfoT),
    .file_date          = 0x20170207,
    .HwInfo         =
    {
        .name               = "gc0308",
        .ctl_type           = GADI_SENSOR_CTL_I2C,
        .addr_len           = sizeof(GADI_U8),
        .data_len           = sizeof(GADI_U8),
        .reset_active_level = GADI_GPIO_HIGH,
        .hw_addr            = {(0x42>>1), 0, 0, 0},
        .id_reg =
        {
            {0x00, 0x9B},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        .reset_reg =
        {
            {0xFE, 0x00},
            {0xFE, 0x80},
            {GADI_VI_SENSOR_TABLE_FLAG_MSLEEP, 0x14},
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
    .sensor_id          = GADI_ISP_SENSOR_GC0308,
    .sensor_double_step = 20,
    .typeofsensor       = 1,
    .sync_time_delay    = 40,
    .source_type.dummy  = GADI_VI_CMOS_CHANNEL_TYPE_AUTO,
    .dev_type           = GADI_VI_DEV_TYPE_DECODER,
    .video_system       = GADI_VIDEO_SYSTEM_AUTO,
    .vs_polarity        = GADI_VI_RISING_EDGE,
    .hs_polarity        = GADI_VI_RISING_EDGE,
    .data_edge          = GADI_VI_RISING_EDGE,
    .emb_sync_switch    = GADI_VI_EMB_SYNC_OFF,
    .emb_sync_loc       = GADI_VI_EMB_SYNC_LOWER_PEL,
    .emb_sync_mode      = GADI_VI_EMB_SYNC_ITU_656,
    .max_width          = 648,
    .max_height         = 488,
    .def_sh_time        = GADI_VIDEO_FPS(60),
    .fmt_num            = 1,
    .auto_fmt           = GADI_VIDEO_MODE(640,  480,  30, 1),
    .init_reg           =
    {
        {0xFE, 0x00}, // page 0x00
         // MCLK=25Mhz  it is 25m when use 24!!!
        {0x01, 0x32},
        {0x02, 0x89},
        {0x0F, 0x01},
        {0xE2, 0x00},
        {0xE3, 0x7D},
        {0xE4, 0x02},
        {0xE5, 0x71},
        {0xE6, 0x03},
        {0xE7, 0x6B},
        {0xE8, 0x0C},
        {0xE9, 0x35},
        {0xEA, 0x0E},
        {0xEB, 0xA6},

        {0xEC, 0x20},
        {0x05, 0x00},
        {0x06, 0x00},
        {0x07, 0x00},
        {0x08, 0x00},
        {0x09, 0x01},
        {0x0A, 0xE8},
        {0x0B, 0x02},
        {0x0C, 0x88},
        {0x0D, 0x02},
        {0x0E, 0x02},
        {0x10, 0x26},
        {0x11, 0x0D},
        {0x12, 0x2A},
        {0x13, 0x00},
        {0x14, 0x11},
        {0x15, 0x0A},
        {0x16, 0x05},
        {0x17, 0x01},
        {0x18, 0x44},
        {0x19, 0x44},
        {0x1A, 0x2A},
        {0x1B, 0x00},
        {0x1C, 0x49},
        {0x1D, 0x9A},
        {0x1E, 0x61},
        {0x1F, 0x16},
        {0x20, 0xFF},
        {0x21, 0xFA},
        {0x22, 0x57},
        {0x24, 0xA2}, // YCbYCr
        {0x25, 0x0F},
        {0x26, 0x03}, // 0x01
        {0x2F, 0x01},
        {0x30, 0xF7},
        {0x31, 0x50},
        {0x32, 0x00},
        {0x39, 0x04},
        {0x3A, 0x20},
        {0x3B, 0x20},
        {0x3C, 0x00},
        {0x3D, 0x00},
        {0x3E, 0x00},
        {0x3F, 0x00},
        {0x50, 0x14}, // 0x14
        {0x53, 0x80},
        {0x54, 0x80},
        {0x55, 0x87},
        {0x56, 0x80},

        {0x57, 0x84}, // james 20111108
        {0x58, 0x80},
        {0x59, 0x80},

        {0x8B, 0x20},
        {0x8C, 0x20},
        {0x8D, 0x20},
        {0x8E, 0x14},
        {0x8F, 0x10},
        {0x90, 0x14},
        {0x91, 0x3C},
        {0x92, 0x50},
        {0x5D, 0x12},
        {0x5E, 0x1A},
        {0x5F, 0x24},
        {0x60, 0x07},
        {0x61, 0x15},
        {0x62, 0x0F}, // 0x08
        {0x64, 0x01}, // 0x03
        {0x66, 0xE8},
        {0x67, 0x86},
        {0x68, 0xA2},
        {0x69, 0x18},
        {0x6A, 0x0F},
        {0x6B, 0x00},
        {0x6C, 0x5F},
        {0x6D, 0x8F},
        {0x6E, 0x55},
        {0x6F, 0x38},
        {0x70, 0x15},
        {0x71, 0x33},
        {0x72, 0xDC},
        {0x73, 0x80},
        {0x74, 0x02},
        {0x75, 0x3F},
        {0x76, 0x02},
        {0x77, 0x57}, // 0x47
        {0x78, 0x88},
        {0x79, 0x81},
        {0x7A, 0x81},
        {0x7B, 0x22},
        {0x7C, 0xFF},
        {0x93, 0x4C}, // 0x46
        {0x94, 0x02},
        {0x95, 0x07},
        {0x96, 0xE0},
        {0x97, 0x40},
        {0x98, 0xF0},
        {0xB1, 0x40},
        {0xB2, 0x40},
        {0xB3, 0x40}, // 0x40
        {0xB6, 0xE0},
        {0xBD, 0x3C},
        {0xBE, 0x36},
        {0xD0, 0xCB},
        {0xD1, 0x10},
        {0xD2, 0x90},
        {0xD3, 0x48},
        {0xD5, 0xF2},
        {0xD6, 0x10},
        {0xDB, 0x92},
        {0xDC, 0xA5},
        {0xDF, 0x23},
        {0xD9, 0x00},
        {0xDA, 0x00},
        {0xE0, 0x09},
        {0xED, 0x04},
        {0xEE, 0xA0},
        {0xEF, 0x40},
        {0x80, 0x03},

        {0x9F, 0x0E}, // case 2:
        {0xA0, 0x1C},
        {0xA1, 0x34},
        {0xA2, 0x48},
        {0xA3, 0x5A},
        {0xA4, 0x6B},
        {0xA5, 0x7B},
        {0xA6, 0x95},
        {0xA7, 0xAB},
        {0xA8, 0xBF},
        {0xA9, 0xCE},
        {0xAA, 0xD9},
        {0xAB, 0xE4},
        {0xAC, 0xEC},
        {0xAD, 0xF7},
        {0xAE, 0xFD},
        {0xAF, 0xFF},

        {0xC0, 0x00},
        {0xC1, 0x14},
        {0xC2, 0x21},
        {0xC3, 0x36},
        {0xC4, 0x49},
        {0xC5, 0x5B},
        {0xC6, 0x6B},
        {0xC7, 0x7B},
        {0xC8, 0x98},
        {0xC9, 0xB4},
        {0xCA, 0xCE},
        {0xCB, 0xE8},
        {0xCC, 0xFF},
        {0xF0, 0x02},
        {0xF1, 0x01},
        {0xF2, 0x02},
        {0xF3, 0x30},
        {0xF7, 0x12},
        {0xF8, 0x0A},
        {0xF9, 0x9F},
        {0xFA, 0x78},

        {0xFE, 0x01}, // page 0x01
        {0x00, 0xF5},
        {0x02, 0x20},
        {0x04, 0x10},
        {0x05, 0x08},
        {0x06, 0x20},
        {0x08, 0x0A},
        {0x0A, 0xA0},
        {0x0B, 0x64},
        {0x0C, 0x08},
        {0x0E, 0x44},
        {0x0F, 0x32},
        {0x10, 0x41},
        {0x11, 0x37},
        {0x12, 0x22},
        {0x13, 0x19},
        {0x14, 0x44},
        {0x15, 0x44},
        {0x16, 0xC2},
        {0x17, 0xA8},
        {0x18, 0x18},
        {0x19, 0x50},
        {0x1A, 0xD8},
        {0x1B, 0xF5},
        {0x70, 0x40},
        {0x71, 0x58},
        {0x72, 0x30},
        {0x73, 0x48},
        {0x74, 0x20},
        {0x75, 0x60},
        {0x77, 0x20},
        {0x78, 0x32},
        {0x30, 0x03},
        {0x31, 0x40},
        {0x32, 0x10},
        {0x33, 0xE0},
        {0x34, 0xE0},
        {0x35, 0x00},
        {0x36, 0x80},
        {0x37, 0x00},
        {0x38, 0x04},
        {0x39, 0x09},
        {0x3A, 0x12},
        {0x3B, 0x1C},
        {0x3C, 0x28},
        {0x3D, 0x31},
        {0x3E, 0x44},
        {0x3F, 0x57},
        {0x40, 0x6C},
        {0x41, 0x81},
        {0x42, 0x94},
        {0x43, 0xA7},
        {0x44, 0xB8},
        {0x45, 0xD6},
        {0x46, 0xEE},
        {0x47, 0x0D},
        {0xFE, 0x00}, // set back for page 0x00
        {GADI_VI_SENSOR_TABLE_FLAG_END, 0x00}
    },
    .pll_table =
    {
        [0] =
        {
            // For 640x480@60fps 640x480@30fps 640x480@25fps
            .pixclk = 24000000,
            .extclk = 24000000,
            .regs =
            {
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
    },
    .video_fmt_table =
    {
        [0] = //640x480@60fps   1800*800*30=43200000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(640,  480,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0xFE, 0x00},   // PAGE_SELECT
                {0x07, 0x00},   // X_ADD_STA_H
                {0x08, 0x04},   // X_ADD_STA_L
                {0x05, 0x00},   // Y_ADD_STA_H
                {0x06, 0x04},   // Y_ADD_STA_L
                {0x0B, 0x02},   // X_OUT_SIZE_H
                {0x0C, 0x80},   // X_OUT_SIZE_L
                {0x09, 0x01},   // Y_OUT_SIZE_H
                {0x0A, 0xE0},   // Y_OUT_SIZE_L
                {0x0D, 0x02},   // VS_ST
                {0x0E, 0x02},   // VS_ET
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 640,
                .height     = 480,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_YUV_601,
                .interface  = GADI_VIDEO_IN_BT601 ,              
                .bits       = GADI_VIDEO_BITS_8,
                .ratio      = GADI_VIDEO_RATIO_4_3,
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
                .def_width      = 640,
                .def_height     = 480,
                .sync_start     = (0 + 0),
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
    .vmax_clk_ref   = GADI_SENSOR_CAL_CLK_PIXCLK,
    .shs_clk_ref    = GADI_SENSOR_CAL_CLK_PIXCLK,
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
        //{0x0B, 0x00, 0xFF, 0x00, 0x08, 0}, // HMAX_MSB
        //{0x0C, 0x00, 0xFF, 0x00, 0x00, 0}, // HMAX_LSB
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    // vmax = (VMAX_MSB & 0xFF) << 8 +
    //        (VMAX_LSB & 0xFF) << 0
    .vmax_reg =
    {
        //{0x0340, 0x00, 0xFF, 0x00, 0x08, 0}, // VMAX_MSB
        //{0x0341, 0x00, 0xFF, 0x00, 0x00, 0}, // VMAX_LSB
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .vmax_wadd_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    // shs = (INTEG_TIME_H & 0x0F) << 8 +
    //       (INTEG_TIME_L & 0xFF) << 0
    .shs_reg =
    {
        //{0x0F, 0x00, 0x0F, 0x00, 0x08, 0}, // INTEG_TIME_H
        //{0x02, 0x00, 0xFF, 0x00, 0x00, 0}, // INTEG_TIME_L
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

