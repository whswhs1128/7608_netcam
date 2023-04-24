/*!
*****************************************************************************
** file        sdk_ptz.c
**
** version     2016-05-16 09:14 heyong -> v1.0
**
** brief       the implementation of sdk_ptz.h.
**
** attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2016-2020 BY GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#include <pthread.h>
#include "sdk_debug.h"
#include "sdk_ptz.h"
#include "sdk_gpio.h"
#include "sdk_cfg.h"

#define SDK_PTZ_ONCE_CTRL_MAX_STEP      4

static sdk_ptz_t g_ptz_ctrl;

//4Ïà4ÅÄ
static char g_clockwise[SDK_PTZ_ONCE_CTRL_MAX_STEP] = {0x3, 0x9, 0xc, 0x6};
static char g_anti_clockwise[SDK_PTZ_ONCE_CTRL_MAX_STEP] = {0x3, 0x6, 0xc, 0x9};
static int g_startup_adjust = 0;

typedef enum {
    PTZ_UP = 0,
    PTZ_DOWN,
    PTZ_LEFT,
    PTZ_RIGHT,
    PTZ_LEFT_UP,
    PTZ_LEFT_DOWN,
    PTZ_RIGHT_UP,
    PTZ_RIGHT_DOWN
} ptz_direction_t;

//70~80*1000
#define SDK_PTZ_PULSE_DELAY_STARTUP()    \
    do {\
        int i, j; \
        for (i = 0; i < 70; i++) \
            for (j = 0; j < 1000; j++);\
    } while(0)

#define SDK_PTZ_PULSE_DELAY()    usleep(1)


/* speed: 1, n*5*/
static int _ptz_step_once(ptz_direction_t direction, int speed)
{
	int i = 0;
	int j = 0;
    int startup_adjust = 0;//g_startup_adjust;
    sdk_ptz_t *ptz_ctrl = &g_ptz_ctrl;

	if (ptz_ctrl->quick_exit_flag)
	    return SDK_PTZ_ERR_QUICK_EXIT;

    switch (direction) {
    case PTZ_UP://up
        if (ptz_ctrl->vertical_step >= ptz_ctrl->max_vertical_step) {
            LOG_INFO("SDK_PTZ: [UP_END]vertical = %d, horizontal = %d", ptz_ctrl->vertical_step, ptz_ctrl->horizontal_step);
            return SDK_PTZ_ERR_UP_END;
        }
        for (i = 0; i < SDK_PTZ_ONCE_CTRL_MAX_STEP; i++) {
            for (j = 0; j < SDK_PTZ_CTRL_GPIO_COUNT; j++) {
                sdk_gpio_set_value(&ptz_ctrl->ud_gpio_handle[j],
                                    (g_anti_clockwise[i] >> j) & 1);
            }
            if (startup_adjust)
                SDK_PTZ_PULSE_DELAY_STARTUP();
            else
                SDK_PTZ_PULSE_DELAY();

		    if (speed > 0)
		        usleep(speed*50);
        }
        ptz_ctrl->vertical_step++;
        break;
    case PTZ_DOWN://down
        if (ptz_ctrl->vertical_step <= 0) {
            LOG_INFO("SDK_PTZ: [DOWN_END]vertical = %d, horizontal = %d", ptz_ctrl->vertical_step, ptz_ctrl->horizontal_step);
            return SDK_PTZ_ERR_DOWN_END;
        }
        for (i = 0; i < SDK_PTZ_ONCE_CTRL_MAX_STEP; i++) {
            for (j = 0; j < SDK_PTZ_CTRL_GPIO_COUNT; j++) {
                sdk_gpio_set_value(&ptz_ctrl->ud_gpio_handle[j],
                                    (g_clockwise[i] >> j) & 1);
            }
            if (startup_adjust)
                SDK_PTZ_PULSE_DELAY_STARTUP();
            else
                SDK_PTZ_PULSE_DELAY();

		    if (speed > 0)
		        usleep(speed*50);
        }
        ptz_ctrl->vertical_step--;
        break;
    case PTZ_LEFT://left
        if (ptz_ctrl->horizontal_step <= 0) {
            LOG_INFO("SDK_PTZ: [LEFT_END]vertical = %d, horizontal = %d", ptz_ctrl->vertical_step, ptz_ctrl->horizontal_step);
            return SDK_PTZ_ERR_LEFT_END;
        }
        for (i = 0; i < SDK_PTZ_ONCE_CTRL_MAX_STEP; i++) {
            for (j = 0; j < SDK_PTZ_CTRL_GPIO_COUNT; j++) {
                sdk_gpio_set_value(&ptz_ctrl->lr_gpio_handle[j],
                                    (g_anti_clockwise[i] >> j) & 1);
            }
            if (startup_adjust)
                SDK_PTZ_PULSE_DELAY_STARTUP();
            else
                SDK_PTZ_PULSE_DELAY();

		    if (speed > 0)
		        usleep(speed*50);
        }
        ptz_ctrl->horizontal_step--;
        break;
    case PTZ_RIGHT://right
        if (ptz_ctrl->horizontal_step >= ptz_ctrl->max_horizontal_step) {
            LOG_INFO("SDK_PTZ: [RIGHT_END]vertical = %d, horizontal = %d", ptz_ctrl->vertical_step, ptz_ctrl->horizontal_step);
            return SDK_PTZ_ERR_RIGHT_END;
        }
        for (i = 0; i < SDK_PTZ_ONCE_CTRL_MAX_STEP; i++) {
            for (j = 0; j < SDK_PTZ_CTRL_GPIO_COUNT; j++) {
                sdk_gpio_set_value(&ptz_ctrl->lr_gpio_handle[j],
                                    (g_clockwise[i] >> j) & 1);
            }
            if (startup_adjust)
                SDK_PTZ_PULSE_DELAY_STARTUP();
            else
                SDK_PTZ_PULSE_DELAY();

		    if (speed > 0)
		        usleep(speed*50);
        }
        ptz_ctrl->horizontal_step++;
        break;
    case PTZ_LEFT_UP://left_up
        if (ptz_ctrl->vertical_step >= ptz_ctrl->max_vertical_step) {
            LOG_INFO("SDK_PTZ: [UP_END]vertical = %d, horizontal = %d", ptz_ctrl->vertical_step, ptz_ctrl->horizontal_step);
            return SDK_PTZ_ERR_UP_END;
        }
        if (ptz_ctrl->horizontal_step <= 0) {
            LOG_INFO("SDK_PTZ: [LEFT_END]vertical = %d, horizontal = %d", ptz_ctrl->vertical_step, ptz_ctrl->horizontal_step);
            return SDK_PTZ_ERR_LEFT_END;
        }
        for (i = 0; i < SDK_PTZ_ONCE_CTRL_MAX_STEP; i++) {
            for (j = 0; j < SDK_PTZ_CTRL_GPIO_COUNT; j++) {
                sdk_gpio_set_value(&ptz_ctrl->ud_gpio_handle[j],
                                    (g_anti_clockwise[i] >> j) & 1);
                sdk_gpio_set_value(&ptz_ctrl->lr_gpio_handle[j],
                                    (g_anti_clockwise[i] >> j) & 1);
            }
            if (startup_adjust)
                SDK_PTZ_PULSE_DELAY_STARTUP();
            else
                SDK_PTZ_PULSE_DELAY();

		    if (speed > 0)
		        usleep(speed*50);
        }
        ptz_ctrl->vertical_step++;
        ptz_ctrl->horizontal_step--;
        break;
    case PTZ_RIGHT_UP://right_up
        if (ptz_ctrl->vertical_step >= ptz_ctrl->max_vertical_step) {
            LOG_INFO("SDK_PTZ: [UP_END]vertical = %d, horizontal = %d", ptz_ctrl->vertical_step, ptz_ctrl->horizontal_step);
            return SDK_PTZ_ERR_UP_END;
        }
        if (ptz_ctrl->horizontal_step >= ptz_ctrl->max_horizontal_step) {
            LOG_INFO("SDK_PTZ: [RIGHT_END]vertical = %d, horizontal = %d", ptz_ctrl->vertical_step, ptz_ctrl->horizontal_step);
            return SDK_PTZ_ERR_RIGHT_END;
        }
        for (i = 0; i < SDK_PTZ_ONCE_CTRL_MAX_STEP; i++) {
            for (j = 0; j < SDK_PTZ_CTRL_GPIO_COUNT; j++) {
                sdk_gpio_set_value(&ptz_ctrl->ud_gpio_handle[j],
                                    (g_anti_clockwise[i] >> j) & 1);
                sdk_gpio_set_value(&ptz_ctrl->lr_gpio_handle[j],
                                    (g_clockwise[i] >> j) & 1);
            }
            if (startup_adjust)
                SDK_PTZ_PULSE_DELAY_STARTUP();
            else
                SDK_PTZ_PULSE_DELAY();

		    if (speed > 0)
		        usleep(speed*50);
        }
        ptz_ctrl->vertical_step++;
        ptz_ctrl->horizontal_step++;
        break;
    case PTZ_LEFT_DOWN://left_down
        if (ptz_ctrl->vertical_step <= 0) {
            LOG_INFO("SDK_PTZ: [DOWN_END]vertical = %d, horizontal = %d", ptz_ctrl->vertical_step, ptz_ctrl->horizontal_step);
            return SDK_PTZ_ERR_DOWN_END;
        }
        if (ptz_ctrl->horizontal_step <= 0) {
            LOG_INFO("SDK_PTZ: [LEFT_END]vertical = %d, horizontal = %d", ptz_ctrl->vertical_step, ptz_ctrl->horizontal_step);
            return SDK_PTZ_ERR_LEFT_END;
        }
        for (i = 0; i < SDK_PTZ_ONCE_CTRL_MAX_STEP; i++) {
            for (j = 0; j < SDK_PTZ_CTRL_GPIO_COUNT; j++) {
                sdk_gpio_set_value(&ptz_ctrl->ud_gpio_handle[j],
                                    (g_clockwise[i] >> j) & 1);
                sdk_gpio_set_value(&ptz_ctrl->lr_gpio_handle[j],
                                    (g_anti_clockwise[i] >> j) & 1);
            }
            if (startup_adjust)
                SDK_PTZ_PULSE_DELAY_STARTUP();
            else
                SDK_PTZ_PULSE_DELAY();

		    if (speed > 0)
		        usleep(speed*50);
        }
        ptz_ctrl->vertical_step--;
        ptz_ctrl->horizontal_step--;
        break;
    case PTZ_RIGHT_DOWN://right_down
        if (ptz_ctrl->vertical_step <= 0) {
            LOG_INFO("SDK_PTZ: [DOWN_END]vertical = %d, horizontal = %d", ptz_ctrl->vertical_step, ptz_ctrl->horizontal_step);
            return SDK_PTZ_ERR_DOWN_END;
        }
        if (ptz_ctrl->horizontal_step >= ptz_ctrl->max_horizontal_step) {
            LOG_INFO("SDK_PTZ: [RIGHT_END]vertical = %d, horizontal = %d", ptz_ctrl->vertical_step, ptz_ctrl->horizontal_step);
            return SDK_PTZ_ERR_RIGHT_END;
        }
        for (i = 0; i < SDK_PTZ_ONCE_CTRL_MAX_STEP; i++) {
            for (j = 0; j < SDK_PTZ_CTRL_GPIO_COUNT; j++) {
                sdk_gpio_set_value(&ptz_ctrl->ud_gpio_handle[j],
                                    (g_clockwise[i] >> j) & 1);
                sdk_gpio_set_value(&ptz_ctrl->lr_gpio_handle[j],
                                    (g_clockwise[i] >> j) & 1);
            }
            if (startup_adjust)
                SDK_PTZ_PULSE_DELAY_STARTUP();
            else
                SDK_PTZ_PULSE_DELAY();

		    if (speed > 0)
		        usleep(speed*50);
        }
        ptz_ctrl->vertical_step--;
        ptz_ctrl->horizontal_step++;
        break;
    default:
        LOG_ERR("unknown direction\n");
        break;
    }

    //LOG_INFO("SDK_PTZ: vertical = %d, horizontal = %d", ptz_ctrl->vertical_step, ptz_ctrl->horizontal_step);
    return 0;
}

