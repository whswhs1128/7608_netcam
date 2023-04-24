/*!
*****************************************************************************
** \file        reg_tbl_rgb_spi_ftd240g2402.c
**
** \version     $Id: reg_tbl_rgb_spi_ftd240g2402.c 10214 2019-04-02 11:52:53Z baochao $
**
** \brief
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2019 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#define DELAY_MS_FTD240G2402(ms)                    (OPERATION_TYPE_DELAY                                  | (ms))
#define LCD_SPI_WR_REG_FTD240G2402(reg)             (OPERATION_TYPE_WR | OPERATION_WR_IMMEDIATELY | (0<<8) | (reg))
#define LCD_SPI_WR_DATA_FTD240G2402(value)          (OPERATION_TYPE_WR | OPERATION_WR_IMMEDIATELY | (1<<8) | (value))

GADI_VO_RgbSpiDrvInfoT    rgb_spi_ftd240g2402 =
{
    .magic_start            = GADI_LCD_HW_INFO_MAGIC_START,
    .version                = GADI_LCD_HW_INFO_VERSION,
    .length                 = sizeof(GADI_VO_RgbSpiDrvInfoT),
    .type                   = GADI_LCD_HW_INFO_TYPE_RGB_SPI,   //rgb with spi interface
    .file_date              = 0x20190402,
    .resolution             =
    {
        .width              = 240,
        .height             = 320,
    },
    .bg_color               = 
    {
        .y                  = 0x10,
        .cb                 = 0x80,
        .cr                 = 0x80,
    },
    .mode                   = GADI_VO_LCD_MODE_RGB888_3CYCLES,
    .rgb_seq_even           = GADI_VO_LCD_SEQ_RGB,
    .rgb_seq_odd            = GADI_VO_LCD_SEQ_RGB,
    .vo_clk_active_edge     = GADI_VO_LCD_CLK_RISING_EDGE,
    .vo_clk_in_freq_hz      = 8000000,    /* 27MHz only when CVBS is opened */
    .vo_clk_div             = 1,           /*reserved*/
    .vo_rotate_en           = GADI_VO_LCD_NotRotate,
    
    .rgb_sync_timing        = 
    {
        .hsync_polar        = GADI_VO_SYNC_LOW_ACTIVE,
        .vsync_polar        = GADI_VO_SYNC_LOW_ACTIVE,
        .hsync_back_porch   = 10, 
        .hsync_front_porch  = 38,
        .vsync_back_porch   = 4,
        .vsync_front_porch  = 8,
        
        .vsync_begin_row    = 0,
        .vsync_end_row      = 3,
        .vsync_begin_col    = 0,
        .vsync_end_col      = 0,
        .hsync_begin_col    = 0,
        .hsync_end_col      = 3,
    },

    .back_light             = 900,    //0/1000~100/1000

    .rgb_spi_cfg            =
    {
        .mode               = GADI_SPI_MODE_0,
        .bits               = 9,     //bits
        .cs_change          = 0,
        .speed              = 6000000,//6MHz
        .lsb                = 0,      //0--transmit high bit first(msb)
        .bus_id             = 1,      //spi1, need change gpio configuration
        .cs_id              = 0,      //cs0 (Slave0)
        .cmd_para_num       = 0,
        .cmd_para           =
        {
            LCD_SPI_WR_REG_FTD240G2402(0x11),
            DELAY_MS_FTD240G2402(120), //Delay 120ms
        
            LCD_SPI_WR_REG_FTD240G2402(0x36), //Display Setting
            LCD_SPI_WR_DATA_FTD240G2402(0x00),

            LCD_SPI_WR_REG_FTD240G2402(0x3A),
            LCD_SPI_WR_DATA_FTD240G2402(0x66), //0x06--->0x66

            LCD_SPI_WR_REG_FTD240G2402(0xB2), //Frame rate setting
            LCD_SPI_WR_DATA_FTD240G2402(0x0C),
            LCD_SPI_WR_DATA_FTD240G2402(0x0C),
            LCD_SPI_WR_DATA_FTD240G2402(0x00),
            LCD_SPI_WR_DATA_FTD240G2402(0x33),
            LCD_SPI_WR_DATA_FTD240G2402 (0x33),

            LCD_SPI_WR_REG_FTD240G2402(0xB7),
            LCD_SPI_WR_DATA_FTD240G2402(0x35),

            LCD_SPI_WR_REG_FTD240G2402(0xB0), //Power setting
            LCD_SPI_WR_DATA_FTD240G2402(0x10),//11 0x13
            LCD_SPI_WR_DATA_FTD240G2402(0xC4),//? 0xC4

        //  LCD_SPI_WR_REG_FTD240G2402(0xB1), //Power setting
        //  LCD_SPI_WR_DATA_FTD240G2402(0x40),//40
        //  LCD_SPI_WR_DATA_FTD240G2402(0x02),//02
        //  LCD_SPI_WR_DATA_FTD240G2402(0x0A),//14

            LCD_SPI_WR_REG_FTD240G2402(0xBB), //Power setting
            LCD_SPI_WR_DATA_FTD240G2402(0x2B),

            LCD_SPI_WR_REG_FTD240G2402(0xC0),
            LCD_SPI_WR_DATA_FTD240G2402(0x2C),

            LCD_SPI_WR_REG_FTD240G2402(0xC2),
            LCD_SPI_WR_DATA_FTD240G2402(0x01),

            LCD_SPI_WR_REG_FTD240G2402(0xC3),
            LCD_SPI_WR_DATA_FTD240G2402(0x13),

            LCD_SPI_WR_REG_FTD240G2402(0xC4),
            LCD_SPI_WR_DATA_FTD240G2402(0x20),

            LCD_SPI_WR_REG_FTD240G2402(0xC6),
            LCD_SPI_WR_DATA_FTD240G2402(0x0F),

            LCD_SPI_WR_REG_FTD240G2402(0xD0),
            LCD_SPI_WR_DATA_FTD240G2402(0xA4),
            LCD_SPI_WR_DATA_FTD240G2402(0xA1),

            LCD_SPI_WR_REG_FTD240G2402(0xE0), //gamma setting
            LCD_SPI_WR_DATA_FTD240G2402(0xD0),
            LCD_SPI_WR_DATA_FTD240G2402(0x00),
            LCD_SPI_WR_DATA_FTD240G2402(0x05),
            LCD_SPI_WR_DATA_FTD240G2402(0x0E),
            LCD_SPI_WR_DATA_FTD240G2402(0x15),
            LCD_SPI_WR_DATA_FTD240G2402(0x0D),
            LCD_SPI_WR_DATA_FTD240G2402(0x37),
            LCD_SPI_WR_DATA_FTD240G2402(0x43),
            LCD_SPI_WR_DATA_FTD240G2402(0x47),
            LCD_SPI_WR_DATA_FTD240G2402(0x09),
            LCD_SPI_WR_DATA_FTD240G2402(0x15),
            LCD_SPI_WR_DATA_FTD240G2402(0x12),
            LCD_SPI_WR_DATA_FTD240G2402(0x16),
            LCD_SPI_WR_DATA_FTD240G2402(0x19),
            LCD_SPI_WR_REG_FTD240G2402(0xE1),
            LCD_SPI_WR_DATA_FTD240G2402(0xD0),
            LCD_SPI_WR_DATA_FTD240G2402(0x00),
            LCD_SPI_WR_DATA_FTD240G2402(0x05),
            LCD_SPI_WR_DATA_FTD240G2402(0x0D),
            LCD_SPI_WR_DATA_FTD240G2402(0x0C),
            LCD_SPI_WR_DATA_FTD240G2402(0x06),
            LCD_SPI_WR_DATA_FTD240G2402(0x2D),
            LCD_SPI_WR_DATA_FTD240G2402(0x44),
            LCD_SPI_WR_DATA_FTD240G2402(0x40),
            LCD_SPI_WR_DATA_FTD240G2402(0x0E),
            LCD_SPI_WR_DATA_FTD240G2402(0x1C),
            LCD_SPI_WR_DATA_FTD240G2402(0x18),
            LCD_SPI_WR_DATA_FTD240G2402(0x16),
            LCD_SPI_WR_DATA_FTD240G2402(0x19),
            LCD_SPI_WR_REG_FTD240G2402(0xbb),
            LCD_SPI_WR_DATA_FTD240G2402(0x35),
            LCD_SPI_WR_REG_FTD240G2402(0xc3),
            LCD_SPI_WR_DATA_FTD240G2402(0x06),
            LCD_SPI_WR_REG_FTD240G2402(0x2A),// set cloumn adrdress
            LCD_SPI_WR_DATA_FTD240G2402(0x00),
            LCD_SPI_WR_DATA_FTD240G2402(0x00),
            LCD_SPI_WR_DATA_FTD240G2402(0x00),
            LCD_SPI_WR_DATA_FTD240G2402(0xef),
            LCD_SPI_WR_REG_FTD240G2402(0x2B),// set page adrdress
            LCD_SPI_WR_DATA_FTD240G2402(0x00),
            LCD_SPI_WR_DATA_FTD240G2402(0x00),
            LCD_SPI_WR_DATA_FTD240G2402(0x01),
            LCD_SPI_WR_DATA_FTD240G2402(0x3f),
            DELAY_MS_FTD240G2402(1),
            LCD_SPI_WR_REG_FTD240G2402(0x29), //set_display_on 
            LCD_SPI_WR_REG_FTD240G2402(0x2c), //write memory start  */
            LCD_CMD_END_FLAG
        },
    },
    .lcd_rst                =
    {
        .rstEnable          = 1,
        .gpioNum            = 47,
        .activeLevel        = 0,
        .activeDelayMs      = 10,
    },        
    .magic_end              = GADI_LCD_HW_INFO_MAGIC_END,

};

