#ifndef __GK_DB_H__
#define __GK_DB_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "gkcommon.h"

/*! OK. */
#define GV_OK                              (0)
/*! Bad parameter passed. */
#define GV_ERR_BAD_PARAMETER               (-1)
/*! Memory allocation failed. */
#define GV_ERR_OUT_OF_MEMORY               (-2)
/*! Device already initialised. */
#define GV_ERR_ALREADY_INITIALIZED         (-3)
/*! Device not initialised. */
#define GV_ERR_NOT_INITIALIZED             (-4)

#define GV_MD_ROI_MAX       (64)
#define GV_MD_LINE_MAX      (4)
#define GV_BD_MAX           (4)

typedef struct
{
    GV_Rect rect;
    GADI_INT score;
    GADI_INT id;
}GV_Detect;

typedef struct
{
    /* 1:use gmd, 0:not use gmd */
    GADI_INT useGmd;
    /* width of gmd */
    GADI_INT gmdWidth;
    /* height of gmd */
    GADI_INT gmdHeight;
    /* width of YUV420 */
    GADI_INT yuvWidth;
    /* height of YUV420 */
    GADI_INT yuvHeight;
    /* body detect enable */
    GADI_INT bdEnable;
    /* 1:enable track, 0:disable track */
    GADI_INT trackEnable;
    /* body detect threshold */
    GADI_INT bdThreshold;
    /* frame diff threshold */
    GADI_INT diffThreshold;
    /* max detect number */
    GADI_INT detectMax;
}GV_InitParam;

typedef enum 
{
    GV_LOG_LEVEL_ERROR = 0,
    GV_LOG_LEVEL_DEBUG,
    GV_LOG_LEVEL_INFO,
    GV_LOG_LEVEL_NUM,
} GV_LogLevelEnumT;

typedef enum 
{
    GV_MD_ALARM_LINE = 0,
    GV_MD_ALARM_ROI,
    GV_MD_ALARM_NUM,
} GV_MD_AlarmEnumT;

typedef struct
{
    GADI_INT roiAlarm[GV_MD_ROI_MAX];
    GADI_INT lineAlarm[GV_MD_LINE_MAX];
    GADI_INT motionAlarm;
    GADI_INT maxMotionCnt;
    GV_Rect  maxMotionRect;
    GADI_INT detectCnt;
    GV_Detect detectInfo[GV_BD_MAX];
}GV_MD_Result;


typedef enum {
    /* left -> right */
    GV_MD_LINE_D0 = 0,  
    /* right -> left */
    GV_MD_LINE_D1,       
    /* left <-> right */
    GV_MD_LINE_D2,        
    GV_MD_LINE_D_NUM,
} GV_MD_DirectEnumT;

/*!
*******************************************************************************
** \brief body detect init.
**
** \param[in]   param: init configuration struct.
**
** \return
** - #GV_OK
** - #GV_ERR_BAD_PARAMETER
** - #GV_ERR_OUT_OF_MEMORY
** - #GV_ERR_ALREADY_INITIALIZED
**
*******************************************************************************
*/
GADI_INT gv_init(GV_InitParam initParam);

/*!
*******************************************************************************
** \brief body detect exit.
**
** \return
** - #GV_OK
**
*******************************************************************************
*/
GADI_INT gv_exit(void);


/*!
*******************************************************************************
** \brief get log level.
**
** \return
** - #log level.
**
*******************************************************************************
*/
GV_LogLevelEnumT gv_get_log_level(void);

/*!
*******************************************************************************
** \brief set log level.
**
** \param[in]   level: log level.
**
** \return
** - #GV_OK
**
*******************************************************************************
*/
GADI_INT gv_set_log_level(GV_LogLevelEnumT level);

/*!
*******************************************************************************
** \brief get body detect threshold.
**
** \return
** - #body detect threshold
**
*******************************************************************************
*/
GADI_INT gv_get_bd_threshold(void);

/*!
*******************************************************************************
** \brief set body detect threshold.
**
** \param[in]   threshold: body detect threshold.
**
** \return
** - #GV_OK
**
*******************************************************************************
*/
GADI_INT gv_set_bd_threshold(GADI_INT threshold);

/*!
*******************************************************************************
** \brief get motion detect frame diff threshold.
**
**
** \return
** - #motion detect frame diff threshold
**
*******************************************************************************
*/
GADI_INT gv_get_md_diffthreshold(void);

/*!
*******************************************************************************
** \brief set motion detect frame diff threshold.
**
** \param[in]   diffThreshold: motion detect frame diff threshold.
**
** \return
** - #GV_OK
**
*******************************************************************************
*/
GADI_INT gv_set_md_diffthreshold(GADI_INT diffThreshold);

/*!
*******************************************************************************
** \brief motion detect, add roi alarm.
**
** \return
** - #GV_OK.
**
*******************************************************************************
*/
GADI_INT gv_md_set_roi(GADI_INT idx, GV_Rect rect, GADI_INT minSize);

/*!
*******************************************************************************
** \brief motion detect, clear roi alarm.
**
** \return
** - #GV_OK.
**
*******************************************************************************
*/
GADI_INT gv_md_clear_roi(GADI_INT idx);

/*!
*******************************************************************************
** \brief motion detect, add cross line alarm.
**
** \return
** - #GV_OK.
**
*******************************************************************************
*/
GADI_INT gv_md_set_line(
    GADI_INT idx,
    GV_Point start, 
    GV_Point end,
    GV_MD_DirectEnumT direct);

/*!
*******************************************************************************
** \brief motion detect, clear cross line alarm.
**
** \return
** - #GV_OK.
**
*******************************************************************************
*/
GADI_INT gv_md_clear_line(GADI_INT idx);

/*!
*******************************************************************************
** \brief set first frame for motion detect, not detect. 
** If use gmd, do not call this function.
**
** \return
** - #GV_OK.
** - #GV_ERR_BAD_PARAMETER
** - #GV_ERR_OUT_OF_MEMORY
**
*******************************************************************************
*/
GADI_INT gv_md_set_first_frame(GV_Image yFrame);

/*!
*******************************************************************************
** \brief run one frame motion detect.
**
** \return
** - #GV_OK.
** - #GV_ERR_BAD_PARAMETER
** - #GV_ERR_OUT_OF_MEMORY
**
*******************************************************************************
*/
GADI_INT gv_md_run(
    GV_Image frame, 
    GV_Image yFrame, 
    GV_Image uFrame, 
    GV_Image vFrame,
    GV_MD_Result* result);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif


