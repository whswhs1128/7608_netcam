/*!
*****************************************************************************
** \file        common.h
**
**
** \brief       gkvision header file.
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2013-2018 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _COMMON_H_
#define _COMMON_H_
//*****************************************************************************
//*****************************************************************************
//** Defines and Macros
//*****************************************************************************
//*****************************************************************************
#define     GV_OK                     (0)     /* Normal End */
#define     GV_ERR_INITIALIZE         (-1)    /* Initialize Error */
#define     GV_ERR_INVALIDPARAM       (-2)    /* Invalid Parameter Error */
#define     GV_ERR_ALLOCMEMORY        (-3)    /* Memory Allocation Error */
#define     GV_ERR_REGISTER           (-4)    /* Register Error */
#define     GV_ERR_READFILE           (-5)    /* read file Error */
#define     GV_ERR_WRITEFILE          (-6)    /* write file Error */

#define     GV_VERSION_MAJOR          (1)
#define     GV_VERSION_MINOR          (1)
#define     GV_VERSION                ((GKVISION_VERSION_MAJOR<<16)|GKVISION_VERSION_MINOR)

//#define     GKVISION_LANDMARK_MAX           (128)

//#define     MIN(a, b) ((a) > (b) ? (b) : (a))
//#define     MAX(a, b) ((a) > (b) ? (a) : (b))

#ifdef _WIN32
#define     gv_printf()
#else
//#define     gv_printf(S...)           printf("gkvisoin: "S)
#define     gv_printf(S...)
#endif

#ifndef NULL
    #define NULL 0
#endif

typedef unsigned char       GADI_U8;         /*!< 8 bit unsigned integer. */
typedef unsigned short      GADI_U16;        /*!< 16 bit unsigned integer. */
typedef unsigned int        GADI_U32;        /*!< 32 bit unsigned integer. */
typedef unsigned long long  GADI_U64;        /*!< 64 bit unsigned integer. */
typedef signed char         GADI_S8;         /*!< 8 bit signed integer. */
typedef signed short        GADI_S16;        /*!< 16 bit signed integer. */
typedef signed int          GADI_S32;        /*!< 32 bit signed integer. */
typedef signed long long    GADI_S64;        /*!< 64 bit unsigned integer. */
typedef unsigned short      GADI_USHORT;     /*!< unsigned short. */
typedef short               GADI_SHORT;      /*!< short. */
typedef unsigned int        GADI_UINT;       /*!< unsigned int. */
typedef int                 GADI_INT;        /*!< int. */
typedef unsigned long       GADI_ULONG;      /*!< unsigned long. */
typedef long                GADI_LONG;       /*!< long. */
typedef unsigned long long  GADI_ULONGLONG; /*!< unsigned long long. */
typedef long long           GADI_LONGLONG;  /*!< long long. */
typedef char                GADI_UCHAR;      /*!< unsigned char */
typedef char                GADI_CHAR;       /*!< char */
typedef void                GADI_VOID;       /*!< void */

//*****************************************************************************
//*****************************************************************************
//** Data Structures
//*****************************************************************************
//*****************************************************************************

typedef struct
{
    int x;
    int y;
}GV_Point;

typedef struct
{
    int x;
    int y;
    int width;
    int height;
}GV_Rect;

typedef struct
{
    /*! \brief width and height */
    int w, h;
    /*! \brief step of a row in the image, usally equals to width */
    int step;
    /*! \brief gray image data */
    unsigned char *data;
}GV_Image;

typedef struct
{
  /*! \brief width and height */
  int w, h;
  /*! \brief step of a row in the image, usally equals to width */
  int step;
  /*! \brief gray image data */
  unsigned short *data;
}GV_ImageU16;

#ifdef __cplusplus
extern "C" {
#endif

int getticks(void);
int gv_read(char* filepath, GV_Image* image);
int gv_write(char* filepath, GV_Image* image);
GV_Image gv_image_create(int w, int h);
void gv_image_release(GV_Image *img);
GV_Image gv_image_resize(GV_Image srcImg, int w, int h);
GV_Image gv_image_create_rect(GV_Image* srcImg, GV_Rect rect);
void gv_Image_rotate(GV_Image* srcImage,
    GV_Image* dstImage, GV_Point center, float theta, float scale);

#ifdef __cplusplus
    }
#endif


#endif /* _COMMON_H_ */

