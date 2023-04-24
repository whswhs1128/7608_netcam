#include <string.h>
#include <unistd.h>
#include "sdk_cfg.h"
#include "cJSON.h"
#include "sdk_debug.h"

#include "cfg_common.h"

extern int copy_file(char *src_name, char *des_name);

SDK_CFG_S sdk_cfg;

static int cfg_sdk_load(char *filename, SDK_CFG_S *sdkcfg)
{
	FILE *f;
	long len;
	char *data, *pname = NULL;
	cJSON* Object;
	cJSON* ArrayObject;
	cJSON* Item;
	int ArraySize=0,i=0;
    int ret = 0;

	if(access(filename, F_OK) < 0)
    {
        LOG_ERR("not have %s\n", filename);
        return -1;
    }
	f=fopen(filename,"rb");
	fseek(f,0,SEEK_END);
	len=ftell(f);
	fseek(f,0,SEEK_SET);
	data=(char*)malloc(len+1);
	fread(data,1,len,f);
	fclose(f);
	// get string from json
	Object = cJSON_Parse(data);
	free(data);
    if(!Object){
        ret = -1;
        goto err;
    }
    pname = cJSON_GetObjectItem(Object,"CUSTOM_BOAD")->valuestring;
    if(pname)
        strcpy(sdkcfg->name, pname);
    else{
        ret = -1;
        goto err;
    }
	ArrayObject = cJSON_GetObjectItem(Object,"BOARD");
	ArraySize = cJSON_GetArraySize(ArrayObject);
	for(i=0;i<ArraySize;i++)
	{
		Item = cJSON_GetArrayItem(ArrayObject,i);
		if(strcmp(sdkcfg->name, cJSON_GetObjectItem(Item,"NAME")->valuestring) == 0)
		{
		    cJSON* LANGUAGE_Item = cJSON_GetObjectItem(Item,"LANGUAGE");
			if (LANGUAGE_Item != NULL) {
                sdkcfg->languageType = LANGUAGE_Item->valueint;
			} else
			    sdkcfg->languageType = -1;// invalid.

			cJSON* IRCUT_ADC_Item = cJSON_GetObjectItem(Item,"IRCUT_ADC");
			cJSON* IRCUT_GPIO_Item = cJSON_GetObjectItem(Item,"IRCUT_GPIO");
			cJSON* RESET_Item = cJSON_GetObjectItem(Item,"RESET");
			cJSON* RESET_Item2 = cJSON_GetObjectItem(Item,"RESET2");
			cJSON* PTZ_Item = cJSON_GetObjectItem(Item,"PTZ");

			if(IRCUT_ADC_Item!=NULL)
			{
				sdkcfg->ircut_adc.use_adc = cJSON_GetObjectItem(IRCUT_ADC_Item,"USE_ADC")->valueint;
				sdkcfg->ircut_adc.normal = cJSON_GetObjectItem(IRCUT_ADC_Item,"ADC_NORMAL")->valueint;
				sdkcfg->ircut_adc.value_day = cJSON_GetObjectItem(IRCUT_ADC_Item,"ADC_VALUE_DAY")->valueint;
				sdkcfg->ircut_adc.value_night = cJSON_GetObjectItem(IRCUT_ADC_Item,"ADC_VALUE_NIGHT")->valueint;
                cJSON* obj = cJSON_GetObjectItem(IRCUT_ADC_Item,"CH_NUM");
                sdkcfg->ircut_adc.ch_num = (obj!=NULL)?obj->valueint:0;
			}

			if(IRCUT_GPIO_Item!=NULL)
			{
				sdkcfg->ircut_gpio.use_ircut_gpio = cJSON_GetObjectItem(IRCUT_GPIO_Item,"USE_IRCUT_GPIO")->valueint;
				sdkcfg->ircut_gpio.ircut_gpio_num = cJSON_GetObjectItem(IRCUT_GPIO_Item,"IRCUT_GPIO_NUM")->valueint;
				sdkcfg->ircut_gpio.ircut_day_value = cJSON_GetObjectItem(IRCUT_GPIO_Item,"GPIO_IRCUT_DAY_VALUE")->valueint;
                cJSON* obj = cJSON_GetObjectItem(IRCUT_GPIO_Item,"ANODE_GPIO");
                sdkcfg->ircut_gpio.anode_gpio = (obj!=NULL)?obj->valueint:-1;
                obj = cJSON_GetObjectItem(IRCUT_GPIO_Item,"CATHODE_GPIO");
				sdkcfg->ircut_gpio.cathode_gpio = (obj!=NULL)?obj->valueint:-1;
			}

			if(RESET_Item!=NULL)
			{
				sdkcfg->reset.gpio_reset = cJSON_GetObjectItem(RESET_Item,"GPIO_RESET")->valueint;
				sdkcfg->reset.gpio_reset_value = cJSON_GetObjectItem(RESET_Item,"GPIO_RESET_VALUE")->valueint;
			}
            
			if(RESET_Item2!=NULL)
			{
				sdkcfg->reset2.gpio_reset = cJSON_GetObjectItem(RESET_Item2,"GPIO_RESET")->valueint;
				sdkcfg->reset2.gpio_reset_value = cJSON_GetObjectItem(RESET_Item2,"GPIO_RESET_VALUE")->valueint;
			}
            else
            {
                sdkcfg->reset2.gpio_reset = -1;
                sdkcfg->reset2.gpio_reset_value = -1;
            }
            
			if(PTZ_Item!=NULL)
			{
				sdkcfg->ptz.enable = cJSON_GetObjectItem(PTZ_Item,"PTZ_ENABLE")->valueint;
				sdkcfg->ptz.clockwise = cJSON_GetObjectItem(PTZ_Item,"PTZ_CLOCKWISE")->valueint;
				sdkcfg->ptz.max_vertical_step = cJSON_GetObjectItem(PTZ_Item,"PTZ_VERTICAL_STEP")->valueint;
				sdkcfg->ptz.max_horizontal_step = cJSON_GetObjectItem(PTZ_Item,"PTZ_HORIZONTAL_STEP")->valueint;
				sdkcfg->ptz.vertical_centre_step = cJSON_GetObjectItem(PTZ_Item,"PTZ_VERTICAL_CENTRE_STEP")->valueint;
				sdkcfg->ptz.horizontal_centre_step = cJSON_GetObjectItem(PTZ_Item,"PTZ_HORIZONTAL_CENTRE_STEP")->valueint;

				sdkcfg->ptz.lr_gpio_a = cJSON_GetObjectItem(PTZ_Item,"PTZ_LR_GPIO_A")->valueint;
				sdkcfg->ptz.lr_gpio_b = cJSON_GetObjectItem(PTZ_Item,"PTZ_LR_GPIO_B")->valueint;
				sdkcfg->ptz.lr_gpio_c = cJSON_GetObjectItem(PTZ_Item,"PTZ_LR_GPIO_C")->valueint;
				sdkcfg->ptz.lr_gpio_d = cJSON_GetObjectItem(PTZ_Item,"PTZ_LR_GPIO_D")->valueint;
				sdkcfg->ptz.lr_gpio_limit = cJSON_GetObjectItem(PTZ_Item,"PTZ_LR_GPIO_R")->valueint;
				sdkcfg->ptz.l_gpio_limit_valu = cJSON_GetObjectItem(PTZ_Item,"PTZ_L_GPIO_LIMIT_VALUE")->valueint;
				sdkcfg->ptz.r_gpio_limit_valu = cJSON_GetObjectItem(PTZ_Item,"PTZ_R_GPIO_LIMIT_VALUE")->valueint;

				sdkcfg->ptz.ud_gpio_a = cJSON_GetObjectItem(PTZ_Item,"PTZ_UD_GPIO_A")->valueint;
				sdkcfg->ptz.ud_gpio_b = cJSON_GetObjectItem(PTZ_Item,"PTZ_UD_GPIO_B")->valueint;
				sdkcfg->ptz.ud_gpio_c = cJSON_GetObjectItem(PTZ_Item,"PTZ_UD_GPIO_C")->valueint;
				sdkcfg->ptz.ud_gpio_d = cJSON_GetObjectItem(PTZ_Item,"PTZ_UD_GPIO_D")->valueint;
				sdkcfg->ptz.ud_gpio_limit = cJSON_GetObjectItem(PTZ_Item,"PTZ_UD_GPIO_R")->valueint;
				sdkcfg->ptz.u_gpio_limit_valu = cJSON_GetObjectItem(PTZ_Item,"PTZ_U_GPIO_LIMIT_VALUE")->valueint;
				sdkcfg->ptz.d_gpio_limit_valu = cJSON_GetObjectItem(PTZ_Item,"PTZ_D_GPIO_LIMIT_VALUE")->valueint;
			}

			sdkcfg->mirror_revert = cJSON_GetObjectItem(Item,"MIRROR_REVERT")->valueint;
			sdkcfg->flip_revert = cJSON_GetObjectItem(Item,"FLIP_REVERT")->valueint;
			sdkcfg->gpio_ircut_led = cJSON_GetObjectItem(Item,"GPIO_IRCUT_LED")->valueint;
			sdkcfg->gpio_ircut_led_on_value = cJSON_GetObjectItem(Item,"GPIO_IRCUT_LED_ON_VALUE")->valueint;

			cJSON* tmpItem = NULL;

			tmpItem = cJSON_GetObjectItem(Item,"GPIO_LIGHT_LED");
            sdkcfg->gpio_light_led = (tmpItem!=NULL)?tmpItem->valueint:-1;

			tmpItem = cJSON_GetObjectItem(Item,"GPIO_LIGHT_LED_ON_VALUE");
            sdkcfg->gpio_light_led_on_value = (tmpItem!=NULL)?tmpItem->valueint:-1;
			
            tmpItem = cJSON_GetObjectItem(Item,"SPK_EN_GPIO");
            sdkcfg->spk_en_gpio = (tmpItem!=NULL)?tmpItem->valueint:-1;

            tmpItem = cJSON_GetObjectItem(Item,"SPK_EN_GPIO_VALUE");
            sdkcfg->spk_en_gpio_value = (tmpItem!=NULL)?tmpItem->valueint:-1;

            tmpItem = cJSON_GetObjectItem(Item,"NET_LED_EN_GPIO");
            sdkcfg->net_led = (tmpItem!=NULL)?tmpItem->valueint:-1;

			tmpItem = cJSON_GetObjectItem(Item,"NET_LED_EN_GPIO_VALUE");            
            sdkcfg->net_led_on_value = (tmpItem!=NULL)?tmpItem->valueint:-1;
            
            tmpItem = cJSON_GetObjectItem(Item,"GPIO_RED_LED");
            sdkcfg->gpio_red_led = (tmpItem!=NULL)?tmpItem->valueint:-1;

            tmpItem = cJSON_GetObjectItem(Item,"GPIO_RED_LED_ON_VALUE");
            sdkcfg->gpio_red_led_on_value = (tmpItem!=NULL)?tmpItem->valueint:-1;

            tmpItem = cJSON_GetObjectItem(Item,"GPIO_COLOR_LED");
            sdkcfg->gpio_color_led = (tmpItem!=NULL)?tmpItem->valueint:-1;
			
            tmpItem = cJSON_GetObjectItem(Item,"GPIO_COLOR_LED_ON_VALUE");
            sdkcfg->gpio_color_led_on_value = (tmpItem!=NULL)?tmpItem->valueint:-1;

			break;
		}else{
            LOG_ERR("CUSTOM_BOAD:%s, CONFIG NAME:%s", sdkcfg->name, cJSON_GetObjectItem(Item,"NAME")->valuestring);
            ret = -1;
        }
	}
err:
	cJSON_Delete(Object);
    return ret;
}



