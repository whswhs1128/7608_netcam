#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "plugsdk.h"

static void * _plug_start(void *param)
{
	if(0!=init_plug_sdk())
	{
		printf("InitPlugSdk init error\n");
		destroy_plug_sdk();
		return NULL;
	}
    printf("mofang_plug_start end !\n");
    return NULL;
}

int mofang_plug_start(char * sn, int len)
{
    pthread_t pid;
    pthread_attr_t attr;
    set_dev_sn(sn, len);

    pthread_attr_init(&attr);  
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED); 
    if(0 != pthread_create(&pid, &attr, _plug_start, NULL))
    {
        printf("erro:creat _plug_start fail\n");
        return -1;
    }
		
	return 0;
}

int mofang_plug_exit()
{
	destroy_plug_sdk();
	return 0;
}

