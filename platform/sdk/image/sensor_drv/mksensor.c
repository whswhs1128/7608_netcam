#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include "adi_types.h"
#include "adi_sys.h"
#include "adi_isp.h"
#include "mksensor.h"

#include "drv/mn168_reg_tbl.c"
#include "drv/imx122_reg_tbl.c"
#include "drv/imx291_reg_tbl.c"
#include "drv/imx238_reg_tbl.c"
#include "drv/jxh42_reg_tbl.c"
#include "drv/jxh61_reg_tbl.c"
#include "drv/jxh62_reg_tbl.c"
#include "drv/jxh65_reg_tbl.c"
#include "drv/jxf22_reg_tbl.c"
#include "drv/jxk02_reg_tbl.c"
#include "drv/jxk03_reg_tbl.c"
#include "drv/jxq03_reg_tbl.c"  //20200724 added
#include "drv/ar0130_reg_tbl.c"
#include "drv/ar0230_reg_tbl.c"
#include "drv/ar0237_reg_tbl.c"
#include "drv/ar0330_reg_tbl.c"
#include "drv/ar0331_reg_tbl.c"
#include "drv/bg0701_reg_tbl.c"
#include "drv/gc1024_reg_tbl.c"
#include "drv/gc0308_reg_tbl.c"
#include "drv/gc0328_reg_tbl.c"
#include "drv/gc1034_reg_tbl.c"
#include "drv/gc2033_reg_tbl.c"
#include "drv/gc2053_reg_tbl.c"
#include "drv/gc2053_dvp_reg_tbl.c"
#include "drv/gc4633_reg_tbl.c"
#include "drv/gc4623_reg_tbl.c"
#include "drv/gc4653_reg_tbl.c" // 20191128 added 
#include "drv/mis2003_reg_tbl.c"
#include "drv/ov2710_reg_tbl.c"
#include "drv/ov9710_reg_tbl.c"
#include "drv/ov9750_reg_tbl.c"
#include "drv/ov9732_reg_tbl.c"
#include "drv/ov4689_reg_tbl.c"
#include "drv/ov2735_reg_tbl.c"
#include "drv/os04b10_reg_tbl.c"
#include "drv/sc1035_reg_tbl.c"
#include "drv/sc1045_reg_tbl.c"
#include "drv/sc1135_reg_tbl.c"
#include "drv/sc1145_reg_tbl.c"
#include "drv/sc1235_reg_tbl.c"
#include "drv/sc2035_reg_tbl.c"
#include "drv/sc2045_reg_tbl.c"
#include "drv/sc2135_reg_tbl.c"
#include "drv/sc2235_reg_tbl.c"
#include "drv/sc3035_reg_tbl.c"
#include "drv/sc2232_reg_tbl.c"
#include "drv/sc4236_reg_tbl.c"
#include "drv/sc4236_dvp_reg_tbl.c"
#include "drv/sc3235_reg_tbl.c"
#include "drv/sc3235_dvp_reg_tbl.c"
#include "drv/sc5235_reg_tbl.c"
#include "drv/sp2307_reg_tbl.c"
#include "drv/sp2508_reg_tbl.c"
#include "drv/tw9912_reg_tbl.c"
#include "drv/pv6418_reg_tbl.c"
#include "drv/nt99231_reg_tbl.c"
#include "drv/pseudo_sensor_reg_tbl.c"


