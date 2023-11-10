/**
 * @file lvgl.h
 * Include all LittleV GL related headers
 */

#ifndef LVGL_H
#define LVGL_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

/*Test misc. module version*/
#include "lv_task.h"

#include "lv_hal.h"

#include "lv_obj.h"
#include "lv_group.h"
#include "lv_vdb.h"

#include "lv_theme.h"

#include "lv_btn.h"
#include "lv_img.h"
#include "lv_label.h"
#include "lv_line.h"
#include "lv_page.h"
#include "lv_cont.h"
#include "lv_list.h"
#include "lv_chart.h"
#include "lv_cb.h"
#include "lv_bar.h"
#include "lv_slider.h"
#include "lv_led.h"
#include "lv_btnm.h"
#include "lv_kb.h"
#include "lv_ddlist.h"
#include "lv_roller.h"
#include "lv_ta.h"
#include "lv_win.h"
#include "lv_tabview.h"
#include "lv_mbox.h"
#include "lv_gauge.h"
#include "lv_lmeter.h"
#include "lv_sw.h"
#include "lv_kb.h"

/*********************
 *      DEFINES
 *********************/
/*Current version of LittlevGL*/
#define LVGL_VERSION_MAJOR   5
#define LVGL_VERSION_MINOR   1
#define LVGL_VERSION_PATCH   1
#define LVGL_VERSION_INFO    ""

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
}
#endif

#endif /*LVGL_H*/
