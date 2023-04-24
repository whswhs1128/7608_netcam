/*
$Id: ssmtp.c,v 2.60 2003/08/17 14:17:57 matt Exp $
sSMTP -- send messages via SMTP to a mailhub for local delivery or forwarding.
This program is used in place of /usr/sbin/sendmail, called by "mail" (et all).
sSMTP does a selected subset of sendmail's standard tasks (including exactly
one rewriting task), and explains if you ask it to do something it can't. It
then sends the mail to the mailhub via an SMTP connection. Believe it or not,
this is nothing but a filter
See COPYRIGHT for the license
 */
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <netdb.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include<arpa/inet.h> //inet_addr , inet_ntoa , ntohs etc
#include<netinet/in.h>
#include<unistd.h>


#ifdef HAVE_SSL
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

#include "Ssmtp.h"
#include "Base64.h"

int use_tls = 0;							/* Use SSL to transfer mail to HUB */

#ifdef HAVE_SSL
SSL *ssl = NULL;
#endif

#define ARPADATE_LENGTH	32

#define PRINT_INFO(fmt, args...) \
do{ \
    printf("[FUN]%s [LINE]%d  "fmt, __FUNCTION__, __LINE__, ##args); \
}while(0)

//List of DNS Servers registered on the system
char dns_servers[10][100] = {0};
int dns_server_count = 0;
//Types of DNS resource records :)

#define T_A 1 //Ipv4 address
#define T_NS 2 //Nameserver
#define T_CNAME 5 // canonical name
#define T_SOA 6 /* start of authority zone */
#define T_PTR 12 /* domain name pointer */
#define T_MX 15 //Mail server


/*
 * Perform a DNS query by sending a packet
 * */
void gethostbyname_mail(struct DNS_IPADDR *dns_addr,char *host, int query_type) {
	unsigned char buf[65536], *qname, *reader;
	int i, j, stop, s;
	struct timeval tm;
	socklen_t len = sizeof(tm);
	struct RES_RECORD answers[20], auth[20], addit[20]; //the replies from the DNS server
	struct sockaddr_in dest;

	struct DNS_HEADER *dns = NULL;
	struct QUESTION *qinfo = NULL;

	printf("Resolving %s", host);

	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); //UDP packet for DNS queries
	
	dest.sin_family = AF_INET;
	dest.sin_port = htons(53);
	dest.sin_addr.s_addr = inet_addr(dns_servers[0]); //dns servers

	//Set the DNS structure to standard queries
	dns = (struct DNS_HEADER *) &buf;

	dns->id = (unsigned short) htons(getpid());
	dns->qr = 0; //This is a query
	dns->opcode = 0; //This is a standard query
	dns->aa = 0; //Not Authoritative
	dns->tc = 0; //This message is not truncated
	dns->rd = 1; //Recursion Desired
	dns->ra = 0; //Recursion not available! hey we dont have it (lol)
	dns->z = 0;
	dns->ad = 0;
	dns->cd = 0;
	dns->rcode = 0;
	dns->q_count = htons(1); //we have only 1 question
	dns->ans_count = 0;
	dns->auth_count = 0;
	dns->add_count = 0;

	//point to the query portion
	qname = (unsigned char*) &buf[sizeof(struct DNS_HEADER)];

	ChangetoDnsNameFormat(qname, host);
	qinfo = (struct QUESTION*) &buf[sizeof(struct DNS_HEADER)
			+ (strlen((const char*) qname) + 1)]; //fill it

	qinfo->qtype = htons(query_type); //type of the query , A , MX , CNAME , NS etc
	qinfo->qclass = htons(1); //its internet (lol)

	printf("\nSending Packet...");
	if (sendto(s, (char*) buf,
			sizeof(struct DNS_HEADER) + (strlen((const char*) qname) + 1)
					+ sizeof(struct QUESTION), 0, (struct sockaddr*) &dest,
			sizeof(dest)) < 0) {
		perror("sendto failed");
	}
	printf("Done");

	 /*设置timeout机制*/
    tm.tv_sec = 5;
    tm.tv_usec = 0;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tm, len);
	
	//Receive the answer
	i = sizeof dest;
	printf("\nReceiving answer...");
	if (recvfrom(s, (char*) buf, 65536, 0, (struct sockaddr*) &dest,
			(socklen_t*) &i) < 0) {
		perror("recvfrom failed");
	}
	printf("Done");

	dns = (struct DNS_HEADER*) buf;

	//move ahead of the dns header and the query field
	reader = &buf[sizeof(struct DNS_HEADER) + (strlen((const char*) qname) + 1)
			+ sizeof(struct QUESTION)];

	printf("\nThe response contains : ");
	printf("\n %d Questions.", ntohs(dns->q_count));
	printf("\n %d Answers.", ntohs(dns->ans_count));
	printf("\n %d Authoritative Servers.", ntohs(dns->auth_count));
	printf("\n %d Additional records.\n\n", ntohs(dns->add_count));

	//Start reading answers
	stop = 0;

	for (i = 0; i < ntohs(dns->ans_count); i++) {
		answers[i].name = ReadName(reader, buf, &stop);
		reader = reader + stop;

		answers[i].resource = (struct R_DATA*) (reader);
		reader = reader + sizeof(struct R_DATA);

		if (ntohs(answers[i].resource->type) == 1) //if its an ipv4 address
				{
			answers[i].rdata = (unsigned char*) malloc(
					ntohs(answers[i].resource->data_len));

			for (j = 0; j < ntohs(answers[i].resource->data_len); j++) {
				answers[i].rdata[j] = reader[j];
			}

			answers[i].rdata[ntohs(answers[i].resource->data_len)] = '\0';

			reader = reader + ntohs(answers[i].resource->data_len);
		} else {
			answers[i].rdata = ReadName(reader, buf, &stop);
			reader = reader + stop;
		}
	}

	//read authorities
	for (i = 0; i < ntohs(dns->auth_count); i++) {
		auth[i].name = ReadName(reader, buf, &stop);
		reader += stop;

		auth[i].resource = (struct R_DATA*) (reader);
		reader += sizeof(struct R_DATA);

		auth[i].rdata = ReadName(reader, buf, &stop);
		reader += stop;
	}

	//read additional
	for (i = 0; i < ntohs(dns->add_count); i++) {
		addit[i].name = ReadName(reader, buf, &stop);
		reader += stop;

		addit[i].resource = (struct R_DATA*) (reader);
		reader += sizeof(struct R_DATA);

		if (ntohs(addit[i].resource->type) == 1) {
			addit[i].rdata = (unsigned char*) malloc(
					ntohs(addit[i].resource->data_len));
			for (j = 0; j < ntohs(addit[i].resource->data_len); j++)
				addit[i].rdata[j] = reader[j];

			addit[i].rdata[ntohs(addit[i].resource->data_len)] = '\0';
			reader += ntohs(addit[i].resource->data_len);
		} else {
			addit[i].rdata = ReadName(reader, buf, &stop);
			reader += stop;
		}
	}

	//print answers
	printf("\nAnswer Records : %d \n", ntohs(dns->ans_count));
	for (i = 0; i < ntohs(dns->ans_count); i++) {
		printf("Name : %s ", answers[i].name);

		if (ntohs(answers[i].resource->type) == T_A) //IPv4 address
		{
			unsigned long *p;
			p = (unsigned long*) answers[i].rdata;
			//a[i].sin_addr.s_addr = (*p); //working without ntohl
			dns_addr[i].ipaddr = (*p);
			printf("has IPv4 address : %lu\n",dns_addr[i].ipaddr);
			//printf("has IPv4 address : %s\n", inet_ntoa(a[i].sin_addr));
		}

		if (ntohs(answers[i].resource->type) == 5) {
			//Canonical name for an alias
			printf("has alias name : %s", answers[i].rdata);
		}

		printf("\n");
	}
	return;
}

