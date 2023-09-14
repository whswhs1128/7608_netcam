/*!
*****************************************************************************
** \file        ./csp/zlib/inc/csp_zlib.h
**
** \version     $Id: csp_zlib.h 2 2014-08-07 07:42:50Z huangjunlei $
**
** \brief       zlib user interface.
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2013 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#ifndef _CSP_ZLIB_H_
#define _CSP_ZLIB_H_

//*****************************************************************************
//*****************************************************************************
//** Enumerated types
//*****************************************************************************
//*****************************************************************************

/*!
*******************************************************************************
** \brief Zlib Compress Type
*******************************************************************************
*/
typedef enum {

    ZIB_NO_COMPRESSION      =  0,    /*!< No compress.          */
    ZIB_BEST_SPEED          =  1,    /*!< Best speed mode.      */
    ZIB_BEST_COMPRESSION    =  9,    /*!< Best compress mode.   */
    ZIB_DEFAULT_COMPRESSION = -1     /*!< Default mode.         */

} GCSP_ZLIB_CompressTypeT;

//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************

#ifdef __cplusplus
extern "C" {
#endif

/*!
*******************************************************************************
** \brief       Get zlib version
**
** \param[out]  version The version of zlib
**
** \return
**  - #SDK_OK  on success
**  - Other     failed
**
**
*******************************************************************************
*/
SDK_ERR gcsp_zlib_get_version(SDK_U8 **version);


/*!
*******************************************************************************
** \brief       Compress data in the given type.
**
** \param[in]   destCompr           pointer to destination compress data
** \param[in]   destComprlen        pointer to destination compress length
** \param[in]   sourUnCompress      pointer to source compress data
** \param[in]   sourUnCompLen       length of source compress
** \param[in]   type                compress type
**
** \return
**  - #SDK_OK  on success
**  - Other     failed
**
** \sa
**  - gcsp_zlib_uncompress()
**
*******************************************************************************
*/
SDK_ERR gcsp_zlib_compress(SDK_U8 *destCompr, SDK_U32 *destComprlen,
                                   SDK_U8 *sourUnCompress, SDK_U32 sourUnCompLen,
                                   GCSP_ZLIB_CompressTypeT type);

/*!
*******************************************************************************
** \brief       Data uncompress
**
** \param[in]   destUnCompr          pointer of destination compress data
** \param[in]   destUnComprlen       pointer of destination compress length
** \param[in]   sourCompress         pointer of source compress data
** \param[in]   sourCompLen          pointer compress length
**
** \return
**  - #SDK_OK   on success
**  - Other      failed
**
** \sa
**  - gcsp_zlib_compress()
**
*******************************************************************************
*/
//typedef int SDK_ERR_BAD_PARAMETER;
SDK_ERR gcsp_zlib_uncompress(SDK_U8 *destUnCompr, SDK_U32 *destUnComprlen,
                                    SDK_U8 *sourCompress, SDK_U32 sourCompLen);
SDK_ERR gcsp_zlib_img_uncompress(SDK_U8 *destUnCompr, SDK_U32 destUnComprlen,
                                                SDK_U8 *sourCompress, SDK_U32 *sourCompLen);
#ifdef USE_MINIZIP
SDK_HANDLE gcsp_zlib_compress_files_open(char* filepath);
SDK_ERR gcsp_zlib_compress_files_add(SDK_HANDLE zipFileHandle, char* filepath);
SDK_ERR gcsp_zlib_compress_mem_files_add(SDK_HANDLE zipFileHandle, char* fileName,
                                                    char* fileMem, SDK_U32 fileSize);
SDK_ERR gcsp_zlib_compress_files_close(SDK_HANDLE zipFileHandle);
#endif
#ifdef __cplusplus
}
#endif

#endif /* _CSP_ZLIB_H_ */
