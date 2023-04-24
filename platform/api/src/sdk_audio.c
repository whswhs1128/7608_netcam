/*!
*****************************************************************************
** \file        gk7101_audio.c
**
** \version
**
** \brief       sdk audio control
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2014-2015 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#include "sdk_debug.h"
#include "sdk_audio.h"
#include "sdk.h"
#include "adi_types.h"
//#include "adi_i2s.h"
#include "adi_audio.h"		//xqq

#ifdef AUDIO_ECHO_CANCELLATION_SUPPORT
#include "webrtc_aec.h"
#include "audio_process.h"


static GADI_S32 speexAecHandle;

#if 0
#include "gk_aec.h"

static GADI_S32 aec_init(GADI_S32 *ptrAecHandle)
{

    GADI_AUDIO_EchoCancellationT speex_aec;

    sprintf(speex_aec.aecName, "speexdsp");
    speex_aec.aecInit = gk_aec_init;
    speex_aec.aecDeinit = gk_aec_deinit;
    speex_aec.aecProcess = gk_aec_process;
    return gadi_audio_register_echo_cancellation(ptrAecHandle, &speex_aec);
}

static GADI_S32 aec_exit(GADI_S32 aecHandle)
{

    return gadi_audio_unregister_echo_cancellation(aecHandle);
}
#endif
#endif

#if 0
static unsigned int FM1188INIT[] = {
0x1E300339,
0x1E340009,
0x1E3E0100,
0x1E3D0300,
0x1E410101,
0x1E440081,
0x1E4503DE,
0x1E460074,
0x1E472800,
0x1E483000,
0x1E4D0200,
0x1E51C000,
0x1E5C0000,
0x1E630002,
0x1E86000A,
0x1E870006,
0x1E8B0180,
0x1E8C0040,
0x1E927800,
0x1E952000,
0x1EA01000,
0x1EA12000,
0x1EA22000,
0x1EBD7020,
0x1EBE7010,
0x1EBF7000,
0x1EC00800,
0x1EC10A00,
0x1EC21000,
0x1ECA0400,
0x1EE21400,
0x1EE30600,
0x1EE40600,
0x1F044100,
0x1F107FFF,
0x1F116050,
0x1F124C9F,
0x1F133F82,
0x1F14363E,
0x1F152EB2,
0x1F162711,
0x1F171EF8,
0x1F1817CB,
0x1F1912F0,
0x1F283000,
0x1F294000,
0x1F2A2000,
0x1F2B4800,
0x1F4B0000,
0x1E3A0000,
0xFFFFFFFF
};

//
// 配置回音消除芯片
//
static int gk_audio_init_fm1188(){
	//GPIO 37: SDA
	//GPIO 38: SCL
    printf("======= gk_audio_init_fm1188 =======\n");
    #if 0
	GADI_CHAR nodePath[128] = "/dev/i2c-1";
	GADI_I2S_OpenParam I2CSet;	
	I2CSet.nodePath = nodePath;
	I2CSet.slaveAddr = 0xC0;

	GADI_ERR Err;
	GADI_SYS_HandleT Handle = gadi_i2c_open(&Err,&I2CSet);
	
	if(Handle == NULL){
		printf("======= GADI I2C OPEN FAILED:[%d] =======\n",Err);
		return  1;
		//exit(0);
	}
	
	// CO FC F3 3B
	int  i = 0;
	unsigned char Set[4] = {0};
	unsigned char Get[4] = {0};
	char SetCmd[4] = {0xFC,0xF3,0x3B};
	char GetCmd[3][4] = {
		{0xFC,0xF3,0x37,0x00},
		{0xFC,0xF3,0x60,0x25},
		{0xFC,0xF3,0x60,0x26}
	};
	
	for(i=0;FM1188INIT[i]!=0xFFFFFFFF;i++){
		Set[3] = (FM1188INIT[i] >>  0) & 0x000000FF;
		Set[2] = (FM1188INIT[i] >>  8) & 0x000000FF;
		Set[1] = (FM1188INIT[i] >> 16) & 0x000000FF;
		Set[0] = (FM1188INIT[i] >> 24) & 0x000000FF;

		char Dat[8] = {0};
		memcpy(&Dat[0],SetCmd,3);
		memcpy(&Dat[3],Set,4);
		
		printf("I2C SET DATA:[%02X][%02X][%02X][%02X][%02X][%02X][%02X][%02X].\n",
			Dat[0],Dat[1],Dat[2],Dat[3],Dat[4],Dat[5],Dat[6],Dat[7]
			);
		
		int Err = gadi_i2c_write(Handle,Dat,7);
		
		if(Err != GADI_OK){
			printf("I2C WRITE TO FM1188 FAILED.\n");
			printf("I2C WRITE TO FM1188 FAILED.\n");
			printf("I2C WRITE TO FM1188 FAILED.\n");
			printf("I2C WRITE TO FM1188 FAILED.\n");
			printf("I2C WRITE TO FM1188 FAILED.\n");
			break;
		}
	}

	gadi_i2c_close(Handle);
    #endif
    return 0;
}

#endif


//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************
#ifdef AUDIO_ECHO_CANCELLATION_SUPPORT
static int ap_aec_init(int sample_rate, int frame_samples);
static int ap_aec_process(void *ai_data, void *ao_data, void *aec_data);
static int ap_aec_deinit(void);
int ap_aec_register(int *ptrAecHandle)
{

    GADI_AUDIO_EchoCancellationT webrtc_aec;

    sprintf(webrtc_aec.aecName, "ap");
    webrtc_aec.aecInit = ap_aec_init;
    webrtc_aec.aecDeinit = ap_aec_deinit;
    webrtc_aec.aecProcess = ap_aec_process;
    return gadi_audio_register_echo_cancellation(ptrAecHandle, &webrtc_aec);
}

int ap_aec_unregister(int aecHandle)
{

    return gadi_audio_unregister_echo_cancellation(aecHandle);
}


static int ap_aec_init(int sample_rate, int frame_samples) 
{
    audio_process_attribute audio_process_attr;

    audio_process_attr.sampleRate = sample_rate;
    audio_process_attr.frameSamples = frame_samples;
    audio_process_attr.aecMode = AUDIO_PROCESS_AEC_FLOAT;
    audio_process_attr.agcEnable = AUDIO_PROCESS_TRUE;//AUDIO_PROCESS_TRUE;
    audio_process_attr.nsEnable = AUDIO_PROCESS_TRUE;//AUDIO_PROCESS_TRUE;
    audio_process_attr.nsParam= 2;
    audio_process_attr.aecFixedParam = 3;
    audio_process_attr.aecFloatParam = 0;
    audio_process_attr.agcParam= 9;

    return audio_process_init(&audio_process_attr);
}

static int ap_aec_process(void *ai_data, void *ao_data, void *aec_data)
{ 
    return audio_process_proc(ai_data, ao_data, aec_data);
  
}

int ap_aec_deinit(void) 
{       
    return audio_process_deinit();
}
#endif
/*****************************************************************************/
int gk_audio_init()
{
    LOG_INFO("enter gk_audio_init.\n");
//    GADI_ERR retVal = gadi_audio_init();	xqq
     GADI_ERR retVal;
    if(retVal != GADI_OK){
        LOG_ERR("gadi_audio_init fail\n");
        return -1;
    } else {
        LOG_INFO("gadi_audio_init ok\n");
    }

#ifdef AUDIO_ECHO_CANCELLATION_SUPPORT
    #if 0
    retVal = aec_init(&speexAecHandle);
    if(retVal != GADI_OK){
        LOG_ERR("aec_init fail\n");
        return -1;
    } else {
        LOG_INFO("aec_init ok\n");
    }
    #else
	retVal = ap_aec_register(&speexAecHandle);
    if(retVal != GADI_OK){
        LOG_ERR("webrtc_aec_register fail\n");
        return -1;
    } else {
        LOG_INFO("webrtc_aec_register ok\n");
    }
    #endif
#else
    #if 0
    retVal = gk_audio_init_fm1188();
    if(retVal != 0){
        LOG_ERR("gk_audio_init_fm1188 fail\n");
        return -1;
    }
    #endif
#endif

    return 0;
}

