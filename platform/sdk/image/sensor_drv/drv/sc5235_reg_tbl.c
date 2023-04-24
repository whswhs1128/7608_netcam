/*!
*****************************************************************************
** \file        sc5235_reg_tbl.c
**
** \version     $Id: sc5235_reg_tbl.c 11807 2017-04-13 09:38:45Z hehuali $
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
GADI_VI_SensorDrvInfoT    sc5235_sensor_mipi =
{
    .magic_start        = GADI_SENSOR_HW_INFO_MAGIC_START,
    .version            = GADI_SENSOR_HW_INFO_VERSION,
    .length             = sizeof(GADI_VI_SensorDrvInfoT),
    .file_date          = 0x20180719,
    .HwInfo         =
    {
        .name               = "sc5235_mipi",
        .ctl_type           = GADI_SENSOR_CTL_I2C,
        .addr_len           = sizeof(GADI_U16),
        .data_len           = sizeof(GADI_U8),
        .reset_active_level = GADI_GPIO_LOW,
        .hw_addr            = {(0x60>>1), (0x64>>1), 0, 0},
        .id_reg =
        {
            {0x3107, 0x52},
            {0x3108, 0x35},
        },
        .reset_reg =
        {
            {0x0103, 0x01}, // soft reset
            {GADI_VI_SENSOR_TABLE_FLAG_MSLEEP, 0x20},   // msleep(20);
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
    .sensor_id          = GADI_ISP_SENSOR_SC5235,
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
    .max_width          = 2592,
    .max_height         = 1944,
    .def_sh_time        = GADI_VIDEO_FPS(60),
    .fmt_num            = 3,
    .auto_fmt           = GADI_VIDEO_MODE(1920, 1080, 25, 1),
    .init_reg           =
    {
        {0x0100,0x00},
        {0x302b,0x10},
        {0x302d,0x00},
        {0x3038,0x44},
        {0x3303,0x28},
        {0x3304,0x10},
        {0x3306,0x50},
        {0x3308,0x10},
        {0x330a,0x00},
        {0x330e,0x20},
        {0x3314,0x14},
        {0x3315,0x02},
        {0x331b,0x83},
        {0x331e,0x19},
        {0x3320,0x01},
        {0x3321,0x04},
        {0x3326,0x00},
        {0x3333,0x20},
        {0x3334,0x40},
        {0x3364,0x05},
        {0x3367,0x08},
        {0x3368,0x03},
        {0x3369,0x00},
        {0x336a,0x00},
        {0x336b,0x00},
        {0x336c,0x01},
        {0x336d,0x40},
        {0x337f,0x03},
        {0x338f,0x40},
        {0x33b6,0x07},
        {0x33b7,0x17},
        {0x33b8,0x20},
        {0x33b9,0x20},
        {0x33ba,0x44},
        {0x3620,0x28},
        {0x3621,0xac},
        {0x3622,0xf6},
        {0x3624,0x47},
        {0x3625,0x0b},
        {0x3630,0x30},
        {0x3631,0x88},
        {0x3632,0x18},
        {0x3633,0x23},
        {0x3634,0x86},
        {0x3635,0x4d},
        {0x3636,0x21},
        {0x3637,0x20},
        {0x3639,0x09},
        {0x363a,0x83},
        {0x363b,0x02},
        {0x363c,0x07},
        {0x363d,0x03},
        {0x3670,0x00},
        {0x3677,0x86},
        {0x3678,0x86},
        {0x3679,0xa8},
        {0x367e,0x08},
        {0x367f,0x18},
        {0x3802,0x01},
        {0x3905,0x98},
        {0x3907,0x00},
        {0x3908,0x02},
        {0x390a,0x00},
        {0x391c,0x9f},
        {0x391d,0x00},
        {0x391e,0x01},
        {0x391f,0xc0},
        {0x3e00,0x00},
        {0x3e02,0x00},
        {0x3e03,0x0b},
        {0x3e06,0x00},
        {0x3e07,0x80},
        {0x3e08,0x03},
        {0x3e09,0x20},
        {0x3e1e,0x30},
        {0x3e26,0x20},
        {0x3f00,0x0d},
        {0x3f04,0x02},
        {0x3f08,0x04},
        {0x4500,0x5d},
        {0x4509,0x10},
        {0x4809,0x01},
        {0x5000,0x06},
        {0x5002,0x06},
        {0x5780,0x7f},
        {0x5781,0x06},
        {0x5782,0x04},
        {0x5783,0x00},
        {0x5784,0x00},
        {0x5785,0x16},
        {0x5786,0x12},
        {0x5787,0x08},
        {0x5788,0x02},
        {0x578b,0x07},
        {0x57a0,0x00},
        {0x57a1,0x72},
        {0x57a2,0x01},
        {0x57a3,0xf2},
        {0x6000,0x20},
        {0x6002,0x00},
        //diffrent config           
        {0x3029,0x27},
        {0x302a,0x34},
        {0x3039,0x26},
        {0x303a,0x37},
        {0x3200,0x00},        
        {0x3201,0x10},
        {0x3202,0x00},
        {0x3203,0xfc},
        {0x3205,0x1f},
        {0x3206,0x06},
        {0x3207,0xab},
        {0x3209,0x00},
        {0x320a,0x05},
        {0x320b,0xa0},
        {0x320c,0x05},     
        {0x320d,0x46},
        {0x320e,0x05},
        {0x320f,0xdc},
        {0x3211,0x08},
        {0x3235,0x0b},
        {0x3236,0xb6},
        {0x3307,0x14},
        {0x3309,0x60},
        {0x330d,0x30},
        {0x330f,0x04},
        {0x3310,0x51},
        {0x3318,0x42},
        {0x3319,0x82},
        {0x331f,0x51},
        {0x3332,0x22},
        {0x3339,0x44},
        {0x3350,0x22},
        {0x3359,0x22},
        {0x335c,0x22},
        {0x3366,0xc8},
        {0x33ae,0x22},
        {0x33af,0x22},
        {0x33b0,0x22},
        {0x33b4,0x22},
        {0x3614,0x00},
        {0x3623,0x08},
        {0x3638,0x1a},
        {0x3e01,0xbb},
        {0x3f02,0x05},
        {0x3f05,0x9b},
        {0x3f06,0x21},
        {0x4502,0x10},
        {0x4837,0x20},
        {0x3204,0x0a},
        {0x3208,0x0a},
        {0x3301,0x1c},
        {0x330b,0xb8},
        {0x3e03,0x03},// AGC config start
        {0x3e08,0x00},
        {0x3e09,0x80},    
        {0x3301,0x30}, 
        {0x3630,0x16}, 
        {0x3633,0x33}, 
        {0x3622,0xf6}, 
        {0x363a,0x9f},//AGC config end          
        {0x0100,0x01},           
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },   
    .pll_table =
    {
        [0] =
        {
            .pixclk = 121500000,
            .extclk = 27000000,             
            .regs =
            {
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
    },
    .video_fmt_table =
    {
        [0] = //  2200 * 1840  * 30 = 121500000 : max 30fps
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1920, 1080,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x0100, 0x00, 0x01}, // stream output hold
                {0x3029,0x40},        
                {0x302a,0x26},
                {0x302c,0x01},    
                {0x3039,0x46},
                {0x303a,0x33},  
                {0x3201,0xc4},//xstar 196
                {0x3202,0x00},
                {0x3203,0xe8},//ystar 232
                {0x3204,0x08},               
                {0x3205,0x4b},//xend 196+8+1920-1=2123
                {0x3206,0x05},
                {0x3207,0x27},//yend 232+8+1080-1=1319
                {0x3208,0x07},                     
                {0x3209,0x80},//1920
                {0x320a,0x04},
                {0x320b,0x38},//1080              
                {0x320c,0x04},     
                {0x320d,0xe2},//hts: 1250*2 = 2500
                {0x320e,0x06},
                {0x320f,0x54},//vts:  1620       
                {0x3211,0x04},// x shift 4
                {0x3213,0x04},// y shift 4   
                {0x3f04,0x02},
                {0x3f05,0x69},// hts/2 - [0x3F08] - 4 
                {0x3235,0x0c},
                {0x3236,0xa6},// vts x 2 - 2      
                {0x3301,0x18}, 
                {0x330b,0xb0},
                {0x3e01,0xbb},    
                {0x0100, 0x01, 0x01}, // stream output on               
                {GADI_VI_SENSOR_TABLE_FLAG_END,},    

            },
            .vi_info =
            {
                .width      = 1920,
                .height     = 1080,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_MIPI_2LANES,
                .bits       = GADI_VIDEO_BITS_10,
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
            .update_after_vsync_start   = 3,
            .update_after_vsync_end     = 10,
        },     
        [1] = // 2700 * 1800  * 25 = 121500000  : max 23fps
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(2560, 1440,  25, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {               
                {0x0100, 0x00, 0x01}, // stream output hold
                {0x3029,0x27},        
                {0x302a,0x34},
                {0x302c,0x00},    
                {0x3039,0x26},
                {0x303a,0x37},   
                {0x3201,0x10},//xstar 16
                {0x3202,0x00},
                {0x3203,0xfc},//ystar 252
                {0x3204,0x0a},               
                {0x3205,0x1f},//xend 16+16+2560-1=2591
                {0x3206,0x06},
                {0x3207,0xab},//yend 252+16+1440-1=1707
                {0x3208,0x0a},                     
                {0x3209,0x00},//2560
                {0x320a,0x05},
                {0x320b,0xa0},//1440
                {0x320c,0x05},     
                {0x320d,0x46},//hts: 1350*2 = 2700
                {0x320e,0x07},
                {0x320f,0x08},//vts:  1800       
                {0x3211,0x08},// x shift 8
                {0x3213,0x08},// y shift 8   
                {0x3f04,0x02},
                {0x3f05,0x9b},// hts/2 - [0x3F08] - 4 
                {0x3235,0x0e},
                {0x3236,0x0e},// vts x 2 - 2         
                {0x3301,0x1c}, 
                {0x330b,0xb8},  
                {0x3e01,0xbb},    
                {0x0100, 0x01, 0x01}, // stream output on               
                {GADI_VI_SENSOR_TABLE_FLAG_END,},       
            },
            .vi_info =
            {
                .width      = 2560,
                .height     = 1440,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_MIPI_2LANES,
                .bits       = GADI_VIDEO_BITS_10,
                .ratio      = GADI_VIDEO_RATIO_16_9,
                .srm        = 0,
                .pll_index  = 0,
                .max_fps    = GADI_VIDEO_FPS(25),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(25),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_1080P_30,  
            },
            .video_info_table =
            {
                .def_start_x    = 0,
                .def_start_y    = 0,
                .def_width      = 2560,
                .def_height     = 1440,
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
            .update_after_vsync_start   = 3,
            .update_after_vsync_end     = 10,
        },
        [2] = //  2700 * 2250  * 20 = 121500000 : max 17fps 
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(2592, 1944,  20, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x0100, 0x00, 0x01}, // stream output hold
                {0x3029,0x37},        
                {0x302a,0x35},
                {0x302c,0x00},    
                {0x3039,0x04},
                {0x303a,0x35},   
                {0x3201,0x04},//xstar 4
                {0x3202,0x00},
                {0x3203,0x00},//ystar 0
                {0x3204,0x0a},               
                {0x3205,0x2B},//xend 4+8+2592-1=2603
                {0x3206,0x07},
                {0x3207,0x9f},//yend 0+8+1944-1=1951
                {0x3208,0x0a},                     
                {0x3209,0x20},//2592
                {0x320a,0x07},
                {0x320b,0x98},//1944
                {0x320c,0x05},     
                {0x320d,0x46},//hts: 1350*2 = 2700
                {0x320e,0x08},
                {0x320f,0xca},//vts:  2250       
                {0x3211,0x04},// x shift 4
                {0x3213,0x04},// y shift 4   
                {0x3f04,0x02},
                {0x3f05,0x9b},// hts/2 - [0x3F08] - 4 
                {0x3235,0x11},
                {0x3236,0x92},// vts x 2 - 2       
                {0x3301,0x1c}, 
                {0x330b,0xb8},  
                {0x3e01,0xf7},                
                {0x0100, 0x01, 0x01}, // stream output on               
                {GADI_VI_SENSOR_TABLE_FLAG_END,},    

            },
            .vi_info =
            {
                .width      = 2592,
                .height     = 1944,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_MIPI_2LANES,
                .bits       = GADI_VIDEO_BITS_10,
                .ratio      = GADI_VIDEO_RATIO_4_3,
                .srm        = 0,
                .pll_index  = 0,
                .max_fps    = GADI_VIDEO_FPS(20),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(20),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_1080P_30, 
            },
            .video_info_table =
            {
                .def_start_x    = 0,
                .def_start_y    = 0,
                .def_width      = 2592,
                .def_height     = 1944,
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
            .update_after_vsync_start   = 3,
            .update_after_vsync_end     = 10,
        },        
    },
    .mirror_table =
    {
#define SC5235_H_MIRROR     (3<<1)
#define SC5235_V_FLIP       (3<<5)
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY_VERTICALLY] =
        {
            {0x3221, SC5235_V_FLIP | SC5235_H_MIRROR, SC5235_V_FLIP | SC5235_H_MIRROR},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY] =
        {
            {0x3221, SC5235_H_MIRROR, SC5235_V_FLIP | SC5235_H_MIRROR},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_VERTICALLY] =
        {
            {0x3221, SC5235_V_FLIP, SC5235_V_FLIP | SC5235_H_MIRROR},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_NONE] =
        {
            {0x3221, 0x00, SC5235_V_FLIP | SC5235_H_MIRROR},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
    },
    .version_reg =
    {
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .reg_ctl_by_ver_num = 0,
    .control_by_version_reg = {0},
    .vmax_clk_ref   = GADI_SENSOR_CAL_CLK_PIXCLK_DIV2,         
    .shs_clk_ref    = GADI_SENSOR_CAL_CLK_PIXCLK_DIV2,
    .cal_shs_mode   = GADI_SENSOR_CAL_SHS_VMAX_MODE,
    .shs_fix        = 4,
    .shs_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_SET,
    .agc_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_SET,
    .vmax_reg_update_mode       = GADI_SENSOR_UPDATE_REG_BY_SET,
    .mirror_reg_update_mode     = GADI_SENSOR_UPDATE_REG_BY_VSYNC,
    .shs_additional_use         = 0,
    .shs_additional_percent     = 0,
    // hmax = (TIMING_HTS_H & 0xFF) << 8 +
    //        (TIMING_HTS_L & 0xFF) << 0
    .hmax_reg =
    {
        {0x320C, 0x00, 0xFF, 0x00, 0x08, 0}, // TIMING_HTS_H
        {0x320D, 0x00, 0xFF, 0x00, 0x00, 0}, // TIMING_HTS_L
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    // vmax = (TIMING_VTS_H & 0xFF) << 8 +
    //        (TIMING_VTS_L & 0xFF) << 0
    .vmax_reg =
    {
        {0x320E, 0x00, 0xFF, 0x00, 0x08, 0}, // TIMING_VTS_H
        {0x320F, 0x00, 0xFF, 0x00, 0x00, 0}, // TIMING_VTS_L
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .vmax_wadd_reg =
    {       
        {GADI_VI_SENSOR_TABLE_FLAG_END,},            
    },
    // shs = (AEC_EXP_TIME_H & 0xFF) << 4 +
    //       (AEC_EXP_TIME_L & 0xF0) >> 4
    .shs_reg =
    {
        {0x3E01, 0x00, 0xFF, 0x00, 0x04, 0}, // AEC_EXP_TIME_H
        {0x3E02, 0x00, 0xF0, 0x01, 0x04, 0}, // AEC_EXP_TIME_L
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .shs_additional_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    .cal_gain_mode      = GADI_SENSOR_CAL_GAIN_MODE1,
    .max_agc_index      = 977,
    .max_again_index    = 0,
    .cur_again_index    = 0,
    .agc_info =
    {
        .db_max     = 0x23FED3F1,    // 64x
        .db_min     = 0x00000000,    // 0dB
        .db_step    = 0x00100000,    // useless
    },    
    .gain_reg =
    {
/*        
        {0x3301, 0x00},
        {0x3633, 0x00},
        {0x3630, 0x00},
        {0x3622, 0x00},
        {0x363A, 0x00},
        {0x3E06, 0x00},
        {0x3E07, 0x00},
*/        
        {0x3E08, 0x00},
        {0x3E09, 0x00},      
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    // gain = (Digital gain(0x3E06[3:0]) * fine gain (0x3E07[7:0]))   *   (coarse gain(0x3E08[4:2] )* fine gain (0x3E09[7:0]))
    //{0x3E03} = 0x0B use gain_table
    //{0x3E03} = 0x03 gain = {0x3e08,0x3e09}/0x10
    .gain_table =
    {
        /*
                [gain<2]   
                {0x3301,0x1c},  //[f,1e]    20180810[14,20]     20180816[e,24]
                {0x3630,0x30},  //20180706
                {0x3633,0x23},//20180419
                {0x3622,0xf6},
                {0x363a,0x83},
        
                [4>gain>=2]
                {0x3301,0x20},  //[15,24]   20180810            20180816[14,28]
                {0x3630,0x23},  //20180327  
                {0x3633,0x33},//20180419
                {0x3622,0xf6},
                {0x363a,0x87},
        
                [8>gain>=4]
                {0x3301,0x24}, //[1a,26]    20180810            20180816[1a,2c]
                {0x3630,0x24}, //1114
                {0x3633,0x33},//20180419
                {0x3622,0xf6},
                {0x363a,0x9f},
        
                [15.75>gain>=8]
                {0x3301,0x30}, //[1e,54]    20180810            20180816[1f,50]
                {0x3630,0x16}, 
                {0x3633,0x33},//20180419
                {0x3622,0xf6},
                {0x363a,0x9f},
        
                [gain>=15.75]
                {0x3301,0x44}, //[1c,54]    20180810            20180816[1b,50]
                {0x3630,0x19}, //0f  1226
                {0x3633,0x45}, //20180829
                {0x3622,0x16}, //0627
                {0x363a,0x9f},
        */
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

