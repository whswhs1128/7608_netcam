#ifndef _GK_ADPCM_H__
#define _GK_ADPCM_H__

#ifdef __cplusplus
extern "C"
{
#endif

void adpcm_encode(unsigned char * raw, int len, unsigned char * encoded, int * pre_sample, int * index);
void adpcm_decode(unsigned char * raw, int len, unsigned char * decoded, int * pre_sample, int * index);

#ifdef __cplusplus
}
#endif
#endif