#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"


int main (int argc, const char * argv[])
{
	cJSON *json;
	double *test;
	char *out;

	json=cJSON_ReadFile("./audio.json");//从文件系统中读取 JSON 项目
	
	out=cJSON_GetItemValue( json, "audioEncode.audioEncodeChannel[0].codecTypeProperty.opt[1]" );//获取项目值
	printf("%s\n",out);

	test=cJSON_GetItemValue( cJSON_GetObjectItem(json, "audioInput"),"audioInputChannel[0].id" );
	printf("%d\n",(int)*test) ;

	out=cJSON_GetItemValue( cJSON_GetObjectItem(json, "audioEncode"), "audioEncodeChannel[0].codecTypeProperty.type" );
	printf("%s\n",out);

	cJSON_SetItemValue( json, "audioEncode.audioEncodeChannel[0].codecTypeProperty.opt[0]", "abcdefjsd" );//设置项目值
	out=cJSON_GetItemValue( json, "audioEncode.audioEncodeChannel[0].codecTypeProperty.opt[0]" );
	printf("%s\n",out);

	cJSON_WriteFile( "./audiotest.json", json );//保存 JSON 项目到文件系统中
	
	return 0;
}
