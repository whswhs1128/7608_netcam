/*!
*****************************************************************************
** \file        reg_tbl_rgb_tpo990000072.c
**
** \version     $Id: reg_tbl_rgb_tpo990000072.c 1361 2019-04-02 09:29:29Z baochao $
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

#define LCD_SPI_WR_REG_TPO990000072(reg, value)     (OPERATION_TYPE_WR    | ((reg)<<(2+8)) | (value))
#define DELAY_MS(ms)                                (OPERATION_TYPE_DELAY | (ms))


GADI_VO_RgbSpiDrvInfoT    rgb_tpo990000072 =
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
    .mode                   = GADI_VO_LCD_MODE_RGB888_3CYCLES,
    .rgb_seq_even           = GADI_VO_LCD_SEQ_RBG,
    .rgb_seq_odd            = GADI_VO_LCD_SEQ_BGR,    
    .vo_clk_active_edge     = GADI_VO_LCD_CLK_RISING_EDGE,
    .vo_clk_in_freq_hz      = 27000000,    /* 27MHz only when CVBS is opened */
    .vo_clk_div             = 1,           /*reserved*/
    .vo_rotate_en           = GADI_VO_LCD_NotRotate,
    
    .rgb_sync_timing        = 
    {
        .hsync_polar        = GADI_VO_SYNC_LOW_ACTIVE,
        .vsync_polar        = GADI_VO_SYNC_LOW_ACTIVE,
        .hsync_back_porch   = 152, 
        .hsync_front_porch  = 59,
        .vsync_back_porch   = 14,
        .vsync_front_porch  = 8,
        
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
        .bits               = 16,     // <=16
        .cs_change          = 0,
        .speed              = 1000000,//1MHz
        .lsb                = 0,      //0--transmit high bit first(msb)
        .bus_id             = 1,      //spi1, need change gpio configuration
        .cs_id              = 0,      //cs0 (Slave0)
        .cmd_para_num       = 0,
        .cmd_para           =
        {
            LCD_SPI_WR_REG_TPO990000072(0x02, (0<<6) | (0<<5) | (0<<4) | (1<<3) | (0<<2) | (2<<0)),
            DELAY_MS(5),
            LCD_SPI_WR_REG_TPO990000072(0x03, (0<<6) | (0<<2) | (0<<1) | (1<<0)),
            DELAY_MS(5),
            LCD_SPI_WR_REG_TPO990000072(0x05, 0x28),
            DELAY_MS(120),
            LCD_CMD_END_FLAG
        },
    },
    .lcd_rst                =
    {
        .rstEnable          = 0,
        .gpioNum            = 0,
        .activeLevel        = 0,
        .activeDelayMs      = 0,        
    },        
    .magic_end              = GADI_LCD_HW_INFO_MAGIC_END,

};

