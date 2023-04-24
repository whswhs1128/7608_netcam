#include <ctype.h>
#include "http_const.h"
#include "http_utils.h"

void str_tolower(char* s)
{
	int i;
	char* p = (char*)s;

	for (i = 0; i < strlen(s); i++)
	{
		p[i] = tolower(p[i]);
	}
}

void http_space_to_sys_space(char* src, int n)
{
	int i = 0;
	int j = 0;
	char* p = src;

	while (i < n)
	{
		if (p[i] == '%' &&
			p[i + 1] == '2' &&
			p[i + 2] == '0')
		{
			p[i] = ' ';
			
			for (j = i + 1; j < n; j++)
				if (j < n - 2)
					p[j] = p[j + 2];
				else
					p[j] = 0;

			n -= 2;
		}

		i++;
	}
}


int dump_chunk(char** buffer, const char* chunk, int len, int append)
{
	int alloc_len;
	char* pbuf;

	*buffer = NULL;
	if (!chunk || len <= 0)
		return -1;

	alloc_len = len + append;
	pbuf = (char*)malloc(alloc_len);
	if (!pbuf)
		PRINT_ERROR_RETURN(("Fail to alloc space for dump chunk\n"), -1);

	memset(pbuf, 0, alloc_len);
	memcpy(pbuf, chunk, len);

	*buffer = pbuf;
	return 0;
}
