/*!
*****************************************************************************
** \file        jxh62_reg_tbl.c
**
** \version     $Id: jxh62_reg_tbl.c 11627 2017-03-10 10:58:12Z dengbiao $
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
GADI_VI_SensorDrvInfoT    jxh62_sensor =
{
    .magic_start        = GADI_SENSOR_HW_INFO_MAGIC_START,
    .version            = GADI_SENSOR_HW_INFO_VERSION,
    .length             = sizeof(GADI_VI_SensorDrvInfoT),
    .file_date          = 0x20161017,
    .HwInfo         =
    {
        .name               = "jxh62",
        .ctl_type           = GADI_SENSOR_CTL_I2C,
        .addr_len           = sizeof(GADI_U8),
        .data_len           = sizeof(GADI_U8),
        .reset_active_level = GADI_GPIO_LOW,
        .hw_addr            = {(0x60>>1), (0x64>>1), (0x68>>1), (0x6C>>1)},
        .id_reg =
        {
            {0x0A, 0xA0},
            {0x0B, 0x62},
        },
        .reset_reg =
        {
            {0x12, 0x80},
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
    .sensor_id          = GADI_ISP_SENSOR_JXH62,
    .sensor_double_step = 20,
    .typeofsensor       = 1,
    .sync_time_delay    = 40,
    .source_type.dummy  = GADI_VI_CMOS_CHANNEL_TYPE_AUTO,
    .dev_type           = GADI_VI_DEV_TYPE_CMOS,
    .video_system       = GADI_VIDEO_SYSTEM_AUTO,
    .vs_polarity        = GADI_VI_RISING_EDGE,
    .hs_polarity        = GADI_VI_RISING_EDGE,
    .data_edge          = GADI_VI_FALLING_EDGE,
    .emb_sync_switch    = GADI_VI_EMB_SYNC_OFF,
    .emb_sync_loc       = GADI_VI_EMB_SYNC_LOWER_PEL,
    .emb_sync_mode      = GADI_VI_EMB_SYNC_ITU_656,
    .max_width          = 1600,
    .max_height         = 900,
    .def_sh_time        = GADI_VIDEO_FPS(50),
    .fmt_num            = 2,
    .auto_fmt           = GADI_VIDEO_MODE(1280,  720,  25, 1),
    .init_reg           =
    {
        {0x0C, 0x00}, // DVP1[7]: RSVD
                      // DVP1[6]: Sensor sun spot elimination control,
                      //  1: disable sun spot elimination,
                      //  0: enable sun spot elimination.
                      // DVP1[5:2]: RSVD
                      // DVP1[1:0]: DVP test mode output option.
                      //  00: DVP output normal image data.
                      //  01: DVP[9:0] = Output 10-bit walk ¡°1¡± test pattern.
                      //  10,11: RSVD.
        {0x19, 0x68},
        {0x1D, 0xFF},
        {0x1E, 0x1F},
        {0x1F, 0x01},
        {0x2A, 0x70},
        {0x2B, 0x21},
        {0x31, 0x0C},
        {0x33, 0x4C},
        {0x34, 0x2F},
        {0x35, 0xA3},
        {0x36, 0x05},
        {0x38, 0x53},
        {0x3A, 0x08},
        {0x41, 0x88},
        {0x42, 0x12},
        {0x56, 0x02},
        {0x57, 0x80},
        {0x58, 0x33},
        {0x5A, 0x04},
        {0x5B, 0xB6},
        {0x5C, 0x08},
        {0x5D, 0x67},
        {0x5E, 0x04},
        {0x5F, 0x08},
        {0x60, 0x02},
        {0x63, 0x82},
        {0x66, 0x28},
        {0x67, 0xF8},
        {0x68, 0x04},
        {0x69, 0x74},
        {0x6A, 0x3F},
        {0x6C, 0xC0},
        {0x6E, 0x5C},
        {0x7A, 0x80},
        {0x82, 0x01},
        {0x46, 0xC2},
        {0x48, 0x7E},
        {0x62, 0x40},
        {0x7D, 0x57},
        {0x7E, 0x28},
        {0x80, 0x00},
        {0x4A, 0x05},
        {0x49, 0x10},
        {0x13, 0x81},
        {0x59, 0x97},
        {0x47, 0x44},
        {GADI_VI_SENSOR_TABLE_FLAG_END, 0x00},
    },
    .pll_table =
    {
        // PLL1[7] = 1: PLLclk = extclk/(1 + PLL1[1:0])
        // PLL1[7] = 0:
        //   PLL2[6] = 1: PLLclk = extclk*PLL3[7:0]/(1 + PLL1[1:0])/2
        //   PLL2[6] = 0: PLLclk = extclk*PLL3[7:0]/(1 + PLL1[1:0])/(1+PLL2[5:3])
        [0] =
        {
            // for 720P@30, 720P@25
            .pixclk = 54000000, // PLLclk = extclk*PLL3[7:0]/(1 + PLL1[1:0])/(1+PLL2[5:3])
            .extclk = 27000000, //        = 27*32/(1+1)/(1+1)/4=54
            .regs = {

                {0x0E, 0x11}, // PLL1[7]0: PLL bypass selection. 1: PLLclk = Input clock/PLL_Pre_Ratio. 0: see PLL2[3:0].
                              // PLL1[1:0]01: PLL pre-divider ratio. PLL_Pre_Ratio = 1 + PLL1[1:0] Default : 00
                {0x0F, 0x08}, // PLL2[6]0: Clock divider, 1: VCO clock divided by 2 to generate system and mipi clock. DAC clock no change. 0: normal;
                              // PLL2[5:3]001: DAC Clock divider: DAC Clock = VCO/(1+PLL2[5:3])
                              // PLL2[2:1]00: System clock divider:Mipi_pclk/(1+PLL2[2:1])
                              // PLL2[0]0: Mipi Pclk divider: 0: 1/8 for Mipi raw 8 data mode; 1: 1/10 for Mipi raw 10 data mode.
                {0x10, 0x20}, // PLL VCO multiplier. VCO = Input clock*PLL3[7:0]/PLL_Pre_Ratio
                {0x11, 0x80}, // CLK[7]: System clock option. 1: system clock use PLLclk directly. 0: system clock use PLLclk after digital divide clock
                              // CLK[6]: system clock digital doubler on/off selection. 1: on, 0: off
                              // CLK[5:0]: system clock divide ratio. default :000000
                {GADI_VI_SENSOR_TABLE_FLAG_END,}
            }
        },
    },
    .video_fmt_table =
    {
        [0] = // 1280x720@30fps 2400 * 750 * 30 = 54000000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x12, 0x40, 0x40},
                {0x20, 0x60}, // FrameW_L       0x0960 = 2400
                {0x21, 0x09}, // FrameW_H
                {0x24, 0x00}, // HWin_L         0x0500 = 1280
                {0x25, 0xD0}, // VWin_L         0x02D0 =  720
                {0x26, 0x25}, // VWin_H HWin_H
                {0x27, 0xEE}, // HWinSt_L       0x02EE =  750
                {0x28, 0x15}, // VWinSt_L       0x0015 =   21
                {0x29, 0x02}, // VWinSt_H HWinSt_H
                {0x2C, 0x10}, // SenHASt        each bit represent 1 pixels
                {0x2D, 0x01}, // SenVSt         each bit represent 4 lines
                {0x2E, 0xBB}, // SenVEnd        (0xBB-0x01)*4 = 0x02E8 = 744
                {0x2F, 0xC0}, // SenVadd Sensor vertical address settings.
                              //  SenVadd[7:4]: RSVD
                              //  SenVadd[3:2]: SenVEnd[9:8]
                              //  SenVadd[1:0]: SenVSt[9:8]
                {0x39, 0x90},
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
                .bits       = GADI_VIDEO_BITS_12,
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
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_BG,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_BG,
                },
            },
            .update_after_vsync_start   = 3,
            .update_after_vsync_end     = 10,
        },
        [1] = // 1280x720@25fps 2880 * 750 * 25 = 54000000
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
                {0x20, 0x40}, // HMAX_L         0x0B40 = 2880
                {0x21, 0x0B}, // HMAX_H
                {0x24, 0x00}, // HWin_L         0x0500 = 1280
                {0x25, 0xD0}, // VWin_L         0x02D0 =  720
                {0x26, 0x25}, // VWin_H HWin_H
                {0x27, 0xEE}, // HWinSt_L       0x02EE =  750
                {0x28, 0x15}, // VWinSt_L       0x0015 =   21
                {0x29, 0x02}, // VWinSt_H HWinSt_H
                {0x2C, 0x10}, // SenHASt        each bit represent 1 pixels
                {0x2D, 0x01}, // SenVSt         each bit represent 4 lines
                {0x2E, 0xBB}, // SenVEnd        (0xBB-0x01)*4 = 0x02E8 = 744
                {0x2F, 0xC0}, // SenVadd Sensor vertical address settings.
                              //  SenVadd[7:4]: RSVD
                              //  SenVadd[3:2]: SenVEnd[9:8]
                              //  SenVadd[1:0]: SenVSt[9:8]
                {0x39, 0x94},
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
                .bits       = GADI_VIDEO_BITS_12,
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
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_BG,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_BG,
                },
            },
            .update_after_vsync_start   = 3,
            .update_after_vsync_end     = 10,
        },
    },
#define jxh62_V_FLIP    (1<<4)
#define jxh62_H_MIRROR  (1<<5)
    .mirror_table =
    {
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY_VERTICALLY] =
        {
            {0x12, jxh62_H_MIRROR | jxh62_V_FLIP, jxh62_H_MIRROR | jxh62_V_FLIP},
			{0x27, 0xed},
			{0x28, 0x14},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY] =
        {
            {0x12, jxh62_H_MIRROR, jxh62_H_MIRROR | jxh62_V_FLIP},
			{0x27, 0xed},
			{0x28, 0x15},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_VERTICALLY] =
        {
            {0x12, jxh62_V_FLIP, jxh62_H_MIRROR | jxh62_V_FLIP},
			{0x27, 0xee},
			{0x28, 0x14},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_NONE] =
        {
            {0x12, 0x00, jxh62_H_MIRROR | jxh62_V_FLIP},
			{0x27, 0xee},
			{0x28, 0x15},
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

    .vmax_clk_ref   = GADI_SENSOR_CAL_CLK_PIXCLK,
    .shs_clk_ref    = GADI_SENSOR_CAL_CLK_PIXCLK,
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

