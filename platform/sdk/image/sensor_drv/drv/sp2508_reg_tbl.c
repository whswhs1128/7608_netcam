/*!
*****************************************************************************
** \file        sp2508_reg_tbl.c
**
** \version     $Id: sp2508_reg_tbl.c 13054 2017-11-28 04:00:55Z yulindeng $
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
GADI_VI_SensorDrvInfoT    sp2508_sensor =
{
    .magic_start        = GADI_SENSOR_HW_INFO_MAGIC_START,
    .version            = GADI_SENSOR_HW_INFO_VERSION,
    .length             = sizeof(GADI_VI_SensorDrvInfoT),
    .file_date          = 0x20171127,
    .HwInfo         =
    {
        .name               = "sp2508",
        .ctl_type           = GADI_SENSOR_CTL_I2C,
        .addr_len           = sizeof(GADI_U8),
        .data_len           = sizeof(GADI_U8),
        .reset_active_level = GADI_GPIO_LOW,
        .hw_addr            = {(0x78>>1), (0x7A>>1), 0, 0},  //or 0x7A
        .id_reg =
        {
            {0x02, 0x25},
            {0x03, 0x08},
        },
        .reset_reg =
        {
            {0xFD, 0x00,},
            {GADI_VI_SENSOR_TABLE_FLAG_MSLEEP, 0x0A},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        .standby_reg =
        {
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
    },
    .debug_print        = 1,
    .input_format       = GADI_VI_INPUT_FORMAT_RGB_RAW,//GADI_VI_INPUT_FORMAT_RGB_RAW,//GADI_VI_INPUT_FORMAT_YUV_422_PROG,
    .field_format       = 1,
    .adapter_id         = 0,
    .sensor_id          = GADI_ISP_SENSOR_SP2508,
    .sensor_double_step = 20,
    .typeofsensor       = 1,
    .sync_time_delay    = 40,
    .source_type.dummy  = GADI_VI_CMOS_CHANNEL_TYPE_AUTO,
    .dev_type           = GADI_VI_DEV_TYPE_CMOS,
    .video_system       = GADI_VIDEO_SYSTEM_AUTO,
    .vs_polarity        = GADI_VI_RISING_EDGE,//GADI_VI_RISING_EDGE,//GADI_VI_FALLING_EDGE
    .hs_polarity        = GADI_VI_RISING_EDGE,//GADI_VI_FALLING_EDGE,//GADI_VI_RISING_EDGE,
    .data_edge          = GADI_VI_RISING_EDGE,//GADI_VI_FALLING_EDGE,//GADI_VI_RISING_EDGE,
    .emb_sync_switch    = GADI_VI_EMB_SYNC_OFF,
    .emb_sync_loc       = GADI_VI_EMB_SYNC_UPPER_PEL,
    .emb_sync_mode      = GADI_VI_EMB_SYNC_ITU_656,
    .max_width          = 1600,
    .max_height         = 1200,
    .def_sh_time        = GADI_VIDEO_FPS(50),
    .fmt_num            = 1,
    .auto_fmt           = GADI_VIDEO_MODE(1280,  720,  25, 1),
    .init_reg           =
    {
#if 0
        {0xFD, 0x01,},
        {0x3F, 0x00,},
        {0xFD, 0x00,},
        {0x1C, 0x03,},
        {0x35, 0x20,},
        {0x2F, 0x04,}, //PCLK 48M
        {0xFD, 0x01,},
        {0x03, 0x03,},
        {0x04, 0x09,},
        {0x06, 0x10,},
        {0x24, 0xA0,},
        {0x01, 0x01,},
        {0x2B, 0xC4,},
        {0x2E, 0x20,},
        {0x79, 0x42,},
        {0x85, 0x0F,},
        {0x09, 0x01,},
        {0x0A, 0x40,},
        {0x21, 0xEF,},
        {0x25, 0xF0,},
        {0x26, 0x00,},
        {0x2A, 0xEA,},
        {0x2C, 0xF0,},
        {0x8A, 0x44,},
        {0x8B, 0x44,},
        {0x19, 0xF3,},
        {0x11, 0x30,},
        {0xD0, 0x01,},
        {0xD1, 0x01,},
        {0xD2, 0xD0,},
        {0x55, 0x10,},
        {0x58, 0x30,},
        {0x5D, 0x15,},
        {0x5E, 0x05,},
        {0x64, 0x40,},
        {0x65, 0x00,},
        {0x66, 0x66,},
        {0x67, 0x00,},
        {0x68, 0x68,},
        {0x72, 0x70,},
        {0xFB, 0x25,},
        {0xF0, 0x00,},
        {0xF1, 0x00,},
        {0xF2, 0x00,},
        {0xF3, 0x00,},
        {0xFD, 0x02,},
        {0x00, 0xC6,},
        {0x01, 0xC6,},
        {0x03, 0xC6,},
        {0x04, 0xC6,},
        {0xFD, 0x01,},
        {0x75, 0x80,}, //减少坏线
        {0x76, 0x80,}, //减少坏线
        {0xB3, 0x00,},
        {0x93, 0x01,},
        {0x9D, 0x17,},
        {0xC5, 0x01,},
        {0xC6, 0x00,},
        {0xB1, 0x01,},
        {0x8E, 0x06,},
        {0x8F, 0x50,},
        {0x90, 0x04,},
        {0x91, 0xC0,},
        {0x92, 0x01,},
        {0xA1, 0x05,},
        {0xAA, 0x01,},
        {0xAC, 0x01,},
#else
        {0xFD, 0x00,},  //
        {0x1B, 0x00,},  //
        {0x1C, 0x00,},  //
        {0x1E, 0x55,},  //
        {0x35, 0x20,},  // pll bias


        {0xFD, 0x01,},  //
        {0x06, 0x10,},  // vblank
        {0x24, 0x10,},  // a0 ;pga gain 10x
        {0x01, 0x01,},  // enable reg write
        {0x2B, 0xC4,},  // readout vref
        {0x2E, 0x20,},  // dclk delay
        {0x79, 0x42,},  // p39 p40
        {0x85, 0x0F,},  // p51
        {0x09, 0x01,},  // hblank
        {0x0A, 0x40,},  //
        {0x21, 0xEF,},  // pcp tx 4.05v
        {0x25, 0xF0,},  // reg dac 2.7v, enable bl_en,vbl 1.28v
        {0x26, 0x00,},  // vref2 1v, disable ramp driver
        {0x2A, 0xEA,},  // bypass dac res, adc range 0.745, vreg counter 0.9
        {0x2C, 0xF0,},  // high 8bit, pldo 2.7v
        {0x8A, 0x44,},  // pixel bias 1.58uA
        {0x8B, 0x44,},  //
        {0x19, 0xF3,},  // icom1 1.7u, icom2 0.6u
        {0x11, 0x30,},  // rst num
        {0xD0, 0x01,},  // boost2 enable
        {0xD1, 0x01,},  // boost2 start point h'1do
        {0xD2, 0xD0,},  //
        {0x55, 0x10,},  //
        {0x58, 0x30,},  //
        {0x5D, 0x15,},  //
        {0x5E, 0x05,},  //
        {0x64, 0x40,},  //
        {0x65, 0x00,},  //
        {0x66, 0x66,},  //
        {0x67, 0x00,},  //
        {0x68, 0x68,},  //
        {0x72, 0x70,},  //
        {0xFB, 0x25,},  //
        {0xF0, 0x00,},  // offset
        {0xF1, 0x00,},  //
        {0xF2, 0x00,},  //
        {0xF3, 0x00,},  //

        {0xFD, 0x02,},  //
        {0x00, 0xC6,},  // ad
        {0x01, 0xC6,},  // ad
        {0x03, 0xC6,},  // ad
        {0x04, 0xC6,},  // ad

        {0xFD, 0x01,},  //
#endif
        {GADI_VI_SENSOR_TABLE_FLAG_END, 0x00},
    },
    .pll_table =
    {
        [0] =
        {
            // Pll_clk_sel=1:pixclk = pll_extclk / Divider
            // Pll_clk_sel=0:pixclk = pll_extclk * (3 + pll_nc) / (1 + pll_mc) / Divider =
            //                      = 24 * (3 + 2) / (1 + 0) / 2 = 60Mhz
            //                      = 24 * (3 + 4) / (1 + 0) / 2 = 84Mhz
            .pixclk = 84000000,
            .extclk = 24000000,
            .regs =
            {
                {0xFD, 0x00,},  //
                {0x30, 0x05,},  // [6]Pll_clk_sel
                //{0x2F, 0x08,},  // [6:2] pll_nc 00010 [1:0] pll_mc 00 pll clk 60M
                {0x2F, 0x10,},  // [6:2] pll_nc 00100 [1:0] pll_mc 00 pll clk 60M
                {0x34, 0x01,},  // [1:0] Divider:00=1 01=2 10=4 11=8
                {0xFD, 0x01,},  //
                {GADI_VI_SENSOR_TABLE_FLAG_END,}
            }
        },
    },
    .video_fmt_table =
    {
        [0] = // 1280x720@25fps  2000*1200*25=60000000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  25, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0xFD, 0x01,},  //  (1160+320*2)*(1225+16*4)*30=55100400
                {0x09, 0x01,},  // hblank 0x0140 =  320
                {0x0A, 0x40,},  //
                {0x05, 0x00,},  // vblank 0x0010 =   16
                {0x06, 0x10,},  //
                {0x03, 0x04,},  // exp time, 4 base
                {0x04, 0x0C,},  //

                {0xFD, 0x02,},  //
                {0x37, 0x00, 0x07},  // V Start 0x0000
                {0x38, 0x00},  //
                {0x39, 0x04, 0x07},  // V Size  0x04C0 = 1216
                {0x3A, 0xC0},  //
                {0x3B, 0x00, 0x07},  // H Start 0x0000
                {0x3C, 0x00},  //
                {0x3D, 0x03, 0x07},  // H Size  0x0328 =  808 * 2 = 1616
                {0x3E, 0x28},  //

                {0xFD, 0x01,},  //
                {0x01, 0x01,},  //
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 1280,
                .height     = 720,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_DVP,
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
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_RG,//GADI_VI_BAYER_PATTERN_GB,GADI_VI_BAYER_PATTERN_BG,GADI_VI_BAYER_PATTERN_GR, GADI_VI_BAYER_PATTERN_RG
                    //GADI_VI_YUV_CR_Y0_CB_Y1,
                    //GADI_VI_YUV_CB_Y0_CR_Y1,
                    //GADI_VI_YUV_Y0_CR_Y1_CB,
                    //GADI_VI_YUV_Y0_CB_Y1_CR,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_RG,
                    GADI_VI_BAYER_PATTERN_RG,
                    GADI_VI_BAYER_PATTERN_RG,//GADI_VI_BAYER_PATTERN_BG, GADI_VI_BAYER_PATTERN_RG
                    GADI_VI_BAYER_PATTERN_RG,
                },
            },
            .update_after_vsync_start   = 3,
            .update_after_vsync_end     = 10,
        },
    },
#define SP2508_V_FLIP    (1<<1)
#define SP2508_H_MIRROR  (1<<0)
    .mirror_table =
    {
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY_VERTICALLY] =
        {
            {0x3F, SP2508_V_FLIP | SP2508_H_MIRROR, SP2508_V_FLIP | SP2508_H_MIRROR},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY] =
        {
            {0x3F, SP2508_H_MIRROR, SP2508_V_FLIP | SP2508_H_MIRROR},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_VERTICALLY] =
        {
            {0x3F, SP2508_V_FLIP, SP2508_V_FLIP | SP2508_H_MIRROR},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_NONE] =
        {
            {0x3F, 0x00, SP2508_V_FLIP | SP2508_H_MIRROR},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
    },
    .version_reg =
    {
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
    .agc_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_SET,
    .vmax_reg_update_mode       = GADI_SENSOR_UPDATE_REG_BY_SET,
    .mirror_reg_update_mode     = GADI_SENSOR_UPDATE_REG_BY_SET,
    .shs_additional_use         = 0,
    .shs_additional_percent     = 0,
    // hmax = (FRAMEW_H & 0x0F) << 8 +
    //        (FRAMEW_L & 0xFF) << 0
    .hmax_reg =
    {
        //{0x09, 0x00, 0x0F, 0x00, 0x08, 0},  //
        //{0x0A, 0x00, 0xFF, 0x00, 0x00, 0},  //
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    // vmax = (FRAMEH_H & 0xFF) << 8 +
    //        (FRAMEH_L & 0xFF) << 0
    .vmax_reg =
    {
        //{0x05, 0x00, 0xFF, 0x00, 0x08, 0},  // VB_h
        //{0x06, 0x00, 0xFF, 0x00, 0x00, 0},  // VB_l
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .vmax_wadd_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    // shs = (EXP_H & 0xFF) << 8 +
    //       (EXP_L & 0xFF) << 0
    .shs_reg =
    {
        //{0x03, 0x04, 0xFF, 0x00, 0x08, 0},  //
        //{0x04, 0x0C, 0xFF, 0x00, 0x00, 0},  //
        {GADI_VI_SENSOR_TABLE_FLAG_END, },
    },
    .shs_additional_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    .cal_gain_mode              = GADI_SENSOR_CAL_GAIN_MODE0,
    .max_agc_index = 0,
    .max_again_index = 0,
    .cur_again_index = 0,
    .agc_info =
    {
        .db_max     = 0x0F800000,   // 15.5dB
        .db_min     = 0x01000000,   // 1dB
        .db_step    = 0x00100000,   // 0.0625dB
    },
    .gain_reg =
    {
        {0x24, 0x00,},
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .gain_table =
    {
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

