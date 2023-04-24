#include "cfg_com.h"
#include "media_fifo.h"
#include "common.h"
#include "sdk_isp.h"
#include "sdk_pwm.h"

#define PWM_MODE_CLOSE 0
#define PWM_MODE_OPEN  1
#define PWM_MODE_AUTO  2

#define GAIN_CONTINUITY  5

pthread_rwlock_t autolight_lock;

static inline int autolight_enter_lock(void)
{
	return pthread_rwlock_wrlock(&autolight_lock);
}

static int autolight_leave_lock(void)
{
	return pthread_rwlock_unlock(&autolight_lock);
}

static unsigned char pwm_mode_last = 3;

static void _pwm_control(unsigned char pwm_mode, unsigned char pwm_duty)
{
	static unsigned char pwm_duty_last = 0;
    if (PWM_MODE_CLOSE == pwm_mode && pwm_mode_last != pwm_mode)
	{
		pwm_mode_last = pwm_mode;
		pwm_duty_last = 0;
        sdk_pwm_stop(0);
    }
	else if (PWM_MODE_OPEN == pwm_mode)
	{
		if(pwm_duty_last != pwm_duty)
		{
			if(pwm_duty >= 100)
				pwm_duty = 99;
			if(pwm_duty <= 1)
				pwm_duty = 1;
			pwm_duty_last = pwm_duty;
			sdk_set_pwm_duty(0, pwm_duty*10);
		}
		if(pwm_mode_last != pwm_mode)
		{
			pwm_mode_last = pwm_mode;
			sdk_pwm_start(0);
            sdk_set_pwm_speed(0, runpwmCfg.pwm_Freq);
		}
	}
}