int gk_audio_exit()
{
    LOG_INFO("enter gk_audio_exit\n");
    int retVal;

#ifdef AUDIO_ECHO_CANCELLATION_SUPPORT
    #if 0
    retVal = aec_exit(speexAecHandle);
    if(retVal != GADI_OK){
        LOG_ERR("aec_exit fail\n");
    } else {
        LOG_INFO("aec_exit ok\n");
    }
    #else
	retVal = ap_aec_unregister(speexAecHandle);
    if(retVal != GADI_OK){
        LOG_ERR("webrtc_aec_unregister fail\n");
    } else {
        LOG_INFO("webrtc_aec_unregister ok\n");
    }
    #endif
#endif

    //retVal = gadi_audio_exit();	xqq

     if(retVal != GADI_OK){
        LOG_ERR("gadi_audio_exit fail\n");
    } else {
        LOG_INFO("gadi_audio_exit ok\n");
    }

    return retVal;
}


int gk_audio_ai_init(GADI_AUDIO_AioAttrT *ptrAttr, GADI_AUDIO_SampleFormatEnumT sample_format, GADI_AUDIO_GainLevelEnumT glevel)
{
    GADI_S32 ai_fd = -1;

    //GADI_AUDIO_SampleFormatEnumT sample_format;
    //GADI_AUDIO_AioAttrT aio_attr;
    //GADI_U32 frame_size;
    //GADI_BOOL flag = GADI_FALSE;
    int retVal;

    /* set audio sample format*/
    //sample_format = GADI_AUDIO_SAMPLE_FORMAT_RAW_PCM;
    //ai_fd = gadi_audio_ai_get_fd();	xqq
    if (ai_fd < 0) {
        LOG_ERR("get ai fd failed \n");
        return -1;
    }
//    retVal = gadi_audio_set_sample_format(ai_fd, sample_format);	xqq
    if (retVal){
        LOG_ERR("Set ai sample format failed \n");
        return -1;
    }

    /* set AI dev attr */
//    retVal = gadi_audio_ai_set_attr(ptrAttr);	xqq
    if (retVal){
        LOG_ERR("Set ai attribute failed \n");
        return -1;
    }

    /* set AI gain */
//    retVal = gadi_audio_ai_set_gain(&glevel);	xqq
    if (retVal){
        LOG_ERR("Set ai set gain failed \n");
        return -1;
    }


    /* enable AI*/
#if 0
    if (gadi_audio_ai_enable()){
        LOG_ERR("Enable ai device failed \n");
        return -1;
    }

#ifdef AUDIO_ECHO_CANCELLATION_SUPPORT
    if(gadi_audio_ai_aec_enable()) {
        LOG_ERR("Enable ai aec failed \n");
        return -1;
    }
#endif
#endif		//xqq
    return 0;
}

