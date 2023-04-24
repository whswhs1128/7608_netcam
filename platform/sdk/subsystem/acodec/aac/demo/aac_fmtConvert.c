/*
****************************************************************************
** \file      /solution/software/subsystem/acodec/aac/demo/aac_fmtConvert.c
**
** \version   $Id: aac_fmtConvert.c 3 2017-09-04 09:29:58Z fengxuequan $
**
** \brief     videc abstraction layer header file.
**
** \attention THIS SAMPLE CODE IS PROVIDED AS IS. GOFORTUNE SEMICONDUCTOR
**            ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**            OMMISSIONS.
**
** (C) Copyright 2012-2013 by GOKE MICROELECTRONICS CO.,LTD
**
****************************************************************************
*/


#include "aac.h"
#include "stdio.h"
#include "string.h"

#include <unistd.h>
#include <getopt.h>
//**************************************************************************
//**************************************************************************
//** Local Defines
//**************************************************************************
//**************************************************************************

#define MAXWAVESIZE     4294967040LU

#define OUTPUT_WAV 1
#define OUTPUT_RAW 2

//**************************************************************************
//**************************************************************************
//** Local structures
//**************************************************************************
//**************************************************************************

typedef struct
{
    AAC_CONFIG_FormatEnumT outputFormat;
    FILE *sndfile;
    unsigned int fileType;
    unsigned long samplerate;
    unsigned int channels;
    unsigned long total_samples;
    long channelMask;
} AAC_AudioFileT;

//**************************************************************************
//**************************************************************************
//** Global Data
//**************************************************************************
//**************************************************************************
//**************************************************************************
//**************************************************************************
//** Local Data
//**************************************************************************
//**************************************************************************
//**************************************************************************
//**************************************************************************
//** Local Functions Declaration
//**************************************************************************
//**************************************************************************
//**************************************************************************
//**************************************************************************
//** API Functions
//**************************************************************************
//**************************************************************************
//**************************************************************************
//**************************************************************************
//** Local Functions
//**************************************************************************
//**************************************************************************
static unsigned int get_audio_bits_per(int format)
{
	unsigned int bits_per = 0;
	switch (format)
    {
    case AAC_FMT_16BIT:
        bits_per = 16;
        break;
    case AAC_FMT_24BIT:
        bits_per = 24;
        break;
    case AAC_FMT_32BIT:
    case AAC_FMT_FLOAT:
        bits_per = 32;
        break;
    default:
        return 0;
	}
	return bits_per;
}

static int write_wav_header(AAC_AudioFileT *aufile)
{
    unsigned char header[44];
    unsigned char* p = header;
	unsigned int per_bits = get_audio_bits_per(aufile->outputFormat);
    unsigned int bytes = (per_bits + 7) / 8;
    float data_size = (float)bytes * aufile->total_samples;
    unsigned long word32;

    *p++ = 'R'; *p++ = 'I'; *p++ = 'F'; *p++ = 'F';

    word32 = (data_size + (44 - 8) < (float)MAXWAVESIZE) ?
        (unsigned long)data_size + (44 - 8)  :  (unsigned long)MAXWAVESIZE;
    *p++ = (unsigned char)(word32 >>  0);
    *p++ = (unsigned char)(word32 >>  8);
    *p++ = (unsigned char)(word32 >> 16);
    *p++ = (unsigned char)(word32 >> 24);

    *p++ = 'W'; *p++ = 'A'; *p++ = 'V'; *p++ = 'E';

    *p++ = 'f'; *p++ = 'm'; *p++ = 't'; *p++ = ' ';

    *p++ = 0x10; *p++ = 0x00; *p++ = 0x00; *p++ = 0x00;

    if (aufile->outputFormat == AAC_FMT_FLOAT)
    {
        *p++ = 0x03; *p++ = 0x00;
    } else {
        *p++ = 0x01; *p++ = 0x00;
    }

    *p++ = (unsigned char)(aufile->channels >> 0);
    *p++ = (unsigned char)(aufile->channels >> 8);

    word32 = (unsigned long)(aufile->samplerate + 0.5);
    *p++ = (unsigned char)(word32 >>  0);
    *p++ = (unsigned char)(word32 >>  8);
    *p++ = (unsigned char)(word32 >> 16);
    *p++ = (unsigned char)(word32 >> 24);

    word32 = aufile->samplerate * bytes * aufile->channels;
    *p++ = (unsigned char)(word32 >>  0);
    *p++ = (unsigned char)(word32 >>  8);
    *p++ = (unsigned char)(word32 >> 16);
    *p++ = (unsigned char)(word32 >> 24);

    word32 = bytes * aufile->channels;
    *p++ = (unsigned char)(word32 >>  0);
    *p++ = (unsigned char)(word32 >>  8);

    *p++ = (unsigned char)(per_bits >> 0);
    *p++ = (unsigned char)(per_bits >> 8);

    *p++ = 'd'; *p++ = 'a'; *p++ = 't'; *p++ = 'a';

    word32 = data_size < MAXWAVESIZE ?
        (unsigned long)data_size : (unsigned long)MAXWAVESIZE;
    *p++ = (unsigned char)(word32 >>  0);
    *p++ = (unsigned char)(word32 >>  8);
    *p++ = (unsigned char)(word32 >> 16);
    *p++ = (unsigned char)(word32 >> 24);

	//printf("header_wav\n");
    return fwrite(header, sizeof(header), 1, aufile->sndfile);
}

