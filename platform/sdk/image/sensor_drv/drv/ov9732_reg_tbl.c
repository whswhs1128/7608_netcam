/*!
*****************************************************************************
** \file        ov9732_reg_tbl.c
**
** \version     $Id$
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
GADI_VI_SensorDrvInfoT    ov9732_sensor =
{
    .magic_start        = GADI_SENSOR_HW_INFO_MAGIC_START,
    .version            = GADI_SENSOR_HW_INFO_VERSION,
    .length             = sizeof(GADI_VI_SensorDrvInfoT),
    .file_date          = 0x20161014,
    .HwInfo         =
    {
        .name               = "ov9732",
        .ctl_type           = GADI_SENSOR_CTL_I2C,
        .addr_len           = sizeof(GADI_U16),
        .data_len           = sizeof(GADI_U8),
        .reset_active_level = GADI_GPIO_LOW,
        .hw_addr            = {(0x20>>1), (0x6C>>1), 0, 0},
        .id_reg =
        {
            {0x300A, 0x97}, // CHIP_ID_H
            {0x300B, 0x32}, // CHIP_ID_L
        },
        .reset_reg =
        {
            {0x0103, 0x01, 0x01},
            {GADI_VI_SENSOR_TABLE_FLAG_MSLEEP, 10},    // msleep(10);
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
    .sensor_id          = GADI_ISP_SENSOR_OV9732,
    .sensor_double_step = 16,
    .typeofsensor       = 2,
    .sync_time_delay    = 0,
    .source_type.dummy  = GADI_VI_CMOS_CHANNEL_TYPE_AUTO,
    .dev_type           = GADI_VI_DEV_TYPE_CMOS,
    .video_system       = GADI_VIDEO_SYSTEM_AUTO,
    .vs_polarity        = GADI_VI_FALLING_EDGE,
    .hs_polarity        = GADI_VI_RISING_EDGE,
    .data_edge          = GADI_VI_RISING_EDGE,
    .emb_sync_switch    = GADI_VI_EMB_SYNC_OFF,
    .emb_sync_loc       = GADI_VI_EMB_SYNC_LOWER_PEL,
    .emb_sync_mode      = GADI_VI_EMB_SYNC_ITU_656,
    .max_width          = 1280,
    .max_height         = 720,
    .def_sh_time        = GADI_VIDEO_FPS(50),
    .fmt_num            = 2,
    .auto_fmt           = GADI_VIDEO_MODE(1280,  720,  25, 1),
    .init_reg           =
    {
        {0x0100, 0x00,},
        {0x3001, 0x1F,},
        {0x3002, 0xFF,},
        {0x3007, 0x40,},
        {0x3008, 0x00,},
        {0x3009, 0x03,},
        {0x3010, 0x00,},
        {0x3011, 0x00,},
        {0x3014, 0x36,},
        {0x301E, 0x15,},
        {0x3030, 0x09,},
        {0x3084, 0x02,},
        {0x3085, 0x01,},
        {0x3086, 0x01,},
        {0x308A, 0x00,},
        {0x3600, 0xF6,},
        {0x3601, 0x72,},
        {0x3605, 0x66,},
        {0x3610, 0x0C,},
        {0x3611, 0x60,},
        {0x3612, 0x35,},
        {0x3654, 0x10,},
        {0x3655, 0x77,},
        {0x3656, 0x77,},
        {0x3657, 0x07,},
        {0x3658, 0x22,},
        {0x3659, 0x22,},
        {0x365A, 0x02,},
        {0x3700, 0x1F,},
        {0x3701, 0x10,},
        {0x3702, 0x0C,},
        {0x3703, 0x0B,},
        {0x3704, 0x3C,},
        {0x3705, 0x51,},
        {0x370D, 0x20,},
        {0x3710, 0x0D,},
        {0x3782, 0x58,},
        {0x3783, 0x60,},
        {0x3784, 0x05,},
        {0x3785, 0x55,},
        {0x37C0, 0x07,},
        {0x3816, 0x00,},
        {0x3817, 0x00,},
        {0x3818, 0x00,},
        {0x3819, 0x01,},
        {0x3820, 0x10,},
        {0x3821, 0x00,},
        {0x382C, 0x06,},
        {0x3503, 0x03,},
        {0x3504, 0x00,},
        {0x3505, 0x00,},
        {0x3509, 0x10,},
        {0x3D00, 0x00,},
        {0x3D01, 0x00,},
        {0x3D02, 0x00,},
        {0x3D03, 0x00,},
        {0x3D04, 0x00,},
        {0x3D05, 0x00,},
        {0x3D06, 0x00,},
        {0x3D07, 0x00,},
        {0x3D08, 0x00,},
        {0x3D09, 0x00,},
        {0x3D0A, 0x00,},
        {0x3D0B, 0x00,},
        {0x3D0C, 0x00,},
        {0x3D0D, 0x00,},
        {0x3D0E, 0x00,},
        {0x3D0F, 0x00,},
        {0x3D80, 0x00,},
        {0x3D81, 0x00,},
        {0x3D82, 0x38,},
        {0x3D83, 0xA4,},
        {0x3D84, 0x00,},
        {0x3D85, 0x00,},
        {0x3D86, 0x1F,},
        {0x3D87, 0x03,},
        {0x3D8B, 0x00,},
        {0x3D8F, 0x00,},
        {0x4001, 0xE0,},
        {0x4004, 0x00,},
        {0x4005, 0x02,},
        {0x4006, 0x01,},
        {0x4007, 0x40,},
        {0x4009, 0x0B,},
        {0x4300, 0x03,},
        {0x4301, 0xFF,},
        {0x4304, 0x00,},
        {0x4305, 0x00,},
        {0x4309, 0x00,},
        {0x4600, 0x00,},
        {0x4601, 0x04,},
        {0x4800, 0x04,},
        {0x4805, 0x00,},
        {0x4821, 0x3C,},
        {0x4823, 0x3C,},
        {0x4837, 0x2D,},
        {0x4A00, 0x00,},
        {0x4F00, 0x80,},
        {0x4F01, 0x10,},
        {0x4F02, 0x00,},
        {0x4F03, 0x00,},
        {0x4F04, 0x00,},
        {0x4F05, 0x00,},
        {0x4F06, 0x00,},
        {0x4F07, 0x00,},
        {0x4F08, 0x00,},
        {0x4F09, 0x00,},
        {0x5000, 0x07,},
        {0x500C, 0x00,},
        {0x500D, 0x00,},
        {0x500E, 0x00,},
        {0x500F, 0x00,},
        {0x5010, 0x00,},
        {0x5011, 0x00,},
        {0x5012, 0x00,},
        {0x5013, 0x00,},
        {0x5014, 0x00,},
        {0x5015, 0x00,},
        {0x5016, 0x00,},
        {0x5017, 0x00,},
        {0x5080, 0x00,},
        {0x5180, 0x01,},
        {0x5181, 0x00,},
        {0x5182, 0x01,},
        {0x5183, 0x00,},
        {0x5184, 0x01,},
        {0x5185, 0x00,},
        {0x5708, 0x06,},
        {0x5781, 0x0E,},
        {0x5783, 0x0F,},
        {0x0100, 0x01,}, // BIT[0]: 0: standby, 1: Streaming
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .pll_table =
    {
        // pixclk = extclk / prediv * multiplier / (1 + sys_div) / pix_div = HTS(fact) * VTS(reg) * fps
        // sysclk = pixclk / dig_sys_div = HTS(reg) * 2 * VTS(reg) * fps
        [0] =
        {
            // for 720P@30, 720P@25
            .pixclk = 37125000, // pixclk = extclk / prediv * multiplier / (1 + sys_div) / pix_div
            .extclk = 27000000, //        = 27 / 2 * 55 / (1 + 4) / 2 = 74.25
            // sysclk = pixclk / dig_sys_div = 74.25 / 1 = 74.25
            .regs =
            {
                {0x3080, 0x02,}, // BIT[2:0] prediv 1/1.5/2/2.5/3/4/5/6
                {0x3081, 0x37,}, // BIT[7:0] multiplier
                {0x3082, 0x04,}, // BIT[3:0] sys_div
                {0x3083, 0x00,}, // BIT[0] pix_div 2/4
                {0x3089, 0x01,},
                {0x3103, 0x01,}, // BIT[6:4] dig_sys_div 1~7
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,}
            },
        },
    },
    .video_fmt_table =
    {
        [0] = // 1280x720P@30fps    1650 * 750 * 30 = 37125000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x0100, 0x00, 0x01,}, // BIT[0]: 0: standby, 1: Streaming
                {0x3800, 0x00,}, // H_crop_start_H
                {0x3801, 0x04,}, // H_crop_start_L
                {0x3802, 0x00,}, // V_crop_start_H
                {0x3803, 0x04,}, // V_crop_start_L
                {0x3804, 0x05,}, // H_crop_end_H    1291 = 0x050B
                {0x3805, 0x0B,}, // H_crop_end_L
                {0x3806, 0x02,}, // V_crop_end_H     731 = 0x02DB
                {0x3807, 0xDB,}, // V_crop_end_L
                {0x3808, 0x05,}, // H_output_size_H 1280 = 0x0500
                {0x3809, 0x00,}, // H_output_size_L
                {0x380A, 0x02,}, // V_output_size_H  720 = 0x02D0
                {0x380B, 0xD0,}, // V_output_size_L
                {0x380C, 0x06,}, // HTS_H           1650 = 0x0672
                {0x380D, 0x72,}, // HTS_L
                {0x3810, 0x00,}, // H_win_off_H
                {0x3811, 0x04,}, // H_win_off_L
                {0x3812, 0x00,}, // V_win_off_H
                {0x3813, 0x04,}, // V_win_off_L
                {0x0100, 0x01, 0x01,}, // BIT[0]: 0: standby, 1: Streaming
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
        [1] = // 1280x720P@25fps    1980 * 750 * 25 = 37125000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  25, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x0100, 0x00, 0x01,}, // BIT[0]: 0: standby, 1: Streaming
                {0x3800, 0x00,}, // H_crop_start_H
                {0x3801, 0x04,}, // H_crop_start_L
                {0x3802, 0x00,}, // V_crop_start_H
                {0x3803, 0x04,}, // V_crop_start_L
                {0x3804, 0x05,}, // H_crop_end_H    1291 = 0x050B
                {0x3805, 0x0B,}, // H_crop_end_L
                {0x3806, 0x02,}, // V_crop_end_H     731 = 0x02DB
                {0x3807, 0xDB,}, // V_crop_end_L
                {0x3808, 0x05,}, // H_output_size_H 1280 = 0x0500
                {0x3809, 0x00,}, // H_output_size_L
                {0x380A, 0x02,}, // V_output_size_H  720 = 0x02D0
                {0x380B, 0xD0,}, // V_output_size_L
                {0x380C, 0x07,}, // HTS_H           1980 = 0x07BC
                {0x380D, 0xBC,}, // HTS_L
                {0x3810, 0x00,}, // H_win_off_H
                {0x3811, 0x04,}, // H_win_off_L
                {0x3812, 0x00,}, // V_win_off_H
                {0x3813, 0x04,}, // V_win_off_L
                {0x0100, 0x01, 0x01,}, // BIT[0]: 0: standby, 1: Streaming
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
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_720P_25,
            },
            .video_info_table =
            {
                .def_start_x    = 0,
                .def_start_y    = 0,
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
    .mirror_table =
    {
#define OV9732_HORIZ_MIRROR     (0x01 << 3)
#define OV9732_VERT_FLIP        (0x01 << 2)
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY_VERTICALLY] =
        {
            {0x3820, OV9732_HORIZ_MIRROR|OV9732_VERT_FLIP,OV9732_HORIZ_MIRROR|OV9732_VERT_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY] =
        {
            {0x3820, OV9732_HORIZ_MIRROR,OV9732_HORIZ_MIRROR|OV9732_VERT_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_VERTICALLY] =
        {
            {0x3820, OV9732_VERT_FLIP,OV9732_HORIZ_MIRROR|OV9732_VERT_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_NONE] =
        {
            {0x3820, 0,OV9732_HORIZ_MIRROR|OV9732_VERT_FLIP},
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
    .agc_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_VSYNC,
    .vmax_reg_update_mode       = GADI_SENSOR_UPDATE_REG_BY_SET,
    .mirror_reg_update_mode     = GADI_SENSOR_UPDATE_REG_BY_SET,
    .shs_additional_use         = 0,
    .shs_additional_percent     = 0,
    // hmax = (TIMING_CONTROL_HTS_HIGHBYTE & 0xFF) << 8 +
    //        (TIMING_CONTROL_HTS_LOWBYTE & 0xFF) << 0
    .hmax_reg =
    {
        {0x380C, 0x00, 0xFF, 0x00, 0x08, 0}, // TIMING_CONTROL_HTS [11:8]
        {0x380D, 0x00, 0xFF, 0x00, 0x00, 0}, // TIMING_CONTROL_HTS [7:0]
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    // vmax = (TIMING_CONTROL_VTS_HIGHBYTE & 0xFF) << 8 +
    //        (TIMING_CONTROL_VTS_LOWBYTE & 0xFF) << 0
    .vmax_reg =
    {
        {0x380E, 0x00, 0xFF, 0x00, 0x08, 0}, // TIMING_CONTROL_VTS [11:8]
        {0x380F, 0x00, 0xFF, 0x00, 0x00, 0}, // TIMING_CONTROL_VTS [7:0]
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .vmax_wadd_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    // shs = (AEC_PK_EXPO_H & 0x0F) << 0x0C +
    //       (AEC_PK_EXPO_M & 0xFF) << 0x04 +
    //       (AEC_PK_EXPO_L & 0xFF) >> 0x04
    .shs_reg =
    {

        {0x3500, 0x00, 0x0F, 0x00, 0x0C, 0}, // AEC_PK_EXPO_H [19:16]
        {0x3501, 0x00, 0xFF, 0x00, 0x04, 0}, // AEC_PK_EXPO_M [15:8]
        {0x3502, 0x00, 0xF0, 0x01, 0x04, 0}, // AEC_PK_EXPO_L [7:0]  Low 4bits are fraction bits
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .shs_additional_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    .cal_gain_mode              = GADI_SENSOR_CAL_GAIN_MODE1,
    .max_agc_index = 112,
    .max_again_index = 0,
    .cur_again_index = 0,
    .agc_info =
    {
        .db_max     = 0x1FEE939E,    // 32db
        .db_min     = 0x01000000,    // 1dB
        .db_step    = 0x00100000,    // 0.0625dB
    },
    .gain_reg =
    {
        {0x350A,},
        {0x350B,},
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .gain_table =
    {
        { 0x0       ,  {0x00,0x010,}},
        { 0x86CDDD  ,  {0x00,0x011,}},
        { 0x105E6A1 ,  {0x00,0x012,}},
        { 0x17E1FC5 ,  {0x00,0x013,}},
        { 0x1F02DE2 ,  {0x00,0x014,}},
        { 0x25CAB1F ,  {0x00,0x015,}},
        { 0x2C41C1A ,  {0x00,0x016,}},
        { 0x326F3C0 ,  {0x00,0x017,}},
        { 0x3859652 ,  {0x00,0x018,}},
        { 0x3E05BC4 ,  {0x00,0x019,}},
        { 0x43791B4 ,  {0x00,0x01A,}},
        { 0x48B7CF3 ,  {0x00,0x01B,}},
        { 0x4DC5ACF ,  {0x00,0x01C,}},
        { 0x52A6223 ,  {0x00,0x01D,}},
        { 0x575C434 ,  {0x00,0x01E,}},
        { 0x5BEAD6E ,  {0x00,0x01F,}},
        { 0x6054603 ,  {0x00,0x020,}},
        { 0x68C13E0 ,  {0x00,0x022,}},
        { 0x70B2CA4 ,  {0x00,0x024,}},
        { 0x78365C8 ,  {0x00,0x026,}},
        { 0x7F573E5 ,  {0x00,0x028,}},
        { 0x861F122 ,  {0x00,0x02A,}},
        { 0x8C9621D ,  {0x00,0x02C,}},
        { 0x92C39C4 ,  {0x00,0x02E,}},
        { 0x98ADC55 ,  {0x00,0x030,}},
        { 0x9E5A1C8 ,  {0x00,0x032,}},
        { 0xA3CD7B8 ,  {0x00,0x034,}},
        { 0xA90C2F6 ,  {0x00,0x036,}},
        { 0xAE1A0D3 ,  {0x00,0x038,}},
        { 0xB2FA827 ,  {0x00,0x03A,}},
        { 0xB7B0A37 ,  {0x00,0x03C,}},
        { 0xBC3F372 ,  {0x00,0x03E,}},
        { 0xC0A8C06 ,  {0x00,0x040,}},
        { 0xC9159E3 ,  {0x00,0x044,}},
        { 0xD1072A7 ,  {0x00,0x048,}},
        { 0xD88ABCB ,  {0x00,0x04C,}},
        { 0xDFAB9E8 ,  {0x00,0x050,}},
        { 0xE673725 ,  {0x00,0x054,}},
        { 0xECEA820 ,  {0x00,0x058,}},
        { 0xF317FC7 ,  {0x00,0x05C,}},
        { 0xF902258 ,  {0x00,0x060,}},
        { 0xFEAE7CB ,  {0x00,0x064,}},
        { 0x10421DBB,  {0x00,0x068,}},
        { 0x109608F9,  {0x00,0x06C,}},
        { 0x10E6E6D6,  {0x00,0x070,}},
        { 0x1134EE2A,  {0x00,0x074,}},
        { 0x1180503B,  {0x00,0x078,}},
        { 0x11C93975,  {0x00,0x07C,}},
        { 0x120FD209,  {0x00,0x080,}},
        { 0x12969FE6,  {0x00,0x088,}},
        { 0x1315B8AA,  {0x00,0x090,}},
        { 0x138DF1CE,  {0x00,0x098,}},
        { 0x13FFFFEC,  {0x00,0x0A0,}},
        { 0x146C7D28,  {0x00,0x0A8,}},
        { 0x14D3EE23,  {0x00,0x0B0,}},
        { 0x1536C5CA,  {0x00,0x0B8,}},
        { 0x1595685B,  {0x00,0x0C0,}},
        { 0x15F02DCE,  {0x00,0x0C8,}},
        { 0x164763BE,  {0x00,0x0D0,}},
        { 0x169B4EFD,  {0x00,0x0D8,}},
        { 0x16EC2CD9,  {0x00,0x0E0,}},
        { 0x173A342D,  {0x00,0x0E8,}},
        { 0x1785963E,  {0x00,0x0F0,}},
        { 0x17CE7F78,  {0x00,0x0F8,}},
        { 0x1815180C,  {0x01,0x080,}},
        { 0x18598475,  {0x01,0x088,}},
        { 0x189BE5EA,  {0x01,0x090,}},
        { 0x18DC5ABB,  {0x01,0x098,}},
        { 0x191AFEAE,  {0x01,0x0A0,}},
        { 0x1957EB3A,  {0x01,0x0A8,}},
        { 0x199337D1,  {0x01,0x0B0,}},
        { 0x19CCFA10,  {0x01,0x0B8,}},
        { 0x1A0545EF,  {0x01,0x0C0,}},
        { 0x1A3C2DEB,  {0x01,0x0C8,}},
        { 0x1A71C32B,  {0x01,0x0D0,}},
        { 0x1AA615A2,  {0x01,0x0D8,}},
        { 0x1AD93426,  {0x01,0x0E0,}},
        { 0x1B0B2C90,  {0x01,0x0E8,}},
        { 0x1B3C0BCD,  {0x01,0x0F0,}},
        { 0x1B6BDDF6,  {0x01,0x0F8,}},
        { 0x1B9AAE5E,  {0x02,0x080,}},
        { 0x1BC887A9,  {0x02,0x088,}},
        { 0x1BF573D1,  {0x02,0x090,}},
        { 0x1C217C3C,  {0x02,0x098,}},
        { 0x1C4CA9C1,  {0x02,0x0A0,}},
        { 0x1C7704B8,  {0x02,0x0A8,}},
        { 0x1CA09500,  {0x02,0x0B0,}},
        { 0x1CC96209,  {0x02,0x0B8,}},
        { 0x1CF172DC,  {0x02,0x0C0,}},
        { 0x1D18CE24,  {0x02,0x0C8,}},
        { 0x1D3F7A30,  {0x02,0x0D0,}},
        { 0x1D657CFF,  {0x02,0x0D8,}},
        { 0x1D8ADC41,  {0x02,0x0E0,}},
        { 0x1DAF9D5E,  {0x02,0x0E8,}},
        { 0x1DD3C57B,  {0x02,0x0F0,}},
        { 0x1DF7597D,  {0x02,0x0F8,}},
        { 0x1E1A5E0F,  {0x03,0x080,}},
        { 0x1E3CD7A4,  {0x03,0x088,}},
        { 0x1E5ECA78,  {0x03,0x090,}},
        { 0x1E803A9C,  {0x03,0x098,}},
        { 0x1EA12BED,  {0x03,0x0A0,}},
        { 0x1EC1A21F,  {0x03,0x0A8,}},
        { 0x1EE1A0BF,  {0x03,0x0B0,}},
        { 0x1F012B2F,  {0x03,0x0B8,}},
        { 0x1F2044B1,  {0x03,0x0C0,}},
        { 0x1F3EF061,  {0x03,0x0C8,}},
        { 0x1F5D313D,  {0x03,0x0D0,}},
        { 0x1F7B0A23,  {0x03,0x0D8,}},
        { 0x1F987DD5,  {0x03,0x0E0,}},
        { 0x1FB58EF6,  {0x03,0x0E8,}},
        { 0x1FD24013,  {0x03,0x0F0,}},
        { 0x1FEE939E,  {0x03,0x0F8,}},
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

