#include "download.h"

#include "time.h"

#include <stdlib.h>

#define MAXHEADERSIZE 1024
char m_requestheader[1024];

char m_ResponseHeader[1024];

int m_port;

char m_ipaddr[256];

bool m_bConnected;

int m_s;

int m_phostent= 0;

int m_nCurIndex;//GetResponsLine();

bool m_bResponsed;

int m_nResponseHeaderSize;

int gDownRate;

int mofang_net_resolve_host(struct in_addr *sin_addr, const char *hostname)
{

	if (!inet_aton(hostname, sin_addr)) {
		struct addrinfo *ai, *cur;
		struct addrinfo hints = { 0 };
		hints.ai_family = AF_INET;
		if (getaddrinfo(hostname, NULL, &hints, &ai))
			return -1;
		/* getaddrinfo returns a linked list of addrinfo structs.
		 * Even if we set ai_family = AF_INET above, make sure
		 * that the returned one actually is of the correct type. */
		for (cur = ai; cur; cur = cur->ai_next) {
			if (cur->ai_family == AF_INET) {
				*sin_addr = ((struct sockaddr_in *)cur->ai_addr)->sin_addr;
				freeaddrinfo(ai);
				return 0;
			}
		}
		freeaddrinfo(ai);
		return -1;
	}
	return 0;
}


void NewCHttpSocket()

{
	int i =0;
	m_s=0;

	m_phostent=NULL;

	//m_port=80; 

	m_bConnected = false; 

	for(i=0;i<256;i++)

	m_ipaddr[i]='\0';

	memset(m_requestheader,0,MAXHEADERSIZE);

	memset(m_ResponseHeader,0,MAXHEADERSIZE); 

	m_nCurIndex = 0;

	m_bResponsed = false;

	m_nResponseHeaderSize = -1;

} 

void DeleteCHttpSocket()

{

	CloseSocket();

} 

bool SocketHttp()

{

	if(m_bConnected)return false; 

	//struct protoent *ppe; 

	//ppe=getprotobyname("tcp"); 

	m_s=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

	if(m_s==-1)

	{

		return false;

	} 

	return true; 

} 

bool ConnectHttp(const char *szHostName,int nPort)

{

	if(szHostName==NULL)

	return false;

	if(m_bConnected)

	{

		CloseSocket();

	    return false;

	}

	m_port=nPort;

	struct in_addr ip_addr;
	
	m_phostent = mofang_net_resolve_host(&ip_addr,szHostName);

	//m_phostent=gethostbyname(szHostName);

	if(m_phostent == -1)

	{
	
		return false;

	}

	
	//memcpy(&ip_addr,m_phostent->h_addr_list[0],4);



	struct sockaddr_in destaddr;

	destaddr.sin_family=AF_INET;

	destaddr.sin_port=htons(m_port);

	destaddr.sin_addr=ip_addr;

    int ret;

	if((ret = connect(m_s,(struct sockaddr*)&destaddr,sizeof(struct sockaddr)))!=0)

	{  

		return false;

	}

	m_bConnected=true;

	return true;

}

char *FormatRequestHeader(const char *pServer,const char *pObject, long *Length,

 char *pCookie,char *pReferer,long nFrom, long nTo,int nServerType)

{

	char szPort[10];

	//char szTemp[20];

	sprintf(szPort,"%d",m_port);

	memset(m_requestheader,'\0',1024);

	strcat(m_requestheader,"GET ");

	strcat(m_requestheader,pObject);

	strcat(m_requestheader," HTTP/1.1");

	strcat(m_requestheader,"\r\n");

	strcat(m_requestheader,"Host:");

	strcat(m_requestheader,pServer);

	strcat(m_requestheader,"\r\n"); 

	if(pReferer != NULL)

	{

		strcat(m_requestheader,"Referer:");

		strcat(m_requestheader,pReferer);

		strcat(m_requestheader,"\r\n");

	}

    strcat(m_requestheader,"Accept:*/*");

    strcat(m_requestheader,"\r\n");

    strcat(m_requestheader,"User-Agent:Mozilla/4.0 (compatible; MSIE 5.00; Windows 98)");

    strcat(m_requestheader,"\r\n");

	strcat(m_requestheader,"Connection:Keep-Alive");

	strcat(m_requestheader,"\r\n");

	if(pCookie != NULL)

	{

		strcat(m_requestheader,"Set Cookie:0");

		strcat(m_requestheader,pCookie);

		strcat(m_requestheader,"\r\n");

	}

	if(nFrom > 0)

	{

	//strcat(m_requestheader,"Range: bytes=");

	//_ltoa(nFrom,szTemp,10);

	//strcat(m_requestheader,szTemp);

	//strcat(m_requestheader,"-");

	//if(nTo > nFrom)

	//{

	//_ltoa(nTo,szTemp,10);

	//strcat(m_requestheader,szTemp);

	//}

	//strcat(m_requestheader,"\r\n");

	}

	strcat(m_requestheader,"\r\n");

	*Length=strlen(m_requestheader);

	return m_requestheader;

}

