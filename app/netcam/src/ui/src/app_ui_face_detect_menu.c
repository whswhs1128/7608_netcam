/**
 * @file app_ui_face_detect_menu.h
 *
 */
/*********************
 *      INCLUDES
 *********************/
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "time.h"
#include "sdk_def.h"
#include "lv_hal_porting.h"
#include "app_ui_face_detect_menu.h"
#include "lv_conf.h"
#include "avi_utility.h"
#include "netcam_api.h"
#include "sdk_sys.h"
/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct
{
    int xPos;
    int yPos;
    int width;
    int height;
}lv_fb_rect_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static APP_UI_FACE_DETECT_MENU_T lv_faceDetect_menu;
static int detWin_w = 0;
static int detWin_h = 0;
static int infWin_w = 0;
static int infWin_h = 0;
static int showTimeWin_w = 0;
static int showTimeWin_h = 0;
static int rec_bold = 4;
static lv_style_t rect_style;
static lv_style_t label_style;
static pthread_mutex_t uifreshMutex;

/**********************
 *  STATIC VARIABLES
 **********************/
#define IMG_H 100
#define IMG_W 480

/**********************
 *      MACROS
 **********************/

/**********************
 *   STATIC FUNCTIONS
 **********************/

static unsigned char * img_map = 0;
static lv_img_t show_img_info;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void get_showtime_str(char *str)
{
    struct tm *ptm;
    long ts;

    ts = time(NULL);
    struct tm tt = {0};
    ptm = localtime_r(&ts, &tt);

	sprintf(str, "%04d年%02d月%02d日 %02d:%02d", ptm->tm_year+1900,
											 ptm->tm_mon+1,
											 ptm->tm_mday,
											 ptm->tm_hour,
											 ptm->tm_min);

    return;
}

void app_ui_face_menu_time_refresh(void)
{
    char strbuf[64];
    char utf8_buf[64];

    memset(strbuf,0,sizeof(strbuf));
    memset(utf8_buf,0,sizeof(utf8_buf));

    pthread_mutex_lock(&uifreshMutex);
    get_showtime_str(strbuf);
    utility_gbk_to_utf8(strbuf, utf8_buf, 64);
    lv_label_set_text(lv_faceDetect_menu.showtime_label,utf8_buf);
    lv_task_handler();
    pthread_mutex_unlock(&uifreshMutex);
}


static void app_ui_face_menu_create(void)
{
    char strbuf[64];
    char utf8_buf[64];
    int win_w = LV_HOR_RES;
    int win_h = LV_VER_RES;

    /*creat base win*/
    lv_faceDetect_menu.win = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(lv_faceDetect_menu.win,win_w,win_h);
    lv_obj_set_pos(lv_faceDetect_menu.win,0,0);
    lv_obj_set_style(lv_faceDetect_menu.win, &lv_style_transp);

    /*creat face detect base win*/
    lv_faceDetect_menu.smallwin = lv_obj_create(lv_faceDetect_menu.win, NULL);;
    lv_obj_set_size(lv_faceDetect_menu.smallwin,detWin_w,detWin_h);
    lv_obj_set_pos(lv_faceDetect_menu.smallwin,0,0);
    lv_obj_set_style(lv_faceDetect_menu.smallwin, &lv_style_transp);

    /*creat face detect rect style*/
    lv_style_copy(&rect_style,&lv_style_pretty);

    rect_style.body.main_color = LV_COLOR_TRANSP;
    rect_style.body.grad_color = LV_COLOR_TRANSP;
    rect_style.body.border.opa = LV_OPA_COVER;
    rect_style.body.border.color = LV_COLOR_WHITE;
    rect_style.body.border.width = rec_bold;
    rect_style.body.border.part = LV_BORDER_FULL;
    rect_style.body.radius = 0;
    rect_style.body.padding.ver = 0;
    rect_style.body.padding.hor = 0;
    rect_style.body.padding.inner = 0;

    /*copy base style*/
    lv_style_copy(&label_style,&lv_style_pretty);

    /*creat show time win*/
    lv_faceDetect_menu.showtimewin = lv_obj_create(lv_faceDetect_menu.win, NULL);;
    lv_obj_set_size(lv_faceDetect_menu.showtimewin,showTimeWin_w,showTimeWin_h);
    lv_obj_set_pos(lv_faceDetect_menu.showtimewin,0,LV_VER_RES-showTimeWin_h);
    lv_obj_set_style(lv_faceDetect_menu.showtimewin, &lv_style_pretty);

    /*creat show time label*/
    lv_faceDetect_menu.showtime_label = lv_label_create(lv_faceDetect_menu.showtimewin, NULL);
    lv_label_set_style( lv_faceDetect_menu.showtime_label, &label_style);
	lv_obj_align(lv_faceDetect_menu.showtime_label, lv_faceDetect_menu.showtimewin,LV_ALIGN_CENTER,-90,0);
    memset(strbuf,0,sizeof(strbuf));
    memset(utf8_buf,0,sizeof(utf8_buf));
    get_showtime_str(strbuf);
    utility_gbk_to_utf8(strbuf, utf8_buf, 64);
    lv_label_set_text(lv_faceDetect_menu.showtime_label,utf8_buf);
}


