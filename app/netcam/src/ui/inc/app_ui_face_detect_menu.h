/**
 * @file app_ui_face_detect_menu.h
 *
 */

#ifndef APP_UI_FACE_DETECT_MENU_H
#define APP_UI_FACE_DETECT_MENU_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lv_ex_conf.h"
#include "lvgl.h"


/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
/**
 * Create a demo GUI
 */
typedef struct
{
    lv_obj_t *win;
    lv_obj_t *smallwin;
    lv_obj_t *faceDetect_rect;
    lv_obj_t *menu_bk;
    lv_obj_t *menu_label1;
    lv_obj_t *menu_label2;
    lv_obj_t *menu_img;
    lv_obj_t *showtimewin;
    lv_obj_t *showtime_label;
}APP_UI_FACE_DETECT_MENU_T;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void app_ui_face_menu_detect_refresh(int xPos,int yPos,int width,int height,lv_color_t color);
void app_ui_face_menu_infor_show(char *namestr,char showflag);
void app_ui_face_menu_init(void);
void app_ui_face_menu_entery(void);
void app_ui_face_menu_exit(void);
void app_ui_face_menu_detect_clear(void);

/**********************
 *      MACROS
 **********************/


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*APP_UI_FACE_DETECT_MENU_H*/