GADI_VI_SensorDrvInfoT* sensor[] =
{
    &mn168_sensor_mipi,
    &imx122_sensor,
    &imx291_sensor,
    &imx238_sensor,
    &jxh42_sensor,
    &jxh61_sensor,
    &jxh62_sensor,
    &jxh65_sensor,
    &jxf22_sensor,
    &jxk02_sensor,
    &jxk03_sensor,
    &jxq03_sensor_mipi, //20200724 added
    &sc1035_sensor,
    &sc1135_sensor,
    &sc1235_sensor,
    &sc2035_sensor,
    &sc2135_sensor,
    &sc2235_sensor,
    &sc1045_sensor,
    &sc1145_sensor,
    &sc2045_sensor,
    &sc3035_sensor,
    &sc2232_sensor,
    &sc4236_sensor_mipi,
    &sc4236_sensor_dvp,
    &sc3235_sensor_dvp,
    &sc3235_sensor_mipi,
    &sc5235_sensor_mipi,
    &mis2003_sensor,
    &mn168_sensor_mipi,
    &bg0701_sensor,
    &gc1024_sensor,
    &gc0308_sensor,
    &gc0328_sensor,
    &gc1034_sensor,
    &gc2033_sensor,
    &gc2053_sensor_mipi,
    &gc2053_sensor,
    &gc4633_sensor_mipi,
    &gc4623_sensor_mipi,
    &gc4653_sensor_mipi, // 20191128 added
    &ov2710_sensor,
    &ov9710_sensor,
    &ov9750_sensor,
    &ov9732_sensor,
    &ov4689_sensor_mipi,
    &os04b10_sensor_mipi,
    &ov2735_sensor,
    &ar0130_sensor,
    &ar0230_sensor,
    &ar0237_sensor,
    &ar0330_sensor,
    &ar0331_sensor,
    &sp2307_sensor,
    &sp2508_sensor,
    &tw9912_sensor,
    &pv6418_mach,
    &nt99231_sensor,
    &pseudo_sensor,
};

GADI_VI_SensorDetectInfoT*   sensor_detect = NULL;

int main(int argc, char* argv[])
{
    FILE *fp = NULL;;
    int i,j;
    int size;
    int count;
    char name[64];
    GADI_U32* magic_end;

    count   = sizeof(sensor)/sizeof(sensor[0]);
    for(i=0;i<count;i++)
    {
        sprintf(name, "./bin/%s_hw.bin", sensor[i]->HwInfo.name);
        fp = fopen(name, "wb");
        if(!fp)
        {
            printf("create %s error\n", name);
            return -1;
        }
        size    = sensor[i]->length;
        for(j=0;;j++)
        {
            if(sensor[i]->init_reg[j].reg == GADI_VI_SENSOR_TABLE_FLAG_END)
            {
                j++;
                sensor[i]->init_length = j;
                break;
            }
        }
        size += sizeof(GADI_VI_SensorRegT)*sensor[i]->init_length;
        sensor[i]->length = size;
        fwrite(sensor[i], 1, size, fp);
        fclose(fp);
        //printf("create %s success\n", name);
    }

    size = sizeof(GADI_VI_SensorDetectInfoT)+
                           count*sizeof(GADI_VI_SensorHWInfoT)+
                           sizeof(GADI_U32);
    sensor_detect = malloc(size);
    if(sensor_detect == NULL)
    {
        printf("malloc sensor_detect failed!!!!\n");
        return -1;
    }
    sensor_detect->magic_start        = GADI_SENSOR_DETECT_MAGIC_START;
    sensor_detect->version            = GADI_SENSOR_DETECT_VERSION;
    sensor_detect->length             = sizeof(GADI_VI_SensorDetectInfoT)+
                                        count*sizeof(GADI_VI_SensorHWInfoT)+
                                        sizeof(GADI_U32);
    sensor_detect->num                = 0;
    sensor_detect->debug_print        = 1;
    sensor_detect->src_id             = 0;
    magic_end = (GADI_U32*)(((GADI_ULONG)sensor_detect) + size - sizeof(GADI_U32));
    *magic_end                        = GADI_SENSOR_DETECT_MAGIC_END;
    for(i=0;i<count;i++)
    {
        if(sensor[i]->HwInfo.id_reg[0].reg != GADI_VI_SENSOR_TABLE_FLAG_END)
        {
            memcpy(&sensor_detect->HwInfo[sensor_detect->num], &sensor[i]->HwInfo,
                sizeof(sensor_detect->HwInfo[sensor_detect->num]));
            sensor_detect->num ++;
        }
    }
    sprintf(name, "./bin/sensor_detect.bin");
    fp = fopen(name, "wb");
    if(!fp)
    {
        printf("create %s error\n", name);
        return -1;
    }
    fwrite(sensor_detect, 1, size, fp);
    fclose(fp);
    //printf("create %s success\n", name);
    if(sensor_detect)
    {
        free(sensor_detect);
    }

    return 0;
}