static int _ptz_stop(int ud_stop, int lr_stop)
{
	int i = 0;
    for (i = 0; i < SDK_PTZ_CTRL_GPIO_COUNT; i++) {
        if (ud_stop) {
            sdk_gpio_set_value(&g_ptz_ctrl.ud_gpio_handle[i], 0);
        }
        if (lr_stop) {
            sdk_gpio_set_value(&g_ptz_ctrl.lr_gpio_handle[i], 0);
        }
    }
    return 0;
}

int sdk_ptz_init(sdk_ptz_t *ptz)
{
    int i = 0;
    memcpy(&g_ptz_ctrl, ptz, sizeof(sdk_ptz_t));

    // init ptz gpios.
    for (i = 0; i < SDK_PTZ_CTRL_GPIO_COUNT; i++) {
        sdk_gpio_open(&g_ptz_ctrl.ud_gpio_handle[i], g_ptz_ctrl.ud_gpio[i]);
        sdk_gpio_set_direction(&g_ptz_ctrl.ud_gpio_handle[i], SDK_GPIO_OUT);
        sdk_gpio_open(&g_ptz_ctrl.lr_gpio_handle[i], g_ptz_ctrl.lr_gpio[i]);
        sdk_gpio_set_direction(&g_ptz_ctrl.lr_gpio_handle[i], SDK_GPIO_OUT);
    }
    sdk_gpio_open(&g_ptz_ctrl.ud_limit_gpio_handle, g_ptz_ctrl.ud_limit_gpio);
    sdk_gpio_set_direction(&g_ptz_ctrl.ud_limit_gpio_handle, SDK_GPIO_IN);
    sdk_gpio_open(&g_ptz_ctrl.lr_limit_gpio_handle, g_ptz_ctrl.lr_limit_gpio);
    sdk_gpio_set_direction(&g_ptz_ctrl.lr_limit_gpio_handle, SDK_GPIO_IN);

    if(g_ptz_ctrl.clockwise)
    {
        g_clockwise[0] = 0x3;
        g_clockwise[1] = 0x9;
        g_clockwise[2] = 0xc;
        g_clockwise[3] = 0x6;
        g_anti_clockwise[0] = 0x3;
        g_anti_clockwise[1] = 0x6;
        g_anti_clockwise[2] = 0xc;
        g_anti_clockwise[3] = 0x9;
    }else{
        g_anti_clockwise[0] = 0x3;
        g_anti_clockwise[1] = 0x9;
        g_anti_clockwise[2] = 0xc;
        g_anti_clockwise[3] = 0x6;
        g_clockwise[0] = 0x3;
        g_clockwise[1] = 0x6;
        g_clockwise[2] = 0xc;
        g_clockwise[3] = 0x9;
    }

    // adjust motor.
    if (!SDK_PTZ_ADJUST_ASYNC)
    {
    
        printf("reboot_ptz_disable = %d\n",ptz->reboot_ptz_disable);
        if(0 == ptz->reboot_ptz_disable)//not run ptz sdk_ptz_start_and_goto_center
        {
            sdk_ptz_start_and_goto_center();
        }
    }
	return 0;
}

