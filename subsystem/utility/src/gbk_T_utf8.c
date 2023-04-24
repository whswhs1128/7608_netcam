#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "gbk_unicode_map.h"
#define MAX_UNI_INDEX	6770
static int gbk_to_unicode(char *gbk_buf, unsigned short *unicode_buf, int max_unicode_buf_size, int endian);
static int unicode2utf8(unsigned short *punicode_str, int inlen,  char *putf8, int outlen);
static int utf82unicode( char *in, unsigned short *unicode, int outlen);
static int unic_str2gbk_str(unsigned short*unic_str, int unic_len, char *gbk_str, int gbk_len);

int utility_gbk_to_utf8( char *gbk, char *utf8, int len)
{
    int uncodeNum;
    int gkbLen = strlen(gbk);
    int ret;
    int unicodeLen = gkbLen*sizeof(unsigned short);
    unsigned short *unicode = malloc(unicodeLen);
    uncodeNum = gbk_to_unicode(gbk, unicode, unicodeLen, 1);
    if(uncodeNum < -1)
    {
        printf("gkb to unicode failed\n");
		if(unicode)
			free(unicode);
        return -1;
    }
    ret = unicode2utf8(unicode, uncodeNum, utf8, len);
	if(unicode)
    	free(unicode);
    return ret;
}


int utility_utf8_to_gbk(char *utf8, char *gbk, int len)
{
    int uncodeNum;
    int utf8Len = strlen(utf8);
    int ret;
    int unicodeLen = utf8Len*sizeof(unsigned short);
    unsigned short *unicode = malloc(unicodeLen);
    uncodeNum = utf82unicode(utf8, unicode, unicodeLen);
    if(uncodeNum < -1)
    {
        printf("gkb to unicode failed\n");
		if(unicode)
			free(unicode);
        return -1;
    }
    ret = unic_str2gbk_str(unicode, uncodeNum, gbk, len);
	if(unicode)
    	free(unicode);
    return ret;
}

/* unicode 2 gbk */
static int unicode2gbk(unsigned short *punicode, unsigned short *pgbk)
{
	int left, right, middle;

	if(NULL == punicode || NULL == pgbk) {
		printf("err: invalid pgbk or punicode\n");
		return -1;
	}

	left = middle = 0;
	right = MAX_UNI_INDEX - 1;

	if(((*punicode >> 8) == 0) && (*punicode & 0xff) < 0x80){	//ascll
		*pgbk = *punicode & 0xff;
		return 0;
	} else {
		while(left <= right) {
			middle = (left + right)/2;
			if(*punicode == uni_table[middle][0]) {
				*pgbk = uni_table[middle][1];
				return 0;
			} else if(*punicode > uni_table[middle][0])
				left = middle + 1;
			else
				right = middle - 1;
		}
	}

	printf("err: not found unicode:0x%x in table.\n", *punicode);
	return -1;
}

static int unic_str2gbk_str(unsigned short*unic_str, int unic_len, char *gbk_str, int gbk_len)
{
	int i, j;
	unsigned short sgbk = 0;
	for(i=0, j=0; i<unic_len;)
	{
		if(*(unic_str + i) < 0x80)
		{
			*(gbk_str + j) = *(unic_str + i);
			j++;
			i ++;
		}else{
			unicode2gbk(unic_str + i, &sgbk);
			*(gbk_str + j) = (sgbk >> 8) & 0xff;
			*(gbk_str + j + 1) = sgbk & 0xff;
			i ++;
			j += 2;
		}
		if(j >= gbk_len)
			break;
	}

    *(gbk_str + j) = 0;
    *(gbk_str + j+1) = 0;
	return 0;
}