/*
 *
 * */
u_char* ReadName(unsigned char* reader, unsigned char* buffer, int* count) {
	unsigned char *name;
	unsigned int p = 0, jumped = 0, offset;
	int i, j;

	*count = 1;
	name = (unsigned char*) malloc(256);

	name[0] = '\0';

	//read the names in 3www6google3com format
	while (*reader != 0) {
		if (*reader >= 192) {
			offset = (*reader) * 256 + *(reader + 1) - 49152; //49152 = 11000000 00000000 ;)
			reader = buffer + offset - 1;
			jumped = 1; //we have jumped to another location so counting wont go up!
		} else {
			name[p++] = *reader;
		}

		reader = reader + 1;

		if (jumped == 0) {
			*count = *count + 1; //if we havent jumped to another location then we can count up
		}
	}

	name[p] = '\0'; //string complete
	if (jumped == 1) {
		*count = *count + 1; //number of steps we actually moved forward in the packet
	}

	//now convert 3www6google3com0 to www.google.com
	for (i = 0; i < (int) strlen((const char*) name); i++) {
		p = name[i];
		for (j = 0; j < (int) p; j++) {
			name[i] = name[i + 1];
			i = i + 1;
		}
		name[i] = '.';
	}
	name[i - 1] = '\0'; //remove the last dot
	return name;
}


/*
 * Get the DNS servers from /etc/resolv.conf file on Linux
 * */
int get_dns_servers(char *dns1, char *dns2)
{
	FILE *fp;
    char line_buf[1000];
	int i = 0;
	char name[16] = "nameserver";
	char *pret;
    fp = fopen("/etc/resolv.conf", "r");
    if (NULL == fp)
    {
      	printf("fopen /etc/resolv.conf failed!\n");
        return -1;
    }
    while(1)
    {
        memset(line_buf, 0, sizeof(line_buf));

        /* read flash_map.ini  file */
        pret = fgets(line_buf, sizeof(line_buf) - 1, fp);
		if(NULL == pret)
		{
			fclose(fp);
			return -1;
		}

		if (NULL != (pret = strstr(line_buf, name)))
		{
			if(i == 0)
			{
				sscanf(line_buf, "%*[^ ] %s", dns1);
			}
			else if (i == 1)
			{
				sscanf(line_buf, "%*[^ ] %s", dns2);
			}
			else if(i > 1)
			{
				break;
			}
			i++;
		}

    }
	fclose(fp);

    return 0;
}

/*
 * This will convert www.google.com to 3www6google3com
 * got it :)
 * */
void ChangetoDnsNameFormat(unsigned char* dns, unsigned char* host) {
	int lock = 0, i;
	strcat((char*) host, ".");

	for (i = 0; i < strlen((char*) host); i++) {
		if (host[i] == '.') {
			*dns++ = i - lock;
			for (; lock < i; lock++) {
				*dns++ = host[lock];
			}
			lock++; //or lock=i+1;
		}
	}
	*dns++ = '\0';
}



