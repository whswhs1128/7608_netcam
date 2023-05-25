/*
 * Copyright (c) Hunan Goke,Chengdu Goke,Shandong Goke. 2021. All rights reserved.
 */

#ifndef __PQ_BIN_H__
#define __PQ_BIN_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define BIN_NULL_POINT    0xCB000001
#define BIN_REG_ATTR_ERR  0xCB000002
#define BIN_MALLOC_ERR    0xCB000003
#define BIN_CHIP_ERR      0xCB000004
#define BIN_CRC_ERR       0xCB000005
#define BIN_SIZE_ERR      0xCB000006
#define BIN_LEBLE_ERR     0xCB000007
#define BIN_DATA_ERR      0xCB000008
#define BIN_GET_MPI_FAILED   0xCB000009
#define BIN_SECURITY_SOLUTION_FAILED 0xCB00000A

typedef struct PQ_BIN_ISP_S {
    int enable;
} PQ_BIN_ISP_S;

typedef struct PQ_BIN_NRX_S {
    int enable;
    int viPipe;
    int vpssGrp;
} PQ_BIN_NRX_S;

typedef struct PQ_BIN_MODULE_S {
    PQ_BIN_ISP_S stISP;
    PQ_BIN_NRX_S st3DNR;
} PQ_BIN_MODULE_S;

/*****************************************************************************
*   Prototype    : PQ_GetISPDataTotalLen
*   Description: : Get length of isp module data
*   Input          : void
*   Output        : void
*   Return Value : the length of isp module data
*****************************************************************************/
unsigned int PQ_GetISPDataTotalLen(void);

/*****************************************************************************
*   Prototype    : PQ_GetStructParamLen
*   Description: : Get length of isp module data
*   Input          : pstBinParam: set isp and nrx params
*   Output        : void
*   Return Value : the length of nrx params
*****************************************************************************/
unsigned int PQ_GetStructParamLen(PQ_BIN_MODULE_S *pstBinParam);

/*****************************************************************************
*   Prototype    : PQ_BIN_ExportBinData
*   Description: : Get bin data from buffer
*   Input          : pstBinParam:set isp and nrx params
*                       pu8Buffer:save bin data
*                      u32DataLength:length of bin data
*   Output       : 
*   Return Value : 0: Success;
*              Error codes: 0xCB000001:input para is null
*                               0xCB000003:malloc error
*                               0xCB000004:chipid not match
*                               0xCB000005:crc error
*                               0xCB000008:data error
*                               0xCB00000A:security solution failed
*****************************************************************************/
int PQ_BIN_ExportBinData(PQ_BIN_MODULE_S *pstBinParam, unsigned char *pu8Buffer, unsigned int u32DataLength);

/*****************************************************************************
*   Prototype    : PQ_BIN_ImportBinData
*   Description: : set bin data from buffer
*   Input        : pstBinParam:set isp and nrx params
*                     pu8Buffer:save bin data
*                    u32DataLength:length of bin data
*   Output       : 
*   Return Value : 0: Success;
*              Error codes: 0xCB000001:input para is null
*                               0xCB000003:malloc error
*                               0xCB000008:data erro
*                               0xCB00000A:security solution failed
*****************************************************************************/
int PQ_BIN_ImportBinData(PQ_BIN_MODULE_S *pstBinParam, unsigned char *pu8Buffer, unsigned int u32DataLength);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* End of #ifndef __PQ_BIN_H__ */