static int write_wav_extensible_header(AAC_AudioFileT *aufile)
{
	long channelMask = aufile->channelMask;
    unsigned char header[68];
    unsigned char* p = header;
	unsigned int per_bits = get_audio_bits_per(aufile->outputFormat);
    unsigned int bytes = (per_bits + 7) / 8;
    float data_size = (float)bytes * aufile->total_samples;
    unsigned long word32;

	//printf("extensible_header_wav\n");
    *p++ = 'R'; *p++ = 'I'; *p++ = 'F'; *p++ = 'F';

    word32 = (data_size + (68 - 8) < (float)MAXWAVESIZE) ?
        (unsigned long)data_size + (68 - 8)  :  (unsigned long)MAXWAVESIZE;
    *p++ = (unsigned char)(word32 >>  0);
    *p++ = (unsigned char)(word32 >>  8);
    *p++ = (unsigned char)(word32 >> 16);
    *p++ = (unsigned char)(word32 >> 24);

    *p++ = 'W'; *p++ = 'A'; *p++ = 'V'; *p++ = 'E';

    *p++ = 'f'; *p++ = 'm'; *p++ = 't'; *p++ = ' ';

    *p++ = /*0x10*/0x28; *p++ = 0x00; *p++ = 0x00; *p++ = 0x00;

    /* WAVE_FORMAT_EXTENSIBLE */
    *p++ = 0xFE; *p++ = 0xFF;

    *p++ = (unsigned char)(aufile->channels >> 0);
    *p++ = (unsigned char)(aufile->channels >> 8);

    word32 = (unsigned long)(aufile->samplerate + 0.5);
    *p++ = (unsigned char)(word32 >>  0);
    *p++ = (unsigned char)(word32 >>  8);
    *p++ = (unsigned char)(word32 >> 16);
    *p++ = (unsigned char)(word32 >> 24);

    word32 = aufile->samplerate * bytes * aufile->channels;
    *p++ = (unsigned char)(word32 >>  0);
    *p++ = (unsigned char)(word32 >>  8);
    *p++ = (unsigned char)(word32 >> 16);
    *p++ = (unsigned char)(word32 >> 24);

    word32 = bytes * aufile->channels;
    *p++ = (unsigned char)(word32 >>  0);
    *p++ = (unsigned char)(word32 >>  8);

    *p++ = (unsigned char)(per_bits >> 0);
    *p++ = (unsigned char)(per_bits >> 8);

    /* cbSize */
    *p++ = (unsigned char)(22);
    *p++ = (unsigned char)(0);

    /* WAVEFORMATEXTENSIBLE */

    /* wValidBitsPerSample */
    *p++ = (unsigned char)(per_bits >> 0);
    *p++ = (unsigned char)(per_bits >> 8);

    /* dwChannelMask */
    word32 = channelMask;
    *p++ = (unsigned char)(word32 >>  0);
    *p++ = (unsigned char)(word32 >>  8);
    *p++ = (unsigned char)(word32 >> 16);
    *p++ = (unsigned char)(word32 >> 24);

    /* SubFormat */
    if (aufile->outputFormat == AAC_FMT_FLOAT)
    {
        /* KSDATAFORMAT_SUBTYPE_IEEE_FLOAT: 00000003-0000-0010-8000-00aa00389b71 */
        *p++ = 0x03;
        *p++ = 0x00;
        *p++ = 0x00;
        *p++ = 0x00;
        *p++ = 0x00; *p++ = 0x00; *p++ = 0x10; *p++ = 0x00; *p++ = 0x80; *p++ = 0x00;
        *p++ = 0x00; *p++ = 0xaa; *p++ = 0x00; *p++ = 0x38; *p++ = 0x9b; *p++ = 0x71;
    } else {
        /* KSDATAFORMAT_SUBTYPE_PCM: 00000001-0000-0010-8000-00aa00389b71 */
        *p++ = 0x01;
        *p++ = 0x00;
        *p++ = 0x00;
        *p++ = 0x00;
        *p++ = 0x00; *p++ = 0x00; *p++ = 0x10; *p++ = 0x00; *p++ = 0x80; *p++ = 0x00;
        *p++ = 0x00; *p++ = 0xaa; *p++ = 0x00; *p++ = 0x38; *p++ = 0x9b; *p++ = 0x71;
    }

    /* end WAVEFORMATEXTENSIBLE */

    *p++ = 'd'; *p++ = 'a'; *p++ = 't'; *p++ = 'a';

    word32 = data_size < MAXWAVESIZE ?
        (unsigned long)data_size : (unsigned long)MAXWAVESIZE;
    *p++ = (unsigned char)(word32 >>  0);
    *p++ = (unsigned char)(word32 >>  8);
    *p++ = (unsigned char)(word32 >> 16);
    *p++ = (unsigned char)(word32 >> 24);

    return fwrite(header, sizeof(header), 1, aufile->sndfile);
}

