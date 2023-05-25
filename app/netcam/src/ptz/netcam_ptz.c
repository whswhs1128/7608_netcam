/*!
*****************************************************************************
** file        netcam_ptz.c
**
** version     2016-05-17 17:26 heyong -> v1.0
**
** brief       the implementation of ptz application.
**
** attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2016-2020 BY GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#include "netcam_api.h"
#include "sdk_cfg.h"
#include "sdk_ptz.h"


typedef struct ptz_preset_s {
	char name[NETCAM_PTZ_PRESET_NAME_SIZE];
    int enable;
    int horizontal;
    int vertical;
} ptz_preset_t;

typedef struct ptz_preset_cruise_s{
    unsigned char preset_order;
    unsigned char remain_time;
    unsigned char speed;
} ptz_preset_cruise_t;


typedef struct ptz_preset_cruise_info_s {
    unsigned char preset_order;
    unsigned char remain_time;
    unsigned char speed;
} ptz_preset_cruise_info_t;

typedef enum ptz_msg_e {
    PTZ_CTRL_CMD_STOP = 0,
    PTZ_CTRL_CMD_START,
    PTZ_CTRL_CMD_UP,
    PTZ_CTRL_CMD_DOWN,
    PTZ_CTRL_CMD_LEFT,
    PTZ_CTRL_CMD_LEFT_UP,
    PTZ_CTRL_CMD_LEFT_DOWN,
    PTZ_CTRL_CMD_RIGHT,
    PTZ_CTRL_CMD_RIGHT_UP,
    PTZ_CTRL_CMD_RIGHT_DOWN,
    PTZ_CTRL_CMD_GO_PRESET,
    PTZ_CTRL_CMD_PRESET_CRUISE,
    PTZ_CTRL_CMD_HOR_CRUISE,
    PTZ_CTRL_CMD_VER_CRUISE,
    PTZ_CTRL_CMD_HOR_VER_CRUISE
} ptz_ctrl_cmd_t;

typedef struct ptz_ctrl_msg_s {
    pthread_mutex_t lock;
    ptz_ctrl_cmd_t cmd;
    int step;
    int speed;
    ptz_preset_cruise_t preset_cruise[NETCAM_PTZ_MAX_PRESET_CRUISE_NUM];
    int preset_cruise_flag;
    int preset_cruise_count;/*大于1表示是预制点巡航, 等于1表示是跳预制点 */
} ptz_ctrl_msg_t;

typedef struct netcam_ptz_s {
    int running;
    int moving;
    ptz_ctrl_msg_t ctrl;
    int ctrl_coming;
    pthread_mutex_t lock;
    ptz_preset_t preset[NETCAM_PTZ_MAX_PRESET_NUM];
} netcam_ptz_t;


static CFG_PTZ_S *cfg_ptz = &sdk_cfg.ptz;
static netcam_ptz_t g_ptz;

#define PTZ_CTRL_MSG_LOCK()         pthread_mutex_lock(&g_ptz.ctrl.lock)
#define PTZ_CTRL_MSG_UNLOCK()       pthread_mutex_unlock(&g_ptz.ctrl.lock)

#define PTZ_LOCK()                  pthread_mutex_lock(&g_ptz.lock)
#define PTZ_UNLOCK()                pthread_mutex_unlock(&g_ptz.lock)