bool SendRequest(const char *pRequestHeader, long Length)

{

	if(!m_bConnected)return false;

	if(pRequestHeader==NULL)

		pRequestHeader=m_requestheader;

	if(Length==0)

		Length=strlen(m_requestheader);

	if(send(m_s,pRequestHeader,Length,0)==-1)
	{

		return false;

	}

	int nLength;

	GetResponseHeader(&nLength);

	return true;

}

long ReceiveHttp(char* pBuffer,long nMaxLength)

{

	if(!m_bConnected)
		return 0;

	long nLength;

	nLength=recv(m_s,pBuffer,nMaxLength,0);

	if(nLength <= 0)
	{

		CloseSocket();

	}

	return nLength;

}

bool CloseSocket()

{

	if(m_s != 0)
	{

		if(close(m_s)==-1)

		{

			return false;

		}

	}

	m_s = 0;

	m_bConnected=false;

	return true;

} 

int GetRequestHeader(char *pHeader, int nMaxLength)

{

	int nLength;

	if((strlen(m_requestheader))>nMaxLength)

	{

		nLength=nMaxLength;

	}

	else

	{

		nLength=strlen(m_requestheader);

	}

	memcpy(pHeader,m_requestheader,nLength);

	return nLength;

}

bool SetTimeout(int nTime, int nType)

{

	if(nType == 0)

	{

		nType = SO_RCVTIMEO;

	}

	else

	{

		nType = SO_SNDTIMEO;

	}

	int dwErr;

    dwErr=setsockopt(m_s,SOL_SOCKET,nType,(char*)&nTime,sizeof(nTime)); 

	if(dwErr)

	{

		return false;

	}

	return true;

}

char* GetResponseHeader(int *nLength)

{

	if(!m_bResponsed)

	{

		char c = 0;

		int nIndex = 0;

		bool bEndResponse = false;

		while(!bEndResponse && nIndex < MAXHEADERSIZE)

		{

			recv(m_s,&c,1,0);

			m_ResponseHeader[nIndex++] = c;

			if(nIndex >= 4)

			{

				if(m_ResponseHeader[nIndex - 4] == '\r' && m_ResponseHeader[nIndex - 3] == '\n'

				&& m_ResponseHeader[nIndex - 2] == '\r' && m_ResponseHeader[nIndex - 1] == '\n')

				bEndResponse = true;

			}

		}

		m_nResponseHeaderSize = nIndex;

		m_bResponsed = true;

	}


	*nLength = m_nResponseHeaderSize;

	return m_ResponseHeader;

}

int GetResponseLine(char *pLine, int nMaxLength)

{

	if(m_nCurIndex >= m_nResponseHeaderSize)

	{

		m_nCurIndex = 0;

		return -1;

	}

	int nIndex = 0;

	char c = 0;

	do 

	{

		c = m_ResponseHeader[m_nCurIndex++];

		pLine[nIndex++] = c;

	} while(c != '\n' && m_nCurIndex < m_nResponseHeaderSize && nIndex < nMaxLength);

	return nIndex;

}

int GetField(const char *szSession, char *szValue, int nMaxLength)

{
#if 0
	if(!m_bResponsed) return -1;

	std::string strRespons;

	strRespons = m_ResponseHeader;

	int nPos = -1;

	nPos = strRespons.find(szSession,0);

	if(nPos != -1)

	{

		nPos += strlen(szSession);

		nPos += 2;

		int nCr = strRespons.find("\r\n",nPos);

		std::string strValue = strRespons.substr(nPos,nCr - nPos);

		strcpy(szValue,strValue.c_str());

		return (nCr - nPos);

	}

	else

	{

		return -1;

	}
	#endif
	return 0;

}

int GetServerState()

{

	if(!m_bResponsed) return -1;

	char szState[3];

	szState[0] = m_ResponseHeader[9];

	szState[1] = m_ResponseHeader[10];

	szState[2] = m_ResponseHeader[11];

	return atoi(szState);

}


