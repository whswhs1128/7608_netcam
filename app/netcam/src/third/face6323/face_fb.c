/******************************************************************************
** \file        adi/test/src/fb.c
**
**
** \brief       Framebuffer test.
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <getopt.h>
#include <sys/ioctl.h>

#include <linux/fb.h>
#include "adi_types.h"
#include "adi_sys.h"
#include "adi_vout.h"
#include <signal.h>
#include "inc/face6323.h"
#include "lv_conf.h"

//*****************************************************************************
//*****************************************************************************
//** Local Defines
//*****************************************************************************
//*****************************************************************************
#define COLORFMT_RGB565    0
#define COLORFMT_RGB555    1
#define COLORFMT_RGB444    2

#define FILL_DEMO_256           (0)
#define FILL_DEMO_RGB565        (0)
#define FILL_DEMO_BGR565        (0)
#define FILL_DEMO_ARGB4444      (1)
#define FILL_DEMO_RGBA8888      (0)
#define FILL_DEMO_ABGR8888      (0)

//#define FILL_XY_EXCHANGE

#if FILL_DEMO_256

#define SHOWPORT0  0x00
#define SHOWPORT1  0x40
#define SHOWPORT2  0x80
#define SHOWPORT3  0xFF

#elif  FILL_DEMO_RGB565

#define SHOWPORT0  0x001f
#define SHOWPORT1  0x07e0
#define SHOWPORT2  0xffff
#define SHOWPORT3  0xf800

#elif  FILL_DEMO_BGR565

#define SHOWPORT0  0xf800
#define SHOWPORT1  0x07e0
#define SHOWPORT2  0xffff
#define SHOWPORT3  0x001f

#elif FILL_DEMO_ARGB4444

#define SHOWPORT0  0xf000
#define SHOWPORT1  0xff00
#define SHOWPORT2  0xf0f0
#define SHOWPORT3  0xf00f

#elif  FILL_DEMO_RGBA8888

#define SHOWPORT0  0x0000ff80
#define SHOWPORT1  0x00ff0080
#define SHOWPORT2  0xffffff80
#define SHOWPORT3  0xff000080

#else  // FILL_DEMO_ABGR8888

#define SHOWPORT0  0x80ff0000
#define SHOWPORT1  0x8000ff00
#define SHOWPORT2  0x80ffffff
#define SHOWPORT3  0x800000ff

#endif


//*****************************************************************************
//*****************************************************************************
//** Local structures
//*****************************************************************************
//*****************************************************************************
struct fb_cmap_user {
    unsigned int start;            /* First entry    */
    unsigned int len;            /* Number of entries */
    unsigned short *red;        /* Red values    */
    unsigned short *green;
    unsigned short *blue;
    unsigned short *transp;        /* transparency, can be NULL */
};

/*14 byptes*/
typedef struct
{
    /*bmp file header: file type.*/
	unsigned short	bfType;
    /*bmp file header: file size.*/
	unsigned int	bfSize;
    /*bmp file header: reserved.*/
	unsigned short	bfReserved1;
    /*bmp file header: reserved.*/
	unsigned short	bfReserved2;
    /*bmp file header: offset bits.*/
	unsigned int	bfOffBits;
}__attribute__ ((packed)) OSD_BmpHeader;

/*bmp inforamtion header 40bytes*/
typedef struct
{
    /*bmp inforamtion header: size*/
	unsigned int	biSize;
    /*bmp inforamtion header: width*/
	unsigned int	biWidth;
    /*bmp inforamtion header: height*/
    unsigned int	biHeight;
    /*bmp inforamtion header: planes*/
	unsigned short	biPlanes;
    /*bmp inforamtion header:  1,4,8,16,24 ,32 color attribute*/
	unsigned short	biBitCount;
	/*bmp inforamtion header: Compression*/
	unsigned int	biCompression;
    /*bmp inforamtion header: Image size*/
	unsigned int	biSizeImage;
	/*bmp inforamtion header: XPelsPerMerer*/
	unsigned int	biXPelsPerMerer;
    /*bmp inforamtion header: YPelsPerMerer*/
	unsigned int	biYPelsPerMerer;
    /*bmp inforamtion header: ClrUsed*/
	unsigned int	biClrUsed;
    /*bmp inforamtion header: ClrImportant*/
	unsigned int	biClrImportant;
}__attribute__ ((packed)) OSD_BmpInfoHeader;

