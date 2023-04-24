/* J. David's webserver */


/* This is a simple webserver.
 * Created November 1999 by J. David Blackstone.
 * CSE 4344 (Network concepts), Prof. Zeigler
 * University of Texas at Arlington
 */


/* This program compiles for Sparc Solaris 2.6.
 * To compile for Linux:
 *  1) Comment out the #include <pthread.h> line.
 *  2) Comment out the line that defines the variable newthread.
 *  3) Comment out the two lines that run pthread_create().
 *  4) Uncomment the line that runs accept_request().
 *  5) Remove -lsocket from the Makefile.
 */
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "httpd.h"

/* 宏定义，是否是空格 */
#define ISspace( x ) isspace( (int) (x) )

#define SERVER_STRING "Server: jdbhttpd/0.1.0\r\n"

static HTTPD_CallBack data_recv_callback;

/*
 * 每次收到请求，创建一个线程来处理接受到的请求
 * 把client_sock转成地址作为参数传入pthread_create
 */
void accept_request( void *arg );


/* 错误请求 */
void bad_request( int );


/* 读取文件 */
void cat( int, FILE * );


/* 无法执行 */
void cannot_execute( int );


/* 错误输出 */
void error_die( const char * );


/* 执行cig脚本 */
void execute_cgi( int, const char *, const char *, const char * );


/*
 * 得到一行数据,只要发现c为\n,就认为是一行结束，如果读到\r,再用MSG_PEEK的方式读入一个字符，如果是\n，从socket用读出
 * 如果是下个字符则不处理，将c置为\n，结束。如果读到的数据为0中断，或者小于0，也视为结束，c置为\n
 */
int get_line( int, char *, int );


/* 返回http头 */
void headers( int, const char * );


/* 没有发现文件 */
void not_found( int );


/* 如果不是CGI文件，直接读取文件返回给请求的http客户端 */
void serve_file( int, const char * );


/* 开启tcp连接，绑定端口等操作 */
int startup( int * );


/* 如果不是Get或者Post，就报方法没有实现 */
void unimplemented( int );


/*
 * Http请求，后续主要是处理这个头
 *
 * GET / HTTP/1.1
 * Host: 192.168.0.23:47310
 * Connection: keep-alive
 * Upgrade-Insecure-Requests: 1
 * User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/55.0.2883.87 Safari/537.36
 * Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,* / *; q = 0.8
 * Accept - Encoding: gzip, deflate, sdch
 * Accept - Language : zh - CN, zh; q = 0.8
 * Cookie: __guid = 179317988.1576506943281708800.1510107225903.8862; monitor_count = 5
 *
 */

/*
 * POST / color1.cgi HTTP / 1.1
 * Host: 192.168.0.23 : 47310
 * Connection : keep - alive
 * Content - Length : 10
 * Cache - Control : max - age = 0
 * Origin : http ://192.168.0.23:40786
 * Upgrade - Insecure - Requests : 1
 * User - Agent : Mozilla / 5.0 (Windows NT 6.1; WOW64) AppleWebKit / 537.36 (KHTML, like Gecko) Chrome / 55.0.2883.87 Safari / 537.36
 * Content - Type : application / x - www - form - urlencoded
 * Accept : text / html, application / xhtml + xml, application / xml; q = 0.9, image / webp, * / *;q=0.8
 * Referer: http://192.168.0.23:47310/
 * Accept-Encoding: gzip, deflate
 * Accept-Language: zh-CN,zh;q=0.8
 * Cookie: __guid=179317988.1576506943281708800.1510107225903.8862; monitor_count=281
 * Form Data
 * color=gray
 */

/**********************************************************************/


/* A request has caused a call to accept() on the server port to
 * return.  Process the request appropriately.
 * Parameters: the socket connected to the client */