static void _pwm_auto_control(unsigned char pwm_mode, unsigned char pwm_duty, unsigned char level)
{
	static unsigned char count=0, level_last=255, level_pwm_duty = 1, adj_pwm_duty=0, once=0,open_once=0;
	static unsigned int gain_last = 0, level_gain, Dvalue, open_close;
	int gain;
	if(PWM_MODE_CLOSE == pwm_mode)
	{
		_pwm_control(pwm_mode, pwm_duty); //runpwmCfg.pwm_duty);
	}
	else
	{
		if(level != level_last)
		{
			level_last     = level;
			level_pwm_duty = runpwmCfg.autolight[level].pwm_duty;
			level_gain     = runpwmCfg.autolight[level].gain;
			Dvalue         = runpwmCfg.autolight[level].Dvalue;
			PRINT_ERR("pwm_auto_control, level:%d, gain:%d, pwm_duty:%d%%, Dvalue:%d\n", 
				level, level_gain, level_pwm_duty, Dvalue);
		}
	    #if 1
		gain = sdk_isp_get_iso();
		//gain = sdk_isp_get_gain();
		if(count%GAIN_CONTINUITY == 0)
		{
			count = 1;
			gain_last = gain;
		}
		else
		{
			if(gain_last == gain)
				count++;
			else
				count = 0;
		}
		
		if(count==GAIN_CONTINUITY)
		{
			if(PWM_MODE_OPEN == pwm_mode)
			{
				if(gain > level_gain)
				{
					adj_pwm_duty = 99;
					open_once= 1;
					open_close = level_gain;
					_pwm_control(PWM_MODE_OPEN, adj_pwm_duty);				
					PRINT_ERR("PWM OPEN iso:%d, level:%d, gain:%d, pwm_duty:%d%%, adj:%d%%, Dvalue:%d\n", 
						gain, level, level_gain, level_pwm_duty, adj_pwm_duty, open_close);
				}
				else if(gain < open_close)
				{
					if(1==open_once)
					{					
						open_close = gain;
						if(open_close > 100)
							open_close -= 20; 
						open_once = 0;
					}
					else
					{
						_pwm_control(PWM_MODE_CLOSE, adj_pwm_duty);
						open_once = 0;
						open_close = 0;
					}
					PRINT_ERR("PWM CLOSE:%d, level:%d, gain:%d, pwm_duty:%d%%, adj:%d%%, Dvalue:%d\n", 
						gain, level, level_gain, level_pwm_duty, adj_pwm_duty, open_close);
				}
			}
			else if(PWM_MODE_AUTO == pwm_mode)
			{
				if(gain > level_gain)
				{
					if(once==0)
					{
						adj_pwm_duty = 99*level_gain/1582;
						once = 1;
					}
					else			
						adj_pwm_duty = 99*gain/1582;
					_pwm_control(PWM_MODE_OPEN, adj_pwm_duty);				
					PRINT_ERR("PWM OPEN iso:%d, level:%d, gain:%d, pwm_duty:%d%%, adj:%d%%, Dvalue:%d\n", 
						gain, level, level_gain, level_pwm_duty, adj_pwm_duty, Dvalue);
				}
				else if(gain < Dvalue)
				{
					if(adj_pwm_duty<2)
					{
						_pwm_control(PWM_MODE_CLOSE, adj_pwm_duty);
						once = 0;
					}
					else
					{
						adj_pwm_duty = adj_pwm_duty/2;
						_pwm_control(PWM_MODE_OPEN, adj_pwm_duty);
					}
					PRINT_ERR("PWM CLOSE:%d, level:%d, gain:%d, pwm_duty:%d%%, adj:%d%%, Dvalue:%d\n", 
						gain, level, level_gain, level_pwm_duty, adj_pwm_duty, Dvalue);
				}
			}
			#if 0
			if(gain > level_gain)
			{
				if(PWM_MODE_OPEN == pwm_mode)
				{
					adj_pwm_duty = 99;
					open_once= 1;
				}
				else
				{
					if(once==0)
					{
						adj_pwm_duty = 99*level_gain/1582;
						once = 1;
					}
					else			
						adj_pwm_duty = 99*gain/1582;
				}
				_pwm_control(PWM_MODE_OPEN, adj_pwm_duty);				
				PRINT_ERR("PWM OPEN iso:%d, level:%d, gain:%d, pwm_duty:%d%%, adj:%d%%, Dvalue:%d\n", 
					gain, level, level_gain, level_pwm_duty, adj_pwm_duty, Dvalue);
			}
			else if(gain < Dvalue)
			{
				if(PWM_MODE_OPEN == pwm_mode)
				{
					if(1==open_once)
					{					
						Dvalue = gain;						
						open_once = 0;
					}
					else
					{
					_pwm_control(PWM_MODE_CLOSE, adj_pwm_duty);
					Dvalue         = runpwmCfg.autolight[level].Dvalue;
					open_once = 0;
					}
				}
				else
				{
					if(adj_pwm_duty<2)
					{
					_pwm_control(PWM_MODE_CLOSE, adj_pwm_duty);
					once = 0;
					}
					else
					{
					adj_pwm_duty = adj_pwm_duty/2;
					_pwm_control(PWM_MODE_OPEN, adj_pwm_duty);
					}
				}
				PRINT_ERR("PWM CLOSE:%d, level:%d, gain:%d, pwm_duty:%d%%, adj:%d%%, Dvalue:%d\n", 
					gain, level, level_gain, level_pwm_duty, adj_pwm_duty, Dvalue);
			}
			#endif
			PRINT_ERR("sdk_isp_get_iso:%d, level:%d, gain:%d, pwm_duty:%d%%, adj:%d%%, Dvalue:%d\n", 
				gain, level, level_gain, level_pwm_duty, adj_pwm_duty, Dvalue);
		}
		#else
		else if(count==3 && PWM_MODE_OPEN==pwm_mode_last)
		{
			adj_pwm_duty = 99*gain/1582;
			_pwm_control(PWM_MODE_OPEN, adj_pwm_duty);
			PRINT_ERR("sdk_isp_get_gain:%d, level:%d, gain:%d, pwm_duty:%d%%, adj:%d%%, Dvalue:%d\n", 
				gain, level, level_gain, level_pwm_duty, adj_pwm_duty, Dvalue);
		}
		
		sdk_isp_get_statistics_enable(1);
		if(level_pwm_duty>=100)
			level_pwm_duty = 1;
		_pwm_control(PWM_MODE_OPEN, level_pwm_duty);
		unsigned int cfa_y = sdk_isp_get_statistics();
		gain = sdk_isp_get_gain();
		PRINT_ERR("pwm,duty:%d%%, cfa_y:%d, gain:%d\n", 
			level_pwm_duty, cfa_y,gain);
		level_pwm_duty++;
		sleep(9);
		#if 0
		if(level_pwm_duty>=100)
			level_pwm_duty = 1;
		_pwm_control(PWM_MODE_OPEN, level_pwm_duty);
		unsigned short y_result = sdk_isp_get_y_result();
		gain = sdk_isp_get_gain();
		PRINT_ERR("pwm,duty:%d%%, y_result:%d, gain:%d\n", 
			level_pwm_duty, y_result,gain);
		level_pwm_duty++;
		sleep(9);
		#endif
		#endif
	}
}