typedef enum {
    GK_FB_COLOR_AUTO      = 0,

    GK_FB_COLOR_CLUT_8BPP = 1,
    GK_FB_COLOR_RGB565    = 2,

    GK_FB_COLOR_BGR565    = 3,
    GK_FB_COLOR_AGBR4444  = 4,    //AYUV 4:4:4:4
    GK_FB_COLOR_RGBA4444  = 5,
    GK_FB_COLOR_BGRA4444  = 6,
    GK_FB_COLOR_ABGR4444  = 7,
    GK_FB_COLOR_ARGB4444  = 8,
    GK_FB_COLOR_AGBR1555  = 9,    //AYUV 1:5:5:5
    GK_FB_COLOR_GBR1555   = 10,    //YUV 1(ignored):5:5:5
    GK_FB_COLOR_RGBA5551  = 11,
    GK_FB_COLOR_BGRA5551  = 12,
    GK_FB_COLOR_ABGR1555  = 13,
    GK_FB_COLOR_ARGB1555  = 14,
    GK_FB_COLOR_AGBR8888  = 15,    //AYUV 8:8:8:8
    GK_FB_COLOR_RGBA8888  = 16,
    GK_FB_COLOR_BGRA8888  = 17,
    GK_FB_COLOR_ABGR8888  = 18,
    GK_FB_COLOR_ARGB8888  = 19,

    GK_FB_COLOR_YUV565    = 20,
    GK_FB_COLOR_AYUV4444  = 21,
    GK_FB_COLOR_AYUV1555  = 22,
    GK_FB_COLOR_YUV555    = 23,

    GK_FB_COLOR_UNSUPPORTED,  //Reserved only, not supported
}COLOR_FORTMAT_T;


//*****************************************************************************
//*****************************************************************************
//** Global Data
//*****************************************************************************
//*****************************************************************************


//*****************************************************************************
//*****************************************************************************
//** Local Data
//*****************************************************************************
//*****************************************************************************
static int fbHandle = -1;
static unsigned char  * fbBuffer = NULL;
static unsigned char  * fbBufferH = NULL;
int hWidth = 480;
int hHeight = 800;
static struct fb_fix_screeninfo fixInfo;
static struct fb_var_screeninfo varInfo;

static const char shortOptions[] = "hocfbswWJjKkr";
static struct option longOptions[] =
{
    {"help",     0, 0, 'h'},
    {"open",     0, 0, 'o'},
    {"select",   0, 0, 's'},
    {"close",    0, 0, 'c'},
    {"fill",     0, 0, 'f'},
    {"show",     0, 0, 'w'},
    {"show",     0, 0, 'W'},
    {"show",     0, 0, 'J'},
    {"show",     0, 0, 'j'},
    {"show",     0, 0, 'K'},
    {"show",     0, 0, 'k'},
    {"cycle demo",     0, 0, 'r'},
    {"blank",    0, 0, 'b'},
    {0,          0, 0, 0}
};
static int voutchannel = GADI_VOUT_A;

extern SDK_HandleT voutHandle;

//*****************************************************************************
//*****************************************************************************
//** Local Functions Declaration
//*****************************************************************************
//*****************************************************************************
static void usage(void);
static SDK_ERR handle_fb_command(int argc, char* argv[]);
int fb_fill(void);
int fb_select(void);


//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//****************************************************************************


static char * rotate_fb(char *data, int width, int height)
{
    int i,j;
    unsigned short *fromData = (unsigned short *)data;
    unsigned short *baseAddr = (unsigned short *)fbBuffer;
    for(i = 0; i < height; i++)
    {
        for(j = 0; j < width; j++)
        {
        #if 0
            tmpData[i * height * 2 + j] = data[(height - j - 1) * width * 2 + i];
            tmpData[i * height * 2 + j + 1] = data[(height - j - 1) * width * 2 + i + 1];
        #else
            //tmpData[i * height + j] = fromData[(height - j - 1) * width + i];
            baseAddr[(width - j - 1) * height + i] = fromData[i * width + j];
        #endif
        }
    }
    return data;
}

int fb_rotate_buff(char *buffer, int width, int height, int srcX, int srcY)
{
    int i, j, k, n;
    int pos0 = (varInfo.yres - srcX - 1) * varInfo.xres + srcY;
    unsigned short *fromData = (unsigned short *)buffer;
    unsigned short *baseAddr = (unsigned short *)fbBuffer;
    n = 0;
    for(i = 0; i < height; i++)
    {
        k = pos0 + i;
        n += width;
        for(j = i * width; j < n; j++)
        {
            baseAddr[k] = fromData[j];
            k -= varInfo.xres;
        }
    }
}

int fb_rotate_fresh(void)
{
    rotate_fb(fbBufferH, hWidth, hHeight);
    varInfo.yoffset= 0;
    if(ioctl(fbHandle, FBIOPAN_DISPLAY, &varInfo) < 0)
    {
        printf("Cannot display Pan\n.");
        return -1;
    }
    return 0;
}