/**********************************************************************/
void accept_request( void *arg )
{
    /* socket */
    int     client = (intptr_t) arg;
    char        buf[1024];
    int     numchars;
    char        method[255];
    char        url[255];
    char        path[512];
    size_t      i, j;
    struct stat st;
    int     cgi = 0; /* becomes true if server decides this is a CGI
                                  * program */
    char *query_string  = NULL;

    pthread_detach(pthread_self());
    /*
     * 根据上面的Get请求，可以看到这边就是取第一行
     * 这边都是在处理第一条http信息
     * "GET / HTTP/1.1\n"
     */
    numchars    = get_line( client, buf, sizeof(buf) );
    i       = 0; j = 0;

    /* 第一行字符串提取Get */
    while ( !ISspace( buf[j] ) && (i < sizeof(method) - 1) )
    {
        method[i] = buf[j];
        i++; j++;
    }
    /* 结束 */
    method[i] = '\0';

    /* 判断是Get还是Post */
    if ( strcasecmp( method, "GET" ) && strcasecmp( method, "POST" ) )
    {
        unimplemented( client );
        return;
    }

    //printf("method:%s\n", method);
    /* 如果是POST，cgi置为1 */
    if ( strcasecmp( method, "POST" ) == 0 )
        cgi = 1;

    i = 0;
    /* 跳过空格 */
    while ( ISspace( buf[j] ) && (j < sizeof(buf) ) )
        j++;

    /*
     * 得到 "/"   注意：如果你的http的网址为http://192.168.0.23:47310/index.html
     *               那么你得到的第一条http信息为GET /index.html HTTP/1.1，那么
     *               解析得到的就是/index.html
     */
    while ( !ISspace( buf[j] ) && (i < sizeof(url) - 1) && (j < sizeof(buf) ) )
    {
        url[i] = buf[j];
        i++; j++;
    }
    url[i] = '\0';

    /* 判断Get请求 */
    if ( strcasecmp( method, "GET" ) == 0 )
    {
        query_string = url;
        while ( (*query_string != '?') && (*query_string != '\0') )
            query_string++;
        if ( *query_string == '?' )
        {
            printf("enter ?\n");
            cgi     = 1;
            *query_string   = '\0';
            query_string++;
        }
    }

    /* 路径 */
    sprintf( path, "%s", url );

    execute_cgi( client, path, method, query_string );
    /* 执行完毕关闭socket */
    close( client );
}


/**********************************************************************/


/* Inform the client that a request it has made has a problem.
 * Parameters: client socket */
/**********************************************************************/
void bad_request( int client )
{
    char buf[1024];

    sprintf( buf, "HTTP/1.0 400 BAD REQUEST\r\n" );
    send( client, buf, sizeof(buf), 0 );
    sprintf( buf, "Content-type: text/html\r\n" );
    send( client, buf, sizeof(buf), 0 );
    sprintf( buf, "\r\n" );
    send( client, buf, sizeof(buf), 0 );
    sprintf( buf, "<P>Your browser sent a bad request, " );
    send( client, buf, sizeof(buf), 0 );
    sprintf( buf, "such as a POST without a Content-Length.\r\n" );
    send( client, buf, sizeof(buf), 0 );
}


/**********************************************************************/


/* Put the entire contents of a file out on a socket.  This function
 * is named after the UNIX "cat" command, because it might have been
 * easier just to do something like pipe, fork, and exec("cat").
 * Parameters: the client socket descriptor
 *             FILE pointer for the file to cat */
/**********************************************************************/

/* 得到文件内容，发送 */
void cat( int client, FILE *resource )
{
    char buf[1024];

    fgets( buf, sizeof(buf), resource );
    /* 循环读 */
    while ( !feof( resource ) )
    {
        send( client, buf, strlen( buf ), 0 );
        fgets( buf, sizeof(buf), resource );
    }
}


/**********************************************************************/


/* Inform the client that a CGI script could not be executed.
 * Parameter: the client socket descriptor. */
/**********************************************************************/
void cannot_execute( int client )
{
    char buf[1024];

    sprintf( buf, "HTTP/1.0 500 Internal Server Error\r\n" );
    send( client, buf, strlen( buf ), 0 );
    sprintf( buf, "Content-type: text/html\r\n" );
    send( client, buf, strlen( buf ), 0 );
    sprintf( buf, "\r\n" );
    send( client, buf, strlen( buf ), 0 );
    sprintf( buf, "<P>Error prohibited CGI execution.\r\n" );
    send( client, buf, strlen( buf ), 0 );
}


/**********************************************************************/


/* Print out an error message with perror() (for system errors; based
 * on value of errno, which indicates system call errors) and exit the
 * program indicating an error. */
