#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
//#include "gk7101.h"
#include "cJSON.h"
#include "netcam_api.h"
#include "factory_test.h"
#include "media_fifo.h"

FACTORY_TEST_S factory_test;

void vFactoryWorkMode_Play_AudioFile(char *file,NETCAM_AUDIO_ENC_TYPE_t audio_type)
{
	int fd,ret,len;
	char *buff;
	char audio_filename[128];
	memset(audio_filename,0,sizeof(audio_filename));
	sprintf(audio_filename,"%s/%s",FACTORY_TEST_PATH,file);
	fd = open(audio_filename, O_RDONLY);
	if(fd <= 0)
	{
		PRINT_ERR("can not open %s", audio_filename);
		close(fd);
		return;
	}
	len = lseek(fd,0,SEEK_END);
    printf("len:%d\n", len);
    //len=ftell(f);
    lseek(fd, 0, SEEK_SET);
    buff = malloc(len);
	ret = read(fd, buff,len);
	
//	while(ret > 0) {
//		PRINT_INFO("audio buffsize:%d\n", ret);
		//netcam_audio_output(buff, ret, audio_type, 1);    xqq delete audio
//		ret = read(fd, buff, sizeof(buff));
//	}
	
	close(fd);
	free(buff);
	buff = NULL;
}

void vFactoryWorkMode_PTZ_Loop(void)
{
	netcam_ptz_hor_ver_cruise(0);
}

void vFactoryWorkMode_PTZ_StopLoop(void)
{
	netcam_ptz_stop();
}

void vFactoryWorkMode_LinkWifi(char *ssid,char *passwd)
{
	vFactoryWorkMode_Play_AudioFile(FACTORY_TEST_AUDIO_START_WIFI_CONNECT,NETCAM_AUDIO_ENC_A_LAW);
	
	WIFI_LINK_INFO_t linkInfo;
	char *WifiName = NULL;
	ST_SDK_NETWORK_ATTR net_attr;
	if(netcam_net_wifi_on() != 0)
	{
		printf("enable wifi function failed");
		return ;
	}
	memset(&linkInfo,0,sizeof(WIFI_LINK_INFO_t));
	netcam_net_wifi_get_connect_info(&linkInfo);

	WifiName = netcam_net_wifi_get_devname();
	if(WifiName == NULL) {
		printf("Not WiFi device.");
		return;
	}
	linkInfo.isConnect = 1;
	netcam_net_wifi_switchWorkMode(NETCAM_WIFI_STA);
	strcpy(linkInfo.linkEssid,ssid);
	strcpy(linkInfo.linkPsd,passwd);
	linkInfo.linkScurity = 8;
	linkInfo.mode = 0;

	snprintf(net_attr.name,sizeof(net_attr.name), WifiName);

	netcam_net_get(&net_attr);
	net_attr.dhcp = 1;

	int ret = netcam_net_wifi_set_connect_info(&linkInfo);
	if( ret == 0 &&  linkInfo.linkStatus == WIFI_CONNECT_OK )
	{
		//netcam_net_set(&net_attr);
		char *ifname = netcam_net_wifi_get_devname();
		if(ifname == NULL)
			return;
		netcam_net_reset_net_ip(ifname,&net_attr);
	}
	//vFactoryWorkMode_Play_AudioFile("wireless.alaw",NETCAM_AUDIO_ENC_A_LAW);
	usleep(50000);
	printf("\r\n\r\nwifi ssid=%s,passwd=%s\r\n\r\n",ssid,passwd);
}

void vFactoryWorkMode_StopLinkWifi(void)
{
	netcam_net_wifi_off();
	vFactoryWorkMode_Play_AudioFile(FACTORY_TEST_AUDIO_STOP_WIFI_CONNECT,NETCAM_AUDIO_ENC_A_LAW);
	printf("\r\n\r\nStop wifi Link\r\n\r\n");
}

