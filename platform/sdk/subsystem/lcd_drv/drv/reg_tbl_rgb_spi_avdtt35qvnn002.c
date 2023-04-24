/*!
*****************************************************************************
** \file        reg_tbl_rgb_spi_avdtt35qvnn002.c
**
** \version     $Id: reg_tbl_rgb_spi_avdtt35qvnn002.c 10214 2016-09-13 11:52:53Z baochao $
**
** \brief
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2016 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#define DELAY_MS(ms)                                (OPERATION_TYPE_DELAY                                      | (ms))
#define LCD_SPI_WR_REG(reg)                         (OPERATION_TYPE_WR | OPERATION_WR_IMMEDIATELY | (0x70<<16) | (reg))
#define LCD_SPI_WR_DATA(value)                      (OPERATION_TYPE_WR | OPERATION_WR_IMMEDIATELY | (0x72<<16) | (value))

#define LCD_SPI_WR_REG_AVDTT35QVNN002(reg, value)     \
    LCD_SPI_WR_REG(reg), \
    LCD_SPI_WR_DATA(value)


GADI_VO_RgbSpiDrvInfoT    rgb_spi_avdtt35qvnn002 =
{
    .magic_start            = GADI_LCD_HW_INFO_MAGIC_START,
    .version                = GADI_LCD_HW_INFO_VERSION,
    .length                 = sizeof(GADI_VO_RgbSpiDrvInfoT),
    .type                   = GADI_LCD_HW_INFO_TYPE_RGB_SPI,   //rgb with spi interface
    .file_date              = 0x20170301,
    .resolution             =
    {
        .width              = 320,
        .height             = 240,
    },
    .bg_color               = 
    {
        .y                  = 0x10,
        .cb                 = 0x80,
        .cr                 = 0x80,
    },
    .mode                   = GADI_VO_LCD_MODE_RGB888_1CYCLE,
    .rgb_seq_even           = GADI_VO_LCD_SEQ_RGB,
    .rgb_seq_odd            = GADI_VO_LCD_SEQ_RGB,
    .vo_clk_active_edge     = GADI_VO_LCD_CLK_RISING_EDGE,
    .vo_clk_in_freq_hz      = 6000000,    /* 27MHz only when CVBS is opened */
    .vo_clk_div             = 1,           /*reserved*/
    .vo_rotate_en           = GADI_VO_LCD_NotRotate,
    
    .rgb_sync_timing        = 
    {
        .hsync_polar        = GADI_VO_SYNC_LOW_ACTIVE,
        .vsync_polar        = GADI_VO_SYNC_LOW_ACTIVE,
        .hsync_back_porch   = 88, 
        .hsync_front_porch  = 92,
        .vsync_back_porch   = 17,
        .vsync_front_porch  = 10,
        
        .vsync_begin_row    = 0,
        .vsync_end_row      = 1,
        .vsync_begin_col    = 0,
        .vsync_end_col      = 0,
        .hsync_begin_col    = 0,
        .hsync_end_col      = 1,
    },

    .back_light             = 900,    //0/1000~100/1000

    .rgb_spi_cfg            =
    {
        .mode               = GADI_SPI_MODE_0,
        .bits               = 24,     // config tansfer bits of spi to 12bits
        .cs_change          = 0,
        .speed              = 1000000,//1MHz
        .lsb                = 0,      //0--transmit high bit first(msb)
        .bus_id             = 1,      //spi1, need change gpio configuration
        .cs_id              = 0,      //cs0 (Slave0)
        .cmd_para_num       = 0,
        .cmd_para           =
        {
            LCD_SPI_WR_REG_AVDTT35QVNN002(0x0001,0x6300),
            LCD_SPI_WR_REG_AVDTT35QVNN002(0x0002,0x0200),
            LCD_SPI_WR_REG_AVDTT35QVNN002(0x0003,0xd164),
            LCD_SPI_WR_REG_AVDTT35QVNN002(0x0004,0x04c7),
            DELAY_MS(10),
            LCD_SPI_WR_REG_AVDTT35QVNN002(0x0005,0x8c40),
            LCD_SPI_WR_REG_AVDTT35QVNN002(0x0008,0x06FF),
            LCD_SPI_WR_REG_AVDTT35QVNN002(0x000A,0x4008),
            LCD_SPI_WR_REG_AVDTT35QVNN002(0x000B,0xD400),
            LCD_SPI_WR_REG_AVDTT35QVNN002(0x000D,0x0230),
            LCD_SPI_WR_REG_AVDTT35QVNN002(0x000E,0x1200),
            LCD_SPI_WR_REG_AVDTT35QVNN002(0x000F,0x0000),
            LCD_SPI_WR_REG_AVDTT35QVNN002(0x0016,0x9F80),
            LCD_SPI_WR_REG_AVDTT35QVNN002(0x0017,0x2212),
            LCD_SPI_WR_REG_AVDTT35QVNN002(0x001E,0x00e4),
            DELAY_MS(50),
            LCD_SPI_WR_REG_AVDTT35QVNN002(0x0030,0x0000),
            LCD_SPI_WR_REG_AVDTT35QVNN002(0x0031,0x0707),
            LCD_SPI_WR_REG_AVDTT35QVNN002(0x0032,0x0206),
            LCD_SPI_WR_REG_AVDTT35QVNN002(0x0033,0x0001),
            LCD_SPI_WR_REG_AVDTT35QVNN002(0x0034,0x0105),
            LCD_SPI_WR_REG_AVDTT35QVNN002(0x0035,0x0000),
            LCD_SPI_WR_REG_AVDTT35QVNN002(0x0036,0x0707),
            LCD_SPI_WR_REG_AVDTT35QVNN002(0x0037,0x0100),
            LCD_SPI_WR_REG_AVDTT35QVNN002(0x003a,0x0502),
            LCD_SPI_WR_REG_AVDTT35QVNN002(0x003b,0x0502),
            LCD_CMD_END_FLAG
        },
    },
    .lcd_rst                =
    {
        .rstEnable          = 1,
        .gpioNum            = 20,
        .activeLevel        = 0,
        .activeDelayMs      = 10,
    },        
    .magic_end              = GADI_LCD_HW_INFO_MAGIC_END,

};