/**********************************************************************/
void error_die( const char *sc )
{
    perror( sc );
    exit( 1 );
}

int indexImg = 0;
/**********************************************************************/


/* Execute a CGI script.  Will need to set environment variables as
 * appropriate.
 * Parameters: client socket descriptor
 *             path to the CGI script */
/**********************************************************************/
void execute_cgi( int client, const char *path,
          const char *method, const char *query_string )
{
/* 缓冲区 */
    char buf[1024] = {0};
    char *recvData = NULL;
    FILE *img = NULL;
    char *tmpBuff = NULL;
    char imgName[128] = {0};
    int len = 0;
    int receiveNeed;

    /* 2根管道 */
    int cgi_output[2];
    int cgi_input[2];

    /* 进程pid和状态 */
    pid_t   pid;
    int status;

    int i;
    char    c;

    /* 读取的字符数 */
    int numchars = 1;

    /* http的content_length */
    int content_length = -1;

    /* 默认字符 */
    buf[0] = 'A'; buf[1] = '\0';


    /* 忽略大小写比较字符串 */
    if ( strcasecmp( method, "GET" ) == 0 )
        /* 读取数据，把整个header都读掉，以为Get写死了直接读取index.html，没有必要分析余下的http信息了 */
        while ( (numchars > 0) && strcmp( "\n", buf ) ) /* read & discard headers */
            numchars = get_line( client, buf, sizeof(buf) );
    else{                                                   /* POST */
        numchars = get_line( client, buf, sizeof(buf) );
        while ( (numchars > 0) && strcmp( "\n", buf ) )
        {
            /*
             * 如果是POST请求，就需要得到Content-Length，Content-Length：这个字符串一共长为15位，所以
             * 取出头部一句后，将第16位设置结束符，进行比较
             * 第16位置为结束
             */
            //buf[15] = '\0';
            if ( strstr( buf, "Content-Length:" ) != NULL )
            {
                /* 内存从第17位开始就是长度，将17位开始的所有字符串转成整数就是content_length */
                //content_length = atoi( &(buf[16]) );
                sscanf(buf, "Content-Length:%d\r\n", &content_length);
            }
            numchars = get_line( client, buf, sizeof(buf) );
        }
        if ( content_length == -1 )
        {
            bad_request( client );
            return;
        }
    }

    if (content_length > 0)
    {
        recvData = (char*)malloc(content_length);
        tmpBuff = recvData;
        receiveNeed = content_length;
        while(receiveNeed > 0)
        {
            len = recv(client, tmpBuff, receiveNeed,0);
            //printf("try to recv:%d, ret:%d\n", receive_len, len);

            if (len == 0)
            {
                //printf("len 0, error:%s\n", strerror(errno));
                break;
            }

            if (len <= receiveNeed)
            {
                tmpBuff += len;
                receiveNeed -= len;
            }
        }

        if (receiveNeed == 0)
        {
            /*
            sprintf(imgName, "get_%02d.txt", indexImg++);
            //printf("save file:%s, len:%d\n", imgName, content_length);
            img = fopen(imgName, "w+");
            if (img != NULL)
            {
                fwrite(recvData, 1, content_length, img);
                fclose(img);
            }
            */
        }
        else
        {
            printf("return len:%d, recv:%d\n", content_length, content_length - receiveNeed);
        }
    }

    sprintf( buf, "HTTP/1.0 200 OK\r\nContent-type:text/html; charset=\"UTF-8\"\r\nContent-length:%d\r\n\r\n", 0);
    send( client, buf, strlen( buf ), 0 );
    if (recvData != NULL)
    {
        if (data_recv_callback != NULL)
            data_recv_callback(recvData, content_length - receiveNeed, (char*)path);
        free(recvData);
        recvData = NULL;
    }
}


/**********************************************************************/


/* Get a line from a socket, whether the line ends in a newline,
 * carriage return, or a CRLF combination.  Terminates the string read
 * with a null character.  If no newline indicator is found before the
 * end of the buffer, the string is terminated with a null.  If any of
 * the above three line terminators is read, the last character of the
 * string will be a linefeed and the string will be terminated with a
 * null character.
 * Parameters: the socket descriptor
 *             the buffer to save the data in
 *             the size of the buffer
 * Returns: the number of bytes stored (excluding null) */
