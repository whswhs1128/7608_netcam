
#ifndef __GK_SDK_ISP_DEF_H__
#define __GK_SDK_ISP_DEF_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ISP scene mode*/
#define ISP_SCENE_MODE_AUTO (0)
#define ISP_SCENE_MODE_INDOOR (1)
#define ISP_SCENE_MODE_OUTDOOR (2)

/* ISP ircut control mode*/
#define ISP_IRCUT_CONTROL_MODE_HARDWARE (0)
#define ISP_IRCUT_CONTROL_MODE_SOFTWARE (1)

/* ISP ircut status */

#define ISP_IRCUT_MODE_AUTO (0)
#define ISP_IRCUT_MODE_DAYLIGHT (1)
#define ISP_IRCUT_MODE_NIGHT (2)
#define ISP_IRCUT_MODE_AUTOCOLOR (3)

/* ISP digital shutter */

#define ISP_VIN_DIGITAL_SHUTTER_50HZ (1)
#define ISP_VIN_DIGITAL_SHUTTER_60HZ (2)

#define ISP_LEN_AF_PARAM_NUM (34)

typedef enum {
    /** ir-mode */
    ISP_BULB_MODE_IR,
    /** all-color-mode */
    ISP_BULB_MODE_ALL_COLOR,
    /** mix-mode */
    ISP_BULB_MODE_MIX,
    /** manual-mode */
    ISP_BULB_MODE_MANUAL,
}ISP_BULB_ModeEnumT;

typedef struct  {
    int32_t offset[ISP_LEN_AF_PARAM_NUM];
    uint16_t length;
    uint16_t enable;
}ST_GK_AF_OFFSET_PARAM, *PST_GK_AF_OFFSET_PARAM;

/* af focus callback */
typedef void (*GK_ISP_AF_FOCUS_CALLBACK)(int32_t *pLensParams, uint16_t length);

/*!
*******************************************************************************
** \brief isp init
**
** \return
** - #0             success
** - #other         failed
**
*******************************************************************************
*/

int gk_isp_init();


/*!
*******************************************************************************
** \brief isp destroy
**
** \return
** - #0             success
** - #other         failed
**
*******************************************************************************
*/
int gk_isp_destroy();

/*!
*******************************************************************************
** \brief Get sensor type
**
** \return
** - #EM_SENSOR_MODEL
**
*******************************************************************************
*/

//EM_SENSOR_MODEL sdk_isp_get_sensor_model(void);

/*!
*******************************************************************************
** \brief Set isp flip
**
** \param[in] mode: 1,enable flip, 0,disable flip.
**
** \return
**
*******************************************************************************
*/
void sdk_isp_set_flip(uint8_t mode);

/*!
*******************************************************************************
** \brief Set isp mirror
**
** \param[in] mode: 1,enable mirror, 0,disable mirror.
**
** \return
**
*******************************************************************************
*/
void sdk_isp_set_mirror(uint8_t mode);

/*!
*******************************************************************************
** \brief Set ImageDefParam
**
** \param[in] : 0 - 100
**
** \return
**
*******************************************************************************
*/
void sdk_isp_set_ImageDefParam(uint8_t saturation, uint8_t brightness, uint8_t hue, uint8_t contrast, uint8_t sharpen);

/*!
*******************************************************************************
** \brief Set hue
**
** \param[in] hue: 0 - 100
**
** \return
**
*******************************************************************************
*/
void sdk_isp_set_hue(uint8_t hue);


/*!
*******************************************************************************
** \brief Set saturation
**
** \param[in] saturation: 0 - 100
**
** \return
**
*******************************************************************************
*/
void sdk_isp_set_saturation(uint8_t saturation);


/*!
*******************************************************************************
** \brief Set brightness
**
** \param[in] brightness: 0 - 100
**
** \return
**
*******************************************************************************
*/
void sdk_isp_set_brightness(uint8_t brightness);

/*!
*******************************************************************************
** \brief Set contrast
**
** \param[in] contrast: 0 - 100
**
** \return
**
*******************************************************************************
*/
void sdk_isp_set_contrast(uint8_t  contrast);

void sdk_isp_set_fps(int fps);

