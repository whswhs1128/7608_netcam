#ifndef _GK_CFG_PWM_H_
#define _GK_CFG_PWM_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"
#define ISO_AL_MAX_NUMBER   5
#define ISO_AL_MAX_ITEM_NUM 5

/***********************************/
/***         pwm              ***/
/***********************************/

typedef struct {
	SDK_U8  level;
	SDK_U8  pwm_duty; /* 1 ~ 99 */
    SDK_U32 gain; /* ISO threshold */
    SDK_U32 Dvalue; /* ISO threshold Dvalue*/
} GK_AUTO_LIGHT, *LPGK_AUTO_LIGHT;

typedef struct {
	SDK_U8  pwm_mode; /* 补光模式 0 - 关闭 1 - 常开, 2 - 自动 */
	SDK_U8  pwm_duty; /* 1 ~ 99 */
	SDK_U8  level;
    SDK_U32 pwm_Freq; /* HZ */
    GK_AUTO_LIGHT autolight[ISO_AL_MAX_NUMBER];
} GK_AUTO_LIGHT_CFG, *LPGK_AUTO_LIGHT_CFG;

int pwmCfgSave();
int pwmCfgLoad();
void pwmCfgPrint();
int pwmCfgLoadDefValue();
char *pwmCfgGetJosnString(void);

extern GK_AUTO_LIGHT_CFG runpwmCfg;

#ifdef __cplusplus
}
#endif
#endif

