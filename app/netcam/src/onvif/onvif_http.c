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

#include "ttl.h"
int uart;
char rtsp_url[64];
int netcam_http_onvif_init(void)
{
//	if(pthread_mutex_init(&g_soapProcMutex, NULL) < 0)
//	{
//		ONVIF_ERR("Fail to initialize soap mutex in CIG.");
//		return -1;
//	}
	
   // http_mini_add_cgi_callback("onvif", onvif_proc, METHOD_GET|METHOD_POST, NULL);
   // http_mini_add_cgi_callback("Subcription", onvif_proc, METHOD_GET|METHOD_POST, NULL);

    sleep(3);

  //  uart = init_uart(0);
  //  pthread_t pid_485;
  //  pthread_create(&pid_485, 0, begin_485, NULL);
  // 
  //  pthread_t rx_tty;
  //  pthread_create(&rx_tty, 0, rx_receive, NULL);

   // sdk_af_lens_init(NULL);

    //sleep(1);
    //sprintf(rtsp_url, "rtsp://%s:554/stream0", g_GkIpc_OnvifInf.discov.hostip);

    return 0;
}

