#include "zlib.h"
#include "zconf.h"
#include <time.h>

#include "http_compress.h"


static int init_inflate(z_stream* _inflater)
{
	_inflater->zalloc = Z_NULL;
	_inflater->zfree = Z_NULL;
	_inflater->opaque = Z_NULL;

	if (Z_OK != inflateInit2(_inflater, -MAX_WBITS))
	{
		return -1;
	}

	return 0;
}


static int init_deflate(z_stream* _deflater)
{
	_deflater->zalloc = Z_NULL;
	_deflater->zfree = Z_NULL;
	_deflater->opaque = Z_NULL;

	if (Z_OK != deflateInit2(_deflater,
		Z_DEFAULT_COMPRESSION,
		Z_DEFLATED,
		-MAX_WBITS,  /* supress zlib-header */
		9,
		Z_DEFAULT_STRATEGY))
	{
		return -1;
	}

	return 0;
}


int deflate_gzip(char* in, unsigned int in_len,
	char* out, unsigned int* out_len, int front, int end)
{
	int 			rv 			= 	0;
	unsigned char* 	c 			= 	NULL;
	unsigned char* 	out_buf 	= 	(unsigned char *)out;
	unsigned int    out_buf_len = 	*out_len;
	unsigned long   _crc        =   0;
	int             flush;
	time_t			mtime;
	z_stream        _deflater;

	mtime=time(NULL);

	_deflater.zalloc = Z_NULL;
    _deflater.zfree = Z_NULL;
    _deflater.opaque = Z_NULL;

    if (Z_OK != deflateInit2(&_deflater,
        Z_DEFAULT_COMPRESSION,
        Z_DEFLATED,
        -MAX_WBITS,  /* supress zlib-header */
        9,
        Z_DEFAULT_STRATEGY))
    {
        return -1;
    }


	if(front)
	{
		// write gzip header
		c = out_buf;
		c[0] = 0x1f;
		c[1] = 0x8b;
		c[2] = Z_DEFLATED;
		c[3] = 0; // options
		c[4] = (mtime >>  0) & 0xff;
		c[5] = (mtime >>  8) & 0xff;
		c[6] = (mtime >> 16) & 0xff;
		c[7] = (mtime >> 24) & 0xff;
		c[8] = 0x00; // extra flags
		c[9] = 0x03; // UNIX

		out_buf += 10;
		out_buf_len -= 10;
	}

	_deflater.next_in = (unsigned char *)in;
	_deflater.avail_in = in_len;

	_deflater.next_out = out_buf;
	if(end)
	{
		out_buf_len -= 8;
		_deflater.avail_out = out_buf_len;
		flush = Z_FINISH;
	}
	else
	{
		_deflater.avail_out = out_buf_len;
		flush = Z_SYNC_FLUSH;
	}


	rv = deflate(&_deflater, flush);
	if(rv < 0)
	{
		return -1;
	}

	out_buf += (out_buf_len - _deflater.avail_out);
	_crc = crc32(_crc, (const unsigned char*)in, in_len);

	if(end)
	{
		//append gzip end
		c = out_buf;

		c[0] = (_crc >>  0) & 0xff;
		c[1] = (_crc >>  8) & 0xff;
		c[2] = (_crc >> 16) & 0xff;
		c[3] = (_crc >> 24) & 0xff;
		c[4] = (_deflater.total_in >>  0) & 0xff;
		c[5] = (_deflater.total_in >>  8) & 0xff;
		c[6] = (_deflater.total_in >> 16) & 0xff;
		c[7] = (_deflater.total_in >> 24) & 0xff;
		out_buf += 8;

		rv = deflateEnd(&_deflater);
		if(rv != Z_OK)
		{
			return -1;
		}
	}

	return *out_len = out_buf - (unsigned char*)out;
}


int deflate_deflate(char* in, unsigned int in_len,
	char* out, unsigned int* out_len, int end)
{
	int rv = 0;
	unsigned char * out_buf     = (unsigned char *)out;
	unsigned int    out_buf_len = *out_len;
	int             flush;
	z_stream        _deflater;


	rv = init_deflate(&_deflater);
	if(rv < 0)
	{
		return -1;
	}

	_deflater.next_in = (unsigned char *)in;
	_deflater.avail_in = in_len;

	_deflater.next_out = out_buf;
	if(end)
	{
		_deflater.avail_out = out_buf_len;
		flush = Z_FINISH;
	}
	else
	{
		_deflater.avail_out = out_buf_len;
		flush = Z_SYNC_FLUSH;
	}

	rv = deflate(&_deflater, flush);
	if(rv < 0)
	{
		return -1;
	}

	out_buf += (out_buf_len - _deflater.avail_out);

	if(end)
	{
		rv = deflateEnd(&_deflater);
		if(rv != Z_OK)
		{
			return -1;
		}
	}

	*out_len = out_buf - (unsigned char*)out;
	return *out_len;
}


int inflate_gzip(char* in, unsigned int in_len,
	char* out, unsigned int* out_len, int front, int end)
{
	int rv = 0;
	unsigned char * out_buf       = (unsigned char *) out;
	unsigned int    out_buf_len   = *out_len;
	z_stream        _inflater;

	rv = init_inflate(&_inflater);
	if(rv < 0)
	{
		return -1;
	}

	if(front)
	{
		in +=10;
		in_len -=10;
	}

	if(end)
	{
		in_len -=8;
	}

	_inflater.next_in = (unsigned char *)in;
	_inflater.avail_in = in_len;

	_inflater.next_out = out_buf;
	_inflater.avail_out = out_buf_len;

	rv = inflate(&_inflater, Z_NO_FLUSH);
	if(rv < 0)
	{
		return -1;
	}

	out_buf += (out_buf_len - _inflater.avail_out);
	*out_len = out_buf - (unsigned char*)out;
	out_buf[0]='\0';

	if(_inflater.avail_out==0 && rv != Z_STREAM_END)
	{
		return -2;
	}

	if(end)
	{
		rv = inflateEnd(&_inflater);
		if(rv != Z_OK)
		{
			return -1;
		}
	}

	return *out_len;
}


