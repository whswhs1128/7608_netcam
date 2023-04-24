/*!
*****************************************************************************
** \file    $gkprj
**
** \version	$id:
**
** \brief
**
** \attention   this code is provided as is. goke microelectronics
		accepts no responsibility or liability for any errors
		orommissions
** (c) copyright 2015 by Qiu qiwei co.,ltd
*****************************************************************************
*/

#include "gk7101.h"
#include "pthread.h"
#include "sdk_sys.h"
#include "sdk_reset.h"
#include "sdk_debug.h"

GADI_SYS_HandleT resetIOin  = NULL;
PS_SDK_RESET_API resetApi   = NULL;

int gk_reset_io_read_in(int *value)
{
	GADI_ERR ret = 0;
	GADI_S32 iovalue = 0;
	if(value == NULL)
	{
        LOG_ERR("bad parameter.");
        return -1;
	}
	ret = gadi_gpio_read_value(resetIOin, &iovalue);
	if(ret != GADI_OK)
	{
        LOG_ERR("fail to read GPIO.");
        return -1;
	}
	*value = iovalue;

	return 0;
}

int gk_reset_io_init(void)
{
	LOG_INFO("gk_reset_io_init\n");
    if(resetApi != NULL)
    {
        LOG_ERR("gk_reset_io_init has been called.");
        goto error1;
    }
    resetApi = (PS_SDK_RESET_API)gadi_sys_malloc(sizeof(ST_SDK_RESET_API));
    if(resetApi == NULL)
    {
        LOG_ERR("fail to malloc.");
        goto error1;
    }

	resetApi->reset_read_in = gk_reset_io_read_in;

	GADI_ERR error;
	GADI_GPIO_OpenParam reset;
	reset.active_low = 0;
	reset.direction = 0;
	reset.num_gpio = 6;
	reset.value = 0;
	resetIOin = gadi_gpio_open(&error, &reset);
	if(resetIOin == NULL)
	{
        LOG_ERR("fail to open GPIO(%d)", error);
		goto error2;
	}

	LOG_INFO("gk_io_init end\n");
	return 0;
error3:
	gadi_gpio_close(resetIOin);
error2:
	gadi_sys_free((void *)resetApi);
error1:
	return -1;
}


void gk_reset_io_exit(void)
{
	if(resetApi != NULL)
		gadi_sys_free((void *)resetApi);
	if(resetIOin != NULL)
		gadi_gpio_close(resetIOin);
}

