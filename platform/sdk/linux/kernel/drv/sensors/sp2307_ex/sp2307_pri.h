/*!
*****************************************************************************
** \file        software/linux/kernel/drv/sensors/sp2307/sp2307_pri.h
**
** \version     $Id: sp2307_pri.h 10273 2017-02-14 11:04:43Z hehuali $
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
#ifndef _SP2307_PRI_H_
#define _SP2307_PRI_H_

//*****************************************************************************
//*****************************************************************************
//** Defines and Macros
//*****************************************************************************
//*****************************************************************************
/*         register name            address default value    R/W    description  */
#define SP2307_REG_PAGE_SET           0xFD

#define SP2307_REG_SET_EN             0x01

#define SP2307_REG_VB_H               0x05
#define SP2307_REG_VB_L               0x06
#define SP2307_REG_HB_H               0x09
#define SP2307_REG_HB_L               0x0A

#define SP2307_REG_WIN_WIDTH_H        0x8C
#define SP2307_REG_WIN_WIDTH_L        0x8D
#define SP2307_REG_WIN_HEIGHT_H       0x4E
#define SP2307_REG_WIN_HEIGHT_L       0x4F


#define SP2307_REG_EXP_TIME_H         0x03
#define SP2307_REG_EXP_TIME_L         0x04

#define SP2307_REG_ANA_GIAN           0x24
#define SP2307_REG_DIG_GIAN           0x39

#define SP2307_REG_CLIP_VER_H         0x91
#define SP2307_REG_CLIP_VER_L         0x92
#define SP2307_REG_CLIP_HOR_H         0x93
#define SP2307_REG_CLIP_HOR_L         0x94

#define SP2307_REG_MIRROR_FLIP        0x3F

#endif /* _SP2307_PRI_H_ */

