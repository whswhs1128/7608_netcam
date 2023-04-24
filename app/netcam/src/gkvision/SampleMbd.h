/*-------------------------------------------------------------------*/
/*  Copyright(C) 2020 by OMRON Corporation                           */
/*  All Rights Reserved.                                             */
/*                                                                   */
/*   This source code is the Confidential and Proprietary Property   */
/*   of OMRON Corporation.  Any unauthorized use, reproduction or    */
/*   transfer of this software is strictly prohibited.               */
/*                                                                   */
/*-------------------------------------------------------------------*/

/* Sample Code for Human Detection for Fixed CameraV2.0 */

#ifndef SAMPLEMBD_H__
#define SAMPLEMBD_H__

#include "CommonDef.h"
#include "OkaoDef.h"
#include "OkaoMbdAPI.h"

#include "OkaoAPI.h"
#include "OkaoCoAPI.h"
#include "OkaoCoStatus.h"

#ifdef  __cplusplus
extern "C" {
#endif

INT32 InitializeMbd(HMBD *hMBD);

INT32 TerminateMbd(HMBD hMBD);

INT32 ExecuteMbd(HMBD hMBD, RAWIMAGE *pImage, INT32 nWidth, INT32 nHeight, INT32 *pnCount, MBD_RESULT *pstResult);

INT32 SetParametersMbd(HMBD hMBD);

INT32 InitComFuncT(VOID *pBMemoryAddrV4, UINT32 unBMemorySizeV4,VOID *pWMemoryAddr, UINT32 unWMemorySize);

INT32 TerminateCo();

UINT32 GetRequiredBMemSizeMbd();

UINT32 GetRequiredWMemSizeMbd();



#ifdef  __cplusplus
}
#endif

#endif /* SAMPLEMBD_H__ */