int sdk_ptz_deinit()
{
    int i = 0;
    _ptz_stop(1, 1);

    // init ptz gpios.
    for (i = 0; i < SDK_PTZ_CTRL_GPIO_COUNT; i++) {
        sdk_gpio_close(&g_ptz_ctrl.ud_gpio_handle[i]);
        sdk_gpio_close(&g_ptz_ctrl.lr_gpio_handle[i]);
    }
    sdk_gpio_close(&g_ptz_ctrl.ud_limit_gpio_handle);
    sdk_gpio_close(&g_ptz_ctrl.lr_limit_gpio_handle);
	return 0;
}


/* speed 1~5 */
int sdk_ptz_up(int step, int speed)
{
    int i = 0;
    int value = -1;
    int ret = 0;
    int hw_limit = 0;//hardware limit enable

    if (step <= 0 || speed < 0)
        return -1;

    if (g_ptz_ctrl.ud_limit_gpio > 0) {
        // set limit_gpio to !limit.
        hw_limit = sdk_gpio_set_pull_up_down(&g_ptz_ctrl.ud_limit_gpio_handle,
                                                !g_ptz_ctrl.u_limit_val);
    }

    g_ptz_ctrl.quick_exit_flag = 0;
    for (i = 0; i < step; i++) {
        if (hw_limit) {
            // read limit_gpio.
            ret = sdk_gpio_get_value(&g_ptz_ctrl.ud_limit_gpio_handle, &value);
            if ((ret >= 0) && (g_ptz_ctrl.u_limit_val == value)) {
                g_ptz_ctrl.max_vertical_step = g_ptz_ctrl.vertical_step;//update max_vertical_step
                ret = -1;
                goto OUT;
            }
        }
        if ((ret = _ptz_step_once(PTZ_UP, speed)) < 0)
        {
            goto OUT;
        }
    }
OUT:
    _ptz_stop(1, 0);
    return ret;
}