int fb_fresh(void)
{
    //printf("-->fb refresh\n");
    varInfo.yoffset= 0;
    if(ioctl(fbHandle, FBIOPAN_DISPLAY, &varInfo) < 0)
    {
        printf("Cannot display Pan\n.");
        return -1;
    }
    return 0;
}

int fb_open(void)
{
    if (fbHandle >= 0) {
        GADI_ERROR("Frame buffer already init.\n");
        return -1;
    }

    fb_select();

    fbHandle = open ("/dev/fb0", O_RDWR);
    if (fbHandle < 0)
    {
        perror("open /dev/fb0 error:\n");
        return -1;
    }

    if(ioctl(fbHandle, FBIOGET_FSCREENINFO, &fixInfo) < 0)
    {
        printf("Cannot get fixed screen info\n.");
        close (fbHandle);
        fbHandle = -1;
        return -1;
    }

    if(ioctl(fbHandle, FBIOGET_VSCREENINFO, &varInfo) < 0)
    {
        GADI_ERROR("Cannot get var screen info\n.");
        close (fbHandle);
        fbHandle = -1;
        return -1;
    }


    printf("framebuffer: %d x %d @%dbyte --%dbpp\n", varInfo.xres, varInfo.yres,
                fixInfo.line_length, varInfo.bits_per_pixel);

    fbBufferH = (unsigned char *)malloc(varInfo.xres * varInfo.yres * varInfo.bits_per_pixel / 8);
    if (fbBufferH == NULL)
    {
        printf("malloc fbBufferH %d failed.\n", varInfo.xres * varInfo.yres * varInfo.bits_per_pixel / 8);
        return -1;
    }
    hWidth = varInfo.yres;
    hHeight = varInfo.xres;
    memset(fbBufferH, 0, varInfo.xres * varInfo.yres * varInfo.bits_per_pixel / 8);
    fbBuffer = (unsigned char *)mmap(NULL, fixInfo.smem_len,
                                     PROT_WRITE,  MAP_SHARED, fbHandle, 0);

    printf("framebuffer addr:0x%08x, len=[0x%x]\n", (int)fbBuffer, fixInfo.smem_len);
    if (fbBuffer == MAP_FAILED)
    {
        GADI_ERROR("Cannot mmap framebuffer memory.");
        close (fbHandle);
        fbHandle = -1;
        return -1;
    }

    fb_change_par(GK_FB_COLOR_RGB565);

    return 0;
}


int fb_change_par(COLOR_FORTMAT_T color_format)
{
    if (fbHandle < 0)
    {
        return -1;
    }
    GADI_INFO("%s\n",__FUNCTION__);

    if(ioctl(fbHandle, FBIOGET_VSCREENINFO, &varInfo) < 0)
    {
        GADI_ERROR("Cannot get var screen info\n.");
        close (fbHandle);
        fbHandle = -1;
        return -1;
    }
    GADI_INFO("%s step1 ok\n",__FUNCTION__);

    switch(color_format)
    {
        case GK_FB_COLOR_CLUT_8BPP://CLUT
            varInfo.bits_per_pixel = 8;
            varInfo.red.offset = 0;
            varInfo.red.length = 0;
            varInfo.red.msb_right = 0;
            varInfo.green.offset = 0;
            varInfo.green.length = 0;
            varInfo.green.msb_right = 0;
            varInfo.blue.offset = 0;
            varInfo.blue.length = 0;
            varInfo.blue.msb_right = 0;
            varInfo.transp.offset = 0;
            varInfo.transp.length = 0;
            varInfo.transp.msb_right = 0;
        break;
        case GK_FB_COLOR_RGB565: //RGB565
            varInfo.bits_per_pixel = 16;
            varInfo.red.offset = 11;
            varInfo.red.length = 5;
            varInfo.red.msb_right = 0;
            varInfo.green.offset = 5;
            varInfo.green.length = 6;
            varInfo.green.msb_right = 0;
            varInfo.blue.offset = 0;
            varInfo.blue.length = 5;
            varInfo.blue.msb_right = 0;
            varInfo.transp.offset = 0;
            varInfo.transp.length = 0;
            varInfo.transp.msb_right = 0;

        break;
        case GK_FB_COLOR_RGBA8888: //RGB565
            varInfo.bits_per_pixel = 32;
            varInfo.red.offset = 24;
            varInfo.red.length = 8;
            varInfo.red.msb_right = 0;
            varInfo.green.offset = 16;
            varInfo.green.length = 8;
            varInfo.green.msb_right = 0;
            varInfo.blue.offset = 8;
            varInfo.blue.length = 8;
            varInfo.blue.msb_right = 0;
            varInfo.transp.offset = 0;
            varInfo.transp.length = 8;
            varInfo.transp.msb_right = 0;

        break;
        default :
            break;
    }
    GADI_INFO("%s step2 ok\n",__FUNCTION__);
    if(ioctl(fbHandle, FBIOPUT_VSCREENINFO, &varInfo) < 0)
    {
        GADI_ERROR("Cannot put var screen info\n.");
        close (fbHandle);
        fbHandle = -1;
        return -1;
    }
	GADI_INFO("%s step3 ok\n",__FUNCTION__);

    return 0;
}

