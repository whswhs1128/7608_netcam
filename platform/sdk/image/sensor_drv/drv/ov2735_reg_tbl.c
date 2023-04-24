/*!
*****************************************************************************
** \file        ov2735_reg_tbl.c
**
** \version     $Id: ov2735_reg_tbl.c 11627 2018-07-19 10:58:12Z baochao $
**
** \brief
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2018 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
GADI_VI_SensorDrvInfoT    ov2735_sensor =
{
    .magic_start        = GADI_SENSOR_HW_INFO_MAGIC_START,
    .version            = GADI_SENSOR_HW_INFO_VERSION,
    .length             = sizeof(GADI_VI_SensorDrvInfoT),
    .file_date          = 0x20180719,
    .HwInfo         =
    {
        .name               = "ov2735",
        .ctl_type           = GADI_SENSOR_CTL_I2C,
        .addr_len           = sizeof(GADI_U8),
        .data_len           = sizeof(GADI_U8),
        .reset_active_level = GADI_GPIO_LOW,
        .hw_addr            = {(0x78>>1), (0x7A>>1), 0, 0},
        .id_reg =
        {
            {0x02, 0x27}, // OV2735_CHIP_ID_H
            {0x03, 0x35}, // OV2735_CHIP_ID_L
        },
        .reset_reg =
        {
            {0xFD, 0x00}, //page0
            {0x20, 0x00, 0x01},//0: enable reset
            {GADI_VI_SENSOR_TABLE_FLAG_MSLEEP, 5},    // msleep(10);
            {0x20, 0x01, 0x01},//1: disable reset
            {GADI_VI_SENSOR_TABLE_FLAG_MSLEEP, 5},
            {0xFD, 0x01}, //page1
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
    .sensor_id          = GADI_ISP_SENSOR_OV2735,
    .sensor_double_step = 16,
    .typeofsensor       = 1,
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
    .max_width          = 2012,
    .max_height         = 1126,
    .def_sh_time        = GADI_VIDEO_FPS(60),
    .fmt_num            = 4,
    .auto_fmt           = GADI_VIDEO_MODE(1920, 1080,  30, 1),
    .init_reg           =
    {
        //colock &pad driver
        {0xFD, 0x00},
        {0x2F, 0x10},
        {0x34, 0x00},
        {0x30, 0x15},
        {0x33, 0x01},
        {0x35, 0x20},
        //{0X1B, 0X00},
        {0x1D, 0xFF}, //{0x1D, 0xA5},
        
        {0xFD, 0x01},
        {0x31, 0x00}, //1936x1096           
        {0x0D, 0x10},//0x00
        {0x30, 0x00},
        {0x03, 0x01},//exposure time H
        {0x04, 0x8F},//exposure time L
        {0x01, 0x01},//refresh
        {0x09, 0x00},   //HBLANK_H in "timer_clk" unit
        {0x0A, 0x20},   //HBLANK_L in "timer_clk" unit
        {0x06, 0x0A},//VBLANK_L in "H" unit
        {0x0E, 0x04},   //VTS_H in "H" unit
        {0x0F, 0x97},   //VTS_L in "H" unit

        {0x24, 0x10},
        {0x01, 0x01},//refresh
        {0xFB, 0x73},//BLC ctrl
        {0x01, 0x01},//refresh
        
        //timing ctrl
        {0xFD, 0x01},
        {0x1A, 0x6B},
        {0x1C, 0xEA},
        {0x16, 0x0C},
        {0x21, 0x00},
        {0x11, 0xE8},//63
        {0x19, 0xC3},
        {0x26, 0xDA},
        {0x29, 0x01},
        {0x33, 0x6F},
        {0x2A, 0xD2},
        {0x2C, 0x40},
        {0xD0, 0x02},
        {0xD1, 0x01},
        {0xD2, 0x20},
        {0xD3, 0x03},//04
        {0xD4, 0xA4},//2a
        {0x50, 0x00},
        {0x51, 0x2C},
        {0x52, 0x29},
        {0x53, 0x00},
        {0x55, 0x44},
        {0x58, 0x29},
        {0x5A, 0x00},
        {0x5B, 0x00},
        {0x5D, 0x00},
        {0x64, 0x2F},
        {0x66, 0x62},
        {0x68, 0x5B},
        {0x75, 0x46},
        {0x76, 0xF0},
        {0x77, 0x4F},
        {0x78, 0xEF},
        {0x72, 0xCF},
        {0x73, 0x36},
        {0x7D, 0x0D},
        {0x7E, 0x0D},
        {0x8A, 0x77},
        {0x8B, 0x77},
        
        //mipi ctrl
        {0xFD, 0x01},
        {0xB1, 0x82},//DPHY enable 8b
        {0xB3, 0x0B},//0b;09;1d
        {0xB4, 0x14},//MIPI PLL enable;14;35;36
        {0x9D, 0x40},//mipi hs dc level 40/03/55
        {0xA1, 0x05},//speed/03
        {0x94, 0x44},//dphy time
        {0x95, 0x33},//dphy time
        {0x96, 0x1F},//dphy time
        {0x98, 0x45},//dphy time
        {0x9C, 0x10},//dphy time
        {0xB5, 0x70},//30
        {0xA0, 0x00},//mipi enable
        {0x25, 0xE0},
        {0x20, 0x7B},
        {0x8F, 0x88},
        {0x91, 0x40},

        {0xFD, 0x01},
        {0xFD, 0x02},
        {0x36, 0x08},//invert Vsync polarity!!!!!!
        {0x5E, 0x03},
        {0xFD, 0x02},
        {0xA1, 0x04},
        {0xA3, 0x40},
        {0xA5, 0x02},
        {0xA7, 0xC4},
        
        //BLC ctrl
        {0xFD, 0x01},
        {0x86, 0x77},
        {0x89, 0x77},
        {0x87, 0x74},
        {0x88, 0x74},
        {0xFC, 0xE0},
        {0xFE, 0xE0},
        {0xF0, 0x40},
        {0xF1, 0x40},
        {0xF2, 0x40},
        {0xF3, 0x40},
            
        {0xFD, 0x02},
        {0x36, 0x08},   //invert Vsync polarity for Gaia
        {0xA0, 0x00},   //Image vertical start MSB3bits
        {0xA1, 0x08},   //Image vertical start LSB8bits
        {0xA2, 0x04},   //image vertical size  MSB8bits
        {0xA3, 0x38},   //image vertical size  LSB8bits 0x438=1080
        {0xA4, 0x00},   //
        {0xA5, 0x04},   //H start 8Lsb
        {0xA6, 0x03},   //
        {0xA7, 0xC0},   //Half H size Lsb8bits  0x3C0*2=1920
                
        {0xFD, 0x03},
        {0xc0, 0x01},//enable transfer OTP BP information

        {0xFD, 0x04},
        {0x21, 0x14},
        {0x22, 0x14},
        {0x23, 0x14},//enhance normal and dummy BPC
        
        {0xFD, 0x01},//page 01
        {0x06, 0xe0},//;insert dummy line , the frame rate is 30.01.
        {0x01, 0x01},//;
        {0xfd, 0x00},//
        {0x1b, 0x00},//;enable output78 06 e0	;insert dummy line , the frame rate is 30.01.
        {0x01, 0x01},//;
        {0xfd, 0x00},//
        {0x1b, 0x00},//;enable output

        {0xFD, 0x01},//page 01
        {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 50},// mdelay(2);
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .pll_table =
    {
        // OV2735_PLL_CTRL_BUF  0x2F
        //      bit[7] 0:disable bypass pll 1:enable bypass pll 
        //      bit[6:2] pll_nc  0~31
        //      bit[1:0] pll_mc  0~3
        // OV2735_PLL_OUTDIV    0x34
        //      bit[7:0] pll_outdiv   pll_clk = 24M*(pll_nc+3)/(pll_mc+1)/(pll_outdiv+1)
        // OV2735_CLK_MODE_BUF  0x30
        //      bit[7:0] pclk_ctrl 0~5  /1 /2 /4 /6 /8 /12
        [0] =
        {
            // for 1080P@30, 720P@60
            .pixclk = 84000000, // pixclk = 24M*(pll_nc+3)/(pll_mc+1)/(pll_outdiv+1)/p0:0x30[6:4]
            .extclk = 24000000, //        = 24M*(4     +3)/(0     +1)/(0         +1)/2 =84M
            .regs =
            {
                {0xFD, 0x00},
                {0x2F, 0x10},
                {0x34, 0x00},
                {0x30, 0x15},
                {0x33, 0x01},
                {0x35, 0x20},
                //{0X1B, 0X00},
                {0x1D, 0xFF},
                {0xFD, 0x01},//page 01
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x10},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,}
            },
        },
        
        [1] =
        {
            // for 1080P@30, 720P@60
            .pixclk = 74250000, // pixclk = 27M*(pll_nc+3)/(pll_mc+1)/(pll_outdiv+1)/p0:0x30[6:4]
            .extclk = 27000000, //        = 27M*(8     +3)/(1     +1)/(0         +1)/2 =74.25M
            .regs =
            {
                {0xFD, 0x00},
                {0x2F, 0x21},
                {0x34, 0x00},
                {0x30, 0x15},
                {0x33, 0x01},
                {0x35, 0x20},
                //{0X1B, 0X00},
                {0x1D, 0xFF},
                {0xFD, 0x01},//page 01    
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x10},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,}
            },
        },

        [2] =
        {
            // for 1080P@30, 720P@60
            .pixclk = 81000000, // pixclk = 27M*(pll_nc+3)/(pll_mc+1)/(pll_outdiv+1)/p0:0x30[6:4]
            .extclk = 27000000, //        = 27M*(9     +3)/(1     +1)/(0         +1)/2 =81M
            .regs =
            {
                {0xFD, 0x00},
                {0x2F, 0x25},
                {0x34, 0x00},
                {0x30, 0x15},
                {0x33, 0x01},
                {0x35, 0x20},
                //{0X1B, 0X00},
                {0x1D, 0xFF},
                {0xFD, 0x01},//page 01    
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x10},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,}
            },
        },      

        [3] =
        {
            // for 1080P@30, 720P@60
            .pixclk = 96000000, // pixclk = 24M*(pll_nc+3)/(pll_mc+1)/(pll_outdiv+1)/p0:0x30[6:4]
            .extclk = 24000000, //        = 24M*(13    +3)/(1     +1)/(0         +1)/2 =96M
            .regs =
            {
                {0xFD, 0x00},
                {0x2F, 0x35},
                {0x34, 0x00},
                {0x30, 0x15},
                {0x33, 0x01},
                {0x35, 0x20},
                //{0X1B, 0X00},
                {0x1D, 0xFF},
                {0xFD, 0x01},//page 01
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x10},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,}
            },
        },                     
    },
    .video_fmt_table =
    {
        // 1920x1080P: HTS must >= 2404, if HTS is to low , must fix the horizontal start before set mirror mode
        // 1280x720P : HTS must >= 1768, if HTS is to low , must fix the horizontal start before set mirror mode
        [0] =  // 1920x1080P@30fps  2106 * 1282 * 30 = 80996760 // 81000000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1920, 1080,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                //{0xFD, 0x00},   //page0
                //{0x1B, 0x1F},   //pad output disable
                {0xFD, 0x01},   //page1
                {0x31, 0x00},   //0:1936x1096 1:1288x728
                {0x0D, 0x10},   //{0x0D, 0x00},   //enable set frame length manually
                //{0x05, 0x00},   //VBLANK_H in "H" unit!!!!!
                {0x06, 0x0A},   //VBLANK_L in "H" unit
                {0x09, 0x00},   //HBLANK_H in "timer_clk" unit
                {0x0A, 0x20},   //HBLANK_L in "timer_clk" unit
                {0x01, 0x01},
                {0x0E, 0x04},   //VTS_H in "H" unit
                {0x0F, 0x97},   //VTS_L in "H" unit
                {0x01, 0x01},
                {0x11, 0xE0},   //
                {0x33, 0x6F},   //
                {0xD4, 0xA4},   //2a
                {0x55, 0x44},
                {0x72, 0xCF},
                {0x76, 0xF0},
                //{0x06, 0xE0},   //insert dummy line , the frame rate is 30.01.
                {0x01, 0x01},   //enable of frame sync signal                
                {0xFD, 0x02},   //page2
                {0x36, 0x08},   //invert Vsync polarity for Gaia
                {0xA0, 0x00},   //Image vertical start MSB3bits
                {0xA1, 0x08},   //Image vertical start LSB8bits
                {0xA2, 0x04},   //image vertical size  MSB8bits
                {0xA3, 0x38},   //image vertical size  LSB8bits 0x438=1080
                {0xA4, 0x00},   //
                {0xA5, 0x04},   //H start 8Lsb
                {0xA6, 0x03},   //
                {0xA7, 0xC0},   //Half H size Lsb8bits  0x3C0*2=1920
                {0xFD, 0x00},   //page0
                {0x1B, 0x00},   //pad output enable
                {0xFD, 0x01},   //page1
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 60},// mdelay(35);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 1920,
                .height     = 1080,
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
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_1080P_30,
            },
            .video_info_table =
            {
                .def_start_x    = 0,
                .def_start_y    = 0,
                .def_width      = 1920,
                .def_height     = 1080,
                .sync_start     = 0,
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_RG,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_RG,
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GR,
                    GADI_VI_BAYER_PATTERN_BG,
                },
            },
            .update_after_vsync_start   = 30,
            .update_after_vsync_end     = 33,
        },

        [1] = // 1280x960P@30fps    2106 * 1282 * 30 = 81000000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  960,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                //{0xFD, 0x00},   //page0
                //{0x1B, 0x1F},   //pad output disable
                {0xFD, 0x01},   //page1
                {0x31, 0x00},   //0:1936x1096 1:1288x728
                {0x0D, 0x10},   //{0x0D, 0x00},   //enable set frame length manually
                //{0x05, 0x00},   //VBLANK_H in "H" unit!!!!!
                {0x06, 0x0A},   //VBLANK_L in "H" unit
                {0x09, 0x00},   //HBLANK_H in "timer_clk" unit
                {0x0A, 0x20},   //HBLANK_L in "timer_clk" unit
                {0x01, 0x01},
                {0x0E, 0x05},   //VTS_H in "H" unit
                {0x0F, 0x02},   //VTS_L in "H" unit
                {0x01, 0x01},
                {0x11, 0xE0},   //
                {0x33, 0x6F},   //
                {0xD4, 0xA4},   //2a
                {0x55, 0x44},
                {0x72, 0xCF},
                {0x76, 0xF0},
                //{0x06, 0xE0},   //insert dummy line , the frame rate is 30.01.
                {0x01, 0x01},   //enable of frame sync signal                
                {0xFD, 0x02},   //page2
                {0x36, 0x08},   //invert Vsync polarity for Gaia
                {0xA0, 0x00},   //Image vertical start MSB3bits
                {0xA1, 0x3C},   //Image vertical start LSB8bits 
                {0xA2, 0x03},   //image vertical size  MSB8bits
                {0xA3, 0xC0},   //image vertical size  LSB8bits 0x3C0=960
                {0xA4, 0x01},   //H start 3Msb
                {0xA5, 0x40},   //H start 8Lsb
                {0xA6, 0x02},   //
                {0xA7, 0x80},   //Half H size Lsb8bits  0x280*2=1280
                {0xFD, 0x00},   //page0
                {0x1B, 0x00},   //pad output enable
                {0xFD, 0x01},   //page1
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 60},// mdelay(35);
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
                .ratio      = GADI_VIDEO_RATIO_16_9,
                .srm        = 0,
                .pll_index  = 2,
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
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GR,
                    GADI_VI_BAYER_PATTERN_BG,
                },
            },
            .update_after_vsync_start   = 13,
            .update_after_vsync_end     = 16,
        },                    

                    
        [2] = // 1280x720P@60fps    1800 * 888.9 * 60 = 96000000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  60, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                //{0xFD, 0x00},   //page0
                //{0x1B, 0x1F},   //pad output disable
                {0xFD, 0x01},   //page1
                {0x11, 0xE0},   //
                {0x33, 0x5F},   //
                {0xD4, 0x42},   //2a
                {0x55, 0x46},
                {0x76, 0x36},                    
                {0x72, 0xBF},
                {0x31, 0x01},   //0:1936x1096 1:1288x728
                {0x0D, 0x10},   //{0x0D, 0x00},   //enable set frame length manually
                {0x05, 0x00},   //VBLANK_H in "H" unit!!!!!
                {0x06, 0x0A},   //VBLANK_L in "H" unit
                //{0x01, 0x01},                
                {0x09, 0x00},   //HBLANK_H in "timer_clk" unit
                {0x0A, 0xCB},   //HBLANK_L in "timer_clk" unit   0xcb   1800 pixel colock
                {0x01, 0x01},
                {0x0E, 0x03},
                {0x0F, 0x79},//VTS  889 
                {0x01, 0x01},               

//                {0xFD, 0x02},   //page2
//                {0x62, 0xB9},   //modify the row start position for OTP BPC
//                {0x63, 0x00},
//                {0x01, 0x01},            
                {0xFD, 0x02},   //page2
                {0x36, 0x08},   //invert Vsync polarity for Gaia
                {0xA0, 0x00},   //Image vertical start MSB3bits
                {0xA1, 0x04},   //Image vertical start LSB8bits
                {0xA2, 0x02},   //image vertical size  MSB8bits
                {0xA3, 0xD0},   //image vertical size  LSB8bits 0x2D0=720
                {0xA4, 0x00},   //
                {0xA5, 0x04},   //H start 8Lsb
                {0xA6, 0x02},   //
                {0xA7, 0x80},   //Half H size Lsb8bits  0x280*2=1280
                {0xFD, 0x00},   //page0
                {0x1B, 0x00},   //pad output enable
                {0xFD, 0x01},   //page1
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 60},// mdelay(60);          
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
                .pll_index  = 3,
                .max_fps    = GADI_VIDEO_FPS(60),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(60),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_720P_60,
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
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GR,
                    GADI_VI_BAYER_PATTERN_BG,
                },
            },
            .update_after_vsync_start   = 13,
            .update_after_vsync_end     = 16,
        },

        [3] = // 1280x720P@50fps    1800 * 900 * 50 = 81000000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  50, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                //{0xFD, 0x00},   //page0
                //{0x1B, 0x1F},   //pad output disable
                {0xFD, 0x01},   //page1  
                {0x11, 0xE0},
                {0x33, 0x5f},                 
                {0xD4, 0x42},   //2a
                {0x55, 0x46},
                {0x76, 0x36},
                {0x72, 0xBF},              
                {0x31, 0x01},   //0:1936x1096 1:1288x728
                //{0x01, 0x01},                
                {0x0D, 0x10},   //{0x0D, 0x00},   //enable set frame length manually
                {0x05, 0x00},   //VBLANK_H in "H" unit!!!!!
                {0x06, 0x0A},   //VBLANK_L in "H" unit                
                {0x09, 0x00},   //HBLANK_H in "timer_clk" unit
                {0x0A, 0xCB},   //HBLANK_L in "timer_clk" unit   0xcb   1800 pixel colock
                {0x01, 0x01},  
                {0x0E, 0x03},
                {0x0F, 0x84},//VTS  900
                {0x01, 0x01},  

                //{0xFD, 0x02},   //page2
                //{0x62, 0xB9},   //modify the row start position for OTP BPC
                //{0x63, 0x00},                                   
                {0xFD, 0x02},   //page2
                {0x36, 0x08},   //invert Vsync polarity for Gaia
                {0xA0, 0x00},   //Image vertical start MSB3bits
                {0xA1, 0x04},   //Image vertical start LSB8bits
                {0xA2, 0x02},   //image vertical size  MSB8bits
                {0xA3, 0xD0},   //image vertical size  LSB8bits 0x2D0=720
                {0xA4, 0x00},   //
                {0xA5, 0x04},   //H start 8Lsb
                {0xA6, 0x02},   //
                {0xA7, 0x80},   //Half H size Lsb8bits  0x280*2=1280
                {0xFD, 0x00},   //page0
                {0x1B, 0x00},   //pad output enable
                {0xFD, 0x01},   //page1
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 60},// mdelay(60);
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
                .max_fps    = GADI_VIDEO_FPS(50),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(50),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_720P_50,
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
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_GR,
                    GADI_VI_BAYER_PATTERN_BG,
                },
            },
            .update_after_vsync_start   = 13,
            .update_after_vsync_end     = 16,
        },                    
    },
    .mirror_table =
    {
#define OV2735_HORIZ_MIRROR     (0x01 << 0)
#define OV2735_VERT_FLIP        (0x01 << 1)
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY_VERTICALLY] =
        {
            {0xFD, 0x02},   //page2
            {0x62, 0x48},
            {0x63, 0x04},
            {0xFD, 0x01},   //page1
            {0x3F, OV2735_HORIZ_MIRROR | OV2735_VERT_FLIP, OV2735_HORIZ_MIRROR | OV2735_VERT_FLIP},
            {0xF8, 0x02, 0x02},
            {0x01, 0x01},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY] =
        {
            {0xFD, 0x02},   //page2
            {0x62, 0x01},
            {0x63, 0x00},
            {0xFD, 0x01},   //page1
            {0x3F, OV2735_HORIZ_MIRROR , OV2735_HORIZ_MIRROR | OV2735_VERT_FLIP},
            {0xF8, 0x00, 0x02},
            {0x01, 0x01},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_VERTICALLY] =
        {
            {0xFD, 0x02},   //page2
            {0x62, 0x48},
            {0x63, 0x04},
            {0xFD, 0x01},   //page1
            {0x3F, OV2735_VERT_FLIP, OV2735_HORIZ_MIRROR | OV2735_VERT_FLIP},
            {0xF8, 0x02, 0x02},
            {0x01, 0x01},                
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_NONE] =
        {
            {0xFD, 0x02},   //page2
            {0x62, 0x01},
            {0x63, 0x00},
            {0xFD, 0x01},   //page1
            {0x3F, 0x00, OV2735_HORIZ_MIRROR | OV2735_VERT_FLIP},
            {0xF8, 0x00, 0x02},
            {0x01, 0x01}, 
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
    .shs_fix        = 3,
    .shs_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_SET,
    .agc_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_SET,
    .vmax_reg_update_mode       = GADI_SENSOR_UPDATE_REG_BY_SET,
    .mirror_reg_update_mode     = GADI_SENSOR_UPDATE_REG_BY_SET,
    .shs_additional_use         = 0,
    .shs_additional_percent     = 0,
    // hmax = (OV2735_TIMING_CONTROL_HTS_HIGHBYTE & 0xFF) << 8 +
    //        (OV2735_TIMING_CONTROL_HTS_LOWBYTE & 0xFF) << 0
    .hmax_reg =
    {
        {0x8C, 0x00, 0x1F, 0x00, 0x08, 0}, // OV2735_TIMING_CONTROL_VTS_HIGHBYTE
        {0x8D, 0x00, 0xFF, 0x00, 0x00, 0}, // OV2735_TIMING_CONTROL_VTS_LOWBYTE   
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    // vmax = (OV2735_TIMING_CONTROL_VTS_HIGHBYTE & 0xFF) << 8 +
    //        (OV2735_TIMING_CONTROL_VTS_LOWBYTE & 0xFF) << 0
    .vmax_reg =
    {
        {0x0E, 0x00, 0xFF, 0x00, 0x08, 0}, // OV2735_TIMING_CONTROL_VTS_HIGHBYTE
        {0x0F, 0x00, 0xFF, 0x00, 0x00, 0}, // OV2735_TIMING_CONTROL_VTS_LOWBYTE     
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .vmax_wadd_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    // shs = (OV2735_AEC_PK_EXPO_H & 0x0F) << 8 +
    //       (OV2735_AEC_PK_EXPO_M & 0xFF) << 0
    .shs_reg =
    {
        {0x03, 0x00, 0xFF, 0x00, 0x08, 0}, // OV2735_AEC_PK_EXPO_H
        {0x04, 0x00, 0xFF, 0x00, 0x00, 0}, // OV2735_AEC_PK_EXPO_L
        {GADI_VI_SENSOR_TABLE_FLAG_WRITE_REG | 0x01, 0x01, 0x00, 0x00, 0x00, 0x00},//{flag|reg,data,mask,0,0,0} refresh
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .shs_additional_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    .cal_gain_mode      = GADI_SENSOR_CAL_GAIN_MODE1,
    .max_agc_index      = 368,
    .max_again_index    = 0,
    .cur_again_index    = 0,
    .agc_info =
    {
        .db_max     = 0x1E08F64B,    // 30.035008dB*(1<<24)   
        .db_min     = 0x00000000,    // 0dB
        .db_step    = 0x00600000,    // useless
    },
    .gain_reg =
    {
        {0xFD,}, // select page 1
        {0x24,}, // pga gain contrl
        {0x39,}, // global digital gain
        {0x01,}, // refresh
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .gain_table =
    {
        {0x00000000, {1, 0x10, 0x80, 1, }},/*gain=1.000000(0.000000 db)*/
        {0x0086CDDD, {1, 0x11, 0x80, 1, }},/*gain=1.062500(0.526579 db)*/
        {0x0105E6A2, {1, 0x12, 0x80, 1, }},/*gain=1.125000(1.023050 db)*/
        {0x017E1FC6, {1, 0x13, 0x80, 1, }},/*gain=1.187500(1.492672 db)*/
        {0x01F02DE4, {1, 0x14, 0x80, 1, }},/*gain=1.250000(1.938200 db)*/
        {0x025CAB21, {1, 0x15, 0x80, 1, }},/*gain=1.312500(2.361986 db)*/
        {0x02C41C1C, {1, 0x16, 0x80, 1, }},/*gain=1.375000(2.766054 db)*/
        {0x0326F3C3, {1, 0x17, 0x80, 1, }},/*gain=1.437500(3.152157 db)*/
        {0x03859655, {1, 0x18, 0x80, 1, }},/*gain=1.500000(3.521825 db)*/
        {0x03E05BC8, {1, 0x19, 0x80, 1, }},/*gain=1.562500(3.876401 db)*/
        {0x043791B9, {1, 0x1A, 0x80, 1, }},/*gain=1.625000(4.217067 db)*/
        {0x048B7CF8, {1, 0x1B, 0x80, 1, }},/*gain=1.687500(4.544876 db)*/
        {0x04DC5AD4, {1, 0x1C, 0x80, 1, }},/*gain=1.750000(4.860761 db)*/
        {0x052A6228, {1, 0x1D, 0x80, 1, }},/*gain=1.812500(5.165560 db)*/
        {0x0575C43A, {1, 0x1E, 0x80, 1, }},/*gain=1.875000(5.460025 db)*/
        {0x05BEAD74, {1, 0x1F, 0x80, 1, }},/*gain=1.937500(5.744834 db)*/
        {0x06054609, {1, 0x20, 0x80, 1, }},/*gain=2.000000(6.020600 db)*/
        {0x0649B272, {1, 0x21, 0x80, 1, }},/*gain=2.062500(6.287879 db)*/
        {0x068C13E7, {1, 0x22, 0x80, 1, }},/*gain=2.125000(6.547179 db)*/
        {0x06CC88B9, {1, 0x23, 0x80, 1, }},/*gain=2.187500(6.798961 db)*/
        {0x070B2CAB, {1, 0x24, 0x80, 1, }},/*gain=2.250000(7.043650 db)*/
        {0x07481938, {1, 0x25, 0x80, 1, }},/*gain=2.312500(7.281635 db)*/
        {0x078365CF, {1, 0x26, 0x80, 1, }},/*gain=2.375000(7.513272 db)*/
        {0x07BD280E, {1, 0x27, 0x80, 1, }},/*gain=2.437500(7.738892 db)*/
        {0x07F573ED, {1, 0x28, 0x80, 1, }},/*gain=2.500000(7.958800 db)*/
        {0x082C5BE9, {1, 0x29, 0x80, 1, }},/*gain=2.562500(8.173277 db)*/
        {0x0861F12A, {1, 0x2A, 0x80, 1, }},/*gain=2.625000(8.382586 db)*/
        {0x089643A1, {1, 0x2B, 0x80, 1, }},/*gain=2.687500(8.586969 db)*/
        {0x08C96226, {1, 0x2C, 0x80, 1, }},/*gain=2.750000(8.786654 db)*/
        {0x08FB5A8F, {1, 0x2D, 0x80, 1, }},/*gain=2.812500(8.981851 db)*/
        {0x092C39CD, {1, 0x2E, 0x80, 1, }},/*gain=2.875000(9.172757 db)*/
        {0x095C0BF5, {1, 0x2F, 0x80, 1, }},/*gain=2.937500(9.359558 db)*/
        {0x098ADC5E, {1, 0x30, 0x80, 1, }},/*gain=3.000000(9.542425 db)*/
        {0x09B8B5A9, {1, 0x31, 0x80, 1, }},/*gain=3.062500(9.721522 db)*/
        {0x09E5A1D2, {1, 0x32, 0x80, 1, }},/*gain=3.125000(9.897000 db)*/
        {0x0A11AA3C, {1, 0x33, 0x80, 1, }},/*gain=3.187500(10.069004 db)*/
        {0x0A3CD7C2, {1, 0x34, 0x80, 1, }},/*gain=3.250000(10.237667 db)*/
        {0x0A6732B9, {1, 0x35, 0x80, 1, }},/*gain=3.312500(10.403118 db)*/
        {0x0A90C301, {1, 0x36, 0x80, 1, }},/*gain=3.375000(10.565476 db)*/
        {0x0AB9900A, {1, 0x37, 0x80, 1, }},/*gain=3.437500(10.724854 db)*/
        {0x0AE1A0DD, {1, 0x38, 0x80, 1, }},/*gain=3.500000(10.881361 db)*/
        {0x0B08FC25, {1, 0x39, 0x80, 1, }},/*gain=3.562500(11.035097 db)*/
        {0x0B2FA832, {1, 0x3A, 0x80, 1, }},/*gain=3.625000(11.186160 db)*/
        {0x0B55AB01, {1, 0x3B, 0x80, 1, }},/*gain=3.687500(11.334641 db)*/
        {0x0B7B0A43, {1, 0x3C, 0x80, 1, }},/*gain=3.750000(11.480625 db)*/
        {0x0B9FCB60, {1, 0x3D, 0x80, 1, }},/*gain=3.812500(11.624197 db)*/
        {0x0BC3F37D, {1, 0x3E, 0x80, 1, }},/*gain=3.875000(11.765434 db)*/
        {0x0BE78780, {1, 0x3F, 0x80, 1, }},/*gain=3.937500(11.904411 db)*/
        {0x0C0A8C12, {1, 0x40, 0x80, 1, }},/*gain=4.000000(12.041200 db)*/
        {0x0C2D05A6, {1, 0x41, 0x80, 1, }},/*gain=4.062500(12.175867 db)*/
        {0x0C4EF87B, {1, 0x42, 0x80, 1, }},/*gain=4.125000(12.308479 db)*/
        {0x0C70689F, {1, 0x43, 0x80, 1, }},/*gain=4.187500(12.439096 db)*/
        {0x0C9159F0, {1, 0x44, 0x80, 1, }},/*gain=4.250000(12.567779 db)*/
        {0x0CB1D022, {1, 0x45, 0x80, 1, }},/*gain=4.312500(12.694582 db)*/
        {0x0CD1CEC2, {1, 0x46, 0x80, 1, }},/*gain=4.375000(12.819561 db)*/
        {0x0CF15932, {1, 0x47, 0x80, 1, }},/*gain=4.437500(12.942767 db)*/
        {0x0D1072B4, {1, 0x48, 0x80, 1, }},/*gain=4.500000(13.064250 db)*/
        {0x0D2F1E65, {1, 0x49, 0x80, 1, }},/*gain=4.562500(13.184058 db)*/
        {0x0D4D5F41, {1, 0x4A, 0x80, 1, }},/*gain=4.625000(13.302235 db)*/
        {0x0D6B3827, {1, 0x4B, 0x80, 1, }},/*gain=4.687500(13.418826 db)*/
        {0x0D88ABD9, {1, 0x4C, 0x80, 1, }},/*gain=4.750000(13.533872 db)*/
        {0x0DA5BCFA, {1, 0x4D, 0x80, 1, }},/*gain=4.812500(13.647415 db)*/
        {0x0DC26E18, {1, 0x4E, 0x80, 1, }},/*gain=4.875000(13.759492 db)*/
        {0x0DDEC1A3, {1, 0x4F, 0x80, 1, }},/*gain=4.937500(13.870142 db)*/
        {0x0DFAB9F6, {1, 0x50, 0x80, 1, }},/*gain=5.000000(13.979400 db)*/
        {0x0E165957, {1, 0x51, 0x80, 1, }},/*gain=5.062500(14.087301 db)*/
        {0x0E31A1F2, {1, 0x52, 0x80, 1, }},/*gain=5.125000(14.193877 db)*/
        {0x0E4C95E4, {1, 0x53, 0x80, 1, }},/*gain=5.187500(14.299162 db)*/
        {0x0E673733, {1, 0x54, 0x80, 1, }},/*gain=5.250000(14.403186 db)*/
        {0x0E8187D4, {1, 0x55, 0x80, 1, }},/*gain=5.312500(14.505979 db)*/
        {0x0E9B89AA, {1, 0x56, 0x80, 1, }},/*gain=5.375000(14.607569 db)*/
        {0x0EB53E87, {1, 0x57, 0x80, 1, }},/*gain=5.437500(14.707985 db)*/
        {0x0ECEA82F, {1, 0x58, 0x80, 1, }},/*gain=5.500000(14.807254 db)*/
        {0x0EE7C853, {1, 0x59, 0x80, 1, }},/*gain=5.562500(14.905400 db)*/
        {0x0F00A099, {1, 0x5A, 0x80, 1, }},/*gain=5.625000(15.002451 db)*/
        {0x0F193297, {1, 0x5B, 0x80, 1, }},/*gain=5.687500(15.098428 db)*/
        {0x0F317FD6, {1, 0x5C, 0x80, 1, }},/*gain=5.750000(15.193357 db)*/
        {0x0F4989D3, {1, 0x5D, 0x80, 1, }},/*gain=5.812500(15.287259 db)*/
        {0x0F6151FF, {1, 0x5E, 0x80, 1, }},/*gain=5.875000(15.380157 db)*/
        {0x0F78D9BD, {1, 0x5F, 0x80, 1, }},/*gain=5.937500(15.472072 db)*/
        {0x0F902268, {1, 0x60, 0x80, 1, }},/*gain=6.000000(15.563025 db)*/
        {0x0FA72D4D, {1, 0x61, 0x80, 1, }},/*gain=6.062500(15.653035 db)*/
        {0x0FBDFBB2, {1, 0x62, 0x80, 1, }},/*gain=6.125000(15.742122 db)*/
        {0x0FD48ED1, {1, 0x63, 0x80, 1, }},/*gain=6.187500(15.830304 db)*/
        {0x0FEAE7DB, {1, 0x64, 0x80, 1, }},/*gain=6.250000(15.917600 db)*/
        {0x100107F7, {1, 0x65, 0x80, 1, }},/*gain=6.312500(16.004028 db)*/
        {0x1016F046, {1, 0x66, 0x80, 1, }},/*gain=6.375000(16.089604 db)*/
        {0x102CA1DD, {1, 0x67, 0x80, 1, }},/*gain=6.437500(16.174345 db)*/
        {0x10421DCB, {1, 0x68, 0x80, 1, }},/*gain=6.500000(16.258267 db)*/
        {0x10576518, {1, 0x69, 0x80, 1, }},/*gain=6.562500(16.341386 db)*/
        {0x106C78C2, {1, 0x6A, 0x80, 1, }},/*gain=6.625000(16.423718 db)*/
        {0x108159C2, {1, 0x6B, 0x80, 1, }},/*gain=6.687500(16.505276 db)*/
        {0x1096090A, {1, 0x6C, 0x80, 1, }},/*gain=6.750000(16.586075 db)*/
        {0x10AA8784, {1, 0x6D, 0x80, 1, }},/*gain=6.812500(16.666130 db)*/
        {0x10BED613, {1, 0x6E, 0x80, 1, }},/*gain=6.875000(16.745454 db)*/
        {0x10D2F597, {1, 0x6F, 0x80, 1, }},/*gain=6.937500(16.824060 db)*/
        {0x10E6E6E7, {1, 0x70, 0x80, 1, }},/*gain=7.000000(16.901961 db)*/
        {0x10FAAAD5, {1, 0x71, 0x80, 1, }},/*gain=7.062500(16.979169 db)*/
        {0x110E422E, {1, 0x72, 0x80, 1, }},/*gain=7.125000(17.055697 db)*/
        {0x1121ADBA, {1, 0x73, 0x80, 1, }},/*gain=7.187500(17.131557 db)*/
        {0x1134EE3B, {1, 0x74, 0x80, 1, }},/*gain=7.250000(17.206760 db)*/
        {0x1148046D, {1, 0x75, 0x80, 1, }},/*gain=7.312500(17.281318 db)*/
        {0x115AF10A, {1, 0x76, 0x80, 1, }},/*gain=7.375000(17.355240 db)*/
        {0x116DB4C5, {1, 0x77, 0x80, 1, }},/*gain=7.437500(17.428540 db)*/
        {0x1180504C, {1, 0x78, 0x80, 1, }},/*gain=7.500000(17.501225 db)*/
        {0x1192C44C, {1, 0x79, 0x80, 1, }},/*gain=7.562500(17.573308 db)*/
        {0x11A51169, {1, 0x7A, 0x80, 1, }},/*gain=7.625000(17.644797 db)*/
        {0x11B73848, {1, 0x7B, 0x80, 1, }},/*gain=7.687500(17.715703 db)*/
        {0x11C93987, {1, 0x7C, 0x80, 1, }},/*gain=7.750000(17.786034 db)*/
        {0x11DB15BF, {1, 0x7D, 0x80, 1, }},/*gain=7.812500(17.855801 db)*/
        {0x11ECCD89, {1, 0x7E, 0x80, 1, }},/*gain=7.875000(17.925011 db)*/
        {0x11FE6178, {1, 0x7F, 0x80, 1, }},/*gain=7.937500(17.993675 db)*/
        {0x120FD21B, {1, 0x80, 0x80, 1, }},/*gain=8.000000(18.061800 db)*/
        {0x12212000, {1, 0x81, 0x80, 1, }},/*gain=8.062500(18.129395 db)*/
        {0x12324BAF, {1, 0x82, 0x80, 1, }},/*gain=8.125000(18.196467 db)*/
        {0x124355B0, {1, 0x83, 0x80, 1, }},/*gain=8.187500(18.263026 db)*/
        {0x12543E84, {1, 0x84, 0x80, 1, }},/*gain=8.250000(18.329079 db)*/
        {0x126506AD, {1, 0x85, 0x80, 1, }},/*gain=8.312500(18.394633 db)*/
        {0x1275AEA8, {1, 0x86, 0x80, 1, }},/*gain=8.375000(18.459696 db)*/
        {0x128636EE, {1, 0x87, 0x80, 1, }},/*gain=8.437500(18.524276 db)*/
        {0x12969FF9, {1, 0x88, 0x80, 1, }},/*gain=8.500000(18.588379 db)*/
        {0x12A6EA3C, {1, 0x89, 0x80, 1, }},/*gain=8.562500(18.652012 db)*/
        {0x12B7162C, {1, 0x8A, 0x80, 1, }},/*gain=8.625000(18.715182 db)*/
        {0x12C72437, {1, 0x8B, 0x80, 1, }},/*gain=8.687500(18.777896 db)*/
        {0x12D714CB, {1, 0x8C, 0x80, 1, }},/*gain=8.750000(18.840161 db)*/
        {0x12E6E854, {1, 0x8D, 0x80, 1, }},/*gain=8.812500(18.901983 db)*/
        {0x12F69F3C, {1, 0x8E, 0x80, 1, }},/*gain=8.875000(18.963367 db)*/
        {0x130639E8, {1, 0x8F, 0x80, 1, }},/*gain=8.937500(19.024321 db)*/
        {0x1315B8BD, {1, 0x90, 0x80, 1, }},/*gain=9.000000(19.084850 db)*/
        {0x13251C1F, {1, 0x91, 0x80, 1, }},/*gain=9.062500(19.144960 db)*/
        {0x1334646E, {1, 0x92, 0x80, 1, }},/*gain=9.125000(19.204657 db)*/
        {0x13439208, {1, 0x93, 0x80, 1, }},/*gain=9.187500(19.263947 db)*/
        {0x1352A54A, {1, 0x94, 0x80, 1, }},/*gain=9.250000(19.322835 db)*/
        {0x13619E8F, {1, 0x95, 0x80, 1, }},/*gain=9.312500(19.381326 db)*/
        {0x13707E30, {1, 0x96, 0x80, 1, }},/*gain=9.375000(19.439426 db)*/
        {0x137F4485, {1, 0x97, 0x80, 1, }},/*gain=9.437500(19.497139 db)*/
        {0x138DF1E2, {1, 0x98, 0x80, 1, }},/*gain=9.500000(19.554472 db)*/
        {0x139C869B, {1, 0x99, 0x80, 1, }},/*gain=9.562500(19.611429 db)*/
        {0x13AB0303, {1, 0x9A, 0x80, 1, }},/*gain=9.625000(19.668015 db)*/
        {0x13B9676B, {1, 0x9B, 0x80, 1, }},/*gain=9.687500(19.724234 db)*/
        {0x13C7B421, {1, 0x9C, 0x80, 1, }},/*gain=9.750000(19.780092 db)*/
        {0x13D5E972, {1, 0x9D, 0x80, 1, }},/*gain=9.812500(19.835593 db)*/
        {0x13E407AC, {1, 0x9E, 0x80, 1, }},/*gain=9.875000(19.890742 db)*/
        {0x13F20F18, {1, 0x9F, 0x80, 1, }},/*gain=9.937500(19.945543 db)*/
        {0x14000000, {1, 0xA0, 0x80, 1, }},/*gain=10.000000(20.000000 db)*/
        {0x140DDAAB, {1, 0xA1, 0x80, 1, }},/*gain=10.062500(20.054118 db)*/
        {0x141B9F60, {1, 0xA2, 0x80, 1, }},/*gain=10.125000(20.107901 db)*/
        {0x14294E64, {1, 0xA3, 0x80, 1, }},/*gain=10.187500(20.161352 db)*/
        {0x1436E7FC, {1, 0xA4, 0x80, 1, }},/*gain=10.250000(20.214477 db)*/
        {0x14446C69, {1, 0xA5, 0x80, 1, }},/*gain=10.312500(20.267279 db)*/
        {0x1451DBED, {1, 0xA6, 0x80, 1, }},/*gain=10.375000(20.319762 db)*/
        {0x145F36CA, {1, 0xA7, 0x80, 1, }},/*gain=10.437500(20.371930 db)*/
        {0x146C7D3C, {1, 0xA8, 0x80, 1, }},/*gain=10.500000(20.423786 db)*/
        {0x1479AF84, {1, 0xA9, 0x80, 1, }},/*gain=10.562500(20.475334 db)*/
        {0x1486CDDD, {1, 0xAA, 0x80, 1, }},/*gain=10.625000(20.526579 db)*/
        {0x1493D884, {1, 0xAB, 0x80, 1, }},/*gain=10.687500(20.577523 db)*/
        {0x14A0CFB3, {1, 0xAC, 0x80, 1, }},/*gain=10.750000(20.628169 db)*/
        {0x14ADB3A5, {1, 0xAD, 0x80, 1, }},/*gain=10.812500(20.678522 db)*/
        {0x14BA8491, {1, 0xAE, 0x80, 1, }},/*gain=10.875000(20.728585 db)*/
        {0x14C742B0, {1, 0xAF, 0x80, 1, }},/*gain=10.937500(20.778361 db)*/
        {0x14D3EE38, {1, 0xB0, 0x80, 1, }},/*gain=11.000000(20.827854 db)*/
        {0x14E08760, {1, 0xB1, 0x80, 1, }},/*gain=11.062500(20.877066 db)*/
        {0x14ED0E5C, {1, 0xB2, 0x80, 1, }},/*gain=11.125000(20.926000 db)*/
        {0x14F98361, {1, 0xB3, 0x80, 1, }},/*gain=11.187500(20.974661 db)*/
        {0x1505E6A2, {1, 0xB4, 0x80, 1, }},/*gain=11.250000(21.023050 db)*/
        {0x15123851, {1, 0xB5, 0x80, 1, }},/*gain=11.312500(21.071172 db)*/
        {0x151E78A0, {1, 0xB6, 0x80, 1, }},/*gain=11.375000(21.119028 db)*/
        {0x152AA7BF, {1, 0xB7, 0x80, 1, }},/*gain=11.437500(21.166622 db)*/
        {0x1536C5DF, {1, 0xB8, 0x80, 1, }},/*gain=11.500000(21.213957 db)*/
        {0x1542D32F, {1, 0xB9, 0x80, 1, }},/*gain=11.562500(21.261035 db)*/
        {0x154ECFDC, {1, 0xBA, 0x80, 1, }},/*gain=11.625000(21.307859 db)*/
        {0x155ABC16, {1, 0xBB, 0x80, 1, }},/*gain=11.687500(21.354432 db)*/
        {0x15669808, {1, 0xBC, 0x80, 1, }},/*gain=11.750000(21.400757 db)*/
        {0x157263DF, {1, 0xBD, 0x80, 1, }},/*gain=11.812500(21.446836 db)*/
        {0x157E1FC6, {1, 0xBE, 0x80, 1, }},/*gain=11.875000(21.492672 db)*/
        {0x1589CBE9, {1, 0xBF, 0x80, 1, }},/*gain=11.937500(21.538268 db)*/
        {0x15956871, {1, 0xC0, 0x80, 1, }},/*gain=12.000000(21.583625 db)*/
        {0x15A0F588, {1, 0xC1, 0x80, 1, }},/*gain=12.062500(21.628747 db)*/
        {0x15AC7356, {1, 0xC2, 0x80, 1, }},/*gain=12.125000(21.673635 db)*/
        {0x15B7E205, {1, 0xC3, 0x80, 1, }},/*gain=12.187500(21.718293 db)*/
        {0x15C341BB, {1, 0xC4, 0x80, 1, }},/*gain=12.250000(21.762722 db)*/
        {0x15CE92A0, {1, 0xC5, 0x80, 1, }},/*gain=12.312500(21.806925 db)*/
        {0x15D9D4DA, {1, 0xC6, 0x80, 1, }},/*gain=12.375000(21.850904 db)*/
        {0x15E5088F, {1, 0xC7, 0x80, 1, }},/*gain=12.437500(21.894662 db)*/
        {0x15F02DE4, {1, 0xC8, 0x80, 1, }},/*gain=12.500000(21.938200 db)*/
        {0x15FB44FE, {1, 0xC9, 0x80, 1, }},/*gain=12.562500(21.981521 db)*/
        {0x16064E00, {1, 0xCA, 0x80, 1, }},/*gain=12.625000(22.024628 db)*/
        {0x16114910, {1, 0xCB, 0x80, 1, }},/*gain=12.687500(22.067521 db)*/
        {0x161C364F, {1, 0xCC, 0x80, 1, }},/*gain=12.750000(22.110204 db)*/
        {0x162715E0, {1, 0xCD, 0x80, 1, }},/*gain=12.812500(22.152678 db)*/
        {0x1631E7E6, {1, 0xCE, 0x80, 1, }},/*gain=12.875000(22.194945 db)*/
        {0x163CAC81, {1, 0xCF, 0x80, 1, }},/*gain=12.937500(22.237007 db)*/
        {0x164763D4, {1, 0xD0, 0x80, 1, }},/*gain=13.000000(22.278867 db)*/
        {0x16520DFF, {1, 0xD1, 0x80, 1, }},/*gain=13.062500(22.320526 db)*/
        {0x165CAB21, {1, 0xD2, 0x80, 1, }},/*gain=13.125000(22.361986 db)*/
        {0x16673B5B, {1, 0xD3, 0x80, 1, }},/*gain=13.187500(22.403249 db)*/
        {0x1671BECB, {1, 0xD4, 0x80, 1, }},/*gain=13.250000(22.444318 db)*/
        {0x167C3591, {1, 0xD5, 0x80, 1, }},/*gain=13.312500(22.485192 db)*/
        {0x16869FCC, {1, 0xD6, 0x80, 1, }},/*gain=13.375000(22.525876 db)*/
        {0x1690FD98, {1, 0xD7, 0x80, 1, }},/*gain=13.437500(22.566370 db)*/
        {0x169B4F13, {1, 0xD8, 0x80, 1, }},/*gain=13.500000(22.606675 db)*/
        {0x16A5945B, {1, 0xD9, 0x80, 1, }},/*gain=13.562500(22.646795 db)*/
        {0x16AFCD8D, {1, 0xDA, 0x80, 1, }},/*gain=13.625000(22.686730 db)*/
        {0x16B9FAC4, {1, 0xDB, 0x80, 1, }},/*gain=13.687500(22.726483 db)*/
        {0x16C41C1C, {1, 0xDC, 0x80, 1, }},/*gain=13.750000(22.766054 db)*/
        {0x16CE31B2, {1, 0xDD, 0x80, 1, }},/*gain=13.812500(22.805446 db)*/
        {0x16D83BA0, {1, 0xDE, 0x80, 1, }},/*gain=13.875000(22.844660 db)*/
        {0x16E23A01, {1, 0xDF, 0x80, 1, }},/*gain=13.937500(22.883698 db)*/
        {0x16EC2CF0, {1, 0xE0, 0x80, 1, }},/*gain=14.000000(22.922561 db)*/
        {0x16F61486, {1, 0xE1, 0x80, 1, }},/*gain=14.062500(22.961251 db)*/
        {0x16FFF0DE, {1, 0xE2, 0x80, 1, }},/*gain=14.125000(22.999769 db)*/
        {0x1709C211, {1, 0xE3, 0x80, 1, }},/*gain=14.187500(23.038117 db)*/
        {0x17138838, {1, 0xE4, 0x80, 1, }},/*gain=14.250000(23.076297 db)*/
        {0x171D436B, {1, 0xE5, 0x80, 1, }},/*gain=14.312500(23.114310 db)*/
        {0x1726F3C3, {1, 0xE6, 0x80, 1, }},/*gain=14.375000(23.152157 db)*/
        {0x17309959, {1, 0xE7, 0x80, 1, }},/*gain=14.437500(23.189840 db)*/
        {0x173A3444, {1, 0xE8, 0x80, 1, }},/*gain=14.500000(23.227360 db)*/
        {0x1743C49B, {1, 0xE9, 0x80, 1, }},/*gain=14.562500(23.264719 db)*/
        {0x174D4A77, {1, 0xEA, 0x80, 1, }},/*gain=14.625000(23.301917 db)*/
        {0x1756C5EC, {1, 0xEB, 0x80, 1, }},/*gain=14.687500(23.338958 db)*/
        {0x17603713, {1, 0xEC, 0x80, 1, }},/*gain=14.750000(23.375840 db)*/
        {0x17699E02, {1, 0xED, 0x80, 1, }},/*gain=14.812500(23.412567 db)*/
        {0x1772FACE, {1, 0xEE, 0x80, 1, }},/*gain=14.875000(23.449139 db)*/
        {0x177C4D8D, {1, 0xEF, 0x80, 1, }},/*gain=14.937500(23.485558 db)*/
        {0x17859655, {1, 0xF0, 0x80, 1, }},/*gain=15.000000(23.521825 db)*/
        {0x178ED53C, {1, 0xF1, 0x80, 1, }},/*gain=15.062500(23.557941 db)*/
        {0x17980A55, {1, 0xF2, 0x80, 1, }},/*gain=15.125000(23.593908 db)*/
        {0x17A135B6, {1, 0xF3, 0x80, 1, }},/*gain=15.187500(23.629726 db)*/
        {0x17AA5773, {1, 0xF4, 0x80, 1, }},/*gain=15.250000(23.665397 db)*/
        {0x17B36FA0, {1, 0xF5, 0x80, 1, }},/*gain=15.312500(23.700922 db)*/
        {0x17BC7E51, {1, 0xF6, 0x80, 1, }},/*gain=15.375000(23.736302 db)*/
        {0x17C5839B, {1, 0xF7, 0x80, 1, }},/*gain=15.437500(23.771539 db)*/
        {0x17CE7F90, {1, 0xF8, 0x80, 1, }},/*gain=15.500000(23.806634 db)*/
        {0x17D77243, {1, 0xF9, 0x80, 1, }},/*gain=15.562500(23.841587 db)*/
        {0x17E05BC8, {1, 0xFA, 0x80, 1, }},/*gain=15.625000(23.876401 db)*/
        {0x17E93C32, {1, 0xFB, 0x80, 1, }},/*gain=15.687500(23.911075 db)*/
        {0x17F21392, {1, 0xFC, 0x80, 1, }},/*gain=15.750000(23.945611 db)*/
        {0x17FAE1FC, {1, 0xFD, 0x80, 1, }},/*gain=15.812500(23.980011 db)*/
        {0x1803A781, {1, 0xFE, 0x80, 1, }},/*gain=15.875000(24.014275 db)*/
        {0x180C6433, {1, 0xFF, 0x80, 1, }},/*gain=15.937500(24.048404 db)*/
        {0x180C6433, {1, 0xFF, 0x80, 1, }},/*gain=15.937500(24.048404 db)*/
        {0x181DB218, {1, 0xFF, 0x81, 1, }},/*gain=16.062012(24.115999 db)*/
        {0x182EDDC7, {1, 0xFF, 0x82, 1, }},/*gain=16.186523(24.183072 db)*/
        {0x183FE7C8, {1, 0xFF, 0x83, 1, }},/*gain=16.311035(24.249630 db)*/
        {0x1850D09D, {1, 0xFF, 0x84, 1, }},/*gain=16.435547(24.315683 db)*/
        {0x186198C5, {1, 0xFF, 0x85, 1, }},/*gain=16.560059(24.381237 db)*/
        {0x187240C0, {1, 0xFF, 0x86, 1, }},/*gain=16.684570(24.446301 db)*/
        {0x1882C906, {1, 0xFF, 0x87, 1, }},/*gain=16.809082(24.510880 db)*/
        {0x18933211, {1, 0xFF, 0x88, 1, }},/*gain=16.933594(24.574983 db)*/
        {0x18A37C54, {1, 0xFF, 0x89, 1, }},/*gain=17.058105(24.638616 db)*/
        {0x18B3A844, {1, 0xFF, 0x8A, 1, }},/*gain=17.182617(24.701786 db)*/
        {0x18C3B64F, {1, 0xFF, 0x8B, 1, }},/*gain=17.307129(24.764501 db)*/
        {0x18D3A6E3, {1, 0xFF, 0x8C, 1, }},/*gain=17.431641(24.826765 db)*/
        {0x18E37A6C, {1, 0xFF, 0x8D, 1, }},/*gain=17.556152(24.888587 db)*/
        {0x18F33154, {1, 0xFF, 0x8E, 1, }},/*gain=17.680664(24.949971 db)*/
        {0x1902CC00, {1, 0xFF, 0x8F, 1, }},/*gain=17.805176(25.010925 db)*/
        {0x19124AD5, {1, 0xFF, 0x90, 1, }},/*gain=17.929688(25.071454 db)*/
        {0x1921AE37, {1, 0xFF, 0x91, 1, }},/*gain=18.054199(25.131565 db)*/
        {0x1930F686, {1, 0xFF, 0x92, 1, }},/*gain=18.178711(25.191262 db)*/
        {0x19402420, {1, 0xFF, 0x93, 1, }},/*gain=18.303223(25.250551 db)*/
        {0x194F3762, {1, 0xFF, 0x94, 1, }},/*gain=18.427734(25.309439 db)*/
        {0x195E30A7, {1, 0xFF, 0x95, 1, }},/*gain=18.552246(25.367930 db)*/
        {0x196D1049, {1, 0xFF, 0x96, 1, }},/*gain=18.676758(25.426030 db)*/
        {0x197BD69D, {1, 0xFF, 0x97, 1, }},/*gain=18.801270(25.483744 db)*/
        {0x198A83FA, {1, 0xFF, 0x98, 1, }},/*gain=18.925781(25.541076 db)*/
        {0x199918B3, {1, 0xFF, 0x99, 1, }},/*gain=19.050293(25.598033 db)*/
        {0x19A7951C, {1, 0xFF, 0x9A, 1, }},/*gain=19.174805(25.654619 db)*/
        {0x19B5F983, {1, 0xFF, 0x9B, 1, }},/*gain=19.299316(25.710839 db)*/
        {0x19C44639, {1, 0xFF, 0x9C, 1, }},/*gain=19.423828(25.766697 db)*/
        {0x19D27B8A, {1, 0xFF, 0x9D, 1, }},/*gain=19.548340(25.822198 db)*/
        {0x19E099C4, {1, 0xFF, 0x9E, 1, }},/*gain=19.672852(25.877346 db)*/
        {0x19EEA130, {1, 0xFF, 0x9F, 1, }},/*gain=19.797363(25.932147 db)*/
        {0x19FC9218, {1, 0xFF, 0xA0, 1, }},/*gain=19.921875(25.986604 db)*/
        {0x1A0A6CC3, {1, 0xFF, 0xA1, 1, }},/*gain=20.046387(26.040722 db)*/
        {0x1A183178, {1, 0xFF, 0xA2, 1, }},/*gain=20.170898(26.094505 db)*/
        {0x1A25E07C, {1, 0xFF, 0xA3, 1, }},/*gain=20.295410(26.147957 db)*/
        {0x1A337A14, {1, 0xFF, 0xA4, 1, }},/*gain=20.419922(26.201082 db)*/
        {0x1A40FE81, {1, 0xFF, 0xA5, 1, }},/*gain=20.544434(26.253883 db)*/
        {0x1A4E6E05, {1, 0xFF, 0xA6, 1, }},/*gain=20.668945(26.306366 db)*/
        {0x1A5BC8E2, {1, 0xFF, 0xA7, 1, }},/*gain=20.793457(26.358534 db)*/
        {0x1A690F54, {1, 0xFF, 0xA8, 1, }},/*gain=20.917969(26.410390 db)*/
        {0x1A76419C, {1, 0xFF, 0xA9, 1, }},/*gain=21.042480(26.461939 db)*/
        {0x1A835FF5, {1, 0xFF, 0xAA, 1, }},/*gain=21.166992(26.513183 db)*/
        {0x1A906A9C, {1, 0xFF, 0xAB, 1, }},/*gain=21.291504(26.564127 db)*/
        {0x1A9D61CB, {1, 0xFF, 0xAC, 1, }},/*gain=21.416016(26.614774 db)*/
        {0x1AAA45BD, {1, 0xFF, 0xAD, 1, }},/*gain=21.540527(26.665127 db)*/
        {0x1AB716A9, {1, 0xFF, 0xAE, 1, }},/*gain=21.665039(26.715190 db)*/
        {0x1AC3D4C8, {1, 0xFF, 0xAF, 1, }},/*gain=21.789551(26.764966 db)*/
        {0x1AD08050, {1, 0xFF, 0xB0, 1, }},/*gain=21.914063(26.814458 db)*/
        {0x1ADD1978, {1, 0xFF, 0xB1, 1, }},/*gain=22.038574(26.863670 db)*/
        {0x1AE9A074, {1, 0xFF, 0xB2, 1, }},/*gain=22.163086(26.912605 db)*/
        {0x1AF61579, {1, 0xFF, 0xB3, 1, }},/*gain=22.287598(26.961265 db)*/
        {0x1B0278BA, {1, 0xFF, 0xB4, 1, }},/*gain=22.412109(27.009655 db)*/
        {0x1B0ECA69, {1, 0xFF, 0xB5, 1, }},/*gain=22.536621(27.057776 db)*/
        {0x1B1B0AB8, {1, 0xFF, 0xB6, 1, }},/*gain=22.661133(27.105632 db)*/
        {0x1B2739D7, {1, 0xFF, 0xB7, 1, }},/*gain=22.785645(27.153226 db)*/
        {0x1B3357F7, {1, 0xFF, 0xB8, 1, }},/*gain=22.910156(27.200561 db)*/
        {0x1B3F6547, {1, 0xFF, 0xB9, 1, }},/*gain=23.034668(27.247639 db)*/
        {0x1B4B61F4, {1, 0xFF, 0xBA, 1, }},/*gain=23.159180(27.294463 db)*/
        {0x1B574E2E, {1, 0xFF, 0xBB, 1, }},/*gain=23.283691(27.341037 db)*/
        {0x1B632A20, {1, 0xFF, 0xBC, 1, }},/*gain=23.408203(27.387362 db)*/
        {0x1B6EF5F7, {1, 0xFF, 0xBD, 1, }},/*gain=23.532715(27.433441 db)*/
        {0x1B7AB1DE, {1, 0xFF, 0xBE, 1, }},/*gain=23.657227(27.479277 db)*/
        {0x1B865E01, {1, 0xFF, 0xBF, 1, }},/*gain=23.781738(27.524872 db)*/
        {0x1B91FA89, {1, 0xFF, 0xC0, 1, }},/*gain=23.906250(27.570229 db)*/
        {0x1B9D87A0, {1, 0xFF, 0xC1, 1, }},/*gain=24.030762(27.615351 db)*/
        {0x1BA9056F, {1, 0xFF, 0xC2, 1, }},/*gain=24.155273(27.660239 db)*/
        {0x1BB4741D, {1, 0xFF, 0xC3, 1, }},/*gain=24.279785(27.704897 db)*/
        {0x1BBFD3D3, {1, 0xFF, 0xC4, 1, }},/*gain=24.404297(27.749326 db)*/
        {0x1BCB24B8, {1, 0xFF, 0xC5, 1, }},/*gain=24.528809(27.793529 db)*/
        {0x1BD666F2, {1, 0xFF, 0xC6, 1, }},/*gain=24.653320(27.837508 db)*/
        {0x1BE19AA7, {1, 0xFF, 0xC7, 1, }},/*gain=24.777832(27.881266 db)*/
        {0x1BECBFFC, {1, 0xFF, 0xC8, 1, }},/*gain=24.902344(27.924804 db)*/
        {0x1BF7D716, {1, 0xFF, 0xC9, 1, }},/*gain=25.026855(27.968126 db)*/
        {0x1C02E018, {1, 0xFF, 0xCA, 1, }},/*gain=25.151367(28.011232 db)*/
        {0x1C0DDB28, {1, 0xFF, 0xCB, 1, }},/*gain=25.275879(28.054125 db)*/
        {0x1C18C867, {1, 0xFF, 0xCC, 1, }},/*gain=25.400391(28.096808 db)*/
        {0x1C23A7F8, {1, 0xFF, 0xCD, 1, }},/*gain=25.524902(28.139282 db)*/
        {0x1C2E79FE, {1, 0xFF, 0xCE, 1, }},/*gain=25.649414(28.181549 db)*/
        {0x1C393E99, {1, 0xFF, 0xCF, 1, }},/*gain=25.773926(28.223611 db)*/
        {0x1C43F5EC, {1, 0xFF, 0xD0, 1, }},/*gain=25.898438(28.265471 db)*/
        {0x1C4EA017, {1, 0xFF, 0xD1, 1, }},/*gain=26.022949(28.307130 db)*/
        {0x1C593D39, {1, 0xFF, 0xD2, 1, }},/*gain=26.147461(28.348590 db)*/
        {0x1C63CD73, {1, 0xFF, 0xD3, 1, }},/*gain=26.271973(28.389854 db)*/
        {0x1C6E50E3, {1, 0xFF, 0xD4, 1, }},/*gain=26.396484(28.430922 db)*/
        {0x1C78C7A9, {1, 0xFF, 0xD5, 1, }},/*gain=26.520996(28.471797 db)*/
        {0x1C8331E4, {1, 0xFF, 0xD6, 1, }},/*gain=26.645508(28.512480 db)*/
        {0x1C8D8FB0, {1, 0xFF, 0xD7, 1, }},/*gain=26.770020(28.552974 db)*/
        {0x1C97E12B, {1, 0xFF, 0xD8, 1, }},/*gain=26.894531(28.593280 db)*/
        {0x1CA22673, {1, 0xFF, 0xD9, 1, }},/*gain=27.019043(28.633399 db)*/
        {0x1CAC5FA5, {1, 0xFF, 0xDA, 1, }},/*gain=27.143555(28.673334 db)*/
        {0x1CB68CDC, {1, 0xFF, 0xDB, 1, }},/*gain=27.268066(28.713087 db)*/
        {0x1CC0AE34, {1, 0xFF, 0xDC, 1, }},/*gain=27.392578(28.752658 db)*/
        {0x1CCAC3CA, {1, 0xFF, 0xDD, 1, }},/*gain=27.517090(28.792050 db)*/
        {0x1CD4CDB8, {1, 0xFF, 0xDE, 1, }},/*gain=27.641602(28.831264 db)*/
        {0x1CDECC19, {1, 0xFF, 0xDF, 1, }},/*gain=27.766113(28.870302 db)*/
        {0x1CE8BF08, {1, 0xFF, 0xE0, 1, }},/*gain=27.890625(28.909165 db)*/
        {0x1CF2A69E, {1, 0xFF, 0xE1, 1, }},/*gain=28.015137(28.947855 db)*/
        {0x1CFC82F6, {1, 0xFF, 0xE2, 1, }},/*gain=28.139648(28.986373 db)*/
        {0x1D065429, {1, 0xFF, 0xE3, 1, }},/*gain=28.264160(29.024722 db)*/
        {0x1D101A50, {1, 0xFF, 0xE4, 1, }},/*gain=28.388672(29.062902 db)*/
        {0x1D19D583, {1, 0xFF, 0xE5, 1, }},/*gain=28.513184(29.100914 db)*/
        {0x1D2385DC, {1, 0xFF, 0xE6, 1, }},/*gain=28.637695(29.138761 db)*/
        {0x1D2D2B71, {1, 0xFF, 0xE7, 1, }},/*gain=28.762207(29.176444 db)*/
        {0x1D36C65C, {1, 0xFF, 0xE8, 1, }},/*gain=28.886719(29.213964 db)*/
        {0x1D4056B3, {1, 0xFF, 0xE9, 1, }},/*gain=29.011230(29.251323 db)*/
        {0x1D49DC8F, {1, 0xFF, 0xEA, 1, }},/*gain=29.135742(29.288522 db)*/
        {0x1D535804, {1, 0xFF, 0xEB, 1, }},/*gain=29.260254(29.325562 db)*/
        {0x1D5CC92B, {1, 0xFF, 0xEC, 1, }},/*gain=29.384766(29.362445 db)*/
        {0x1D66301A, {1, 0xFF, 0xED, 1, }},/*gain=29.509277(29.399171 db)*/
        {0x1D6F8CE6, {1, 0xFF, 0xEE, 1, }},/*gain=29.633789(29.435744 db)*/
        {0x1D78DFA5, {1, 0xFF, 0xEF, 1, }},/*gain=29.758301(29.472163 db)*/
        {0x1D82286D, {1, 0xFF, 0xF0, 1, }},/*gain=29.882813(29.508429 db)*/
        {0x1D8B6754, {1, 0xFF, 0xF1, 1, }},/*gain=30.007324(29.544545 db)*/
        {0x1D949C6D, {1, 0xFF, 0xF2, 1, }},/*gain=30.131836(29.580512 db)*/
        {0x1D9DC7CE, {1, 0xFF, 0xF3, 1, }},/*gain=30.256348(29.616330 db)*/
        {0x1DA6E98B, {1, 0xFF, 0xF4, 1, }},/*gain=30.380859(29.652001 db)*/
        {0x1DB001B8, {1, 0xFF, 0xF5, 1, }},/*gain=30.505371(29.687526 db)*/
        {0x1DB91069, {1, 0xFF, 0xF6, 1, }},/*gain=30.629883(29.722907 db)*/
        {0x1DC215B3, {1, 0xFF, 0xF7, 1, }},/*gain=30.754395(29.758144 db)*/
        {0x1DCB11A8, {1, 0xFF, 0xF8, 1, }},/*gain=30.878906(29.793238 db)*/
        {0x1DD4045B, {1, 0xFF, 0xF9, 1, }},/*gain=31.003418(29.828192 db)*/
        {0x1DDCEDE0, {1, 0xFF, 0xFA, 1, }},/*gain=31.127930(29.863005 db)*/
        {0x1DE5CE4A, {1, 0xFF, 0xFB, 1, }},/*gain=31.252441(29.897679 db)*/
        {0x1DEEA5AA, {1, 0xFF, 0xFC, 1, }},/*gain=31.376953(29.932215 db)*/
        {0x1DF77414, {1, 0xFF, 0xFD, 1, }},/*gain=31.501465(29.966615 db)*/
        {0x1E003999, {1, 0xFF, 0xFE, 1, }},/*gain=31.625977(30.000879 db)*/
        {0x1E08F64B, {1, 0xFF, 0xFF, 1, }},/*gain=31.750488(30.035008 db)*/

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

