

#include <sys/sysinfo.h>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include "sdk_debug.h"
#include "sdk_gpio.h"


int sdk_gpio_open(sdk_gpio_handle_t *gpio, int pin)
{
    char dir_tmp[64] = {0};
    char cmd[64] = {0};
    int ret = 0;
    int export_fd = -1;

    if (pin < 0 || gpio == NULL)
        return -1;

    memset(gpio, 0, sizeof(sdk_gpio_handle_t));
    gpio->pin = pin;

    sprintf(dir_tmp, "/sys/class/gpio/gpio%d", gpio->pin);
    if(access(dir_tmp, F_OK) != 0 ) {//不存在，则创建
        export_fd = open("/sys/class/gpio/export", O_WRONLY);
        if (export_fd < 0) {
            LOG_ERR("open gpio export error\n");
            return -1;
        }
        sprintf(cmd, "%d", gpio->pin);
        ret = write(export_fd, cmd, strlen(cmd));
        if (ret < 0) {
            LOG_ERR("add gpio %d export error\n", gpio->pin);
            return -1;
        }
        close(export_fd);
    }

    sprintf(gpio->direction_path, "/sys/class/gpio/gpio%d/direction", gpio->pin);
    sprintf(gpio->value_path, "/sys/class/gpio/gpio%d/value", gpio->pin);

    gpio->direction_fd = -1;
    gpio->value_fd = -1;
    return 0;
}

int sdk_gpio_close(sdk_gpio_handle_t *gpio)
{
    gpio->pin = -1;
    gpio->direction_fd = -1;
    gpio->value_fd = -1;
    return 0;
}

int sdk_gpio_set_direction(sdk_gpio_handle_t *gpio, sdk_gpio_direction_t in_out)
{
    char *cmd;
    int ret = 0;

    if (gpio == NULL || gpio->pin < 0 || gpio->direction == in_out || gpio->direction_fd != -1)
        return -1;

    gpio->direction_fd = open(gpio->direction_path, O_WRONLY);
    if (gpio->direction_fd < 0) {
        LOG_ERR("open %s error\n", gpio->direction_path);
        return -1;
    }

    cmd = (in_out == SDK_GPIO_IN)?"in":"out";
    //LOG_INFO("set gpio %d direction to %s\n", gpio->pin, cmd);

    ret = write(gpio->direction_fd, cmd, strlen(cmd));
    if (ret < 0) {
        LOG_ERR("write %d direction error\n", gpio->pin);
        return -1;
    }

    close(gpio->direction_fd);
    gpio->direction_fd = -1;

    gpio->direction = in_out;
    return 0;
}

int sdk_gpio_set_value(sdk_gpio_handle_t *gpio, int value)
{
    int ret = 0;

    if (gpio == NULL || gpio->pin < 0)
        return -1;

    gpio->value_fd = open(gpio->value_path, O_WRONLY);
    if (gpio->value_fd < 0) {
        LOG_ERR("open %s error\n", gpio->value_path);
        return -1;
    }

    ret = write(gpio->value_fd, (value)?"1":"0", 1);
    if (ret < 0) {
        LOG_ERR("set %d value %d error\n", gpio->pin, value);
        return -1;
    }

    close(gpio->value_fd);
    return 0;
}

int sdk_gpio_get_value(sdk_gpio_handle_t *gpio, int *value)
{
    char cmd[64] = {0};

    if (gpio == NULL || gpio->pin < 0)
        return -1;

    gpio->value_fd = open(gpio->value_path, O_RDONLY);
    if (gpio->value_fd < 0) {
        LOG_ERR("open %s error\n", gpio->value_path);
        return -1;
    }

    int ret = read(gpio->value_fd, cmd, sizeof(cmd));
    if (ret < 0) {
        LOG_ERR("get %d value error\n", gpio->pin);
        return -1;
    }

    close(gpio->value_fd);

	*value = atoi(cmd);
    return 0;
}

