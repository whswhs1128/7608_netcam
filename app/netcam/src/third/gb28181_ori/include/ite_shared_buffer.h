
#ifndef ITE_SHARED_BUFFER_H
#define ITE_SHARED_BUFFER_H

#define H264_SHM_BUFF_SIZE 256  			
#define ITE_BITS_CACHE_BUFFER_COUNT 256 	
#define DATA_TYPE_H264   1 					
#define SEM_SLOT_INDEX_USER1 1 				
#define SEMOP_TIMEOUT_MS 5 					
#define MEDIA_DATA_NON_EMPTY 1 				

typedef struct ITE_SHARED_BUFFER_INFO
{
	int key;
	int size;
	int cnt;
} ITE_SHARED_BUFFER_INFO_T;

typedef struct ITE_SHARED_BUFFER_PROCMGR_STATE
{
	int ncodetype;
	int nindex;
	int nmilliseconds;
	int ptpayload;
	int npayloadlen;	
} ITE_SHARED_BUFFER_PROCMGR_STATE_T;

typedef enum MSHARE_KEY_T
{
	ITE_BITS_0_MSHARE_KEY = 0,
	ITE_BITS_1_MSHARE_KEY,
	ITE_BITS_2_MSHARE_KEY,
}MSHARE_KEY;

#endif //ITE_SHARED_BUFFER_H
