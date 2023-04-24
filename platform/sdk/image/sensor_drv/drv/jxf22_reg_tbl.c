/*!
*****************************************************************************
** \file        jxf22_reg_tbl.c
**
** \version     $Id: jxf22_reg_tbl.c 11627 2017-03-10 10:58:12Z dengbiao $
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
GADI_VI_SensorDrvInfoT    jxf22_sensor =
{
    .magic_start        = GADI_SENSOR_HW_INFO_MAGIC_START,
    .version            = GADI_SENSOR_HW_INFO_VERSION,
    .length             = sizeof(GADI_VI_SensorDrvInfoT),
    .file_date          = 0x20161017,
    .HwInfo         =
    {
        .name               = "jxf22",
        .ctl_type           = GADI_SENSOR_CTL_I2C,
        .addr_len           = sizeof(GADI_U8),
        .data_len           = sizeof(GADI_U8),
        .reset_active_level = GADI_GPIO_LOW,
        .hw_addr            = {(0x80>>1), (0x84>>1), (0x88>>1), (0x8C>>1)},
        .id_reg =
        {
            {0x0A, 0x0F},
            {0x0B, 0x22},
        },
        .reset_reg =
        {
            {0x12, 0x80},
            {GADI_VI_SENSOR_TABLE_FLAG_MSLEEP, 0x0A},
            {0x12, 0x00},
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
    .sensor_id          = GADI_ISP_SENSOR_JXF22,
    .sensor_double_step = 20,
    .typeofsensor       = 1,
    .sync_time_delay    = 40,
    .source_type.dummy  = GADI_VI_CMOS_CHANNEL_TYPE_AUTO,
    .dev_type           = GADI_VI_DEV_TYPE_CMOS,
    .video_system       = GADI_VIDEO_SYSTEM_AUTO,
    .vs_polarity        = GADI_VI_RISING_EDGE,
    .hs_polarity        = GADI_VI_RISING_EDGE,
    .data_edge          = GADI_VI_RISING_EDGE,
    .emb_sync_switch    = GADI_VI_EMB_SYNC_OFF,
    .emb_sync_loc       = GADI_VI_EMB_SYNC_LOWER_PEL,
    .emb_sync_mode      = GADI_VI_EMB_SYNC_FULL_RANGE,
    .max_width          = 1932,
    .max_height         = 1088,
    .def_sh_time        = GADI_VIDEO_FPS(50),
    .fmt_num            = 6,
    .auto_fmt           = GADI_VIDEO_MODE(1920,  1080,  30, 1),
    .init_reg           =
    {
        {0x0C, 0x00}, // DVP1[6]: Sensor sun spot elimination control, 1: disable sun spot elimination, 0: enable sun spot elimination.
                      // DVP1[1:0]: DVP test mode output option. 00: DVP output normal image data. 01: DVP[9:0] = Output 10-bit walk 1 test pattern.; others: RSVD
        {0x12, 0x00}, // SYS[7]: Soft reset initialize, 1: initial system reset, it will reset whole sensor to factory default status, and this bit will clear after reset. Default : 0: normal mode
                      // SYS[6]: Sleep mode on/off selection, 1: sensor into sleep mode. No data output and all internal operation stops. External controller can stop sensor clock, while I2C interface still can work.
                      // Default : 0 : normal mode
                      // SYS[5]: mirror image on/off, 1: mirrored image output, 0: normal image output
                      // SYS[4]: flip image on/off, 1: flipped image output, 0: normal image output.
                      // SYS[3]: HDR mode on/off selection. 0: normal mode, 1: HDR mode.
                      // SYS[2]: horizontal skip or binning mode select default : 0 : binning mode
                      // SYS[1]: vertical skip or full mode selection. Default : 0 : full mode
        {0x13, 0x81}, // LCCtrl1[7:6]: Gain ceiling at auto mode. 00: maximum 4X analog gain. 01: 8X, 10: 16X, 11: 32X
                      // LCCtrl1[5:3]: Auto Frame rate ceiling control. 000: disable auto frame rate adjust. 001: max frame rate down 1/2, 011: max frame rate down 1/4; 111: max frame rate down 1/8;
                      // LCCtrl1[2]: Banding filter on/off selection. 1: on. 0: off
                      // LCCtrl1[1]: RSVD.
                      // LCCtrl1[0]: automatic luminance control on/off selection, 0: auto, 1: manual
        {0x19, 0x20}, // LCCtrl7[7:6]: PCLK delay option.
                      // LCCtrl7[5]: AGC delay 1 frame valid option. 0: manual gain will apply at next VSYNC;1: manual gain will delay 1 frame to apply.
                      // LCCtrl7[3:2]: System clock selection,
                      // 00: use 1/4 clock as hclk, and 1/2 clock as ADC clock, known as half clock option.
                      // 01: use 1/2 clock as hclk and 1/2 clock as ADC clock.
                      // 1x: use full PLL clock as hclk and ADC clock. So no 2x clock for DVP output. Can use in MIPI mode.
                      // LLCCtrl7[1:0]: Banding filter minimum exposure line MSBs. Band[9:8]
        {0x1D, 0xFF}, // DVP3[7:0]: GPIO direction control for data output port D[7:0]. 1: enable output. 0: tri-state output.
        {0x1E, 0x1F}, // DVP4[7]: PCLK polarity control.
                      // 0: data output at rising edge of PCLK, 1:data output at falling edge of PCLK
                      // DVP4[6]: HREF polarity control. 0: positive, 1:negative
                      // DVP4[5]: VSYNC polarity control. . 0: positive, 1:negative
                      // DVP4[4:0]: GPIO direction control for output port: PCLK, HREF, VSYNC and D[9:8] .
                      // 1: enable output. 0: tri-state output.
        {0x1F, 0x01}, // Glat[7]: Group write trigger. 1: system will write reg0xC0 to 0xFF content to proper register. This bit will clear after group write. 0: inactive group write function..
                      // Glat[6]: Group latch trigger time option, 0: trigger at vertical blanking period. 1: group latch trigger immediately.
                      // Glat[0]: Scan_Trig: When write 1 ,Trig scan once then reset to 0
        {0x2A, 0xC0}, // Cshift1[7:6]: Column SRAM data shift start position LSBs. Cshift[7:6]
        {0x2B, 0x21}, // Cshift2[1:0]: Column SRAM data shift start position MSBs, Cshift[9:8]
        {0x30, 0x8C},
        {0x31, 0x0C},
        {0x32, 0xF0},
        {0x33, 0x0C},
        {0x34, 0x1F},
        {0x35, 0xE3},
        {0x36, 0x0E},
        {0x37, 0x34},
        {0x38, 0x13},
        {0x39, 0x90}, // SenT10[4]: delay HREF output by one PCLK. Can be used as output color sequence change.
        {0x3A, 0x08},
        {0x3B, 0x30},
        {0x3C, 0xC0},
        {0x3D, 0x00},
        {0x3E, 0x00},
        {0x3F, 0x00},
        {0x40, 0x00},
        {0x45, 0x89},
        {0x45, 0x09},
        {0x47, 0x22}, // LBLC[7]: BLC limitation control, 0: BLC max value is 0x200, 1: BLC max value is 0x400
        {0x48, 0x05}, // BLCopt1[7]: 0: normal, 1:BLC adjust only when the difference bigger than a threshold
                      // BLCopt1[6:4]: RSVD
                      // BLCopt1[3:0]: BLC adjustment threshold.
        {0x49, 0x10}, // BLC_TGT[7]: sign bit.0 positive; 1 negative
                      // BLC_TGT[6:0]: target level.
        {0x4A, 0x05}, // BLCCtrl[7]: BLC_B bit 10
                      // BLCCtrl[6]: BLC_Gb bit 10
                      // BLCCtrl[5]: BLC_Gr bit 10
                      // BLCCtrl[4]: BLC_R bit 10
                      // BLCCtrl[1]: BLC action option. 0: Sensor do BLC only when triggered. 1: always do BLC.
                      // BLCCtrl[0]: auto BLC function on/off selection. 0: sensor stop calculate black value. 1: sensor calculates black value automatically.
        {0x50, 0x02}, // SBLCCtrl[7]: SBLC_B bit 10
                      // SBLCCtrl[6]: SBLC_Gb bit 10
                      // SBLCCtrl[5]: SBLC_Gr bit 10
                      // SBLCCtrl[4]: SBLC_R bit 10
                      // SBLCCtrl[3:1]: reserved
                      // SBLCCtrl[0]: BLC target option when digital gain apply, 1: BLC target level not effect when apply digital
                      //  gain, 0: BLC target level will change when apply digital gain.
        {0x56, 0x32},
        {0x59, 0x97}, // Vsun2[7:5]: reserved.
                      // Vsun2[4]: Second stage black sun switch on/off enable, 0: always off. 1: Black sun will switch to second stage when analog gain greater than 2x.
                      // Vsun2[3:0]: Second stage black sun reference control.
                      // Strength : (Strong) 1,0,F,E,D,C,B,A,9,8,7,6,5,4,3,2. (Weak
        {0x5A, 0x20},
        {0x5B, 0xB3},
        {0x5C, 0xF7},
        {0x5D, 0xF0},
        {0x5F, 0x01}, // DACPLL0[7]: DAC PLL bypass on/off selection ; 1: Bypass on, 0: By pass off
                      // DACPLL0[3:2]: DAC PLL post divider bypass on/off selection ; 1: Bypass on, 0: By pass off
                      // DACPLL0[1:0]: DAC PLL pre- divider
                      // DAC_CLK=input clock/(DAC PLL pre-divider + 1) * DAC PLL VCO multiplier / (DAC PLL post divider + 1)
        {0x60, 0x09}, // DACPLL1[5:0]:DAC PLL VCO multiplier
        {0x62, 0x80},
        {0x63, 0x82},
        {0x64, 0x00},
        {0x67, 0x75},
        {0x68, 0x04},
        {0x6A, 0x4D}, // PWC4[3:0]: first stage black sun control.
                      // Strength : (Strong) 1,0,F,E,D,C,B,A,9,8,7,6,5,4,3,2. (Weak)
        {0x6C, 0x90},
        {0x6F, 0x03},
        {0x7E, 0xCD},
        {0x7F, 0x52},
        {0x7B, 0x57},
        {0x7C, 0x28},
        {0x80, 0x00},
        {0x8F, 0x18},
        {0x91, 0x04},
        {0x93, 0x5C},
        {GADI_VI_SENSOR_TABLE_FLAG_END,}
    },
    .pll_table =
    {
        // PLL1[7] = 1: PLLclk = extclk/(1 + PLL1[1:0])
        // PLL1[7] = 0:
        //   PLL2[3] = 1: PLLclk = extclk*PLL3[7:0]/(1 + PLL1[1:0])/2
        //   PLL2[3] = 0: PLLclk = extclk*PLL3[7:0]/(1 + PLL1[1:0])/(1+PLL2[1:0])
        [0] =
        {
            // for 1080P@30, 1080P@25, 720P@60, 720P@50
            .pixclk = 74250000, // PLLclk = extclk*PLL3[7:0]/(1 + PLL1[1:0])/(1+PLL2[1:0])
            .extclk = 27000000, //        = 27*66/(1+2)/(1+0)/8=74.25
            .regs =
            {
                {0x0E, 0x12}, // PLL1[7]0: PLL bypass selection. 1: PLLclk = Input clock/PLL_Pre_Ratio. 0: see PLL2[3:0].
                              // PLL1[1:0]01: PLL pre-divider ratio. PLL_Pre_Ratio = 1 + PLL1[1:0] Default : 00
                {0x0F, 0x00}, // PLL2[3]0: R_MIPI;1: MIPIPCLK=1/2*VCO; 0: MIPIPCLK=VCO;
                              // PLL2[1:0]00: PLL clock divider. PLLclk = VCO/(1+PLL2[1:0])
                {0x10, 0x42}, // PLL VCO multiplier. VCO = Input clock*PLL3[7:0]/PLL_Pre_Ratio
                {0x11, 0x80}, // CLK[7]: System clock option. 1: system clock use PLLclk directly. 0: system clock use PLLclk after digital divide clock
                              // CLK[6]: system clock digital doubler on/off selection. 1: on, 0: off
                              // CLK[5:0]: system clock divide ratio. default :000000
                              // When CLK[5:0] > 0: System clock = PLLclk*(1+CLK[6])/(2*CLK[5:0])
                              // When CLK[5:0] = 0: System clock = PLLclk*(1+CLK[6])/2
                {GADI_VI_SENSOR_TABLE_FLAG_END,}
            }
        },
        [1] =
        {
            // for 960P@30, 960P@25
            .pixclk = 54000000, // PLLclk = extclk*PLL3[7:0]/(1 + PLL1[1:0])/(1+PLL2[1:0])
            .extclk = 27000000, //        = 27*64/(1+3)/(1+0)/8=54
            .regs =
            {
                {0x0E, 0x13}, // PLL1[7]0: PLL bypass selection. 1: PLLclk = Input clock/PLL_Pre_Ratio. 0: see PLL2[3:0].
                              // PLL1[1:0]01: PLL pre-divider ratio. PLL_Pre_Ratio = 1 + PLL1[1:0] Default : 00
                {0x0F, 0x00}, // PLL2[3]0: R_MIPI;1: MIPIPCLK=1/2*VCO; 0: MIPIPCLK=VCO;
                              // PLL2[1:0]00: PLL clock divider. PLLclk = VCO/(1+PLL2[1:0])
                {0x10, 0x40}, // PLL VCO multiplier. VCO = Input clock*PLL3[7:0]/PLL_Pre_Ratio
                {0x11, 0x80}, // CLK[7]: System clock option. 1: system clock use PLLclk directly. 0: system clock use PLLclk after digital divide clock
                              // CLK[6]: system clock digital doubler on/off selection. 1: on, 0: off
                              // CLK[5:0]: system clock divide ratio. default :000000
                              // When CLK[5:0] > 0: System clock = PLLclk*(1+CLK[6])/(2*CLK[5:0])
                              // When CLK[5:0] = 0: System clock = PLLclk*(1+CLK[6])/2
                {GADI_VI_SENSOR_TABLE_FLAG_END,}
            }
        },
        [2] =
        {
            // for 720P@30, 720P@25
            .pixclk = 37125000, // PLLclk = extclk*PLL3[7:0]/(1 + PLL1[1:0])/(1+PLL2[1:0])
            .extclk = 27000000, //        = 27*44/(1+1)/(1+1)/8=37.125
            .regs =
            {
                {0x0E, 0x11}, // PLL1[7]0: PLL bypass selection. 1: PLLclk = Input clock/PLL_Pre_Ratio. 0: see PLL2[3:0].
                              // PLL1[1:0]01: PLL pre-divider ratio. PLL_Pre_Ratio = 1 + PLL1[1:0] Default : 00
                {0x0F, 0x01}, // PLL2[3]0: R_MIPI;1: MIPIPCLK=1/2*VCO; 0: MIPIPCLK=VCO;
                              // PLL2[1:0]00: PLL clock divider. PLLclk = VCO/(1+PLL2[1:0])
                {0x10, 0x2C}, // PLL VCO multiplier. VCO = Input clock*PLL3[7:0]/PLL_Pre_Ratio
                {0x11, 0x80}, // CLK[7]: System clock option. 1: system clock use PLLclk directly. 0: system clock use PLLclk after digital divide clock
                              // CLK[6]: system clock digital doubler on/off selection. 1: on, 0: off
                              // CLK[5:0]: system clock divide ratio. default :000000
                              // When CLK[5:0] > 0: System clock = PLLclk*(1+CLK[6])/(2*CLK[5:0])
                              // When CLK[5:0] = 0: System clock = PLLclk*(1+CLK[6])/2
                {GADI_VI_SENSOR_TABLE_FLAG_END,}
            }
        },
    },
    .video_fmt_table =
    {
        [0] = // 1920x1080P@30fps   (1100*2) * 1125 * 30 = 74250000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1920, 1080,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x12, 0x40, 0x40},
                {0x20, 0x4C}, // FrameW_L       0x044C = 1100
                {0x21, 0x04}, // FrameW_H
                {0x24, 0xC0}, // HWin_L         0x03C0 = 1920 / 2
                {0x25, 0x38}, // VWin_L         0x0438 = 1080
                {0x26, 0x43}, // VWin_H HWin_H
                {0x27, 0xC9}, // HWinSt_L       0x01C9 =  457
                {0x28, 0x18}, // VWinSt_L       0x0018 =   24
                {0x29, 0x01}, // VWinSt_H HWinSt_H
                {0x2C, 0x10}, // SenHASt        each bit represent 2 pixels
                {0x2D, 0x01}, // SenVSt         each bit represent 4 lines
                {0x2E, 0x15}, // SenVEnd        (0x115-0x01)*4 = 0x0450 = 1104
                {0x2F, 0x44}, // SenVadd Sensor vertical address settings.
                              //  SenVadd[7:4]: RSVD
                              //  SenVadd[3:2]: SenVEnd[9:8]
                              //  SenVadd[1:0]: SenVSt[9:8]
                {0x41, 0xCC}, // SenT12[7:0]: Array SRAM shift out pixel number LSBs, in 2 pixels step.
                {0x42, 0x03}, // SenT13[1:0]: Array SRAM shift out pixel number MSBs.
                {0x12, 0x00, 0x40},
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
        [1] = // 1920x1080P@25fps   (1320*2) * 1125 * 25 = 74250000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1920, 1080,  25, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x20, 0x28}, // FrameW_L       0x0528 = 1320
                {0x21, 0x05}, // FrameW_H
                {0x24, 0xC0}, // HWin_L         0x03C0 = 1920 / 2
                {0x25, 0x38}, // VWin_L         0x0438 = 1080
                {0x26, 0x43}, // VWin_H HWin_H
                {0x27, 0xC9}, // HWinSt_L       0x01C9 =  457
                {0x28, 0x18}, // VWinSt_L       0x0018 =   24
                {0x29, 0x01}, // VWinSt_H HWinSt_H
                {0x2C, 0x10}, // SenHASt        each bit represent 2 pixels
                {0x2D, 0x01}, // SenVSt         each bit represent 4 lines
                {0x2E, 0x15}, // SenVEnd        (0x115-0x01)*4 = 0x0450 = 1104
                {0x2F, 0x44}, // SenVadd Sensor vertical address settings.
                              //  SenVadd[7:4]: RSVD
                              //  SenVadd[3:2]: SenVEnd[9:8]
                              //  SenVadd[1:0]: SenVSt[9:8]
                {0x41, 0xCC},
                {0x42, 0x03},
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
        [2] = // 1280x960P@30fps    (1100*2) * 1125 * 30 = 74250000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  960,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x12, 0x40, 0x40},
                {0x20, 0x4C}, // FrameW_L       0x044C = 1100
                {0x21, 0x04}, // FrameW_H
                {0x24, 0x80}, // HWin_L         0x0280 = 1280 / 2
                {0x25, 0xC0}, // VWin_L         0x03C0 = 960
                {0x26, 0x32}, // VWin_H HWin_H
                {0x27, 0xC9}, // HWinSt_L       0x01C9 =  457
                {0x28, 0x18}, // VWinSt_L       0x0018 =   24
                {0x29, 0x01}, // VWinSt_H HWinSt_H
                {0x2C, 0xA4}, // SenHASt        each bit represent 2 pixels
                {0x2D, 0x10}, // SenVSt         each bit represent 4 lines
                {0x2E, 0x06}, // SenVEnd        (0x106-0x10)*4 = 0x03D8 = 984
                {0x2F, 0x44}, // SenVadd Sensor vertical address settings.
                              //  SenVadd[7:4]: RSVD
                              //  SenVadd[3:2]: SenVEnd[9:8]
                              //  SenVadd[1:0]: SenVSt[9:8]
                {0x41, 0x8C}, // SenT12[7:0]: Array SRAM shift out pixel number LSBs, in 2 pixels step.
                {0x42, 0x02}, // SenT13[1:0]: Array SRAM shift out pixel number MSBs.
                {0x12, 0x00, 0x40},
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
            .update_after_vsync_start   = 3,
            .update_after_vsync_end     = 10,
        },
        [3] = // 1280x960P@25fps   (1320*2) * 1125 * 25 = 74250000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  960,  25, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x12, 0x40, 0x40},
                {0x20, 0x28}, // FrameW_L       0x0528 = 1320
                {0x21, 0x05}, // FrameW_H
                {0x24, 0x80}, // HWin_L         0x0280 = 1280 / 2
                {0x25, 0xC0}, // VWin_L         0x03C0 = 960
                {0x26, 0x32}, // VWin_H HWin_H
                {0x27, 0xC9}, // HWinSt_L       0x01C9 =  457
                {0x28, 0x18}, // VWinSt_L       0x0018 =   24
                {0x29, 0x01}, // VWinSt_H HWinSt_H
                {0x2C, 0xA4}, // SenHASt        each bit represent 2 pixels
                {0x2D, 0x10}, // SenVSt         each bit represent 4 lines
                {0x2E, 0x06}, // SenVEnd        (0x106-0x10)*4 = 0x03D8 = 984
                {0x2F, 0x44}, // SenVadd Sensor vertical address settings.
                              //  SenVadd[7:4]: RSVD
                              //  SenVadd[3:2]: SenVEnd[9:8]
                              //  SenVadd[1:0]: SenVSt[9:8]
                {0x41, 0x8C}, // SenT12[7:0]: Array SRAM shift out pixel number LSBs, in 2 pixels step.
                {0x42, 0x02}, // SenT13[1:0]: Array SRAM shift out pixel number MSBs.
                {0x12, 0x00, 0x40},
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
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_960P_25,
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
            .update_after_vsync_start   = 3,
            .update_after_vsync_end     = 10,
        },
        [4] =   // 1280x720P@30fps    (1100*2) * 1125 * 30 = 74250000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x12, 0x40, 0x40},
                {0x20, 0x4C}, // FrameW_L       0x044C = 1100
                {0x21, 0x04}, // FrameW_H
                {0x24, 0x80}, // HWin_L         0x0280 = 1280 / 2
                {0x25, 0xD0}, // VWin_L         0x02D0 = 720
                {0x26, 0x22}, // VWin_H HWin_H
                {0x27, 0xC9}, // HWinSt_L       0x01C9 =  457
                {0x28, 0x18}, // VWinSt_L       0x0018 =   24
                {0x29, 0x01}, // VWinSt_H HWinSt_H
                {0x2C, 0xA4}, // SenHASt        each bit represent 2 pixels
                {0x2D, 0x2E}, // SenVSt         each bit represent 4 lines
                {0x2E, 0xE8}, // SenVEnd        (0xE8-0x2E)*4 = 0x02E8 = 744
                {0x2F, 0x40}, // SenVadd Sensor vertical address settings.
                              //  SenVadd[7:4]: RSVD
                              //  SenVadd[3:2]: SenVEnd[9:8]
                              //  SenVadd[1:0]: SenVSt[9:8]
                {0x41, 0x8C}, // SenT12[7:0]: Array SRAM shift out pixel number LSBs, in 2 pixels step.
                {0x42, 0x02}, // SenT13[1:0]: Array SRAM shift out pixel number MSBs.
                {0x12, 0x00, 0x40},
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
                .auto_fps   = GADI_VIDEO_FPS(30),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_720P_30,
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
            .update_after_vsync_start   = 3,
            .update_after_vsync_end     = 10,
        },
        [5] =   // 1280x720P@25fps    (1320*2) * 1125 * 25 = 74250000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  25, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x12, 0x40, 0x40},
                {0x20, 0x4C}, // FrameW_L       0x044C = 1100
                {0x21, 0x04}, // FrameW_H
                {0x24, 0x80}, // HWin_L         0x0280 = 1280 / 2
                {0x25, 0xD0}, // VWin_L         0x02D0 = 720
                {0x26, 0x22}, // VWin_H HWin_H
                {0x27, 0xC9}, // HWinSt_L       0x01C9 =  457
                {0x28, 0x18}, // VWinSt_L       0x0018 =   24
                {0x29, 0x01}, // VWinSt_H HWinSt_H
                {0x2C, 0xA4}, // SenHASt        each bit represent 2 pixels
                {0x2D, 0x2E}, // SenVSt         each bit represent 4 lines
                {0x2E, 0xE8}, // SenVEnd        (0xE8-0x2E)*4 = 0x02E8 = 744
                {0x2F, 0x40}, // SenVadd Sensor vertical address settings.
                              //  SenVadd[7:4]: RSVD
                              //  SenVadd[3:2]: SenVEnd[9:8]
                              //  SenVadd[1:0]: SenVSt[9:8]
                {0x41, 0x8C}, // SenT12[7:0]: Array SRAM shift out pixel number LSBs, in 2 pixels step.
                {0x42, 0x02}, // SenT13[1:0]: Array SRAM shift out pixel number MSBs.
                {0x12, 0x00, 0x40},
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
#define JXF22_V_FLIP    (1<<4)
#define JXF22_H_MIRROR  (1<<5)
    .mirror_table =
    {
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY_VERTICALLY] =
        {
            {0x12, JXF22_H_MIRROR | JXF22_V_FLIP, JXF22_H_MIRROR | JXF22_V_FLIP},
            {0x2C, 0x10, 0x1F},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY] =
        {
            {0x12, JXF22_H_MIRROR, JXF22_H_MIRROR | JXF22_V_FLIP},
            {0x2C, 0x10, 0x1F},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_VERTICALLY] =
        {
            {0x12, JXF22_V_FLIP, JXF22_H_MIRROR | JXF22_V_FLIP},
            {0x2C, 0x04, 0x1F},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_NONE] =
        {
            {0x12, 0x00, JXF22_H_MIRROR | JXF22_V_FLIP},
            {0x2C, 0x04, 0x1F},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
    },
    .version_reg =
    {
        {0x09, 0x00},
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .reg_ctl_by_ver_num = 3,
    .control_by_version_reg =
    {
        [0] =
        {
            .version = 0x00,
            .mask    = 0xFF,
            .regs =
            {
                {0x2C, 0x00, GADI_SENSOR_VER_CTL_REG_INIT},
                {0x48, 0x40, GADI_SENSOR_VER_CTL_REG_INIT},
                {0x63, 0x19, GADI_SENSOR_VER_CTL_REG_INIT},
                {0x27, 0x49, GADI_SENSOR_VER_CTL_REG_FMT},
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
        [1] =
        {
            .version = 0x80,
            .mask    = 0xFF,
            .regs =
            {
                {0x2C, 0x00, GADI_SENSOR_VER_CTL_REG_INIT},
                {0x48, 0x40, GADI_SENSOR_VER_CTL_REG_INIT},
                {0x63, 0x19, GADI_SENSOR_VER_CTL_REG_INIT},
                {0x27, 0x49, GADI_SENSOR_VER_CTL_REG_FMT},
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
        [2] =
        {
            .version = 0x81,
            .mask    = 0xFF,
            .regs =
            {
                {0x2C, 0x04, GADI_SENSOR_VER_CTL_REG_INIT},
                {0x48, 0x60, GADI_SENSOR_VER_CTL_REG_INIT},
                {0x63, 0x51, GADI_SENSOR_VER_CTL_REG_INIT},
                {0x27, 0x3B, GADI_SENSOR_VER_CTL_REG_FMT},
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
    },

    .vmax_clk_ref   = GADI_SENSOR_CAL_CLK_PIXCLK_DIV2,
    .shs_clk_ref    = GADI_SENSOR_CAL_CLK_PIXCLK_DIV2,
    .cal_shs_mode   = GADI_SENSOR_CAL_SHS_VMAX_MODE,
    .shs_fix        = 0,
    .shs_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_VSYNC,
    .agc_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_VSYNC,
    .vmax_reg_update_mode       = GADI_SENSOR_UPDATE_REG_BY_VSYNC,
    .mirror_reg_update_mode     = GADI_SENSOR_UPDATE_REG_BY_VSYNC,
    .shs_additional_use         = 0,
    .shs_additional_percent     = 0,
    // hmax = (FRAMEW_H & 0x3F) << 8 +
    //        (FRAMEW_L & 0xFF) << 0
    .hmax_reg =
    {
        {0x21, 0x00, 0x3F, 0x00, 0x08, 0}, // FRAMEW_H
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
        {0x02, 0x00, 0xFF, 0x00, 0x08, 0}, // EXP_H
        {0x01, 0x00, 0xFF, 0x00, 0x00, 0}, // EXP_L
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
        {0x18000000, {0x30, 0x51,}}, //24.000000
        {0x188657c9, {0x31, 0x51,}}, //24.524777
        {0x1905013a, {0x32, 0x51,}}, //25.019550
        {0x197cd110, {0x33, 0x51,}}, //25.487565
        {0x19ee7b47, {0x34, 0x51,}}, //25.931569
        {0x1a5a997c, {0x35, 0x51,}}, //26.353905
        {0x1ac1afdd, {0x36, 0x51,}}, //26.756590
        {0x1b2430f0, {0x37, 0x51,}}, //27.141372
        {0x1b82809d, {0x38, 0x51,}}, //27.509775
        {0x1bdcf68e, {0x39, 0x51,}}, //27.863137
        {0x1c33e01a, {0x3a, 0x51,}}, //28.202638
        {0x1c8781d8, {0x3b, 0x51,}}, //28.529325
        {0x1cd818df, {0x3c, 0x51,}}, //28.844130
        {0x1d25dbda, {0x3d, 0x51,}}, //29.147886
        {0x1d70fbe4, {0x3e, 0x51,}}, //29.441344
        {0x1db9a541, {0x3f, 0x51,}}, //29.725178

        {0x1db9a541, {0x40, 0x51,}}, //29.725178
        {0x128657c9, {0x41, 0x51,}}, //18.524777
        {0x1305013a, {0x42, 0x51,}}, //19.019550
        {0x137cd110, {0x43, 0x51,}}, //19.487565
        {0x13ee7b47, {0x44, 0x51,}}, //19.931569
        {0x145a997c, {0x45, 0x51,}}, //20.353905
        {0x14c1afdd, {0x46, 0x51,}}, //20.756590
        {0x152430f0, {0x47, 0x51,}}, //21.141372
        {0x1582809d, {0x48, 0x51,}}, //21.509775
        {0x15dcf68e, {0x49, 0x51,}}, //21.863137
        {0x1633e01a, {0x4a, 0x51,}}, //22.202638
        {0x168781d8, {0x4b, 0x51,}}, //22.529325
        {0x16d818df, {0x4c, 0x51,}}, //22.844130
        {0x1725dbda, {0x4d, 0x51,}}, //23.147886
        {0x1770fbe4, {0x4e, 0x51,}}, //23.441344
        {0x17b9a541, {0x4f, 0x51,}}, //23.725178
    },
    .temperatue_gain_table =
    {
        .agc_control_by_temperatue      = 0,
        .Aptina_temperatue_mode        = 0,
        .temperatue_control_reg_index   = 0,
        .temperatue_gain_num            = 0,
        .temperatue_reg                 = {GADI_VI_SENSOR_TABLE_FLAG_END,},
        .gain_reg                       = {GADI_VI_SENSOR_TABLE_FLAG_END,},
        .temperatue_gain                = {0},
        .temperatue_control             = {0},
    },
    .magic_end  = GADI_SENSOR_HW_INFO_MAGIC_END,
};

