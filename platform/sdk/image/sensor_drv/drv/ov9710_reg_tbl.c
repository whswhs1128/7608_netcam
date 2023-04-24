/*!
*****************************************************************************
** \file        ov9710_reg_tbl.c
**
** \version     $Id: ov9710_reg_tbl.c 11627 2017-03-10 10:58:12Z dengbiao $
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
GADI_VI_SensorDrvInfoT    ov9710_sensor =
{
    .magic_start        = GADI_SENSOR_HW_INFO_MAGIC_START,
    .version            = GADI_SENSOR_HW_INFO_VERSION,
    .length             = sizeof(GADI_VI_SensorDrvInfoT),
    .file_date          = 0x20161014,
    .HwInfo         =
    {
        .name               = "ov9710",
        .ctl_type           = GADI_SENSOR_CTL_I2C,
        .addr_len           = sizeof(GADI_U8),
        .data_len           = sizeof(GADI_U8),
        .reset_active_level = GADI_GPIO_LOW,
        .hw_addr            = {(0x60>>1), 0, 0, 0},
        .id_reg =
        {
            {0x0A, 0x97},
            {0x0B, 0x11},
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
    .input_format       = GADI_VI_INPUT_FORMAT_RGB_RAW,
    .field_format       = 1,
    .adapter_id         = 0,
    .sensor_id          = GADI_ISP_SENSOR_OV9710,
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
    .max_width          = 1296,
    .max_height         = 818,
    .def_sh_time        = GADI_VIDEO_FPS(30),
    .fmt_num            = 2,
    .auto_fmt           = GADI_VIDEO_MODE(1280,  720,  30, 1),
    .init_reg           =
    {
        {0x12, 0x80},
        {0x1e, 0x07},
        {0x5f, 0x18},
        {0x69, 0x04},
        {0x65, 0x2a},
        {0x68, 0x0a},
        {0x39, 0x28},
        {0x4d, 0x90},
        {0xc1, 0x80},

        {0x96, 0x01},        //karst
        {0xbc, 0x68},
        //{0x12, 0x00},
        //{0x3b, 0x00},
        {0x97, 0x80},
        {0x37, 0x02},
        {0x38, 0x10},
        {0x4e, 0x55},
        {0x4f, 0x55},
        {0x50, 0x55},
        {0x51, 0x55},
        {0x24, 0x55},
        {0x25, 0x40},
        {0x26, 0xa1},

        {0x5c, 0x59},
        {0x5d, 0x00},
        {0x11, 0x01},

        {0x13, 0x80},
        {0x14, 0x60},
        {0xc2, 0x41},

        {0xcb, 0x32},
        //{0xcb, 0xb6},               // consistent with DV code
        {0xc3, 0x21},
        {0x41, 0x80},
        {0x6d, 0x02},    // added for FPN optimum
        {GADI_VI_SENSOR_TABLE_FLAG_END, 0x00}//end flag
    },
    .pll_table =
    {
        // 80MHz >=pixclk = (extclk/REG5C[6:5])*(32-REG5C[4:0])/(REG5D[3:2]+1)/((CLK[5:0]+1)*2)
        [0] = { //for 29.97fps
            .pixclk = 41962436,         // pixclk = (extclk/REG5C[6:5])*(32-REG5C[4:0])/(REG5D[3:2]+1)/((CLK[5:0]+1)*2)
            .extclk = 23978535,         //        = (23978535/2)*(32-25)/(0+1)/((0+1)*2)=41962436.25
            .regs = {
                {0x5C, 0x59},       // 0 10 11001
                {0x5D, 0x30},       // 0000 00 00
                {0x11, 0x00},       // 000 00000
                {GADI_VI_SENSOR_TABLE_FLAG_END, 0x00}//end flag
            }
        },
        [1] = { //for 59.94fps(640x400)
            .pixclk = 41887990,         // pixclk = (extclk/REG5C[6:5])*(32-REG5C[4:0])/(REG5D[3:2]+1)/((CLK[5:0]+1)*2)
            .extclk = 23935994,         //        = (23935994/2)*(32-25)/(0+1)/((0+1)*2)=41887989.5
            .regs = {
                {0x5C, 0x59},           // 0 10 11001
                {0x5D, 0x30},           // 0000 00 00
                {0x11, 0x00},           // 000 00000
                {GADI_VI_SENSOR_TABLE_FLAG_END, 0x00}//end flag
            }
        },
        [2] = { //for 30fps
            .pixclk = 42004440,         // pixclk = (extclk/REG5C[6:5])*(32-REG5C[4:0])/(REG5D[3:2]+1)/((CLK[5:0]+1)*2)
            .extclk = 24002537,         //        = (24002537/2)*(32-25)/(0+1)/((0+1)*2)=42004439.75
            .regs = {
                {0x5C, 0x59},           // 0 10 11001
                {0x5D, 0x30},           // 0000 00 00
                {0x11, 0x00},           // 000 00000
                {GADI_VI_SENSOR_TABLE_FLAG_END, 0x00}//end flag
            }
        },
        [3] = { //for 60fps(640x400)
            .pixclk = 41929920,         // pixclk = (extclk/REG5C[6:5])*(32-REG5C[4:0])/(REG5D[3:2]+1)/((CLK[5:0]+1)*2)
            .extclk = 23959954,         //        = (23959954/2)*(32-25)/(0+1)/((0+1)*2)=41929919.5
            .regs = {
                {0x5C, 0x59},           // 0 10 11001
                {0x5D, 0x30},           // 0000 00 00
                {0x11, 0x00},           // 000 00000
                {GADI_VI_SENSOR_TABLE_FLAG_END, 0x00}//end flag
            }
        },
    },
    .video_fmt_table =
    {
        [0] = //1280x800
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .video_info_table =
            {
                .def_start_x    = 0,
                .def_start_y    = 40,
                .def_width      = 1280,
                .def_height     = 720,
                .sync_start     = (0 + 5),
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
            .fmt_reg =
            {
                {0x17, 0x25},   //OV9710_HSTART,
                {0x18, 0xA2},   //OV9710_AHSIZE,
                {0x19, 0x01},   //OV9710_VSTART,
                {0x1A, 0xCA},   //OV9710_AVSIZE,
                {0x03, 0x0A},   //OV9710_REG03,
                {0x32, 0x07},   //OV9710_REG32,
                {0x98, 0x00},   //OV9710_DSP_CTRL_2,
                {0x99, 0x00},   //OV9710_DSP_CTRL_3,
                {0x9A, 0x00},   //OV9710_DSP_CTRL_4,
                {0x57, 0x00},   //OV9710_REG57,
                {0x58, 0xC8},   //OV9710_REG58,
                {0x59, 0xA0},   //OV9710_REG59,
                {0x4C, 0x13},   //OV9710_RSVD4C,
                {0x4B, 0x36},   //OV9710_RSVD4B,
                {0x3D, 0x3C},   //OV9710_RENDL,
                {0x3E, 0x03},   //OV9710_RENDH,
                {0xBD, 0xA0},   //OV9710_YAVG_CTRL_0,
                {0xBE, 0xC8},   //OV9710_YAVG_CTRL_1,
                {0x12, 0x00},   //OV9710_COM7
                {0x3B, 0x00},   //OV9710_REG3B
                {0x2A, 0x9B},   //OV9710_REG2A
                {0x2B, 0x06},   //OV9710_REG2B
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 1280,
                .height     = 800,
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
                .def_start_y    = 40,
                .def_width      = 1280,
                .def_height     = 720,
                .sync_start     = (0 + 5),
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
        [1] = //VGA
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE( 640,  400,  25, 0),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x17, 0x25},   //OV9710_HSTART,
                {0x18, 0xA2},   //OV9710_AHSIZE,
                {0x19, 0x01},   //OV9710_VSTART,
                {0x1A, 0x64},   //OV9710_AVSIZE,
                {0x03, 0x02},   //OV9710_REG03,
                {0x32, 0x07},   //OV9710_REG32,
                {0x98, 0x00},   //OV9710_DSP_CTRL_2,
                {0x99, 0x00},   //OV9710_DSP_CTRL_3,
                {0x9A, 0x00},   //OV9710_DSP_CTRL_4,
                {0x57, 0x00},   //OV9710_REG57,
                {0x58, 0x64},   //OV9710_REG58,
                {0x59, 0x50},   //OV9710_REG59,
                {0x4C, 0x09},   //OV9710_RSVD4C,
                {0x4B, 0x9A},   //OV9710_RSVD4B,
                {0x3D, 0x9E},   //OV9710_RENDL,
                {0x3E, 0x01},   //OV9710_RENDH,
                {0xBD, 0x50},   //OV9710_YAVG_CTRL_0,
                {0xBE, 0x64},   //OV9710_YAVG_CTRL_1,
                {0x12, 0x40},   //OV9710_COM7
                {0x3B, 0x01},   //OV9710_REG3B
                {0x2A, 0x98},   //OV9710_REG2A
                {0x2B, 0x06},   //OV9710_REG2B
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 640,
                .height     = 400,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_DVP, 
                .bits       = GADI_VIDEO_BITS_10,
                .ratio      = GADI_VIDEO_RATIO_4_3,
                .srm        = 0,
                .pll_index  = 3,
                .max_fps    = GADI_VIDEO_FPS(60),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(60),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN,
            },
            .video_info_table =
            {
                .def_start_x    = 0,
                .def_start_y    = 0,
                .def_width      = 640,
                .def_height     = 400,
                .sync_start     = (0 + 5),
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
#define OV9710_HORIZ_MIRROR     (0x01 << 7)
#define OV9710_VERT_FLIP        (0x01 << 6)
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY_VERTICALLY] =
        {
            {0x04, 0x01, 0x01},
            {0x04, OV9710_HORIZ_MIRROR | OV9710_VERT_FLIP, OV9710_HORIZ_MIRROR | OV9710_VERT_FLIP},
            {0x03, 0x01, 0x0F},
            {0xC3, 0x21},
            {0xFF, 0xFF},
            {GADI_VI_SENSOR_TABLE_FLAG_SLEEP, 0x01},
            {0x04, 0x00, 0x01},
            {0xC3, 0x21},
            {0xFF, 0xFF},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY] =
        {
            {0x04, 0x01, 0x01},
            {0x04, OV9710_HORIZ_MIRROR, OV9710_HORIZ_MIRROR | OV9710_VERT_FLIP},
            {0x03, 0x00, 0x0F},
            {0xC3, 0x21},
            {0xFF, 0xFF},
            {GADI_VI_SENSOR_TABLE_FLAG_SLEEP, 0x01},
            {0x04, 0x00, 0x01},
            {0xC3, 0x21},
            {0xFF, 0xFF},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_VERTICALLY] =
        {
            {0x04, 0x01, 0x01},
            {0x04, OV9710_VERT_FLIP, OV9710_HORIZ_MIRROR | OV9710_VERT_FLIP},
            {0x03, 0x01, 0x0F},
            {0xC3, 0x21},
            {0xFF, 0xFF},
            {GADI_VI_SENSOR_TABLE_FLAG_SLEEP, 0x01},
            {0x04, 0x00, 0x01},
            {0xC3, 0x21},
            {0xFF, 0xFF},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_NONE] =
        {
            {0x04, 0x01, 0x01},
            {0x04, 0x00, OV9710_HORIZ_MIRROR | OV9710_VERT_FLIP},
            {0x03, 0x0A, 0x0F},
            {0xC3, 0x21},
            {0xFF, 0xFF},
            {GADI_VI_SENSOR_TABLE_FLAG_SLEEP, 0x01},
            {0x04, 0x00, 0x01},
            {0xC3, 0x21},
            {0xFF, 0xFF},
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
    .shs_fix        = 2,
    .shs_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_SET,
    .agc_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_SET,
    .vmax_reg_update_mode       = GADI_SENSOR_UPDATE_REG_BY_SET,
    .mirror_reg_update_mode     = GADI_SENSOR_UPDATE_REG_BY_SET,
    .shs_additional_use         = 0,
    .shs_additional_percent     = 0,
    // hmax = (HTS_H & 0xFF) << 8 +
    //        (HTS_L & 0xFF) << 0
    .hmax_reg =
    {
        {0x2B, 0x00, 0xFF, 0x00, 0x08, 0},
        {0x2A, 0x00, 0xFF, 0x00, 0x00, 0},
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    // vmax = (VTS_H & 0xFF) << 8 +
    //        (VTS_L & 0xFF) << 0
    .vmax_reg =
    {
        {0x3E, 0x00, 0xFF, 0x00, 0x08, 0}, // VTS_H
        {0x3D, 0x00, 0xFF, 0x00, 0x00, 0}, // VTS_L
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .vmax_wadd_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    // shs = (AEC_H & 0xFF) << 8 +
    //       (AEC_L & 0xFF) << 0
    .shs_reg =
    {
        {0x16, 0x00, 0xFF, 0x00, 0x08, 0}, // AEC_H
        {0x10, 0x00, 0xFF, 0x00, 0x00, 0}, // AEC_L
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .shs_additional_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    .cal_gain_mode              = GADI_SENSOR_CAL_GAIN_MODE0,
    .max_agc_index = 81,
    .max_again_index = 0,
    .cur_again_index = 0,
    .agc_info =
    {
        .db_max     = 0x1E000000,    // 30dB
        .db_min     = 0x00000000,    // 0dB
        .db_step    = 0x00600000,    // 0.375dB
    },
    .gain_reg =
    {
        {0x00,}, // [7] 0:apply dgain only gain>=2X 1:apply dgain only gain>=4X
                 // [6:0] again
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .gain_table =
    {
        // gain_value*256,  register
        {0x00600000 * 0x0050, {0xff}},    /* index 0   0.000000   : 30dB */
        {0x00600000 * 0x004F, {0xff}},    /* index 1   0.356695   */
        {0x00600000 * 0x004E, {0xfe}},    /* index 2   -0.344130  */
        {0x00600000 * 0x004D, {0xfd}},    /* index 3   -0.100035  */
        {0x00600000 * 0x004C, {0xfc}},    /* index 4   0.091314   */
        {0x00600000 * 0x004B, {0xfb}},    /* index 5   0.232155   */
        {0x00600000 * 0x004A, {0xfa}},    /* index 6   0.324627   */
        {0x00600000 * 0x0049, {0xf9}},    /* index 7   0.370781   */
        {0x00600000 * 0x0048, {0xf8}},    /* index 8   0.372583   */
        {0x00600000 * 0x0047, {0xf7}},    /* index 9   0.331911   */
        {0x00600000 * 0x0046, {0xf6}},    /* index 10  0.250566   */
        {0x00600000 * 0x0045, {0xf5}},    /* index 11  0.130276   */
        {0x00600000 * 0x0044, {0xf4}},    /* index 12  -0.027313  */
        {0x00600000 * 0x0043, {0xf3}},    /* index 13  -0.220617  */
        {0x00600000 * 0x0042, {0xf2}},    /* index 14  -0.448124  */
        {0x00600000 * 0x0041, {0xf1}},    /* index 15  0.291620   */
        {0x00600000 * 0x0040, {0xf0}},    /* index 16  0.000000   : 24dB */
        {0x00600000 * 0x003F, {0x7f}},    /* index 17  -0.321652  */
        {0x00600000 * 0x003E, {0x7e}},    /* index 18  0.327935   */
        {0x00600000 * 0x003D, {0x7d}},    /* index 19  -0.050017  */
        {0x00600000 * 0x003C, {0x7c}},    /* index 20  -0.454343  */
        {0x00600000 * 0x003B, {0x7b}},    /* index 21  0.116077   */
        {0x00600000 * 0x003A, {0x7a}},    /* index 22  -0.337687  */
        {0x00600000 * 0x0039, {0x79}},    /* index 23  0.185390   */
        {0x00600000 * 0x0038, {0x78}},    /* index 24  -0.313708  */
        {0x00600000 * 0x0037, {0x77}},    /* index 25  0.165956   */
        {0x00600000 * 0x0036, {0x76}},    /* index 26  -0.374717  */
        {0x00600000 * 0x0035, {0x75}},    /* index 27  0.065138   */
        {0x00600000 * 0x0034, {0x74}},    /* index 28  0.486343   */
        {0x00600000 * 0x0033, {0x73}},    /* index 29  -0.110309  */
        {0x00600000 * 0x0032, {0x72}},    /* index 30  0.275938   */
        {0x00600000 * 0x0031, {0x71}},    /* index 31  -0.354190  */
        {0x00600000 * 0x0030, {0x70}},    /* index 32  0.000000   : 18dB */
        {0x00600000 * 0x002F, {0x3f}},    /* index 33  0.356695   */
        {0x00600000 * 0x002E, {0x3e}},    /* index 34  -0.344130  */
        {0x00600000 * 0x002D, {0x3d}},    /* index 35  -0.100035  */
        {0x00600000 * 0x002C, {0x3c}},    /* index 36  0.091314   */
        {0x00600000 * 0x002B, {0x3b}},    /* index 37  0.232155   */
        {0x00600000 * 0x002A, {0x3a}},    /* index 38  0.324627   */
        {0x00600000 * 0x0029, {0x39}},    /* index 39  0.370781   */
        {0x00600000 * 0x0028, {0x38}},    /* index 40  0.372583   */
        {0x00600000 * 0x0027, {0x37}},    /* index 41  0.331911   */
        {0x00600000 * 0x0026, {0x36}},    /* index 42  0.250566   */
        {0x00600000 * 0x0025, {0x35}},    /* index 43  0.130276   */
        {0x00600000 * 0x0024, {0x34}},    /* index 44  -0.027313  */
        {0x00600000 * 0x0023, {0x33}},    /* index 45  -0.220617  */
        {0x00600000 * 0x0022, {0x32}},    /* index 46  -0.448124  */
        {0x00600000 * 0x0021, {0x31}},    /* index 47  0.291620   */
        {0x00600000 * 0x0020, {0x30}},    /* index 48  0.000000   : 12dB */
        {0x00600000 * 0x001F, {0x1f}},    /* index 49  0.356695   */
        {0x00600000 * 0x001E, {0x1e}},    /* index 50  -0.344130  */
        {0x00600000 * 0x001D, {0x1d}},    /* index 51  -0.100035  */
        {0x00600000 * 0x001C, {0x1c}},    /* index 52  0.091314   */
        {0x00600000 * 0x001B, {0x1b}},    /* index 53  0.232155   */
        {0x00600000 * 0x001A, {0x1a}},    /* index 54  0.324627   */
        {0x00600000 * 0x0019, {0x19}},    /* index 55  0.370781   */
        {0x00600000 * 0x0018, {0x18}},    /* index 56  0.372583   */
        {0x00600000 * 0x0017, {0x17}},    /* index 57  0.331911   */
        {0x00600000 * 0x0016, {0x16}},    /* index 58  0.250566   */
        {0x00600000 * 0x0015, {0x15}},    /* index 59  0.130276   */
        {0x00600000 * 0x0014, {0x14}},    /* index 60  -0.027313  */
        {0x00600000 * 0x0013, {0x13}},    /* index 61  -0.220617  */
        {0x00600000 * 0x0012, {0x12}},    /* index 62  -0.448124  */
        {0x00600000 * 0x0011, {0x11}},    /* index 63  0.291620   */
        {0x00600000 * 0x0010, {0x10}},    /* index 64  0.000000   : 6dB */
        {0x00600000 * 0x000F, {0x0f}},    /* index 65  -0.321652  */
        {0x00600000 * 0x000E, {0x0e}},    /* index 66  0.327935   */
        {0x00600000 * 0x000D, {0x0d}},    /* index 67  -0.050017  */
        {0x00600000 * 0x000C, {0x0c}},    /* index 68  -0.454343  */
        {0x00600000 * 0x000B, {0x0b}},    /* index 69  0.116077   */
        {0x00600000 * 0x000A, {0x0a}},    /* index 70  -0.337687  */
        {0x00600000 * 0x0009, {0x09}},    /* index 71  0.185390   */
        {0x00600000 * 0x0008, {0x08}},    /* index 72  -0.313708  */
        {0x00600000 * 0x0007, {0x07}},    /* index 73  0.165956   */
        {0x00600000 * 0x0006, {0x06}},    /* index 74  -0.374717  */
        {0x00600000 * 0x0005, {0x05}},    /* index 75  0.065138   */
        {0x00600000 * 0x0004, {0x04}},    /* index 76  0.486343   */
        {0x00600000 * 0x0003, {0x03}},    /* index 77  -0.110309  */
        {0x00600000 * 0x0002, {0x02}},    /* index 78  0.275938   */
        {0x00600000 * 0x0001, {0x01}},    /* index 79  -0.354190  */
        {0x00600000 * 0x0000, {0x00}},    /* index 80  0.000000   : 0dB */
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

