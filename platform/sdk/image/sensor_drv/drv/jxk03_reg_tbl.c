/*!
*****************************************************************************
** \file        jxk03_reg_tbl.c
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
GADI_VI_SensorDrvInfoT    jxk03_sensor =
{
    .magic_start        = GADI_SENSOR_HW_INFO_MAGIC_START,
    .version            = GADI_SENSOR_HW_INFO_VERSION,
    .length             = sizeof(GADI_VI_SensorDrvInfoT),
    .file_date          = 0x20171016,
    .HwInfo         =
    {
        .name               = "jxk03",
        .ctl_type           = GADI_SENSOR_CTL_I2C,
        .addr_len           = sizeof(GADI_U8),
        .data_len           = sizeof(GADI_U8),
        .reset_active_level = GADI_GPIO_LOW,
        .hw_addr            = {(0x80>>1), (0x84>>1), (0x88>>1), (0x8C>>1)},
        .id_reg =
        {
            {0x0A, 0x05},   // PIDH[7:0]:Product ID MSBs. 0x05
            {0x0B, 0x03},   // PIDL[7:0]:Product ID LSBs. 0x03
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
                            // SYS[2:0]: Down-Sample mode:
                            //      000: Full;
                            //      001: VBin,HBin;
                            //      010: V-FastBin,HBin;
                            //      011: VSkip,HSkip;
                            //      101: VBin, HSkip;
                            //      110: V-FastBin,HSkip,
                            //      111: VSkip,HBin
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
    .sensor_id          = GADI_ISP_SENSOR_JXK03,
    .sensor_double_step = 20,
    .typeofsensor       = 1,
    .sync_time_delay    = 40,
    .source_type.dummy  = GADI_VI_CMOS_CHANNEL_TYPE_AUTO,
    .dev_type           = GADI_VI_DEV_TYPE_CMOS,
    .video_system       = GADI_VIDEO_SYSTEM_AUTO,
    .vs_polarity        = GADI_VI_RISING_EDGE,
    .hs_polarity        = GADI_VI_RISING_EDGE,
    .data_edge          = GADI_VI_FALLING_EDGE,
    .emb_sync_switch    = GADI_VI_EMB_SYNC_OFF,
    .emb_sync_loc       = GADI_VI_EMB_SYNC_LOWER_PEL,
    .emb_sync_mode      = GADI_VI_EMB_SYNC_FULL_RANGE,
    .max_width          = 2704,
    .max_height         = 1950,
    .def_sh_time        = GADI_VIDEO_FPS(50),
    .fmt_num            = 6,
    .auto_fmt           = GADI_VIDEO_MODE(2560,  1440,  15, 1),
    .init_reg           =
    {
        // 0x03~0x04    // RSVD
        //{0x05, 0x00}, // Short exposure lines in HDR mode,
                        // EXP_S[7:0]: each bit equals to 2 lines, short_exposure_time = (EXP_S[7:0] *2 + 1) * TLine
        // 0x06~0x09    // RSVD
        {0x12, 0x40},   // SYS[7]: Soft reset initialize, 1: initial system reset,
                        //      it will reset whole sensor to factory default status, and
                        //      this bit will clear after reset.
                        // SYS[6]: Sleep mode on/off selection, 1: sensor into sleep mode.
                        //      No data output and all internal operation stops.
                        //      External controller can stop sensor clock, while I2C interface still can work.
                        // SYS[5]: mirror image on/off, 1: mirrored image output, 0: normal image output
                        // SYS[4]: flip image on/off, 1: flipped image output, 0: normal image output.
                        // SYS[3]: HDR mode on/off selection. 0: normal mode, 1: HDR mode.
                        // SYS[2:0]: Down-Sample mode:
                        //      000: Full;
                        //      001: VBin,HBin;
                        //      010: V-FastBin,HBin;
                        //      011: VSkip,HSkip;
                        //      101: VBin, HSkip;
                        //      110: V-FastBin,HSkip,
                        //      111: VSkip,HBin
        {0x0D, 0x07},   // DVP2[7:4]: RSVD
                        // DVP2[3:2]: PAD drive capability. 00: min, 11: max.
                        // DVP2[1]: AGC Latch at Frame End
                        // DVP2[0]: AGC Latch at Frame Start
        //{0x13, 0x30}, // LCCtrl1[0]: automatic luminance control on/off selection, 0:, manual; 1: auto
        //{0x14, 0x64}, // LCCtrl2[7:0]: Image luminance expect target.
        //{0x15, 0x24}, // LCCtrl3[7:4]: auto Luminance control adjustment trigger range. LCCtrl3[3:0]: auto luminance control stable range
        //{0x16, 0xFF}, // LCCtrl4[7:4]: Luminance convergence low-speed step, [3]: 16/16, [2:0]: 8/16 ~ 15/16
                        // LCCtrl4[3:0]: Luminance convergence high-speed step, [3]: 16/16, [2:0]: 8/16 ~ 15/16
        // 0x17~0x1B    // RSVD
        //{0x1C, 0x00}, // read only LCCtrl10[7:0]: Image luminance average value.
        {0x1D, 0xFF},   // DVP3[7:0]: GPIO direction control for data output port D[7:0]. 1: enable output. 0: tri-state output.
        {0x1E, 0x1F},   // DVP4[7]: PCLK polarity control.
                        // 0: data output at rising edge of PCLK, 1:data output at falling edge of PCLK
                        // DVP4[6]: HREF polarity control."1": Negative."0": Positive
                        // DVP4[5]: VSYNC polarity control. "1": Negative."0": Positive
                        // DVP4[4:0]: GPIO direction control for output port: PCLK, HREF, VSYNC and D[9:8]
                        // "1": Enable output."0": Tri-state output
        //{0x1F, 0x20}, // Group latch control
                        // GLat[7]: Group write trigger. 1: system will write reg0xC0 to 0xFF content to proper register.
                        // This bit will clear after group write. 0: inactive group write function.
                        // GLat[6]: Group latch trigger time option, 0: trigger at vertical blanking period.
                        // 1: group latch trigger immediately.
                        // GLat[5:4]: PCLK delay control
                        // GLat[3:2]: RSVD
                        // GLat[1]: Precharge off/on, 0: normal, 1: no precharge when PGA >=4x
                        // GLat[0]: RSVD
        {0x2A, 0x40},   // Column shift control 1 Cshift1[7:6]: Column SRAM data shift start position LSBs.
        // 0x31~0x38    // RSVD
{0x32, 0x06},
{0x33, 0x09},
{0x34, 0x1C},
{0x35, 0x87},
        //{0x39, 0x41},
        {0x39, 0x01},   // Sensor timing control 9
                        // SenT9[7]: HDR Short Gain option 0: follow Long Exposure Gain, 1: always x1 Gain.
                        // SenT9[6]: Sensor format change option, 1: reg0x12[6:0] valid any timing when I2C applied.
                        //      0:reg0x12[6:0] valid only at new frame start.
                        // SenT9[5]: RSVD
                        // SenT9[4]: Frame Sync Enable, 0: disable, 1 enable
                        // SenT9[3:0]: RSVD
        // 0x3A~0x40    // RSVD
        //{0x41, 0x00}, // VS_POS VSYNC position LSBs: VS_POS[7:0]
        //{0x42, 0x40}, // [7:4]: VSYNC width selection. [3:0]: VS_POS[11:8]
        //{0x43, 0x00}, // DGain0 Color Red Digital Gain LSB, DG_R[7:0]
        //{0x44, 0x00}, // DGain1 Color Gr Digital Gain LSB, DG_Gr[7:0]
        //{0x45, 0x00}, // DGain2 Color Gb Digital Gain LSB, DG_Gb[7:0]
        //{0x46, 0x00}, // DGain3 Color Blue Digital Gain LSB, DG_B[7:0]
        //{0x47, 0x00}, // Dgain4[7:6]: Color Blue Digital Gain MSB,DG_B[9:8]
                        // Dgain4[5:4]: Color Gb Digital Gain MSB, DG_Gb[9:8]
                        // Dgain4[3:2]: Color Gr Digital Gain MSB, DG_Gr[9:8]
                        // Dgain4[1:0]: Color Red Digital Gain MSB, DG_R[9:8]
        {0x48, 0x81},   // Dgain5[7:3]: RSVD
                        // Dgain5[2:1]: Digital gain code as digital gain step
                        //      '00': U2.8 (step 1/256);;'01': U4.6 (step 1/64)
                        //      '10': U6.4 (step 1/16) ;;'11': U8.2 (step 1/4)
                        // Dgain5[0]: Digital gain bypass
                        //      '1': Digital gain off. '0': Digital gain on.
        {0x49, 0x10},   // Black level calibration target level.
                        // BLC_TGT[7]: RSVD
                        // BLC_TGT[6:0]: target level.
        //{0x4A, 0x03}, // BLC control
                        // BLCCtrl[7]: BLC_B bit 10
                        // BLCCtrl[6]: BLC_Gb bit 10
                        // BLCCtrl[5]: BLC_Gr bit 10
                        // BLCCtrl[4]: BLC_R bit 10
                        // BLCCtrl[3:0]: RSVD
        //{0x4B, 0x35}, // BLC_B B channel black value LSBs. BLC_B[7:0]
        //{0x4C, 0x34}, // BLC_Gb Gb channel black value LSBs. BLC_Gb[7:0]
        //{0x4D, 0x34}, // BLC_Gr Gr channel black value LSBs. BLC_Gr[7:0]
        //{0x4E, 0x34}, // BLC_R R channel black value LSBs. BLC_R[7:0]
        //{0x4F, 0x00}, // Black value MSBs. {BLC_R[9:8],BLC_Gr[9:8],BLC_Gb[9:8],BLC_B[9:8]}
        //{0x50, 0x03}, // Short exposure BLC control in HDR mode
                        // SBLCCtrl[7]: SBLC_B bit 10
                        // SBLCCtrl[6]: SBLC_Gb bit 10
                        // SBLCCtrl[5]: SBLC_Gr bit 10
                        // SBLCCtrl[4]: SBLC_R bit 10
                        // SBLCCtrl[3:0]: RSVD
        //{0x51, 0x00}, // SBLC_B  Short exposure B channel black value LSBs. SBLC_B[7:0]
        //{0x52, 0x00}, // SBLC_Gb Short exposure Gb channel black value LSBs. SBLC_Gb[7:0]
        //{0x53, 0x00}, // SBLC_Gr Short exposure Gr channel black value LSBs. SBLC_Gr[7:0]
        //{0x54, 0x00}, // SBLC_R Short exposure R channel black value LSBs. SBLC_R[7:0]
        //{0x55, 0x00}, // Short exposure black value MSBs. {SBLC_R[9:8],SBLC_Gr[9:8],SBLC_Gb[9:8],SBLC_B[9:8]}
        // 0x56~0x5E    // RSVD
{0x58, 0x90},   // RSVD
{0x5B, 0x57},   // RSVD
{0x5D, 0x2F},   // RSVD
{0x5E, 0x83},   // RSVD
        {0x5F, 0x01},   // DAC PLL control 0
                        // DACPLL0[7]: DAC PLL bypass on/off selection
                        // DACPLL0[6]: RSVD
                        // DACPLL0[5:4]: DAC PLL current control
                        // DACPLL0[3:2]: DAC PLL post divider
                        // DACPLL0[1:0]: DAC PLL pre-divider
        {0x60, 0x0F},   // DAC PLL control 1
                        // DACPLL1[7:6]: RSVD
                        // DACPLL1[5:0]: DAC PLL VCO multiplier
        // 0x61~0x6A    // RSVD
{0x62, 0xC4},   // RSVD
{0x63, 0x00},   // RSVD
{0x64, 0x14},   // RSVD
{0x66, 0x04},   // RSVD
{0x67, 0x38},   // RSVD
{0x68, 0x00},   // RSVD
{0x6A, 0x4F},   // RSVD
        //{0x6B, 0x00}, // Mipi PHY control 1
                        // DPHY1[7:6]: MIPI internal byte clock skew control
                        // DPHY1[5:4]: MIPI internal high speed clock skew control
                        // DPHY1[3:2]: MIPI clock lane skew control
                        // DPHY1[1:0]: MIPI data lane0 skew control
        {0x6C, 0x80},   // DPHY2[7]: Mipi interface power down.
                        // "0" MIPI interface power down disable; for MIPI normal operation setting
                        // "1" MIPI interface power down enable; for DVP normal operation setting.
                        // DPHY2[6:5]:RSVD
                        // DPHY2[4]: MIPI PHY clock option, data valid sampling clock, 0: keep 1: reverse
                        // DPHY2[3:2]: RSVD
                        // DPHY2[1:0]: MIPI data lane1 skew control
        //{0x6D, 0x02}, // DPHY3[7:6]:RSVD
                        // DPHY3[5:4]: MIPI low power mode option
                        // DPHY3[3:0]: RSVD
        // 0x6E         // RSVD
        //{0x6F, 0x00}, // DPHY5[7]: MIPI data lane0 disable control
                        // DPHY5[6]: MIPI data lane1 disable control
                        // DPHY5[5]: MIPI data lane2 disable control
                        // DPHY5[4]: MIPI data lane3 disable control
                        // DPHY5[3:2]: MIPI data lane3 skew control
                        // DPHY5[1:0]: MIPI data lane2 skew control
        {0x70, 0x49},   // Mipi timing control 1
                        // Mipi1[7:5]: Tlpx control
                        // Mipi1[4:2]: Tck-preP, before CK_ZERO
                        // Mipi1[1]: Mipi 8/10-bit mode switch, 1: 10-bit, 0: 8-bit mode
                        // Mipi1[0]: RSVD
        //{0x71, 0x8A}, // Mipi timing control 2 Mipi2[7:5]: Ths-zero Mipi2[4:0]: Tck-zero
        //{0x72, 0x68}, // Mipi timing control 3 Mipi3[7:5]: Ths-prepare Mipi3[4:0]: Tck-post
        //{0x73, 0x33}, // Mipi timing control 4 Mipi4[7]: Mipi pixel clock option Mipi4[6:4]: Ths-trail Mipi4[3:0]: Tck-trail
        //{0x74, 0x83}, // Mipi timing control 5
                        // Mipi5[7:6]: Mipi lanes option, 11 RSVD, 10 4lanes, 01 2lanes, 00 1lane
                        // Mipi5[5]: Manual Virtual ID for HDR short packet, bit1
                        // Mipi5[4]: 0 sleep off with FS; 1: sleep off with de skew; depends on de skew on/off, Reg75[5:4]
                        // Mipi5[3]: Mipi HS ck_d[1:0] to D_PHY, 0: 10b, 1: 01b
                        // Mipi5[2]: Manual Virtual ID for HDR short packet, bit0
                        // Mipi5[1]: Mipi continues mode or strobe mode selection; 0: strobe mode; 1: continuous mode
                        // Mipi5[0]: Mipi interface sleep on/off. 0:MIPI sleep off; 1:MIPI sleep on.
        //{0x75, 0x03}, // Mipi6[7]: 0: VC Based HDR 1: Single Frame HDR
                        // Mipi6[6]: 1: When Single Frame HDR, frame with short head dummy and long tail dummy
                        // Mipi6[5]: 0: MIPI VC with normal(long)/short 01/10; 1: MIPI VC with normal(long)/short 00/01
                        // Mipi6[4]: RSVD
                        // Mipi6[3:0]: MipiTck-pre, after CK_ZERO
        {0x76, 0x00},   // Mipi word count LSBs
        //{0x78, 0x07}, // Mipi timing control 9
                        // Mipi9[7]: MIPI Virtual ID option; 0: auto, 1: manual
                        // Mipi9[6:0]: Mipi TX start point adjust related to DVP HREF and internal FIFO
        // 0x79~0x7F    // RSVD
        {0x79, 0x90},   // RSVD
        {0x7B, 0x0C},   // RSVD
        {0x7C, 0x2F},   // RSVD
        {0x7D, 0x42},   // RSVD
        {0x80, 0x80},   // BLCCtrl0 [7:2]: RSVD
                        // BLCCtrl0 [1]: Digital BLC , ¡°0¡±: enable, ¡°1¡± disable
                        // BLCCtrl0 [0]: Analog BLC, ¡°0¡±: enable, ¡°1¡± disable
        {0x81, 0x54},   // BLCCtrl1 [7:2]: RSVD
                        // BLCCtrl1 [1]: Digital BLC refresh mode, ¡°0¡±: trigger, ¡°1¡±: always
                        // BLCCtrl1 [0]: Analog BLC refresh mode, ¡°0¡±: trigger, ¡°1¡±: always
        {0x82, 0x0F},   // BLCCtrl2[7:0]*8, Digital BLC max limit
        {0x83, 0x0A},   // BLCCtrl3[7:0]*8, Digital BLC min limit
        {0x84, 0x04},   // RSVD
        {0x85, 0x80},   // BLCCtrl5 [7:6]: RSVD
                        // BLCCtrl5 [5:0]: DC ratio, DC pixel = DC OB*(1¡ÀN/16) [5]: sign bit; [4:0] ratio.
        // 0x86~0x8B    // RSVD
        {0x89, 0x00},   // RSVD
        //{0x8C, 0x20}, // BSUN option [7]: BSUN programmable ¡°0¡±: off ¡°1¡±: on
                        // [6:1]: RSVD
                        // [0]: Black sun on/off. ¡°0¡±: off ¡°1¡±: on
        {0x8D, 0x20},   // RSVD
        //{0x8E, 0x00}, // Deskew0 MIPI de-skew calibration sync code (eg. Set as 0xFF)
        //{0x8F, 0x00}, // Deskew1 MIPI de-skew calibration code (eg. Set as 0x55)
        // 0x90~0xB1    // RSVD
        {0x91, 0x56},   // RSVD
        {0xAA, 0x4C},   // RSVD
        {0xAB, 0x6C},   // RSVD
        {0xAD, 0x44},   // RSVD
        {0xAE, 0x0E},   // RSVD
        {0xAF, 0x04},   // RSVD
        // 0xC0~0xFF    // Group write 1st~32nd data address
        {0x12, 0x00},
        {GADI_VI_SENSOR_TABLE_FLAG_END,}
    },
    .pll_table =
    {
        [0] =
        {
            // for 2048x1536P@30, 2048x1536P@25
            .pixclk = 43200000,//86400000,
            .extclk = 27000000,
            .regs = {

                                // PLLclk = sysclk/DVP1[7:6] = PLLclk*(1+CLK[6])/2/DVP1[7:6]
                                //        = 432000000 * (1 + 0) / 2 / 2 = 86400000
                {0x0C, 0x80},   // DVP1[7:2]: RSVD
                                // DVP1[1:0]: DVP test mode output option.
                                // 00: DVP output normal image data.
                                // 01: DVP[9:0] = Output 10-bit walk 1 test pattern.
                                // 10:,11: RSVD
                {0x0E, 0x10},   // PLL1[7]: PLL bypass selection. 1: PLLclk = Input clock/PLL_Pre_Ratio. 0: see PLL2[3:0].
                                // PLL1[6:4]: PLL current adjustment.
                                // PLL1[3]: PLL power down for mipi
                                // PLL1[2]: external clock input pad circuit option, 0: internal no Schmitt trigger; 1: internal has Schmitt trigger.
                                // PLL1[1:0]: PLL pre-divider ratio. PLL_Pre_Ratio = 1 + PLL[1:0]
                {0x0F, 0x20},   // PLL2[7:6]: DVP_PCLK clock divider. 0:by 1, 1: by2, 2: by 4, 3: by8
                                // PLL2[5:4]: SRAM_CLK clock divider. 0:by 1, 1: by2, 2: by 4, 3: by8
                                // PLL2[3]: Mipi clock divider. Mipiclk = VCO/(1+PLL2[3])
                                //                                      = 432000000/(1 + 0) = 432000000
                                // PLL2[1:0]: Sys clock divider. PLLclk = VCO/(1+PLL2[3:0])
                                //                                      = 432000000/(1 + 0) = 432000000
                {0x10, 0x20},   // PLL3 VCO = Input clock*PLL3[7:0]/PLL_Pre_Ratio
                                //     = (27000000*32/2)=432000000
                {0x11, 0x80},   // CLK[7]: System clock option. 1: system clock use PLLclk directly. 0: system clock use PLLclkafter digital divide clock.
                                // CLK[6]: system clock digital doubler on/off selection. 1: on, 0: off
                                // CLK[5:0]: system clock divide ratio.
                                // Equation:
                                // When CLK[5:0] > 0: System clock = PLLclk *(1+CLK[6])/(2*CLK[5:0])
                                // When CLK[5:0] = 0: System clock = PLLclk*(1+CLK[6])/2
                {GADI_VI_SENSOR_TABLE_FLAG_END,}
            }
        },
    },
    .video_fmt_table =
    {
        [0] = // 2560x1440@15fps (750 * 4) * 1920 * 15 = 86400000  1920x1440 2400x1920
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(2560,  1440,  15, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x12, 0x40, 0x40},
                {0x20, 0xEE},   // FrameW_L
                {0x21, 0x02},   // FrameW_H     0x2EE = 750 * 4 = 3000
                //{0x22, 0x80}, // FrameH_H
                //{0x23, 0x07}, // FrameH_L     0x780 = 1920
                {0x24, 0x80},   // HWin_L       0x280 = 640 * 4 = 2560
                {0x25, 0xA0},   // VWin_L       0x5A0 = 1440
                {0x26, 0x52},   // VWin_H HWin_H
                {0x27, 0x61},   // HWinSt_L     0x161 = 353     0x153 < HWinSt< 0x175 0x153 < HWinSt< 0x168
                {0x28, 0xDB},   // VWinSt_L     0x1DB/0x1D0 = 475/464
                {0x29, 0x11},   // VWinSt_L HWinSt_H
                {0x2B, 0x21},   // [7:6]SenHASt_H
                {0x2C, 0x0E},   // SenHASt_L    0x00E =  14 * 4 =   56
                {0x2D, 0x00},   // SenVSt_L     0x000 =   0 * 4 =    0
                {0x2E, 0xDF},   // SenVEnd_L    0x1DF = 479 * 4 = 1916
                {0x2F, 0x94},   // [7:6]SenHWid_H [3:2]SenVEnd_H [1:0]SenVSt_H
                {0x30, 0xA4},   // SenHWid_L    0x2A4 = 676 * 4 = 2704
                {0x77, 0x0A},
                {0x87, 0xB7},
                {0x88, 0x0F},
                {0x12, 0x00, 0x40},
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 2560,
                .height     = 1440,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_DVP, 
                .bits       = GADI_VIDEO_BITS_10,
                .ratio      = GADI_VIDEO_RATIO_16_9,
                .srm        = 0,
                .pll_index  = 0,
                .max_fps    = GADI_VIDEO_FPS(15),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(15),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_1080P_25,
            },
            .video_info_table =
            {
                .def_start_x    = 0,
                .def_start_y    = 0,
                .def_width      = 2560,
                .def_height     = 1440,
                .sync_start     = 0,
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
        [1] = // 2304x1728@15fps (720 * 4) * 2000 * 15 = 86400000  1920x1440 2400x1920
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(2304,  1728,  15, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x12, 0x40, 0x40},
                {0x20, 0xD0},   // FrameW_L
                {0x21, 0x02},   // FrameW_H     0x2D0 = 720 * 4 = 2880
                //{0x22, 0xD0}, // FrameH_H
                //{0x23, 0x07}, // FrameH_L     0x7D0 = 2000
                {0x24, 0x40},   // HWin_L       0x240 = 576 * 4 = 2304
                {0x25, 0xC0},   // VWin_L       0x6C0 = 1728
                {0x26, 0x62},   // VWin_H HWin_H
                {0x27, 0x80},   // HWinSt_L     0x180 = 384     0x180 < HWinSt< 0x1B0  0x150 < HWinSt < 0x180
                {0x28, 0x8F},   // VWinSt_L     0x08B/0x080 = 139/128
                {0x29, 0x01},   // VWinSt_L HWinSt_H
                {0x2B, 0x21},   // [7:6]SenHASt_H
                {0x2C, 0x2D},   // SenHASt_L    0x02D =  45 * 4 =  180
                {0x2D, 0x00},   // SenVSt_L     0x000 =   0 * 4 =    0
                {0x2E, 0xEF},   // SenVEnd_L    0x1EF = 495 * 4 = 1980
                {0x2F, 0x94},   // [7:6]SenHWid_H [3:2]SenVEnd_H [1:0]SenVSt_H
                {0x30, 0xA4},   // SenHWid_L    0x2A4 = 676 * 4 = 2704
                {0x77, 0x09},
                {0x87, 0xB7},
                {0x88, 0x0F},
                {0x12, 0x00, 0x40},
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 2304,
                .height     = 1728,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_DVP, 
                .bits       = GADI_VIDEO_BITS_10,
                .ratio      = GADI_VIDEO_RATIO_4_3,
                .srm        = 0,
                .pll_index  = 0,
                .max_fps    = GADI_VIDEO_FPS(15),
                .min_fps    = GADI_VIDEO_FPS(1),
                .auto_fps   = GADI_VIDEO_FPS(15),
                .dgain_type = GADI_ISP_TUNNING_DATA_AE_SHT_DGAIN_1080P_25,
            },
            .video_info_table =
            {
                .def_start_x    = 0,
                .def_start_y    = 0,
                .def_width      = 2304,
                .def_height     = 1728,
                .sync_start     = 0,
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
        [2] = // 2048x1536@25fps (540 * 4) * 1600 * 25 = 86400000   encode max 21fps
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(2048,  1536,  25, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x12, 0x40, 0x40},
                {0x20, 0x1C},   // FrameW_L
                {0x21, 0x02},   // FrameW_H     0x21C = 540 * 4 = 2160
                //{0x22, 0x40}, // FrameH_H
                //{0x23, 0x06}, // FrameH_L     0x640 = 1600
                {0x24, 0x00},   // HWin_L       0x200 = 512
                {0x25, 0x00},   // VWin_L       0x600 = 1536
                {0x26, 0x62},   // VWin_H HWin_H
                {0x27, 0x51},   // HWinSt_L     0x151 = 337
                {0x28, 0x3B},   // VWinSt_L     0x03B/0x030 = 59/48
                {0x29, 0x01},   // VWinSt_L HWinSt_H
                {0x2B, 0x21},   // [7:6]SenHASt_H
                                // [4]: DVP_PCLK delay option
                                // [3]: Short Exposure Latch, 0: EXP[15:0] delay one frame valid,
                                //      1: EXP[15:0] valid when write
                                // [2]: Long Exposure Latch, 0: EXP[15:0] Frame Start Reflection,
                                //      1: EXP[15:0] Reflection when write
                                // [1:0]: SRAM data shift start position MSBs
                {0x2C, 0x51},   // SenHASt_L    0x051 =  81 * 4 =  324
                {0x2D, 0x00},   // SenVSt_L     0x000 =   0 * 4 =    0
                {0x2E, 0x8F},   // SenVEnd_L    0x18F = 399 * 4 = 1596
                {0x2F, 0x94},   // [7:6]SenHWid_H [3:2]SenVEnd_H [1:0]SenVSt_H
                {0x30, 0x1B},   // SenHWid_L    0x21B = 539 * 4 = 2156
                {0x77, 0x08},   // Mipi word count MSBs
                {0x87, 0xA6},
                {0x88, 0x0E},
                {0x12, 0x00, 0x40},
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 2048,
                .height     = 1536,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_DVP, 
                .bits       = GADI_VIDEO_BITS_10,
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
                .def_width      = 2048,
                .def_height     = 1536,
                .sync_start     = 0,
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
        [3] = // 2048x1536@25fps (540 * 4) * 1600 * 25 = 86400000   encode max 21fps
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1504,  1500,  25, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x12, 0x40, 0x40},
                {0x20, 0x1C},   // FrameW_L
                {0x21, 0x02},   // FrameW_H     0x21C = 540 * 4 = 2160
                //{0x22, 0x40}, // FrameH_H
                //{0x23, 0x06}, // FrameH_L     0x640 = 1600
                {0x24, 0x00},   // HWin_L       0x200 = 512
                {0x25, 0x00},   // VWin_L       0x600 = 1536
                {0x26, 0x62},   // VWin_H HWin_H
                {0x27, 0x51},   // HWinSt_L     0x151 = 337
                {0x28, 0x3B},   // VWinSt_L     0x03B/0x030 = 59/48
                {0x29, 0x01},   // VWinSt_L HWinSt_H
                {0x2B, 0x21},   // [7:6]SenHASt_H
                                // [4]: DVP_PCLK delay option
                                // [3]: Short Exposure Latch, 0: EXP[15:0] delay one frame valid,
                                //      1: EXP[15:0] valid when write
                                // [2]: Long Exposure Latch, 0: EXP[15:0] Frame Start Reflection,
                                //      1: EXP[15:0] Reflection when write
                                // [1:0]: SRAM data shift start position MSBs
                {0x2C, 0x51},   // SenHASt_L    0x051 =  81 * 4 =  324
                {0x2D, 0x00},   // SenVSt_L     0x000 =   0 * 4 =    0
                {0x2E, 0x8F},   // SenVEnd_L    0x18F = 399 * 4 = 1596
                {0x2F, 0x94},   // [7:6]SenHWid_H [3:2]SenVEnd_H [1:0]SenVSt_H
                {0x30, 0x1B},   // SenHWid_L    0x21B = 539 * 4 = 2156
                {0x77, 0x08},   // Mipi word count MSBs
                {0x87, 0xA6},
                {0x88, 0x0E},
                {0x12, 0x00, 0x40},
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
            .vi_info =
            {
                .width      = 1504,
                .height     = 1500,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_DVP, 
                .bits       = GADI_VIDEO_BITS_10,
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
                .def_width      = 1504,
                .def_height     = 1500,
                .sync_start     = 0,
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
        [4] = // 1440x1440@15fps (450 * 4) * 3200 * 15 = 86400000   encode max 21fps
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1440,  1440,  25, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
				{0x12, 0x40, 0x40},
				{0x20, 0xc2},	// FrameW_L
				{0x21, 0x01},	// FrameW_H 	0x1c2 = 450 * 4 = 1800	//1f4
				//{0x22, 0x40}, // FrameH_H
				//{0x23, 0x06}, // FrameH_L 	0xc80 = 3200
				{0x24, 0x68},	// HWin_L		0x168 = 360
				{0x25, 0xa0},	// VWin_L		0x5A0 = 1440
				{0x26, 0x51},	// VWin_H HWin_H
				{0x27, 0x51},	// HWinSt_L 	0x151 = 337
				{0x28, 0x3B},	// VWinSt_L 	0x03B/0x030 = 59/48
				{0x29, 0x01},	// VWinSt_L HWinSt_H
				{0x2B, 0x21},	// [7:6]SenHASt_H
								// [4]: DVP_PCLK delay option
								// [3]: Short Exposure Latch, 0: EXP[15:0] delay one frame valid,
								//		1: EXP[15:0] valid when write
								// [2]: Long Exposure Latch, 0: EXP[15:0] Frame Start Reflection,
								//		1: EXP[15:0] Reflection when write
								// [1:0]: SRAM data shift start position MSBs
				{0x2C, 0x51},	// SenHASt_L	0x051 =  81 * 4 =  324
				{0x2D, 0x00},	// SenVSt_L 	0x000 =   0 * 4 =	 0
				{0x2E, 0x77},	// SenVEnd_L	0x18F = 399 * 4 = 1596
				{0x2F, 0x54},	// [7:6]SenHWid_H [3:2]SenVEnd_H [1:0]SenVSt_H
				{0x30, 0xc1},	// SenHWid_L	0x1c1 = 449 * 4 = 1796
				{0x77, 0x08},	// Mipi word count MSBs
				{0x87, 0xA6},
				{0x88, 0x0E},
				{0x12, 0x00, 0x40},
				{GADI_VI_SENSOR_TABLE_FLAG_END,},

            },
            .vi_info =
            {
                .width      = 1440,
                .height     = 1440,
                .format     = GADI_VIDEO_FORMAT_PROGRESSIVE,
                .type       = GADI_VIDEO_TYPE_RGB_RAW,
                .interface  = GADI_VIDEO_IN_DVP, 
                .bits       = GADI_VIDEO_BITS_10,
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
                .def_width      = 1440,
                .def_height     = 1440,
                .sync_start     = 0,
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
        [5] = // 1920x1080@25fps (540 * 4) * 1600 * 25 = 86400000   encode max 21fps
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1920,  1080,  25, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x12, 0x40, 0x40},
                {0x20, 0x1C},   // FrameW_L
                {0x21, 0x02},   // FrameW_H     0x21C = 540 * 4 = 2160
                //{0x22, 0x40}, // FrameH_H
                //{0x23, 0x06}, // FrameH_L     0x640 = 1600
                {0x24, 0x00},   // HWin_L       0x200 = 512
                {0x25, 0x00},   // VWin_L       0x600 = 1536
                {0x26, 0x62},   // VWin_H HWin_H
                {0x27, 0x51},   // HWinSt_L     0x151 = 337
                {0x28, 0x3B},   // VWinSt_L     0x03B/0x030 = 59/48
                {0x29, 0x01},   // VWinSt_L HWinSt_H
                {0x2B, 0x21},   // [7:6]SenHASt_H
                                // [4]: DVP_PCLK delay option
                                // [3]: Short Exposure Latch, 0: EXP[15:0] delay one frame valid,
                                //      1: EXP[15:0] valid when write
                                // [2]: Long Exposure Latch, 0: EXP[15:0] Frame Start Reflection,
                                //      1: EXP[15:0] Reflection when write
                                // [1:0]: SRAM data shift start position MSBs
                {0x2C, 0x51},   // SenHASt_L    0x051 =  81 * 4 =  324
                {0x2D, 0x00},   // SenVSt_L     0x000 =   0 * 4 =    0
                {0x2E, 0x8F},   // SenVEnd_L    0x18F = 399 * 4 = 1596
                {0x2F, 0x94},   // [7:6]SenHWid_H [3:2]SenVEnd_H [1:0]SenVSt_H
                {0x30, 0x1B},   // SenHWid_L    0x21B = 539 * 4 = 2156
                {0x77, 0x08},   // Mipi word count MSBs
                {0x87, 0xA6},
                {0x88, 0x0E},
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
                .bits       = GADI_VIDEO_BITS_10,
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
                .sync_start     = 0,
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
#define JXK03_V_FLIP    (1<<4)
#define JXK03_H_MIRROR  (1<<5)
    .mirror_table =
    {
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY_VERTICALLY] =
        {
            {0x12, 0x40, 0x40},
            {0x12, JXK03_H_MIRROR | JXK03_V_FLIP, JXK03_H_MIRROR | JXK03_V_FLIP},
            {0x28, 0x00, 0x0F},
            {0x12, 0x00, 0x40},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY] =
        {
            {0x12, 0x40, 0x40},
            {0x12, JXK03_H_MIRROR, JXK03_H_MIRROR | JXK03_V_FLIP},
            {0x28, 0x0B, 0x0F},
            {0x12, 0x00, 0x40},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_VERTICALLY] =
        {
            {0x12, 0x40, 0x40},
            {0x12, JXK03_V_FLIP, JXK03_H_MIRROR | JXK03_V_FLIP},
            {0x28, 0x00, 0x0F},
            {0x12, 0x00, 0x40},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_NONE] =
        {
            {0x12, 0x40, 0x40},
            {0x12, 0x00, JXK03_H_MIRROR | JXK03_V_FLIP},
            {0x28, 0x0B, 0x0F},
            {0x12, 0x00, 0x40},
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

    .vmax_clk_ref   = GADI_SENSOR_CAL_CLK_PIXCLK_DIV2,
    .shs_clk_ref    = GADI_SENSOR_CAL_CLK_PIXCLK_DIV2,
    .cal_shs_mode   = GADI_SENSOR_CAL_SHS_VMAX_MODE,
    .shs_fix        = 0,
    .shs_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_SET,
    .agc_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_SET,
    .vmax_reg_update_mode       = GADI_SENSOR_UPDATE_REG_BY_SET,
    .mirror_reg_update_mode     = GADI_SENSOR_UPDATE_REG_BY_SET,
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
    .max_agc_index = 96,
    .max_again_index = 0,
    .cur_again_index = 0,
    .agc_info =
    {
        .db_max     = 0x23D90BA2,   // 35.xdb     //bug bigger than 42db.
        .db_min     = 0x01000000,   // 1dB
        .db_step    = 0x00100000,   // 0.0625dB
    },
    .gain_reg =
    {
        {0x00, 0x00},   // Total gain = 2^PGA[7:4]*(1+PGA[3:0]/16)
        {0x0D, 0x00},   // DVP2[7:4]: RSVD
                        // DVP2[3:2]: PAD drive capability. 00: min, 11: max.
                        // DVP2[1]: AGC Latch at Frame End
                        // DVP2[0]: AGC Latch at Frame Start
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .gain_table =
    {
        /*REG_PGA addr 0x00;  bit[6:0];  again = (2^PGA[6:4])*(1+PGA[3:0]/16)*/
        /*REG_DVP2 addr 0x0d; bit[1:0];  dgain: bit[1:0]=00, no digital gain;  bit[1:0]=01 or 10, 2x;  bit[1:0]=11, 4x;*/
        /*Total gain = dgain*(2^PGA[6:4])*(1+PGA[3:0]/16)  */
        /*PGA   DVP2     agc_db    */ //   db
        {0x00000000, {0x00, 0x04,}}, //0.000000
        {0x008657C9, {0x01, 0x04,}}, //0.524777
        {0x0105013A, {0x02, 0x04,}}, //1.019550
        {0x017CD110, {0x03, 0x04,}}, //1.487565
        {0x01EE7B47, {0x04, 0x04,}}, //1.931569
        {0x025A997C, {0x05, 0x04,}}, //2.353905
        {0x02C1AFDD, {0x06, 0x04,}}, //2.756590
        {0x032430F0, {0x07, 0x04,}}, //3.141372
        {0x0382809D, {0x08, 0x04,}}, //3.509775
        {0x03DCF68E, {0x09, 0x04,}}, //3.863137
        {0x0433E01A, {0x0A, 0x04,}}, //4.202638
        {0x048781D8, {0x0B, 0x04,}}, //4.529325
        {0x04D818DF, {0x0C, 0x04,}}, //4.844130
        {0x0525DBDA, {0x0D, 0x04,}}, //5.147886
        {0x0570FBE4, {0x0E, 0x04,}}, //5.441344
        {0x05B9A541, {0x0F, 0x04,}}, //5.725178
        {0x06000000, {0x10, 0x04,}}, //6.000000
        {0x068657C9, {0x11, 0x04,}}, //6.524777
        {0x0705013A, {0x12, 0x04,}}, //7.019550
        {0x077CD110, {0x13, 0x04,}}, //7.487565
        {0x07EE7B47, {0x14, 0x04,}}, //7.931569
        {0x085A997C, {0x15, 0x04,}}, //8.353905
        {0x08C1AFDD, {0x16, 0x04,}}, //8.756590
        {0x092430F0, {0x17, 0x04,}}, //9.141372
        {0x0982809D, {0x18, 0x04,}}, //9.509775
        {0x09DCF68E, {0x19, 0x04,}}, //9.863137
        {0x0A33E01A, {0x1A, 0x04,}}, //10.202638
        {0x0A8781D8, {0x1B, 0x04,}}, //10.529325
        {0x0AD818DF, {0x1C, 0x04,}}, //10.844130
        {0x0B25DBDA, {0x1D, 0x04,}}, //11.147886
        {0x0B70FBE4, {0x1E, 0x04,}}, //11.441344
        {0x0BB9A541, {0x1F, 0x04,}}, //11.725178
        {0x0C000000, {0x20, 0x04,}}, //12.000000
        {0x0C8657C9, {0x21, 0x04,}}, //12.524777
        {0x0D05013A, {0x22, 0x04,}}, //13.019550
        {0x0D7CD110, {0x23, 0x04,}}, //13.487565
        {0x0DEE7B47, {0x24, 0x04,}}, //13.931569
        {0x0E5A997C, {0x25, 0x04,}}, //14.353905
        {0x0EC1AFDD, {0x26, 0x04,}}, //14.756590
        {0x0F2430F0, {0x27, 0x04,}}, //15.141372
        {0x0F82809D, {0x28, 0x04,}}, //15.509775
        {0x0FDCF68E, {0x29, 0x04,}}, //15.863137
        {0x1033E01A, {0x2A, 0x04,}}, //16.202638
        {0x108781D8, {0x2B, 0x04,}}, //16.529325
        {0x10D818DF, {0x2C, 0x04,}}, //16.844130
        {0x1125DBDA, {0x2D, 0x04,}}, //17.147886
        {0x1170FBE4, {0x2E, 0x04,}}, //17.441344
        {0x11B9A541, {0x2F, 0x04,}}, //17.725178
        {0x12000000, {0x30, 0x04,}}, //18.000000
        {0x128657C9, {0x31, 0x04,}}, //18.524777
        {0x1305013A, {0x32, 0x04,}}, //19.019550
        {0x137CD110, {0x33, 0x04,}}, //19.487565
        {0x13EE7B47, {0x34, 0x04,}}, //19.931569
        {0x145A997C, {0x35, 0x04,}}, //20.353905
        {0x14C1AFDD, {0x36, 0x04,}}, //20.756590
        {0x152430F0, {0x37, 0x04,}}, //21.141372
        {0x1582809D, {0x38, 0x04,}}, //21.509775
        {0x15DCF68E, {0x39, 0x04,}}, //21.863137
        {0x1633E01A, {0x3A, 0x04,}}, //22.202638
        {0x168781D8, {0x3B, 0x04,}}, //22.529325
        {0x16D818DF, {0x3C, 0x04,}}, //22.844130
        {0x1725DBDA, {0x3D, 0x04,}}, //23.147886
        {0x1770FBE4, {0x3E, 0x04,}}, //23.441344
        {0x17B9A541, {0x3F, 0x04,}}, //23.725178
        {0x18000000, {0x40, 0x04,}}, //24.000000
        {0x188657C9, {0x41, 0x04,}}, //24.524777
        {0x1905013A, {0x42, 0x04,}}, //25.019550
        {0x197CD110, {0x43, 0x04,}}, //25.487565
        {0x19EE7B47, {0x44, 0x04,}}, //25.931569
        {0x1A5A997C, {0x45, 0x04,}}, //26.353905
        {0x1AC1AFDD, {0x46, 0x04,}}, //26.756590
        {0x1B2430F0, {0x47, 0x04,}}, //27.141372
        {0x1B82809D, {0x48, 0x04,}}, //27.509775
        {0x1BDCF68E, {0x49, 0x04,}}, //27.863137
        {0x1C33E01A, {0x4A, 0x04,}}, //28.202638
        {0x1C8781D8, {0x4B, 0x04,}}, //28.529325
        {0x1CD818DF, {0x4C, 0x04,}}, //28.844130
        {0x1D25DBDA, {0x4D, 0x04,}}, //29.147886
        {0x1D70FBE4, {0x4E, 0x04,}}, //29.441344
        {0x1DB9A541, {0x4F, 0x04,}}, //29.725178
        {0x1DB9A541, {0x40, 0x05,}}, //29.725178
        {0x128657C9, {0x41, 0x05,}}, //18.524777
        {0x1305013A, {0x42, 0x05,}}, //19.019550
        {0x137CD110, {0x43, 0x05,}}, //19.487565
        {0x13EE7B47, {0x44, 0x05,}}, //19.931569
        {0x145A997C, {0x45, 0x05,}}, //20.353905
        {0x14C1AFDD, {0x46, 0x05,}}, //20.756590
        {0x152430F0, {0x47, 0x05,}}, //21.141372
        {0x1582809D, {0x48, 0x05,}}, //21.509775
        {0x15DCF68E, {0x49, 0x05,}}, //21.863137
        {0x1633E01A, {0x4A, 0x05,}}, //22.202638
        {0x168781D8, {0x4B, 0x05,}}, //22.529325
        {0x16D818DF, {0x4C, 0x05,}}, //22.844130
        {0x1725DBDA, {0x4D, 0x05,}}, //23.147886
        {0x1770FBE4, {0x4E, 0x05,}}, //23.441344
        {0x17B9A541, {0x4F, 0x05,}}, //23.725178
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

