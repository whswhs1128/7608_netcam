#include <stdio.h>
#include <stdlib.h>


#ifndef SDK_CFG_H_
#define SDK_CFG_H_
#ifdef __cplusplus
extern "C" {
#endif

#define SDK_CFG_CJSON_FILE "sdk_cfg.cjson"
#define SDK_CFG_CJSON_PATCH_FILE "sdk_cfg_patch.cjson"

/* 使用光敏控制IRCUT的结构， 主要针对adc值越大和越小做了区分，
若光敏硬件上没有使用红外滤光片，则value_day，value_night，需要有一个整荡区间，防止红外打开后来回切换。
若使用了红外滤光片，则只需要确定白天的阀值就可以了*/
typedef struct IRCUT_ADC {
	int use_adc;  /*是否使用adc来控制IRCUT切换，0功能无效，1 adc加增益判断光敏强度，2 adc直接判断有效, */
	int normal;   /*adc控制的正常值，为1时adc值越大则越亮，为0时adc越小越亮*/
	int value_day;  /*白天时的临界值*/
	int value_night; /*夜晚时临界值*/
	int ch_num; /*ADC ch*/		//xqq
} IRCUT_ADC;

/*使用灯板读出黑夜和白天*/
typedef struct IRCUT_GPIO {
	int use_ircut_gpio; /*是否使用GPIO来控制IRCUT切换，0功能无效，1有效*/
	int ircut_gpio_num; /*GPIO 号，该值为0或-1时，该功能无效*/
	int ircut_day_value;  /*灯板输出为白天时的GPIO输入值*/
	int anode_gpio; /*IRCUT- GPIO 号，该值为-1时，该功能无效*/	//xqq
        int cathode_gpio; /*IRCUT+ GPIO 号，该值为-1时，该功能无效*/	//xqq

} IRCUT_GPIO;


/*恢复出厂设置的GPIO控制*/
typedef struct RESET {
	int gpio_reset;    /*恢复出厂设置的GPI0号，该值为0或-1时，该功能无效*/
	int gpio_reset_value; /*按键被按下时的值*/
} RESET;

/*PTZ控制相关参数，主要针对摇头机
GPI0号为0或-1时，该功能无效。
有的摇头机支持限位，有的不支持，该配置中lr_gpio_limit为0或-1，则表示不支持限位控制
*/
typedef struct CFG_PTZ {
    int enable;             /*0功能无效，1有效*/
    int clockwise;          /*0电机正向转动，1电机反向转动*/
    int max_vertical_step;  /*垂直转动时的最大步长，不同的电机和硬件需要调整*/
    int max_horizontal_step; /*水平转动时的最大步长，不同的电机和硬件需要调整*/
    int vertical_centre_step; /*垂直转动时的镜头在中间的步长，不同的电机和硬件需要调整*/
    int horizontal_centre_step; /*水平转动时的镜头在中间的步长，不同的电机和硬件需要调整*/

	int lr_gpio_a;  /*水平旋转的GPIO配置*/
	int lr_gpio_b;
	int lr_gpio_c;
	int lr_gpio_d;
	int lr_gpio_limit;      /*水平旋转的GPIO限位GPIO 号*/
    int l_gpio_limit_valu;  /*水平旋转时，电机在最左边时GPI0读出的值*/
    int r_gpio_limit_valu;  /*水平旋转时，电机在最右边时GPI0读出的值*/

	int ud_gpio_a;      /*垂直旋转的GPIO配置*/
	int ud_gpio_b;
	int ud_gpio_c;
	int ud_gpio_d;
	int ud_gpio_limit;      /*垂直旋转的GPIO限位GPIO 号*/
    int u_gpio_limit_valu;  /*垂直旋转时，电机在最上时GPI0读出的值*/
    int d_gpio_limit_valu;  /*垂直旋转时，电机在最下时GPI0读出的值*/
} CFG_PTZ_S;

/*应用控制中与硬件相关联的一些参数，使用json文件来定义，app和sdk层都会用到该配置
*/
/*
"languageType": 0,
"languageTypeProperty": {"type":"S32","mode":"rw","min":0,"max":18,"def":0,"opt":"0: chiness; 1 english 2 ..."},
*/
typedef struct sdk_cfg{
	char name[32];              /*硬件版本名*/
    int languageType;     /* 0: 中文; 1 英文 2 ... */
	IRCUT_ADC ircut_adc;        /*adc控制参数*/
	IRCUT_GPIO ircut_gpio;      /*gpio控制参数*/
	RESET reset;                /*恢复出厂参数*/
	CFG_PTZ_S ptz;              /*电机控制参数*/
	int mirror_revert;      /* sensor输入是否需要默认左右翻转*/
	int flip_revert;        /* sensor输入是否需要默认上下翻转*/
	int gpio_ircut_led;         /* IRCUT切换时，是否需要控制 LED切换，该值为0或-1时，该功能无效*/
	int gpio_ircut_led_on_value;    /* IRCUT切换时，LED 打开时GPIO的输出值*/
    int spk_en_gpio;         /* 是否需要控制 spk，该值为0或-1时，该功能无效*/
	int spk_en_gpio_value;    /* spk控制，spk 打开时GPIO的输出值*/
    int gpio_light_led;         /* 全彩模式切换时，是否需要控制 LED切换，该值为0或-1时，该功能无效*/
	int gpio_light_led_on_value;    /* 全彩模式切换时，LED 打开时GPIO的输出值*/
	//xqq
	int net_led;         /* 网口指示灯*/
        int net_led_on_value;    /* 指示灯打开输出值*/
    int gpio_red_led;         /* 设备状态切换时，是否需要控制红灯 LED切换，该值为0或-1时，该功能无效*/
        int gpio_red_led_on_value;    /* 红灯状态切换时，LED 打开时GPIO的输出值*/
    int gpio_color_led;         /* 设备状态切换时，是否需要控制彩灯 LED（绿/蓝）切换，该值为0或-1时，该功能无效*/
        int gpio_color_led_on_value;    /* 彩灯切换时，LED 打开时GPIO的输出值*/
        RESET reset2;	//xqq add lines
} SDK_CFG_S;

/*gloable variable,the value will be setted when application init */
extern SDK_CFG_S sdk_cfg;

int init_cfg_sdk();

#ifdef __cplusplus
};
#endif
#endif //SDK_CFG_H_

