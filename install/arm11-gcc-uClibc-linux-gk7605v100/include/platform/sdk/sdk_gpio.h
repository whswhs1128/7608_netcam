#ifndef _SDK_GPIO_H_
#define _SDK_GPIO_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SDK_GPIO_IN = 0,
    SDK_GPIO_OUT,
} sdk_gpio_direction_t;

typedef struct sdk_gpio_handle_s {
    int pin;
    sdk_gpio_direction_t direction;
    char direction_path[64];
    char value_path[64];
    int direction_fd;
    int value_fd;
} sdk_gpio_handle_t;

int sdk_gpio_open(sdk_gpio_handle_t *gpio, int pin);
int sdk_gpio_close(sdk_gpio_handle_t *gpio);
int sdk_gpio_set_direction(sdk_gpio_handle_t *gpio, sdk_gpio_direction_t in_out);
int sdk_gpio_set_value(sdk_gpio_handle_t *gpio, int value);
int sdk_gpio_get_value(sdk_gpio_handle_t *gpio, int *value);
int sdk_gpio_set_pull_up_down(sdk_gpio_handle_t *gpio, int updown);


/*!
*******************************************************************************
** \brief set gpio output value
**
** \param[in]  gpio     the gpio number
**
** \param[in]  value    the out value of gpio, zone is low level, none-zore is high level
**
** \return
** - #SDK_OK   success
**   other     failed
**
**
*******************************************************************************
*/
int sdk_gpio_set_output_value(int gpio, int value);


/*!
*******************************************************************************
** \brief get gpio's input value
**
** \param[in]  gpio     the gpio number
**
** \param[out]  value    the input value of gpio, zone is low level, none-zore is high level
**
** \return
** - #SDK_OK   success
**   other     failed
**
**
*******************************************************************************
*/
int sdk_gpio_get_intput_value(int gpio, int *value);

/*!
*******************************************************************************
** \brief get gpio's output value
**
** \param[in]  gpio     the gpio number
**
** \param[out]  value    the output value of gpio, zone is low level, none-zore is high level
**
** \return
** - #SDK_OK   success
**   other     failed
**
**
*******************************************************************************
*/
int sdk_gpio_get_output_value(int gpio, int *value);


/*!
*******************************************************************************
** \brief set default input value of gpio
**
** \param[in]  gpio     the gpio number
**
** \param[in]  value    the default input value of gpio, zone is low level, none-zore is high
**
** \return
** - #SDK_OK   success
**   other     failed
**
**
*******************************************************************************
*/
int sdk_gpio_set_intput_default_value(int gpio, int value);



#ifdef __cplusplus
};
#endif
#endif

