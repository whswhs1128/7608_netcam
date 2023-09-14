/*!
*****************************************************************************
** file        sdk_ptz.h
**
** version     2016-05-16 09:14 heyong -> v1.0
**
** brief       the api defines of sdk_ptz.
**
** attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2016-2020 BY GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#ifndef _SDK_PTZ_H_
#define _SDK_PTZ_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sdk_gpio.h"

#define SDK_PTZ_ADJUST_ASYNC        1

#define SDK_PTZ_CTRL_GPIO_COUNT     4

#define SDK_PTZ_ERR_UP_END          (-1)
#define SDK_PTZ_ERR_DOWN_END        (-2)
#define SDK_PTZ_ERR_LEFT_END        (-3)
#define SDK_PTZ_ERR_RIGHT_END       (-4)
#define SDK_PTZ_ERR_QUICK_EXIT      (-5)

typedef struct sdk_ptz_s {
	int quick_exit_flag;/*用于电机正在执行循环时的快速退出*/
	int clockwise;/*PTZ电机正反转切换控制*/
	int ud_gpio[SDK_PTZ_CTRL_GPIO_COUNT];
	sdk_gpio_handle_t ud_gpio_handle[SDK_PTZ_CTRL_GPIO_COUNT];
	int ud_limit_gpio;/*垂直旋转的GPIO限位GPIO配置*/
	sdk_gpio_handle_t ud_limit_gpio_handle;/*垂直旋转的GPIO限位GPIO配置*/
    int u_limit_val;  /*垂直旋转时，电机在最上时GPI0读出的值*/
    int d_limit_val;  /*垂直旋转时，电机在最下时GPI0读出的值*/

	int lr_gpio[SDK_PTZ_CTRL_GPIO_COUNT];
	sdk_gpio_handle_t lr_gpio_handle[SDK_PTZ_CTRL_GPIO_COUNT];
	int lr_limit_gpio;/*水平旋转的GPIO限位GPIO配置*/
	sdk_gpio_handle_t lr_limit_gpio_handle;/*水平旋转的GPIO限位GPIO配置*/
    int l_limit_val;  /*水平旋转时，电机在最左边时GPI0读出的值*/
    int r_limit_val;  /*水平旋转时，电机在最右边时GPI0读出的值*/

    int horizontal_step;/*垂直转动时的步长*/
    int max_horizontal_step; /*垂直转动时的最大步长，不同的电机和硬件需要调整*/
    int horizontal_centre_step; /*垂直转动时的镜头在中间的步长，不同的电机和硬件需要调整*/
    int vertical_step;/*水平转动时的步长*/
    int max_vertical_step;  /*水平转动时的最大步长，不同的电机和硬件需要调整*/
    int vertical_centre_step; /*水平转动时的镜头在中间的步长，不同的电机和硬件需要调整*/
    int reboot_ptz_disable;   /**/

} sdk_ptz_t;

int sdk_ptz_init(sdk_ptz_t *ptz);
int sdk_ptz_deinit();
int sdk_ptz_up(int step, int speed);
int sdk_ptz_down(int step, int speed);
int sdk_ptz_left(int step, int speed);
int sdk_ptz_right(int step, int speed);
int sdk_ptz_left_up(int step, int speed);
int sdk_ptz_left_down(int step, int speed);
int sdk_ptz_right_up(int step, int speed);
int sdk_ptz_right_down(int step, int speed);
int sdk_ptz_get_current_step(int *horizontal, int *vertical);
int sdk_ptz_stop_loop();
int sdk_ptz_start_and_goto_center();

#ifdef __cplusplus
};
#endif
#endif /* _SDK_PTZ_H_ */

