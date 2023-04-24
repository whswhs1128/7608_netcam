/*!
*****************************************************************************
** \file        gc2053_reg_tbl.c
**
** \version     $Id: gc2053_reg_tbl.c 10611 2018-08-24 17:12:00Z baochao $
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
GADI_VI_SensorDrvInfoT    gc2053_sensor =
{
    .magic_start        = GADI_SENSOR_HW_INFO_MAGIC_START,
    .version            = GADI_SENSOR_HW_INFO_VERSION,
    .length             = sizeof(GADI_VI_SensorDrvInfoT),
    .file_date          = 0x20180824,
    .HwInfo             =
    {
        .name               = "gc2053",
        .ctl_type           = GADI_SENSOR_CTL_I2C,
        .addr_len           = sizeof(GADI_U8),
        .data_len           = sizeof(GADI_U8),
        .reset_active_level = GADI_GPIO_LOW,
        .hw_addr            = {(0x6e>>1), (0x7e>>1), 0, 0},
        .id_reg =
        {
            {0xF0, 0x20},
            {0xF1, 0x53},
        },
        .reset_reg =
        {
            {0xFE, 0x80, 0x80},
            {GADI_VI_SENSOR_TABLE_FLAG_MSLEEP, 0x14},
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
    .sensor_id          = GADI_ISP_SENSOR_GC2053,
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
    .def_sh_time        = GADI_VIDEO_FPS(50),
    .fmt_num            = 5,
    .auto_fmt           = GADI_VIDEO_MODE(1920, 1080,  30, 1),
    .init_reg           =
    {
        /*   SYS   */ 
        {0xf2, 0x00},//[1]I2C_open_ena [0]pwd_dn
        {0xf3, 0x0f},//00[3]Sdata_pad_io [2:0]Ssync_pad_io
        {0xf4, 0x36},//[6:4]pll_ldo_set
        {0xf5, 0xc0},//[7]soc_mclk_enable [6]pll_ldo_en [5:4]cp_clk_sel [3:0]cp_clk_div
        {0xf6, 0x44},//[7:3]wpllclk_div [2:0]refmp_div
        {0xf7, 0x01},//[7]refdiv2d5_en [6]refdiv1d5_en [5:4]scaler_mode [3]refmp_enb [1]div2en [0]pllmp_en 
        {0xf8, 0x2c},//38////38//[7:0]pllmp_div
        {0xf9, 0x42},//82//[7:3]rpllclk_div [2:1]pllmp_prediv [0]analog_pwc
        {0xfc, 0x8e},
        /*cisctl&analog*/
        {0xfe, 0x00},//pape 0
        {0x87, 0x18},//[6]aec_delay_mode
        {0xee, 0x30},//[5:4]dwen_sramen
        {0xd0, 0xb7},//ramp_en
        {0x03, 0x04},//exp_h
        {0x04, 0x10},//exp_l
        {0x05, 0x04},//05//[11:8]hb
        {0x06, 0x4c},//28//[7:0]hb
        {0x07, 0x00},//[13:8]vb
        {0x08, 0x0d},//[7:0]vb
        {0x09, 0x00},
        {0x0a, 0x02},//cisctl row start
        {0x0b, 0x00},
        {0x0c, 0x02},//cisctl col start
        {0x12, 0xe2},//vsync_ahead_mode
        {0x13, 0x16},
        {0x19, 0x0a},//ad_pipe_num
        {0x21, 0x1c},//eqc1fc_eqc2fc_sw
        {0x28, 0x0a},//16//eqc2_c2clpen_sw
        {0x29, 0x24},//eq_post_width
        {0x2b, 0x04},//c2clpen --eqc2
        {0x32, 0xf8},//0x32[7]=1,0xd3[7]=1 rsth=vref
        {0x37, 0x03},//[3:2]eqc2sel=0
        {0x39, 0x15},//[3:0]rsgl
        {0x43, 0x07},//vclamp
        {0x44, 0x40},//0e//post_tx_width
        {0x46, 0x0b},//txh_r -->3.2v
        {0x4b, 0x20},//rst_tx_width
        {0x4e, 0x08},//12//ramp_t1_width
        {0x55, 0x20},//read_tx_width_pp
        {0x66, 0x05},//18//stspd_width_r1
        {0x67, 0x05},//40//5//stspd_width_r
        {0x77, 0x01},//dacin offset x31
        {0x78, 0x00},//dacin offset
        {0x7c, 0x93},//[1:0] co1comp
        {0x8c, 0x12},//12 ramp_t1_ref
        {0x8d, 0x92},
        {0x90, 0x00},
        {0x41, 0x04},//VTS[13:8]
        {0x42, 0x65},//VTS[7:0]
        {0x9d, 0x10},
        {0xce, 0x7c},//70//78//[4:2]c1isel
        {0xd2, 0x41},//[5:3]c2clamp
        {0xd3, 0xdc},//0x39[7]=0,0xd3[3]=1 rsgh=vref
        {0xe6, 0x50},//ramps offset
        /*gain*/
        {0xb6, 0xc0},
        {0xb0, 0x70},
        /*blk*/ 
        {0x26, 0x30},//[4]Ð´0£¬È«n mode
        {0xfe, 0x01},//pape 1
        {0x40, 0x23},
        {0x55, 0x07},
        {0x60, 0x40},//[7:0]WB_offset
		{0xfe,0x04},
		{0x14,0x78}, //g1 ratio
		{0x15,0x78}, //r ratio
		{0x16,0x78}, //b ratio
		{0x17,0x78}, //g2 ratio
        /*window*/
        {0xfe, 0x01},//pape 1
        {0x92, 0x00}, //win y1
        {0x94, 0x03},
        {0x95, 0x04},
        {0x96, 0x38},//[10:0]out_height
        {0x97, 0x07},
        {0x98, 0x80},//[11:0]out_width
        /*ISP*/
		{0xfe,0x01},
		{0x01,0x05},//03//[3]dpc blending mode [2]noise_mode [1:0]center_choose 2b'11:median 2b'10:avg 2'b00:near
		{0x02,0x89}, //[7:0]BFF_sram_mode
		{0x04,0x01}, //[0]DD_en
		{0x07,0xa6},
		{0x08,0xa9},
		{0x09,0xa8},
		{0x0a,0xa7},
		{0x0b,0xff},
		{0x0c,0xff},
		{0x0f,0x00},
		{0x50,0x1c},
		{0x89,0x03},
		{0xfe,0x04},
		{0x28,0x86},
		{0x29,0x86},
		{0x2a,0x86},
		{0x2b,0x68},
		{0x2c,0x68},
		{0x2d,0x68},
		{0x2e,0x68},
		{0x2f,0x68},
		{0x30,0x4f},
		{0x31,0x68},
		{0x32,0x67},
		{0x33,0x66},
		{0x34,0x66},
		{0x35,0x66},
		{0x36,0x66},
		{0x37,0x66},
		{0x38,0x62},
		{0x39,0x62},
		{0x3a,0x62},
		{0x3b,0x62},
		{0x3c,0x62},
		{0x3d,0x62},
		{0x3e,0x62},
		{0x3f,0x62},
        {0xfe, 0x01},//pape 1
        {0x9a, 0x06},//[5]OUT_gate_mode [4]hsync_delay_half_pclk [3]data_delay_half_pclk [2]vsync_polarity [1]hsync_polarity [0]pclk_out_polarity
        {0xfe, 0x00},//pape 0
        {0x7b, 0x3F},//{0x7b, 0x2a},//[7:6]updn [5:4]drv_high_data [3:2]drv_low_data [1:0]drv_pclk
        {0x23, 0x2F},//{0x23, 0x2d},//[3]rst_rc [2:1]drv_sync [0]pwd_rc
        {0xfe, 0x03},//pape 3
        {0x01, 0x20},//27[6:5]clkctr [2]phy-lane1_en [1]phy-lane0_en [0]phy_clk_en
        {0x02, 0x56},//[7:6]data1ctr [5:4]data0ctr [3:0]mipi_diff
        {0x03, 0xb2},//b6[7]clklane_p2s_sel [6:5]data0hs_ph [4]data0_delay1s [3]clkdelay1s [2]mipi_en [1:0]clkhs_ph
        {0x12, 0x80},
        {0x13, 0x07},//LWC
        {0xfe, 0x00},//pape 0
        {0x3e, 0x40},//91[7]lane_ena [6]DVPBUF_ena [5]ULPEna [4]MIPI_ena [3]mipi_set_auto_disable [2]RAW8_mode [1]ine_sync_mode [0]double_lane_en
        {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 50},// mdelay(2);
        {GADI_VI_SENSOR_TABLE_FLAG_END, 0x00}
},
    .pll_table =
    {
        [0] =
        {
            .pixclk = 74250000,
            .extclk = 27000000,
            .regs =
            {
                {0xf6, 0x44},//[7:3]wpllclk_div [2:0]refmp_div
                {0xf7, 0x01},//[7]refdiv2d5_en [6]refdiv1d5_en [5:4]scaler_mode [3]refmp_enb [1]div2en [0]pllmp_en 
                {0xf8, 0x2c},//38////38//[7:0]pllmp_div
                {0xf9, 0x42},//82//[7:3]rpllclk_div [2:1]pllmp_prediv [0]analog_pwc
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x0A},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
        [1] =
        {
            .pixclk = 84375000,
            .extclk = 27000000,
            .regs =
            {
                {0xf6, 0x44},//[7:3]wpllclk_div [2:0]refmp_div
                {0xf7, 0x01},//[7]refdiv2d5_en [6]refdiv1d5_en [5:4]scaler_mode [3]refmp_enb [1]div2en [0]pllmp_en 
                {0xf8, 0x32},//38////38//[7:0]pllmp_div
                {0xf9, 0x42},//82//[7:3]rpllclk_div [2:1]pllmp_prediv [0]analog_pwc
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x0A},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },

        [2] =
        {
            .pixclk = 66000000,
            .extclk = 24000000,
            .regs =
            {
                {0xf6, 0x44},//[7:3]wpllclk_div [2:0]refmp_div
                {0xf7, 0x01},//[7]refdiv2d5_en [6]refdiv1d5_en [5:4]scaler_mode [3]refmp_enb [1]div2en [0]pllmp_en 
                {0xf8, 0x2c},//38////38//[7:0]pllmp_div
                {0xf9, 0x42},//82//[7:3]rpllclk_div [2:1]pllmp_prediv [0]analog_pwc
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x0A},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },


    },
    .video_fmt_table =
    {
        [0] = // 1920x1080P@30fps   (1100 * 2) * 1125 * 30 = 74250000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1920, 1080,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0xfe, 0x01},//pape 1
                {0x94, 0x01},
                {0x95, 0x04},
                {0x96, 0x38+0},//[10:0]out_height  0x438 = 1080
                {0x97, 0x07},
                {0x98, 0x80},//[11:0]out_width  0x780 = 1920        
                {0xfe, 0x00},//pape 0
                {0x05, 0x04},//[11:8]hb   hts_h
                {0x06, 0x4c},//[7:0]hb    hts_l
                {0x07, 0x00},//[13:8]vb
                {0x08, 0x0d},//[7:0]vb
                {0x09, 0x00},//[10:8]cisctl row start
                {0x0a, 0x02},//[7:0]cisctl row start
                {0x0b, 0x00},//[10:8]cisctl col start
                {0x0c, 0x02},//[7:1]cisctl col start
                {0x0d, 0x04},//[10:8]window height 1080+2
                {0x0e, 0x3A},//[7:0]window height   
                {0x0f, 0x07},//[10:8]window width 1920+16
                {0x10, 0x90},//[7:0]window width   
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 50},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 1920,
                .height     = 1080,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_DVP, 
                .bits       = GADI_VIDEO_BITS_10,//10bit sensor, but the D[9:0] of gc2053 module is connected with the D[11:2] of gk720x 
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
        [1] = // 1920x1080P@25fps   (1320 * 2) * 1125 * 25 = 74250000
        {                         //(1100 * 2) * 1350 * 25 = 74250000
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1920, 1080,  25, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0xfe, 0x01},//pape 1
                {0x94, 0x01},
                {0x95, 0x04},
                {0x96, 0x38},//[10:0]out_height  0x438 = 1080
                {0x97, 0x07},
                {0x98, 0x80},//[11:0]out_width  0x780 = 1920  
                {0xfe, 0x00},//pape 0
                {0x05, 0x04},//[11:8]hb   hts_h  0x05
                {0x06, 0x4C},//[7:0]hb    hts_l  0x28
                {0x07, 0x00},//[13:8]vb
                {0x08, 0x19},//[7:0]vb
                {0x09, 0x00},//[10:8]cisctl row start
                {0x0a, 0x02},//[7:0]cisctl row start
                {0x0b, 0x00},//[10:8]cisctl col start
                {0x0c, 0x02},//[7:1]cisctl col start    
                {0x0d, 0x04},//[10:8]window height 1080+2
                {0x0e, 0x3A},//[7:0]window height   
                {0x0f, 0x07},//[10:8]window width 1920+16
                {0x10, 0x90},//[7:0]window width                 
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 50},// mdelay(2)
                {GADI_VI_SENSOR_TABLE_FLAG_END,},

            },
            .vi_info =
            {
                .width      = 1920,
                .height     = 1080,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_DVP, 
                .bits       = GADI_VIDEO_BITS_10,//10bit sensor, but the D[9:0] of gc2053 module is connected with the D[11:2] of gk720x 
                .ratio      = GADI_VIDEO_RATIO_16_9,
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
        
        [2] = // 1920x1080P@20fps   (1320 * 2) * 1250 * 20 = 66000000
        {     //                    (1100 * 2) * 1500 * 20 = 66000000
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1920, 1080,  20, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0xfe, 0x01},//pape 1
                {0x94, 0x01},
                {0x95, 0x04},
                {0x96, 0x38},//[10:0]out_height  0x438 = 1080
                {0x97, 0x07},
                {0x98, 0x80},//[11:0]out_width  0x780 = 1920
                {0xfe, 0x00},//pape 0
                {0x05, 0x05},//[11:8]hb   hts_h  0x05
                {0x06, 0xe0},//[7:0]hb    hts_l  0xe0
                {0x07, 0x00},//[13:8]vb
                {0x08, 0x19},//[7:0]vb
                {0x09, 0x00},//[10:8]cisctl row start
                {0x0a, 0x02},//[7:0]cisctl row start
                {0x0b, 0x00},//[10:8]cisctl col start
                {0x0c, 0x02},//[7:1]cisctl col start    
                {0x0d, 0x04},//[10:8]window height 1080+2
                {0x0e, 0x3A},//[7:0]window height   
                {0x0f, 0x07},//[10:8]window width 1920+16
                {0x10, 0x90},//[7:0]window width                 
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 50},// mdelay(2)
                {GADI_VI_SENSOR_TABLE_FLAG_END,},

            },
            .vi_info =
            {
                .width      = 1920,
                .height     = 1080,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_DVP, 
                .bits       = GADI_VIDEO_BITS_10,//10bit sensor, but the D[9:0] of gc2053 module is connected with the D[11:2] of gk720x 
                .ratio      = GADI_VIDEO_RATIO_16_9,
                .srm        = 0,
                .pll_index  = 2,
                .max_fps    = GADI_VIDEO_FPS(20),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(20),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_720P_25,
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
            .update_after_vsync_start   = 3,
            .update_after_vsync_end     = 10,
        },

        [3] = // 1280x720@50fps   (900 * 2) * 937.5 * 50 = 84375000
        {     // 1280x720@60fps   (900 * 2) * 781.25 * 60= 84375000                   
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280, 720,  60, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0xfe, 0x01},//pape 1
                {0x94, 0x01},
                {0x95, 0x02},//[10:0]out_height  0x2D0 = 720
                {0x96, 0xD0},
                {0x97, 0x05},//[11:0]out_width  0x500 = 1280
                {0x98, 0x00},  
                {0xfe, 0x00},//pape 0
                {0x05, 0x03},//[11:8]hb   hts_h    900x2=1800 
                {0x06, 0x84},//[7:0]hb    hts_l  
                {0x07, 0x00},//[13:8]vb
                {0x08, 0x0F},//[7:0]vb
                {0x09, 0x00},//[10:8]cisctl row start
                {0x0a, 0x02},//[7:0]cisctl row start
                {0x0b, 0x00},//[10:8]cisctl col start
                {0x0c, 0x02},//[7:1]cisctl col start  
                {0x0d, 0x02},//[10:8]window height 720+2
                {0x0e, 0xD2},//[7:0]window height   
                {0x0f, 0x05},//[10:8]window width 1280+0xc0
                {0x10, 0x81},//[7:0]window width  0x581~0x58C
                /* given by FAE of GC
                {0xfe, 0x01},//pape 1
                {0x94, 0x01},
                {0x95, 0x02},//[10:0]out_height  0x2D0 = 720
                {0x96, 0xD0+2},
                {0x97, 0x05},//[11:0]out_width  0x500 = 1280
                {0x98, 0x00},  
                {0xfe, 0x00},//pape 0
                {0x05, 0x04},//[11:8]hb   hts_h    1100x2=2200 
                {0x06, 0x4C},//[7:0]hb    hts_l  
                {0x07, 0x00},//[13:8]vb
                {0x08, 0x0f},//[7:0]vb
                {0x09, 0x00},//[10:8]cisctl row start
                {0x0a, 0x02},//[7:0]cisctl row start
                {0x0b, 0x00},//[10:8]cisctl col start
                {0x0c, 0x00},//{0x0c, 0x02},//[7:1]cisctl col start  
                {0x0d, 0x02},//[10:8]window height 720
                {0x0e, 0xD0+2},//[7:0]window height   
                {0x0f, 0x05},//[10:8]window width 1280
                {0x10, 0x00+0x81},//[7:0]window width        
                */
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 50},// mdelay(2)
                {GADI_VI_SENSOR_TABLE_FLAG_END,},

            },
            .vi_info =
            {
                .width      = 1280,
                .height     = 720,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_DVP, 
                .bits       = GADI_VIDEO_BITS_10,//10bit sensor, but the D[9:0] of gc2053 module is connected with the D[11:2] of gk720x 
                .ratio      = GADI_VIDEO_RATIO_16_9,
                .srm        = 0,
                .pll_index  = 1,
                .max_fps    = GADI_VIDEO_FPS(60),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(60),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_720P_60,//GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_720P_60,
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
            .update_after_vsync_start   = 3,
            .update_after_vsync_end     = 10,
        },
        [4] = // 1280x960P@30fps   (1100 * 2) * 1125 * 30 = 74250000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280, 960,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0xfe, 0x01},//pape 1
                {0x94, 0x01},
                {0x95, 0x03},
                {0x96, 0xC0+0},//[10:0]out_height  0x3C0 = 960
                {0x97, 0x05},
                {0x98, 0x00},//[11:0]out_width  0x500 = 1280        
                {0xfe, 0x00},//pape 0
                {0x05, 0x04},//[11:8]hb   hts_h 0x44c=1100
                {0x06, 0x4c},//[7:0]hb    hts_l
                {0x07, 0x00},//[13:8]vb
                {0x08, 0x0d},//[7:0]vb
                {0x09, 0x00},//[10:8]cisctl row start
                {0x0a, 0x02},//[7:0]cisctl row start
                {0x0b, 0x00},//[10:8]cisctl col start
                {0x0c, 0x02},//[7:1]cisctl col start
                {0x0d, 0x03},//[10:8]window height 960+2
                {0x0e, 0xC2},//[7:0]window height   
                {0x0f, 0x05},//[10:8]window width 1280+0x81
                {0x10, 0x81},//[7:0]window width   0x581~0x7FA
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 50},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 1280,
                .height     = 960,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_DVP, 
                .bits       = GADI_VIDEO_BITS_10,//10bit sensor, but the D[9:0] of gc2053 module is connected with the D[11:2] of gk720x 
                .ratio      = GADI_VIDEO_RATIO_16_9,
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
            .update_after_vsync_start   = 3,
            .update_after_vsync_end     = 10,
        },


                    
    },