int sdk_ptz_down(int step, int speed)
{
    int i = 0;
    int value = -1;
    int ret = 0;
    int hw_limit = 0;//hardware limit enable

    if (step <= 0 || speed < 0)
        return -1;

    if (g_ptz_ctrl.ud_limit_gpio > 0) {
        // set limit_gpio to !limit.
        hw_limit = sdk_gpio_set_pull_up_down(&g_ptz_ctrl.ud_limit_gpio_handle,
                                                !g_ptz_ctrl.d_limit_val);
    }

    g_ptz_ctrl.quick_exit_flag = 0;
    for (i = 0; i < step; i++) {
        if (hw_limit) {
            // read limit_gpio.
            ret = sdk_gpio_get_value(&g_ptz_ctrl.ud_limit_gpio_handle, &value);
            if ((ret >= 0) && (g_ptz_ctrl.d_limit_val == value)) {
                g_ptz_ctrl.vertical_step = 0;//update vertical_step
                ret = -1;
                goto OUT;
            }
        }
        if ((ret = _ptz_step_once(PTZ_DOWN, speed)) < 0) {
            goto OUT;
        }
    }
OUT:
    _ptz_stop(1, 0);
    return ret;
}

int sdk_ptz_left(int step, int speed)
{
    int i = 0;
    int value = -1;
    int ret = 0;
    int hw_limit = 0;//hardware limit enable

    if (step <= 0 || speed < 0)
        return -1;

    if (g_ptz_ctrl.lr_limit_gpio > 0) {
        // set limit_gpio to !limit.
        hw_limit = sdk_gpio_set_pull_up_down(&g_ptz_ctrl.lr_limit_gpio_handle,
                                                !g_ptz_ctrl.l_limit_val);
    }

    g_ptz_ctrl.quick_exit_flag = 0;
    for (i = 0; i < step; i++) {
        if (hw_limit) {
            // read limit_gpio.
            ret = sdk_gpio_get_value(&g_ptz_ctrl.lr_limit_gpio_handle, &value);
            if ((ret >= 0) && (g_ptz_ctrl.l_limit_val == value)) {
                g_ptz_ctrl.horizontal_step = 0;
                ret = -1;
                goto OUT;
            }
        }

        if ((ret = _ptz_step_once(PTZ_LEFT, speed)) < 0) {
            goto OUT;
        }
    }
OUT:
    _ptz_stop(0, 1);
    return ret;
}