static int _ptz_preset_cfg_ctrl(int write_opt/*1->write, 0->read*/)
{
    int ret = 0;
    char path[128] = {0};
    FILE *fp = NULL;

    if (access(CFG_DIR, F_OK) != 0 ) {
        LOG_INFO("create preset cfg dir: %s\n", CFG_DIR);
        if ((mkdir(CFG_DIR, 0777)) < 0) {
            LOG_ERR("mkdir %s 0777 failed\n", CFG_DIR);
            return -1;
        }
    }

    sprintf(path, "%s%s", CFG_DIR, NETCAM_PTZ_PRESET_CFG_FILENAME);
    fp = fopen(path, (write_opt)?("wb+"):("rb"));
    if (fp == NULL) {
        LOG_WARNING("file %s not exist!\n", path);
        return -1;
    }

    if (write_opt) {//write
        ret = fwrite(g_ptz.preset, 1, sizeof(g_ptz.preset), fp);
    } else {//read
        ret = fread(g_ptz.preset, 1, sizeof(g_ptz.preset), fp);
    }

    if (ret != sizeof(g_ptz.preset)) {
        LOG_ERR("%s preset cfg error\n", (write_opt)?("write"):("read"));
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}

#define NETCAM_PTZ_CRUISE_TIME              (600)   /* 10min */

#define NETCAM_PTZ_CURISE_LIMIT_TIME_CHECK(start_time) \
    do { \
        /* 限制巡航时间，不超过NETCAM_PTZ_CRUISE_TIME设定值. */ \
        if ((time(NULL) - (start_time)) >= NETCAM_PTZ_CRUISE_TIME) { \
            /* 如果预制点设置后,  开机默认跳到预制点0上. */ \
            if (g_ptz.preset[0].enable) { \
                _ptz_goto_preset(0, 0); \
            } \
            /* stop cruise. */ \
            g_ptz.ctrl_coming = 1; \
            g_ptz.ctrl.cmd = PTZ_CTRL_CMD_STOP; \
        } \
    } while(0)

static int _ptz_goto_preset(int order, int speed)
{
    int offset_h = 0;
    int offset_v = 0;
    int cur_hor = 0;
    int cur_ver = 0;
    if (order >= NETCAM_PTZ_MAX_PRESET_NUM) {
        LOG_ERR("order error %d(0~%d)\n", order, NETCAM_PTZ_MAX_PRESET_NUM);
        return -1;
    }

    PTZ_LOCK();
    if (g_ptz.preset[order].enable == 0) {
        PTZ_UNLOCK();
        LOG_INFO("This preset point:%d is not enable!\n", order);
        return 0;
    }

    sdk_ptz_get_current_step(&cur_hor, &cur_ver);

    offset_h = g_ptz.preset[order].horizontal - cur_hor;
    offset_v = g_ptz.preset[order].vertical - cur_ver;
    PTZ_UNLOCK();

    LOG_INFO("preset:  %d (%d,%d)\n", order, g_ptz.preset[order].horizontal,
                                                g_ptz.preset[order].vertical);
    LOG_INFO("current: %d (%d,%d)\n", order, cur_hor, cur_ver);
    LOG_INFO("offset:  %d (%d,%d)\n", order, offset_v, offset_h);
    if (offset_h > 0) {
        if (offset_v > 0) {// 2 quadrant. right_up
            sdk_ptz_right(offset_h, speed);
            sdk_ptz_up(offset_v, speed);
        } else {// 4 quadrant. right_down
            offset_v = 0 - offset_v;
            sdk_ptz_right(offset_h, speed);
            sdk_ptz_down(offset_v, speed);
        }
    } else {
        offset_h = 0 - offset_h;
        if (offset_v > 0) {// 1 quadrant. left_up
            sdk_ptz_left(offset_h, speed);
            sdk_ptz_up(offset_v, speed);
        } else {// 3 quadrant. left_down
            offset_v = 0 - offset_v;
            sdk_ptz_left(offset_h, speed);
            sdk_ptz_down(offset_v, speed);
        }
    }
    return 0;
}

static int _set_default_preset(void)
{
    netcam_ptz_set_preset(0, "ptz_default");
    LOG_INFO("save ptz_default preset\n");
    return 0;
}
static void *_ptz_thread(void *arg)
{
#define STOP_SLEE_100MS     100000
    sdk_sys_thread_set_name("ptz_thread");
    int ret = 0;
    int i = 0;
    int reverse = 0;
    time_t start_time = 0;
    //time_t run_time = 0;
	int count = 0;

    ptz_ctrl_msg_t cur_msg;
    cur_msg.cmd = PTZ_CTRL_CMD_STOP;

    g_ptz.moving = 0;
    while (g_ptz.running) {
        PTZ_CTRL_MSG_LOCK();
        memcpy(&cur_msg, &g_ptz.ctrl, sizeof(g_ptz.ctrl));
        g_ptz.ctrl_coming = 0;
		if (cur_msg.cmd == PTZ_CTRL_CMD_STOP) {
            if((count ++ > 4) && (g_ptz.moving == 1)){//2s
                //LOG_ERR("xxxxxxxxxxxxx ptz stop count:%d\n", count);
                g_ptz.moving = 0;
                count = 0;
            }
        }else{
            g_ptz.moving = 1;
            count = 0;
        }
        PTZ_CTRL_MSG_UNLOCK();

		if (cur_msg.cmd == PTZ_CTRL_CMD_STOP) {
            usleep(STOP_SLEE_100MS);
            continue;
        }

        LOG_INFO("PTZ RUN CMD: %d, (%d - %d)", cur_msg.cmd,
                                                cur_msg.step, cur_msg.speed);
        switch (cur_msg.cmd) {
        case PTZ_CTRL_CMD_START:
            if (SDK_PTZ_ADJUST_ASYNC)
            {
            	LOG_INFO("PTZ_CTRL_CMD_START sdk_ptz_start_and_goto_center!\n");
                sdk_ptz_start_and_goto_center();
                
                /* 如果预制点设置后,  开机默认跳到预制点0上. */
                if (g_ptz.preset[0].enable) {
                    _ptz_goto_preset(0, cur_msg.speed);
                }
            }
            break;
        case PTZ_CTRL_CMD_UP:
            sdk_ptz_up(cur_msg.step, cur_msg.speed);
            break;
        case PTZ_CTRL_CMD_DOWN:
            sdk_ptz_down(cur_msg.step, cur_msg.speed);
            break;
        case PTZ_CTRL_CMD_LEFT:
            sdk_ptz_left(cur_msg.step, cur_msg.speed);
            break;
        case PTZ_CTRL_CMD_LEFT_UP:
            sdk_ptz_left_up(cur_msg.step, cur_msg.speed);
            break;
        case PTZ_CTRL_CMD_LEFT_DOWN:
            sdk_ptz_left_down(cur_msg.step, cur_msg.speed);
            break;
        case PTZ_CTRL_CMD_RIGHT:
            sdk_ptz_right(cur_msg.step, cur_msg.speed);
            break;
        case PTZ_CTRL_CMD_RIGHT_UP:
            sdk_ptz_right_up(cur_msg.step, cur_msg.speed);
            break;
        case PTZ_CTRL_CMD_RIGHT_DOWN:
            sdk_ptz_right_down(cur_msg.step, cur_msg.speed);
            break;
        case PTZ_CTRL_CMD_GO_PRESET:
        case PTZ_CTRL_CMD_PRESET_CRUISE:
            do {
                for (i = 0; i < cur_msg.preset_cruise_count; i++) {
                    if(_ptz_goto_preset(cur_msg.preset_cruise[i].preset_order,
                                         cur_msg.preset_cruise[i].speed) < 0) {
                        break;
                    }

                    /* sleep in preset point for crusie */
                    start_time = cur_msg.preset_cruise[i].remain_time;
                    while (start_time > 0 && !g_ptz.ctrl_coming) {
                        sleep(1);
                        start_time--;
                    }
                }
            } while (cur_msg.preset_cruise_flag && !g_ptz.ctrl_coming);
            break;
        case PTZ_CTRL_CMD_HOR_CRUISE:
            start_time = time(NULL);
            while (!g_ptz.ctrl_coming) {
                while (!g_ptz.ctrl_coming) {
                    ret = sdk_ptz_left(cur_msg.step, cur_msg.speed);
                    if (ret < 0)
                        break;

                    NETCAM_PTZ_CURISE_LIMIT_TIME_CHECK(start_time);
                }
                usleep(50000);
                while (!g_ptz.ctrl_coming) {
                    ret = sdk_ptz_right(cur_msg.step, cur_msg.speed);
                    if (ret < 0)
                        break;

                    NETCAM_PTZ_CURISE_LIMIT_TIME_CHECK(start_time);
                }
                usleep(50000);
            }
            break;
        case PTZ_CTRL_CMD_VER_CRUISE:
            start_time = time(NULL);
            while (!g_ptz.ctrl_coming) {
                while (!g_ptz.ctrl_coming) {
                    ret = sdk_ptz_up(cur_msg.step, cur_msg.speed);
                    if (ret < 0)
                        break;

                    NETCAM_PTZ_CURISE_LIMIT_TIME_CHECK(start_time);
                }
                usleep(50000);
                while (!g_ptz.ctrl_coming) {
                    ret = sdk_ptz_down(cur_msg.step, cur_msg.speed);
                    if (ret < 0)
                        break;

                    NETCAM_PTZ_CURISE_LIMIT_TIME_CHECK(start_time);
                }
                usleep(50000);
            }
            break;
        case PTZ_CTRL_CMD_HOR_VER_CRUISE:
            start_time = time(NULL);
            int verReverse = 0;
            while (!g_ptz.ctrl_coming) {
                int horStep = 0;
                int verStep = 0;
                while (!g_ptz.ctrl_coming) {
                    if (reverse)
                        ret = sdk_ptz_left(cur_msg.step, cur_msg.speed);
                    else
                        ret = sdk_ptz_right(cur_msg.step, cur_msg.speed);
                    horStep += ret;
                    if (ret < 0) {
                        if (ret == SDK_PTZ_ERR_LEFT_END
                            || ret == SDK_PTZ_ERR_RIGHT_END)
                            reverse = !reverse;
                        break;
                    }
                    printf("horStep:%d, %d\n", horStep, cfg_ptz->max_horizontal_step);

                    if (horStep > cfg_ptz->max_horizontal_step)
                    {
                        printf("hor stop...\n");
                        reverse = !reverse;
                        break;
                    }

                    NETCAM_PTZ_CURISE_LIMIT_TIME_CHECK(start_time);
                    usleep(850000);
                }
                while (!g_ptz.ctrl_coming) {
                    if (verReverse)
                        ret = sdk_ptz_down(cur_msg.step, cur_msg.speed);
                    else
                        ret = sdk_ptz_up(cur_msg.step, cur_msg.speed);
                    verStep += ret;
                    if (ret < 0) {
                        if (ret == SDK_PTZ_ERR_LEFT_END
                            || ret == SDK_PTZ_ERR_RIGHT_END)
                            horStep = !horStep;
                        break;
                    }
                    printf("verStep:%d, %d\n", verStep, cfg_ptz->max_vertical_step);

                    if (verStep > cfg_ptz->max_vertical_step)
                    {
                        printf("ver stop...\n");
                        verReverse = !verReverse;
                        break;
                    }

                    NETCAM_PTZ_CURISE_LIMIT_TIME_CHECK(start_time);
                    usleep(850000);
                }
            }
            break;
        default:
            break;
        }

        LOG_INFO("PTZ RUN CMD: %d end", cur_msg.cmd);

        PTZ_CTRL_MSG_LOCK();
        if (!g_ptz.ctrl_coming)
            g_ptz.ctrl.cmd = PTZ_CTRL_CMD_STOP;
        PTZ_CTRL_MSG_UNLOCK();
        netcam_timer_add_task((TIMER_HANDLER)_set_default_preset, NETCAM_TIMER_FOUR_SEC, SDK_FALSE, SDK_TRUE);
    }
    LOG_INFO("ptz thread exit");
    return NULL;
}

static int _ptz_preset_cruise(ptz_preset_cruise_info_t *cruise_info, int count)
{
    if ((cruise_info == NULL) || (count >= NETCAM_PTZ_MAX_PRESET_NUM)) {
        LOG_ERR("params error %d(0~%d)\n", count, NETCAM_PTZ_MAX_PRESET_NUM);
        return -1;
    }

    PTZ_CTRL_MSG_LOCK();
    g_ptz.ctrl.cmd = PTZ_CTRL_CMD_GO_PRESET;
    memcpy(g_ptz.ctrl.preset_cruise, cruise_info, count*sizeof(ptz_preset_cruise_info_t));
    g_ptz.ctrl.preset_cruise_count = count;

    if (count > 1)/*大于1表示是预制点巡航, 等于1表示是跳预制点 */
        g_ptz.ctrl.preset_cruise_flag  = 1;
    else
        g_ptz.ctrl.preset_cruise_flag  = 0;

    g_ptz.ctrl_coming = 1;
    sdk_ptz_stop_loop();
    PTZ_CTRL_MSG_UNLOCK();
    return 0;
}

static int _ptz_send_cmd(ptz_ctrl_cmd_t cmd, int step, int speed)
{
    if (!g_ptz.running)
        return -1;

    PTZ_CTRL_MSG_LOCK();

    //LOG_ERR("SEND PTZ CMD: begin step = %d, speed = %d", step, speed);
    /* step: 10, 20, 30, ... */
    if (step < 10)
        step = 10;

    /* speed: 0~64 */
    #ifdef MODULE_SUPPORT_AF
    if (speed < 0)
        speed = 0;
    else if(speed > NETCAM_PTZ_MAX_SPEED) {
        speed = NETCAM_PTZ_MAX_SPEED;
    }
    #else
    if (speed <= 0)
    speed = NETCAM_PTZ_MAX_SPEED;
    else if(speed > NETCAM_PTZ_MAX_SPEED) {
        speed = 0;
    }
    #endif
    speed = NETCAM_PTZ_MAX_SPEED - speed;
    //LOG_ERR("SEND PTZ CMD: after step = %d, speed = %d", step, speed);
    #ifdef MODULE_SUPPORT_AF
    step = 50;
    if ((cmd == PTZ_CTRL_CMD_UP) || (cmd == PTZ_CTRL_CMD_DOWN))
    {
        step = 20;
    }
    speed = NETCAM_PTZ_MAX_SPEED;
    #endif
    g_ptz.ctrl.cmd = cmd;
    g_ptz.ctrl.step = step;
    g_ptz.ctrl.speed = speed;
    g_ptz.ctrl_coming = 1;
    sdk_ptz_stop_loop();
    PTZ_CTRL_MSG_UNLOCK();
    return 0;
}

int netcam_ptz_init(void)
{
    sdk_ptz_t ptz;

    memset(&g_ptz, 0, sizeof(netcam_ptz_t));

    // judge enable after init g_ptz.
    if(!cfg_ptz->enable)
    {
        LOG_WARNING("ptz disable.\n");
        return -1;
    }

    /* cfg_ptz 赋值给 ptz */
    memset(&ptz, 0 , sizeof(sdk_ptz_t));
    ptz.clockwise = cfg_ptz->clockwise;

    // up down motor.
	ptz.ud_gpio[0] = cfg_ptz->ud_gpio_a;
	ptz.ud_gpio[1] = cfg_ptz->ud_gpio_b;
	ptz.ud_gpio[2] = cfg_ptz->ud_gpio_c;
	ptz.ud_gpio[3] = cfg_ptz->ud_gpio_d;

    ptz.ud_limit_gpio = cfg_ptz->ud_gpio_limit;
    ptz.u_limit_val = cfg_ptz->u_gpio_limit_valu;
    ptz.d_limit_val = cfg_ptz->d_gpio_limit_valu;

    // left right motor.
	ptz.lr_gpio[0] = cfg_ptz->lr_gpio_a;
	ptz.lr_gpio[1] = cfg_ptz->lr_gpio_b;
	ptz.lr_gpio[2] = cfg_ptz->lr_gpio_c;
	ptz.lr_gpio[3] = cfg_ptz->lr_gpio_d;

    ptz.lr_limit_gpio = cfg_ptz->lr_gpio_limit;
    ptz.l_limit_val = cfg_ptz->l_gpio_limit_valu;
    ptz.r_limit_val = cfg_ptz->r_gpio_limit_valu;

    ptz.max_vertical_step = cfg_ptz->max_vertical_step;
    ptz.max_horizontal_step = cfg_ptz->max_horizontal_step;
    ptz.horizontal_centre_step = cfg_ptz->horizontal_centre_step;
    ptz.vertical_centre_step = cfg_ptz->vertical_centre_step;
    if(runAudioCfg.rebootMute == 1)
    {
        ptz.reboot_ptz_disable = 1;
    }
    else
    {    
        ptz.reboot_ptz_disable = 1;
    }


    /* 用 ptz 结构做初始化 */
    sdk_ptz_init(&ptz);

    pthread_mutex_init(&g_ptz.ctrl.lock, NULL);
    pthread_mutex_init(&g_ptz.lock, NULL);

    // read and init preset.
    _ptz_preset_cfg_ctrl(0);

    g_ptz.running = 1;

    pthread_t ptz_th;
    if(pthread_create(&ptz_th, NULL, _ptz_thread, NULL))
    {
        LOG_ERR("create ptz thread error\n");
        return -1;
    }
    pthread_detach(ptz_th);

    netcam_ptz_start();
    return 0;
}

int netcam_ptz_exit(void)
{
    if (!g_ptz.running)
        return -1;

    // stop ptz thread.
    g_ptz.running = 0;
    g_ptz.ctrl_coming = 1;
    sdk_ptz_stop_loop();

    sdk_ptz_deinit();
    pthread_mutex_destroy(&g_ptz.ctrl.lock);
    pthread_mutex_destroy(&g_ptz.lock);
    return 0;
}

int netcam_ptz_set_preset(int order, char *name)
{
    if (!g_ptz.running)
        return -1;

    if (order >= NETCAM_PTZ_MAX_PRESET_NUM) {
        LOG_ERR("order error %d(0~%d)\n", order, NETCAM_PTZ_MAX_PRESET_NUM);
        return -1;
    }

    PTZ_LOCK();
    if (name != NULL) {
        memset(g_ptz.preset[order].name, 0, sizeof(g_ptz.preset[order].name));
		strncpy(g_ptz.preset[order].name, name, sizeof(g_ptz.preset[order].name));
	}
    g_ptz.preset[order].enable = 1;
    sdk_ptz_get_current_step(&g_ptz.preset[order].horizontal,
                                &g_ptz.preset[order].vertical);

    PTZ_UNLOCK();

    LOG_INFO("set preset point: %d (%d,%d)\n", order,
                g_ptz.preset[order].vertical, g_ptz.preset[order].horizontal);

    // save preset.
    _ptz_preset_cfg_ctrl(1);
    return 0;
}

int netcam_ptz_clr_preset(int order)
{
    if (!g_ptz.running)
        return -1;

    if (order >= NETCAM_PTZ_MAX_PRESET_NUM) {
        LOG_ERR("order error %d(0~%d)\n", order, NETCAM_PTZ_MAX_PRESET_NUM);
        return -1;
    }

    PTZ_LOCK();
    g_ptz.preset[order].enable = 0;
    PTZ_UNLOCK();

    // save preset.
    _ptz_preset_cfg_ctrl(1);
    return 0;
}

int netcam_ptz_start()
{
    _ptz_send_cmd(PTZ_CTRL_CMD_START, 0, 0);
    return 0;
}

int netcam_ptz_up(int step, int speed)
{
	if(!runImageCfg.flipEnabled)
    	_ptz_send_cmd(PTZ_CTRL_CMD_UP, step, speed);
	else
		_ptz_send_cmd(PTZ_CTRL_CMD_DOWN, step, speed);
    return 0;
}

int netcam_ptz_down(int step, int speed)
{
	if(!runImageCfg.flipEnabled)
    	_ptz_send_cmd(PTZ_CTRL_CMD_DOWN, step, speed);
	else
		_ptz_send_cmd(PTZ_CTRL_CMD_UP, step, speed);
    return 0;
}

int netcam_ptz_left(int step, int speed)
{
	if(!runImageCfg.mirrorEnabled)
    	_ptz_send_cmd(PTZ_CTRL_CMD_LEFT, step, speed);
	else
		_ptz_send_cmd(PTZ_CTRL_CMD_RIGHT, step, speed);
		
    return 0;
}

int netcam_ptz_right(int step, int speed)
{
	if(!runImageCfg.mirrorEnabled)
    	_ptz_send_cmd(PTZ_CTRL_CMD_RIGHT, step, speed);
	else
    	_ptz_send_cmd(PTZ_CTRL_CMD_LEFT, step, speed);
		
    return 0;
}

int netcam_ptz_left_up(int step, int speed)
{
	if(!runImageCfg.flipEnabled && !runImageCfg.mirrorEnabled)
    	_ptz_send_cmd(PTZ_CTRL_CMD_LEFT_UP, step, speed);
	else if(runImageCfg.flipEnabled && !runImageCfg.mirrorEnabled)
    	_ptz_send_cmd(PTZ_CTRL_CMD_LEFT_DOWN, step, speed);
	else if(!runImageCfg.flipEnabled && runImageCfg.mirrorEnabled)
    	_ptz_send_cmd(PTZ_CTRL_CMD_LEFT_DOWN, step, speed);
	else if(runImageCfg.flipEnabled && runImageCfg.mirrorEnabled)
    	_ptz_send_cmd(PTZ_CTRL_CMD_RIGHT_DOWN, step, speed);
	
    return 0;
}

int netcam_ptz_left_down(int step, int speed)
{
	if(!runImageCfg.flipEnabled && !runImageCfg.mirrorEnabled)
    	_ptz_send_cmd(PTZ_CTRL_CMD_LEFT_DOWN, step, speed);
	else if(runImageCfg.flipEnabled && !runImageCfg.mirrorEnabled)
    	_ptz_send_cmd(PTZ_CTRL_CMD_LEFT_UP, step, speed);
	else if(!runImageCfg.flipEnabled && runImageCfg.mirrorEnabled)
    	_ptz_send_cmd(PTZ_CTRL_CMD_RIGHT_DOWN, step, speed);
	else if(runImageCfg.flipEnabled && runImageCfg.mirrorEnabled)
    	_ptz_send_cmd(PTZ_CTRL_CMD_RIGHT_UP, step, speed);
	
    return 0;
}

int netcam_ptz_right_up(int step, int speed)
{
	if(!runImageCfg.flipEnabled && !runImageCfg.mirrorEnabled)
    	_ptz_send_cmd(PTZ_CTRL_CMD_RIGHT_UP, step, speed);
	else if(runImageCfg.flipEnabled && !runImageCfg.mirrorEnabled)
    	_ptz_send_cmd(PTZ_CTRL_CMD_RIGHT_DOWN, step, speed);
	else if(!runImageCfg.flipEnabled && runImageCfg.mirrorEnabled)
    	_ptz_send_cmd(PTZ_CTRL_CMD_LEFT_UP, step, speed);
	else if(runImageCfg.flipEnabled && runImageCfg.mirrorEnabled)
    	_ptz_send_cmd(PTZ_CTRL_CMD_LEFT_DOWN, step, speed);
	
    return 0;
}

int netcam_ptz_right_down(int step, int speed)
{
	if(!runImageCfg.flipEnabled && !runImageCfg.mirrorEnabled)
    	_ptz_send_cmd(PTZ_CTRL_CMD_RIGHT_DOWN, step, speed);
	else if(runImageCfg.flipEnabled && !runImageCfg.mirrorEnabled)
    	_ptz_send_cmd(PTZ_CTRL_CMD_RIGHT_UP, step, speed);
	else if(!runImageCfg.flipEnabled && runImageCfg.mirrorEnabled)
    	_ptz_send_cmd(PTZ_CTRL_CMD_LEFT_DOWN, step, speed);
	else if(runImageCfg.flipEnabled && runImageCfg.mirrorEnabled)
    	_ptz_send_cmd(PTZ_CTRL_CMD_LEFT_UP, step, speed);

    return 0;
}

int netcam_ptz_get_presets(ptz_preset_info_t *preset_info)
{
    if (!g_ptz.running){
        return -1;
    }

	if (preset_info == NULL) {
        LOG_ERR("Invalid parameters.");
        return -1;
	}

	int i = 0;
    memset(preset_info, 0, sizeof(ptz_preset_info_t));
	for (i = 0; i < NETCAM_PTZ_MAX_PRESET_NUM; i++) {
	    PTZ_LOCK();
		if (g_ptz.preset[i].enable) {
    		strncpy(preset_info->presetName[i], g_ptz.preset[i].name,
    		                                sizeof(g_ptz.preset[i].name));
			preset_info->presetNum[i] = i;
			preset_info->sizePreset++;
		}
		PTZ_UNLOCK();
	}
    return 0;
}

int netcam_ptz_horizontal_cruise(int speed)
{
    _ptz_send_cmd(PTZ_CTRL_CMD_HOR_CRUISE, 2, speed);
    return 0;
}

int netcam_ptz_vertical_cruise(int speed)
{
    _ptz_send_cmd(PTZ_CTRL_CMD_VER_CRUISE, 2, speed);
    return 0;
}

int netcam_ptz_hor_ver_cruise(int speed)
{
    _ptz_send_cmd(PTZ_CTRL_CMD_HOR_VER_CRUISE, 180, speed);
    return 0;
}

int netcam_ptz_preset_cruise(GK_NET_CRUISE_GROUP *cruise_info)
{
    int i;
    if (!g_ptz.running)
        return -1;

    if (cruise_info->byPointNum >= NETCAM_PTZ_MAX_PRESET_NUM) {
        LOG_ERR("order error %d(0~%d)\n", cruise_info->byPointNum, NETCAM_PTZ_MAX_PRESET_NUM);
        return -1;
    }

    ptz_preset_cruise_info_t preset_cruise[NETCAM_PTZ_MAX_PRESET_NUM];

    for (i = 0; i < cruise_info->byPointNum; i ++) {
        preset_cruise[i].preset_order = cruise_info->struCruisePoint[i].byPresetNo;
        preset_cruise[i].remain_time =  cruise_info->struCruisePoint[i].byRemainTime;
        preset_cruise[i].speed = cruise_info->struCruisePoint[i].bySpeed;
    	if (preset_cruise[i].speed == -1)
        	preset_cruise[i].speed = 4;
    }

    if (_ptz_preset_cruise(preset_cruise, cruise_info->byPointNum) < 0)
        return -1;
    else
        return 0;
}

int netcam_ptz_stop(void)
{
    //FIXME(heyong): if use step, the stop cmd should be invalid.
    if (g_ptz.ctrl.cmd == PTZ_CTRL_CMD_HOR_VER_CRUISE)// only cruise can be stop.
        _ptz_send_cmd(PTZ_CTRL_CMD_STOP, 0, 0);

    #ifdef MODULE_SUPPORT_AF
    sdk_isp_set_af_zoom(0);
    sdk_isp_set_af_focus(0);
    #endif
    
    return 0;
}

int netcam_ptz_ciruise_stop(void)
{
	g_ptz.ctrl.cmd = PTZ_CTRL_CMD_HOR_VER_CRUISE;
	return netcam_ptz_stop();
}

int netcam_ptz_is_moving(void)
{
    int moving;
    PTZ_CTRL_MSG_LOCK();
    moving = g_ptz.moving;
    PTZ_CTRL_MSG_UNLOCK();

    return moving;
}

#ifdef MODULE_SUPPORT_AF
int netcam_ptz_zoom_stop(void)
{
    sdk_isp_set_af_zoom(0);

	return 0;
}

int netcam_ptz_zoom_add(void)
{
	/* 变倍 +*/
	sdk_isp_set_af_zoom(1);
	return 0;
}

int netcam_ptz_zoom_sub(void)
{
	/* 变倍 -  */
	sdk_isp_set_af_zoom(2);
	return 0;
}

int netcam_ptz_focus_stop(void)
{
    sdk_isp_set_af_focus(0);

	return 0;
}

int netcam_ptz_focus_add(void)
{
	/* 变焦 +  */
	sdk_isp_set_af_focus(1);

	return 0;
}

int netcam_ptz_focus_sub(void)
{
	/* 变焦 -  */
	sdk_isp_set_af_focus(2);

	return 0;
}
#endif

