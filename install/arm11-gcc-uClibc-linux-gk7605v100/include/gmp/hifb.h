/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */

#ifndef __HIFB_H__
#define __HIFB_H__

#include "gfbg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define IOC_TYPE_HIFB       IOC_TYPE_GFBG
/* To obtain the colorkey of an overlay layer */
#define FBIOGET_COLORKEY_HIFB       FBIOGET_COLORKEY_GFBG
/* To set the colorkey of an overlay layer */
#define FBIOPUT_COLORKEY_HIFB       FBIOPUT_COLORKEY_GFBG
/* To get the alpha of an overlay layer */
#define FBIOGET_ALPHA_HIFB          FBIOGET_ALPHA_GFBG
/* To set the alpha of an overlay layer */
#define FBIOPUT_ALPHA_HIFB          FBIOPUT_ALPHA_GFBG
/* To get the origin of an overlay layer on the screen */
#define FBIOGET_SCREEN_ORIGIN_HIFB  FBIOGET_SCREEN_ORIGIN_GFBG
/* To set the origin of an overlay layer on the screen */
#define FBIOPUT_SCREEN_ORIGIN_HIFB  FBIOPUT_SCREEN_ORIGIN_GFBG
/* To obtain the anti-flicker setting of an overlay layer */
#define FBIOGET_DEFLICKER_HIFB      FBIOGET_DEFLICKER_GFBG
/* To set the anti-flicker setting of an overlay layer */
#define FBIOPUT_DEFLICKER_HIFB      FBIOPUT_DEFLICKER_GFBG
/* To wait for the vertical blanking region of an overlay layer */
#define FBIOGET_VER_BLANK_HIFB      FBIOGET_VER_BLANK_GFBG
/* To set the display state of an overlay layer */
#define FBIOPUT_SHOW_HIFB           FBIOPUT_SHOW_GFBG
/* To obtain the display state of an overlay layer */
#define FBIOGET_SHOW_HIFB           FBIOGET_SHOW_GFBG
/* to obtain the capability of an overlay layer */
#define FBIOGET_CAPABILITY_HIFB     FBIOGET_CAPABILITY_GFBG
/* To set the compression function status of an overlay layer */
#define FBIOPUT_COMPRESSION_HIFB    FBIOPUT_COMPRESSION_GFBG
/* To obtain the compression function status of an overlay layer */
#define FBIOGET_COMPRESSION_HIFB    FBIOGET_COMPRESSION_GFBG
/* To set the hdr function status of an overlay layer */
#define FBIOPUT_DYNAMIC_RANGE_HIFB  FBIOPUT_DYNAMIC_RANGE_GFBG
/* To get the hdr function status of an overlay layer */
#define FBIOGET_DYNAMIC_RANGE_HIFB  FBIOGET_DYNAMIC_RANGE_GFBG
/* To set the DDR detect zone of an overlay layer */
#define FBIOPUT_MDDRDETECT_HIFB    FBIOPUT_MDDRDETECT_GFBG
/* To get the DDR detect zone of an overlay layer */
#define FBIOGET_MDDRDETECT_HIFB    FBIOGET_MDDRDETECT_GFBG

typedef ot_fb_size hi_fb_size;

#define HI_FB_DYNAMIC_RANGE_SDR8  OT_FB_DYNAMIC_RANGE_SDR8
#define HI_FB_DYNAMIC_RANGE_SDR10 OT_FB_DYNAMIC_RANGE_SDR10
#define HI_FB_DYNAMIC_RANGE_HDR10 OT_FB_DYNAMIC_RANGE_HDR10
#define HI_FB_DYNAMIC_RANGE_HLG   OT_FB_DYNAMIC_RANGE_HLG
#define HI_FB_DYNAMIC_RANGE_SLF   OT_FB_DYNAMIC_RANGE_SLF
#define HI_FB_DYNAMIC_RANGE_BUTT  OT_FB_DYNAMIC_RANGE_BUTT
typedef ot_fb_dynamic_range hi_fb_dynamic_range;

typedef ot_fb_colorkey hi_fb_colorkey;

typedef ot_fb_rect hi_fb_rect;

typedef ot_fb_point hi_fb_point;

typedef ot_fb_deflicker hi_fb_deflicker;

/* Alpha info */
typedef ot_fb_alpha hi_fb_alpha;

