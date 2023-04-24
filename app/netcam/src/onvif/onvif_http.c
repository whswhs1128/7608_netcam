#include "stdio.h"
#include "string.h"
#include "unistd.h"
#include "netcam_api.h"
#include "http_export.h"
#include "onvif.h"

static pthread_mutex_t g_soapProcMutex;


int onvif_proc(HTTP_OPS* ops, void* arg)
{
    pthread_mutex_lock(&g_soapProcMutex);
	ONVIF_DBG("HTTP: enter CGI(tid: %d).", (pid_t)syscall(SYS_gettid));
    int len;
    ONVIF_Http_package_S *pstHttpPackage;
    const char *content;
    content = ops->get_http_request(ops, &len);
	ONVIF_DBG("HTTP: enter ONVIF.");
    GK_NVT_SoapProc_InWeb(0, 0, 0, content, len);    
	ONVIF_DBG("HTTP: out ONVIF.");
    pstHttpPackage = soap_proc_GetHttpPackage();
    ops->set_http_respond(ops, pstHttpPackage->buffer, pstHttpPackage->Len);
    soap_proc_ReleaseHttpPackage();
	ONVIF_DBG("HTTP: out CGI.");
    pthread_mutex_unlock(&g_soapProcMutex);
    return HPE_RET_DISCONNECT;
}
int netcam_http_onvif_init(void)
{
	if(pthread_mutex_init(&g_soapProcMutex, NULL) < 0)
	{
		ONVIF_ERR("Fail to initialize soap mutex in CIG.");
		return -1;
	}
	
    http_mini_add_cgi_callback("onvif", onvif_proc, METHOD_GET|METHOD_POST, NULL);
    http_mini_add_cgi_callback("Subcription", onvif_proc, METHOD_GET|METHOD_POST, NULL);

    return 0;
}

