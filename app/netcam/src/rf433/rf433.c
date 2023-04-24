#include"rf433.h"
#include"gk7101.h"

strRf433PthreadCtl rf433pthreadCtl;
strRf433Dev rf433Dev;
#ifdef MODULE_SUPPORT_TUTK
extern void tutk_rf433_learn_handle(int flag);
extern void tutk_rf433_event_handle(int areaid,int type,int code);
#endif
extern void gk_cms_rf433_learn_handle(int flag);
extern void gk_cms_rf433_event_handle(int areaid,int type,int code);

void rf433_learn_handle(int flag)
{
#ifdef MODULE_SUPPORT_TUTK
	tutk_rf433_learn_handle(flag);
#endif

#ifdef MODULE_SUPPORT_RF433
	gk_cms_rf433_learn_handle(flag);
#endif
}

void rf433_event_handle(int areaid,int type,int code)
{
#ifdef MODULE_SUPPORT_TUTK
	tutk_rf433_event_handle(areaid,type,code);
#endif

#ifdef MODULE_SUPPORT_RF433
	gk_cms_rf433_event_handle(areaid,type,code);
#endif
}



int rf433_ReadData(void)
{
	if(rf433pthreadCtl.fd <= 0) {RF433_DEBUG("Error 433 dev fd\r\n");return -1;}
	int rf433Data = -1;
	unsigned char buf[32];
	unsigned char buflen;
	unsigned int Ret = 0;
	
	strSbiData pData;
	pData.pbBuffer = buf;
	pData.pbBufferLen = &buflen;
	pData.pRet = &Ret;
	if(ioctl(rf433pthreadCtl.fd,CMD_RECV,&pData) != -1)
	{
		if(Ret > 0)
		{
			rf433Data = (pData.pbBuffer[1] << 16) | (pData.pbBuffer[2] << 8) | pData.pbBuffer[3];
			return rf433Data;
		}
	}
	return -1;
}

int rf433_get_status(void)
{
	int status;
	pthread_mutex_lock(&(rf433pthreadCtl.mutex));
	status = rf433pthreadCtl.status;
	pthread_mutex_unlock(&(rf433pthreadCtl.mutex));
	return status;
}

void rf433_set_status(int status)
{
	pthread_mutex_lock(&(rf433pthreadCtl.mutex));
	rf433pthreadCtl.status = status;
	pthread_mutex_unlock(&(rf433pthreadCtl.mutex));
}

void rf433_learn_code(int codetype,char *name)
{
	if(!name) return;
	RF433_DEBUG("start learning code \r\n");
	rf433_set_status(RF433STATUS_LEARN);
	rf433pthreadCtl.codetype = codetype;
	#if 0
	int ii;
	for(ii=0;ii<RF433NAMESIZE_MAX;ii++)
	{
		if((name[ii] <= '9' && name[ii] >= '0')||
			(name[ii] <= 'z' && name[ii] >= 'a')||
			(name[ii] <= 'Z' && name[ii] >= 'A'))
		{
		}
		else
		{
			name[ii] = 0;
			break;
		}
	}
	#endif
	memset(rf433pthreadCtl.name,0,RF433NAMESIZE_MAX);
	memcpy(rf433pthreadCtl.name,name,strlen(name));
}
void rf433_clean_code(int codetype,char *name)
{
	rf433_set_status(RF433STATUS_IDLE);
	rf433pthreadCtl.codetype = 0;
	memset(rf433pthreadCtl.name,0,sizeof(RF433NAMESIZE_MAX));
}

void rf433_save_code(strRf433Dev *rf433,char *file)
{
	FILE *fp = NULL;
	if(access(file,F_OK) == 0)
	{
		fp = fopen(file,"r+");
		if(fp == NULL)
		{
			RF433ERR_PRINT("Open 433 data file failure\n");
			return;
		}
		fwrite(rf433,sizeof(strRf433Dev),1,fp);
		fclose(fp);
	}
}



int rf433_get_code(strRf433Dev *rf433,char *file)
{
	if(!rf433 || !file) return -1;
	FILE *fp = NULL;
	if(access(file,F_OK) != 0)
	{
		//文件不存在，要重新创建文件
		fp = fopen(file,"wb+");
		memset(rf433,0,sizeof(strRf433Dev));
		if(fp == NULL)
		{
			RF433ERR_PRINT("Open 433 data file failure\n");
			return -1;
		}
		fwrite(rf433,sizeof(strRf433Dev),1,fp);
		fclose(fp);
	}
	else
	{
		fp = fopen(file,"r");
		if(fp == NULL)
		{
			RF433ERR_PRINT("Open 433 data file failure\n");
			return -1;
		}
		fread(rf433,sizeof(strRf433Dev),1,fp);
		fclose(fp);
	}
	return 0;
}

