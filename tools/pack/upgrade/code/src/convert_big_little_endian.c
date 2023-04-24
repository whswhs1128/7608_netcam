#include <stdio.h>

#include <errno.h>

#include <stdlib.h>
#include <string.h>
#include "parser_cfg.h"
#include "convert_ascii.h"


int conver_big_little_endian(char* input_file, char* output_file)
{
	FILE* fp_in;
	FILE* fp_out;
	char *inputbuf;
	char *outputbuf;
	int ret = 0;
	int len;
	int i,j,k;
	int cnt = 0;
	int buf_len = 1024*1024;
	char *pin;
	char *pout;

	
	fp_in = fopen(input_file, "rb");	
	if (fp_in == NULL)    
	{		
		ERROR_PRT("open %s ERROR! %s\n", input_file, strerror(errno));		 
		return -1;    
	}
	
	fp_out = fopen(output_file, "wb");	
	if (fp_out < 0)    
	{		
		ERROR_PRT("open %s ERROR! %s\n", output_file, strerror(errno));		
		ret = -1;		 
		goto conv_err;    
	}

	inputbuf = malloc(buf_len);
	outputbuf = malloc(buf_len);
	if(inputbuf == NULL || outputbuf == NULL)
	{
		ERROR_PRT("malloc %dKB ERROR! %s\n", buf_len/1024, strerror(errno));		
		ret = -1;		 
		goto conv_err;    
	}

	while (!feof(fp_in))
	{
	    len = fread(inputbuf, 1, buf_len, fp_in);
	    
	    if(len == 0)
	    {
	       break;
        }
           
		if(len < 0)
		{
			 
            ERROR_PRT("read input file %s ERROR! %s\n", input_file, strerror(errno)); 	
			ret  = -1;
			break;
		}
		
		cnt = len;
		pout = outputbuf;
		pin  = inputbuf;		
		while(cnt > 0)
		{
			if(cnt > 4)
				k = 4;
			else 
				k = cnt;
			
			for(i = 0; i < k; i++)
			{
				pout[ k - 1 - i] = pin[ i ];
				
			}

			cnt -= k;
			pout += k;
			pin  += k;
		}
		
		if(fwrite(outputbuf,1,len,fp_out) != len)
		{
			ERROR_PRT("write output buf ERROR! %s\n", strerror(errno)); 	
			ret  = -1;
			break;
		}	
	}

conv_err:
	if(outputbuf)
		free(outputbuf);
	
	if(inputbuf)
		free(inputbuf);
	
	
	if(fp_in)
		fclose(fp_in);
	if(fp_out)
		fclose(fp_out);
	return ret;

}

