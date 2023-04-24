/*!
*****************************************************************************
** \file        ar0230_reg_tbl.c
**
** \version     $Id: ar0230_reg_tbl.c 11627 2017-03-10 10:58:12Z dengbiao $
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
GADI_VI_SensorDrvInfoT    ar0230_sensor =
{
    .magic_start        = GADI_SENSOR_HW_INFO_MAGIC_START,
    .version            = GADI_SENSOR_HW_INFO_VERSION,
    .length             = sizeof(GADI_VI_SensorDrvInfoT),
    .file_date          = 0x20161014,
    .HwInfo         =
    {
        .name               = "ar0230",
        .ctl_type           = GADI_SENSOR_CTL_I2C,
        .addr_len           = sizeof(GADI_U16),
        .data_len           = sizeof(GADI_U16),
        .reset_active_level = GADI_GPIO_LOW,
        .hw_addr            = {(0x20>>1), (0x30>>1), 0, 0},
        .id_reg =
        {
            {0x3000, 0x00}, // AR0230_CHIP_ID
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        .reset_reg =
        {
            {0x301A, 0x0001}, // soft reset
            {GADI_VI_SENSOR_TABLE_FLAG_MSLEEP, 200},    // msleep(200);
            {0x301A, 0x10D8}, // soft reset
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
    .sensor_id          = GADI_ISP_SENSOR_AR0230,
    .sensor_double_step = 32,
    .typeofsensor       = 2,
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
    .max_width          = 1412,
    .max_height         = 1016,
    .def_sh_time        = GADI_VIDEO_FPS(30),
    .fmt_num            = 2,
    .auto_fmt           = GADI_VIDEO_MODE(1920, 1080,  30, 1),
    .init_reg           =
    {
        //Reset
        {0x301A, 0x0001},
        {0x301A, 0x10D8},
        //Delay 200
        {GADI_VI_SENSOR_TABLE_FLAG_SLEEP, 200},
        //Linear Mode Sequencer - Rev1.3
        {0x3088, 0x8242 },
        {0x3086, 0x4558 },
        {0x3086, 0x729B },
        {0x3086, 0x4A31 },
        {0x3086, 0x4342 },
        {0x3086, 0x8E03 },
        {0x3086, 0x2A14 },
        {0x3086, 0x4578 },
        {0x3086, 0x7B3D },
        {0x3086, 0xFF3D },
        {0x3086, 0xFF3D },
        {0x3086, 0xEA2A },
        {0x3086, 0x043D },
        {0x3086, 0x102A },
        {0x3086, 0x052A },
        {0x3086, 0x1535 },
        {0x3086, 0x2A05 },
        {0x3086, 0x3D10 },
        {0x3086, 0x4558 },
        {0x3086, 0x2A04 },
        {0x3086, 0x2A14 },
        {0x3086, 0x3DFF },
        {0x3086, 0x3DFF },
        {0x3086, 0x3DEA },
        {0x3086, 0x2A04 },
        {0x3086, 0x622A },
        {0x3086, 0x288E },
        {0x3086, 0x0036 },
        {0x3086, 0x2A08 },
        {0x3086, 0x3D64 },
        {0x3086, 0x7A3D },
        {0x3086, 0x0444 },
        {0x3086, 0x2C4B },
        {0x3086, 0x8F03 },
        {0x3086, 0x430D },
        {0x3086, 0x2D46 },
        {0x3086, 0x4316 },
        {0x3086, 0x5F16 },
        {0x3086, 0x530D },
        {0x3086, 0x1660 },
        {0x3086, 0x3E4C },
        {0x3086, 0x2904 },
        {0x3086, 0x2984 },
        {0x3086, 0x8E03 },
        {0x3086, 0x2AFC },
        {0x3086, 0x5C1D },
        {0x3086, 0x5754 },
        {0x3086, 0x495F },
        {0x3086, 0x5305 },
        {0x3086, 0x5307 },
        {0x3086, 0x4D2B },
        {0x3086, 0xF810 },
        {0x3086, 0x164C },
        {0x3086, 0x0955 },
        {0x3086, 0x562B },
        {0x3086, 0xB82B },
        {0x3086, 0x984E },
        {0x3086, 0x1129 },
        {0x3086, 0x9460 },
        {0x3086, 0x5C19 },
        {0x3086, 0x5C1B },
        {0x3086, 0x4548 },
        {0x3086, 0x4508 },
        {0x3086, 0x4588 },
        {0x3086, 0x29B6 },
        {0x3086, 0x8E01 },
        {0x3086, 0x2AF8 },
        {0x3086, 0x3E02 },
        {0x3086, 0x2AFA },
        {0x3086, 0x3F09 },
        {0x3086, 0x5C1B },
        {0x3086, 0x29B2 },
        {0x3086, 0x3F0C },
        {0x3086, 0x3E03 },
        {0x3086, 0x3E15 },
        {0x3086, 0x5C13 },
        {0x3086, 0x3F11 },
        {0x3086, 0x3E0F },
        {0x3086, 0x5F2B },
        {0x3086, 0x902A },
        {0x3086, 0xF22B },
        {0x3086, 0x803E },
        {0x3086, 0x063F },
        {0x3086, 0x0660 },
        {0x3086, 0x29A2 },
        {0x3086, 0x29A3 },
        {0x3086, 0x5F4D },
        {0x3086, 0x1C2A },
        {0x3086, 0xFA29 },
        {0x3086, 0x8345 },
        {0x3086, 0xA83E },
        {0x3086, 0x072A },
        {0x3086, 0xFB3E },
        {0x3086, 0x2945 },
        {0x3086, 0x8824 },
        {0x3086, 0x3E08 },
        {0x3086, 0x2AFA },
        {0x3086, 0x5D29 },
        {0x3086, 0x9288 },
        {0x3086, 0x102B },
        {0x3086, 0x048B },
        {0x3086, 0x1686 },
        {0x3086, 0x8D48 },
        {0x3086, 0x4D4E },
        {0x3086, 0x2B80 },
        {0x3086, 0x4C0B },
        {0x3086, 0x603F },
        {0x3086, 0x302A },
        {0x3086, 0xF23F },
        {0x3086, 0x1029 },
        {0x3086, 0x8229 },
        {0x3086, 0x8329 },
        {0x3086, 0x435C },
        {0x3086, 0x155F },
        {0x3086, 0x4D1C },
        {0x3086, 0x2AFA },
        {0x3086, 0x4558 },
        {0x3086, 0x8E00 },
        {0x3086, 0x2A98 },
        {0x3086, 0x3F0A },
        {0x3086, 0x4A0A },
        {0x3086, 0x4316 },
        {0x3086, 0x0B43 },
        {0x3086, 0x168E },
        {0x3086, 0x032A },
        {0x3086, 0x9C45 },
        {0x3086, 0x783F },
        {0x3086, 0x072A },
        {0x3086, 0x9D3E },
        {0x3086, 0x305D },
        {0x3086, 0x2944 },
        {0x3086, 0x8810 },
        {0x3086, 0x2B04 },
        {0x3086, 0x530D },
        {0x3086, 0x4558 },
        {0x3086, 0x3E08 },
        {0x3086, 0x8E01 },
        {0x3086, 0x2A98 },
        {0x3086, 0x8E00 },
        {0x3086, 0x769C },
        {0x3086, 0x779C },
        {0x3086, 0x4644 },
        {0x3086, 0x1616 },
        {0x3086, 0x907A },
        {0x3086, 0x1244 },
        {0x3086, 0x4B18 },
        {0x3086, 0x4A04 },
        {0x3086, 0x4316 },
        {0x3086, 0x0643 },
        {0x3086, 0x1605 },
        {0x3086, 0x4316 },
        {0x3086, 0x0743 },
        {0x3086, 0x1658 },
        {0x3086, 0x4316 },
        {0x3086, 0x5A43 },
        {0x3086, 0x1645 },
        {0x3086, 0x588E },
        {0x3086, 0x032A },
        {0x3086, 0x9C45 },
        {0x3086, 0x787B },
        {0x3086, 0x3F07 },
        {0x3086, 0x2A9D },
        {0x3086, 0x530D },
        {0x3086, 0x8B16 },
        {0x3086, 0x863E },
        {0x3086, 0x2345 },
        {0x3086, 0x5825 },
        {0x3086, 0x3E10 },
        {0x3086, 0x8E01 },
        {0x3086, 0x2A98 },
        {0x3086, 0x8E00 },
        {0x3086, 0x3E10 },
        {0x3086, 0x8D60 },
        {0x3086, 0x1244 },
        {0x3086, 0x4B2C },
        {0x3086, 0x2C2C},
        //BITFIELD= 0x3ED6, 0x00E0, 0x5
        //AR0230 REV1.2 Optimized Settings
        { 0x3ED6, 0x0000},
        { 0x2436, 0x000E},
        { 0x320C, 0x0180},
        { 0x320E, 0x0300},
        { 0x3210, 0x0500},
        { 0x3204, 0x0B6D},
        { 0x30FE, 0x0080},
        { 0x3ED8, 0x7B99},
        { 0x3EDC, 0x9BA8},
        { 0x3EDA, 0x9B9B},
        { 0x3092, 0x006F},
        { 0x3EEC, 0x1C04},
        { 0x30BA, 0x779C},
        { 0x3EF6, 0xA70F},
        { 0x3044, 0x0410},
        { 0x3ED0, 0xFF44},
        { 0x3ED4, 0x031F},
        { 0x30FE, 0x0080},
        { 0x3EE2, 0x8866},
        { 0x3EE4, 0x6623},
        { 0x3EE6, 0x2263},
        { 0x30E0, 0x4283},
        { 0x30F0, 0x1283},

        { 0x301A, 0x10D8}, // RESET_REGISTER
        { 0x30B0, 0x1118}, // DIGITAL_TEST
        { 0x31AC, 0x0C0C}, // DATA_FORMAT_BITS
        //Linear Mode Setup
        { 0x3082, 0x0009}, // OPERATION_MODE_CTRL
        { 0x30BA, 0x07EC}, // DIGITAL_CTRL
        { 0x3096, 0x0080}, // ROW_NOISE_ADJUST_TOP
        { 0x3098, 0x0080}, // ROW_NOISE_ADJUST_BTM
        // 2D Defect Correction
        { 0x31E0, 0x0200},
        // Motion Compensation Off
        { 0x318C, 0x0000},
        // Linear Mode Low Conversion Gain
        { 0x3176, 0x0080},
        { 0x3178, 0x0080},
        { 0x317A, 0x0080},
        { 0x317C, 0x0080},
        { 0x3060, 0x000B}, // 1.5x analog Gain Minimum analog gain for LCG
        // ADACD_Low_Conversion_Gain
        { 0x3206, 0x0B08},
        { 0x3208, 0x1E13},
        { 0x3202, 0x0080}, // large value have greater noise reduction
        { 0x3200, 0x0002},
        { 0x3100, 0x0000}, // AECTRLREG
        // Adacd disable
        { 0x3200, 0x0000},
        { 0x31D0, 0x0000}, // Companding Disable
        // ALTM Bypassed
        { 0x2400, 0x0003},
        { 0x301E, 0x00A8},
        { 0x2450, 0x0000},
        { 0x320A, 0x0080},
        // Enable Embedded Data and Stats
        { 0x3064, 0x1982}, // 0x1802 disable ; 0x1982 enable
        {GADI_VI_SENSOR_TABLE_FLAG_END, 0x00}//end flag
    },
    .pll_table =
    {
        [0] =
        {
            //For 30fps
            .pixclk = 74250000,
            .extclk = 27000000,
            .regs =
            {
                {0x302A,  0x0004},
                {0x302C,  0x0001},
                {0x302E,  0x0002},
                {0x3030,  0x002C},
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
        [1] =
        {
            //For 29.97fps
            .pixclk = 74175750,
            .extclk = 26973000,
            .regs =
            {
                {0x302A,  0x0004},
                {0x302C,  0x0001},
                {0x302E,  0x0002},
                {0x3030,  0x002C},
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
    },
    .video_fmt_table =
    {
        [0] = //1280x960@30fps hts 1800,vts 1000 1800*1000*30 = 54000000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1920, 1080,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x3002, 0x0000},   //AR0230_Y_ADDR_START,
                {0x3004, 0x0000},   //AR0230_X_ADDR_START,
                {0x3006, 0x0437},   //AR0230_Y_ADDR_END,
                {0x3008, 0x0787},   //AR0230_X_ADDR_END,
                {0x300A, 0x0452},   //AR0230_FRAME_LENGTH_LINES,
                {0x300C, 0x045E},   //AR0230_LINE_LENGTH_PCK,
                {0x3012, 0x03F4},   //AR0230_COARSE_INTEGRATION_TIME,
                {0x3012, 0x0416},   //AR0230_COARSE_INTEGRATION_TIME,   ????  0x3014
                {0x30A2, 0x0001},   //AR0230_X_ODD_INC,
                {0x30A6, 0x0001},   //AR0230_Y_ODD_INC,
                {0x3040, 0x0000},   //AR0230_READ_MODE,
                {0x31AE, 0x0301},   //AR0230_SERIAL_FORMAT,
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 1920,
                .height     = 1080,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_DVP,
                .bits       = GADI_VIDEO_BITS_12,
                .ratio      = GADI_VIDEO_RATIO_16_9,
                .srm        = 0,
                .pll_index  = 0,
                .max_fps    = GADI_VIDEO_FPS(30),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(25),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_960P_30,
            },
            .video_info_table =
            {
                .def_start_x    = 0,
                .def_start_y    = 0,
                .def_width      = 1920,
                .def_height     = 1080,
                .sync_start     = (0 + 0),
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_RG,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_RG,
                    GADI_VI_BAYER_PATTERN_RG,
                    GADI_VI_BAYER_PATTERN_RG,
                    GADI_VI_BAYER_PATTERN_RG,
                },
            },
            .update_after_vsync_start   = 30,
            .update_after_vsync_end     = 33,
        },
        [1] = //1280x960@25fps hts 1800,vts 1000 1800*1000*25 = 45000000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x3002, 0x00B4},   //AR0230_Y_ADDR_START,
                {0x3004, 0x0140},   //AR0230_X_ADDR_START,
                {0x3006, 0x0383},   //AR0230_Y_ADDR_END,
                {0x3008, 0x063F},   //AR0230_X_ADDR_END,
                {0x300A, 0x0452},   //AR0230_FRAME_LENGTH_LINES,
                {0x300C, 0x045E},   //AR0230_LINE_LENGTH_PCK,
                {0x3012, 0x03F4},   //AR0230_COARSE_INTEGRATION_TIME,
                {0x3012, 0x0416},   //AR0230_COARSE_INTEGRATION_TIME,   ????  0x3014
                {0x30A2, 0x0001},   //AR0230_X_ODD_INC,
                {0x30A6, 0x0001},   //AR0230_Y_ODD_INC,
                {0x3040, 0x0000},   //AR0230_READ_MODE,
                {0x31AE, 0x0301},   //AR0230_SERIAL_FORMAT,
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
                .auto_fps   = GADI_VIDEO_FPS(25),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_960P_25,
            },
            .video_info_table =
            {
                .def_start_x    = 0,
                .def_start_y    = 0,
                .def_width      = 1280,
                .def_height     = 720,
                .sync_start     = (0 + 0),
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_RG,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_RG,
                    GADI_VI_BAYER_PATTERN_RG,
                    GADI_VI_BAYER_PATTERN_RG,
                    GADI_VI_BAYER_PATTERN_RG,
                },
            },
            .update_after_vsync_start   = 30,
            .update_after_vsync_end     = 33,
        },
    },
    .mirror_table =
    {
#define AR0230_HORIZ_MIRROR     0x4000
#define AR0230_VERT_FLIP        0x8000
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY_VERTICALLY] =
        {
            {0x3040, AR0230_HORIZ_MIRROR | AR0230_VERT_FLIP, AR0230_HORIZ_MIRROR | AR0230_VERT_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY] =
        {
            {0x3040, AR0230_HORIZ_MIRROR, AR0230_HORIZ_MIRROR | AR0230_VERT_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_VERTICALLY] =
        {
            {0x3040, AR0230_VERT_FLIP, AR0230_HORIZ_MIRROR | AR0230_VERT_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_NONE] =
        {
            {0x3040, 0x00, AR0230_HORIZ_MIRROR | AR0230_VERT_FLIP},
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
    .shs_fix        = 0,
    .shs_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_SET,
    .agc_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_SET,
    .vmax_reg_update_mode       = GADI_SENSOR_UPDATE_REG_BY_SET,
    .mirror_reg_update_mode     = GADI_SENSOR_UPDATE_REG_BY_SET,
    .shs_additional_use         = 0,
    .shs_additional_percent     = 0,
    .hmax_reg =
    {
        {0x300C, 0x0000, 0xFFFF, 0x00, 0x00, 0}, // AR0230_LINE_LENGTH_PCK
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .vmax_reg =
    {
        {0x300A, 0x0000, 0xFFFF, 0x00, 0x00, 0}, // AR0230_FRAME_LENGTH_LINES
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .vmax_wadd_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    .shs_reg =
    {
        {0x3012, 0x0000, 0xFFFF, 0x00, 0x00, 0}, // AR0230_COARSE_INTEGRATION_TIME
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .shs_additional_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    .cal_gain_mode              = GADI_SENSOR_CAL_GAIN_MODE2,
    .max_agc_index = 241,
    .max_again_index = 0,
    .cur_again_index = 0,
    .agc_info =
    {
        .db_max     = 0x2A000000,    // 42dB
        .db_min     = 0x00000000,    // 0dB
        .db_step    = 0x000C0000,    // 0.046875db;0x00180000:0.09375db
    },
    .gain_reg =
    {
        //{0x3100, 0x00}, // AR0230_AE_CTRL_REG
        //{0x30B0, 0x00}, // AR0230_DIGITAL_TEST
        {0x3060, 0x00}, // AR0230_GAIN_COL_REG_AGAIN
        {0x305E, 0x00}, // AR0230_GLOBAL_GAIN
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .gain_table =
    {
        {896*0xC0000, {0x3C , 0x400,},},        /*index: 128, 42.1236dB */
        {895*0xC0000, {0x3C , 0x3FA,},},        /*index: 129, 42.0766dB */
        {894*0xC0000, {0x3C , 0x3F5,},},        /*index: 130, 42.0295dB */
        {893*0xC0000, {0x3C , 0x3EF,},},        /*index: 131, 41.9825dB */
        {892*0xC0000, {0x3C , 0x3EA,},},        /*index: 132, 41.9355dB */
        {891*0xC0000, {0x3C , 0x3E5,},},        /*index: 133, 41.8884dB */
        {890*0xC0000, {0x3C , 0x3DF,},},        /*index: 134, 41.8414dB */
        {889*0xC0000, {0x3C , 0x3DA,},},        /*index: 135, 41.7943dB */
        {888*0xC0000, {0x3C , 0x3D5,},},        /*index: 136, 41.7473dB */
        {887*0xC0000, {0x3C , 0x3CF,},},        /*index: 137, 41.7003dB */
        {886*0xC0000, {0x3C , 0x3CA,},},        /*index: 138, 41.6532dB */
        {885*0xC0000, {0x3C , 0x3C5,},},        /*index: 139, 41.6062dB */
        {884*0xC0000, {0x3C , 0x3C0,},},        /*index: 140, 41.5592dB */
        {883*0xC0000, {0x3C , 0x3BA,},},        /*index: 141, 41.5121dB */
        {882*0xC0000, {0x3C , 0x3B5,},},        /*index: 142, 41.4651dB */
        {881*0xC0000, {0x3C , 0x3B0,},},        /*index: 143, 41.4181dB */
        {880*0xC0000, {0x3C , 0x3AB,},},        /*index: 144, 41.371dB */
        {879*0xC0000, {0x3C , 0x3A6,},},        /*index: 145, 41.324dB */
        {878*0xC0000, {0x3C , 0x3A1,},},        /*index: 146, 41.277dB */
        {877*0xC0000, {0x3C , 0x39C,},},        /*index: 147, 41.2299dB */
        {876*0xC0000, {0x3C , 0x397,},},        /*index: 148, 41.1829dB */
        {875*0xC0000, {0x3C , 0x392,},},        /*index: 149, 41.1358dB */
        {874*0xC0000, {0x3C , 0x38D,},},        /*index: 150, 41.0888dB */
        {873*0xC0000, {0x3C , 0x388,},},        /*index: 151, 41.0418dB */
        {872*0xC0000, {0x3C , 0x383,},},        /*index: 152, 40.9947dB */
        {871*0xC0000, {0x3C , 0x37E,},},        /*index: 153, 40.9477dB */
        {870*0xC0000, {0x3C , 0x37A,},},        /*index: 154, 40.9007dB */
        {869*0xC0000, {0x3C , 0x375,},},        /*index: 155, 40.8536dB */
        {868*0xC0000, {0x3C , 0x370,},},        /*index: 156, 40.8066dB */
        {867*0xC0000, {0x3C , 0x36B,},},        /*index: 157, 40.7596dB */
        {866*0xC0000, {0x3C , 0x366,},},        /*index: 158, 40.7125dB */
        {865*0xC0000, {0x3C , 0x362,},},        /*index: 159, 40.6655dB */
        {864*0xC0000, {0x3C , 0x35D,},},        /*index: 160, 40.6184dB */
        {863*0xC0000, {0x3C , 0x358,},},        /*index: 161, 40.5714dB */
        {862*0xC0000, {0x3C , 0x354,},},        /*index: 162, 40.5244dB */
        {861*0xC0000, {0x3C , 0x34F,},},        /*index: 163, 40.4773dB */
        {860*0xC0000, {0x3C , 0x34B,},},        /*index: 164, 40.4303dB */
        {859*0xC0000, {0x3C , 0x346,},},        /*index: 165, 40.3833dB */
        {858*0xC0000, {0x3C , 0x342,},},        /*index: 166, 40.3362dB */
        {857*0xC0000, {0x3C , 0x33D,},},        /*index: 167, 40.2892dB */
        {856*0xC0000, {0x3C , 0x339,},},        /*index: 168, 40.2422dB */
        {855*0xC0000, {0x3C , 0x334,},},        /*index: 169, 40.1951dB */
        {854*0xC0000, {0x3C , 0x330,},},        /*index: 170, 40.1481dB */
        {853*0xC0000, {0x3C , 0x32B,},},        /*index: 171, 40.1011dB */
        {852*0xC0000, {0x3C , 0x327,},},        /*index: 172, 40.054dB */
        {851*0xC0000, {0x3C , 0x323,},},        /*index: 173, 40.007dB */
        {850*0xC0000, {0x3C , 0x31E,},},        /*index: 174, 39.9599dB */
        {849*0xC0000, {0x3C , 0x31A,},},        /*index: 175, 39.9129dB */
        {848*0xC0000, {0x3C , 0x316,},},        /*index: 176, 39.8659dB */
        {847*0xC0000, {0x3C , 0x311,},},        /*index: 177, 39.8188dB */
        {846*0xC0000, {0x3C , 0x30D,},},        /*index: 178, 39.7718dB */
        {845*0xC0000, {0x3C , 0x309,},},        /*index: 179, 39.7248dB */
        {844*0xC0000, {0x3C , 0x305,},},        /*index: 180, 39.6777dB */
        {843*0xC0000, {0x3C , 0x301,},},        /*index: 181, 39.6307dB */
        {842*0xC0000, {0x3C , 0x2FC,},},        /*index: 182, 39.5837dB */
        {841*0xC0000, {0x3C , 0x2F8,},},        /*index: 183, 39.5366dB */
        {840*0xC0000, {0x3C , 0x2F4,},},        /*index: 184, 39.4896dB */
        {839*0xC0000, {0x3C , 0x2F0,},},        /*index: 185, 39.4426dB */
        {838*0xC0000, {0x3C , 0x2EC,},},        /*index: 186, 39.3955dB */
        {837*0xC0000, {0x3C , 0x2E8,},},        /*index: 187, 39.3485dB */
        {836*0xC0000, {0x3C , 0x2E4,},},        /*index: 188, 39.3014dB */
        {835*0xC0000, {0x3C , 0x2E0,},},        /*index: 189, 39.2544dB */
        {834*0xC0000, {0x3C , 0x2DC,},},        /*index: 190, 39.2074dB */
        {833*0xC0000, {0x3C , 0x2D8,},},        /*index: 191, 39.1603dB */
        {832*0xC0000, {0x3C , 0x2D4,},},        /*index: 192, 39.1133dB */
        {831*0xC0000, {0x3C , 0x2D0,},},        /*index: 193, 39.0663dB */
        {830*0xC0000, {0x3C , 0x2CC,},},        /*index: 194, 39.0192dB */
        {829*0xC0000, {0x3C , 0x2C8,},},        /*index: 195, 38.9722dB */
        {828*0xC0000, {0x3C , 0x2C5,},},        /*index: 196, 38.9252dB */
        {827*0xC0000, {0x3C , 0x2C1,},},        /*index: 197, 38.8781dB */
        {826*0xC0000, {0x3C , 0x2BD,},},        /*index: 198, 38.8311dB */
        {825*0xC0000, {0x3C , 0x2B9,},},        /*index: 199, 38.784dB */
        {824*0xC0000, {0x3C , 0x2B5,},},        /*index: 200, 38.737dB */
        {823*0xC0000, {0x3C , 0x2B2,},},        /*index: 201, 38.69dB */
        {822*0xC0000, {0x3C , 0x2AE,},},        /*index: 202, 38.6429dB */
        {821*0xC0000, {0x3C , 0x2AA,},},        /*index: 203, 38.5959dB */
        {820*0xC0000, {0x3C , 0x2A7,},},        /*index: 204, 38.5489dB */
        {819*0xC0000, {0x3C , 0x2A3,},},        /*index: 205, 38.5018dB */
        {818*0xC0000, {0x3C , 0x29F,},},        /*index: 206, 38.4548dB */
        {817*0xC0000, {0x3C , 0x29C,},},        /*index: 207, 38.4078dB */
        {816*0xC0000, {0x3C , 0x298,},},        /*index: 208, 38.3607dB */
        {815*0xC0000, {0x3C , 0x294,},},        /*index: 209, 38.3137dB */
        {814*0xC0000, {0x3C , 0x291,},},        /*index: 210, 38.2667dB */
        {813*0xC0000, {0x3C , 0x28D,},},        /*index: 211, 38.2196dB */
        {812*0xC0000, {0x3C , 0x28A,},},        /*index: 212, 38.1726dB */
        {811*0xC0000, {0x3C , 0x286,},},        /*index: 213, 38.1255dB */
        {810*0xC0000, {0x3C , 0x283,},},        /*index: 214, 38.0785dB */
        {809*0xC0000, {0x3C , 0x27F,},},        /*index: 215, 38.0315dB */
        {808*0xC0000, {0x3C , 0x27C,},},        /*index: 216, 37.9844dB */
        {807*0xC0000, {0x3C , 0x278,},},        /*index: 217, 37.9374dB */
        {806*0xC0000, {0x3C , 0x275,},},        /*index: 218, 37.8904dB */
        {805*0xC0000, {0x3C , 0x272,},},        /*index: 219, 37.8433dB */
        {804*0xC0000, {0x3C , 0x26E,},},        /*index: 220, 37.7963dB */
        {803*0xC0000, {0x3C , 0x26B,},},        /*index: 221, 37.7493dB */
        {802*0xC0000, {0x3C , 0x268,},},        /*index: 222, 37.7022dB */
        {801*0xC0000, {0x3C , 0x264,},},        /*index: 223, 37.6552dB */
        {800*0xC0000, {0x3C , 0x261,},},        /*index: 224, 37.6081dB */
        {799*0xC0000, {0x3C , 0x25E,},},        /*index: 225, 37.5611dB */
        {798*0xC0000, {0x3C , 0x25A,},},        /*index: 226, 37.5141dB */
        {797*0xC0000, {0x3C , 0x257,},},        /*index: 227, 37.467dB */
        {796*0xC0000, {0x3C , 0x254,},},        /*index: 228, 37.42dB */
        {795*0xC0000, {0x3C , 0x251,},},        /*index: 229, 37.373dB */
        {794*0xC0000, {0x3C , 0x24D,},},        /*index: 230, 37.3259dB */
        {793*0xC0000, {0x3C , 0x24A,},},        /*index: 231, 37.2789dB */
        {792*0xC0000, {0x3C , 0x247,},},        /*index: 232, 37.2319dB */
        {791*0xC0000, {0x3C , 0x244,},},        /*index: 233, 37.1848dB */
        {790*0xC0000, {0x3C , 0x241,},},        /*index: 234, 37.1378dB */
        {789*0xC0000, {0x3C , 0x23E,},},        /*index: 235, 37.0908dB */
        {788*0xC0000, {0x3C , 0x23B,},},        /*index: 236, 36.0437dB */
        {787*0xC0000, {0x3C , 0x237,},},        /*index: 237, 36.9967dB */
        {786*0xC0000, {0x3C , 0x234,},},        /*index: 238, 36.9496dB */
        {785*0xC0000, {0x3C , 0x231,},},        /*index: 239, 36.9026dB */
        {784*0xC0000, {0x3C , 0x22E,},},        /*index: 240, 36.8556dB */
        {783*0xC0000, {0x3C , 0x22B,},},        /*index: 241, 36.8085dB */
        {782*0xC0000, {0x3C , 0x228,},},        /*index: 242, 36.7615dB */
        {781*0xC0000, {0x3C , 0x225,},},        /*index: 243, 36.7145dB */
        {780*0xC0000, {0x3C , 0x222,},},        /*index: 244, 36.6674dB */
        {779*0xC0000, {0x3C , 0x21F,},},        /*index: 245, 36.6204dB */
        {778*0xC0000, {0x3C , 0x21C,},},        /*index: 246, 36.5734dB */
        {777*0xC0000, {0x3C , 0x21A,},},        /*index: 247, 36.5263dB */
        {776*0xC0000, {0x3C , 0x217,},},        /*index: 248, 36.4793dB */
        {775*0xC0000, {0x3C , 0x214,},},        /*index: 249, 36.4323dB */
        {774*0xC0000, {0x3C , 0x211,},},        /*index: 250, 36.3852dB */
        {773*0xC0000, {0x3C , 0x20E,},},        /*index: 251, 36.3382dB */
        {772*0xC0000, {0x3C , 0x20B,},},        /*index: 252, 36.2911dB */
        {771*0xC0000, {0x3C , 0x208,},},        /*index: 253, 36.2441dB */
        {770*0xC0000, {0x3C , 0x206,},},        /*index: 254, 36.1971dB */
        {769*0xC0000, {0x3C , 0x203,},},        /*index: 255, 36.15dB */
        {768*0xC0000, {0x3C , 0x200,},},        /*index: 256, 36.103dB */
        {767*0xC0000, {0x3C , 0x1FD,},},        /*index: 257, 36.056dB */
        {766*0xC0000, {0x3C , 0x1FA,},},        /*index: 258, 36.0089dB */
        {765*0xC0000, {0x3C , 0x1F8,},},        /*index: 259, 35.9619dB */
        {764*0xC0000, {0x3C , 0x1F5,},},        /*index: 260, 35.9149dB */
        {763*0xC0000, {0x3C , 0x1F2,},},        /*index: 261, 35.8678dB */
        {762*0xC0000, {0x3C , 0x1F0,},},        /*index: 262, 35.8208dB */
        {761*0xC0000, {0x3C , 0x1ED,},},        /*index: 263, 35.7737dB */
        {760*0xC0000, {0x3C , 0x1EA,},},        /*index: 264, 35.7267dB */
        {759*0xC0000, {0x3C , 0x1E8,},},        /*index: 265, 35.6797dB */
        {758*0xC0000, {0x3C , 0x1E5,},},        /*index: 266, 35.6326dB */
        {757*0xC0000, {0x3C , 0x1E2,},},        /*index: 267, 35.5856dB */
        {756*0xC0000, {0x3C , 0x1E0,},},        /*index: 268, 35.5386dB */
        {755*0xC0000, {0x3C , 0x1DD,},},        /*index: 269, 35.4915dB */
        {754*0xC0000, {0x3C , 0x1DB,},},        /*index: 270, 35.4445dB */
        {753*0xC0000, {0x3C , 0x1D8,},},        /*index: 271, 35.3975dB */
        {752*0xC0000, {0x3C , 0x1D6,},},        /*index: 272, 35.3504dB */
        {751*0xC0000, {0x3C , 0x1D3,},},        /*index: 273, 35.3034dB */
        {750*0xC0000, {0x3C , 0x1D0,},},        /*index: 274, 35.2564dB */
        {749*0xC0000, {0x3C , 0x1CE,},},        /*index: 275, 35.2093dB */
        {748*0xC0000, {0x3C , 0x1CB,},},        /*index: 276, 35.1623dB */
        {747*0xC0000, {0x3C , 0x1C9,},},        /*index: 277, 35.1152dB */
        {746*0xC0000, {0x3C , 0x1C6,},},        /*index: 278, 35.0682dB */
        {745*0xC0000, {0x3C , 0x1C4,},},        /*index: 279, 35.0212dB */
        {744*0xC0000, {0x3C , 0x1C2,},},        /*index: 280, 34.9741dB */
        {743*0xC0000, {0x3C , 0x1BF,},},        /*index: 281, 34.9271dB */
        {742*0xC0000, {0x3C , 0x1BD,},},        /*index: 282, 34.8801dB */
        {741*0xC0000, {0x3C , 0x1BA,},},        /*index: 283, 34.833dB */
        {740*0xC0000, {0x3C , 0x1B8,},},        /*index: 284, 34.786dB */
        {739*0xC0000, {0x3C , 0x1B6,},},        /*index: 285, 34.739dB */
        {738*0xC0000, {0x3C , 0x1B3,},},        /*index: 286, 34.6919dB */
        {737*0xC0000, {0x3C , 0x1B1,},},        /*index: 287, 34.6449dB */
        {736*0xC0000, {0x3C , 0x1AF,},},        /*index: 288, 34.5978dB */
        {735*0xC0000, {0x3C , 0x1AC,},},        /*index: 289, 34.5508dB */
        {734*0xC0000, {0x3C , 0x1AA,},},        /*index: 290, 34.5038dB */
        {733*0xC0000, {0x3C , 0x1A8,},},        /*index: 291, 34.4567dB */
        {732*0xC0000, {0x3C , 0x1A5,},},        /*index: 292, 34.4097dB */
        {731*0xC0000, {0x3C , 0x1A3,},},        /*index: 293, 34.3627dB */
        {730*0xC0000, {0x3C , 0x1A1,},},        /*index: 294, 34.3156dB */
        {729*0xC0000, {0x3C , 0x19F,},},        /*index: 295, 34.2686dB */
        {728*0xC0000, {0x3C , 0x19C,},},        /*index: 296, 34.2216dB */
        {727*0xC0000, {0x3C , 0x19A,},},        /*index: 297, 34.1745dB */
        {726*0xC0000, {0x3C , 0x198,},},        /*index: 298, 34.1275dB */
        {725*0xC0000, {0x3C , 0x196,},},        /*index: 299, 34.0805dB */
        {724*0xC0000, {0x3C , 0x193,},},        /*index: 300, 34.0334dB */
        {723*0xC0000, {0x3C , 0x191,},},        /*index: 301, 33.9864dB */
        {722*0xC0000, {0x3C , 0x18F,},},        /*index: 302, 33.9393dB */
        {721*0xC0000, {0x3C , 0x18D,},},        /*index: 303, 33.8923dB */
        {720*0xC0000, {0x3C , 0x18B,},},        /*index: 304, 33.8453dB */
        {719*0xC0000, {0x3C , 0x189,},},        /*index: 305, 33.7982dB */
        {718*0xC0000, {0x3C , 0x187,},},        /*index: 306, 33.7512dB */
        {717*0xC0000, {0x3C , 0x184,},},        /*index: 307, 33.7042dB */
        {716*0xC0000, {0x3C , 0x182,},},        /*index: 308, 33.6571dB */
        {715*0xC0000, {0x3C , 0x180,},},        /*index: 309, 33.6101dB */
        {714*0xC0000, {0x3C , 0x17E,},},        /*index: 310, 33.5631dB */
        {713*0xC0000, {0x3C , 0x17C,},},        /*index: 311, 33.516dB */
        {712*0xC0000, {0x3C , 0x17A,},},        /*index: 312, 33.469dB */
        {711*0xC0000, {0x3C , 0x178,},},        /*index: 313, 33.422dB */
        {710*0xC0000, {0x3C , 0x176,},},        /*index: 314, 33.3749dB */
        {709*0xC0000, {0x3C , 0x174,},},        /*index: 315, 33.3279dB */
        {708*0xC0000, {0x3C , 0x172,},},        /*index: 316, 33.2808dB */
        {707*0xC0000, {0x3C , 0x170,},},        /*index: 317, 33.2338dB */
        {706*0xC0000, {0x3C , 0x16E,},},        /*index: 318, 33.1868dB */
        {705*0xC0000, {0x3C , 0x16C,},},        /*index: 319, 33.1397dB */
        {704*0xC0000, {0x3C , 0x16A,},},        /*index: 320, 33.0927dB */
        {703*0xC0000, {0x3C , 0x168,},},        /*index: 321, 33.0457dB */
        {702*0xC0000, {0x3C , 0x166,},},        /*index: 322, 32.9986dB */
        {701*0xC0000, {0x3C , 0x164,},},        /*index: 323, 32.9516dB */
        {700*0xC0000, {0x3C , 0x162,},},        /*index: 324, 32.9046dB */
        {699*0xC0000, {0x3C , 0x160,},},        /*index: 325, 32.8575dB */
        {698*0xC0000, {0x3C , 0x15E,},},        /*index: 326, 32.8105dB */
        {697*0xC0000, {0x3C , 0x15D,},},        /*index: 327, 32.7634dB */
        {696*0xC0000, {0x3C , 0x15B,},},        /*index: 328, 32.7164dB */
        {695*0xC0000, {0x3C , 0x159,},},        /*index: 329, 32.6694dB */
        {694*0xC0000, {0x3C , 0x157,},},        /*index: 330, 32.6223dB */
        {693*0xC0000, {0x3C , 0x155,},},        /*index: 331, 32.5753dB */
        {692*0xC0000, {0x3C , 0x153,},},        /*index: 332, 32.5283dB */
        {691*0xC0000, {0x3C , 0x151,},},        /*index: 333, 32.4812dB */
        {690*0xC0000, {0x3C , 0x150,},},        /*index: 334, 32.4342dB */
        {689*0xC0000, {0x3C , 0x14E,},},        /*index: 335, 32.3872dB */
        {688*0xC0000, {0x3C , 0x14C,},},        /*index: 336, 32.3401dB */
        {687*0xC0000, {0x3C , 0x14A,},},        /*index: 337, 32.2931dB */
        {686*0xC0000, {0x3C , 0x148,},},        /*index: 338, 32.2461dB */
        {685*0xC0000, {0x3C , 0x147,},},        /*index: 339, 32.199dB */
        {684*0xC0000, {0x3C , 0x145,},},        /*index: 340, 32.152dB */
        {683*0xC0000, {0x3C , 0x143,},},        /*index: 341, 32.1049dB */
        {682*0xC0000, {0x3C , 0x141,},},        /*index: 342, 32.0579dB */
        {681*0xC0000, {0x3C , 0x140,},},        /*index: 343, 32.0109dB */
        {680*0xC0000, {0x3C , 0x13E,},},        /*index: 344, 31.9638dB */
        {679*0xC0000, {0x3C , 0x13C,},},        /*index: 345, 31.9168dB */
        {678*0xC0000, {0x3C , 0x13A,},},        /*index: 346, 31.8698dB */
        {677*0xC0000, {0x3C , 0x139,},},        /*index: 347, 31.8227dB */
        {676*0xC0000, {0x3C , 0x137,},},        /*index: 348, 31.7757dB */
        {675*0xC0000, {0x3C , 0x135,},},        /*index: 349, 31.7287dB */
        {674*0xC0000, {0x3C , 0x134,},},        /*index: 350, 31.6816dB */
        {673*0xC0000, {0x3C , 0x132,},},        /*index: 351, 31.6346dB */
        {672*0xC0000, {0x3C , 0x130,},},        /*index: 352, 31.5875dB */
        {671*0xC0000, {0x3C , 0x12F,},},        /*index: 353, 31.5405dB */
        {670*0xC0000, {0x3C , 0x12D,},},        /*index: 354, 31.4935dB */
        {669*0xC0000, {0x3C , 0x12C,},},        /*index: 355, 31.4464dB */
        {668*0xC0000, {0x3C , 0x12A,},},        /*index: 356, 31.3994dB */
        {667*0xC0000, {0x3C , 0x128,},},        /*index: 357, 31.3524dB */
        {666*0xC0000, {0x3C , 0x127,},},        /*index: 358, 31.3053dB */
        {665*0xC0000, {0x3C , 0x125,},},        /*index: 359, 31.2583dB */
        {664*0xC0000, {0x3C , 0x124,},},        /*index: 360, 31.2113dB */
        {663*0xC0000, {0x3C , 0x122,},},        /*index: 361, 31.1642dB */
        {662*0xC0000, {0x3C , 0x120,},},        /*index: 362, 31.1172dB */
        {661*0xC0000, {0x3C , 0x11F,},},        /*index: 363, 31.0702dB */
        {660*0xC0000, {0x3C , 0x11D,},},        /*index: 364, 31.0231dB */
        {659*0xC0000, {0x3C , 0x11C,},},        /*index: 365, 30.9761dB */
        {658*0xC0000, {0x3C , 0x11A,},},        /*index: 366, 30.929dB */
        {657*0xC0000, {0x3C , 0x119,},},        /*index: 367, 30.882dB */
        {656*0xC0000, {0x3C , 0x117,},},        /*index: 368, 30.835dB */
        {655*0xC0000, {0x3C , 0x116,},},        /*index: 369, 30.7879dB */
        {654*0xC0000, {0x3C , 0x114,},},        /*index: 370, 30.7409dB */
        {653*0xC0000, {0x3C , 0x113,},},        /*index: 371, 30.6939dB */
        {652*0xC0000, {0x3C , 0x111,},},        /*index: 372, 30.6468dB */
        {651*0xC0000, {0x3C , 0x110,},},        /*index: 373, 30.5998dB */
        {650*0xC0000, {0x3C , 0x10E,},},        /*index: 374, 30.5528dB */
        {649*0xC0000, {0x3C , 0x10D,},},        /*index: 375, 30.5057dB */
        {648*0xC0000, {0x3C , 0x10B,},},        /*index: 376, 30.4587dB */
        {647*0xC0000, {0x3C , 0x10A,},},        /*index: 377, 30.4117dB */
        {646*0xC0000, {0x3C , 0x108,},},        /*index: 378, 30.3646dB */
        {645*0xC0000, {0x3C , 0x107,},},        /*index: 379, 30.3176dB */
        {644*0xC0000, {0x3C , 0x106,},},        /*index: 380, 30.2705dB */
        {643*0xC0000, {0x3C , 0x104,},},        /*index: 381, 30.2235dB */
        {642*0xC0000, {0x3C , 0x103,},},        /*index: 382, 30.1765dB */
        {641*0xC0000, {0x3C , 0x101,},},        /*index: 383, 30.1294dB */
        {640*0xC0000, {0x3C , 0x100,},},        /*index: 384, 30.0824dB */
        {639*0xC0000, {0x3C , 0xFF, },},        /*index: 385, 30.0354dB */
        {638*0xC0000, {0x3C , 0xFD, },},        /*index: 386, 29.9883dB */
        {637*0xC0000, {0x3C , 0xFC, },},        /*index: 387, 29.9413dB */
        {636*0xC0000, {0x3C , 0xFB, },},        /*index: 388, 29.8943dB */
        {635*0xC0000, {0x3C , 0xF9, },},        /*index: 389, 29.8472dB */
        {634*0xC0000, {0x3C , 0xF8, },},        /*index: 390, 29.8002dB */
        {633*0xC0000, {0x3C , 0xF6, },},        /*index: 391, 29.7531dB */
        {632*0xC0000, {0x3C , 0xF5, },},        /*index: 392, 29.7061dB */
        {631*0xC0000, {0x3C , 0xF4, },},        /*index: 393, 29.6591dB */
        {630*0xC0000, {0x3C , 0xF3, },},        /*index: 394, 29.612dB */
        {629*0xC0000, {0x3C , 0xF1, },},        /*index: 395, 29.565dB */
        {628*0xC0000, {0x3C , 0xF0, },},        /*index: 396, 29.518dB */
        {627*0xC0000, {0x3C , 0xEF, },},        /*index: 397, 29.4709dB */
        {626*0xC0000, {0x3C , 0xED, },},        /*index: 398, 29.4239dB */
        {625*0xC0000, {0x3C , 0xEC, },},        /*index: 399, 29.3769dB */
        {624*0xC0000, {0x3C , 0xEB, },},        /*index: 400, 29.3298dB */
        {623*0xC0000, {0x3C , 0xE9, },},        /*index: 401, 29.2828dB */
        {622*0xC0000, {0x3C , 0xE8, },},        /*index: 402, 29.2358dB */
        {621*0xC0000, {0x3C , 0xE7, },},        /*index: 403, 29.1887dB */
        {620*0xC0000, {0x3C , 0xE6, },},        /*index: 404, 29.1417dB */
        {619*0xC0000, {0x3C , 0xE4, },},        /*index: 405, 29.0946dB */
        {618*0xC0000, {0x3C , 0xE3, },},        /*index: 406, 29.0476dB */
        {617*0xC0000, {0x3C , 0xE2, },},        /*index: 407, 29.0006dB */
        {616*0xC0000, {0x3C , 0xE1, },},        /*index: 408, 28.9535dB */
        {615*0xC0000, {0x3C , 0xE0, },},        /*index: 409, 28.9065dB */
        {614*0xC0000, {0x3C , 0xDE, },},        /*index: 410, 28.8595dB */
        {613*0xC0000, {0x3C , 0xDD, },},        /*index: 411, 28.8124dB */
        {612*0xC0000, {0x3C , 0xDC, },},        /*index: 412, 28.7654dB */
        {611*0xC0000, {0x3C , 0xDB, },},        /*index: 413, 28.7184dB */
        {610*0xC0000, {0x3C , 0xDA, },},        /*index: 414, 28.6713dB */
        {609*0xC0000, {0x3C , 0xD8, },},        /*index: 415, 28.6243dB */
        {608*0xC0000, {0x3C , 0xD7, },},        /*index: 416, 28.5772dB */
        {607*0xC0000, {0x3C , 0xD6, },},        /*index: 417, 28.5302dB */
        {606*0xC0000, {0x3C , 0xD5, },},        /*index: 418, 28.4832dB */
        {605*0xC0000, {0x3C , 0xD4, },},        /*index: 419, 28.4361dB */
        {604*0xC0000, {0x3C , 0xD3, },},        /*index: 420, 28.3891dB */
        {603*0xC0000, {0x3C , 0xD2, },},        /*index: 421, 28.3421dB */
        {602*0xC0000, {0x3C , 0xD0, },},        /*index: 422, 28.295dB */
        {601*0xC0000, {0x3C , 0xCF, },},        /*index: 423, 28.248dB */
        {600*0xC0000, {0x3C , 0xCE, },},        /*index: 424, 28.201dB */
        {599*0xC0000, {0x3C , 0xCD, },},        /*index: 425, 28.1539dB */
        {598*0xC0000, {0x3C , 0xCC, },},        /*index: 426, 28.1069dB */
        {597*0xC0000, {0x3C , 0xCB, },},        /*index: 427, 28.0599dB */
        {596*0xC0000, {0x3C , 0xCA, },},        /*index: 428, 28.0128dB */
        {595*0xC0000, {0x3C , 0xC9, },},        /*index: 429, 27.9658dB */
        {594*0xC0000, {0x3C , 0xC8, },},        /*index: 430, 27.9187dB */
        {593*0xC0000, {0x3C , 0xC6, },},        /*index: 431, 27.8717dB */
        {592*0xC0000, {0x3C , 0xC5, },},        /*index: 432, 27.8247dB */
        {591*0xC0000, {0x3C , 0xC4, },},        /*index: 433, 27.7776dB */
        {590*0xC0000, {0x3C , 0xC3, },},        /*index: 434, 27.7306dB */
        {589*0xC0000, {0x3C , 0xC2, },},        /*index: 435, 27.6836dB */
        {588*0xC0000, {0x3C , 0xC1, },},        /*index: 436, 27.6365dB */
        {587*0xC0000, {0x3C , 0xC0, },},        /*index: 437, 27.5895dB */
        {586*0xC0000, {0x3C , 0xBF, },},        /*index: 438, 27.5425dB */
        {585*0xC0000, {0x3C , 0xBE, },},        /*index: 439, 27.4954dB */
        {584*0xC0000, {0x3C , 0xBD, },},        /*index: 440, 27.4484dB */
        {583*0xC0000, {0x3C , 0xBC, },},        /*index: 441, 27.4014dB */
        {582*0xC0000, {0x3C , 0xBB, },},        /*index: 442, 27.3543dB */
        {581*0xC0000, {0x3C , 0xBA, },},        /*index: 443, 27.3073dB */
        {580*0xC0000, {0x3C , 0xB9, },},        /*index: 444, 27.2602dB */
        {579*0xC0000, {0x3C , 0xB8, },},        /*index: 445, 27.2132dB */
        {578*0xC0000, {0x3C , 0xB7, },},        /*index: 446, 27.1662dB */
        {577*0xC0000, {0x3C , 0xB6, },},        /*index: 447, 27.1191dB */
        {576*0xC0000, {0x3C , 0xB5, },},        /*index: 448, 27.0721dB */
        {575*0xC0000, {0x3C , 0xB4, },},        /*index: 449, 27.0251dB */
        {574*0xC0000, {0x3C , 0xB3, },},        /*index: 450, 26.978dB */
        {573*0xC0000, {0x3C , 0xB2, },},        /*index: 451, 26.931dB */
        {572*0xC0000, {0x3C , 0xB1, },},        /*index: 452, 26.884dB */
        {571*0xC0000, {0x3C , 0xB0, },},        /*index: 453, 26.8369dB */
        {570*0xC0000, {0x3C , 0xAF, },},        /*index: 454, 26.7899dB */
        {569*0xC0000, {0x3C , 0xAE, },},        /*index: 455, 26.7428dB */
        {568*0xC0000, {0x3C , 0xAD, },},        /*index: 456, 26.6958dB */
        {567*0xC0000, {0x3C , 0xAC, },},        /*index: 457, 26.6488dB */
        {566*0xC0000, {0x3C , 0xAB, },},        /*index: 458, 26.6017dB */
        {565*0xC0000, {0x3C , 0xAB, },},        /*index: 459, 26.5547dB */
        {564*0xC0000, {0x3C , 0xAA, },},        /*index: 460, 26.5077dB */
        {563*0xC0000, {0x3C , 0xA9, },},        /*index: 461, 26.4606dB */
        {562*0xC0000, {0x3C , 0xA8, },},        /*index: 462, 26.4136dB */
        {561*0xC0000, {0x3C , 0xA7, },},        /*index: 463, 26.3666dB */
        {560*0xC0000, {0x3C , 0xA6, },},        /*index: 464, 26.3195dB */
        {559*0xC0000, {0x3C , 0xA5, },},        /*index: 465, 26.2725dB */
        {558*0xC0000, {0x3C , 0xA4, },},        /*index: 466, 26.2255dB */
        {557*0xC0000, {0x3C , 0xA3, },},        /*index: 467, 26.1784dB */
        {556*0xC0000, {0x3C , 0xA2, },},        /*index: 468, 26.1314dB */
        {555*0xC0000, {0x3C , 0xA2, },},        /*index: 469, 26.0843dB */
        {554*0xC0000, {0x3C , 0xA1, },},        /*index: 470, 26.0373dB */
        {553*0xC0000, {0x3C , 0xA0, },},        /*index: 471, 25.9903dB */
        {552*0xC0000, {0x3C , 0x9F, },},        /*index: 472, 25.9432dB */
        {551*0xC0000, {0x3C , 0x9E, },},        /*index: 473, 25.8962dB */
        {550*0xC0000, {0x3C , 0x9D, },},        /*index: 474, 25.8492dB */
        {549*0xC0000, {0x3C , 0x9C, },},        /*index: 475, 25.8021dB */
        {548*0xC0000, {0x3C , 0x9C, },},        /*index: 476, 25.7551dB */
        {547*0xC0000, {0x3C , 0x9B, },},        /*index: 477, 25.7081dB */
        {546*0xC0000, {0x3C , 0x9A, },},        /*index: 478, 25.661dB */
        {545*0xC0000, {0x3C , 0x99, },},        /*index: 479, 25.614dB */
        {544*0xC0000, {0x3C , 0x98, },},        /*index: 480, 25.5669dB */
        {543*0xC0000, {0x3C , 0x97, },},        /*index: 481, 25.5199dB */
        {542*0xC0000, {0x3C , 0x97, },},        /*index: 482, 25.4729dB */
        {541*0xC0000, {0x3C , 0x96, },},        /*index: 483, 25.4258dB */
        {540*0xC0000, {0x3C , 0x95, },},        /*index: 484, 25.3788dB */
        {539*0xC0000, {0x3C , 0x94, },},        /*index: 485, 25.3318dB */
        {538*0xC0000, {0x3C , 0x93, },},        /*index: 486, 25.2847dB */
        {537*0xC0000, {0x3C , 0x93, },},        /*index: 487, 25.2377dB */
        {536*0xC0000, {0x3C , 0x92, },},        /*index: 488, 25.1907dB */
        {535*0xC0000, {0x3C , 0x91, },},        /*index: 489, 25.1436dB */
        {534*0xC0000, {0x3C , 0x90, },},        /*index: 490, 25.0966dB */
        {533*0xC0000, {0x3C , 0x8F, },},        /*index: 491, 25.0496dB */
        {532*0xC0000, {0x3C , 0x8F, },},        /*index: 492, 25.0025dB */
        {531*0xC0000, {0x3C , 0x8E, },},        /*index: 493, 24.9555dB */
        {530*0xC0000, {0x3C , 0x8D, },},        /*index: 494, 24.9084dB */
        {529*0xC0000, {0x3C , 0x8C, },},        /*index: 495, 24.8614dB */
        {528*0xC0000, {0x3C , 0x8C, },},        /*index: 496, 24.8144dB */
        {527*0xC0000, {0x3C , 0x8B, },},        /*index: 497, 24.7673dB */
        {526*0xC0000, {0x3C , 0x8A, },},        /*index: 498, 24.7203dB */
        {525*0xC0000, {0x3C , 0x89, },},        /*index: 499, 24.6733dB */
        {524*0xC0000, {0x3C , 0x89, },},        /*index: 500, 24.6262dB */
        {523*0xC0000, {0x3C , 0x88, },},        /*index: 501, 24.5792dB */
        {522*0xC0000, {0x3C , 0x87, },},        /*index: 502, 24.5322dB */
        {521*0xC0000, {0x3C , 0x86, },},        /*index: 503, 24.4851dB */
        {520*0xC0000, {0x3C , 0x86, },},        /*index: 504, 24.4381dB */
        {519*0xC0000, {0x3C , 0x85, },},        /*index: 505, 24.3911dB */
        {518*0xC0000, {0x3C , 0x84, },},        /*index: 506, 24.344dB */
        {517*0xC0000, {0x3C , 0x84, },},        /*index: 507, 24.297dB */
        {516*0xC0000, {0x3C , 0x83, },},        /*index: 508, 24.2499dB */
        {515*0xC0000, {0x3C , 0x82, },},        /*index: 509, 24.2029dB */
        {514*0xC0000, {0x3C , 0x81, },},        /*index: 510, 24.1559dB */
        {513*0xC0000, {0x3C , 0x81, },},        /*index: 511, 24.1088dB */
        {512*0xC0000, {0x3C , 0x80, },},        /*index: 512, 24.0618dB */
        {511*0xC0000, {0x30 , 0xFF, },},        /*index: 385, 24.0354dB */
        {510*0xC0000, {0x30 , 0xFD, },},        /*index: 386, 23.9883dB */
        {509*0xC0000, {0x30 , 0xFC, },},        /*index: 387, 23.9413dB */
        {508*0xC0000, {0x30 , 0xFB, },},        /*index: 388, 23.8943dB */
        {507*0xC0000, {0x30 , 0xF9, },},        /*index: 389, 23.8472dB */
        {506*0xC0000, {0x30 , 0xF8, },},        /*index: 390, 23.8002dB */
        {505*0xC0000, {0x30 , 0xF6, },},        /*index: 391, 23.7531dB */
        {504*0xC0000, {0x30 , 0xF5, },},        /*index: 392, 23.7061dB */
        {503*0xC0000, {0x30 , 0xF4, },},        /*index: 393, 23.6591dB */
        {502*0xC0000, {0x30 , 0xF3, },},        /*index: 394, 23.612dB */
        {501*0xC0000, {0x30 , 0xF1, },},        /*index: 395, 23.565dB */
        {500*0xC0000, {0x30 , 0xF0, },},        /*index: 396, 23.518dB */
        {499*0xC0000, {0x30 , 0xEF, },},        /*index: 397, 23.4709dB */
        {498*0xC0000, {0x30 , 0xED, },},        /*index: 398, 23.4239dB */
        {497*0xC0000, {0x30 , 0xEC, },},        /*index: 399, 23.3769dB */
        {496*0xC0000, {0x30 , 0xEB, },},        /*index: 400, 23.3298dB */
        {495*0xC0000, {0x30 , 0xE9, },},        /*index: 401, 23.2828dB */
        {494*0xC0000, {0x30 , 0xE8, },},        /*index: 402, 23.2358dB */
        {493*0xC0000, {0x30 , 0xE7, },},        /*index: 403, 23.1887dB */
        {492*0xC0000, {0x30 , 0xE6, },},        /*index: 404, 23.1417dB */
        {491*0xC0000, {0x30 , 0xE4, },},        /*index: 405, 23.0946dB */
        {490*0xC0000, {0x30 , 0xE3, },},        /*index: 406, 23.0476dB */
        {489*0xC0000, {0x30 , 0xE2, },},        /*index: 407, 23.0006dB */
        {488*0xC0000, {0x30 , 0xE1, },},        /*index: 408, 22.9535dB */
        {487*0xC0000, {0x30 , 0xE0, },},        /*index: 409, 22.9065dB */
        {486*0xC0000, {0x30 , 0xDE, },},        /*index: 410, 22.8595dB */
        {485*0xC0000, {0x30 , 0xDD, },},        /*index: 411, 22.8124dB */
        {484*0xC0000, {0x30 , 0xDC, },},        /*index: 412, 22.7654dB */
        {483*0xC0000, {0x30 , 0xDB, },},        /*index: 413, 22.7184dB */
        {482*0xC0000, {0x30 , 0xDA, },},        /*index: 414, 22.6713dB */
        {481*0xC0000, {0x30 , 0xD8, },},        /*index: 415, 22.6243dB */
        {480*0xC0000, {0x30 , 0xD7, },},        /*index: 416, 22.5772dB */
        {479*0xC0000, {0x30 , 0xD6, },},        /*index: 417, 22.5302dB */
        {478*0xC0000, {0x30 , 0xD5, },},        /*index: 418, 22.4832dB */
        {477*0xC0000, {0x30 , 0xD4, },},        /*index: 419, 22.4361dB */
        {476*0xC0000, {0x30 , 0xD3, },},        /*index: 420, 22.3891dB */
        {475*0xC0000, {0x30 , 0xD2, },},        /*index: 421, 22.3421dB */
        {474*0xC0000, {0x30 , 0xD0, },},        /*index: 422, 22.295dB */
        {473*0xC0000, {0x30 , 0xCF, },},        /*index: 423, 22.248dB */
        {472*0xC0000, {0x30 , 0xCE, },},        /*index: 424, 22.201dB */
        {471*0xC0000, {0x30 , 0xCD, },},        /*index: 425, 22.1539dB */
        {470*0xC0000, {0x30 , 0xCC, },},        /*index: 426, 22.1069dB */
        {469*0xC0000, {0x30 , 0xCB, },},        /*index: 427, 22.0599dB */
        {468*0xC0000, {0x30 , 0xCA, },},        /*index: 428, 22.0128dB */
        {467*0xC0000, {0x30 , 0xC9, },},        /*index: 429, 21.9658dB */
        {466*0xC0000, {0x30 , 0xC8, },},        /*index: 430, 21.9187dB */
        {465*0xC0000, {0x30 , 0xC6, },},        /*index: 431, 21.8717dB */
        {464*0xC0000, {0x30 , 0xC5, },},        /*index: 432, 21.8247dB */
        {463*0xC0000, {0x30 , 0xC4, },},        /*index: 433, 21.7776dB */
        {462*0xC0000, {0x30 , 0xC3, },},        /*index: 434, 21.7306dB */
        {461*0xC0000, {0x30 , 0xC2, },},        /*index: 435, 21.6836dB */
        {460*0xC0000, {0x30 , 0xC1, },},        /*index: 436, 21.6365dB */
        {459*0xC0000, {0x30 , 0xC0, },},        /*index: 437, 21.5895dB */
        {458*0xC0000, {0x30 , 0xBF, },},        /*index: 438, 21.5425dB */
        {457*0xC0000, {0x30 , 0xBE, },},        /*index: 439, 21.4954dB */
        {456*0xC0000, {0x30 , 0xBD, },},        /*index: 440, 21.4484dB */
        {455*0xC0000, {0x30 , 0xBC, },},        /*index: 441, 21.4014dB */
        {454*0xC0000, {0x30 , 0xBB, },},        /*index: 442, 21.3543dB */
        {453*0xC0000, {0x30 , 0xBA, },},        /*index: 443, 21.3073dB */
        {452*0xC0000, {0x30 , 0xB9, },},        /*index: 444, 21.2602dB */
        {451*0xC0000, {0x30 , 0xB8, },},        /*index: 445, 21.2132dB */
        {450*0xC0000, {0x30 , 0xB7, },},        /*index: 446, 21.1662dB */
        {449*0xC0000, {0x30 , 0xB6, },},        /*index: 447, 21.1191dB */
        {448*0xC0000, {0x30 , 0xB5, },},        /*index: 448, 21.0721dB */
        {447*0xC0000, {0x30 , 0xB4, },},        /*index: 449, 21.0251dB */
        {446*0xC0000, {0x30 , 0xB3, },},        /*index: 450, 20.978dB */
        {445*0xC0000, {0x30 , 0xB2, },},        /*index: 451, 20.931dB */
        {444*0xC0000, {0x30 , 0xB1, },},        /*index: 452, 20.884dB */
        {443*0xC0000, {0x30 , 0xB0, },},        /*index: 453, 20.8369dB */
        {442*0xC0000, {0x30 , 0xAF, },},        /*index: 454, 20.7899dB */
        {441*0xC0000, {0x30 , 0xAE, },},        /*index: 455, 20.7428dB */
        {440*0xC0000, {0x30 , 0xAD, },},        /*index: 456, 20.6958dB */
        {439*0xC0000, {0x30 , 0xAC, },},        /*index: 457, 20.6488dB */
        {438*0xC0000, {0x30 , 0xAB, },},        /*index: 458, 20.6017dB */
        {437*0xC0000, {0x30 , 0xAB, },},        /*index: 459, 20.5547dB */
        {436*0xC0000, {0x30 , 0xAA, },},        /*index: 460, 20.5077dB */
        {435*0xC0000, {0x30 , 0xA9, },},        /*index: 461, 20.4606dB */
        {434*0xC0000, {0x30 , 0xA8, },},        /*index: 462, 20.4136dB */
        {433*0xC0000, {0x30 , 0xA7, },},        /*index: 463, 20.3666dB */
        {432*0xC0000, {0x30 , 0xA6, },},        /*index: 464, 20.3195dB */
        {431*0xC0000, {0x30 , 0xA5, },},        /*index: 465, 20.2725dB */
        {430*0xC0000, {0x30 , 0xA4, },},        /*index: 466, 20.2255dB */
        {429*0xC0000, {0x30 , 0xA3, },},        /*index: 467, 20.1784dB */
        {428*0xC0000, {0x30 , 0xA2, },},        /*index: 468, 20.1314dB */
        {427*0xC0000, {0x30 , 0xA2, },},        /*index: 469, 20.0843dB */
        {426*0xC0000, {0x30 , 0xA1, },},        /*index: 470, 20.0373dB */
        {425*0xC0000, {0x30 , 0xA0, },},        /*index: 471, 19.9903dB */
        {424*0xC0000, {0x30 , 0x9F, },},        /*index: 472, 19.9432dB */
        {423*0xC0000, {0x30 , 0x9E, },},        /*index: 473, 19.8962dB */
        {422*0xC0000, {0x30 , 0x9D, },},        /*index: 474, 19.8492dB */
        {421*0xC0000, {0x30 , 0x9C, },},        /*index: 475, 19.8021dB */
        {420*0xC0000, {0x30 , 0x9C, },},        /*index: 476, 19.7551dB */
        {419*0xC0000, {0x30 , 0x9B, },},        /*index: 477, 19.7081dB */
        {418*0xC0000, {0x30 , 0x9A, },},        /*index: 478, 19.661dB */
        {417*0xC0000, {0x30 , 0x99, },},        /*index: 479, 19.614dB */
        {416*0xC0000, {0x30 , 0x98, },},        /*index: 480, 19.5669dB */
        {415*0xC0000, {0x30 , 0x97, },},        /*index: 481, 19.5199dB */
        {414*0xC0000, {0x30 , 0x97, },},        /*index: 482, 19.4729dB */
        {413*0xC0000, {0x30 , 0x96, },},        /*index: 483, 19.4258dB */
        {412*0xC0000, {0x30 , 0x95, },},        /*index: 484, 19.3788dB */
        {411*0xC0000, {0x30 , 0x94, },},        /*index: 485, 19.3318dB */
        {410*0xC0000, {0x30 , 0x93, },},        /*index: 486, 19.2847dB */
        {409*0xC0000, {0x30 , 0x93, },},        /*index: 487, 19.2377dB */
        {408*0xC0000, {0x30 , 0x92, },},        /*index: 488, 19.1907dB */
        {407*0xC0000, {0x30 , 0x91, },},        /*index: 489, 19.1436dB */
        {406*0xC0000, {0x30 , 0x90, },},        /*index: 490, 19.0966dB */
        {405*0xC0000, {0x30 , 0x8F, },},        /*index: 491, 19.0496dB */
        {404*0xC0000, {0x30 , 0x8F, },},        /*index: 492, 19.0025dB */
        {403*0xC0000, {0x30 , 0x8E, },},        /*index: 493, 18.9555dB */
        {402*0xC0000, {0x30 , 0x8D, },},        /*index: 494, 18.9084dB */
        {401*0xC0000, {0x30 , 0x8C, },},        /*index: 495, 18.8614dB */
        {400*0xC0000, {0x30 , 0x8C, },},        /*index: 496, 18.8144dB */
        {399*0xC0000, {0x30 , 0x8B, },},        /*index: 497, 18.7673dB */
        {398*0xC0000, {0x30 , 0x8A, },},        /*index: 498, 18.7203dB */
        {397*0xC0000, {0x30 , 0x89, },},        /*index: 499, 18.6733dB */
        {396*0xC0000, {0x30 , 0x89, },},        /*index: 500, 18.6262dB */
        {395*0xC0000, {0x30 , 0x88, },},        /*index: 501, 18.5792dB */
        {394*0xC0000, {0x30 , 0x87, },},        /*index: 502, 18.5322dB */
        {393*0xC0000, {0x30 , 0x86, },},        /*index: 503, 18.4851dB */
        {392*0xC0000, {0x30 , 0x86, },},        /*index: 504, 18.4381dB */
        {391*0xC0000, {0x30 , 0x85, },},        /*index: 505, 18.3911dB */
        {390*0xC0000, {0x30 , 0x84, },},        /*index: 506, 18.344dB */
        {389*0xC0000, {0x30 , 0x84, },},        /*index: 507, 18.297dB */
        {388*0xC0000, {0x30 , 0x83, },},        /*index: 508, 18.2499dB */
        {387*0xC0000, {0x30 , 0x82, },},        /*index: 509, 18.2029dB */
        {386*0xC0000, {0x30 , 0x81, },},        /*index: 510, 18.1559dB */
        {385*0xC0000, {0x30 , 0x81, },},        /*index: 511, 18.1088dB */
        {384*0xC0000, {0x30 , 0x80, },},        /*index: 512, 18.0618dB */
        {383*0xC0000, {0x20 , 0xFF, },},        /*index: 513, 18.0148dB */
        {382*0xC0000, {0x20 , 0xFD, },},        /*index: 514, 17.9677dB */
        {381*0xC0000, {0x20 , 0xFC, },},        /*index: 515, 17.9207dB */
        {380*0xC0000, {0x20 , 0xFB, },},        /*index: 516, 17.8737dB */
        {379*0xC0000, {0x20 , 0xF9, },},        /*index: 517, 17.8266dB */
        {378*0xC0000, {0x20 , 0xF8, },},        /*index: 518, 17.7796dB */
        {377*0xC0000, {0x20 , 0xF6, },},        /*index: 519, 17.7325dB */
        {376*0xC0000, {0x20 , 0xF5, },},        /*index: 520, 17.6855dB */
        {375*0xC0000, {0x20 , 0xF4, },},        /*index: 521, 17.6385dB */
        {374*0xC0000, {0x20 , 0xF3, },},        /*index: 522, 17.5914dB */
        {373*0xC0000, {0x20 , 0xF1, },},        /*index: 523, 17.5444dB */
        {372*0xC0000, {0x20 , 0xF0, },},        /*index: 524, 17.4974dB */
        {371*0xC0000, {0x20 , 0xEF, },},        /*index: 525, 17.4503dB */
        {370*0xC0000, {0x20 , 0xED, },},        /*index: 526, 17.4033dB */
        {369*0xC0000, {0x20 , 0xEC, },},        /*index: 527, 17.3563dB */
        {368*0xC0000, {0x20 , 0xEB, },},        /*index: 528, 17.3092dB */
        {367*0xC0000, {0x20 , 0xE9, },},        /*index: 529, 17.2622dB */
        {366*0xC0000, {0x20 , 0xE8, },},        /*index: 530, 17.2152dB */
        {365*0xC0000, {0x20 , 0xE7, },},        /*index: 531, 17.1681dB */
        {364*0xC0000, {0x20 , 0xE6, },},        /*index: 532, 17.1211dB */
        {363*0xC0000, {0x20 , 0xE4, },},        /*index: 533, 17.074dB */
        {362*0xC0000, {0x20 , 0xE3, },},        /*index: 534, 17.027dB */
        {361*0xC0000, {0x20 , 0xE2, },},        /*index: 535, 16.98dB */
        {360*0xC0000, {0x20 , 0xE1, },},        /*index: 536, 16.9329dB */
        {359*0xC0000, {0x20 , 0xE0, },},        /*index: 537, 16.8859dB */
        {358*0xC0000, {0x20 , 0xDE, },},        /*index: 538, 16.8389dB */
        {357*0xC0000, {0x20 , 0xDD, },},        /*index: 539, 16.7918dB */
        {356*0xC0000, {0x20 , 0xDC, },},        /*index: 540, 16.7448dB */
        {355*0xC0000, {0x20 , 0xDB, },},        /*index: 541, 16.6978dB */
        {354*0xC0000, {0x20 , 0xDA, },},        /*index: 542, 16.6507dB */
        {353*0xC0000, {0x20 , 0xD8, },},        /*index: 543, 16.6037dB */
        {352*0xC0000, {0x20 , 0xD7, },},        /*index: 544, 16.5566dB */
        {351*0xC0000, {0x20 , 0xD6, },},        /*index: 545, 16.5096dB */
        {350*0xC0000, {0x20 , 0xD5, },},        /*index: 546, 16.4626dB */
        {349*0xC0000, {0x20 , 0xD4, },},        /*index: 547, 16.4155dB */
        {348*0xC0000, {0x20 , 0xD3, },},        /*index: 548, 16.3685dB */
        {347*0xC0000, {0x20 , 0xD2, },},        /*index: 549, 16.3215dB */
        {346*0xC0000, {0x20 , 0xD0, },},        /*index: 550, 16.2744dB */
        {345*0xC0000, {0x20 , 0xCF, },},        /*index: 551, 16.2274dB */
        {344*0xC0000, {0x20 , 0xCE, },},        /*index: 552, 16.1804dB */
        {343*0xC0000, {0x20 , 0xCD, },},        /*index: 553, 16.1333dB */
        {342*0xC0000, {0x20 , 0xCC, },},        /*index: 554, 16.0863dB */
        {341*0xC0000, {0x20 , 0xCB, },},        /*index: 555, 16.0393dB */
        {340*0xC0000, {0x20 , 0xCA, },},        /*index: 556, 15.9922dB */
        {339*0xC0000, {0x20 , 0xC9, },},        /*index: 557, 15.9452dB */
        {338*0xC0000, {0x20 , 0xC8, },},        /*index: 558, 15.8981dB */
        {337*0xC0000, {0x20 , 0xC6, },},        /*index: 559, 15.8511dB */
        {336*0xC0000, {0x20 , 0xC5, },},        /*index: 560, 15.8041dB */
        {335*0xC0000, {0x20 , 0xC4, },},        /*index: 561, 15.757dB */
        {334*0xC0000, {0x20 , 0xC3, },},        /*index: 562, 15.71dB */
        {333*0xC0000, {0x20 , 0xC2, },},        /*index: 563, 15.663dB */
        {332*0xC0000, {0x20 , 0xC1, },},        /*index: 564, 15.6159dB */
        {331*0xC0000, {0x20 , 0xC0, },},        /*index: 565, 15.5689dB */
        {330*0xC0000, {0x20 , 0xBF, },},        /*index: 566, 15.5219dB */
        {329*0xC0000, {0x20 , 0xBE, },},        /*index: 567, 15.4748dB */
        {328*0xC0000, {0x20 , 0xBD, },},        /*index: 568, 15.4278dB */
        {327*0xC0000, {0x20 , 0xBC, },},        /*index: 569, 15.3808dB */
        {326*0xC0000, {0x20 , 0xBB, },},        /*index: 570, 15.3337dB */
        {325*0xC0000, {0x20 , 0xBA, },},        /*index: 571, 15.2867dB */
        {324*0xC0000, {0x20 , 0xB9, },},        /*index: 572, 15.2396dB */
        {323*0xC0000, {0x20 , 0xB8, },},        /*index: 573, 15.1926dB */
        {322*0xC0000, {0x20 , 0xB7, },},        /*index: 574, 15.1456dB */
        {321*0xC0000, {0x20 , 0xB6, },},        /*index: 575, 15.0985dB */
        {320*0xC0000, {0x20 , 0xB5, },},        /*index: 576, 15.0515dB */
        {319*0xC0000, {0x20 , 0xB4, },},        /*index: 577, 15.0045dB */
        {318*0xC0000, {0x20 , 0xB3, },},        /*index: 578, 14.9574dB */
        {317*0xC0000, {0x20 , 0xB2, },},        /*index: 579, 14.9104dB */
        {316*0xC0000, {0x20 , 0xB1, },},        /*index: 580, 14.8634dB */
        {315*0xC0000, {0x20 , 0xB0, },},        /*index: 581, 14.8163dB */
        {314*0xC0000, {0x20 , 0xAF, },},        /*index: 582, 14.7693dB */
        {313*0xC0000, {0x20 , 0xAE, },},        /*index: 583, 14.7222dB */
        {312*0xC0000, {0x20 , 0xAD, },},        /*index: 584, 14.6752dB */
        {311*0xC0000, {0x20 , 0xAC, },},        /*index: 585, 14.6282dB */
        {310*0xC0000, {0x20 , 0xAB, },},        /*index: 586, 14.5811dB */
        {309*0xC0000, {0x20 , 0xAB, },},        /*index: 587, 14.5341dB */
        {308*0xC0000, {0x20 , 0xAA, },},        /*index: 588, 14.4871dB */
        {307*0xC0000, {0x20 , 0xA9, },},        /*index: 589, 14.44dB */
        {306*0xC0000, {0x20 , 0xA8, },},        /*index: 590, 14.393dB */
        {305*0xC0000, {0x20 , 0xA7, },},        /*index: 591, 14.346dB */
        {304*0xC0000, {0x20 , 0xA6, },},        /*index: 592, 14.2989dB */
        {303*0xC0000, {0x20 , 0xA5, },},        /*index: 593, 14.2519dB */
        {302*0xC0000, {0x20 , 0xA4, },},        /*index: 594, 14.2049dB */
        {301*0xC0000, {0x20 , 0xA3, },},        /*index: 595, 14.1578dB */
        {300*0xC0000, {0x20 , 0xA2, },},        /*index: 596, 14.1108dB */
        {299*0xC0000, {0x20 , 0xA2, },},        /*index: 597, 14.0637dB */
        {298*0xC0000, {0x20 , 0xA1, },},        /*index: 598, 14.0167dB */
        {297*0xC0000, {0x20 , 0xA0, },},        /*index: 599, 13.9697dB */
        {296*0xC0000, {0x20 , 0x9F, },},        /*index: 600, 13.9226dB */
        {295*0xC0000, {0x20 , 0x9E, },},        /*index: 601, 13.8756dB */
        {294*0xC0000, {0x20 , 0x9D, },},        /*index: 602, 13.8286dB */
        {293*0xC0000, {0x20 , 0x9C, },},        /*index: 603, 13.7815dB */
        {292*0xC0000, {0x20 , 0x9C, },},        /*index: 604, 13.7345dB */
        {291*0xC0000, {0x20 , 0x9B, },},        /*index: 605, 13.6875dB */
        {290*0xC0000, {0x20 , 0x9A, },},        /*index: 606, 13.6404dB */
        {289*0xC0000, {0x20 , 0x99, },},        /*index: 607, 13.5934dB */
        {288*0xC0000, {0x20 , 0x98, },},        /*index: 608, 13.5463dB */
        {287*0xC0000, {0x20 , 0x97, },},        /*index: 609, 13.4993dB */
        {286*0xC0000, {0x20 , 0x97, },},        /*index: 610, 13.4523dB */
        {285*0xC0000, {0x20 , 0x96, },},        /*index: 611, 13.4052dB */
        {284*0xC0000, {0x20 , 0x95, },},        /*index: 612, 13.3582dB */
        {283*0xC0000, {0x20 , 0x94, },},        /*index: 613, 13.3112dB */
        {282*0xC0000, {0x20 , 0x93, },},        /*index: 614, 13.2641dB */
        {281*0xC0000, {0x20 , 0x93, },},        /*index: 615, 13.2171dB */
        {280*0xC0000, {0x20 , 0x92, },},        /*index: 616, 13.1701dB */
        {279*0xC0000, {0x20 , 0x91, },},        /*index: 617, 13.123dB */
        {278*0xC0000, {0x20 , 0x90, },},        /*index: 618, 13.076dB */
        {277*0xC0000, {0x20 , 0x8F, },},        /*index: 619, 13.029dB */
        {276*0xC0000, {0x20 , 0x8F, },},        /*index: 620, 12.9819dB */
        {275*0xC0000, {0x20 , 0x8E, },},        /*index: 621, 12.9349dB */
        {274*0xC0000, {0x20 , 0x8D, },},        /*index: 622, 12.8878dB */
        {273*0xC0000, {0x20 , 0x8C, },},        /*index: 623, 12.8408dB */
        {272*0xC0000, {0x20 , 0x8C, },},        /*index: 624, 12.7938dB */
        {271*0xC0000, {0x20 , 0x8B, },},        /*index: 625, 12.7467dB */
        {270*0xC0000, {0x20 , 0x8A, },},        /*index: 626, 12.6997dB */
        {269*0xC0000, {0x20 , 0x89, },},        /*index: 627, 12.6527dB */
        {268*0xC0000, {0x20 , 0x89, },},        /*index: 628, 12.6056dB */
        {267*0xC0000, {0x20 , 0x88, },},        /*index: 629, 12.5586dB */
        {266*0xC0000, {0x20 , 0x87, },},        /*index: 630, 12.5116dB */
        {265*0xC0000, {0x20 , 0x86, },},        /*index: 631, 12.4645dB */
        {264*0xC0000, {0x20 , 0x86, },},        /*index: 632, 12.4175dB */
        {263*0xC0000, {0x20 , 0x85, },},        /*index: 633, 12.3705dB */
        {262*0xC0000, {0x20 , 0x84, },},        /*index: 634, 12.3234dB */
        {261*0xC0000, {0x20 , 0x84, },},        /*index: 635, 12.2764dB */
        {260*0xC0000, {0x20 , 0x83, },},        /*index: 636, 12.2293dB */
        {259*0xC0000, {0x20 , 0x82, },},        /*index: 637, 12.1823dB */
        {258*0xC0000, {0x20 , 0x81, },},        /*index: 638, 12.1353dB */
        {257*0xC0000, {0x20 , 0x81, },},        /*index: 639, 12.0882dB */
        {256*0xC0000, {0x20 , 0x80, },},        /*index: 640, 12.0412dB */
        {255*0xC0000, {0x10 , 0xFF, },},        /*index: 641, 11.9942dB */
        {254*0xC0000, {0x10 , 0xFD, },},        /*index: 642, 11.9471dB */
        {253*0xC0000, {0x10 , 0xFC, },},        /*index: 643, 11.9001dB */
        {252*0xC0000, {0x10 , 0xFB, },},        /*index: 644, 11.8531dB */
        {251*0xC0000, {0x10 , 0xF9, },},        /*index: 645, 11.806dB */
        {250*0xC0000, {0x10 , 0xF8, },},        /*index: 646, 11.759dB */
        {249*0xC0000, {0x10 , 0xF6, },},        /*index: 647, 11.7119dB */
        {248*0xC0000, {0x10 , 0xF5, },},        /*index: 648, 11.6649dB */
        {247*0xC0000, {0x10 , 0xF4, },},        /*index: 649, 11.6179dB */
        {246*0xC0000, {0x10 , 0xF3, },},        /*index: 650, 11.5708dB */
        {245*0xC0000, {0x10 , 0xF1, },},        /*index: 651, 11.5238dB */
        {244*0xC0000, {0x10 , 0xF0, },},        /*index: 652, 11.4768dB */
        {243*0xC0000, {0x10 , 0xEF, },},        /*index: 653, 11.4297dB */
        {242*0xC0000, {0x10 , 0xED, },},        /*index: 654, 11.3827dB */
        {241*0xC0000, {0x10 , 0xEC, },},        /*index: 655, 11.3357dB */
        {240*0xC0000, {0x10 , 0xEB, },},        /*index: 656, 11.2886dB */
        {239*0xC0000, {0x10 , 0xE9, },},        /*index: 657, 11.2416dB */
        {238*0xC0000, {0x10 , 0xE8, },},        /*index: 658, 11.1946dB */
        {237*0xC0000, {0x10 , 0xE7, },},        /*index: 659, 11.1475dB */
        {236*0xC0000, {0x10 , 0xE6, },},        /*index: 660, 11.1005dB */
        {235*0xC0000, {0x10 , 0xE4, },},        /*index: 661, 11.0534dB */
        {234*0xC0000, {0x10 , 0xE3, },},        /*index: 662, 11.0064dB */
        {233*0xC0000, {0x10 , 0xE2, },},        /*index: 663, 10.9594dB */
        {232*0xC0000, {0x10 , 0xE1, },},        /*index: 664, 10.9123dB */
        {231*0xC0000, {0x10 , 0xE0, },},        /*index: 665, 10.8653dB */
        {230*0xC0000, {0x10 , 0xDE, },},        /*index: 666, 10.8183dB */
        {229*0xC0000, {0x10 , 0xDD, },},        /*index: 667, 10.7712dB */
        {228*0xC0000, {0x10 , 0xDC, },},        /*index: 668, 10.7242dB */
        {227*0xC0000, {0x10 , 0xDB, },},        /*index: 669, 10.6772dB */
        {226*0xC0000, {0x10 , 0xDA, },},        /*index: 670, 10.6301dB */
        {225*0xC0000, {0x10 , 0xD8, },},        /*index: 671, 10.5831dB */
        {224*0xC0000, {0x10 , 0xD7, },},        /*index: 672, 10.536dB */
        {223*0xC0000, {0x10 , 0xD6, },},        /*index: 673, 10.489dB */
        {222*0xC0000, {0x10 , 0xD5, },},        /*index: 674, 10.442dB */
        {221*0xC0000, {0x10 , 0xD4, },},        /*index: 675, 10.3949dB */
        {220*0xC0000, {0x10 , 0xD3, },},        /*index: 676, 10.3479dB */
        {219*0xC0000, {0x10 , 0xD2, },},        /*index: 677, 10.3009dB */
        {218*0xC0000, {0x10 , 0xD0, },},        /*index: 678, 10.2538dB */
        {217*0xC0000, {0x10 , 0xCF, },},        /*index: 679, 10.2068dB */
        {216*0xC0000, {0x10 , 0xCE, },},        /*index: 680, 10.1598dB */
        {215*0xC0000, {0x10 , 0xCD, },},        /*index: 681, 10.1127dB */
        {214*0xC0000, {0x10 , 0xCC, },},        /*index: 682, 10.0657dB */
        {213*0xC0000, {0x10 , 0xCB, },},        /*index: 683, 10.0187dB */
        {212*0xC0000, {0x10 , 0xCA, },},        /*index: 684, 9.9716dB */
        {211*0xC0000, {0x10 , 0xC9, },},        /*index: 685, 9.9246dB */
        {210*0xC0000, {0x10 , 0xC8, },},        /*index: 686, 9.8775dB */
        {209*0xC0000, {0x10 , 0xC6, },},        /*index: 687, 9.8305dB */
        {208*0xC0000, {0x10 , 0xC5, },},        /*index: 688, 9.7835dB */
        {207*0xC0000, {0x10 , 0xC4, },},        /*index: 689, 9.7364dB */
        {206*0xC0000, {0x10 , 0xC3, },},        /*index: 690, 9.6894dB */
        {205*0xC0000, {0x10 , 0xC2, },},        /*index: 691, 9.6424dB */
        {204*0xC0000, {0x10 , 0xC1, },},        /*index: 692, 9.5953dB */
        {203*0xC0000, {0x10 , 0xC0, },},        /*index: 693, 9.5483dB */
        {202*0xC0000, {0x10 , 0xBF, },},        /*index: 694, 9.5013dB */
        {201*0xC0000, {0x10 , 0xBE, },},        /*index: 695, 9.4542dB */
        {200*0xC0000, {0x10 , 0xBD, },},        /*index: 696, 9.4072dB */
        {199*0xC0000, {0x10 , 0xBC, },},        /*index: 697, 9.3602dB */
        {198*0xC0000, {0x10 , 0xBB, },},        /*index: 698, 9.3131dB */
        {197*0xC0000, {0x10 , 0xBA, },},        /*index: 699, 9.2661dB */
        {196*0xC0000, {0x10 , 0xB9, },},        /*index: 700, 9.219dB */
        {195*0xC0000, {0x10 , 0xB8, },},        /*index: 701, 9.172dB */
        {194*0xC0000, {0x10 , 0xB7, },},        /*index: 702, 9.125dB */
        {193*0xC0000, {0x10 , 0xB6, },},        /*index: 703, 9.0779dB */
        {192*0xC0000, {0x10 , 0xB5, },},        /*index: 704, 9.0309dB */
        {191*0xC0000, {0x10 , 0xB4, },},        /*index: 705, 8.9839dB */
        {190*0xC0000, {0x10 , 0xB3, },},        /*index: 706, 8.9368dB */
        {189*0xC0000, {0x10 , 0xB2, },},        /*index: 707, 8.8898dB */
        {188*0xC0000, {0x10 , 0xB1, },},        /*index: 708, 8.8428dB */
        {187*0xC0000, {0x10 , 0xB0, },},        /*index: 709, 8.7957dB */
        {186*0xC0000, {0x10 , 0xAF, },},        /*index: 710, 8.7487dB */
        {185*0xC0000, {0x10 , 0xAE, },},        /*index: 711, 8.7016dB */
        {184*0xC0000, {0x10 , 0xAD, },},        /*index: 712, 8.6546dB */
        {183*0xC0000, {0x10 , 0xAC, },},        /*index: 713, 8.6076dB */
        {182*0xC0000, {0x10 , 0xAB, },},        /*index: 714, 8.5605dB */
        {181*0xC0000, {0x10 , 0xAB, },},        /*index: 715, 8.5135dB */
        {180*0xC0000, {0x10 , 0xAA, },},        /*index: 716, 8.4665dB */
        {179*0xC0000, {0x10 , 0xA9, },},        /*index: 717, 8.4194dB */
        {178*0xC0000, {0x10 , 0xA8, },},        /*index: 718, 8.3724dB */
        {177*0xC0000, {0x10 , 0xA7, },},        /*index: 719, 8.3254dB */
        {176*0xC0000, {0x10 , 0xA6, },},        /*index: 720, 8.2783dB */
        {175*0xC0000, {0x10 , 0xA5, },},        /*index: 721, 8.2313dB */
        {174*0xC0000, {0x10 , 0xA4, },},        /*index: 722, 8.1843dB */
        {173*0xC0000, {0x10 , 0xA3, },},        /*index: 723, 8.1372dB */
        {172*0xC0000, {0x10 , 0xA2, },},        /*index: 724, 8.0902dB */
        {171*0xC0000, {0x10 , 0xA2, },},        /*index: 725, 8.0431dB */
        {170*0xC0000, {0x10 , 0xA1, },},        /*index: 726, 7.9961dB */
        {169*0xC0000, {0x10 , 0xA0, },},        /*index: 727, 7.9491dB */
        {168*0xC0000, {0x10 , 0x9F, },},        /*index: 728, 7.902dB */
        {167*0xC0000, {0x10 , 0x9E, },},        /*index: 729, 7.855dB */
        {166*0xC0000, {0x10 , 0x9D, },},        /*index: 730, 7.808dB */
        {165*0xC0000, {0x10 , 0x9C, },},        /*index: 731, 7.7609dB */
        {164*0xC0000, {0x10 , 0x9C, },},        /*index: 732, 7.7139dB */
        {163*0xC0000, {0x10 , 0x9B, },},        /*index: 733, 7.6669dB */
        {162*0xC0000, {0x10 , 0x9A, },},        /*index: 734, 7.6198dB */
        {161*0xC0000, {0x10 , 0x99, },},        /*index: 735, 7.5728dB */
        {160*0xC0000, {0x10 , 0x98, },},        /*index: 736, 7.5257dB */
        {159*0xC0000, {0x10 , 0x97, },},        /*index: 737, 7.4787dB */
        {158*0xC0000, {0x10 , 0x97, },},        /*index: 738, 7.4317dB */
        {157*0xC0000, {0x10 , 0x96, },},        /*index: 739, 7.3846dB */
        {156*0xC0000, {0x10 , 0x95, },},        /*index: 740, 7.3376dB */
        {155*0xC0000, {0x10 , 0x94, },},        /*index: 741, 7.2906dB */
        {154*0xC0000, {0x10 , 0x93, },},        /*index: 742, 7.2435dB */
        {153*0xC0000, {0x10 , 0x93, },},        /*index: 743, 7.1965dB */
        {152*0xC0000, {0x10 , 0x92, },},        /*index: 744, 7.1495dB */
        {151*0xC0000, {0x10 , 0x91, },},        /*index: 745, 7.1024dB */
        {150*0xC0000, {0x10 , 0x90, },},        /*index: 746, 7.0554dB */
        {149*0xC0000, {0x10 , 0x8F, },},        /*index: 747, 7.0084dB */
        {148*0xC0000, {0x10 , 0x8F, },},        /*index: 748, 6.9613dB */
        {147*0xC0000, {0x10 , 0x8E, },},        /*index: 749, 6.9143dB */
        {146*0xC0000, {0x10 , 0x8D, },},        /*index: 750, 6.8672dB */
        {145*0xC0000, {0x10 , 0x8C, },},        /*index: 751, 6.8202dB */
        {144*0xC0000, {0x10 , 0x8C, },},        /*index: 752, 6.7732dB */
        {143*0xC0000, {0x10 , 0x8B, },},        /*index: 753, 6.7261dB */
        {142*0xC0000, {0x10 , 0x8A, },},        /*index: 754, 6.6791dB */
        {141*0xC0000, {0x10 , 0x89, },},        /*index: 755, 6.6321dB */
        {140*0xC0000, {0x10 , 0x89, },},        /*index: 756, 6.585dB */
        {139*0xC0000, {0x10 , 0x88, },},        /*index: 757, 6.538dB */
        {138*0xC0000, {0x10 , 0x87, },},        /*index: 758, 6.491dB */
        {137*0xC0000, {0x10 , 0x86, },},        /*index: 759, 6.4439dB */
        {136*0xC0000, {0x10 , 0x86, },},        /*index: 760, 6.3969dB */
        {135*0xC0000, {0x10 , 0x85, },},        /*index: 761, 6.3499dB */
        {134*0xC0000, {0x10 , 0x84, },},        /*index: 762, 6.3028dB */
        {133*0xC0000, {0x10 , 0x84, },},        /*index: 763, 6.2558dB */
        {132*0xC0000, {0x10 , 0x83, },},        /*index: 764, 6.2087dB */
        {131*0xC0000, {0x10 , 0x82, },},        /*index: 765, 6.1617dB */
        {130*0xC0000, {0x10 , 0x81, },},        /*index: 766, 6.1147dB */
        {129*0xC0000, {0x10 , 0x81, },},        /*index: 767, 6.0676dB */
        {128*0xC0000, {0x10 , 0x80, },},        /*index: 768, 6.0206dB */
        {127*0xC0000, {0xB  , 0xFF, },},        /*index: 769, 5.9736dB */
        {126*0xC0000, {0xB  , 0xFD, },},        /*index: 770, 5.9265dB */
        {125*0xC0000, {0xB  , 0xFC, },},        /*index: 771, 5.8795dB */
        {124*0xC0000, {0xB  , 0xFB, },},        /*index: 772, 5.8325dB */
        {123*0xC0000, {0xB  , 0xF9, },},        /*index: 773, 5.7854dB */
        {122*0xC0000, {0xB  , 0xF8, },},        /*index: 774, 5.7384dB */
        {121*0xC0000, {0xB  , 0xF6, },},        /*index: 775, 5.6913dB */
        {120*0xC0000, {0xB  , 0xF5, },},        /*index: 776, 5.6443dB */
        {119*0xC0000, {0xB  , 0xF4, },},        /*index: 777, 5.5973dB */
        {118*0xC0000, {0xB  , 0xF3, },},        /*index: 778, 5.5502dB */
        {117*0xC0000, {0xB  , 0xF1, },},        /*index: 779, 5.5032dB */
        {116*0xC0000, {0xB  , 0xF0, },},        /*index: 780, 5.4562dB */
        {115*0xC0000, {0xB  , 0xEF, },},        /*index: 781, 5.4091dB */
        {114*0xC0000, {0xB  , 0xED, },},        /*index: 782, 5.3621dB */
        {113*0xC0000, {0xB  , 0xEC, },},        /*index: 783, 5.3151dB */
        {112*0xC0000, {0xB  , 0xEB, },},        /*index: 784, 5.268dB */
        {111*0xC0000, {0xB  , 0xE9, },},        /*index: 785, 5.221dB */
        {110*0xC0000, {0xB  , 0xE8, },},        /*index: 786, 5.174dB */
        {109*0xC0000, {0xB  , 0xE7, },},        /*index: 787, 5.1269dB */
        {108*0xC0000, {0xB  , 0xE6, },},        /*index: 788, 5.0799dB */
        {107*0xC0000, {0xB  , 0xE4, },},        /*index: 789, 5.0328dB */
        {106*0xC0000, {0xB  , 0xE3, },},        /*index: 790, 4.9858dB */
        {105*0xC0000, {0xB  , 0xE2, },},        /*index: 791, 4.9388dB */
        {104*0xC0000, {0xB  , 0xE1, },},        /*index: 792, 4.8917dB */
        {103*0xC0000, {0xB  , 0xE0, },},        /*index: 793, 4.8447dB */
        {102*0xC0000, {0xB  , 0xDE, },},        /*index: 794, 4.7977dB */
        {101*0xC0000, {0xB  , 0xDD, },},        /*index: 795, 4.7506dB */
        {100*0xC0000, {0xB  , 0xDC, },},        /*index: 796, 4.7036dB */
        { 99*0xC0000, {0xB  , 0xDB, },},        /*index: 797, 4.6566dB */
        { 98*0xC0000, {0xB  , 0xDA, },},        /*index: 798, 4.6095dB */
        { 97*0xC0000, {0xB  , 0xD8, },},        /*index: 799, 4.5625dB */
        { 96*0xC0000, {0xB  , 0xD7, },},        /*index: 800, 4.5154dB */
        { 95*0xC0000, {0xB  , 0xD6, },},        /*index: 801, 4.4684dB */
        { 94*0xC0000, {0xB  , 0xD5, },},        /*index: 802, 4.4214dB */
        { 93*0xC0000, {0xB  , 0xD4, },},        /*index: 803, 4.3743dB */
        { 92*0xC0000, {0xB  , 0xD3, },},        /*index: 804, 4.3273dB */
        { 91*0xC0000, {0xB  , 0xD2, },},        /*index: 805, 4.2803dB */
        { 90*0xC0000, {0xB  , 0xD0, },},        /*index: 806, 4.2332dB */
        { 89*0xC0000, {0xB  , 0xCF, },},        /*index: 807, 4.1862dB */
        { 88*0xC0000, {0xB  , 0xCE, },},        /*index: 808, 4.1392dB */
        { 87*0xC0000, {0xB  , 0xCD, },},        /*index: 809, 4.0921dB */
        { 86*0xC0000, {0xB  , 0xCC, },},        /*index: 810, 4.0451dB */
        { 85*0xC0000, {0xB  , 0xCB, },},        /*index: 811, 3.9981dB */
        { 84*0xC0000, {0xB  , 0xCA, },},        /*index: 812, 3.951dB */
        { 83*0xC0000, {0xB  , 0xC9, },},        /*index: 813, 3.904dB */
        { 82*0xC0000, {0xB  , 0xC8, },},        /*index: 814, 3.8569dB */
        { 81*0xC0000, {0xB  , 0xC6, },},        /*index: 815, 3.8099dB */
        { 80*0xC0000, {0xB  , 0xC5, },},        /*index: 816, 3.7629dB */
        { 79*0xC0000, {0xB  , 0xC4, },},        /*index: 817, 3.7158dB */
        { 78*0xC0000, {0xB  , 0xC3, },},        /*index: 818, 3.6688dB */
        { 77*0xC0000, {0xB  , 0xC2, },},        /*index: 819, 3.6218dB */
        { 76*0xC0000, {0xB  , 0xC1, },},        /*index: 820, 3.5747dB */
        { 75*0xC0000, {0xB  , 0xC0, },},        /*index: 821, 3.5277dB */
        { 74*0xC0000, {0xB  , 0xBF, },},        /*index: 822, 3.4807dB */
        { 73*0xC0000, {0xB  , 0xBE, },},        /*index: 823, 3.4336dB */
        { 72*0xC0000, {0xB  , 0xBD, },},        /*index: 824, 3.3866dB */
        { 71*0xC0000, {0xB  , 0xBC, },},        /*index: 825, 3.3396dB */
        { 70*0xC0000, {0xB  , 0xBB, },},        /*index: 826, 3.2925dB */
        { 69*0xC0000, {0xB  , 0xBA, },},        /*index: 827, 3.2455dB */
        { 68*0xC0000, {0xB  , 0xB9, },},        /*index: 828, 3.1984dB */
        { 67*0xC0000, {0xB  , 0xB8, },},        /*index: 829, 3.1514dB */
        { 66*0xC0000, {0xB  , 0xB7, },},        /*index: 830, 3.1044dB */
        { 65*0xC0000, {0xB  , 0xB6, },},        /*index: 831, 3.0573dB */
        { 64*0xC0000, {0xB  , 0xB5, },},        /*index: 832, 3.0103dB */
        { 63*0xC0000, {0xB  , 0xB4, },},        /*index: 833, 2.9633dB */
        { 62*0xC0000, {0xB  , 0xB3, },},        /*index: 834, 2.9162dB */
        { 61*0xC0000, {0xB  , 0xB2, },},        /*index: 835, 2.8692dB */
        { 60*0xC0000, {0xB  , 0xB1, },},        /*index: 836, 2.8222dB */
        { 59*0xC0000, {0xB  , 0xB0, },},        /*index: 837, 2.7751dB */
        { 58*0xC0000, {0xB  , 0xAF, },},        /*index: 838, 2.7281dB */
        { 57*0xC0000, {0xB  , 0xAE, },},        /*index: 839, 2.681dB */
        { 56*0xC0000, {0xB  , 0xAD, },},        /*index: 840, 2.634dB */
        { 55*0xC0000, {0xB  , 0xAC, },},        /*index: 841, 2.587dB */
        { 54*0xC0000, {0xB  , 0xAB, },},        /*index: 842, 2.5399dB */
        { 53*0xC0000, {0xB  , 0xAB, },},        /*index: 843, 2.4929dB */
        { 52*0xC0000, {0xB  , 0xAA, },},        /*index: 844, 2.4459dB */
        { 51*0xC0000, {0xB  , 0xA9, },},        /*index: 845, 2.3988dB */
        { 50*0xC0000, {0xB  , 0xA8, },},        /*index: 846, 2.3518dB */
        { 49*0xC0000, {0xB  , 0xA7, },},        /*index: 847, 2.3048dB */
        { 48*0xC0000, {0xB  , 0xA6, },},        /*index: 848, 2.2577dB */
        { 47*0xC0000, {0xB  , 0xA5, },},        /*index: 849, 2.2107dB */
        { 46*0xC0000, {0xB  , 0xA4, },},        /*index: 850, 2.1637dB */
        { 45*0xC0000, {0xB  , 0xA3, },},        /*index: 851, 2.1166dB */
        { 44*0xC0000, {0xB  , 0xA2, },},        /*index: 852, 2.0696dB */
        { 43*0xC0000, {0xB  , 0xA2, },},        /*index: 853, 2.0225dB */
        { 42*0xC0000, {0xB  , 0xA1, },},        /*index: 854, 1.9755dB */
        { 41*0xC0000, {0xB  , 0xA0, },},        /*index: 855, 1.9285dB */
        { 40*0xC0000, {0xB  , 0x9F, },},        /*index: 856, 1.8814dB */
        { 39*0xC0000, {0xB  , 0x9E, },},        /*index: 857, 1.8344dB */
        { 38*0xC0000, {0xB  , 0x9D, },},        /*index: 858, 1.7874dB */
        { 37*0xC0000, {0xB  , 0x9C, },},        /*index: 859, 1.7403dB */
        { 36*0xC0000, {0xB  , 0x9C, },},        /*index: 860, 1.6933dB */
        { 35*0xC0000, {0xB  , 0x9B, },},        /*index: 861, 1.6463dB */
        { 34*0xC0000, {0xB  , 0x9A, },},        /*index: 862, 1.5992dB */
        { 33*0xC0000, {0xB  , 0x99, },},        /*index: 863, 1.5522dB */
        { 32*0xC0000, {0xB  , 0x98, },},        /*index: 864, 1.5051dB */
        { 31*0xC0000, {0xB  , 0x97, },},        /*index: 865, 1.4581dB */
        { 30*0xC0000, {0xB  , 0x97, },},        /*index: 866, 1.4111dB */
        { 29*0xC0000, {0xB  , 0x96, },},        /*index: 867, 1.364dB */
        { 28*0xC0000, {0xB  , 0x95, },},        /*index: 868, 1.317dB */
        { 27*0xC0000, {0xB  , 0x94, },},        /*index: 869, 1.27dB */
        { 26*0xC0000, {0xB  , 0x93, },},        /*index: 870, 1.2229dB */
        { 25*0xC0000, {0xB  , 0x93, },},        /*index: 871, 1.1759dB */
        { 24*0xC0000, {0xB  , 0x92, },},        /*index: 872, 1.1289dB */
        { 23*0xC0000, {0xB  , 0x91, },},        /*index: 873, 1.0818dB */
        { 22*0xC0000, {0xB  , 0x90, },},        /*index: 874, 1.0348dB */
        { 21*0xC0000, {0xB  , 0x8F, },},        /*index: 875, 0.9878dB */
        { 20*0xC0000, {0xB  , 0x8F, },},        /*index: 876, 0.9407dB */
        { 19*0xC0000, {0xB  , 0x8E, },},        /*index: 877, 0.8937dB */
        { 18*0xC0000, {0xB  , 0x8D, },},        /*index: 878, 0.8466dB */
        { 17*0xC0000, {0xB  , 0x8C, },},        /*index: 879, 0.7996dB */
        { 16*0xC0000, {0xB  , 0x8C, },},        /*index: 880, 0.7526dB */
        { 15*0xC0000, {0xB  , 0x8B, },},        /*index: 881, 0.7055dB */
        { 14*0xC0000, {0xB  , 0x8A, },},        /*index: 882, 0.6585dB */
        { 13*0xC0000, {0xB  , 0x89, },},        /*index: 883, 0.6115dB */
        { 12*0xC0000, {0xB  , 0x89, },},        /*index: 884, 0.5644dB */
        { 11*0xC0000, {0xB  , 0x88, },},        /*index: 885, 0.5174dB */
        { 10*0xC0000, {0xB  , 0x87, },},        /*index: 886, 0.4704dB */
        {  9*0xC0000, {0xB  , 0x86, },},        /*index: 887, 0.4233dB */
        {  8*0xC0000, {0xB  , 0x86, },},        /*index: 888, 0.3763dB */
        {  7*0xC0000, {0xB  , 0x85, },},        /*index: 889, 0.3293dB */
        {  6*0xC0000, {0xB  , 0x84, },},        /*index: 890, 0.2822dB */
        {  5*0xC0000, {0xB  , 0x84, },},        /*index: 891, 0.2352dB */
        {  4*0xC0000, {0xB  , 0x83, },},        /*index: 892, 0.1881dB */
        {  3*0xC0000, {0xB  , 0x82, },},        /*index: 893, 0.1411dB */
        {  2*0xC0000, {0xB  , 0x81, },},        /*index: 894, 0.0941dB */
        {  1*0xC0000, {0xB  , 0x81, },},        /*index: 895, 0.047dB */
        {  0*0xC0000, {0xB  , 0x80, },},        /*index: 896, 0dB */
    },
    .temperatue_gain_table =
    {
        .agc_control_by_temperatue      = 0,
        .Aptina_temperatue_mode         = 1,
        .temperatue_control_reg_index   = 0,
        .temperatue_gain_num            = 0,
        .temperatue_reg                 = {0x30B2, 0x0000, 0x3FFF},// must to be fix
        .gain_reg                       = {GADI_VI_SENSOR_TABLE_FLAG_END,},
        .temperatue_gain                = {0},
        .temperatue_control             = {0},
    },
    .magic_end  = GADI_SENSOR_HW_INFO_MAGIC_END,
};