static int unicode2utf8(unsigned short *punicode_str, int inlen, char *putf8, int outlen)
{
	int i, j;
	unsigned short sunicode;
	unsigned char c1, c2;
	const unsigned short *punicode = NULL;

	if(NULL == punicode_str|| NULL == putf8) {
		printf("err: invalid parameters\n");
		return -1;
	}

	punicode = (unsigned short*)punicode_str;
	i = j = 0;

	while(i < inlen && j < outlen) {
		if(((*(punicode + i) >> 8) == 0) && (*(punicode + i) & 0xff) < 0x80)	//ascll
		{
			*(putf8 + j) = *(punicode + i) & 0xff;
			i++;
			j++;
		} else {
			/* unicode to utf8  */
			sunicode = *((unsigned short *)(punicode + i));
			c1 = (sunicode >> 8) & 0xff;
			c2 = sunicode & 0xff;
			putf8[j] = 0xef & ((c1 >> 4)|0xf0);
			putf8[j+1] = 0xbf & ((c1<<2|c2>>6)|0xc0);
			putf8[j+2] = 0xbf & (c2|0xc0);
			j += 3;
			i ++;
		}
	}
    if(i != inlen)
    {
        printf("utf8 convert failed, input len is too small,cur:%d,out len:%d,inlen:%d,conver len:%d\n",j,outlen,inlen,i);
        return -1;
    }
    else
    {
	    return 0;
    }
}

/* utf8±àÂë×ªgbk±àÂë API */
static int utf82unicode( char *in, unsigned short *unicode, int outlen)
{
	unsigned char *p = NULL;
	int i, j, valid_len;
	unsigned short *punicode = unicode;

	if(NULL == in || NULL == unicode) {
		printf("err: invalid parameters.\n");
		return -1;
	}
	valid_len = strlen(in);

	if(valid_len > outlen) {
		printf("err: invalid outlen[%d], valid_len[%d].\n", outlen, valid_len);
		return -1;
	}

	p = (unsigned char *)in;
	i = j = 0;
	while(i < valid_len) {
		if(p[i] < 0x80) {	//ascll
			punicode[j] = p[i];
			i++;
			j++;

		} else {
			/* utf8 to unicode */
			punicode[j] = 0xffff;
			punicode[j] &= (((p[i] | 0xf0) << 12) | 0x0fff);
			punicode[j] &= (((p[i + 1] | 0xc0) << 6) | 0xf03f);
			punicode[j] &= (p[i + 2] | 0xffc0);
			i += 3;
			j ++;
		}
	}

	return j;
}

unsigned short g_gbk_to_unicode_buf_size = 32095;
unsigned short g_gbk_first_code = 33088;
unsigned short g_gbk_last_code = 65183;
/*-----------------+-------------------------+------------------------
                   |    endian = 1 ???      |
				   |    endian = 0 §³??      |
-------------------+-------------------------+-----------------------*/
static int gbk_to_unicode( char *gbk_buf, unsigned short *unicode_buf, int max_unicode_buf_size, int endian)
{
	unsigned short word;
	unsigned char *gbk_ptr = (unsigned char *)gbk_buf;
	unsigned short *uni_ptr = unicode_buf;
	unsigned int uni_ind = 0, gbk_ind = 0, uni_num = 0;
	unsigned char ch;
  int word_pos;

  if( !gbk_buf || !unicode_buf )
      return -1;

  while(1)
  {
  	ch = *(gbk_ptr + gbk_ind);

		if(ch == 0x00)
			break;

    if( ch > 0x80 )
    {
			//chinese
			//word = *p *256 + *(p+1);

			if(endian == 1)  //???
			{
				word = *(gbk_ptr + gbk_ind);
				word <<= 8;
				gbk_ind++;
				word += *(gbk_ptr + gbk_ind);
				gbk_ind++;
			}
			else
			{
				word = *(gbk_ptr + gbk_ind + 1);
				word <<= 8;
				word += *(gbk_ptr + gbk_ind);
				gbk_ind += 2;
			}
			//printf("word: 0x%x\n", word);
			word_pos = word - g_gbk_first_code;
			if(word >= g_gbk_first_code && word <= g_gbk_last_code  && (word_pos < g_gbk_to_unicode_buf_size))
			{
				//*p_out = g_gbk_to_unicode_buf[word_pos];
				//*p_out ++;
				//n++;
				*(uni_ptr + uni_ind) = g_gbk_to_unicode_buf[word_pos];
				uni_ind++;
				uni_num++;
			}
    }
		else
		{
			//for ASCII
			//*p_out = (unsigned short)*p;
			//*p_out ++;
			//n++;

			gbk_ind++;

			//*(uni_ptr + uni_ind) = g_gbk_to_unicode_buf[word_pos];
			*(uni_ptr + uni_ind) = ch;
			uni_ind++;

			uni_num++;
		}

    if(uni_num > max_unicode_buf_size - 1)
			break;
  }

  return uni_num;
}