int init_cfg_sdk()
{
	int ret=-1;
    char sdk_cfg_patch_path[128] = {0};
    char sdk_cfg_default_path[128] = {0};
    char sdk_cfg_user_path[128] = {0};
    SDK_CFG_S sdk_cfg_patch;

    // 第一次启动，拷贝默认配置
    snprintf(sdk_cfg_user_path,sizeof(sdk_cfg_user_path), "%s%s", CFG_DIR, SDK_CFG_CJSON_FILE);
	if (access(sdk_cfg_user_path, F_OK) != 0) {
	    LOG_WARNING("first boot, use default sdk_cfg file\n");
        snprintf(sdk_cfg_default_path,sizeof(sdk_cfg_default_path), "%s%s", DEFCFG_DIR, SDK_CFG_CJSON_FILE);
	    copy_file(sdk_cfg_default_path, sdk_cfg_user_path);
    }

    // 读取用户SDK_CFG
    ret = cfg_sdk_load(sdk_cfg_user_path, &sdk_cfg);
    if (ret < 0) {
        LOG_ERR("parse default sdk_cfg error\n");
        return -1;
    }

    // 判断SDK_CFG_PATH是否存在
    snprintf(sdk_cfg_patch_path,sizeof(sdk_cfg_patch_path), "%s%s", CUSTOM_DIR, SDK_CFG_CJSON_PATCH_FILE);
	if (access(sdk_cfg_patch_path, F_OK) == 0) {
        LOG_INFO("find and parse sdk_cfg_patch file.\n");
        ret = cfg_sdk_load(sdk_cfg_patch_path, &sdk_cfg_patch);
        if (ret < 0) {//SDK_CFG_PATH存在，但读取失败
            LOG_WARNING("parse sdk_cfg_patch error, use default.\n");
        } else {//SDK_CFG_PATH存在，且读取成功
            if (strcmp(sdk_cfg.name, sdk_cfg_patch.name) != 0) {
                LOG_WARNING("sdk_cfg_patch: not match! (%s != %s)\n", sdk_cfg_patch.name, sdk_cfg.name);
            } else {
                memcpy(&sdk_cfg, &sdk_cfg_patch, sizeof(sdk_cfg));
                LOG_INFO("sdk_cfg_patch ok, use it.\n");
            }
        }
    } // else SDK_CFG_PATH不存在，使用默认值

    #if 0
	printf("sdk_cfg.name:%s\n", sdk_cfg.name);
	printf("LANGUAGE:%d\n",sdk_cfg.languageType);
	printf("IRCUT_ADC:%d,%d,%d,%d\n",sdk_cfg.ircut_adc.use_adc,sdk_cfg.ircut_adc.normal,sdk_cfg.ircut_adc.value_day,sdk_cfg.ircut_adc.value_night);
	printf("IRCUT_GPIO:%d,%d,%d\n",sdk_cfg.ircut_gpio.use_ircut_gpio,sdk_cfg.ircut_gpio.ircut_gpio_num,sdk_cfg.ircut_gpio.ircut_day_value);
	printf("RESET:%d,%d\n",sdk_cfg.reset.gpio_reset,sdk_cfg.reset.gpio_reset_value);
    printf("PTZ enable:%d, %d, %d, %d, %d, %d\n", sdk_cfg.ptz.enable, sdk_cfg.ptz.clockwise, sdk_cfg.ptz.max_vertical_step, sdk_cfg.ptz.max_horizontal_step, sdk_cfg.ptz.vertical_centre_step, sdk_cfg.ptz.horizontal_centre_step);
	printf("PTZLR:%d,%d,%d,%d,%d,%d, %d\n",sdk_cfg.ptz.lr_gpio_a,sdk_cfg.ptz.lr_gpio_b,sdk_cfg.ptz.lr_gpio_c,sdk_cfg.ptz.lr_gpio_d,sdk_cfg.ptz.lr_gpio_limit,
            sdk_cfg.ptz.l_gpio_limit_valu, sdk_cfg.ptz.r_gpio_limit_valu);
	printf("PTZUD:%d,%d,%d,%d,%d, %d, %d\n",sdk_cfg.ptz.ud_gpio_a,sdk_cfg.ptz.ud_gpio_b,sdk_cfg.ptz.ud_gpio_c,sdk_cfg.ptz.ud_gpio_d,sdk_cfg.ptz.ud_gpio_limit,
            sdk_cfg.ptz.u_gpio_limit_valu, sdk_cfg.ptz.d_gpio_limit_valu);
	printf("MIRROR_REVERT:%d\n",sdk_cfg.mirror_revert);
	printf("FLIP_REVERT:%d\n",sdk_cfg.flip_revert);
	printf("GPIO_IRCUT_LED:%d\n",sdk_cfg.gpio_ircut_led);
	printf("GPIO_IRCUT_LED_ON_VALUE:%d\n",sdk_cfg.gpio_ircut_led_on_value);
    printf("SPK_EN_GPIO:%d, EN_VALUE:%d\n",sdk_cfg.spk_en_gpio, sdk_cfg.spk_en_gpio_value);
    #endif
	return 0;
}
