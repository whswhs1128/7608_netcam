#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>
#include <AVSTREAM_IO_Proto.h>
#include <PPCS_API.h>
#include <AVAPIs.h>
#include <IOTCAPIs.h>
#include <AVFRAMEINFO.h>
#include <AVIOCTRLDEFs.h>
#include "utility.h"
#define  DATA_TYPE_IOCTRL   0
#define  DATA_TYPE_AUDIO    1
#define  DATA_TYPE_VIDEO    2
#define PACKAGE_HEAD      0xfefefafa
#define MAX_CHANNEL      256
#define MAX_SESSION      256

typedef struct{	
	INT32  package_head;
	INT32  package_size;
	INT32  frameinfo_size;
	INT32  frame_size;					
} st_PACKAGE_INFO;
typedef struct{
	int result;
	st_PACKAGE_INFO packinfo;
	char *Data;
	int len;
	void *next;
}st_PACKAGE;	
typedef struct{	
	INT32  nIOTCSessionID;					
	authFn pfxAuthFn;
	int    nRun;
	int    nWaitforKeyFrame;
	int    nBufTooBig;
	CHAR cszViewAccount[24];					
	CHAR cszViewPassword[24];				  
	INT32 nTimeout;						
	INT32 nServType;
	CHAR 	nIOTCChannelID;	
	char *sendAudioData;
	int   sendAudioSize;
	char *sendVideoData;
	int   sendVideoSize;
	char *sendIOCtrlData;
	int   sendIOCtrlSize;	
	
        int avStartRecvAudio;
	int avStartRecvVideo;
	st_PACKAGE_INFO package_audio;
	st_PACKAGE_INFO package_video;
	st_PACKAGE_INFO package_ioctrl;
	st_PACKAGE *iocttl_head;
	st_PACKAGE *video_head;	
	st_PACKAGE *audio_head;
		
	pthread_t Thread_Video_Rec_ID;
	pthread_cond_t video_cond;
	pthread_mutex_t video_lock;
	pthread_t Thread_Audio_Rec_ID;
	pthread_cond_t audio_cond;
	pthread_mutex_t audio_lock;
	pthread_t Thread_IoCtrl_Rec_ID;
	pthread_cond_t ioctrl_cond;
	pthread_mutex_t ioctrl_lock;
	pthread_mutex_t ioctrl_listmutex;
	pthread_mutex_t video_listmutex;
	pthread_mutex_t audio_listmutex;
	pthread_mutex_t mutex;
} st_PPCS_Channel;


st_PPCS_Channel g_ppcs_channel[MAX_CHANNEL];
avlogCB avlogcbmsg = NULL;

st_PACKAGE* Packet_Alloc(st_PACKAGE_INFO *package_info, char *buf, int nSize,int result)
{
	  st_PACKAGE *p = ( st_PACKAGE *)malloc(sizeof(st_PACKAGE));
	  if(p)
	  {
	     p->Data = NULL;
	     p->len = nSize;
	     p->result = result;
	     p->next= NULL;
		 memset(&p->packinfo,0,sizeof(st_PACKAGE_INFO));
	     if(nSize>0 && package_info!= NULL && buf!= NULL)
	     	{
	      p->Data = 	(char*)malloc(nSize+1);
	      if(p->Data== NULL)
	     	{
	     		   free(p);
	     		   return NULL;
	     	}
	        memcpy(p->Data,buf,nSize);
	       memcpy(&p->packinfo,package_info,sizeof(st_PACKAGE_INFO));    		
	     	}

	     
	  }
	  return p;
}
void Packet_Free(st_PACKAGE *p)
{
	  if(p!= NULL)
	  {
	  		if(p->Data!= NULL)
	  		{
	  		  free(p->Data);
	  		  p->Data =NULL;	
	  		}
	  		free(p);
	  		p = NULL;
	  }
}
void Add_Ioctrl_Packet(st_PPCS_Channel *pChan,st_PACKAGE *p)
{
     pthread_mutex_lock(&pChan->ioctrl_listmutex);
	   st_PACKAGE *last = NULL;

     if(pChan->iocttl_head==NULL)
       {
			pChan->iocttl_head = p;
	   }
	   else
	   {
	      last = pChan->iocttl_head;
          while(last->next!= NULL)
          {
				   last = (st_PACKAGE*)last->next;
		      }
		  last->next = p;
	   }
	 
	pthread_mutex_unlock(&pChan->ioctrl_listmutex); 
}
st_PACKAGE* Get_Ioctrl_Packet(st_PPCS_Channel *pChan)
{
   pthread_mutex_lock(&pChan->ioctrl_listmutex);
	 st_PACKAGE *packet= NULL;
	 if(pChan->iocttl_head==NULL)
	 {
	    // Log("Get_Ioctrl_Packet:111\n");
	    pthread_mutex_unlock(&pChan->ioctrl_listmutex); 
		  return NULL;
	 }
	 else
	 {
	 	//Log3("Get_Ioctrl_Packet:222\n");
		packet = pChan->iocttl_head;
		pChan->iocttl_head = (st_PACKAGE*)pChan->iocttl_head->next;
		
	 }
	 pthread_mutex_unlock(&pChan->ioctrl_listmutex); 
	 return packet;
}
void ReleaseAll_Ioctrl_Packet(st_PPCS_Channel *pChan)
{
  pthread_mutex_lock(&pChan->ioctrl_listmutex);
	st_PACKAGE *packet= NULL;
	if(pChan->iocttl_head!=NULL)
	{
      while(pChan->iocttl_head)
      {
		  packet = pChan->iocttl_head;
		  pChan->iocttl_head =(st_PACKAGE*) pChan->iocttl_head->next;
		  Packet_Free(packet);

	  }
	}
   pthread_mutex_unlock(&pChan->ioctrl_listmutex); 
}
int Check_Ioctrl_Packet(st_PPCS_Channel *pChan)
{
	int result = 0;
  pthread_mutex_lock(&pChan->ioctrl_listmutex);
	if(pChan->iocttl_head!=NULL)
	{
    result = 1; 
	}
   pthread_mutex_unlock(&pChan->ioctrl_listmutex);
   return result; 	
}

void Add_Audio_Packet(st_PPCS_Channel *pChan,st_PACKAGE *p)
{
     pthread_mutex_lock(&pChan->audio_listmutex);
	   st_PACKAGE *last = NULL;

     if(pChan->audio_head==NULL)
       {
			pChan->audio_head = p;
	   }
	   else
	   {
	      last = pChan->audio_head;
          while(last->next!= NULL)
          {
				   last = (st_PACKAGE*)last->next;
		      }
		  last->next = p;
	   }
	 
	pthread_mutex_unlock(&pChan->audio_listmutex); 
}
st_PACKAGE* Get_Audio_Packet(st_PPCS_Channel *pChan)
{
   pthread_mutex_lock(&pChan->audio_listmutex);
	 st_PACKAGE *packet= NULL;
	 if(pChan->audio_head==NULL)
	 {
	    pthread_mutex_unlock(&pChan->audio_listmutex); 
		  return NULL;
	 }
	 else
	 {
		packet = pChan->audio_head;
		pChan->audio_head = (st_PACKAGE*)pChan->audio_head->next;
		
	 }
	 pthread_mutex_unlock(&pChan->audio_listmutex); 
	 return packet;
}
void ReleaseAll_Audio_Packet(st_PPCS_Channel *pChan)
{
  pthread_mutex_lock(&pChan->audio_listmutex);
	st_PACKAGE *packet= NULL;
	if(pChan->audio_head!=NULL)
	{
      while(pChan->audio_head)
      {
		  packet = pChan->audio_head;
		  pChan->audio_head =(st_PACKAGE*) pChan->audio_head->next;
		  Packet_Free(packet);

	  }
	}
   pthread_mutex_unlock(&pChan->audio_listmutex); 
}
int Check_Audio_Packet(st_PPCS_Channel *pChan)
{
	int result = 0;
  pthread_mutex_lock(&pChan->audio_listmutex);
	if(pChan->audio_head!=NULL)
	{
    result = 1; 
	}
   pthread_mutex_unlock(&pChan->audio_listmutex);
   return result; 	
}

void Add_Video_Packet(st_PPCS_Channel *pChan,st_PACKAGE *p)
{
     pthread_mutex_lock(&pChan->video_listmutex);
	   st_PACKAGE *last = NULL;

     if(pChan->video_head==NULL)
       {
			pChan->video_head = p;
	   }
	   else
	   {
	      last = pChan->video_head;
          while(last->next!= NULL)
          {
				   last = (st_PACKAGE*)last->next;
		      }
		  last->next = p;
	   }
	 
	pthread_mutex_unlock(&pChan->video_listmutex); 
}
st_PACKAGE* Get_Video_Packet(st_PPCS_Channel *pChan)
{
   pthread_mutex_lock(&pChan->video_listmutex);
	 st_PACKAGE *packet= NULL;
	 if(pChan->video_head==NULL)
	 {
	    pthread_mutex_unlock(&pChan->video_listmutex); 
		  return NULL;
	 }
	 else
	 {
		packet = pChan->video_head;
		pChan->video_head = (st_PACKAGE*)pChan->video_head->next;
		
	 }
	 pthread_mutex_unlock(&pChan->video_listmutex); 
	 return packet;
}
void ReleaseAll_Video_Packet(st_PPCS_Channel *pChan)
{
  pthread_mutex_lock(&pChan->video_listmutex);
	st_PACKAGE *packet= NULL;
	if(pChan->video_head!=NULL)
	{
      while(pChan->video_head)
      {
		  packet = pChan->video_head;
		  pChan->video_head =(st_PACKAGE*) pChan->video_head->next;
		  Packet_Free(packet);

	  }
	}
   pthread_mutex_unlock(&pChan->video_listmutex); 
}
int Check_Video_Packet(st_PPCS_Channel *pChan)
{
	int result = 0;
    pthread_mutex_lock(&pChan->video_listmutex);
	if(pChan->video_head!=NULL)
	{
    result = 1; 
	}
   pthread_mutex_unlock(&pChan->video_listmutex);
   return result; 	
}


