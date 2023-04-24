/*******************************************************************************
       Copyright: 2007-2008 Hangzhou Hikvision Digital Technology Co.,Ltd
        Filename:  base64.h
     Description:  head file of Base64
         Version:  1.0
         Created:  01/02/2008 00 CST
          Author:  xmq <xiemq@hikvision.com>
******************************************************************************/

#ifndef _HIK_BASE64_H
#define _HIK_BASE64_H

#ifdef  __cplusplus
extern "C" {
#endif

int hik_base64_encode(const char *in_str, int in_len, char *out_str);
int hik_base64_decode(const char *in_str, int in_len, char *out_str);

#ifdef  __cplusplus
}
#endif
#endif	/* _HIK_BASE64_H */