int fb_close(void)
{
    if (fbHandle < 0) {
        GADI_ERROR("Frame buffer already closed.\n");
        return -1;
    }

    munmap(fbBuffer, fixInfo.smem_len);
    fbBuffer = NULL;

    close(fbHandle);
    fbHandle = -1;
    GADI_INFO("Close frame buffer.\n");

    return 0;
}

int fb_select(void)
{
    GADI_VOUT_SelectFbParamsT fbPar;

    fbPar.voutChannel = voutchannel;
    fbPar.fbChannel   = 0;
    return gadi_vout_select_fb(voutHandle, &fbPar);
}

int fb_draw_line(int x1, int y1, int x2, int y2, int color, int bold)
{
    int i, j;
    unsigned short *baseAddr = (unsigned short *)fbBufferH;
    if (x1 == x2)
    {
        for (i = 0; i < bold; i++)
        {
            for (j = y1; j <= y2; j++)
            {
                baseAddr[j * hWidth + x1 + i] = color;
            }
        }
    }
    else if (y1 == y2)
    {
        for (i = 0; i < bold; i++)
        {
            for (j = x1; j <= x2; j++)
            {
                baseAddr[(y1 + i) * hWidth + j] = color;
            }
        }
    }
    return 0;
}

int fb_draw_line2(int x1, int y1, int x2, int y2, int color, int bold)
{
    int i, j;
    unsigned short *baseAddr = (unsigned short *)fbBuffer;
    if (x1 == x2)
    {
        for (i = 0; i < bold; i++)
        {
            for (j = y1; j <= y2; j++)
            {
                baseAddr[j * hHeight + x1 + i] = color;
            }
        }
    }
    else if (y1 == y2)
    {
        for (i = 0; i < bold; i++)
        {
            for (j = x1; j <= x2; j++)
            {
                baseAddr[(y1 + i) * hHeight + j] = color;
            }
        }
    }
    return 0;
}


int fb_draw_rect(int x, int y, int width, int height, int color, int bold)
{
    int i, j;
    fb_draw_line(x, y, x + width, y, color, bold);
    fb_draw_line(x, y + height - bold, x + width, y + height - bold, color, bold);
    fb_draw_line(x, y, x, y + height, color, bold);
    fb_draw_line(x + width, y, x + width, y + height - bold, color, bold);
    fb_rotate_fresh();
    return 0;
}

int fb_draw_rect2(int x, int y, int width, int height, int color, int bold)
{
    int i, j;

    if((x < 0 )||(y < 0 )||(width < 0 )||(height < 0 ) ||(x+width >= 700))
        return;

    fb_draw_line2(x, y, x + width - 1, y, color, bold);
    fb_draw_line2(x, y + height - bold, x + width - 1, y + height - bold, color, bold);
    fb_draw_line2(x, y, x, y + height - 1, color, bold);
    fb_draw_line2(x + width - bold, y, x + width - bold, y + height - 1, color, bold);
    return 0;
}

int fb_draw_clear(void)
{
    //memset(fbBufferH, 0, hWidth * hHeight * 2);
    memset(fbBuffer, 0, hWidth * hHeight * 2);
    fb_fresh();
}


