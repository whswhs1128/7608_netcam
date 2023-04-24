#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include "k24c02_eeprom.h"

#define K24C02_EEPROM_I2C_DEV_NODE "/dev/i2c-1"
#define K24C02_MAX_DATA_LEN_PER_MSG	 1
#define K24C02_DELAY_US_PER_MSG	 	 1500

int k24c02_eeprom_i2c_read(unsigned char device_addr, unsigned char sub_addr, unsigned char *buff, int len)
{
    int fd = -1, ret = 0;
    unsigned char buftmp[8];
    struct i2c_rdwr_ioctl_data i2c_data;

    device_addr >>= 1;
    fd = open(K24C02_EEPROM_I2C_DEV_NODE, O_RDWR);
    if (fd<0)
    {
        printf("open %s failed!\r\n",K24C02_EEPROM_I2C_DEV_NODE);
        return -1;
    }

    i2c_data.nmsgs = 2;
    i2c_data.msgs = (struct i2c_msg *)malloc(i2c_data.nmsgs *sizeof(struct i2c_msg));
    if (i2c_data.msgs == NULL)
    {
        printf("eeprom_i2c_read malloc error");
        ret = -1;
		goto exit;
    }

    ioctl(fd, I2C_TIMEOUT, 1);
    ioctl(fd, I2C_RETRIES, 2);

    //write reg
    memset(buftmp, 0, sizeof(buftmp));
    buftmp[0] = sub_addr;
    i2c_data.msgs[0].len = 1;
    i2c_data.msgs[0].addr = device_addr;
    i2c_data.msgs[0].flags = 0;     // 0: write 1:read
    i2c_data.msgs[0].buf = buftmp;
    //read data
    i2c_data.msgs[1].len = len;
    i2c_data.msgs[1].addr = device_addr;
    i2c_data.msgs[1].flags = 1;     // 0: write 1:read
    i2c_data.msgs[1].buf = buff;

    ret = ioctl(fd, I2C_RDWR, (unsigned long)&i2c_data);
    if (ret < 0)
    {
        printf("read data %x %x error\r\n", device_addr, sub_addr);
        ret = -1;
		goto exit;
    }

exit:

	if(i2c_data.msgs)
    	free(i2c_data.msgs);

	if(fd > 0)
    	close(fd);

    return ret;
}

int k24c02_eeprom_i2c_write(unsigned char device_addr, unsigned char sub_addr, unsigned char *buff, int len)
{

    int fd = -1, ret = 0,i = 0;
	unsigned char buf[K24C02_MAX_DATA_LEN_PER_MSG + 1] = {0};
    struct i2c_rdwr_ioctl_data i2c_data;
	int trans_cnt = 0,remain_len = 0;

	if((sub_addr + len) > K24C02_EEPROM_MAX_LEN)
		len = (K24C02_EEPROM_MAX_LEN - sub_addr);

	trans_cnt = len/K24C02_MAX_DATA_LEN_PER_MSG;
	remain_len = len%K24C02_MAX_DATA_LEN_PER_MSG;

    device_addr >>= 1;

    fd = open(K24C02_EEPROM_I2C_DEV_NODE, O_RDWR);
    if (fd < 0)
    {
        printf("open %s failed!\r\n",K24C02_EEPROM_I2C_DEV_NODE);
        return -1;
    }

    i2c_data.nmsgs = 1;
    i2c_data.msgs = (struct i2c_msg *)malloc(i2c_data.nmsgs *sizeof(struct i2c_msg));
    if (i2c_data.msgs == NULL)
    {
        printf("malloc i2c_msg faied!\n");
        ret = -1;
		goto exit;
    }

    ioctl(fd, I2C_TIMEOUT, 1);
    ioctl(fd, I2C_RETRIES, 2);

	for(i=0; i<trans_cnt; i++)
	{
		buf[0] = sub_addr+i*K24C02_MAX_DATA_LEN_PER_MSG;

	    memcpy(&buf[1], &buff[i*K24C02_MAX_DATA_LEN_PER_MSG], K24C02_MAX_DATA_LEN_PER_MSG);

	    i2c_data.msgs[0].len = K24C02_MAX_DATA_LEN_PER_MSG + 1;;
	    i2c_data.msgs[0].addr = device_addr;
	    i2c_data.msgs[0].flags = 0;     // 0: write 1:read
	    i2c_data.msgs[0].buf = buf;
	    ret = ioctl(fd, I2C_RDWR, (unsigned long)&i2c_data);
	    if (ret < 0)
	    {
	        printf("write reg %x %x error\r\n", device_addr, sub_addr);
	        ret = -1;
			goto exit;
	    }
		usleep(K24C02_DELAY_US_PER_MSG);
	}

	if(remain_len > 0)
	{
		memset(buf,0,sizeof(buf));
		buf[0] = sub_addr+trans_cnt*K24C02_MAX_DATA_LEN_PER_MSG;

		memcpy(&buf[1], &buff[trans_cnt*K24C02_MAX_DATA_LEN_PER_MSG], remain_len);

		i2c_data.msgs[0].len = remain_len + 1;;
		i2c_data.msgs[0].addr = device_addr;
		i2c_data.msgs[0].flags = 0;     // 0: write 1:read
		i2c_data.msgs[0].buf = buf;
		ret = ioctl(fd, I2C_RDWR, (unsigned long)&i2c_data);
		if (ret < 0)
		{
			printf("write reg %x %x error\r\n", device_addr, sub_addr);
			ret = -1;
			goto exit;
		}
	}

exit:

	if(i2c_data.msgs)
    	free(i2c_data.msgs);

	if(fd > 0)
    	close(fd);

    return ret;
}

int k24c02_eeprom_data_check(void)
{
	#define MAGIC_DATA	"GOKE-GK720X-IPCAM-FIRMARE"

	int ret = 0,i = 0;
	unsigned char buf[K24C02_EEPROM_MAX_LEN];

#if 0
	memset(buf,0,sizeof(buf));
	ret = k24c02_eeprom_i2c_write(K24C02_EEPROM_DEVICE_ADDR,0,MAGIC_DATA,strlen(MAGIC_DATA));
    printf("k24c02_eeprom_i2c_write ret = %d\n", ret);
#endif


	memset(buf,0,sizeof(buf));

	ret = k24c02_eeprom_i2c_read(K24C02_EEPROM_DEVICE_ADDR,0,buf,K24C02_EEPROM_MAX_LEN);
	if(ret < 0)
	{
		printf("\nk24c02_eeprom_i2c_read failed!\n");
		return 1;
	}

    return 0;

	for(i=0; i<(K24C02_EEPROM_MAX_LEN-strlen(MAGIC_DATA)); i++)
	{
		if(!memcmp(&buf[i],MAGIC_DATA,strlen(MAGIC_DATA)))
			break;
	}

	if(i >= (K24C02_EEPROM_MAX_LEN-strlen(MAGIC_DATA)))
	{
		printf("\n#############k24c02_eeprom_data_check failed!\n");
		ret = 1;

	}
	else
	{
		printf("\n############k24c02_eeprom_data_check OK!\n");
		ret = 0;
	}

	return ret;
}


