/*!
*****************************************************************************
** \file        jxq03_reg_tbl.c
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
GADI_VI_SensorDrvInfoT    jxq03_sensor_mipi =
{
    .magic_start        = GADI_SENSOR_HW_INFO_MAGIC_START,
    .version            = GADI_SENSOR_HW_INFO_VERSION,
    .length             = sizeof(GADI_VI_SensorDrvInfoT),
    .file_date          = 0x20200724,
    .HwInfo         =
    {
        .name               = "jxq03",
        .ctl_type           = GADI_SENSOR_CTL_I2C,
        .addr_len           = sizeof(GADI_U8),
        .data_len           = sizeof(GADI_U8),
        .reset_active_level = GADI_GPIO_LOW,
        .hw_addr            = {(0x80>>1), (0x84>>1), (0x88>>1), (0x8C>>1)},
        .id_reg =
        {
            {0x0A, 0x05},   // PIDH[7:0]:Product ID MSBs. 0x05
            {0x0B, 0x07},   // PIDL[7:0]:Product ID LSBs. 0x07
        },
        .reset_reg =
        {
            {0x12, 0x80},   // SYS[7]: Soft reset initialize, 1: initial system reset,
                            //      it will reset whole sensor to factory default status, and
                            //      this bit will clear after reset.
                            // SYS[6]: Sleep mode on/off selection, 1: sensor into sleep mode.
                            //      No data output and all internal operation stops.
                            //      External controller can stop sensor clock, while I2C interface still can work.
                            // SYS[5]: mirror image on/off, 1: mirrored image output, 0: normal image output
                            // SYS[4]: flip image on/off, 1: flipped image output, 0: normal image output.
                            // SYS[3]: HDR mode on/off selection. 0: normal mode, 1: HDR mode.
                            //SYS[2]: RSVD.
                            //SYS[1]: vertical skip or full mode selection. ¡°0¡± : full mode,"1":vertical skip mode
                            //SYS[0]: Horizontal down sample mode enable
            {GADI_VI_SENSOR_TABLE_FLAG_MSLEEP, 0x0A},
            {0x12, 0x00},
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
    .sensor_id          = GADI_ISP_SENSOR_JXQ03,
    .sensor_double_step = 20,
    .typeofsensor       = 1,
    .sync_time_delay    = 0, //40
    .source_type.dummy  = GADI_VI_CMOS_CHANNEL_TYPE_AUTO,
    .dev_type           = GADI_VI_DEV_TYPE_CMOS,
    .video_system       = GADI_VIDEO_SYSTEM_AUTO,
    .vs_polarity        = GADI_VI_RISING_EDGE,
    .hs_polarity        = GADI_VI_RISING_EDGE,
    .data_edge          = GADI_VI_FALLING_EDGE,
    .emb_sync_switch    = GADI_VI_EMB_SYNC_OFF,
    .emb_sync_loc       = GADI_VI_EMB_SYNC_LOWER_PEL,
    .emb_sync_mode      = GADI_VI_EMB_SYNC_FULL_RANGE,
    .max_width          = 2312,
    .max_height         = 1320,
    .def_sh_time        = GADI_VIDEO_FPS(30),
    .fmt_num            = 1,
    .auto_fmt           = GADI_VIDEO_MODE(2304,  1296,  30, 1),
    .init_reg           =
    {
        {0x12,0x40},  //sleep mode bit[6]=1
        {0x48,0x96},  //BLCOpt1, BLCopt1 [3:2]= "01": HCLK = SYS_CLK / 2=72/2; BLCopt1[1:0]: "11": SRAM_CLK = SYS_CLK
        {0x48,0x16},
        {0x0E,0x11}, //PLL control 1. PLL1[6:4]: reserved; PLL1[1:0]: PLL pre-divider ratio. PLL_Pre_Ratio = 1 + PLL[1:0] = 2;
        {0x0F,0x14}, //PLL control 2, PLL2[2]: "1": work with MIPI 10bit mode
        {0x10,0x3C}, //PLL control 3. PLL VCO multiplier. VCO = Input clock*PLL3[7:0]/PLL_Pre_Ratio
        {0x11,0x80}, //Digital system clock control CLK[7]: System clock option. ¡°1¡±
        {0x0D,0xF2}, //DVP control 2. DVP2[7:4]: P-Pump and N-Pump clock selection. DVP2[3:2]: PAD drive capability. ¡°00¡±: min, ¡°11¡±: max; DVP2[1:0]: RSVD
        {0x5F,0x41}, //DAC PLL control 0
        {0x60,0x20}, //DAC PLL control 1
        {0x58,0x1A},  //RSVD  //0x50~0x5E
        {0x57,0x60},
        //HTS & VTS
        {0x20,0x84},  //FrameWidth_L
        {0x21,0x03},  //FrameWidth_H   // 0x384=900; 900*4 =3600;
        {0x22,0x35},  //FrameHeight_L
        {0x23,0x05},  //FrameHeight_H  //0x535 = 1333
       /* outputsize, H: 0x240=576,576*4= 2304; V: 0x510=1296 */
        {0x24,0x40},  //horizontal output window width_L
        {0x25,0x10},  //Image vertical output window high_L
        {0x26,0x52},  //Image output window horizontal and vertical MSBs. { Vwin[11:8],Hwin[11:8]} 
        /* start position H:0x354=852; V:0x015=21*/
        {0x27,0x54}, //Image horizontal output window start position LSBs. HwinSt[7:0]
        {0x28,0x15}, //Image vertical output window start position LSBs. VwinSt[7:0]
        {0x29,0x03}, //Image output window horizontal and vertical start position MSBs. {VwinSt[11:8],HwinSt[11:8]}
        {0x2A,0x4E}, //Column shift control 1. Column SRAM data shift start position LSBs. Cshift[7:0]  Cshift: 0x34E=846
        {0x2B,0x13}, //Column shift control 2
                     //Cshift2[7:6]: SRAM read out start address MSBs. SenHASt[9:8]
                     //Cshift2[5:4]: RSVD
                     //Cshift2[3:0]: Column SRAM data shift timing start position MSBs, Cshift[9:8]
        // SenHASt;0x000=0 0*4=0;
        {0x2C,0x00}, //SRAM read out start address LSBs, SenHASt[7:0]; 
        /* SenVSt 0x000 0*4=0; SenVEnd:0x14A=324 324*4=1320 */
        {0x2D,0x00}, //Sensor physical vertical start address, LSBs. SenVSt[7:0]; each bit represent 4 lines
        {0x2E,0x4A}, //Sensor physical vertical end address, LSBs. SenVEnd[7:0] ;each bit represent 4 lines
        {0x2F,0x44}, //Sensor vertical address settings. 
                     //SenVadd[7:4]: RSVD
                     //SenVadd[3:2]: SenVEnd[9:8]=01
                     //SenVadd[1:0]: SenVSt[9:8]=00
        {0x41,0x84}, //Sensor timing control 12,SenT12[7:0]: Array SRAM shift out pixel number LSBs, in 2 pixels step
        {0x42,0x14}, //SenT13[1:0]: Array SRAM shift out pixel number MSBs. 0x084 =132;
        {0x47,0x42}, 
        {0x76,0x40}, // mipi7, Mipi word count LSBs
        {0x77,0x0B}, //mipi8,Mipi word count MSBs
        {0x80,0x00}, //DigData, DigData[7]: frame sync function enable
                              //DigData[6]: DVP data output sequence adjust
                              //DigData[5:0]: RSVD
        {0xAF,0x22}, //RSD
        {0xAB,0x00}, // Skip_Step  [7:4] : Vskip step. [3:0]: Hskip step
        {0x46,0x00},
        {0x1D,0x00}, //DVP control 3,DVP3[7:0]: GPIO direction control for data output port D[7:0]. ¡°1¡±: enable output. ¡°0¡±: tri-state output.
        {0x1E,0x04}, //DVP control 4
        {0x6C,0x40}, //DPHY2, DPHY2[7]: Mipi interface power down. ¡°0¡±: enable ;¡±1¡± normal mode
        {0x6E,0x2C}, 
        {0x70,0xD9}, //Mipi1
        {0x71,0xD0}, //Mipi2
        {0x72,0xD5}, //Mipi3
        {0x73,0x59}, //mipi4
        {0x74,0x02}, //mipi5
        {0x78,0x96}, //mipi9
        {0x89,0x01},
        {0x6B,0x20},  //DPHY1, 
        {0x86,0x40},  //DPHY5, DPHY5[7:6]: Mipi high speed clock input skew adjust
                             //DPHY5[5:4]: Mipi data lane 1 clock delay adjust
                             //DPHY5[3:2]: Mipi data lane 0 clock delay adjust
                             //DPHY5[1:0]: Mipi clock lane clock delay adjust
        {0x31,0x0A},
        {0x32,0x21},
        {0x33,0x5C},
        {0x34,0x34},
        {0x35,0x30},
        {0x3A,0xA0},
        {0x3B,0x00},
        {0x3C,0x50},
        {0x3D,0x5B},
        {0x3E,0xFF},
        {0x3F,0x48},
        {0x40,0xFF},
        {0x56,0xB2},
        {0x59,0x34},
        {0x5A,0x04},
        {0x85,0x2C},
        {0x8A,0x04}, // [1]: Precharge off/on, ¡°0¡±: normal, ¡°1¡±: no precharge
        {0x91,0x08},
        {0xA9,0x08},
        {0x9C,0xE1},
        {0x5B,0xAC},
        {0x5C,0x81},
        {0x5D,0xEF},
        {0x5E,0x14},
        {0x64,0xE0},
        {0x66,0x04}, //PWC0, PWC5[5:4]: D-phy Lp high voltage reference voltage control; 00- min, 11- max.
        {0x67,0x77},
        {0x68,0x00},
        {0x69,0x41},//PWC3, PWC3[3]: Second stage black sun switch on/off enable, ¡°0¡±: always off. ¡°1¡±: Black sun will switch to second
                                        //stage when analog gain greater than 2x.
        {0x7A,0xA0},
        {0x8F,0x91},
        {0x9D,0x70},
        {0xAE,0x30}, //FastAE3
        {0x13,0x81}, //Luminance control register 1. LCCtrl1[0]: automatic luminance control on/off selection, ¡°0¡±: auto, ¡°1¡±: manual
        {0x96,0x04}, //Gain ctrl: Gain_Ctrl[7]: AutoDGain.
                                //1: Auto Digital Gain distribution in 0x00
                                //0: Manual set Digital Gain: 0x96[6:4]
                                //Gain_Ctrl[6:4]: DigitalGain.
                                //Gain_Ctrl[3:0]: Maximum Analog Gain
        {0x4A,0x05}, //BLC
        {0x7E,0xCD},
        {0x50,0x02},
        {0x49,0x10}, //Black level calibration target level.
        {0x7B,0x4A},
        {0x7C,0x0F},
        {0x7F,0x56},
        {0x62,0x21},
        {0x90,0x00},
        {0x8C,0xFF},
        {0x8D,0xC7},
        {0x8E,0x00},
        {0x8B,0x01},
        {0x0C,0x00},
        {0xBB,0x11},
        {0x6A,0x12}, //Power control 4
        {0x65,0x32}, //RAMP3 [7:4]: reserved.
                    //RAMP3 [3:0]: Second stage black sun reference control.
                    //Strength: (Strong) 1,0,F,E,D,C,B,A,9,8,7,6,5,4,3,2. (Weak)
        {0x82,0x00},
        {0x81,0x74},
        {0x19,0x20}, //Luminance control register 7
        {0x12,0x00}, //sys
        {0x48,0x96},
        {0x48,0x16},
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .pll_table =
    {
        [0] =
        {
            .pixclk = 144000000,
            .extclk = 24000000,
            .regs = {
                {GADI_VI_SENSOR_TABLE_FLAG_END,}
            }
        },
    },
    .video_fmt_table =
    {
        [0] = //  for 2304x1296P@30fps
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(2304,  1296,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
            /* 
             */
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 2304,
                .height     = 1296,
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
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_1080P_25,
            },
            .video_info_table =
            {
                .def_start_x    = 0,
                .def_start_y    = 0,
                .def_width      = 2304,
                .def_height     = 1296,
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
            .update_after_vsync_start   = 3,
            .update_after_vsync_end     = 10,
        },
    },