int DownLoadFIle(char* serverName, int port,char* URL,char* localDirectory)

{

	
	FILE *fp;

    if(!(fp = fopen(localDirectory,"wb")))

    {

        printf("can't open local file\n");

        return -1;

    }
	m_port = port;
    NewCHttpSocket();

    SocketHttp();    

    ConnectHttp(serverName,port);

    long len = 0;

    char* req = FormatRequestHeader(serverName,URL,&len,NULL,NULL,0,0,0);

    SendRequest(req,len);   

    int lens;

    char* head = GetResponseHeader(&lens);
	if(head)
    	printf("head %s\n",head);
	else
	 {
        printf("head error\n"); 
        fclose(fp);
		return -2;
	}
    
	char* ptmp=strstr(head,"404 Not Found");

	//std::string::size_type idx;
	//idx=head.find("404 Not Found");

    if(ptmp != NULL ) 
    {
        printf("found\n"); 
        fclose(fp);
		return -3;
	}
    

	int cnt = 0;

    //int flag = head.find("Content-Length:",0);

    //int endFlag = head.find("\r\n",flag);

	char* p1 = strstr(head, "Content-Length:");
	
	char* p2 = strstr(p1, "\r\n");
	
	int lengthlen = p2-p1;
	printf("lengthlen %d",lengthlen);
	if(lengthlen>63){
        fclose(fp);
		return -4;
    }
	
	char subStr[64];
	memset(subStr,0,sizeof(subStr));
	memcpy(subStr,p1,lengthlen);
    //std::string subStr = head.substr(flag,endFlag-flag);

    sscanf(subStr,"Content-Length: %d",&lens);

	printf("lens = %d\n",lens);

	if(lens<=1024){
        fclose(fp);
		return -5;
    }   
    fseek(fp,0,0);

    while(cnt < lens)

    {

        char buff[1025];

        int tmplen = ReceiveHttp(buff,1024);

        cnt += tmplen;

        fwrite(buff,1,tmplen,fp);      

    }

    fclose(fp);

    return 0;

}

int DownTestRateFIle(char* serverName, int port,char* URL,char* localDirectory)

{

	
/*	FILE *fp;

    if(!(fp = fopen(localDirectory,"wb")))

    {

        printf("can't open local file\n");

        return -1;

    }*/

	gDownRate = -1;
	m_port = port;
    NewCHttpSocket();

    SocketHttp();    

    ConnectHttp(serverName,port);

    long len = 0;

    char* req = FormatRequestHeader(serverName,URL,&len,NULL,NULL,0,0,0);

    SendRequest(req,len);   

    int lens;

    char* head = GetResponseHeader(&lens);
	if(head)
    	printf("head %s\n",head);
	else
	 {
        printf("head error\n"); 
		return -1;
	}
    
	char* ptmp=strstr(head,"404 Not Found");

	//std::string::size_type idx;
	//idx=head.find("404 Not Found");

    if(ptmp != NULL ) 
    {
        printf("found\n"); 
		return -1;
	}
    

	int cnt = 0;

    //int flag = head.find("Content-Length:",0);

    //int endFlag = head.find("\r\n",flag);

	char* p1 = strstr(head, "Content-Length:");
	
	char* p2 = strstr(p1, "\r\n");
	
	int lengthlen = p2-p1;
	printf("lengthlen %d",lengthlen);
	if(lengthlen>63)
		return -1;
	
	char subStr[64];
	memset(subStr,0,sizeof(subStr));
	memcpy(subStr,p1,lengthlen);
    //std::string subStr = head.substr(flag,endFlag-flag);

    sscanf(subStr,"Content-Length: %d",&lens);

	printf("lens = %d\n",lens);

	if(lens<=1024)
		return -1;
	
   // fseek(fp,0,0);
	time_t timestart,timeend;
	time(&timestart);
    while(cnt < lens)

    {

        char buff[1025];

        int tmplen = ReceiveHttp(buff,1024);

        cnt += tmplen;

       // fwrite(buff,1,tmplen,fp);      

    }
	time(&timeend);
	printf("--------------subtime %ld,lens = %d\n",(timeend-timestart),lens);
	gDownRate =cnt/ (timeend-timestart);
	printf("--------------rate %d\n",gDownRate);

//-------------------------------------------------------up 
	
   // fclose(fp);

    return 0;

}

