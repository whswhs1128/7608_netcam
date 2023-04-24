#ifndef CONVERT_ASCII_H
#define CONVERT_ASCII_H

/* convert byte order*/
typedef enum
{
	ABCD_TO_ABCD,  
	ABCD_TO_DCBA,
	ABCD_TO_BADC,
	ABCD_TO_CDAB
}ENDIAN_TYPE;

typedef enum
{
	BYTE_ONE,  		// one byte for asicc
	BYTE_WORD,		// two byte for asicc
	BYTE_LOGWORD,	// four byte for asicc

}BIN_SIZE_TYPE;


int conver_bin_ascii(char* input_file, char* output_file,BIN_SIZE_TYPE char_len,ENDIAN_TYPE endian);

#endif