int gk_audio_ai_exit()
{
#ifdef AUDIO_ECHO_CANCELLATION_SUPPORT
    //gadi_audio_ai_aec_disable();xqq
#endif

//    gadi_audio_ai_disable();	xqq
    return 0;
}

int gk_audio_ao_init(GADI_AUDIO_AioAttrT *ptrAttr, GADI_AUDIO_SampleFormatEnumT sample_format, GADI_AUDIO_VolumeLevelEnumT volumeLevel)
{
    GADI_S32 ao_fd = -1;
    //unsigned int  count  = 0;
    //GADI_AUDIO_AioFrameT audio_frame;
    
    int retVal = 0;

    /* set audio sample format*/
//    ao_fd = gadi_audio_ao_get_fd();	xqq
    if (ao_fd < 0) {
        LOG_ERR("get ao fd failed \n");
        return -1;
    }

//    retVal = gadi_audio_set_sample_format(ao_fd, sample_format);	xqq
    if (retVal){
        LOG_ERR("Set ao sample format failed \n");
        return -1;
    }

    /* set AO dev attr */
//    retVal = gadi_audio_ao_set_attr(ptrAttr);	xqq
    if (retVal){
        LOG_ERR("Set ao attribute failed \n");
        return -1;
    }

//    retVal = gadi_audio_ao_set_volume(&volumeLevel);	xqq
    if(retVal != GADI_OK){
        LOG_ERR("gadi_audio_ao_set_volume fail\n");
        return -1;
    }

    /* enable AO*/
#if 0
    if (gadi_audio_ao_enable()){
        LOG_ERR("Enable ao device failed \n");
        return -1;
    }
#endif		//xqq
    return 0;
}

