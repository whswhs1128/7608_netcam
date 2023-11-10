#ifndef _SDK_PWM_H_
#define _SDK_PWM_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_PWM_CHANNEL   5

int sdk_set_pwm_speed(int channel, unsigned int speed);
unsigned int sdk_get_pwm_speed(int channel);
int sdk_set_pwm_duty(int channel, unsigned int duty);
unsigned int sdk_get_pwm_duty(int channel);
int sdk_pwm_init();
int sdk_pwm_exit();
int sdk_pwm_start(int channel);
int sdk_pwm_stop(int channel);
int sdk_pwm_get_channel_status(int channel);

#ifdef __cplusplus
    }
#endif

#endif /* _SDK_PWM_H_ */
