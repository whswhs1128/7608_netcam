/*!
*****************************************************************************
** \file        ar0330_reg_tbl.c
**
** \version     $Id: ar0330_reg_tbl.c 11627 2017-03-10 10:58:12Z dengbiao $
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
GADI_VI_SensorDrvInfoT    ar0330_sensor =
{
    .magic_start        = GADI_SENSOR_HW_INFO_MAGIC_START,
    .version            = GADI_SENSOR_HW_INFO_VERSION,
    .length             = sizeof(GADI_VI_SensorDrvInfoT),
    .file_date          = 0x20161014,
    .HwInfo         =
    {
        .name               = "ar0330",
        .ctl_type           = GADI_SENSOR_CTL_I2C,
        .addr_len           = sizeof(GADI_U16),
        .data_len           = sizeof(GADI_U16),
        .reset_active_level = GADI_GPIO_LOW,
        .hw_addr            = {(0x20>>1), (0x30>>1), 0, 0},
        .id_reg =
        {
            {0x3000, 0x2604}, // CHIP_ID
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        .reset_reg =
        {
            {0x301A, 0x10D8}, // soft reset
                              // [12]smia_serialiser_dis 1:disable
                              // [11]forced_pll_on 1:standby
                              // [10]restart_bad 1:restart
                              // [9]mask_bad 0: The sensor will produce bad (corrupted) frames as a result of some register changes.
                              //             1: Bad (corrupted) frames are masked within the sensor by extending the vertical blanking time for the duration of the bad frame.
                              // [8]gpi_en 0: The primary input buffers associated with the OE_BAR, TRIGGER and STANDBY inputs are powered down and cannot be used.
                              //           1: The input buffers are enabled and can be read through R0x3026-7.
                              // [7]parallel_en 0: The parallel data interface (DOUT[11:0], LINE_VALID, FRAME_VALID, and PIXCLK) is disabled and the outputs are placed in a high-impedance state.
                              //                1: The parallel data interface is enabled. The output signals can be switched between a driven and a high-impedance state using output-enable control.
                              // [6]drive_pins 0: The parallel data interface (DOUT[11:0], LINE_VALID, FRAME_VALID, and PIXCLK) may enter a high-impedance state (depending upon the enabling and use of the pad OE_BAR)
                              //               1: The parallel data interface is driven. This is bit is "do not care" unless bit[7]=1.
                              // [4]reset_register_unused
                              // [3]lock_reg Many parameter limitation registers that are specified as read-only are actually implemented as read/write registers. Clearing this bit allows writing to such registers.
                              // [2]stream Setting this bit places the sensor in streaming mode. Clearing this bit places the sensor in a low power mode. The result of clearing this bit depends upon the operating mode of the sensor. Entry and exit from streaming mode can also be controlled from the signal interface.
                              // [1]restart This bit always reads as 0. Setting this bit causes the sensor to truncate the current frame at the end of the current row and start resetting (integrating) the first row. The delay before the first valid frame is read out is equal to the integration time.
                              // [0]reset This bit always reads as 0. Setting this bit initiates a reset sequence: the frame being generated will be truncated.
            {GADI_VI_SENSOR_TABLE_FLAG_MSLEEP, 100},    // msleep(100);
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
    .sensor_id          = GADI_ISP_SENSOR_AR0330,
    .sensor_double_step = 128,
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
    .emb_sync_mode      = GADI_VI_EMB_SYNC_FULL_RANGE,
    .max_width          = 2304,
    .max_height         = 1536,
    .def_sh_time        = GADI_VIDEO_FPS(30),
    .fmt_num            = 2,
    .auto_fmt           = GADI_VIDEO_MODE(1920, 1080,  30, 1),
    .init_reg           =
    {
        {0x31AE, 0x0301},
        {0x3064, 0x1802}, //Disable Embedded Data, 0x3064[8]=0
        {0x3078, 0x0001}, //Marker to say that 'Defaults' have been run
        {0x31E0, 0x0003}, //defect correction setting
        {0x306E, 0xFC10}, //slew rate
        {0x3046, 0x4038}, // Enable Flash Pin
        {0x3048, 0x8480}, // Flash Pulse Length
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .pll_table =
    {
        [0] =
        {
            // for 1080P@30
            .pixclk = 81000000, //
            .extclk = 27000000, // (6-27MHz)
            // fvco(384-768) = extclk / pre_pll_clk_div * pll_multiplier
            // pixclk(<=98MHz) = extclk / pre_pll_clk_div * pll_multiplier / vt_sys_clk_div / vt_pix_clk_div
            //        = 27 / 1 * 24 / 1 / 8 = 81
            // opclk(<=98MHz)  = extclk / pre_pll_clk_div * pll_multiplier / op_sys_clk_div / op_pix_clk_div
            //        = 27 / 1 * 24 / 1 / 8 = 81
            .regs =
            {
                {0x302A, 0x0008}, // vt_pix_clk_div(4-16)
                {0x302C, 0x0001}, // vt_sys_clk_div(1, 2, 4, 6, 8, 12, 14, 16)
                                  // Divides the input VCO clock and outputs the vt_sys_clk.
                                  // Set this divider to "2" to enable 2-lane MIPI and "4" to enable 1-lane MIPI.
                                  // Refer to the sensor datasheet for more details.
                {0x302E, 0x0001}, // pre_pll_clk_div(1-64)
                {0x3030, 0x0018}, // pll_multiplier(32-384)
                {0x3036, 0x0008}, // op_pix_clk_div Clock divisor applied to the op_sys_clk to generate the output pixel clock.
                                  // The divisor indicates the bit-depth of the output pixel word. (i.e. "12" 12-bit, "10" 10-bit, "8", 8-bit)
                {0x3038, 0x0001}, // op_sys_clk_div Clock divisor applied to PLL output clock to generate output system clock.
                                  // Can only be programmed to "1" in the AR0330 sensor. Read-only.
                {0x31AC, 0x0C0C}, // data_format is 12-bit
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,}
            },
        },
        [1] =
        {
            // for 1080P@25
            .pixclk = 74250000,
            .extclk = 27000000, // (6-27MHz)
            // fvco(384-768) = extclk / pre_pll_clk_div * pll_multiplier
            // pixclk(<=98MHz) = extclk / pre_pll_clk_div * pll_multiplier / vt_sys_clk_div / vt_pix_clk_div
            //        = 27 / 1 * 33 / 1 / 12 = 74.25
            // opclk(<=98MHz)  = extclk / pre_pll_clk_div * pll_multiplier / op_sys_clk_div / op_pix_clk_div
            //        = 27 / 1 * 33 / 1 / 12 = 74.25
            .regs =
            {
                {0x302A, 0x000C}, // vt_pix_clk_div
                {0x302C, 0x0001}, // vt_sys_clk_div
                {0x302E, 0x0001}, // pre_pll_clk_div
                {0x3030, 0x0021}, // pll_multiplier
                {0x3036, 0x000C}, // op_pix_clk_div
                {0x3038, 0x0001}, // op_sys_clk_div
                {0x31AC, 0x0C0C}, // data_format is 12-bit
                {GADI_VI_SENSOR_TABLE_FLAG_MDELAY, 0x02},// mdelay(2);
                {GADI_VI_SENSOR_TABLE_FLAG_END,}
            },
        },
    },
    .video_fmt_table =
    {
        [0] = // 1920x1080P@30fps   (1200 * 2) * 1125 * 30 = 81000000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1920, 1080,  30, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x3004, 0x00A0}, // X_ADDR_START
                {0x3008, 0x081F}, // X_ADDR_END         0x081F - 0x00A0 + 1 = 0x0780 = 1920
                {0x3002, 0x00E4}, // Y_ADDR_START
                {0x3006, 0x051B}, // Y_ADDR_END         0x051B - 0x00E4 + 1 = 0x0438 = 1080
                {0x30A2, 0x0001}, // X_ODD_INCREMENT
                {0x30A6, 0x0001}, // Y_ODD_INCREMENT
                {0x300C, 0x04B0}, // LINE_LENGTH_PCK    1200 = 0x04B0 >=0x0454
                {0x300A, 0x0465}, // FRAME_LENGTH_LINE  1125 = 0x0465
                {0x3014, 0x0000}, // FINE_INTEGRATION_TIME
                {0x3012, 0x0198}, // Coarse_Integration_Time
                {0x3042, 0x0000}, // EXTRA_DELAY
                {0x3040, 0x0000}, // Row Bin
                {0x30BA, 0x002C}, // digital_ctrl
                {0x301A, 0x0004, 0x0004}, // start_streaming
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
                .sync_start     = (0 + 5),
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_GR,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_GR,
                    GADI_VI_BAYER_PATTERN_GR,
                    GADI_VI_BAYER_PATTERN_GR,
                    GADI_VI_BAYER_PATTERN_GR,
                },
            },
            .update_after_vsync_start   = 30,
            .update_after_vsync_end     = 33,
        },
        [1] = // 1920x1080P@25fps   (1320 * 2) * 1125 * 25 = 74250000
        {
            .video_mode_table =
            {
                .mode = GADI_VIDEO_MODE(1920, 1080,  25, 1),
                .preview_mode_type = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
                .still_mode_type   = GADI_VI_SRC_ENABLED_FOR_VIDEO | GADI_VI_SRC_ENABLED_FOR_STILL,
            },
            .fmt_reg =
            {
                {0x3004, 0x00A0}, // X_ADDR_START
                {0x3008, 0x081F}, // X_ADDR_END         0x081F - 0x00A0 + 1 = 0x0780 = 1920
                {0x3002, 0x00E4}, // Y_ADDR_START
                {0x3006, 0x051B}, // Y_ADDR_END         0x051B - 0x00E4 + 1 = 0x0438 = 1080
                {0x30A2, 0x0001}, // X_ODD_INCREMENT
                {0x30A6, 0x0001}, // Y_ODD_INCREMENT
                {0x300C, 0x0528}, // LINE_LENGTH_PCK    1320 = 0x0528 >=0x0454
                {0x300A, 0x0465}, // FRAME_LENGTH_LINE  1125 = 0x0465
                {0x3014, 0x0000}, // FINE_INTEGRATION_TIME
                {0x3012, 0x0198}, // Coarse_Integration_Time
                {0x3042, 0x0000}, // EXTRA_DELAY
                {0x3040, 0x0000}, // Row Bin
                {0x30BA, 0x002C}, // digital_ctrl
                {0x301A, 0x0004, 0x0004}, // start_streaming
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
                .pll_index  = 1,
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
                .sync_start     = (0 + 5),
                .bayer_pattern  = GADI_VI_BAYER_PATTERN_GR,
                .mirror_bayer_pattern =
                {
                    GADI_VI_BAYER_PATTERN_GR,
                    GADI_VI_BAYER_PATTERN_GR,
                    GADI_VI_BAYER_PATTERN_GR,
                    GADI_VI_BAYER_PATTERN_GR,
                },
            },
            .update_after_vsync_start   = 37,
            .update_after_vsync_end     = 40,
        },
    },
    .mirror_table =
    {
#define AR0330_HORIZ_MIRROR     0x4000
#define AR0330_VERT_FLIP        0x8000
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY_VERTICALLY] =
        {
            {0x3040, AR0330_HORIZ_MIRROR | AR0330_VERT_FLIP, AR0330_HORIZ_MIRROR | AR0330_VERT_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_HORRIZONTALLY] =
        {
            {0x3040, AR0330_HORIZ_MIRROR, AR0330_HORIZ_MIRROR | AR0330_VERT_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_VERTICALLY] =
        {
            {0x3040, AR0330_VERT_FLIP, AR0330_HORIZ_MIRROR | AR0330_VERT_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
        [GADI_VI_SRC_MIRROR_NONE] =
        {
            {0x3040, 0x00, AR0330_HORIZ_MIRROR | AR0330_VERT_FLIP},
            {GADI_VI_SENSOR_TABLE_FLAG_END,},
        },
    },
    .version_reg =
    {
        {0x30F0,},
        {0x3072,},
    },
    .reg_ctl_by_ver_num = 5,
    .control_by_version_reg =
    {
        [0] =
        {
            .version = 0x12000000,
            .mask    = 0xFFFF0000,
            .regs =
            {
                // default register changes V1
                {0x30BA, 0x002C, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x30FE, 0x0080, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x31E0, 0x0000, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3ECE, 0x10FF, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3ED0, 0xE4F6, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3ED2, 0x0146, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3ED4, 0x8F6C, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3ED6, 0x66CC, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3ED8, 0x8C42, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3EDA, 0x8822, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3EDC, 0x2222, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3EDE, 0x22C0, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3EE0, 0x1500, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3EE6, 0x0080, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3EE8, 0x2027, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3EEA, 0x001D, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3F06, 0x046A, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},

                // Sequencer A, don't change
                {0x3088, 0x8000, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4540, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x6134, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4A31, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4342, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4560, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2714, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x3DFF, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x3DFF, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x3DEA, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2704, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x3D10, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2705, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x3D10, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2715, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x3527, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x053D, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x1045, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4027, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0427, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x143D, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0xFF3D, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0xFF3D, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0xEA62, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2728, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x3627, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x083D, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x6444, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2C2C, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2C2C, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4B01, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x432D, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4643, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x1647, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x435F, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4F50, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2604, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2684, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2027, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0xFC53, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0D5C, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0D60, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x5754, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x1709, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x5556, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4917, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x145C, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0945, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0045, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x8026, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0xA627, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0xF817, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0227, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0xFA5C, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0B5F, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x5307, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x5302, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4D28, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x6C4C, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0928, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2C28, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x294E, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x1718, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x26A2, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x5C03, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x1744, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2809, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x27F2, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x1714, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2808, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x164D, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x1A26, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x8317, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0145, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0xA017, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0727, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0xF317, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2945, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x8017, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0827, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0xF217, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x285D, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x27FA, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x170E, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2681, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x5300, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x17E6, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x5302, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x1710, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2683, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2682, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4827, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0xF24D, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4E28, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x094C, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0B17, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x6D28, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0817, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x014D, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x1A17, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0126, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x035C, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0045, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4027, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x9017, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2A4A, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0A43, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x160B, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4327, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x9445, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x6017, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0727, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x9517, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2545, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4017, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0827, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x905D, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2808, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x530D, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2645, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x5C01, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2798, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4B12, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4452, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x5117, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0260, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x184A, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0343, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x1604, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4316, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x5843, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x1659, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4316, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x5A43, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x165B, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4327, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x9C45, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x6017, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0727, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x9D17, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2545, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4017, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x1027, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x9817, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2022, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4B12, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x442C, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2C2C, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2C00, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
        [1] =
        {
            .version = 0x12080000,
            .mask    = 0xFFFFFFFF,
            .regs =
            {
                // default register changes V2
                {0x30BA, 0x002C, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x30FE, 0x0080, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x31E0, 0x0000, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3ECE, 0x10FF, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3ED0, 0xE4F6, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3ED2, 0x0146, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3ED4, 0x8F6C, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3ED6, 0x66CC, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3ED8, 0x8C42, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3EDA, 0x889B, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3EDC, 0x8863, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3EDE, 0xAA04, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3EE0, 0x15F0, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3EE6, 0x008C, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3EE8, 0x2024, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3EEA, 0xFF1F, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3F06, 0x046A, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},

                // Sequencer B, don't change
                {0x3088, 0x8000, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4A03, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4316, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0443, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x1645, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4045, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x6017, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2045, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x404B, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x1244, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x6134, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4A31, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4342, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4560, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2714, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x3DFF, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x3DFF, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x3DEA, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2704, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x3D10, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2705, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x3D10, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2715, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x3527, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x053D, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x1045, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4027, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0427, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x143D, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0xFF3D, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0xFF3D, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0xEA62, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2728, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x3627, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x083D, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x6444, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2C2C, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2C2C, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4B01, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x432D, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4643, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x1647, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x435F, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4F50, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2604, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2684, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2027, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0xFC53, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0D5C, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0D57, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x5417, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0955, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x5649, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x5307, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x5302, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4D28, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x6C4C, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0928, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2C28, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x294E, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x5C09, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x6045, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0045, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x8026, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0xA627, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0xF817, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0227, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0xFA5C, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0B17, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x1826, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0xA25C, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0317, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4427, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0xF25F, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2809, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x1714, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2808, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x1701, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4D1A, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2683, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x1701, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x27FA, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x45A0, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x1707, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x27FB, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x1729, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4580, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x1708, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x27FA, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x1728, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x5D17, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0E26, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x8153, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0117, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0xE653, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0217, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x1026, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x8326, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x8248, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4D4E, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2809, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4C0B, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x6017, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2027, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0xF217, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x535F, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2808, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x164D, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x1A17, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0127, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0xFA26, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x035C, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0145, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4027, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x9817, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2A4A, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0A43, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x160B, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4327, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x9C45, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x6017, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0727, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x9D17, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2545, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4017, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0827, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x985D, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2645, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4B17, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0A28, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0853, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0D52, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x5112, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4460, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x184A, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0343, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x1604, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4316, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x5843, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x1659, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4316, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x5A43, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x165B, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4327, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x9C45, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x6017, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0727, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x9D17, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2545, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4017, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x1027, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x9817, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2022, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x4B12, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x442C, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2C2C, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x2C00, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3086, 0x0000, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
        [2] =
        {
            .version = 0x12080006,
            .mask    = 0xFFFFFFFF,
            .regs =
            {
                // default register changes V3
                {0x3ED2, 0x0146, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3ED4, 0x8F6C, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3ED6, 0x66CC, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3ED8, 0x8C42, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},

                // Sequencer Patch 1, don't change
                {0x3088, 0x800C},
                {0x3086, 0x2045},
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
        [3] =
        {
            .version = 0x12080007,
            .mask    = 0xFFFFFFFF,
            .regs =
            {
                // default register changes V4
                {0x3ED2, 0x0146, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3ED6, 0x66CC, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {0x3ED8, 0x8C42, GADI_SENSOR_VER_CTL_REG_INIT_FIRST},
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            }
        },
        [4] =
        {
            .version = 0x12080008,
            .mask    = 0xFFFFFFFF,
            .regs =
            {
                // default register changes V5
                {GADI_VI_SENSOR_TABLE_FLAG_END,},
            },
        },
    },
    .vmax_clk_ref   = GADI_SENSOR_CAL_CLK_PIXCLK_DIV2,
    .shs_clk_ref    = GADI_SENSOR_CAL_CLK_PIXCLK_DIV2,
    .cal_shs_mode   = GADI_SENSOR_CAL_SHS_VMAX_MODE,
    .shs_fix        = 1,
    .shs_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_SET,
    .agc_reg_update_mode        = GADI_SENSOR_UPDATE_REG_BY_SET,
    .vmax_reg_update_mode       = GADI_SENSOR_UPDATE_REG_BY_SET,
    .mirror_reg_update_mode     = GADI_SENSOR_UPDATE_REG_BY_SET,
    .shs_additional_use         = 0,
    .shs_additional_percent     = 0,
    .hmax_reg =
    {
        {0x300C, 0x0000, 0xFFFF, 0x00, 0x00, 0}, // LINE_LENGTH_PCK
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .vmax_reg =
    {
        {0x300A, 0x0000, 0xFFFF, 0x00, 0x00, 0}, // FRAME_LENGTH_LINES
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .vmax_wadd_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    .shs_reg =
    {
        {0x3012, 0x0000, 0xFFFF, 0x00, 0x00, 0}, // COARSE_INTEGRATION_TIME
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .shs_additional_reg = {GADI_VI_SENSOR_TABLE_FLAG_END,},
    .cal_gain_mode              = GADI_SENSOR_CAL_GAIN_MODE2,
    .max_agc_index = 897,
    .max_again_index = 0,
    .cur_again_index = 0,
    .agc_info =
    {
        .db_max     = 0x2A000000,    // 42dB
        .db_min     = 0x00000000,    // 0dB
        .db_step    = 0x000C0000,    // 0.046875dB
    },
    .gain_reg =
    {
        {0x3060, 0x00}, // AR0330_GAIN_COL_REG_AGAIN
        {0x305E, 0x00}, // AR0330_GAIN_COL_REG_DGAIN
        {GADI_VI_SENSOR_TABLE_FLAG_END,},
    },
    .gain_table =
    {
        // dcg,  col_gain,  dig_gain
        // 0x0004 is high dcg gain,
        //       0x1330 is 8X column gain,
        //                  R0x305E (Context A) or R0x30C4 (Context B).
        //                  0x00FF is digital gain. digital gain step is about 0.1875 db
        //                  xxx.yyyyy The step size for yyyyy is 0.03125 while the step size for xxx is 1
        //       R0x30B0[5:4](Context A) or R0x30B0[9:8] (Context B). 1x, 2x, 4x or 8x
        //       1x or 1.25x gain which can be set in R0x3EE4[9:8]. 0xD208 1101 0010 0000 1000
        {0x000C0000 * 0x0380, {0x0030 , 0x07FF},},        /*index: 0, 42.1442dB */
        {0x000C0000 * 0x037F, {0x0030 , 0x07F5},},        /*index: 1, 42.0972dB */
        {0x000C0000 * 0x037E, {0x0030 , 0x07EA},},        /*index: 2, 42.0501dB */
        {0x000C0000 * 0x037D, {0x0030 , 0x07DF},},        /*index: 3, 42.0031dB */
        {0x000C0000 * 0x037C, {0x0030 , 0x07D4},},        /*index: 4, 41.9561dB */
        {0x000C0000 * 0x037B, {0x0030 , 0x07C9},},        /*index: 5, 41.909dB */
        {0x000C0000 * 0x037A, {0x0030 , 0x07BF},},        /*index: 6, 41.862dB */
        {0x000C0000 * 0x0379, {0x0030 , 0x07B4},},        /*index: 7, 41.8149dB */
        {0x000C0000 * 0x0378, {0x0030 , 0x07A9},},        /*index: 8, 41.7679dB */
        {0x000C0000 * 0x0377, {0x0030 , 0x079F},},        /*index: 9, 41.7209dB */
        {0x000C0000 * 0x0376, {0x0030 , 0x0794},},        /*index: 10, 41.6738dB */
        {0x000C0000 * 0x0375, {0x0030 , 0x078A},},        /*index: 11, 41.6268dB */
        {0x000C0000 * 0x0374, {0x0030 , 0x077F},},        /*index: 12, 41.5798dB */
        {0x000C0000 * 0x0373, {0x0030 , 0x0775},},        /*index: 13, 41.5327dB */
        {0x000C0000 * 0x0372, {0x0030 , 0x076A},},        /*index: 14, 41.4857dB */
        {0x000C0000 * 0x0371, {0x0030 , 0x0760},},        /*index: 15, 41.4387dB */
        {0x000C0000 * 0x0370, {0x0030 , 0x0756},},        /*index: 16, 41.3916dB */
        {0x000C0000 * 0x036F, {0x0030 , 0x074C},},        /*index: 17, 41.3446dB */
        {0x000C0000 * 0x036E, {0x0030 , 0x0742},},        /*index: 18, 41.2976dB */
        {0x000C0000 * 0x036D, {0x0030 , 0x0738},},        /*index: 19, 41.2505dB */
        {0x000C0000 * 0x036C, {0x0030 , 0x072E},},        /*index: 20, 41.2035dB */
        {0x000C0000 * 0x036B, {0x0030 , 0x0724},},        /*index: 21, 41.1564dB */
        {0x000C0000 * 0x036A, {0x0030 , 0x071A},},        /*index: 22, 41.1094dB */
        {0x000C0000 * 0x0369, {0x0030 , 0x0710},},        /*index: 23, 41.0624dB */
        {0x000C0000 * 0x0368, {0x0030 , 0x0706},},        /*index: 24, 41.0153dB */
        {0x000C0000 * 0x0367, {0x0030 , 0x06FD},},        /*index: 25, 40.9683dB */
        {0x000C0000 * 0x0366, {0x0030 , 0x06F3},},        /*index: 26, 40.9213dB */
        {0x000C0000 * 0x0365, {0x0030 , 0x06E9},},        /*index: 27, 40.8742dB */
        {0x000C0000 * 0x0364, {0x0030 , 0x06E0},},        /*index: 28, 40.8272dB */
        {0x000C0000 * 0x0363, {0x0030 , 0x06D6},},        /*index: 29, 40.7802dB */
        {0x000C0000 * 0x0362, {0x0030 , 0x06CD},},        /*index: 30, 40.7331dB */
        {0x000C0000 * 0x0361, {0x0030 , 0x06C4},},        /*index: 31, 40.6861dB */
        {0x000C0000 * 0x0360, {0x0030 , 0x06BA},},        /*index: 32, 40.639dB */
        {0x000C0000 * 0x035F, {0x0030 , 0x06B1},},        /*index: 33, 40.592dB */
        {0x000C0000 * 0x035E, {0x0030 , 0x06A8},},        /*index: 34, 40.545dB */
        {0x000C0000 * 0x035D, {0x0030 , 0x069E},},        /*index: 35, 40.4979dB */
        {0x000C0000 * 0x035C, {0x0030 , 0x0695},},        /*index: 36, 40.4509dB */
        {0x000C0000 * 0x035B, {0x0030 , 0x068C},},        /*index: 37, 40.4039dB */
        {0x000C0000 * 0x035A, {0x0030 , 0x0683},},        /*index: 38, 40.3568dB */
        {0x000C0000 * 0x0359, {0x0030 , 0x067A},},        /*index: 39, 40.3098dB */
        {0x000C0000 * 0x0358, {0x0030 , 0x0671},},        /*index: 40, 40.2628dB */
        {0x000C0000 * 0x0357, {0x0030 , 0x0668},},        /*index: 41, 40.2157dB */
        {0x000C0000 * 0x0356, {0x0030 , 0x065F},},        /*index: 42, 40.1687dB */
        {0x000C0000 * 0x0355, {0x0030 , 0x0657},},        /*index: 43, 40.1217dB */
        {0x000C0000 * 0x0354, {0x0030 , 0x064E},},        /*index: 44, 40.0746dB */
        {0x000C0000 * 0x0353, {0x0030 , 0x0645},},        /*index: 45, 40.0276dB */
        {0x000C0000 * 0x0352, {0x0030 , 0x063C},},        /*index: 46, 39.9805dB */
        {0x000C0000 * 0x0351, {0x0030 , 0x0634},},        /*index: 47, 39.9335dB */
        {0x000C0000 * 0x0350, {0x0030 , 0x062B},},        /*index: 48, 39.8865dB */
        {0x000C0000 * 0x034F, {0x0030 , 0x0623},},        /*index: 49, 39.8394dB */
        {0x000C0000 * 0x034E, {0x0030 , 0x061A},},        /*index: 50, 39.7924dB */
        {0x000C0000 * 0x034D, {0x0030 , 0x0612},},        /*index: 51, 39.7454dB */
        {0x000C0000 * 0x034C, {0x0030 , 0x0609},},        /*index: 52, 39.6983dB */
        {0x000C0000 * 0x034B, {0x0030 , 0x0601},},        /*index: 53, 39.6513dB */
        {0x000C0000 * 0x034A, {0x0030 , 0x05F9},},        /*index: 54, 39.6043dB */
        {0x000C0000 * 0x0349, {0x0030 , 0x05F0},},        /*index: 55, 39.5572dB */
        {0x000C0000 * 0x0348, {0x0030 , 0x05E8},},        /*index: 56, 39.5102dB */
        {0x000C0000 * 0x0347, {0x0030 , 0x05E0},},        /*index: 57, 39.4632dB */
        {0x000C0000 * 0x0346, {0x0030 , 0x05D8},},        /*index: 58, 39.4161dB */
        {0x000C0000 * 0x0345, {0x0030 , 0x05D0},},        /*index: 59, 39.3691dB */
        {0x000C0000 * 0x0344, {0x0030 , 0x05C8},},        /*index: 60, 39.322dB */
        {0x000C0000 * 0x0343, {0x0030 , 0x05C0},},        /*index: 61, 39.275dB */
        {0x000C0000 * 0x0342, {0x0030 , 0x05B8},},        /*index: 62, 39.228dB */
        {0x000C0000 * 0x0341, {0x0030 , 0x05B0},},        /*index: 63, 39.1809dB */
        {0x000C0000 * 0x0340, {0x0030 , 0x05A8},},        /*index: 64, 39.1339dB */
        {0x000C0000 * 0x033F, {0x0030 , 0x05A0},},        /*index: 65, 39.0869dB */
        {0x000C0000 * 0x033E, {0x0030 , 0x0599},},        /*index: 66, 39.0398dB */
        {0x000C0000 * 0x033D, {0x0030 , 0x0591},},        /*index: 67, 38.9928dB */
        {0x000C0000 * 0x033C, {0x0030 , 0x0589},},        /*index: 68, 38.9458dB */
        {0x000C0000 * 0x033B, {0x0030 , 0x0581},},        /*index: 69, 38.8987dB */
        {0x000C0000 * 0x033A, {0x0030 , 0x057A},},        /*index: 70, 38.8517dB */
        {0x000C0000 * 0x0339, {0x0030 , 0x0572},},        /*index: 71, 38.8046dB */
        {0x000C0000 * 0x0338, {0x0030 , 0x056B},},        /*index: 72, 38.7576dB */
        {0x000C0000 * 0x0337, {0x0030 , 0x0563},},        /*index: 73, 38.7106dB */
        {0x000C0000 * 0x0336, {0x0030 , 0x055C},},        /*index: 74, 38.6635dB */
        {0x000C0000 * 0x0335, {0x0030 , 0x0554},},        /*index: 75, 38.6165dB */
        {0x000C0000 * 0x0334, {0x0030 , 0x054D},},        /*index: 76, 38.5695dB */
        {0x000C0000 * 0x0333, {0x0030 , 0x0546},},        /*index: 77, 38.5224dB */
        {0x000C0000 * 0x0332, {0x0030 , 0x053E},},        /*index: 78, 38.4754dB */
        {0x000C0000 * 0x0331, {0x0030 , 0x0537},},        /*index: 79, 38.4284dB */
        {0x000C0000 * 0x0330, {0x0030 , 0x0530},},        /*index: 80, 38.3813dB */
        {0x000C0000 * 0x032F, {0x0030 , 0x0529},},        /*index: 81, 38.3343dB */
        {0x000C0000 * 0x032E, {0x0030 , 0x0522},},        /*index: 82, 38.2873dB */
        {0x000C0000 * 0x032D, {0x0030 , 0x051B},},        /*index: 83, 38.2402dB */
        {0x000C0000 * 0x032C, {0x0030 , 0x0514},},        /*index: 84, 38.1932dB */
        {0x000C0000 * 0x032B, {0x0030 , 0x050C},},        /*index: 85, 38.1461dB */
        {0x000C0000 * 0x032A, {0x0030 , 0x0506},},        /*index: 86, 38.0991dB */
        {0x000C0000 * 0x0329, {0x0030 , 0x04FF},},        /*index: 87, 38.0521dB */
        {0x000C0000 * 0x0328, {0x0030 , 0x04F8},},        /*index: 88, 38.005dB */
        {0x000C0000 * 0x0327, {0x0030 , 0x04F1},},        /*index: 89, 37.958dB */
        {0x000C0000 * 0x0326, {0x0030 , 0x04EA},},        /*index: 90, 37.911dB */
        {0x000C0000 * 0x0325, {0x0030 , 0x04E3},},        /*index: 91, 37.8639dB */
        {0x000C0000 * 0x0324, {0x0030 , 0x04DC},},        /*index: 92, 37.8169dB */
        {0x000C0000 * 0x0323, {0x0030 , 0x04D6},},        /*index: 93, 37.7699dB */
        {0x000C0000 * 0x0322, {0x0030 , 0x04CF},},        /*index: 94, 37.7228dB */
        {0x000C0000 * 0x0321, {0x0030 , 0x04C8},},        /*index: 95, 37.6758dB */
        {0x000C0000 * 0x0320, {0x0030 , 0x04C2},},        /*index: 96, 37.6287dB */
        {0x000C0000 * 0x031F, {0x0030 , 0x04BB},},        /*index: 97, 37.5817dB */
        {0x000C0000 * 0x031E, {0x0030 , 0x04B5},},        /*index: 98, 37.5347dB */
        {0x000C0000 * 0x031D, {0x0030 , 0x04AE},},        /*index: 99, 37.4876dB */
        {0x000C0000 * 0x031C, {0x0030 , 0x04A8},},        /*index: 100, 37.4406dB */
        {0x000C0000 * 0x031B, {0x0030 , 0x04A1},},        /*index: 101, 37.3936dB */
        {0x000C0000 * 0x031A, {0x0030 , 0x049B},},        /*index: 102, 37.3465dB */
        {0x000C0000 * 0x0319, {0x0030 , 0x0494},},        /*index: 103, 37.2995dB */
        {0x000C0000 * 0x0318, {0x0030 , 0x048E},},        /*index: 104, 37.2525dB */
        {0x000C0000 * 0x0317, {0x0030 , 0x0488},},        /*index: 105, 37.2054dB */
        {0x000C0000 * 0x0316, {0x0030 , 0x0482},},        /*index: 106, 37.1584dB */
        {0x000C0000 * 0x0315, {0x0030 , 0x047B},},        /*index: 107, 37.1114dB */
        {0x000C0000 * 0x0314, {0x0030 , 0x0475},},        /*index: 108, 37.0643dB */
        {0x000C0000 * 0x0313, {0x0030 , 0x046F},},        /*index: 109, 37.0173dB */
        {0x000C0000 * 0x0312, {0x0030 , 0x0469},},        /*index: 110, 36.9702dB */
        {0x000C0000 * 0x0311, {0x0030 , 0x0463},},        /*index: 111, 36.9232dB */
        {0x000C0000 * 0x0310, {0x0030 , 0x045D},},        /*index: 112, 36.8762dB */
        {0x000C0000 * 0x030F, {0x0030 , 0x0457},},        /*index: 113, 36.8291dB */
        {0x000C0000 * 0x030E, {0x0030 , 0x0451},},        /*index: 114, 36.7821dB */
        {0x000C0000 * 0x030D, {0x0030 , 0x044B},},        /*index: 115, 36.7351dB */
        {0x000C0000 * 0x030C, {0x0030 , 0x0445},},        /*index: 116, 36.688dB */
        {0x000C0000 * 0x030B, {0x0030 , 0x043F},},        /*index: 117, 36.641dB */
        {0x000C0000 * 0x030A, {0x0030 , 0x0439},},        /*index: 118, 36.594dB */
        {0x000C0000 * 0x0309, {0x0030 , 0x0433},},        /*index: 119, 36.5469dB */
        {0x000C0000 * 0x0308, {0x0030 , 0x042D},},        /*index: 120, 36.4999dB */
        {0x000C0000 * 0x0307, {0x0030 , 0x0428},},        /*index: 121, 36.4529dB */
        {0x000C0000 * 0x0306, {0x0030 , 0x0422},},        /*index: 122, 36.4058dB */
        {0x000C0000 * 0x0305, {0x0030 , 0x041C},},        /*index: 123, 36.3588dB */
        {0x000C0000 * 0x0304, {0x0030 , 0x0416},},        /*index: 124, 36.3117dB */
        {0x000C0000 * 0x0303, {0x0030 , 0x0411},},        /*index: 125, 36.2647dB */
        {0x000C0000 * 0x0302, {0x0030 , 0x040B},},        /*index: 126, 36.2177dB */
        {0x000C0000 * 0x0301, {0x0030 , 0x0406},},        /*index: 127, 36.1706dB */
        {0x000C0000 * 0x0300, {0x0030 , 0x0400},},        /*index: 128, 36.1236dB */
        {0x000C0000 * 0x02FF, {0x0030 , 0x03FA},},        /*index: 129, 36.0766dB */
        {0x000C0000 * 0x02FE, {0x0030 , 0x03F5},},        /*index: 130, 36.0295dB */
        {0x000C0000 * 0x02FD, {0x0030 , 0x03EF},},        /*index: 131, 35.9825dB */
        {0x000C0000 * 0x02FC, {0x0030 , 0x03EA},},        /*index: 132, 35.9355dB */
        {0x000C0000 * 0x02FB, {0x0030 , 0x03E5},},        /*index: 133, 35.8884dB */
        {0x000C0000 * 0x02FA, {0x0030 , 0x03DF},},        /*index: 134, 35.8414dB */
        {0x000C0000 * 0x02F9, {0x0030 , 0x03DA},},        /*index: 135, 35.7943dB */
        {0x000C0000 * 0x02F8, {0x0030 , 0x03D5},},        /*index: 136, 35.7473dB */
        {0x000C0000 * 0x02F7, {0x0030 , 0x03CF},},        /*index: 137, 35.7003dB */
        {0x000C0000 * 0x02F6, {0x0030 , 0x03CA},},        /*index: 138, 35.6532dB */
        {0x000C0000 * 0x02F5, {0x0030 , 0x03C5},},        /*index: 139, 35.6062dB */
        {0x000C0000 * 0x02F4, {0x0030 , 0x03C0},},        /*index: 140, 35.5592dB */
        {0x000C0000 * 0x02F3, {0x0030 , 0x03BA},},        /*index: 141, 35.5121dB */
        {0x000C0000 * 0x02F2, {0x0030 , 0x03B5},},        /*index: 142, 35.4651dB */
        {0x000C0000 * 0x02F1, {0x0030 , 0x03B0},},        /*index: 143, 35.4181dB */
        {0x000C0000 * 0x02F0, {0x0030 , 0x03AB},},        /*index: 144, 35.371dB */
        {0x000C0000 * 0x02EF, {0x0030 , 0x03A6},},        /*index: 145, 35.324dB */
        {0x000C0000 * 0x02EE, {0x0030 , 0x03A1},},        /*index: 146, 35.277dB */
        {0x000C0000 * 0x02ED, {0x0030 , 0x039C},},        /*index: 147, 35.2299dB */
        {0x000C0000 * 0x02EC, {0x0030 , 0x0397},},        /*index: 148, 35.1829dB */
        {0x000C0000 * 0x02EB, {0x0030 , 0x0392},},        /*index: 149, 35.1358dB */
        {0x000C0000 * 0x02EA, {0x0030 , 0x038D},},        /*index: 150, 35.0888dB */
        {0x000C0000 * 0x02E9, {0x0030 , 0x0388},},        /*index: 151, 35.0418dB */
        {0x000C0000 * 0x02E8, {0x0030 , 0x0383},},        /*index: 152, 34.9947dB */
        {0x000C0000 * 0x02E7, {0x0030 , 0x037E},},        /*index: 153, 34.9477dB */
        {0x000C0000 * 0x02E6, {0x0030 , 0x037A},},        /*index: 154, 34.9007dB */
        {0x000C0000 * 0x02E5, {0x0030 , 0x0375},},        /*index: 155, 34.8536dB */
        {0x000C0000 * 0x02E4, {0x0030 , 0x0370},},        /*index: 156, 34.8066dB */
        {0x000C0000 * 0x02E3, {0x0030 , 0x036B},},        /*index: 157, 34.7596dB */
        {0x000C0000 * 0x02E2, {0x0030 , 0x0366},},        /*index: 158, 34.7125dB */
        {0x000C0000 * 0x02E1, {0x0030 , 0x0362},},        /*index: 159, 34.6655dB */
        {0x000C0000 * 0x02E0, {0x0030 , 0x035D},},        /*index: 160, 34.6184dB */
        {0x000C0000 * 0x02DF, {0x0030 , 0x0358},},        /*index: 161, 34.5714dB */
        {0x000C0000 * 0x02DE, {0x0030 , 0x0354},},        /*index: 162, 34.5244dB */
        {0x000C0000 * 0x02DD, {0x0030 , 0x034F},},        /*index: 163, 34.4773dB */
        {0x000C0000 * 0x02DC, {0x0030 , 0x034B},},        /*index: 164, 34.4303dB */
        {0x000C0000 * 0x02DB, {0x0030 , 0x0346},},        /*index: 165, 34.3833dB */
        {0x000C0000 * 0x02DA, {0x0030 , 0x0342},},        /*index: 166, 34.3362dB */
        {0x000C0000 * 0x02D9, {0x0030 , 0x033D},},        /*index: 167, 34.2892dB */
        {0x000C0000 * 0x02D8, {0x0030 , 0x0339},},        /*index: 168, 34.2422dB */
        {0x000C0000 * 0x02D7, {0x0030 , 0x0334},},        /*index: 169, 34.1951dB */
        {0x000C0000 * 0x02D6, {0x0030 , 0x0330},},        /*index: 170, 34.1481dB */
        {0x000C0000 * 0x02D5, {0x0030 , 0x032B},},        /*index: 171, 34.1011dB */
        {0x000C0000 * 0x02D4, {0x0030 , 0x0327},},        /*index: 172, 34.054dB */
        {0x000C0000 * 0x02D3, {0x0030 , 0x0323},},        /*index: 173, 34.007dB */
        {0x000C0000 * 0x02D2, {0x0030 , 0x031E},},        /*index: 174, 33.9599dB */
        {0x000C0000 * 0x02D1, {0x0030 , 0x031A},},        /*index: 175, 33.9129dB */
        {0x000C0000 * 0x02D0, {0x0030 , 0x0316},},        /*index: 176, 33.8659dB */
        {0x000C0000 * 0x02CF, {0x0030 , 0x0311},},        /*index: 177, 33.8188dB */
        {0x000C0000 * 0x02CE, {0x0030 , 0x030D},},        /*index: 178, 33.7718dB */
        {0x000C0000 * 0x02CD, {0x0030 , 0x0309},},        /*index: 179, 33.7248dB */
        {0x000C0000 * 0x02CC, {0x0030 , 0x0305},},        /*index: 180, 33.6777dB */
        {0x000C0000 * 0x02CB, {0x0030 , 0x0301},},        /*index: 181, 33.6307dB */
        {0x000C0000 * 0x02CA, {0x0030 , 0x02FC},},        /*index: 182, 33.5837dB */
        {0x000C0000 * 0x02C9, {0x0030 , 0x02F8},},        /*index: 183, 33.5366dB */
        {0x000C0000 * 0x02C8, {0x0030 , 0x02F4},},        /*index: 184, 33.4896dB */
        {0x000C0000 * 0x02C7, {0x0030 , 0x02F0},},        /*index: 185, 33.4426dB */
        {0x000C0000 * 0x02C6, {0x0030 , 0x02EC},},        /*index: 186, 33.3955dB */
        {0x000C0000 * 0x02C5, {0x0030 , 0x02E8},},        /*index: 187, 33.3485dB */
        {0x000C0000 * 0x02C4, {0x0030 , 0x02E4},},        /*index: 188, 33.3014dB */
        {0x000C0000 * 0x02C3, {0x0030 , 0x02E0},},        /*index: 189, 33.2544dB */
        {0x000C0000 * 0x02C2, {0x0030 , 0x02DC},},        /*index: 190, 33.2074dB */
        {0x000C0000 * 0x02C1, {0x0030 , 0x02D8},},        /*index: 191, 33.1603dB */
        {0x000C0000 * 0x02C0, {0x0030 , 0x02D4},},        /*index: 192, 33.1133dB */
        {0x000C0000 * 0x02BF, {0x0030 , 0x02D0},},        /*index: 193, 33.0663dB */
        {0x000C0000 * 0x02BE, {0x0030 , 0x02CC},},        /*index: 194, 33.0192dB */
        {0x000C0000 * 0x02BD, {0x0030 , 0x02C8},},        /*index: 195, 32.9722dB */
        {0x000C0000 * 0x02BC, {0x0030 , 0x02C5},},        /*index: 196, 32.9252dB */
        {0x000C0000 * 0x02BB, {0x0030 , 0x02C1},},        /*index: 197, 32.8781dB */
        {0x000C0000 * 0x02BA, {0x0030 , 0x02BD},},        /*index: 198, 32.8311dB */
        {0x000C0000 * 0x02B9, {0x0030 , 0x02B9},},        /*index: 199, 32.784dB */
        {0x000C0000 * 0x02B8, {0x0030 , 0x02B5},},        /*index: 200, 32.737dB */
        {0x000C0000 * 0x02B7, {0x0030 , 0x02B2},},        /*index: 201, 32.69dB */
        {0x000C0000 * 0x02B6, {0x0030 , 0x02AE},},        /*index: 202, 32.6429dB */
        {0x000C0000 * 0x02B5, {0x0030 , 0x02AA},},        /*index: 203, 32.5959dB */
        {0x000C0000 * 0x02B4, {0x0030 , 0x02A7},},        /*index: 204, 32.5489dB */
        {0x000C0000 * 0x02B3, {0x0030 , 0x02A3},},        /*index: 205, 32.5018dB */
        {0x000C0000 * 0x02B2, {0x0030 , 0x029F},},        /*index: 206, 32.4548dB */
        {0x000C0000 * 0x02B1, {0x0030 , 0x029C},},        /*index: 207, 32.4078dB */
        {0x000C0000 * 0x02B0, {0x0030 , 0x0298},},        /*index: 208, 32.3607dB */
        {0x000C0000 * 0x02AF, {0x0030 , 0x0294},},        /*index: 209, 32.3137dB */
        {0x000C0000 * 0x02AE, {0x0030 , 0x0291},},        /*index: 210, 32.2667dB */
        {0x000C0000 * 0x02AD, {0x0030 , 0x028D},},        /*index: 211, 32.2196dB */
        {0x000C0000 * 0x02AC, {0x0030 , 0x028A},},        /*index: 212, 32.1726dB */
        {0x000C0000 * 0x02AB, {0x0030 , 0x0286},},        /*index: 213, 32.1255dB */
        {0x000C0000 * 0x02AA, {0x0030 , 0x0283},},        /*index: 214, 32.0785dB */
        {0x000C0000 * 0x02A9, {0x0030 , 0x027F},},        /*index: 215, 32.0315dB */
        {0x000C0000 * 0x02A8, {0x0030 , 0x027C},},        /*index: 216, 31.9844dB */
        {0x000C0000 * 0x02A7, {0x0030 , 0x0278},},        /*index: 217, 31.9374dB */
        {0x000C0000 * 0x02A6, {0x0030 , 0x0275},},        /*index: 218, 31.8904dB */
        {0x000C0000 * 0x02A5, {0x0030 , 0x0272},},        /*index: 219, 31.8433dB */
        {0x000C0000 * 0x02A4, {0x0030 , 0x026E},},        /*index: 220, 31.7963dB */
        {0x000C0000 * 0x02A3, {0x0030 , 0x026B},},        /*index: 221, 31.7493dB */
        {0x000C0000 * 0x02A2, {0x0030 , 0x0268},},        /*index: 222, 31.7022dB */
        {0x000C0000 * 0x02A1, {0x0030 , 0x0264},},        /*index: 223, 31.6552dB */
        {0x000C0000 * 0x02A0, {0x0030 , 0x0261},},        /*index: 224, 31.6081dB */
        {0x000C0000 * 0x029F, {0x0030 , 0x025E},},        /*index: 225, 31.5611dB */
        {0x000C0000 * 0x029E, {0x0030 , 0x025A},},        /*index: 226, 31.5141dB */
        {0x000C0000 * 0x029D, {0x0030 , 0x0257},},        /*index: 227, 31.467dB */
        {0x000C0000 * 0x029C, {0x0030 , 0x0254},},        /*index: 228, 31.42dB */
        {0x000C0000 * 0x029B, {0x0030 , 0x0251},},        /*index: 229, 31.373dB */
        {0x000C0000 * 0x029A, {0x0030 , 0x024D},},        /*index: 230, 31.3259dB */
        {0x000C0000 * 0x0299, {0x0030 , 0x024A},},        /*index: 231, 31.2789dB */
        {0x000C0000 * 0x0298, {0x0030 , 0x0247},},        /*index: 232, 31.2319dB */
        {0x000C0000 * 0x0297, {0x0030 , 0x0244},},        /*index: 233, 31.1848dB */
        {0x000C0000 * 0x0296, {0x0030 , 0x0241},},        /*index: 234, 31.1378dB */
        {0x000C0000 * 0x0295, {0x0030 , 0x023E},},        /*index: 235, 31.0908dB */
        {0x000C0000 * 0x0294, {0x0030 , 0x023B},},        /*index: 236, 31.0437dB */
        {0x000C0000 * 0x0293, {0x0030 , 0x0237},},        /*index: 237, 30.9967dB */
        {0x000C0000 * 0x0292, {0x0030 , 0x0234},},        /*index: 238, 30.9496dB */
        {0x000C0000 * 0x0291, {0x0030 , 0x0231},},        /*index: 239, 30.9026dB */
        {0x000C0000 * 0x0290, {0x0030 , 0x022E},},        /*index: 240, 30.8556dB */
        {0x000C0000 * 0x028F, {0x0030 , 0x022B},},        /*index: 241, 30.8085dB */
        {0x000C0000 * 0x028E, {0x0030 , 0x0228},},        /*index: 242, 30.7615dB */
        {0x000C0000 * 0x028D, {0x0030 , 0x0225},},        /*index: 243, 30.7145dB */
        {0x000C0000 * 0x028C, {0x0030 , 0x0222},},        /*index: 244, 30.6674dB */
        {0x000C0000 * 0x028B, {0x0030 , 0x021F},},        /*index: 245, 30.6204dB */
        {0x000C0000 * 0x028A, {0x0030 , 0x021C},},        /*index: 246, 30.5734dB */
        {0x000C0000 * 0x0289, {0x0030 , 0x021A},},        /*index: 247, 30.5263dB */
        {0x000C0000 * 0x0288, {0x0030 , 0x0217},},        /*index: 248, 30.4793dB */
        {0x000C0000 * 0x0287, {0x0030 , 0x0214},},        /*index: 249, 30.4323dB */
        {0x000C0000 * 0x0286, {0x0030 , 0x0211},},        /*index: 250, 30.3852dB */
        {0x000C0000 * 0x0285, {0x0030 , 0x020E},},        /*index: 251, 30.3382dB */
        {0x000C0000 * 0x0284, {0x0030 , 0x020B},},        /*index: 252, 30.2911dB */
        {0x000C0000 * 0x0283, {0x0030 , 0x0208},},        /*index: 253, 30.2441dB */
        {0x000C0000 * 0x0282, {0x0030 , 0x0206},},        /*index: 254, 30.1971dB */
        {0x000C0000 * 0x0281, {0x0030 , 0x0203},},        /*index: 255, 30.15dB */
        {0x000C0000 * 0x0280, {0x0030 , 0x0200},},        /*index: 256, 30.103dB */
        {0x000C0000 * 0x027F, {0x0030 , 0x01FD},},        /*index: 257, 30.056dB */
        {0x000C0000 * 0x027E, {0x0030 , 0x01FA},},        /*index: 258, 30.0089dB */
        {0x000C0000 * 0x027D, {0x0030 , 0x01F8},},        /*index: 259, 29.9619dB */
        {0x000C0000 * 0x027C, {0x0030 , 0x01F5},},        /*index: 260, 29.9149dB */
        {0x000C0000 * 0x027B, {0x0030 , 0x01F2},},        /*index: 261, 29.8678dB */
        {0x000C0000 * 0x027A, {0x0030 , 0x01F0},},        /*index: 262, 29.8208dB */
        {0x000C0000 * 0x0279, {0x0030 , 0x01ED},},        /*index: 263, 29.7737dB */
        {0x000C0000 * 0x0278, {0x0030 , 0x01EA},},        /*index: 264, 29.7267dB */
        {0x000C0000 * 0x0277, {0x0030 , 0x01E8},},        /*index: 265, 29.6797dB */
        {0x000C0000 * 0x0276, {0x0030 , 0x01E5},},        /*index: 266, 29.6326dB */
        {0x000C0000 * 0x0275, {0x0030 , 0x01E2},},        /*index: 267, 29.5856dB */
        {0x000C0000 * 0x0274, {0x0030 , 0x01E0},},        /*index: 268, 29.5386dB */
        {0x000C0000 * 0x0273, {0x0030 , 0x01DD},},        /*index: 269, 29.4915dB */
        {0x000C0000 * 0x0272, {0x0030 , 0x01DB},},        /*index: 270, 29.4445dB */
        {0x000C0000 * 0x0271, {0x0030 , 0x01D8},},        /*index: 271, 29.3975dB */
        {0x000C0000 * 0x0270, {0x0030 , 0x01D6},},        /*index: 272, 29.3504dB */
        {0x000C0000 * 0x026F, {0x0030 , 0x01D3},},        /*index: 273, 29.3034dB */
        {0x000C0000 * 0x026E, {0x0030 , 0x01D0},},        /*index: 274, 29.2564dB */
        {0x000C0000 * 0x026D, {0x0030 , 0x01CE},},        /*index: 275, 29.2093dB */
        {0x000C0000 * 0x026C, {0x0030 , 0x01CB},},        /*index: 276, 29.1623dB */
        {0x000C0000 * 0x026B, {0x0030 , 0x01C9},},        /*index: 277, 29.1152dB */
        {0x000C0000 * 0x026A, {0x0030 , 0x01C6},},        /*index: 278, 29.0682dB */
        {0x000C0000 * 0x0269, {0x0030 , 0x01C4},},        /*index: 279, 29.0212dB */
        {0x000C0000 * 0x0268, {0x0030 , 0x01C2},},        /*index: 280, 28.9741dB */
        {0x000C0000 * 0x0267, {0x0030 , 0x01BF},},        /*index: 281, 28.9271dB */
        {0x000C0000 * 0x0266, {0x0030 , 0x01BD},},        /*index: 282, 28.8801dB */
        {0x000C0000 * 0x0265, {0x0030 , 0x01BA},},        /*index: 283, 28.833dB */
        {0x000C0000 * 0x0264, {0x0030 , 0x01B8},},        /*index: 284, 28.786dB */
        {0x000C0000 * 0x0263, {0x0030 , 0x01B6},},        /*index: 285, 28.739dB */
        {0x000C0000 * 0x0262, {0x0030 , 0x01B3},},        /*index: 286, 28.6919dB */
        {0x000C0000 * 0x0261, {0x0030 , 0x01B1},},        /*index: 287, 28.6449dB */
        {0x000C0000 * 0x0260, {0x0030 , 0x01AF},},        /*index: 288, 28.5978dB */
        {0x000C0000 * 0x025F, {0x0030 , 0x01AC},},        /*index: 289, 28.5508dB */
        {0x000C0000 * 0x025E, {0x0030 , 0x01AA},},        /*index: 290, 28.5038dB */
        {0x000C0000 * 0x025D, {0x0030 , 0x01A8},},        /*index: 291, 28.4567dB */
        {0x000C0000 * 0x025C, {0x0030 , 0x01A5},},        /*index: 292, 28.4097dB */
        {0x000C0000 * 0x025B, {0x0030 , 0x01A3},},        /*index: 293, 28.3627dB */
        {0x000C0000 * 0x025A, {0x0030 , 0x01A1},},        /*index: 294, 28.3156dB */
        {0x000C0000 * 0x0259, {0x0030 , 0x019F},},        /*index: 295, 28.2686dB */
        {0x000C0000 * 0x0258, {0x0030 , 0x019C},},        /*index: 296, 28.2216dB */
        {0x000C0000 * 0x0257, {0x0030 , 0x019A},},        /*index: 297, 28.1745dB */
        {0x000C0000 * 0x0256, {0x0030 , 0x0198},},        /*index: 298, 28.1275dB */
        {0x000C0000 * 0x0255, {0x0030 , 0x0196},},        /*index: 299, 28.0805dB */
        {0x000C0000 * 0x0254, {0x0030 , 0x0193},},        /*index: 300, 28.0334dB */
        {0x000C0000 * 0x0253, {0x0030 , 0x0191},},        /*index: 301, 27.9864dB */
        {0x000C0000 * 0x0252, {0x0030 , 0x018F},},        /*index: 302, 27.9393dB */
        {0x000C0000 * 0x0251, {0x0030 , 0x018D},},        /*index: 303, 27.8923dB */
        {0x000C0000 * 0x0250, {0x0030 , 0x018B},},        /*index: 304, 27.8453dB */
        {0x000C0000 * 0x024F, {0x0030 , 0x0189},},        /*index: 305, 27.7982dB */
        {0x000C0000 * 0x024E, {0x0030 , 0x0187},},        /*index: 306, 27.7512dB */
        {0x000C0000 * 0x024D, {0x0030 , 0x0184},},        /*index: 307, 27.7042dB */
        {0x000C0000 * 0x024C, {0x0030 , 0x0182},},        /*index: 308, 27.6571dB */
        {0x000C0000 * 0x024B, {0x0030 , 0x0180},},        /*index: 309, 27.6101dB */
        {0x000C0000 * 0x024A, {0x0030 , 0x017E},},        /*index: 310, 27.5631dB */
        {0x000C0000 * 0x0249, {0x0030 , 0x017C},},        /*index: 311, 27.516dB */
        {0x000C0000 * 0x0248, {0x0030 , 0x017A},},        /*index: 312, 27.469dB */
        {0x000C0000 * 0x0247, {0x0030 , 0x0178},},        /*index: 313, 27.422dB */
        {0x000C0000 * 0x0246, {0x0030 , 0x0176},},        /*index: 314, 27.3749dB */
        {0x000C0000 * 0x0245, {0x0030 , 0x0174},},        /*index: 315, 27.3279dB */
        {0x000C0000 * 0x0244, {0x0030 , 0x0172},},        /*index: 316, 27.2808dB */
        {0x000C0000 * 0x0243, {0x0030 , 0x0170},},        /*index: 317, 27.2338dB */
        {0x000C0000 * 0x0242, {0x0030 , 0x016E},},        /*index: 318, 27.1868dB */
        {0x000C0000 * 0x0241, {0x0030 , 0x016C},},        /*index: 319, 27.1397dB */
        {0x000C0000 * 0x0240, {0x0030 , 0x016A},},        /*index: 320, 27.0927dB */
        {0x000C0000 * 0x023F, {0x0030 , 0x0168},},        /*index: 321, 27.0457dB */
        {0x000C0000 * 0x023E, {0x0030 , 0x0166},},        /*index: 322, 26.9986dB */
        {0x000C0000 * 0x023D, {0x0030 , 0x0164},},        /*index: 323, 26.9516dB */
        {0x000C0000 * 0x023C, {0x0030 , 0x0162},},        /*index: 324, 26.9046dB */
        {0x000C0000 * 0x023B, {0x0030 , 0x0160},},        /*index: 325, 26.8575dB */
        {0x000C0000 * 0x023A, {0x0030 , 0x015E},},        /*index: 326, 26.8105dB */
        {0x000C0000 * 0x0239, {0x0030 , 0x015D},},        /*index: 327, 26.7634dB */
        {0x000C0000 * 0x0238, {0x0030 , 0x015B},},        /*index: 328, 26.7164dB */
        {0x000C0000 * 0x0237, {0x0030 , 0x0159},},        /*index: 329, 26.6694dB */
        {0x000C0000 * 0x0236, {0x0030 , 0x0157},},        /*index: 330, 26.6223dB */
        {0x000C0000 * 0x0235, {0x0030 , 0x0155},},        /*index: 331, 26.5753dB */
        {0x000C0000 * 0x0234, {0x0030 , 0x0153},},        /*index: 332, 26.5283dB */
        {0x000C0000 * 0x0233, {0x0030 , 0x0151},},        /*index: 333, 26.4812dB */
        {0x000C0000 * 0x0232, {0x0030 , 0x0150},},        /*index: 334, 26.4342dB */
        {0x000C0000 * 0x0231, {0x0030 , 0x014E},},        /*index: 335, 26.3872dB */
        {0x000C0000 * 0x0230, {0x0030 , 0x014C},},        /*index: 336, 26.3401dB */
        {0x000C0000 * 0x022F, {0x0030 , 0x014A},},        /*index: 337, 26.2931dB */
        {0x000C0000 * 0x022E, {0x0030 , 0x0148},},        /*index: 338, 26.2461dB */
        {0x000C0000 * 0x022D, {0x0030 , 0x0147},},        /*index: 339, 26.199dB */
        {0x000C0000 * 0x022C, {0x0030 , 0x0145},},        /*index: 340, 26.152dB */
        {0x000C0000 * 0x022B, {0x0030 , 0x0143},},        /*index: 341, 26.1049dB */
        {0x000C0000 * 0x022A, {0x0030 , 0x0141},},        /*index: 342, 26.0579dB */
        {0x000C0000 * 0x0229, {0x0030 , 0x0140},},        /*index: 343, 26.0109dB */
        {0x000C0000 * 0x0228, {0x0030 , 0x013E},},        /*index: 344, 25.9638dB */
        {0x000C0000 * 0x0227, {0x0030 , 0x013C},},        /*index: 345, 25.9168dB */
        {0x000C0000 * 0x0226, {0x0030 , 0x013A},},        /*index: 346, 25.8698dB */
        {0x000C0000 * 0x0225, {0x0030 , 0x0139},},        /*index: 347, 25.8227dB */
        {0x000C0000 * 0x0224, {0x0030 , 0x0137},},        /*index: 348, 25.7757dB */
        {0x000C0000 * 0x0223, {0x0030 , 0x0135},},        /*index: 349, 25.7287dB */
        {0x000C0000 * 0x0222, {0x0030 , 0x0134},},        /*index: 350, 25.6816dB */
        {0x000C0000 * 0x0221, {0x0030 , 0x0132},},        /*index: 351, 25.6346dB */
        {0x000C0000 * 0x0220, {0x0030 , 0x0130},},        /*index: 352, 25.5875dB */
        {0x000C0000 * 0x021F, {0x0030 , 0x012F},},        /*index: 353, 25.5405dB */
        {0x000C0000 * 0x021E, {0x0030 , 0x012D},},        /*index: 354, 25.4935dB */
        {0x000C0000 * 0x021D, {0x0030 , 0x012C},},        /*index: 355, 25.4464dB */
        {0x000C0000 * 0x021C, {0x0030 , 0x012A},},        /*index: 356, 25.3994dB */
        {0x000C0000 * 0x021B, {0x0030 , 0x0128},},        /*index: 357, 25.3524dB */
        {0x000C0000 * 0x021A, {0x0030 , 0x0127},},        /*index: 358, 25.3053dB */
        {0x000C0000 * 0x0219, {0x0030 , 0x0125},},        /*index: 359, 25.2583dB */
        {0x000C0000 * 0x0218, {0x0030 , 0x0124},},        /*index: 360, 25.2113dB */
        {0x000C0000 * 0x0217, {0x0030 , 0x0122},},        /*index: 361, 25.1642dB */
        {0x000C0000 * 0x0216, {0x0030 , 0x0120},},        /*index: 362, 25.1172dB */
        {0x000C0000 * 0x0215, {0x0030 , 0x011F},},        /*index: 363, 25.0702dB */
        {0x000C0000 * 0x0214, {0x0030 , 0x011D},},        /*index: 364, 25.0231dB */
        {0x000C0000 * 0x0213, {0x0030 , 0x011C},},        /*index: 365, 24.9761dB */
        {0x000C0000 * 0x0212, {0x0030 , 0x011A},},        /*index: 366, 24.929dB */
        {0x000C0000 * 0x0211, {0x0030 , 0x0119},},        /*index: 367, 24.882dB */
        {0x000C0000 * 0x0210, {0x0030 , 0x0117},},        /*index: 368, 24.835dB */
        {0x000C0000 * 0x020F, {0x0030 , 0x0116},},        /*index: 369, 24.7879dB */
        {0x000C0000 * 0x020E, {0x0030 , 0x0114},},        /*index: 370, 24.7409dB */
        {0x000C0000 * 0x020D, {0x0030 , 0x0113},},        /*index: 371, 24.6939dB */
        {0x000C0000 * 0x020C, {0x0030 , 0x0111},},        /*index: 372, 24.6468dB */
        {0x000C0000 * 0x020B, {0x0030 , 0x0110},},        /*index: 373, 24.5998dB */
        {0x000C0000 * 0x020A, {0x0030 , 0x010E},},        /*index: 374, 24.5528dB */
        {0x000C0000 * 0x0209, {0x0030 , 0x010D},},        /*index: 375, 24.5057dB */
        {0x000C0000 * 0x0208, {0x0030 , 0x010B},},        /*index: 376, 24.4587dB */
        {0x000C0000 * 0x0207, {0x0030 , 0x010A},},        /*index: 377, 24.4117dB */
        {0x000C0000 * 0x0206, {0x0030 , 0x0108},},        /*index: 378, 24.3646dB */
        {0x000C0000 * 0x0205, {0x0030 , 0x0107},},        /*index: 379, 24.3176dB */
        {0x000C0000 * 0x0204, {0x0030 , 0x0106},},        /*index: 380, 24.2705dB */
        {0x000C0000 * 0x0203, {0x0030 , 0x0104},},        /*index: 381, 24.2235dB */
        {0x000C0000 * 0x0202, {0x0030 , 0x0103},},        /*index: 382, 24.1765dB */
        {0x000C0000 * 0x0201, {0x0030 , 0x0101},},        /*index: 383, 24.1294dB */
        {0x000C0000 * 0x0200, {0x0030 , 0x0100},},        /*index: 384, 24.0824dB */
        {0x000C0000 * 0x01FF, {0x0030 , 0x00FF},},        /*index: 385, 24.0354dB */
        {0x000C0000 * 0x01FE, {0x0030 , 0x00FD},},        /*index: 386, 23.9883dB */
        {0x000C0000 * 0x01FD, {0x0030 , 0x00FC},},        /*index: 387, 23.9413dB */
        {0x000C0000 * 0x01FC, {0x0030 , 0x00FB},},        /*index: 388, 23.8943dB */
        {0x000C0000 * 0x01FB, {0x0030 , 0x00F9},},        /*index: 389, 23.8472dB */
        {0x000C0000 * 0x01FA, {0x0030 , 0x00F8},},        /*index: 390, 23.8002dB */
        {0x000C0000 * 0x01F9, {0x0030 , 0x00F6},},        /*index: 391, 23.7531dB */
        {0x000C0000 * 0x01F8, {0x0030 , 0x00F5},},        /*index: 392, 23.7061dB */
        {0x000C0000 * 0x01F7, {0x0030 , 0x00F4},},        /*index: 393, 23.6591dB */
        {0x000C0000 * 0x01F6, {0x0030 , 0x00F3},},        /*index: 394, 23.612dB */
        {0x000C0000 * 0x01F5, {0x0030 , 0x00F1},},        /*index: 395, 23.565dB */
        {0x000C0000 * 0x01F4, {0x0030 , 0x00F0},},        /*index: 396, 23.518dB */
        {0x000C0000 * 0x01F3, {0x0030 , 0x00EF},},        /*index: 397, 23.4709dB */
        {0x000C0000 * 0x01F2, {0x0030 , 0x00ED},},        /*index: 398, 23.4239dB */
        {0x000C0000 * 0x01F1, {0x0030 , 0x00EC},},        /*index: 399, 23.3769dB */
        {0x000C0000 * 0x01F0, {0x0030 , 0x00EB},},        /*index: 400, 23.3298dB */
        {0x000C0000 * 0x01EF, {0x0030 , 0x00E9},},        /*index: 401, 23.2828dB */
        {0x000C0000 * 0x01EE, {0x0030 , 0x00E8},},        /*index: 402, 23.2358dB */
        {0x000C0000 * 0x01ED, {0x0030 , 0x00E7},},        /*index: 403, 23.1887dB */
        {0x000C0000 * 0x01EC, {0x0030 , 0x00E6},},        /*index: 404, 23.1417dB */
        {0x000C0000 * 0x01EB, {0x0030 , 0x00E4},},        /*index: 405, 23.0946dB */
        {0x000C0000 * 0x01EA, {0x0030 , 0x00E3},},        /*index: 406, 23.0476dB */
        {0x000C0000 * 0x01E9, {0x0030 , 0x00E2},},        /*index: 407, 23.0006dB */
        {0x000C0000 * 0x01E8, {0x0030 , 0x00E1},},        /*index: 408, 22.9535dB */
        {0x000C0000 * 0x01E7, {0x0030 , 0x00E0},},        /*index: 409, 22.9065dB */
        {0x000C0000 * 0x01E6, {0x0030 , 0x00DE},},        /*index: 410, 22.8595dB */
        {0x000C0000 * 0x01E5, {0x0030 , 0x00DD},},        /*index: 411, 22.8124dB */
        {0x000C0000 * 0x01E4, {0x0030 , 0x00DC},},        /*index: 412, 22.7654dB */
        {0x000C0000 * 0x01E3, {0x0030 , 0x00DB},},        /*index: 413, 22.7184dB */
        {0x000C0000 * 0x01E2, {0x0030 , 0x00DA},},        /*index: 414, 22.6713dB */
        {0x000C0000 * 0x01E1, {0x0030 , 0x00D8},},        /*index: 415, 22.6243dB */
        {0x000C0000 * 0x01E0, {0x0030 , 0x00D7},},        /*index: 416, 22.5772dB */
        {0x000C0000 * 0x01DF, {0x0030 , 0x00D6},},        /*index: 417, 22.5302dB */
        {0x000C0000 * 0x01DE, {0x0030 , 0x00D5},},        /*index: 418, 22.4832dB */
        {0x000C0000 * 0x01DD, {0x0030 , 0x00D4},},        /*index: 419, 22.4361dB */
        {0x000C0000 * 0x01DC, {0x0030 , 0x00D3},},        /*index: 420, 22.3891dB */
        {0x000C0000 * 0x01DB, {0x0030 , 0x00D2},},        /*index: 421, 22.3421dB */
        {0x000C0000 * 0x01DA, {0x0030 , 0x00D0},},        /*index: 422, 22.295dB */
        {0x000C0000 * 0x01D9, {0x0030 , 0x00CF},},        /*index: 423, 22.248dB */
        {0x000C0000 * 0x01D8, {0x0030 , 0x00CE},},        /*index: 424, 22.201dB */
        {0x000C0000 * 0x01D7, {0x0030 , 0x00CD},},        /*index: 425, 22.1539dB */
        {0x000C0000 * 0x01D6, {0x0030 , 0x00CC},},        /*index: 426, 22.1069dB */
        {0x000C0000 * 0x01D5, {0x0030 , 0x00CB},},        /*index: 427, 22.0599dB */
        {0x000C0000 * 0x01D4, {0x0030 , 0x00CA},},        /*index: 428, 22.0128dB */
        {0x000C0000 * 0x01D3, {0x0030 , 0x00C9},},        /*index: 429, 21.9658dB */
        {0x000C0000 * 0x01D2, {0x0030 , 0x00C8},},        /*index: 430, 21.9187dB */
        {0x000C0000 * 0x01D1, {0x0030 , 0x00C6},},        /*index: 431, 21.8717dB */
        {0x000C0000 * 0x01D0, {0x0030 , 0x00C5},},        /*index: 432, 21.8247dB */
        {0x000C0000 * 0x01CF, {0x0030 , 0x00C4},},        /*index: 433, 21.7776dB */
        {0x000C0000 * 0x01CE, {0x0030 , 0x00C3},},        /*index: 434, 21.7306dB */
        {0x000C0000 * 0x01CD, {0x0030 , 0x00C2},},        /*index: 435, 21.6836dB */
        {0x000C0000 * 0x01CC, {0x0030 , 0x00C1},},        /*index: 436, 21.6365dB */
        {0x000C0000 * 0x01CB, {0x0030 , 0x00C0},},        /*index: 437, 21.5895dB */
        {0x000C0000 * 0x01CA, {0x0030 , 0x00BF},},        /*index: 438, 21.5425dB */
        {0x000C0000 * 0x01C9, {0x0030 , 0x00BE},},        /*index: 439, 21.4954dB */
        {0x000C0000 * 0x01C8, {0x0030 , 0x00BD},},        /*index: 440, 21.4484dB */
        {0x000C0000 * 0x01C7, {0x0030 , 0x00BC},},        /*index: 441, 21.4014dB */
        {0x000C0000 * 0x01C6, {0x0030 , 0x00BB},},        /*index: 442, 21.3543dB */
        {0x000C0000 * 0x01C5, {0x0030 , 0x00BA},},        /*index: 443, 21.3073dB */
        {0x000C0000 * 0x01C4, {0x0030 , 0x00B9},},        /*index: 444, 21.2602dB */
        {0x000C0000 * 0x01C3, {0x0030 , 0x00B8},},        /*index: 445, 21.2132dB */
        {0x000C0000 * 0x01C2, {0x0030 , 0x00B7},},        /*index: 446, 21.1662dB */
        {0x000C0000 * 0x01C1, {0x0030 , 0x00B6},},        /*index: 447, 21.1191dB */
        {0x000C0000 * 0x01C0, {0x0030 , 0x00B5},},        /*index: 448, 21.0721dB */
        {0x000C0000 * 0x01BF, {0x0030 , 0x00B4},},        /*index: 449, 21.0251dB */
        {0x000C0000 * 0x01BE, {0x0030 , 0x00B3},},        /*index: 450, 20.978dB */
        {0x000C0000 * 0x01BD, {0x0030 , 0x00B2},},        /*index: 451, 20.931dB */
        {0x000C0000 * 0x01BC, {0x0030 , 0x00B1},},        /*index: 452, 20.884dB */
        {0x000C0000 * 0x01BB, {0x0030 , 0x00B0},},        /*index: 453, 20.8369dB */
        {0x000C0000 * 0x01BA, {0x0030 , 0x00AF},},        /*index: 454, 20.7899dB */
        {0x000C0000 * 0x01B9, {0x0030 , 0x00AE},},        /*index: 455, 20.7428dB */
        {0x000C0000 * 0x01B8, {0x0030 , 0x00AD},},        /*index: 456, 20.6958dB */
        {0x000C0000 * 0x01B7, {0x0030 , 0x00AC},},        /*index: 457, 20.6488dB */
        {0x000C0000 * 0x01B6, {0x0030 , 0x00AB},},        /*index: 458, 20.6017dB */
        {0x000C0000 * 0x01B5, {0x0030 , 0x00AB},},        /*index: 459, 20.5547dB */
        {0x000C0000 * 0x01B4, {0x0030 , 0x00AA},},        /*index: 460, 20.5077dB */
        {0x000C0000 * 0x01B3, {0x0030 , 0x00A9},},        /*index: 461, 20.4606dB */
        {0x000C0000 * 0x01B2, {0x0030 , 0x00A8},},        /*index: 462, 20.4136dB */
        {0x000C0000 * 0x01B1, {0x0030 , 0x00A7},},        /*index: 463, 20.3666dB */
        {0x000C0000 * 0x01B0, {0x0030 , 0x00A6},},        /*index: 464, 20.3195dB */
        {0x000C0000 * 0x01AF, {0x0030 , 0x00A5},},        /*index: 465, 20.2725dB */
        {0x000C0000 * 0x01AE, {0x0030 , 0x00A4},},        /*index: 466, 20.2255dB */
        {0x000C0000 * 0x01AD, {0x0030 , 0x00A3},},        /*index: 467, 20.1784dB */
        {0x000C0000 * 0x01AC, {0x0030 , 0x00A2},},        /*index: 468, 20.1314dB */
        {0x000C0000 * 0x01AB, {0x0030 , 0x00A2},},        /*index: 469, 20.0843dB */
        {0x000C0000 * 0x01AA, {0x0030 , 0x00A1},},        /*index: 470, 20.0373dB */
        {0x000C0000 * 0x01A9, {0x0030 , 0x00A0},},        /*index: 471, 19.9903dB */
        {0x000C0000 * 0x01A8, {0x0030 , 0x009F},},        /*index: 472, 19.9432dB */
        {0x000C0000 * 0x01A7, {0x0030 , 0x009E},},        /*index: 473, 19.8962dB */
        {0x000C0000 * 0x01A6, {0x0030 , 0x009D},},        /*index: 474, 19.8492dB */
        {0x000C0000 * 0x01A5, {0x0030 , 0x009C},},        /*index: 475, 19.8021dB */
        {0x000C0000 * 0x01A4, {0x0030 , 0x009C},},        /*index: 476, 19.7551dB */
        {0x000C0000 * 0x01A3, {0x0030 , 0x009B},},        /*index: 477, 19.7081dB */
        {0x000C0000 * 0x01A2, {0x0030 , 0x009A},},        /*index: 478, 19.661dB */
        {0x000C0000 * 0x01A1, {0x0030 , 0x0099},},        /*index: 479, 19.614dB */
        {0x000C0000 * 0x01A0, {0x0030 , 0x0098},},        /*index: 480, 19.5669dB */
        {0x000C0000 * 0x019F, {0x0030 , 0x0097},},        /*index: 481, 19.5199dB */
        {0x000C0000 * 0x019E, {0x0030 , 0x0097},},        /*index: 482, 19.4729dB */
        {0x000C0000 * 0x019D, {0x0030 , 0x0096},},        /*index: 483, 19.4258dB */
        {0x000C0000 * 0x019C, {0x0030 , 0x0095},},        /*index: 484, 19.3788dB */
        {0x000C0000 * 0x019B, {0x0030 , 0x0094},},        /*index: 485, 19.3318dB */
        {0x000C0000 * 0x019A, {0x0030 , 0x0093},},        /*index: 486, 19.2847dB */
        {0x000C0000 * 0x0199, {0x0030 , 0x0093},},        /*index: 487, 19.2377dB */
        {0x000C0000 * 0x0198, {0x0030 , 0x0092},},        /*index: 488, 19.1907dB */
        {0x000C0000 * 0x0197, {0x0030 , 0x0091},},        /*index: 489, 19.1436dB */
        {0x000C0000 * 0x0196, {0x0030 , 0x0090},},        /*index: 490, 19.0966dB */
        {0x000C0000 * 0x0195, {0x0030 , 0x008F},},        /*index: 491, 19.0496dB */
        {0x000C0000 * 0x0194, {0x0030 , 0x008F},},        /*index: 492, 19.0025dB */
        {0x000C0000 * 0x0193, {0x0030 , 0x008E},},        /*index: 493, 18.9555dB */
        {0x000C0000 * 0x0192, {0x0030 , 0x008D},},        /*index: 494, 18.9084dB */
        {0x000C0000 * 0x0191, {0x0030 , 0x008C},},        /*index: 495, 18.8614dB */
        {0x000C0000 * 0x0190, {0x0030 , 0x008C},},        /*index: 496, 18.8144dB */
        {0x000C0000 * 0x018F, {0x0030 , 0x008B},},        /*index: 497, 18.7673dB */
        {0x000C0000 * 0x018E, {0x0030 , 0x008A},},        /*index: 498, 18.7203dB */
        {0x000C0000 * 0x018D, {0x0030 , 0x0089},},        /*index: 499, 18.6733dB */
        {0x000C0000 * 0x018C, {0x0030 , 0x0089},},        /*index: 500, 18.6262dB */
        {0x000C0000 * 0x018B, {0x0030 , 0x0088},},        /*index: 501, 18.5792dB */
        {0x000C0000 * 0x018A, {0x0030 , 0x0087},},        /*index: 502, 18.5322dB */
        {0x000C0000 * 0x0189, {0x0030 , 0x0086},},        /*index: 503, 18.4851dB */
        {0x000C0000 * 0x0188, {0x0030 , 0x0086},},        /*index: 504, 18.4381dB */
        {0x000C0000 * 0x0187, {0x0030 , 0x0085},},        /*index: 505, 18.3911dB */
        {0x000C0000 * 0x0186, {0x0030 , 0x0084},},        /*index: 506, 18.344dB */
        {0x000C0000 * 0x0185, {0x0030 , 0x0084},},        /*index: 507, 18.297dB */
        {0x000C0000 * 0x0184, {0x0030 , 0x0083},},        /*index: 508, 18.2499dB */
        {0x000C0000 * 0x0183, {0x0030 , 0x0082},},        /*index: 509, 18.2029dB */
        {0x000C0000 * 0x0182, {0x0030 , 0x0081},},        /*index: 510, 18.1559dB */
        {0x000C0000 * 0x0181, {0x0030 , 0x0081},},        /*index: 511, 18.1088dB */
        {0x000C0000 * 0x0180, {0x0030 , 0x0080},},        /*index: 512, 18.0618dB */
        {0x000C0000 * 0x017F, {0x0020 , 0x00FF},},        /*index: 513, 18.0148dB */
        {0x000C0000 * 0x017E, {0x0020 , 0x00FD},},        /*index: 514, 17.9677dB */
        {0x000C0000 * 0x017D, {0x0020 , 0x00FC},},        /*index: 515, 17.9207dB */
        {0x000C0000 * 0x017C, {0x0020 , 0x00FB},},        /*index: 516, 17.8737dB */
        {0x000C0000 * 0x017B, {0x0020 , 0x00F9},},        /*index: 517, 17.8266dB */
        {0x000C0000 * 0x017A, {0x0020 , 0x00F8},},        /*index: 518, 17.7796dB */
        {0x000C0000 * 0x0179, {0x0020 , 0x00F6},},        /*index: 519, 17.7325dB */
        {0x000C0000 * 0x0178, {0x0020 , 0x00F5},},        /*index: 520, 17.6855dB */
        {0x000C0000 * 0x0177, {0x0020 , 0x00F4},},        /*index: 521, 17.6385dB */
        {0x000C0000 * 0x0176, {0x0020 , 0x00F3},},        /*index: 522, 17.5914dB */
        {0x000C0000 * 0x0175, {0x0020 , 0x00F1},},        /*index: 523, 17.5444dB */
        {0x000C0000 * 0x0174, {0x0020 , 0x00F0},},        /*index: 524, 17.4974dB */
        {0x000C0000 * 0x0173, {0x0020 , 0x00EF},},        /*index: 525, 17.4503dB */
        {0x000C0000 * 0x0172, {0x0020 , 0x00ED},},        /*index: 526, 17.4033dB */
        {0x000C0000 * 0x0171, {0x0020 , 0x00EC},},        /*index: 527, 17.3563dB */
        {0x000C0000 * 0x0170, {0x0020 , 0x00EB},},        /*index: 528, 17.3092dB */
        {0x000C0000 * 0x016F, {0x0020 , 0x00E9},},        /*index: 529, 17.2622dB */
        {0x000C0000 * 0x016E, {0x0020 , 0x00E8},},        /*index: 530, 17.2152dB */
        {0x000C0000 * 0x016D, {0x0020 , 0x00E7},},        /*index: 531, 17.1681dB */
        {0x000C0000 * 0x016C, {0x0020 , 0x00E6},},        /*index: 532, 17.1211dB */
        {0x000C0000 * 0x016B, {0x0020 , 0x00E4},},        /*index: 533, 17.074dB */
        {0x000C0000 * 0x016A, {0x0020 , 0x00E3},},        /*index: 534, 17.027dB */
        {0x000C0000 * 0x0169, {0x0020 , 0x00E2},},        /*index: 535, 16.98dB */
        {0x000C0000 * 0x0168, {0x0020 , 0x00E1},},        /*index: 536, 16.9329dB */
        {0x000C0000 * 0x0167, {0x0020 , 0x00E0},},        /*index: 537, 16.8859dB */
        {0x000C0000 * 0x0166, {0x0020 , 0x00DE},},        /*index: 538, 16.8389dB */
        {0x000C0000 * 0x0165, {0x0020 , 0x00DD},},        /*index: 539, 16.7918dB */
        {0x000C0000 * 0x0164, {0x0020 , 0x00DC},},        /*index: 540, 16.7448dB */
        {0x000C0000 * 0x0163, {0x0020 , 0x00DB},},        /*index: 541, 16.6978dB */
        {0x000C0000 * 0x0162, {0x0020 , 0x00DA},},        /*index: 542, 16.6507dB */
        {0x000C0000 * 0x0161, {0x0020 , 0x00D8},},        /*index: 543, 16.6037dB */
        {0x000C0000 * 0x0160, {0x0020 , 0x00D7},},        /*index: 544, 16.5566dB */
        {0x000C0000 * 0x015F, {0x0020 , 0x00D6},},        /*index: 545, 16.5096dB */
        {0x000C0000 * 0x015E, {0x0020 , 0x00D5},},        /*index: 546, 16.4626dB */
        {0x000C0000 * 0x015D, {0x0020 , 0x00D4},},        /*index: 547, 16.4155dB */
        {0x000C0000 * 0x015C, {0x0020 , 0x00D3},},        /*index: 548, 16.3685dB */
        {0x000C0000 * 0x015B, {0x0020 , 0x00D2},},        /*index: 549, 16.3215dB */
        {0x000C0000 * 0x015A, {0x0020 , 0x00D0},},        /*index: 550, 16.2744dB */
        {0x000C0000 * 0x0159, {0x0020 , 0x00CF},},        /*index: 551, 16.2274dB */
        {0x000C0000 * 0x0158, {0x0020 , 0x00CE},},        /*index: 552, 16.1804dB */
        {0x000C0000 * 0x0157, {0x0020 , 0x00CD},},        /*index: 553, 16.1333dB */
        {0x000C0000 * 0x0156, {0x0020 , 0x00CC},},        /*index: 554, 16.0863dB */
        {0x000C0000 * 0x0155, {0x0020 , 0x00CB},},        /*index: 555, 16.0393dB */
        {0x000C0000 * 0x0154, {0x0020 , 0x00CA},},        /*index: 556, 15.9922dB */
        {0x000C0000 * 0x0153, {0x0020 , 0x00C9},},        /*index: 557, 15.9452dB */
        {0x000C0000 * 0x0152, {0x0020 , 0x00C8},},        /*index: 558, 15.8981dB */
        {0x000C0000 * 0x0151, {0x0020 , 0x00C6},},        /*index: 559, 15.8511dB */
        {0x000C0000 * 0x0150, {0x0020 , 0x00C5},},        /*index: 560, 15.8041dB */
        {0x000C0000 * 0x014F, {0x0020 , 0x00C4},},        /*index: 561, 15.757dB */
        {0x000C0000 * 0x014E, {0x0020 , 0x00C3},},        /*index: 562, 15.71dB */
        {0x000C0000 * 0x014D, {0x0020 , 0x00C2},},        /*index: 563, 15.663dB */
        {0x000C0000 * 0x014C, {0x0020 , 0x00C1},},        /*index: 564, 15.6159dB */
        {0x000C0000 * 0x014B, {0x0020 , 0x00C0},},        /*index: 565, 15.5689dB */
        {0x000C0000 * 0x014A, {0x0020 , 0x00BF},},        /*index: 566, 15.5219dB */
        {0x000C0000 * 0x0149, {0x0020 , 0x00BE},},        /*index: 567, 15.4748dB */
        {0x000C0000 * 0x0148, {0x0020 , 0x00BD},},        /*index: 568, 15.4278dB */
        {0x000C0000 * 0x0147, {0x0020 , 0x00BC},},        /*index: 569, 15.3808dB */
        {0x000C0000 * 0x0146, {0x0020 , 0x00BB},},        /*index: 570, 15.3337dB */
        {0x000C0000 * 0x0145, {0x0020 , 0x00BA},},        /*index: 571, 15.2867dB */
        {0x000C0000 * 0x0144, {0x0020 , 0x00B9},},        /*index: 572, 15.2396dB */
        {0x000C0000 * 0x0143, {0x0020 , 0x00B8},},        /*index: 573, 15.1926dB */
        {0x000C0000 * 0x0142, {0x0020 , 0x00B7},},        /*index: 574, 15.1456dB */
        {0x000C0000 * 0x0141, {0x0020 , 0x00B6},},        /*index: 575, 15.0985dB */
        {0x000C0000 * 0x0140, {0x0020 , 0x00B5},},        /*index: 576, 15.0515dB */
        {0x000C0000 * 0x013F, {0x0020 , 0x00B4},},        /*index: 577, 15.0045dB */
        {0x000C0000 * 0x013E, {0x0020 , 0x00B3},},        /*index: 578, 14.9574dB */
        {0x000C0000 * 0x013D, {0x0020 , 0x00B2},},        /*index: 579, 14.9104dB */
        {0x000C0000 * 0x013C, {0x0020 , 0x00B1},},        /*index: 580, 14.8634dB */
        {0x000C0000 * 0x013B, {0x0020 , 0x00B0},},        /*index: 581, 14.8163dB */
        {0x000C0000 * 0x013A, {0x0020 , 0x00AF},},        /*index: 582, 14.7693dB */
        {0x000C0000 * 0x0139, {0x0020 , 0x00AE},},        /*index: 583, 14.7222dB */
        {0x000C0000 * 0x0138, {0x0020 , 0x00AD},},        /*index: 584, 14.6752dB */
        {0x000C0000 * 0x0137, {0x0020 , 0x00AC},},        /*index: 585, 14.6282dB */
        {0x000C0000 * 0x0136, {0x0020 , 0x00AB},},        /*index: 586, 14.5811dB */
        {0x000C0000 * 0x0135, {0x0020 , 0x00AB},},        /*index: 587, 14.5341dB */
        {0x000C0000 * 0x0134, {0x0020 , 0x00AA},},        /*index: 588, 14.4871dB */
        {0x000C0000 * 0x0133, {0x0020 , 0x00A9},},        /*index: 589, 14.44dB */
        {0x000C0000 * 0x0132, {0x0020 , 0x00A8},},        /*index: 590, 14.393dB */
        {0x000C0000 * 0x0131, {0x0020 , 0x00A7},},        /*index: 591, 14.346dB */
        {0x000C0000 * 0x0130, {0x0020 , 0x00A6},},        /*index: 592, 14.2989dB */
        {0x000C0000 * 0x012F, {0x0020 , 0x00A5},},        /*index: 593, 14.2519dB */
        {0x000C0000 * 0x012E, {0x0020 , 0x00A4},},        /*index: 594, 14.2049dB */
        {0x000C0000 * 0x012D, {0x0020 , 0x00A3},},        /*index: 595, 14.1578dB */
        {0x000C0000 * 0x012C, {0x0020 , 0x00A2},},        /*index: 596, 14.1108dB */
        {0x000C0000 * 0x012B, {0x0020 , 0x00A2},},        /*index: 597, 14.0637dB */
        {0x000C0000 * 0x012A, {0x0020 , 0x00A1},},        /*index: 598, 14.0167dB */
        {0x000C0000 * 0x0129, {0x0020 , 0x00A0},},        /*index: 599, 13.9697dB */
        {0x000C0000 * 0x0128, {0x0020 , 0x009F},},        /*index: 600, 13.9226dB */
        {0x000C0000 * 0x0127, {0x0020 , 0x009E},},        /*index: 601, 13.8756dB */
        {0x000C0000 * 0x0126, {0x0020 , 0x009D},},        /*index: 602, 13.8286dB */
        {0x000C0000 * 0x0125, {0x0020 , 0x009C},},        /*index: 603, 13.7815dB */
        {0x000C0000 * 0x0124, {0x0020 , 0x009C},},        /*index: 604, 13.7345dB */
        {0x000C0000 * 0x0123, {0x0020 , 0x009B},},        /*index: 605, 13.6875dB */
        {0x000C0000 * 0x0122, {0x0020 , 0x009A},},        /*index: 606, 13.6404dB */
        {0x000C0000 * 0x0121, {0x0020 , 0x0099},},        /*index: 607, 13.5934dB */
        {0x000C0000 * 0x0120, {0x0020 , 0x0098},},        /*index: 608, 13.5463dB */
        {0x000C0000 * 0x011F, {0x0020 , 0x0097},},        /*index: 609, 13.4993dB */
        {0x000C0000 * 0x011E, {0x0020 , 0x0097},},        /*index: 610, 13.4523dB */
        {0x000C0000 * 0x011D, {0x0020 , 0x0096},},        /*index: 611, 13.4052dB */
        {0x000C0000 * 0x011C, {0x0020 , 0x0095},},        /*index: 612, 13.3582dB */
        {0x000C0000 * 0x011B, {0x0020 , 0x0094},},        /*index: 613, 13.3112dB */
        {0x000C0000 * 0x011A, {0x0020 , 0x0093},},        /*index: 614, 13.2641dB */
        {0x000C0000 * 0x0119, {0x0020 , 0x0093},},        /*index: 615, 13.2171dB */
        {0x000C0000 * 0x0118, {0x0020 , 0x0092},},        /*index: 616, 13.1701dB */
        {0x000C0000 * 0x0117, {0x0020 , 0x0091},},        /*index: 617, 13.123dB */
        {0x000C0000 * 0x0116, {0x0020 , 0x0090},},        /*index: 618, 13.076dB */
        {0x000C0000 * 0x0115, {0x0020 , 0x008F},},        /*index: 619, 13.029dB */
        {0x000C0000 * 0x0114, {0x0020 , 0x008F},},        /*index: 620, 12.9819dB */
        {0x000C0000 * 0x0113, {0x0020 , 0x008E},},        /*index: 621, 12.9349dB */
        {0x000C0000 * 0x0112, {0x0020 , 0x008D},},        /*index: 622, 12.8878dB */
        {0x000C0000 * 0x0111, {0x0020 , 0x008C},},        /*index: 623, 12.8408dB */
        {0x000C0000 * 0x0110, {0x0020 , 0x008C},},        /*index: 624, 12.7938dB */
        {0x000C0000 * 0x010F, {0x0020 , 0x008B},},        /*index: 625, 12.7467dB */
        {0x000C0000 * 0x010E, {0x0020 , 0x008A},},        /*index: 626, 12.6997dB */
        {0x000C0000 * 0x010D, {0x0020 , 0x0089},},        /*index: 627, 12.6527dB */
        {0x000C0000 * 0x010C, {0x0020 , 0x0089},},        /*index: 628, 12.6056dB */
        {0x000C0000 * 0x010B, {0x0020 , 0x0088},},        /*index: 629, 12.5586dB */
        {0x000C0000 * 0x010A, {0x0020 , 0x0087},},        /*index: 630, 12.5116dB */
        {0x000C0000 * 0x0109, {0x0020 , 0x0086},},        /*index: 631, 12.4645dB */
        {0x000C0000 * 0x0108, {0x0020 , 0x0086},},        /*index: 632, 12.4175dB */
        {0x000C0000 * 0x0107, {0x0020 , 0x0085},},        /*index: 633, 12.3705dB */
        {0x000C0000 * 0x0106, {0x0020 , 0x0084},},        /*index: 634, 12.3234dB */
        {0x000C0000 * 0x0105, {0x0020 , 0x0084},},        /*index: 635, 12.2764dB */
        {0x000C0000 * 0x0104, {0x0020 , 0x0083},},        /*index: 636, 12.2293dB */
        {0x000C0000 * 0x0103, {0x0020 , 0x0082},},        /*index: 637, 12.1823dB */
        {0x000C0000 * 0x0102, {0x0020 , 0x0081},},        /*index: 638, 12.1353dB */
        {0x000C0000 * 0x0101, {0x0020 , 0x0081},},        /*index: 639, 12.0882dB */
        {0x000C0000 * 0x0100, {0x0020 , 0x0080},},        /*index: 640, 12.0412dB */
        {0x000C0000 * 0x00FF, {0x0010 , 0x00FF},},        /*index: 641, 11.9942dB */
        {0x000C0000 * 0x00FE, {0x0010 , 0x00FD},},        /*index: 642, 11.9471dB */
        {0x000C0000 * 0x00FD, {0x0010 , 0x00FC},},        /*index: 643, 11.9001dB */
        {0x000C0000 * 0x00FC, {0x0010 , 0x00FB},},        /*index: 644, 11.8531dB */
        {0x000C0000 * 0x00FB, {0x0010 , 0x00F9},},        /*index: 645, 11.806dB */
        {0x000C0000 * 0x00FA, {0x0010 , 0x00F8},},        /*index: 646, 11.759dB */
        {0x000C0000 * 0x00F9, {0x0010 , 0x00F6},},        /*index: 647, 11.7119dB */
        {0x000C0000 * 0x00F8, {0x0010 , 0x00F5},},        /*index: 648, 11.6649dB */
        {0x000C0000 * 0x00F7, {0x0010 , 0x00F4},},        /*index: 649, 11.6179dB */
        {0x000C0000 * 0x00F6, {0x0010 , 0x00F3},},        /*index: 650, 11.5708dB */
        {0x000C0000 * 0x00F5, {0x0010 , 0x00F1},},        /*index: 651, 11.5238dB */
        {0x000C0000 * 0x00F4, {0x0010 , 0x00F0},},        /*index: 652, 11.4768dB */
        {0x000C0000 * 0x00F3, {0x0010 , 0x00EF},},        /*index: 653, 11.4297dB */
        {0x000C0000 * 0x00F2, {0x0010 , 0x00ED},},        /*index: 654, 11.3827dB */
        {0x000C0000 * 0x00F1, {0x0010 , 0x00EC},},        /*index: 655, 11.3357dB */
        {0x000C0000 * 0x00F0, {0x0010 , 0x00EB},},        /*index: 656, 11.2886dB */
        {0x000C0000 * 0x00EF, {0x0010 , 0x00E9},},        /*index: 657, 11.2416dB */
        {0x000C0000 * 0x00EE, {0x0010 , 0x00E8},},        /*index: 658, 11.1946dB */
        {0x000C0000 * 0x00ED, {0x0010 , 0x00E7},},        /*index: 659, 11.1475dB */
        {0x000C0000 * 0x00EC, {0x0010 , 0x00E6},},        /*index: 660, 11.1005dB */
        {0x000C0000 * 0x00EB, {0x0010 , 0x00E4},},        /*index: 661, 11.0534dB */
        {0x000C0000 * 0x00EA, {0x0010 , 0x00E3},},        /*index: 662, 11.0064dB */
        {0x000C0000 * 0x00E9, {0x0010 , 0x00E2},},        /*index: 663, 10.9594dB */
        {0x000C0000 * 0x00E8, {0x0010 , 0x00E1},},        /*index: 664, 10.9123dB */
        {0x000C0000 * 0x00E7, {0x0010 , 0x00E0},},        /*index: 665, 10.8653dB */
        {0x000C0000 * 0x00E6, {0x0010 , 0x00DE},},        /*index: 666, 10.8183dB */
        {0x000C0000 * 0x00E5, {0x0010 , 0x00DD},},        /*index: 667, 10.7712dB */
        {0x000C0000 * 0x00E4, {0x0010 , 0x00DC},},        /*index: 668, 10.7242dB */
        {0x000C0000 * 0x00E3, {0x0010 , 0x00DB},},        /*index: 669, 10.6772dB */
        {0x000C0000 * 0x00E2, {0x0010 , 0x00DA},},        /*index: 670, 10.6301dB */
        {0x000C0000 * 0x00E1, {0x0010 , 0x00D8},},        /*index: 671, 10.5831dB */
        {0x000C0000 * 0x00E0, {0x0010 , 0x00D7},},        /*index: 672, 10.536dB */
        {0x000C0000 * 0x00DF, {0x0010 , 0x00D6},},        /*index: 673, 10.489dB */
        {0x000C0000 * 0x00DE, {0x0010 , 0x00D5},},        /*index: 674, 10.442dB */
        {0x000C0000 * 0x00DD, {0x0010 , 0x00D4},},        /*index: 675, 10.3949dB */
        {0x000C0000 * 0x00DC, {0x0010 , 0x00D3},},        /*index: 676, 10.3479dB */
        {0x000C0000 * 0x00DB, {0x0010 , 0x00D2},},        /*index: 677, 10.3009dB */
        {0x000C0000 * 0x00DA, {0x0010 , 0x00D0},},        /*index: 678, 10.2538dB */
        {0x000C0000 * 0x00D9, {0x0010 , 0x00CF},},        /*index: 679, 10.2068dB */
        {0x000C0000 * 0x00D8, {0x0010 , 0x00CE},},        /*index: 680, 10.1598dB */
        {0x000C0000 * 0x00D7, {0x0010 , 0x00CD},},        /*index: 681, 10.1127dB */
        {0x000C0000 * 0x00D6, {0x0010 , 0x00CC},},        /*index: 682, 10.0657dB */
        {0x000C0000 * 0x00D5, {0x0010 , 0x00CB},},        /*index: 683, 10.0187dB */
        {0x000C0000 * 0x00D4, {0x0010 , 0x00CA},},        /*index: 684, 9.9716dB */
        {0x000C0000 * 0x00D3, {0x0010 , 0x00C9},},        /*index: 685, 9.9246dB */
        {0x000C0000 * 0x00D2, {0x0010 , 0x00C8},},        /*index: 686, 9.8775dB */
        {0x000C0000 * 0x00D1, {0x0010 , 0x00C6},},        /*index: 687, 9.8305dB */
        {0x000C0000 * 0x00D0, {0x0010 , 0x00C5},},        /*index: 688, 9.7835dB */
        {0x000C0000 * 0x00CF, {0x0010 , 0x00C4},},        /*index: 689, 9.7364dB */
        {0x000C0000 * 0x00CE, {0x0010 , 0x00C3},},        /*index: 690, 9.6894dB */
        {0x000C0000 * 0x00CD, {0x0010 , 0x00C2},},        /*index: 691, 9.6424dB */
        {0x000C0000 * 0x00CC, {0x0010 , 0x00C1},},        /*index: 692, 9.5953dB */
        {0x000C0000 * 0x00CB, {0x0010 , 0x00C0},},        /*index: 693, 9.5483dB */
        {0x000C0000 * 0x00CA, {0x0010 , 0x00BF},},        /*index: 694, 9.5013dB */
        {0x000C0000 * 0x00C9, {0x0010 , 0x00BE},},        /*index: 695, 9.4542dB */
        {0x000C0000 * 0x00C8, {0x0010 , 0x00BD},},        /*index: 696, 9.4072dB */
        {0x000C0000 * 0x00C7, {0x0010 , 0x00BC},},        /*index: 697, 9.3602dB */
        {0x000C0000 * 0x00C6, {0x0010 , 0x00BB},},        /*index: 698, 9.3131dB */
        {0x000C0000 * 0x00C5, {0x0010 , 0x00BA},},        /*index: 699, 9.2661dB */
        {0x000C0000 * 0x00C4, {0x0010 , 0x00B9},},        /*index: 700, 9.219dB */
        {0x000C0000 * 0x00C3, {0x0010 , 0x00B8},},        /*index: 701, 9.172dB */
        {0x000C0000 * 0x00C2, {0x0010 , 0x00B7},},        /*index: 702, 9.125dB */
        {0x000C0000 * 0x00C1, {0x0010 , 0x00B6},},        /*index: 703, 9.0779dB */
        {0x000C0000 * 0x00C0, {0x0010 , 0x00B5},},        /*index: 704, 9.0309dB */
        {0x000C0000 * 0x00BF, {0x0010 , 0x00B4},},        /*index: 705, 8.9839dB */
        {0x000C0000 * 0x00BE, {0x0010 , 0x00B3},},        /*index: 706, 8.9368dB */
        {0x000C0000 * 0x00BD, {0x0010 , 0x00B2},},        /*index: 707, 8.8898dB */
        {0x000C0000 * 0x00BC, {0x0010 , 0x00B1},},        /*index: 708, 8.8428dB */
        {0x000C0000 * 0x00BB, {0x0010 , 0x00B0},},        /*index: 709, 8.7957dB */
        {0x000C0000 * 0x00BA, {0x0010 , 0x00AF},},        /*index: 710, 8.7487dB */
        {0x000C0000 * 0x00B9, {0x0010 , 0x00AE},},        /*index: 711, 8.7016dB */
        {0x000C0000 * 0x00B8, {0x0010 , 0x00AD},},        /*index: 712, 8.6546dB */
        {0x000C0000 * 0x00B7, {0x0010 , 0x00AC},},        /*index: 713, 8.6076dB */
        {0x000C0000 * 0x00B6, {0x0010 , 0x00AB},},        /*index: 714, 8.5605dB */
        {0x000C0000 * 0x00B5, {0x0010 , 0x00AB},},        /*index: 715, 8.5135dB */
        {0x000C0000 * 0x00B4, {0x0010 , 0x00AA},},        /*index: 716, 8.4665dB */
        {0x000C0000 * 0x00B3, {0x0010 , 0x00A9},},        /*index: 717, 8.4194dB */
        {0x000C0000 * 0x00B2, {0x0010 , 0x00A8},},        /*index: 718, 8.3724dB */
        {0x000C0000 * 0x00B1, {0x0010 , 0x00A7},},        /*index: 719, 8.3254dB */
        {0x000C0000 * 0x00B0, {0x0010 , 0x00A6},},        /*index: 720, 8.2783dB */
        {0x000C0000 * 0x00AF, {0x0010 , 0x00A5},},        /*index: 721, 8.2313dB */
        {0x000C0000 * 0x00AE, {0x0010 , 0x00A4},},        /*index: 722, 8.1843dB */
        {0x000C0000 * 0x00AD, {0x0010 , 0x00A3},},        /*index: 723, 8.1372dB */
        {0x000C0000 * 0x00AC, {0x0010 , 0x00A2},},        /*index: 724, 8.0902dB */
        {0x000C0000 * 0x00AB, {0x0010 , 0x00A2},},        /*index: 725, 8.0431dB */
        {0x000C0000 * 0x00AA, {0x0010 , 0x00A1},},        /*index: 726, 7.9961dB */
        {0x000C0000 * 0x00A9, {0x0010 , 0x00A0},},        /*index: 727, 7.9491dB */
        {0x000C0000 * 0x00A8, {0x0010 , 0x009F},},        /*index: 728, 7.902dB */
        {0x000C0000 * 0x00A7, {0x0010 , 0x009E},},        /*index: 729, 7.855dB */
        {0x000C0000 * 0x00A6, {0x0010 , 0x009D},},        /*index: 730, 7.808dB */
        {0x000C0000 * 0x00A5, {0x0010 , 0x009C},},        /*index: 731, 7.7609dB */
        {0x000C0000 * 0x00A4, {0x0010 , 0x009C},},        /*index: 732, 7.7139dB */
        {0x000C0000 * 0x00A3, {0x0010 , 0x009B},},        /*index: 733, 7.6669dB */
        {0x000C0000 * 0x00A2, {0x0010 , 0x009A},},        /*index: 734, 7.6198dB */
        {0x000C0000 * 0x00A1, {0x0010 , 0x0099},},        /*index: 735, 7.5728dB */
        {0x000C0000 * 0x00A0, {0x0010 , 0x0098},},        /*index: 736, 7.5257dB */
        {0x000C0000 * 0x009F, {0x0010 , 0x0097},},        /*index: 737, 7.4787dB */
        {0x000C0000 * 0x009E, {0x0010 , 0x0097},},        /*index: 738, 7.4317dB */
        {0x000C0000 * 0x009D, {0x0010 , 0x0096},},        /*index: 739, 7.3846dB */
        {0x000C0000 * 0x009C, {0x0010 , 0x0095},},        /*index: 740, 7.3376dB */
        {0x000C0000 * 0x009B, {0x0010 , 0x0094},},        /*index: 741, 7.2906dB */
        {0x000C0000 * 0x009A, {0x0010 , 0x0093},},        /*index: 742, 7.2435dB */
        {0x000C0000 * 0x0099, {0x0010 , 0x0093},},        /*index: 743, 7.1965dB */
        {0x000C0000 * 0x0098, {0x0010 , 0x0092},},        /*index: 744, 7.1495dB */
        {0x000C0000 * 0x0097, {0x0010 , 0x0091},},        /*index: 745, 7.1024dB */
        {0x000C0000 * 0x0096, {0x0010 , 0x0090},},        /*index: 746, 7.0554dB */
        {0x000C0000 * 0x0095, {0x0010 , 0x008F},},        /*index: 747, 7.0084dB */
        {0x000C0000 * 0x0094, {0x0010 , 0x008F},},        /*index: 748, 6.9613dB */
        {0x000C0000 * 0x0093, {0x0010 , 0x008E},},        /*index: 749, 6.9143dB */
        {0x000C0000 * 0x0092, {0x0010 , 0x008D},},        /*index: 750, 6.8672dB */
        {0x000C0000 * 0x0091, {0x0010 , 0x008C},},        /*index: 751, 6.8202dB */
        {0x000C0000 * 0x0090, {0x0010 , 0x008C},},        /*index: 752, 6.7732dB */
        {0x000C0000 * 0x008F, {0x0010 , 0x008B},},        /*index: 753, 6.7261dB */
        {0x000C0000 * 0x008E, {0x0010 , 0x008A},},        /*index: 754, 6.6791dB */
        {0x000C0000 * 0x008D, {0x0010 , 0x0089},},        /*index: 755, 6.6321dB */
        {0x000C0000 * 0x008C, {0x0010 , 0x0089},},        /*index: 756, 6.585dB */
        {0x000C0000 * 0x008B, {0x0010 , 0x0088},},        /*index: 757, 6.538dB */
        {0x000C0000 * 0x008A, {0x0010 , 0x0087},},        /*index: 758, 6.491dB */
        {0x000C0000 * 0x0089, {0x0010 , 0x0086},},        /*index: 759, 6.4439dB */
        {0x000C0000 * 0x0088, {0x0010 , 0x0086},},        /*index: 760, 6.3969dB */
        {0x000C0000 * 0x0087, {0x0010 , 0x0085},},        /*index: 761, 6.3499dB */
        {0x000C0000 * 0x0086, {0x0010 , 0x0084},},        /*index: 762, 6.3028dB */
        {0x000C0000 * 0x0085, {0x0010 , 0x0084},},        /*index: 763, 6.2558dB */
        {0x000C0000 * 0x0084, {0x0010 , 0x0083},},        /*index: 764, 6.2087dB */
        {0x000C0000 * 0x0083, {0x0010 , 0x0082},},        /*index: 765, 6.1617dB */
        {0x000C0000 * 0x0082, {0x0010 , 0x0081},},        /*index: 766, 6.1147dB */
        {0x000C0000 * 0x0081, {0x0010 , 0x0081},},        /*index: 767, 6.0676dB */
        {0x000C0000 * 0x0080, {0x0010 , 0x0080},},        /*index: 768, 6.0206dB */
        {0x000C0000 * 0x007F, {0x0000 , 0x00FF},},        /*index: 769, 5.9736dB */
        {0x000C0000 * 0x007E, {0x0000 , 0x00FD},},        /*index: 770, 5.9265dB */
        {0x000C0000 * 0x007D, {0x0000 , 0x00FC},},        /*index: 771, 5.8795dB */
        {0x000C0000 * 0x007C, {0x0000 , 0x00FB},},        /*index: 772, 5.8325dB */
        {0x000C0000 * 0x007B, {0x0000 , 0x00F9},},        /*index: 773, 5.7854dB */
        {0x000C0000 * 0x007A, {0x0000 , 0x00F8},},        /*index: 774, 5.7384dB */
        {0x000C0000 * 0x0079, {0x0000 , 0x00F6},},        /*index: 775, 5.6913dB */
        {0x000C0000 * 0x0078, {0x0000 , 0x00F5},},        /*index: 776, 5.6443dB */
        {0x000C0000 * 0x0077, {0x0000 , 0x00F4},},        /*index: 777, 5.5973dB */
        {0x000C0000 * 0x0076, {0x0000 , 0x00F3},},        /*index: 778, 5.5502dB */
        {0x000C0000 * 0x0075, {0x0000 , 0x00F1},},        /*index: 779, 5.5032dB */
        {0x000C0000 * 0x0074, {0x0000 , 0x00F0},},        /*index: 780, 5.4562dB */
        {0x000C0000 * 0x0073, {0x0000 , 0x00EF},},        /*index: 781, 5.4091dB */
        {0x000C0000 * 0x0072, {0x0000 , 0x00ED},},        /*index: 782, 5.3621dB */
        {0x000C0000 * 0x0071, {0x0000 , 0x00EC},},        /*index: 783, 5.3151dB */
        {0x000C0000 * 0x0070, {0x0000 , 0x00EB},},        /*index: 784, 5.268dB */
        {0x000C0000 * 0x006F, {0x0000 , 0x00E9},},        /*index: 785, 5.221dB */
        {0x000C0000 * 0x006E, {0x0000 , 0x00E8},},        /*index: 786, 5.174dB */
        {0x000C0000 * 0x006D, {0x0000 , 0x00E7},},        /*index: 787, 5.1269dB */
        {0x000C0000 * 0x006C, {0x0000 , 0x00E6},},        /*index: 788, 5.0799dB */
        {0x000C0000 * 0x006B, {0x0000 , 0x00E4},},        /*index: 789, 5.0328dB */
        {0x000C0000 * 0x006A, {0x0000 , 0x00E3},},        /*index: 790, 4.9858dB */
        {0x000C0000 * 0x0069, {0x0000 , 0x00E2},},        /*index: 791, 4.9388dB */
        {0x000C0000 * 0x0068, {0x0000 , 0x00E1},},        /*index: 792, 4.8917dB */
        {0x000C0000 * 0x0067, {0x0000 , 0x00E0},},        /*index: 793, 4.8447dB */
        {0x000C0000 * 0x0066, {0x0000 , 0x00DE},},        /*index: 794, 4.7977dB */
        {0x000C0000 * 0x0065, {0x0000 , 0x00DD},},        /*index: 795, 4.7506dB */
        {0x000C0000 * 0x0064, {0x0000 , 0x00DC},},        /*index: 796, 4.7036dB */
        {0x000C0000 * 0x0063, {0x0000 , 0x00DB},},        /*index: 797, 4.6566dB */
        {0x000C0000 * 0x0062, {0x0000 , 0x00DA},},        /*index: 798, 4.6095dB */
        {0x000C0000 * 0x0061, {0x0000 , 0x00D8},},        /*index: 799, 4.5625dB */
        {0x000C0000 * 0x0060, {0x0000 , 0x00D7},},        /*index: 800, 4.5154dB */
        {0x000C0000 * 0x005F, {0x0000 , 0x00D6},},        /*index: 801, 4.4684dB */
        {0x000C0000 * 0x005E, {0x0000 , 0x00D5},},        /*index: 802, 4.4214dB */
        {0x000C0000 * 0x005D, {0x0000 , 0x00D4},},        /*index: 803, 4.3743dB */
        {0x000C0000 * 0x005C, {0x0000 , 0x00D3},},        /*index: 804, 4.3273dB */
        {0x000C0000 * 0x005B, {0x0000 , 0x00D2},},        /*index: 805, 4.2803dB */
        {0x000C0000 * 0x005A, {0x0000 , 0x00D0},},        /*index: 806, 4.2332dB */
        {0x000C0000 * 0x0059, {0x0000 , 0x00CF},},        /*index: 807, 4.1862dB */
        {0x000C0000 * 0x0058, {0x0000 , 0x00CE},},        /*index: 808, 4.1392dB */
        {0x000C0000 * 0x0057, {0x0000 , 0x00CD},},        /*index: 809, 4.0921dB */
        {0x000C0000 * 0x0056, {0x0000 , 0x00CC},},        /*index: 810, 4.0451dB */
        {0x000C0000 * 0x0055, {0x0000 , 0x00CB},},        /*index: 811, 3.9981dB */
        {0x000C0000 * 0x0054, {0x0000 , 0x00CA},},        /*index: 812, 3.951dB */
        {0x000C0000 * 0x0053, {0x0000 , 0x00C9},},        /*index: 813, 3.904dB */
        {0x000C0000 * 0x0052, {0x0000 , 0x00C8},},        /*index: 814, 3.8569dB */
        {0x000C0000 * 0x0051, {0x0000 , 0x00C6},},        /*index: 815, 3.8099dB */
        {0x000C0000 * 0x0050, {0x0000 , 0x00C5},},        /*index: 816, 3.7629dB */
        {0x000C0000 * 0x004F, {0x0000 , 0x00C4},},        /*index: 817, 3.7158dB */
        {0x000C0000 * 0x004E, {0x0000 , 0x00C3},},        /*index: 818, 3.6688dB */
        {0x000C0000 * 0x004D, {0x0000 , 0x00C2},},        /*index: 819, 3.6218dB */
        {0x000C0000 * 0x004C, {0x0000 , 0x00C1},},        /*index: 820, 3.5747dB */
        {0x000C0000 * 0x004B, {0x0000 , 0x00C0},},        /*index: 821, 3.5277dB */
        {0x000C0000 * 0x004A, {0x0000 , 0x00BF},},        /*index: 822, 3.4807dB */
        {0x000C0000 * 0x0049, {0x0000 , 0x00BE},},        /*index: 823, 3.4336dB */
        {0x000C0000 * 0x0048, {0x0000 , 0x00BD},},        /*index: 824, 3.3866dB */
        {0x000C0000 * 0x0047, {0x0000 , 0x00BC},},        /*index: 825, 3.3396dB */
        {0x000C0000 * 0x0046, {0x0000 , 0x00BB},},        /*index: 826, 3.2925dB */
        {0x000C0000 * 0x0045, {0x0000 , 0x00BA},},        /*index: 827, 3.2455dB */
        {0x000C0000 * 0x0044, {0x0000 , 0x00B9},},        /*index: 828, 3.1984dB */
        {0x000C0000 * 0x0043, {0x0000 , 0x00B8},},        /*index: 829, 3.1514dB */
        {0x000C0000 * 0x0042, {0x0000 , 0x00B7},},        /*index: 830, 3.1044dB */
        {0x000C0000 * 0x0041, {0x0000 , 0x00B6},},        /*index: 831, 3.0573dB */
        {0x000C0000 * 0x0040, {0x0000 , 0x00B5},},        /*index: 832, 3.0103dB */
        {0x000C0000 * 0x003F, {0x0000 , 0x00B4},},        /*index: 833, 2.9633dB */
        {0x000C0000 * 0x003E, {0x0000 , 0x00B3},},        /*index: 834, 2.9162dB */
        {0x000C0000 * 0x003D, {0x0000 , 0x00B2},},        /*index: 835, 2.8692dB */
        {0x000C0000 * 0x003C, {0x0000 , 0x00B1},},        /*index: 836, 2.8222dB */
        {0x000C0000 * 0x003B, {0x0000 , 0x00B0},},        /*index: 837, 2.7751dB */
        {0x000C0000 * 0x003A, {0x0000 , 0x00AF},},        /*index: 838, 2.7281dB */
        {0x000C0000 * 0x0039, {0x0000 , 0x00AE},},        /*index: 839, 2.681dB */
        {0x000C0000 * 0x0038, {0x0000 , 0x00AD},},        /*index: 840, 2.634dB */
        {0x000C0000 * 0x0037, {0x0000 , 0x00AC},},        /*index: 841, 2.587dB */
        {0x000C0000 * 0x0036, {0x0000 , 0x00AB},},        /*index: 842, 2.5399dB */
        {0x000C0000 * 0x0035, {0x0000 , 0x00AB},},        /*index: 843, 2.4929dB */
        {0x000C0000 * 0x0034, {0x0000 , 0x00AA},},        /*index: 844, 2.4459dB */
        {0x000C0000 * 0x0033, {0x0000 , 0x00A9},},        /*index: 845, 2.3988dB */
        {0x000C0000 * 0x0032, {0x0000 , 0x00A8},},        /*index: 846, 2.3518dB */
        {0x000C0000 * 0x0031, {0x0000 , 0x00A7},},        /*index: 847, 2.3048dB */
        {0x000C0000 * 0x0030, {0x0000 , 0x00A6},},        /*index: 848, 2.2577dB */
        {0x000C0000 * 0x002F, {0x0000 , 0x00A5},},        /*index: 849, 2.2107dB */
        {0x000C0000 * 0x002E, {0x0000 , 0x00A4},},        /*index: 850, 2.1637dB */
        {0x000C0000 * 0x002D, {0x0000 , 0x00A3},},        /*index: 851, 2.1166dB */
        {0x000C0000 * 0x002C, {0x0000 , 0x00A2},},        /*index: 852, 2.0696dB */
        {0x000C0000 * 0x002B, {0x0000 , 0x00A2},},        /*index: 853, 2.0225dB */
        {0x000C0000 * 0x002A, {0x0000 , 0x00A1},},        /*index: 854, 1.9755dB */
        {0x000C0000 * 0x0029, {0x0000 , 0x00A0},},        /*index: 855, 1.9285dB */
        {0x000C0000 * 0x0028, {0x0000 , 0x009F},},        /*index: 856, 1.8814dB */
        {0x000C0000 * 0x0027, {0x0000 , 0x009E},},        /*index: 857, 1.8344dB */
        {0x000C0000 * 0x0026, {0x0000 , 0x009D},},        /*index: 858, 1.7874dB */
        {0x000C0000 * 0x0025, {0x0000 , 0x009C},},        /*index: 859, 1.7403dB */
        {0x000C0000 * 0x0024, {0x0000 , 0x009C},},        /*index: 860, 1.6933dB */
        {0x000C0000 * 0x0023, {0x0000 , 0x009B},},        /*index: 861, 1.6463dB */
        {0x000C0000 * 0x0022, {0x0000 , 0x009A},},        /*index: 862, 1.5992dB */
        {0x000C0000 * 0x0021, {0x0000 , 0x0099},},        /*index: 863, 1.5522dB */
        {0x000C0000 * 0x0020, {0x0000 , 0x0098},},        /*index: 864, 1.5051dB */
        {0x000C0000 * 0x001F, {0x0000 , 0x0097},},        /*index: 865, 1.4581dB */
        {0x000C0000 * 0x001E, {0x0000 , 0x0097},},        /*index: 866, 1.4111dB */
        {0x000C0000 * 0x001D, {0x0000 , 0x0096},},        /*index: 867, 1.364dB */
        {0x000C0000 * 0x001C, {0x0000 , 0x0095},},        /*index: 868, 1.317dB */
        {0x000C0000 * 0x001B, {0x0000 , 0x0094},},        /*index: 869, 1.27dB */
        {0x000C0000 * 0x001A, {0x0000 , 0x0093},},        /*index: 870, 1.2229dB */
        {0x000C0000 * 0x0019, {0x0000 , 0x0093},},        /*index: 871, 1.1759dB */
        {0x000C0000 * 0x0018, {0x0000 , 0x0092},},        /*index: 872, 1.1289dB */
        {0x000C0000 * 0x0017, {0x0000 , 0x0091},},        /*index: 873, 1.0818dB */
        {0x000C0000 * 0x0016, {0x0000 , 0x0090},},        /*index: 874, 1.0348dB */
        {0x000C0000 * 0x0015, {0x0000 , 0x008F},},        /*index: 875, 0.9878dB */
        {0x000C0000 * 0x0014, {0x0000 , 0x008F},},        /*index: 876, 0.9407dB */
        {0x000C0000 * 0x0013, {0x0000 , 0x008E},},        /*index: 877, 0.8937dB */
        {0x000C0000 * 0x0012, {0x0000 , 0x008D},},        /*index: 878, 0.8466dB */
        {0x000C0000 * 0x0011, {0x0000 , 0x008C},},        /*index: 879, 0.7996dB */
        {0x000C0000 * 0x0010, {0x0000 , 0x008C},},        /*index: 880, 0.7526dB */
        {0x000C0000 * 0x000F, {0x0000 , 0x008B},},        /*index: 881, 0.7055dB */
        {0x000C0000 * 0x000E, {0x0000 , 0x008A},},        /*index: 882, 0.6585dB */
        {0x000C0000 * 0x000D, {0x0000 , 0x0089},},        /*index: 883, 0.6115dB */
        {0x000C0000 * 0x000C, {0x0000 , 0x0089},},        /*index: 884, 0.5644dB */
        {0x000C0000 * 0x000B, {0x0000 , 0x0088},},        /*index: 885, 0.5174dB */
        {0x000C0000 * 0x000A, {0x0000 , 0x0087},},        /*index: 886, 0.4704dB */
        {0x000C0000 * 0x0009, {0x0000 , 0x0086},},        /*index: 887, 0.4233dB */
        {0x000C0000 * 0x0008, {0x0000 , 0x0086},},        /*index: 888, 0.3763dB */
        {0x000C0000 * 0x0007, {0x0000 , 0x0085},},        /*index: 889, 0.3293dB */
        {0x000C0000 * 0x0006, {0x0000 , 0x0084},},        /*index: 890, 0.2822dB */
        {0x000C0000 * 0x0005, {0x0000 , 0x0084},},        /*index: 891, 0.2352dB */
        {0x000C0000 * 0x0004, {0x0000 , 0x0083},},        /*index: 892, 0.1881dB */
        {0x000C0000 * 0x0003, {0x0000 , 0x0082},},        /*index: 893, 0.1411dB */
        {0x000C0000 * 0x0002, {0x0000 , 0x0081},},        /*index: 894, 0.0941dB */
        {0x000C0000 * 0x0001, {0x0000 , 0x0081},},        /*index: 895, 0.047dB */
        {0x000C0000 * 0x0000, {0x0000 , 0x0080},},        /*index: 896, 0dB */
    },
    .temperatue_gain_table =
    {
        .agc_control_by_temperatue      = 0,
        .Aptina_temperatue_mode         = 1,
        .temperatue_control_reg_index   = 0,
        .temperatue_gain_num            = 0,
        .temperatue_reg                 = {GADI_VI_SENSOR_TABLE_FLAG_END,},
        .gain_reg                       = {GADI_VI_SENSOR_TABLE_FLAG_END,},
        .temperatue_gain                = {0},
        .temperatue_control             = {0},
    },
    .magic_end  = GADI_SENSOR_HW_INFO_MAGIC_END,
};