int gk_audio_ao_exit()
{
    //gadi_audio_ao_disable();	xqq
    return 0;
}

static int get_volume_value(GADI_AUDIO_VolumeLevelEnumT volumeLevel)
{
    int value = -1;

    switch(volumeLevel)
    {
        case VLEVEL_0:
            value = 0;
            break;
        case VLEVEL_1:
            value = 1;
            break;
        case VLEVEL_2:
            value = 2;
            break;
        case VLEVEL_3:
            value = 3;
            break;
        case VLEVEL_4:
            value = 4;
            break;
        case VLEVEL_5:
            value = 5;
            break;
        case VLEVEL_6:
            value = 6;
            break;
        case VLEVEL_7:
            value = 7;
            break;
        case VLEVEL_8:
            value = 8;
            break;
        case VLEVEL_9:
            value = 9;
            break;
        case VLEVEL_10:
            value = 10;
            break;
        case VLEVEL_11:
            value = 11;
            break;
        case VLEVEL_12:
            value = 12;
            break;
        default:
            ;
    }

    return value;
}

GADI_AUDIO_VolumeLevelEnumT get_volume_level(int value)
{
    GADI_AUDIO_VolumeLevelEnumT volume_level;

    if(value > 12) {
        value = 12;
    }else if (value < 0) {
        value = 0;
    }

    switch(value)
    {
        case 0:
            volume_level = VLEVEL_0;
            break;
        case 1:
            volume_level = VLEVEL_1;
            break;
        case 2:
            volume_level = VLEVEL_2;
            break;
        case 3:
            volume_level = VLEVEL_3;
            break;
        case 4:
            volume_level = VLEVEL_4;
            break;
        case 5:
            volume_level = VLEVEL_5;
            break;
        case 6:
            volume_level = VLEVEL_6;
            break;
        case 7:
            volume_level = VLEVEL_7;
            break;
        case 8:
            volume_level = VLEVEL_8;
            break;
        case 9:
            volume_level = VLEVEL_9;
            break;
        case 10:
            volume_level = VLEVEL_10;
            break;
        case 11:
            volume_level = VLEVEL_11;
            break;
        case 12:
            volume_level = VLEVEL_12;
            break;
        default:
            volume_level = VLEVEL_8;
            break;
    }

    return volume_level;
}

GADI_AUDIO_GainLevelEnumT get_gain_level(int value)
{
    GADI_AUDIO_GainLevelEnumT volume_level;

    if(value >= GLEVEL_MAX) {
        value = GLEVEL_MAX-1;
    }else if (value < GLEVEL_0) {
        value = GLEVEL_0;
    }

    switch(value)
    {
        case 0:
            volume_level = GLEVEL_0;
            break;
        case 1:
            volume_level = GLEVEL_1;
            break;
        case 2:
            volume_level = GLEVEL_2;
            break;
        case 3:
            volume_level = GLEVEL_3;
            break;
        case 4:
            volume_level = GLEVEL_4;
            break;
        case 5:
            volume_level = GLEVEL_5;
            break;
        case 6:
            volume_level = GLEVEL_6;
            break;
        case 7:
            volume_level = GLEVEL_7;
            break;
        case 8:
            volume_level = GLEVEL_8;
            break;
        case 9:
            volume_level = GLEVEL_9;
            break;
        case 10:
            volume_level = GLEVEL_10;
            break;
        case 11:
            volume_level = GLEVEL_11;
            break;
        case 12:
            volume_level = GLEVEL_12;
            break;
        case 13:
            volume_level = GLEVEL_13;
            break;
        case 14:
            volume_level = GLEVEL_14;
            break;
        case 15:
            volume_level = GLEVEL_15;
            break;
        default:
            volume_level = GLEVEL_8;
            break;
    }

    return volume_level;
}

