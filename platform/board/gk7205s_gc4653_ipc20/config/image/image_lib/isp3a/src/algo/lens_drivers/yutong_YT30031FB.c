/*!
*****************************************************************************
** \file        src/algo/lens_drivers/Foctek_D14_02812IR.c
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

#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/i2c-dev.h>
#include <assert.h>
#include "time.h"
#include "isp3a_lens.h"




//*****************************************************************************
//*****************************************************************************
//** Local Defines
//*****************************************************************************
//*****************************************************************************
#ifndef ABS
#define ABS(x) ({                        \
        int __x = (x);                    \
        (__x < 0) ? -__x : __x;            \
            })
#endif

#define I2C_M_PIN_MUXING    (0x8000)

//*****************************************************************************
//*****************************************************************************
//** Local structures
/*
static int zoom_gpio_id[4] = {37,38,39,40};
static int force_gpio_id[4] =  {24,27,28,36};
*/
static int fd = 0;

//*****************************************************************************
//*****************************************************************************

//*****************************************************************************
//*****************************************************************************
//** Global Data
//*****************************************************************************
//*****************************************************************************

//*****************************************************************************
//*****************************************************************************
//** Local Data
//*****************************************************************************
//*****************************************************************************

static int focuse_runing = 0;
static int zoom_runing = 0;

static int zoom_dir  = 0;
static int focus_dir  = 0;

static bool bstandby = 1;
static bool  zoom_pulse_reverse = 0;
static bool  focus_pulse_reverse = 0;

extern s32 g_focus_reverse_err;
extern s32 g_zoom_reverse_err;

//** Local structures
typedef enum {
	PTZ_CMD_xxx=4,
	PTZ_CMD_LEFT,
	PTZ_CMD_RIGHT,
	PTZ_CMD_UP,
	PTZ_CMD_DOWN,
	PTZ_CMD_SET_SPEED,
	PTZ_CMD_GET_SPEED,
	PTZ_ZOOM_CHECK,
	PTZ_FOCUS_CHECK,
	PTZ_CMD_STANDBY,
} PTZ_CMD;

//*****************************************************************************
//*****************************************************************************
//** Local Functions Declaration
//*****************************************************************************
//*****************************************************************************
/*
static void lens_gpio_set(u8 gpio_id);
static void gpio_clr(u8 gpio_id);
static int lens_gpio_get(int gpio_id, int *level);
static int    lens_gpio_config(int gpio_id, char* config);
*/
static int zoom_in(int pps,int distance);
static int zoom_out(int pps,int distance);
static int af_focus_f(int pps,int distance);
static int af_focus_n(int pps,int distance);
//static int _ic_lens_init(int zoom_gpio[4],int force_gpio[4]);
static int _ic_lens_init();

static int checking_focuse_runing();
static int checking_zoom_runing();

static int af_lens_standby(u8 en);


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

static int checking_focuse_runing()
{
    PTZ_CMD cmd;
	
	char focus[1]={'0'};
	char status[1];

	write(fd,focus,1);
	read(fd,status,1);

	if(status[0]  == '1')
	{
		focuse_runing = 1;
	}else
	{
		focuse_runing = 0;
	}
	
	//printf("focuse_runing = %d\n",focuse_runing);
	return focuse_runing;
}

static int checking_zoom_runing()
{
	PTZ_CMD cmd;
	char zoom[1] = {'1'};
	char status[1];
	
	write(fd,zoom,1);
	read(fd,status,1);

	if(status[0]  == '1')
	{
		zoom_runing = 1;
	}else
	{
		zoom_runing = 0;
	}
	
	//printf("zoom_runing = %d\n",zoom_runing);
	return zoom_runing;
}


 static int zoom_out(int pps,int distance)
{
	int sleep_time = 1000/pps;
	PTZ_CMD cmd;
	int ret;
	
    #if 1
    if (zoom_dir == 1)
    {
         if(zoom_pulse_reverse == 0)
         {
            zoom_pulse_reverse = 1;
         }
         else
         {
            distance += g_zoom_reverse_err;
         }
        //distance += 70;
    }
    zoom_dir = -1;
    #endif

    //printf("zoom_out distance is %d\n", distance);
	bstandby = 1;
	
	cmd = PTZ_CMD_SET_SPEED;
	ret = ioctl(fd, cmd, &sleep_time);
	
	cmd = PTZ_CMD_UP;
	ret = ioctl(fd, cmd, &distance);
	
	return 1;
}

static int zoom_in(int pps,int distance)
{
	int sleep_time = 1000/pps;
	PTZ_CMD cmd;
	int ret;
	
    #if 1
	if (zoom_dir == -1)
    {
       if(zoom_pulse_reverse == 0)
       {
            zoom_pulse_reverse = 1;
       }
       else
       {
            distance += g_zoom_reverse_err;
       }
       //distance += 70;
    }
    zoom_dir = 1;
	#endif

	bstandby = 1;
    
	cmd = PTZ_CMD_SET_SPEED;
	ret = ioctl(fd, cmd, &sleep_time);
	
	cmd = PTZ_CMD_DOWN;
	ret = ioctl(fd, cmd, &distance);
	//printf("zoom_in distance is %d\n", distance);

	return 1;
}

