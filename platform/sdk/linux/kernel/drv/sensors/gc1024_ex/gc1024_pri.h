/*!
*****************************************************************************
** \file        software/linux/kernel/drv/sensors/gc1024/gc1024_pri.h
**
** \version     $Id: gc1024_pri.h 10273 2017-02-14 11:04:43Z hehuali $
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
#ifndef _GC1024_PRI_H_
#define _GC1024_PRI_H_

//*****************************************************************************
//*****************************************************************************
//** Defines and Macros
//*****************************************************************************
//*****************************************************************************
/*         register name            address default value    R/W    description  */
#define GC1024_REG_PAGE_SET           0xFE

#define GC1024_REG_VB_H               0x07
#define GC1024_REG_VB_L               0x08
#define GC1024_REG_HB_H               0x05
#define GC1024_REG_HB_L               0x06
#define GC1024_REG_SH_DELAY_H         0x11
#define GC1024_REG_SH_DELAY_L         0x12

#define GC1024_REG_WIN_WIDTH_H        0x0F
#define GC1024_REG_WIN_WIDTH_L        0x10
#define GC1024_REG_WIN_HEIGHT_H       0x0D
#define GC1024_REG_WIN_HEIGHT_L       0x0E


#define GC1024_REG_EXP_TIME_H         0x03
#define GC1024_REG_EXP_TIME_L         0x04

#define GC1024_REG_ANA_GIAN           0xB6
#define GC1024_REG_DIG_GIAN_IN        0xB1
#define GC1024_REG_DIR_GIAN_DE        0xB2

#define GC1024_REG_CLIP_VER_H         0x91
#define GC1024_REG_CLIP_VER_L         0x92
#define GC1024_REG_CLIP_HOR_H         0x93
#define GC1024_REG_CLIP_HOR_L         0x94

#define GC1024_REG_MIRROR_FLIP        0x17

#endif /* _GC1024_PRI_H_ */

