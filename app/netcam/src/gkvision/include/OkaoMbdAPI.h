/*-------------------------------------------------------------------*/
/*  Copyright(C) 2019-2020 by OMRON Corporation                      */
/*  All Rights Reserved.                                             */
/*                                                                   */
/*   This source code is the Confidential and Proprietary Property   */
/*   of OMRON Corporation.  Any unauthorized use, reproduction or    */
/*   transfer of this software is strictly prohibited.               */
/*                                                                   */
/*-------------------------------------------------------------------*/
#ifndef OKAOMBDAPI_H__
#define OKAOMBDAPI_H__

#define OKAO_API
#include    "CommonDef.h"
#include    "OkaoDef.h"

#ifndef OKAO_DEF_HMBD
#define OKAO_DEF_HMBD
    typedef VOID*    HMBD;
#endif /* OKAO_DEF_HMBD */


typedef struct {
    INT32           nID;            /* Tracking ID      */
    RECT            rcBody;         /* Human Position   */
} MBD_RESULT;

typedef enum {
    MBD_MODE_VL = 0,                /* VL setting  */
    MBD_MODE_IR                     /* IR setting  */
} MBD_MODE;

typedef enum {
    MBD_ANGLE_UP = 0,               /* UP    */
    MBD_ANGLE_RIGHT,                /* Right */
    MBD_ANGLE_DOWN,                 /* Down  */
    MBD_ANGLE_LEFT                  /* Left  */
} MBD_ANGLE;

typedef enum {
    MBD_DETECTABILITY_HIGH = 0,     /* High     */
    MBD_DETECTABILITY_MIDDLE,       /* Middle   */
    MBD_DETECTABILITY_LOW           /* Low      */
} MBD_DETECTABILITY;

#define        MBD_WARNING_NODETECTION      (100)
#define        MBD_WARNING_FRAMERATE        (101)

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************/
/* Version infomation                                     */
/**********************************************************/
OKAO_API INT32      OKAO_MBD_GetVersion(UINT8 *pucMajor, UINT8 *pucMinor);

/**********************************************************/
/* Handle Creation/Deletion/Clear                         */
/**********************************************************/
/* Create/Delete Human Detection Handle */
OKAO_API HMBD       OKAO_MBD_CreateHandle(BOOL bIDTracking);
OKAO_API INT32      OKAO_MBD_DeleteHandle(HMBD hMBD);

/* Clear Human Detection Handle */
OKAO_API INT32      OKAO_MBD_ClearHandle(HMBD hMBD);

/************************************************************/
/* Execute Human Detection Process                          */
/************************************************************/
OKAO_API INT32      OKAO_MBD_Execute(HMBD hMBD, RAWIMAGE *pImage, INT32 nWidth, INT32 nHeight, INT32 *pnNum);

/************************************************************/
/* Get Detection Result                                     */
/************************************************************/
OKAO_API INT32      OKAO_MBD_GetResult(HMBD hMBD, INT32 nIndex, MBD_RESULT *pstResult);

/************************************************************/
/* Set/Get Detection Parameters                             */
/************************************************************/
/* Set/Get Detection Mode */
OKAO_API INT32      OKAO_MBD_SetMode(HMBD hMBD, MBD_MODE eMode);
OKAO_API INT32      OKAO_MBD_GetMode(HMBD hMBD, MBD_MODE *peMode);

/* Set/Get Detection Angle */
OKAO_API INT32      OKAO_MBD_SetAngle(HMBD hMBD, MBD_ANGLE eAngle);
OKAO_API INT32      OKAO_MBD_GetAngle(HMBD hMBD, MBD_ANGLE *peAngle);

/* Set/Get Detectability */
OKAO_API INT32      OKAO_MBD_SetDetectability(HMBD hMBD, MBD_DETECTABILITY eDetectability);
OKAO_API INT32      OKAO_MBD_GetDetectability(HMBD hMBD, MBD_DETECTABILITY *peDetectability);

/* Set/Get Maximum Detection Number of Moving Object */
OKAO_API INT32      OKAO_MBD_SetMaxDetectionCount(HMBD hMBD, INT32 nCount);
OKAO_API INT32      OKAO_MBD_GetMaxDetectionCount(HMBD hMBD, INT32 *pnCount);

#ifdef  __cplusplus
}
#endif

#endif  /* OKAOMBDAPI_H__ */