void app_ui_face_menu_detect_refresh(int xPos,int yPos,int width,int height,lv_color_t color)
{
    static lv_fb_rect_t lastfaceRect={0,0,0,0};
    static lv_color_t lastfaceColor;


    if((abs(xPos-lastfaceRect.xPos) < 20)&& (abs(yPos-lastfaceRect.yPos) < 20)&&\
       (abs(width-lastfaceRect.width) < 20)&&(abs(height-lastfaceRect.height) < 20)&&\
       !memcmp(&lastfaceColor, &color, sizeof(lv_color_t)))
    {
        return;
    }
    pthread_mutex_lock(&uifreshMutex);

    lv_task_handler();

   // printf("draw %d,%d,%d,%d \n",xPos,yPos,width,height);
    if((xPos + width + rec_bold) >= (detWin_w - 1))
    {
        printf("app_ui_face_menu_detect_refresh:error xPos+width > %d\n",detWin_w);
        pthread_mutex_unlock(&uifreshMutex);
        return;
    }

    if((yPos + height + rec_bold) >= (detWin_h - 1))
    {
        printf("app_ui_face_menu_detect_refresh:error yPos+height > %d\n",detWin_h);
        pthread_mutex_unlock(&uifreshMutex);
        return;
    }

    rect_style.body.border.color = color;

    if(lv_faceDetect_menu.faceDetect_rect == 0)
    {
        /*creat face detect rect*/
        lv_faceDetect_menu.faceDetect_rect = lv_obj_create(lv_faceDetect_menu.smallwin, NULL);
        lv_obj_set_pos(lv_faceDetect_menu.faceDetect_rect,xPos,yPos);
        lv_obj_set_size(lv_faceDetect_menu.faceDetect_rect,width,height);
        lv_obj_set_style(lv_faceDetect_menu.faceDetect_rect, &rect_style);
    }
    else
    {
        lv_obj_set_hidden(lv_faceDetect_menu.faceDetect_rect,1);
        lv_task_handler();

        lv_obj_set_pos(lv_faceDetect_menu.faceDetect_rect,xPos,yPos);
        lv_obj_set_size(lv_faceDetect_menu.faceDetect_rect,width,height);
        lv_obj_set_style(lv_faceDetect_menu.faceDetect_rect, &rect_style);
        lv_obj_set_hidden(lv_faceDetect_menu.faceDetect_rect,0);
    }
    lv_task_handler();
    pthread_mutex_unlock(&uifreshMutex);


    lastfaceColor = color;
    lastfaceRect.xPos = xPos;
    lastfaceRect.yPos = yPos;
    lastfaceRect.width = width;
    lastfaceRect.height = height;
}

void app_ui_face_menu_detect_clear(void)
{
    if(lv_faceDetect_menu.faceDetect_rect != 0)
    {
        pthread_mutex_lock(&uifreshMutex);
        lv_task_handler();
        lv_obj_set_hidden(lv_faceDetect_menu.faceDetect_rect,1);
        lv_task_handler();
        pthread_mutex_unlock(&uifreshMutex);
    }
}

