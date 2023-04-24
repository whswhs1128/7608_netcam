/*!
*****************************************************************************
** \file        gc0328_reg_tbl.c
**
** \version     $Id: gc0328_reg_tbl.c 10611 2017-1-18 09:26:10Z hehuali $
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
GADI_VI_SensorDrvInfoT    gc0328_sensor =
{
    .magic_start        = GADI_SENSOR_HW_INFO_MAGIC_START,
    .version            = GADI_SENSOR_HW_INFO_VERSION,
    .length             = sizeof(GADI_VI_SensorDrvInfoT),
    .file_date          = 0x20170118,
    .HwInfo         =
    {
        .name               = "gc0328",
        .ctl_type           = GADI_SENSOR_CTL_I2C,
        .addr_len           = sizeof(GADI_U8),
        .data_len           = sizeof(GADI_U8),
        .reset_active_level = GADI_GPIO_LOW,
        .hw_addr            = {(0x42>>1), 0, 0, 0},
        .id_reg =
        {
            {0xF0, 0x9D},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        .reset_reg =
        {
            {0xFE, 0x80, 0x80},
            {GADI_VI_SENSOR_TABLE_FLAG_MSLEEP, 10},    // msleep(10);
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        .standby_reg =
        {
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
    },
    .debug_print        = 1,
    .input_format       = GADI_VI_INPUT_FORMAT_RGB_RAW,
    .field_format       = 1,
    .adapter_id         = 0,
    .sensor_id          = GADI_ISP_SENSOR_GC0328,
    .sensor_double_step = 16,
    .typeofsensor       = 1,
    .sync_time_delay    = 0,
    .source_type.dummy  = GADI_VI_CMOS_CHANNEL_TYPE_AUTO,
    .dev_type           = GADI_VI_DEV_TYPE_CMOS,
    .video_system       = GADI_VIDEO_SYSTEM_AUTO,
    .vs_polarity        = GADI_VI_RISING_EDGE,
    .hs_polarity        = GADI_VI_RISING_EDGE,
    .data_edge          = GADI_VI_RISING_EDGE,
    .emb_sync_switch    = GADI_VI_EMB_SYNC_OFF,
    .emb_sync_loc       = GADI_VI_EMB_SYNC_LOWER_PEL,
    .emb_sync_mode      = GADI_VI_EMB_SYNC_ITU_656,
    .max_width          = 640,
    .max_height         = 480,
    .def_sh_time        = GADI_VIDEO_FPS(30),
    .fmt_num            = 3,
    .auto_fmt           = GADI_VIDEO_MODE(640,  480,  25, 1),
    .init_reg           =
    {
        {0xFE, 0x80},
        {0xFC, 0x16},
        {0xF1, 0x00},
        {0xF2, 0x00},
        
        {0xFE, 0x00},
        {0x4F, 0x00},  //AEC disable.
        {0x03, 0x00},  //eps_time_h[3:0]
        {0x04, 0xC0},  //eps_time_l[7:0]
        {0x42, 0x00},  //AAAA disable.
        {0x77, 0x5A},  //AWB red gain
        {0x78, 0x40},  //AWB green gain
        {0x79, 0x56},  //AWB blue gain
    
        {0x0D, 0x01},  //0x1e8=488
        {0x0E, 0xE8},
        {0x0F, 0x02},  //0x288=648
        {0x10, 0x88},
        {0x09, 0x00},  //row start
        {0x0A, 0x00},
        {0x0B, 0x00},  //col start
        {0x0C, 0x00},
        {0x16, 0x00},  //[7]again disable.
        {0x17, 0x14},  //[1]flip  [0]mirror.  //reserver bits.
        {0x18, 0x0E},
        {0x19, 0x06},
    
        {0x1B, 0x48},
        {0x1F, 0xC8},
        {0x20, 0x01},
        {0x21, 0x78},
        {0x22, 0xB0},
        {0x23, 0x04},  //0x06 20140519 lanking C
        {0x24, 0x11},  //pad driver.
        {0x26, 0x00},
        {0x50, 0x01},  //crop mode

        //////////// BLK//////////////////////
        {0xFE, 0x00},
        {0x27, 0xB7},
        {0x28, 0x7F},
        {0x29, 0x20},
        {0x33, 0x20},
        {0x34, 0x20},
        {0x35, 0x20},
        {0x36, 0x20},
        {0x32, 0x08},
        {0x3B, 0x00},
        {0x3C, 0x00},
        {0x3D, 0x00},
        {0x3E, 0x00},
        {0x47, 0x00},
        {0x48, 0x00},
        //////////// block enable/////////////
        {0x40, 0x7E},
        {0x41, 0x26},
        {0x44, 0x19},  //LSC_out_mode
        {0x45, 0x00},
        {0x46, 0x02},
        {0x4B, 0x01},
        {0x50, 0x01},  //crop window mode enable
        
        /////////gain init////////
        {0x70, 0x40},  //global_gain: 1x
        {0x71, 0x40},  //pre_gain: 1x
        {0x72, 0x40},  //post_gain: 1x
        
        /////////output enable////////
        {0xFE, 0x00},
        {0xF1, 0x07},
        {0xF2, 0x01},
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .pll_table =
    {
        [0] =  //for 320x240@50/60fps   640x480@25/30fps
        {
            .pixclk = 24000000,
            .extclk = 24000000,
            .regs =
            {
                {0xFA, 0x00},   //clk_div
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
        [1] =  //for 320x240@25/30fps
        {
            .pixclk = 12000000,
            .extclk = 24000000,
            .regs =
            {
                {0xFA, 0x11},   //clk_div
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
    },
    .video_fmt_table =
    {
        [0] = //320x240@50fps   //750*320*50=12000000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(320,  240,  50, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },            
            .fmt_reg =
            {
                {0xFE, 0x00}, // SYS
                                 
                {0x05, 0x01}, // HB  
                {0x06, 0x68}, // 
                {0x07, 0x00}, // VB  
                {0x08, 0x48}, // 
                                 
                {0x0F, 0x01}, // HWIN_H
                {0x10, 0x48}, // HWIN_L  //0x148=328
                {0x0D, 0x00}, // VWIN_H
                {0x0E, 0xF8}, // VWIN_L  //0xF8=248
                {0x12, 0x02}, // VS_ST
                {0x13, 0x02}, // VS_ET
                {0x11, 0x3A}, // sh_delay 
                //row_time = HB + sh_delay + win_width + 4 = 0x168+0x3a+328+4=750
                //fram_tiem = VB + Vt + 8 = VB + (win_height - 8) + 8 = VB + win_height = 0x48+248=320
                //750*320*50 = 12M

                {0xFE, 0x01},
                
                {0x29, 0x00}, // anti-flicker step [11:8]
                {0x2A, 0x96}, // anti-flicker step [7:0]
                {0x2B, 0x01}, // exp level 0  50fps
                {0x2C, 0x00}, //
                {0x2D, 0x01}, //
                {0x2E, 0x00}, //
                {0x2F, 0x01}, //
                {0x30, 0x00}, //
                {0x31, 0x01}, //
                {0x32, 0x00}, //
                //{0x06, 0x01}, // Window setting for AEC & AWB
                //{0x07, 0x01},
                //{0x08, 0x01},
                //{0x09, 0x01},
                {0x13, 0x60}, // expected luminance value
                
                {0xFE, 0x00},
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 320,
                .height     = 240,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW, 
                .interface  = GADI_VIDEO_IN_DVP, 
                .bits       = GADI_VIDEO_BITS_8,
                .ratio      = GADI_VIDEO_RATIO_4_3,
                .srm        = 0,
                .pll_index  = 0,
                .max_fps    = GADI_VIDEO_FPS(50),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(50),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN,
            },
            .video_info_table =
            {
                .def_start_x    = 0,
                .def_start_y    = 0,
                .def_width      = 320,
                .def_height     = 240,
                .sync_start     = (0 + 0),
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_BG,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_GR,
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_RG,
                },
            },
            .update_after_vsync_start   = 18,
            .update_after_vsync_end     = 20,
        },        
        [1] = //320x240@25fps   //750*320*25=6000000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(320,  240,  25, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },            
            .fmt_reg =
            {
                {0xFE, 0x00}, // SYS
                
                {0x05, 0x01}, // HB
                {0x06, 0x68}, //
                {0x07, 0x00}, // VB
                {0x08, 0x48}, //
                
                {0x0F, 0x01}, // HWIN_H
                {0x10, 0x48}, // HWIN_L  //0x148=328
                {0x0D, 0x00}, // VWIN_H
                {0x0E, 0xF8}, // VWIN_L  //0xF8=248
                {0x12, 0x02}, // VS_ST
                {0x13, 0x02}, // VS_ET
                {0x11, 0x3A}, // sh_delay 
                //row_time = HB + sh_delay + win_width + 4 = 0x168+0x3a+328+4=750
                //fram_tiem = VB + Vt + 8 = VB + (win_height - 8) + 8 = VB + win_height = 0x48+248=320
                //750*320*50 = 12M

                {0xFE, 0x01},
                
                {0x29, 0x00}, // anti-flicker step [11:8]
                {0x2A, 0x96}, // anti-flicker step [7:0]
                {0x2B, 0x02}, // exp level 0  25fps
                {0x2C, 0x00}, //
                {0x2D, 0x02}, //
                {0x2E, 0x00}, //
                {0x2F, 0x02}, //
                {0x30, 0x00}, //
                {0x31, 0x02}, //
                {0x32, 0x00}, //
                //{0x06, 0x01}, // Window setting for AEC & AWB
                //{0x07, 0x01},
                //{0x08, 0x01},
                //{0x09, 0x01},
                {0x13, 0x60}, // expected luminance value
                
                {0xFE, 0x00},
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 320,
                .height     = 240,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW, 
                .interface  = GADI_VIDEO_IN_DVP, 
                .bits       = GADI_VIDEO_BITS_8,
                .ratio      = GADI_VIDEO_RATIO_4_3,
                .srm        = 0,
                .pll_index  = 1,
                .max_fps    = GADI_VIDEO_FPS(25),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(25),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN,
            },
            .video_info_table =
            {
                .def_start_x    = 0,
                .def_start_y    = 0,
                .def_width      = 320,
                .def_height     = 240,
                .sync_start     = (0 + 0),
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_BG,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_GR,
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_RG,
                },
            },
            .update_after_vsync_start   = 37,
            .update_after_vsync_end     = 40,
        },
        
        [2] = //640x480@25fps   //800*600*25=12000000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(640,  480,  25, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },            
            .fmt_reg =
            {
                {0xFE, 0x00}, // SYS
                {0x05, 0x00}, // hb  //[3:0]:HBLANK high bit [11:8]
                {0x06, 0x74}, //     HBLANK low bit [7:0]
                {0x07, 0x00}, // vb  //[3:0]:VBLANK high bit[11:8]
                {0x08, 0x70}, // VBLANK low bit[7:0]
                
                {0x0F, 0x02}, // HWIN_H    //0x288=648
                {0x10, 0x88}, // HWIN_L
                {0x0D, 0x01}, // VWIN_H    //0x1e8=488
                {0x0E, 0xE8}, // VWIN_L
                {0x12, 0x02}, // VS_ST
                {0x13, 0x02}, // VS_ET
                {0x11, 0x20}, // sh_delay
                //row_time = HB + sh_delay + win_width + 4 = 0x74+0x20+648+4=800
                //fram_tiem = VB + Vt + 8 = VB + (win_height - 8) + 8 = VB + win_height = 0x70+488=600
                //800*600*30 = 12M

                {0xFE, 0x01},
                
                {0x29, 0x00}, // anti-flicker step [11:8]
                {0x2A, 0xA1}, // anti-flicker step [7:0]
                {0x2B, 0x02}, // exp level 0  25fps
                {0x2C, 0x50},
                {0x2D, 0x02},
                {0x2E, 0x50},
                {0x2F, 0x05},
                {0x30, 0x50},
                {0x31, 0x02},
                {0x32, 0x50},
                
                {0x06, 0x08},
                {0x07, 0x06},
                {0x08, 0xA0},
                {0x09, 0xE8},
                
                {0x13, 0x45}, // expected luminance value
                
                {0xFE, 0x00},
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 640,
                .height     = 480,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_DVP, 
                .bits       = GADI_VIDEO_BITS_8,
                .ratio      = GADI_VIDEO_RATIO_4_3,
                .srm        = 0,
                .pll_index  = 0,
                .max_fps    = GADI_VIDEO_FPS(25),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(25),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_VGA_25,
            },
            .video_info_table =
            {
                .def_start_x    = 0,
                .def_start_y    = 0,
                .def_width      = 640,
                .def_height     = 480,
                .sync_start     = (0 + 0),
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_BG,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_GR,
                    GADI_VI_BAYER_PATTERN_GB,
                    GADI_VI_BAYER_PATTERN_RG,
                },
            },
            .update_after_vsync_start   = 37,
            .update_after_vsync_end     = 40,
        },
    },

    .mirror_table =
    {
        #define GC0328_V_FLIP       (1<<1)
        #define GC0328_H_MIRROR     (1<<0)
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY_VERTICALLY] =
        {
            {0xFE, 0x00},
            {0x17, GC0328_H_MIRROR | GC0328_V_FLIP, GC0328_H_MIRROR | GC0328_V_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY] =
        {
            {0xFE, 0x00},
            {0x17, GC0328_H_MIRROR, GC0328_H_MIRROR | GC0328_V_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_VERTICALLY] =
        {
            {0xFE, 0x00},
            {0x17, GC0328_V_FLIP, GC0328_H_MIRROR | GC0328_V_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_NONE] =
        {
            {0xFE, 0x00},
            {0x17, 0x00, GC0328_H_MIRROR | GC0328_V_FLIP},
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
    .agc_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_SET,
    .vmax_reg_update_mode       = GADI_SENSOR_UPDATE_REG_BY_SET,
    .mirror_reg_update_mode     = GADI_SENSOR_UPDATE_REG_BY_SET,
    .shs_additional_use         = 0,
    .shs_additional_percent     = 0,
    //note: Following registers should be setted in page0.
    // row_time = HB + sh_delay + win_width + 4
    .hmax_reg =
    {
        //{0x05, 0x00, 0xFF, 0x00, 0x08, 0},  // HB_h
        //{0x06, 0x00, 0xFF, 0x00, 0x00, 0},  // HB_l
        //{0x11, 0x00, 0xFF, 0x00, 0x00, 0},  // sh_delay
        //{0x0F, 0x00, 0xFF, 0x00, 0x08, 0},  // win_width_h
        //{0x10, 0x00, 0xFF, 0x00, 0x00, 0},  // win_width_l
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    // fram_tiem = VB + Vt + 8 = VB + (win_height - 8) + 8 = VB + win_height
    .vmax_reg =
    {
        //{0x07, 0x00, 0xFF, 0x00, 0x08, 0},  // VB_h
        //{0x08, 0x00, 0xFF, 0x00, 0x00, 0},  // VB_l
        //{0x0D, 0x00, 0xFF, 0x00, 0x08, 0},  // win_height_h
        //{0x0E, 0x00, 0xFF, 0x00, 0x00, 0},  // win_height_l
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .vmax_wadd_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    //note: Following registers should be setted in page0.
    // shs = exposure[11:8]<<8 + Exposure[7:0]
    .shs_reg =
    {  
        //first, value of reg"0x4f" is 0x0;
        {0x03, 0x00, 0x0F, 0x00, 0x08, 0}, 
        {0x04, 0x00, 0xFF, 0x00, 0x00, 0}, 
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .shs_additional_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    .cal_gain_mode              = GADI_SENSOR_CAL_GAIN_MODE0,
    .max_agc_index = 145,
    .max_again_index = 0,
    .cur_again_index = 0,
    .agc_info =
    {
        .db_max     = 0x24000000,    // 36dB   //smaller than 36: 35.9db
        .db_min     = 0x00000000,    // 0dB
        .db_step    = 0x00865000,    // use
    },
    //note: Following registers should be setted in page0.    
    .gain_reg =
    {
        {0xfe, 0x00},   //page set.
        {0x70, 0x00},   //global_gain
        {0x71, 0x00},   //pre_gain
        {0x72, 0x00},   //post_gain
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .gain_table =
    {
        /*total_gain  page  global_gain  pre_gain   post_gain                                                  total_gain     total_gain */   
        /*            0xfe  [P0]0x70     [P0]0x71   [P0]0x72                                                      multi           db    */
        {0x00000000, {0x00, 0x40, 0x40, 0x40,/*global_gain 1.0000; pre_gain 1.0000; post_gain 1.0000; */ }},  //1.0000	   //0.000000 
        {0x008657c9, {0x00, 0x40, 0x40, 0x44,/*global_gain 1.0000; pre_gain 1.0000; post_gain 1.0625; */ }},  //1.0625	   //0.524777 
        {0x0105013a, {0x00, 0x40, 0x40, 0x48,/*global_gain 1.0000; pre_gain 1.0000; post_gain 1.1250; */ }},  //1.1250	   //1.019550 
        {0x017cd110, {0x00, 0x40, 0x40, 0x4c,/*global_gain 1.0000; pre_gain 1.0000; post_gain 1.1875; */ }},  //1.1875	   //1.487565 
        {0x01ee7b47, {0x00, 0x40, 0x40, 0x50,/*global_gain 1.0000; pre_gain 1.0000; post_gain 1.2500; */ }},  //1.2500	   //1.931569 
        {0x025a997c, {0x00, 0x40, 0x40, 0x54,/*global_gain 1.0000; pre_gain 1.0000; post_gain 1.3125; */ }},  //1.3125	   //2.353905 
        {0x02c1afdd, {0x00, 0x40, 0x40, 0x58,/*global_gain 1.0000; pre_gain 1.0000; post_gain 1.3750; */ }},  //1.3750	   //2.756590 
        {0x032430f0, {0x00, 0x40, 0x40, 0x5c,/*global_gain 1.0000; pre_gain 1.0000; post_gain 1.4375; */ }},  //1.4375	   //3.141372 
        {0x0382809d, {0x00, 0x40, 0x40, 0x60,/*global_gain 1.0000; pre_gain 1.0000; post_gain 1.5000; */ }},  //1.5000	   //3.509775 
        {0x03dcf68e, {0x00, 0x40, 0x40, 0x64,/*global_gain 1.0000; pre_gain 1.0000; post_gain 1.5625; */ }},  //1.5625	   //3.863137 
        {0x0433e01a, {0x00, 0x40, 0x40, 0x68,/*global_gain 1.0000; pre_gain 1.0000; post_gain 1.6250; */ }},  //1.6250	   //4.202638 
        {0x048781d8, {0x00, 0x40, 0x40, 0x6c,/*global_gain 1.0000; pre_gain 1.0000; post_gain 1.6875; */ }},  //1.6875	   //4.529325 
        {0x04d818df, {0x00, 0x40, 0x40, 0x70,/*global_gain 1.0000; pre_gain 1.0000; post_gain 1.7500; */ }},  //1.7500	   //4.844130 
        {0x0525dbda, {0x00, 0x40, 0x40, 0x74,/*global_gain 1.0000; pre_gain 1.0000; post_gain 1.8125; */ }},  //1.8125	   //5.147886 
        {0x0570fbe4, {0x00, 0x40, 0x40, 0x78,/*global_gain 1.0000; pre_gain 1.0000; post_gain 1.8750; */ }},  //1.8750	   //5.441344 
        {0x05b9a541, {0x00, 0x40, 0x40, 0x7c,/*global_gain 1.0000; pre_gain 1.0000; post_gain 1.9375; */ }},  //1.9375	   //5.725178 
        {0x06000000, {0x00, 0x40, 0x40, 0x80,/*global_gain 1.0000; pre_gain 1.0000; post_gain 2.0000; */ }},  //2.0000	   //6.000000 
        {0x0644307a, {0x00, 0x40, 0x40, 0x84,/*global_gain 1.0000; pre_gain 1.0000; post_gain 2.0625; */ }},  //2.0625	   //6.266365 
        {0x068657c9, {0x00, 0x40, 0x40, 0x88,/*global_gain 1.0000; pre_gain 1.0000; post_gain 2.1250; */ }},  //2.1250	   //6.524777 
        {0x06c69426, {0x00, 0x40, 0x40, 0x8c,/*global_gain 1.0000; pre_gain 1.0000; post_gain 2.1875; */ }},  //2.1875	   //6.775698 
        {0x0705013a, {0x00, 0x40, 0x40, 0x90,/*global_gain 1.0000; pre_gain 1.0000; post_gain 2.2500; */ }},  //2.2500	   //7.019550 
        {0x0741b86a, {0x00, 0x40, 0x40, 0x94,/*global_gain 1.0000; pre_gain 1.0000; post_gain 2.3125; */ }},  //2.3125	   //7.256720 
        {0x077cd110, {0x00, 0x40, 0x40, 0x98,/*global_gain 1.0000; pre_gain 1.0000; post_gain 2.3750; */ }},  //2.3750	   //7.487565 
        {0x07b660b8, {0x00, 0x40, 0x40, 0x9c,/*global_gain 1.0000; pre_gain 1.0000; post_gain 2.4375; */ }},  //2.4375	   //7.712413 
        {0x07ee7b47, {0x00, 0x40, 0x40, 0xa0,/*global_gain 1.0000; pre_gain 1.0000; post_gain 2.5000; */ }},  //2.5000	   //7.931569 
        {0x0825332b, {0x00, 0x40, 0x40, 0xa4,/*global_gain 1.0000; pre_gain 1.0000; post_gain 2.5625; */ }},  //2.5625	   //8.145312 
        {0x085a997c, {0x00, 0x40, 0x40, 0xa8,/*global_gain 1.0000; pre_gain 1.0000; post_gain 2.6250; */ }},  //2.6250	   //8.353905 
        {0x088ebe1f, {0x00, 0x40, 0x40, 0xac,/*global_gain 1.0000; pre_gain 1.0000; post_gain 2.6875; */ }},  //2.6875	   //8.557589 
        {0x08c1afdd, {0x00, 0x40, 0x40, 0xb0,/*global_gain 1.0000; pre_gain 1.0000; post_gain 2.7500; */ }},  //2.7500	   //8.756590 
        {0x08f37c81, {0x00, 0x40, 0x40, 0xb4,/*global_gain 1.0000; pre_gain 1.0000; post_gain 2.8125; */ }},  //2.8125	   //8.951119 
        {0x092430f0, {0x00, 0x40, 0x40, 0xb8,/*global_gain 1.0000; pre_gain 1.0000; post_gain 2.8750; */ }},  //2.8750	   //9.141372 
        {0x0953d935, {0x00, 0x40, 0x40, 0xbc,/*global_gain 1.0000; pre_gain 1.0000; post_gain 2.9375; */ }},  //2.9375	   //9.327533 
        {0x0982809d, {0x00, 0x40, 0x40, 0xc0,/*global_gain 1.0000; pre_gain 1.0000; post_gain 3.0000; */ }},  //3.0000	   //9.509775 
        {0x09b031be, {0x00, 0x40, 0x40, 0xc4,/*global_gain 1.0000; pre_gain 1.0000; post_gain 3.0625; */ }},  //3.0625	   //9.688259 
        {0x09dcf68e, {0x00, 0x40, 0x40, 0xc8,/*global_gain 1.0000; pre_gain 1.0000; post_gain 3.1250; */ }},  //3.1250	   //9.863137 
        {0x0a08d867, {0x00, 0x40, 0x40, 0xcc,/*global_gain 1.0000; pre_gain 1.0000; post_gain 3.1875; */ }},  //3.1875	   //10.034552
        {0x0a33e01a, {0x00, 0x40, 0x40, 0xd0,/*global_gain 1.0000; pre_gain 1.0000; post_gain 3.2500; */ }},  //3.2500	   //10.202638
        {0x0a5e15f8, {0x00, 0x40, 0x40, 0xd4,/*global_gain 1.0000; pre_gain 1.0000; post_gain 3.3125; */ }},  //3.3125	   //10.367523
        {0x0a8781d8, {0x00, 0x40, 0x40, 0xd8,/*global_gain 1.0000; pre_gain 1.0000; post_gain 3.3750; */ }},  //3.3750	   //10.529325
        {0x0ab02b24, {0x00, 0x40, 0x40, 0xdc,/*global_gain 1.0000; pre_gain 1.0000; post_gain 3.4375; */ }},  //3.4375	   //10.688158
        {0x0ad818df, {0x00, 0x40, 0x40, 0xe0,/*global_gain 1.0000; pre_gain 1.0000; post_gain 3.5000; */ }},  //3.5000	   //10.844130
        {0x0aff51ae, {0x00, 0x40, 0x40, 0xe4,/*global_gain 1.0000; pre_gain 1.0000; post_gain 3.5625; */ }},  //3.5625	   //10.997340
        {0x0b25dbda, {0x00, 0x40, 0x40, 0xe8,/*global_gain 1.0000; pre_gain 1.0000; post_gain 3.6250; */ }},  //3.6250	   //11.147886
        {0x0b4bbd5e, {0x00, 0x40, 0x40, 0xec,/*global_gain 1.0000; pre_gain 1.0000; post_gain 3.6875; */ }},  //3.6875	   //11.295858
        {0x0b70fbe4, {0x00, 0x40, 0x40, 0xf0,/*global_gain 1.0000; pre_gain 1.0000; post_gain 3.7500; */ }},  //3.7500	   //11.441344
        {0x0b959cd0, {0x00, 0x40, 0x40, 0xf4,/*global_gain 1.0000; pre_gain 1.0000; post_gain 3.8125; */ }},  //3.8125	   //11.584424
        {0x0bb9a541, {0x00, 0x40, 0x40, 0xf8,/*global_gain 1.0000; pre_gain 1.0000; post_gain 3.8750; */ }},  //3.8750	   //11.725178
        {0x0bdd1a1a, {0x00, 0x40, 0x40, 0xfc,/*global_gain 1.0000; pre_gain 1.0000; post_gain 3.9375; */ }},  //3.9375	   //11.863680	 
        {0x0bf7573d, {0x00, 0x40, 0x40, 0xff,/*global_gain 1.0000; pre_gain 1.0000; post_gain 3.9844; */ }},  //3.984400,  //11.966175
        {0x0c7daf07, {0x00, 0x40, 0x44, 0xff,/*global_gain 1.0000; pre_gain 1.0625; post_gain 3.9844; */ }},  //4.233425,  //12.490952
        {0x0cfc5878, {0x00, 0x40, 0x48, 0xff,/*global_gain 1.0000; pre_gain 1.1250; post_gain 3.9844; */ }},  //4.482450,  //12.985725
        {0x0d74284e, {0x00, 0x40, 0x4c, 0xff,/*global_gain 1.0000; pre_gain 1.1875; post_gain 3.9844; */ }},  //4.731475,  //13.453740
        {0x0de5d284, {0x00, 0x40, 0x50, 0xff,/*global_gain 1.0000; pre_gain 1.2500; post_gain 3.9844; */ }},  //4.980500,  //13.897744
        {0x0e51f0ba, {0x00, 0x40, 0x54, 0xff,/*global_gain 1.0000; pre_gain 1.3125; post_gain 3.9844; */ }},  //5.229525,  //14.320079
        {0x0eb9071a, {0x00, 0x40, 0x58, 0xff,/*global_gain 1.0000; pre_gain 1.3750; post_gain 3.9844; */ }},  //5.478550,  //14.722765
        {0x0f1b882d, {0x00, 0x40, 0x5c, 0xff,/*global_gain 1.0000; pre_gain 1.4375; post_gain 3.9844; */ }},  //5.727575,  //15.107547
        {0x0f79d7da, {0x00, 0x40, 0x60, 0xff,/*global_gain 1.0000; pre_gain 1.5000; post_gain 3.9844; */ }},  //5.976600,  //15.475950
        {0x0fd44dcb, {0x00, 0x40, 0x64, 0xff,/*global_gain 1.0000; pre_gain 1.5625; post_gain 3.9844; */ }},  //6.225625,  //15.829312
        {0x102b3758, {0x00, 0x40, 0x68, 0xff,/*global_gain 1.0000; pre_gain 1.6250; post_gain 3.9844; */ }},  //6.474650,  //16.168813
        {0x107ed915, {0x00, 0x40, 0x6c, 0xff,/*global_gain 1.0000; pre_gain 1.6875; post_gain 3.9844; */ }},  //6.723675,  //16.495500
        {0x10cf701d, {0x00, 0x40, 0x70, 0xff,/*global_gain 1.0000; pre_gain 1.7500; post_gain 3.9844; */ }},  //6.972700,  //16.810304
        {0x111d3318, {0x00, 0x40, 0x74, 0xff,/*global_gain 1.0000; pre_gain 1.8125; post_gain 3.9844; */ }},  //7.221725,  //17.114061
        {0x11685322, {0x00, 0x40, 0x78, 0xff,/*global_gain 1.0000; pre_gain 1.8750; post_gain 3.9844; */ }},  //7.470750,  //17.407519
        {0x11b0fc7f, {0x00, 0x40, 0x7c, 0xff,/*global_gain 1.0000; pre_gain 1.9375; post_gain 3.9844; */ }},  //7.719775,  //17.691353
        {0x11f7573d, {0x00, 0x40, 0x80, 0xff,/*global_gain 1.0000; pre_gain 2.0000; post_gain 3.9844; */ }},  //7.968800,  //17.966175
        {0x123b87b7, {0x00, 0x40, 0x84, 0xff,/*global_gain 1.0000; pre_gain 2.0625; post_gain 3.9844; */ }},  //8.217825,  //18.232540
        {0x127daf07, {0x00, 0x40, 0x88, 0xff,/*global_gain 1.0000; pre_gain 2.1250; post_gain 3.9844; */ }},  //8.466850,  //18.490952
        {0x12bdeb64, {0x00, 0x40, 0x8c, 0xff,/*global_gain 1.0000; pre_gain 2.1875; post_gain 3.9844; */ }},  //8.715875,  //18.741873
        {0x12fc5878, {0x00, 0x40, 0x90, 0xff,/*global_gain 1.0000; pre_gain 2.2500; post_gain 3.9844; */ }},  //8.964900,  //18.985725
        {0x13390fa7, {0x00, 0x40, 0x94, 0xff,/*global_gain 1.0000; pre_gain 2.3125; post_gain 3.9844; */ }},  //9.213925,  //19.222895
        {0x1374284e, {0x00, 0x40, 0x98, 0xff,/*global_gain 1.0000; pre_gain 2.3750; post_gain 3.9844; */ }},  //9.462950,  //19.453740
        {0x13adb7f5, {0x00, 0x40, 0x9c, 0xff,/*global_gain 1.0000; pre_gain 2.4375; post_gain 3.9844; */ }},  //9.711975,  //19.678588
        {0x13e5d284, {0x00, 0x40, 0xa0, 0xff,/*global_gain 1.0000; pre_gain 2.5000; post_gain 3.9844; */ }},  //9.961000,  //19.897744
        {0x141c8a68, {0x00, 0x40, 0xa4, 0xff,/*global_gain 1.0000; pre_gain 2.5625; post_gain 3.9844; */ }},  //10.210025, //20.111487
        {0x1451f0ba, {0x00, 0x40, 0xa8, 0xff,/*global_gain 1.0000; pre_gain 2.6250; post_gain 3.9844; */ }},  //10.459050, //20.320079
        {0x1486155c, {0x00, 0x40, 0xac, 0xff,/*global_gain 1.0000; pre_gain 2.6875; post_gain 3.9844; */ }},  //10.708075, //20.523763
        {0x14b9071a, {0x00, 0x40, 0xb0, 0xff,/*global_gain 1.0000; pre_gain 2.7500; post_gain 3.9844; */ }},  //10.957100, //20.722765
        {0x14ead3bf, {0x00, 0x40, 0xb4, 0xff,/*global_gain 1.0000; pre_gain 2.8125; post_gain 3.9844; */ }},  //11.206125, //20.917294
        {0x151b882d, {0x00, 0x40, 0xb8, 0xff,/*global_gain 1.0000; pre_gain 2.8750; post_gain 3.9844; */ }},  //11.455150, //21.107547
        {0x154b3073, {0x00, 0x40, 0xbc, 0xff,/*global_gain 1.0000; pre_gain 2.9375; post_gain 3.9844; */ }},  //11.704175, //21.293708
        {0x1579d7da, {0x00, 0x40, 0xc0, 0xff,/*global_gain 1.0000; pre_gain 3.0000; post_gain 3.9844; */ }},  //11.953200, //21.475950
        {0x15a788fc, {0x00, 0x40, 0xc4, 0xff,/*global_gain 1.0000; pre_gain 3.0625; post_gain 3.9844; */ }},  //12.202225, //21.654434
        {0x15d44dcb, {0x00, 0x40, 0xc8, 0xff,/*global_gain 1.0000; pre_gain 3.1250; post_gain 3.9844; */ }},  //12.451250, //21.829312
        {0x16002fa4, {0x00, 0x40, 0xcc, 0xff,/*global_gain 1.0000; pre_gain 3.1875; post_gain 3.9844; */ }},  //12.700275, //22.000727
        {0x162b3758, {0x00, 0x40, 0xc0, 0xff,/*global_gain 1.0000; pre_gain 3.2500; post_gain 3.9844; */ }},  //12.949300, //22.168813
        {0x16556d35, {0x00, 0x40, 0xc4, 0xff,/*global_gain 1.0000; pre_gain 3.3125; post_gain 3.9844; */ }},  //13.198325, //22.333698
        {0x167ed915, {0x00, 0x40, 0xc8, 0xff,/*global_gain 1.0000; pre_gain 3.3750; post_gain 3.9844; */ }},  //13.447350, //22.495500
        {0x16a78261, {0x00, 0x40, 0xcc, 0xff,/*global_gain 1.0000; pre_gain 3.4375; post_gain 3.9844; */ }},  //13.696375, //22.654333
        {0x16cf701d, {0x00, 0x40, 0xe0, 0xff,/*global_gain 1.0000; pre_gain 3.5000; post_gain 3.9844; */ }},  //13.945400, //22.810304
        {0x16f6a8eb, {0x00, 0x40, 0xe4, 0xff,/*global_gain 1.0000; pre_gain 3.5625; post_gain 3.9844; */ }},  //14.194425, //22.963515
        {0x171d3318, {0x00, 0x40, 0xe8, 0xff,/*global_gain 1.0000; pre_gain 3.6250; post_gain 3.9844; */ }},  //14.443450, //23.114061
        {0x1743149c, {0x00, 0x40, 0xec, 0xff,/*global_gain 1.0000; pre_gain 3.6875; post_gain 3.9844; */ }},  //14.692475, //23.262033
        {0x17685322, {0x00, 0x40, 0xf0, 0xff,/*global_gain 1.0000; pre_gain 3.7500; post_gain 3.9844; */ }},  //14.941500, //23.407519
        {0x178cf40d, {0x00, 0x40, 0xf4, 0xff,/*global_gain 1.0000; pre_gain 3.8125; post_gain 3.9844; */ }},  //15.190525, //23.550599
        {0x17b0fc7f, {0x00, 0x40, 0xf8, 0xff,/*global_gain 1.0000; pre_gain 3.8750; post_gain 3.9844; */ }},  //15.439550, //23.691353
        {0x17d47157, {0x00, 0x40, 0xfc, 0xff,/*global_gain 1.0000; pre_gain 3.9375; post_gain 3.9844; */ }},  //15.688575, //23.829854
        {0x17eeae77, {0x00, 0x40, 0xff, 0xff,/*global_gain 1.0000; pre_gain 3.9844; post_gain 3.9844; */ }},  //15.875443, //23.932350
        {0x18750645, {0x00, 0x44, 0xff, 0xff,/*global_gain 1.0625; pre_gain 3.9844; post_gain 3.9844; */ }},  //16.867659, //24.457127
        {0x18f3afb5, {0x00, 0x48, 0xff, 0xff,/*global_gain 1.1250; pre_gain 3.9844; post_gain 3.9844; */ }},  //17.859874, //24.951900
        {0x196b7f8b, {0x00, 0x4c, 0xff, 0xff,/*global_gain 1.1875; pre_gain 3.9844; post_gain 3.9844; */ }},  //18.852089, //25.419915
        {0x19dd29c2, {0x00, 0x50, 0xff, 0xff,/*global_gain 1.2500; pre_gain 3.9844; post_gain 3.9844; */ }},  //19.844304, //25.863918
        {0x1a4947f7, {0x00, 0x54, 0xff, 0xff,/*global_gain 1.3125; pre_gain 3.9844; post_gain 3.9844; */ }},  //20.836519, //26.286254
        {0x1ab05e58, {0x00, 0x58, 0xff, 0xff,/*global_gain 1.3750; pre_gain 3.9844; post_gain 3.9844; */ }},  //21.828735, //26.688940
        {0x1b12df6b, {0x00, 0x5c, 0xff, 0xff,/*global_gain 1.4375; pre_gain 3.9844; post_gain 3.9844; */ }},  //22.820950, //27.073722
        {0x1b712f18, {0x00, 0x60, 0xff, 0xff,/*global_gain 1.5000; pre_gain 3.9844; post_gain 3.9844; */ }},  //23.813165, //27.442125
        {0x1bcba509, {0x00, 0x64, 0xff, 0xff,/*global_gain 1.5625; pre_gain 3.9844; post_gain 3.9844; */ }},  //24.805380, //27.795487
        {0x1c228e95, {0x00, 0x68, 0xff, 0xff,/*global_gain 1.6250; pre_gain 3.9844; post_gain 3.9844; */ }},  //25.797595, //28.134988
        {0x1c763053, {0x00, 0x6c, 0xff, 0xff,/*global_gain 1.6875; pre_gain 3.9844; post_gain 3.9844; */ }},  //26.789811, //28.461675
        {0x1cc6c75a, {0x00, 0x70, 0xff, 0xff,/*global_gain 1.7500; pre_gain 3.9844; post_gain 3.9844; */ }},  //27.782026, //28.776479
        {0x1d148a56, {0x00, 0x74, 0xff, 0xff,/*global_gain 1.8125; pre_gain 3.9844; post_gain 3.9844; */ }},  //28.774241, //29.080236
        {0x1d5faa5f, {0x00, 0x78, 0xff, 0xff,/*global_gain 1.8750; pre_gain 3.9844; post_gain 3.9844; */ }},  //29.766456, //29.373693
        {0x1da853bc, {0x00, 0x7c, 0xff, 0xff,/*global_gain 1.9375; pre_gain 3.9844; post_gain 3.9844; */ }},  //30.758672, //29.657528
        {0x1deeae7b, {0x00, 0x80, 0xff, 0xff,/*global_gain 2.0000; pre_gain 3.9844; post_gain 3.9844; */ }},  //31.750887, //29.932350
        {0x1e32def5, {0x00, 0x84, 0xff, 0xff,/*global_gain 2.0625; pre_gain 3.9844; post_gain 3.9844; */ }},  //32.743102, //30.198715
        {0x1e750645, {0x00, 0x88, 0xff, 0xff,/*global_gain 2.1250; pre_gain 3.9844; post_gain 3.9844; */ }},  //33.735317, //30.457127
        {0x1eb542a1, {0x00, 0x8c, 0xff, 0xff,/*global_gain 2.1875; pre_gain 3.9844; post_gain 3.9844; */ }},  //34.727532, //30.708048
        {0x1ef3afb5, {0x00, 0x90, 0xff, 0xff,/*global_gain 2.2500; pre_gain 3.9844; post_gain 3.9844; */ }},  //35.719748, //30.951900
        {0x1f3066e5, {0x00, 0x94, 0xff, 0xff,/*global_gain 2.3125; pre_gain 3.9844; post_gain 3.9844; */ }},  //36.711963, //31.189070
        {0x1f6b7f8b, {0x00, 0x98, 0xff, 0xff,/*global_gain 2.3750; pre_gain 3.9844; post_gain 3.9844; */ }},  //37.704178, //31.419915
        {0x1fa50f33, {0x00, 0x9c, 0xff, 0xff,/*global_gain 2.4375; pre_gain 3.9844; post_gain 3.9844; */ }},  //38.696393, //31.644763
        {0x1fdd29c2, {0x00, 0xa0, 0xff, 0xff,/*global_gain 2.5000; pre_gain 3.9844; post_gain 3.9844; */ }},  //39.688608, //31.863918
        {0x2013e1a6, {0x00, 0xa4, 0xff, 0xff,/*global_gain 2.5625; pre_gain 3.9844; post_gain 3.9844; */ }},  //40.680824, //32.077662
        {0x204947f7, {0x00, 0xa8, 0xff, 0xff,/*global_gain 2.6250; pre_gain 3.9844; post_gain 3.9844; */ }},  //41.673039, //32.286254
        {0x207d6c9a, {0x00, 0xac, 0xff, 0xff,/*global_gain 2.6875; pre_gain 3.9844; post_gain 3.9844; */ }},  //42.665254, //32.489938
        {0x20b05e58, {0x00, 0xb0, 0xff, 0xff,/*global_gain 2.7500; pre_gain 3.9844; post_gain 3.9844; */ }},  //43.657469, //32.688940
        {0x20e22afc, {0x00, 0xb4, 0xff, 0xff,/*global_gain 2.8125; pre_gain 3.9844; post_gain 3.9844; */ }},  //44.649684, //32.883468
        {0x2112df6b, {0x00, 0xb8, 0xff, 0xff,/*global_gain 2.8750; pre_gain 3.9844; post_gain 3.9844; */ }},  //45.641900, //33.073722
        {0x214287b0, {0x00, 0xbc, 0xff, 0xff,/*global_gain 2.9375; pre_gain 3.9844; post_gain 3.9844; */ }},  //46.634115, //33.259883
        {0x21712f18, {0x00, 0xc0, 0xff, 0xff,/*global_gain 3.0000; pre_gain 3.9844; post_gain 3.9844; */ }},  //47.626330, //33.442125
        {0x219ee03a, {0x00, 0xc4, 0xff, 0xff,/*global_gain 3.0625; pre_gain 3.9844; post_gain 3.9844; */ }},  //48.618545, //33.620609
        {0x21cba509, {0x00, 0xc8, 0xff, 0xff,/*global_gain 3.1250; pre_gain 3.9844; post_gain 3.9844; */ }},  //49.610760, //33.795487
        {0x21f786e2, {0x00, 0xcc, 0xff, 0xff,/*global_gain 3.1875; pre_gain 3.9844; post_gain 3.9844; */ }},  //50.602976, //33.966902
        {0x22228e95, {0x00, 0xd0, 0xff, 0xff,/*global_gain 3.2500; pre_gain 3.9844; post_gain 3.9844; */ }},  //51.595191, //34.134988
        {0x224cc473, {0x00, 0xd4, 0xff, 0xff,/*global_gain 3.3125; pre_gain 3.9844; post_gain 3.9844; */ }},  //52.587406, //34.299873
        {0x22763053, {0x00, 0xd8, 0xff, 0xff,/*global_gain 3.3750; pre_gain 3.9844; post_gain 3.9844; */ }},  //53.579621, //34.461675
        {0x229ed99f, {0x00, 0xdc, 0xff, 0xff,/*global_gain 3.4375; pre_gain 3.9844; post_gain 3.9844; */ }},  //54.571837, //34.620508
        {0x22c6c75a, {0x00, 0xe0, 0xff, 0xff,/*global_gain 3.5000; pre_gain 3.9844; post_gain 3.9844; */ }},  //55.564052, //34.776479
        {0x22ee0029, {0x00, 0xe4, 0xff, 0xff,/*global_gain 3.5625; pre_gain 3.9844; post_gain 3.9844; */ }},  //56.556267, //34.929690
        {0x23148a56, {0x00, 0xe8, 0xff, 0xff,/*global_gain 3.6250; pre_gain 3.9844; post_gain 3.9844; */ }},  //57.548482, //35.080236
        {0x233a6bd9, {0x00, 0xec, 0xff, 0xff,/*global_gain 3.6875; pre_gain 3.9844; post_gain 3.9844; */ }},  //58.540697, //35.228208
        {0x235faa5f, {0x00, 0xf0, 0xff, 0xff,/*global_gain 3.7500; pre_gain 3.9844; post_gain 3.9844; */ }},  //59.532913, //35.373693
        {0x23844b4b, {0x00, 0xf4, 0xff, 0xff,/*global_gain 3.8125; pre_gain 3.9844; post_gain 3.9844; */ }},  //60.525128, //35.516774
        {0x23a853bc, {0x00, 0xf8, 0xff, 0xff,/*global_gain 3.8750; pre_gain 3.9844; post_gain 3.9844; */ }},  //61.517343, //35.657528
        {0x23cbc895, {0x00, 0xfc, 0xff, 0xff,/*global_gain 3.9375; pre_gain 3.9844; post_gain 3.9844; */ }},  //62.509558, //35.796029
        {0x23e605b8, {0x00, 0xff, 0xff, 0xff,/*global_gain 3.9844; pre_gain 3.9844; post_gain 3.9844; */ }},  //63.254117, //35.898525 
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
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