void vFactoryWorkMode_WriteRecordFile(int record_fd)
{
	#if 0
	int ret;
	GADI_AUDIO_AioFrameT ptrFrame = {0};
	ret = gadi_audio_ai_get_frame(&ptrFrame, GADI_TRUE);
	if((ret != 0) || (ptrFrame.len == 0))
	{
		return;
	}
	#endif
	int ret,recv_frame_len = 0;
	char *buffer = NULL;
	GK_NET_FRAME_HEADER frame_header;
	memset(&frame_header, 0, sizeof(GK_NET_FRAME_HEADER));
	if(factory_test.work_mode.test_mode.mhandle == NULL)
	{
		printf("add mhandle error!!!\r\n");
		return;
	}
	
	ret = mediabuf_read_frame(factory_test.work_mode.test_mode.mhandle, (void**)&buffer, &recv_frame_len, &frame_header);
	if (ret <= 0)
	{
    	printf("ERROR!Write record file; read no data.\n");
		return;
	}
	if(frame_header.frame_type == GK_NET_FRAME_TYPE_A)
	{
		write(factory_test.work_mode.test_mode.record_fd,buffer,recv_frame_len);
		//printf("recv_frame_len=%d\r\n",recv_frame_len);
	}
}


void vFactoryWorkMode_StartRecord(void)
{
	char recordfile[128];
	vFactoryWorkMode_Play_AudioFile(FACTORY_TEST_AUDIO_RECORD,NETCAM_AUDIO_ENC_A_LAW);
	memset(recordfile,0,sizeof(recordfile));
	sprintf(recordfile,"%s/%s",FACTORY_TEST_PATH,FACTORY_WORKMODE_TEST_RECORD_FILE);
	if(access(recordfile,F_OK) == 0)
	{
		system(recordfile);
	}
	factory_test.work_mode.test_mode.mhandle = mediabuf_add_reader(1);
	factory_test.work_mode.test_mode.record_fd = open(recordfile,O_RDWR | O_CREAT);
	mediabuf_set_newest_frame(factory_test.work_mode.test_mode.mhandle);
}

void vFactoryWorkMode_StopRecord(void)
{
	close(factory_test.work_mode.test_mode.record_fd);
	mediabuf_del_reader(factory_test.work_mode.test_mode.mhandle);
	vFactoryWorkMode_Play_AudioFile(FACTORY_TEST_AUDIO_STOPRECORD,NETCAM_AUDIO_ENC_A_LAW);
}

void vFactoryWorkMode_StartPlayRecord(char *filename)
{
	vFactoryWorkMode_Play_AudioFile(filename,NETCAM_AUDIO_ENC_RAW_PCM);
}

void vFactoryWorkMode_TestInit(cJSON *json)
{
	factory_test.mode = FACTORY_WORKMODE_TEST;
	factory_test.work_mode.test_mode.flag = 1;
	factory_test.work_mode.test_mode.wifi.flag = 1;
	if(cJSON_GetObjectItem(json,"recode_time"))
	factory_test.work_mode.test_mode.record_time = cJSON_GetObjectItem(json,"recode_time")->valueint;
	if(cJSON_GetObjectItem(json,"wifi_ssid"))
	strcpy(factory_test.work_mode.test_mode.wifi.ssid,cJSON_GetObjectItem(json,"wifi_ssid")->valuestring);
	if(cJSON_GetObjectItem(json,"wifi_passwd"))
	strcpy(factory_test.work_mode.test_mode.wifi.passwd,cJSON_GetObjectItem(json,"wifi_passwd")->valuestring);
	#if 0
	printf("mode=%d\r\n",FACTORY_WORKMODE_TEST);
	printf("recode_time=%d\r\n",factory_test.work_mode.test_mode.record_time);
	printf("wifi_ssid=%s\r\n",factory_test.work_mode.test_mode.wifi.ssid);
	printf("wifi_passwd=%s\r\n",factory_test.work_mode.test_mode.wifi.passwd);
	#endif
	vFactoryWorkMode_Play_AudioFile(FACTORY_TEST_AUDIO_TEST,NETCAM_AUDIO_ENC_A_LAW);
}