/**********************************************************************/

/*
 * 得到一行数据,只要发现c为\n,就认为是一行结束，如果读到\r,再用MSG_PEEK的方式读入一个字符，如果是\n，从socket用读出
 * 如果是下个字符则不处理，将c置为\n，结束。如果读到的数据为0中断，或者小于0，也视为结束，c置为\n
 */
int get_line( int sock, char *buf, int size )
{
    int i   = 0;
    char    c   = '\0';
    int n;

    while ( (i < size - 1) && (c != '\n') )
    {
        n = recv( sock, &c, 1, 0 );
        /* DEBUG printf("%02X\n", c); */
        if ( n > 0 )
        {
            if ( c == '\r' )
            {
                /* 偷窥一个字节，如果是\n就读走 */
                n = recv( sock, &c, 1, MSG_PEEK );
                /* DEBUG printf("%02X\n", c); */
                if ( (n > 0) && (c == '\n') )
                    recv( sock, &c, 1, 0 );
                else
                    /*不是\n（读到下一行的字符）或者没读到，置c为\n 跳出循环,完成一行读取 */
                    c = '\n';
            }
            buf[i] = c;
            i++;
        }else
            c = '\n';
    }
    buf[i] = '\0';

    return(i);
}


/**********************************************************************/
/* Return the informational HTTP headers about a file. */


/* Parameters: the socket to print the headers on
 *             the name of the file */
/**********************************************************************/

/* 加入http的headers */
void headers( int client, const char *filename )
{
    char buf[1024];
    (void) filename; /* could use filename to determine file type */

    strcpy( buf, "HTTP/1.0 200 OK\r\n" );
    send( client, buf, strlen( buf ), 0 );
    strcpy( buf, SERVER_STRING );
    send( client, buf, strlen( buf ), 0 );
    sprintf( buf, "Content-Type: text/html\r\n" );
    send( client, buf, strlen( buf ), 0 );
    strcpy( buf, "\r\n" );
    send( client, buf, strlen( buf ), 0 );
}


/**********************************************************************/
/* Give a client a 404 not found status message. */
/**********************************************************************/

/* 如果资源没有找到得返回给客户端下面的信息 */
void not_found( int client )
{
    char buf[1024];

    sprintf( buf, "HTTP/1.0 404 NOT FOUND\r\n" );
    send( client, buf, strlen( buf ), 0 );
    sprintf( buf, SERVER_STRING );
    send( client, buf, strlen( buf ), 0 );
    sprintf( buf, "Content-Type: text/html\r\n" );
    send( client, buf, strlen( buf ), 0 );
    sprintf( buf, "\r\n" );
    send( client, buf, strlen( buf ), 0 );
    sprintf( buf, "<HTML><TITLE>Not Found</TITLE>\r\n" );
    send( client, buf, strlen( buf ), 0 );
    sprintf( buf, "<BODY><P>The server could not fulfill\r\n" );
    send( client, buf, strlen( buf ), 0 );
    sprintf( buf, "your request because the resource specified\r\n" );
    send( client, buf, strlen( buf ), 0 );
    sprintf( buf, "is unavailable or nonexistent.\r\n" );
    send( client, buf, strlen( buf ), 0 );
    sprintf( buf, "</BODY></HTML>\r\n" );
    send( client, buf, strlen( buf ), 0 );
}


/**********************************************************************/


/* Send a regular file to the client.  Use headers, and report
 * errors to client if they occur.
 * Parameters: a pointer to a file structure produced from the socket
 *              file descriptor
 *             the name of the file to serve */
/**********************************************************************/

/* 如果不是CGI文件，直接读取文件返回给请求的http客户端 */
void serve_file( int client, const char *filename )
{
    FILE    *resource   = NULL;
    int numchars    = 1;
    char    buf[1024];

    printf("-->serve_file:%s\n", filename);
    /* 默认字符 */
    buf[0] = 'A'; buf[1] = '\0';
    while ( (numchars > 0) && strcmp( "\n", buf ) ) /* read & discard headers */
        numchars = get_line( client, buf, sizeof(buf) );

    resource = fopen( filename, "r" );
    if ( resource == NULL )
        not_found( client );
    else{
        headers( client, filename );
        cat( client, resource );
    }
    fclose( resource );
}


