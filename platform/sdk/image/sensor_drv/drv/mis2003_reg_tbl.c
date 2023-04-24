/*!
*****************************************************************************
** \file        mis2003_reg_tbl.c
**
** \version     $Id: mis2003_reg_tbl.c 12863 2017-10-17 08:32:21Z hehuali $
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
GADI_VI_SensorDrvInfoT    mis2003_sensor =
{
    .magic_start        = GADI_SENSOR_HW_INFO_MAGIC_START,
    .version            = GADI_SENSOR_HW_INFO_VERSION,
    .length             = sizeof(GADI_VI_SensorDrvInfoT),
    .file_date          = 0x20170810,
    .HwInfo         =
    {
        .name               = "mis2003",
        .ctl_type           = GADI_SENSOR_CTL_I2C,
        .addr_len           = sizeof(GADI_U16),
        .data_len           = sizeof(GADI_U8),
        .reset_active_level = GADI_GPIO_LOW,
        .hw_addr            = {(0x60>>1), (0x62>>1), (0x64>>1), (0x66>>1)},  //hhl note: 0x60/0x62/0x64/0x66
        .id_reg =
        {
            {0x3000, 0x20}, 
            {0x3001, 0x03}, 
            //{0x3002, 0x01},  //chip version no.
        },
        .reset_reg =
        {
            {0x3006, 0x01}, // soft reset
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
    .sensor_id          = GADI_ISP_SENSOR_MIS2003,
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
    .max_width          = 1928,
    .max_height         = 1088,
    .def_sh_time        = GADI_VIDEO_FPS(30),
    .fmt_num            = 3,
    .auto_fmt           = GADI_VIDEO_MODE(1920,  1080,  25, 1),
    .init_reg           =
    {
        {0x3006, 0x04},  
        {0x3700, 0xD9}, // BLC 
        {0x3800, 0x00}, // DRNC  ARNC all open.  
        {0x3B00, 0x01}, // no overflow 
        {0x3B02, 0x10},  
        {0x3B04, 0x10},  
        {0x3B06, 0x10}, 
        {0x3B08, 0x10},   
        {0x3400, 0x0b}, //sample while falling edge. 
        //{0x3400, 0x09}, //sample while rising edge.   
        {0x410c, 0x42},  
        {0x400E, 0x24},  
        {0x4018, 0x18}, 
        {0x4020, 0x14}, 
        {0x4026, 0x1E}, 
        {0x402A, 0x26},  
        {0x402C, 0x3C}, 
        {0x4030, 0x34},  
        {0x4034, 0x34},  
        {0x4036, 0xE0},  //lens sharding.
        {0x4111, 0x0f},    
        {0x4110, 0x48},   
        {0x410E, 0x02}, //LM mode.  
        {0x3100, 0x04}, //shutter time  
        {0x3101, 0x64}, 

        {0x3009, 0x01},  //1024 hhl add: pad_drv, 0x00~0x03.
        //{0x3102, 0x00},  //PGA reg: 1x,2x,4x,8x
        //{0x3103, 0x00},  //ADC reg: 1x¡«8(1+31/32)x
        
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .pll_table =
    {
        [0] =
        {
            .pixclk = 74250000, // 
            .extclk = 27000000, //
            .regs =
            {
                {0x3300, 0x42},
                {0x3301, 0x02},
                {0x3302, 0x02},
                {0x3303, 0x04},
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
    },
    .video_fmt_table =
    {
        [0] = // 1920x1080P@30fps   //2200*1125*30=74250000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1920,  1080,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                //(2200*1125);
                {0x3200, 0x04},
                {0x3201, 0x65},  //1125
                {0x3202, 0x08},
                {0x3203, 0x98},  //2200
                
                {0x3204, 0x00},
                {0x3205, 0x04},
                {0x3206, 0x04},
                {0x3207, 0x3B},
                {0x3208, 0x00},
                {0x3209, 0x04},
                {0x320A, 0x07},
                {0x320B, 0x83},
                
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
                .ratio      = GADI_VIDEO_RATIO_4_3,
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
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_RG,
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_GR,
                },
            },
            .update_after_vsync_start   = 30,
            .update_after_vsync_end     = 33,
        },
        #if 1
        [1] = // 1920x1080P@25fps   //2640*1125*25=74250000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1920,  1080,  25, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                //(2200*1125);
                {0x3200, 0x04},
                {0x3201, 0x65},  //1125
                {0x3202, 0x0A},
                {0x3203, 0x50},  //2640
                
                {0x3204, 0x00},
                {0x3205, 0x04},
                {0x3206, 0x04},
                {0x3207, 0x3B},
                {0x3208, 0x00},
                {0x3209, 0x04},
                {0x320A, 0x07},
                {0x320B, 0x83},
                
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
                .ratio      = GADI_VIDEO_RATIO_4_3,
                .srm        = 0,
                .pll_index  = 0,
                .max_fps    = GADI_VIDEO_FPS(25),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(25),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_1080P_25,
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
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_RG,
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_GR,
                },
            },
            .update_after_vsync_start   = 37,
            .update_after_vsync_end     = 40,
        },
        #else
        [1] = // 1920x1080P@30fps   //2200*2250*15=74250000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1920,  1080,  15, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                //(2200*1125);
                {0x3200, 0x08},
                {0x3201, 0xca},  //2250
                {0x3202, 0x08},
                {0x3203, 0x98},  //2200
                
                {0x3204, 0x00},
                {0x3205, 0x00},  //1031 hhl modify 4 to 2, for anti-noise of the bottom lines.
                {0x3206, 0x04},
                {0x3207, 0x3F},
                {0x3208, 0x00},
                {0x3209, 0x00},
                {0x320A, 0x07},
                {0x320B, 0x87},
                
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
                .ratio      = GADI_VIDEO_RATIO_4_3,
                .srm        = 0,
                .pll_index  = 0,
                .max_fps    = GADI_VIDEO_FPS(15),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(15),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_1080P_30,
            },
            .video_info_table =
            {
                .def_start_x    = 4,
                .def_start_y    = 4,
                .def_width      = 1920,
                .def_height     = 1080,
                .sync_start     = (0 + 0),
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_BG,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_RG,
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_GR,
                },
            },
            .update_after_vsync_start   = 62,
            .update_after_vsync_end     = 66,
        },
        #endif
        [2] = // 1280*960P@25fps   //2640*1125*25=74250000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  960,  25, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                //(2200*1125);
                {0x3200, 0x04},
                {0x3201, 0x65},  //1125
                {0x3202, 0x08},
                {0x3203, 0x98},  //2200
                
                {0x3204, 0x00},
                {0x3205, 0x04},
                {0x3206, 0x04},
                {0x3207, 0x3B},
                {0x3208, 0x00},
                {0x3209, 0x04},
                {0x320A, 0x07},
                {0x320B, 0x83},
                
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
                .ratio      = GADI_VIDEO_RATIO_4_3,
                .srm        = 0,
                .pll_index  = 0,
                .max_fps    = GADI_VIDEO_FPS(25),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(25),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_720P_25,
            },
            .video_info_table =
            {
                .def_start_x    = 320,
                .def_start_y    = 60,
                .def_width      = 1280,
                .def_height     = 960,
                .sync_start     = (0 + 0),
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_BG,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_RG,
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_GR,
                },
            },
            .update_after_vsync_start   = 37,
            .update_after_vsync_end     = 40,
        },
    },
    .mirror_table =
    {
#define MIS2003_H_MIRROR     (1)
#define MIS2003_V_FLIP       (1<<1)
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY_VERTICALLY] =
        {
            {0x3007, MIS2003_V_FLIP | MIS2003_H_MIRROR, MIS2003_V_FLIP | MIS2003_H_MIRROR},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY] =
        {
            {0x3007, MIS2003_H_MIRROR, MIS2003_V_FLIP | MIS2003_H_MIRROR},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_VERTICALLY] =
        {
            {0x3007, MIS2003_V_FLIP, MIS2003_V_FLIP | MIS2003_H_MIRROR},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_NONE] =
        {
            {0x3007, 0x00, MIS2003_V_FLIP | MIS2003_H_MIRROR},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
    },
    .version_reg =
    {
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .reg_ctl_by_ver_num = 0,
    .control_by_version_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    .vmax_clk_ref   = GADI_SENSOR_CAL_CLK_PIXCLK,
    .shs_clk_ref    = GADI_SENSOR_CAL_CLK_PIXCLK,
    .cal_shs_mode   = GADI_SENSOR_CAL_SHS_VMAX_MODE,
    .shs_fix        = 4,
    .shs_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_VSYNC,
    .agc_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_VSYNC,
    .vmax_reg_update_mode       = GADI_SENSOR_UPDATE_REG_BY_SET,
    .mirror_reg_update_mode     = GADI_SENSOR_UPDATE_REG_BY_SET,
    .shs_additional_use         = 0,
    .shs_additional_percent     = 70,
    // hmax = (TIMING_HTS_H & 0xFF) << 8 +
    //        (TIMING_HTS_L & 0xFF) << 0
    .hmax_reg =
    {
        {0x3202, 0x00, 0xFF, 0x00, 0x08, 0}, // TIMING_HTS_H
        {0x3203, 0x00, 0xFF, 0x00, 0x00, 0}, // TIMING_HTS_L
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    // vmax = (TIMING_VTS_H & 0xFF) << 8 +
    //        (TIMING_VTS_L & 0xFF) << 0
    .vmax_reg =
    {
        {0x3200, 0x00, 0xFF, 0x00, 0x08, 0}, // TIMING_VTS_H
        {0x3201, 0x00, 0xFF, 0x00, 0x00, 0}, // TIMING_VTS_L
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
        {0x3100, 0x00, 0xFF, 0x00, 0x08, 0}, // AEC_EXP_TIME_H
        {0x3101, 0x00, 0xFF, 0x00, 0x00, 0}, // AEC_EXP_TIME_L
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .shs_additional_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    .cal_gain_mode      = GADI_SENSOR_CAL_GAIN_MODE1,
    .max_agc_index      = 128,
    .max_again_index    = 0,
    .cur_again_index    = 0,
    .agc_info =
    {
        .db_max     = 0x17dd1a1a,   // 23.863680db.
        .db_min     = 0x01000000,   // 1dB
        .db_step    = 0x00100000,   // 0.0625dB
    },
    .gain_reg =
    {
        {0x3102, 0x00},    //pga_gain
        {0x3103, 0x00},    //adc_gain      
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .gain_table =
    {
        {0x00000000,  {0x00,  0x00,}},  //1.000000,   //0.000000   //    
        {0x0044307a,  {0x00,  0x01,}},  //1.031250,   //0.266365   //    
        {0x008657c9,  {0x00,  0x02,}},  //1.062500,   //0.524777   //    
        {0x00c69426,  {0x00,  0x03,}},  //1.093750,   //0.775698   //    
        {0x0105013a,  {0x00,  0x04,}},  //1.125000,   //1.019550   //    
        {0x0141b86a,  {0x00,  0x05,}},  //1.156250,   //1.256720   //    
        {0x017cd110,  {0x00,  0x06,}},  //1.187500,   //1.487565   //    
        {0x01b660b8,  {0x00,  0x07,}},  //1.218750,   //1.712413   //    
        {0x01ee7b47,  {0x00,  0x08,}},  //1.250000,   //1.931569   //    
        {0x0225332b,  {0x00,  0x09,}},  //1.281250,   //2.145312   //    
        {0x025a997c,  {0x00,  0x0a,}},  //1.312500,   //2.353905   //    
        {0x028ebe1f,  {0x00,  0x0b,}},  //1.343750,   //2.557589   //    
        {0x02c1afdd,  {0x00,  0x0c,}},  //1.375000,   //2.756590   //    
        {0x02f37c81,  {0x00,  0x0d,}},  //1.406250,   //2.951119   //    
        {0x032430f0,  {0x00,  0x0e,}},  //1.437500,   //3.141372   //    
        {0x0353d935,  {0x00,  0x0f,}},  //1.468750,   //3.327533   //    
        {0x0382809d,  {0x00,  0x10,}},  //1.500000,   //3.509775   //     
        {0x03b031be,  {0x00,  0x11,}},  //1.531250,   //3.688259   //    
        {0x03dcf68e,  {0x00,  0x12,}},  //1.562500,   //3.863137   //    
        {0x0408d867,  {0x00,  0x13,}},  //1.593750,   //4.034552   //    
        {0x0433e01a,  {0x00,  0x14,}},  //1.625000,   //4.202638   //    
        {0x045e15f8,  {0x00,  0x15,}},  //1.656250,   //4.367523   //    
        {0x048781d8,  {0x00,  0x16,}},  //1.687500,   //4.529325   //    
        {0x04b02b24,  {0x00,  0x17,}},  //1.718750,   //4.688158   //    
        {0x04d818df,  {0x00,  0x18,}},  //1.750000,   //4.844130   //    
        {0x04ff51ae,  {0x00,  0x19,}},  //1.781250,   //4.997340   //    
        {0x0525dbda,  {0x00,  0x1a,}},  //1.812500,   //5.147886   //    
        {0x054bbd5e,  {0x00,  0x1b,}},  //1.843750,   //5.295858   //    
        {0x0570fbe4,  {0x00,  0x1c,}},  //1.875000,   //5.441344   //    
        {0x05959cd0,  {0x00,  0x1d,}},  //1.906250,   //5.584424   //    
        {0x05b9a541,  {0x00,  0x1e,}},  //1.937500,   //5.725178   //    
        {0x05dd1a1a,  {0x00,  0x1f,}},  //1.968750,   //5.863680   //  
        {0x06000000,  {0x01,  0x00,}},  //2.000000,   //6.000000   //   
        {0x0644307a,  {0x01,  0x01,}},  //2.062500,   //6.266365   //  
        {0x068657c9,  {0x01,  0x02,}},  //2.125000,   //6.524777   //  
        {0x06c69426,  {0x01,  0x03,}},  //2.187500,   //6.775698   //  
        {0x0705013a,  {0x01,  0x04,}},  //2.250000,   //7.019550   //  
        {0x0741b86a,  {0x01,  0x05,}},  //2.312500,   //7.256720   //  
        {0x077cd110,  {0x01,  0x06,}},  //2.375000,   //7.487565   //  
        {0x07b660b8,  {0x01,  0x07,}},  //2.437500,   //7.712413   //  
        {0x07ee7b47,  {0x01,  0x08,}},  //2.500000,   //7.931569   //  
        {0x0825332b,  {0x01,  0x09,}},  //2.562500,   //8.145312   //  
        {0x085a997c,  {0x01,  0x0a,}},  //2.625000,   //8.353905   //  
        {0x088ebe1f,  {0x01,  0x0b,}},  //2.687500,   //8.557589   //  
        {0x08c1afdd,  {0x01,  0x0c,}},  //2.750000,   //8.756590   //  
        {0x08f37c81,  {0x01,  0x0d,}},  //2.812500,   //8.951119   //  
        {0x092430f0,  {0x01,  0x0e,}},  //2.875000,   //9.141372   //  
        {0x0953d935,  {0x01,  0x0f,}},  //2.937500,   //9.327533   //  
        {0x0982809d,  {0x01,  0x10,}},  //3.000000,   //9.509775   //  
        {0x09b031be,  {0x01,  0x11,}},  //3.062500,   //9.688259   //  
        {0x09dcf68e,  {0x01,  0x12,}},  //3.125000,   //9.863137   //  
        {0x0a08d867,  {0x01,  0x13,}},  //3.187500,   //10.034552  // 
        {0x0a33e01a,  {0x01,  0x14,}},  //3.250000,   //10.202638  // 
        {0x0a5e15f8,  {0x01,  0x15,}},  //3.312500,   //10.367523  // 
        {0x0a8781d8,  {0x01,  0x16,}},  //3.375000,   //10.529325  // 
        {0x0ab02b24,  {0x01,  0x17,}},  //3.437500,   //10.688158  // 
        {0x0ad818df,  {0x01,  0x18,}},  //3.500000,   //10.844130  // 
        {0x0aff51ae,  {0x01,  0x19,}},  //3.562500,   //10.997340  // 
        {0x0b25dbda,  {0x01,  0x1a,}},  //3.625000,   //11.147886  // 
        {0x0b4bbd5e,  {0x01,  0x1b,}},  //3.687500,   //11.295858  // 
        {0x0b70fbe4,  {0x01,  0x1c,}},  //3.750000,   //11.441344  // 
        {0x0b959cd0,  {0x01,  0x1d,}},  //3.812500,   //11.584424  // 
        {0x0bb9a541,  {0x01,  0x1e,}},  //3.875000,   //11.725178  // 
        {0x0bdd1a1a,  {0x01,  0x1f,}},  //3.937500,   //11.863680  // 
        {0x0c000000,  {0x02,  0x00,}},  //4.000000,   //12.000000  // 
        {0x0c44307a,  {0x02,  0x01,}},  //4.125000,   //12.266365  // 
        {0x0c8657c9,  {0x02,  0x02,}},  //4.250000,   //12.524777  // 
        {0x0cc69426,  {0x02,  0x03,}},  //4.375000,   //12.775698  // 
        {0x0d05013a,  {0x02,  0x04,}},  //4.500000,   //13.019550  // 
        {0x0d41b86a,  {0x02,  0x05,}},  //4.625000,   //13.256720  // 
        {0x0d7cd110,  {0x02,  0x06,}},  //4.750000,   //13.487565  // 
        {0x0db660b8,  {0x02,  0x07,}},  //4.875000,   //13.712413  // 
        {0x0dee7b47,  {0x02,  0x08,}},  //5.000000,   //13.931569  // 
        {0x0e25332b,  {0x02,  0x09,}},  //5.125000,   //14.145312  // 
        {0x0e5a997c,  {0x02,  0x0a,}},  //5.250000,   //14.353905  // 
        {0x0e8ebe1f,  {0x02,  0x0b,}},  //5.375000,   //14.557589  // 
        {0x0ec1afdd,  {0x02,  0x0c,}},  //5.500000,   //14.756590  // 
        {0x0ef37c81,  {0x02,  0x0d,}},  //5.625000,   //14.951119  // 
        {0x0f2430f0,  {0x02,  0x0e,}},  //5.750000,   //15.141372  // 
        {0x0f53d935,  {0x02,  0x0f,}},  //5.875000,   //15.327533  // 
        {0x0f82809d,  {0x02,  0x10,}},  //6.000000,   //15.509775  // 
        {0x0fb031be,  {0x02,  0x11,}},  //6.125000,   //15.688259  // 
        {0x0fdcf68e,  {0x02,  0x12,}},  //6.250000,   //15.863137  // 
        {0x1008d867,  {0x02,  0x13,}},  //6.375000,   //16.034552  // 
        {0x1033e01a,  {0x02,  0x14,}},  //6.500000,   //16.202638  // 
        {0x105e15f8,  {0x02,  0x15,}},  //6.625000,   //16.367523  // 
        {0x108781d8,  {0x02,  0x16,}},  //6.750000,   //16.529325  // 
        {0x10b02b24,  {0x02,  0x17,}},  //6.875000,   //16.688158  // 
        {0x10d818df,  {0x02,  0x18,}},  //7.000000,   //16.844130  // 
        {0x10ff51ae,  {0x02,  0x19,}},  //7.125000,   //16.997340  // 
        {0x1125dbda,  {0x02,  0x1a,}},  //7.250000,   //17.147886  // 
        {0x114bbd5e,  {0x02,  0x1b,}},  //7.375000,   //17.295858  // 
        {0x1170fbe4,  {0x02,  0x1c,}},  //7.500000,   //17.441344  // 
        {0x11959cd0,  {0x02,  0x1d,}},  //7.625000,   //17.584424  // 
        {0x11b9a541,  {0x02,  0x1e,}},  //7.750000,   //17.725178  // 
        {0x11dd1a1a,  {0x02,  0x1f,}},  //7.875000,   //17.863680  // 
        {0x12000000,  {0x03,  0x00,}},  //8.000000,   //18.000000  // 
        {0x1244307a,  {0x03,  0x01,}},  //8.250000,   //18.266365  // 
        {0x128657c9,  {0x03,  0x02,}},  //8.500000,   //18.524777  // 
        {0x12c69426,  {0x03,  0x03,}},  //8.750000,   //18.775698  // 
        {0x1305013a,  {0x03,  0x04,}},  //9.000000,   //19.019550  // 
        {0x1341b86a,  {0x03,  0x05,}},  //9.250000,   //19.256720  // 
        {0x137cd110,  {0x03,  0x06,}},  //9.500000,   //19.487565  // 
        {0x13b660b8,  {0x03,  0x07,}},  //9.750000,   //19.712413  // 
        {0x13ee7b47,  {0x03,  0x08,}},  //10.000000,  //19.931569  //
        {0x1425332b,  {0x03,  0x09,}},  //10.250000,  //20.145312  //
        {0x145a997c,  {0x03,  0x0a,}},  //10.500000,  //20.353905  //
        {0x148ebe1f,  {0x03,  0x0b,}},  //10.750000,  //20.557589  //
        {0x14c1afdd,  {0x03,  0x0c,}},  //11.000000,  //20.756590  //
        {0x14f37c81,  {0x03,  0x0d,}},  //11.250000,  //20.951119  //
        {0x152430f0,  {0x03,  0x0e,}},  //11.500000,  //21.141372  //
        {0x1553d935,  {0x03,  0x0f,}},  //11.750000,  //21.327533  //
        {0x1582809d,  {0x03,  0x10,}},  //12.000000,  //21.509775  //
        {0x15b031be,  {0x03,  0x11,}},  //12.250000,  //21.688259  //
        {0x15dcf68e,  {0x03,  0x12,}},  //12.500000,  //21.863137  //
        {0x1608d867,  {0x03,  0x13,}},  //12.750000,  //22.034552  //
        {0x1633e01a,  {0x03,  0x14,}},  //13.000000,  //22.202638  //
        {0x165e15f8,  {0x03,  0x15,}},  //13.250000,  //22.367523  //
        {0x168781d8,  {0x03,  0x16,}},  //13.500000,  //22.529325  //
        {0x16b02b24,  {0x03,  0x17,}},  //13.750000,  //22.688158  //
        {0x16d818df,  {0x03,  0x18,}},  //14.000000,  //22.844130  //
        {0x16ff51ae,  {0x03,  0x19,}},  //14.250000,  //22.997340  //
        {0x1725dbda,  {0x03,  0x1a,}},  //14.500000,  //23.147886  //
        {0x174bbd5e,  {0x03,  0x1b,}},  //14.750000,  //23.295858  //
        {0x1770fbe4,  {0x03,  0x1c,}},  //15.000000,  //23.441344  //
        {0x17959cd0,  {0x03,  0x1d,}},  //15.250000,  //23.584424  //
        {0x17b9a541,  {0x03,  0x1e,}},  //15.500000,  //23.725178  //
        {0x17dd1a1a,  {0x03,  0x1f,}},  //15.750000,  //23.863680  //
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .temperatue_gain_table =
    {
        .agc_control_by_temperatue        = 0,
        .Aptina_temperatue_mode         = 0,
        .temperatue_control_reg_index    = 0,
        .temperatue_gain_num            = 0,
        .temperatue_reg                 = {GADI_VI_SENSOR_TABLE_FLAG_END,},
        .gain_reg                        = {GADI_VI_SENSOR_TABLE_FLAG_END,},
        .temperatue_gain                = {0},
        .temperatue_control             = {0},
    },
    .magic_end  = GADI_SENSOR_HW_INFO_MAGIC_END,
};

