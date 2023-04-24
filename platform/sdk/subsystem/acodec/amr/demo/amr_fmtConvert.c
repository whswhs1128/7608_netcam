/*
****************************************************************************
** \file      /solution/software/subsystem/acodec/aac/demo/aac_fmtConvert.c
**
** \version   $Id: amr_fmtConvert.c 3 2017-09-04 09:29:58Z fengxuequan $
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


#include "interf_enc.h"
#include "interf_dec.h"
#include "stdio.h"
#include "string.h"
#include <stdlib.h>
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
    int outputFormat;
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
static int write_wav_header(AAC_AudioFileT *aufile)
{
    unsigned char header[44];
    unsigned char* p = header;
	unsigned int per_bits = aufile->outputFormat;
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

    if (aufile->outputFormat == 32)
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
	unsigned int per_bits = aufile->outputFormat;
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
    if (aufile->outputFormat == 32)
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
static const int sizes[] = { 12, 13, 15, 17, 19, 20, 26, 31, 5, 6, 5, 5, 0, 0, 0, 0 };
static const union {char c[4]; unsigned long l;} endian_test = {'l', '?', '?', 'b'};

#define ENDIANNESS  ((char)endian_test.l)
#define MAR_FRAME_SIZE(n)   (sizes[((n) >> 3) & 0x0f])
int amr_to_pcm(void)
{
	long ret;
	long filelenth = 0;
	FILE *fd_s, *fd_d;
    AAC_AudioFileT    aufile;
	unsigned char *buf = NULL;
	unsigned short *outbuf = NULL;
	unsigned char *littlebuf = NULL;
    unsigned char *ptr = NULL;
	int buf_size = 500;
	int outbuf_size = 160;
    unsigned char wav_header = 0;
    long filesize = 0;
    void *amr = NULL;
    char header[6];
    int size = 0;
    int i;

#ifdef USE_WAV_HEADER
    wav_header = 1;
#endif
    /* check output file suffix */
    if (wav_header == 1) {
        if(strstr(outfile, ".wav") == NULL)
            strcat(outfile, ".wav");
    } else {
        if(strstr(outfile, ".pcm") == NULL)
            strcat(outfile, ".pcm");
    }

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

    /* create output buffer */
    outbuf = calloc(sizeof(outbuf[0]), outbuf_size);
    if(outbuf == NULL) {
        printf("malloc output buffer fail!\n");
        goto exit1;
    }

    /* ceate input buffer */
	buf = calloc(sizeof(buf[0]), buf_size);
    if(buf == NULL) {
        printf("malloc input buffer fail!\n");
        free(outbuf);
        goto exit1;
    }

    /* ceate little to big buffer */
	littlebuf = calloc(sizeof(littlebuf[0]), outbuf_size * sizeof(outbuf[0]));
    if(littlebuf == NULL) {
        printf("malloc input buffer fail!\n");
        free(outbuf);
        free(buf);
        goto exit1;
    }

    ret = fread(header, 1, 6, fd_s);
    if (ret != 6 || memcmp(header, "#!AMR\n", 6))
    {
        fprintf(stderr, "Bad header\n");
        return 1;
    }

    if(wav_header) {
        aufile.channels = 1;
        aufile.channelMask = 0;
        aufile.samplerate = 8000;
        aufile.outputFormat = 16;
        aufile.fileType = OUTPUT_WAV;
        aufile.total_samples = 0;
        aufile.sndfile = fd_d;

        printf("channels:%d,samplerate:%ld\n",aufile.channels,aufile.samplerate);
        add_wav_header(&aufile);
        wav_header = 0;
    }

    amr = Decoder_Interface_init();

    printf("Decoding...\n");
	do {
        /* Read the mode byte */
        ret = fread(buf, 1, 1, fd_s);
        if (ret <= 0)
            break;
        filelenth += ret;

        /* Find the packet size */
        size = MAR_FRAME_SIZE(buf[0]);

        ret = fread(buf + 1, 1, size, fd_s);
        if (ret != size)
        {
            break;
        }
        filelenth += ret;

        /* Decode the packet */
        Decoder_Interface_Decode(amr, buf, outbuf, 0);

		/* if ARM is little endian,  Convert to little endian and write to wav */
        if(ENDIANNESS == 'l') {
            ptr = littlebuf;
            for (i = 0; i < outbuf_size; i++) {
                *ptr++ = (outbuf[i] >> 0) & 0xff;
                *ptr++ = (outbuf[i] >> 8) & 0xff;
            }
        }

        fwrite(littlebuf, 1, outbuf_size * sizeof(outbuf[0]), fd_d);
	}while(1);

	printf("\nDecoded complied!\n");
	printf("Input filesize:%ld\n", filelenth);

	Decoder_Interface_exit(amr);
    free(buf);
    free(outbuf);
    free(littlebuf);
exit1:
	fclose(fd_s);
	fclose(fd_d);

	return 0;
}


int pcm_to_amr(void)
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
    /* requested mode */
    enum Mode req_mode = MR122;
    int dtx = 0;

	/* pointer to encoder state structure */
    int *enstate;
    /* counters */
    int byte_counter;

    /* check output file suffix */
    if(strstr(outfile, ".amr") == NULL)
       strcat(outfile, ".amr");

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
    enstate = Encoder_Interface_init(dtx);

    fseek(fd_s, 0, SEEK_END);
    filesize = ftell(fd_s);
    fseek(fd_s, 0, SEEK_SET);

	inbuf_lenth = 160 * 2;
	inbuf = malloc(inbuf_lenth);
	if(inbuf == NULL){
		printf("malloc error!  [size:%ld}\n", inbuf_lenth);
		return -1;
	}
	memset(inbuf, 0, inbuf_lenth);

    outbuf_size = 160 * 2;
	outbuf = malloc(outbuf_size);
	if(outbuf == NULL){
		free(inbuf);
		printf("malloc error!  [size:%ld}\n", outbuf_size);
		return -1;
	}
	memset(outbuf, 0, outbuf_size);

	ret = fread(inbuf, 1, inbuf_lenth, fd_s);
    filelenth += ret;

    #define AMR_MAGIC_NUMBER "#!AMR\n"
    /* write magic number to indicate single channel AMR file storage format */
    fwrite(AMR_MAGIC_NUMBER, sizeof(char), strlen(AMR_MAGIC_NUMBER), fd_d);
	printf("Start encode...\n");
	do {
        /* call encoder */
        byte_counter = Encoder_Interface_Encode(enstate, req_mode, inbuf, outbuf, 0);

		ret = fwrite(outbuf, 1, byte_counter, fd_d);
        fprintf(stderr, "\rComplete:%5.1f %%", (float)filelenth/filesize*100);

		ret = fread(inbuf, 1, inbuf_lenth,fd_s);
        filelenth += ret;
		if (ret < 160*2)
			break;
	}while(1);

	printf("\nEncoded complied!\n");
	printf("Input filesize:%ld\n", filelenth);

	Encoder_Interface_exit(enstate);
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
    printf("\t-h help\n");
    printf("Example:\n");
    printf("\t%s -d -i <infile> -o <outfile>\n", progname);
    printf("\t%s -e -i <infile> -o <outfile>\n", progname);
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
        case 'h':
            usage(argv[0]);
            break;
        }
    }

}

int main(int argc, char *argv[])
{
    opt_handle(argc, argv);
    if (acodec_flag == 1){
        amr_to_pcm();
    } else if (acodec_flag == 2){
        pcm_to_amr();
    }

    return 0;
}
