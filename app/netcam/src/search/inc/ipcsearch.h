#ifndef __IPC_SEARCH_H_
#define __IPC_SEARCH_H_

#ifndef _HAVE_TYPE_BYTE
#define _HAVE_TYPE_BYTE
	typedef unsigned char       BYTE;
#endif

#ifndef _HAVE_TYPE_Bool
#define _HAVE_TYPE_Bool
typedef unsigned char       Bool;
#endif

#ifndef BOOL
#ifndef _HAVE_TYPE_BOOL
#define _HAVE_TYPE_BOOL
	typedef int					BOOL;
#endif
#endif

#ifndef _HAVE_TYPE_WORD
#define _HAVE_TYPE_WORD
	typedef unsigned short      WORD;
#endif

#ifndef _HAVE_TYPE_DWORD
#define _HAVE_TYPE_DWORD
	typedef unsigned long		DWORD;
	typedef unsigned long		ULONG;
#endif

#ifndef _HAVE_TYPE_ULONGLONG
#define _HAVE_TYPE_ULONGLONG
	typedef unsigned long long		ULONGLONG;
#endif

#ifndef _HAVE_TYPE_LONG
#define _HAVE_TYPE_LONG
	typedef long				LONG;
#endif

#ifndef _HAVE_TYPE_UINT
#define _HAVE_TYPE_UINT
	typedef unsigned int		UINT;
#endif

#ifndef _HAVE_TYPE_VOID
#define _HAVE_TYPE_VOID
	typedef void				VOID;
#endif

#ifndef _HAVE_TYPE_DWORD64
#define _HAVE_TYPE_DWORD64
	typedef unsigned long long  DWORD64;
	typedef unsigned long long	QWORD;
#endif

#ifndef _HAVE_TYPE_UINT8
#define _HAVE_TYPE_UINT8
	typedef unsigned char   	UINT8;
#endif


#ifndef _HAVE_TYPE_UINT16
#define _HAVE_TYPE_UINT16
	typedef unsigned short 		UINT16;
#endif

#ifndef _HAVE_TYPE_UINT32
#define _HAVE_TYPE_UINT32
	typedef unsigned int    	UINT32;
#endif

#ifndef _HAVE_TYPE_HWND
#define _HAVE_TYPE_HWND
	typedef unsigned int 		HWND;
#endif

#ifndef _HAVE_TYPE_WPARAM
#define _HAVE_TYPE_WPARAM
	typedef unsigned int    	WPARAM;
#endif

#ifndef _HAVE_TYPE_LPARAM
#define _HAVE_TYPE_LPARAM
	typedef unsigned long   	LPARAM;
#endif

#ifndef _HAVE_TYPE_SINT64
#define _HAVE_TYPE_SINT64
	typedef signed long long	SINT64;
#endif

#ifndef _HAVE_TYPE_UINT64
#define _HAVE_TYPE_UINT64
	typedef unsigned long long 	UINT64;
#endif

#ifndef _HAVE_TYPE_Int
#define _HAVE_TYPE_Int
	typedef int		Int;
#endif

#ifndef _HAVE_TYPE_Char
#define _HAVE_TYPE_Char
	typedef char	Char;
#endif


typedef struct Dana_info
{
	char Dana_ID[32];
	int 	file_len;
	char Dana_file[4*1024];
}Dana_infoT;


/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/******************************************************************************
 * 函数介绍:  用于监听并处理生成测试工具和批量生产工具
 * 输入参数:  无
 * 输出参数:  无
 * 返回值      : 0   : success
 *                           <0 : error
 *                               v1.0.0    2015.09.09  wangguixing
 *****************************************************************************/
int IPCsearch_init(void);


/******************************************************************************
 * 函数介绍:  退出生产测试程序和生成批量工具
 * 输入参数:  无
 * 输出参数:  无
 * 返回值      : 0   : success
 *                           <0 : error
 *                               v1.0.0    2015.09.09  wangguixing
 *****************************************************************************/
int IPCsearch_exit(void);

/*****************************************************************************
*函数介绍: 添加设备支持P2P类型
*输入参数: P2P 类型名,P2P类型名保存数组,P2P 个数
*输出参数: 无
*返回值	   : 无
*				v1.0.0 2016.03.11	wangpeng
******************************************************************************/
void P2PName_add(int name);


//设置保留分区中wifi信息
int ipc_search_save_to_resave_wifi(char *ssid, char* psd,int enc_type);

int IPCsearch_get_test_result(void *pTestResultInfo);
int IPCsearch_set_test_result(void *pTestResultInfo);
int IPCsearch_get_p2p_info(char deviceID[5][64], char mac[5][64]);
int IPCsearch_set_p2p_info(int P2P_Type, char *deviceID, char *mac);


#endif //__IPC_SEARCH_H_
