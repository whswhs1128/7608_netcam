#ifndef _MTWS_BASE64_H_
#define _MTWS_BASE64_H_

#ifdef __cplusplus
extern "C" {
#endif


int mtws_base64_encode(unsigned char *source, unsigned int sourcelen, char *target,unsigned int targetlen);
int mtws_base64_decode(const char *source, unsigned char *target, unsigned int targetlen);


#ifdef __cplusplus
}
#endif



#endif /* _MTWS_BASE64_H_ */