void vFactoryWorkMode_BurninInit(cJSON *json)
{
	factory_test.mode = FACTORY_WORKMODE_BURNIN;
	factory_test.work_mode.buinin_mode.flag = 1;
	if(cJSON_GetObjectItem(json,"ptz_loop"))
	factory_test.work_mode.buinin_mode.ptz_loop = cJSON_GetObjectItem(json,"ptz_loop")->valueint;
	
	if(cJSON_GetObjectItem(json,"wifi_ssid"))
	strcpy(factory_test.work_mode.buinin_mode.wifi.ssid,cJSON_GetObjectItem(json,"wifi_ssid")->valuestring);
	else return;
	if(cJSON_GetObjectItem(json,"wifi_passwd"))
	strcpy(factory_test.work_mode.buinin_mode.wifi.passwd,cJSON_GetObjectItem(json,"wifi_passwd")->valuestring);
	else return;
	factory_test.work_mode.buinin_mode.wifi.flag = 1;
	if(cJSON_GetObjectItem(json,"wifi_link_time"))
		factory_test.work_mode.buinin_mode.link_wifi_time = cJSON_GetObjectItem(json,"wifi_link_time")->valueint;
	else
		factory_test.work_mode.buinin_mode.link_wifi_time = 60;
	if(cJSON_GetObjectItem(json,"wifi_unlink_time"))
		factory_test.work_mode.buinin_mode.unlink_wifi_time = cJSON_GetObjectItem(json,"wifi_unlink_time")->valueint;
	else
		factory_test.work_mode.buinin_mode.unlink_wifi_time = 30;
	#if 0
	printf("mode=%d\r\n",FACTORY_WORKMODE_BURNIN);//cJSON_GetObjectItem(json,"mode")->valuestring);
	printf("ptz_loop=%d\r\n",factory_test.work_mode.buinin_mode.ptz_loop);
	printf("wifi_ssid=%s\r\n",factory_test.work_mode.buinin_mode.wifi.ssid);
	printf("wifi_passwd=%s\r\n",factory_test.work_mode.buinin_mode.wifi.passwd);
	printf("wifi_link_time=%d\r\n",factory_test.work_mode.buinin_mode.link_wifi_time);
	printf("wifi_unlink_time=%d\r\n",factory_test.work_mode.buinin_mode.unlink_wifi_time);
	#endif
	vFactoryWorkMode_Play_AudioFile(FACTORY_TEST_AUDIO_BURNIN,NETCAM_AUDIO_ENC_A_LAW);
}

int iFactoryWorkMode_PreInit(char *path,char *cfgfile)
{
	char file[128];
	cJSON *json = NULL;
	sprintf(file,"%s/%s",path,cfgfile);
	if(access(file,F_OK) != 0)
	{
		printf("%s is not exist!!!\r\n",path);
		return -1;
	}
    json = cJSON_ReadFile(file);
	
	if(strcmp(cJSON_GetObjectItem(json,"mode")->valuestring,"test") == 0)
	{
		vFactoryWorkMode_TestInit(json);
	}
	else if(strcmp(cJSON_GetObjectItem(json,"mode")->valuestring,"burnin") == 0)
	{
		vFactoryWorkMode_BurninInit(json);
	}
	cJSON_Delete(json);
	memset(file,0,sizeof(file));
	sprintf(file,"%s/%s",path,FACTORY_TEST_AUDIO_TEST);
	strcpy(factory_test.audio_file.audio_test,file);
	memset(file,0,sizeof(file));
	sprintf(file,"%s/%s",path,FACTORY_TEST_AUDIO_BURNIN);
	strcpy(factory_test.audio_file.audio_burnin,file);
	memset(file,0,sizeof(file));
	sprintf(file,"%s/%s",path,FACTORY_TEST_AUDIO_RECORD);
	strcpy(factory_test.audio_file.audio_record,file);
	return 0;
}