static void * autolight_iso_pwm(void *param)
{
    sdk_sys_thread_set_name("autolight_iso_pwm");
	PRINT_ERR("pwm,Freq:%d, mode:%d, duty:%d, level:%d\n", 
		runpwmCfg.pwm_Freq, runpwmCfg.pwm_mode, runpwmCfg.pwm_duty, runpwmCfg.level);
	sdk_pwm_init();
	while (1) 
	{
		_pwm_auto_control(runpwmCfg.pwm_mode, runpwmCfg.pwm_duty, runpwmCfg.level);
		usleep(500*1000);
    }
    return NULL;
}

void netcam_autolight_init(void)
{
    #ifdef MODULE_SUPPORT_PWM_IRCUT_LED
    int i;
	sdk_pwm_init();
    for (i = 0; i < MAX_PWM_CHANNEL; i++)
    {
        if (runpwmCfg.autolight[i].Dvalue)
        {
            sdk_pwm_start(i);
            sdk_set_pwm_speed(i, runpwmCfg.pwm_Freq);
            //:TODO defaut close. need to check the close value
            sdk_set_pwm_duty(i, 1);
            sdk_isp_pwm_set_on_value(i, runpwmCfg.autolight[i].pwm_duty);
        }
    }
    #else
    pthread_rwlock_init(&autolight_lock, NULL);
	CreateDetachThread(autolight_iso_pwm, NULL, NULL);
    #endif
}

int netcam_autolight_get(GK_AUTO_LIGHT_CFG *autolightAttr)
{
    autolight_enter_lock();
    memcpy(autolightAttr,&runpwmCfg,sizeof(GK_AUTO_LIGHT_CFG));
    autolight_leave_lock();
    return 0;
}

int netcam_autolight_set(GK_AUTO_LIGHT_CFG autolightAttr)
{
    autolight_enter_lock();
    memcpy(&runpwmCfg,&autolightAttr,sizeof(GK_AUTO_LIGHT_CFG));
    #if MODULE_SUPPORT_PWM_IRCUT_LED
    int i;
    
    for (i = 0; i < MAX_PWM_CHANNEL; i++)
    {
        if (runpwmCfg.autolight[i].Dvalue)
        {
            sdk_isp_pwm_set_on_value(i, runpwmCfg.autolight[i].pwm_duty);
        }
    }
    #endif
    autolight_leave_lock();
    return 0;
}

void netcam_autolight_cfg_save(void)
{
	if(0 == autolight_enter_lock())
    {
		PRINT_INFO("save cfg!!");
		pwmCfgSave();
		autolight_leave_lock();
	}
}