#define JXQ03_V_FLIP    (1<<4)
#define JXQ03_H_MIRROR  (1<<5)
    .mirror_table =
    {

        [GADI_VI_SRC_MIRROR_HORRIZONTALLY_VERTICALLY] =
        {
          {0x12, JXQ03_H_MIRROR | JXQ03_V_FLIP, JXQ03_H_MIRROR | JXQ03_V_FLIP},
          {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY] =
        {
           {0x12, JXQ03_H_MIRROR, JXQ03_H_MIRROR | JXQ03_V_FLIP},
           {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_VERTICALLY] =
        {
            {0x12, JXQ03_V_FLIP, JXQ03_H_MIRROR | JXQ03_V_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_NONE] =
        {
            {0x12, 0x00, JXQ03_H_MIRROR | JXQ03_V_FLIP},
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

    .vmax_clk_ref   = GADI_SENSOR_CAL_CLK_PIXCLK_DIV4,
    .shs_clk_ref    = GADI_SENSOR_CAL_CLK_PIXCLK_DIV4,
    .cal_shs_mode   = GADI_SENSOR_CAL_SHS_VMAX_MODE,
    .shs_fix        = 0,
    .shs_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_VSYNC,
    .agc_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_VSYNC,
    .vmax_reg_update_mode       = GADI_SENSOR_UPDATE_REG_BY_VSYNC,
    .mirror_reg_update_mode     = GADI_SENSOR_UPDATE_REG_BY_VSYNC,
    .shs_additional_use         = 0,
    .shs_additional_percent     = 0,
    // hmax = (FRAMEW_H & 0xFF) << 8 +
    //        (FRAMEW_L & 0xFF) << 0
    .hmax_reg =
    {
        {0x21, 0x00, 0xFF, 0x00, 0x08, 0}, // FRAMEW_H
        {0x20, 0x00, 0xFF, 0x00, 0x00, 0}, // FRAMEW_L
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    // vmax = (FRAMEH_H & 0xFF) << 8 +
    //        (FRAMEH_L & 0xFF) << 0
    .vmax_reg =
    {
        {0x23, 0x00, 0xFF, 0x00, 0x08, 0}, // FRAMEH_H
        {0x22, 0x00, 0xFF, 0x00, 0x00, 0}, // FRAMEH_L
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .vmax_wadd_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    // shs = (EXP_H & 0xFF) << 8 +
    //       (EXP_L & 0xFF) << 0
    .shs_reg =
    {
        {0x02, 0x00, 0xFF, 0x00, 0x08, 0}, // EXP_H Exposure line MSBs, EXP[15:8].;AEC[15:8]
        {0x01, 0x00, 0xFF, 0x00, 0x00, 0}, // EXP_L Exposure line LSBs, EXP[7:0]
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .shs_additional_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    .cal_gain_mode              = GADI_SENSOR_CAL_GAIN_MODE1,
    .max_agc_index = 80,
    .max_again_index = 0,
    .cur_again_index = 0,
    .agc_info =
    {
        .db_max     = 0x1db9a541,   // 15.5dB*4 max_db<<24
        .db_min     = 0x01000000,   // 1dB
        .db_step    = 0x00100000,   // 0.0625dB
    },
    .gain_reg =
    {
        {0x00, 0x00},
        {0x0D, 0x00},
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .gain_table =
    {
        /*REG_PGA addr 0x00;  bit[6:0];   again = (2^PGA[6:4])*(1+PGA[3:0]/16)*/
        /*REG_DVP2 addr 0x0d; bit[1:0];  dgain: bit[1:0]=00, no digital gain;  bit[1:0]=01 or 10, 2x;  bit[1:0]=11, 4x;*/
        /*Total gain = dgain*(2^PGA[6:4])*(1+PGA[3:0]/16)  */
        /*PGA   DVP2     agc_db    */ //   db
        {0x00000000, {0x00, 0x50,}}, //0.000000
        {0x008657c9, {0x01, 0x50,}}, //0.524777
        {0x0105013a, {0x02, 0x50,}}, //1.019550
        {0x017cd110, {0x03, 0x50,}}, //1.487565
        {0x01ee7b47, {0x04, 0x50,}}, //1.931569
        {0x025a997c, {0x05, 0x50,}}, //2.353905
        {0x02c1afdd, {0x06, 0x50,}}, //2.756590
        {0x032430f0, {0x07, 0x50,}}, //3.141372
        {0x0382809d, {0x08, 0x50,}}, //3.509775
        {0x03dcf68e, {0x09, 0x50,}}, //3.863137
        {0x0433e01a, {0x0a, 0x50,}}, //4.202638
        {0x048781d8, {0x0b, 0x50,}}, //4.529325
        {0x04d818df, {0x0c, 0x50,}}, //4.844130
        {0x0525dbda, {0x0d, 0x50,}}, //5.147886
        {0x0570fbe4, {0x0e, 0x50,}}, //5.441344
        {0x05b9a541, {0x0f, 0x50,}}, //5.725178
        {0x06000000, {0x10, 0x50,}}, //6.000000
        {0x068657c9, {0x11, 0x50,}}, //6.524777
        {0x0705013a, {0x12, 0x50,}}, //7.019550
        {0x077cd110, {0x13, 0x50,}}, //7.487565
        {0x07ee7b47, {0x14, 0x50,}}, //7.931569
        {0x085a997c, {0x15, 0x50,}}, //8.353905
        {0x08c1afdd, {0x16, 0x50,}}, //8.756590
        {0x092430f0, {0x17, 0x50,}}, //9.141372
        {0x0982809d, {0x18, 0x50,}}, //9.509775
        {0x09dcf68e, {0x19, 0x50,}}, //9.863137
        {0x0a33e01a, {0x1a, 0x50,}}, //10.202638
        {0x0a8781d8, {0x1b, 0x50,}}, //10.529325
        {0x0ad818df, {0x1c, 0x50,}}, //10.844130
        {0x0b25dbda, {0x1d, 0x50,}}, //11.147886
        {0x0b70fbe4, {0x1e, 0x50,}}, //11.441344
        {0x0bb9a541, {0x1f, 0x50,}}, //11.725178
        {0x0c000000, {0x20, 0x50,}}, //12.000000
        {0x0c8657c9, {0x21, 0x50,}}, //12.524777
        {0x0d05013a, {0x22, 0x50,}}, //13.019550
        {0x0d7cd110, {0x23, 0x50,}}, //13.487565
        {0x0dee7b47, {0x24, 0x50,}}, //13.931569
        {0x0e5a997c, {0x25, 0x50,}}, //14.353905
        {0x0ec1afdd, {0x26, 0x50,}}, //14.756590
        {0x0f2430f0, {0x27, 0x50,}}, //15.141372
        {0x0f82809d, {0x28, 0x50,}}, //15.509775
        {0x0fdcf68e, {0x29, 0x50,}}, //15.863137
        {0x1033e01a, {0x2a, 0x50,}}, //16.202638
        {0x108781d8, {0x2b, 0x50,}}, //16.529325
        {0x10d818df, {0x2c, 0x50,}}, //16.844130
        {0x1125dbda, {0x2d, 0x50,}}, //17.147886
        {0x1170fbe4, {0x2e, 0x50,}}, //17.441344
        {0x11b9a541, {0x2f, 0x50,}}, //17.725178
        {0x12000000, {0x30, 0x50,}}, //18.000000
        {0x128657c9, {0x31, 0x50,}}, //18.524777
        {0x1305013a, {0x32, 0x50,}}, //19.019550
        {0x137cd110, {0x33, 0x50,}}, //19.487565
        {0x13ee7b47, {0x34, 0x50,}}, //19.931569
        {0x145a997c, {0x35, 0x50,}}, //20.353905
        {0x14c1afdd, {0x36, 0x50,}}, //20.756590
        {0x152430f0, {0x37, 0x50,}}, //21.141372
        {0x1582809d, {0x38, 0x50,}}, //21.509775
        {0x15dcf68e, {0x39, 0x50,}}, //21.863137
        {0x1633e01a, {0x3a, 0x50,}}, //22.202638
        {0x168781d8, {0x3b, 0x50,}}, //22.529325
        {0x16d818df, {0x3c, 0x50,}}, //22.844130
        {0x1725dbda, {0x3d, 0x50,}}, //23.147886
        {0x1770fbe4, {0x3e, 0x50,}}, //23.441344
        {0x17b9a541, {0x3f, 0x50,}}, //23.725178
        {0x18000000, {0x40, 0x50,}}, //24.000000
        {0x188657c9, {0x41, 0x50,}}, //24.524777
        {0x1905013a, {0x42, 0x50,}}, //25.019550
        {0x197cd110, {0x43, 0x50,}}, //25.487565
        {0x19ee7b47, {0x44, 0x50,}}, //25.931569
        {0x1a5a997c, {0x45, 0x50,}}, //26.353905
        {0x1ac1afdd, {0x46, 0x50,}}, //26.756590
        {0x1b2430f0, {0x47, 0x50,}}, //27.141372
        {0x1b82809d, {0x48, 0x50,}}, //27.509775
        {0x1bdcf68e, {0x49, 0x50,}}, //27.863137
        {0x1c33e01a, {0x4a, 0x50,}}, //28.202638
        {0x1c8781d8, {0x4b, 0x50,}}, //28.529325
        {0x1cd818df, {0x4c, 0x50,}}, //28.844130
        {0x1d25dbda, {0x4d, 0x50,}}, //29.147886
        {0x1d70fbe4, {0x4e, 0x50,}}, //29.441344
        {0x1db9a541, {0x4f, 0x50,}}, //29.725178
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