int gk_audio_ao_set_volume(int value)
{
    int retVal;
    GADI_AUDIO_VolumeLevelEnumT volumeLevel = get_volume_level(value);
//    retVal = gadi_audio_ao_set_volume(&volumeLevel);	xqq
    if(retVal != GADI_OK){
        LOG_ERR("gadi_audio_ao_set_volume fail\n");
        return -1;
    }
    return 0;
}

int gk_audio_ao_get_volume()
{
    int retVal;
    GADI_AUDIO_VolumeLevelEnumT volumeLevel;
//    retVal = gadi_audio_ao_get_volume(&volumeLevel);	xqq
    if(retVal != GADI_OK) {
        LOG_ERR("gadi_audio_ao_get_volume fail\n");
        return -1;
    }
    int value = get_volume_value(volumeLevel);
    LOG_INFO("volume level:%d Lv\n", value);
    return value;
}


int gk_audio_aout_play(unsigned char *audio_buffer, unsigned int lenth, bool flag)
{
	int retVal = 0;
	unsigned char *ap=audio_buffer;
	int number = 0;
	int outPutLen = 0;
	GADI_AUDIO_AioFrameT frame;
	//not support ai when aout
	//gadi_audio_ai_disable();

	// enable AO

	// play
	while(lenth > 0)
	{
	    //最后一帧不满足320B，直接舍弃
	    if (lenth < AUDIO_FRAMESAMPLES*2)
            break;
		outPutLen = AUDIO_FRAMESAMPLES*2;
    
		frame.virAddr = ap;
		frame.len = outPutLen;

		number = 0;
		while(number < 30)
		{
#if 0
#ifdef AUDIO_ECHO_CANCELLATION_SUPPORT
            retVal = gadi_audio_ao_send_frame_aec(&frame, GADI_TRUE);
#else
            retVal = gadi_audio_ao_send_frame(&frame, GADI_TRUE);
#endif
#endif		//xqq
			if(retVal == 0)
			{
				ap += outPutLen;
				lenth -= outPutLen;
				break;
			}
			else
			{
				number++;
			}
		}
		if(number == 30)
		{
			LOG_ERR("audio output play error\n");
			break;
		}
	}


	//not support ai when aout
	//gadi_audio_ai_enable();

	return retVal;
}


int gk_audio_input_enable(bool flag)
{
    int ret;

    if(flag == 1)
    {
//        ret = gadi_audio_ai_enable();	xqq
        if(ret != GADI_OK)
			LOG_ERR("gadi_audio_ai_enable fail\n");
		
#ifdef AUDIO_ECHO_CANCELLATION_SUPPORT
		ret = gadi_audio_ai_aec_enable();
		if(ret != GADI_OK)
			LOG_ERR("gadi_audio_ai_aec_enable fail\n");
#endif
    }
    else
    {
#ifdef AUDIO_ECHO_CANCELLATION_SUPPORT
		ret = gadi_audio_ai_aec_disable();
		if(ret != GADI_OK)
			LOG_ERR("gadi_audio_ai_aec_disable fail\n");
#endif
//        ret = gadi_audio_ai_disable();	xqq
        if(ret != GADI_OK)
			LOG_ERR("gadi_audio_ai_disable fail\n");
    }

    return ret;
}

int gk_audio_output_enable(bool flag)
{
    int ret;

    if(flag)
    {
        //ret = gadi_audio_ao_enable();	xqq
        if(ret != GADI_OK)
			LOG_ERR("gadi_audio_ao_enable fail\n");
    }
    else
    {
//        ret = gadi_audio_ao_disable();	xqq
        if(ret != GADI_OK)
			LOG_ERR("gadi_audio_ao_disable fail\n");
    
    }

    return ret;
}