int fb_fill2(void)
{
    int x, y;
    int width, height;
    unsigned short *baseAddr = (unsigned short *)fbBufferH;
    unsigned int pitch_pix = 0;

    width = 480;
    height = 800;

    pitch_pix= width;

    /*fill rec.*/
    printf("256[%d] RGB[%d] BGR[%d] RGBA[%d] ABGR[%d]\r\n",FILL_DEMO_256,FILL_DEMO_RGB565,FILL_DEMO_BGR565,FILL_DEMO_RGBA8888,FILL_DEMO_ABGR8888);
    printf("X[%d] Y[%d] PITCH[%d]\r\n",width,varInfo.yres,fixInfo.line_length);


    for(y=0; y < height/2; y++)
    {
        for(x=0; x < width/2; x++)
        {
            /*fill BLUE only.*/
            *(baseAddr + x + y*(pitch_pix)) = SHOWPORT0;
        }
        for(; x < width; x++)
        {
            /*fill GREEN only.*/
            *(baseAddr + x + y*(pitch_pix)) = SHOWPORT1;
        }
    }
    for(; y < height; y++)
    {
        for(x=0; x < width/2; x++)
        {
            /*fill WHITE only.*/
            *(baseAddr + x + y*(pitch_pix)) = SHOWPORT2;
        }
        for(; x < width; x++)
        {
            /*fill RED only.*/
            *(baseAddr + x + y*(pitch_pix)) = SHOWPORT3;
        }
    }

    #if 0
    printf("\n");
    for (x = 0; x < 480; x++)
    {
        for (y = 0; y < 800; y++)
            printf("%04x ", baseAddr[x * 800 + y]);
        printf("\n");
    }
    #endif

    rotate_fb(fbBufferH, width, height);

    /*
    baseAddr = (unsigned short *)fbBuffer;
    printf("\n");
    for (x = 0; x < 480; x++)
    {
        for (y = 0; y < 800; y++)
            printf("%04x ", baseAddr[x * 800 + y]);
        printf("\n");
    }
    */
    fb_fresh();

    return 0;
}


int fb_fill(void)
{
    int x, y;
#if   ((FILL_DEMO_RGBA8888)|(FILL_DEMO_ABGR8888))
    unsigned int *baseAddr = (unsigned int *)fbBuffer;
#elif ((FILL_DEMO_RGB565)|(FILL_DEMO_BGR565)|(FILL_DEMO_ARGB4444))
    unsigned short *baseAddr = (unsigned short *)fbBuffer;
#else //FILL_DEMO_256
    unsigned char *baseAddr = (unsigned char *)fbBuffer;
#endif

    //unsigned int tmpx = varInfo.xres;
    //unsigned int tmpy = varInfo.yres;
    unsigned int pitch_pix = 0;

#ifdef FILL_XY_EXCHANGE
    varInfo.xres = tmpy;
    varInfo.yres = tmpx;
#endif

    pitch_pix= varInfo.xres;

    /*fill rec.*/
    printf("256[%d] RGB[%d] BGR[%d] RGBA[%d] ABGR[%d]\r\n",FILL_DEMO_256,FILL_DEMO_RGB565,FILL_DEMO_BGR565,FILL_DEMO_RGBA8888,FILL_DEMO_ABGR8888);
    printf("X[%d] Y[%d] PITCH[%d]\r\n",varInfo.xres,varInfo.yres,fixInfo.line_length);


    for(y=0; y < varInfo.yres/2; y++)
    {
        for(x=0; x < varInfo.xres/2; x++)
        {
            /*fill BLUE only.*/
            *(baseAddr + x + y*(pitch_pix)) = SHOWPORT0;
        }
        for(; x < varInfo.xres; x++)
        {
            /*fill GREEN only.*/
            *(baseAddr + x + y*(pitch_pix)) = SHOWPORT1;
        }
    }
    for(; y < varInfo.yres; y++)
    {
        for(x=0; x < varInfo.xres/2; x++)
        {
            /*fill WHITE only.*/
            *(baseAddr + x + y*(pitch_pix)) = SHOWPORT2;
        }
        for(; x < varInfo.xres; x++)
        {
            /*fill RED only.*/
            *(baseAddr + x + y*(pitch_pix)) = SHOWPORT3;
        }
    }


#ifdef FILL_XY_EXCHANGE
    varInfo.xres = tmpx;
    varInfo.yres = tmpy;
#endif

    printf("X[%d] Y[%d] PITCH[%d]\r\n",varInfo.xres,varInfo.yres,fixInfo.line_length);

    varInfo.yoffset= 0;
    if(ioctl(fbHandle, FBIOPAN_DISPLAY, &varInfo) < 0)
    {
        printf("Cannot display Pan\n.");
        return -1;
    }

    return 0;
}