int add_wav_header(AAC_AudioFileT *aufile)
{
	if(aufile->fileType == OUTPUT_WAV){
    	if (aufile->channelMask)
        	write_wav_extensible_header(aufile);
    	else
   		    write_wav_header(aufile);
		return 0;
	}
	return -1;
}

static char outfile[256] = {"audio"};
static char filename[256] = {"test"};
static unsigned long samplerates = 44100;
static unsigned int  channels    = 2;
static unsigned int  formate     = 16;
static unsigned int  acodec_flag = 0;
static const char *shortopt = "hdei:o:s:f:c:";
static struct option longopt[] = {
    {"help",       0, 0, 'h'},
    {"decode",     0, 0, 'd'},
    {"encode",     0, 0, 'e'},
    {"fromate",    0, 0, 'f'},
    {"output",     0, 0, 'o'},
    {"samplerate", 0, 0, 's'},
    {"channels",   0, 0, 'c'},
    {"input",      0, 0, 'i'},
    {0,            0, 0, 0}

};

int aac_to_pcm(void)
{
	long ret;
	long filelenth = 0;
	FILE *fd_s, *fd_d;
    AAC_AudioFileT    aufile;
	AAC_DecodeInfoT aac_info ={
		.format           = AAC_FMT_16BIT,//sample precision
		.samplerate       = AAC_SAMPLE_RATE_8000,//samplerate
		.channels         = 0,//default
		.channel_mask     = 0,//ditto
		.real_samplerate  = 0,//ditto
	};
	unsigned char *buf = NULL;
	unsigned char *outbuf = NULL;
	int buf_size = AAC_DEFAULT_BUFFER_SIZE;
	long outbuf_size = 0;
    unsigned char wav_header = 0;
    long filesize = 0;

#ifdef USE_WAV_HEADER
    wav_header = 1;
#endif

	fd_s = fopen(filename, "r");
	if(fd_s ==NULL){
		printf("fopen fd_d error!\n");
		return -1;
	}

	fd_d = fopen(outfile, "w");
	if(fd_d == NULL){
		fclose(fd_s);
		printf("fopen fd_d error!\n");
		return -1;
	}
    fseek(fd_s, 0, SEEK_END);
    filesize = ftell(fd_s);
    fseek(fd_s, 0, SEEK_SET);

    outbuf = malloc(buf_size);
    if(outbuf == NULL) {
        printf("malloc output buffer fail!\n");
        goto exit1;
    }
	memset(outbuf, 0, buf_size);

	buf = malloc(buf_size);
    if(buf == NULL) {
        printf("malloc input buffer fail!\n");
        free(outbuf);
        goto exit1;
    }
	memset(buf, 0, buf_size);
	ret = fread(buf, 1, buf_size, fd_s);

    filelenth += ret;
    aac_info.inbuf.buffer            = buf;
    aac_info.inbuf.bytes_into_buffer = ret;
    aac_info.inbuf.bytes_consumed    = 0;
    aac_info.inbuf.buffer_size       = buf_size;

    aac_info.outbuf.buffer            = outbuf;
    aac_info.outbuf.bytes_into_buffer = 0;
    aac_info.outbuf.bytes_consumed    = 0;
    aac_info.outbuf.buffer_size       = buf_size;

	aac_decode_open(&aac_info);

    printf("Decoding...\n");

	do {
			int pos = aac_decoding(&aac_info);

            if (pos == AAC_NULL) {
                printf("buffer is NULL\n");
                break;
            }
            if (pos == AAC_FLOW) {
                printf("buffer is full or empty!\n");
                if (aac_info.real_samplerate*aac_info.channels > buf_size) {
                    buf_size = aac_info.real_samplerate*aac_info.channels;

                    printf("Out buffer is small, reallocate buffer size [%d byte]!\n", buf_size);

                    outbuf = realloc(outbuf, buf_size);
                    if (outbuf == NULL) {
                        printf("realloc output space [%d byte] failed!\n", buf_size);
                        break;
                    }
                    aac_info.outbuf.buffer = outbuf;
                    aac_info.outbuf.bytes_into_buffer = 0;
                    aac_info.outbuf.bytes_consumed    = 0;
                    aac_info.outbuf.buffer_size       = buf_size;
                } else {
                    break;
                }
            }
			if(wav_header) {
				aufile.channels = aac_info.channels;
				aufile.channelMask = aac_info.channel_mask;
				aufile.samplerate = aac_info.real_samplerate;
				aufile.outputFormat = aac_info.format;
				aufile.fileType = OUTPUT_WAV;
				aufile.total_samples = 0;
				aufile.sndfile = fd_d;

                printf("channels:%d,samplerate:%ld\n",aufile.channels,aufile.samplerate);
				add_wav_header(&aufile);
				wav_header = 0;
			}

			outbuf_size = aac_info.outbuf.bytes_into_buffer;
			if(aac_info.outbuf.bytes_into_buffer > 0){
				int tmp = 0;
				do {
					ret = fwrite(outbuf + tmp, 1, outbuf_size, fd_d);
					outbuf_size -= ret;
					tmp += ret;
				} while(outbuf_size);
                aac_info.outbuf.bytes_into_buffer = 0;
                aac_info.outbuf.bytes_consumed    = 0;
			}
            fprintf(stderr, "\rComplete:%5.1f %%", (float)filelenth/filesize*100);
            fflush(stderr);

            memmove(buf, buf + aac_info.inbuf.bytes_consumed, aac_info.inbuf.bytes_into_buffer);
            aac_info.inbuf.bytes_consumed = 0;

			ret = fread(buf + aac_info.inbuf.bytes_into_buffer, 1,
                aac_info.inbuf.buffer_size - aac_info.inbuf.bytes_into_buffer, fd_s);

            aac_info.inbuf.bytes_into_buffer += ret;
			if (aac_info.inbuf.bytes_into_buffer <= 0){
				printf("read file complied!\n");
				break;
			}

            filelenth += ret;
	}while(1);

	printf("\nDecoded complied!\n");
	printf("Input filesize:%ld\n", filelenth);

	aac_decode_close(&aac_info);
    free(buf);
    free(outbuf);
exit1:
	fclose(fd_s);
	fclose(fd_d);

	return 0;
}


