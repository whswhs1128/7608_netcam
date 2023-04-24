#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "faad.h"
#include "aac.h"

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



//*****************************************************************************
//*****************************************************************************
//** Global Data
//*****************************************************************************
//*****************************************************************************


short			m_outBuff[AVCODEC_MAX_AUDIO_FRAME_SIZE];
unsigned long	m_outLen;
unsigned long	m_samplerate;
char			m_channels;

//*****************************************************************************
//*****************************************************************************
//** Local Data
//*****************************************************************************
//*****************************************************************************
static NeAACDecHandle	m_hAac;
static int				m_init;

//*****************************************************************************
//*****************************************************************************
//** Local Functions Declaration
//*****************************************************************************
bool CAACDecoder_Init()
{
    m_init = 0;
    m_hAac = NeAACDecOpen();
    
    NeAACDecConfigurationPtr conf = NeAACDecGetCurrentConfiguration(m_hAac);
    conf->defSampleRate = 8000;
    conf->defObjectType = 2;
    conf->outputFormat = 1;
    conf->dontUpSampleImplicitSBR = 1;
    NeAACDecSetConfiguration(m_hAac, conf);
    
    return true;
}

bool CAACDecoder_UnInit()
{
    if (m_hAac != NULL)
    {
        NeAACDecClose(m_hAac);
    }
    m_hAac = NULL;
    m_init =0;
        
    return true;
}

bool CAACDecoder_PushAudioFrame(unsigned int u32Handle,char *audioFrameData,unsigned int len, char is_end)
{    

    int ret = 0;
    NeAACDecFrameInfo hInfo;

    if (m_init == 0)
    {       
        ret = NeAACDecInit (m_hAac, (unsigned char *) audioFrameData, len, &m_samplerate, &m_channels);
        if (ret<0)
		{
            printf("NeAACDecInit false\n");
            return false;
        }
        m_init = 1;
    }

    m_outLen =0;
    unsigned char *p = (unsigned char *)audioFrameData;  
        
    do
    {

        void* out = NeAACDecDecode (m_hAac, &hInfo, p, len);
        if ((hInfo.error == 0) && (hInfo.samples > 0))
        {
            p += hInfo.bytesconsumed;
            len -= hInfo.bytesconsumed;

            memcpy(((char*)m_outBuff) + m_outLen, out, hInfo.samples*2);
            m_outLen += hInfo.samples*2;
                        
			if ( m_outLen >= sizeof(m_outBuff))
			{
				printf("pcm buff overflow\n");
				return false;
			}
        }
        else if (hInfo.error != 0)
        {
            ret = -1;
            break;
        }
    }while (len > 0);

    return true;
}