int fb_show_bitmap_bit8(char *filepath)
{
    int fd;
    unsigned int bitMapSize;
    unsigned int bitMapPitch;
    OSD_BmpHeader bmpHeader;
    OSD_BmpInfoHeader bmpInfo;
    unsigned int index;
    //unsigned char colorMode;
    unsigned char *data;
    //unsigned short *switchData;

    fd = open(filepath, O_RDONLY);
    if (fd < 0) {
        GADI_ERROR("Open logo bitmap file failed\n");
        return -1;
    }

    read(fd, &bmpHeader, sizeof(bmpHeader));
    read(fd, &bmpInfo, sizeof(bmpInfo));

    if(bmpInfo.biBitCount != 8){
        GADI_ERROR("cor:%d not color conver!!!.framebuffer use bit8 color mode.\n",bmpInfo.biBitCount);
        return -1;
    }

    printf("biCompression:%d\n",bmpInfo.biCompression);

    if(bmpInfo.biCompression != 0){
        GADI_ERROR("unknow bitfields.\n");
        return -1;

    }


#if 1
    //fill bmp data
    bitMapPitch = (((bmpInfo.biWidth*(bmpInfo.biBitCount/8) + 3)>>2)<<2);// 4byte align in bmp
    bitMapSize = bmpInfo.biHeight * bitMapPitch;

    data = (unsigned char *)(fbBuffer) + bitMapSize;
    lseek(fd, bmpHeader.bfOffBits, SEEK_SET);//jump read bitmap header
    for (index = 0; index < bmpInfo.biHeight; index++) {
        data -= bitMapPitch;
        read(fd, data, bitMapPitch);
    }
#else
    data = (unsigned char *)(fbBuffer) + 0;
    lseek(fd, bmpHeader.bfOffBits, SEEK_SET);//jump read bitmap header
    for (index = 0; index < bmpInfo.biHeight; index++) {
        data -= bitMapPitch;
        read(fd, data, bitMapPitch);
    }
#endif
    close(fd);


    varInfo.yoffset= 0;
    printf("bits_per_pixel:%d\n",varInfo.bits_per_pixel);
    if(ioctl(fbHandle, FBIOPAN_DISPLAY, &varInfo) < 0)
    {
        printf("Cannot display Pan\n.");
        return -1;
    }

    return 0;

}

int fb_show_bitmap_bit16(char *filepath)
{
    int fd;
    unsigned int bitMapSize;
    unsigned int bitMapPitch;
    OSD_BmpHeader bmpHeader;
    OSD_BmpInfoHeader bmpInfo;
    unsigned int index;
    unsigned char colorMode;
    unsigned char *data;
    unsigned short *switchData;

    fd = open(filepath, O_RDONLY);
    if (fd < 0) {
        GADI_ERROR("Open logo bitmap file failed\n");
        return -1;
    }

    read(fd, &bmpHeader, sizeof(bmpHeader));
    read(fd, &bmpInfo, sizeof(bmpInfo));

    if(bmpInfo.biBitCount != 16){
        GADI_ERROR("cor:%d not color conver!!!.framebuffer use ARGB888 color mode.\n",bmpInfo.biBitCount);
        return -1;
    }

    printf("biCompression:%d\n",bmpInfo.biCompression);

    if(bmpInfo.biCompression == 0){
        colorMode = GK_FB_COLOR_ARGB1555;
    }
    else if (bmpInfo.biCompression == 3){
        GADI_ERROR("please parase and convert color by yourslef.\n");
        return -1;
    }
    else{
        GADI_ERROR("unknow bitfields.\n");
        return -1;
    }

    //fill bmp data
    bitMapPitch = (((bmpInfo.biWidth*(bmpInfo.biBitCount/8) + 3)>>2)<<2);// 4byte align in bmp
    bitMapSize = bmpInfo.biHeight * bitMapPitch;

    data = (unsigned char *)(fbBuffer) + bitMapSize;
    lseek(fd, bmpHeader.bfOffBits, SEEK_SET);//jump read bitmap header
    for (index = 0; index < bmpInfo.biHeight; index++) {
        data -= bitMapPitch;
        read(fd, data, bitMapPitch);
    }

    close(fd);

    /*framebuffer is RGB565, bitmap is 16bit RGB555. so convert!!!*/
    if(colorMode == GK_FB_COLOR_ARGB1555){
        switchData = (unsigned short *)fbBuffer;
        for (index = 0; index < fixInfo.smem_len/2; index++) {
            *(switchData+index) = (*(switchData+index) & 0x001f) |
                                  ((*(switchData+index) & 0x03e0) << 1) |
                                  ((*(switchData+index) & 0x7c00) << 1);
        }
    }

    varInfo.yoffset= 0;
    printf("bits_per_pixel:%d\n",varInfo.bits_per_pixel);
    if(ioctl(fbHandle, FBIOPAN_DISPLAY, &varInfo) < 0)
    {
        printf("Cannot display Pan\n.");
        return -1;
    }

    return 0;

}


