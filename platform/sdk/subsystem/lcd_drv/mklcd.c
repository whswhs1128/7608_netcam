#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include "adi_types.h"
#include "adi_sys.h"
#include "adi_vout.h"
#include "mklcd.h"

#include "drv/reg_tbl_rgb_my500q40p01.c"
#include "drv/reg_tbl_rgb_my430c40p01.c"
#include "drv/reg_tbl_i80_my240h20p01.c"
#include "drv/reg_tbl_rgb_tpo990000072.c"
#include "drv/reg_tbl_rgb_adt07006mr50.c"
#include "drv/reg_tbl_rgb_adt07011cr50.c"
#include "drv/reg_tbl_rgb_spi_avdtt35qvnn002.c"
#include "drv/reg_tbl_rgb_spi_ftd240g2402.c"

/*
    &rgb_my500q40p01
    &rgb_my430c40p01
    &i80_my240h20p01
    &rgb_tpo990000072
	&rgb_adt07006mr50
    &rgb_adt07011cr50
    &rgb_spi_avdtt35qvnn002
    &rgb_spi_ftd240g2402
*/

#define LCD_MODULE (&rgb_my500q40p01)

int main(int argc, char* argv[])
{
    FILE *fp = NULL;

    int size;
    unsigned char i=0;
    char name[64];
    GADI_VO_LcdHwInfoTagT  *plcd;
    GADI_VO_I80DrvInfoT    *pi80;
    GADI_VO_RgbSpiDrvInfoT *prgb_spi;
    plcd     = (GADI_VO_LcdHwInfoTagT*)LCD_MODULE;
    sprintf(name, "./bin/lcd_hw.bin");

    if( (fp = fopen(name, "wb")) == NULL )
    {
        printf("create %s error\n", name);
        return -1;
    }
    
    if(plcd->type == GADI_LCD_HW_INFO_TYPE_RGB)
    {
    
        printf("use the RGB interface\n");
        size    = sizeof(GADI_VO_RgbDrvInfoT);
        fwrite(plcd, 1, size, fp);
        fclose(fp);
    }
    else if(plcd->type == GADI_LCD_HW_INFO_TYPE_I80)
    {
        printf("use the I80 interface\n");
        pi80 = (GADI_VO_I80DrvInfoT*)plcd;
        if(pi80->i80_para.cmd_para_num == 0)
        {
            while(pi80->i80_para.cmd_para[i] != LCD_CMD_END_FLAG)
            {
                i++;
                if(i>128)
                {
                    printf("the parameter number for I80 is too large!\n");
                    fclose(fp);
                    return -1;
                }
            }
            pi80->i80_para.cmd_para_num = i;
        }
        printf("the parameter number for I80 is %d\n", pi80->i80_para.cmd_para_num );
        size = sizeof(GADI_VO_I80DrvInfoT);
        fwrite(plcd, 1, size, fp);        
        fclose(fp);
    }
    else if(plcd->type == GADI_LCD_HW_INFO_TYPE_RGB_SPI)
    {
        printf("use the RGB-SPI interface\n");
        prgb_spi =  (GADI_VO_RgbSpiDrvInfoT*)plcd;
        if( prgb_spi->rgb_spi_cfg.cmd_para_num == 0)
        {
            while(prgb_spi->rgb_spi_cfg.cmd_para[i] != LCD_CMD_END_FLAG)
            {
                i++;
                if(i>128)
                {
                    printf("the parameter number for RGB-SPI is too large!\n");
                    fclose(fp);
                    return -1;
                }
            }
            prgb_spi->rgb_spi_cfg.cmd_para_num = i;
        }
        printf("the parameter number for RGB-SPI is %d\n", prgb_spi->rgb_spi_cfg.cmd_para_num);
        size = sizeof(GADI_VO_RgbSpiDrvInfoT);
        fwrite(plcd, 1, size, fp);        
        fclose(fp);
    }
    else
    {
        printf("unsupported lcd type %d\n", plcd->type);
        fclose(fp);
        return -1;
    }

    printf("create %s success\n", name);
    return 0;
}

