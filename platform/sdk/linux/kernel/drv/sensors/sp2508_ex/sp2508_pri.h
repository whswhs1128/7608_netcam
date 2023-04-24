/*!
*****************************************************************************
** \file        software/linux/kernel/drv/sensors/sp2508/sp2508_pri.h
**
** \version     $Id: sp2508_pri.h 13054 2017-11-28 04:00:55Z yulindeng $
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
#ifndef _SP2508_PRI_H_
#define _SP2508_PRI_H_

//*****************************************************************************
//*****************************************************************************
//** Defines and Macros
//*****************************************************************************
//*****************************************************************************
/*         register name            address default value    R/W    description  */
#define SP2508_REG_PAGE_SET         0xFD

#define SP2508_REG_EN               0x01

#define SP2508_REG_VB_H             0x05
#define SP2508_REG_VB_L             0x06
#define SP2508_REG_HB_H             0x09
#define SP2508_REG_HB_L             0x0A

#define SP2508_REG_EXP_TIME_H       0x03
#define SP2508_REG_EXP_TIME_L       0x04

#define SP2508_REG_AGC              0x24

#endif /* _SP2508_PRI_H_ */