int sdk_ptz_right(int step, int speed)
{
    int i = 0;
    int value = -1;
    int ret = 0;
    int hw_limit = 0;//hardware limit enable

    if (step <= 0 || speed < 0)
        return -1;

    if (g_ptz_ctrl.lr_limit_gpio > 0) {
        // set limit_gpio to !limit.
        hw_limit = sdk_gpio_set_pull_up_down(&g_ptz_ctrl.lr_limit_gpio_handle,
                                                !g_ptz_ctrl.r_limit_val);
    }

    g_ptz_ctrl.quick_exit_flag = 0;
    for (i = 0; i < step; i++) {
        if (hw_limit) {
            // read limit_gpio.
            ret = sdk_gpio_get_value(&g_ptz_ctrl.lr_limit_gpio_handle, &value);
            if ((ret >= 0) && (g_ptz_ctrl.r_limit_val == value)) {
                g_ptz_ctrl.max_horizontal_step = g_ptz_ctrl.horizontal_step;
                ret = -1;
                goto OUT;
            }
        }

        if ((ret = _ptz_step_once(PTZ_RIGHT, speed)) < 0) {
            goto OUT;
        }
    }
OUT:
    _ptz_stop(0, 1);
    return ret;
}

int sdk_ptz_left_up(int step, int speed)
{
    int i = 0;
    int value = -1;
    int ret = 0;
    int hw_limit_ud = 0;//ud_gpio limit enable
    int hw_limit_lr = 0;//lr_gpio limit enable

    if (step <= 0 || speed < 0)
        return -1;

    if (g_ptz_ctrl.ud_limit_gpio > 0) {
        // set limit_gpio to !limit.
        hw_limit_ud = sdk_gpio_set_pull_up_down(&g_ptz_ctrl.ud_limit_gpio_handle,
                                                !g_ptz_ctrl.u_limit_val);
    }

    if (g_ptz_ctrl.lr_limit_gpio > 0) {
        // set limit_gpio to !limit.
        hw_limit_lr = sdk_gpio_set_pull_up_down(&g_ptz_ctrl.lr_limit_gpio_handle,
                                                !g_ptz_ctrl.l_limit_val);
    }

    g_ptz_ctrl.quick_exit_flag = 0;
    for (i = 0; i < step; i++) {
        if (hw_limit_ud) {
            // read limit_gpio.
            ret = sdk_gpio_get_value(&g_ptz_ctrl.ud_limit_gpio_handle, &value);
            if ((ret >= 0) && (g_ptz_ctrl.u_limit_val == value)) {
                g_ptz_ctrl.max_vertical_step = g_ptz_ctrl.vertical_step;//update max_vertical_step
                _ptz_stop(1, 0);
            }
        }

        if (hw_limit_lr) {
            // read limit_gpio.
            ret = sdk_gpio_get_value(&g_ptz_ctrl.lr_limit_gpio_handle, &value);
            if ((ret >= 0) && (g_ptz_ctrl.l_limit_val == value)) {
                g_ptz_ctrl.horizontal_step = 0;
                _ptz_stop(0, 1);
            }
        }

        if ((ret = _ptz_step_once(PTZ_LEFT_UP, speed)) < 0) {
            goto OUT;
        }
    }
OUT:
    _ptz_stop(1, 1);
    return ret;
}