AAC_CONFIG_FormatEnumT get_formate(unsigned int formate)
{
    if(formate == 16)
        return AAC_FMT_16BIT;
    else if(formate == 24)
        return AAC_FMT_24BIT;
    else if(formate == 32)
        return AAC_FMT_32BIT;
    else
        return AAC_FMT_NULL;
}

AAC_CONFIG_SampleRateEnumT get_samplerate(unsigned long samp)
{
    if(samp <= 8000)
        return AAC_SAMPLE_RATE_8000;
    else if (samp <= 11025)
        return AAC_SAMPLE_RATE_11025;
    else if (samp <= 12000)
        return AAC_SAMPLE_RATE_12000;
    else if (samp <= 16000)
        return AAC_SAMPLE_RATE_16000;
    else if (samp <= 22050)
        return AAC_SAMPLE_RATE_22050;
    else if (samp <= 24000)
        return AAC_SAMPLE_RATE_24000;
    else if (samp <= 32000)
        return AAC_SAMPLE_RATE_32000;
    else if (samp <= 44100)
        return AAC_SAMPLE_RATE_44100;
    else if (samp <= 48000)
        return AAC_SAMPLE_RATE_48000;
    else if (samp <= 64000)
        return AAC_SAMPLE_RATE_64000;
    else if (samp <= 88200)
        return AAC_SAMPLE_RATE_88200;
    else if (samp <= 96000)
        return AAC_SAMPLE_RATE_96000;
    else
        return AAC_SAMPLE_RATE_DEFAULT;

}

