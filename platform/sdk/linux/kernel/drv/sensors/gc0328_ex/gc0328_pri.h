/*!
*****************************************************************************
** \file        software/linux/kernel/drv/sensors/gc0328/gc0328_pri.h
**
** \version     $Id: gc0328_pri.h 117 2018-02-26 08:16:32Z fengxuequan $
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
#ifndef _GC0328_PRI_H_
#define _GC0328_PRI_H_

//*****************************************************************************
//*****************************************************************************
//** Defines and Macros
//*****************************************************************************
//*****************************************************************************
/*         register name            address default value    R/W    description  */
#define GC0328_REG_PAGE_SET           0xFE

#define GC0328_REG_PAGE_SET           0xFE
#define GC0328_REG_VB_H               0x07
#define GC0328_REG_VB_L               0x08
#define GC0328_REG_HB_H               0x05
#define GC0328_REG_HB_L               0x06
#define GC0328_REG_SH_DELAY           0x11

#define GC0328_REG_WIN_WIDTH_H        0x0F
#define GC0328_REG_WIN_WIDTH_L        0x10
#define GC0328_REG_WIN_HEIGHT_H       0x0D
#define GC0328_REG_WIN_HEIGHT_L       0x0E


#define GC0328_REG_EXP_TIME_H         0x03
#define GC0328_REG_EXP_TIME_L         0x04

#define GC0328_REG_GLOBAL_GIAN        0x70
#define GC0328_REG_PRE_GIAN           0x71
#define GC0328_REG_POST_GIAN          0x72


#endif /* _GC0328_PRI_H_ */

