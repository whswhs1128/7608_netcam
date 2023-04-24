/*!
*****************************************************************************
** \file        gk7101.c
**
** \version
**
** \brief       GK7101 porting.
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2014-2015 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _GK7101_H_
#define _GK7101_H_

//#include "sdk_sys.h"
#include "sdk_enc.h"

#include "adi_types.h"
#include "adi_sys.h"
#include "adi_vi.h"
#include "adi_vout.h"
#include "adi_venc.h"
#include "adi_vpss.h"
#include "adi_pda.h"
#include "adi_audio.h"
#include "adi_isp.h"
#include "adi_gpio.h"
#include "adi_osd.h"

//*****************************************************************************
//*****************************************************************************
//** Local Defines
//*****************************************************************************
//*****************************************************************************



//*****************************************************************************
//*****************************************************************************
//** Local structures
//*****************************************************************************
//*****************************************************************************
/*encode stream parameters.*/
typedef struct
{
    ST_GK_ENC_STREAM_H264_ATTR h264Attr;
    GADI_VENC_StrAttr          streamFormat;
    GADI_VENC_H264QpConfigT    h264QpConf;		//v2.1.0
    GADI_VENC_H265QpConfigT    h265QpConf;
    unsigned char              dptz;
    int                        bufId;
    bool                       enFlag;
}goke_encode_stream_params;


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



//*****************************************************************************
//*****************************************************************************
//** Local Functions Declaration
//*****************************************************************************
//*****************************************************************************



//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************

#ifdef __cplusplus
extern "C" {
#endif
int gk_get_max_stream_nums(void);
int gk_set_max_stream_nums(unsigned char strNums);
goke_encode_stream_params* gk_get_enc_stream_par(unsigned char strId);
int gk_isp_init_start(void);
int gk_vin_reset_cover(void);

int gk_get_enc_stream_updated(void);
void  gk_clear_enc_stream_updated(void);

int gk_gpio_set_valu(int pin, int value);
int gk_isp_led_on(void);
int gk_isp_led_off(void);
void start_gpio_reset_timer(void);

int new_system_call(const char *cmdline);
int sdk_venc_bps_limit(int const enc_width, int const enc_height, int const enc_fps, int const enc_bps);

int gk_enc_set_flip(uint8_t enable);
int gk_enc_set_mirror(uint8_t enable);

// for sdk internal call.
int gk_osd_set_mirror(int enable);
int gk_osd_set_flip(int enable);

//audio
int gk_audio_init();
int gk_audio_exit();
int gk_audio_ai_init(GADI_AUDIO_AioAttrT *ptrAttr, GADI_AUDIO_SampleFormatEnumT sample_format, GADI_AUDIO_GainLevelEnumT glevel);
int gk_audio_ai_exit();
int gk_audio_ao_init(GADI_AUDIO_AioAttrT *ptrAttr, GADI_AUDIO_SampleFormatEnumT sample_format, GADI_AUDIO_VolumeLevelEnumT volumeLevel);
int gk_audio_ao_exit();
int gk_audio_ao_set_volume(int value);
int gk_audio_ao_get_volume();
int gk_audio_aout_play(unsigned char *audio_buffer, unsigned int lenth, bool flag);


int gk_adjust_bps(int stream_id, int bps);
int gk_adjust_fps(int stream_id, int fps);
int gk_venc_set_h264_qp_config(GADI_VENC_H264QpConfigT *pQpConfig);




#ifdef __cplusplus
}
#endif

#endif /* _GK7101_H_ */