void *vFactoryWorkMode_Proc(void *arg)
{
	char file[128];
	struct timeval last_time;
	struct timeval now_time;
	int loop_flag = 0,Init = 0,RecordFlag = 0;
	int StopRecord = 0;
	int wifi_link_flag = 0;
	factory_test.pthread_ctl.flag = 1;
	memset(file,0,sizeof(file));
	sprintf(file,"%s/%s",FACTORY_TEST_PATH,FACTORY_TEST_CFG);
	sleep(6);
	while(factory_test.pthread_ctl.flag)
	{
		if(access(file,F_OK) != 0)
		{
			if(Init)
			{
				Init = 0;
			}
			if(factory_test.mode == FACTORY_WORKMODE_BURNIN)
			{
				if(loop_flag)
				{
					vFactoryWorkMode_PTZ_StopLoop();
					loop_flag = 0;
				}
				gettimeofday(&last_time, NULL);
			}
			else if(factory_test.mode == FACTORY_WORKMODE_TEST)
			{
				wifi_link_flag = 0;
				RecordFlag = 0;
				StopRecord = 0;
				gettimeofday(&last_time, NULL);
			}			
			if(access("/mnt/sd_card/factorytest/factory_test_cfg.cjson",F_OK) == 0)
			{
				system("rm -Rf /tmp/factorytest");
				usleep(100000);
				system("cp /mnt/sd_card/factorytest /tmp/ -Rf");
				usleep(100000);
				system("sync");
				printf("cp /mnt/sd_card/factorytest /tmp/ -Rf\r\n");
			}
		}
		else
		{
			if(!Init)
			{
				if(0 == iFactoryWorkMode_PreInit(FACTORY_TEST_PATH,FACTORY_TEST_CFG))
				{
					Init = 1;
				}
			}
		}
		if(!Init)
		{
			sleep(1);
			continue;
		}
		if(factory_test.mode == FACTORY_WORKMODE_BURNIN)
		{
			#if 0
			//ptz  loop
			//(!loop_flag)
			if(factory_test.work_mode.buinin_mode.ptz_loop)
			{
			//	loop_flag = 1;
				vFactoryWorkMode_PTZ_Loop();
			}
			#endif
			//link wifi
			if(!wifi_link_flag)
			{//wifi link
				gettimeofday(&now_time, NULL);
				if(now_time.tv_sec - last_time.tv_sec >= factory_test.work_mode.buinin_mode.unlink_wifi_time)
				{
					if(factory_test.work_mode.buinin_mode.ptz_loop)
					{
						vFactoryWorkMode_PTZ_Loop();
					}
					vFactoryWorkMode_LinkWifi(factory_test.work_mode.buinin_mode.wifi.ssid,factory_test.work_mode.buinin_mode.wifi.passwd);
					wifi_link_flag = 1;
					gettimeofday(&last_time, NULL);
				}
			}
			else
			{//wifi unlink
				gettimeofday(&now_time, NULL);
				if(now_time.tv_sec - last_time.tv_sec >= factory_test.work_mode.buinin_mode.link_wifi_time)
				{
					vFactoryWorkMode_StopLinkWifi();
					wifi_link_flag = 0;
					gettimeofday(&last_time, NULL);
				}
			}
		}
		else if(factory_test.mode == FACTORY_WORKMODE_TEST)
		{
			//record audio
			//link wifi
			if(!wifi_link_flag)
			{
				if(factory_test.work_mode.test_mode.flag)
				{
					if(factory_test.work_mode.test_mode.wifi.flag)
					{
						wifi_link_flag = 1;
						printf("test mode wifi link\r\n");
						vFactoryWorkMode_LinkWifi(factory_test.work_mode.test_mode.wifi.ssid,factory_test.work_mode.test_mode.wifi.passwd);
						sleep(6);
					}
				}
			}
			if(!RecordFlag)
			{
				vFactoryWorkMode_StartRecord();
				RecordFlag = 1;
				gettimeofday(&last_time, NULL);
				printf("record time = %d\r\n",factory_test.work_mode.test_mode.record_time);
			}
			if(RecordFlag && !StopRecord)
			{
				gettimeofday(&now_time, NULL);
				if(now_time.tv_sec - last_time.tv_sec >= factory_test.work_mode.test_mode.record_time + 1)
				{
					vFactoryWorkMode_StopRecord();
					usleep(50000);
					vFactoryWorkMode_Play_AudioFile(FACTORY_WORKMODE_TEST_RECORD_FILE,NETCAM_AUDIO_ENC_RAW_PCM);
					StopRecord = 1;
				}
				else
				{
					vFactoryWorkMode_WriteRecordFile(factory_test.work_mode.test_mode.record_fd);
					usleep(10000);
					continue;
				}
			}
		}
		usleep(20000);
	}
	return NULL;
}
void vFactoryWorkMode_Init(void)
{
	memset(&factory_test,0,sizeof(FACTORY_TEST_S));
	if(pthread_create(&(factory_test.pthread_ctl.phreadid),NULL,vFactoryWorkMode_Proc,NULL) < 0)
	{
		printf("Create vFactoryWorkMode_Init pthread failure\n");
		return;
	}
}
void vFactoryWorkMode_Exit(void)
{
	factory_test.pthread_ctl.flag = 0;
	if(factory_test.pthread_ctl.phreadid > 0)
	{
		pthread_join(factory_test.pthread_ctl.phreadid,NULL);
		factory_test.pthread_ctl.phreadid = 0;
	}
	memset(&factory_test,0,sizeof(FACTORY_TEST_S));
}

#if 0
int main()
{
	vFactoryWorkMode_Init(FACTORY_TEST_PATH,FACTORY_TEST_CFG);
	return 0;
}
#endif

