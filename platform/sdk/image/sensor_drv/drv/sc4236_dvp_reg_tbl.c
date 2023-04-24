/*!
*****************************************************************************
** \file        sc4236_dvp_reg_tbl.c
**
** \version     $Id: sc4236_dvp_reg_tbl.c 11807 2017-04-13 09:38:45Z hehuali $
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

GADI_VI_SensorDrvInfoT    sc4236_sensor_dvp =
{
    .magic_start        = GADI_SENSOR_HW_INFO_MAGIC_START,
    .version            = GADI_SENSOR_HW_INFO_VERSION,
    .length             = sizeof(GADI_VI_SensorDrvInfoT),
    .file_date          = 0x20180710,
    .HwInfo         =
    {
        .name               = "sc4236",
        .ctl_type           = GADI_SENSOR_CTL_I2C,
        .addr_len           = sizeof(GADI_U16),
        .data_len           = sizeof(GADI_U8),
        .reset_active_level = GADI_GPIO_LOW,
        .hw_addr            = {(0x60>>1), (0x64>>1), 0, 0},
        .id_reg =
        {
            {0x3107, 0x32},
            {0x3108, 0x35},
        },
        .reset_reg =
        {
            {0x0103, 0x01}, // soft reset
            {GADI_VI_SENSOR_TABLE_FLAG_MSLEEP, 0x20},
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
    .sensor_id          = GADI_ISP_SENSOR_SC4236,
    .sensor_double_step = 32,
    .typeofsensor       = 2,
    .sync_time_delay    = 40,
    .source_type.dummy  = GADI_VI_CMOS_CHANNEL_TYPE_AUTO,
    .dev_type           = GADI_VI_DEV_TYPE_CMOS,
    .video_system       = GADI_VIDEO_SYSTEM_AUTO,
    .vs_polarity        = GADI_VI_FALLING_EDGE,//GADI_VI_RISING_EDGE,
    .hs_polarity        = GADI_VI_RISING_EDGE,
    .data_edge          = GADI_VI_RISING_EDGE,
    .emb_sync_switch    = GADI_VI_EMB_SYNC_OFF,
    .emb_sync_loc       = GADI_VI_EMB_SYNC_LOWER_PEL,
    .emb_sync_mode      = GADI_VI_EMB_SYNC_ITU_656,
    .max_width          = 2304,
    .max_height         = 1536,
    .def_sh_time        = GADI_VIDEO_FPS(60),
    .fmt_num            = 2,
    .auto_fmt           = GADI_VIDEO_MODE(1920, 1080,  25, 1),
    .init_reg           =
    {  
        {0x0100,0x00},
        {0x36e9,0xc4},
        
        //close mipi
        {0x3018,0x1f},
        {0x3019,0xff},
        {0x301c,0xb4},

        //close DVP
//        {0x301c,0x78}, // [7]:0  [3]:1 
        
        {0x3641,0x02}, 
        {0x3640,0x00},   //{0x3640,0x02},
        {0x3d08,0x00},
        
        {0x3333,0x00},
        {0x3e01,0xc7},
        {0x3e02,0xe0},

        {0x3908,0x11},
        
        //row noise
        {0x3e06,0x03},
        {0x3e08,0x1f},
        {0x3e09,0x1f},
        
        //mipi
//        {0x3018,0x33},//[7:5] lane_num-1
//        {0x3031,0x0a},//[3:0] bitmode
//        {0x3037,0x20},//[6:5] bitsel
//        {0x3001,0xFE},//[0] c_y
//        {0x4603,0x00},//[0] data_fifo mipi mode
//        {0x4837,0x35},//[7:0] pclk period * 2
////        {0x4827,0x88},//[7:0] hs_prepare_time[7:0]
//        {0x303f,0x01}, //[7] 1: pll_sclk  0: pll_pclk
        
        {0x5784,0x10}, //1114 0x04
        {0x5788,0x10}, //1114 0x04
        {0x337f,0x03}, //new auto precharge  330e in 3372   [7:6] 11: close div_rst 00:open div_rst
        {0x3368,0x04},
        {0x3369,0x00},
        {0x336a,0x00},
        {0x336b,0x00},
        {0x3367,0x08},
        {0x330e,0x30},
        {0x3320,0x06}, // New ramp offset timing 
//        {0x3321,0x06},
        {0x3326,0x00},
        {0x331e,0x2f},
        {0x331f,0x2f},
        {0x3308,0x18},
        {0x3303,0x40},
        {0x3309,0x40},
        {0x3306,0x40},
        {0x3301,0x50},
        {0x3638,0x88},
        {0x3307,0x18}, //[3:0]nedd >=7
        {0x3366,0x7c}, // div_rst gap
        
        //read noise
        {0x3622,0x22},
        {0x3633,0x88},
        {0x3635,0xc2},
        
        //fwc&Noise FWC4K
        {0x3632,0x18},
        {0x3038,0xcc},
        {0x363c,0x05},
        {0x363d,0x05},
        {0x3637,0x62},
        {0x3638,0x8c},
        {0x330b,0xc8},
        
        //10.16
        {0x3639,0x09},
        {0x363a,0x1f},
        {0x3638,0x98},
        {0x3306,0x60},
        {0x363b,0x0c},
//        {0x3e03,0x03},
        {0x3908,0x15},
        
        //low power
        {0x3620,0x28},
        
        //PLL
        {0x36e9,0x24},
        
        //25fps
        {0x320d,0x8c},
        {0x330b,0xd8},
        
        //blksun
        {0x3622,0x26},
        {0x3630,0xb8},
        {0x3631,0x88}, //margin 6 levels
        
        //dataspeed 117Mpix/s  351M cnt  sys 117M
        {0x36e9,0x46},
        {0x36ea,0x33},
        {0x36f9,0x06},
        {0x36fa,0xca},

        {0x3e01,0xbb},
        {0x3e02,0x60},
        {0x330b,0xec},
        {0x3636,0x24},
        {0x3637,0x64},
        {0x3638,0x18},
        {0x3625,0x03},

        {0x4837,0x20}, //mipi prepare floor(2 x Tmipi_pclk or bitres x 2000/ mipi lane speed bps )
        {0x3333,0x20}, //pix_samp all high

        {0x3e06,0x00},
        {0x3e08,0x03},
        {0x3e09,0x10},
        {0x3622,0x06},
        {0x3306,0x50},
        {0x330a,0x01},
        {0x330b,0x10},
        {0x366e,0x08}, // ofs auto en [3]
        {0x366f,0x2f}, // ofs+finegain  real ofs in 0x3687[4:0]
        {0x3235,0x0b}, //group hold position 1115C
        {0x3236,0xb0},

        //for 3.7k
        {0x3637,0x61},
        {0x3306,0x60},
        {0x330b,0x10},
        {0x3633,0x83},
        {0x3301,0x50},
        {0x3630,0xc8},
        {0x330e,0x80}, //for nir

        //pll test
        {0x36e9,0x44},
        {0x36eb,0x0e},
        {0x36ec,0x1e},
        {0x36ed,0x23},

        //sram write
        {0x3f00,0x0f}, //[2] 
        {0x3f04,0x05},
        {0x3f05,0x00},
        {0x3962,0x04}, //[0] 1  high temp cal_en

        {0x3622,0x16},
        {0x3320,0x06}, // New ramp offset timing 
//        {0x3321,0x06},
        {0x3326,0x00},
        {0x331e,0x21},
        {0x331f,0x71},
        {0x3308,0x18},
        {0x3303,0x30},
        {0x3309,0x80},

        /////////////////////////////////////high temp/////////////////////////////////////
        //blc max
        {0x3933,0x0a},
        {0x3934,0x08},
        {0x3942,0x02},
        {0x3943,0x0d},
        {0x3940,0x19},
        {0x3941,0x14},
        
        //blc temp
        {0x3946,0x20}, //kh0
        {0x3947,0x18}, //kh1
        {0x3948,0x06}, //kh2 
        {0x3949,0x06}, //kh3
        {0x394a,0x18}, //kh4
        {0x394b,0x2c}, //kh5
        {0x394c,0x08},
        {0x394d,0x14},
        {0x394e,0x24},
        {0x394f,0x34},
        {0x3950,0x14},
        {0x3951,0x08},

        {0x3952,0x78}, //kv0 20171211
        {0x3953,0x48}, //kv1
        {0x3954,0x18}, //kv2
        {0x3955,0x18}, //kv3
        {0x3956,0x48}, //kv4
        {0x3957,0x80}, //kv5
        {0x3958,0x10}, //posv0
        {0x3959,0x20}, //posv1
        {0x395a,0x38}, //posv2
        {0x395b,0x38}, //posv3
        {0x395c,0x20}, //posv4
        {0x395d,0x10}, //posv5

        {0x395e,0x24}, // alpha threshold 1115B
        {0x395f,0x00},
        {0x3960,0xc4},
        {0x3961,0xb1},
        {0x3962,0x0d},
        {0x3963,0x44}, //0x80

        //////////////////////////////////////////////////////////////////////////////////////////
        {0x3637,0x63},//add fullwell
        {0x3802,0x01},
        {0x3962,0x04}, //[0] 1  high temp cal_en
        {0x3366,0x78},//row noise optimize
        {0x33aa,0x00},//save power

        {0x3222,0x29},
        {0x3901,0x02},
        {0x3635,0xe2}, 
        {0x3963,0x80},//edges brighting when high temp 
        {0x3e1e,0x34},// digital finegain enable
//        {0x3962,0x0c},// H<=1920

        ////////////20180201  preprecharge
        // prchg max time =(0x3307[7:4]+0x3307[3:0]+0x330e+0x330f+0x3310[7:0])X3 when short exposure
        {0x330f,0x04},
        {0x3310,0x20},//0201B
        {0x3314,0x04},
        {0x330e,0x50},//0201B
        
        ///////////////////////////
        //20180208 
//        {0x4827,0x46},// [40ns+4/585M,85ns]
//        {0x3650,0x42},

        //20180508
        {0x5000,0x06}, //dpc enable
        {0x5780,0x7f}, //auto dpc
        {0x5781,0x04}, //white 1x
        {0x5782,0x03}, //      4x
        {0x5783,0x02}, //      8x
        {0x5784,0x01}, //      128x
        {0x5785,0x18}, //black 1x
        {0x5786,0x10}, //      4x
        {0x5787,0x08}, //      8x
        {0x5788,0x02}, //      128x
        {0x5789,0x20}, //Gain_thre1=4x
        {0x578a,0x30}, //Gain_thre2=8x

        //20180517  high temperature logic
        {0x3962,0x09},
        {0x3940,0x17},
        {0x3946,0x48},
        {0x3947,0x20},
        {0x3948,0x0A},
        {0x3949,0x10},
        {0x394A,0x28},
        {0x394B,0x48},
        {0x394E,0x28},
        {0x394F,0x50},
        {0x3950,0x20},
        {0x3951,0x10},
        {0x3952,0x70},
        {0x3953,0x40},
        {0x3956,0x40},

        //20180529 20180530
        {0x3940,0x15},     
        {0x3934,0x16},     
        {0x3943,0x20},     
        {0x3952,0x68},     
        {0x3953,0x38},     
        {0x3956,0x38},     
        {0x3957,0x78},     
        {0x394F,0x40},     

        //AGC
        {0x3E03,0x03},// gain = {0x3e08,0x3e09}/0x10
        {0x3301,0x1e},  //[a,21]
        {0x3633,0x23},
        {0x3630,0x80},
        {0x3622,0xf6},

        //TPG 
//        {0x4501,0x08},
//        {0x3902,0xb0},

        {0x0100,0x01},     
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },   
    .pll_table =
    {
        [0] =
        {
            // for 1296P@30, 1296P@20
            .pixclk = 117000000,
            .extclk = 27000000, 
            .regs =
            {            
//                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
    },
    .video_fmt_table =
    {
        [0] = // 1920x1080P@30fps   3900 * 1500 * 30 = 117000000
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
                {0x3200,0x00},//xstar 196
                {0x3201,0xc4},
                {0x3202,0x00},//ystar 232
                {0x3203,0xe8},
                {0x3204,0x08},//xend 196+8+1920-1=2123
                {0x3205,0x4b},        
                {0x3206,0x05},//yend 232+8+1080-1=1319
                {0x3207,0x27},  
                {0x3208,0x07},//1920
                {0x3209,0x80},        
                {0x320a,0x04},//1080
                {0x320b,0x38},  
                {0x320c,0x08},//hts=2166
                {0x320d,0x76},
                {0x320e,0x07},//vts=1800
                {0x320f,0x08},
                {0x3211,0x04},// x shift 4        
                {0x3213,0x04},// y shift 4
                {0x3f04,0x04},
                {0x3f05,0x17},// hts/2 - 0x24       
                {0x0100, 0x01, 0x01}, // stream output on
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
        [1] = // 2304x1296P@20fps   3900 * 1500 * 20 = 117000000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(2304, 1296,  25, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {                
                {0x0100, 0x00, 0x01}, // stream output hold          
                {0x3200,0x00},//xstar 4
                {0x3201,0x04},
                {0x3202,0x00},//ystar 124
                {0x3203,0x7c},
                {0x3204,0x09},//xend 4+8+2304-1=2315
                {0x3205,0x0b},        
                {0x3206,0x05},//yend 124+8+1296-1=1427
                {0x3207,0x93},                    
                {0x3208,0x09},//2304 width
                {0x3209,0x00},        
                {0x320a,0x05},//1296 hight
                {0x320b,0x10},  
                {0x320c,0x0B},//hts=2925
                {0x320d,0x6D},
                {0x320e,0x06},//vts=1600
                {0x320f,0x40},
                //{0x320c,0x0A},//hts=2600
                //{0x320d,0x28},
                //{0x320e,0x08},//vts=2250
                //{0x320f,0xCA},      
                {0x3211,0x04},// x shift 4        
                {0x3213,0x04},// y shift 4
                {0x3f04,0x05},
                {0x3f05,0x92},// hts/2 - 0x24        
                {0x3235,0x0F},
                {0x3236,0x9E},// vts x 2 - 2
                //{0x3f04,0x04},
                //{0x3f05,0xF0},// hts/2 - 0x24        
                //{0x3235,0x11},
                //{0x3236,0x92},// vts x 2 - 2     
                {0x0100, 0x01, 0x01}, // stream output on                
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
                .max_fps    = GADI_VIDEO_FPS(25),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(25),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_1080P_30,
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
            .update_after_vsync_start   = 3,
            .update_after_vsync_end     = 10,
        },
    },
    .mirror_table =
    {
#define SC4236_H_MIRROR     (3<<1)
#define SC4236_V_FLIP       (3<<5)
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY_VERTICALLY] =
        {
            {0x3221, SC4236_V_FLIP | SC4236_H_MIRROR, SC4236_V_FLIP | SC4236_H_MIRROR},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY] =
        {
            {0x3221, SC4236_H_MIRROR, SC4236_V_FLIP | SC4236_H_MIRROR},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_VERTICALLY] =
        {
            {0x3221, SC4236_V_FLIP, SC4236_V_FLIP | SC4236_H_MIRROR},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_NONE] =
        {
            {0x3221, 0x00, SC4236_V_FLIP | SC4236_H_MIRROR},
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
    .shs_fix        = 4,
    .shs_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_VSYNC,
    .agc_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_VSYNC,
    .vmax_reg_update_mode       = GADI_SENSOR_UPDATE_REG_BY_VSYNC,
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
    .max_agc_index      = 994,
    .max_again_index    = 0,//64,
    .cur_again_index    = 0,
    .agc_info =
    {
        .db_max     = 0x23FED3F1,    // 64x
        .db_min     = 0x00000000,    // 0dB
        .db_step    = 0x00100000,    // useless
    },    
    .gain_reg =
    {
//        {0x3E06, 0x00},
//        {0x3E07, 0x00},        
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
                    0x3301,0x1e,  //[a,21]
                    0x3633,0x23,
                    0x3630,0x80,
                    0x3622,0xf6,
            
                    [4>gain>=2]
                    0x3301,0x50,   //[11,b8]
                    0x3633,0x23,
                    0x3630,0x80,
                    0x3622,0xf6,
            
                    [8>gain>=4] 
                    0x3301,0x50,  //[18,b8]
                    0x3633,0x23,
                    0x3630,0x80,
                    0x3622,0xf6,
            
                    [15.5>=gain>=8]
                    0x3301,0x50, //[1e,b8]
                    0x3633,0x23,
                    0x3630,0x80,
                    0x3622,0xf6,
            
                    [gain>15.5]
                    0x3301,0xb8, //[22,b8]
                    0x3633,0x43,
                    0x3630,0x82,
                    0x3622,0x16,    
            */    
            
            //                      0x3E06       0x3E07               0x3E08              0x3E09
            //                       [3:0]        bit[7:0]              [4:2][1:0=11]       bit[7:0]
            //                  (000: 1X)   0x80 = 1dB             (000: 1X)            0x10 = 1dB
            //                  (001: 2X)   0x88 = 1.0625dB      (001: 2X)           0x11 = 1.0625dB
            //                  (011: 4X)    ...                         (011: 4X)             ...
            //                  (111: 8X)  0xF8 = 1.9375dB       (111: 8X)           0x1F = 1.9375dB
            //                  (1111: 16X) 
            /* Analog agc only:           |digital agc = 1|                    */
            {0x00100000 * 0x0000, {/*0x00, 0x80,*/ 0x03, 0x10,},}, //  1     dB
            {0x00100000 * 0x0001, {/*0x00, 0x80,*/ 0x03, 0x11,},}, //  1.0625dB
            {0x00100000 * 0x0002, {/*0x00, 0x80,*/ 0x03, 0x12,},}, //  1.125 dB
            {0x00100000 * 0x0003, {/*0x00, 0x80,*/ 0x03, 0x13,},}, //  1.1875dB
            {0x00100000 * 0x0004, {/*0x00, 0x80,*/ 0x03, 0x14,},}, //  1.25  dB
            {0x00100000 * 0x0005, {/*0x00, 0x80,*/ 0x03, 0x15,},}, //  1.3125dB
            {0x00100000 * 0x0006, {/*0x00, 0x80,*/ 0x03, 0x16,},}, //  1.375 dB
            {0x00100000 * 0x0007, {/*0x00, 0x80,*/ 0x03, 0x17,},}, //  1.4375dB
            {0x00100000 * 0x0008, {/*0x00, 0x80,*/ 0x03, 0x18,},}, //  1.5   dB
            {0x00100000 * 0x0009, {/*0x00, 0x80,*/ 0x03, 0x19,},}, //  1.5625dB
            {0x00100000 * 0x000A, {/*0x00, 0x80,*/ 0x03, 0x1A,},}, //  1.625 dB
            {0x00100000 * 0x000B, {/*0x00, 0x80,*/ 0x03, 0x1B,},}, //  1.6875dB
            {0x00100000 * 0x000C, {/*0x00, 0x80,*/ 0x03, 0x1C,},}, //  1.75  dB
            {0x00100000 * 0x000D, {/*0x00, 0x80,*/ 0x03, 0x1D,},}, //  1.8125dB
            {0x00100000 * 0x000E, {/*0x00, 0x80,*/ 0x03, 0x1E,},}, //  1.875 dB
            {0x00100000 * 0x000F, {/*0x00, 0x80,*/ 0x03, 0x1F,},}, //  1.9375dB
            {0x00100000 * 0x0010, {/*0x00, 0x80,*/ 0x07, 0x10,},}, //  2     dB
            {0x00100000 * 0x0011, {/*0x00, 0x80,*/ 0x07, 0x11,},}, //  2.125 dB
            {0x00100000 * 0x0012, {/*0x00, 0x80,*/ 0x07, 0x12,},}, //  2.25  dB
            {0x00100000 * 0x0013, {/*0x00, 0x80,*/ 0x07, 0x13,},}, //  2.375 dB
            {0x00100000 * 0x0014, {/*0x00, 0x80,*/ 0x07, 0x14,},}, //  2.5   dB
            {0x00100000 * 0x0015, {/*0x00, 0x80,*/ 0x07, 0x15,},}, //  2.625 dB
            {0x00100000 * 0x0016, {/*0x00, 0x80,*/ 0x07, 0x16,},}, //  2.75  dB
            {0x00100000 * 0x0017, {/*0x00, 0x80,*/ 0x07, 0x17,},}, //  2.875 dB
            {0x00100000 * 0x0018, {/*0x00, 0x80,*/ 0x07, 0x18,},}, //  3     dB
            {0x00100000 * 0x0019, {/*0x00, 0x80,*/ 0x07, 0x19,},}, //  3.125 dB
            {0x00100000 * 0x001A, {/*0x00, 0x80,*/ 0x07, 0x1A,},}, //  3.25  dB
            {0x00100000 * 0x001B, {/*0x00, 0x80,*/ 0x07, 0x1B,},}, //  3.375 dB
            {0x00100000 * 0x001C, {/*0x00, 0x80,*/ 0x07, 0x1C,},}, //  3.5   dB
            {0x00100000 * 0x001D, {/*0x00, 0x80,*/ 0x07, 0x1D,},}, //  3.625 dB
            {0x00100000 * 0x001E, {/*0x00, 0x80,*/ 0x07, 0x1E,},}, //  3.75  dB
            {0x00100000 * 0x001F, {/*0x00, 0x80,*/ 0x07, 0x1F,},}, //  3.875 dB
            {0x00100000 * 0x0020, {/*0x00, 0x80,*/ 0x0F, 0x10,},}, //  4     dB
            {0x00100000 * 0x0021, {/*0x00, 0x80,*/ 0x0F, 0x11,},}, //  4.25  dB
            {0x00100000 * 0x0022, {/*0x00, 0x80,*/ 0x0F, 0x12,},}, //  4.5   dB
            {0x00100000 * 0x0023, {/*0x00, 0x80,*/ 0x0F, 0x13,},}, //  4.75  dB
            {0x00100000 * 0x0024, {/*0x00, 0x80,*/ 0x0F, 0x14,},}, //  5     dB
            {0x00100000 * 0x0025, {/*0x00, 0x80,*/ 0x0F, 0x15,},}, //  5.25  dB
            {0x00100000 * 0x0026, {/*0x00, 0x80,*/ 0x0F, 0x16,},}, //  5.5   dB
            {0x00100000 * 0x0027, {/*0x00, 0x80,*/ 0x0F, 0x17,},}, //  5.75  dB
            {0x00100000 * 0x0028, {/*0x00, 0x80,*/ 0x0F, 0x18,},}, //  6     dB
            {0x00100000 * 0x0029, {/*0x00, 0x80,*/ 0x0F, 0x19,},}, //  6.25  dB
            {0x00100000 * 0x002A, {/*0x00, 0x80,*/ 0x0F, 0x1A,},}, //  6.5   dB
            {0x00100000 * 0x002B, {/*0x00, 0x80,*/ 0x0F, 0x1B,},}, //  6.75  dB
            {0x00100000 * 0x002C, {/*0x00, 0x80,*/ 0x0F, 0x1C,},}, //  7     dB
            {0x00100000 * 0x002D, {/*0x00, 0x80,*/ 0x0F, 0x1D,},}, //  7.25  dB
            {0x00100000 * 0x002E, {/*0x00, 0x80,*/ 0x0F, 0x1E,},}, //  7.5   dB
            {0x00100000 * 0x002F, {/*0x00, 0x80,*/ 0x0F, 0x1F,},}, //  7.75  dB
            {0x00100000 * 0x0030, {/*0x00, 0x80,*/ 0x1F, 0x10,},}, //  8     dB
            {0x00100000 * 0x0031, {/*0x00, 0x80,*/ 0x1F, 0x11,},}, //  8.5   dB
            {0x00100000 * 0x0032, {/*0x00, 0x80,*/ 0x1F, 0x12,},}, //  9     dB
            {0x00100000 * 0x0033, {/*0x00, 0x80,*/ 0x1F, 0x13,},}, //  9.5   dB
            {0x00100000 * 0x0034, {/*0x00, 0x80,*/ 0x1F, 0x14,},}, //  10    dB
            {0x00100000 * 0x0035, {/*0x00, 0x80,*/ 0x1F, 0x15,},}, //  10.5  dB
            {0x00100000 * 0x0036, {/*0x00, 0x80,*/ 0x1F, 0x16,},}, //  11    dB
            {0x00100000 * 0x0037, {/*0x00, 0x80,*/ 0x1F, 0x17,},}, //  11.5  dB
            {0x00100000 * 0x0038, {/*0x00, 0x80,*/ 0x1F, 0x18,},}, //  12    dB
            {0x00100000 * 0x0039, {/*0x00, 0x80,*/ 0x1F, 0x19,},}, //  12.5  dB
            {0x00100000 * 0x003A, {/*0x00, 0x80,*/ 0x1F, 0x1A,},}, //  13    dB
            {0x00100000 * 0x003B, {/*0x00, 0x80,*/ 0x1F, 0x1B,},}, //  13.5  dB
            {0x00100000 * 0x003C, {/*0x00, 0x80,*/ 0x1F, 0x1C,},}, //  14    dB
            {0x00100000 * 0x003D, {/*0x00, 0x80,*/ 0x1F, 0x1D,},}, //  14.5  dB
            {0x00100000 * 0x003E, {/*0x00, 0x80,*/ 0x1F, 0x1E,},}, //  15    dB
            {0x00100000 * 0x003F, {/*0x00, 0x80,*/ 0x1F, 0x1F,},}, //  15.5  dB
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