#define gc2053_V_FLIP       (1<<1)
#define gc2053_H_MIRROR     (1<<0)
    .mirror_table =
    {
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY_VERTICALLY] =
        {
            //{0xFE, 0x01},                        
            //{0x92, 0x01}, //v_l         
            //{0x94, 0x01}, //h_l
            {0xfe, 0x00},//pape 0
            {0x17, gc2053_H_MIRROR | gc2053_V_FLIP, gc2053_H_MIRROR | gc2053_V_FLIP},
            //{0x94, 0x03},
            //{0x92, 0x01},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY] =
        {
            {0xfe, 0x00},//pape 0
            {0x17, gc2053_H_MIRROR , gc2053_H_MIRROR | gc2053_V_FLIP},
            //{0x94, 0x02},
            //{0x92, 0x02},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_VERTICALLY] =
        {
            {0xfe, 0x00},//pape 0
            {0x17, gc2053_V_FLIP, gc2053_H_MIRROR | gc2053_V_FLIP},
            //{0x94, 0x02},
            //{0x92, 0x01},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_NONE] =
        {
            {0xfe, 0x00},//pape 0
            {0x17, 0x00, gc2053_H_MIRROR | gc2053_V_FLIP},
            //{0x94, 0x03},
            //{0x92, 0x03},
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
    .shs_fix        = 8,
    .shs_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_VSYNC,
    .agc_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_VSYNC,
    .vmax_reg_update_mode       = GADI_SENSOR_UPDATE_REG_BY_VSYNC,
    .mirror_reg_update_mode     = GADI_SENSOR_UPDATE_REG_BY_VSYNC,
    .shs_additional_use         = 0,
    .shs_additional_percent     = 0,
    // hmax = (gc2053_HMAX_MSB & 0xFF) << 8 +
    //        (gc2053_HMAX_LSB & 0xFF) << 0
    .hmax_reg =
    {
        {0x05, 0x00, 0x0F, 0x00, 0x08, 0}, // gc2053_HMAX_MSB
        {0x06, 0x00, 0xFF, 0x00, 0x00, 0}, // gc2053_HMAX_LSB
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    // vmax = (gc2053_VMAX_MSB & 0xFF) << 8 +
    //        (gc2053_VMAX_LSB & 0xFF) << 0
    .vmax_reg =
    {
        {0x41, 0x00, 0x3F, 0x00, 0x08, 0}, // gc2053_VMAX_MSB
        {0x42, 0x00, 0xFF, 0x00, 0x00, 0}, // gc2053_VMAX_LSB
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .vmax_wadd_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    // shs = (INTEG_TIME_H & 0xFF) << 8 +
    //       (INTEG_TIME_L & 0xFF) << 0
    .shs_reg =
    {
        {0x03, 0x00, 0x3F, 0x00, 0x08, 0}, // INTEG_TIME_H
        {0x04, 0x00, 0xFF, 0x00, 0x00, 0}, // INTEG_TIME_L
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .shs_additional_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    .cal_gain_mode      = GADI_SENSOR_CAL_GAIN_MODE1,
    .max_agc_index   = 385,
    .max_again_index = 0,
    .cur_again_index = 0,
    .agc_info =
    {
        .db_max     = 0x30000000,    // 48dB
        .db_min     = 0x00000000,    // 0dB
        .db_step    = 0x00300000,    // Useless
    },
    .gain_reg =
    {    
        {0xfe,},
        {0xB4,},
        {0xB3,},
        {0xB1,},
        {0xB2,},    
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .gain_table =
   {
        {0x00000000, {0x00, 0x00, 0x00, 0x01,0x00}},/*gain=1.000000(0.000000 db)*/
        {0x00300000, {0x00, 0x00, 0x00, 0x01,0x04}},/*gain=1.021897(0.187500 db)*/
        {0x00480000, {0x00, 0x00, 0x00, 0x01,0x08}},/*gain=1.033025(0.281250 db)*/
        {0x00780000, {0x00, 0x00, 0x00, 0x01,0x0C}},/*gain=1.055645(0.468750 db)*/
        {0x00900000, {0x00, 0x00, 0x00, 0x01,0x10}},/*gain=1.067140(0.562500 db)*/
        {0x00A80000, {0x00, 0x00, 0x00, 0x01,0x14}},/*gain=1.078761(0.656250 db)*/
        {0x00D80000, {0x00, 0x00, 0x00, 0x01,0x18}},/*gain=1.102383(0.843750 db)*/
        {0x00F00000, {0x00, 0x00, 0x00, 0x01,0x1C}},/*gain=1.114387(0.937500 db)*/
        {0x01080000, {0x00, 0x00, 0x00, 0x01,0x20}},/*gain=1.126522(1.031250 db)*/
        {0x01380000, {0x00, 0x00, 0x00, 0x01,0x24}},/*gain=1.151189(1.218750 db)*/
        {0x01500000, {0x00, 0x00, 0x00, 0x01,0x28}},/*gain=1.163725(1.312500 db)*/
        {0x01680000, {0x00, 0x00, 0x00, 0x01,0x2C}},/*gain=1.176397(1.406250 db)*/
        {0x01800000, {0x00, 0x00, 0x10, 0x01,0x00}},/*gain=1.189207(1.500000 db)*/
        {0x01C80000, {0x00, 0x00, 0x10, 0x01,0x04}},/*gain=1.228481(1.781250 db)*/
        {0x01E00000, {0x00, 0x00, 0x10, 0x01,0x08}},/*gain=1.241858(1.875000 db)*/
        {0x01F80000, {0x00, 0x00, 0x10, 0x01,0x0C}},/*gain=1.255381(1.968750 db)*/
        {0x02100000, {0x00, 0x00, 0x10, 0x01,0x10}},/*gain=1.269051(2.062500 db)*/
        {0x02280000, {0x00, 0x00, 0x10, 0x01,0x14}},/*gain=1.282870(2.156250 db)*/
        {0x02700000, {0x00, 0x00, 0x10, 0x01,0x18}},/*gain=1.325237(2.437500 db)*/
        {0x02880000, {0x00, 0x00, 0x10, 0x01,0x1C}},/*gain=1.339668(2.531250 db)*/
        {0x02A00000, {0x00, 0x00, 0x10, 0x01,0x20}},/*gain=1.354256(2.625000 db)*/
        {0x02B80000, {0x00, 0x00, 0x10, 0x01,0x24}},/*gain=1.369002(2.718750 db)*/
        {0x02D00000, {0x00, 0x00, 0x10, 0x01,0x28}},/*gain=1.383910(2.812500 db)*/
        {0x03000000, {0x00, 0x00, 0x20, 0x01,0x00}},/*gain=1.414214(3.000000 db)*/
        {0x03300000, {0x00, 0x00, 0x20, 0x01,0x04}},/*gain=1.445181(3.187500 db)*/
        {0x03480000, {0x00, 0x00, 0x20, 0x01,0x08}},/*gain=1.460918(3.281250 db)*/
        {0x03780000, {0x00, 0x00, 0x20, 0x01,0x0C}},/*gain=1.492908(3.468750 db)*/
        {0x03900000, {0x00, 0x00, 0x20, 0x01,0x10}},/*gain=1.509164(3.562500 db)*/
        {0x03C00000, {0x00, 0x00, 0x20, 0x01,0x14}},/*gain=1.542211(3.750000 db)*/
        {0x03D80000, {0x00, 0x00, 0x20, 0x01,0x18}},/*gain=1.559004(3.843750 db)*/
        {0x03F00000, {0x00, 0x00, 0x20, 0x01,0x1C}},/*gain=1.575981(3.937500 db)*/
        {0x04200000, {0x00, 0x00, 0x20, 0x01,0x24}},/*gain=1.610490(4.125000 db)*/
        {0x04500000, {0x00, 0x00, 0x20, 0x01,0x28}},/*gain=1.645755(4.312500 db)*/
        {0x04680000, {0x00, 0x00, 0x30, 0x01,0x00}},/*gain=1.663677(4.406250 db)*/
        {0x04980000, {0x00, 0x00, 0x30, 0x01,0x04}},/*gain=1.700106(4.593750 db)*/
        {0x04C80000, {0x00, 0x00, 0x30, 0x01,0x0C}},/*gain=1.737334(4.781250 db)*/
        {0x04F80000, {0x00, 0x00, 0x30, 0x01,0x10}},/*gain=1.775376(4.968750 db)*/
        {0x05280000, {0x00, 0x00, 0x30, 0x01,0x18}},/*gain=1.814252(5.156250 db)*/
        {0x05580000, {0x00, 0x00, 0x30, 0x01,0x1C}},/*gain=1.853979(5.343750 db)*/
        {0x05880000, {0x00, 0x00, 0x30, 0x01,0x24}},/*gain=1.894576(5.531250 db)*/
        {0x05B80000, {0x00, 0x00, 0x30, 0x01,0x28}},/*gain=1.936062(5.718750 db)*/
        {0x05D00000, {0x00, 0x00, 0x30, 0x01,0x2C}},/*gain=1.957144(5.812500 db)*/
        {0x05E80000, {0x00, 0x00, 0x30, 0x01,0x30}},/*gain=1.978456(5.906250 db)*/
        {0x06000000, {0x00, 0x00, 0x40, 0x01,0x00}},/*gain=2.000000(6.000000 db)*/
        {0x06300000, {0x00, 0x00, 0x40, 0x01,0x04}},/*gain=2.043794(6.187500 db)*/
        {0x06480000, {0x00, 0x00, 0x40, 0x01,0x08}},/*gain=2.066050(6.281250 db)*/
        {0x06780000, {0x00, 0x00, 0x40, 0x01,0x0C}},/*gain=2.111290(6.468750 db)*/
        {0x06900000, {0x00, 0x00, 0x40, 0x01,0x10}},/*gain=2.134281(6.562500 db)*/
        {0x06A80000, {0x00, 0x00, 0x40, 0x01,0x14}},/*gain=2.157522(6.656250 db)*/
        {0x06D80000, {0x00, 0x00, 0x40, 0x01,0x18}},/*gain=2.204765(6.843750 db)*/
        {0x06F00000, {0x00, 0x00, 0x40, 0x01,0x1C}},/*gain=2.228773(6.937500 db)*/
        {0x07080000, {0x00, 0x00, 0x40, 0x01,0x20}},/*gain=2.253043(7.031250 db)*/
        {0x07380000, {0x00, 0x00, 0x40, 0x01,0x24}},/*gain=2.302378(7.218750 db)*/
        {0x07500000, {0x00, 0x00, 0x40, 0x01,0x28}},/*gain=2.327450(7.312500 db)*/
        {0x07680000, {0x00, 0x00, 0x40, 0x01,0x2C}},/*gain=2.352794(7.406250 db)*/
        {0x07800000, {0x00, 0x00, 0x50, 0x01,0x00}},/*gain=2.378414(7.500000 db)*/
        {0x07B00000, {0x00, 0x00, 0x50, 0x01,0x04}},/*gain=2.430495(7.687500 db)*/
        {0x07C80000, {0x00, 0x00, 0x50, 0x01,0x08}},/*gain=2.456961(7.781250 db)*/
        {0x07F80000, {0x00, 0x00, 0x50, 0x01,0x0C}},/*gain=2.510762(7.968750 db)*/
        {0x08100000, {0x00, 0x00, 0x50, 0x01,0x10}},/*gain=2.538102(8.062500 db)*/
        {0x08280000, {0x00, 0x00, 0x50, 0x01,0x14}},/*gain=2.565740(8.156250 db)*/
        {0x08580000, {0x00, 0x00, 0x50, 0x01,0x18}},/*gain=2.621922(8.343750 db)*/
        {0x08700000, {0x00, 0x00, 0x50, 0x01,0x1C}},/*gain=2.650473(8.437500 db)*/
        {0x08880000, {0x00, 0x00, 0x50, 0x01,0x20}},/*gain=2.679335(8.531250 db)*/
        {0x08B80000, {0x00, 0x00, 0x50, 0x01,0x24}},/*gain=2.738005(8.718750 db)*/
        {0x08D00000, {0x00, 0x00, 0x50, 0x01,0x28}},/*gain=2.767820(8.812500 db)*/
        {0x08E80000, {0x00, 0x00, 0x60, 0x01,0x00}},/*gain=2.797959(8.906250 db)*/
        {0x09180000, {0x00, 0x00, 0x60, 0x01,0x04}},/*gain=2.859227(9.093750 db)*/
        {0x09480000, {0x00, 0x00, 0x60, 0x01,0x08}},/*gain=2.921836(9.281250 db)*/
        {0x09600000, {0x00, 0x00, 0x60, 0x01,0x0C}},/*gain=2.953652(9.375000 db)*/
        {0x09780000, {0x00, 0x00, 0x60, 0x01,0x10}},/*gain=2.985815(9.468750 db)*/
        {0x09900000, {0x00, 0x00, 0x60, 0x01,0x14}},/*gain=3.018329(9.562500 db)*/
        {0x09C00000, {0x00, 0x00, 0x60, 0x01,0x18}},/*gain=3.084422(9.750000 db)*/
        {0x09D80000, {0x00, 0x00, 0x60, 0x01,0x1C}},/*gain=3.118009(9.843750 db)*/
        {0x0A080000, {0x00, 0x00, 0x60, 0x01,0x20}},/*gain=3.186284(10.031250 db)*/
        {0x0A200000, {0x00, 0x00, 0x60, 0x01,0x24}},/*gain=3.220981(10.125000 db)*/
        {0x0A380000, {0x00, 0x00, 0x60, 0x01,0x28}},/*gain=3.256055(10.218750 db)*/
        {0x0A500000, {0x00, 0x00, 0x60, 0x01,0x2C}},/*gain=3.291511(10.312500 db)*/
        {0x0A680000, {0x00, 0x00, 0x70, 0x01,0x00}},/*gain=3.327353(10.406250 db)*/
        {0x0A980000, {0x00, 0x00, 0x70, 0x01,0x04}},/*gain=3.400213(10.593750 db)*/
        {0x0AB00000, {0x00, 0x00, 0x70, 0x01,0x08}},/*gain=3.437239(10.687500 db)*/
        {0x0AC80000, {0x00, 0x00, 0x70, 0x01,0x0C}},/*gain=3.474668(10.781250 db)*/
        {0x0AF80000, {0x00, 0x00, 0x70, 0x01,0x10}},/*gain=3.550753(10.968750 db)*/
        {0x0B100000, {0x00, 0x00, 0x70, 0x01,0x14}},/*gain=3.589418(11.062500 db)*/
        {0x0B280000, {0x00, 0x00, 0x70, 0x01,0x18}},/*gain=3.628504(11.156250 db)*/
        {0x0B580000, {0x00, 0x00, 0x70, 0x01,0x1C}},/*gain=3.707958(11.343750 db)*/
        {0x0B700000, {0x00, 0x00, 0x70, 0x01,0x20}},/*gain=3.748335(11.437500 db)*/
        {0x0B880000, {0x00, 0x00, 0x70, 0x01,0x24}},/*gain=3.789152(11.531250 db)*/
        {0x0BB80000, {0x00, 0x00, 0x70, 0x01,0x28}},/*gain=3.872124(11.718750 db)*/
        {0x0BD00000, {0x00, 0x00, 0x70, 0x01,0x2C}},/*gain=3.914288(11.812500 db)*/
        {0x0BE80000, {0x00, 0x00, 0x70, 0x01,0x30}},/*gain=3.956912(11.906250 db)*/
        {0x0C000000, {0x00, 0x00, 0x80, 0x01,0x00}},/*gain=4.000000(12.000000 db)*/
        {0x0C300000, {0x00, 0x00, 0x80, 0x01,0x04}},/*gain=4.087589(12.187500 db)*/
        {0x0C480000, {0x00, 0x00, 0x80, 0x01,0x08}},/*gain=4.132100(12.281250 db)*/
        {0x0C780000, {0x00, 0x00, 0x80, 0x01,0x0C}},/*gain=4.222581(12.468750 db)*/
        {0x0C900000, {0x00, 0x00, 0x80, 0x01,0x10}},/*gain=4.268562(12.562500 db)*/
        {0x0CA80000, {0x00, 0x00, 0x80, 0x01,0x14}},/*gain=4.315043(12.656250 db)*/
        {0x0CD80000, {0x00, 0x00, 0x80, 0x01,0x18}},/*gain=4.409530(12.843750 db)*/
        {0x0CF00000, {0x00, 0x00, 0x80, 0x01,0x1C}},/*gain=4.457547(12.937500 db)*/
        {0x0D080000, {0x00, 0x00, 0x80, 0x01,0x20}},/*gain=4.506086(13.031250 db)*/
        {0x0D380000, {0x00, 0x00, 0x80, 0x01,0x24}},/*gain=4.604757(13.218750 db)*/
        {0x0D500000, {0x00, 0x00, 0x80, 0x01,0x28}},/*gain=4.654899(13.312500 db)*/
        {0x0D680000, {0x00, 0x00, 0x80, 0x01,0x2C}},/*gain=4.705588(13.406250 db)*/
        {0x0D800000, {0x00, 0x00, 0x90, 0x01,0x00}},/*gain=4.756828(13.500000 db)*/
        {0x0DB00000, {0x00, 0x00, 0x90, 0x01,0x04}},/*gain=4.860989(13.687500 db)*/
        {0x0DC80000, {0x00, 0x00, 0x90, 0x01,0x08}},/*gain=4.913922(13.781250 db)*/
        {0x0DF80000, {0x00, 0x00, 0x90, 0x01,0x0C}},/*gain=5.021523(13.968750 db)*/
        {0x0E100000, {0x00, 0x00, 0x90, 0x01,0x10}},/*gain=5.076204(14.062500 db)*/
        {0x0E280000, {0x00, 0x00, 0x90, 0x01,0x14}},/*gain=5.131480(14.156250 db)*/
        {0x0E580000, {0x00, 0x00, 0x90, 0x01,0x18}},/*gain=5.243845(14.343750 db)*/
        {0x0E700000, {0x00, 0x00, 0x90, 0x01,0x1C}},/*gain=5.300947(14.437500 db)*/
        {0x0E880000, {0x00, 0x00, 0x90, 0x01,0x20}},/*gain=5.358670(14.531250 db)*/
        {0x0EA00000, {0x00, 0x00, 0x90, 0x01,0x24}},/*gain=5.417022(14.625000 db)*/
        {0x0ED00000, {0x00, 0x00, 0x90, 0x01,0x28}},/*gain=5.535640(14.812500 db)*/
        {0x0EE80000, {0x00, 0x00, 0xA0, 0x01,0x00}},/*gain=5.595919(14.906250 db)*/
        {0x0F180000, {0x00, 0x00, 0xA0, 0x01,0x04}},/*gain=5.718453(15.093750 db)*/
        {0x0F480000, {0x00, 0x00, 0xA0, 0x01,0x08}},/*gain=5.843671(15.281250 db)*/
        {0x0F600000, {0x00, 0x00, 0xA0, 0x01,0x0C}},/*gain=5.907305(15.375000 db)*/
        {0x0F780000, {0x00, 0x00, 0xA0, 0x01,0x10}},/*gain=5.971631(15.468750 db)*/
        {0x0F900000, {0x00, 0x00, 0xA0, 0x01,0x14}},/*gain=6.036658(15.562500 db)*/
        {0x0FC00000, {0x00, 0x00, 0xA0, 0x01,0x18}},/*gain=6.168843(15.750000 db)*/
        {0x0FD80000, {0x00, 0x00, 0xA0, 0x01,0x1C}},/*gain=6.236018(15.843750 db)*/
        {0x0FF00000, {0x00, 0x00, 0xA0, 0x01,0x20}},/*gain=6.303923(15.937500 db)*/
        {0x10200000, {0x00, 0x00, 0xA0, 0x01,0x24}},/*gain=6.441961(16.125000 db)*/
        {0x10380000, {0x00, 0x00, 0xA0, 0x01,0x28}},/*gain=6.512110(16.218750 db)*/
        {0x10500000, {0x00, 0x00, 0xA0, 0x01,0x2C}},/*gain=6.583022(16.312500 db)*/
        {0x10680000, {0x00, 0x00, 0xB0, 0x01,0x00}},/*gain=6.654706(16.406250 db)*/
        {0x10980000, {0x00, 0x00, 0xB0, 0x01,0x04}},/*gain=6.800425(16.593750 db)*/
        {0x10B00000, {0x00, 0x00, 0xB0, 0x01,0x08}},/*gain=6.874477(16.687500 db)*/
        {0x10E00000, {0x00, 0x00, 0xB0, 0x01,0x0C}},/*gain=7.025009(16.875000 db)*/
        {0x10F80000, {0x00, 0x00, 0xB0, 0x01,0x10}},/*gain=7.101506(16.968750 db)*/
        {0x11100000, {0x00, 0x00, 0xB0, 0x01,0x14}},/*gain=7.178836(17.062500 db)*/
        {0x11400000, {0x00, 0x00, 0xB0, 0x01,0x18}},/*gain=7.336032(17.250000 db)*/
        {0x11580000, {0x00, 0x00, 0xB0, 0x01,0x1C}},/*gain=7.415917(17.343750 db)*/
        {0x11700000, {0x00, 0x00, 0xB0, 0x01,0x20}},/*gain=7.496671(17.437500 db)*/
        {0x11880000, {0x00, 0x00, 0xB0, 0x01,0x24}},/*gain=7.578304(17.531250 db)*/
        {0x11B80000, {0x00, 0x00, 0xB0, 0x01,0x28}},/*gain=7.744247(17.718750 db)*/
        {0x11D00000, {0x00, 0x00, 0xB0, 0x01,0x2C}},/*gain=7.828576(17.812500 db)*/
        {0x11E80000, {0x00, 0x00, 0xB0, 0x01,0x30}},/*gain=7.913824(17.906250 db)*/
        {0x12000000, {0x00, 0x00, 0xC0, 0x01,0x00}},/*gain=8.000000(18.000000 db)*/
        {0x12300000, {0x00, 0x00, 0xC0, 0x01,0x04}},/*gain=8.175177(18.187500 db)*/
        {0x12480000, {0x00, 0x00, 0xC0, 0x01,0x08}},/*gain=8.264199(18.281250 db)*/
        {0x12780000, {0x00, 0x00, 0xC0, 0x01,0x0C}},/*gain=8.445161(18.468750 db)*/
        {0x12900000, {0x00, 0x00, 0xC0, 0x01,0x10}},/*gain=8.537123(18.562500 db)*/
        {0x12A80000, {0x00, 0x00, 0xC0, 0x01,0x14}},/*gain=8.630086(18.656250 db)*/
        {0x12D80000, {0x00, 0x00, 0xC0, 0x01,0x18}},/*gain=8.819061(18.843750 db)*/
        {0x12F00000, {0x00, 0x00, 0xC0, 0x01,0x1C}},/*gain=8.915094(18.937500 db)*/
        {0x13080000, {0x00, 0x00, 0xC0, 0x01,0x20}},/*gain=9.012173(19.031250 db)*/
        {0x13380000, {0x00, 0x00, 0xC0, 0x01,0x24}},/*gain=9.209514(19.218750 db)*/
        {0x13500000, {0x00, 0x00, 0xC0, 0x01,0x28}},/*gain=9.309799(19.312500 db)*/
        {0x13680000, {0x00, 0x00, 0xC0, 0x01,0x2C}},/*gain=9.411176(19.406250 db)*/
        {0x13800000, {0x00, 0x00, 0x5A, 0x01,0x00}},/*gain=9.513657(19.500000 db)*/
        {0x13B00000, {0x00, 0x00, 0x5A, 0x01,0x04}},/*gain=9.721979(19.687500 db)*/
        {0x13C80000, {0x00, 0x00, 0x5A, 0x01,0x08}},/*gain=9.827844(19.781250 db)*/
        {0x13F80000, {0x00, 0x00, 0x5A, 0x01,0x0C}},/*gain=10.043046(19.968750 db)*/
        {0x14100000, {0x00, 0x00, 0x5A, 0x01,0x10}},/*gain=10.152408(20.062500 db)*/
        {0x14280000, {0x00, 0x00, 0x5A, 0x01,0x14}},/*gain=10.262960(20.156250 db)*/
        {0x14580000, {0x00, 0x00, 0x5A, 0x01,0x18}},/*gain=10.487690(20.343750 db)*/
        {0x14700000, {0x00, 0x00, 0x5A, 0x01,0x1C}},/*gain=10.601893(20.437500 db)*/
        {0x14880000, {0x00, 0x00, 0x5A, 0x01,0x20}},/*gain=10.717340(20.531250 db)*/
        {0x14A00000, {0x00, 0x00, 0x5A, 0x01,0x24}},/*gain=10.834044(20.625000 db)*/
        {0x14D00000, {0x00, 0x00, 0x5A, 0x01,0x28}},/*gain=11.071279(20.812500 db)*/
        {0x14E80000, {0x00, 0x00, 0x5A, 0x01,0x2C}},/*gain=11.191837(20.906250 db)*/
        {0x15000000, {0x00, 0x00, 0x83, 0x01,0x00}},/*gain=11.313708(21.000000 db)*/
        {0x15180000, {0x00, 0x00, 0x83, 0x01,0x04}},/*gain=11.436907(21.093750 db)*/
        {0x15480000, {0x00, 0x00, 0x83, 0x01,0x08}},/*gain=11.687342(21.281250 db)*/
        {0x15600000, {0x00, 0x00, 0x83, 0x01,0x0C}},/*gain=11.814609(21.375000 db)*/
        {0x15780000, {0x00, 0x00, 0x83, 0x01,0x10}},/*gain=11.943262(21.468750 db)*/
        {0x15A80000, {0x00, 0x00, 0x83, 0x01,0x14}},/*gain=12.204785(21.656250 db)*/
        {0x15C00000, {0x00, 0x00, 0x83, 0x01,0x18}},/*gain=12.337687(21.750000 db)*/
        {0x15D80000, {0x00, 0x00, 0x83, 0x01,0x1C}},/*gain=12.472035(21.843750 db)*/
        {0x16080000, {0x00, 0x00, 0x83, 0x01,0x20}},/*gain=12.745137(22.031250 db)*/
        {0x16200000, {0x00, 0x00, 0x83, 0x01,0x24}},/*gain=12.883923(22.125000 db)*/
        {0x16380000, {0x00, 0x00, 0x83, 0x01,0x28}},/*gain=13.024219(22.218750 db)*/
        {0x16500000, {0x00, 0x00, 0x83, 0x01,0x2C}},/*gain=13.166044(22.312500 db)*/
        {0x16680000, {0x00, 0x00, 0x93, 0x01,0x00}},/*gain=13.309413(22.406250 db)*/
        {0x16980000, {0x00, 0x00, 0x93, 0x01,0x04}},/*gain=13.600851(22.593750 db)*/
        {0x16B00000, {0x00, 0x00, 0x93, 0x01,0x08}},/*gain=13.748954(22.687500 db)*/
        {0x16C80000, {0x00, 0x00, 0x93, 0x01,0x0C}},/*gain=13.898671(22.781250 db)*/
        {0x16F80000, {0x00, 0x00, 0x93, 0x01,0x10}},/*gain=14.203012(22.968750 db)*/
        {0x17100000, {0x00, 0x00, 0x93, 0x01,0x14}},/*gain=14.357673(23.062500 db)*/
        {0x17400000, {0x00, 0x00, 0x93, 0x01,0x18}},/*gain=14.672065(23.250000 db)*/
        {0x17580000, {0x00, 0x00, 0x93, 0x01,0x1C}},/*gain=14.831833(23.343750 db)*/
        {0x17700000, {0x00, 0x00, 0x93, 0x01,0x20}},/*gain=14.993341(23.437500 db)*/
        {0x17880000, {0x00, 0x00, 0x93, 0x01,0x24}},/*gain=15.156608(23.531250 db)*/
        {0x17B80000, {0x00, 0x00, 0x93, 0x01,0x28}},/*gain=15.488494(23.718750 db)*/
        {0x17D00000, {0x00, 0x00, 0x93, 0x01,0x2C}},/*gain=15.657153(23.812500 db)*/
        {0x17E80000, {0x00, 0x00, 0x93, 0x01,0x30}},/*gain=15.827648(23.906250 db)*/
        {0x18000000, {0x00, 0x00, 0x84, 0x01,0x00}},/*gain=16.000000(24.000000 db)*/
        {0x18300000, {0x00, 0x00, 0x84, 0x01,0x04}},/*gain=16.350354(24.187500 db)*/
        {0x18480000, {0x00, 0x00, 0x84, 0x01,0x08}},/*gain=16.528398(24.281250 db)*/
        {0x18780000, {0x00, 0x00, 0x84, 0x01,0x0C}},/*gain=16.890323(24.468750 db)*/
        {0x18900000, {0x00, 0x00, 0x84, 0x01,0x10}},/*gain=17.074246(24.562500 db)*/
        {0x18A80000, {0x00, 0x00, 0x84, 0x01,0x14}},/*gain=17.260173(24.656250 db)*/
        {0x18D80000, {0x00, 0x00, 0x84, 0x01,0x18}},/*gain=17.638121(24.843750 db)*/
        {0x18F00000, {0x00, 0x00, 0x84, 0x01,0x1C}},/*gain=17.830188(24.937500 db)*/
        {0x19080000, {0x00, 0x00, 0x84, 0x01,0x20}},/*gain=18.024346(25.031250 db)*/
        {0x19380000, {0x00, 0x00, 0x84, 0x01,0x24}},/*gain=18.419028(25.218750 db)*/
        {0x19500000, {0x00, 0x00, 0x84, 0x01,0x28}},/*gain=18.619598(25.312500 db)*/
        {0x19680000, {0x00, 0x00, 0x84, 0x01,0x2C}},/*gain=18.822352(25.406250 db)*/
        {0x19800000, {0x00, 0x00, 0x94, 0x01,0x00}},/*gain=19.027314(25.500000 db)*/
        {0x19B00000, {0x00, 0x00, 0x94, 0x01,0x04}},/*gain=19.443958(25.687500 db)*/
        {0x19C80000, {0x00, 0x00, 0x94, 0x01,0x08}},/*gain=19.655689(25.781250 db)*/
        {0x19E00000, {0x00, 0x00, 0x94, 0x01,0x0C}},/*gain=19.869725(25.875000 db)*/
        {0x1A100000, {0x00, 0x00, 0x94, 0x01,0x10}},/*gain=20.304815(26.062500 db)*/
        {0x1A280000, {0x00, 0x00, 0x94, 0x01,0x14}},/*gain=20.525920(26.156250 db)*/
        {0x1A400000, {0x00, 0x00, 0x94, 0x01,0x18}},/*gain=20.749433(26.250000 db)*/
        {0x1A700000, {0x00, 0x00, 0x94, 0x01,0x1C}},/*gain=21.203786(26.437500 db)*/
        {0x1A880000, {0x00, 0x00, 0x94, 0x01,0x20}},/*gain=21.434680(26.531250 db)*/
        {0x1AA00000, {0x00, 0x00, 0x94, 0x01,0x24}},/*gain=21.668089(26.625000 db)*/
        {0x1AD00000, {0x00, 0x00, 0x94, 0x01,0x28}},/*gain=22.142558(26.812500 db)*/
        {0x1AE80000, {0x00, 0x00, 0x94, 0x01,0x2C}},/*gain=22.383675(26.906250 db)*/
        {0x1B000000, {0x00, 0x01, 0x2C, 0x01,0x00}},/*gain=22.627417(27.000000 db)*/
        {0x1B180000, {0x00, 0x01, 0x2C, 0x01,0x04}},/*gain=22.873813(27.093750 db)*/
        {0x1B300000, {0x00, 0x01, 0x2C, 0x01,0x08}},/*gain=23.122893(27.187500 db)*/
        {0x1B600000, {0x00, 0x01, 0x2C, 0x01,0x0C}},/*gain=23.629218(27.375000 db)*/
        {0x1B780000, {0x00, 0x01, 0x2C, 0x01,0x10}},/*gain=23.886524(27.468750 db)*/
        {0x1BA80000, {0x00, 0x01, 0x2C, 0x01,0x14}},/*gain=24.409570(27.656250 db)*/
        {0x1BC00000, {0x00, 0x01, 0x2C, 0x01,0x18}},/*gain=24.675373(27.750000 db)*/
        {0x1BD80000, {0x00, 0x01, 0x2C, 0x01,0x1C}},/*gain=24.944070(27.843750 db)*/
        {0x1C080000, {0x00, 0x01, 0x2C, 0x01,0x20}},/*gain=25.490274(28.031250 db)*/
        {0x1C200000, {0x00, 0x01, 0x2C, 0x01,0x24}},/*gain=25.767845(28.125000 db)*/
        {0x1C380000, {0x00, 0x01, 0x2C, 0x01,0x28}},/*gain=26.048439(28.218750 db)*/
        {0x1C500000, {0x00, 0x01, 0x2C, 0x01,0x2C}},/*gain=26.332088(28.312500 db)*/
        {0x1C680000, {0x00, 0x01, 0x3C, 0x01,0x00}},/*gain=26.618825(28.406250 db)*/
        {0x1C980000, {0x00, 0x01, 0x3C, 0x01,0x04}},/*gain=27.201702(28.593750 db)*/
        {0x1CB00000, {0x00, 0x01, 0x3C, 0x01,0x08}},/*gain=27.497909(28.687500 db)*/
        {0x1CC80000, {0x00, 0x01, 0x3C, 0x01,0x0C}},/*gain=27.797341(28.781250 db)*/
        {0x1CF80000, {0x00, 0x01, 0x3C, 0x01,0x10}},/*gain=28.406024(28.968750 db)*/
        {0x1D100000, {0x00, 0x01, 0x3C, 0x01,0x14}},/*gain=28.715345(29.062500 db)*/
        {0x1D400000, {0x00, 0x01, 0x3C, 0x01,0x18}},/*gain=29.344129(29.250000 db)*/
        {0x1D580000, {0x00, 0x01, 0x3C, 0x01,0x1C}},/*gain=29.663666(29.343750 db)*/
        {0x1D700000, {0x00, 0x01, 0x3C, 0x01,0x20}},/*gain=29.986682(29.437500 db)*/
        {0x1D880000, {0x00, 0x01, 0x3C, 0x01,0x24}},/*gain=30.313216(29.531250 db)*/
        {0x1DB80000, {0x00, 0x01, 0x3C, 0x01,0x28}},/*gain=30.976989(29.718750 db)*/
        {0x1DD00000, {0x00, 0x01, 0x3C, 0x01,0x2C}},/*gain=31.314306(29.812500 db)*/
        {0x1DE80000, {0x00, 0x01, 0x3C, 0x01,0x30}},/*gain=31.655296(29.906250 db)*/
        {0x1E000000, {0x00, 0x00, 0x8C, 0x01,0x00}},/*gain=32.000000(30.000000 db)*/
        {0x1E300000, {0x00, 0x00, 0x8C, 0x01,0x04}},/*gain=32.700709(30.187500 db)*/
        {0x1E480000, {0x00, 0x00, 0x8C, 0x01,0x08}},/*gain=33.056796(30.281250 db)*/
        {0x1E780000, {0x00, 0x00, 0x8C, 0x01,0x0C}},/*gain=33.780646(30.468750 db)*/
        {0x1E900000, {0x00, 0x00, 0x8C, 0x01,0x10}},/*gain=34.148493(30.562500 db)*/
        {0x1EA80000, {0x00, 0x00, 0x8C, 0x01,0x14}},/*gain=34.520346(30.656250 db)*/
        {0x1ED80000, {0x00, 0x00, 0x8C, 0x01,0x18}},/*gain=35.276243(30.843750 db)*/
        {0x1EF00000, {0x00, 0x00, 0x8C, 0x01,0x1C}},/*gain=35.660376(30.937500 db)*/
        {0x1F080000, {0x00, 0x00, 0x8C, 0x01,0x20}},/*gain=36.048692(31.031250 db)*/
        {0x1F380000, {0x00, 0x00, 0x8C, 0x01,0x24}},/*gain=36.838055(31.218750 db)*/
        {0x1F500000, {0x00, 0x00, 0x8C, 0x01,0x28}},/*gain=37.239195(31.312500 db)*/
        {0x1F680000, {0x00, 0x00, 0x8C, 0x01,0x2C}},/*gain=37.644704(31.406250 db)*/
        {0x1F800000, {0x00, 0x00, 0x9C, 0x01,0x00}},/*gain=38.054628(31.500000 db)*/
        {0x1FB00000, {0x00, 0x00, 0x9C, 0x01,0x04}},/*gain=38.887916(31.687500 db)*/
        {0x1FC80000, {0x00, 0x00, 0x9C, 0x01,0x08}},/*gain=39.311377(31.781250 db)*/
        {0x1FE00000, {0x00, 0x00, 0x9C, 0x01,0x0C}},/*gain=39.739450(31.875000 db)*/
        {0x20100000, {0x00, 0x00, 0x9C, 0x01,0x10}},/*gain=40.609631(32.062500 db)*/
        {0x20280000, {0x00, 0x00, 0x9C, 0x01,0x14}},/*gain=41.051841(32.156250 db)*/
        {0x20400000, {0x00, 0x00, 0x9C, 0x01,0x18}},/*gain=41.498866(32.250000 db)*/
        {0x20700000, {0x00, 0x00, 0x9C, 0x01,0x1C}},/*gain=42.407573(32.437500 db)*/
        {0x20880000, {0x00, 0x00, 0x9C, 0x01,0x20}},/*gain=42.869361(32.531250 db)*/
        {0x20A00000, {0x00, 0x00, 0x9C, 0x01,0x24}},/*gain=43.336178(32.625000 db)*/
        {0x20D00000, {0x00, 0x00, 0x9C, 0x01,0x28}},/*gain=44.285116(32.812500 db)*/
        {0x20E80000, {0x00, 0x00, 0x9C, 0x01,0x2C}},/*gain=44.767350(32.906250 db)*/
        {0x21000000, {0x00, 0x02, 0x64, 0x01,0x00}},/*gain=45.254834(33.000000 db)*/
        {0x21180000, {0x00, 0x02, 0x64, 0x01,0x04}},/*gain=45.747627(33.093750 db)*/
        {0x21300000, {0x00, 0x02, 0x64, 0x01,0x08}},/*gain=46.245786(33.187500 db)*/
        {0x21600000, {0x00, 0x02, 0x64, 0x01,0x0C}},/*gain=47.258437(33.375000 db)*/
        {0x21780000, {0x00, 0x02, 0x64, 0x01,0x10}},/*gain=47.773047(33.468750 db)*/
        {0x21A80000, {0x00, 0x02, 0x64, 0x01,0x14}},/*gain=48.819141(33.656250 db)*/
        {0x21C00000, {0x00, 0x02, 0x64, 0x01,0x18}},/*gain=49.350746(33.750000 db)*/
        {0x21D80000, {0x00, 0x02, 0x64, 0x01,0x1C}},/*gain=49.888141(33.843750 db)*/
        {0x21F00000, {0x00, 0x02, 0x64, 0x01,0x20}},/*gain=50.431387(33.937500 db)*/
        {0x22200000, {0x00, 0x02, 0x64, 0x01,0x24}},/*gain=51.535691(34.125000 db)*/
        {0x22380000, {0x00, 0x02, 0x64, 0x01,0x28}},/*gain=52.096877(34.218750 db)*/
        {0x22500000, {0x00, 0x02, 0x64, 0x01,0x2C}},/*gain=52.664175(34.312500 db)*/
        {0x22680000, {0x00, 0x02, 0x74, 0x01,0x00}},/*gain=53.237651(34.406250 db)*/
        {0x22980000, {0x00, 0x02, 0x74, 0x01,0x04}},/*gain=54.403403(34.593750 db)*/
        {0x22B00000, {0x00, 0x02, 0x74, 0x01,0x08}},/*gain=54.995818(34.687500 db)*/
        {0x22C80000, {0x00, 0x02, 0x74, 0x01,0x0C}},/*gain=55.594683(34.781250 db)*/
        {0x22F80000, {0x00, 0x02, 0x74, 0x01,0x10}},/*gain=56.812048(34.968750 db)*/
        {0x23100000, {0x00, 0x02, 0x74, 0x01,0x14}},/*gain=57.430690(35.062500 db)*/
        {0x23400000, {0x00, 0x02, 0x74, 0x01,0x18}},/*gain=58.688259(35.250000 db)*/
        {0x23580000, {0x00, 0x02, 0x74, 0x01,0x1C}},/*gain=59.327332(35.343750 db)*/
        {0x23700000, {0x00, 0x02, 0x74, 0x01,0x20}},/*gain=59.973364(35.437500 db)*/
        {0x23880000, {0x00, 0x02, 0x74, 0x01,0x24}},/*gain=60.626431(35.531250 db)*/
        {0x23B80000, {0x00, 0x02, 0x74, 0x01,0x28}},/*gain=61.953977(35.718750 db)*/
        {0x23D00000, {0x00, 0x02, 0x74, 0x01,0x2C}},/*gain=62.628612(35.812500 db)*/
        {0x23E80000, {0x00, 0x02, 0x74, 0x01,0x30}},/*gain=63.310593(35.906250 db)*/
        {0x24000000, {0x00, 0x00, 0xC6, 0x01,0x00}},/*gain=64.000000(36.000000 db)*/
        {0x24300000, {0x00, 0x00, 0xC6, 0x01,0x04}},/*gain=65.401418(36.187500 db)*/
        {0x24480000, {0x00, 0x00, 0xC6, 0x01,0x08}},/*gain=66.113592(36.281250 db)*/
        {0x24780000, {0x00, 0x00, 0xC6, 0x01,0x0C}},/*gain=67.561291(36.468750 db)*/
        {0x24900000, {0x00, 0x00, 0xC6, 0x01,0x10}},/*gain=68.296986(36.562500 db)*/
        {0x24A80000, {0x00, 0x00, 0xC6, 0x01,0x14}},/*gain=69.040691(36.656250 db)*/
        {0x24D80000, {0x00, 0x00, 0xC6, 0x01,0x18}},/*gain=70.552485(36.843750 db)*/
        {0x24F00000, {0x00, 0x00, 0xC6, 0x01,0x1C}},/*gain=71.320752(36.937500 db)*/
        {0x25080000, {0x00, 0x00, 0xC6, 0x01,0x20}},/*gain=72.097384(37.031250 db)*/
        {0x25380000, {0x00, 0x00, 0xC6, 0x01,0x24}},/*gain=73.676111(37.218750 db)*/
        {0x25500000, {0x00, 0x00, 0xC6, 0x01,0x28}},/*gain=74.478391(37.312500 db)*/
        {0x25680000, {0x00, 0x00, 0xC6, 0x01,0x2C}},/*gain=75.289407(37.406250 db)*/
        {0x25800000, {0x00, 0x00, 0xDC, 0x01,0x00}},/*gain=76.109255(37.500000 db)*/
        {0x25B00000, {0x00, 0x00, 0xDC, 0x01,0x04}},/*gain=77.775831(37.687500 db)*/
        {0x25C80000, {0x00, 0x00, 0xDC, 0x01,0x08}},/*gain=78.622754(37.781250 db)*/
        {0x25E00000, {0x00, 0x00, 0xDC, 0x01,0x0C}},/*gain=79.478900(37.875000 db)*/
        {0x26100000, {0x00, 0x00, 0xDC, 0x01,0x10}},/*gain=81.219261(38.062500 db)*/
        {0x26280000, {0x00, 0x00, 0xDC, 0x01,0x14}},/*gain=82.103681(38.156250 db)*/
        {0x26400000, {0x00, 0x00, 0xDC, 0x01,0x18}},/*gain=82.997731(38.250000 db)*/
        {0x26700000, {0x00, 0x00, 0xDC, 0x01,0x1C}},/*gain=84.815145(38.437500 db)*/
        {0x26880000, {0x00, 0x00, 0xDC, 0x01,0x20}},/*gain=85.738722(38.531250 db)*/
        {0x26A00000, {0x00, 0x00, 0xDC, 0x01,0x24}},/*gain=86.672355(38.625000 db)*/
        {0x26D00000, {0x00, 0x00, 0xDC, 0x01,0x28}},/*gain=88.570232(38.812500 db)*/
        {0x26E80000, {0x00, 0x00, 0xDC, 0x01,0x2C}},/*gain=89.534699(38.906250 db)*/
        {0x27000000, {0x00, 0x02, 0x85, 0x01,0x00}},/*gain=90.509668(39.000000 db)*/
        {0x27180000, {0x00, 0x02, 0x85, 0x01,0x04}},/*gain=91.495254(39.093750 db)*/
        {0x27300000, {0x00, 0x02, 0x85, 0x01,0x08}},/*gain=92.491572(39.187500 db)*/
        {0x27600000, {0x00, 0x02, 0x85, 0x01,0x0C}},/*gain=94.516873(39.375000 db)*/
        {0x27780000, {0x00, 0x02, 0x85, 0x01,0x10}},/*gain=95.546095(39.468750 db)*/
        {0x27A80000, {0x00, 0x02, 0x85, 0x01,0x14}},/*gain=97.638282(39.656250 db)*/
        {0x27C00000, {0x00, 0x02, 0x85, 0x01,0x18}},/*gain=98.701493(39.750000 db)*/
        {0x27D80000, {0x00, 0x02, 0x85, 0x01,0x1C}},/*gain=99.776282(39.843750 db)*/
        {0x27F00000, {0x00, 0x02, 0x85, 0x01,0x20}},/*gain=100.862774(39.937500 db)*/
        {0x28200000, {0x00, 0x02, 0x85, 0x01,0x24}},/*gain=103.071381(40.125000 db)*/
        {0x28380000, {0x00, 0x02, 0x85, 0x01,0x28}},/*gain=104.193755(40.218750 db)*/
        {0x28500000, {0x00, 0x02, 0x85, 0x01,0x2C}},/*gain=105.328351(40.312500 db)*/
        {0x28680000, {0x00, 0x03, 0x36, 0x01,0x00}},/*gain=106.475301(40.406250 db)*/
        {0x28980000, {0x00, 0x03, 0x36, 0x01,0x04}},/*gain=108.806807(40.593750 db)*/
        {0x28B00000, {0x00, 0x03, 0x36, 0x01,0x08}},/*gain=109.991635(40.687500 db)*/
        {0x28C80000, {0x00, 0x03, 0x36, 0x01,0x0C}},/*gain=111.189365(40.781250 db)*/
        {0x28F80000, {0x00, 0x03, 0x36, 0x01,0x10}},/*gain=113.624096(40.968750 db)*/
        {0x29100000, {0x00, 0x03, 0x36, 0x01,0x14}},/*gain=114.861381(41.062500 db)*/
        {0x29400000, {0x00, 0x03, 0x36, 0x01,0x18}},/*gain=117.376518(41.250000 db)*/
        {0x29580000, {0x00, 0x03, 0x36, 0x01,0x1C}},/*gain=118.654664(41.343750 db)*/
        {0x29700000, {0x00, 0x03, 0x36, 0x01,0x20}},/*gain=119.946729(41.437500 db)*/
        {0x29880000, {0x00, 0x03, 0x36, 0x01,0x24}},/*gain=121.252863(41.531250 db)*/
        {0x29B80000, {0x00, 0x03, 0x36, 0x01,0x28}},/*gain=123.907955(41.718750 db)*/
        {0x29D00000, {0x00, 0x03, 0x36, 0x01,0x2C}},/*gain=125.257224(41.812500 db)*/
        {0x29E80000, {0x00, 0x03, 0x36, 0x01,0x30}},/*gain=126.621186(41.906250 db)*/
        {0x2A000000, {0x00, 0x00, 0xCE, 0x01,0x00}},/*gain=128.000000(42.000000 db)*/
        {0x2A300000, {0x00, 0x00, 0xCE, 0x01,0x04}},/*gain=130.802835(42.187500 db)*/
        {0x2A480000, {0x00, 0x00, 0xCE, 0x01,0x08}},/*gain=132.227185(42.281250 db)*/
        {0x2A780000, {0x00, 0x00, 0xCE, 0x01,0x0C}},/*gain=135.122583(42.468750 db)*/
        {0x2A900000, {0x00, 0x00, 0xCE, 0x01,0x10}},/*gain=136.593971(42.562500 db)*/
        {0x2AA80000, {0x00, 0x00, 0xCE, 0x01,0x14}},/*gain=138.081382(42.656250 db)*/
        {0x2AD80000, {0x00, 0x00, 0xCE, 0x01,0x18}},/*gain=141.104971(42.843750 db)*/
        {0x2AF00000, {0x00, 0x00, 0xCE, 0x01,0x1C}},/*gain=142.641503(42.937500 db)*/
        {0x2B080000, {0x00, 0x00, 0xCE, 0x01,0x20}},/*gain=144.194767(43.031250 db)*/
        {0x2B380000, {0x00, 0x00, 0xCE, 0x01,0x24}},/*gain=147.352221(43.218750 db)*/
        {0x2B500000, {0x00, 0x00, 0xCE, 0x01,0x28}},/*gain=148.956782(43.312500 db)*/
        {0x2B680000, {0x00, 0x00, 0xCE, 0x01,0x2C}},/*gain=150.578815(43.406250 db)*/
        {0x2B800000, {0x00, 0x00, 0x7C, 0x01,0x00}},/*gain=152.218511(43.500000 db)*/
        {0x2BB00000, {0x00, 0x00, 0x7C, 0x01,0x04}},/*gain=155.551662(43.687500 db)*/
        {0x2BC80000, {0x00, 0x00, 0x7C, 0x01,0x08}},/*gain=157.245509(43.781250 db)*/
        {0x2BE00000, {0x00, 0x00, 0x7C, 0x01,0x0C}},/*gain=158.957800(43.875000 db)*/
        {0x2C100000, {0x00, 0x00, 0x7C, 0x01,0x10}},/*gain=162.438523(44.062500 db)*/
        {0x2C280000, {0x00, 0x00, 0x7C, 0x01,0x14}},/*gain=164.207362(44.156250 db)*/
        {0x2C400000, {0x00, 0x00, 0x7C, 0x01,0x18}},/*gain=165.995463(44.250000 db)*/
        {0x2C700000, {0x00, 0x00, 0x7C, 0x01,0x1C}},/*gain=169.630290(44.437500 db)*/
        {0x2C880000, {0x00, 0x00, 0x7C, 0x01,0x20}},/*gain=171.477443(44.531250 db)*/
        {0x2CA00000, {0x00, 0x00, 0x7C, 0x01,0x24}},/*gain=173.344710(44.625000 db)*/
        {0x2CD00000, {0x00, 0x00, 0x7C, 0x01,0x28}},/*gain=177.140465(44.812500 db)*/
        {0x2CE80000, {0x00, 0x00, 0x7C, 0x01,0x2C}},/*gain=179.069398(44.906250 db)*/
        {0x2D000000, {0x00, 0x00, 0x75, 0x01,0x00}},/*gain=181.019336(45.000000 db)*/
        {0x2D180000, {0x00, 0x00, 0x75, 0x01,0x04}},/*gain=182.990507(45.093750 db)*/
        {0x2D300000, {0x00, 0x00, 0x75, 0x01,0x08}},/*gain=184.983143(45.187500 db)*/
        {0x2D600000, {0x00, 0x00, 0x75, 0x01,0x0C}},/*gain=189.033747(45.375000 db)*/
        {0x2D780000, {0x00, 0x00, 0x75, 0x01,0x10}},/*gain=191.092189(45.468750 db)*/
        {0x2DA80000, {0x00, 0x00, 0x75, 0x01,0x14}},/*gain=195.276563(45.656250 db)*/
        {0x2DC00000, {0x00, 0x00, 0x75, 0x01,0x18}},/*gain=197.402986(45.750000 db)*/
        {0x2DD80000, {0x00, 0x00, 0x75, 0x01,0x1C}},/*gain=199.552563(45.843750 db)*/
        {0x2DF00000, {0x00, 0x00, 0x75, 0x01,0x20}},/*gain=201.725548(45.937500 db)*/
        {0x2E200000, {0x00, 0x00, 0x75, 0x01,0x24}},/*gain=206.142762(46.125000 db)*/
        {0x2E380000, {0x00, 0x00, 0x75, 0x01,0x28}},/*gain=208.387510(46.218750 db)*/
        {0x2E500000, {0x00, 0x00, 0x75, 0x01,0x2C}},/*gain=210.656701(46.312500 db)*/
        {0x2E680000, {0x00, 0x00, 0x7D, 0x01,0x00}},/*gain=212.950602(46.406250 db)*/
        {0x2E980000, {0x00, 0x00, 0x7D, 0x01,0x04}},/*gain=217.613613(46.593750 db)*/
        {0x2EB00000, {0x00, 0x00, 0x7D, 0x01,0x08}},/*gain=219.983270(46.687500 db)*/
        {0x2EC80000, {0x00, 0x00, 0x7D, 0x01,0x0C}},/*gain=222.378731(46.781250 db)*/
        {0x2EF80000, {0x00, 0x00, 0x7D, 0x01,0x10}},/*gain=227.248191(46.968750 db)*/
        {0x2F100000, {0x00, 0x00, 0x7D, 0x01,0x14}},/*gain=229.722762(47.062500 db)*/
        {0x2F400000, {0x00, 0x00, 0x7D, 0x01,0x18}},/*gain=234.753035(47.250000 db)*/
        {0x2F580000, {0x00, 0x00, 0x7D, 0x01,0x1C}},/*gain=237.309328(47.343750 db)*/
        {0x2F700000, {0x00, 0x00, 0x7D, 0x01,0x20}},/*gain=239.893457(47.437500 db)*/
        {0x2F880000, {0x00, 0x00, 0x7D, 0x01,0x24}},/*gain=242.505726(47.531250 db)*/
        {0x2FB80000, {0x00, 0x00, 0x7D, 0x01,0x28}},/*gain=247.815910(47.718750 db)*/
        {0x2FD00000, {0x00, 0x00, 0x7D, 0x01,0x2C}},/*gain=250.514448(47.812500 db)*/
        {0x2FE80000, {0x00, 0x00, 0x7D, 0x01,0x30}},/*gain=253.242371(47.906250 db)*/
        {0x30000000, {0x00, 0x00, 0xF5, 0x01,0x00}},/*gain=256.000000(48.000000 db)*/ 
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
