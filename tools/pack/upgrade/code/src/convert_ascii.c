
#include <stdio.h>

#include <errno.h>

#include <stdlib.h>
#include <string.h>
#include "parser_cfg.h"
#include "convert_ascii.h"


int conver_bin_ascii(char* input_file, char* output_file,BIN_SIZE_TYPE char_len,ENDIAN_TYPE endian)
{
	FILE* fp_in;
	FILE* fp_out;
	unsigned char inputbuf[128];
	unsigned char outputbuf[256];
	int ret;
	int len;
	int i;
	
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
	fprintf(fp_out,"	/************************************************/\n \
	/* Original file name = %s */\n",input_file);

	while (!feof(fp_in))
	{
	    len = fread(inputbuf, 1, 16, fp_in);
		for(i = 0; i < len; i++)
			fprintf(fp_out,"0x%02X,",inputbuf[i]);
		fprintf(fp_out,"\n");
	}

conv_err:
	if(fp_in)
		fclose(fp_in);
	if(fp_out)
		fclose(fp_out);
	return ret;

}

