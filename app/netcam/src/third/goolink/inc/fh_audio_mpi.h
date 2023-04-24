#ifndef FH_AC_MPI_H_
#define FH_AC_MPI_H_

#include "types/type_def.h"
#include "fh_audio_mpipara.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

/**
* FH_AC_Init
*@brief 音频系统初始化，完成打开驱动设备
*@param [in] 无
*@param [out] 无
*@return 是否成功
* - RETURN_OK(0):  成功
* - 其他：失败,错误详见错误号
*/
FH_SINT32 FH_AC_Init();

/**
* FH_AC_DeInit
*@brief 音频系统释放资源，关闭音频设备
*@param [in] 无
*@param [out] 无
*@return 是否成功
* - RETURN_OK(0):  成功
* - 其他：失败,错误详见错误号
*/
FH_SINT32 FH_AC_DeInit();

/**
* FH_AC_Set_Config
*@brief 设置AI、AO设备参数
*@param [in] pstConfig 设备参数信息结构体指针
*@param [out] 无
*@return 是否成功
* - RETURN_OK(0):  成功
* - 其他：失败,错误详见错误号
*/
FH_SINT32 FH_AC_Set_Config(FH_AC_CONFIG *pstConfig);

/**
* FH_AC_AI_Enable
*@brief 使能AI设备
*@param [in] 无
*@param [out] 无
*@return 是否成功
* - RETURN_OK(0):  成功
* - 其他：失败,错误详见错误号
*/
FH_SINT32 FH_AC_AI_Enable();

/**
* FH_AC_AI_Disable
*@brief 禁用AI设备
*@param [in] 无
*@param [out] 无
*@return 是否成功
* - RETURN_OK(0):  成功
* - 其他：失败,错误详见错误号
*/
FH_SINT32 FH_AC_AI_Disable();

/**
* FH_AC_AO_Enable
*@brief 使能AO设备
*@param [in] 无
*@param [out] 无
*@return 是否成功
* - RETURN_OK(0):  成功
* - 其他：失败,错误详见错误号
*/
FH_SINT32 FH_AC_AO_Enable();

/**
* FH_AC_AO_Disable
*@brief 禁用AO设备
*@param [in] 无
*@param [out] 无
*@return 是否成功
* - RETURN_OK(0):  成功
* - 其他：失败,错误详见错误号
*/
FH_SINT32 FH_AC_AO_Disable();

/**
* FH_AC_AI_Pause
*@brief 暂停AI设备运行
*@param [in] 无
*@param [out] 无
*@return 是否成功
* - RETURN_OK(0):  成功
* - 其他：失败,错误详见错误号
*/
FH_SINT32 FH_AC_AI_Pause();

/**
* FH_AC_AI_Resume
*@brief 恢复AI设备运行
*@param [in] 无
*@param [out] 无
*@return 是否成功
* - RETURN_OK(0):  成功
* - 其他：失败,错误详见错误号
*/
FH_SINT32 FH_AC_AI_Resume();

/**
* FH_AC_AO_Pause
*@brief 暂停AO设备运行
*@param [in] 无
*@param [out] 无
*@return 是否成功
* - RETURN_OK(0):  成功
* - 其他：失败,错误详见错误号
*/
FH_SINT32 FH_AC_AO_Pause();

/**
* FH_AC_AO_Resume
*@brief 恢复AO设备运行
*@param [in] 无
*@param [out] 无
*@return 是否成功
* - RETURN_OK(0):  成功
* - 其他：失败,错误详见错误号
*/
FH_SINT32 FH_AC_AO_Resume();

/**
* FH_AC_AI_SetVol
*@brief 设置AI设备音量大小
*@param [in] volume 音量大小,0 ~ 100
*@param [out] 无
*@return 是否成功
* - RETURN_OK(0):  成功
* - 其他：失败,错误详见错误号
*/
FH_SINT32 FH_AC_AI_SetVol(FH_SINT32 volume);

/**
* FH_AC_AI_GetFrame
*@brief 获取一帧音频数据
*@param [in]  bBlock   阻塞/非阻塞标识
*@param [out] pstFrame 音频数据结构体指针
*@return 是否成功
* - RETURN_OK(0):  成功
* - -1 ： 超时，超时时间为1个采样帧所需时间
* - 其他：失败,错误详见错误号
*/
FH_SINT32 FH_AC_AI_GetFrame(FH_AC_FRAME_S *pstFrame, FH_BOOL bBlock);

/**
* FH_AC_AO_SendFrame
*@brief 发送一帧音频数据
*@param [in] pstFrame 音频数据结构体指针
*@param [in] bBlock   阻塞/非阻塞标识
*@param [out] 无
*@return 是否成功
* - RETURN_OK(0):  成功
* - -1 ： 超时，超时时间为1个采样帧所需时间
* - 其他：失败,错误详见错误号
*/
FH_SINT32 FH_AC_AO_SendFrame(FH_AC_FRAME_S *pstFrame, FH_BOOL bBlock);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* FH_AC_MPI_H_ */
