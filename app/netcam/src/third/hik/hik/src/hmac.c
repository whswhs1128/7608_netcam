/*******************************************************************************

 ******************************************************************************/
#include <string.h>

#include "hik_md5.h"

#include "hmac.h"


/*******************************************************************************
 *         Name: encryptHMAC 
 *  Description: 使用HMAC加密算法对数据进行加密 
 * 
 *       Mode   Type                Name         Description
 * -----------------------------------------------------------------------------
 *        in:   unsigned char *     in          原始输入数据
 *              int                 in_len      输入数据长度
 *              unsigned char *     key         加密使用的密钥
 *              int                 ken_len     密钥的长度
 *    in-out:   无 
 *       out:   unsigned char *     digest      加密后的结果输出
 *    return:   void
 ******************************************************************************/
void encryptHMAC(unsigned char *in, int in_len, unsigned char *key, int key_len, unsigned char *digest)
{
    HIK_MD5_CTX context;
    unsigned char k_ipad[65];    /* inner padding - key XORd with ipad*/
    unsigned char k_opad[65];    /* outer padding - key XORd with opad*/
    unsigned char tk[16] = {0};
    int i;
	
    /* if key is longer than 64 bytes reset it to key=HIK_MD5(key) */
    if(key_len > 64)
    {
        HIK_MD5_CTX      tctx;

        HIK_MD5Init(&tctx);
        HIK_MD5Update(&tctx, key, key_len);
        HIK_MD5Final(tk, &tctx);

        key = tk;
        key_len = 16;
    }
	
	/*
	* the HMAC_HIK_MD5 transform looks like:
	*
	* HIK_MD5(K XOR opad, HIK_MD5(K XOR ipad, text))
	*
	* where K is an n byte key
	* ipad is the byte 0x36 repeated 64 times
    * opad is the byte 0x5c repeated 64 times
    * and text is the data being protected
	*/
	
    /* start out by storing key in pads */
    memset(k_ipad, 0, sizeof(k_ipad));
    memset(k_opad, 0, sizeof(k_opad));
    memcpy(k_ipad, key,key_len);
    memcpy(k_opad, key,key_len);
	
    /* XOR key with ipad and opad values */
    for(i=0; i<64; i++)
    {
        k_ipad[i] ^= 0x36;
        k_opad[i] ^= 0x5c;
    }

    /*
    * perform inner HIK_MD5
    */
    HIK_MD5Init(&context);                     /* init context for 1st pass */
    HIK_MD5Update(&context, k_ipad, 64);       /* start with inner pad */
    HIK_MD5Update(&context, in, in_len);   /* then text of datagram */
    HIK_MD5Final(digest, &context);            /* finish up 1st pass */
	
    /* 
    * perform outer HIK_MD5
    */
    HIK_MD5Init(&context);                     /* init context for 2nd pass */
    HIK_MD5Update(&context, k_opad, 64);       /* start with outer pad */
    HIK_MD5Update(&context, digest, 16);       /* then results of 1st hash */
    HIK_MD5Final(digest, &context);            /* finish up 2nd pass */
}





