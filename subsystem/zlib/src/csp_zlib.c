/*!
*****************************************************************************
** \file        csp/zlib/trunk/src/csp_zlib.c
**
** \version     $Id: csp_zlib.c 2 2014-08-07 07:42:50Z huangjunlei $
**
** \brief       csp zlib implementation
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2013 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#include <stdio.h>
//#include "adi_types.h"
//#include "adi_sys.h"
#include "sdk_def.h"

#include "csp_zlib.h"
#include "zlib.h"
#include <stdlib.h>

#ifdef USE_MINIZIP
#include "zip.h"
#include "unzip.h"
#include "gfs.h"
#include <string.h>
#endif
//*****************************************************************************
//*****************************************************************************
//** Local Defines
//*****************************************************************************
//*****************************************************************************

#define VERSION "v0.0.1"

#define	ZALLOC_ALIGNMENT	16
#define HEAD_CRC		2
#define EXTRA_FIELD		4
#define ORIG_NAME		8
#define COMMENT			0x10
#define RESERVED		0xe0
#define DEFLATED		8



//#define DEBUG
#ifdef DEBUG
#define csp_zlib_printk(S...)      GM_Printf("csp_zlib: "S)
#define csp_zlib_error(S...)       GM_Printf("csp_zlib error: "S)
#define csp_zlib_warning(S...)     GM_Printf("csp_zlib war: "S)
#else
#define csp_zlib_printk(S...)
#define csp_zlib_error(S...)      GM_Printf("csp_zlib error: "S)
#define csp_zlib_warning(S...)
#endif


//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************

void *gzalloc(void *x, unsigned items, unsigned size)
{
	void *p;
	size *= items;
	size = (size + ZALLOC_ALIGNMENT - 1) & ~(ZALLOC_ALIGNMENT - 1);
	p = malloc (size);
	return (p);
}
void gzfree(void *x, void *addr, unsigned nb)
{
	free (addr);
}
/*
 * Uncompress blocks compressed with zlib without headers
 */
static int gzunzip(void *dst, int dstlen, unsigned char *src, unsigned long *lenp,
						int stoponerr, int offset)
{
	z_stream s;
	int r;
	s.zalloc = gzalloc;
	s.zfree = gzfree;
	r = inflateInit2(&s, -MAX_WBITS);
	if (r != Z_OK) {
		csp_zlib_error ("Error: inflateInit2() returned %d\n", r);
		return -1;
	}
	s.next_in = src + offset;
	s.avail_in = *lenp - offset;
	s.next_out = dst;
	s.avail_out = dstlen;
	do {
		r = inflate(&s, Z_FINISH);
		if (r != Z_STREAM_END && r != Z_BUF_ERROR && stoponerr == 1) {
			csp_zlib_error("Error: inflate() returned %d\n", r);
			inflateEnd(&s);
			return -1;
		}
		s.avail_in = *lenp - offset - (int)(s.next_out - (unsigned char*)dst);
		s.avail_out = dstlen;
	} while (r == Z_BUF_ERROR);
	*lenp = s.next_out - (unsigned char *) dst;
	inflateEnd(&s);
	return 0;
}

static SDK_ERR gzuncompress(SDK_U8 *destUnCompr, SDK_U32 destUnComprlen,
                                            SDK_U8 *sourCompress, SDK_U32 *sourCompLen)
{
	int i, flags;
	i = 10;
	flags = sourCompress[3];
	if (sourCompress[2] != DEFLATED || (flags & RESERVED) != 0) {
		puts ("Error: Bad gzipped data\n");
		return (-1);
	}
	if ((flags & EXTRA_FIELD) != 0)
		i = 12 + sourCompress[10] + (sourCompress[11] << 8);
	if ((flags & ORIG_NAME) != 0)
		while (sourCompress[i++] != 0)
			;
	if ((flags & COMMENT) != 0)
		while (sourCompress[i++] != 0)
			;
	if ((flags & HEAD_CRC) != 0)
		i += 2;
	if (i >= *sourCompLen) {
		puts ("Error: gunzip out of data in header\n");
		return (-1);
	}
	return gzunzip(destUnCompr, destUnComprlen, sourCompress, sourCompLen, 1, i);
}

