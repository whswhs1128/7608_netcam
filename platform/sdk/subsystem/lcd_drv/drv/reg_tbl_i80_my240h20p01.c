/*!
*****************************************************************************
** \file        reg_tbl_rgb_my240h20p01.c
**
** \version     $Id: reg_tbl_i80_my240h20p01.c 3 2017-09-04 09:29:58Z fengxuequan $
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
#define MADCTL_MY  7
#define MADCTL_MX  6
#define MADCTL_MV  5
#define MADCTL_ML  4
#define MADCTL_BGR 3
#define MADCTL_MH  2

#define I80_DELAY_CMD   0xFFFF

GADI_VO_I80DrvInfoT    i80_my240h20p01 =
{
    .magic_start            = GADI_LCD_HW_INFO_MAGIC_START,
    .version                = GADI_LCD_HW_INFO_VERSION,
    .length                 = sizeof(GADI_VO_I80DrvInfoT),
    .type                   = GADI_LCD_HW_INFO_TYPE_I80,   //i80 only
    .file_date              = 0x20160920,
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
    .vo_rotate_en           = GADI_VO_LCD_NotRotate,
    .i80_para               =
    {
        .bus_width          = GADI_VO_I80_8BIT,
        .pixel_width        = GADI_VO_I80_PIXEL_16BIT,
        .trans_format       = GADI_VO_I80_TWICE_1PIXEL,    /*pixel_width=18bit only*/
        .cmd_width          = GADI_VO_I80_CMD_8BIT,
        .cmd_endian         = GADI_VO_I80_CMD_BIG_ENDIAN,
        .wt_cmd             = 0x002C,
        .rd_cmd             = 0x002E,
        .hw_delay_en        = 1,
        .hw_delay_cmd       = I80_DELAY_CMD,
        .hw_delay_ms        = 120,
        .dc_polar           = GADI_VO_I80_LOW_CMD,
        .cs_polar           = GADI_VO_I80_RISING_ACTIVE,
        .lcdrst_polar       = GADI_VO_I80_LOW_ACTIVE,
        .rd_polar           = GADI_VO_I80_RISING_ACTIVE,
        .wt_polar           = GADI_VO_I80_RISING_ACTIVE,
        .rst_timing         = 
        {
            .rst_1st_h_ms   = 1,
            .rst_1st_l_ms   = 10,
            .rst_2nd_h_ms   = 120,
        },
        .trans_timing       = 
        {
            .tas_ns         = 0,
            .todh_ns        = 0,
            .csref          = 0,
            .twrh_ns        = 50,//40//
            .twrl_ns        = 60,//60//
            .trdh_ns        = 90+5,
            .trdl_ns        = 450-90+5,
            .pwcsh_wt_ns    = 0,
            .pwcsl_wt_ns    = 0,
            .pwcsh_rd_ns    = 0,
            .pwcsl_rd_ns    = 0,            
        },
        .cmd_para_num       = 0,
        .cmd_para           = 
        {
                    LCD_WRITE_CMD( 0x00EF),//
                    LCD_WRITE_PARA(0x0003),
                    LCD_WRITE_PARA(0x0080),
                    LCD_WRITE_PARA(0x0002),
                
                
                    LCD_WRITE_CMD( 0x00CF),//Power Control B
                    LCD_WRITE_PARA(0x0000),
                    LCD_WRITE_PARA(0x00BA),
                    LCD_WRITE_PARA(0x00B0),
                
                    LCD_WRITE_CMD( 0x00ED),//Power on sequence control
                    LCD_WRITE_PARA(0x0067),
                    LCD_WRITE_PARA(0x0003),
                    LCD_WRITE_PARA(0x0012),
                    LCD_WRITE_PARA(0x0081),
                
                    LCD_WRITE_CMD( 0x00E8),//Driver timing control A
                    LCD_WRITE_PARA(0x0085),
                    LCD_WRITE_PARA(0x0011),
                    LCD_WRITE_PARA(0x0079),
                
                    LCD_WRITE_CMD( 0x00CB),//Power Control A
                    LCD_WRITE_PARA(0x0039),
                    LCD_WRITE_PARA(0x002C),
                    LCD_WRITE_PARA(0x0000),
                    LCD_WRITE_PARA(0x0034),
                    LCD_WRITE_PARA(0x0006),
                
                    LCD_WRITE_CMD( 0x00F7),//Pump ratio control
                    LCD_WRITE_PARA(0x0020),
                
                    LCD_WRITE_CMD( 0x00EA),//Driver timing control C
                    LCD_WRITE_PARA(0x0000),
                    LCD_WRITE_PARA(0x0000),
                
                
                    LCD_WRITE_CMD( 0x00C0), //Power control
                    LCD_WRITE_PARA(0x001D),//VRH[5:0]
                
                    LCD_WRITE_CMD( 0x00C1), //Power control
                    LCD_WRITE_PARA(0x0012),//SAP[2:0];BT[3:0]
                
                    LCD_WRITE_CMD( 0x00C5),//VCM control
                    LCD_WRITE_PARA(0x0044),
                    LCD_WRITE_PARA(0x003C),
                
                    LCD_WRITE_CMD( 0x00C7),//VCM control2
                    LCD_WRITE_PARA(0X009E),
                
                    LCD_WRITE_CMD( 0x003A),//pixel format set
                    LCD_WRITE_PARA(0x0055),//16bit/pixel    RGB565
                
                    LCD_WRITE_CMD( 0x0036),//Gram access control
                    //LCD_WRITE_PARA(0x0048),
                //  LCD_WRITE_PARA((0<<MADCTL_MY) | (0<<MADCTL_MX) | (1<<MADCTL_MV) | (0<<MADCTL_ML) | (0<<MADCTL_BGR) | (0<<MADCTL_MH)),//exchange column and row
                    LCD_WRITE_PARA((0<<MADCTL_MY) | (1<<MADCTL_MX) | (0<<MADCTL_MV) | (0<<MADCTL_ML) | (1<<MADCTL_BGR) | (0<<MADCTL_MH)),
                
                    LCD_WRITE_CMD( 0x00B1),//Frame Rate Control
                    LCD_WRITE_PARA(0x0000),//LCD_WRITE_PARA(0x0000),
                    LCD_WRITE_PARA(0x001f),//LCD_WRITE_PARA(0x0017),
                
                    LCD_WRITE_CMD( 0x00B2),//Frame Rate Control
                    LCD_WRITE_PARA(0x0000),//LCD_WRITE_PARA(0x0000),
                    LCD_WRITE_PARA(0x001f),//LCD_WRITE_PARA(0x0017),
                
                    LCD_WRITE_CMD( 0x00B3),//Frame Rate Control
                    LCD_WRITE_PARA(0x0000),//LCD_WRITE_PARA(0x0000),
                    LCD_WRITE_PARA(0x001f),//LCD_WRITE_PARA(0x0017),
                
                    LCD_WRITE_CMD( 0x00B5), // Blanking Porch Control
                    LCD_WRITE_PARA(40),
                    LCD_WRITE_PARA(39),
                    LCD_WRITE_PARA(0x000A),
                    LCD_WRITE_PARA(0x0014),
                
                    LCD_WRITE_CMD( 0x0035),//Tearing Effect Line ON
                    LCD_WRITE_PARA(0x0000),// 0----consists of V-Blanking information only
                
                    LCD_WRITE_CMD( 0x00B6), // Display Function Control
                    LCD_WRITE_PARA(0x000A),
                    LCD_WRITE_PARA(0x0082),
                
                    LCD_WRITE_CMD( 0x00F2),// 3Gamma Function Disable
                    LCD_WRITE_PARA(0x0000),
                
                    LCD_WRITE_CMD( 0x0026),//Gamma curve selected
                    LCD_WRITE_PARA(0x0001),
                
                    LCD_WRITE_CMD( 0x00E0),//positive gamma correction
                    LCD_WRITE_PARA(0x000F),
                    LCD_WRITE_PARA(0x0022),
                    LCD_WRITE_PARA(0x001C),
                    LCD_WRITE_PARA(0x001B),
                    LCD_WRITE_PARA(0x0008),
                    LCD_WRITE_PARA(0x000F),
                    LCD_WRITE_PARA(0x0048),
                    LCD_WRITE_PARA(0x00B8),
                    LCD_WRITE_PARA(0x0034),
                    LCD_WRITE_PARA(0x0005),
                    LCD_WRITE_PARA(0x000C),
                    LCD_WRITE_PARA(0x0009),
                    LCD_WRITE_PARA(0x000F),
                    LCD_WRITE_PARA(0x0007),
                    LCD_WRITE_PARA(0x0000),
                
                    LCD_WRITE_CMD( 0x00E1),//negtive gamma correction
                    LCD_WRITE_PARA(0x0000),
                    LCD_WRITE_PARA(0x0023),
                    LCD_WRITE_PARA(0x0024),
                    LCD_WRITE_PARA(0x0007),
                    LCD_WRITE_PARA(0x0010),
                    LCD_WRITE_PARA(0x0007),
                    LCD_WRITE_PARA(0x0038),
                    LCD_WRITE_PARA(0x0047),
                    LCD_WRITE_PARA(0x004B),
                    LCD_WRITE_PARA(0x000A),
                    LCD_WRITE_PARA(0x0013),
                    LCD_WRITE_PARA(0x0006),
                    LCD_WRITE_PARA(0x0030),
                    LCD_WRITE_PARA(0x0038),
                    LCD_WRITE_PARA(0x000F),
                
                    LCD_WRITE_CMD( 0x0011),//Exit Sleep
                    LCD_WRITE_CMD( I80_DELAY_CMD),//Delay 120ms--I80 pause,see the register of cmd_sram_state  (100ms is OK)
                    LCD_WRITE_CMD( 0x0029),//display on
                
                
                    LCD_WRITE_CMD( 0x002A),//Set width(Column) 240
                    LCD_WRITE_PARA(0x0000),
                    LCD_WRITE_PARA(0x0000),
                    LCD_WRITE_PARA(0x0000),
                    LCD_WRITE_PARA(0x00EF),
                
                    LCD_WRITE_CMD( 0x002B),//Set hight(row) 320
                    LCD_WRITE_PARA(0x0000),
                    LCD_WRITE_PARA(0x0000),
                    LCD_WRITE_PARA(0x0001),
                    LCD_WRITE_PARA(0x003F),

        
                    LCD_WRITE_CMD( 0x002C),//write memory--send pixel mode            

                    LCD_CMD_END_FLAG
        },
    },
                   
    .back_light            = 1000,    //0/1000~1000/1000

    .magic_end              = GADI_LCD_HW_INFO_MAGIC_END,

};