#define  HI_FB_FORMAT_RGB565      OT_FB_FORMAT_RGB565
#define  HI_FB_FORMAT_RGB888      OT_FB_FORMAT_RGB888

#define  HI_FB_FORMAT_KRGB444     OT_FB_FORMAT_KRGB444
#define  HI_FB_FORMAT_KRGB555     OT_FB_FORMAT_KRGB555
#define  HI_FB_FORMAT_KRGB888     OT_FB_FORMAT_KRGB888

#define  HI_FB_FORMAT_ARGB4444    OT_FB_FORMAT_ARGB4444
#define  HI_FB_FORMAT_ARGB1555    OT_FB_FORMAT_ARGB1555
#define  HI_FB_FORMAT_ARGB8888    OT_FB_FORMAT_ARGB8888
#define  HI_FB_FORMAT_ARGB8565    OT_FB_FORMAT_ARGB8565

#define  HI_FB_FORMAT_RGBA4444    OT_FB_FORMAT_RGBA4444
#define  HI_FB_FORMAT_RGBA5551    OT_FB_FORMAT_RGBA5551
#define  HI_FB_FORMAT_RGBA5658    OT_FB_FORMAT_RGBA5658
#define  HI_FB_FORMAT_RGBA8888    OT_FB_FORMAT_RGBA8888

#define  HI_FB_FORMAT_BGR565      OT_FB_FORMAT_BGR565
#define  HI_FB_FORMAT_BGR888      OT_FB_FORMAT_BGR888
#define  HI_FB_FORMAT_ABGR4444    OT_FB_FORMAT_ABGR4444
#define  HI_FB_FORMAT_ABGR1555    OT_FB_FORMAT_ABGR1555
#define  HI_FB_FORMAT_ABGR8888    OT_FB_FORMAT_ABGR8888
#define  HI_FB_FORMAT_ABGR8565    OT_FB_FORMAT_ABGR8565
#define  HI_FB_FORMAT_KBGR444     OT_FB_FORMAT_KBGR444
#define  HI_FB_FORMAT_KBGR555     OT_FB_FORMAT_KBGR555
#define  HI_FB_FORMAT_KBGR888     OT_FB_FORMAT_KBGR888

#define  HI_FB_FORMAT_1BPP        OT_FB_FORMAT_1BPP
#define  HI_FB_FORMAT_2BPP        OT_FB_FORMAT_2BPP
#define  HI_FB_FORMAT_4BPP        OT_FB_FORMAT_4BPP
#define  HI_FB_FORMAT_8BPP        OT_FB_FORMAT_8BPP
#define  HI_FB_FORMAT_ACLUT44     OT_FB_FORMAT_ACLUT44
#define  HI_FB_FORMAT_ACLUT88     OT_FB_FORMAT_ACLUT88
#define  HI_FB_FORMAT_PUYVY       OT_FB_FORMAT_PUYVY
#define  HI_FB_FORMAT_PYUYV       OT_FB_FORMAT_PYUYV
#define  HI_FB_FORMAT_PYVYU       OT_FB_FORMAT_PYVYU
#define  HI_FB_FORMAT_YUV888      OT_FB_FORMAT_YUV888
#define  HI_FB_FORMAT_AYUV8888    OT_FB_FORMAT_AYUV8888
#define  HI_FB_FORMAT_YUVA8888    OT_FB_FORMAT_YUVA8888
#define  HI_FB_FORMAT_BUTT        OT_FB_FORMAT_BUTT

typedef ot_fb_color_format hi_fb_color_format;

typedef ot_fb_capability hi_fb_capability;

#define  HI_FB_LAYER_BUF_DOUBLE            OT_FB_LAYER_BUF_DOUBLE
#define  HI_FB_LAYER_BUF_ONE               OT_FB_LAYER_BUF_ONE
#define  HI_FB_LAYER_BUF_NONE              OT_FB_LAYER_BUF_NONE
#define  HI_FB_LAYER_BUF_DOUBLE_IMMEDIATE  OT_FB_LAYER_BUF_DOUBLE_IMMEDIATE
#define  HI_FB_LAYER_BUF_BUTT              OT_FB_LAYER_BUF_BUTT
typedef ot_fb_layer_buf hi_fb_layer_buf;

/* surface info */
typedef ot_fb_surface hi_fb_surface;

