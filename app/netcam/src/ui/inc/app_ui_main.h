/******************************************************************************
**
** \file      app_ui_main.h
**
**
**            (C) Goke Microelectronics 2012
**
**
** \version   $Id: app_ui_main.h 138 2017-12-05 03:28:25Z chenke $
**
******************************************************************************/
#ifndef _APP_UI_MAIN_H_
#define _APP_UI_MAIN_H_

/*---------------------------------------------------------------------------*/
/* function prototypes                                                       */
/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

#include "sdk_def.h"

/*********************
 *      INCLUDES
 *********************/
#include "lv_conf.h"
#include "lv_ex_conf.h"
#include "lvgl.h"

/*********************/
#define APP_UI_WIN_W (480-1)
#define APP_UI_WIN_H  (272-1)

/*********************/

typedef enum
{
    ENGLISH = 0,
    CHINESE	= 1,
    TOTAL	= 2,
    SYS_LANGUAGE_MAX,

} SYSTEM_LANGUAGE_SORT_e;



int gapp_ui_init(void);
int jpeg2rgb565(char *jpgfile,unsigned char * fbp);
int gapp_ui_lvgl_refrsh(void);

#ifdef __cplusplus
}
#endif

#endif /* _APP_UI_MAIN_H_ */