void rf433_delete_code(int codeid)
{
	if(codeid >= RF433NUMBER_MAX || codeid < 0) return;
	if(rf433Dev.useflag[codeid])
	{
		rf433Dev.useflag[codeid] = 0;
		memset((unsigned char *)(&rf433Dev.Data[codeid]),0,sizeof(strRf433Data));
		rf433_save_code(&rf433Dev,rf433pthreadCtl.CodeDataFile);
	}
}
int rf433_modify_code_name(int codeid,char *name)
{
	if(name == NULL) return -1;
	if(rf433Dev.useflag[codeid])
	{
		#if 0
		int ii;
		for(ii=0;ii<RF433NAMESIZE_MAX;ii++)
		{
			if((name[ii] <= '9' && name[ii] >= '0')||
				(name[ii] <= 'z' && name[ii] >= 'a')||
				(name[ii] <= 'Z' && name[ii] >= 'A'))
			{
			}
			else
			{
				name[ii] = 0;
				break;
			}
		}
		#endif
		memset(rf433Dev.Data[codeid].name,0,RF433NAMESIZE_MAX);
		memcpy(rf433Dev.Data[codeid].name,name,strlen(name));
		rf433_save_code(&rf433Dev,rf433pthreadCtl.CodeDataFile);
		return 0;
	}
	else
	{
		return -1;
	}
}



int rf433_is_exist(strRf433Dev *rf433,int code)
{
	if(!rf433) return -1;
	int ii;
	for(ii = 0;ii < RF433NUMBER_MAX;ii ++)
	{
		if(rf433->useflag[ii])
		{
			if(rf433->Data[ii].type == RF433_DEV_TYPE_CONTROL)
			{
				if(((rf433->Data[ii].code) >> 4) == (code >> 4))
				{
					RF433_DEBUG("Dev%dCode = 0x%x\r\n",ii,rf433->Data[ii].code);
					return ii;
				}
			}
			else if(rf433->Data[ii].type == RF433_DEV_TYPE_ALARM)
			{
				if(rf433->Data[ii].code == code)
				{
					RF433_DEBUG("Dev%dCode = 0x%x\r\n",ii,rf433->Data[ii].code);
					return ii;
				}
			}
		}
	}
	return -1;
}

int rf433_find_save_pos(strRf433Dev *rf433)
{
	if(!rf433) return -1;
	int ii;
	for(ii = 0;ii < RF433NUMBER_MAX;ii ++)
	{
		if(!(rf433->useflag[ii]))
		{
			return ii;
		}
	}
	return -1;
}

int rf433_get_dev_num(void)
{
	int ii,dev_num = 0;
	for(ii = 0;ii < RF433NUMBER_MAX;ii ++)
	{
		if(rf433Dev.useflag[ii])
		{
			dev_num ++;
		}
	}
	return dev_num;
}

void rf433_get_dev_corresponding_info(int seq,int *id,int *type,char *name)
{
	int ii,dev_num = 0;
	for(ii = 0;ii < RF433NUMBER_MAX;ii ++)
	{
		if(rf433Dev.useflag[ii])
		{
			//RF433_DEBUG("ii=%d,flag=%d,dev_num=%d,seq=%d,id=%d,type=%d,name=%s\r\n",ii,rf433Dev.useflag[ii],dev_num,seq,rf433Dev.Data[seq].areaid,rf433Dev.Data[seq].type,rf433Dev.Data[seq].name);
			dev_num ++;
			if(seq == dev_num - 1)
			{
				*id = rf433Dev.Data[ii].areaid;
				*type = rf433Dev.Data[ii].type;
				memcpy(name,rf433Dev.Data[ii].name,strlen(rf433Dev.Data[ii].name));
				break;
			}
		}
	}
}
void rf433_exit_learn(void)
{
	rf433_set_status(RF433STATUS_IDLE);
}
int rf433_get_dev_type(int id)
{
	if(rf433Dev.useflag[id] && id < RF433NUMBER_MAX && id >= 0)
	{
		return rf433Dev.Data[id].type;
	}
	else
	{
		return -1;
	}
}

