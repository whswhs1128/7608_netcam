
#include "sdk_def.h"
#include <stdint.h>
#include <stdbool.h>

#ifndef SDK_AUDIO_H_
#define SDK_AUDIO_H_
#ifdef __cplusplus
extern "C" {
#endif

#define AUDIO_FRAMESAMPLES		(160)
#define GK_AUDIO_G711A_FRAME_LEN      (0xF0)	//xqq

typedef enum SDK_AUDIO_HW_SPEC {
	GK_AUDIO_HW_SPEC_IGNORE = 0,
	//
}EM_GK_AUDIO_HW_SPEC;

#if 0
typedef enum {
    GK_AUDIO_SAMPLE_RATE_8000   = 8000,    /* 8K samplerate*/
    GK_AUDIO_SAMPLE_RATE_11025  = 11025,   /* 11.025K samplerate*/
    GK_AUDIO_SAMPLE_RATE_16000  = 16000,   /* 16K samplerate*/
    GK_AUDIO_SAMPLE_RATE_22050  = 22050,   /* 22.050K samplerate*/
    GK_AUDIO_SAMPLE_RATE_24000  = 24000,   /* 24K samplerate*/
    GK_AUDIO_SAMPLE_RATE_32000  = 32000,   /* 32K samplerate*/
    GK_AUDIO_SAMPLE_RATE_44100  = 44100,   /* 44.1K samplerate*/
    GK_AUDIO_SAMPLE_RATE_48000  = 48000,   /* 48K samplerate*/
    GK_AUDIO_SAMPLE_RATE_BUTT,
} GK_AUDIO_SAMPLE_RATE_E;
#endif

typedef enum GK_AUDIO_SAMPLE_RATE {
    GK_AUDIO_SAMPLE_RATE_ATUO   = 0,       /* Auto samplerate*/
    GK_AUDIO_SAMPLE_RATE_8000   = 8000,    /* 8K samplerate*/
    GK_AUDIO_SAMPLE_RATE_11025  = 11025,   /* 11.025K samplerate*/
    GK_AUDIO_SAMPLE_RATE_16000  = 16000,   /* 16K samplerate*/
    GK_AUDIO_SAMPLE_RATE_22050  = 22050,   /* 22.050K samplerate*/
    GK_AUDIO_SAMPLE_RATE_24000  = 24000,   /* 24K samplerate*/
    GK_AUDIO_SAMPLE_RATE_32000  = 32000,   /* 32K samplerate*/
    GK_AUDIO_SAMPLE_RATE_44100  = 44100,   /* 44.1K samplerate*/
    GK_AUDIO_SAMPLE_RATE_48000  = 48000,   /* 48K samplerate*/

}EM_GK_AUDIO_SAMPLE_RATE;

/*!
*******************************************************************************
** \brief audio sound channel mode enum.
*******************************************************************************
*/
typedef enum {
#if 0
	GK_AUDIO_SOUND_MODE_MONO = 0,
	GK_AUDIO_SOUND_MODE_STEREO = 1,
	GK_AUDIO_SOUND_MODE_NUM,
#endif
	/*!left sound mode.*/
    GK_AUDIO_SOUND_MODE_LEFT = 0,
    /*!right sound mode.*/
    GK_AUDIO_SOUND_MODE_RIGHT = 1,
    /*!stereo sound mode.*/
    GK_AUDIO_SOUND_MODE_STEREO = 2,
    /*!mono sound mode.*/
    GK_AUDIO_SOUND_MODE_MONO = 3,
    /*!single sound mode.*/
    GK_AUDIO_SOUND_MODE_SINGLE = 4,
    /*!sound mode number.*/
    GK_AUDIO_SOUND_MODE_NUM,
//xqq
}EM_GK_AUDIO_SOUNDMODE;

/*!
*******************************************************************************
** \brief audio sample precision enum.
*******************************************************************************
*/
typedef enum {
	GK_AUDIO_BIT_WIDTH_16 = 0,
	GK_AUDIO_BIT_WIDTH_NUM,
}EM_GK_AUDIO_BITWIDTH;

/*!
*******************************************************************************
** \brief audio sample date format enum.
*******************************************************************************
*/
typedef enum {
    /** Raw-Pcm */
    GK_AUDIO_SAMPLE_FORMAT_RAW_PCM,
	/** Mu-Law */
	GK_AUDIO_SAMPLE_FORMAT_MU_LAW,
	/** A-Law */
	GK_AUDIO_SAMPLE_FORMAT_A_LAW,
	GK_AUDIO_SAMPLE_FORMAT_AAC,	//xqq
}EM_GK_AUDIO_SAMPLEFORMAT;

/*!
*******************************************************************************
** \brief audio AO device volume level.
*******************************************************************************
*/
typedef enum
{
    GK_VLEVEL_0 = 0xaf,
    GK_VLEVEL_1 = 0xad,
    GK_VLEVEL_2 = 0xab,
    GK_VLEVEL_3 = 0xa9,
    GK_VLEVEL_4 = 0xa7,
    GK_VLEVEL_5 = 0xa5,
    GK_VLEVEL_6 = 0xa3,
    GK_VLEVEL_7 = 0xa1,
    GK_VLEVEL_8 = 0xa0,
    GK_VLEVEL_9 = 0xbe,
    GK_VLEVEL_10 = 0xb9,
    GK_VLEVEL_11 = 0xb4,
    GK_VLEVEL_12 = 0xb0,
}EM_GK_AUDIO_VOLUMELEVEL;

typedef enum
{
    /*! AI volume gain: 0.*/
    GK_GLEVEL_0 = 0,
    /*! AI volume gain: 1.*/
    GK_GLEVEL_1,
    /*! AI volume gain: 2.*/
    GK_GLEVEL_2,
    /*! AI volume gain: 3.*/
    GK_GLEVEL_3,
    /*! AI volume gain: 4.*/
    GK_GLEVEL_4,
    /*! AI volume gain: 5.*/
    GK_GLEVEL_5,
    /*! AI volume gain: 6.*/
    GK_GLEVEL_6,
    /*! AI volume gain: 7.*/
    GK_GLEVEL_7,
    /*! AI volume gain: 8.*/
    GK_GLEVEL_8,
    /*! AI volume gain: 9.*/
    GK_GLEVEL_9,
    /*! AI volume gain: 10.*/
    GK_GLEVEL_10,
    /*! AI volume gain: 11.*/
    GK_GLEVEL_11,
    /*! AI volume gain: 12.*/
    GK_GLEVEL_12,
    /*! AI volume gain: 13.*/
    GK_GLEVEL_13,
    /*! AI volume gain: 14.*/
    GK_GLEVEL_14,
    /*! AI volume gain: 15.*/
    GK_GLEVEL_15,
    /*! AI volume gain max.*/
    GK_GLEVEL_MAX,
}EM_GK_AUDIO_GainLevelEnumT;	//xqq


typedef struct
{
    /*! audio sample rate.*/
    EM_GK_AUDIO_SAMPLE_RATE            sampleRate;
    /*! audio sample precision.*/
    EM_GK_AUDIO_BITWIDTH              bitWidth;
    /*! audio sound channel mode.*/
    EM_GK_AUDIO_SOUNDMODE             soundMode;
    /*! audio samples per frame.*/
    SDK_U32                              frameSamples;
    /*! audio frame buffer number.*/
    SDK_U32                              frameNum;

}ST_GK_AUDIO_AioAttrT;		//xqq


#ifdef __cplusplus
};
#endif
#endif //SDK_AUDIO_H_