void app_ui_face_menu_infor_show(char *namestr,char showflag)
{
    static char lastshowflag = 0;
    static lv_style_t bk_style;
    char strbuf[12];
    char utf8str[12];

    pthread_mutex_lock(&uifreshMutex);
    lv_task_handler();

    if(lastshowflag == showflag && showflag != 2)
    {
        pthread_mutex_unlock(&uifreshMutex);
        return;
    }

    if (showflag == 2)
        showflag = 1;
    if(showflag == 1)
    {
        if(lv_faceDetect_menu.menu_bk == 0)
        {
            /*creat face infor menu*/
            lv_faceDetect_menu.menu_bk = lv_obj_create(lv_faceDetect_menu.win, NULL);;
            lv_obj_set_size(lv_faceDetect_menu.menu_bk,infWin_w,infWin_h - 10);
            lv_obj_set_pos(lv_faceDetect_menu.menu_bk,150,detWin_h);

            /*creat face detect rect style*/
            lv_style_copy(&bk_style,&lv_style_btn_rel);
            bk_style.body.radius = 5;//LV_DPI / 15;
            bk_style.body.border.color = LV_COLOR_WHITE;//LV_COLOR_MAKE(0x0d, 0x14, 0x27);//LV_COLOR_MAKE(0x0b, 0x19, 0x28);
            bk_style.body.border.width = 2;//LV_DPI / 50 >= 1 ? LV_DPI / 50  : 1;
            lv_obj_set_style(lv_faceDetect_menu.menu_bk, &bk_style);

            memset(strbuf,0,sizeof(strbuf));
            memset(utf8str,0,sizeof(utf8str));
            sprintf(strbuf, "姓名: ");
            utility_gbk_to_utf8(strbuf, utf8str, 12);
            lv_faceDetect_menu.menu_label1 = lv_label_create(lv_faceDetect_menu.menu_bk, NULL);
            lv_label_set_style( lv_faceDetect_menu.menu_label1, &label_style);
        	lv_obj_align(lv_faceDetect_menu.menu_label1, lv_faceDetect_menu.menu_bk,LV_ALIGN_IN_LEFT_MID,40,0);
            lv_label_set_text(lv_faceDetect_menu.menu_label1,utf8str);

            lv_faceDetect_menu.menu_label2 = lv_label_create(lv_faceDetect_menu.menu_bk, NULL);
            lv_label_set_style( lv_faceDetect_menu.menu_label2, &label_style);
        	lv_obj_align(lv_faceDetect_menu.menu_label2, lv_faceDetect_menu.menu_label1,LV_ALIGN_IN_RIGHT_MID,10,0);
            if(namestr != NULL)
            {
                lv_label_set_text(lv_faceDetect_menu.menu_label2,namestr);
            }
        }
        else
        {
            if(namestr != NULL)
            {
                lv_label_set_text(lv_faceDetect_menu.menu_label2,namestr);
            }
            lv_obj_set_hidden(lv_faceDetect_menu.menu_bk,0);
        }
    }
    else
    {
        if(lv_faceDetect_menu.menu_bk != 0)
        {
            lv_label_set_text(lv_faceDetect_menu.menu_label2," ");
            lv_obj_set_hidden(lv_faceDetect_menu.menu_bk,1);
        }
    }
    lv_task_handler();
    pthread_mutex_unlock(&uifreshMutex);

    lastshowflag = showflag;
}


void app_ui_face_menu_init(void)
{
    showTimeWin_w = IMG_W;
    showTimeWin_h = 40;
    infWin_w = 200;
    infWin_h = 60;
    detWin_w = LV_HOR_RES;
    detWin_h = LV_VER_RES - infWin_h - showTimeWin_h;
    memset(&lv_faceDetect_menu, 0, sizeof(lv_faceDetect_menu));
	if(pthread_mutex_init(&uifreshMutex, NULL) < 0)
	{
	    printf("pthread_mutex_init create uifreshMutex erro\n");
	}

}

void app_ui_face_menu_entery(void)
{
    pthread_mutex_lock(&uifreshMutex);
    lv_task_handler();
    app_ui_face_menu_create();
    lv_task_handler();
    pthread_mutex_unlock(&uifreshMutex);
    netcam_timer_add_task(app_ui_face_menu_time_refresh, 58*NETCAM_TIMER_ONE_SEC, SDK_FALSE, SDK_FALSE);
}

void app_ui_face_menu_exit(void)
{
    lv_faceDetect_menu.win = 0;
    lv_obj_set_hidden(lv_faceDetect_menu.win,1);
}