int fb_show_bitmap_bit32(char *filepath)
{
    int fd;
    unsigned int bitMapSize;
    unsigned int bitMapPitch;
    OSD_BmpHeader bmpHeader;
    OSD_BmpInfoHeader bmpInfo;
    unsigned int index;
    unsigned char colorMode;
    unsigned char *data;
    //unsigned short *switchData;

    fd = open(filepath, O_RDONLY);
    if (fd < 0) {
        GADI_ERROR("Open logo bitmap file failed\n");
        return -1;
    }

    read(fd, &bmpHeader, sizeof(bmpHeader));
    read(fd, &bmpInfo, sizeof(bmpInfo));

    if(bmpInfo.biBitCount != 32){
        GADI_ERROR("color attribute:%d !!! please make sure the color attribute correct and mode.\n",bmpInfo.biBitCount);
        return -1;
    }

    printf("biCompression:%d\n",bmpInfo.biCompression);

    if(bmpInfo.biCompression == 0){

        if(varInfo.transp.offset == 0)
        {
            colorMode = GK_FB_COLOR_RGBA8888;

        }else
        {
            colorMode = GK_FB_COLOR_ABGR8888;
        }


    }
    else if (bmpInfo.biCompression == 3){
        GADI_ERROR("please parase and convert color by yourslef.\n");
        return -1;
    }
    else{
        GADI_ERROR("unknow bitfields.\n");
        return -1;
    }

    //fill bmp data
    bitMapPitch = (((bmpInfo.biWidth*(bmpInfo.biBitCount/8) + 3)>>2)<<2);// 4byte align in bmp
    bitMapSize = bmpInfo.biHeight * bitMapPitch;

    printf("size     [0x%x] \r\n",bitMapSize);
    printf("pitch    [%d] \r\n",bitMapPitch);
    printf("biWidth  [%d] \r\n",bmpInfo.biWidth);
    printf("biHeight [%d] \r\n",bmpInfo.biHeight);
    printf("bfOffBits[0x%x] \r\n",bmpHeader.bfOffBits);

#if 1

   data = (unsigned char *)(fbBuffer) + bitMapSize;
   lseek(fd, bmpHeader.bfOffBits, SEEK_SET);//jump read bitmap header
    for (index = 0; index < bmpInfo.biHeight; index++) {
        data -= bitMapPitch;
        read(fd, data, bitMapPitch);
    }
#else
    data = (unsigned char *)(fbBuffer);
    lseek(fd, bmpHeader.bfOffBits, SEEK_SET);//jump read bitmap header
    read(fd, data, bitMapSize);
#endif


    close(fd);

   // varInfo.xoffset= 0;
    varInfo.yoffset= 0;
    printf("bits_per_pixel:%d\n",varInfo.bits_per_pixel);
    printf("     colorMode:%d\n",colorMode);

    if(ioctl(fbHandle, FBIOPAN_DISPLAY, &varInfo) < 0)
    {
        printf("Cannot display Pan\n.");
        return -1;
    }

    return 0;

}


int fb_blank(void)
{
    printf("blank framebuffer.\n");

    if(ioctl(fbHandle, FBIOBLANK, FB_BLANK_NORMAL) < 0)
    {
        printf("Cannot blank Pan\n.");
        return -1;
    }

    varInfo.yoffset= 0;
    if(ioctl(fbHandle, FBIOPAN_DISPLAY, &varInfo) < 0)
    {
        printf("Cannot display Pan\n.");
        return -1;
    }


    return 0;
}



#if 0
static void sig_alarm_fb(void)
{
    static int cnt = 0;
    cnt++;
    printf("%s",__FUNCTION__);
    if(cnt%2 == 0)
    {
        fb_show_bitmap_bit16("/usr/local/bin/framebufferdemo2.bmp");
    }
    else
    {
        fb_show_bitmap_bit16("/usr/local/bin/framebufferdemo.bmp");
    }
    alarm(1);
}

static SDK_ERR fb_rgb5652yuv422(void)
{
    unsigned short *data,index;
    u8  r1, g1, b1;
    u8  y1, u1, v1;
    u8  r2, g2, b2;
    u8  y2, u2, v2;

    data = (unsigned short *)fbBuffer;
    for (index = 1; index < fixInfo.smem_len/sizeof(unsigned short); ) {
        b1 = ((*(data+(index-1))) & 0x001f);
        g1 = (((*(data+(index-1))) & 0x03e0)>>5);
        r1 = (((*(data+(index-1))) & 0x7c00)>>11);
        y1 = (( 66 * r1 + 129 * g1 +  25 * b1 + 128) >> 8) +  16;
        u1 = ((-38 * r1 -  74 * g1 + 112 * b1 + 128) >> 8) + 128;
        v1 = ((112 * r1 -  94 * g1 -  18 * b1 + 128) >> 8) + 128;

        b2 = ((*(data+(index))) & 0x001f);
        g2 = (((*(data+(index))) & 0x03e0)>>5);
        r2 = (((*(data+(index))) & 0x7c00)>>11);

        y2 = (( 66 * r2 + 129 * g2 +  25 * b2 + 128) >> 8) +  16;
        u2 = ((-38 * r2 -  74 * g2 + 112 * b2 + 128) >> 8) + 128;
        v2 = ((112 * r2 -  94 * g2 -  18 * b2 + 128) >> 8) + 128;

        #if 1
        (*(data+(index-1))) = (y1<<8)+u1;
        (*(data+(index))) = (y2<<8)+v2;
        #else
        (*(data+(index-1))) = (u1<<8)+y1;
        (*(data+(index))) = (u2<<8)+y2;
        #endif
    }
    return SDK_OK;
}
#endif