int pcm_to_aac(void)
{
	int ret;
	long filelenth = 0;
	FILE *fd_s, *fd_d;
	unsigned char *inbuf = NULL;
	unsigned char *outbuf= NULL;
	long buf_lenth = 0;
    long outbuf_size = 0;
	long inbuf_lenth;
    long filesize = 0;

	AAC_EncodeInfoT encode_info;

    memset(&encode_info, 0, sizeof(encode_info));
    encode_info.samplerates = get_samplerate(samplerates);
    encode_info.samplebits  = get_formate(formate);
    encode_info.channels    = channels;
    encode_info.quantqual   = 100;

	fd_s = fopen(filename, "r");
	if(fd_s ==NULL){
		printf("fopen fd_d error!\n");
		return -1;
	}

	fd_d = fopen(outfile, "w");
	if(fd_d == NULL){
		fclose(fd_s);
		printf("fopen fd_d error!\n");
		return -1;
	}

	printf("Open encoder\n");
	ret = aac_encode_open(&encode_info);
	if (ret < 0) {
		printf("open encode fail\n");
		return -1;
	}

    fseek(fd_s, 0, SEEK_END);
    filesize = ftell(fd_s);
    fseek(fd_s, 0, SEEK_SET);

	inbuf_lenth = encode_info.min_framesize;
	inbuf = malloc(inbuf_lenth);
	if(inbuf == NULL){
		printf("malloc error!  [size:%ld}\n", inbuf_lenth);
		return -1;
	}
	memset(inbuf, 0, inbuf_lenth);

    outbuf_size = encode_info.aac_maxbytes;
	outbuf = malloc(outbuf_size);
	if(outbuf == NULL){
		free(inbuf);
		printf("malloc error!  [size:%ld}\n", outbuf_size);
		return -1;
	}
	memset(outbuf, 0, outbuf_size);

	ret = fread(inbuf, 1, inbuf_lenth, fd_s);
    filelenth += ret;

    encode_info.inbuf.buffer            = inbuf;
    encode_info.inbuf.buffer_size       = inbuf_lenth;
    encode_info.inbuf.bytes_into_buffer = ret;
    encode_info.inbuf.bytes_consumed    = 0;

    encode_info.outbuf.buffer            = outbuf;
    encode_info.outbuf.buffer_size       = outbuf_size;
    encode_info.outbuf.bytes_into_buffer = 0;
    encode_info.outbuf.bytes_consumed    = 0;

	printf("Start encode...\n");
	do {
		if(aac_encoding(&encode_info) < 0){
			printf("This frame encode fail\n");
		}
        /* move vaild data */
        if(encode_info.inbuf.bytes_into_buffer) {
	    	memmove(encode_info.inbuf.buffer,
				encode_info.inbuf.buffer + encode_info.inbuf.bytes_consumed,
				encode_info.inbuf.bytes_into_buffer);
    	}
        encode_info.inbuf.bytes_consumed = 0;//clean consumed

		buf_lenth = encode_info.outbuf.bytes_into_buffer;

		if(buf_lenth > 0){
			ret = fwrite(outbuf, 1, buf_lenth, fd_d);
            encode_info.outbuf.bytes_into_buffer = 0;
            encode_info.outbuf.bytes_consumed = 0;
		}

        fprintf(stderr, "\rComplete:%5.1f %%", (float)filelenth/filesize*100);
        fflush(stderr);

		ret = fread(inbuf + encode_info.inbuf.bytes_into_buffer, 1,
                encode_info.inbuf.buffer_size - encode_info.inbuf.bytes_into_buffer,fd_s);
        encode_info.inbuf.bytes_into_buffer += ret;

        filelenth += ret;

		if (encode_info.inbuf.bytes_into_buffer <= 0)
			break;
	}while(1);

	printf("\nEncoded complied!\n");
	printf("Input filesize:%ld\n", filelenth);

	aac_encode_close(&encode_info);
    free(inbuf);
    free(outbuf);
	fclose(fd_s);
	fclose(fd_d);

	return 0;
}

