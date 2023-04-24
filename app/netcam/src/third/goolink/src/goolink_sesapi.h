/**
 * goolink_sesapi.h
 *
 * External APIs for device side implementation.
 *
 * Created by Morningstar Shen at 20141014
 */

#ifndef __GOO_LINK_SESAPI_H__
#define __GOO_LINK_SESAPI_H__

#ifdef __cplusplus
extern "C" {
#endif


#include<stdint.h>
#include<inttypes.h>

/*****************************************************************************
 							通明通道接口
 *****************************************************************************/

/*	轮询session接口
 * 	msTimeOut：	 轮询超时，单位ms，如果设为0，则调用一直阻塞，知道有新的session创建
 * 	retutn : 0--失败   >0--成功，session id	*/
int glnk_ses_poll(int msTimeOut);


/*	关闭session接口
 * 	hConnectionID：	 	glnk_ses_poll的返回值
 * 	retutn : 0--失败   >0--成功*/
int glnk_ses_close(int hConnectionID);


/*	读取session接口
 * 	hConnectionID：	 	glnk_ses_poll的返回值
 * 	pReadBuf：			读数据缓存
 * 	nBufLen：			读取的数据长度
 * 	msTimeOut：			超时时间
 * 	retutn : 			0--失败，连接中断，调用glnk_ses_close清理session资源
 * 			   			>0--成功，实际读取的数据长度
 * 			   			-1--读数据超时		*/
int glnk_ses_read(int hConnectionID, char *pReadBuf, int nBufLen, int msTimeOut);

/*	发送session接口
 * 	hConnectionID：	 	glnk_ses_poll的返回值
 * 	pWriteBuf：			写入的数据
 * 	nBufLen：			写入的数据长度
 * 	msTimeOut：			超时时间
 * 	retutn : 			0--失败，连接中断，调用glnk_ses_close清理session资源
 * 			   			>0--成功，实际写入的数据长度
 * 			   			-1--写入数据超时		*/
int glnk_ses_write(int hConnectionID, char *pWriteBuf, int nWriteLen, int msTimeOut);



#ifdef __cplusplus
}
#endif

#endif // __GOO_LINK_API_H__

