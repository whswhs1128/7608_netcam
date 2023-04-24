/*!
*****************************************************************************
** \file        src/algo/af_params/Foctek_D14_02812IR_distance_table.c
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
#include "basetypes.h"
#include "gk_isp.h"
#include "gk_isp3a_api.h"
#include "isp3a_lens.h"

//*****************************************************************************
//*****************************************************************************
//** Local Defines
//*****************************************************************************
//*****************************************************************************

//*****************************************************************************
//*****************************************************************************
//** Local structures
//*****************************************************************************
//*****************************************************************************

//*****************************************************************************
//*****************************************************************************
//** Global Data
//*****************************************************************************
//*****************************************************************************
/*    This table covers physical distance(mm) to focus pulse
*    ranges from 50000cm to 1cm
*    dist_idx = 50000/dist(cm)
*    {wide, zoom_pos1, zoom_pos2, ... , zoom_pos N, tele}
*
*    index 1        500m
*     index 5        10m
*    index 100    5m
*    index 167    3m
*    index 250    2m
*    index 333    1.5m
*    index 417    1.2m
*    index 600    1m
*    index 625    0.8m
*    index 714    0.7m
*    index 833    0.6m
*    index 1000    0.5m
*    index 1111    0.45m
*
*    pulse = (a>>20) * dist_idx^2 + (b>>15) * dist_idx + (c>>2)
*/

/*
* a: less than 512
* b:
* c:
*/
//gc2053+塑料底座+yutong_xx
zoom_t YuTong_YT30021FB_DISTANCE_MAP[ZOOM_STEP] = {
 
#if 0 //标定初始化(focus length=2364; zoom length= 2372)
    {0, -32768, 0, 0, 0}
                      ,        // zp_0
    {0, -32768, 668, 136, 0}
                      ,        // zp_1
    {0, -32768, 960, 272, 0}
                      ,        // zp_2
    {0, -32768, 1204, 408, 0}
                      ,        // zp_3
    {0, -32768, 1404, 544, 0}
                      ,        // zp_4
    {0, -32768, 1568, 680, 0}
                      ,        // zp_5
    {0, -32768, 1708, 816, 0}
                      ,        // zp_6
    {0, -32768, 1828, 952, 0}
                      ,        // zp_7
    {0, -32768, 1924, 1088, 0}
                      ,        // zp_8
    {0, -32768, 2008, 1224, 0}
                      ,        // zp_9
    {0, -32768, 2076, 1360, 0}
                      ,        // zp_10
    {0, -32768, 2132, 1496, 0}
                      ,        // zp_11
    {0, -32768, 2172, 1632, 0}
                      ,        // zp_12
    {0, -32768, 2172, 1768, 0}
                      ,        // zp_13
    {0, -32768, 2172, 1904, 0}
                      ,        // zp_14
    {0, -32768, 2172, 2040, 0}
                      ,        // zp_15
    {0, -32768, 2172, 2176, 0}
                      ,        // zp_16
    {0, -32768, 2172, 2312, 0}                 
#endif 

#if 1 //gc2053(100即-3276，也可用-920）zoom正向，focus正向，无偏移。focus_reverse _err=0,zoom_reverse _err=16
    {0, -920, 0, 0, 0}
                      ,        // zp_0
    {0, -920, 692, 136, 0}
                      ,        // zp_1
    {0, -920, 984, 272, 0}
                      ,        // zp_2
    {0, -920, 1220, 408, 0}
                      ,        // zp_3
    {0, -920, 1424, 544, 0}
                      ,        // zp_4
    {0, -920, 1584, 680, 0}
                      ,        // zp_5
    {0, -920, 1724, 816, 0}
                      ,        // zp_6
    {0, -920, 1844, 952, 0}
                      ,        // zp_7
    {0, -920, 1940, 1088, 0}
                      ,        // zp_8
    {0, -920, 2024, 1224, 0}
                      ,        // zp_9
    {0, -920, 2088 - 200, 1360, 0}
                      ,        // zp_10
    {0, -920, 2144 - 200, 1496, 0}
                      ,        // zp_11
    {0, -920, 2184 - 200, 1632, 0}
                      ,        // zp_12
    {0, -920, 2220 - 200, 1768, 0}
                      ,        // zp_13
    {0, -920, 2244 - 200, 1904, 0}
                      ,        // zp_14
    {0, -920, 2260 - 200, 2040, 0}
                      ,        // zp_15
    {0, -920, 2272 - 200, 2176, 0}
                      ,        // zp_16
    {0, -920, 2276 - 200, 2312, 0}
#endif
                               
    ,                           // zp_17
    {0, 0, 0, 0, 0}
    ,                           // zp_18
    {0, 0, 0, 0, 0}
    ,                           // zp_19
    {0, 0, 0, 0, 0}
    ,                           // zp_20
    {0, 0, 0, 0, 0}
    ,                           // zp_21
    {0, 0, 0, 0, 0}
    ,                           // zp_22
    {0, 0, 0, 0, 0}
    ,                           // zp_23
    {0, 0, 0, 0, 0}
    ,                           // zp_24
    {0, 0, 0, 0, 0}
    ,                           // zp_25
    {0, 0, 0, 0, 0}
    ,                           // zp_26
    {0, 0, 0, 0, 0}
    ,                           // zp_27
    {0, 0, 0, 0, 0}
    ,                           // zp_28
    {0, 0, 0, 0, 0}
    ,                           // zp_29
    {0, 0, 0, 0, 0}
    ,                           // zp_30
    {0, 0, 0, 0, 0}
    ,                           // zp_31
};


//*****************************************************************************
//*****************************************************************************
//** Local Data
//*****************************************************************************
//*****************************************************************************

//*****************************************************************************
//*****************************************************************************
//** Local Functions Declaration
//*****************************************************************************
//*****************************************************************************

//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************

//*****************************************************************************
//*****************************************************************************
//** Local Functions
//*****************************************************************************
//*****************************************************************************