void usage(char *progname)
{
    printf("Usage:\n");
    printf("\t-o <outfile> output file\n");
    printf("\t-i <infile> input file\n");
    printf("\t-d run decode program\n");
    printf("\t-e run encode program\n");
    printf("\t-f <bits> pcm sample bits\n");
    printf("\t-s <samplerate> samplerate\n");
    printf("\t-c <channels> channel num(mono: 1, stereo: 2)\n");
    printf("\t-h help\n");
    printf("Example:\n");
    printf("\t%s -d -i <infile> -o <outfile>\n", progname);
    printf("\t%s -e -i <infile> -o <outfile> -c 1 -f 16 -s 8000\n", progname);
}

void opt_handle(int argc, char *argv[])
{
    int option;
    int ch;
    while((ch = getopt_long(argc, argv, shortopt, longopt, &option)) != -1)
    {
        switch(ch)
        {
        case 'o':
            strcpy(outfile, optarg);
            printf("outfile:%s\n",optarg);
            break;
        case 'i':
            strcpy(filename, optarg);
            printf("filename:%s\n",optarg);
            break;
        case 'd':
            acodec_flag = 1;
            printf("flag = 1\n");
            break;
        case 'e':
            acodec_flag = 2;
            printf("flag = 2\n");
            break;
        case 'f':
            formate = atoi(optarg);
            printf("formate:%s\n",optarg);
            break;
        case 'h':
            usage(argv[0]);
            break;
        case 's':
            samplerates = atoi(optarg);
            printf("samplerate:%s\n",optarg);
            break;
        case 'c':
            channels   = atoi(optarg);
            printf("channels:%s\n",optarg);
            break;
        }
    }

}

int main(int argc, char *argv[])
{
    opt_handle(argc, argv);
    if (acodec_flag == 1){
       aac_to_pcm();
    } else if (acodec_flag == 2){
        pcm_to_aac();
    }

    return 0;
}