SDK_S32 zlib_print_error(SDK_S32 err)
{
    switch(err)
    {
        case Z_BUF_ERROR:
            csp_zlib_error("[%s]:there was not enough memory!\n", "zlib_print_error");
            break;

        case Z_MEM_ERROR:
            csp_zlib_error("[%s]:there was not enough output buffer!\n", "zlib_print_error");
            break;

        case Z_DATA_ERROR:
            csp_zlib_error("[%s]:there was data error!\n", "zlib_print_error");
            break;

        case Z_OK:
            break;

        default:
            csp_zlib_error("[%s]:there was some error:%d!\n", "zlib_print_error", err);
            break;
    }
    return err;
}

SDK_ERR gcsp_zlib_get_version(SDK_U8 **version)
{
    *version = (SDK_U8 *)VERSION;

    return SDK_OK;
}

SDK_ERR gcsp_zlib_compress(SDK_U8 *destCompr, SDK_U32 *destComprlen,
                        SDK_U8 *sourUnCompress, SDK_U32 sourUnCompLen, GCSP_ZLIB_CompressTypeT type)
{

    z_stream c_stream; /* compression stream */
    int err;

    if(destCompr == NULL || sourUnCompress == NULL)
    {
        csp_zlib_error("[%s]: parameter have null\n", "gcsp_zlib_compress");
        return SDK_ERR_BAD_PARAMETER;
    }

    if (*destComprlen == 0)
    {
        csp_zlib_error("[%s]: destComprlen can't be zero\n", "gcsp_zlib_compress");
        return SDK_ERR_BAD_PARAMETER;
    }

    c_stream.next_in = (Bytef*)sourUnCompress;
    c_stream.avail_in = (uInt)sourUnCompLen;

    c_stream.next_out = destCompr;
    c_stream.avail_out = (uInt)*destComprlen;

    c_stream.zalloc = (alloc_func)0;
    c_stream.zfree = (free_func)0;
    c_stream.opaque = (voidpf)0;


    err = deflateInit(&c_stream, type);
    if(err != Z_OK)
    {
        return zlib_print_error(err);
    }

    err = deflate(&c_stream, Z_FINISH);
    if (err != Z_STREAM_END)
    {
        deflateEnd(&c_stream);
        err = (err == Z_OK ? Z_BUF_ERROR : err);

        return zlib_print_error(err);
    }

    *destComprlen = c_stream.total_out;
    err= deflateEnd(&c_stream);

    if(err != Z_OK)
    {
        return zlib_print_error(err);
    }
    else
    {
        return Z_OK;
    }
}

SDK_ERR gcsp_zlib_uncompress(SDK_U8 *destUnCompr, SDK_U32 *destUnComprlen,
                            SDK_U8 *sourCompress, SDK_U32 sourCompLen)
{
    z_stream stream;
    int err =SDK_OK;
    if(destUnCompr == NULL || sourCompress == NULL)
    {
        csp_zlib_error("[%s]: parameter have null\n", "gcsp_zlib_uncompress");
        return SDK_ERR_BAD_PARAMETER;
    }

    if (*destUnComprlen == 0)
    {
        csp_zlib_error("[%s]: destUnComprlen can't be zero\n", "gcsp_zlib_uncompress");
        return SDK_ERR_BAD_PARAMETER;
    }
    stream.next_in  = (Bytef*)sourCompress;
    stream.avail_in = (uInt)sourCompLen;

    stream.next_out = (Bytef*)destUnCompr;
    stream.avail_out = (uInt)*destUnComprlen;

    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;

    err = inflateInit(&stream);
    if (err != Z_OK)
    {
        return zlib_print_error(err);
    }

    err = inflate(&stream, Z_FINISH);
    if (err != Z_STREAM_END)
    {
        inflateEnd(&stream);
        if (err == Z_NEED_DICT || (err == Z_BUF_ERROR && stream.avail_in == 0))
        {
            err = Z_DATA_ERROR;
        }
       
        return zlib_print_error(err);
    }

    *destUnComprlen = stream.total_out;

    err = inflateEnd(&stream);

    if(err != Z_OK)
    {
        return zlib_print_error(err);
    }
    else
    {
        return Z_OK;
    }

}

