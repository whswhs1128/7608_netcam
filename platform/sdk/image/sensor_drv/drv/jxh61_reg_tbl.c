/*!
*****************************************************************************
** \file        jxh61_reg_tbl.c
**
** \version     $Id: jxh61_reg_tbl.c 11627 2017-03-10 10:58:12Z dengbiao $
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
GADI_VI_SensorDrvInfoT    jxh61_sensor =
{
    .magic_start        = GADI_SENSOR_HW_INFO_MAGIC_START,
    .version            = GADI_SENSOR_HW_INFO_VERSION,
    .length             = sizeof(GADI_VI_SensorDrvInfoT),
    .file_date          = 0x20161014,
    .HwInfo         =
    {
        .name               = "jxh61",
        .ctl_type           = GADI_SENSOR_CTL_I2C,
        .addr_len           = sizeof(GADI_U8),
        .data_len           = sizeof(GADI_U8),
        .reset_active_level = GADI_GPIO_LOW,
        .hw_addr            = {(0x60>>1), (0x64>>1), (0x68>>1), (0x6C>>1)},
        .id_reg =
        {
            {0x0A, 0x01},
            {0x0B, 0x42},
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
    .sensor_id          = GADI_ISP_SENSOR_JXH61,
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
    .max_width          = 1600,
    .max_height         = 900,
    .def_sh_time        = GADI_VIDEO_FPS(30),
    .fmt_num            = 2,
    .auto_fmt           = GADI_VIDEO_MODE(1280,  720,  25, 1),
    .init_reg           =
    {
        //soft_reset
        //{JXH61_SYS,                0x40},
        {0x0D,      0x40},
        {0x1F,      0x04},
        //{JXH61_PLL1,                0x1D},
        //{JXH61_PLL2,                0x08},
        //{JXH61_PLL3,                0x18},
        //{JXH61_CLK,                0x80},
        {0x20,      0xBC},
        {0x21,      0x07},
        {0x22,      0xEE},
        {0x23,      0x02},
        {0x24,      0x00},
        {0x25,      0xD0},
        {0x26,      0x25},
        {0x27,      0x3B},
        {0x28,      0x0D},
        {0x29,      0x01},
        {0x2A,      0x24},
        {0x2B,      0x29},
        {0x2C,      0x04},
        {0x2D,      0x00},
        {0x2E,      0xB9},
        {0x2F,      0x00},
        {0x30,      0x92},
        {0x31,      0x0A},
        {0x32,      0xAA},
        {0x33,      0x14},
        {0x34,      0x38},
        {0x35,      0x54},
        {0x42,      0x41},
        {0x43,      0x18},
        {0x1D,      0xFF},
        {0x1E,      0x1F},
        {0x6C,      0x90},
        {0x73,      0xB3},
        {0x70,      0x68},
        {0x76,      0x40},
        {0x77,      0x06},
        {0x72,      0x48},
        {0x6D,      0xA2},
        {0x48,      0x40},
        {0x60,      0xA4},
        {0x61,      0xFF},
        {0x62,      0x40},
        {0x65,      0x00},
        {0x67,      0x30},
        {0x68,      0x04},
        {0x69,      0x74},
        {0x6F,      0x24},
        {0x63,      0x59},
        {0x6A,      0x09},
        {0x13,      0x87},
        {0x14,      0x80},
        {0x16,      0xC0},
        {0x17,      0x40},
        {0x38,      0x35},
        {0x39,      0x98},
        {0x4A,      0x03},
        {0x49,      0x10},
        {0x66,      0x20},
        //{JXH61_SYS,                0x00},
        {GADI_VI_SENSOR_TABLE_FLAG_END, 0x00}//end flag
    },
    .pll_table =
    {
        [0] = {//for 25fps,30fps
            .pixclk = 36000000,
            .extclk = 27000000,
            .regs = {                   // PLLclk = (Input clock(extclk)*PLL3[7:0]/PLL_Pre_Ratio)/(1+PLL2[3:0])
                                        //        = (27000000*24/2)/(1+8)=36000000
                {0x0E, 0x1D},     // JXH61_PLL1 PLL_Pre_Ratio = (1 + PLL[1:0]) = 2
                {0x0F, 0x08},     // JXH61_PLL2 Mipiclk = VCO/(1+PLL2[7:4])  PLLclk = VCO/(1+PLL2[3:0])
                {0x10, 0x18},     // JXH61_PLL3 VCO = Input clock(extclk)*PLL3[7:0]/PLL_Pre_Ratio
                {0x11, 0x80},     // JXH61_CLK system clock use PLLclk directly
                {GADI_VI_SENSOR_TABLE_FLAG_END,}
            }
        },
        [1] = {//for 10fps,15fps,20fps
            .pixclk = 18000000,
            .extclk = 27000000,
            .regs = {                   // PLLclk = (Input clock(extclk)*PLL3[7:0]/PLL_Pre_Ratio)/(1+PLL2[3:0])
                                        //        = (27000000*24/2)/(1+8)=36000000
                {0x0E, 0x1D},     // JXH61_PLL1 PLL_Pre_Ratio = (1 + PLL[1:0]) = 2
                {0x0F, 0x08},     // JXH61_PLL2 Mipiclk = VCO/(1+PLL2[7:4])  PLLclk = VCO/(1+PLL2[3:0])
                {0x10, 0x18},     // JXH61_PLL3 VCO = Input clock(extclk)*PLL3[7:0]/PLL_Pre_Ratio
                {0x11, 0x01},     // JXH61_CLK When CLK[5:0] > 0: System clock = PLLclk *(1+CLK[6])/(2*CLK[5:0])
                {GADI_VI_SENSOR_TABLE_FLAG_END,}
            }
        },
        [2] = {//0728 hhl add for new test
            .pixclk = 37125000,
            .extclk = 27000000,
            .regs = {                   // PLLclk = (Input clock(extclk)*PLL3[7:0]/PLL_Pre_Ratio)/(1+PLL2[3:0])
                                        //        = (27000000*22/2)/(1+7)=37125000
                {0x0E, 0x1D},     // PLL_Pre_Ratio = (1 + PLL[1:0]) = 2
                {0x0F, 0x07},     // Mipiclk = VCO/(1+PLL2[7:4])  PLLclk = VCO/(1+PLL2[3:0])
                {0x10, 0x16},     // VCO = Input clock(extclk)*PLL3[7:0]/PLL_Pre_Ratio
                {0x11,  0x80},     // system clock use PLLclk directly
                {GADI_VI_SENSOR_TABLE_FLAG_END,}
            }
        },
    },
    .video_fmt_table =
    {
        [0] =
        {    //1280x720@30fps
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x20, 0x72},   // LINE_LENGTH_L=JXH61_FRAMEW_L
                {0x21, 0x06},   // LINE_LENGTH_H=JXH61_FRAMEW_H   // 0x640=1600 must >1440
                {0x22, 0xEE},   // FRM_LENGTH_L=JXH61_FRAMEH_L
                {0x23, 0x02},   // FRM_LENGTH_H=JXH61_FRAMEH_H    // 0x2EE=750    1600*750*30=36000000
                {0x27, 0x3B},   // X_ADD_STA_L=JXH61_HWINST       // must >1600-1296=0x130 >0x13B < 1600-1280=0x140<0x14B
                {0x28, 0x0D},   // Y_ADD_STA_L=JXH61_VWINST       // 750-720=0x1E
                {0x29, 0x01},   // Y/X_ADD_STA_H=JXH61_HVWINST
                {0x24, 0x00},   // X_OUT_SIZE_L=JXH61_HWIN        // 0x500=1280
                {0x25, 0xD0},   // X_OUT_SIZE_H=JXH61_VWIN        // 0x2D0=720
                {0x26, 0x25},   // Y/X_OUT_SIZE_L=JXH61_HVWIN
                {0x18, 0xBB},   //
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
                    GADI_VI_BAYER_PATTERN_RG,
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GR,
                    GADI_VI_BAYER_PATTERN_BG,
                },
            },
            .update_after_vsync_start   = 3,
            .update_after_vsync_end     = 10,
        },
        [1] =
        {    //1280x720@25fps
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  25, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x20, 0xBC},   // LINE_LENGTH_L=JXH61_FRAMEW_L
                {0x21, 0x07},   // LINE_LENGTH_H=JXH61_FRAMEW_H   // 0x640=1600 must >1440
                {0x22, 0xEE},   // FRM_LENGTH_L=JXH61_FRAMEH_L
                {0x23, 0x02},   // FRM_LENGTH_H=JXH61_FRAMEH_H    // 0x384=900    1600*900*25=36000000
                {0x27, 0x3B},   // X_ADD_STA_L=JXH61_HWINST       // must >1600-1296=0x130 >0x13B < 1600-1280=0x140<0x14B
                {0x28, 0x0D},   // Y_ADD_STA_L=JXH61_VWINST       // 900-720=0xB4
                {0x29, 0x01},   // Y/X_ADD_STA_H=JXH61_HVWINST
                {0x24, 0x00},   // X_OUT_SIZE_L=JXH61_HWIN        // 0x500=1280
                {0x25, 0xD0},   // X_OUT_SIZE_H=JXH61_VWIN        // 0x2D0=720
                {0x26, 0x25},   // Y/X_OUT_SIZE_L=JXH61_HVWIN
                {0x18, 0xBB},   //
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
                    GADI_VI_BAYER_PATTERN_RG,
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GR,
                    GADI_VI_BAYER_PATTERN_BG,
                },
            },
            .update_after_vsync_start   = 3,
            .update_after_vsync_end     = 10,
        },
    },
#define JXH61_V_FLIP    (1<<4)
#define JXH61_H_MIRROR  (1<<5)
    .mirror_table =
    {
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY_VERTICALLY] =
        {
            {0x12, JXH61_H_MIRROR | JXH61_V_FLIP, JXH61_H_MIRROR | JXH61_V_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY] =
        {
            {0x12, JXH61_H_MIRROR, JXH61_H_MIRROR | JXH61_V_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_VERTICALLY] =
        {
            {0x12, JXH61_V_FLIP, JXH61_H_MIRROR | JXH61_V_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_NONE] =
        {
            {0x12, 0x00, JXH61_H_MIRROR | JXH61_V_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
    },
    .version_reg =
    {
        {0x09, 0x00},
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .reg_ctl_by_ver_num = 3,
    .control_by_version_reg =
    {
        [0] =
        {
            .version = 0x00,
            .mask    = 0xFF,
            .regs =
            {
                {0x2C, 0x00, GADI_SENSOR_VER_CTL_REG_INIT},
                {0x48, 0x40, GADI_SENSOR_VER_CTL_REG_INIT},
                {0x63, 0x19, GADI_SENSOR_VER_CTL_REG_INIT},
                {0x27, 0x49, GADI_SENSOR_VER_CTL_REG_FMT},
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
        [1] =
        {
            .version = 0x80,
            .mask    = 0xFF,
            .regs =
            {
                {0x2C, 0x00, GADI_SENSOR_VER_CTL_REG_INIT},
                {0x48, 0x40, GADI_SENSOR_VER_CTL_REG_INIT},
                {0x63, 0x19, GADI_SENSOR_VER_CTL_REG_INIT},
                {0x27, 0x49, GADI_SENSOR_VER_CTL_REG_FMT},
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
        [2] =
        {
            .version = 0x81,
            .mask    = 0xFF,
            .regs =
            {
                {0x2C, 0x04, GADI_SENSOR_VER_CTL_REG_INIT},
                {0x48, 0x60, GADI_SENSOR_VER_CTL_REG_INIT},
                {0x63, 0x51, GADI_SENSOR_VER_CTL_REG_INIT},
                {0x27, 0x3B, GADI_SENSOR_VER_CTL_REG_FMT},
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
    },

    .vmax_clk_ref   = GADI_SENSOR_CAL_CLK_PIXCLK,
    .shs_clk_ref    = GADI_SENSOR_CAL_CLK_PIXCLK_DIV2,
    .cal_shs_mode   = GADI_SENSOR_CAL_SHS_VMAX_MODE,
    .shs_fix        = 0,
    .shs_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_SET,
    .agc_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_VSYNC,
    .vmax_reg_update_mode       = GADI_SENSOR_UPDATE_REG_BY_SET,
    .mirror_reg_update_mode     = GADI_SENSOR_UPDATE_REG_BY_SET,
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
    .max_agc_index = 80,
    .max_again_index = 0,
    .cur_again_index = 0,
    .agc_info =
    {
        .db_max     = 0x1db9a541,   // 15.5dB*4 max_db<<24
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
        {0x00000000, {0x00, 0x50,}}, //0.000000
        {0x008657c9, {0x01, 0x50,}}, //0.524777
        {0x0105013a, {0x02, 0x50,}}, //1.019550
        {0x017cd110, {0x03, 0x50,}}, //1.487565
        {0x01ee7b47, {0x04, 0x50,}}, //1.931569
        {0x025a997c, {0x05, 0x50,}}, //2.353905
        {0x02c1afdd, {0x06, 0x50,}}, //2.756590
        {0x032430f0, {0x07, 0x50,}}, //3.141372
        {0x0382809d, {0x08, 0x50,}}, //3.509775
        {0x03dcf68e, {0x09, 0x50,}}, //3.863137
        {0x0433e01a, {0x0a, 0x50,}}, //4.202638
        {0x048781d8, {0x0b, 0x50,}}, //4.529325
        {0x04d818df, {0x0c, 0x50,}}, //4.844130
        {0x0525dbda, {0x0d, 0x50,}}, //5.147886
        {0x0570fbe4, {0x0e, 0x50,}}, //5.441344
        {0x05b9a541, {0x0f, 0x50,}}, //5.725178
        {0x06000000, {0x10, 0x50,}}, //6.000000
        {0x068657c9, {0x11, 0x50,}}, //6.524777
        {0x0705013a, {0x12, 0x50,}}, //7.019550
        {0x077cd110, {0x13, 0x50,}}, //7.487565
        {0x07ee7b47, {0x14, 0x50,}}, //7.931569
        {0x085a997c, {0x15, 0x50,}}, //8.353905
        {0x08c1afdd, {0x16, 0x50,}}, //8.756590
        {0x092430f0, {0x17, 0x50,}}, //9.141372
        {0x0982809d, {0x18, 0x50,}}, //9.509775
        {0x09dcf68e, {0x19, 0x50,}}, //9.863137
        {0x0a33e01a, {0x1a, 0x50,}}, //10.202638
        {0x0a8781d8, {0x1b, 0x50,}}, //10.529325
        {0x0ad818df, {0x1c, 0x50,}}, //10.844130
        {0x0b25dbda, {0x1d, 0x50,}}, //11.147886
        {0x0b70fbe4, {0x1e, 0x50,}}, //11.441344
        {0x0bb9a541, {0x1f, 0x50,}}, //11.725178
        {0x0c000000, {0x20, 0x50,}}, //12.000000
        {0x0c8657c9, {0x21, 0x50,}}, //12.524777
        {0x0d05013a, {0x22, 0x50,}}, //13.019550
        {0x0d7cd110, {0x23, 0x50,}}, //13.487565
        {0x0dee7b47, {0x24, 0x50,}}, //13.931569
        {0x0e5a997c, {0x25, 0x50,}}, //14.353905
        {0x0ec1afdd, {0x26, 0x50,}}, //14.756590
        {0x0f2430f0, {0x27, 0x50,}}, //15.141372
        {0x0f82809d, {0x28, 0x50,}}, //15.509775
        {0x0fdcf68e, {0x29, 0x50,}}, //15.863137
        {0x1033e01a, {0x2a, 0x50,}}, //16.202638
        {0x108781d8, {0x2b, 0x50,}}, //16.529325
        {0x10d818df, {0x2c, 0x50,}}, //16.844130
        {0x1125dbda, {0x2d, 0x50,}}, //17.147886
        {0x1170fbe4, {0x2e, 0x50,}}, //17.441344
        {0x11b9a541, {0x2f, 0x50,}}, //17.725178
        {0x12000000, {0x30, 0x50,}}, //18.000000
        {0x128657c9, {0x31, 0x50,}}, //18.524777
        {0x1305013a, {0x32, 0x50,}}, //19.019550
        {0x137cd110, {0x33, 0x50,}}, //19.487565
        {0x13ee7b47, {0x34, 0x50,}}, //19.931569
        {0x145a997c, {0x35, 0x50,}}, //20.353905
        {0x14c1afdd, {0x36, 0x50,}}, //20.756590
        {0x152430f0, {0x37, 0x50,}}, //21.141372
        {0x1582809d, {0x38, 0x50,}}, //21.509775
        {0x15dcf68e, {0x39, 0x50,}}, //21.863137
        {0x1633e01a, {0x3a, 0x50,}}, //22.202638
        {0x168781d8, {0x3b, 0x50,}}, //22.529325
        {0x16d818df, {0x3c, 0x50,}}, //22.844130
        {0x1725dbda, {0x3d, 0x50,}}, //23.147886
        {0x1770fbe4, {0x3e, 0x50,}}, //23.441344
        {0x17b9a541, {0x3f, 0x50,}}, //23.725178
        {0x18000000, {0x30, 0x51,}}, //24.000000
        {0x188657c9, {0x31, 0x51,}}, //24.524777
        {0x1905013a, {0x32, 0x51,}}, //25.019550
        {0x197cd110, {0x33, 0x51,}}, //25.487565
        {0x19ee7b47, {0x34, 0x51,}}, //25.931569
        {0x1a5a997c, {0x35, 0x51,}}, //26.353905
        {0x1ac1afdd, {0x36, 0x51,}}, //26.756590
        {0x1b2430f0, {0x37, 0x51,}}, //27.141372
        {0x1b82809d, {0x38, 0x51,}}, //27.509775
        {0x1bdcf68e, {0x39, 0x51,}}, //27.863137
        {0x1c33e01a, {0x3a, 0x51,}}, //28.202638
        {0x1c8781d8, {0x3b, 0x51,}}, //28.529325
        {0x1cd818df, {0x3c, 0x51,}}, //28.844130
        {0x1d25dbda, {0x3d, 0x51,}}, //29.147886
        {0x1d70fbe4, {0x3e, 0x51,}}, //29.441344
        {0x1db9a541, {0x3f, 0x51,}}, //29.725178
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

