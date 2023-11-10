#include <stdio.h>
#include <stdlib.h>


#ifndef SDK_CFG_H_
#define SDK_CFG_H_
#ifdef __cplusplus
extern "C" {
#endif

#define SDK_CFG_CJSON_FILE "sdk_cfg.cjson"
#define SDK_CFG_CJSON_PATCH_FILE "sdk_cfg_patch.cjson"

/* ʹ�ù�������IRCUT�Ľṹ�� ��Ҫ���adcֵԽ���ԽС�������֣�
������Ӳ����û��ʹ�ú����˹�Ƭ����value_day��value_night����Ҫ��һ���������䣬��ֹ����򿪺������л���
��ʹ���˺����˹�Ƭ����ֻ��Ҫȷ������ķ�ֵ�Ϳ�����*/
typedef struct IRCUT_ADC {
	int use_adc;  /*�Ƿ�ʹ��adc������IRCUT�л���0������Ч��1 adc�������жϹ���ǿ�ȣ�2 adcֱ���ж���Ч, */
	int normal;   /*adc���Ƶ�����ֵ��Ϊ1ʱadcֵԽ����Խ����Ϊ0ʱadcԽСԽ��*/
	int value_day;  /*����ʱ���ٽ�ֵ*/
	int value_night; /*ҹ��ʱ�ٽ�ֵ*/
	int ch_num; /*ADC ch*/		//xqq
} IRCUT_ADC;

/*ʹ�õư������ҹ�Ͱ���*/
typedef struct IRCUT_GPIO {
	int use_ircut_gpio; /*�Ƿ�ʹ��GPIO������IRCUT�л���0������Ч��1��Ч*/
	int ircut_gpio_num; /*GPIO �ţ���ֵΪ0��-1ʱ���ù�����Ч*/
	int ircut_day_value;  /*�ư����Ϊ����ʱ��GPIO����ֵ*/
	int anode_gpio; /*IRCUT- GPIO �ţ���ֵΪ-1ʱ���ù�����Ч*/	//xqq
        int cathode_gpio; /*IRCUT+ GPIO �ţ���ֵΪ-1ʱ���ù�����Ч*/	//xqq

} IRCUT_GPIO;


/*�ָ��������õ�GPIO����*/
typedef struct RESET {
	int gpio_reset;    /*�ָ��������õ�GPI0�ţ���ֵΪ0��-1ʱ���ù�����Ч*/
	int gpio_reset_value; /*����������ʱ��ֵ*/
} RESET;

/*PTZ������ز�������Ҫ���ҡͷ��
GPI0��Ϊ0��-1ʱ���ù�����Ч��
�е�ҡͷ��֧����λ���еĲ�֧�֣���������lr_gpio_limitΪ0��-1�����ʾ��֧����λ����
*/
typedef struct CFG_PTZ {
    int enable;             /*0������Ч��1��Ч*/
    int clockwise;          /*0�������ת����1�������ת��*/
    int max_vertical_step;  /*��ֱת��ʱ����󲽳�����ͬ�ĵ����Ӳ����Ҫ����*/
    int max_horizontal_step; /*ˮƽת��ʱ����󲽳�����ͬ�ĵ����Ӳ����Ҫ����*/
    int vertical_centre_step; /*��ֱת��ʱ�ľ�ͷ���м�Ĳ�������ͬ�ĵ����Ӳ����Ҫ����*/
    int horizontal_centre_step; /*ˮƽת��ʱ�ľ�ͷ���м�Ĳ�������ͬ�ĵ����Ӳ����Ҫ����*/

	int lr_gpio_a;  /*ˮƽ��ת��GPIO����*/
	int lr_gpio_b;
	int lr_gpio_c;
	int lr_gpio_d;
	int lr_gpio_limit;      /*ˮƽ��ת��GPIO��λGPIO ��*/
    int l_gpio_limit_valu;  /*ˮƽ��תʱ������������ʱGPI0������ֵ*/
    int r_gpio_limit_valu;  /*ˮƽ��תʱ����������ұ�ʱGPI0������ֵ*/

	int ud_gpio_a;      /*��ֱ��ת��GPIO����*/
	int ud_gpio_b;
	int ud_gpio_c;
	int ud_gpio_d;
	int ud_gpio_limit;      /*��ֱ��ת��GPIO��λGPIO ��*/
    int u_gpio_limit_valu;  /*��ֱ��תʱ�����������ʱGPI0������ֵ*/
    int d_gpio_limit_valu;  /*��ֱ��תʱ�����������ʱGPI0������ֵ*/
} CFG_PTZ_S;

/*Ӧ�ÿ�������Ӳ���������һЩ������ʹ��json�ļ������壬app��sdk�㶼���õ�������
*/
/*
"languageType": 0,
"languageTypeProperty": {"type":"S32","mode":"rw","min":0,"max":18,"def":0,"opt":"0: chiness; 1 english 2 ..."},
*/
typedef struct sdk_cfg{
	char name[32];              /*Ӳ���汾��*/
    int languageType;     /* 0: ����; 1 Ӣ�� 2 ... */
	IRCUT_ADC ircut_adc;        /*adc���Ʋ���*/
	IRCUT_GPIO ircut_gpio;      /*gpio���Ʋ���*/
	RESET reset;                /*�ָ���������*/
	CFG_PTZ_S ptz;              /*������Ʋ���*/
	int mirror_revert;      /* sensor�����Ƿ���ҪĬ�����ҷ�ת*/
	int flip_revert;        /* sensor�����Ƿ���ҪĬ�����·�ת*/
	int gpio_ircut_led;         /* IRCUT�л�ʱ���Ƿ���Ҫ���� LED�л�����ֵΪ0��-1ʱ���ù�����Ч*/
	int gpio_ircut_led_on_value;    /* IRCUT�л�ʱ��LED ��ʱGPIO�����ֵ*/
    int spk_en_gpio;         /* �Ƿ���Ҫ���� spk����ֵΪ0��-1ʱ���ù�����Ч*/
	int spk_en_gpio_value;    /* spk���ƣ�spk ��ʱGPIO�����ֵ*/
    int gpio_light_led;         /* ȫ��ģʽ�л�ʱ���Ƿ���Ҫ���� LED�л�����ֵΪ0��-1ʱ���ù�����Ч*/
	int gpio_light_led_on_value;    /* ȫ��ģʽ�л�ʱ��LED ��ʱGPIO�����ֵ*/
	//xqq
	int net_led;         /* ����ָʾ��*/
        int net_led_on_value;    /* ָʾ�ƴ����ֵ*/
    int gpio_red_led;         /* �豸״̬�л�ʱ���Ƿ���Ҫ���ƺ�� LED�л�����ֵΪ0��-1ʱ���ù�����Ч*/
        int gpio_red_led_on_value;    /* ���״̬�л�ʱ��LED ��ʱGPIO�����ֵ*/
    int gpio_color_led;         /* �豸״̬�л�ʱ���Ƿ���Ҫ���Ʋʵ� LED����/�����л�����ֵΪ0��-1ʱ���ù�����Ч*/
        int gpio_color_led_on_value;    /* �ʵ��л�ʱ��LED ��ʱGPIO�����ֵ*/
        RESET reset2;	//xqq add lines
} SDK_CFG_S;

/*gloable variable,the value will be setted when application init */
extern SDK_CFG_S sdk_cfg;

int init_cfg_sdk();

#ifdef __cplusplus
};
#endif
#endif //SDK_CFG_H_