/*!
*******************************************************************************
** \brief Set color mode
**
** \param[in] color_mode: 0 standard, 1 more brigntness, 2 more saturation
**
** \return
**
*******************************************************************************
*/
void sdk_isp_set_color_mode(uint8_t color_mode);


/*!
*******************************************************************************
** \brief Set flicker frequency
**
** \param[in] frequency:  50:50HZ, 60: 60HZ
**
** \return
**
*******************************************************************************
*/
void sdk_isp_set_vi_flicker(uint8_t frequency);

/*!
*******************************************************************************
** \brief Set sharpeness
**
** \param[in] val:      0 - 100
** \param[in] manual:   1,manual, 0,auto
**
** \return
**
*******************************************************************************
*/
void sdk_isp_set_sharpen(uint8_t val, uint8_t manual);

/*!
*******************************************************************************
** \brief Set warp
**
** \param[in] h_enable: 1,enable, 0,disable
** \param[in] h_val:    0 - 100
** \param[in] v_enable: 1,enable, 0,disable
** \param[in] v_val:    0 - 100
**
** \return
**
*******************************************************************************
*/
void sdk_isp_set_warp(uint8_t h_enable, uint8_t h_val, uint8_t v_enable, uint8_t v_val);

/*!
*******************************************************************************
** \brief Set scene mode
**
** \param[in] mode:     0 auto, 1 indoor, 2 outdoor
**
** \return
**
*******************************************************************************
*/
void sdk_isp_set_scene_mode(uint8_t mode);

/*!
*******************************************************************************
** \brief Set wb mode
**
** \param[in] mode:     0 auto 1 indoor 2 outdoor
**
** \return
**
*******************************************************************************
*/
void sdk_isp_set_wb_mode(uint8_t wb_mode);

/*!
*******************************************************************************
** \brief Set ircut control mode
**
** \param[in] mode:     0 software  ,  1 hardware
**
** \return
**
*******************************************************************************
*/
void sdk_isp_set_ircut_control_mode(uint8_t control_mode);

/*!
*******************************************************************************
** \brief ircut control check each intervalms, this function will auto check brightness
**        and control ircut mode.
**
** \return
**
*******************************************************************************
*/
void sdk_isp_ircut_auto_switch(void);

/*!
*******************************************************************************
** \brief set ircut mode
**
** \param[in] mode: 0 auto, 1 day, 2 night
**
** \return
**
*******************************************************************************
*/
void sdk_isp_set_ircut_mode(uint8_t mode);


/*!
*******************************************************************************
** \brief set low light mode
**
** \param[in] mode: 0 close, 1 only night, 2 day-night, 3 auto
**
** \return
**
*******************************************************************************
*/
void sdk_isp_set_lowlight_mode(uint8_t mode);
int sdk_isp_change_timeout(void);
int sdk_isp_changed(void);
void sdk_ircu_detect(void);
int sdk_isp_get_iso();

void sdk_isp_set_af_zoom(uint8_t mode);
void sdk_isp_set_af_focus(uint8_t mode);

void sdk_isp_set_bulb_mode(ISP_BULB_ModeEnumT mode);
ISP_BULB_ModeEnumT sdk_isp_get_bulb_mode(void);
void sdk_isp_set_bulb_level(uint8_t level);
uint8_t sdk_isp_get_bulb_level(void);
int gk_isp_led_on(void);
int gk_isp_led_off(void);
int gk_isp_get_led_state(void);
void gk_isp_get_af_offset(uint8_t mode, GK_ISP_AF_FOCUS_CALLBACK getParam);
void gk_isp_set_af_offset(PST_GK_AF_OFFSET_PARAM param);
int sdk_isp_ircut_led_set(int isOn);
int sdk_isp_ircut_led_get_status(void);
int sdk_isp_light_led_set(int isOn);
int sdk_isp_light_led_get_status(void);
int sdk_isp_led_test_start(void);
int sdk_isp_pwm_set_on_value(int channel, int value);
int sdk_isp_get_cur_mode(void);

#ifdef __cplusplus
};
#endif

#endif //__GK_SDK_ISP_DEF_H__