SDK_ERR gcsp_zlib_img_uncompress(SDK_U8 *destUnCompr, SDK_U32 destUnComprlen,
                                                    SDK_U8 *sourCompress, SDK_U32 *sourCompLen)
{
    if ((NULL == destUnCompr) || (destUnComprlen == 0) || (NULL == sourCompress) || (NULL == sourCompLen) )
    {
        csp_zlib_error("gcsp_zlib_img_uncompress params error!\n");
        return -1;
    }
    
	if (gzuncompress(destUnCompr, destUnComprlen,(SDK_U8 *)sourCompress, (SDK_U32 *)sourCompLen) != 0)
    {
        csp_zlib_error("gzuncompress error!\n");
        return -1;
    }   
    return 0;
}

#ifdef USE_MINIZIP

SDK_HANDLE gcsp_zlib_compress_files_open(char* filepath)
{
    zipFile zf;
    zf = zipOpen64(filepath, 0);
    if (zf == NULL)
    {
        return 0;
    }
    else
    {
        return (SDK_HANDLE)zf;
    }
}
SDK_ERR gcsp_zlib_compress_files_add(SDK_HANDLE zipFileHandle, char* filepath)
{
    SDK_ERR ret = SDK_OK;
    SDK_S32 err;
    zip_fileinfo zi;
    GFS_S *fileAdd = NULL;
    SDK_U8 *buf = NULL;
    char *fileName = GFS_GetFileName(filepath, '/');
    SDK_S32 size_read;
    SDK_U32 size_buf = 16384;
    zipFile zf = (zipFile)zipFileHandle;
    zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =
    zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
    zi.dosDate = 0;
    zi.internal_fa = 0;
    zi.external_fa = 0;
    err = zipOpenNewFileInZip3_64(zf,fileName,&zi,
                     NULL,0,NULL,0,NULL /* comment*/,
                     8,
                     8,0,
                     -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                     NULL,0, 0);
    if (err != ZIP_OK)
        csp_zlib_error("error %d in opening %s in zipfile\n", err, fileName);
    else
    {
        csp_zlib_error("open file %s success:%s\n", fileName, filepath);
        fileAdd = GFS_Open(filepath, "r");
        if (fileAdd != NULL)
        {
            buf = (void*)gadi_sys_malloc(size_buf);
            if (buf != NULL)
            {
                do
                {
                    size_read = GFS_Read(fileAdd, buf, size_buf);
                    if (size_read>0)
                    {
                        err = zipWriteInFileInZip (zf,buf,size_read);
                        if (err<0)
                        {
                            csp_zlib_error("error %d in writing1 %s in the zipfile\n", err, fileName);
                        }
                    }
                } while ((size_read>0));
            }
        }
        GFS_Close(fileAdd);
        err = zipCloseFileInZip(zf);
        if (err!=ZIP_OK)
            csp_zlib_error("error %d in writing2 %s in the zipfile\n", err, filepath);
        if (buf != NULL)
        {
            gadi_sys_free(buf);
        }
    }
    if (err != ZIP_OK)
    {
        ret = SDK_ERR_BAD_PARAMETER;
    }
    return ret;
}
SDK_ERR gcsp_zlib_compress_mem_files_add(SDK_HANDLE zipFileHandle, char* fileName, char* fileMem, SDK_U32 fileSize)
{
    SDK_ERR ret = SDK_OK;
    SDK_S32 err;
    zip_fileinfo zi;
    zipFile zf = (zipFile)zipFileHandle;
    zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =
    zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
    zi.dosDate = 0;
    zi.internal_fa = 0;
    zi.external_fa = 0;
    err = zipOpenNewFileInZip3_64(zf,fileName,&zi,
                     NULL,0,NULL,0,NULL /* comment*/,
                     8,
                     8,0,
                     -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                     NULL,0, 0);
    if (err != ZIP_OK)
        csp_zlib_error("error in mem opening %s in zipfile\n",fileName);
    else
    {
        err = zipWriteInFileInZip (zf,fileMem,fileSize);
        if (err<0)
        {
            csp_zlib_error("error in mem writing %s in the zipfile\n", fileName);
        }
        err = zipCloseFileInZip(zf);
        if (err!=ZIP_OK)
            csp_zlib_error("error in mem writing %s in the zipfile\n", fileName);
    }
    if (err != ZIP_OK)
    {
        ret = SDK_ERR_BAD_PARAMETER;
    }
    return ret;
}
SDK_ERR gcsp_zlib_compress_files_close(SDK_HANDLE zipFileHandle)
{
    int errclose;
    zipFile zf = (zipFile)zipFileHandle;
    errclose = zipClose(zf,NULL);
    if (errclose != ZIP_OK)
    {
        return SDK_ERR_BAD_PARAMETER;
    }
    else
    {
        return SDK_OK;
    }
}
int do_extract_currentfile(uf,popt_extract_without_path,popt_overwrite,password,rootPath)
    unzFile uf;
    const int* popt_extract_without_path;
    int* popt_overwrite;
    const char* password;
    char* rootPath;
{
    char filename_inzip[256];
    char filenameOut[256];
    char* filename_withoutpath;
    char* p;
    int err=UNZ_OK;
    FILE *fout=NULL;
    void* buf;
    int retW;
    uInt size_buf;
    unz_file_info64 file_info;
    err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);
    if (err!=UNZ_OK)
    {
        csp_zlib_error("error %d with zipfile in unzGetCurrentFileInfo\n",err);
        return err;
    }
    size_buf = 16384;
    buf = (void*)malloc(size_buf);
    if (buf==NULL)
    {
        csp_zlib_error("Error allocating memory\n");
        return UNZ_INTERNALERROR;
    }
    p = filename_withoutpath = filename_inzip;
    while ((*p) != '\0')
    {
        if (((*p)=='/') || ((*p)=='\\'))
            filename_withoutpath = p+1;
        p++;
    }
    if ((*filename_withoutpath)=='\0')
    {
        if ((*popt_extract_without_path)==0)
        {
            csp_zlib_error("creating directory: %s\n",filename_inzip);
        }
    }
    else
    {
        const char* write_filename;
        int skip=0;
        if ((*popt_extract_without_path)==0)
            write_filename = filename_inzip;
        else
            write_filename = filename_withoutpath;
        err = unzOpenCurrentFilePassword(uf,password);
        if (err!=UNZ_OK)
        {
            csp_zlib_error("error %d with zipfile in unzOpenCurrentFilePassword\n",err);
        }
        sprintf(filenameOut,"%s%s",  rootPath, write_filename);
        if ((skip==0) && (err==UNZ_OK))
        {
            fout=fopen(filenameOut,"w");
            if (fout==NULL)
            {
                csp_zlib_error("error opening %s\n",write_filename);
            }
        }
        if (fout!=NULL)
        {
            csp_zlib_error(" extracting: %s\n",write_filename);
            do
            {
                err = unzReadCurrentFile(uf,buf,size_buf);
                if (err<0)
                {
                    csp_zlib_error("error %d with zipfile in unzReadCurrentFile\n",err);
                    break;
                }
                if (err>0)
                {
                    retW = fwrite(buf,1,err,fout);
                    if (retW != err)
                    {
                        csp_zlib_error("error in writing extracted file\n");
                        err=UNZ_ERRNO;
                        break;
                    }
                }
            }
            while (err>0);
            if (fout)
                    fclose(fout);
        }
        if (err==UNZ_OK)
        {
            err = unzCloseCurrentFile (uf);
            if (err!=UNZ_OK)
            {
                csp_zlib_error("error %d with zipfile in unzCloseCurrentFile\n",err);
            }
        }
        else
            unzCloseCurrentFile(uf); /* don't lose the error */
    }
    free(buf);
    return err;
}
SDK_ERR gcsp_zlib_uncompress_files(char* filepath)
{
    int err;
    unz_global_info64 gi;
    unzFile uf=NULL;
    int i;
    int opt_extract_without_path= 1;
    int opt_overwrite = 1;
    char rootPath[64] = {0};
    uf = unzOpen64(filepath);
    if (uf == NULL)
    {
        csp_zlib_error("error %d to uncompress file:%s\n", filepath);
        return SDK_ERR_BAD_PARAMETER;
    }
    strncpy(rootPath, filepath, strlen(filepath) - strlen(strrchr(filepath, '/')) + 1);
    err = unzGetGlobalInfo64(uf,&gi);
    if (err!=UNZ_OK)
        csp_zlib_error("error %d with zipfile in unzGetGlobalInfo \n",err);
    for (i=0;i<gi.number_entry;i++)
    {
        if (do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      NULL, rootPath) != UNZ_OK)
            break;
        if ((i+1)<gi.number_entry)
        {
            err = unzGoToNextFile(uf);
            if (err!=UNZ_OK)
            {
                csp_zlib_error("error %d with zipfile in unzGoToNextFile\n",err);
                break;
            }
        }
    }
    return SDK_OK;
}
#endif