int sdk_gpio_set_pull_up_down(sdk_gpio_handle_t *gpio, int updown)
{
    char *cmd;
    int ret = 0;

    if (gpio == NULL || gpio->pin < 0 || gpio->direction_fd != -1)
        return -1;

    gpio->direction_fd = open(gpio->direction_path, O_WRONLY);
    if (gpio->direction_fd < 0) {
        LOG_ERR("open %s error\n", gpio->direction_path);
        return -1;
    }

    cmd = (updown)?"in1":"in0";
    ret = write(gpio->direction_fd, cmd, strlen(cmd));
    if (ret < 0) {
        LOG_ERR("write %d direction error\n", gpio->pin);
        return -1;
    }

    close(gpio->direction_fd);
    gpio->direction_fd = -1;
    gpio->direction = SDK_GPIO_IN;
    return 0;
}



////////////////////////
// old
static int gk_gpio_set(char *dir, char *cmd, int cmd_size)
{
    if (dir == NULL || cmd == NULL) {
        LOG_ERR("param error\n");
        return -1;
    }

    int fd = open(dir, O_WRONLY);
    if (fd < 0) {
        LOG_ERR("open %s error\n", dir);
        return -1;
    }

    int ret = write(fd, cmd, cmd_size);
    if (ret < 0) {
        LOG_ERR("write %s to %s error\n", cmd, dir);
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

static int gk_gpio_get(char *dir,char *cmd, int cmd_size)
{

	if (dir == NULL || cmd == NULL) {
        LOG_ERR("param error\n");
        return -1;
    }

    int fd = open(dir, O_RDONLY);
    if (fd < 0) {
        LOG_ERR("open %s error\n", dir);
        return -1;
    }

    int ret = read(fd, cmd, sizeof(cmd_size));
    if (ret < 0) {
        LOG_ERR("read %s value error\n", dir);
        perror("read error");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}


static int gk_gpio_set_valu(int pin, int value)
{
    char dir[64] = {0};
    char dir_tmp[64] = {0};
    char cmd[64] = {0};
    int ret = 0;

    if (pin <= 0)
        return -1;
    
    sprintf(dir_tmp, "/sys/class/gpio/gpio%d", pin);
    if(access(dir_tmp, F_OK) != 0 ) {//不存在，则创建
        sprintf(dir, "/sys/class/gpio/export");
        sprintf(cmd, "%d", pin);
        ret = gk_gpio_set(dir, cmd, strlen(cmd));
        if (ret < 0) {
            LOG_ERR("sd_gpio_set %s %s error\n", dir, cmd);
            return -1;
        }
    }

    sprintf(dir, "/sys/class/gpio/gpio%d/direction", pin);
    sprintf(cmd, "%s", "out");
    ret = gk_gpio_set(dir, cmd, strlen(cmd));
    if (ret < 0) {
        LOG_ERR("sd_gpio_set %s %s error\n", dir, cmd);
        return -1;
    }

    sprintf(dir, "/sys/class/gpio/gpio%d/value", pin);
    sprintf(cmd, "%d", value);
    ret = gk_gpio_set(dir, cmd, strlen(cmd));
    if (ret < 0) {
        LOG_ERR("sd_gpio_set %s %s error\n", dir, cmd);
        return -1;
    }

    return 0;
}

static int gk_gpio_get_valu(int pin, int *value)
{
    char dir[64] = {0};
    char dir_tmp[64] = {0};
    char cmd[64] = {0};
    int ret = 0;

    if (pin <= 0)
        return -1;

    sprintf(dir_tmp, "/sys/class/gpio/gpio%d", pin);
    if(access(dir_tmp, F_OK) != 0 ) {//不存在，则创建
        sprintf(dir, "/sys/class/gpio/export");
        sprintf(cmd, "%d", pin);
        ret = gk_gpio_set(dir, cmd, strlen(cmd));
        if (ret < 0) {
            LOG_ERR(" %s %s error\n", dir, cmd);
            return -1;
        }
    }

    sprintf(dir, "/sys/class/gpio/gpio%d/direction", pin);
    sprintf(cmd, "%s", "in1");
    ret = gk_gpio_set(dir, cmd, strlen(cmd));
    if (ret < 0) {
        LOG_ERR(" %s %s error\n", dir, cmd);
        return -1;
    }

    sprintf(dir, "/sys/class/gpio/gpio%d/value", pin);
	memset(cmd,0,sizeof(cmd));
    ret = gk_gpio_get(dir, cmd, sizeof(cmd));
    if (ret < 0) {
        LOG_ERR(" %s error\n", dir);
        return -1;
    }
	*value = atoi(cmd);

    return 0;
}

static int gk_gpio_get_output_valu(int pin, int *value)
{
    char dir[64] = {0};
    char dir_tmp[64] = {0};
    char cmd[64] = {0};
    int ret = 0;

    if (pin <= 0)
        return -1;

    sprintf(dir_tmp, "/sys/class/gpio/gpio%d", pin);
    if(access(dir_tmp, F_OK) != 0 ) {//不存在，则创建
        sprintf(dir, "/sys/class/gpio/export");
        sprintf(cmd, "%d", pin);
        ret = gk_gpio_set(dir, cmd, strlen(cmd));
        if (ret < 0) {
            LOG_ERR(" %s %s error\n", dir, cmd);
            return -1;
        }
    }

    sprintf(dir, "/sys/class/gpio/gpio%d/value", pin);
	memset(cmd,0,sizeof(cmd));
    ret = gk_gpio_get(dir, cmd, sizeof(cmd));
    if (ret < 0) {
        LOG_ERR(" %s error\n", dir);
        return -1;
    }
	*value = atoi(cmd);

    return 0;
}


static int gk_gpio_set_input_pull_up_down(int pin, int updown)
{
    char dir[64] = {0};
    char dir_tmp[64] = {0};
    char cmd[64] = {0};
    int ret = 0;
    
    if (pin <= 0)
        return -1;

    sprintf(dir_tmp, "/sys/class/gpio/gpio%d", pin);
    if(access(dir_tmp, F_OK) != 0 ) {//不存在，则创建
        sprintf(dir, "/sys/class/gpio/export");
        sprintf(cmd, "%d", pin);
        ret = gk_gpio_set(dir, cmd, strlen(cmd));
        if (ret < 0) {
            LOG_ERR(" %s %s error\n", dir, cmd);
            return -1;
        }
    }

    sprintf(dir, "/sys/class/gpio/gpio%d/direction", pin);
    if(updown == 0)
    	snprintf(cmd, sizeof(cmd)-1, "in0");
	else
    	snprintf(cmd, sizeof(cmd)-1, "in1");

    ret = gk_gpio_set(dir, cmd, strlen(cmd));
    if (ret < 0) {
        LOG_ERR("sd_gpio_set %s %s error\n", dir, cmd);
        return -1;
    }

    return 0;
}

int sdk_gpio_set_output_value(int gpio, int value)
{
    int ret ;
    if (gpio <= 0)
    {
        return -1;
    }
    ret = gk_gpio_set_valu(gpio,value);
    if(ret != 0)
    {
        LOG_ERR("set value error\n");
        ret = -1;
    }

    return ret;
}

int sdk_gpio_get_intput_value(int gpio, int *value)
{
    int ret ;
    if (gpio <= 0)
    {
        return -1;
    }
    ret = gk_gpio_get_valu(gpio,value);
    if(ret != 0)
    {
        LOG_ERR("get value error\n");
        ret = -1;
    }

    return ret;

}

int sdk_gpio_get_output_value(int gpio, int *value)
{
    int ret ;
    if (gpio <= 0)
    {
        return -1;
    }
    ret = gk_gpio_get_output_valu(gpio,value);
    if(ret != 0)
    {
        LOG_ERR("get value error\n");
        ret = -1;
    }

    return ret;

}
int sdk_gpio_set_intput_default_value(int gpio, int value)
{

    int ret ;
    if (gpio <= 0)
    {
        return -1;
    }
    ret = gk_gpio_set_input_pull_up_down(gpio,value);
    if(ret != 0)
    {
        LOG_ERR("value error\n");
        ret = -1;
    }

    return ret;

}