/**********************************************************************/


/* This function starts the process of listening for web connections
 * on a specified port.  If the port is 0, then dynamically allocate a
 * port and modify the original port variable to reflect the actual
 * port.
 * Parameters: pointer to variable containing the port to connect on
 * Returns: the socket */
/**********************************************************************/
int startup( int *port )
{
    int         httpd = 0;
    struct sockaddr_in  name;

    httpd = socket( PF_INET, SOCK_STREAM, 0 );
    if ( httpd == -1 )
        error_die( "socket" );
    memset( &name, 0, sizeof(name) );
    name.sin_family     = AF_INET;
    name.sin_port       = htons( *port );
    name.sin_addr.s_addr    = htonl( INADDR_ANY );

    int on=1;
    if((setsockopt(httpd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))<0)
    {
        perror("setsockopt failed");
    }

    /* 绑定socket */
    if ( bind( httpd, (struct sockaddr *) &name, sizeof(name) ) < 0 )
        error_die( "bind" );
    /* 如果端口没有设置，提供个随机端口 */
    if ( *port == 0 ) /* if dynamically allocating a port */
    {
        socklen_t namelen = sizeof(name);
        if ( getsockname( httpd, (struct sockaddr *) &name, &namelen ) == -1 )
            error_die( "getsockname" );
        *port = ntohs( name.sin_port );
    }
    /* 监听 */
    if ( listen( httpd, 5 ) < 0 )
        error_die( "listen" );
    return(httpd);
}


/**********************************************************************/


/* Inform the client that the requested web method has not been
 * implemented.
 * Parameter: the client socket */
/**********************************************************************/

/* 如果方法没有实现，就返回此信息 */
void unimplemented( int client )
{
    char buf[1024];

    sprintf( buf, "HTTP/1.0 501 Method Not Implemented\r\n" );
    send( client, buf, strlen( buf ), 0 );
    sprintf( buf, SERVER_STRING );
    send( client, buf, strlen( buf ), 0 );
    sprintf( buf, "Content-Type: text/html\r\n" );
    send( client, buf, strlen( buf ), 0 );
    sprintf( buf, "\r\n" );
    send( client, buf, strlen( buf ), 0 );
    sprintf( buf, "<HTML><HEAD><TITLE>Method Not Implemented\r\n" );
    send( client, buf, strlen( buf ), 0 );
    sprintf( buf, "</TITLE></HEAD>\r\n" );
    send( client, buf, strlen( buf ), 0 );
    sprintf( buf, "<BODY><P>HTTP request method not supported.\r\n" );
    send( client, buf, strlen( buf ), 0 );
    sprintf( buf, "</BODY></HTML>\r\n" );
    send( client, buf, strlen( buf ), 0 );
}


/**********************************************************************/

void httpd_thread( void *arg )
{
    int         server_sock = -1;
    int         port        = (int)arg;
    int         client_sock = -1;
    struct sockaddr_in  client_name;

    /* 这边要为socklen_t类型 */
    socklen_t   client_name_len = sizeof(client_name);
    pthread_t   newthread;

    server_sock = startup( &port );

    while ( 1 )
    {
        /*
         * 接受请求，函数原型
         * #include <sys/types.h>
         * #include <sys/socket.h>
         * int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
         */
        client_sock = accept( server_sock,
                      (struct sockaddr *) &client_name,
                      &client_name_len );
        if ( client_sock == -1 )
            error_die( "accept" );
        /* accept_request(client_sock); */

        /*
         * 每次收到请求，创建一个线程来处理接受到的请求
         * 把client_sock转成地址作为参数传入pthread_create
         */
        if ( pthread_create( &newthread, NULL, (void *) accept_request, (void *) (intptr_t) client_sock ) != 0 )
            perror( "pthread_create" );
    }

    close( server_sock );
}

int goke_httpd_start(int port, HTTPD_CallBack data_recv_func)
{
    pthread_t   newthread;
    if (data_recv_func == NULL)
    {
        return -1;
    }
    data_recv_callback = data_recv_func;
    int ret = pthread_create( &newthread, NULL, (void *) httpd_thread, (void *) port);

    return ret;
}