void *rf433_proc(void* arg)
{
	int code = -1,areaid;
	int status;
	int pos;
	int learn_flag = RF433_FALSE;
	struct timeval begin_learncode_time;
	struct timeval time;
	RF433_DEBUG("433 proc start\n");
	while(!rf433pthreadCtl.exit_proc)
	{
		status = rf433_get_status();
		if(status == RF433STATUS_LEARN)
		{
			if(!learn_flag)
			{
				learn_flag = RF433_TRUE;
				gettimeofday(&begin_learncode_time, NULL);
			}
			gettimeofday(&time, NULL);
			if((time.tv_sec - begin_learncode_time.tv_sec) > RF433LEARNTIMEOUT)
			{
				rf433pthreadCtl.learn_handle(AVIOCTRL_CFG_433_TIMEOUT);
				rf433_set_status(RF433STATUS_IDLE);
				learn_flag = RF433_FALSE;
			}
		}
		else
		{
			learn_flag = RF433_FALSE;
		}
		code = rf433_ReadData();
		if(code != -1)
		{
			//receive 433 data
			RF433_DEBUG("433Data = 0x%x\n",code);
			areaid = rf433_is_exist(&rf433Dev,code);
			switch(status)
			{
				case RF433STATUS_IDLE:
				{
					if((areaid >= 0) && (areaid < RF433NUMBER_MAX))
					{
						rf433pthreadCtl.event_handle(areaid,rf433_get_dev_type(areaid),code);//rf433_event_handle(areaid,RF433_DEV_TYPE_ALARM);
					}
				}
				break;
				case RF433STATUS_LEARN:
				{
					pos = rf433_find_save_pos(&rf433Dev);
					if((pos >= 0) && (pos < RF433NUMBER_MAX))
					{
						if(areaid < 0)
						{
							rf433Dev.useflag[pos] = 1;
							rf433Dev.Data[pos].areaid = pos;
							rf433Dev.Data[pos].type = rf433pthreadCtl.codetype;
							rf433Dev.Data[pos].code = code;
							if(strlen(rf433pthreadCtl.name) > 0)
							{
								memset(rf433Dev.Data[pos].name,0,RF433NAMESIZE_MAX);
								memcpy(rf433Dev.Data[pos].name,rf433pthreadCtl.name,sizeof(rf433pthreadCtl.name));
								rf433_save_code(&rf433Dev,rf433pthreadCtl.CodeDataFile);
							}
							rf433pthreadCtl.learn_handle(AVIOCTRL_CFG_433_OK);
							RF433_DEBUG("areaid = %d,pos=%d,type=%d,learn code success!!!\r\n",areaid,pos,rf433Dev.Data[pos].type);
						}
						else
						{
							rf433pthreadCtl.learn_handle(AVIOCTRL_CFG_433_EXISTS);
							RF433_DEBUG("code exist!!!\r\n");
						}
					}
					else
					{
						rf433pthreadCtl.learn_handle(AVIOCTRL_CFG_433_MAX);
						RF433_DEBUG("code max!!!\r\n");
					}
					rf433_set_status(RF433STATUS_IDLE);
				}
				break;
				default:
				break;
			}
		}
		usleep(10000);
	}
	RF433_DEBUG("433 proc end\n");
	return NULL;
}


int rf433_Init(char *codeDataPath)
{
	RF433_DEBUG("Init rf433 dev\r\n\r\n");
	system("/sbin/insmod /lib/modules/3.4.43-gk/extra/sbi.ko");
	sleep(1);
	memset(&rf433pthreadCtl,0,sizeof(strRf433PthreadCtl));
	rf433pthreadCtl.exit_proc = -1;
	if(access("/dev/goke_sbi",F_OK) != 0) return -1;
	rf433pthreadCtl.fd  = -1; 
	rf433pthreadCtl.fd = open("/dev/goke_sbi",O_RDONLY);

	if(rf433pthreadCtl.fd <= 0)
	{
		RF433ERR_PRINT("Unable open /dev/goke_sbi device\n");
		return -1;
	}
	if(pthread_mutex_init(&(rf433pthreadCtl.mutex), NULL) < 0)
	{
		RF433ERR_PRINT("Init rf433 phread mutex failure\n");
		return -1;
	}
	rf433pthreadCtl.exit_proc = 0;
	if(pthread_create(&(rf433pthreadCtl.phreadid),NULL,rf433_proc,NULL) < 0)
	{
		RF433ERR_PRINT("Create rf433 pthread failure\n");
		return -1;
	}
       
	rf433pthreadCtl.status = RF433STATUS_IDLE;//RF433STATUS_LEARN;//RF433STATUS_IDLE;
	if(codeDataPath == NULL)
		memcpy(rf433pthreadCtl.CodeDataFile,RF433DATAFILE,strlen(RF433DATAFILE));
	else
		memcpy(rf433pthreadCtl.CodeDataFile,codeDataPath,strlen(codeDataPath));
	rf433pthreadCtl.learn_handle = rf433_learn_handle;
	rf433pthreadCtl.event_handle = rf433_event_handle;
	rf433_get_code(&rf433Dev,rf433pthreadCtl.CodeDataFile);
	RF433_DEBUG("Init rf433 success!!!\r\n");
	return 0;
}

void rf433_Exit(void)
{
	rf433pthreadCtl.exit_proc = 1;
	if(rf433pthreadCtl.phreadid > 0)
	{
		pthread_join(rf433pthreadCtl.phreadid,NULL);
		rf433pthreadCtl.phreadid = 0;
	}
	pthread_mutex_destroy(&(rf433pthreadCtl.mutex));
	close(rf433pthreadCtl.fd);
	rf433pthreadCtl.fd = -1;
}

#if 0
int main()
{
	if(rf433_Init("/home/rf433.data") < 0)
	{
		return -1;
	}
	while(1)
	{
		
	}
	rf433_Exit();
	return 0;
}
#endif

