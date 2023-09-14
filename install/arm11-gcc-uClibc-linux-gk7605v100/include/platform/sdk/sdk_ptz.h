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
	int quick_exit_flag;/*���ڵ������ִ��ѭ��ʱ�Ŀ����˳�*/
	int clockwise;/*PTZ�������ת�л�����*/
	int ud_gpio[SDK_PTZ_CTRL_GPIO_COUNT];
	sdk_gpio_handle_t ud_gpio_handle[SDK_PTZ_CTRL_GPIO_COUNT];
	int ud_limit_gpio;/*��ֱ��ת��GPIO��λGPIO����*/
	sdk_gpio_handle_t ud_limit_gpio_handle;/*��ֱ��ת��GPIO��λGPIO����*/
    int u_limit_val;  /*��ֱ��תʱ�����������ʱGPI0������ֵ*/
    int d_limit_val;  /*��ֱ��תʱ�����������ʱGPI0������ֵ*/

	int lr_gpio[SDK_PTZ_CTRL_GPIO_COUNT];
	sdk_gpio_handle_t lr_gpio_handle[SDK_PTZ_CTRL_GPIO_COUNT];
	int lr_limit_gpio;/*ˮƽ��ת��GPIO��λGPIO����*/
	sdk_gpio_handle_t lr_limit_gpio_handle;/*ˮƽ��ת��GPIO��λGPIO����*/
    int l_limit_val;  /*ˮƽ��תʱ������������ʱGPI0������ֵ*/
    int r_limit_val;  /*ˮƽ��תʱ����������ұ�ʱGPI0������ֵ*/

    int horizontal_step;/*��ֱת��ʱ�Ĳ���*/
    int max_horizontal_step; /*��ֱת��ʱ����󲽳�����ͬ�ĵ����Ӳ����Ҫ����*/
    int horizontal_centre_step; /*��ֱת��ʱ�ľ�ͷ���м�Ĳ�������ͬ�ĵ����Ӳ����Ҫ����*/
    int vertical_step;/*ˮƽת��ʱ�Ĳ���*/
    int max_vertical_step;  /*ˮƽת��ʱ����󲽳�����ͬ�ĵ����Ӳ����Ҫ����*/
    int vertical_centre_step; /*ˮƽת��ʱ�ľ�ͷ���м�Ĳ�������ͬ�ĵ����Ӳ����Ҫ����*/
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