static int af_focus_f(int pps,int distance)
{
	int sleep_time = 1000/pps;
	PTZ_CMD cmd;
	int ret;
	
	#if 1
    if (focus_dir == 1)
    {
       if(focus_pulse_reverse == 0)
       {
          focus_pulse_reverse = 1;
       }
       else
       {
           distance += g_focus_reverse_err;
       }
        //distance += 90;//16
    }
    focus_dir = -1;
	#endif

	bstandby = 1;
    
	//printf("_af_focus_f distance is %d\n", distance);
	cmd = PTZ_CMD_SET_SPEED;
	ret = ioctl(fd, cmd, &sleep_time);
	cmd = PTZ_CMD_RIGHT;
	ret = ioctl(fd, cmd, &distance);

	return 1;
}


static int af_focus_n(int pps,int distance)
{
	int sleep_time = 1000/pps;
	PTZ_CMD cmd;
	int ret;

    #if 1
    if (focus_dir == -1)
    {
       if(focus_pulse_reverse == 0)
       {
          focus_pulse_reverse = 1;
       }
       else
       {
           distance += g_focus_reverse_err;
       }
       //distance += 90;//16
    }
    focus_dir = 1;
    #endif

	bstandby = 1;
    
	cmd = PTZ_CMD_SET_SPEED;
	ret = ioctl(fd, cmd, &sleep_time);
	
	cmd = PTZ_CMD_LEFT;
	ret = ioctl(fd, cmd, &distance);
    
	//printf("_af_focus_n distance is %d\n",j);
	return 1;
}


//#define DEFARG(name,defval) ((#name[0])?(name+0):(defval))
static int _ic_lens_init(/*int zoom_gpio[4],int force_gpio[4]*/)
{
    /*
	char export_path[128] = "/sys/class/gpio/export";
	char gpio_path[128];
	char buf[128];
	char *out = "out";
	int export_fd,gpio_fd;
	int i;
	
	for(i=0;i<4;i++)
	{
		zoom_gpio_id[i] = zoom_gpio[i];
		force_gpio_id[i] = force_gpio[i];
	}

	for(i=0;i<4;i++)
	{
		export_fd = open(export_path, O_WRONLY);
		sprintf(buf,"%d",zoom_gpio_id[i]);
		write(export_fd,buf,sizeof(buf));
		close(export_fd);

		sprintf(gpio_path, "/sys/class/gpio/gpio%d/direction", zoom_gpio_id[i]);
		gpio_fd = open(gpio_path, O_WRONLY);
		write(gpio_fd,out,sizeof(out));
		close(gpio_fd);	
	}

	for(i=0;i<4;i++)
	{
		export_fd = open(export_path, O_WRONLY);
		sprintf(buf,"%d",force_gpio_id[i]);
		write(export_fd,buf,sizeof(buf));
		close(export_fd);

		sprintf(gpio_path, "/sys/class/gpio/gpio%d/direction", force_gpio_id[i]);
		gpio_fd = open(gpio_path, O_WRONLY);
		write(gpio_fd,out,sizeof(out));
		close(gpio_fd);
	}
	*/

	fd = open("/dev/ptz", O_RDWR);
	if(fd < 0) {
		perror("open failed");
		return -1;
	}
	
   // zoom_out(300,2300);
	//af_focus_n(300,2300);
    zoom_out(300,2700);
    af_focus_n(300,3700);
    zoom_pulse_reverse = 0;
    focus_pulse_reverse = 0;
	return 1;
}

static int af_lens_standby(u8 en)
{

	PTZ_CMD cmd;
	int ret;
    int	x;
	if(bstandby == 0)
		return 1;
	cmd = PTZ_CMD_STANDBY;
	ret = ioctl(fd, cmd, &x);
	bstandby = 0;
    return 1;
}

static int ic_lens_init()
{
    //return _ic_lens_init(zoom_gpio_id,force_gpio_id);
    return _ic_lens_init();
} 

/*
int main()
{
	int zoom_test[4] = {1,2,3,4};
	int force_test[4] = {5,6,7,8};
	ic_lens_init(zoom_test,force_test);
    ic_lens_init_1();
	int i=0;
	#if 0
	if( checking_zoom_runing() == 1)
		{
			printf("zoom_in fail\n");
		}else
		{
			zoom_in(200,500);
		}
	if( checking_zoom_runing() == 1)
		{
			printf("zoom_out fail\n");
		}else
		{
			zoom_out(200,500);
		}
	if( checking_focuse_runing() == 1)
		{
			printf("af_focus_f fail\n");
		}else
		{
			af_focus_f(200,500);
		}
	if( checking_focuse_runing() == 1)
		{
			printf("af_focus_n fail\n");
		}else
		{
			af_focus_n(200,500);
		}


	sleep(60);

		if( checking_zoom_runing() == 1)
		{
			printf("zoom_in fail\n");
		}else
		{
			zoom_in(200,500);
			printf("okokokok\n");
		}

	#endif
	while(1);
}
*/

lens_dev_drv_t YuTong_YT30031FB_dev_drv = {
    .ic_lens_init = ic_lens_init,
    .af_focus_n = af_focus_n,
    .af_focus_f = af_focus_f,
    .zoom_in = zoom_in,
    .zoom_out = zoom_out,
    .check_isFocusRuning = checking_focuse_runing,
    .check_isZoomRuning = checking_zoom_runing,
    .af_lens_standby = af_lens_standby
};

