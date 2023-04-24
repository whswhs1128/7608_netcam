/*******************************************************************************
       Copyright: 
        Filename:  
     Description: 
         Version: 
         Created:  
          Author:
******************************************************************************/

#ifndef _CRYPTOMISC_H
#define _CRYPTOMISC_H

//#include "./rsa/rsaeuro.h"

#ifdef  __cplusplus
extern "C" {
#endif

int encryptMagic(char *in, char *out, int in_len);

int encryptDes(char *pInput, int iInputMaxLen, int iInputLen, char *pOutput, int iOutputMaxLen, int *iOutputLen);
int decryptDes(char *pInput, int iInputLen, char *pOutput, int iOutputMaxLen, int *iOutputLen);

/* int RSA_GeneratePEMKeys(R_RSA_PUBLIC_KEY *publicKey, R_RSA_PRIVATE_KEY *privateKey, R_RSA_PROTO_KEY *protoKey); */

#ifdef  __cplusplus
}
#endif
#endif	/* _CRYPTOMISC_H */

