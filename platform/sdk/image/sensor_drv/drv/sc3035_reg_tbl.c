/*!
*****************************************************************************
** \file        sc3035_reg_tbl.c
**
** \version     $Id: sc3035_reg_tbl.c 11914 2017-09-27 07:00:24Z hehuali $
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
GADI_VI_SensorDrvInfoT    sc3035_sensor =
{
    .magic_start        = GADI_SENSOR_HW_INFO_MAGIC_START,
    .version            = GADI_SENSOR_HW_INFO_VERSION,
    .length             = sizeof(GADI_VI_SensorDrvInfoT),
    .file_date          = 0x20171017,
    .HwInfo         =
    {
        .name               = "sc3035",
        .ctl_type           = GADI_SENSOR_CTL_I2C,
        .addr_len           = sizeof(GADI_U16),
        .data_len           = sizeof(GADI_U8),
        .reset_active_level = GADI_GPIO_LOW,
        .hw_addr            = {(0x60>>1), 0, 0, 0},
        .id_reg =
        {
            {0x3107, 0x30},
            {0x3108, 0x35},
        },
        .reset_reg =
        {
            {0x0103, 0x01}, // soft reset
            {GADI_VI_SENSOR_TABLE_FLAG_MSLEEP, 0x14},   // msleep(20);
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
    .sensor_id          = GADI_ISP_SENSOR_SC3035,
    .sensor_double_step = 32,
    .typeofsensor       = 2,
    .sync_time_delay    = 40,
    .source_type.dummy  = GADI_VI_CMOS_CHANNEL_TYPE_AUTO,
    .dev_type           = GADI_VI_DEV_TYPE_CMOS,
    .video_system       = GADI_VIDEO_SYSTEM_AUTO,
    .vs_polarity        = GADI_VI_FALLING_EDGE,
    .hs_polarity        = GADI_VI_RISING_EDGE,
    .data_edge          = GADI_VI_RISING_EDGE,
    .emb_sync_switch    = GADI_VI_EMB_SYNC_OFF,
    .emb_sync_loc       = GADI_VI_EMB_SYNC_LOWER_PEL,
    .emb_sync_mode      = GADI_VI_EMB_SYNC_ITU_656,
    .max_width          = 2304,
    .max_height         = 1536,
    .def_sh_time        = GADI_VIDEO_FPS(50),
    .fmt_num            = 4,
    .auto_fmt           = GADI_VIDEO_MODE(2304,  1536,  18, 1),
    .init_reg           =
    {
        {0x0103,0x01},// soft reset
        {0x0100,0x00},
        
        {0x4500,0x31}, // rnc sel
        {0x3416,0x11},
        {0x4501,0xa4}, // bit ctrl 
        
        {0x3e03,0x03}, // aec
        {0x3e08,0x00},
        {0x3e09,0x10},
        {0x3e01,0x30},
        
        //old timing
        {0x322e,0x00},
        {0x322f,0xaf},
        {0x3306,0x20},
        {0x3307,0x17},
        {0x330b,0x54},
        {0x3303,0x20},
        {0x3309,0x20},
        {0x3308,0x08},
        {0x331e,0x16},
        {0x331f,0x16},
        {0x3320,0x18},
        {0x3321,0x18},
        {0x3322,0x18},
        {0x3323,0x18},
        {0x330c,0x0b},
        {0x330f,0x07},
        {0x3310,0x42},
        {0x3324,0x07},
        {0x3325,0x07},
        {0x335b,0xca},
        {0x335e,0x07},
        {0x335f,0x10},
        {0x3334,0x00},
        
        //mem write
        {0x3F01,0x04},
        {0x3F04,0x01},
        {0x3F05,0x30},
        {0x3626,0x01},
        
        //analog
        {0x3635,0x60},
        {0x3631,0x84},
        {0x3636,0x8d}, //0607
        {0x3633,0x3f},
        {0x3639,0x80},
        //{0x3622,0x1e},
        {0x3622,0x02},  //hhl modify to 0x02, by smartsens.
        {0x3627,0x02},
        {0x3038,0xa4},
        {0x3621,0x18},
        {0x363a,0x1c},
        
        //ramp
        {0x3637,0xbe},
        {0x3638,0x85},
        {0x363c,0x48}, // ramp cur
        
        {0x5780,0xff}, // dpc
        {0x5781,0x04},
        {0x5785,0x10},
        
        //close mipi
        {0x301c,0xa4}, // dig
        {0x301a,0xf8}, // ana
        {0x3019,0xff},
        {0x3022,0x13},
        
        {0x301e,0xe0}, // [4] 0:close tempsens
        {0x3662,0x82},
        {0x3d0d,0x00}, // close random code
        
        //2048x1536
        //{0x3039,0x20},
        //{0x303a,0x35}, //74.25M pclk
        //{0x303b,0x00},
        
        {0x3306,0x46},
        {0x330b,0xa0},
        {0x3038,0xf8}, //pump clk div
        
        //{0x320c,0x05},	//hts=3000
        //{0x320d,0xdc},	
        //{0x320e,0x06},	//vts=1584
        //{0x320f,0x30},
        
        {0x3202,0x00}, // ystart=48
        {0x3203,0x00}, 
        {0x3206,0x06}, // yend=1545   1545 rows selected
        {0x3207,0x08},
        
        {0x3200,0x01}, // xstart= 264
        {0x3201,0x08},
        {0x3204,0x09}, // xend = 2319  2056 cols selected
        {0x3205,0x0f},
        
        {0x3211,0x04},	// xstart
        {0x3213,0x04},	// ystart 
        
        //2048x1536
        {0x3208,0x08},	//output_win_width 
        {0x3209,0x00},
        {0x320a,0x06},  //output_win_height 
        {0x320b,0x00},
        
        //0513 
        {0x3312,0x06}, // sa1 timing
        {0x3340,0x04},
        {0x3341,0xd2},
        {0x3342,0x01},
        {0x3343,0x80},
        {0x335d,0x2a}, // cmp timing
        {0x3348,0x04},
        {0x3349,0xd2},
        {0x334a,0x01},
        {0x334b,0x80},
        {0x3368,0x03}, // auto precharge
        {0x3369,0x30},
        {0x336a,0x06},
        {0x336b,0x30},
        {0x3367,0x05},
        {0x330e,0x17},
        
        //{0x3d08,0x00}, // pclk inv
        
        //fifo
        {0x303f,0x82},
        {0x3c03,0x28}, //fifo sram read position
        {0x3c00,0x45}, // Dig SRAM reset
        
        //0607
        {0x3c03,0x02}, //anti smear
        {0x3211,0x06},
        {0x3213,0x06},
        {0x3620,0x82},
        
        
        //logic change@ gain<2
        {0x3630,0xb1}, //0x67
        {0x3635,0x60}, //0x66
        
        //0704
        {0x3630,0x67},
        {0x3626,0x11},
        
        //0910
        {0x363c,0x88}, //fine gain correction
        {0x3312,0x00},
        {0x3333,0x80},
        {0x3334,0xa0},
        {0x3620,0x62},	//0xd2
        {0x3300,0x10},
        
        //0912
        {0x3627,0x06},
        {0x3312,0x06},
        {0x3340,0x03},
        {0x3341,0x80},
        {0x3334,0x20},
        
        {0x331e,0x10},
        {0x331f,0x13},
        {0x3320,0x18},
        {0x3321,0x18},
        
        //118.8M pclk 30fps
        //{0x3039,0x30},
        //{0x303a,0x2a}, //118.8M pclk
        //{0x303b,0x00},
        {0x3640,0x02},
        {0x3641,0x01},
        {0x5000,0x21},
        
        {0x3340,0x04},
        {0x3342,0x02},
        {0x3343,0x60},
        {0x334a,0x02},
        {0x334b,0x60},
        {0x3306,0x66},
        {0x3367,0x01},
        {0x330b,0xff},
        {0x3300,0x20},
        {0x331f,0x10},
        {0x3f05,0xe0},
        {0x3635,0x62},
        
        {0x3620,0x63}, //d2
        
        {0x3630,0x67}, //a9,
        
        //0926
        {0x3633,0x3c},
        {0x363a,0x04},
        
        //1022
        {0x3633,0x3d},
        {0x3300,0x30},
        {0x3f05,0xf8},
        {0x3343,0x70},
        {0x334b,0x70},
        
        //1028
        {0x3211,0x04},
        
        //1215
        
        {0x3208,0x09}, //2304
        {0x3209,0x00},
        {0x320a,0x05}, //1536 
        {0x320b,0x10},
        
        {0x3202,0x00}, // ystart=4
        {0x3203,0x04}, 
        {0x3206,0x06}, // yend=1544   1541 rows selected
        {0x3207,0x08},
        
        {0x3200,0x00}, // xstart= 144
        {0x3201,0x90},
        {0x3204,0x09}, // xend = 2455  2312 cols selected
        {0x3205,0x97},		
        
        {0x320c,0x04}, //hts=2500
        {0x320d,0xe2},	
        {0x320e,0x06}, //vts=1584
        {0x320f,0x30},
        
        {0x3368,0x03},
        {0x3369,0x30},
        {0x336a,0x06},
        {0x336b,0x30},
        
        {0x335e,0x05},
        {0x335f,0x0e},
        
        {0x330a,0x01},
        {0x330b,0x0e},
        
        //0923
        {0x3637,0xb9},
        
        //{0x320c,0x04},
        //{0x320d,0xd4},
        
        {0x3202,0x00}, // ystart=0
        {0x3203,0x00}, 
        {0x3206,0x06}, // yend=1544   1545 rows selected
        {0x3207,0x08},
        
        //{0x320e,0x06},		
        //{0x320f,0x46},
        
        {0x320a,0x06},
        {0x320b,0x00},
        
        {0x330a,0x00},
        {0x330b,0xb0},
        {0x3306,0x56},
        
        //0927 hhl add
        {0x3d08,0x01}, // pclk inv
        
        {0x0100,0x01},
        
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
        {0x3039,0x20},
        {0x303a,0x35}, //74.25M pclk
        {0x303b,0x00},
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
        [1] =
        {
            .pixclk = 118800000, 
            .extclk = 27000000, 
            .regs =
            {
        {0x3039,0x30}, //118.8M pclk
        {0x303a,0x2a},
        {0x303b,0x00},
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
        [2] =
        {
            .pixclk = 80250000, 
            .extclk = 29000000,   //29000000,  //hhl note: 19~20fps under 29M, 20fps under 29.5M
            .regs =
            {
        {0x3039,0x20},
        {0x303a,0x35}, //80.25M pclk
        {0x303b,0x00},
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
    },
    .video_fmt_table =
    {
        [0] = // 2304x1536P@18fps   //1236x1606x18.7=74250000   //1236x2x1668x18=74.22M
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(2304,  1536,  18, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                //hts&vts
                {0x320c,0x04}, //0x4d4=1236
                {0x320d,0xd4},
                {0x320e,0x06}, //0x646=1606
                {0x320f,0x46},
                //output_win: 2304x1536
                {0x3208,0x09}, //output_win_width 
                {0x3209,0x00},
                {0x320a,0x06}, //output_win_height 
                {0x320b,0x00},
                //output area.
                {0x3202,0x00}, // ystart=2
                {0x3203,0x02}, 
                {0x3206,0x06}, // yend=1544   1543 rows selected
                {0x3207,0x08},
                {0x3200,0x00}, // xstart= 144
                {0x3201,0x90},
                {0x3204,0x09}, // xend = 2455  2312 cols selected
                {0x3205,0x97},
                
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 2304,
                .height     = 1536,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_DVP,
                .bits       = GADI_VIDEO_BITS_12,
                .ratio      = GADI_VIDEO_RATIO_16_9,
                .srm        = 0,
                .pll_index  = 0,
                .max_fps    = GADI_VIDEO_FPS(18),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(18),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_720P_25,
            },
            .video_info_table =
            {
                .def_start_x    = 0,
                .def_start_y    = 0,
                .def_width      = 2304,
                .def_height     = 1536,
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
            .update_after_vsync_start   = 51,
            .update_after_vsync_end     = 55,
        },
        [1] = // 2048x1536P@18fps  
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(2048,  1536,  18, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                //hts&vts
                {0x320c,0x04}, //0x4d4=1236
                {0x320d,0xd4},	
                {0x320e,0x06}, //0x646=1606
                {0x320f,0x46},
                //output_win: 2304x1536
                {0x3208,0x09}, //output_win_width 
                {0x3209,0x00},
                {0x320a,0x06}, //output_win_height 
                {0x320b,0x00},
                //output area.
                {0x3202,0x00}, // ystart=2
                {0x3203,0x02}, 
                {0x3206,0x06}, // yend=1544   1543 rows selected
                {0x3207,0x08},
                {0x3200,0x00}, // xstart= 144
                {0x3201,0x90},
                {0x3204,0x09}, // xend = 2455  2312 cols selected
                {0x3205,0x97},
                
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 2048,
                .height     = 1536,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_DVP,
                .bits       = GADI_VIDEO_BITS_12,
                .ratio      = GADI_VIDEO_RATIO_4_3,
                .srm        = 0,
                .pll_index  = 0,
                .max_fps    = GADI_VIDEO_FPS(18),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(18),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_720P_25,
            },
            .video_info_table =
            {
                .def_start_x    = 0,
                .def_start_y    = 0,
                .def_width      = 2048,
                .def_height     = 1536,
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
            .update_after_vsync_start   = 51,
            .update_after_vsync_end     = 55,
        },
        [2] = // 2304x1296P@18fps   //1236x2x1668x18=74.22M   
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(2304,  1296,  18, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                //hts&vts
                {0x320c,0x04}, //0x4d4=1236
                {0x320d,0xd4},
                {0x320e,0x06}, //0x646=1606
                {0x320f,0x46},
                //output_win: 2304x1536
                {0x3208,0x09}, //output_win_width 
                {0x3209,0x00},
                {0x320a,0x06}, //output_win_height 
                {0x320b,0x00},
                //output area.
                {0x3202,0x00}, // ystart=2
                {0x3203,0x02}, 
                {0x3206,0x06}, // yend=1544   1543 rows selected
                {0x3207,0x08},
                {0x3200,0x00}, // xstart= 144
                {0x3201,0x90},
                {0x3204,0x09}, // xend = 2455  2312 cols selected
                {0x3205,0x97},
                
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 2304,
                .height     = 1296,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_DVP,
                .bits       = GADI_VIDEO_BITS_12,
                .ratio      = GADI_VIDEO_RATIO_16_9,
                .srm        = 0,
                .pll_index  = 0,
                .max_fps    = GADI_VIDEO_FPS(18),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(18),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_720P_25,
            },
            .video_info_table =
            {
                .def_start_x    = 0,
                .def_start_y    = 0,
                .def_width      = 2304,
                .def_height     = 1296,
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
            .update_after_vsync_start   = 51,
            .update_after_vsync_end     = 55,
        },
        [3] = // 1280x960P@25fps    //2500x1188x25=74250000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  960,  25, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                //hts&vts
                {0x320c,0x04}, //hts=2500  //0x4e2=1250
                {0x320d,0xe2},
                {0x320e,0x04}, //vts=1188  //0x4A4=1188
                {0x320f,0xA4},
                //output_win: 1280x960
                {0x3208,0x05}, //output_win_width 
                {0x3209,0x00},
                {0x320a,0x03}, //output_win_height 
                {0x320b,0xc0},
                //output area.
                {0x3202,0x00}, // ystart=241
                {0x3203,0xf1}, 
                {0x3206,0x04}, // yend=1216   976 rows selected
                {0x3207,0xC0},
                {0x3200,0x00}, // xstart= 241
                {0x3201,0xf1},
                {0x3204,0x06}, // xend = 1536  1296 cols selected
                {0x3205,0x00},
                
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 1280,
                .height     = 960,
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
                .def_height     = 960,
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
#define SC3035_H_MIRROR     (3<<1)  //(1<<1)picture err.
#define SC3035_V_FLIP       (3<<1)  //
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY_VERTICALLY] =
        {
            {0x3220, SC3035_V_FLIP, SC3035_V_FLIP},
            {0x3221, SC3035_H_MIRROR, SC3035_H_MIRROR},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY] =
        {
            {0x3220, 0x00, SC3035_V_FLIP},
            {0x3221, SC3035_H_MIRROR, SC3035_H_MIRROR},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_VERTICALLY] =
        {
            {0x3220, SC3035_V_FLIP, SC3035_V_FLIP},
            {0x3221, 0x00, SC3035_H_MIRROR},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_NONE] =
        {
            {0x3220, 0x00, SC3035_V_FLIP},
            {0x3221, 0x00, SC3035_H_MIRROR},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
    },
    .version_reg =
    {
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .reg_ctl_by_ver_num = 0,
    .control_by_version_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    .vmax_clk_ref   = GADI_SENSOR_CAL_CLK_PIXCLK_DIV2,
    .shs_clk_ref    = GADI_SENSOR_CAL_CLK_PIXCLK_DIV2,
    .cal_shs_mode   = GADI_SENSOR_CAL_SHS_VMAX_MODE,
    .shs_fix        = 4,
    .shs_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_SET,
    .agc_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_SET,
    .vmax_reg_update_mode       = GADI_SENSOR_UPDATE_REG_BY_SET,
    .mirror_reg_update_mode     = GADI_SENSOR_UPDATE_REG_BY_SET,
    .shs_additional_use         = 1,
    .shs_additional_percent     = 70,
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
    .max_agc_index      = 112,
    .max_again_index    = 64,
    .cur_again_index    = 0,
    .agc_info =
    {
        .db_max     = 0x29b9a541,    // 42dB
        .db_min     = 0x00000000,    // 0dB
        .db_step    = 0x00400000,    // useless
    },
    .gain_reg =
    {
        {0x3630, 0x00,},
        {0x3627, 0x00,},
        {0x3620, 0x00,},
        {0x3E08, 0x00,},
        {0x3E09, 0x00,},
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .gain_table =
    {                                                                                                                                                                                                                                                                                                                                               
      //        0x3630              0x3627              0x3620           0x3e08[15:0]   0x3e09[15:0]                                           
      //      restriction         restriction         restriction                                        
      // (  gain < 2: 0xa9 ) ( gain < 16: 0x02 ) (  gain < 2: 0xd2 )    //when 0x3e03=0x03
      // (  gain >= 2: 0x67) ( gain >= 16: 0x06) (  gain >= 2: 0x62)    //gain=(0x3e08,0x3e09)/0x10.  max:0x7c0 for 124x.                                 
      { 0x00000000,  {0xa9,  0x02,  0xd2,  0x00,  0x10, }},//  1.000000,  //0.000000   //
      { 0x008657c9,  {0xa9,  0x02,  0xd2,  0x00,  0x11, }},//  1.062500,  //0.524777   //
      { 0x0105013a,  {0xa9,  0x02,  0xd2,  0x00,  0x12, }},//  1.125000,  //1.019550   //
      { 0x017cd110,  {0xa9,  0x02,  0xd2,  0x00,  0x13, }},//  1.187500,  //1.487565   //
      { 0x01ee7b47,  {0xa9,  0x02,  0xd2,  0x00,  0x14, }},//  1.250000,  //1.931569   //
      { 0x025a997c,  {0xa9,  0x02,  0xd2,  0x00,  0x15, }},//  1.312500,  //2.353905   //
      { 0x02c1afdd,  {0xa9,  0x02,  0xd2,  0x00,  0x16, }},//  1.375000,  //2.756590   //
      { 0x032430f0,  {0xa9,  0x02,  0xd2,  0x00,  0x17, }},//  1.437500,  //3.141372   //
      { 0x0382809d,  {0xa9,  0x02,  0xd2,  0x00,  0x18, }},//  1.500000,  //3.509775   //
      { 0x03dcf68e,  {0xa9,  0x02,  0xd2,  0x00,  0x19, }},//  1.562500,  //3.863137   //
      { 0x0433e01a,  {0xa9,  0x02,  0xd2,  0x00,  0x1a, }},//  1.625000,  //4.202638   //
      { 0x048781d8,  {0xa9,  0x02,  0xd2,  0x00,  0x1b, }},//  1.687500,  //4.529325   //
      { 0x04d818df,  {0xa9,  0x02,  0xd2,  0x00,  0x1c, }},//  1.750000,  //4.844130   //
      { 0x0525dbda,  {0xa9,  0x02,  0xd2,  0x00,  0x1d, }},//  1.812500,  //5.147886   //
      { 0x0570fbe4,  {0xa9,  0x02,  0xd2,  0x00,  0x1e, }},//  1.875000,  //5.441344   //
      { 0x05b9a541,  {0xa9,  0x02,  0xd2,  0x00,  0x1f, }},//  1.937500,  //5.725178   //
      { 0x06000000,  {0x67,  0x02,  0x62,  0x00,  0x20, }},//  2.000000,  //6.000000   //
      { 0x068657c9,  {0x67,  0x02,  0x62,  0x00,  0x22, }},//  2.125000,  //6.524777   //
      { 0x0705013a,  {0x67,  0x02,  0x62,  0x00,  0x24, }},//  2.250000,  //7.019550   //
      { 0x077cd110,  {0x67,  0x02,  0x62,  0x00,  0x26, }},//  2.375000,  //7.487565   //
      { 0x07ee7b47,  {0x67,  0x02,  0x62,  0x00,  0x28, }},//  2.500000,  //7.931569   //
      { 0x085a997c,  {0x67,  0x02,  0x62,  0x00,  0x2a, }},//  2.625000,  //8.353905   //
      { 0x08c1afdd,  {0x67,  0x02,  0x62,  0x00,  0x2c, }},//  2.750000,  //8.756590   //
      { 0x092430f0,  {0x67,  0x02,  0x62,  0x00,  0x2e, }},//  2.875000,  //9.141372   //
      { 0x0982809d,  {0x67,  0x02,  0x62,  0x00,  0x30, }},//  3.000000,  //9.509775   //
      { 0x09dcf68e,  {0x67,  0x02,  0x62,  0x00,  0x32, }},//  3.125000,  //9.863137   //
      { 0x0a33e01a,  {0x67,  0x02,  0x62,  0x00,  0x34, }},//  3.250000,  //10.202638  //
      { 0x0a8781d8,  {0x67,  0x02,  0x62,  0x00,  0x36, }},//  3.375000,  //10.529325  //
      { 0x0ad818df,  {0x67,  0x02,  0x62,  0x00,  0x38, }},//  3.500000,  //10.844130  //
      { 0x0b25dbda,  {0x67,  0x02,  0x62,  0x00,  0x3a, }},//  3.625000,  //11.147886  //
      { 0x0b70fbe4,  {0x67,  0x02,  0x62,  0x00,  0x3c, }},//  3.750000,  //11.441344  //
      { 0x0bb9a541,  {0x67,  0x02,  0x62,  0x00,  0x3e, }},//  3.875000,  //11.725178  //
      { 0x0c000000,  {0x67,  0x02,  0x62,  0x00,  0x40, }},//  4.000000,  //12.000000  //
      { 0x0c8657c9,  {0x67,  0x02,  0x62,  0x00,  0x44, }},//  4.250000,  //12.524777  //
      { 0x0d05013a,  {0x67,  0x02,  0x62,  0x00,  0x48, }},//  4.500000,  //13.019550  //
      { 0x0d7cd110,  {0x67,  0x02,  0x62,  0x00,  0x4c, }},//  4.750000,  //13.487565  //
      { 0x0dee7b47,  {0x67,  0x02,  0x62,  0x00,  0x50, }},//  5.000000,  //13.931569  //
      { 0x0e5a997c,  {0x67,  0x02,  0x62,  0x00,  0x54, }},//  5.250000,  //14.353905  //
      { 0x0ec1afdd,  {0x67,  0x02,  0x62,  0x00,  0x58, }},//  5.500000,  //14.756590  //
      { 0x0f2430f0,  {0x67,  0x02,  0x62,  0x00,  0x5c, }},//  5.750000,  //15.141372  //
      { 0x0f82809d,  {0x67,  0x02,  0x62,  0x00,  0x60, }},//  6.000000,  //15.509775  //
      { 0x0fdcf68e,  {0x67,  0x02,  0x62,  0x00,  0x64, }},//  6.250000,  //15.863137  //
      { 0x1033e01a,  {0x67,  0x02,  0x62,  0x00,  0x68, }},//  6.500000,  //16.202638  //
      { 0x108781d8,  {0x67,  0x02,  0x62,  0x00,  0x6c, }},//  6.750000,  //16.529325  //
      { 0x10d818df,  {0x67,  0x02,  0x62,  0x00,  0x70, }},//  7.000000,  //16.844130  //
      { 0x1125dbda,  {0x67,  0x02,  0x62,  0x00,  0x74, }},//  7.250000,  //17.147886  //
      { 0x1170fbe4,  {0x67,  0x02,  0x62,  0x00,  0x78, }},//  7.500000,  //17.441344  //
      { 0x11b9a541,  {0x67,  0x02,  0x62,  0x00,  0x7c, }},//  7.750000,  //17.725178  //
      { 0x12000000,  {0x67,  0x02,  0x62,  0x00,  0x80, }},//  8.000000,  //18.000000  //
      { 0x128657c9,  {0x67,  0x02,  0x62,  0x00,  0x88, }},//  8.500000,  //18.524777  //
      { 0x1305013a,  {0x67,  0x02,  0x62,  0x00,  0x90, }},//  9.000000,  //19.019550  //
      { 0x137cd110,  {0x67,  0x02,  0x62,  0x00,  0x98, }},//  9.500000,  //19.487565  //
      { 0x13ee7b47,  {0x67,  0x02,  0x62,  0x00,  0xa0, }},// 10.000000,  //19.931569  //
      { 0x145a997c,  {0x67,  0x02,  0x62,  0x00,  0xa8, }},// 10.500000,  //20.353905  //
      { 0x14c1afdd,  {0x67,  0x02,  0x62,  0x00,  0xb0, }},// 11.000000,  //20.756590  //
      { 0x152430f0,  {0x67,  0x02,  0x62,  0x00,  0xb8, }},// 11.500000,  //21.141372  //
      { 0x1582809d,  {0x67,  0x02,  0x62,  0x00,  0xc0, }},// 12.000000,  //21.509775  //
      { 0x15dcf68e,  {0x67,  0x02,  0x62,  0x00,  0xc8, }},// 12.500000,  //21.863137  //
      { 0x1633e01a,  {0x67,  0x02,  0x62,  0x00,  0xd0, }},// 13.000000,  //22.202638  //
      { 0x168781d8,  {0x67,  0x02,  0x62,  0x00,  0xd8, }},// 13.500000,  //22.529325  //
      { 0x16d818df,  {0x67,  0x02,  0x62,  0x00,  0xe0, }},// 14.000000,  //22.844130  //
      { 0x1725dbda,  {0x67,  0x02,  0x62,  0x00,  0xe8, }},// 14.500000,  //23.147886  //
      { 0x1770fbe4,  {0x67,  0x02,  0x62,  0x00,  0xf0, }},// 15.000000,  //23.441344  //
      { 0x17b9a541,  {0x67,  0x02,  0x62,  0x00,  0xf8, }},// 15.500000,  //23.725178  //
      { 0x18000000,  {0x67,  0x06,  0x62,  0x01,  0x00, }},// 16.000000,  //24.000000  //
      { 0x188657c9,  {0x67,  0x06,  0x62,  0x01,  0x10, }},// 17.000000,  //24.524777  //
      { 0x1905013a,  {0x67,  0x06,  0x62,  0x01,  0x20, }},// 18.000000,  //25.019550  //
      { 0x197cd110,  {0x67,  0x06,  0x62,  0x01,  0x30, }},// 19.000000,  //25.487565  //
      { 0x19ee7b47,  {0x67,  0x06,  0x62,  0x01,  0x40, }},// 20.000000,  //25.931569  //
      { 0x1a5a997c,  {0x67,  0x06,  0x62,  0x01,  0x50, }},// 21.000000,  //26.353905  //
      { 0x1ac1afdd,  {0x67,  0x06,  0x62,  0x01,  0x60, }},// 22.000000,  //26.756590  //
      { 0x1b2430f0,  {0x67,  0x06,  0x62,  0x01,  0x70, }},// 23.000000,  //27.141372  //
      { 0x1b82809d,  {0x67,  0x06,  0x62,  0x01,  0x80, }},// 24.000000,  //27.509775  //
      { 0x1bdcf68e,  {0x67,  0x06,  0x62,  0x01,  0x90, }},// 25.000000,  //27.863137  //
      { 0x1c33e01a,  {0x67,  0x06,  0x62,  0x01,  0xa0, }},// 26.000000,  //28.202638  //
      { 0x1c8781d8,  {0x67,  0x06,  0x62,  0x01,  0xb0, }},// 27.000000,  //28.529325  //
      { 0x1cd818df,  {0x67,  0x06,  0x62,  0x01,  0xc0, }},// 28.000000,  //28.844130  //
      { 0x1d25dbda,  {0x67,  0x06,  0x62,  0x01,  0xd0, }},// 29.000000,  //29.147886  //
      { 0x1d70fbe4,  {0x67,  0x06,  0x62,  0x01,  0xe0, }},// 30.000000,  //29.441344  //
      { 0x1db9a541,  {0x67,  0x06,  0x62,  0x01,  0xf0, }},// 31.000000,  //29.725178  //
      { 0x1e000000,  {0x67,  0x06,  0x62,  0x02,  0x00, }},// 32.000000,  //30.000000  //
      { 0x1e8657c9,  {0x67,  0x06,  0x62,  0x02,  0x20, }},// 34.000000,  //30.524777  //
      { 0x1f05013a,  {0x67,  0x06,  0x62,  0x02,  0x40, }},// 36.000000,  //31.019550  //
      { 0x1f7cd110,  {0x67,  0x06,  0x62,  0x02,  0x60, }},// 38.000000,  //31.487565  //
      { 0x1fee7b47,  {0x67,  0x06,  0x62,  0x02,  0x80, }},// 40.000000,  //31.931569  //
      { 0x205a997c,  {0x67,  0x06,  0x62,  0x02,  0xa0, }},// 42.000000,  //32.353905  //
      { 0x20c1afdd,  {0x67,  0x06,  0x62,  0x02,  0xc0, }},// 44.000000,  //32.756590  //
      { 0x212430f0,  {0x67,  0x06,  0x62,  0x02,  0xe0, }},// 46.000000,  //33.141372  //
      { 0x2182809d,  {0x67,  0x06,  0x62,  0x03,  0x00, }},// 48.000000,  //33.509775  //
      { 0x21dcf68e,  {0x67,  0x06,  0x62,  0x03,  0x20, }},// 50.000000,  //33.863137  //
      { 0x2233e01a,  {0x67,  0x06,  0x62,  0x03,  0x40, }},// 52.000000,  //34.202638  //
      { 0x228781d8,  {0x67,  0x06,  0x62,  0x03,  0x60, }},// 54.000000,  //34.529325  //
      { 0x22d818df,  {0x67,  0x06,  0x62,  0x03,  0x80, }},// 56.000000,  //34.844130  //
      { 0x2325dbda,  {0x67,  0x06,  0x62,  0x03,  0xa0, }},// 58.000000,  //35.147886  //
      { 0x2370fbe4,  {0x67,  0x06,  0x62,  0x03,  0xc0, }},// 60.000000,  //35.441344  //
      { 0x23b9a541,  {0x67,  0x06,  0x62,  0x03,  0xe0, }},// 62.000000,  //35.725178  //
      { 0x24000000,  {0x67,  0x06,  0x62,  0x04,  0x00, }},// 64.000000,  //36.000000  //
      { 0x248657c9,  {0x67,  0x06,  0x62,  0x04,  0x40, }},// 68.000000,  //36.524777  //
      { 0x2505013a,  {0x67,  0x06,  0x62,  0x04,  0x80, }},// 72.000000,  //37.019550  //
      { 0x257cd110,  {0x67,  0x06,  0x62,  0x04,  0xc0, }},// 76.000000,  //37.487565  //
      { 0x25ee7b47,  {0x67,  0x06,  0x62,  0x05,  0x00, }},// 80.000000,  //37.931569  //
      { 0x265a997c,  {0x67,  0x06,  0x62,  0x05,  0x40, }},// 84.000000,  //38.353905  //
      { 0x26c1afdd,  {0x67,  0x06,  0x62,  0x05,  0x80, }},// 88.000000,  //38.756590  //
      { 0x272430f0,  {0x67,  0x06,  0x62,  0x05,  0xc0, }},// 92.000000,  //39.141372  //
      { 0x2782809d,  {0x67,  0x06,  0x62,  0x06,  0x00, }},// 96.000000,  //39.509775  //
      { 0x27dcf68e,  {0x67,  0x06,  0x62,  0x06,  0x40, }},//100.000000,  //39.863137  //
      { 0x2833e01a,  {0x67,  0x06,  0x62,  0x06,  0x80, }},//104.000000,  //40.202638  //
      { 0x288781d8,  {0x67,  0x06,  0x62,  0x06,  0xc0, }},//108.000000,  //40.529325  //
      { 0x28d818df,  {0x67,  0x06,  0x62,  0x07,  0x00, }},//112.000000,  //40.844130  //
      { 0x2925dbda,  {0x67,  0x06,  0x62,  0x07,  0x40, }},//116.000000,  //41.147886  //
      { 0x2970fbe4,  {0x67,  0x06,  0x62,  0x07,  0x80, }},//120.000000,  //41.441344  //
      { 0x29b9a541,  {0x67,  0x06,  0x62,  0x07,  0xc0, }},//124.000000,  //41.725178  //
      {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .temperatue_gain_table =
    {
        .agc_control_by_temperatue      = 0,
        .Aptina_temperatue_mode         = 0,
        .temperatue_control_reg_index   = 3,
        .temperatue_gain_num            = 10,
        .temperatue_reg                 = {GADI_VI_SENSOR_TABLE_FLAG_END,},
        .gain_reg                       =
        {
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        .temperatue_gain                =
        {
            0x00,
        },
        .temperatue_control             =
        {
            0x00,
        },
    },
    .magic_end  = GADI_SENSOR_HW_INFO_MAGIC_END,
};

