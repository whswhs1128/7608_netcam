/*!
*****************************************************************************
** \file        reg_tbl_rgb_my500q40p01.c
**
** \version     $Id: reg_tbl_rgb_my500q40p01.c 1286 2019-02-26 08:40:43Z baochao $
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
GADI_VO_RgbDrvInfoT    rgb_my500q40p01 =
{
    .magic_start            = GADI_LCD_HW_INFO_MAGIC_START,
    .version                = GADI_LCD_HW_INFO_VERSION,
    .length                 = sizeof(GADI_VO_RgbDrvInfoT),
    .type                   = GADI_LCD_HW_INFO_TYPE_RGB,   //rgb only
    .file_date              = 0x20160920,
    .resolution             =
    {
        .width              = 800,
        .height             = 480,
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
    .vo_clk_active_edge     = GADI_VO_LCD_CLK_FALLING_EDGE,
    .vo_clk_in_freq_hz      = 27000000,    /* 27MHz only when CVBS is opened*/
    .vo_clk_div             = 1,           /*reserved*/
    .vo_rotate_en           = GADI_VO_LCD_NotRotate,

    .rgb_sync_timing        = 
    {
        .hsync_polar        = GADI_VO_SYNC_LOW_ACTIVE,
        .vsync_polar        = GADI_VO_SYNC_LOW_ACTIVE,
        .hsync_back_porch   = 46, 
        .hsync_front_porch  = 210,
        .vsync_back_porch   = 23,
        .vsync_front_porch  = 8,
        
        .vsync_begin_row    = 0,
        .vsync_end_row      = 1,
        .vsync_begin_col    = 0,
        .vsync_end_col      = 0,
        .hsync_begin_col    = 0,
        .hsync_end_col      = 3,
    },
                   
    .back_light             = 900,    //0/1000~1000/1000
    .lcd_rst                =
    {
        .rstEnable          = 0,
        .gpioNum            = 0,
        .activeLevel        = 0,
        .activeDelayMs      = 0,        
    },    
    .magic_end              = GADI_LCD_HW_INFO_MAGIC_END,

};

