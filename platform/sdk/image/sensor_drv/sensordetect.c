#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include "adi_types.h"
#include "adi_sys.h"
#include "mksensor.h"
#include <getopt.h>
#include <string.h>



static const GADI_CHAR *arg_short_options = "c:s:a:h:";
static struct option arg_long_options[] =
{
    {"channel",  1, 0, 'c'},
    {"sensor", 1, 0, 's'},
    {"addr", 1, 0, 'a'},
    {"hts",  1, 0, 'h'},
    {0,      0, 0, 0}
};


GADI_VI_SensorDetectInfoT*   sensor_detect = NULL;

extern GADI_ERR gadi_priv_driver_get_fd(GADI_S32* driverFd);
#define GK_MEDIA_IOC_VI_SRC_SENSOR_DETECT   _IOW('s', 0x18, GADI_VI_SensorDetectInfoT*)
int main(int argc, char* argv[])
{
    char cmd[1000];
    char sensor[GADI_VI_MAX_NAME_LENGTH];
    GADI_U32 retVal;
    GADI_U32 i;
    GADI_S32 fdmedia, ch;
    GADI_U32 hts = 0;
    GADI_U32 src_id = 0;
    GADI_S32 addr_id = -1;
    GADI_U32 sensor_idx[2];
    GADI_U32 addr_idx[2];
    GADI_U32 loop_start,loop_end;
    FILE *fp;
    GADI_VI_SensorDetectInfoT   sensor_detect_tmp;
    char ex_name[GADI_VI_MAX_NAME_LENGTH];
    char* ex_end;

    memset(sensor, 0, GADI_VI_MAX_NAME_LENGTH);
    /*change parameters when giving input options.*/
    while ((ch = getopt_long(argc, argv, arg_short_options, arg_long_options, NULL)) != -1)
    {
        switch (ch)
        {
            case 'c':
                src_id = strtol(optarg,NULL,0);
//                printf("channel=%d ", src_id);    
                break;

            case 's':
                strcpy(sensor, optarg);
//                printf("sensor=%s ", sensor);
                break;

            case 'a':
                addr_id = strtol(optarg,NULL,0);
//                printf("addr=%d ", addr_id);
                break;

            case 'h':
                hts = strtol(optarg,NULL,0);
//                printf("hts=0x%X ", hts);
                break;                

            default:
                break;
        }
    }
    printf("\n");

    retVal = gadi_sys_init();
    if(retVal != GADI_OK)
    {
        printf("gadi_sys_init() failed!\n");
        return 0;
    }
    retVal = gadi_priv_driver_get_fd(&fdmedia);
    if(retVal != GADI_OK)
    {
        printf("gadi_priv_driver_get_fd() failed!\n");
        gadi_sys_exit();
        return 0;
    }

    sprintf(cmd, "/usr/local/bin/sensors/sensor_detect.bin");

    if ((fp = fopen(cmd, "rb")) != NULL)
    {
        if (fread(&sensor_detect_tmp, 1, sizeof(GADI_VI_SensorDetectInfoT), fp) !=
            sizeof(GADI_VI_SensorDetectInfoT))
        {
            GADI_ERROR("fread %s failed\n", cmd);
            fclose(fp);
            gadi_sys_exit();
            return 0;
        }
        sensor_detect = malloc(sensor_detect_tmp.length);
        if(sensor_detect == NULL)
        {
            printf("malloc sensor_detect failed!!!!\n");
            fclose(fp);
            gadi_sys_exit();
            return -1;
        }
        fseek(fp, 0, SEEK_SET);
        if (fread(sensor_detect, 1, sensor_detect_tmp.length, fp) !=
            sensor_detect_tmp.length)
        {
            GADI_ERROR("fread %s failed\n", cmd);
            fclose(fp);
            if(sensor_detect)
            {
                free(sensor_detect);
            }
            gadi_sys_exit();
            return 0;
        }
        fclose(fp);
    }
    else
    {
        gadi_sys_exit();
        return 0;
    }

    switch(src_id)
    {
        case 0:
          loop_start = 0;
          loop_end = 1;
          break;
        case 1:
          loop_start = 1;
          loop_end = 2;
          break;
        case 2:
          loop_start = 0;
          loop_end = 2;
          break;
        default:
          loop_start = 0;
          loop_end = 1;
          break;
    }

    if(strlen(sensor) != 0)
    {
        for(i=0;i<sensor_detect->num;i++)
        {
            if(strncmp(sensor_detect->HwInfo[i].name, sensor, strlen(sensor)) == 0)
            {
                sensor_detect->num = 0x01;
                if(i != 0)
                {
                    memcpy(&sensor_detect->HwInfo[0], &sensor_detect->HwInfo[i], sizeof(sensor_detect->HwInfo[i]));
                }
                break;
            }
        }
    }
    
    if(addr_id != -1)
    {
        sensor_detect->HwInfo[0].hw_addr_index= 0x01000000|addr_id;
    } 

    sensor_detect->debug_hts = hts;
    
    for(i=loop_start;i<loop_end;i++)
    {
        sensor_detect->src_id = i;
        retVal = ioctl(fdmedia, GK_MEDIA_IOC_VI_SRC_SENSOR_DETECT, sensor_detect);
        sensor_idx[i] = retVal&0xfff;
        addr_idx[i] = (retVal>>12)&0xf;
        if((sensor_idx[i] >= 0) && (sensor_idx[i] < sensor_detect->num))
        {
            printf("find:%s at 0x%0x on ch%d\n", sensor_detect->HwInfo[sensor_idx[i]].name,
                sensor_detect->HwInfo[sensor_idx[i]].hw_addr[addr_idx[i]] << 1,
                sensor_detect->src_id);

            memcpy(ex_name, sensor_detect->HwInfo[sensor_idx[i]].name, GADI_VI_MAX_NAME_LENGTH);
            if((ex_end = strstr(ex_name, "_mipi"))!= NULL ||
                (ex_end = strstr(ex_name, "_MIPI"))!= NULL ||
                (ex_end = strstr(ex_name, "_dvp"))!= NULL||
                (ex_end = strstr(ex_name, "_DVP"))!= NULL)
            {
                *ex_end = '\0';//delete the substring "_dvp"/"_DVP"/"_mipi"/"MIPI"
            }
                
            if(i)
            {
                system("rm /tmp/sensor_hw_ch1.bin");
                sprintf(cmd, "rm /tmp/sensor_ex_ch1.ko");
                system(cmd);
                sprintf(cmd, "ln -s /usr/local/bin/sensors/%s_hw.bin /tmp/sensor_hw_ch1.bin",
                    sensor_detect->HwInfo[sensor_idx[i]].name);
                system(cmd);
                sprintf(cmd, "ln -s /lib/modules/$(uname -r)/extra/%s_ex.ko /tmp/sensor_ex_ch1.ko",
                    ex_name);
                system(cmd);
            }
            else
            {
                system("rm /tmp/sensor_hw_ch0.bin");
                sprintf(cmd, "rm /tmp/sensor_ex_ch0.ko");
                system(cmd);
                sprintf(cmd, "ln -s /usr/local/bin/sensors/%s_hw.bin /tmp/sensor_hw_ch0.bin",
                    sensor_detect->HwInfo[sensor_idx[i]].name);
                system(cmd);
                sprintf(cmd, "ln -s /lib/modules/$(uname -r)/extra/%s_ex.ko /tmp/sensor_ex_ch0.ko",
                    ex_name);
                system(cmd);
            }
        }
    }

    if(sensor_detect)
    {
        free(sensor_detect);
    }
    gadi_sys_exit();
    return retVal;
usage:
    printf("usage:\n");
    printf("1. sensordetect 0/1, default is ch0.\n");
    printf("2. sensordetect 0/1 name.\n");
    printf("3. sensordetect 0/1 name addr.\n");
    if(sensor_detect)
    {
        free(sensor_detect);
    }
    gadi_sys_exit();
    return retVal;
}