int sdk_ptz_left_down(int step, int speed)
{
    int i = 0;
    int value = -1;
    int ret = 0;
    int hw_limit_ud = 0;//ud_gpio limit enable
    int hw_limit_lr = 0;//lr_gpio limit enable

    if (step <= 0 || speed < 0)
        return -1;

    if (g_ptz_ctrl.ud_limit_gpio > 0) {
        // set limit_gpio to !limit.
        hw_limit_ud = sdk_gpio_set_pull_up_down(&g_ptz_ctrl.ud_limit_gpio_handle,
                                                !g_ptz_ctrl.d_limit_val);
    }

    if (g_ptz_ctrl.lr_limit_gpio > 0) {
        // set limit_gpio to !limit.
        hw_limit_lr = sdk_gpio_set_pull_up_down(&g_ptz_ctrl.lr_limit_gpio_handle,
                                                !g_ptz_ctrl.r_limit_val);
    }

    g_ptz_ctrl.quick_exit_flag = 0;
    for (i = 0; i < step; i++) {
        if (hw_limit_ud) {
            // read limit_gpio.
            ret = sdk_gpio_get_value(&g_ptz_ctrl.ud_limit_gpio_handle, &value);
            if ((ret >= 0) && (g_ptz_ctrl.d_limit_val == value)) {
                g_ptz_ctrl.vertical_step = 0;//update vertical_step
                _ptz_stop(1, 0);
            }
        }

        if (hw_limit_lr) {
            // read limit_gpio.
            ret = sdk_gpio_get_value(&g_ptz_ctrl.lr_limit_gpio_handle, &value);
            if ((ret >= 0) && (g_ptz_ctrl.l_limit_val == value)) {
                g_ptz_ctrl.horizontal_step = 0;
                _ptz_stop(0, 1);
            }
        }

        if ((ret = _ptz_step_once(PTZ_LEFT_DOWN, speed)) < 0) {
            goto OUT;
        }
    }
OUT:
    _ptz_stop(1, 1);
    return ret;
}

