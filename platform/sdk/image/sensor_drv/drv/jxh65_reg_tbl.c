/*!
*****************************************************************************
** \file        jxh65_reg_tbl.c
**
** \version     $Id: jxh65_reg_tbl.c 10856 2016-11-01 08:24:06Z yulindeng $
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
GADI_VI_SensorDrvInfoT    jxh65_sensor =
{
    .magic_start        = GADI_SENSOR_HW_INFO_MAGIC_START,
    .version            = GADI_SENSOR_HW_INFO_VERSION,
    .length             = sizeof(GADI_VI_SensorDrvInfoT),
    .file_date          = 0x20170801,
    .HwInfo         =
    {
        .name               = "jxh65",
        .ctl_type           = GADI_SENSOR_CTL_I2C,
        .addr_len           = sizeof(GADI_U8),
        .data_len           = sizeof(GADI_U8),
        .reset_active_level = GADI_GPIO_HIGH,
        .hw_addr            = {(0x60>>1), 0, 0 ,0},
        .id_reg =
        {
            {0x0A, 0x0A},
            {0x0B, 0x65},
        },
        .reset_reg =
        {
            {0x12, 0x40},
            {GADI_VI_SENSOR_TABLE_FLAG_MSLEEP, 0x0A},
            {0x12, 0x00},
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
    .sensor_id          = GADI_ISP_SENSOR_JXH65,
    .sensor_double_step = 20,
    .typeofsensor       = 1,
    .sync_time_delay    = 40,
    .source_type.dummy  = GADI_VI_CMOS_CHANNEL_TYPE_AUTO,
    .dev_type           = GADI_VI_DEV_TYPE_CMOS,
    .video_system       = GADI_VIDEO_SYSTEM_AUTO,
    .vs_polarity        = GADI_VI_FALLING_EDGE,
    .hs_polarity        = GADI_VI_RISING_EDGE,
    .data_edge          = GADI_VI_FALLING_EDGE,
    .emb_sync_switch    = GADI_VI_EMB_SYNC_OFF,
    .emb_sync_loc       = GADI_VI_EMB_SYNC_LOWER_PEL,
    .emb_sync_mode      = GADI_VI_EMB_SYNC_ITU_656,
    .max_width          = 1280,
    .max_height         = 960,
    .def_sh_time        = GADI_VIDEO_FPS(50),
    .fmt_num            = 3,
    .auto_fmt           = GADI_VIDEO_MODE(1280,  960,  25, 1),
    .init_reg           =
    {
        {0x12,0x40},
        {0x0E,0x11},
        {0x0F,0x04},
        {0x10,0x20},
        {0x11,0x80},
        {0x5F,0x01},
        {0x60,0x0F},
        {0x19,0x64},
        {0x48,0x25},
        {0x2A,0x48},
        {0x2B,0x25},
        {0x2C,0x00},
        {0x2D,0x00},
        {0x2E,0xF9},
        {0x2F,0x40},
        {0x41,0x90},
        {0x42,0x12},
        {0x39,0x90},
        {0x1D,0xFF},
        {0x1E,0x1F},
        {0x6C,0x80},
        {0x1F,0x00},
        {0x31,0x0C},
        {0x32,0x20},
        {0x33,0x0C},
        {0x34,0x4F},
        {0x36,0x06},
        {0x38,0x39},
        {0x3A,0x08},
        {0x3B,0x50},
        {0x3C,0xA0},
        {0x3D,0x00},
        {0x3E,0x01},
        {0x3F,0x00},
        {0x40,0x00},
        {0x0D,0x50},
        {0x5A,0x43},
        {0x5B,0xB3},
        {0x5C,0x0C},
        {0x5D,0x7E},
        {0x5E,0x24},
        {0x62,0x40},
        {0x67,0x48},
        {0x6A,0x11},
        {0x68,0x04},
        {0x8F,0x9F},
        {0x0C,0x00},
        {0x59,0x97},
        {0x4A,0x05},
        {0x50,0x03},
        {0x47,0x62},
        {0x7E,0xCD},
        {0x8D,0x87},
        {0x49,0x10},
        {0x7F,0x52},
        {0x8E,0x00},
        {0x8C,0xFF},
        {0x8B,0x01},
        {0x57,0x02},
        {0x94,0x00},
        {0x95,0x00},
        {0x63,0x80},
        {0x7B,0x46},
        {0x7C,0x2D},
        {0x90,0x00},
        {0x79,0x00},
        {0x13,0x81},
        {0x12,0x00},
        {0x45,0x89},
        {0x93,0x68},

        {0x45,0x19},
        {0x1F,0x01},

        {GADI_VI_SENSOR_TABLE_FLAG_END, 0x00},
    },
    .pll_table =
    {
        [0] =
        {
            // for 960P@30, 960P@25
            .pixclk = 43200000,
            .extclk = 27000000,
            .regs = {

                {0x0E, 0x11},    // PLLclk = (Input clock(extclk)*PLL3[7:0]/PLL_Pre_Ratio)/(1+PLL2[1:0])
                {0x0F, 0x04},    //        = (27000000*32/2)/(1+0)/10=43200000
                {0x10, 0x20},    // PLL_Pre_Ratio = (1 + PLL[1:0]) = 2
                {0x11, 0x80},    // Mipiclk = VCO/(1+PLL2[7:4])  PLLclk = VCO/(1+PLL2[1:0]) sysdiv2[2] 0:1->1/8 1->1/10
                                 // VCO = Input clock(extclk)*PLL3[7:0]/PLL_Pre_Ratio
                                 // system clock use PLLclk directly
                {GADI_VI_SENSOR_TABLE_FLAG_END,}
            }
        },
    },
    .video_fmt_table =
    {
        [0] = // 1280x960@30fps 1440 * 1000 * 30 = 43200000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  960,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x12, 0x40, 0x40},
                {0x20,0xD0},
                {0x21,0x02},
                {0x22,0xE8},
                {0x23,0x03},
                {0x24,0x80},
                {0x25,0xC0},
                {0x26,0x32},
                {0x27,0x5C},
                {0x28,0x1C},
                {0x29,0x01},
                {0x12, 0x00, 0x40},
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 1280,
                .height     = 960,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_DVP, 
                .bits       = GADI_VIDEO_BITS_10,
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
                .def_start_x    = 0,
                .def_start_y    = 0,
                .def_width      = 1280,
                .def_height     = 960,
                .sync_start     = 0,
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_RG,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_RG,
                    GADI_VI_BAYER_PATTERN_RG,
                    GADI_VI_BAYER_PATTERN_RG,
                    GADI_VI_BAYER_PATTERN_RG,
                },
            },
            .update_after_vsync_start   = 3,
            .update_after_vsync_end     = 10,
        },
        [1] = // 1280x960@25fps 1440 * 1200 * 25 = 43200000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  960,  25, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x12, 0x40, 0x40},
                {0x20,0xD0},
                {0x21,0x02},
                {0x22,0xB0},
                {0x23,0x04},
                {0x24,0x80},
                {0x25,0xC0},
                {0x26,0x32},
                {0x27,0x5C},
                {0x28,0x1C},
                {0x29,0x01},
                {0x12, 0x00, 0x40},
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 1280,
                .height     = 960,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_DVP, 
                .bits       = GADI_VIDEO_BITS_10,
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
                .def_start_x    = 0,
                .def_start_y    = 0,
                .def_width      = 1280,
                .def_height     = 960,
                .sync_start     = 0,
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_RG,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_RG,
                    GADI_VI_BAYER_PATTERN_RG,
                    GADI_VI_BAYER_PATTERN_RG,
                    GADI_VI_BAYER_PATTERN_RG,
                },
            },
            .update_after_vsync_start   = 3,
            .update_after_vsync_end     = 10,
        },
        [2] = // 1280x720@25fps  1440*1000*25=43200000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  25, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x12, 0x40, 0x40},
                //{0x20,0x60},
                //{0x21,0x03},
                //{0x22,0xE8},
                //{0x23,0x03},
                {0x20,0xD0},
                {0x21,0x02},
                {0x22,0xB0},
                {0x23,0x04},
                {0x24,0x80},
                {0x25,0xD0},
                {0x26,0x22},
                {0x27,0x54},
                {0x28,0x93},
                {0x29,0x01},
                {0x12, 0x00, 0x40},
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
                .def_height     = 720,
                .sync_start     = 0,
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_RG,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_RG,
                    GADI_VI_BAYER_PATTERN_RG,
                    GADI_VI_BAYER_PATTERN_RG,
                    GADI_VI_BAYER_PATTERN_RG,
                },
            },
            .update_after_vsync_start   = 3,
            .update_after_vsync_end     = 10,
        },
    },
#define jxh65_V_FLIP    (1<<4)
#define jxh65_H_MIRROR  (1<<5)
    .mirror_table =
    {
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY_VERTICALLY] =
        {
            {0x12, jxh65_H_MIRROR | jxh65_V_FLIP, jxh65_H_MIRROR | jxh65_V_FLIP},
            {0x39, 0x90 },
            {0x27, 0x54 },
            {0x28, 0x1c },
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY] =
        {
            {0x12, jxh65_H_MIRROR, jxh65_H_MIRROR | jxh65_V_FLIP},
            {0x39, 0x90 },
            {0x27, 0x54 },
            {0x28, 0x1d },
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_VERTICALLY] =
        {
            {0x12, jxh65_V_FLIP, jxh65_H_MIRROR | jxh65_V_FLIP},
            {0x39, 0x80 },
            {0x27, 0x5C },
            {0x28, 0x1c },
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_NONE] =
        {
            {0x12, 0x00, jxh65_H_MIRROR | jxh65_V_FLIP},
            {0x39, 0x80 },
            {0x27, 0x5C },
            {0x28, 0x1d },
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
    },
    .version_reg =
    {
        {0x09, 0x01},
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .reg_ctl_by_ver_num = 0,
    .control_by_version_reg =
    {
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },

    .vmax_clk_ref   = GADI_SENSOR_CAL_CLK_PIXCLK_DIV2,
    .shs_clk_ref    = GADI_SENSOR_CAL_CLK_PIXCLK_DIV2,
    .cal_shs_mode   = GADI_SENSOR_CAL_SHS_VMAX_MODE,
    .shs_fix        = 0,
    .shs_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_SET,
    .agc_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_VSYNC,
    .vmax_reg_update_mode       = GADI_SENSOR_UPDATE_REG_BY_SET,
    .mirror_reg_update_mode     = GADI_SENSOR_UPDATE_REG_BY_VSYNC,
    .shs_additional_use         = 0,
    .shs_additional_percent     = 0,
    // hmax = (FRAMEW_H & 0x3F) << 8 +
    //        (FRAMEW_L & 0xFF) << 0
    .hmax_reg =
    {
        {0x21, 0x00, 0x3F, 0x00, 0x08, 0}, // FRAMEW_H
        {0x20, 0x00, 0xFF, 0x00, 0x00, 0}, // FRAMEW_L
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    // vmax = (FRAMEH_H & 0xFF) << 8 +
    //        (FRAMEH_L & 0xFF) << 0
    .vmax_reg =
    {
        {0x23, 0x00, 0xFF, 0x00, 0x08, 0}, // FRAMEH_H
        {0x22, 0x00, 0xFF, 0x00, 0x00, 0}, // FRAMEH_L
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .vmax_wadd_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    // shs = (EXP_H & 0xFF) << 8 +
    //       (EXP_L & 0xFF) << 0
    .shs_reg =
    {
        {0x02, 0x00, 0xFF, 0x00, 0x08, 0}, // EXP_H
        {0x01, 0x00, 0xFF, 0x00, 0x00, 0}, // EXP_L
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .shs_additional_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    .cal_gain_mode              = GADI_SENSOR_CAL_GAIN_MODE1,
    .max_agc_index = 128,
    .max_again_index = 0,
    .cur_again_index = 0,
    .agc_info =
    {
        #if 0  //new: again<24db
        .db_max     = 0x23D90BA2,   //35.84db
        #else  //old: dgain=1x
        .db_max     = 0x23D90BA2,   //35.xdb     //bug bigger than 42db.
        #endif
        .db_min     = 0x01000000,   // 1dB
        .db_step    = 0x00100000,   // 0.0625dB
    },
    .gain_reg =
    {
        {0x00, 0x00},
        {0x0D, 0x00},
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .gain_table =
    {
        /*REG_PGA addr 0x00;  bit[6:0];  again = (2^PGA[6:4])*(1+PGA[3:0]/16)*/
        /*REG_DVP2 addr 0x0d; bit[1:0];  dgain: bit[1:0]=00, no digital gain;  bit[1:0]=01 or 10, 2x;  bit[1:0]=11, 4x;*/
        /*Total gain = dgain*(2^PGA[6:4])*(1+PGA[3:0]/16)  */
        /*PGA   DVP2     agc_db    */ //   db
        #if 0  //again<24db
        {0x00000000, {0x00, 0x50,}}, //0
        {0x0086CDDD, {0x01, 0x50,}}, //0.526578774
        {0x0105E6A2, {0x02, 0x50,}}, //1.023050449
        {0x017E1FC6, {0x03, 0x50,}}, //1.492672366
        {0x01F02DE4, {0x04, 0x50,}}, //1.93820026
        {0x025CAB21, {0x05, 0x50,}}, //2.361986242
        {0x02C41C1C, {0x06, 0x50,}}, //2.766053963
        {0x0326F3C3, {0x07, 0x50,}}, //3.152157067
        {0x03859655, {0x08, 0x50,}}, //3.521825181
        {0x03E05BC8, {0x09, 0x50,}}, //3.87640052
        {0x043791B9, {0x0A, 0x50,}}, //4.217067306
        {0x048B7CF8, {0x0B, 0x50,}}, //4.54487563
        {0x04DC5AD4, {0x0C, 0x50,}}, //4.860760974
        {0x052A6228, {0x0D, 0x50,}}, //5.165560305
        {0x0575C43A, {0x0E, 0x50,}}, //5.460025441
        {0x05BEAD74, {0x0F, 0x50,}}, //5.744834224
        {0x06054609, {0x10, 0x50,}}, //6.020599913
        {0x068C13E7, {0x11, 0x50,}}, //6.547178688
        {0x070B2CAB, {0x12, 0x50,}}, //7.043650362
        {0x078365CF, {0x13, 0x50,}}, //7.513272279
        {0x07F573ED, {0x14, 0x50,}}, //7.958800173
        {0x0861F12A, {0x15, 0x50,}}, //8.382586155
        {0x08C96226, {0x16, 0x50,}}, //8.786653877
        {0x092C39CD, {0x17, 0x50,}}, //9.172756981
        {0x098ADC5E, {0x18, 0x50,}}, //9.542425094
        {0x09E5A1D2, {0x19, 0x50,}}, //9.897000434
        {0x0A3CD7C2, {0x1A, 0x50,}}, //10.23766722
        {0x0A90C301, {0x1B, 0x50,}}, //10.56547554
        {0x0AE1A0DD, {0x1C, 0x50,}}, //10.88136089
        {0x0B2FA832, {0x1D, 0x50,}}, //11.18616022
        {0x0B7B0A43, {0x1E, 0x50,}}, //11.48062535
        {0x0BC3F37D, {0x1F, 0x50,}}, //11.76543414
        {0x0C0A8C12, {0x20, 0x50,}}, //12.04119983
        {0x0C9159F0, {0x21, 0x50,}}, //12.5677786
        {0x0D1072B4, {0x22, 0x50,}}, //13.06425028
        {0x0D88ABD9, {0x23, 0x50,}}, //13.53387219
        {0x0DFAB9F6, {0x24, 0x50,}}, //13.97940009
        {0x0E673733, {0x25, 0x50,}}, //14.40318607
        {0x0ECEA82F, {0x26, 0x50,}}, //14.80725379
        {0x0F317FD6, {0x27, 0x50,}}, //15.19335689
        {0x0F902268, {0x28, 0x50,}}, //15.56302501
        {0x0FEAE7DB, {0x29, 0x50,}}, //15.91760035
        {0x10421DCB, {0x2A, 0x50,}}, //16.25826713
        {0x1096090A, {0x2B, 0x50,}}, //16.58607546
        {0x10E6E6E7, {0x2C, 0x50,}}, //16.9019608
        {0x1134EE3B, {0x2D, 0x50,}}, //17.20676013
        {0x1180504C, {0x2E, 0x50,}}, //17.50122527
        {0x11C93987, {0x2F, 0x50,}}, //17.78603405
        {0x120FD21B, {0x30, 0x50,}}, //18.06179974
        {0x12969FF9, {0x31, 0x50,}}, //18.58837851
        {0x1315B8BD, {0x32, 0x50,}}, //19.08485019
        {0x138DF1E2, {0x33, 0x50,}}, //19.55447211
        {0x14000000, {0x34, 0x50,}}, //20
        {0x146C7D3C, {0x35, 0x50,}}, //20.42378598
        {0x14D3EE38, {0x36, 0x50,}}, //20.8278537
        {0x1536C5DF, {0x37, 0x50,}}, //21.21395681
        {0x15956871, {0x38, 0x50,}}, //21.58362492
        {0x15F02DE4, {0x39, 0x50,}}, //21.93820026
        {0x164763D4, {0x3A, 0x50,}}, //22.27886705
        {0x169B4F13, {0x3B, 0x50,}}, //22.60667537
        {0x16EC2CF0, {0x3C, 0x50,}}, //22.92256071
        {0x173A3444, {0x3D, 0x50,}}, //23.22736004
        {0x17859655, {0x3E, 0x50,}}, //23.52182518
        {0x17CE7F90, {0x3F, 0x50,}}, //23.80663396
        {0x18151824, {0x30, 0x51,}}, //24.08239965
        {0x189BE602, {0x31, 0x51,}}, //24.60897843
        {0x191AFEC7, {0x32, 0x51,}}, //25.1054501
        {0x199337EB, {0x33, 0x51,}}, //25.57507202
        {0x1A054609, {0x34, 0x51,}}, //26.02059991
        {0x1A71C346, {0x35, 0x51,}}, //26.44438589
        {0x1AD93441, {0x36, 0x51,}}, //26.84845362
        {0x1B3C0BE8, {0x37, 0x51,}}, //27.23455672
        {0x1B9AAE7A, {0x38, 0x51,}}, //27.60422483
        {0x1BF573ED, {0x39, 0x51,}}, //27.95880017
        {0x1C4CA9DD, {0x3A, 0x51,}}, //28.29946696
        {0x1CA0951C, {0x3B, 0x51,}}, //28.62727528
        {0x1CF172F9, {0x3C, 0x51,}}, //28.94316063
        {0x1D3F7A4D, {0x3D, 0x51,}}, //29.24795996
        {0x1D8ADC5E, {0x3E, 0x51,}}, //29.54242509
        {0x1DD3C599, {0x3F, 0x51,}}, //29.82723388
        {0x1E1A5E2D, {0x30, 0x53,}}, //30.10299957
        {0x1EA12C0B, {0x31, 0x53,}}, //30.62957834
        {0x1F2044D0, {0x32, 0x53,}}, //31.12605002
        {0x1F987DF4, {0x33, 0x53,}}, //31.59567193
        {0x200A8C12, {0x34, 0x53,}}, //32.04119983
        {0x2077094F, {0x35, 0x53,}}, //32.46498581
        {0x20DE7A4A, {0x36, 0x53,}}, //32.86905353
        {0x214151F1, {0x37, 0x53,}}, //33.25515663
        {0x219FF483, {0x38, 0x53,}}, //33.62482475
        {0x21FAB9F6, {0x39, 0x53,}}, //33.97940009
        {0x2251EFE7, {0x3A, 0x53,}}, //34.32006687
        {0x22A5DB26, {0x3B, 0x53,}}, //34.6478752
        {0x22F6B902, {0x3C, 0x53,}}, //34.96376054
        {0x2344C056, {0x3D, 0x53,}}, //35.26855987
        {0x23902268, {0x3E, 0x53,}}, //35.56302501
        {0x23D90BA2, {0x3F, 0x53,}}, //35.84783379
        #else  //old: dgain=1
        {0x00000000, {0x00, 0x50,}}, //0
        {0x0086CDDD, {0x01, 0x50,}}, //0.526578774
        {0x0105E6A2, {0x02, 0x50,}}, //1.023050449
        {0x017E1FC6, {0x03, 0x50,}}, //1.492672366
        {0x01F02DE4, {0x04, 0x50,}}, //1.93820026
        {0x025CAB21, {0x05, 0x50,}}, //2.361986242
        {0x02C41C1C, {0x06, 0x50,}}, //2.766053963
        {0x0326F3C3, {0x07, 0x50,}}, //3.152157067
        {0x03859655, {0x08, 0x50,}}, //3.521825181
        {0x03E05BC8, {0x09, 0x50,}}, //3.87640052
        {0x043791B9, {0x0A, 0x50,}}, //4.217067306
        {0x048B7CF8, {0x0B, 0x50,}}, //4.54487563
        {0x04DC5AD4, {0x0C, 0x50,}}, //4.860760974
        {0x052A6228, {0x0D, 0x50,}}, //5.165560305
        {0x0575C43A, {0x0E, 0x50,}}, //5.460025441
        {0x05BEAD74, {0x0F, 0x50,}}, //5.744834224
        {0x06054609, {0x10, 0x50,}}, //6.020599913
        {0x068C13E7, {0x11, 0x50,}}, //6.547178688
        {0x070B2CAB, {0x12, 0x50,}}, //7.043650362
        {0x078365CF, {0x13, 0x50,}}, //7.513272279
        {0x07F573ED, {0x14, 0x50,}}, //7.958800173
        {0x0861F12A, {0x15, 0x50,}}, //8.382586155
        {0x08C96226, {0x16, 0x50,}}, //8.786653877
        {0x092C39CD, {0x17, 0x50,}}, //9.172756981
        {0x098ADC5E, {0x18, 0x50,}}, //9.542425094
        {0x09E5A1D2, {0x19, 0x50,}}, //9.897000434
        {0x0A3CD7C2, {0x1A, 0x50,}}, //10.23766722
        {0x0A90C301, {0x1B, 0x50,}}, //10.56547554
        {0x0AE1A0DD, {0x1C, 0x50,}}, //10.88136089
        {0x0B2FA832, {0x1D, 0x50,}}, //11.18616022
        {0x0B7B0A43, {0x1E, 0x50,}}, //11.48062535
        {0x0BC3F37D, {0x1F, 0x50,}}, //11.76543414
        {0x0C0A8C12, {0x20, 0x50,}}, //12.04119983
        {0x0C9159F0, {0x21, 0x50,}}, //12.5677786
        {0x0D1072B4, {0x22, 0x50,}}, //13.06425028
        {0x0D88ABD9, {0x23, 0x50,}}, //13.53387219
        {0x0DFAB9F6, {0x24, 0x50,}}, //13.97940009
        {0x0E673733, {0x25, 0x50,}}, //14.40318607
        {0x0ECEA82F, {0x26, 0x50,}}, //14.80725379
        {0x0F317FD6, {0x27, 0x50,}}, //15.19335689
        {0x0F902268, {0x28, 0x50,}}, //15.56302501
        {0x0FEAE7DB, {0x29, 0x50,}}, //15.91760035
        {0x10421DCB, {0x2A, 0x50,}}, //16.25826713
        {0x1096090A, {0x2B, 0x50,}}, //16.58607546
        {0x10E6E6E7, {0x2C, 0x50,}}, //16.9019608
        {0x1134EE3B, {0x2D, 0x50,}}, //17.20676013
        {0x1180504C, {0x2E, 0x50,}}, //17.50122527
        {0x11C93987, {0x2F, 0x50,}}, //17.78603405
        {0x120FD21B, {0x30, 0x50,}}, //18.06179974
        {0x12969FF9, {0x31, 0x50,}}, //18.58837851
        {0x1315B8BD, {0x32, 0x50,}}, //19.08485019
        {0x138DF1E2, {0x33, 0x50,}}, //19.55447211
        {0x14000000, {0x34, 0x50,}}, //20
        {0x146C7D3C, {0x35, 0x50,}}, //20.42378598
        {0x14D3EE38, {0x36, 0x50,}}, //20.8278537
        {0x1536C5DF, {0x37, 0x50,}}, //21.21395681
        {0x15956871, {0x38, 0x50,}}, //21.58362492
        {0x15F02DE4, {0x39, 0x50,}}, //21.93820026
        {0x164763D4, {0x3A, 0x50,}}, //22.27886705
        {0x169B4F13, {0x3B, 0x50,}}, //22.60667537
        {0x16EC2CF0, {0x3C, 0x50,}}, //22.92256071
        {0x173A3444, {0x3D, 0x50,}}, //23.22736004
        {0x17859655, {0x3E, 0x50,}}, //23.52182518
        {0x17CE7F90, {0x3F, 0x50,}}, //23.80663396
        {0x18151824, {0x40, 0x50,}}, //24.08239965
        {0x189BE602, {0x41, 0x50,}}, //24.60897843
        {0x191AFEC7, {0x42, 0x50,}}, //25.1054501
        {0x199337EB, {0x43, 0x50,}}, //25.57507202
        {0x1A054609, {0x44, 0x50,}}, //26.02059991
        {0x1A71C346, {0x45, 0x50,}}, //26.44438589
        {0x1AD93441, {0x46, 0x50,}}, //26.84845362
        {0x1B3C0BE8, {0x47, 0x50,}}, //27.23455672
        {0x1B9AAE7A, {0x48, 0x50,}}, //27.60422483
        {0x1BF573ED, {0x49, 0x50,}}, //27.95880017
        {0x1C4CA9DD, {0x4A, 0x50,}}, //28.29946696
        {0x1CA0951C, {0x4B, 0x50,}}, //28.62727528
        {0x1CF172F9, {0x4C, 0x50,}}, //28.94316063
        {0x1D3F7A4D, {0x4D, 0x50,}}, //29.24795996
        {0x1D8ADC5E, {0x4E, 0x50,}}, //29.54242509
        {0x1DD3C599, {0x4F, 0x50,}}, //29.82723388
        {0x1E1A5E2D, {0x50, 0x50,}}, //30.10299957
        {0x1EA12C0B, {0x51, 0x50,}}, //30.62957834
        {0x1F2044D0, {0x52, 0x50,}}, //31.12605002
        {0x1F987DF4, {0x53, 0x50,}}, //31.59567193
        {0x200A8C12, {0x54, 0x50,}}, //32.04119983
        {0x2077094F, {0x55, 0x50,}}, //32.46498581
        {0x20DE7A4A, {0x56, 0x50,}}, //32.86905353
        {0x214151F1, {0x57, 0x50,}}, //33.25515663
        {0x219FF483, {0x58, 0x50,}}, //33.62482475
        {0x21FAB9F6, {0x59, 0x50,}}, //33.97940009
        {0x2251EFE7, {0x5A, 0x50,}}, //34.32006687
        {0x22A5DB26, {0x5B, 0x50,}}, //34.6478752
        {0x22F6B902, {0x5C, 0x50,}}, //34.96376054
        {0x2344C056, {0x5D, 0x50,}}, //35.26855987
        {0x23902268, {0x5E, 0x50,}}, //35.56302501
        {0x23D90BA2, {0x5F, 0x50,}}, //35.84783379
        {0x241FA437, {0x60, 0x50,}}, //36.12359948
        {0x24A67215, {0x61, 0x50,}}, //36.65017825
        {0x25258AD9, {0x62, 0x50,}}, //37.14664993
        {0x259DC3FD, {0x63, 0x50,}}, //37.61627185
        {0x260FD21B, {0x64, 0x50,}}, //38.06179974
        {0x267C4F58, {0x65, 0x50,}}, //38.48558572
        {0x26E3C053, {0x66, 0x50,}}, //38.88965344
        {0x274697FB, {0x67, 0x50,}}, //39.27575655
        {0x27A53A8C, {0x68, 0x50,}}, //39.64542466
        {0x28000000, {0x69, 0x50,}}, //40
        {0x285735F0, {0x6A, 0x50,}}, //40.34066679
        {0x28AB212F, {0x6B, 0x50,}}, //40.66847511
        {0x28FBFF0B, {0x6C, 0x50,}}, //40.98436045
        {0x294A0660, {0x6D, 0x50,}}, //41.28915978
        {0x29956871, {0x6E, 0x50,}}, //41.58362492
        {0x29DE51AB, {0x6F, 0x50,}}, //41.8684337
        {0x2A24EA40, {0x70, 0x50,}}, //42.14419939
        {0x2AABB81E, {0x71, 0x50,}}, //42.67077817
        {0x2B2AD0E2, {0x72, 0x50,}}, //43.16724984
        {0x2BA30A07, {0x73, 0x50,}}, //43.63687176
        {0x2C151824, {0x74, 0x50,}}, //44.08239965
        {0x2C819561, {0x75, 0x50,}}, //44.50618563
        {0x2CE9065D, {0x76, 0x50,}}, //44.91025336
        {0x2D4BDE04, {0x77, 0x50,}}, //45.29635646
        {0x2DAA8096, {0x78, 0x50,}}, //45.66602457
        {0x2E054609, {0x79, 0x50,}}, //46.02059991
        {0x2E5C7BF9, {0x7A, 0x50,}}, //46.3612667
        {0x2EB06738, {0x7B, 0x50,}}, //46.68907502
        {0x2F014515, {0x7C, 0x50,}}, //47.00496037
        {0x2F4F4C69, {0x7D, 0x50,}}, //47.3097597
        {0x2F9AAE7A, {0x7E, 0x50,}}, //47.60422483
        {0x2FE397B5, {0x7F, 0x50,}}, //47.88903362
		#endif

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

