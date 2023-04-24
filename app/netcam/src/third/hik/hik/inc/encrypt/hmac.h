/*******************************************************************************
       Copyright: 
        Filename: 
     Description: 
         Version: 
         Created: 
          Author:  
******************************************************************************/

#ifndef _HMAC_H
#define _HMAC_H

#ifdef  __cplusplus
extern "C" {
#endif

void encryptHMAC(unsigned char *in, int in_len,     /* input string */
                unsigned char *key, int key_len,    /* input key */
                unsigned char digest[16]);          /* result */

#ifdef  __cplusplus
}
#endif
#endif	/* _HMAC_H */