int sdk_ptz_right_up(int step, int speed)
{
    int i = 0;
    int value = -1;
    int ret = 0;
    int hw_limit_ud = 0;//ud_gpio limit enable
    int hw_limit_lr = 0;//lr_gpio limit enable

    if (step <= 0 || speed < 0)
        return -1;

    if (g_ptz_ctrl.ud_limit_gpio > 0) {
        // set limit_gpio to !limit.
        hw_limit_ud = sdk_gpio_set_pull_up_down(&g_ptz_ctrl.ud_limit_gpio_handle,
                                                !g_ptz_ctrl.u_limit_val);
    }

    if (g_ptz_ctrl.lr_limit_gpio > 0) {
        // set limit_gpio to !limit.
        hw_limit_lr = sdk_gpio_set_pull_up_down(&g_ptz_ctrl.lr_limit_gpio_handle,
                                                !g_ptz_ctrl.r_limit_val);
    }

    g_ptz_ctrl.quick_exit_flag = 0;
    for (i = 0; i < step; i++) {
        if (hw_limit_ud) {
            // read limit_gpio.
            ret = sdk_gpio_get_value(&g_ptz_ctrl.ud_limit_gpio_handle, &value);
            if ((ret >= 0) && (g_ptz_ctrl.u_limit_val == value)) {
                g_ptz_ctrl.max_vertical_step = g_ptz_ctrl.vertical_step;//update max_vertical_step
                _ptz_stop(1, 0);
            }
        }

        if (hw_limit_lr) {
            // read limit_gpio.
            ret = sdk_gpio_get_value(&g_ptz_ctrl.lr_limit_gpio_handle, &value);
            if ((ret >= 0) && (g_ptz_ctrl.r_limit_val == value)) {
                g_ptz_ctrl.max_horizontal_step = g_ptz_ctrl.horizontal_step;
                _ptz_stop(0, 1);
            }
        }

        if ((ret = _ptz_step_once(PTZ_RIGHT_UP, speed)) < 0) {
            goto OUT;
        }
    }
OUT:
    _ptz_stop(1, 1);
    return ret;
}

int sdk_ptz_right_down(int step, int speed)
{
    int i = 0;
    int value = -1;
    int ret = 0;
    int hw_limit_ud = 0;//ud_gpio limit enable
    int hw_limit_lr = 0;//lr_gpio limit enable

    if (step <= 0 || speed < 0)
        return -1;

    if (g_ptz_ctrl.ud_limit_gpio > 0) {
        // set limit_gpio to !limit.
        hw_limit_ud = sdk_gpio_set_pull_up_down(&g_ptz_ctrl.ud_limit_gpio_handle,
                                                !g_ptz_ctrl.d_limit_val);
    }

    if (g_ptz_ctrl.lr_limit_gpio > 0) {
        // set limit_gpio to !limit.
        hw_limit_lr = sdk_gpio_set_pull_up_down(&g_ptz_ctrl.lr_limit_gpio_handle,
                                                !g_ptz_ctrl.r_limit_val);
    }

    g_ptz_ctrl.quick_exit_flag = 0;
    for (i = 0; i < step; i++) {
        if (hw_limit_ud) {
            // read limit_gpio.
            ret = sdk_gpio_get_value(&g_ptz_ctrl.ud_limit_gpio_handle, &value);
            if ((ret >= 0) && (g_ptz_ctrl.d_limit_val == value)) {
                g_ptz_ctrl.vertical_step = 0;//update vertical_step
                _ptz_stop(1, 0);
            }
        }

        if (hw_limit_lr) {
            // read limit_gpio.
            ret = sdk_gpio_get_value(&g_ptz_ctrl.lr_limit_gpio_handle, &value);
            if ((ret >= 0) && (g_ptz_ctrl.r_limit_val == value)) {
                g_ptz_ctrl.max_horizontal_step = g_ptz_ctrl.horizontal_step;
                _ptz_stop(0, 1);
            }
        }

        if ((ret = _ptz_step_once(PTZ_RIGHT_DOWN, speed)) < 0) {
            goto OUT;
        }
    }
OUT:
    _ptz_stop(1, 1);
    return ret;
}

int sdk_ptz_get_current_step(int *horizontal, int *vertical)
{
    if ((horizontal == NULL) || (vertical == NULL)) {
        LOG_ERR("horizontal or vertical is null\n");
        return -1;
    }
    *horizontal = g_ptz_ctrl.horizontal_step;
    *vertical = g_ptz_ctrl.vertical_step;
    return 0;
}

