/*!
*****************************************************************************
** \file        ov9750_reg_tbl.c
**
** \version     $Id: ov9750_reg_tbl.c 11807 2017-04-13 09:38:45Z hehuali $
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
GADI_VI_SensorDrvInfoT    ov9750_sensor =
{
    .magic_start        = GADI_SENSOR_HW_INFO_MAGIC_START,
    .version            = GADI_SENSOR_HW_INFO_VERSION,
    .length             = sizeof(GADI_VI_SensorDrvInfoT),
    .file_date          = 0x20161031,
    .HwInfo         =
    {
        .name               = "ov9750",
        .ctl_type           = GADI_SENSOR_CTL_I2C,
        .addr_len           = sizeof(GADI_U16),
        .data_len           = sizeof(GADI_U8),
        .reset_active_level = GADI_GPIO_LOW,
        .hw_addr            = {(0x20>>1), (0x6C>>1), 0, 0},
        .id_reg =
        {
            //{0x300A, 0x00}, // CHIP_ID_H
            {0x300B, 0x97}, // CHIP_ID_M
            {0x300C, 0x50}, // CHIP_ID_L
        },
        .reset_reg =
        {
            {0x0103, 0x01, 0x01},
            {GADI_VI_SENSOR_TABLE_FLAG_MSLEEP, 10},    // msleep(10);
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
    .sensor_id          = GADI_ISP_SENSOR_OV9750,
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
    .max_width          = 1280,
    .max_height         = 960,
    .def_sh_time        = GADI_VIDEO_FPS(50),
    .fmt_num            = 6,
    .auto_fmt           = GADI_VIDEO_MODE(1280,  960,  25, 1),
    .init_reg           =
    {
        {0x0100, 0x00,},
        {0x3000, 0x0F,}, // BIT[4]: io_ulpm_oen BIT[3:0]: io_y11/8_oen
        {0x3001, 0xFF,}, // BIT[7:0]: io_y7/0_oen
        {0x3002, 0xE1,}, // BIT[7]: io_vsync_oen BIT[6]: io_href_oen BIT[5]: io_pclk_oen
        //{0x3004, 0x6C,}, // SCCB ID
        {0x3005, 0xF0,}, // BIT[5]: sclk_psram BIT[4]: sclk_syncfifo BIT[1]: rts_psram BIT[0]: rts_syncfifo
        //{0x3006, 0x42,}, // SCCB ID2
        //{0x3007, 0x20,}, // BIT[7]: pll12_daclk_sel BIT[6]: r_pump_clk_sel 0:pll2_sclk, 1: pll1_sclk BIT[5]: r_ispin_array_addr_sel BIT[4]: r_ilpwm_out_sel BIT[3]: r_rst_pll_sleep_dis BIT[2]: r_db_out_en BIT[1:0]: r_vsync_sel
        //{0x3008, 0x00,}, // PAD_OUT0 BIT[4]: io_ulpm_o BIT[3:0]: io_y11/8_o
        //{0x3009, 0x00,}, // PAD_OUT1 BIT[7:0]: io_y7/0_o
        //{0x300D, 0x00,}, // PAD_OUT2 BIT[7]: io_vsync_o BIT[6]: io_href_o BIT[5]: io_pclk_o
        //{0x300E, 0x00,}, // PAD_SEL0 BIT[4]: io_ulpm_sel BIT[3:0]: io_y11/8_sel
        {0x300F, 0x00,}, // 0x00:12Bit, 0x03:10Bit PAD_SEL1 BIT[7:0]: io_y7/0_sel
        //{0x3010, 0x00,}, // PAD_SEL2 BIT[7]: io_vsync_sel BIT[6]: io_href_sel BIT[5]: io_pclk_sel
        {0x3011, 0x20,}, // PAD BIT[6:5]: Pad drive strenth 00:1X, 01:2X, 10:3X 11:4X
        //{0x3012, 0x20,}, // SCCB R12
        //{0x3015, 0x00,}, // PUMP CLK DIV BIT[7:4]: Npump clock div 0000: /1, Others: /1~/15 BIT[3:0]: Ppump clock div 0000: /1, Others: /1~/15
        {0x3016, 0x00,}, // MIPI_PHY_CTRL BIT[7:6] Lph BIT[4] mipi_pad_enable BIT[3] bp_c_hs_en_lat BIT[2] bp_d_hs_en_lat BIT[1:0] ictl[1:0]
        //{0x3017, 0x10,}, // MIPI_PHY_CTRL BIT[7:6] pgm_vcm[1:0] BIT[5:4] pgm_iptx[1:0]
        {0x3018, 0x32,}, // MIPI_SC_CTRL BIT[7:5] mipi_lane_mode N+1 lane BIT[4] mipi_lvds_sel 0:LVDS enable 1: MIPI enable BIT[3:2] r_phy_pd_mipi_man BIT[0] lane_dis_option 1:disbale
        //{0x3019, 0x00,}, // MIPI_SC_CTRL BIT[7:0] mipi lane diable manual
        {0x301A, 0xF0,}, // CLKRST0 BIT[6] sclk_stb BIT[5] sclk_ac BIT[4] sclk_tc BIT[3] mipi_phy_rst_o BIT[2] rst_stb BIT[1] rst_ac BIT[0] rst_tc
        {0x301B, 0xF0,}, // CLKRST1 BIT[7] sclk_blc BIT[6] sclk_isp BIT[5] sclk_testmode BIT[4] sclk_vfifo BIT[3] rst_blc BIT[2] rst_isp BIT[1] rst_testmode BIT[0] rst_vfifo
        {0x301C, 0xF0,}, // CLKRST2 BIT[7] pclk_dvp BIT[6] sclk_mipi BIT[5] sclk_dpcm BIT[4] sclk_otp BIT[3] rst_dvp BIT[2] rst_mipi BIT[1] rst_dpmc BIT[0] rst_otp
        {0x301D, 0xF0,}, // CLKRST3 BIT[7] sclk_asram_tst BIT[6] sclk_grp BIT[5] sclk_bist BIT[4] sclk_aec BIT[3] rst_asram_tst BIT[2] rst_grp BIT[1] rst_bist BIT[0] rst_aec_pk
        {0x301E, 0xF0,}, // CLKRST4 BIT[7] sclk_ilpwm BIT[6] pclk_lvds BIT[5] pclk_vfifo BIT[4] pclk_mipi BIT[3] rst_ilpwm BIT[2] rst_lvds
        //{0x3020, 0x93,}, // CLOCK SEL BIT[7] Clock switch output 0:Padclk 1:nomal BIT[3] pclk_div 0:/1 1:/2
        //{0x3021, 0x03,}, // MISC CTRL BIT[6] sleep no latch option 1:no latch BIT[5] fst_stby_ctr 0:software standby enter at v_blk 1:software standby enter at i_blk or v_blk BIT[0] cen_global_o global control all CEN of SRAM
        {0x3022, 0x21,}, // MIPI SC CTRL BIT[5] DVP mode BIT[4] mipi_auto_slp_dis BIT[3] lvds_mode_o BIT[2] clk_lane_disable_1 BIT[1] clock lane disable when pd_mipi BIT[0] pd_mipi enable when rst_sync
        //{0x3023, 0x20,}, // MIPI SC CTRL BIT[5:4] mipi_ck_skew BIT[3:2] mipi_d1_skiew BIT[1:0] mipi_d0_skiew
        //{0x302A, 0xF0,}, // SUB ID BIT[7:4] Process BIT[3:0] version
        //{0x302B, 0x00,}, // R GPIO_IN0 BIT[7:0] y_i[11:4]
        //{0x302C, 0x00,}, // R GPIO_IN1 BIT[7:4] y_i[3:0] BIT[3:0] split_id[3:0]
        //{0x302D, 0x00,}, // R GPIO_IN2 BIT[7] pclk_i BIT[6] sid_i BIT[5] ulpm_i BIT[2] href_i BIT[1] vsync_i
        //{0x3030, 0x00,}, // BIT[5] SCLK inv BIT[4] PCLK inv
        {0x3031, 0x0A,}, // BIT[4:0] mipi_bit_sel 0x8: 8-bit mode, 0xA:10-bit mode
        {0x3032, 0x80,}, // BIT[7] pll2_sysclk_sel BIT[6] asram_clk_sel BIT[5] array_hskip_man_en BIT[4] r_rst_otp_sleep_dis BIT[3] r_rst_ana_sleep_dis BIT[2:0] array_hskip_man[3:1]
        //{0x3033, 0x24,}, // BIT[7] array_hskip_man[0] BIT[5] asram_clk_sel BIT[5] array_hskip_man_en BIT[4] r_rst_otp_sleep_dis BIT[3] r_rst_ana_sleep_dis BIT[2:0] array_hskip_man[3:1]
        {0x303C, 0xFF,},
        {0x303E, 0xFF,},
        {0x3040, 0xF0,},
        {0x3041, 0x00,},
        {0x3042, 0xF0,},
        {0x3104, 0x01,},
        {0x3106, 0x15,}, // PLL_SCKL[3:2] sys_pre_div1 1/2/4/1   PLL_SCKL[7:4] sclk_pdiv 1/1/2/3/4/5.../15
        {0x3107, 0x01,},
        {0x3504, 0x03,},
        {0x3505, 0x83,},
        {0x3600, 0x65,},
        {0x3601, 0x60,},
        {0x3602, 0x32,}, // 0x32:12Bit 0x22:10Bit
        {0x3610, 0xE8,},
        {0x3614, 0x96,},
        {0x3615, 0x79,},
        {0x3617, 0x07,},
        {0x3620, 0x84,},
        {0x3621, 0x90,},
        {0x3622, 0x00,},
        {0x3623, 0x00,},
        {0x3633, 0x10,},
        {0x3634, 0x10,},
        {0x3635, 0x14,},
        {0x3650, 0x00,},
        {0x3652, 0xFF,},
        {0x3654, 0x20,},
        {0x3653, 0x34,},
        {0x3655, 0x20,},
        {0x3656, 0xFF,},
        {0x3657, 0xC4,},
        {0x365A, 0xFF,},
        {0x365B, 0xFF,},
        {0x365E, 0xFF,},
        {0x365F, 0x00,},
        {0x3668, 0x00,},
        {0x366A, 0x07,},
        {0x366D, 0x00,},
        {0x366E, 0x10,},
        //{0x3700, 0x14,},
        //{0x3701, 0x08,},
        {0x3702, 0x1D,},
        {0x3703, 0x10,},
        {0x3704, 0x14,},
        {0x3705, 0x00,},
        {0x3706, 0x48,}, // 0x48:12Bit 0x27:10Bit
        //{0x3707, 0x04,},
        //{0x3708, 0x12,},
        {0x3709, 0x24,},
        {0x370A, 0x00,},
        {0x370B, 0xFA,}, // 0xFA:12Bit 0x7D:10Bit
        //{0x370C, 0x04,},
        //{0x370D, 0x00,},
        //{0x370E, 0x00,},
        //{0x370F, 0x05,},
        //{0x3710, 0x18,},
        //{0x3711, 0x0E,},
        {0x3714, 0x24,},
        //{0x3718, 0x13,},
        {0x371A, 0x5E,},
        //{0x3720, 0x05,},
        //{0x3721, 0x05,},
        //{0x3726, 0x06,},
        //{0x3728, 0x05,},
        //{0x372A, 0x03,},
        //{0x372B, 0x53,},
        //{0x372C, 0x53,},
        //{0x372D, 0x53,},
        //{0x372E, 0x06,},
        //{0x372F, 0x10,},
        {0x3730, 0x82,},
        //{0x3731, 0x06,},
        //{0x3732, 0x14,},
        {0x3733, 0x10,},
        //{0x3736, 0x30,},
        //{0x373A, 0x02,},
        //{0x373B, 0x03,},
        //{0x373C, 0x05,},
        {0x373E, 0x18,},
        {0x3755, 0x00,},
        {0x3758, 0x00,},
        //{0x375A, 0x06,},
        {0x375B, 0x13,},
        //{0x375F, 0x14,},
        {0x3772, 0x23,},
        {0x3773, 0x05,},
        {0x3774, 0x16,},
        {0x3775, 0x12,},
        {0x3776, 0x08,},
        //{0x377A, 0x06,},
        {0x3788, 0x00,},
        {0x3789, 0x04,},
        {0x378A, 0x01,},
        {0x378B, 0x60,},
        {0x3799, 0x27,},
        //{0x37A0, 0x44,},
        //{0x37A1, 0x2D,},
        //{0x37A7, 0x44,},
        {0x37A8, 0x38,},
        //{0x37AA, 0x44,},
        //{0x37AB, 0x24,},
        //{0x37B3, 0x33,},
        {0x37B5, 0x36,},
        //{0x37C0, 0x42,},
        //{0x37C1, 0x42,},
        {0x37C2, 0x04,},
        {0x37C5, 0x00,},
        {0x37C8, 0x00,},
        {0x37D1, 0x13,},
        {0x3814, 0x01,},
        {0x3815, 0x01,},
        {0x3816, 0x00,},
        {0x3817, 0x00,},
        {0x3818, 0x00,},
        {0x3819, 0x00,},
        {0x3820, 0x80,},
        {0x3821, 0x40,},
        {0x3826, 0x00,},
        {0x3827, 0x08,},
        {0x382A, 0x01,},
        {0x382B, 0x01,},
        {0x3836, 0x02,},
        {0x3838, 0x10,},
        {0x3861, 0x00,},
        {0x3862, 0x00,},
        {0x3863, 0x02,},
        {0x3B00, 0x00,},
        {0x3C00, 0x89,},
        {0x3C01, 0xAB,},
        {0x3C02, 0x01,},
        {0x3C03, 0x00,},
        {0x3C04, 0x00,},
        {0x3C05, 0x03,},
        {0x3C06, 0x00,},
        {0x3C07, 0x05,},
        {0x3C0C, 0x00,},
        {0x3C0D, 0x00,},
        {0x3C0E, 0x00,},
        {0x3C0F, 0x00,},
        {0x3C40, 0x00,},
        {0x3C41, 0xA3,},
        {0x3C43, 0x7D,},
        {0x3C56, 0x80,},
        {0x3C80, 0x08,},
        {0x3C82, 0x01,},
        {0x3C83, 0x61,},
        {0x3D85, 0x17,},
        {0x3F08, 0x08,},
        {0x3F0A, 0x00,},
        {0x3F0B, 0x30,},
        {0x4000, 0xCD,},
        {0x4003, 0x40,},
        {0x4009, 0x0D,},
        {0x4010, 0xF0,},
        {0x4011, 0x70,},
        {0x4017, 0x10,},
        {0x4040, 0x00,},
        {0x4041, 0x00,},
        {0x4303, 0x00,},
        {0x4307, 0x30,},
        {0x4500, 0x30,},
        {0x4502, 0x40,},
        {0x4503, 0x06,},
        {0x4508, 0xAA,},
        {0x450C, 0x00,},
        {0x4600, 0x00,},
        {0x4601, 0x80,},
        {0x4700, 0x04,},
        {0x4704, 0x00,},
        {0x4705, 0x04,},
        {0x4837, 0x14,},
        {0x484A, 0x3F,},
        {0x5000, 0x30,}, // 0x30:12Bit 0x10:10Bit
        {0x5001, 0x01,},
        {0x5002, 0x28,},
        {0x5004, 0x0C,},
        {0x5006, 0x0C,},
        {0x5007, 0xE0,},
        {0x5008, 0x01,},
        {0x5009, 0xB0,},
        {0x502A, 0x14,}, // 0x14:12Bit 0x18:10Bit
        {0x5901, 0x00,},
        {0x5A01, 0x00,},
        {0x5A03, 0x00,},
        {0x5A04, 0x0C,},
        {0x5A05, 0xE0,},
        {0x5A06, 0x09,},
        {0x5A07, 0xB0,},
        {0x5A08, 0x06,},
        {0x5E00, 0x00,},
        {0x5E10, 0xFC,},
        {0x3503, 0x38,},
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .pll_table =
    {
        // PLL1:pixclk = pll1_extclk / pll1_prediv0 / pll1_pre_div * pll1_multiplier / (1 + pll1_m-divider) / pll1_MIPI_divider
        // PLL2:sysclk:0x3033=0x24 0x3032=0x80
        // 0x3033[1]=0 0x3032[7]=0 sysclk = extclk / pll1_prediv0 / pll1_pre_div * pll1_multiplier / pll1_sys_pre_div / pll1_sys_divider / sys_pre_div1 / sclk_pdiv
        // 0x3033[1]=0 0x3032[7]=1 sysclk = extclk / pll2_prediv0 / pll2_pre_div * pll2_multiplier / (1 + pll2_sys_pre_div) / pll2_sys_divider / sys_pre_div1 / sclk_pdiv
        // 0x3033[1]=1 0x3032[7]=X sysclk = extclk / pll2_prediv0 / pll2_pre_div * pll2_multiplier / (1 + pll2_dac-divider) / sys_pre_div1 / sclk_pdiv
        // 0x3106 = 0x15 PLL_SCKL[3:2] sys_pre_div1 1/2/4/1   PLL_SCKL[7:4] sclk_pdiv 1/1/2/3/4/5.../15
        // PLL1:pixclk(fact out) = HTS(fact) * VTS(reg) * fps
        // PLL2:sys-clock must be 96Mhz = HTS(reg) * 2 * VTS(reg) * fps
        [0] =
        {
            // for 720P@60, 720P@50
            .pixclk = 48000000,
            .extclk = 27000000,
            // PLL1:pixclk = pll1_extclk / pll1_prediv0 / pll1_pre_div * pll1_multiplier / (1 + pll1_m-divider) / pll1_MIPI_divider
            //             = 27 / 1 / 2 * 55 / (1 + 1) / 5 = 74.25
            // PLL2:sysclk = extclk / pll2_prediv0 / pll2_pre_div * pll2_multiplier / (1 + pll2_sys_pre_div) / pll2_sys_divider / sys_pre_div1 / sclk_pdiv
            //             = 27 / 1 / 3 * 80 / (1 + 4) / 1.5 / 2 / 1 = 48
            .regs =
            {
                {0x0300, 0x02,}, // PLL1[3:0] pll1_pre_div      1/1.5/2/2.5/3/3.5/4/4.5/5/5.5/6/6.5/7/8/9
                {0x0302, 0x37,}, // PLL1[7:0] pll1_multiplier
                {0x0303, 0x01,}, // PLL1[3:0] pll1_m-divider
                {0x0304, 0x01,}, // PLL1[1:0] pll1_MIPI_divider 4/5/6/8
                {0x0305, 0x01,}, // PLL1[1:0] pll1_sys_pre_div  3/4/5/6
                {0x0306, 0x01,}, // PLL1[0:0] pll1_sys_divider  1/2
                {0x030A, 0x00,}, // PLL1[0:0] pll1_prediv0      1/2
                {0x030B, 0x04,}, // PLL2[3:0] pll2_pre_div      1/1.5/2/2.5/3/3.5/4/4.5/5/5.5/6/6.5/7/8/9
                {0x030D, 0x50,}, // PLL2[7:0] pll2_multiplier
                {0x030E, 0x01,}, // PLL2[2:0] pll2_sys_divider  1/1.5/2/2.5/3/3.5/4/4.5
                {0x030F, 0x04,}, // PLL2[3:0] pll2_sys_pre_div
                {0x0312, 0x01,}, // PLL2[4:4] pll2_prediv0 1/2 PLL2[3:0] pll2_dac-divider
                {0x031E, 0x04,},
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,}
            },
        },
        [1] =
        {
            // for 960P@30, 960P@25
            .pixclk = 48000000,
            .extclk = 27000000,
            // PLL1:pixclk = pll1_extclk / pll1_prediv0 / pll1_pre_div * pll1_multiplier / (1 + pll1_m-divider) / pll1_MIPI_divider
            //             = 27 / 1 / 1.5 * 30 / (1 + 1) / 5 = 54
            // PLL2:sysclk = extclk / pll2_prediv0 / pll2_pre_div * pll2_multiplier / (1 + pll2_sys_pre_div) / pll2_sys_divider / sys_pre_div1 / sclk_pdiv
            //             = 27 / 1 / 3 * 80 / (1 + 4) / 1.5 / 2 / 1 = 48
            .regs =
            {
                {0x0300, 0x01,}, // PLL1[3:0] pll1_pre_div      1/1.5/2/2.5/3/3.5/4/4.5/5/5.5/6/6.5/7/8/9
                {0x0302, 0x1E,}, // PLL1[7:0] pll1_multiplier
                {0x0303, 0x01,}, // PLL1[3:0] pll1_m-divider
                {0x0304, 0x01,}, // PLL1[1:0] pll1_MIPI_divider 4/5/6/8
                {0x0305, 0x01,}, // PLL1[1:0] pll1_sys_pre_div  3/4/5/6
                {0x0306, 0x01,}, // PLL1[0:0] pll1_sys_divider  1/2
                {0x030A, 0x00,}, // PLL1[0:0] pll1_prediv0      1/2
                {0x030B, 0x04,}, // PLL2[3:0] pll2_pre_div      1/1.5/2/2.5/3/3.5/4/4.5/5/5.5/6/6.5/7/8/9
                {0x030D, 0x50,}, // PLL2[7:0] pll2_multiplier
                {0x030E, 0x01,}, // PLL2[0:0] pll2_sys_divider
                {0x030F, 0x04,}, // PLL2[3:0] pll2_sys_pre_div
                {0x0312, 0x01,}, // PLL2[4:4] pll2_prediv0 1/2 PLL2[3:0] pll2_dac-divider
                {0x031E, 0x04,},
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,}
            },
        },
        [2] =
        {
            // for 720P@30, 720P@25
            .pixclk = 48000000,
            .extclk = 27000000,
            // PLL1:pixclk = pll1_extclk / pll1_prediv0 / pll1_pre_div * pll1_multiplier / (1 + pll1_m-divider) / pll1_MIPI_divider
            //             = 27 / 1 / 4 * 110 / (1 + 3) / 5 = 37.125
            // PLL2:sysclk = extclk / pll2_prediv0 / pll2_pre_div * pll2_multiplier / (1 + pll2_sys_pre_div) / pll2_sys_divider / sys_pre_div1 / sclk_pdiv
            //             = 27 / 1 / 3 * 80 / (1 + 4) / 1.5 / 2 / 1 = 48
            .regs =
            {
                {0x0300, 0x06,}, // PLL1[3:0] pll1_pre_div      1/1.5/2/2.5/3/3.5/4/4.5/5/5.5/6/6.5/7/8/9
                {0x0302, 0x6E,}, // PLL1[7:0] pll1_multiplier
                {0x0303, 0x03,}, // PLL1[3:0] pll1_m-divider
                {0x0304, 0x01,}, // PLL1[1:0] pll1_MIPI_divider 4/5/6/8
                {0x0305, 0x01,}, // PLL1[1:0] pll1_sys_pre_div  3/4/5/6
                {0x0306, 0x07,}, // PLL1[0:0] pll1_sys_divider  1/2
                {0x030A, 0x00,}, // PLL1[0:0] pll1_prediv0      1/2
                {0x030B, 0x04,}, // PLL2[3:0] pll2_pre_div      1/1.5/2/2.5/3/3.5/4/4.5/5/5.5/6/6.5/7/8/9
                {0x030D, 0x50,}, // PLL2[7:0] pll2_multiplier
                {0x030E, 0x01,}, // PLL2[0:0] pll2_sys_divider
                {0x030F, 0x04,}, // PLL2[3:0] pll2_sys_pre_div
                {0x0312, 0x01,}, // PLL2[4:4] pll2_prediv0 1/2 PLL2[3:0] pll2_dac-divider
                {0x031E, 0x04,},
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,}
            },
        },
    },
    .video_fmt_table =
    {
        [0] = // 1280x960P@30fps    1800 * 1000 * 30 = 54000000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  960,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x0100, 0x00, 0x01,}, // BIT[0]: 0: standby, 1: Streaming
                {0x3800, 0x00,}, // H_crop_start_H
                {0x3801, 0x00,}, // H_crop_start_L
                {0x3802, 0x00,}, // V_crop_start_H
                {0x3803, 0x04,}, // V_crop_start_L
                {0x3804, 0x05,}, // H_crop_end_H    1295 = 0x050F
                {0x3805, 0x0F,}, // H_crop_end_L
                {0x3806, 0x03,}, // V_crop_end_H     971 = 0x03CB
                {0x3807, 0xCB,}, // V_crop_end_L
                {0x3808, 0x05,}, // H_output_size_H 1280 = 0x0500
                {0x3809, 0x00,}, // H_output_size_L
                {0x380A, 0x03,}, // V_output_size_H  960 = 0x03C0
                {0x380B, 0xC0,}, // V_output_size_L
                {0x380C, 0x06,}, // HTS_H           1600 * 1000 * 30 = 48000000
                {0x380D, 0x40,}, // HTS_L
                {0x3810, 0x00,}, // H_win_off_H
                {0x3811, 0x08,}, // H_win_off_L
                {0x3812, 0x00,}, // V_win_off_H
                {0x3813, 0x04,}, // V_win_off_L
                {0x3611, 0x56,},
                {0x3612, 0x48,},
                {0x3613, 0x5A,},
                {0x3625, 0x07,},
                {0x3636, 0x13,},
                {0x37C7, 0x38,},
                {0x481F, 0x26,},
                {0x0100, 0x01, 0x01,}, // BIT[0]: 0: standby, 1: Streaming
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
                .pll_index  = 1,
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
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_BG,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_BG,
                },
            },
            .update_after_vsync_start   = 30,
            .update_after_vsync_end     = 33,
        },
        [1] = // 1280x960P@25fps    2160 * 1000 * 25 = 54000000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  960,  25, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x0100, 0x00, 0x01,}, // BIT[0]: 0: standby, 1: Streaming
                {0x3800, 0x00,}, // H_crop_start_H
                {0x3801, 0x00,}, // H_crop_start_L
                {0x3802, 0x00,}, // V_crop_start_H
                {0x3803, 0x04,}, // V_crop_start_L
                {0x3804, 0x05,}, // H_crop_end_H    1295 = 0x050F
                {0x3805, 0x0F,}, // H_crop_end_L
                {0x3806, 0x03,}, // V_crop_end_H     971 = 0x03CB
                {0x3807, 0xCB,}, // V_crop_end_L
                {0x3808, 0x05,}, // H_output_size_H 1280 = 0x0500
                {0x3809, 0x00,}, // H_output_size_L
                {0x380A, 0x03,}, // V_output_size_H  960 = 0x03C0
                {0x380B, 0xC0,}, // V_output_size_L
                {0x380C, 0x07,}, // HTS_H           1920 * 1000 * 25 = 48000000
                {0x380D, 0x80,}, // HTS_L
                {0x3810, 0x00,}, // H_win_off_H
                {0x3811, 0x08,}, // H_win_off_L
                {0x3812, 0x00,}, // V_win_off_H
                {0x3813, 0x04,}, // V_win_off_L
                {0x3611, 0x56,},
                {0x3612, 0x48,},
                {0x3613, 0x5A,},
                {0x3625, 0x07,},
                {0x3636, 0x13,},
                {0x37C7, 0x38,},
                {0x481F, 0x26,},
                {0x0100, 0x01, 0x01,}, // BIT[0]: 0: standby, 1: Streaming
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
                .pll_index  = 1,
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
        [2] = // 1280x720P@60fps    1650 * 750 * 60 = 74250000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  60, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x0100, 0x00, 0x01,}, // BIT[0]: 0: standby, 1: Streaming
                {0x3800, 0x00,}, // H_crop_start_H
                {0x3801, 0x00,}, // H_crop_start_L
                {0x3802, 0x00,}, // V_crop_start_H
                {0x3803, 0x7C,}, // V_crop_start_L
                {0x3804, 0x05,}, // H_crop_end_H    1295 = 0x050F
                {0x3805, 0x0F,}, // H_crop_end_L
                {0x3806, 0x03,}, // V_crop_end_H     971 = 0x03CB
                {0x3807, 0x53,}, // V_crop_end_L
                {0x3808, 0x05,}, // H_output_size_H 1280 = 0x0500
                {0x3809, 0x00,}, // H_output_size_L
                {0x380A, 0x02,}, // V_output_size_H  720 = 0x02D0
                {0x380B, 0xD0,}, // V_output_size_L
                {0x380C, 0x04,}, // HTS_H           1066 * 750 * 60 = 48000000
                {0x380D, 0x2A,}, // HTS_L 0x042A 0x0339
                {0x3810, 0x00,}, // H_win_off_H
                {0x3811, 0x08,}, // H_win_off_L
                {0x3812, 0x00,}, // V_win_off_H
                {0x3813, 0x04,}, // V_win_off_L
                {0x3611, 0x5C,},
                {0x3612, 0x18,},
                {0x3613, 0x3A,},
                {0x3636, 0x14,},
                {0x37C7, 0x01,},
                {0x481F, 0x30,},
                {0x0100, 0x01, 0x01,}, // BIT[0]: 0: standby, 1: Streaming
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
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_BG,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_BG,
                },
            },
            .update_after_vsync_start   = 13,
            .update_after_vsync_end     = 16,
        },
        [3] = // 1280x720P@50fps    1980 * 750 * 50 = 74250000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  50, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x0100, 0x00, 0x01,}, // BIT[0]: 0: standby, 1: Streaming
                {0x3800, 0x00,}, // H_crop_start_H
                {0x3801, 0x00,}, // H_crop_start_L
                {0x3802, 0x00,}, // V_crop_start_H
                {0x3803, 0x7C,}, // V_crop_start_L
                {0x3804, 0x05,}, // H_crop_end_H    1295 = 0x050F
                {0x3805, 0x0F,}, // H_crop_end_L
                {0x3806, 0x03,}, // V_crop_end_H     971 = 0x03CB
                {0x3807, 0x53,}, // V_crop_end_L
                {0x3808, 0x05,}, // H_output_size_H 1280 = 0x0500
                {0x3809, 0x00,}, // H_output_size_L
                {0x380A, 0x02,}, // V_output_size_H  720 = 0x02D0
                {0x380B, 0xD0,}, // V_output_size_L
                {0x380C, 0x05,}, // HTS_H           1280 * 750 * 50 = 48000000
                {0x380D, 0x00,}, // HTS_L
                {0x3810, 0x00,}, // H_win_off_H
                {0x3811, 0x08,}, // H_win_off_L
                {0x3812, 0x00,}, // V_win_off_H
                {0x3813, 0x04,}, // V_win_off_L
                {0x3611, 0x5C,},
                {0x3612, 0x18,},
                {0x3613, 0x3A,},
                {0x3636, 0x14,},
                {0x37C7, 0x01,},
                {0x481F, 0x30,},
                {0x0100, 0x01, 0x01,}, // BIT[0]: 0: standby, 1: Streaming
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
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_BG,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_BG,
                },
            },
            .update_after_vsync_start   = 17,
            .update_after_vsync_end     = 20,
        },
        [4] = // 1280x720P@30fps    1650 * 750 * 30 = 37125000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x0100, 0x00, 0x01,}, // BIT[0]: 0: standby, 1: Streaming
                {0x3800, 0x00,}, // H_crop_start_H
                {0x3801, 0x00,}, // H_crop_start_L
                {0x3802, 0x00,}, // V_crop_start_H
                {0x3803, 0x7C,}, // V_crop_start_L
                {0x3804, 0x05,}, // H_crop_end_H    1295 = 0x050F
                {0x3805, 0x0F,}, // H_crop_end_L
                {0x3806, 0x03,}, // V_crop_end_H     971 = 0x03CB
                {0x3807, 0x53,}, // V_crop_end_L
                {0x3808, 0x05,}, // H_output_size_H 1280 = 0x0500
                {0x3809, 0x00,}, // H_output_size_L
                {0x380A, 0x02,}, // V_output_size_H  720 = 0x02D0
                {0x380B, 0xD0,}, // V_output_size_L
                {0x380C, 0x08,}, // HTS_H           2133 * 750 * 30 = 48000000
                {0x380D, 0x55,}, // HTS_L
                {0x3810, 0x00,}, // H_win_off_H
                {0x3811, 0x08,}, // H_win_off_L
                {0x3812, 0x00,}, // V_win_off_H
                {0x3813, 0x04,}, // V_win_off_L
                {0x3611, 0x5C,},
                {0x3612, 0x18,},
                {0x3613, 0x3A,},
                {0x3636, 0x14,},
                {0x37C7, 0x01,},
                {0x481F, 0x30,},
                {0x0100, 0x01, 0x01,}, // BIT[0]: 0: standby, 1: Streaming
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
                .pll_index  = 2,
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
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_BG,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_BG,
                    GADI_VI_BAYER_PATTERN_BG,
                },
            },
            .update_after_vsync_start   = 30,
            .update_after_vsync_end     = 33,
        },
        [5] = // 1280x720P@25fps    1980 * 750 * 25 = 37125000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1280,  720,  25, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x0100, 0x00, 0x01,}, // BIT[0]: 0: standby, 1: Streaming
                {0x3800, 0x00,}, // H_crop_start_H
                {0x3801, 0x00,}, // H_crop_start_L
                {0x3802, 0x00,}, // V_crop_start_H
                {0x3803, 0x7C,}, // V_crop_start_L
                {0x3804, 0x05,}, // H_crop_end_H    1295 = 0x050F
                {0x3805, 0x0F,}, // H_crop_end_L
                {0x3806, 0x03,}, // V_crop_end_H     971 = 0x03CB
                {0x3807, 0x53,}, // V_crop_end_L
                {0x3808, 0x05,}, // H_output_size_H 1280 = 0x0500
                {0x3809, 0x00,}, // H_output_size_L
                {0x380A, 0x02,}, // V_output_size_H  720 = 0x02D0
                {0x380B, 0xD0,}, // V_output_size_L
                {0x380C, 0x0A,}, // HTS_H           2560 * 750 * 25 = 48000000
                {0x380D, 0x00,}, // HTS_L
                {0x3810, 0x00,}, // H_win_off_H
                {0x3811, 0x08,}, // H_win_off_L
                {0x3812, 0x00,}, // V_win_off_H
                {0x3813, 0x04,}, // V_win_off_L
                {0x3611, 0x5C,},
                {0x3612, 0x18,},
                {0x3613, 0x3A,},
                {0x3636, 0x14,},
                {0x37C7, 0x01,},
                {0x481F, 0x30,},
                {0x0100, 0x01, 0x01,}, // BIT[0]: 0: standby, 1: Streaming
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
                .pll_index  = 2,
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
#define OV9750_HORIZ_MIRROR     (0x03 << 1)
#define OV9750_VERT_FLIP        (0x03 << 1)
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY_VERTICALLY] =
        {
            {0x0100, 0x00, 0x01,}, // BIT[0]: 0: standby, 1: Streaming
            {0x3821, OV9750_HORIZ_MIRROR, OV9750_HORIZ_MIRROR},
            {0x3820, OV9750_VERT_FLIP, OV9750_VERT_FLIP},
            {0x450B, 0x20,},
            {0x0100, 0x01, 0x01,}, // BIT[0]: 0: standby, 1: Streaming
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY] =
        {
            {0x0100, 0x00, 0x01,}, // BIT[0]: 0: standby, 1: Streaming
            {0x3821, OV9750_HORIZ_MIRROR, OV9750_HORIZ_MIRROR},
            {0x3820, 0x00, OV9750_VERT_FLIP},
            {0x450B, 0x00,},
            {0x0100, 0x01, 0x01,}, // BIT[0]: 0: standby, 1: Streaming
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_VERTICALLY] =
        {
            {0x0100, 0x00, 0x01,}, // BIT[0]: 0: standby, 1: Streaming
            {0x3821, 0x00, OV9750_HORIZ_MIRROR},
            {0x3820, OV9750_VERT_FLIP, OV9750_VERT_FLIP},
            {0x450B, 0x20,},
            {0x0100, 0x01, 0x01,}, // BIT[0]: 0: standby, 1: Streaming
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_NONE] =
        {
            {0x0100, 0x00, 0x01,}, // BIT[0]: 0: standby, 1: Streaming
            {0x3821, 0x00, OV9750_HORIZ_MIRROR},
            {0x3820, 0x00, OV9750_VERT_FLIP},
            {0x450B, 0x00,},
            {0x0100, 0x01, 0x01,}, // BIT[0]: 0: standby, 1: Streaming
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
    // hmax = (TIMING_CONTROL_HTS_HIGHBYTE & 0xFF) << 8 +
    //        (TIMING_CONTROL_HTS_LOWBYTE & 0xFF) << 0
    .hmax_reg =
    {
        {0x380C, 0x00, 0xFF, 0x00, 0x08, 0}, // TIMING_CONTROL_HTS [11:8]
        {0x380D, 0x00, 0xFF, 0x00, 0x00, 0}, // TIMING_CONTROL_HTS [7:0]
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    // vmax = (TIMING_CONTROL_VTS_HIGHBYTE & 0xFF) << 8 +
    //        (TIMING_CONTROL_VTS_LOWBYTE & 0xFF) << 0
    .vmax_reg =
    {
        {0x380E, 0x00, 0xFF, 0x00, 0x08, 0}, // TIMING_CONTROL_VTS [11:8]
        {0x380F, 0x00, 0xFF, 0x00, 0x00, 0}, // TIMING_CONTROL_VTS [7:0]
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .vmax_wadd_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    // shs = (AEC_PK_EXPO_H & 0x0F) << 0x0C +
    //       (AEC_PK_EXPO_M & 0xFF) << 0x04 +
    //       (AEC_PK_EXPO_L & 0xFF) >> 0x04
    .shs_reg =
    {
        {0x3500, 0x00, 0x0F, 0x00, 0x0C, 0}, // AEC_PK_EXPO_H [19:16]
        {0x3501, 0x00, 0xFF, 0x00, 0x04, 0}, // AEC_PK_EXPO_M [15:8]
        {0x3502, 0x00, 0xF0, 0x01, 0x04, 0}, // AEC_PK_EXPO_L [7:0]  Low 4bits are fraction bits
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .shs_additional_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    .cal_gain_mode              = GADI_SENSOR_CAL_GAIN_MODE1,
    .max_agc_index = 64,
    .max_again_index = 64,
    .cur_again_index = 0,
    .agc_info =
    {
        .db_max     = 0x18CE7F78,    // 15.5dB
        .db_min     = 0x01000000,    // 1dB
        .db_step    = 0x00100000,    // 0.0625dB
    },
    .gain_reg =
    {
        //{0x3503,},
        {0x3508,},
        {0x3509,},
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .gain_table =
    {
        // When 0x3503[2]=0, Linear Gain = (0x3508[7:0]+3509[7:0]/128)
#if 1
        {0x0       , {0x00, 0x80,}},
        {0x86CDDD  , {0x00, 0x88,}},
        {0x105E6A1 , {0x00, 0x90,}},
        {0x17E1FC5 , {0x00, 0x98,}},
        {0x1F02DE2 , {0x00, 0xA0,}},
        {0x25CAB1F , {0x00, 0xA8,}},
        {0x2C41C1A , {0x00, 0xB0,}},
        {0x326F3C0 , {0x00, 0xB8,}},
        {0x3859652 , {0x00, 0xC0,}},
        {0x3E05BC4 , {0x00, 0xC8,}},
        {0x43791B4 , {0x00, 0xD0,}},
        {0x48B7CF3 , {0x00, 0xD8,}},
        {0x4DC5ACF , {0x00, 0xE0,}},
        {0x52A6223 , {0x00, 0xE8,}},
        {0x575C434 , {0x00, 0xF0,}},
        {0x5BEAD6E , {0x00, 0xF8,}},
        {0x6054603 , {0x01, 0x00,}},
        {0x68C13E0 , {0x01, 0x10,}},
        {0x70B2CA4 , {0x01, 0x20,}},
        {0x78365C8 , {0x01, 0x30,}},
        {0x7F573E5 , {0x01, 0x40,}},
        {0x861F122 , {0x01, 0x50,}},
        {0x8C9621D , {0x01, 0x60,}},
        {0x92C39C4 , {0x01, 0x70,}},
        {0x98ADC55 , {0x01, 0x80,}},
        {0x9E5A1C8 , {0x01, 0x90,}},
        {0xA3CD7B8 , {0x01, 0xA0,}},
        {0xA90C2F6 , {0x01, 0xB0,}},
        {0xAE1A0D3 , {0x01, 0xC0,}},
        {0xB2FA827 , {0x01, 0xD0,}},
        {0xB7B0A37 , {0x01, 0xE0,}},
        {0xBC3F372 , {0x01, 0xF0,}},
        {0xC0A8C06 , {0x02, 0x00,}},
        {0xC9159E3 , {0x02, 0x20,}},
        {0xD1072A7 , {0x02, 0x40,}},
        {0xD88ABCB , {0x02, 0x60,}},
        {0xDFAB9E8 , {0x02, 0x80,}},
        {0xE673725 , {0x02, 0xA0,}},
        {0xECEA820 , {0x02, 0xC0,}},
        {0xF317FC7 , {0x02, 0xE0,}},
        {0xF902258 , {0x03, 0x00,}},
        {0xFEAE7CB , {0x03, 0x20,}},
        {0x10421DBB, {0x03, 0x40,}},
        {0x109608F9, {0x03, 0x60,}},
        {0x10E6E6D6, {0x03, 0x80,}},
        {0x1134EE2A, {0x03, 0xA0,}},
        {0x1180503B, {0x03, 0xC0,}},
        {0x11C93975, {0x03, 0xE0,}},
        {0x120FD209, {0x04, 0x00,}},
        {0x12969FE6, {0x04, 0x40,}},
        {0x1315B8AA, {0x04, 0x80,}},
        {0x138DF1CE, {0x04, 0xC0,}},
        {0x13FFFFEC, {0x05, 0x00,}},
        {0x146C7D28, {0x05, 0x40,}},
        {0x14D3EE23, {0x05, 0x80,}},
        {0x1536C5CA, {0x05, 0xC0,}},
        {0x1595685B, {0x06, 0x00,}},
        {0x15F02DCE, {0x06, 0x40,}},
        {0x164763BE, {0x06, 0x80,}},
        {0x169B4EFD, {0x06, 0xC0,}},
        {0x16EC2CD9, {0x07, 0x00,}},
        {0x173A342D, {0x07, 0x40,}},
        {0x1785963E, {0x07, 0x80,}},
        {0x17CE7F78, {0x07, 0xC0,}},
#else
        {0x0       , {0x08,0x00,0x80,}},
        {0x86CDDD  , {0x08,0x00,0x88,}},
        {0x105E6A2 , {0x08,0x00,0x90,}},
        {0x17E1FC6 , {0x08,0x00,0x98,}},
        {0x1F02DE4 , {0x08,0x00,0xA0,}},
        {0x25CAB21 , {0x08,0x00,0xA8,}},
        {0x2C41C1C , {0x08,0x00,0xB0,}},
        {0x326F3C3 , {0x08,0x00,0xB8,}},
        {0x3859655 , {0x08,0x00,0xC0,}},
        {0x3E05BC8 , {0x08,0x00,0xC8,}},
        {0x43791B9 , {0x08,0x00,0xD0,}},
        {0x48B7CF8 , {0x08,0x00,0xD8,}},
        {0x4DC5AD4 , {0x08,0x00,0xE0,}},
        {0x52A6228 , {0x08,0x00,0xE8,}},
        {0x575C43A , {0x08,0x00,0xF0,}},
        {0x5BEAD74 , {0x08,0x00,0xF8,}},
        {0x6054609 , {0x08,0x01,0x00,}},
        {0x649B272 , {0x08,0x01,0x08,}},
        {0x68C13E7 , {0x08,0x01,0x10,}},
        {0x6CC88B9 , {0x08,0x01,0x18,}},
        {0x70B2CAB , {0x08,0x01,0x20,}},
        {0x7481938 , {0x08,0x01,0x28,}},
        {0x78365CF , {0x08,0x01,0x30,}},
        {0x7BD280E , {0x08,0x01,0x38,}},
        {0x7F573ED , {0x08,0x01,0x40,}},
        {0x82C5BE9 , {0x08,0x01,0x48,}},
        {0x861F12A , {0x08,0x01,0x50,}},
        {0x89643A1 , {0x08,0x01,0x58,}},
        {0x8C96226 , {0x08,0x01,0x60,}},
        {0x8FB5A8F , {0x08,0x01,0x68,}},
        {0x92C39CD , {0x08,0x01,0x70,}},
        {0x95C0BF5 , {0x08,0x01,0x78,}},
        {0x98ADC5E , {0x08,0x01,0x80,}},
        {0x9B8B5A9 , {0x08,0x01,0x88,}},
        {0x9E5A1D2 , {0x08,0x01,0x90,}},
        {0xA11AA3C , {0x08,0x01,0x98,}},
        {0xA3CD7C2 , {0x08,0x01,0xA0,}},
        {0xA6732B9 , {0x08,0x01,0xA8,}},
        {0xA90C301 , {0x08,0x01,0xB0,}},
        {0xAB9900A , {0x08,0x01,0xB8,}},
        {0xAE1A0DD , {0x08,0x01,0xC0,}},
        {0xB08FC25 , {0x08,0x01,0xC8,}},
        {0xB2FA832 , {0x08,0x01,0xD0,}},
        {0xB55AB01 , {0x08,0x01,0xD8,}},
        {0xB7B0A43 , {0x08,0x01,0xE0,}},
        {0xB9FCB60 , {0x08,0x01,0xE8,}},
        {0xBC3F37D , {0x08,0x01,0xF0,}},
        {0xBE78780 , {0x08,0x01,0xF8,}},
        {0xC0A8C12 , {0x08,0x02,0x00,}},
        {0xC2D05A6 , {0x08,0x02,0x08,}},
        {0xC4EF87B , {0x08,0x02,0x10,}},
        {0xC70689F , {0x08,0x02,0x18,}},
        {0xC9159F0 , {0x08,0x02,0x20,}},
        {0xCB1D022 , {0x08,0x02,0x28,}},
        {0xCD1CEC2 , {0x08,0x02,0x30,}},
        {0xCF15932 , {0x08,0x02,0x38,}},
        {0xD1072B4 , {0x08,0x02,0x40,}},
        {0xD2F1E65 , {0x08,0x02,0x48,}},
        {0xD4D5F41 , {0x08,0x02,0x50,}},
        {0xD6B3827 , {0x08,0x02,0x58,}},
        {0xD88ABD9 , {0x08,0x02,0x60,}},
        {0xDA5BCFA , {0x08,0x02,0x68,}},
        {0xDC26E18 , {0x08,0x02,0x70,}},
        {0xDDEC1A3 , {0x08,0x02,0x78,}},
        {0xDFAB9F6 , {0x08,0x02,0x80,}},
        {0xE165957 , {0x08,0x02,0x88,}},
        {0xE31A1F2 , {0x08,0x02,0x90,}},
        {0xE4C95E4 , {0x08,0x02,0x98,}},
        {0xE673733 , {0x08,0x02,0xA0,}},
        {0xE8187D4 , {0x08,0x02,0xA8,}},
        {0xE9B89AA , {0x08,0x02,0xB0,}},
        {0xEB53E87 , {0x08,0x02,0xB8,}},
        {0xECEA82F , {0x08,0x02,0xC0,}},
        {0xEE7C853 , {0x08,0x02,0xC8,}},
        {0xF00A099 , {0x08,0x02,0xD0,}},
        {0xF193297 , {0x08,0x02,0xD8,}},
        {0xF317FD6 , {0x08,0x02,0xE0,}},
        {0xF4989D3 , {0x08,0x02,0xE8,}},
        {0xF6151FF , {0x08,0x02,0xF0,}},
        {0xF78D9BD , {0x08,0x02,0xF8,}},
        {0xF902268 , {0x08,0x03,0x00,}},
        {0xFA72D4D , {0x08,0x03,0x08,}},
        {0xFBDFBB2 , {0x08,0x03,0x10,}},
        {0xFD48ED1 , {0x08,0x03,0x18,}},
        {0xFEAE7DB , {0x08,0x03,0x20,}},
        {0x100107F7, {0x08,0x03,0x28,}},
        {0x1016F046, {0x08,0x03,0x30,}},
        {0x102CA1DD, {0x08,0x03,0x38,}},
        {0x10421DCB, {0x08,0x03,0x40,}},
        {0x10576518, {0x08,0x03,0x48,}},
        {0x106C78C2, {0x08,0x03,0x50,}},
        {0x108159C2, {0x08,0x03,0x58,}},
        {0x1096090A, {0x08,0x03,0x60,}},
        {0x10AA8784, {0x08,0x03,0x68,}},
        {0x10BED613, {0x08,0x03,0x70,}},
        {0x10D2F597, {0x08,0x03,0x78,}},
        {0x10E6E6E7, {0x08,0x03,0x80,}},
        {0x10FAAAD5, {0x08,0x03,0x88,}},
        {0x110E422E, {0x08,0x03,0x90,}},
        {0x1121ADBA, {0x08,0x03,0x98,}},
        {0x1134EE3B, {0x08,0x03,0xA0,}},
        {0x1148046D, {0x08,0x03,0xA8,}},
        {0x115AF10A, {0x08,0x03,0xB0,}},
        {0x116DB4C5, {0x08,0x03,0xB8,}},
        {0x1180504C, {0x08,0x03,0xC0,}},
        {0x1192C44C, {0x08,0x03,0xC8,}},
        {0x11A51169, {0x08,0x03,0xD0,}},
        {0x11B73848, {0x08,0x03,0xD8,}},
        {0x11C93987, {0x08,0x03,0xE0,}},
        {0x11DB15BF, {0x08,0x03,0xE8,}},
        {0x11ECCD89, {0x08,0x03,0xF0,}},
        {0x11FE6178, {0x08,0x03,0xF8,}},
        {0x120FD21B, {0x08,0x04,0x00,}},
        {0x12212000, {0x08,0x04,0x08,}},
        {0x12324BAF, {0x08,0x04,0x10,}},
        {0x124355B0, {0x08,0x04,0x18,}},
        {0x12543E84, {0x08,0x04,0x20,}},
        {0x126506AD, {0x08,0x04,0x28,}},
        {0x1275AEA8, {0x08,0x04,0x30,}},
        {0x128636EE, {0x08,0x04,0x38,}},
        {0x12969FF9, {0x08,0x04,0x40,}},
        {0x12A6EA3C, {0x08,0x04,0x48,}},
        {0x12B7162C, {0x08,0x04,0x50,}},
        {0x12C72437, {0x08,0x04,0x58,}},
        {0x12D714CB, {0x08,0x04,0x60,}},
        {0x12E6E854, {0x08,0x04,0x68,}},
        {0x12F69F3C, {0x08,0x04,0x70,}},
        {0x130639E8, {0x08,0x04,0x78,}},
        {0x1315B8BD, {0x08,0x04,0x80,}},
        {0x13251C1F, {0x08,0x04,0x88,}},
        {0x1334646E, {0x08,0x04,0x90,}},
        {0x13439208, {0x08,0x04,0x98,}},
        {0x1352A54A, {0x08,0x04,0xA0,}},
        {0x13619E8F, {0x08,0x04,0xA8,}},
        {0x13707E30, {0x08,0x04,0xB0,}},
        {0x137F4485, {0x08,0x04,0xB8,}},
        {0x138DF1E2, {0x08,0x04,0xC0,}},
        {0x139C869B, {0x08,0x04,0xC8,}},
        {0x13AB0303, {0x08,0x04,0xD0,}},
        {0x13B9676B, {0x08,0x04,0xD8,}},
        {0x13C7B421, {0x08,0x04,0xE0,}},
        {0x13D5E972, {0x08,0x04,0xE8,}},
        {0x13E407AC, {0x08,0x04,0xF0,}},
        {0x13F20F18, {0x08,0x04,0xF8,}},
        {0x14000000, {0x08,0x05,0x00,}},
        {0x140DDAAB, {0x08,0x05,0x08,}},
        {0x141B9F60, {0x08,0x05,0x10,}},
        {0x14294E64, {0x08,0x05,0x18,}},
        {0x1436E7FC, {0x08,0x05,0x20,}},
        {0x14446C69, {0x08,0x05,0x28,}},
        {0x1451DBED, {0x08,0x05,0x30,}},
        {0x145F36CA, {0x08,0x05,0x38,}},
        {0x146C7D3C, {0x08,0x05,0x40,}},
        {0x1479AF84, {0x08,0x05,0x48,}},
        {0x1486CDDD, {0x08,0x05,0x50,}},
        {0x1493D884, {0x08,0x05,0x58,}},
        {0x14A0CFB3, {0x08,0x05,0x60,}},
        {0x14ADB3A5, {0x08,0x05,0x68,}},
        {0x14BA8491, {0x08,0x05,0x70,}},
        {0x14C742B0, {0x08,0x05,0x78,}},
        {0x14D3EE38, {0x08,0x05,0x80,}},
        {0x14E08760, {0x08,0x05,0x88,}},
        {0x14ED0E5C, {0x08,0x05,0x90,}},
        {0x14F98361, {0x08,0x05,0x98,}},
        {0x1505E6A2, {0x08,0x05,0xA0,}},
        {0x15123851, {0x08,0x05,0xA8,}},
        {0x151E78A0, {0x08,0x05,0xB0,}},
        {0x152AA7BF, {0x08,0x05,0xB8,}},
        {0x1536C5DF, {0x08,0x05,0xC0,}},
        {0x1542D32F, {0x08,0x05,0xC8,}},
        {0x154ECFDC, {0x08,0x05,0xD0,}},
        {0x155ABC16, {0x08,0x05,0xD8,}},
        {0x15669808, {0x08,0x05,0xE0,}},
        {0x157263DF, {0x08,0x05,0xE8,}},
        {0x157E1FC6, {0x08,0x05,0xF0,}},
        {0x1589CBE9, {0x08,0x05,0xF8,}},
        {0x15956871, {0x08,0x06,0x00,}},
        {0x15A0F588, {0x08,0x06,0x08,}},
        {0x15AC7356, {0x08,0x06,0x10,}},
        {0x15B7E205, {0x08,0x06,0x18,}},
        {0x15C341BB, {0x08,0x06,0x20,}},
        {0x15CE92A0, {0x08,0x06,0x28,}},
        {0x15D9D4DA, {0x08,0x06,0x30,}},
        {0x15E5088F, {0x08,0x06,0x38,}},
        {0x15F02DE4, {0x08,0x06,0x40,}},
        {0x15FB44FE, {0x08,0x06,0x48,}},
        {0x16064E00, {0x08,0x06,0x50,}},
        {0x16114910, {0x08,0x06,0x58,}},
        {0x161C364F, {0x08,0x06,0x60,}},
        {0x162715E0, {0x08,0x06,0x68,}},
        {0x1631E7E6, {0x08,0x06,0x70,}},
        {0x163CAC81, {0x08,0x06,0x78,}},
        {0x164763D4, {0x08,0x06,0x80,}},
        {0x16520DFF, {0x08,0x06,0x88,}},
        {0x165CAB21, {0x08,0x06,0x90,}},
        {0x16673B5B, {0x08,0x06,0x98,}},
        {0x1671BECB, {0x08,0x06,0xA0,}},
        {0x167C3591, {0x08,0x06,0xA8,}},
        {0x16869FCC, {0x08,0x06,0xB0,}},
        {0x1690FD98, {0x08,0x06,0xB8,}},
        {0x169B4F13, {0x08,0x06,0xC0,}},
        {0x16A5945B, {0x08,0x06,0xC8,}},
        {0x16AFCD8D, {0x08,0x06,0xD0,}},
        {0x16B9FAC4, {0x08,0x06,0xD8,}},
        {0x16C41C1C, {0x08,0x06,0xE0,}},
        {0x16CE31B2, {0x08,0x06,0xE8,}},
        {0x16D83BA0, {0x08,0x06,0xF0,}},
        {0x16E23A01, {0x08,0x06,0xF8,}},
        {0x16EC2CF0, {0x08,0x07,0x00,}},
        {0x16F61486, {0x08,0x07,0x08,}},
        {0x16FFF0DE, {0x08,0x07,0x10,}},
        {0x1709C211, {0x08,0x07,0x18,}},
        {0x17138838, {0x08,0x07,0x20,}},
        {0x171D436B, {0x08,0x07,0x28,}},
        {0x1726F3C3, {0x08,0x07,0x30,}},
        {0x17309959, {0x08,0x07,0x38,}},
        {0x173A3444, {0x08,0x07,0x40,}},
        {0x1743C49B, {0x08,0x07,0x48,}},
        {0x174D4A77, {0x08,0x07,0x50,}},
        {0x1756C5EC, {0x08,0x07,0x58,}},
        {0x17603713, {0x08,0x07,0x60,}},
        {0x17699E02, {0x08,0x07,0x68,}},
        {0x1772FACE, {0x08,0x07,0x70,}},
        {0x177C4D8D, {0x08,0x07,0x78,}},
        {0x17859655, {0x08,0x07,0x80,}},
        {0x178ED53C, {0x08,0x07,0x88,}},
        {0x17980A55, {0x08,0x07,0x90,}},
        {0x17A135B6, {0x08,0x07,0x98,}},
        {0x17AA5773, {0x08,0x07,0xA0,}},
        {0x17B36FA0, {0x08,0x07,0xA8,}},
        {0x17BC7E51, {0x08,0x07,0xB0,}},
        {0x17C5839B, {0x08,0x07,0xB8,}},
        {0x17CE7F90, {0x08,0x07,0xC0,}},
#endif
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