static SDK_ERR osd_switch_demo_bit16_bit8(void)
{
    //GADI_VOUT_SelectFbParamsT fbPar;
    int cnt = 10;

    while(cnt--)
    {
        fb_change_par(GK_FB_COLOR_RGB565);
        fb_show_bitmap_bit16("/usr/local/bin/fbdemo_bit16.bmp");
        sleep(1);
        fb_change_par(GK_FB_COLOR_CLUT_8BPP);
        fb_show_bitmap_bit8("/usr/local/bin/fbdemo_256.bmp");
        sleep(1);

    }
    return SDK_OK;
}


void fb_lvgl_memory_cpy(SDK_U16* bmpAdd,fb_rect_t* bmpInfo, SDK_U32 clearKeyColor)
{
    SDK_U32 bitAmount,count=0,offset,osdBitsPerLine;
    int mode = GK_FB_COLOR_RGB565;
    int bitCount = 0;
    fb_rect_t osdInfo;

    osdInfo.xPos = 0;
    osdInfo.yPos = 0;
    osdInfo.width= LV_HOR_RES;
    osdInfo.height= LV_VER_RES;

    if(mode == GK_FB_COLOR_RGB565)
        bitCount = 16;

    //printf("fb_memory_cpy, x,y (%d,%d) w,h(%d %d) \n",bmpInfo->xPos,bmpInfo->yPos,bmpInfo->width,bmpInfo->height );
    if(bitCount == 16)
    {
        volatile SDK_U16 *addr_16 = (unsigned short *)fbBuffer;
        SDK_U16* bmpAddress = (unsigned short *)bmpAdd;
        int addroffset = 0;
        int x1,y1 = 0;
        int i,j = 0;


        for(i = 0; i < bmpInfo->height; i++)
        {
            x1 = bmpInfo->yPos + i;
           // printf("x1,y1 (%d,%d)\n",x1,y1);
            for(j = 0; j < bmpInfo->width; j++) //300
            {
                y1 = osdInfo.width - 1 - bmpInfo->xPos - j;

                addroffset = (y1*osdInfo.height + x1)*2;
                addr_16 = (volatile SDK_U16*)(fbBuffer + addroffset);

                if (*bmpAddress != clearKeyColor)
                {
                    *addr_16 = *bmpAddress;
                   // printf("0x%x\n",*bmpAddress);
                }
                else
                {
                    *addr_16 = 0;
                }
                bmpAddress++;
            }
        }
    }
    fb_fresh();

}

int fb_lvgl_fill_rectangle(fb_rect_t rect, SDK_U32 color)
{
#if 0
    //SDK_U32 addr = (SDK_U32 *)fbBuffer;
    //unsigned short *addr = (unsigned short *)fbBufferH;
    SDK_U32  colorMode = GK_FB_COLOR_RGB565;
    SDK_U32  pixcol;
    SDK_U32 *addr_32;
    SDK_U32 i, j;
    fb_rect_t osdBound;

    osdBound.xPos = 0;
    osdBound.yPos = 0;
    osdBound.width= LV_HOR_RES;
    osdBound.height= LV_VER_RES;

    if((rect.xPos + rect.width) > osdBound.width || (rect.yPos+ rect.height) > osdBound.height)
    {
        printf("fb_fill_rectangle,bad parameter of x,y psotion than osd info\n");
        printf("fb_fill_rectangle, x,y (%d,%d) w,h(%d %d) \n",rect.xPos,rect.yPos,rect.width,rect.height );
        return -1;
    }

    if(colorMode == GK_FB_COLOR_RGB565)
    {
        SDK_S16 alignWidth = 0;
        pixcol = (color & 0xffff) | ((color & 0xffff) << 16);
        alignWidth = rect.width & (~0x1);

        for(i = 0; i < rect.height; i++)
        {
            addr_32 = (volatile SDK_U32 *)(fbBufferH + (osdBound.width * (rect.yPos + i) + rect.xPos) * 2);

            for(j = 0; j < rect.width; j += 2)
            {
                *addr_32++ = pixcol;
            }

            if(alignWidth != rect.width)
            {
                *addr_32 = (*addr_32) | (color & 0xffff);
            }
        }
    }

    fb_fresh();
#endif
    return 0;
}