typedef ot_fb_surfaceex hi_fb_surfaceex;

/* refresh surface info */
typedef ot_fb_buf hi_fb_buf;

/* cursor info */
typedef ot_fb_cursor hi_fb_cursor;

/* DDR detect zone info */
typedef ot_fb_ddr_zone hi_fb_ddr_zone;

/* antiflicker level */
/* Auto means fb will choose a appropriate antiflicker level automatically according to the color info of map */
#define HI_FB_LAYER_ANTIFLICKER_NONE  OT_FB_LAYER_ANTIFLICKER_NONE
#define HI_FB_LAYER_ANTIFLICKER_LOW   OT_FB_LAYER_ANTIFLICKER_LOW
#define HI_FB_LAYER_ANTIFLICKER_MID   OT_FB_LAYER_ANTIFLICKER_MID
#define HI_FB_LAYER_ANTIFLICKER_HIGH  OT_FB_LAYER_ANTIFLICKER_HIGH
#define HI_FB_LAYER_ANTIFLICKER_AUTO  OT_FB_LAYER_ANTIFLICKER_AUTO
#define HI_FB_LAYER_ANTIFLICKER_BUTT  OT_FB_LAYER_ANTIFLICKER_BUTT

typedef ot_fb_layer_antiflicker_level hi_fb_layer_antiflicker_level;

/* mirror mode  */
#define HI_FB_MIRROR_NONE OT_FB_MIRROR_NONE
#define HI_FB_MIRROR_HOR  OT_FB_MIRROR_HOR
#define HI_FB_MIRROR_VER  OT_FB_MIRROR_VER
#define HI_FB_MIRROR_BOTH OT_FB_MIRROR_BOTH
#define HI_FB_MIRROR_BUTT OT_FB_MIRROR_BUTT
typedef ot_fb_mirror_mode hi_fb_mirror_mode;

/* rotate mode */

#define HI_FB_ROTATE_NONE  OT_FB_ROTATE_NONE
#define HI_FB_ROTATE_90    OT_FB_ROTATE_90
#define HI_FB_ROTATE_180   OT_FB_ROTATE_180
#define HI_FB_ROTATE_270   OT_FB_ROTATE_270
#define HI_FB_ROTATE_BUTT  OT_FB_ROTATE_BUTT
typedef ot_fb_rotate_mode hi_fb_rotate_mode;

/* layer info maskbit */
#define HI_FB_LAYER_MASK_BUF_MODE           OT_FB_LAYER_MASK_BUF_MODE
#define HI_FB_LAYER_MASK_ANTIFLICKER_MODE   OT_FB_LAYER_MASK_ANTIFLICKER_MODE
#define HI_FB_LAYER_MASK_POS                OT_FB_LAYER_MASK_POS
#define HI_FB_LAYER_MASK_CANVAS_SIZE        OT_FB_LAYER_MASK_CANVAS_SIZE
#define HI_FB_LAYER_MASK_DISPLAY_SIZE       OT_FB_LAYER_MASK_DISPLAY_SIZE
#define HI_FB_LAYER_MASK_SCREEN_SIZE        OT_FB_LAYER_MASK_SCREEN_SIZE
#define HI_FB_LAYER_MASK_MUL                OT_FB_LAYER_MASK_MUL
#define HI_FB_LAYER_MASK_BUTT               OT_FB_LAYER_MASK_BUTT
typedef ot_fb_layer_info_maskbit hi_fb_layer_info_maskbit;

/* layer info */
typedef ot_fb_layer_info hi_fb_layer_info;

/* smart rect mode  */
#define HI_FB_SMART_RECT_NONE   OT_FB_SMART_RECT_NONE
#define HI_FB_SMART_RECT_SOLID  OT_FB_SMART_RECT_SOLID
#define HI_FB_SMART_RECT_FILLED OT_FB_SMART_RECT_FILLED
#define HI_FB_SMART_RECT_CORNER OT_FB_SMART_RECT_CORNER
#define HI_FB_SMART_RECT_BUTT   OT_FB_SMART_RECT_BUTT
typedef ot_fb_smart_rect_mode hi_fb_smart_rect_mode;

/* smart rect */
typedef ot_fb_smart_rect hi_fb_smart_rect;

typedef ot_fb_smart_rect_param hi_fb_smart_rect_param;
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __HIFB_H__ */

