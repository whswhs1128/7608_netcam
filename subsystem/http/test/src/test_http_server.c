#include <stdio.h>
#include <string.h>

#include "http_export.h"
#include "onvif_nvt.h"

/*
	/test?param1=tesdke&param2=yy&param3=123456#goke
*/

static pthread_mutex_t g_soapProcMutex;

GK_S32 onvif_mutex_init(void)
{
	if(pthread_mutex_init(&g_soapProcMutex, NULL) < 0)
    {
		return -1;
	}

	return 0;
}

int test_cgi(HTTP_OPS* ops, void* arg)
{
	int len = 0;
	char* q;
	const char* bo;
	int i = *(int*)arg;
	const char* p = "If there is still no video on live view page, please download";

	printf("i = %d\n", i);
	printf("Host=%s\n", ops->get_tag(ops, "hoSt"));

	printf("param1=%s\n", ops->get_param_string(ops, "param1"));
	printf("param2=%s\n", ops->get_param_string(ops, "Param2"));
	printf("param3=%d\n", ops->get_param_int(ops, "parAm3"));

	if (ops->get_fragment(ops))
		printf("fragment=%s\n", ops->get_fragment(ops));

	bo = ops->get_body(ops, &len);
	if (bo)
	{
		printf("body=[%s]\n", bo);
		printf("body len %d\n", len);
	}

	ops->add_tag(ops, "GOKE-Inc", "IPC-7101");

	q = ops->set_body(ops, strlen(p));
	memcpy(q, p, strlen(p));
	return 0;
}

/*
	/testaa.cgi?param1=wewe&param3=10#goke122
*/
int test_cgi1(HTTP_OPS* ops, void* arg)
{
	int len = 0;

	const char* bo;
	const char* p = "Make sure your site 192.168.103.49 has been added";



	printf("User-Agent=%s\n", ops->get_tag(ops, "User-Agent"));

	printf("param1=%s\n", ops->get_param_string(ops, "param1"));
	printf("param3=%d\n", ops->get_param_int(ops, "parAm3"));

	printf("fragment=%s\n", ops->get_fragment(ops));

	bo = ops->get_body(ops, &len);
	if (bo)
	{
		printf("body=[%s]\n", bo);
		printf("body len %d\n", len);
	}

	ops->add_tag(ops, "GOKE-Inc_1", "IPC-7101000000");

	ops->set_body_ex(ops, p, strlen(p));

	return 0;
}


int test_onvif(HTTP_OPS* ops, void* arg)
{
    int len;
    ONVIF_Http_package_S *pstHttpPackage;
    const char *content;
    printf("===================>1\n");
    content = ops->get_http_request(ops, &len);
    printf("\n\n%s\n\n%d",content,len);
#if 0   
    printf("===================>2\n");
    pthread_mutex_lock(&g_soapProcMutex);
    GK_NVT_SoapProc_InWeb(0, 0, 0, content, len);
    
    printf("===================>3\n");
    pstHttpPackage = soap_proc_GetHttpPackage();
    
    printf("===================>4\n");
    ops->set_http_respond(ops, pstHttpPackage->buffer, pstHttpPackage->Len);
    
    printf("===================>5\n");
    soap_proc_ReleaseHttpPackage();
    printf("===================>6]\n");

    pthread_mutex_unlock(&g_soapProcMutex);
#endif    
    return 0;
}

#ifdef XXXX
int main(int argc, char** argv)
{
	int i = 100;
	if (http_mini_server_init("0.0.0.0", 80, "/home/www") < 0)
		return -1;

	http_mini_add_cgi_callback("test", test_cgi, METHOD_GET, &i);
	http_mini_add_cgi_callback("testaa.cgi", test_cgi1, METHOD_POST, NULL);
	http_mini_add_cgi_callback("onvif", test_onvif, METHOD_GET|METHOD_POST, &i);

	http_mini_server_run();
	while (1)
	{
        sleep(10);
    }

	http_mini_server_exit();
	return 0;
}

#else

int main(int argc, char** argv)
{
	int i = 100;	
    onvif_mutex_init();
	if (http_mini_server_init("0.0.0.0", 80, "/home/html_demo") < 0)
		return -1;

    GK_NVT_Onvif_Init(GK_TRUE, 80);

	http_mini_add_cgi_callback("test", test_cgi, METHOD_GET, &i);
	http_mini_add_cgi_callback("testaa.cgi", test_cgi1, METHOD_POST, NULL);
    http_mini_add_cgi_callback("onvif", test_onvif, METHOD_GET|METHOD_POST, &i);

	http_mini_server_run();
	while (1)
	{
        sleep(10);
    }

	http_mini_server_exit();
	return 0;
}

#endif