void avSetlogByCallBackFn(avlogCB pfxavLogcbFn)
{
	avlogcbmsg = pfxavLogcbFn;
}
int avGetServAVChannel(int nIOTCSessionID, unsigned char nIOTCChannelID)
{
	 	  INT32 i =0;
 	 	  INT32 result =-1;
 	   for( i =0;i<MAX_CHANNEL;i++)
 	   {
 	   	 if(g_ppcs_channel[i].nIOTCSessionID ==nIOTCSessionID && g_ppcs_channel[i].nIOTCChannelID == nIOTCChannelID)
 	   	 {
 	   	 	 result = i;
 	   	 	 break;
 	   	 }

 	   }
 	   return result;
}
int avGetClientAVChannel(int nIOTCSessionID, unsigned char nIOTCChannelID)
{
	 	  INT32 i =0;
 	 	  INT32 result =-1;
 	   for( i =0;i<MAX_CHANNEL;i++)
 	   {
 	   	 if(g_ppcs_channel[i].nIOTCSessionID ==nIOTCSessionID && g_ppcs_channel[i].nIOTCChannelID == nIOTCChannelID)
 	   	 {
 	   	 	 result = i;
 	   	 	 break;
 	   	 }

 	   }
 	   return result;
}
 int avGetServFreeAVChannel(void)
 {
 	 	  INT32 i =0;
 	 	  INT32 result =-1;
 	   for( i =0;i<MAX_CHANNEL;i++)
 	   {
 	   	 if(g_ppcs_channel[i].nIOTCSessionID == -1)
 	   	 {
 	   	 	 result = i;
 	   	 	 break;
 	   	 }

 	   }
 	   return result;
 }
  int avGetClientFreeAVChannel(void)
 {
 	 	  INT32 i =0;
 	 	  INT32 result =-1;
 	   for( i =0;i<MAX_CHANNEL;i++)
 	   {
 	   	 if(g_ppcs_channel[i].nIOTCSessionID == -1)
 	   	 {
 	   	 	 result = i;
 	   	 	 break;
 	   	 }

 	   }
 	   return result;
 }
 #define AUDIO_BUF_SIZE  4*1024
 static void *thread_ReceiveAudio(void *arg)
 {
 	  //  pthread_detach(pthread_self());
 	    st_PPCS_Channel *pChan = (st_PPCS_Channel*)arg;
 	    char szabBuf[AUDIO_BUF_SIZE];
 	    int nTimeout = 3600;
 	    UINT32 WriteSize;
 	    UINT32 ReadSize;
 	    st_PACKAGE_INFO *packge = NULL;
 	    int read;
 	    int ret;
 	    st_PACKAGE *pa = NULL;
 	    int start = 0;
 	    int left = AUDIO_BUF_SIZE;
 	    char *buf = szabBuf;
 	    char *readstart;
 	    char *tmp;
 	    int count;
 	    char *data;
 	    
 	    Log3("thread_ReceiveAudio: SessionID = %d\n", pChan->nIOTCSessionID);
 	    while(pChan->nRun == 1 && pChan->nIOTCSessionID!= -1)
 	   {
 	   	  
 	  		   	  	
 	   	  ret = PPCS_Check_Buffer(pChan->nIOTCSessionID,DATA_TYPE_AUDIO,&WriteSize,&ReadSize);
 	   	  if(ret<0)
 	   	  	{
 	   	  		       Log3("thread_ReceiveAudio: PPCS_Check_Buffer error %d\n", ret);
	 	   	  	         if(pChan->avStartRecvAudio==1)
					    {
		 	   	  		 pa = Packet_Alloc(NULL,NULL,0,ret);
	 	   	  	 		 if(pa!= NULL)
	 	   	  	  	 	{
	 	   	  	  	  	    Add_Audio_Packet(pChan,pa);	
	 	   	  	  	 	} 
					    }
     			  		pthread_mutex_lock(&pChan->audio_lock);
 	    				pthread_cond_signal(&pChan->audio_cond); 
 	    				pthread_mutex_unlock(&pChan->audio_lock); 
 	   	  	  		break;
 	   	  	}
   	     if(ReadSize>0)
 	   	   {
			 readstart = buf+start;
			 if(ReadSize>AUDIO_BUF_SIZE-start)
			 {
				  left = AUDIO_BUF_SIZE-start;
		     }
			 else
			 {
				 left = ReadSize;
			 }  	   	       	   	        	    	      
 	   	     read = PPCS_Read(pChan->nIOTCSessionID,DATA_TYPE_AUDIO,readstart,&left,nTimeout);
 	   	    	
 	   	  	  if(read<0)
 	   	  	  {
 	   	  	  				if(read== ERROR_PPCS_TIME_OUT)
 	   	  	  				{	   	  	  				
 	   	  	  	 	  	 	 continue;
 	   	  	  			 	 }
							 if(pChan->avStartRecvAudio==1)
							  {
 	   	  	  			 		 pa = Packet_Alloc(NULL,NULL,0,read);
 	   	  	 					 if(pa!= NULL)
	 	   	  	  				{
	 	   	  	  	  			 Add_Audio_Packet(pChan,pa);	
	 	   	  	  				} 
							 }
 	   	  	  				pthread_mutex_lock(&pChan->audio_lock);
 	    					pthread_cond_signal(&pChan->audio_cond); 
 	    					pthread_mutex_unlock(&pChan->audio_lock); 
 	   	  	  				break;
 	   	  	  }
 	   	  	  else
 	   	  	  {
 	   	  	  	
 	   	  	  	  
 	   	  	  	  count = 0;
 	   	  	  	  left+= start;
 	   	  	  	  packge = (st_PACKAGE_INFO *)buf;	
 	   	  	  	  tmp = buf;  
 	   	  	  	 // printf("PPCS_Read audio = %d   %d   %d   %d\n", left,read,start,packge->package_size+sizeof(st_PACKAGE_INFO)); 	  	  	  
 	   	  	  	  while(left>=sizeof(st_PACKAGE_INFO)+packge->package_size)
 	   	  	  	  {
 	   	  	  	  	
 	   	  	  	  	if(packge->package_head != PACKAGE_HEAD)
 	   	  	  	  	{
 	   	  	  	  			Log3("packge->package_head is error = %d  \n", packge->package_size);
							while(left>0)
							
						  {							
							packge = (st_PACKAGE_INFO *)tmp;							
							if(packge->package_head == PACKAGE_HEAD)							
							{							
									memcpy(buf,tmp,left);							
									tmp = buf;							
									break;
							
							}
							tmp ++;
							left--;
							}
							packge = (st_PACKAGE_INFO *)buf;
							if(left>=sizeof(st_PACKAGE_INFO)+packge->package_size)
							{
								tmp = buf;
							}
							else
							{
							start = left;
							buf[start] = 0;
							break;
							}

 	   	  	  	  	}
 	   	  	  	  	
 	   	  	  	  	data = tmp+sizeof(st_PACKAGE_INFO);
					if(pChan->avStartRecvAudio==1)
					{
 	   	  	  	  		pa = Packet_Alloc(packge,data,packge->package_size,read);
 	   	  	 				if(pa!= NULL)
 	   	  	  			{
 	   	  	  	  			 Add_Audio_Packet(pChan,pa);	
 	   	  	  			} 
					}
 	   	  	  	  	left-= sizeof(st_PACKAGE_INFO)+packge->package_size;
 	   	  	  	  	tmp+= sizeof(st_PACKAGE_INFO)+packge->package_size;
 	   	  	  	  	count+=sizeof(st_PACKAGE_INFO)+packge->package_size;
 	   	  	  	  	packge = (st_PACKAGE_INFO *)tmp;
 	   	  	  	  	  
 	   	  	  	  }
 	   	  	  	  if(left>0)
 	   	  	  	  {
 	   	  	  	  	  memcpy(buf,tmp,left);
 	   	  	  	  	  start = left;
 	   	  	  	  }
 	   	  	  	  else
 	   	  	  	  {
 	   	  	  	  	start = 0;
 	   	  	  	  }
 	   	  	  	  
 	   	  	  }
    			  pthread_mutex_lock(&pChan->audio_lock);
 	    		  pthread_cond_signal(&pChan->audio_cond); 
 	    		  pthread_mutex_unlock(&pChan->audio_lock); 
 	   	  	}
 	   	  	else
 	   	  	{
 	   	  		
 	   	  	     	if(Check_Audio_Packet(pChan) == 1)
            	    {
    							pthread_mutex_lock(&pChan->audio_lock);
 	    						pthread_cond_signal(&pChan->audio_cond); 
 	    						pthread_mutex_unlock(&pChan->audio_lock); 
 	   	  	 				    usleep(200);	
            	    }
            	    else
            	    {
            	    usleep(1000);		
            	    }
            	    
 	   	  		 
 	   	  	} 	
 	   }
 	    Log3("thread_ReceiveAudio: pthread_exit\n");
 	    return NULL;
 }
 #define VIDEO_BUF_SIZE  60*1024
 static void *thread_ReceiveVideo(void *arg)
 {
 	   //pthread_detach(pthread_self());
 	  	st_PPCS_Channel *pChan = (st_PPCS_Channel*)arg;
 	    char *szabBuf = (char*)malloc((VIDEO_BUF_SIZE)*sizeof(char));;
 	    int nTimeout = 3600;
 	    UINT32 WriteSize;
 	    UINT32 ReadSize;
 	    st_PACKAGE_INFO *packge = NULL;
 	    int read;
 	    int ret; 	     
 	    st_PACKAGE *pa = NULL;
 	    int start = 0;
 	    int left = AUDIO_BUF_SIZE;
 	    char *buf = szabBuf;
 	    char *readstart;
 	    char *tmp;
 	    int count;
 	    char *data;
 	    
 	    Log3("thread_ReceiveVideo: SessionID = %d\n", pChan->nIOTCSessionID);
	
 	    while(pChan->nRun == 1&& pChan->nIOTCSessionID!= -1)
 	   {	   	      	  	
 	   	  ret = PPCS_Check_Buffer(pChan->nIOTCSessionID,DATA_TYPE_VIDEO,&WriteSize,&ReadSize);
 	   	  if(ret<0)
 	   	  {
 	   	  		   Log3("thread_ReceiveVideo: PPCS_Check_Buffer error %d\n", ret);
	 	   	  	        if(pChan->avStartRecvVideo==1)
					{
		 	   	  		 pa = Packet_Alloc(NULL,NULL,0,ret);
	 	   	  	 		 if(pa!= NULL)
	 	   	  	  	 	{
	 	   	  	  	  			  Add_Video_Packet(pChan,pa);	
	 	   	  	  	 	} 
					}
 	   	  	  	   pthread_mutex_lock(&pChan->video_lock);
 	    		   pthread_cond_signal(&pChan->video_cond); 
 	    		   pthread_mutex_unlock(&pChan->video_lock);
 	   	  	  	 break;
 	   	  }
   	      if(ReadSize>0)
 	   	   {
 	   	      readstart = buf+start;
 	          if(ReadSize>VIDEO_BUF_SIZE-start)
		      {
 	   	      	left = VIDEO_BUF_SIZE-start;
		      }
		      else
		      {
				left = ReadSize;
		      }       	    	      
 	   	      read = PPCS_Read(pChan->nIOTCSessionID,DATA_TYPE_VIDEO,readstart,&left,nTimeout);
 	   	     //avlogcbmsg("thread_ReceiveVideo::PPCS_Read");  
 	   	  	  if(read<0)
 	   	  	  {
 	   	  	  			  if(read== ERROR_PPCS_TIME_OUT)
 	   	  	  			  {	   	  	  				
 	   	  	  	 	  	  continue;
 	   	  	  			  }
						  if(pChan->avStartRecvVideo==1)
						  {
 	   	  	  			  	pa = Packet_Alloc(NULL,NULL,0,read);
 	   	  	 					 if(pa!= NULL)
 	   	  	  				{
 	   	  	  	  			 Add_Video_Packet(pChan,pa);	
 	   	  	  				} 
						  }
 	   	  	  			  pthread_mutex_lock(&pChan->video_lock);
 	    			   	  pthread_cond_signal(&pChan->video_cond); 
 	    			   	  pthread_mutex_unlock(&pChan->video_lock);
 	   	  	  				break;
 	   	  	  }
 	   	  	  else
 	   	  	  {
 	   	  	  	
 	   	  	  	  
 	   	  	  	  count = 0;
 	   	  	  	  left+= start;
 	   	  	  	  packge = (st_PACKAGE_INFO *)buf;	
 	   	  	  	  tmp = buf;  
                              
 	   	  	  	// Log3("PPCS_Read = %d   %d   %d   %d\n", left,read,start,packge->package_size+sizeof(st_PACKAGE_INFO)); 	  	  	  
 	   	  	  	  while(left>=sizeof(st_PACKAGE_INFO)+packge->package_size)
 	   	  	  	  {
 	   	  	  	  	
 	   	  	  	  	if(packge->package_head != PACKAGE_HEAD)
 	   	  	  	  	{
 	   	  	  	  			Log3("packge->package_head is error = %d  \n", packge->package_size);
							while(left>0)
							{
   								packge = (st_PACKAGE_INFO *)tmp;
								if(packge->package_head == PACKAGE_HEAD)
								{
									memcpy(buf,tmp,left);
									tmp = buf;
									break;
								}
								tmp ++;
								left--;
							}
							packge = (st_PACKAGE_INFO *)buf;
							if(left>=sizeof(st_PACKAGE_INFO)+packge->package_size)
							{
							tmp = buf;
							}
							else
							{
							start = left;
							buf[start] = 0;
							break;
							}
							
 	   	  	  	  	}
 	   	  	  	  	
 	   	  	  	  	data = tmp+sizeof(st_PACKAGE_INFO);
					if(pChan->avStartRecvVideo==1)
					{
	 	   	  	  	  	pa = Packet_Alloc(packge,data,packge->package_size,read);
	 	   	  	 		if(pa!= NULL)
	 	   	  	  		{
	 	   	  	  	  		Add_Video_Packet(pChan,pa);	
	 	   	  	  		} 
					}
 	   	  	  	  	left-= sizeof(st_PACKAGE_INFO)+packge->package_size;
 	   	  	  	  	tmp+= sizeof(st_PACKAGE_INFO)+packge->package_size;
 	   	  	  	  	count+=sizeof(st_PACKAGE_INFO)+packge->package_size;
 	   	  	  	  	packge = (st_PACKAGE_INFO *)tmp;
 	   	  	  	  	  
 	   	  	  	  }
 	   	  	  	  if(left>0)
 	   	  	  	  {
 	   	  	  	  	  memcpy(buf,tmp,left);
 	   	  	  	  	  start = left;
 	   	  	  	  }
 	   	  	  	  else
 	   	  	  	  {
 	   	  	  	  	start = 0;
 	   	  	  	  }
 	   	  	  	  
 	   	  	  }
 	    			pthread_mutex_lock(&pChan->video_lock);
 	    			pthread_cond_signal(&pChan->video_cond); 
 	    			pthread_mutex_unlock(&pChan->video_lock); 	 
 	   	  	}
 	   	  	else
 	   	  	{
 	   	  		
 	   	  	     	if(Check_Video_Packet(pChan) == 1)
            	    {
 	    						pthread_mutex_lock(&pChan->video_lock);
 	    						pthread_cond_signal(&pChan->video_cond); 
 	    						pthread_mutex_unlock(&pChan->video_lock); 	
 	   	  	 				    usleep(200);	
            	   	}
            	    else
            	    {
            	   		 usleep(500);		
            	    }
            	    
 	   	  		 
 	   	  	} 	
 	   }
 	   if(szabBuf)
 	   	{
 	   		free(szabBuf);
 	   		szabBuf = NULL;	
 	   	}
 	
 	    Log3("thread_ReceiveVideo: pthread_exit\n");
 	    return NULL;
 	    
 }
 #define IO_BUF_SIZE  4*1024
 static void *thread_ReceiveIoCtrl(void *arg)
 {
 	   // pthread_detach(pthread_self());
 	    st_PPCS_Channel *pChan = (st_PPCS_Channel*)arg;

 	    char szabBuf[IO_BUF_SIZE];
 	    int nTimeout = 3600;
 	    UINT32 WriteSize;
 	    UINT32 ReadSize;
 	    st_PACKAGE_INFO *packge = NULL;
       	st_PACKAGE *pa = NULL;
 	    int read;
 	    int ret;
 	    int start = 0;
 	    int left = IO_BUF_SIZE;
 	    char *buf = szabBuf;
 	    char *readstart;
 	    char *tmp;
 	    int count;
 	    char *data;
 	    Log3("thread_ReceiveIoCtrl: SessionID = %d\n", pChan->nIOTCSessionID);
 	    while(pChan->nRun == 1&& pChan->nIOTCSessionID!= -1)
 	   {  
 	   		   	
 	   	 	  ret = PPCS_Check_Buffer(pChan->nIOTCSessionID,DATA_TYPE_IOCTRL,&WriteSize,&ReadSize);
	 	   	  if(ret<0)
	 	   	  {
	 	   	  	           Log3("thread_ReceiveIoCtrl: PPCS_Check_Buffer error  = %d\n", ret); 
	 	   	  		  	   pa = Packet_Alloc(NULL,NULL,0,ret);
 	   	  	 				if(pa!= NULL)
 	   	  	  				{
 	   	  	  	  			  Add_Ioctrl_Packet(pChan,pa);	
 	   	  	  				} 
 	   	  	  				pthread_mutex_lock(&pChan->ioctrl_lock);
 	   	  	 				pthread_cond_signal(&pChan->ioctrl_cond);
 	   	  	 				pthread_mutex_unlock(&pChan->ioctrl_lock); 
 	   	  	  				break;
	 	   	  }	
 	   	    if(ReadSize>0)
 	   	    	{

				 readstart = buf+start;				
				if(ReadSize>IO_BUF_SIZE-start)			
				{			
				  left = IO_BUF_SIZE-start;			
				}				
				else				
				{				
					left = ReadSize;				
				 }  
				
   	        	    	      
 	   	    	read = PPCS_Read(pChan->nIOTCSessionID,DATA_TYPE_IOCTRL,readstart,&left,nTimeout);
 	   	    	
 	   	  	  if(read<0)
 	   	  	  {
 	   	  	  			  if(read== ERROR_PPCS_TIME_OUT)
 	   	  	  			  {	   	  	  				
 	   	  	  	 	  	  continue;
 	   	  	  			  }
 	   	  	  			  pa = Packet_Alloc(NULL,NULL,0,read);
 	   	  	 					 if(pa!= NULL)
 	   	  	  				{
 	   	  	  	  			 Add_Ioctrl_Packet(pChan,pa);	
 	   	  	  				} 
 	   	  	  					pthread_mutex_lock(&pChan->ioctrl_lock);
 	   	  	 					pthread_cond_signal(&pChan->ioctrl_cond);
 	   	  	 					pthread_mutex_unlock(&pChan->ioctrl_lock); 
 	   	  	  				break;
 	   	  	  }
 	   	  	  else
 	   	  	  {
 	   	  	  	
 	   	  	  	  
 	   	  	  	  count = 0;
 	   	  	  	  left+= start;
 	   	  	  	  packge = (st_PACKAGE_INFO *)buf;	
 	   	  	  	  tmp = buf;  
 	   	  	  	//  Log3("PPCS_Read = %d   %d   %d   %d\n", left,read,start,packge->package_size+sizeof(st_PACKAGE_INFO)); 	  	  	  
 	   	  	  	  while(left>=sizeof(st_PACKAGE_INFO)+packge->package_size)
 	   	  	  	  {
 	   	  	  	  	
 	   	  	  	  	if(packge->package_head != PACKAGE_HEAD)
 	   	  	  	  	{
 	   	  	  	  			Log3("packge->package_head is ture = %d  \n", packge->package_size);
							while(left>0)
							{
								packge = (st_PACKAGE_INFO *)tmp;
								if(packge->package_head == PACKAGE_HEAD)
								{
									memcpy(buf,tmp,left);
									tmp = buf;
									break;
								}
								tmp ++;
								left--;

							}
							packge = (st_PACKAGE_INFO *)buf;
							if(left>=sizeof(st_PACKAGE_INFO)+packge->package_size)
							{
								tmp = buf;
							}
							else
							{
							start = left;
							buf[start] = 0;
							break;
							}
							
 	   	  	  	  	}
 	   	  	  	  	
 	   	  	  	  	data = tmp+sizeof(st_PACKAGE_INFO);
 	   	  	  	  //	Log3("Packet_Alloc = %d    0x%04X\n", packge->package_size,*(unsigned int *)data);
 	   	  	  	  	pa = Packet_Alloc(packge,data,packge->package_size,read);
 	   	  	 				if(pa!= NULL)
 	   	  	  			{
 	   	  	  	  			 Add_Ioctrl_Packet(pChan,pa);	
 	   	  	  			} 
 	   	  	  	  	left-= sizeof(st_PACKAGE_INFO)+packge->package_size;
 	   	  	  	  	tmp+= sizeof(st_PACKAGE_INFO)+packge->package_size;
 	   	  	  	  	count+=sizeof(st_PACKAGE_INFO)+packge->package_size;
 	   	  	  	  	packge = (st_PACKAGE_INFO *)tmp;
 	   	  	  	  	  
 	   	  	  	  }
 	   	  	  	  if(left>0)
 	   	  	  	  {
 	   	  	  	  	  memcpy(buf,tmp,left);
 	   	  	  	  	  start = left;
 	   	  	  	  }
 	   	  	  	  else
 	   	  	  	  {
 	   	  	  	  	start = 0;
 	   	  	  	  }
 	   	  	  	  
 	   	  	  }
 	   	  	 pthread_mutex_lock(&pChan->ioctrl_lock);
 	   	  	 pthread_cond_signal(&pChan->ioctrl_cond);
 	   	  	 pthread_mutex_unlock(&pChan->ioctrl_lock); 
 	   	  	}
 	   	  	else
 	   	  	{
 	   	  		
 	   	  	     	if(Check_Ioctrl_Packet(pChan) == 1)
            	   		 {
 	   	  	 				pthread_mutex_lock(&pChan->ioctrl_lock);
 	   	  	 				pthread_cond_signal(&pChan->ioctrl_cond);
 	   	  	 				pthread_mutex_unlock(&pChan->ioctrl_lock); 
 	   	  	 				usleep(200);	
            	   		 }
            	   		 else
            	   		 {
            	   		 usleep(2000);		
            	   		 }
            	    
 	   	  		 
 	   	  	}
 	  
 	   }
 	   
 	    Log3("thread_ReceiveIoCtrl: pthread_exit\n");
 	    return NULL;
 }
 int avGetAVApiVer(void)
 {
 	 return PPCS_GetAPIVersion();
 }
 int avServResetBuffer(int avIndex, AV_RESET_TARGET eTarget, unsigned int Timeout_ms)
 {
 	 return 0;
 }
 int avInitialize(int nMaxChannelNum)
 {
 	    Log3("avInitialize\n");
 	   INT32 i =0;
 	   for( i =0;i<MAX_CHANNEL;i++)
 	   {
 	   	 g_ppcs_channel[i].nIOTCSessionID = -1;
 	   	 g_ppcs_channel[i].pfxAuthFn = NULL;
 	   	 g_ppcs_channel[i].sendAudioData = NULL;
			 g_ppcs_channel[i].sendAudioSize = 0;
			 g_ppcs_channel[i].sendVideoData = NULL;
			 g_ppcs_channel[i].sendVideoSize = 0;	
			 g_ppcs_channel[i].sendIOCtrlData = NULL;
	     g_ppcs_channel[i].sendIOCtrlSize = 0;
	     g_ppcs_channel[i].avStartRecvAudio = 0;
	     g_ppcs_channel[i].avStartRecvVideo = 0;
	     g_ppcs_channel[i].Thread_Video_Rec_ID = -1;
	     g_ppcs_channel[i].Thread_Audio_Rec_ID = -1;
	     g_ppcs_channel[i].Thread_IoCtrl_Rec_ID = -1;
	     
	     g_ppcs_channel[i].nWaitforKeyFrame = 0;
	     g_ppcs_channel[i].nBufTooBig = 0;
	
	     g_ppcs_channel[i].iocttl_head = NULL;
	     g_ppcs_channel[i].video_head = NULL;
	     g_ppcs_channel[i].audio_head = NULL;
	     g_ppcs_channel[i].nRun = 0;	
	     pthread_mutex_init(&g_ppcs_channel[i].mutex, NULL);
 	   }
 	   
 	 	 return 0;
 }
 int avDeInitialize(void)
 {  
 	    INT32 i =0;
 	 	  for( i =0;i<MAX_CHANNEL;i++)
 	   {
 	   	 if( g_ppcs_channel[i].nIOTCSessionID!= -1)
 	   	 	{
 	   	 		PPCS_Close(g_ppcs_channel[i].nIOTCSessionID);
 	   	 	}
 	   	 g_ppcs_channel[i].nIOTCSessionID = -1;
 	   	 g_ppcs_channel[i].pfxAuthFn = NULL;
 	   	 if(g_ppcs_channel[i].sendAudioData != NULL)
 	   	 {
 	   	 	   free(g_ppcs_channel[i].sendAudioData);
 	   	 	   g_ppcs_channel[i].sendAudioData = NULL;
 	   	 }
 	   	 if(g_ppcs_channel[i].sendVideoData != NULL)
 	   	 {
 	   	 	   free(g_ppcs_channel[i].sendVideoData);
 	   	 	   g_ppcs_channel[i].sendVideoData = NULL;
 	   	 }
 	   	 if(g_ppcs_channel[i].sendIOCtrlData != NULL)
 	   	 {
 	   	 	   free(g_ppcs_channel[i].sendIOCtrlData);
 	   	 	   g_ppcs_channel[i].sendIOCtrlData = NULL;
 	   	 }
 	   	 
	     g_ppcs_channel[i].nWaitforKeyFrame = 0;
	     g_ppcs_channel[i].nBufTooBig = 0;
 	   	 
 	   	 	g_ppcs_channel[i].sendVideoSize = 0;	
 	   	  g_ppcs_channel[i].sendAudioSize = 0;
 	   	  g_ppcs_channel[i].sendIOCtrlSize = 0;

 	   	  g_ppcs_channel[i].nRun = 0;	
		  g_ppcs_channel[i].avStartRecvAudio = 0;
		  g_ppcs_channel[i].avStartRecvVideo = 0;
 	   	  pthread_mutex_destroy(&g_ppcs_channel[i].mutex);
 	   }
 	 	 return 0;
 }
 int  avServStart(int nIOTCSessionID, const char *cszViewAccount, const char *cszViewPassword, unsigned int nTimeout, unsigned int nServType, unsigned char nIOTCChannelID)
 {
 	   int ret;
 	   int nAVChannelID = avGetServFreeAVChannel();
 	   if(nAVChannelID!= -1)
 	   	{
 	   	 g_ppcs_channel[nAVChannelID].nIOTCSessionID = nIOTCSessionID;
 	   	 if(cszViewAccount!=NULL)
 	   	 	{
 	   	 		sprintf(g_ppcs_channel[nAVChannelID].cszViewAccount,"%s",cszViewAccount);
 	   	 	}
 	   	 if(cszViewPassword!=NULL)
 	   	 	{
 	   	 sprintf(g_ppcs_channel[nAVChannelID].cszViewPassword,"%s",cszViewPassword);
 	     }
 	   	 g_ppcs_channel[nAVChannelID].nTimeout = nTimeout;
 	   	 g_ppcs_channel[nAVChannelID].nServType = nServType;
 	   	 g_ppcs_channel[nAVChannelID].nIOTCChannelID = nIOTCChannelID; 
	     g_ppcs_channel[nAVChannelID].nRun = 1;
		   g_ppcs_channel[nAVChannelID].sendVideoSize = 0;	
 	   	 g_ppcs_channel[nAVChannelID].sendAudioSize = 0;
 	   	 g_ppcs_channel[nAVChannelID].sendIOCtrlSize = 0;
 	   	 g_ppcs_channel[nAVChannelID].nWaitforKeyFrame = 0;
	     g_ppcs_channel[nAVChannelID].nBufTooBig = 0; 
 	   	 if(g_ppcs_channel[nAVChannelID].sendAudioData != NULL)
 	   	 {
 	   	 	   free(g_ppcs_channel[nAVChannelID].sendAudioData);
 	   	 	   g_ppcs_channel[nAVChannelID].sendAudioData = NULL;
 	   	 }
 	   	 if(g_ppcs_channel[nAVChannelID].sendVideoData != NULL)
 	   	 {
 	   	 	   free(g_ppcs_channel[nAVChannelID].sendVideoData);
 	   	 	   g_ppcs_channel[nAVChannelID].sendVideoData = NULL;
 	   	 }
 	   	 if(g_ppcs_channel[nAVChannelID].sendIOCtrlData != NULL)
 	   	 {
 	   	 	   free(g_ppcs_channel[nAVChannelID].sendIOCtrlData);
 	   	 	   g_ppcs_channel[nAVChannelID].sendIOCtrlData = NULL;
 	   	 }

 	   	 
 	   	 
	      ret = pthread_cond_init(&g_ppcs_channel[nAVChannelID].video_cond, NULL);
   		  if (ret != 0) {
       	 Log3("pthread_cond_init error: %s\n", strerror(ret));
       	 return -1;
  		  }
  		  ret = pthread_mutex_init(&g_ppcs_channel[nAVChannelID].video_lock, NULL);
   		  if (ret != 0) {
       	 Log3("pthread_mutex_init error: %s\n", strerror(ret));
       	 return -1;
  		  }
        ret = pthread_cond_init(&g_ppcs_channel[nAVChannelID].audio_cond, NULL);
   		  if (ret != 0) {
       	 Log3("pthread_cond_init error: %s\n", strerror(ret));
       	 return -1;
  		  }
  		  ret = pthread_mutex_init(&g_ppcs_channel[nAVChannelID].audio_lock, NULL);
   		  if (ret != 0) {
       	 Log3("pthread_mutex_init error: %s\n", strerror(ret));
       	 return -1;
  		  } 
  		  ret = pthread_cond_init(&g_ppcs_channel[nAVChannelID].ioctrl_cond, NULL);
   		  if (ret != 0) {
       	 Log3("pthread_cond_init error: %s\n", strerror(ret));
       	 return -1;
  		  }

  		  
  		  ret = pthread_mutex_init(&g_ppcs_channel[nAVChannelID].ioctrl_lock, NULL);
   		  if (ret != 0) {
       	 Log3("pthread_mutex_init error: %s\n", strerror(ret));
       	 return -1;
  		  } 
  		  ret = pthread_mutex_init(&g_ppcs_channel[nAVChannelID].ioctrl_listmutex, NULL);
   		  if (ret != 0) {
       	 Log3("pthread_mutex_init error: %s\n", strerror(ret));
       	 return -1;
  		  } 
  		  
  		   ret = pthread_mutex_init(&g_ppcs_channel[nAVChannelID].video_listmutex, NULL);
   		  if (ret != 0) {
       	 Log3("pthread_mutex_init error: %s\n", strerror(ret));
       	 return -1;
  		  }
  		  
  		   ret = pthread_mutex_init(&g_ppcs_channel[nAVChannelID].audio_listmutex, NULL);
   		  if (ret != 0) {
       	 Log3("pthread_mutex_init error: %s\n", strerror(ret));
       	 return -1;
  		  }
  		  		  
  		  

	
		   
 	   	 if(pthread_create(&g_ppcs_channel[nAVChannelID].Thread_IoCtrl_Rec_ID, NULL, &thread_ReceiveIoCtrl, (void *)&g_ppcs_channel[nAVChannelID]))
 	   	 	{
 	   	 		
 	   	 	}
 	   	 	if(pthread_create(&g_ppcs_channel[nAVChannelID].Thread_Audio_Rec_ID, NULL, &thread_ReceiveAudio, (void *)&g_ppcs_channel[nAVChannelID]))
 	   	 	{
 	   	 		
 	   	 	}
 	   	 	if(pthread_create(&g_ppcs_channel[nAVChannelID].Thread_Video_Rec_ID, NULL, &thread_ReceiveVideo, (void *)&g_ppcs_channel[nAVChannelID]))
 	   	 	{
 	   	 		
 	   	 	}
 
 	   	}
 	 	 return nAVChannelID;
 }
 int  avServStart2(int nIOTCSessionID, authFn pfxAuthFn, unsigned int nTimeout, unsigned int nServType, unsigned char nIOTCChannelID)
 {
 	     int ret;
 	   int nAVChannelID = avGetServFreeAVChannel();
 	   if(nAVChannelID!= -1)
 	   	{
 	   	 g_ppcs_channel[nAVChannelID].nIOTCSessionID = nIOTCSessionID;
 	   	 g_ppcs_channel[nAVChannelID].pfxAuthFn = pfxAuthFn;
 	   	 g_ppcs_channel[nAVChannelID].nTimeout = nTimeout;
 	   	 g_ppcs_channel[nAVChannelID].nServType = nServType;
 	   	 g_ppcs_channel[nAVChannelID].nIOTCChannelID = nIOTCChannelID;
 	   	 g_ppcs_channel[nAVChannelID].nRun = 1; 
		 g_ppcs_channel[nAVChannelID].sendVideoSize = 0;	
 	   	 g_ppcs_channel[nAVChannelID].sendAudioSize = 0;
 	   	 g_ppcs_channel[nAVChannelID].sendIOCtrlSize = 0;
 	   	 g_ppcs_channel[nAVChannelID].nWaitforKeyFrame = 0;
	         g_ppcs_channel[nAVChannelID].nBufTooBig = 0;  	   	 
 	   	 if(g_ppcs_channel[nAVChannelID].sendAudioData != NULL)
 	   	 {
 	   	 	   free(g_ppcs_channel[nAVChannelID].sendAudioData);
 	   	 	   g_ppcs_channel[nAVChannelID].sendAudioData = NULL;
 	   	 }
 	   	 if(g_ppcs_channel[nAVChannelID].sendVideoData != NULL)
 	   	 {
 	   	 	   free(g_ppcs_channel[nAVChannelID].sendVideoData);
 	   	 	   g_ppcs_channel[nAVChannelID].sendVideoData = NULL;
 	   	 }
 	   	 if(g_ppcs_channel[nAVChannelID].sendIOCtrlData != NULL)
 	   	 {
 	   	 	   free(g_ppcs_channel[nAVChannelID].sendIOCtrlData);
 	   	 	   g_ppcs_channel[nAVChannelID].sendIOCtrlData = NULL;
 	   	 }

 	   	 
 	   	 
 	   	  ret = pthread_cond_init(&g_ppcs_channel[nAVChannelID].video_cond, NULL);
   		  if (ret != 0) {
       	 Log3("pthread_cond_init error: %s\n", strerror(ret));
       	 return -1;
  		  }
  		  ret = pthread_mutex_init(&g_ppcs_channel[nAVChannelID].video_lock, NULL);
   		  if (ret != 0) {
       	 Log3("pthread_mutex_init error: %s\n", strerror(ret));
       	 return -1;
  		  }
        ret = pthread_cond_init(&g_ppcs_channel[nAVChannelID].audio_cond, NULL);
   		  if (ret != 0) {
       	 Log3("pthread_cond_init error: %s\n", strerror(ret));
       	 return -1;
  		  }
  		  ret = pthread_mutex_init(&g_ppcs_channel[nAVChannelID].audio_lock, NULL);
   		  if (ret != 0) {
       	 Log3("pthread_mutex_init error: %s\n", strerror(ret));
       	 return -1;
  		  } 
  		  ret = pthread_cond_init(&g_ppcs_channel[nAVChannelID].ioctrl_cond, NULL);
   		  if (ret != 0) {
       	 Log3("pthread_cond_init error: %s\n", strerror(ret));
       	 return -1;
  		  }

  		  ret = pthread_mutex_init(&g_ppcs_channel[nAVChannelID].ioctrl_lock, NULL);
   		  if (ret != 0) {
       	 Log3("pthread_mutex_init error: %s\n", strerror(ret));
       	 return -1;
  		  } 
  		  ret = pthread_mutex_init(&g_ppcs_channel[nAVChannelID].ioctrl_listmutex, NULL);
   		  if (ret != 0) {
       	 Log3("pthread_mutex_init error: %s\n", strerror(ret));
       	 return -1;
  		  } 
  		  
  		  ret = pthread_mutex_init(&g_ppcs_channel[nAVChannelID].video_listmutex, NULL);
   		  if (ret != 0) {
       	 Log3("pthread_mutex_init error: %s\n", strerror(ret));
       	 return -1;
  		  }
  		  
  		   ret = pthread_mutex_init(&g_ppcs_channel[nAVChannelID].audio_listmutex, NULL);
   		  if (ret != 0) {
       	 Log3("pthread_mutex_init error: %s\n", strerror(ret));
       	 return -1;
  		  }    		  
  		  		
 	   	 if(pthread_create(&g_ppcs_channel[nAVChannelID].Thread_IoCtrl_Rec_ID, NULL, &thread_ReceiveIoCtrl, (void *)&g_ppcs_channel[nAVChannelID]))
 	   	 	{
 	   	 		
 	   	 	}
 	   	 	if(pthread_create(&g_ppcs_channel[nAVChannelID].Thread_Audio_Rec_ID, NULL, &thread_ReceiveAudio, (void *)&g_ppcs_channel[nAVChannelID]))
 	   	 	{
 	   	 		
 	   	 	}
 	   	 	if(pthread_create(&g_ppcs_channel[nAVChannelID].Thread_Video_Rec_ID, NULL, &thread_ReceiveVideo, (void *)&g_ppcs_channel[nAVChannelID]))
 	   	 	{
 	   	 		
 	   	 	}
 	   	}
 	 	 return nAVChannelID;
 }
 int  avServStart3(int nIOTCSessionID, authFn pfxAuthFn, unsigned int nTimeout, unsigned int nServType, unsigned char nIOTCChannelID, int *pnResend)
 {
 	   int ret;
 	   int nAVChannelID = avGetServFreeAVChannel();
 	   if(nAVChannelID!= -1)
 	   	{
 	   	 g_ppcs_channel[nAVChannelID].nIOTCSessionID = nIOTCSessionID;
 	   	 g_ppcs_channel[nAVChannelID].pfxAuthFn = pfxAuthFn;
 	   	 g_ppcs_channel[nAVChannelID].nTimeout = nTimeout;
 	   	 g_ppcs_channel[nAVChannelID].nServType = nServType;
 	   	 g_ppcs_channel[nAVChannelID].nIOTCChannelID = nIOTCChannelID; 
 	   	 g_ppcs_channel[nAVChannelID].nRun = 1;	
		   g_ppcs_channel[nAVChannelID].sendVideoSize = 0;	
 	   	 g_ppcs_channel[nAVChannelID].sendAudioSize = 0;
 	   	 g_ppcs_channel[nAVChannelID].sendIOCtrlSize = 0;
 	   	 g_ppcs_channel[nAVChannelID].nWaitforKeyFrame = 0;
	     g_ppcs_channel[nAVChannelID].nBufTooBig = 0;  	   	 
 	   	 if(g_ppcs_channel[nAVChannelID].sendAudioData != NULL)
 	   	 {
 	   	 	   free(g_ppcs_channel[nAVChannelID].sendAudioData);
 	   	 	   g_ppcs_channel[nAVChannelID].sendAudioData = NULL;
 	   	 }
 	   	 if(g_ppcs_channel[nAVChannelID].sendVideoData != NULL)
 	   	 {
 	   	 	   free(g_ppcs_channel[nAVChannelID].sendVideoData);
 	   	 	   g_ppcs_channel[nAVChannelID].sendVideoData = NULL;
 	   	 }
 	   	 if(g_ppcs_channel[nAVChannelID].sendIOCtrlData != NULL)
 	   	 {
 	   	 	   free(g_ppcs_channel[nAVChannelID].sendIOCtrlData);
 	   	 	   g_ppcs_channel[nAVChannelID].sendIOCtrlData = NULL;
 	   	 }

 	   	 
 	   	 
 	   	 ret = pthread_cond_init(&g_ppcs_channel[nAVChannelID].video_cond, NULL);
   		  if (ret != 0) {
       	 Log3("pthread_cond_init error: %s\n", strerror(ret));
       	 return -1;
  		  }
  		  ret = pthread_mutex_init(&g_ppcs_channel[nAVChannelID].video_lock, NULL);
   		  if (ret != 0) {
       	 Log3("pthread_mutex_init error: %s\n", strerror(ret));
       	 return -1;
  		  }
        ret = pthread_cond_init(&g_ppcs_channel[nAVChannelID].audio_cond, NULL);
   		  if (ret != 0) {
       	 Log3("pthread_cond_init error: %s\n", strerror(ret));
       	 return -1;
  		  }
  		  ret = pthread_mutex_init(&g_ppcs_channel[nAVChannelID].audio_lock, NULL);
   		  if (ret != 0) {
       	 Log3("pthread_mutex_init error: %s\n", strerror(ret));
       	 return -1;
  		  } 
  		  ret = pthread_cond_init(&g_ppcs_channel[nAVChannelID].ioctrl_cond, NULL);
   		  if (ret != 0) {
       	 Log3("pthread_cond_init error: %s\n", strerror(ret));
       	 return -1;
  		  }

  		  ret = pthread_mutex_init(&g_ppcs_channel[nAVChannelID].ioctrl_lock, NULL);
   		  if (ret != 0) {
       	 Log3("pthread_mutex_init error: %s\n", strerror(ret));
       	 return -1;
  		  } 
  		  ret = pthread_mutex_init(&g_ppcs_channel[nAVChannelID].ioctrl_listmutex, NULL);
   		  if (ret != 0) {
       	 Log3("pthread_mutex_init error: %s\n", strerror(ret));
       	 return -1;
  		  }
  		  ret = pthread_mutex_init(&g_ppcs_channel[nAVChannelID].video_listmutex, NULL);
   		  if (ret != 0) {
       	 Log3("pthread_mutex_init error: %s\n", strerror(ret));
       	 return -1;
  		  }
  		  
  		   ret = pthread_mutex_init(&g_ppcs_channel[nAVChannelID].audio_listmutex, NULL);
   		  if (ret != 0) {
       	 Log3("pthread_mutex_init error: %s\n", strerror(ret));
       	 return -1;
  		  } 
 	   	 if(pthread_create(&g_ppcs_channel[nAVChannelID].Thread_IoCtrl_Rec_ID, NULL, &thread_ReceiveIoCtrl, (void *)&g_ppcs_channel[nAVChannelID]))
 	   	 	{
 	   	 		
 	   	 	}
 	   	 	if(pthread_create(&g_ppcs_channel[nAVChannelID].Thread_Audio_Rec_ID, NULL, &thread_ReceiveAudio, (void *)&g_ppcs_channel[nAVChannelID]))
 	   	 	{
 	   	 		
 	   	 	}
 	   	 	if(pthread_create(&g_ppcs_channel[nAVChannelID].Thread_Video_Rec_ID, NULL, &thread_ReceiveVideo, (void *)&g_ppcs_channel[nAVChannelID]))
 	   	 	{
 	   	 		
 	   	 	}	
 	   	}
 	 	 return nAVChannelID;
 }
 void avServExit(int nIOTCSessionID, unsigned char nIOTCChannelID)
 {
 	     Log3("avServExit: nIOTCSessionID = %d nIOTCChannelID = %d\n",nIOTCSessionID, nIOTCChannelID);
 	    int ret;
 	    int nAVChannelID = avGetServAVChannel(nIOTCSessionID,nIOTCChannelID);
 	    if(nAVChannelID!= -1)
 	    {
 	    	Log3("avServExit:nAVChannelID = %d\n",nAVChannelID);
			pthread_mutex_lock(&g_ppcs_channel[nAVChannelID].mutex);
 	    	g_ppcs_channel[nAVChannelID].nRun = 0;
 	    	if(g_ppcs_channel[nAVChannelID].nIOTCSessionID!= -1)
 	    	{ 	
 	    	Log3("avServExit:PPCS_Close = %d\n",nAVChannelID);
 	    	PPCS_Close(g_ppcs_channel[nAVChannelID].nIOTCSessionID);
 	    	g_ppcs_channel[nAVChannelID].nIOTCSessionID = -1;
 	    	
			g_ppcs_channel[nAVChannelID].avStartRecvAudio = 0;
			g_ppcs_channel[nAVChannelID].avStartRecvVideo = 0;

	      	Log3("avServExit:pthread_join = %d\n",nAVChannelID);	
 		   
 	 	    if(g_ppcs_channel[nAVChannelID].Thread_IoCtrl_Rec_ID!= -1) pthread_join(g_ppcs_channel[nAVChannelID].Thread_IoCtrl_Rec_ID, NULL);
 	   		if(g_ppcs_channel[nAVChannelID].Thread_Audio_Rec_ID!= -1) pthread_join(g_ppcs_channel[nAVChannelID].Thread_Audio_Rec_ID, NULL);
 	   		if(g_ppcs_channel[nAVChannelID].Thread_Video_Rec_ID!= -1) pthread_join(g_ppcs_channel[nAVChannelID].Thread_Video_Rec_ID, NULL); 
 	   		g_ppcs_channel[nAVChannelID].Thread_IoCtrl_Rec_ID = -1;
 	   		g_ppcs_channel[nAVChannelID].Thread_Audio_Rec_ID = -1;
 	   		g_ppcs_channel[nAVChannelID].Thread_Video_Rec_ID = -1;

            Log3("avServExit:pthread_cond_signal = %d\n",nAVChannelID);
    		pthread_mutex_lock(&g_ppcs_channel[nAVChannelID].ioctrl_lock);
 	    	pthread_cond_signal(&g_ppcs_channel[nAVChannelID].ioctrl_cond);
    		pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].ioctrl_lock); 
	        
 	    	pthread_mutex_lock(&g_ppcs_channel[nAVChannelID].video_lock);
 	    	pthread_cond_signal(&g_ppcs_channel[nAVChannelID].video_cond);
    		pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].video_lock);
    
  	    	pthread_mutex_lock(&g_ppcs_channel[nAVChannelID].audio_lock);
 	    	pthread_cond_signal(&g_ppcs_channel[nAVChannelID].audio_cond);
    		pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].audio_lock);
    		
	
 	   		 Log3("avServExit:ReleaseAll_Ioctrl_Packet = %d\n",nAVChannelID);
 	   	    ReleaseAll_Ioctrl_Packet(&g_ppcs_channel[nAVChannelID]);
        	ReleaseAll_Audio_Packet(&g_ppcs_channel[nAVChannelID]);
        	ReleaseAll_Video_Packet(&g_ppcs_channel[nAVChannelID]);
        	g_ppcs_channel[nAVChannelID].iocttl_head = NULL;
	        g_ppcs_channel[nAVChannelID].video_head = NULL;
	        g_ppcs_channel[nAVChannelID].audio_head = NULL; 
	      

        
           Log3("avServExit:free send buf = %d\n",nAVChannelID);

 	   	  
		  if(g_ppcs_channel[nAVChannelID].sendAudioData != NULL)
 	   	 {
 	   	 	   free(g_ppcs_channel[nAVChannelID].sendAudioData);
 	   	 	   g_ppcs_channel[nAVChannelID].sendAudioData = NULL;
 	   	 }
 	   	 if(g_ppcs_channel[nAVChannelID].sendVideoData != NULL)
 	   	 {
 	   	 	   free(g_ppcs_channel[nAVChannelID].sendVideoData);
 	   	 	   g_ppcs_channel[nAVChannelID].sendVideoData = NULL;
 	   	 }
 	   	 if(g_ppcs_channel[nAVChannelID].sendIOCtrlData != NULL)
 	   	 {
 	   	 	   free(g_ppcs_channel[nAVChannelID].sendIOCtrlData);
 	   	 	   g_ppcs_channel[nAVChannelID].sendIOCtrlData = NULL;
 	   	 }
		 g_ppcs_channel[nAVChannelID].sendVideoSize = 0;	
 	   	 g_ppcs_channel[nAVChannelID].sendAudioSize = 0;
 	   	 g_ppcs_channel[nAVChannelID].sendIOCtrlSize = 0;
 	   	 g_ppcs_channel[nAVChannelID].nWaitforKeyFrame = 0;
	     g_ppcs_channel[nAVChannelID].nBufTooBig = 0; 

		 Log3("avServExit:pthread_cond_destroy = %d\n",nAVChannelID);
  	    ret = pthread_cond_destroy(&g_ppcs_channel[nAVChannelID].video_cond);
        if (ret != 0) {
        Log3("pthread_cond_destroy error: %s\n", strerror(ret));
        }
        ret = pthread_cond_destroy(&g_ppcs_channel[nAVChannelID].audio_cond);
        if (ret != 0) {
        Log3("pthread_cond_destroy error: %s\n", strerror(ret));
        }
        ret = pthread_cond_destroy(&g_ppcs_channel[nAVChannelID].ioctrl_cond);
        if (ret != 0) {
        Log3("pthread_cond_destroy error: %s\n", strerror(ret));
        }

        pthread_mutex_destroy(&g_ppcs_channel[nAVChannelID].video_lock);
        pthread_mutex_destroy(&g_ppcs_channel[nAVChannelID].audio_lock);
        pthread_mutex_destroy(&g_ppcs_channel[nAVChannelID].ioctrl_lock);
        
    
	     
        pthread_mutex_destroy(&g_ppcs_channel[nAVChannelID].ioctrl_listmutex);
        pthread_mutex_destroy(&g_ppcs_channel[nAVChannelID].video_listmutex);
        pthread_mutex_destroy(&g_ppcs_channel[nAVChannelID].audio_listmutex);	
		Log3("avServExit:end = %d\n",nAVChannelID);
		pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].mutex);	 
       }
 	    }
 }
 void avServStop(int nAVChannelID)
 {
 	  Log3("avServStop: nAVChannelID = %d \n",nAVChannelID);
 	   if(nAVChannelID>=0 && nAVChannelID<MAX_CHANNEL/2)
 	   	{
 	     		 
 	   	if(g_ppcs_channel[nAVChannelID].nIOTCSessionID!= -1)
 	    	{ 
 	    	 pthread_mutex_lock(&g_ppcs_channel[nAVChannelID].mutex);
 	    	 Log3("avServStop: nAVChannelID = %d  pthread_cond_signal\n",nAVChannelID);
            g_ppcs_channel[nAVChannelID].avStartRecvAudio = 0;
		    g_ppcs_channel[nAVChannelID].avStartRecvVideo = 0;
 	     	pthread_mutex_lock(&g_ppcs_channel[nAVChannelID].audio_lock);
 	    	pthread_cond_signal(&g_ppcs_channel[nAVChannelID].audio_cond);
    		pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].audio_lock);
    			
 	    	pthread_mutex_lock(&g_ppcs_channel[nAVChannelID].video_lock);
 	    	pthread_cond_signal(&g_ppcs_channel[nAVChannelID].video_cond);
    		pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].video_lock);
        	Log3("avServStop: nAVChannelID = %d  pthread_cond_signal  end \n",nAVChannelID);
			pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].mutex);
  
    			   	 
       }
 	   	}
 }
 void avServStopEx(int nAVChannelID)
 {
     Log3("avServStopEx: nAVChannelID = %d \n",nAVChannelID);
	  if(nAVChannelID>=0 && nAVChannelID<MAX_CHANNEL/2)
	  {
			   
	  	 if(g_ppcs_channel[nAVChannelID].nIOTCSessionID!= -1)
		  { 
		   Log3("avServStopEx: nAVChannelID = %d  pthread_cond_signal\n",nAVChannelID);
		  pthread_mutex_lock(&g_ppcs_channel[nAVChannelID].mutex);
		  g_ppcs_channel[nAVChannelID].nRun = 0;
		  g_ppcs_channel[nAVChannelID].avStartRecvAudio = 0;
		  g_ppcs_channel[nAVChannelID].avStartRecvVideo = 0;

		   pthread_mutex_lock(&g_ppcs_channel[nAVChannelID].ioctrl_lock);
 	       pthread_cond_signal(&g_ppcs_channel[nAVChannelID].ioctrl_cond);
    	   pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].ioctrl_lock);
			/*
		  pthread_mutex_lock(&g_ppcs_channel[nAVChannelID].audio_lock);
		  pthread_cond_signal(&g_ppcs_channel[nAVChannelID].audio_cond);
		  pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].audio_lock);
			  
		  pthread_mutex_lock(&g_ppcs_channel[nAVChannelID].video_lock);
		  pthread_cond_signal(&g_ppcs_channel[nAVChannelID].video_cond);
		  pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].video_lock);
		  */
		  pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].mutex);
		  Log3("avServStopEx: nAVChannelID = %d  pthread_cond_signal  end \n",nAVChannelID);  
	 
				   
	 }
	  }

 }
 void avServSetResendSize(int nAVChannelID, unsigned int nSize)
 {
 	
 }
 int  avSendFrameData(int nAVChannelID, const char *cabFrameData, int nFrameDataSize,
								const void *cabFrameInfo, int nFrameInfoSize)
{
		//Log3("avSendFrameData: nAVChannelID = %d\n", nAVChannelID);
	  int ssize = 0;
	  int send = 0;
	  int ret;
	  int bsend = 1;
	  UINT32 WriteSize = 0;
 	  UINT32 ReadSize = 0;
	  char* temp = NULL;
	  FRAMEINFO_t *frameinfo = (FRAMEINFO_t*)cabFrameInfo;
	  st_PACKAGE_INFO package_info;
	  package_info.package_head = PACKAGE_HEAD;
		if(nAVChannelID>=0 && nAVChannelID<MAX_CHANNEL)
		{
			  pthread_mutex_lock(&g_ppcs_channel[nAVChannelID].mutex);
			 if(g_ppcs_channel[nAVChannelID].nIOTCSessionID!= -1)
			 	{
			 		
			 		ret = PPCS_Check_Buffer(g_ppcs_channel[nAVChannelID].nIOTCSessionID,DATA_TYPE_VIDEO,&WriteSize,&ReadSize);
	 	   	  if(ret<0)
	 	   	  {
	 	   	  	    pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].mutex);
	 	   	  	    return ret;
	 	   	  }
	 	   	  else
	 	   	  {
	 	   	  	    if(WriteSize>800*1024)
	 	   	  	    {
	 	   	  	    	Log3("avSendFrameData : WriteSize = %d\n", WriteSize);
	 	   	  	    	bsend = 0;
	 	   	  	    	g_ppcs_channel[nAVChannelID].nBufTooBig = 1;
	 	   	  	    	g_ppcs_channel[nAVChannelID].nWaitforKeyFrame = 0;
	 	   	  	    }
	 	   	  	    else
	 	   	  	    {
	 	   	  	    	g_ppcs_channel[nAVChannelID].nBufTooBig = 0;	 	   	  	    	
	 	   	  	    	if(g_ppcs_channel[nAVChannelID].nWaitforKeyFrame == 0)
	 	   	  				{
	 	   	  						if(frameinfo->flags == IPC_FRAME_FLAG_IFRAME)
	 	   	  						{
	 	   	  							g_ppcs_channel[nAVChannelID].nWaitforKeyFrame = 1;
	 	   	  						}
	 	   	  				}
	 	   	  				if(g_ppcs_channel[nAVChannelID].nWaitforKeyFrame == 1)
	 	   	  				{
	 	   	  					bsend = 1;
	 	   	  				}
	 	   	  				else
	 	   	  				{
	 	   	  					bsend = 0;
	 	   	  				}
	 	   	  
	 	   	  	    }
	 	   	  }
	 	   	  if(g_ppcs_channel[nAVChannelID].nWaitforKeyFrame == 0)
	 	   	  {
	 	   	  	if(frameinfo->flags == IPC_FRAME_FLAG_IFRAME)
	 	   	  	{
	 	   	  				g_ppcs_channel[nAVChannelID].nWaitforKeyFrame = 1;
	 	   	  	}
	 	   	  	else
	 	   	  	{
	 	   	  		  bsend = 0;
	 	   	  	}
	 	   	  }
	 	   	  if(bsend == 0)
	 	   	  {
				 		  	   pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].mutex);
	 	   	  	       return AV_ER_NoERROR; 	   	  		
	 	   	  }			 		
			 		
			 		  ssize = nFrameDataSize+nFrameInfoSize+sizeof(st_PACKAGE_INFO);
			 		  if(g_ppcs_channel[nAVChannelID].sendVideoData!= NULL)
			 		  	{			 		  	
			 		  			if(ssize>g_ppcs_channel[nAVChannelID].sendVideoSize)
			 		  			{
			 		  	   		if(g_ppcs_channel[nAVChannelID].sendVideoData!= NULL)
			 		  	   			{
			 		  	   				free(g_ppcs_channel[nAVChannelID].sendVideoData);
			 		  	   			}
			 		  	   g_ppcs_channel[nAVChannelID].sendVideoSize = ssize+1;
			 		  	   g_ppcs_channel[nAVChannelID].sendVideoData =(char*)malloc(g_ppcs_channel[nAVChannelID].sendVideoSize*sizeof(char));
			 		 			 }
			 		  }
			 		  else
			 		  {
			 		  	   g_ppcs_channel[nAVChannelID].sendVideoSize = ssize+1;
			 		  	   g_ppcs_channel[nAVChannelID].sendVideoData =(char*)malloc(g_ppcs_channel[nAVChannelID].sendVideoSize*sizeof(char));			 		  	
			 		  }
			 		  if(g_ppcs_channel[nAVChannelID].sendVideoData!= NULL)
			 		  	{
			 		  		 temp = g_ppcs_channel[nAVChannelID].sendVideoData;
			 		  		 package_info.package_head = PACKAGE_HEAD;
			 		  		 package_info.package_size = nFrameDataSize+nFrameInfoSize;
			 		  		 package_info.frameinfo_size = nFrameInfoSize;
								 package_info.frame_size = nFrameDataSize;
			 		  		 memcpy(temp,&package_info,sizeof(st_PACKAGE_INFO));
			 		  		 temp+=sizeof(st_PACKAGE_INFO);
			 		  		 memcpy(temp,cabFrameInfo,nFrameInfoSize);
			 		  		 temp+=nFrameInfoSize;
			 		  		 memcpy(temp,cabFrameData,nFrameDataSize);
			 		  	   send = PPCS_Write(g_ppcs_channel[nAVChannelID].nIOTCSessionID,DATA_TYPE_VIDEO,g_ppcs_channel[nAVChannelID].sendVideoData,ssize);
			 		  	   
			 		  	   	if(send>=0)
			 		  	   	{
			 		  	   			pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].mutex);
			 		  	   		  return AV_ER_NoERROR;
			 		  	   	}
			 		  	   	else
			 		  	   	{
			 		  	   		
			 		  	   		switch(send)
			 		  	   		{
			 		  	   			  case ERROR_PPCS_NOT_INITIALIZED:
			 		  	   			  	 		pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].mutex);
			 		  	   			  	return AV_ER_INVALID_ARG;
			 		  	   			 	break;
			 		  	   			 	case ERROR_PPCS_INVALID_PARAMETER:			 		  	   			 	
			 		  	   			 	break;
			 		  	   			 	case ERROR_PPCS_INVALID_SESSION_HANDLE:
			 		  	   			 		 		pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].mutex);
			 		  	   			 		return AV_ER_INVALID_SID;
			 		  	   			 	break;
			 		  	   			 	case ERROR_PPCS_SESSION_CLOSED_REMOTE:
			 		  	   			 		 		pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].mutex);
			 		  	   			 		return AV_ER_SESSION_CLOSE_BY_REMOTE ;
			 		  	   			 	break;
			 		  	   			 	case ERROR_PPCS_SESSION_CLOSED_TIMEOUT:
			 		  	   			 		 		pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].mutex);
			 		  	   			 		return AV_ER_REMOTE_TIMEOUT_DISCONNECT;
			 		  	   			 	break;
			 		  	   			 	case ERROR_PPCS_REMOTE_SITE_BUFFER_FULL:
			 		  	   			 		 		pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].mutex);
			 		  	   			 		return AV_ER_EXCEED_MAX_SIZE;
			 		  	   			 	break;
			 		  	   		}
			 		  	   		pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].mutex);
			 		  	   		return send;

			 		  	   	}
			 		  	   				 		  	   	
			 		  	}			 		 
			 	}
			pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].mutex);
		}
	    
	 	 return -1;
}
int  avSendAudioData(int nAVChannelID, const char *cabAudioData, int nAudioDataSize,
								const void *cabFrameInfo, int nFrameInfoSize)
{
	 // Log3("avSendAudioData: nAVChannelID = %d\n", nAVChannelID);
	 int ssize = 0;
	 int ret;
	 int bsend = 1;
	 UINT32 WriteSize = 0;
	 UINT32 ReadSize = 0;
	 char* temp = NULL;
	 st_PACKAGE_INFO package_info;
	 int send = 0;	  
		if(nAVChannelID>=0 && nAVChannelID<MAX_CHANNEL)
		{
			pthread_mutex_lock(&g_ppcs_channel[nAVChannelID].mutex);
			 if(g_ppcs_channel[nAVChannelID].nIOTCSessionID!= -1)
			 	{


					  ret = PPCS_Check_Buffer(g_ppcs_channel[nAVChannelID].nIOTCSessionID,DATA_TYPE_AUDIO,&WriteSize,&ReadSize);
			 	   	  if(ret<0)
			 	   	  {
			 	   	  	    pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].mutex);
			 	   	  	    return ret;
			 	   	  }
			 	   	  else
			 	   	  {
			 	   	  	    if(WriteSize>30*1024)
			 	   	  	    {
			 	   	  	    	Log3("avSendAudioData : WriteSize = %d\n", WriteSize);
			 	   	  	    	bsend = 0;

			 	   	  	    }
			 	   	  	    else
			 	   	  	    {

			 	   	  
			 	   	  	    }
			 	   	  }
			 	   	  if(bsend == 0)
			 	   	  {
						 	   pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].mutex);
			 	   	  	       return AV_ER_NoERROR; 	   	  		
			 	   	  }	
			  
			 		  ssize = nAudioDataSize+nFrameInfoSize+sizeof(st_PACKAGE_INFO);
			 		  if(g_ppcs_channel[nAVChannelID].sendAudioData!= NULL)
			 		  	{
			 		  		
					 		  if(ssize>g_ppcs_channel[nAVChannelID].sendAudioSize)
			 				  {
			 		  	   if(g_ppcs_channel[nAVChannelID].sendAudioData!= NULL)
			 		  	   	{
			 		  	   		free(g_ppcs_channel[nAVChannelID].sendAudioData);
			 		  	   	}
			 		  	   g_ppcs_channel[nAVChannelID].sendAudioSize = ssize+1;
			 		  	   g_ppcs_channel[nAVChannelID].sendAudioData =(char*)malloc(g_ppcs_channel[nAVChannelID].sendAudioSize*sizeof(char));
			 		 		 }	 		  	
			 		  	}
			 		  	else
			 		  	{
			 		  	   g_ppcs_channel[nAVChannelID].sendAudioSize = ssize+1;
			 		  	   g_ppcs_channel[nAVChannelID].sendAudioData =(char*)malloc(g_ppcs_channel[nAVChannelID].sendAudioSize*sizeof(char));			 		  		
			 		  	}

			 		  if(g_ppcs_channel[nAVChannelID].sendAudioData!= NULL)
			 		  	{			 		  		 
			 		  		 temp = g_ppcs_channel[nAVChannelID].sendAudioData;
			 		  		 package_info.package_head = PACKAGE_HEAD;
			 		  		 package_info.package_size = nAudioDataSize+nFrameInfoSize;
			 		  		 package_info.frameinfo_size = nFrameInfoSize;
								 package_info.frame_size = nAudioDataSize;
			 		  		 memcpy(temp,&package_info,sizeof(st_PACKAGE_INFO));
			 		  		 temp+=sizeof(st_PACKAGE_INFO);			 		  		
			 		  		 memcpy(temp,cabFrameInfo,nFrameInfoSize);
			 		  		 temp+=nFrameInfoSize;
			 		  		 memcpy(temp,cabAudioData,nAudioDataSize);
			 		  	   send =  PPCS_Write(g_ppcs_channel[nAVChannelID].nIOTCSessionID,DATA_TYPE_AUDIO,g_ppcs_channel[nAVChannelID].sendAudioData,ssize);
			 		  	   
			 		  	   if(send>=0)
			 		  	   	{
			 		  	   		  pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].mutex);
			 		  	   		  return AV_ER_NoERROR;
			 		  	   	}
			 		  	   	else
			 		  	   	{
			 		  	   		
			 		  	   		switch(send)
			 		  	   		{
			 		  	   			  case ERROR_PPCS_NOT_INITIALIZED:
			 		  	   			  	 		 		pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].mutex);
			 		  	   			  	return AV_ER_INVALID_ARG;
			 		  	   			 	break;
			 		  	   			 	case ERROR_PPCS_INVALID_PARAMETER:			 		  	   			 	
			 		  	   			 	break;
			 		  	   			 	case ERROR_PPCS_INVALID_SESSION_HANDLE:
			 		  	   			 		 		pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].mutex);
			 		  	   			 		return AV_ER_INVALID_SID;
			 		  	   			 	break;
			 		  	   			 	case ERROR_PPCS_SESSION_CLOSED_REMOTE:
			 		  	   			 		 		pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].mutex);
			 		  	   			 		return AV_ER_SESSION_CLOSE_BY_REMOTE ;
			 		  	   			 	break;
			 		  	   			 	case ERROR_PPCS_SESSION_CLOSED_TIMEOUT:
			 		  	   			 		 		pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].mutex);
			 		  	   			 		return AV_ER_REMOTE_TIMEOUT_DISCONNECT;
			 		  	   			 	break;
			 		  	   			 	case ERROR_PPCS_REMOTE_SITE_BUFFER_FULL:
			 		  	   			 		 		pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].mutex);
			 		  	   			 		return AV_ER_EXCEED_MAX_SIZE;
			 		  	   			 	break;
			 		  	   		}
			 		  	   		pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].mutex);
			 		  	   		return send;

			 		  	   	}
			 		  	   	
			 		  	   	
			 		  	}
			 		 
			 	}
			pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].mutex);
		}
	    
	 	 return -1;
}
int avServSetDelayInterval(int nAVChannelID, unsigned short nPacketNum, unsigned short nDelayMs)
{
	 	 return 0;
}
int  avClientStart(int nIOTCSessionID, const char *cszViewAccount, const char *cszViewPassword,
								unsigned int nTimeout,unsigned int *pnServType, unsigned char nIOTCChannelID)
{
 	   int ret;
 	   int nAVChannelID = avGetClientFreeAVChannel();
 	   if(nAVChannelID!= -1)
 	   	{
 	   	 g_ppcs_channel[nAVChannelID].nIOTCSessionID = nIOTCSessionID;
 	   	 g_ppcs_channel[nAVChannelID].nTimeout = nTimeout;
 	   	// g_ppcs_channel[nAVChannelID].nServType = nServType;
 	   	 g_ppcs_channel[nAVChannelID].nIOTCChannelID = nIOTCChannelID; 
 	   	 if(cszViewAccount!= NULL)
 	   	 {
 	   	    sprintf(g_ppcs_channel[nAVChannelID].cszViewAccount,"%s",cszViewAccount);
 	   	 }
 	   	 if(cszViewPassword!= NULL)
 	   	 {
 	   	    sprintf(g_ppcs_channel[nAVChannelID].cszViewPassword,"%s",cszViewPassword);
 	   	 }
 	   	 g_ppcs_channel[nAVChannelID].nRun = 1;	
 	   	 
		   g_ppcs_channel[nAVChannelID].sendVideoSize = 0;	
 	   	 g_ppcs_channel[nAVChannelID].sendAudioSize = 0;
 	   	 g_ppcs_channel[nAVChannelID].sendIOCtrlSize = 0;
 	   	 g_ppcs_channel[nAVChannelID].nWaitforKeyFrame = 0;
	     	 g_ppcs_channel[nAVChannelID].nBufTooBig = 0; 
 	   	  if(g_ppcs_channel[nAVChannelID].sendAudioData != NULL)
 	   	 {
 	   	 	   free(g_ppcs_channel[nAVChannelID].sendAudioData);
 	   	 	   g_ppcs_channel[nAVChannelID].sendAudioData = NULL;
 	   	 }
 	   	 if(g_ppcs_channel[nAVChannelID].sendVideoData != NULL)
 	   	 {
 	   	 	   free(g_ppcs_channel[nAVChannelID].sendVideoData);
 	   	 	   g_ppcs_channel[nAVChannelID].sendVideoData = NULL;
 	   	 }
 	   	 if(g_ppcs_channel[nAVChannelID].sendIOCtrlData != NULL)
 	   	 {
 	   	 	   free(g_ppcs_channel[nAVChannelID].sendIOCtrlData);
 	   	 	   g_ppcs_channel[nAVChannelID].sendIOCtrlData = NULL;
 	   	 }

 	   	 
 	   	 
 	   	 ret = pthread_cond_init(&g_ppcs_channel[nAVChannelID].video_cond, NULL);
   		  if (ret != 0) {
       	 Log3("pthread_cond_init error: %s\n", strerror(ret));
       	 return -1;
  		  }
  		  ret = pthread_mutex_init(&g_ppcs_channel[nAVChannelID].video_lock, NULL);
   		  if (ret != 0) {
       	 Log3("pthread_mutex_init error: %s\n", strerror(ret));
       	 return -1;
  		  }
        ret = pthread_cond_init(&g_ppcs_channel[nAVChannelID].audio_cond, NULL);
   		  if (ret != 0) {
       	 Log3("pthread_cond_init error: %s\n", strerror(ret));
       	 return -1;
  		  }
  		  ret = pthread_mutex_init(&g_ppcs_channel[nAVChannelID].audio_lock, NULL);
   		  if (ret != 0) {
       	 Log3("pthread_mutex_init error: %s\n", strerror(ret));
       	 return -1;
  		  } 
  		  ret = pthread_cond_init(&g_ppcs_channel[nAVChannelID].ioctrl_cond, NULL);
   		  if (ret != 0) {
       	 Log3("pthread_cond_init error: %s\n", strerror(ret));
       	 return -1;
  		  }
  		  ret = pthread_mutex_init(&g_ppcs_channel[nAVChannelID].ioctrl_lock, NULL);
   		  if (ret != 0) {
       	 Log3("pthread_mutex_init error: %s\n", strerror(ret));
       	 return -1;
  		  } 
  		  ret = pthread_mutex_init(&g_ppcs_channel[nAVChannelID].ioctrl_listmutex, NULL);
   		  if (ret != 0) {
       	 Log3("pthread_mutex_init error: %s\n", strerror(ret));
       	 return -1;
  		  }  
  		  ret = pthread_mutex_init(&g_ppcs_channel[nAVChannelID].video_listmutex, NULL);
   		  if (ret != 0) {
       	 Log3("pthread_mutex_init error: %s\n", strerror(ret));
       	 return -1;
  		  }
  		  
  		   ret = pthread_mutex_init(&g_ppcs_channel[nAVChannelID].audio_listmutex, NULL);
   		  if (ret != 0) {
       	 Log3("pthread_mutex_init error: %s\n", strerror(ret));
       	 return -1;
  		  }
 	   	 if(pthread_create(&g_ppcs_channel[nAVChannelID].Thread_IoCtrl_Rec_ID, NULL, &thread_ReceiveIoCtrl, (void *)&g_ppcs_channel[nAVChannelID]))
 	   	 	{
 	   	 		
 	   	 	}
 	   	 	if(pthread_create(&g_ppcs_channel[nAVChannelID].Thread_Audio_Rec_ID, NULL, &thread_ReceiveAudio, (void *)&g_ppcs_channel[nAVChannelID]))
 	   	 	{
 	   	 		
 	   	 	}
 	   	 	if(pthread_create(&g_ppcs_channel[nAVChannelID].Thread_Video_Rec_ID, NULL, &thread_ReceiveVideo, (void *)&g_ppcs_channel[nAVChannelID]))
 	   	 	{
 	   	 		
 	   	 	}	
 	   	}
 	 	 return nAVChannelID;						
}
int  avClientStart2(int nIOTCSessionID, const char *cszViewAccount, const char *cszViewPassword,
								unsigned int nTimeout,unsigned int *pnServType, unsigned char nIOTCChannelID, int *pnResend)
{
	
	   
 	   int ret;
 	   int nAVChannelID = avGetClientFreeAVChannel();
 	   Log3("avClientStart2 nAVChannelID: %d\n", nAVChannelID);
 	   if(nAVChannelID!= -1)
 	   	{
 	   	 pthread_mutex_lock(&g_ppcs_channel[nAVChannelID].mutex);
 	   	 g_ppcs_channel[nAVChannelID].nIOTCSessionID = nIOTCSessionID;
 	   	 g_ppcs_channel[nAVChannelID].nTimeout = nTimeout;
 	   	// g_ppcs_channel[nAVChannelID].nServType = nServType;
 	   	 g_ppcs_channel[nAVChannelID].nIOTCChannelID = nIOTCChannelID; 
 	   	 if(cszViewAccount!= NULL)
 	   	 {
 	   	    sprintf(g_ppcs_channel[nAVChannelID].cszViewAccount,"%s",cszViewAccount);
 	   	 }
 	   	 if(cszViewPassword!= NULL)
 	   	 {
 	   	    sprintf(g_ppcs_channel[nAVChannelID].cszViewPassword,"%s",cszViewPassword);
 	   	 }
 	   	 g_ppcs_channel[nAVChannelID].nRun = 1;	
		   g_ppcs_channel[nAVChannelID].sendVideoSize = 0;	
 	   	 g_ppcs_channel[nAVChannelID].sendAudioSize = 0;
 	   	 g_ppcs_channel[nAVChannelID].sendIOCtrlSize = 0;
 	   	 g_ppcs_channel[nAVChannelID].nWaitforKeyFrame = 0;
	     g_ppcs_channel[nAVChannelID].nBufTooBig = 0;  	   	 
 	   	  if(g_ppcs_channel[nAVChannelID].sendAudioData != NULL)
 	   	 {
 	   	 	   free(g_ppcs_channel[nAVChannelID].sendAudioData);
 	   	 	   g_ppcs_channel[nAVChannelID].sendAudioData = NULL;
 	   	 }
 	   	 if(g_ppcs_channel[nAVChannelID].sendVideoData != NULL)
 	   	 {
 	   	 	   free(g_ppcs_channel[nAVChannelID].sendVideoData);
 	   	 	   g_ppcs_channel[nAVChannelID].sendVideoData = NULL;
 	   	 }
 	   	 if(g_ppcs_channel[nAVChannelID].sendIOCtrlData != NULL)
 	   	 {
 	   	 	   free(g_ppcs_channel[nAVChannelID].sendIOCtrlData);
 	   	 	   g_ppcs_channel[nAVChannelID].sendIOCtrlData = NULL;
 	   	 }
 	   	 
      Log3("avClientStart2 nAVChannelID: %d\n", nAVChannelID);
 	   	 
 	   	 ret = pthread_cond_init(&g_ppcs_channel[nAVChannelID].video_cond, NULL);
   		  if (ret != 0) {
   		  	pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].mutex);
       	 Log3("pthread_cond_init error: %s\n", strerror(ret));
       	 return -1;
  		  }
  		  ret = pthread_mutex_init(&g_ppcs_channel[nAVChannelID].video_lock, NULL);
   		  if (ret != 0) {
   		  		pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].mutex);
       	 Log3("pthread_mutex_init error: %s\n", strerror(ret));
       	 return -1;
  		  }
        ret = pthread_cond_init(&g_ppcs_channel[nAVChannelID].audio_cond, NULL);
   		  if (ret != 0) {
   		  		pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].mutex);
       	 Log3("pthread_cond_init error: %s\n", strerror(ret));
       	 return -1;
  		  }
  		  ret = pthread_mutex_init(&g_ppcs_channel[nAVChannelID].audio_lock, NULL);
   		  if (ret != 0) {
   		  		pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].mutex);
       	 Log3("pthread_mutex_init error: %s\n", strerror(ret));
       	 return -1;
  		  } 
  		  ret = pthread_cond_init(&g_ppcs_channel[nAVChannelID].ioctrl_cond, NULL);
   		  if (ret != 0) {
   		  		pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].mutex);
       	 Log3("pthread_cond_init error: %s\n", strerror(ret));
       	 return -1;
  		  }
  		  ret = pthread_mutex_init(&g_ppcs_channel[nAVChannelID].ioctrl_lock, NULL);
   		  if (ret != 0) {
   		  		pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].mutex);
       	 Log3("pthread_mutex_init error: %s\n", strerror(ret));
       	 return -1;
  		  } 
  		  ret = pthread_mutex_init(&g_ppcs_channel[nAVChannelID].ioctrl_listmutex, NULL);
   		  if (ret != 0) {
   		  		pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].mutex);
       	 Log3("pthread_mutex_init error: %s\n", strerror(ret));
       	 return -1;
  		  } 
  		  ret = pthread_mutex_init(&g_ppcs_channel[nAVChannelID].video_listmutex, NULL);
   		  if (ret != 0) {
   		  		pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].mutex);
       	 Log3("pthread_mutex_init error: %s\n", strerror(ret));
       	 return -1;
  		  }
  		  
  		   ret = pthread_mutex_init(&g_ppcs_channel[nAVChannelID].audio_listmutex, NULL);
   		  if (ret != 0) {
   		  		pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].mutex);
       	 Log3("pthread_mutex_init error: %s\n", strerror(ret));
       	 return -1;
  		  }  
 	   	 if(pthread_create(&g_ppcs_channel[nAVChannelID].Thread_IoCtrl_Rec_ID, NULL, &thread_ReceiveIoCtrl, (void *)&g_ppcs_channel[nAVChannelID]))
 	   	 	{
 	   	 		
 	   	 	}
 	   	 	if(pthread_create(&g_ppcs_channel[nAVChannelID].Thread_Audio_Rec_ID, NULL, &thread_ReceiveAudio, (void *)&g_ppcs_channel[nAVChannelID]))
 	   	 	{
 	   	 		
 	   	 	}
 	   	 	if(pthread_create(&g_ppcs_channel[nAVChannelID].Thread_Video_Rec_ID, NULL, &thread_ReceiveVideo, (void *)&g_ppcs_channel[nAVChannelID]))
 	   	 	{
 	   	 		
 	   	 	}	
 	   	 	*pnResend = 1;
 	   	 		pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].mutex);
 	   	}
 	 	 return nAVChannelID;	
}
void avClientExit(int nIOTCSessionID, unsigned char nIOTCChannelID)
{
	     Log3("avClientExit: nIOTCSessionID = %d nIOTCChannelID = %d\n",nIOTCSessionID, nIOTCChannelID);
 	    int nAVChannelID = avGetClientAVChannel(nIOTCSessionID,nIOTCChannelID);
 	    if(nAVChannelID!= -1)
 	    {
 	    	pthread_mutex_lock(&g_ppcs_channel[nAVChannelID].mutex);
 	    	Log3("avClientExit: nAVChannelID = %d \n",nAVChannelID);
 	    	g_ppcs_channel[nAVChannelID].nRun = 0;
 	    	if(g_ppcs_channel[nAVChannelID].nIOTCSessionID!= -1)
 	    	{ 	
 	    	PPCS_Close(g_ppcs_channel[nAVChannelID].nIOTCSessionID);
 	        g_ppcs_channel[nAVChannelID].nIOTCSessionID = -1;


			g_ppcs_channel[nAVChannelID].avStartRecvAudio = 0;
			g_ppcs_channel[nAVChannelID].avStartRecvVideo = 0;
			
 	    	Log3("all recive thread exit = %d \n",nAVChannelID); 	      
    		
 	 	   	 if(g_ppcs_channel[nAVChannelID].Thread_IoCtrl_Rec_ID!= -1) 
			{
				pthread_join(g_ppcs_channel[nAVChannelID].Thread_IoCtrl_Rec_ID, NULL);
			}
 	   		if(g_ppcs_channel[nAVChannelID].Thread_Audio_Rec_ID!= -1) 
			{
				pthread_join(g_ppcs_channel[nAVChannelID].Thread_Audio_Rec_ID, NULL);
			}
 	   		if(g_ppcs_channel[nAVChannelID].Thread_Video_Rec_ID!= -1) 
			{
				pthread_join(g_ppcs_channel[nAVChannelID].Thread_Video_Rec_ID, NULL); 
			}

 	   		g_ppcs_channel[nAVChannelID].Thread_IoCtrl_Rec_ID = -1;
 	   		g_ppcs_channel[nAVChannelID].Thread_Audio_Rec_ID = -1;
 	   		g_ppcs_channel[nAVChannelID].Thread_Video_Rec_ID = -1; 
		 
		   Log3("pthread_cond_signal: nAVChannelID = %d \n",nAVChannelID);  


    		pthread_mutex_lock(&g_ppcs_channel[nAVChannelID].ioctrl_lock);
 	    	pthread_cond_signal(&g_ppcs_channel[nAVChannelID].ioctrl_cond);
    		pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].ioctrl_lock);
	
		    Log3("pthread_cond_signal: nAVChannelID 11 = %d \n",nAVChannelID);  
 	    	pthread_mutex_lock(&g_ppcs_channel[nAVChannelID].video_lock);
 	    	pthread_cond_signal(&g_ppcs_channel[nAVChannelID].video_cond);
    		pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].video_lock);

    		Log3("pthread_cond_signal: nAVChannelID 22 = %d \n",nAVChannelID);  
  	    	pthread_mutex_lock(&g_ppcs_channel[nAVChannelID].audio_lock);
 	    	pthread_cond_signal(&g_ppcs_channel[nAVChannelID].audio_cond);
    		pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].audio_lock);
    		//sleep(1000);

		  Log3("pavClientExit = %d \n",333); 
 	   		
 		    g_ppcs_channel[nAVChannelID].sendVideoSize = 0;	
 	   	    g_ppcs_channel[nAVChannelID].sendAudioSize = 0;
 	   	    g_ppcs_channel[nAVChannelID].sendIOCtrlSize = 0;
 	   	    g_ppcs_channel[nAVChannelID].nWaitforKeyFrame = 0;
	     	   g_ppcs_channel[nAVChannelID].nBufTooBig = 0;  
                   g_ppcs_channel[nAVChannelID].avStartRecvAudio = 0;
		   g_ppcs_channel[nAVChannelID].avStartRecvVideo = 0;	   	  
		  if(g_ppcs_channel[nAVChannelID].sendAudioData != NULL)
 	   	 {
 	   	 	   free(g_ppcs_channel[nAVChannelID].sendAudioData);
 	   	 	   g_ppcs_channel[nAVChannelID].sendAudioData = NULL;
 	   	 }
 	   	 if(g_ppcs_channel[nAVChannelID].sendVideoData != NULL)
 	   	 {
 	   	 	   free(g_ppcs_channel[nAVChannelID].sendVideoData);
 	   	 	   g_ppcs_channel[nAVChannelID].sendVideoData = NULL;
 	   	 }
 	   	 if(g_ppcs_channel[nAVChannelID].sendIOCtrlData != NULL)
 	   	 {
 	   	 	   free(g_ppcs_channel[nAVChannelID].sendIOCtrlData);
 	   	 	   g_ppcs_channel[nAVChannelID].sendIOCtrlData = NULL;
 	   	 }
       g_ppcs_channel[nAVChannelID].sendAudioData = NULL;
       g_ppcs_channel[nAVChannelID].sendVideoData = NULL;
       g_ppcs_channel[nAVChannelID].sendIOCtrlData = NULL;	   		
 	   Log3("pavClientExit = %d \n",444); 
 	   		
 	   ReleaseAll_Ioctrl_Packet(&g_ppcs_channel[nAVChannelID]);
        ReleaseAll_Audio_Packet(&g_ppcs_channel[nAVChannelID]);
        ReleaseAll_Video_Packet(&g_ppcs_channel[nAVChannelID]);   
        g_ppcs_channel[nAVChannelID].iocttl_head = NULL;
	   g_ppcs_channel[nAVChannelID].video_head = NULL;
	   g_ppcs_channel[nAVChannelID].audio_head = NULL;
        Log3("pavClientExit = %d \n",555); 
	      
 	    pthread_cond_destroy(&g_ppcs_channel[nAVChannelID].video_cond);
     
        pthread_cond_destroy(&g_ppcs_channel[nAVChannelID].audio_cond);
       
        pthread_cond_destroy(&g_ppcs_channel[nAVChannelID].ioctrl_cond);
        
        pthread_mutex_destroy(&g_ppcs_channel[nAVChannelID].video_lock);
        pthread_mutex_destroy(&g_ppcs_channel[nAVChannelID].audio_lock);
        pthread_mutex_destroy(&g_ppcs_channel[nAVChannelID].ioctrl_lock);

	      
        pthread_mutex_destroy(&g_ppcs_channel[nAVChannelID].ioctrl_listmutex);
        pthread_mutex_destroy(&g_ppcs_channel[nAVChannelID].video_listmutex);
        pthread_mutex_destroy(&g_ppcs_channel[nAVChannelID].audio_listmutex);
        

       }
       	pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].mutex);
       }
 	Log3("avClientExit: nAVChannelID = %d  end\n", nAVChannelID);
}
void avClientStop(int nAVChannelID)
{
	
	    Log3("avClientStop: nAVChannelID = %d\n", nAVChannelID);
 	   if(nAVChannelID>=0 && nAVChannelID<MAX_CHANNEL)
 	   { 	   		 
 	   	if(g_ppcs_channel[nAVChannelID].nIOTCSessionID!= -1)
 	    	{ 	
             g_ppcs_channel[nAVChannelID].avStartRecvAudio = 0;
		     g_ppcs_channel[nAVChannelID].avStartRecvVideo = 0;
 	    	pthread_mutex_lock(&g_ppcs_channel[nAVChannelID].video_lock);
 	    	pthread_cond_signal(&g_ppcs_channel[nAVChannelID].video_cond);
    		pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].video_lock);
    
  	        pthread_mutex_lock(&g_ppcs_channel[nAVChannelID].audio_lock);
 	    	pthread_cond_signal(&g_ppcs_channel[nAVChannelID].audio_cond);
    		pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].audio_lock);
    
      
                }
 	   }	
}
void avClientStopEx(int nAVChannelID)
{
	
	    Log3("avClientStop: nAVChannelID = %d\n", nAVChannelID);
 	   if(nAVChannelID>=0 && nAVChannelID<MAX_CHANNEL)
 	   { 	   		 
 	   	if(g_ppcs_channel[nAVChannelID].nIOTCSessionID!= -1)
 	    	{ 	
            g_ppcs_channel[nAVChannelID].avStartRecvAudio = 0;
			g_ppcs_channel[nAVChannelID].avStartRecvVideo = 0;

    		pthread_mutex_lock(&g_ppcs_channel[nAVChannelID].ioctrl_lock);
 	    	pthread_cond_signal(&g_ppcs_channel[nAVChannelID].ioctrl_cond);
    		pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].ioctrl_lock);

 	    	pthread_mutex_lock(&g_ppcs_channel[nAVChannelID].video_lock);
 	    	pthread_cond_signal(&g_ppcs_channel[nAVChannelID].video_cond);
    		pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].video_lock);
    
  	        pthread_mutex_lock(&g_ppcs_channel[nAVChannelID].audio_lock);
 	    	pthread_cond_signal(&g_ppcs_channel[nAVChannelID].audio_cond);
    		pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].audio_lock);
    
      
                }
 	   }	
}
int avRecvFrameData(int nAVChannelID, char *abFrameData, int nFrameDataMaxSize,
								char *abFrameInfo, int nFrameInfoMaxSize, unsigned int *pnFrameIdx)
{
	// Log3("avRecvFrameData: nAVChannelID = %d\n", nAVChannelID);
	    	  int ret = AV_ER_DATA_NOREADY;
		  char * temp = NULL;
		  int err;
		  int result;
		  if(abFrameInfo == NULL || abFrameData == NULL)
		  {
		  	  return -1;
		  }
		 if(g_ppcs_channel[nAVChannelID].nIOTCSessionID!= -1)
	 	 {

		 	  result = Check_Video_Packet(&g_ppcs_channel[nAVChannelID]);
			 if(result == 1)
			 {
				goto RECV_VIDEO_FRAME;
			 }
	 	    pthread_mutex_lock(&g_ppcs_channel[nAVChannelID].video_lock);
 	    	err = pthread_cond_wait(&g_ppcs_channel[nAVChannelID].video_cond,&g_ppcs_channel[nAVChannelID].video_lock);   	
 	    	pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].video_lock);
		
 	  	  if(err == 0)
 	    	{
RECV_VIDEO_FRAME:

 	    	   if(g_ppcs_channel[nAVChannelID].nRun == 0)	
		   {
			return -1;
		   }
 	          st_PACKAGE* pa =  Get_Video_Packet(&g_ppcs_channel[nAVChannelID]);
           	 if(pa!= NULL)
           	 {
            	    if(pa->result>=0  && pa->Data!= NULL)
            	    {
  	                 temp = pa->Data;
	 	    		 if(nFrameInfoMaxSize>= pa->packinfo.frameinfo_size  && nFrameDataMaxSize>= pa->packinfo.frame_size)
	 	    		 {
			 	    	memcpy(abFrameInfo,temp,pa->packinfo.frameinfo_size);
			 	    	temp+=pa->packinfo.frameinfo_size;
			 	        memcpy(abFrameData,temp,pa->packinfo.frame_size);
			 	    	ret = pa->packinfo.frame_size;
			 	    	*pnFrameIdx = 0;
	 	    		 	
	 	 
	 	    		 		// 	Log3("avRecvFrameData2: nAVChannelID = %d\n", nAVChannelID);
	 	    		 }      	  	            	  	     
        	  		              	    		
            	    }
            	    else
            	    {
            	    	      ret = pa->result;
            	    	       switch(pa->result)
													{
						    		   	  case ERROR_PPCS_TIME_OUT:
					 	    		   	  	ret = AV_ER_REMOTE_TIMEOUT_DISCONNECT;
					 	    		   	 	break;
					 	    		   	 	case ERROR_PPCS_NOT_INITIALIZED:
					 	    		   	 	break;
					 	    		   	 	case ERROR_PPCS_INVALID_SESSION_HANDLE:
					 	    		   	 		ret = AV_ER_INVALID_SID;
					 	    		   	 	break;
					 	    		   	 	case ERROR_PPCS_SESSION_CLOSED_REMOTE:
					 	    		   	 		ret = AV_ER_SESSION_CLOSE_BY_REMOTE;
					 	    		   	 	break;
					 	    		   	 	case ERROR_PPCS_SESSION_CLOSED_TIMEOUT:
					 	    		   	 		ret = AV_ER_REMOTE_TIMEOUT_DISCONNECT;
					 	    		   	 	break; 	
					 	    		   	 	default:
					 	    		   	 	break;	    		   	
							 	    		   } 
            	    }
            	    Packet_Free(pa);
            	    
            }  		
 	    	}
    		
	 	 }
	 	
	 	 return ret;
}
int avRecvFrameData2(int nAVChannelID, char *abFrameData, int nFrameDataMaxSize, int *pnActualFrameSize,
								int *pnExpectedFrameSize, char *abFrameInfo, int nFrameInfoMaxSize,
								int *pnActualFrameInfoSize, unsigned int *pnFrameIdx)
{
	  // Log3("avRecvFrameData2: nAVChannelID = %d\n", nAVChannelID);
	  int ret = AV_ER_DATA_NOREADY;
	  char * temp = NULL;
	  int err;
	  int result;
	  if(abFrameInfo == NULL || abFrameData == NULL)
	  {
	  	  return -1;
	  }
		 if(g_ppcs_channel[nAVChannelID].nIOTCSessionID!= -1)
	 	 {
	 	     result = Check_Video_Packet(&g_ppcs_channel[nAVChannelID]);
			 if(result == 1)
			 {
				goto RECV_VIDEO_FRAME2;
			 }
	 		  pthread_mutex_lock(&g_ppcs_channel[nAVChannelID].video_lock);
 	    	err = pthread_cond_wait(&g_ppcs_channel[nAVChannelID].video_cond,&g_ppcs_channel[nAVChannelID].video_lock);   	
 	    	pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].video_lock);
 	  	  if(err == 0)
 	    	{

RECV_VIDEO_FRAME2:
  	    	   if(g_ppcs_channel[nAVChannelID].nRun == 0)	
		   {
			return -1;
		   }	    		
 	        st_PACKAGE* pa =  Get_Video_Packet(&g_ppcs_channel[nAVChannelID]);
            if(pa!= NULL)
            {
            	    if(pa->result>=0 && pa->Data!= NULL)
            	    {
  	                temp = pa->Data;
 	    		 			  if(nFrameInfoMaxSize>= pa->packinfo.frameinfo_size  && nFrameDataMaxSize>= pa->packinfo.frame_size)
 	    		 				{
 	    		 				 memcpy(abFrameInfo,temp,pa->packinfo.frameinfo_size);
 	    		 		 		 temp+=pa->packinfo.frameinfo_size;
 	    		 				 memcpy(abFrameData,temp,pa->packinfo.frame_size);
 	    		 				 ret = pa->packinfo.frame_size;
 	    		 				 *pnFrameIdx = 0;
 	    		 				 *pnExpectedFrameSize = ret;
 	    		 				 *pnActualFrameSize = ret;
 	    		 				 *pnActualFrameInfoSize = pa->packinfo.frameinfo_size;
 	 
 	    		 		// 	Log3("avRecvFrameData2: nAVChannelID = %d\n", nAVChannelID);
 	    		 				}      	  	            	  	     
        	  		              	    		
            	    }
            	    else
            	    {
            	    	      ret = pa->result;
            	    	       switch(pa->result)
													{
						    		   	  case ERROR_PPCS_TIME_OUT:
					 	    		   	  	ret = AV_ER_REMOTE_TIMEOUT_DISCONNECT;
					 	    		   	 	break;
					 	    		   	 	case ERROR_PPCS_NOT_INITIALIZED:
					 	    		   	 	break;
					 	    		   	 	case ERROR_PPCS_INVALID_SESSION_HANDLE:
					 	    		   	 		ret = AV_ER_INVALID_SID;
					 	    		   	 	break;
					 	    		   	 	case ERROR_PPCS_SESSION_CLOSED_REMOTE:
					 	    		   	 		ret = AV_ER_SESSION_CLOSE_BY_REMOTE;
					 	    		   	 	break;
					 	    		   	 	case ERROR_PPCS_SESSION_CLOSED_TIMEOUT:
					 	    		   	 		ret = AV_ER_REMOTE_TIMEOUT_DISCONNECT;
					 	    		   	 	break; 	
					 	    		   	 	default:
					 	    		   	 	break;	    		   	
							 	    		   } 
            	    }
            	    Packet_Free(pa);
            	    
            }  		
 	    	}
    		
	 	 }
	 	
	 	 return ret;
}
int avRecvAudioData(int nAVChannelID, char *abAudioData, int nAudioDataMaxSize,
								char *abFrameInfo, int nFrameInfoMaxSize, unsigned int *pnFrameIdx)
{
	 // Log3("avRecvAudioData: nAVChannelID = %d\n", nAVChannelID);
	  int ret = AV_ER_DATA_NOREADY;
	  int err;
	  char * temp = NULL;
	  int result;
	  if(abFrameInfo == NULL || abAudioData == NULL)
	  {
	  	  return -1;
	  }
		 if(g_ppcs_channel[nAVChannelID].nIOTCSessionID!= -1)
	 	{

		     result = Check_Audio_Packet(&g_ppcs_channel[nAVChannelID]);
			 if(result == 1)
			 {
				goto RECV_AUDIO_RAME;
			 }
	         pthread_mutex_lock(&g_ppcs_channel[nAVChannelID].audio_lock);
 	    	err = pthread_cond_wait(&g_ppcs_channel[nAVChannelID].audio_cond,&g_ppcs_channel[nAVChannelID].audio_lock);
 	        pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].audio_lock);
 	    	if(err == 0)
 	    	{
RECV_AUDIO_RAME:
  	    	   if(g_ppcs_channel[nAVChannelID].nRun == 0)	
		   {
			return -1;
		   }	    		
 	          st_PACKAGE* pa =  Get_Audio_Packet(&g_ppcs_channel[nAVChannelID]);
            if(pa!= NULL)
            {
            	    if(pa->result>=0 && pa->Data!= NULL)
            	    {
  	              			 temp = pa->Data;
		 								if(nFrameInfoMaxSize>= pa->packinfo.frameinfo_size  && nAudioDataMaxSize>= pa->packinfo.frame_size)
		 	    		 			{
		 	    		 			 memcpy(abFrameInfo,temp,pa->packinfo.frameinfo_size);
		 	    		 			 temp+=pa->packinfo.frameinfo_size;
		 	    		 			 memcpy(abAudioData,temp,pa->packinfo.frame_size);
		 	    		 			 ret = pa->packinfo.frame_size;
		 	    		 			 *pnFrameIdx = 0;
		 	    		 	
		 	    		 			}       	  	            	  	     
        	  		              	    		
            	    }
            	    else
            	    {
            	    	       ret = pa->result;
            	    	       switch(pa->result)
												 	 {
						 	    		   	  case ERROR_PPCS_TIME_OUT:
						 	    		   	  	ret = AV_ER_REMOTE_TIMEOUT_DISCONNECT;
						 	    		   	 	break;
						 	    		   	 	case ERROR_PPCS_NOT_INITIALIZED:
						 	    		   	 	break;
						 	    		   	 	case ERROR_PPCS_INVALID_SESSION_HANDLE:
						 	    		   	 		ret = AV_ER_INVALID_SID;
						 	    		   	 	break;
						 	    		   	 	case ERROR_PPCS_SESSION_CLOSED_REMOTE:
						 	    		   	 		ret = AV_ER_SESSION_CLOSE_BY_REMOTE;
						 	    		   	 	break;
						 	    		   	 	case ERROR_PPCS_SESSION_CLOSED_TIMEOUT:
						 	    		   	 		ret = AV_ER_REMOTE_TIMEOUT_DISCONNECT;
						 	    		   	 	break; 	
						 	    		   	 	default:
						 	    		   	 	break;		    		   	
							 	    		   } 
            	    }
            	    Packet_Free(pa);
            	    
            }  		
 	    	}
 	    	
 	    	
    	
	 	}
	 	
	 	 return ret;
}
int avCheckAudioBuf(int nAVChannelID)
{
	 	 return 0;
}
void avClientSetMaxBufSize(unsigned int nMaxBufSize)
{
	 
}
int avClientCleanBuf(int nAVChannelID)
{
	 	   if(nAVChannelID>=0 && nAVChannelID<MAX_CHANNEL)
 	   	{ 	   		 
 	   		if(g_ppcs_channel[nAVChannelID].nIOTCSessionID!= -1)
 	    	{ 	
        	ReleaseAll_Audio_Packet(&g_ppcs_channel[nAVChannelID]);
        	ReleaseAll_Video_Packet(&g_ppcs_channel[nAVChannelID]);   
	      	g_ppcs_channel[nAVChannelID].video_head = NULL;
	      	g_ppcs_channel[nAVChannelID].audio_head = NULL;
	      }
	    }
        
	 	 return 0;
}
int avClientCleanVideoBuf(int nAVChannelID)
{
	
	 	   if(nAVChannelID>=0 && nAVChannelID<MAX_CHANNEL)
 	   	{ 	   		 
 	   		if(g_ppcs_channel[nAVChannelID].nIOTCSessionID!= -1)
 	    		{ 	
        		ReleaseAll_Video_Packet(&g_ppcs_channel[nAVChannelID]);   
	      		g_ppcs_channel[nAVChannelID].video_head = NULL;
	      		}
	    	}
	    
	 	 return 0;
}
int avClientCleanLocalBuf(int nAVChannelID)
{
	 	 return 0;
}	
int avClientCleanLocalVideoBuf(int nAVChannelID)
{
	 	 return 0;
}	
int avClientCleanAudioBuf(int nAVChannelID)
{
	
	 	   if(nAVChannelID>=0 && nAVChannelID<MAX_CHANNEL)
 	   	{ 	   		 
 	   		if(g_ppcs_channel[nAVChannelID].nIOTCSessionID!= -1)
 	    	{ 	
        ReleaseAll_Audio_Packet(&g_ppcs_channel[nAVChannelID]); 
	      g_ppcs_channel[nAVChannelID].audio_head = NULL;
	      }
	    }
	    
	 	 return 0;
}	
int avSendIOCtrl(int nAVChannelID, unsigned int nIOCtrlType, const char *cabIOCtrlData, int nIOCtrlDataSize)
{
	//  Log3("avSendIOCtrl: nAVChannelID = %d\n", nAVChannelID);
	  int ssize = 0;
	  int send = 0;
	  char* temp = NULL;
	  st_PACKAGE_INFO package_info;
	  package_info.package_head = PACKAGE_HEAD;
		if(nAVChannelID>=0 && nAVChannelID<MAX_CHANNEL)
		{
			 if(g_ppcs_channel[nAVChannelID].nIOTCSessionID!= -1)
			 	{
			 		  ssize = nIOCtrlDataSize+sizeof(unsigned int)+sizeof(st_PACKAGE_INFO);
			 		  if(ssize>g_ppcs_channel[nAVChannelID].sendIOCtrlSize)
			 		  {
			 		  	   if(g_ppcs_channel[nAVChannelID].sendIOCtrlData!= NULL)
			 		  	   	{
			 		  	   		free(g_ppcs_channel[nAVChannelID].sendIOCtrlData);
			 		  	   	}
			 		  	   g_ppcs_channel[nAVChannelID].sendIOCtrlSize = ssize+1;
			 		  	   g_ppcs_channel[nAVChannelID].sendIOCtrlData =(char*)malloc(g_ppcs_channel[nAVChannelID].sendIOCtrlSize*sizeof(char));
			 		  }
			 		  if(g_ppcs_channel[nAVChannelID].sendIOCtrlData!= NULL)
			 		  	{
			 		  		 temp = g_ppcs_channel[nAVChannelID].sendIOCtrlData;			 		  		 
			 		  		 package_info.package_size = nIOCtrlDataSize+sizeof(unsigned int);
			 		  		 package_info.frameinfo_size = sizeof(unsigned int);
								 package_info.frame_size = nIOCtrlDataSize;
			 		  		 memcpy(temp,&package_info,sizeof(st_PACKAGE_INFO));
			 		  		 temp+=sizeof(st_PACKAGE_INFO);				 		  		 
			 		  		 memcpy(temp,&nIOCtrlType,sizeof(unsigned int));
			 		  		 temp+=sizeof(unsigned int);
			 		  		 memcpy(temp,cabIOCtrlData,nIOCtrlDataSize);
			 		  	   send =  PPCS_Write(g_ppcs_channel[nAVChannelID].nIOTCSessionID,DATA_TYPE_IOCTRL,g_ppcs_channel[nAVChannelID].sendIOCtrlData,ssize);
			 		  	   if(send>=0)
			 		  	   	{
			 		  	   		  return AV_ER_NoERROR;
			 		  	   	}
			 		  	   	else
			 		  	   	{
			 		  	   		
			 		  	   		switch(send)
			 		  	   		{
			 		  	   			  case ERROR_PPCS_NOT_INITIALIZED:
			 		  	   			  	return AV_ER_INVALID_ARG;
			 		  	   			 	break;
			 		  	   			 	case ERROR_PPCS_INVALID_PARAMETER:			 		  	   			 	
			 		  	   			 	break;
			 		  	   			 	case ERROR_PPCS_INVALID_SESSION_HANDLE:
			 		  	   			 		return AV_ER_INVALID_SID;
			 		  	   			 	break;
			 		  	   			 	case ERROR_PPCS_SESSION_CLOSED_REMOTE:
			 		  	   			 		return AV_ER_SESSION_CLOSE_BY_REMOTE ;
			 		  	   			 	break;
			 		  	   			 	case ERROR_PPCS_SESSION_CLOSED_TIMEOUT:
			 		  	   			 		return AV_ER_REMOTE_TIMEOUT_DISCONNECT;
			 		  	   			 	break;
			 		  	   			 	case ERROR_PPCS_REMOTE_SITE_BUFFER_FULL:
			 		  	   			 		return AV_ER_EXCEED_MAX_SIZE;
			 		  	   			 	break;
			 		  	   		}
			 		  	   		return send;

			 		  	   	}
			 		  	   	
			 		  	}
			 		 
			 	}
			
		}
	    
	 	 return -1;	
	
}	
int avRecvIOCtrl(int nAVChannelID, unsigned int *pnIOCtrlType, char *abIOCtrlData, int nIOCtrlMaxDataSize, unsigned int nTimeout)
{
	
      Log3("avRecvIOCtrl: nAVChannelID = %d enter\n", nAVChannelID);
	  int err;
	  int res;
	  char* temp = NULL;
	  unsigned int *nIOCtrlType;
	  int nIOCtrlDataSize = -100;
	  st_PACKAGE* pa = NULL;
		if(g_ppcs_channel[nAVChannelID].nIOTCSessionID!= -1)
	 	{
	 	      res = Check_Ioctrl_Packet(&g_ppcs_channel[nAVChannelID]);
			  if(res == 1)
			  {
				  goto RECV_IOCTRL;
			  }
	 		  pthread_mutex_lock(&g_ppcs_channel[nAVChannelID].ioctrl_lock);	  		 		
			  err = pthread_cond_wait(&g_ppcs_channel[nAVChannelID].ioctrl_cond, &g_ppcs_channel[nAVChannelID].ioctrl_lock);
			  pthread_mutex_unlock(&g_ppcs_channel[nAVChannelID].ioctrl_lock);
        Log3("avRecvIOCtrl: result = %d\n", err);
       	if(err == 0)
       	 {
RECV_IOCTRL:
	 	    if(g_ppcs_channel[nAVChannelID].nRun == 0)	
			{
				return -1;
			}
            pa =  Get_Ioctrl_Packet(&g_ppcs_channel[nAVChannelID]);
            if(pa!= NULL)
            {
            	   Log3("avRecvIOCtrl: Get_Ioctrl_Packet = %d\n", err);
            	    if(pa->result>=0  && pa->Data!= NULL)
            	    {
  	                  temp = pa->Data;
        	  	     nIOCtrlType = (unsigned int *)temp;
        	  	     *pnIOCtrlType = *nIOCtrlType;
        	  	     temp+=sizeof(unsigned int);
        	  	     nIOCtrlDataSize = pa->len-sizeof(unsigned int);
        	  	     if(nIOCtrlMaxDataSize>=nIOCtrlDataSize && nIOCtrlDataSize>0)
        	  	     	{
        	  	     		memcpy(abIOCtrlData,temp,nIOCtrlDataSize);
        	  	     	}       	  	     
        	  		              	    		
            	    }
            	    else
            	    {
            	    	        nIOCtrlDataSize = pa->result;
            	    	       switch(pa->result)
							 	    		   {
							 	    		   	  case ERROR_PPCS_TIME_OUT:
							 	    		   	  	nIOCtrlDataSize = AV_ER_REMOTE_TIMEOUT_DISCONNECT;
							 	    		   	 	break;
							 	    		   	 	case ERROR_PPCS_NOT_INITIALIZED:
							 	    		   	 		nIOCtrlDataSize = pa->result;
							 	    		   	 	break;
							 	    		   	 	case ERROR_PPCS_INVALID_SESSION_HANDLE:
							 	    		   	 		nIOCtrlDataSize = AV_ER_INVALID_SID;
							 	    		   	 	break;
							 	    		   	 	case ERROR_PPCS_SESSION_CLOSED_REMOTE:
							 	    		   	 		nIOCtrlDataSize = AV_ER_SESSION_CLOSE_BY_REMOTE;
							 	    		   	 	break;
							 	    		   	 	case ERROR_PPCS_SESSION_CLOSED_TIMEOUT:
							 	    		   	 		nIOCtrlDataSize = AV_ER_REMOTE_TIMEOUT_DISCONNECT;
							 	    		   	 	break; 	
							 	    		   	 	default:
							 	    		   	 			nIOCtrlDataSize = pa->result;
							 	    		   	 	break;	    		   	
							 	    		   } 
            	    }
            	    Packet_Free(pa);
            	    
            }
        
        	}
        	else if(err == 110)
       	 	{
        	  nIOCtrlDataSize =  AV_ER_TIMEOUT;
        	}
       
    
    	
	 	}
	 	//Log3("avRecvIOCtrl: nAVChannelID = %d  leave\n", nAVChannelID);
	 	return nIOCtrlDataSize;
}
void avStartRecvAudio(int nAVChannelID)
{
 	   if(nAVChannelID>=0 && nAVChannelID<MAX_CHANNEL)
 	   { 	   		 
 	   	if(g_ppcs_channel[nAVChannelID].nIOTCSessionID!= -1)
 	    	{ 	
		g_ppcs_channel[nAVChannelID].avStartRecvAudio = 1;
		}
	   }
}
void avStartRecvVideo(int nAVChannelID)
{
 	   if(nAVChannelID>=0 && nAVChannelID<MAX_CHANNEL)
 	   { 	   		 
 	   	if(g_ppcs_channel[nAVChannelID].nIOTCSessionID!= -1)
 	    	{ 	
		g_ppcs_channel[nAVChannelID].avStartRecvVideo = 1;
		}
	   }
}

	
int avSendIOCtrlExit(int nAVChannelID)
{
	 	 return 0;
}
float avResendBufUsageRate (int nAVChannelID)
{
	 	 return 0.0;
}	
void AV_Set_Log_Path(char *path, int nMaxSize)
{
	
}																				