void get_arpadate(char *d_string)
{
	struct tm *date = NULL;
#ifdef USE_OLD_ARPADATE
	static char *week_day[] =
	{
		"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
	};
	static char *month[] =
	{
		"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};
	static char timezone[3] = {0};

	time_t current = 0;
	int offset = 0, gmt_yday = 0, gmt_hour = 0, gmt_min = 0;

	/* Get current time */
	(void)time(&current);

	/* Get GMT and then local dates */
	date = gmtime((const time_t *)&current);
	gmt_yday = date->tm_yday;
	gmt_hour = date->tm_hour;
	gmt_min = date->tm_min;
	date = localtime((const time_t *)&current);

	/* Calculates offset */
	offset = (date->tm_hour - gmt_hour) * 60 + (date->tm_min - gmt_min);

	/* Be careful, there can be problems if the day has changed between the
	evaluation of local and gmt's one */
	if (date->tm_yday != gmt_yday)
	{
		if (date->tm_yday == (gmt_yday + 1))
			offset += 1440;
		else if (date->tm_yday == (gmt_yday - 1))
			offset -= 1440;
		else
			offset += (date->tm_yday > gmt_yday) ?  -1440: 1440;
	}

	if (offset >= 0)
		sprintf(timezone, "+%02d%02d", offset / 60, offset % 60);
	else
		sprintf(timezone, "-%02d%02d",  -offset / 60,  -offset % 60);

	sprintf(d_string, "%s, %d %s %04d %02d:%02d:%02d %s",
		week_day[date->tm_wday],
		date->tm_mday,
		month[date->tm_mon],
		date->tm_year + 1900,
		date->tm_hour,
		date->tm_min,
		date->tm_sec,
		timezone);
#else
	time_t now;

	/* RFC822 format string borrowed from GNU shellutils date.c */
	const char *format = "%a, %d %b %Y %H:%M:%S %z";

	now = time(NULL);

	date = localtime((const time_t *)&now);
	(void)strftime(d_string, ARPADATE_LENGTH, format, date);
#endif
}


/***********************************************************************************
*Function Name   :fd_getc()
*Description     :网络套接字 按字符接收 支持超时处理
*Input           :
*Output          :
*Return          :
*Auhor           :成功返回1, 失败返回-1,超时返回0
***********************************************************************************/
ssize_t fd_getc(int fd, void *c)
{
#ifdef HAVE_SSL
	if (use_tls == 1)
	{
		return (SSL_read(ssl, c, 1));
	}
#endif
	fd_set	file_set;
    int		ret;
	struct  timeval		timeout;

    while(1)
	{
		FD_ZERO(&file_set);
		FD_SET(fd,&file_set);

		timeout.tv_sec = SMTP_CONNECT_TIMEOUT;
		timeout.tv_usec = 0;
		ret = select(fd+1,&file_set,NULL,NULL,&timeout);
		if(ret < 0)
		{
				if(errno==EINTR)
				{
					printf("socket fd_getc eintr error\n");
					continue;
				}
				return(-1);
		}
		else if(ret == 0)
		{
			return(0);
		}
		else
		{
			return (read(fd, c, 1));
		}
	}
}

/*
fd_gets() -- Get characters from a fd instead of an fp
 */
char *fd_gets(char *buf, int size, int fd)
{
	int i = 0;
	char c;

	while ((i < size) && (fd_getc(fd, &c) == 1))
	{
		if (c == '\r')
		{
			;
		}
		 /* Strip <CR> */
		else if (c == '\n')
		{
			break;
		}
		else
		{
			buf[i++] = c;
		}
	}
	buf[i] = 0;

	return (buf);
}

/*
smtp_read() -- Get a line and return the initial digit
 */
int smtp_read(int fd, char *response)
{
	memset(response, 0, SMTP_BUF_SIZE);
	do
	{
		if (fd_gets(response, SMTP_BUF_SIZE, fd) == NULL)
		{
			return -1;
		}
	}
	while (response[3] == '-');

	printf("smtp S->C:\n\r%s\n",response);
	return 0;
}

/*
smtp_okay() -- Get a line and test the three-number string at the beginning
If it starts with a 2, it's OK
 */
int smtp_okay(int fd, char *response)
{
    int ret = 0;
    ret = smtp_read(fd, response);
    if (ret)
    {
        return ret;
    }

    /* check smtp server return code! */
    if (NULL != strstr(response, "553") ||
        NULL != strstr(response, "535") || 
        NULL != strstr(response, "554"))
    {
        printf("smtp_okay  response:%s error!\n", response);
        return -1;
    }

	return 0;
}

/*
fd_puts() -- Write characters to fd
 */
ssize_t fd_puts(int fd, const void *buf, size_t count)
{
#ifdef HAVE_SSL
	if (use_tls == 1)
	{
		return (SSL_write(ssl, buf, count));
	}
#endif
	int len;
	int totalSend = 0;
	char *buf_ptr = buf;
	char *buf_end = buf_ptr + count;
	int sned_count = 0;
	while (1) {
		len = send(fd, buf_ptr, count, 0);	
		if (len <= 0) {
			if (len < 0 && (errno == EINTR || errno == EAGAIN)) {
				perror("send");
				if (sned_count > 2){
					printf("Network connect timeout,please check network!\n");
					return -1;
				}
				sned_count = sned_count + 1;
				printf("send_count : %d\n",sned_count);
				usleep(10000);
				continue;
			}

			return -1;
		}
		count -= len;
		buf_ptr += len;
		totalSend +=len;
		if (buf_ptr >= buf_end) {	
			break;
		}

	}
	return totalSend;
}

/*
smtp_write() -- A printf to an fd and append <CR/LF>
 */
void smtp_write(int fd, char *format, ...)
{
	char buf[(SMTP_BUF_SIZE + 1)] = {0};
	va_list ap;

	va_start(ap, format);
	if (vsnprintf(buf, (SMTP_BUF_SIZE - 2), format, ap) ==  -1)
	{
		printf("smtp_write() -- vsnprintf() failed\n");
	}
	va_end(ap);

	(void)strcat(buf, "\r\n");

	(void)fd_puts(fd, buf, strlen(buf));
	printf("smtp C->S:\n\r%s\n",buf);
}

/***********************************************************************************
*Function Name   :smtp_open()
*Description     :链接smtp服务器
*Input           :
*Output          :
*Return          :
*Auhor           :成功返回一个已连接socket,失败返回-1,超时返回0
***********************************************************************************/
int smtp_open(char *host, unsigned short port,int timeout)
{
	static struct sockaddr_in name;
	static char host_old[32] = {0};
	int s = 0, namelen = 0;
	struct timeval tm;
	socklen_t len = sizeof(tm);
	struct DNS_IPADDR dns_addr[10] = {0};
	int ret = 0;
#ifdef HAVE_SSL
	int err = 0;
	/* Init SSL stuff */
	SSL_CTX *ctx = NULL;
	SSL_METHOD *meth = NULL;
	X509 *server_cert = NULL;
	SSL_load_error_strings();
	SSLeay_add_ssl_algorithms();
	meth = SSLv23_client_method();
	ctx = SSL_CTX_new(meth);
	if (!ctx)
	{
		return (-1);
	}
#endif
    if((name.sin_addr.s_addr <= 0)||(strncmp(host_old, host, sizeof(host_old))))
    {
		memset(host_old,0,sizeof(host_old));
    	strncpy(host_old, host, sizeof(host_old));
		printf("**********************&&&&&&&&&&&&:%lu\n", name.sin_addr.s_addr);
    	/* Check we can reach the host */

		ret = get_dns_servers(dns_servers[0],dns_servers[1]);
		if(-1 == ret){
			strcpy(dns_servers[0], "208.67.222.222");
			strcpy(dns_servers[1], "208.67.220.220");	
		}
		gethostbyname_mail(dns_addr,host,1);//1:ipv4  
		memset(&(name.sin_addr.s_addr),0,sizeof(name.sin_addr.s_addr));
    	name.sin_family = AF_INET;
    	name.sin_port = htons(port);
		name.sin_addr.s_addr = dns_addr[0].ipaddr;
	
		if(dns_addr[0].ipaddr <= 0){
			name.sin_addr.s_addr = dns_addr[1].ipaddr;
			if (dns_addr[1].ipaddr <= 0){
				name.sin_addr.s_addr = dns_addr[2].ipaddr;
				if (dns_addr[2].ipaddr <= 0){
					name.sin_addr.s_addr = dns_addr[3].ipaddr;
				}
			}
		}	
		
    }
    /* Create a socket for the connection */
    if ((s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
    	return (-1);
    }

    /*设置timeout机制*/
    tm.tv_sec = timeout;
    tm.tv_usec = 0;
    setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &tm, len);
    namelen = sizeof(struct sockaddr_in);
    if (connect(s, (struct sockaddr *)&name, namelen) < 0)
    {
    	if (errno == EINPROGRESS)
    	{
    			printf("connect smtp timeout\n");
    			close(s);
    			return 0;
    	}
    	close(s);
    	return -1;
    }

#ifdef HAVE_SSL
	if (use_tls == 1)
	{
		ssl = SSL_new(ctx);
		if (!ssl)
		{
			return (-1);
		}
		SSL_set_fd(ssl, s);
		err = SSL_connect(ssl);
		if (err < 0)
		{
			perror("SSL_connect error!\n");
			return (-1);
		}
		printf("SSL connection using %s\n", SSL_get_cipher(ssl));
		server_cert = SSL_get_peer_certificate(ssl);
		if (!server_cert)
		{
			return (-1);
		}
		X509_free(server_cert);
		/* TODO: Check server cert if changed! */
	}
#endif
	return (s);
}


/***********************************************************************************
*Function Name   :ssmtp_transmit()
*Description     :发送电子邮件
*Input           :smtpconf 用户参数配置

*Output          :err_msg 返回出错缓冲区 大小SMTP_BUF_SIZE
*Return          :
*Auhor           :成功0,失败返回-1
***********************************************************************************/
int ssmtp_transmit(SMTP_CFG *Ecfg, char *err_msg)
{
	char buf[SMTP_BUF_SIZE] = {0};
	char arpadate[ARPADATE_LENGTH] = {0};
	int s = 0;

	/*SSL传输标志*/
    use_tls = Ecfg->Usetls;

    PRINT_INFO("To do smtp_open!\n");
	get_arpadate(arpadate);
	if ((s = smtp_open(Ecfg->SmtpServer, Ecfg->SmtpPort, SMTP_CONNECT_TIMEOUT)) <= 0)
	{
		snprintf(err_msg, SMTP_ERROR_SIZE, "Cannot open %s:%d\n", Ecfg->SmtpServer, Ecfg->SmtpPort);
		return -1;
	}

	if (smtp_okay(s, buf))
	{
		snprintf(err_msg, SMTP_ERROR_SIZE, "Invalid response SMTP server\n");
		close(s);
		return -1;
	}

	/* If user supplied username and password, then try ELHO */
    PRINT_INFO("Try to ELHO !\n");
	if (0 == strlen(Ecfg->UserName))
	{
		smtp_write(s, "EHLO %s", Ecfg->FromAddr);
	}
	else
	{
		smtp_write(s, "HELO %s", Ecfg->FromAddr);
	}

	if (smtp_okay(s, buf))
	{
		snprintf(err_msg, SMTP_ERROR_SIZE, "%s (%s)\n", buf, Ecfg->FromAddr);
		close(s);
		return -1;
	}
	/* Try to log in if username was supplied */
    PRINT_INFO("Try to log in if username was supplied!\n");
	if (0 != strlen(Ecfg->UserName))
	{
		memset(buf, 0, sizeof(buf));
		to64frombits((unsigned char *)buf, (const unsigned char *)Ecfg->UserName, strlen(Ecfg->UserName));
        PRINT_INFO("To do AUTH LOGIN %s \n", buf);
		smtp_write(s, "AUTH LOGIN %s", buf);
		if (smtp_read(s, buf))
		{
			snprintf(err_msg, SMTP_ERROR_SIZE, "Server didn't accept AUTH LOGIN (%s)\n", buf);
			close(s);
			return -1;
		}
		memset(buf, 0, sizeof(buf));
		to64frombits((unsigned char *)buf, (const unsigned char *)Ecfg->PassWord, strlen(Ecfg->PassWord));
        PRINT_INFO("To do PassWord %s \n", buf);
		smtp_write(s, "%s", buf);
		if (smtp_okay(s, buf))
		{
			snprintf(err_msg, SMTP_ERROR_SIZE, "Authorization failed (%s)\n", buf);
			close(s);
			return -1;
		}
	}

	/* Start sendmail head , "MAIL FROM:" line */
    PRINT_INFO("To do MAIL FROM:<%s>\n", Ecfg->FromAddr);
	smtp_write(s, "MAIL FROM:<%s>", Ecfg->FromAddr);
	if (smtp_okay(s, buf))
	{
		snprintf(err_msg, SMTP_ERROR_SIZE, "%s\n", buf);
		close(s);
		return -1;
	}

	if(strlen(Ecfg->ToAddr0) > 0)
	{
	    PRINT_INFO("To do RCPT ToAddr0:<%s>\n", Ecfg->ToAddr0);
		smtp_write(s, "RCPT TO:<%s>", Ecfg->ToAddr0);
		if (smtp_okay(s, buf))
		{
			memset(err_msg,0,SMTP_BUF_SIZE);
			snprintf(err_msg, SMTP_ERROR_SIZE, "RCPT TO:<%s> (%s)\n", Ecfg->ToAddr0, buf);
			//(void)close(s);
			//return(-1);
		}
	}
	if(strlen(Ecfg->ToAddr1) > 0)
	{
	    PRINT_INFO("To do RCPT ToAddr1:<%s>\n", Ecfg->ToAddr1);
		smtp_write(s, "RCPT TO:<%s>", Ecfg->ToAddr1);
		if (smtp_okay(s, buf))
		{
			memset(err_msg,0,SMTP_BUF_SIZE);
			snprintf(err_msg, SMTP_ERROR_SIZE, "RCPT TO:<%s> (%s)\n", Ecfg->ToAddr1, buf);
			//(void)close(s);
			//return(-1);
		}
	}
	if(strlen(Ecfg->ToAddr2) > 0)
	{
	    PRINT_INFO("To do RCPT ToAddr2:<%s>\n", Ecfg->ToAddr2);
		smtp_write(s, "RCPT TO:<%s>", Ecfg->ToAddr2);
		if (smtp_okay(s, buf))
		{
			memset(err_msg,0,SMTP_BUF_SIZE);
			snprintf(err_msg, SMTP_ERROR_SIZE, "RCPT TO:<%s> (%s)\n", Ecfg->ToAddr2, buf);
			//(void)close(s);
			//return(-1);
		}
	}
	if(strlen(Ecfg->ToAddr3) > 0)
	{
	    PRINT_INFO("To do RCPT ToAddr3:<%s>\n", Ecfg->ToAddr3);
		smtp_write(s, "RCPT TO:<%s>", Ecfg->ToAddr3);
		if (smtp_okay(s, buf))
		{
			memset(err_msg,0,SMTP_BUF_SIZE);
			snprintf(err_msg, SMTP_ERROR_SIZE, "RCPT TO:<%s> (%s)\n", Ecfg->ToAddr3, buf);
			//(void)close(s);
			//return(-1);
		}
	}
	if(strlen(Ecfg->CCToAddr0) > 0)
	{
	    PRINT_INFO("To do RCPT CCToAddr0:<%s>\n", Ecfg->CCToAddr0);
		smtp_write(s, "RCPT TO:<%s>", Ecfg->CCToAddr0);
		if (smtp_okay(s, buf))
		{
			memset(err_msg,0,SMTP_BUF_SIZE);
			snprintf(err_msg, SMTP_ERROR_SIZE, "RCPT TO:<%s> (%s)\n", Ecfg->CCToAddr0, buf);
			//(void)close(s);
			//return(-1);
		}
	}

	if(strlen(Ecfg->CCToAddr1) > 0)
	{
	    PRINT_INFO("To do RCPT CCToAddr1:<%s>\n", Ecfg->CCToAddr1);
		smtp_write(s, "RCPT TO:<%s>", Ecfg->CCToAddr1);
		if (smtp_okay(s, buf))
		{
			memset(err_msg,0,SMTP_BUF_SIZE);
			snprintf(err_msg, SMTP_ERROR_SIZE, "RCPT TO:<%s> (%s)\n", Ecfg->CCToAddr1, buf);
			//(void)close(s);
			//return(-1);
		}
	}
	if(strlen(Ecfg->CCToAddr2) > 0)
	{
	    PRINT_INFO("To do RCPT CCToAddr2:<%s>\n", Ecfg->CCToAddr2);
		smtp_write(s, "RCPT TO:<%s>", Ecfg->CCToAddr2);
		if (smtp_okay(s, buf))
		{
			memset(err_msg,0,SMTP_BUF_SIZE);
			snprintf(err_msg, SMTP_ERROR_SIZE, "RCPT TO:<%s> (%s)\n", Ecfg->CCToAddr2, buf);
			//(void)close(s);
			//return(-1);
		}
	}
	if(strlen(Ecfg->CCToAddr3) > 0)
	{
	    PRINT_INFO("To do RCPT CCToAddr3:<%s>\n", Ecfg->CCToAddr3);
		smtp_write(s, "RCPT TO:<%s>", Ecfg->CCToAddr3);
		if (smtp_okay(s, buf))
		{
			memset(err_msg,0,SMTP_BUF_SIZE);
			snprintf(err_msg, SMTP_ERROR_SIZE, "RCPT TO:<%s> (%s)\n", Ecfg->CCToAddr3, buf);
			//(void)close(s);
			//return(-1);
		}
	}
	if(strlen(Ecfg->BCCToAddr0) > 0)
	{
	    PRINT_INFO("To do RCPT BCCToAddr0:<%s>\n", Ecfg->BCCToAddr0);
		smtp_write(s, "RCPT TO:<%s>", Ecfg->BCCToAddr0);
		if (smtp_okay(s, buf))
		{
			memset(err_msg,0,SMTP_BUF_SIZE);
			snprintf(err_msg, SMTP_ERROR_SIZE, "RCPT TO:<%s> (%s)\n", Ecfg->BCCToAddr0, buf);
			//(void)close(s);
			//return(-1);
		}
	}

	if(strlen(Ecfg->BCCToAddr1) > 0)
	{
	    PRINT_INFO("To do RCPT BCCToAddr1:<%s>\n", Ecfg->BCCToAddr1);
		smtp_write(s, "RCPT TO:<%s>", Ecfg->BCCToAddr1);
		if (smtp_okay(s, buf))
		{
			memset(err_msg,0,SMTP_BUF_SIZE);
			snprintf(err_msg, SMTP_ERROR_SIZE, "RCPT TO:<%s> (%s)\n", Ecfg->BCCToAddr1, buf);
			//(void)close(s);
			//return(-1);
		}
	}

	if(strlen(Ecfg->BCCToAddr2) > 0)
	{
	    PRINT_INFO("To do RCPT BCCToAddr2:<%s>\n", Ecfg->BCCToAddr2);
		smtp_write(s, "RCPT TO:<%s>", Ecfg->BCCToAddr2);
		if (smtp_okay(s, buf))
		{
			memset(err_msg,0,SMTP_BUF_SIZE);
			snprintf(err_msg, SMTP_ERROR_SIZE, "RCPT TO:<%s> (%s)\n", Ecfg->BCCToAddr2, buf);
			//(void)close(s);
			//return(-1);
		}
	}

	if(strlen(Ecfg->BCCToAddr3) > 0)
	{
	    PRINT_INFO("To do RCPT BCCToAddr3:<%s>\n", Ecfg->BCCToAddr3);
		smtp_write(s, "RCPT TO:<%s>", Ecfg->BCCToAddr3);
		if (smtp_okay(s, buf))
		{
			memset(err_msg,0,SMTP_BUF_SIZE);
			snprintf(err_msg, SMTP_ERROR_SIZE, "RCPT TO:<%s> (%s)\n", Ecfg->BCCToAddr3, buf);
			//(void)close(s);
			//return(-1);
		}
	}
	/* Send DATA */
    PRINT_INFO("To start DATA code\n");
	smtp_write(s, "DATA");
	if (smtp_read(s, buf))
	{
		snprintf(err_msg, SMTP_ERROR_SIZE, "%s\n", buf);
		close(s);
		return -1;
	}

    if (0 == Ecfg->Subject[0])
    {
        strncpy(Ecfg->Subject, SMTP_SUBJECT, sizeof(Ecfg->Subject) - 1);
    }

    if (0 == Ecfg->TextBody[0])
    {
        strncpy(Ecfg->TextBody, SMTP_TEXTBODY, sizeof(Ecfg->TextBody) - 1);
    }
    PRINT_INFO("To send DATA ......\n");
	smtp_write(s, "From: %s\r\n"
			"To: %s;%s;%s;%s\r\n"
			"CC: %s;%s;%s;%s\r\n"
			"BCC: %s;%s;%s;%s\r\n"
			"Subject: %s\r\n"
			"Date: %s\r\n"
			"MIME-Version: 1.0\r\n"
			"Content-Type: multipart/mixed;\r\n"
			"\tboundary=\"Camera_Stmp\"\r\n\r\n"
			"X-Priority: 0\r\n"
			"X-MSMail-Priority: Normal\r\n"
			"X-Mailer: Microsoft Outlook Express 6.00.2800.1437\r\n"
			"X-MimeOLE: Produced By Microsoft MimeOLE V6.00.2800.1441\r\n"
			"This is a multi-part message in MIME format.\r\n\r\n"
			"--Camera_Stmp\r\n"		// boundary
			"Content-Type: text/plain;\r\n"
			"Content-Transfer-Encoding: quoted-printable\r\n\r\n"
			"%s\r\n\r\n",								//content
			Ecfg->FromAddr,
			Ecfg->ToAddr0,   Ecfg->ToAddr1,   Ecfg->ToAddr2,   Ecfg->ToAddr3,
			Ecfg->CCToAddr0, Ecfg->CCToAddr1, Ecfg->CCToAddr2, Ecfg->CCToAddr3,
			Ecfg->BCCToAddr0,Ecfg->BCCToAddr1,Ecfg->BCCToAddr2,Ecfg->BCCToAddr3,
			Ecfg->Subject, arpadate, Ecfg->TextBody);

    PRINT_INFO("To send DATA end CODE .\n");
	smtp_write(s, ".");
	if (smtp_okay(s, buf))
	{
		snprintf(err_msg, SMTP_ERROR_SIZE, "%s\n", buf);
		close(s);
		return -1;
	}

	/* Close connection */
    PRINT_INFO("To send QUIT CODE .\n");
	smtp_write(s, "QUIT");
	smtp_okay(s, buf);
	close(s);

	return 0;
}

/***********************************************************************************
*Function Name   :ssmtp()
*Description     :发送电子邮件
*Input           :smtpconf 用户参数配置, pic_list 图片链表

*Output          :err_msg 返回出错缓冲区
*Return          :成功1,失败返回-1
*Auhor           :liaoshiliang xxr
***********************************************************************************/
int ssmtp(SMTP_CFG *Ecfg, struct list_head *pic_list, char *err_msg)
{
	char buf[SMTP_BUF_SIZE] = {0};
	char arpadate[ARPADATE_LENGTH] = {0};
	struct list_head *pos;
    JPEG_PICTURE *jpeg;
	int s = 0;

	/*SSL传输标志*/
    use_tls = Ecfg->Usetls;

    PRINT_INFO("To do smtp_open!\n");
	get_arpadate(arpadate);
	if ((s = smtp_open(Ecfg->SmtpServer, Ecfg->SmtpPort, SMTP_CONNECT_TIMEOUT)) <= 0)
	{
		snprintf(err_msg, SMTP_ERROR_SIZE, "Cannot open %s:%d\n", Ecfg->SmtpServer, Ecfg->SmtpPort);
		printf("Cannot open %s:%d\n", Ecfg->SmtpServer, Ecfg->SmtpPort);
		return (-1);
	}
	if (smtp_okay(s, buf))
	{
		snprintf(err_msg, SMTP_ERROR_SIZE, "Invalid response SMTP server\n");
		(void)close(s);
		return (-1);
	}

	/* If user supplied username and password, then try ELHO */
    PRINT_INFO("try ELHO.  If user supplied username and password!\n");
	if (0 == strlen(Ecfg->UserName))
	{
		smtp_write(s, "EHLO %s", Ecfg->FromAddr);
	}
	else
	{
		smtp_write(s, "HELO %s", Ecfg->FromAddr);
	}

	if (smtp_okay(s, buf))
	{
		snprintf(err_msg, SMTP_ERROR_SIZE, "%s (%s)\n", buf, Ecfg->FromAddr);
		(void)close(s);
		return(-1);
	}

	/* Try to log in if username was supplied */
    PRINT_INFO("Try to log in if username was supplied!\n");
	if (0 != strlen(Ecfg->UserName))
	{
		memset(buf, 0, sizeof(buf));
		to64frombits((unsigned char *)buf, (unsigned char *)Ecfg->UserName, strlen(Ecfg->UserName));
        PRINT_INFO("To do AUTH LOGIN:<%s>\n", buf);
		smtp_write(s, "AUTH LOGIN %s", buf);
		if (smtp_read(s, buf))
		{
			snprintf(err_msg, SMTP_ERROR_SIZE, "Server didn't accept AUTH LOGIN (%s)\n", buf);
			(void)close(s);
			return(-1);
		}
		memset(buf, 0, sizeof(buf));
		to64frombits((unsigned char *)buf, (unsigned char *)Ecfg->PassWord, strlen(Ecfg->PassWord));
        PRINT_INFO("To do PassWord:<%s>\n", buf);
		smtp_write(s, "%s", buf);
		if (smtp_okay(s, buf))
		{
			snprintf(err_msg, SMTP_ERROR_SIZE, "Authorization failed (%s)\n", buf);
			(void)close(s);
			return(-1);
		}
	}

	/* Start sendmail head , "MAIL FROM:" line */
    PRINT_INFO("To do MAIL FROM:<%s>\n", Ecfg->FromAddr);
	smtp_write(s, "MAIL FROM:<%s>", Ecfg->FromAddr);
	if (smtp_okay(s, buf))
	{
		snprintf(err_msg, SMTP_ERROR_SIZE, "%s\n", buf);
		(void)close(s);
		return(-1);
	}

	if(strlen(Ecfg->ToAddr0) > 0)
	{
	    PRINT_INFO("RCPT TO ToAddr0:<%s>\n", Ecfg->ToAddr0);
		smtp_write(s, "RCPT TO:<%s>", Ecfg->ToAddr0);
		if (smtp_okay(s, buf))
		{
			memset(err_msg,0,SMTP_BUF_SIZE);
			snprintf(err_msg, SMTP_ERROR_SIZE, "RCPT TO:<%s> (%s)\n", Ecfg->ToAddr0, buf);
			//(void)close(s);
			//return(-1);
		}
	}
	if(strlen(Ecfg->ToAddr1) > 0)
	{
	    PRINT_INFO("RCPT TO ToAddr1:<%s>\n", Ecfg->ToAddr1);
		smtp_write(s, "RCPT TO:<%s>", Ecfg->ToAddr1);
		if (smtp_okay(s, buf))
		{
			memset(err_msg,0,SMTP_BUF_SIZE);
			snprintf(err_msg, SMTP_ERROR_SIZE, "RCPT TO:<%s> (%s)\n", Ecfg->ToAddr1, buf);
			//(void)close(s);
			//return(-1);
		}
	}
	if(strlen(Ecfg->ToAddr2) > 0)
	{
	    PRINT_INFO("RCPT TO ToAddr2:<%s>\n", Ecfg->ToAddr2);
		smtp_write(s, "RCPT TO:<%s>", Ecfg->ToAddr2);
		if (smtp_okay(s, buf))
		{
			memset(err_msg,0,SMTP_BUF_SIZE);
			snprintf(err_msg, SMTP_ERROR_SIZE, "RCPT TO:<%s> (%s)\n", Ecfg->ToAddr2, buf);
			//(void)close(s);
			//return(-1);
		}
	}

	if(strlen(Ecfg->ToAddr3) > 0)
	{
	    PRINT_INFO("RCPT TO ToAddr3:<%s>\n", Ecfg->ToAddr3);
		smtp_write(s, "RCPT TO:<%s>", Ecfg->ToAddr3);
		if (smtp_okay(s, buf))
		{
			memset(err_msg,0,SMTP_BUF_SIZE);
			snprintf(err_msg, SMTP_ERROR_SIZE, "RCPT TO:<%s> (%s)\n", Ecfg->ToAddr3, buf);
			//(void)close(s);
			//return(-1);
		}
	}

	if(strlen(Ecfg->CCToAddr0) > 0)
	{
	    PRINT_INFO("RCPT TO CCToAddr0:<%s>\n", Ecfg->CCToAddr0);
		smtp_write(s, "RCPT TO:<%s>", Ecfg->CCToAddr0);
		if (smtp_okay(s, buf))
		{
			memset(err_msg,0,SMTP_BUF_SIZE);
			snprintf(err_msg, SMTP_ERROR_SIZE, "RCPT TO:<%s> (%s)\n", Ecfg->CCToAddr0, buf);
			//(void)close(s);
			//return(-1);
		}
	}
	if(strlen(Ecfg->CCToAddr1) > 0)
	{
	    PRINT_INFO("RCPT TO CCToAddr1:<%s>\n", Ecfg->CCToAddr1);
		smtp_write(s, "RCPT TO:<%s>", Ecfg->CCToAddr1);
		if (smtp_okay(s, buf))
		{
			memset(err_msg,0,SMTP_BUF_SIZE);
			snprintf(err_msg, SMTP_ERROR_SIZE, "RCPT TO:<%s> (%s)\n", Ecfg->CCToAddr1, buf);
			//(void)close(s);
			//return(-1);
		}
	}
	if(strlen(Ecfg->CCToAddr2) > 0)
	{
	    PRINT_INFO("RCPT TO CCToAddr2:<%s>\n", Ecfg->CCToAddr2);
		smtp_write(s, "RCPT TO:<%s>", Ecfg->CCToAddr2);
		if (smtp_okay(s, buf))
		{
			memset(err_msg,0,SMTP_BUF_SIZE);
			snprintf(err_msg, SMTP_ERROR_SIZE, "RCPT TO:<%s> (%s)\n", Ecfg->CCToAddr2, buf);
			//(void)close(s);
			//return(-1);
		}
	}

	if(strlen(Ecfg->CCToAddr3) > 0)
	{
	    PRINT_INFO("RCPT TO CCToAddr3:<%s>\n", Ecfg->CCToAddr3);
		smtp_write(s, "RCPT TO:<%s>", Ecfg->CCToAddr3);
		if (smtp_okay(s, buf))
		{
			memset(err_msg,0,SMTP_BUF_SIZE);
			snprintf(err_msg, SMTP_ERROR_SIZE, "RCPT TO:<%s> (%s)\n", Ecfg->CCToAddr3, buf);
			//(void)close(s);
			//return(-1);
		}
	}
	if(strlen(Ecfg->BCCToAddr0) > 0)
	{
	    PRINT_INFO("RCPT TO BCCToAddr0:<%s>\n", Ecfg->BCCToAddr0);
		smtp_write(s, "RCPT TO:<%s>", Ecfg->BCCToAddr0);
		if (smtp_okay(s, buf))
		{
			memset(err_msg,0,SMTP_BUF_SIZE);
			snprintf(err_msg, SMTP_ERROR_SIZE, "RCPT TO:<%s> (%s)\n", Ecfg->BCCToAddr0, buf);
			//(void)close(s);
			//return(-1);
		}
	}
	if(strlen(Ecfg->BCCToAddr1) > 0)
	{
	    PRINT_INFO("RCPT TO BCCToAddr1:<%s>\n", Ecfg->BCCToAddr1);
		smtp_write(s, "RCPT TO:<%s>", Ecfg->BCCToAddr1);
		if (smtp_okay(s, buf))
		{
			memset(err_msg,0,SMTP_BUF_SIZE);
			snprintf(err_msg, SMTP_ERROR_SIZE, "RCPT TO:<%s> (%s)\n", Ecfg->BCCToAddr1, buf);
			//(void)close(s);
			//return(-1);
		}
	}
	if(strlen(Ecfg->BCCToAddr2) > 0)
	{
	    PRINT_INFO("RCPT TO BCCToAddr2:<%s>\n", Ecfg->BCCToAddr2);
		smtp_write(s, "RCPT TO:<%s>", Ecfg->BCCToAddr2);
		if (smtp_okay(s, buf))
		{
			memset(err_msg,0,SMTP_BUF_SIZE);
			snprintf(err_msg, SMTP_ERROR_SIZE, "RCPT TO:<%s> (%s)\n", Ecfg->BCCToAddr2, buf);
			//(void)close(s);
			//return(-1);
		}
	}
	if(strlen(Ecfg->BCCToAddr3) > 0)
	{
	    PRINT_INFO("RCPT TO BCCToAddr3:<%s>\n", Ecfg->BCCToAddr3);
		smtp_write(s, "RCPT TO:<%s>", Ecfg->BCCToAddr3);
		if (smtp_okay(s, buf))
		{
			memset(err_msg,0,SMTP_BUF_SIZE);
			snprintf(err_msg, SMTP_ERROR_SIZE, "RCPT TO:<%s> (%s)\n", Ecfg->BCCToAddr3, buf);
			//(void)close(s);
			//return(-1);
		}
	}
	/* Send DATA */
	PRINT_INFO("Start to send DATA\n");
	smtp_write(s, "DATA");
	if (smtp_read(s, buf))
	{
		snprintf(err_msg, SMTP_ERROR_SIZE, "%s\n", buf);
		(void)close(s);
		return(-1);
	}
	if(NULL != pic_list)
	{
	    PRINT_INFO("Send DATA and attachPicture ......\n");
		smtp_write(s, "From: %s\r\n"
			"To: %s;%s;%s;%s\r\n"
			"CC: %s;%s;%s;%s\r\n"
			"BCC: %s;%s;%s;%s\r\n"
			"Subject: %s Ipcamera\r\n"
			"Date: %s\r\n"
			"MIME-Version: 1.0\r\n"
			"Content-Type: multipart/mixed;\r\n"
			"\tboundary=\"=====IPCamera_Stmp=====\"\r\n\r\n"
			"X-Priority: 0\r\n"
			"X-MSMail-Priority: Normal\r\n"
			"X-Mailer: Microsoft Outlook Express 6.00.2800.1437\r\n"
			"X-MimeOLE: Produced By Microsoft MimeOLE V6.00.2800.1441\r\n"
			"This is a multi-part message in MIME format.\r\n\r\n"
			"--=====IPCamera_Stmp=====\r\n"		// boundary
			"Content-Type: text/plain;charset=\"UTF-8\"\r\n"
			"Content-Transfer-Encoding: quoted-printable\r\n\r\n"
			"%s\r\n\r\n",								//content
			Ecfg->FromAddr, Ecfg->ToAddr0,Ecfg->ToAddr1,Ecfg->ToAddr2,Ecfg->ToAddr3, Ecfg->CCToAddr0,Ecfg->CCToAddr1,Ecfg->CCToAddr2,Ecfg->CCToAddr3,Ecfg->BCCToAddr0,Ecfg->BCCToAddr1,Ecfg->BCCToAddr2,Ecfg->BCCToAddr3,SMTP_SUBJECT, arpadate, SMTP_TEXTBODY);

		/*循环jpeg链表 email*/
		list_for_each(pos, pic_list)
		{
			jpeg = list_entry(pos, JPEG_PICTURE, jpeg_list);
			if((jpeg->length > 0)&&(NULL != jpeg->buf)&&(NULL != jpeg->file_name))
			{
				smtp_write(s, "--=====IPCamera_Stmp=====\r\n"		// boundary
							"Content-Type:image/jpeg; name=\"%s\"\r\n"
							"Content-Transfer-Encoding: base64\r\n"
							"Content-Disposition: attachment;\r\n"// filename=\"%s\"\r\n\r\n",
							"\tfilename=\"%s\"\r\n\r\n",				// attach_file_name
							jpeg->file_name,jpeg->file_name);

				/*传输附件内容*/
				int ret_send = 0 ;
				ret_send = send_to64frombits(s, (unsigned char *)jpeg->buf, jpeg->length);
				if (ret_send != 0){
					printf("fail to send mail\n");
					(void)close(s);
					return(-1);
				
				}
			}
		}
	}
	else
	{
	    PRINT_INFO("Send DATA ......\n");
		smtp_write(s, "From: %s\r\n"
				"To: %s;%s;%s;%s\r\n"
				"CC: %s;%s;%s;%s\r\n"
				"BCC: %s;%s;%s;%s\r\n"
				"Subject: %s\r\n"
				"Date: %s\r\n"
				"MIME-Version: 1.0\r\n"
				"Content-Type: multipart/mixed;\r\n"
				"\tboundary=\"IPCamera_Stmp\"\r\n\r\n"
				"X-Priority: 0\r\n"
				"X-MSMail-Priority: Normal\r\n"
				"X-Mailer: Microsoft Outlook Express 6.00.2800.1437\r\n"
				"X-MimeOLE: Produced By Microsoft MimeOLE V6.00.2800.1441\r\n"
				"This is a multi-part message in MIME format.\r\n\r\n"
				"--IPCamera_Stmp\r\n"		// boundary
				"Content-Type: text/plain;\r\n"
				"Content-Transfer-Encoding: quoted-printable\r\n\r\n"
				"%s\r\n\r\n",								//content
				Ecfg->FromAddr, Ecfg->ToAddr0,Ecfg->ToAddr1,Ecfg->ToAddr2,Ecfg->ToAddr3, Ecfg->CCToAddr0,Ecfg->CCToAddr1,Ecfg->CCToAddr2,Ecfg->CCToAddr3,Ecfg->BCCToAddr0,Ecfg->BCCToAddr1,Ecfg->BCCToAddr2,Ecfg->BCCToAddr3,SMTP_SUBJECT, arpadate, SMTP_TEXTBODY);
	}
	smtp_write(s, ".");
	if (smtp_okay(s, buf))
	{
		snprintf(err_msg, SMTP_ERROR_SIZE, "%s\n", buf);
		(void)close(s);
		return(-1);
	}

	/* Close connection */
    PRINT_INFO("Send QUIT CODE.......\n");
	smtp_write(s, "QUIT");
	(void)smtp_okay(s, buf);
	(void)close(s);

	return(1);
}




