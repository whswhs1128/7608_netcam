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

#include "SampleMbd.h"

/**
 * Create Handle
 * @param [out] hMBD				Human Detection handle
 * @return Error code
 */
INT32 InitializeMbd(HMBD *hMBD)
{
	/* Creates the Human Detection handle */
	*hMBD = OKAO_MBD_CreateHandle(FALSE);
	if(*hMBD == NULL){
		return OKAO_ERR_ALLOCMEMORY;
	}

	return OKAO_NORMAL;
}

/**
 * Delete Handle
 * @param [in] hMBD			Human Detection handle
 * @return Error code
 */
INT32 TerminateMbd(HMBD hMBD)
{
	INT32 nRet = OKAO_ERR_VARIOUS;

	/* Deletes the Human Detection handle */
	if(hMBD != NULL){
		nRet = OKAO_MBD_DeleteHandle(hMBD);
	}

	return nRet;
}

/**
 * Execute and Get Results
 * @param [in] hMBD				Human Detection handle
 * @param [in] pImage			Image data
 * @param [in] nWidth			Image width
 * @param [in] nHeight			Image height
 * @param [out] pnCount			Result count
 * @param [out] pstResult		Result infos
 * @return Error code
 */
INT32 ExecuteMbd(HMBD hMBD, RAWIMAGE *pImage, INT32 nWidth, INT32 nHeight, INT32 *pnCount, MBD_RESULT *pstResult)
{
	INT32 nRet = OKAO_ERR_VARIOUS;
	INT32 nI;

	/* Executes Human detection */
	nRet = OKAO_MBD_Execute(hMBD, pImage, nWidth, nHeight, pnCount);
	if((nRet != OKAO_NORMAL) && (nRet != MBD_WARNING_FRAMERATE)){
		return nRet;
	}

	/* Gets the detection result for each human */
	for(nI = 0; nI < *pnCount; nI++){
		nRet = OKAO_MBD_GetResult(hMBD, nI, &pstResult[nI]);
		if((nRet != OKAO_NORMAL) && (nRet != MBD_WARNING_NODETECTION)){
			return nRet;
		}
	}

	return nRet;
}

/**
 * Setting parameters
 * @param [in] hMBD		Human Detection handle
 * @return Error code
 */
INT32 SetParametersMbd(HMBD hMBD)
{
	INT32 nRet = OKAO_ERR_VARIOUS;

	/* Set detection mode */
	nRet = OKAO_MBD_SetMode(hMBD, MBD_MODE_VL);
	if(nRet != OKAO_NORMAL){
		return nRet;
	}

	/* Set detection angle */
	nRet = OKAO_MBD_SetAngle(hMBD, MBD_ANGLE_UP);
	if(nRet != OKAO_NORMAL){
		return nRet;
	}

	/* Set detectability */
	nRet = OKAO_MBD_SetDetectability(hMBD, MBD_DETECTABILITY_HIGH);
	if(nRet != OKAO_NORMAL){
		return nRet;
	}

	/* Set max detection count */
	nRet = OKAO_MBD_SetMaxDetectionCount(hMBD, 10);
	if(nRet != OKAO_NORMAL){
		return nRet;
	}

	return nRet;
}

/**
 * Get Backup Memory Size
 * @return Backup memory size
 */
UINT32 GetRequiredBMemSizeMbd()
{
	return (210 + 670) * 1024;
}

/**
 * Get Work Memory Size
 * @return Working memory size
 */
UINT32 GetRequiredWMemSizeBd()
{
	return 64 * 1024;
}
/*
* init common function
*/

INT32 InitComFuncT(VOID *pBMemoryAddrV4, UINT32 unBMemorySizeV4,VOID *pWMemoryAddr, UINT32 unWMemorySize)
{
    INT32 nRet = OKAO_ERR_VARIOUS;
    nRet = OKAO_SetBMemoryArea(pBMemoryAddrV4, unBMemorySizeV4);
	if(nRet != OKAO_NORMAL){
		return nRet;
	}

	nRet = OKAO_SetWMemoryArea(pWMemoryAddr, unWMemorySize);
	if(nRet != OKAO_NORMAL){
		return nRet;
	}

	return nRet;

}
/*
*deinit common function
*/
INT32 TerminateCo()
{

	INT32 nRet = OKAO_ERR_VARIOUS;
	//nRet = OKAO_FreeBMemoryArea();
	if(nRet != OKAO_NORMAL){
		return nRet;
	}

	//nRet = OKAO_FreeWMemoryArea();
	if(nRet != OKAO_NORMAL){
		return nRet;
	}

	return nRet;

}