int sdk_ptz_stop_loop()
{
    g_ptz_ctrl.quick_exit_flag = 1;
    return 0;
}

int sdk_ptz_start_and_goto_center()
{
    //int i;
    int ver_loop_count;
    int ver_center_loop_count;
    int hor_loop_count;
    int hor_center_loop_count;
	char slow_mode = 0;
	
    g_startup_adjust = 1;

    g_ptz_ctrl.vertical_step = g_ptz_ctrl.max_vertical_step;
    g_ptz_ctrl.horizontal_step = g_ptz_ctrl.max_horizontal_step;

    ver_loop_count = g_ptz_ctrl.max_vertical_step;
    ver_center_loop_count = g_ptz_ctrl.vertical_centre_step;
    hor_loop_count = g_ptz_ctrl.max_horizontal_step;
    hor_center_loop_count = g_ptz_ctrl.horizontal_centre_step;

    LOG_INFO("SDK_PTZ_ADJUST: vertical(%d,%d) horizontal(%d,%d)",
        ver_loop_count, ver_center_loop_count, hor_loop_count, hor_center_loop_count);
	
	if(strstr(sdk_cfg.name, "CUS_TB_GC2053_V20"))
	{
		slow_mode = 1;
	}

    int remain_step = hor_loop_count - (ver_loop_count + ver_center_loop_count);
    if (remain_step > 0 && !slow_mode) {/* adjust the up_down and left_right motor at the same time. (heyong) */
        remain_step = hor_loop_count;

        sdk_ptz_left_down(ver_loop_count, 0);
        remain_step -= ver_loop_count;

        sdk_ptz_left_up(ver_center_loop_count, 0);
        remain_step -= ver_center_loop_count;

        sdk_ptz_left(remain_step, 0);//goto left position where vertical_step eq 0.
        sdk_ptz_right(hor_center_loop_count, 0);//goto horizontal center position
    } else {
        sdk_ptz_down(ver_loop_count, 0);//goto down position where vertical_step eq 0.
        sdk_ptz_up(ver_center_loop_count, 0);//goto vertical center position
        
		if(strstr(sdk_cfg.name, "CUS_TB_GC2053_V20"))
			usleep(100000);
		
        sdk_ptz_left(hor_loop_count, 0);//goto left position where vertical_step eq 0.
        sdk_ptz_right(hor_center_loop_count, 0);//goto horizontal center position
    }

    g_startup_adjust = 0;
    return 0;
}

int sdk_ptz_get_startup_adjust_flag(void)
{
	if(strstr(sdk_cfg.name, "CUS_TB_GC2053_V20"))
		return g_startup_adjust;
	else
		return 0;
}

#if 0
void *y_ptz_test(void *arg)
{
    sdk_ptz_init();

    LOG_WARNING(">>>>>>>>>>>>>>>>> PTZ START.");
    sdk_ptz_start_and_goto_center();

    LOG_WARNING(">>>>>>>>>>>>>>>>> PTZ IN CENTER.");

    LOG_WARNING("PTZ POSI: ver = %d(0~%d)", g_ptz_ctrl.vertical_step, g_ptz_ctrl.max_vertical_step);
    LOG_WARNING("PTZ POSI: hor = %d(0~%d)", g_ptz_ctrl.horizontal_step, g_ptz_ctrl.max_horizontal_step);
    while(1)
    {
        sdk_ptz_up(100, 1);
        sdk_ptz_down(100, 1);

        sleep(2);
        LOG_WARNING(">>>>>>>>>>>>>>>>> PTZ IN CENTER.");

        sdk_ptz_right(100, 1);
        sdk_ptz_left(100, 1);

        sleep(2);
        LOG_WARNING(">>>>>>>>>>>>>>>>> PTZ IN CENTER.");

        sdk_ptz_left_up(100, 1);
        sdk_ptz_right_down(100, 1);

        sleep(2);
        LOG_WARNING(">>>>>>>>>>>>>>>>> PTZ IN CENTER.");

        sdk_ptz_left_down(100, 1);
        sdk_ptz_right_up(100, 1);

        sleep(2);
        LOG_WARNING(">>>>>>>>>>>>>>>>> PTZ IN CENTER.");
    }
    return 0;
}
#endif
