/******************************************************************************
  File Name     : hik_netfuns.c
  Version       : v1.0
  Author        : 
  Created       : 2014/11/20
  Description   : 
  History       :
  1.Date        : 2014/11/24
  Author        : 
  Modification: Created file
******************************************************************************/
#include "hik_netfun.h"
#include "cfg_all.h"
#include "hik_md5.h"
#include "hik_base64.h"

extern int g_hik_handle;

//#define CHECK_CLIENT_IP			/* check peer's client IP address */
//#define CHECK_MAC_ADDR		/* check peer's MAC address */
#define PWD_LEN     32

/* extern variables */
extern pthread_mutex_t usrMutex;
extern HikUser *pGlobalUserInfo;

/* extern functions */
extern UINT32 checkByteSum(const char *pBuf, int len);
extern int writen(int connfd, void *vptr, size_t n);
static void handleIllegalAccess(char *username, struct in_addr *clientIp, time_t currTime, UINT32 sdkVersion);
void handleLegalAccess(int userIdx,	char *username,	struct in_addr *clientIp, struct in_addr *socketIp,	
								char *macAddr, int *userID,	time_t currTime, UINT32 sdkVersion);

/* local functions */
//int challenge_login(int connfd, NET_LOGIN_REQ *reqdata, int *userid, struct sockaddr_in *pClientSockAddr);
int genRandom(void);

static int genChallenge(UINT32 clientIp, char **out); 
static int verifyUser(char *username, char *chanllenge);
static int verifyChallengePassword(int *userIdx, char *username, char *pwd, char *challenge, struct in_addr *clientIp, char *macAddr);

int hik_UserInfoInit(USER UserInfo[32])
{
	int i;
	//QMAPI_NET_USER_INFO UserCfg;
	//memset(&UserCfg,0,sizeof(UserCfg));
	memset(UserInfo, 0, MAX_USERNUM*sizeof(UserInfo));
	for(i = 0; i < GK_MAX_USER_NUM; i++)
	{
		if(runUserCfg.user[i].enable)
		{
			//QMapi_sys_ioctrl(g_hik_handle,QMAPI_SYSCFG_GET_USERCFG,0,&UserCfg,sizeof(UserCfg));
			strcpy((char *)UserInfo[i].username, runUserCfg.user[i].userName);
			//if(strlen(runUserCfg.user[0].password) > 16)
				memcpy((char *)UserInfo[i].password, runUserCfg.user[i].password, sizeof(UserInfo[i].password));
			//else
				//memcpy((char *)UserInfo[i].password,UserCfg.csPassword,strlen(UserCfg.csPassword));
			UserInfo[i].priority = 2;
			memset(&UserInfo[i].permission, 0xff, sizeof(UINT32));
			
			memset(&UserInfo[i].localBackupPermission, 0xff, sizeof(UINT64));
			memset(&UserInfo[i].localPlayPermission, 0xff, sizeof(UINT64));
			memset(&UserInfo[i].localPrevPermission, 0xff, sizeof(UINT64));
			memset(&UserInfo[i].localPtzCtrlPermission, 0xff, sizeof(UINT64));
			memset(&UserInfo[i].localRecordPermission, 0xff, sizeof(UINT64));

			memset(&UserInfo[i].netPlayPermission, 0xff, sizeof(UINT64));
			memset(&UserInfo[i].netPrevPermission, 0xff, sizeof(UINT64));
			memset(&UserInfo[i].netPtzCtrlPermission, 0xff, sizeof(UINT64));
			memset(&UserInfo[i].netRecordPermission, 0xff, sizeof(UINT64));
		}
	}

	return 0;
}

/*
 * Function: encryptUser
 * Description: encrypt
 * Input:   in - to encrypt string
 *          inLen - length of the string
 * Output:  out - encrypted string
 * Return:  N/A
 *
 */
static void encryptUser(signed char *in, signed char *out, int inLen)
{
	int            ix;
	unsigned long  magic     = 0x686b7773;
	unsigned long  passwdInt = 0;

	out[0] = 0;
	if(inLen/*strlen(in)*/==0)
	{
		return;
	}

	for(ix = 0; ix < inLen/*strlen(in)*/; ix++)
	{
		passwdInt += (in[ix]) * (ix+1) ^ (ix+1);
	}

	sprintf((char *)out, "%lu", (long)(passwdInt * magic));

	/* make encrypted passwd printable */
	for(ix = 0; ix < strlen ((char *)out); ix++)
	{
		if(out[ix] < '3')
		{
			out[ix] = out[ix] + 'B';
		}
		else if(out[ix] < '5')
		{
			out[ix] = out[ix] + '/';
		}
		else if(out[ix] < '7')
		{
			out[ix] = out[ix] + '>';
		}
		else if(out[ix] < '9')
		{
			out[ix] = out[ix] + '!';
		}
	}

	/* printf("in:%s, inlen=%d, out:%s\n", in, inLen, out); */

	return;
}


/*
 * Function: verifyUserPasswd
 * Description: verify the user's name and password,get various permission information
 * Input:   username - the user's name
 *          passwd - the user's password
 *          clientIp - points to the client's ip address
 *          macAddr - points to the MAC address
 *          sdkVersion - sdk version
 * Output:
 * Return:  OK if successful, otherwise return relative error number
 *
 */
int verifyUserPasswd(char *username, char *passwd, struct in_addr *clientIp, char *macAddr, UINT32 sdkVersion, int *userIdx)
{
	USER	userCfg[MAX_USERNUM];
	memset(userCfg,0,sizeof(userCfg));
	int 	i;
	//DEVICECONFIG *pDevCfg = pDevCfgParam;
	char encUser[NAME_LEN+1], encPasswd[PASSWD_LEN+1];
	char nn[ACTUAL_MAX_NAME_LEN+1], pp[PASSWD_LEN+1];

	bzero(nn, ACTUAL_MAX_NAME_LEN+1);
	bzero(pp, PASSWD_LEN+1);
	memcpy(nn, username, ACTUAL_MAX_NAME_LEN);
	memcpy(pp, passwd, PASSWD_LEN);
	bzero(encUser, NAME_LEN+1);
	bzero(encPasswd, PASSWD_LEN+1);

	hik_UserInfoInit(userCfg);
	for(i=0; i<MAX_USERNUM; i++)
	{
		if(clientIp == NULL
			|| sdkVersion == NETSDK_VERSION1_0			/* local user/SDK v1.0 user */
			|| (sdkVersion&RELOGIN_FLAG) != 0)			/* Relogin */
		{
			memcpy(encUser, userCfg[i].username, ACTUAL_MAX_NAME_LEN/*NAME_LEN*/);
			memcpy(encPasswd, userCfg[i].password, PASSWD_LEN);
		}
		else
		{
		    int tmp_len = 0;
		    if (strlen((char *)userCfg[i].username)  < ACTUAL_MAX_NAME_LEN)
		        tmp_len = strlen((char *)userCfg[i].username);
            else
                tmp_len = ACTUAL_MAX_NAME_LEN;
			encryptUser((signed char *)userCfg[i].username, (signed char *)encUser, tmp_len);

		    if (strlen((char *)userCfg[i].password)  < PASSWD_LEN)
		        tmp_len = strlen((char *)userCfg[i].password);
            else
                tmp_len = PASSWD_LEN;

			encryptUser((signed char *)userCfg[i].password, (signed char *)encPasswd, tmp_len);
		}

		if(strcmp(nn, encUser)==0 && strcmp(pp, encPasswd)==0)
		{	/* username/passwd verified */

#ifdef CHECK_CLIENT_IP
			if(clientIp!=NULL && userCfg[i].ipAddr.v4.s_addr!=0 &&
				clientIp->s_addr!=userCfg[i].ipAddr.v4.s_addr)
			{
				return NETRET_IP_MISMATCH;
			}
#endif

#ifdef CHECK_MAC_ADDR
			if(macAddr!=NULL && (userCfg[i].macAddr[0]!=0 || userCfg[i].macAddr[1]!=0
									|| userCfg[i].macAddr[2]!=0 || userCfg[i].macAddr[3]!=0
									|| userCfg[i].macAddr[4]!=0 || userCfg[i].macAddr[5]!=0)
				&& memcmp(macAddr, userCfg[i].macAddr, MACADDR_LEN)!=0)
			{
				return NETRET_MAC_MISMATCH;
			}
#endif

			memcpy(username, userCfg[i].username, NAME_LEN);	/* 2005/04/07: For encrypt username */

			*userIdx = i;
		 	HPRINT_INFO("username/passwd verify passed.\n");
			return 0;
		}
	}

	HPRINT_ERR("username/passwd  :%s/%s\n", username, passwd);
	HPRINT_ERR("verify fail.\n");
	return NETRET_ERRORPASSWD;
}


/*
 * Function: handleIllegalAccess
 * Description:handle illegal access
 * Input:
 *         username-user name
 *         clientIp-client ip
 *         currTime-curr time
 *         sdkVersion-sdk version
 * Output: NONE
 * Return: NONE
 */
void handleIllegalAccess(char *username, struct in_addr *clientIp, time_t currTime, UINT32 sdkVersion)
{
#if 0
	EXCEPTION_MSGS exceptionMsgs;
	DVR_LOG_ENTRY logEnt = {0};

	/* 1.send message to exceptionCtrlTask */
	exceptionMsgs.exceptionType = ILLEGALACCESS_EXCEPTION;
	mqSend(exceptionMsgQ, (char *)&exceptionMsgs, sizeof(EXCEPTION_MSGS), 200, MQ_PRIO_NORMAL);

	/* 2. write a log entry(illegal access) */
	logEnt.majorLogType = MAJOR_EXCEPTION;
	logEnt.minorLogType = MINOR_ILLEGAL_ACCESS;
	logEnt.logTime = currTime;
	if(sdkVersion >= CURRENT_NETSDK_VERSION)/*9000一次登录过程*/
	{
		memcpy((char *)(logEnt.logParam.exception.params), username, LIMITED_NAME_LEN);
	}
	else if ((sdkVersion&RELOGIN_FLAG)!=0)/*8000两次登录过程，两个tcp连接*/
	{
		memcpy((char *)(logEnt.logParam.exception.params), username, LIMITED_NAME_LEN);
	}
	else
	{
		/*如果用户名是老版本的加密字串，则日志不予显示*/
		memset((char *)(logEnt.logParam.exception.params), 0, LIMITED_NAME_LEN);
	}
	if(clientIp == NULL)
	{
		logEnt.logParam.exception.exc32parms = 0;
	}
	else
	{					/* network */
		logEnt.logParam.exception.exc32parms = clientIp->s_addr;
	}
	logEnt.infoLen = 0;
	//fprintf (stderr, "Ill log sdkVersion(%d )usrname:%s pass name:%s\n",(sdkVersion&RELOGIN_FLAG), (char *)(logEnt.logParam.exception.params), username);
	writeIllegLog ((char *)&logEnt);


	/*3. Beep*/
	if(clientIp == NULL)
	{
		funcBeep(3, 300, 100, 0, 0, 0);
	}
#endif

	return;
}


/*
 * Function: handleLegalAccess
 * Description:handle legal access
 * Input:
 *		 userIdx-user idx
 *		 username-user name
 *		 clientIp-client ip
 *		 socketIp-socket ip
 *       macAddr-mac addr
		 userID-user ID
		 currTime-curr time
		 sdkVersion-sdk version
 * Output: NONE
 * Return: NONE
 */
void handleLegalAccess(int userIdx,	char *username,	struct in_addr *clientIp, struct in_addr *socketIp,	
								char *macAddr, int *userID,	time_t currTime, UINT32 sdkVersion)
{
	USER	userCfg[MAX_LOGIN_USERS];
	memset(userCfg,0,sizeof(userCfg));
	HikUSER_INFO *pUserInfo = NULL;
	UINT32  permission;
	int i;

	*userID = NO_USER;
	pthreadMutexLock(&usrMutex, WAIT_FOREVER);

	int noUser = 0;
	int  needNoUserNum = (clientIp==NULL) ? 1 : 2;
	for(i=0; i<MAX_LOGIN_USERS; i++)
	{
		pUserInfo = (HikUSER_INFO *)&pGlobalUserInfo->puserinfo[i];
		if (pUserInfo->userID == NO_USER)
		{
			noUser++;
		}
		else
		{
			if (pUserInfo->bLocalUser)
			{
				needNoUserNum = 1;
			}
		}
		if (noUser >= needNoUserNum)
		{
			break;
		}
	}
	if (i == MAX_LOGIN_USERS)
	{
		HPRINT_INFO("max users\n");
		pthreadMutexUnlock(&usrMutex);
		return;
	}

	//userCfg = pDevCfg->user;
	//userCfg = g_user;
	HPRINT_INFO("=== 5 1\n");
	hik_UserInfoInit(userCfg);
    HPRINT_INFO("=== 5 2\n");
	for(i=0; i<MAX_LOGIN_USERS; i++)
	{
	    HPRINT_INFO("=== 5 3\n");
		pUserInfo = (HikUSER_INFO *)&pGlobalUserInfo->puserinfo[i];
        HPRINT_INFO("=== 5 4\n");
        if(pUserInfo == NULL)
            HPRINT_INFO("=== 888888888888888\n");
		if(pUserInfo->userID == NO_USER)
		{
		    HPRINT_INFO("=== 5 5\n");
			/* find available slot, and fill in user information */
			bzero((char *)pUserInfo, sizeof(HikUSER_INFO));
			memcpy(pUserInfo->userName, username, LIMITED_NAME_LEN);
			permission = userCfg[userIdx].permission;
			pUserInfo->permission = (permission|LOCALSHOWPARAMETER|REMOTESHOWPARAMETER);
			pUserInfo->priority = userCfg[userIdx].priority;
			pUserInfo->localPrevPermission = userCfg[userIdx].localPrevPermission;
			pUserInfo->netPrevPermission = userCfg[userIdx].netPrevPermission;
			pUserInfo->localPlayPermission = userCfg[userIdx].localPrevPermission;
			pUserInfo->netPlayPermission = userCfg[userIdx].netPlayPermission;
			pUserInfo->localRecordPermission = userCfg[userIdx].localRecordPermission;
			pUserInfo->netRecordPermission = userCfg[userIdx].netRecordPermission;
			pUserInfo->localPtzCtrlPermission = userCfg[userIdx].localPtzCtrlPermission;
			pUserInfo->netPtzCtrlPermission = userCfg[userIdx].netPtzCtrlPermission;
			pUserInfo->localBackupPermission = userCfg[userIdx].localBackupPermission;
			pUserInfo->bLocalUser = (clientIp==NULL);		/* local or network user login */
			pUserInfo->loginTime = currTime;				/* login time */
			if(pUserInfo->bLocalUser)
			{
				pUserInfo->expireTime = 0x7fffffff;
			}
			else
			{
				pUserInfo->expireTime = pUserInfo->loginTime + KEEP_ALIVE_INTERVAL;
				pUserInfo->peerClientIp.s_addr = clientIp->s_addr;
				if(socketIp!=NULL)
				{
					pUserInfo->peerSocketIp.s_addr = socketIp->s_addr;
				}
				else
				{
					pUserInfo->peerSocketIp.s_addr = 0;
				}
				if(macAddr!=NULL)
				{
					memcpy(pUserInfo->peerMacAddr, macAddr, MACADDR_LEN);
				}
			}
            HPRINT_INFO("=== 5 6\n");

			pUserInfo->sdkVersion = sdkVersion&(~RELOGIN_FLAG);		/* for network user */
			if(pUserInfo->bLocalUser)
			{
				*userID = i + BASE_USER_ID;
			}
			else
			{
#if 1
				if(pUserInfo->sdkVersion >= NETSDK_VERSION4_0)
				{
					 *userID = genRandom();			/*9000登录分配随机ID*/
				}
				else
				{
					*userID = i + BASE_USER_ID;
				}
#else
				*userID = i + BASE_USER_ID;
#endif
			}
            HPRINT_INFO("=== 5 7\n");
			pUserInfo->userID = *userID;			/* occupy the slot */
			pUserInfo->cfgUserID = userIdx;
			break;
		}
	}
	pthreadMutexUnlock(&usrMutex);
    HPRINT_INFO("=== 5 8\n");

	return;
}


/*******************************************************************************
 *  Description: 生成随机数函数
 ******************************************************************************/
int genRandom(void)
{
    int fd = -1;
    int l = 0;
    unsigned int ticks = 0;
    struct timeval tv;
    static int random_seed_set = 0;

    if(random_seed_set == 0)
    {
        gettimeofday(&tv, NULL);
        ticks = tv.tv_sec + tv.tv_usec;

        fd = open("/dev/urandom", O_RDONLY);
        if(fd > 0)
        {
            int i;
            unsigned int r;

            for(i = 0; i < 256; i++)
            {
                l = read(fd, &r, sizeof(r));
                if (l != sizeof(r))
                {
                    HPRINT_INFO("Cannot read random data, errno=%d\n", errno);
                    break;
                }
                else
                {
                    ticks += r;
                }
            }
            close(fd);
        }

        srand(ticks);
    }

    return (rand());
}


/*******************************************************************************
 *  Name: genChallenge
 *  Description: 使用设备当前时间、客户端连接IP等参数生成随机挑战串
 *
 *       Mode   Type            Name         Description
 * -----------------------------------------------------------------------------
 *        in:   unsigned int    clientIp    客户端连接IP地址
 *    in-out:   无
 *       out:   char **         out         生成的随机挑战串
 *    return:   int                         成功返回挑战串的长度，失败返回-1
 ******************************************************************************/
static int genChallenge(UINT32 clientIp, char **out)
{
    time_t  t;
    char    tmp[40] = {0};
    unsigned int    i = 0;
    unsigned int    ran = 0;
    unsigned char   hash[16] = {0};
    struct timeval  tv;

    if((NULL == out) || (NULL == *out))
    {
        return (-1);
    }

    gettimeofday(&tv, 0);
    t = tv.tv_sec + tv.tv_usec;
    i += sprintf(tmp, "%lx", t);
    ran = genRandom();
    //HPRINT_INFO("random number: %u\n", ran);
    i += sprintf((tmp + i), "%x", ran);
    i += sprintf((tmp + i), "%x", clientIp);

    /* HIK_MD5 generated challenge string */
    HIK_MD5String(tmp, i, hash);
    HIK_MD5toStr(hash, 16, *out);
    //HPRINT_INFO("Original challenge string: %s\n", *out);

    return (32);
}


/*******************************************************************************
 *         Name: verifyUser
 *  Description: 判断客户端发送的用户名是否有效
 *
 *       Mode   Type         Name         Description
 * -----------------------------------------------------------------------------
 *        in:   char *      username    从客户端接收到经过Magic加密之后的用户名
 *    in-out:   无
 *       out:   无
 *    return:   int                     成功则返回该用户在设备配置文件中的位置
 *                                      索引值，失败返回-1
 ******************************************************************************/
static int verifyUser(char *username, char *challenge)
{
    int     i = 0;
    int     len = 0;
    int     clen = 0;
    int     inlen = 0;
    char    tmp[NAME_LEN] = {0};
    char    usr[NAME_LEN] = {0};
	USER	userCfg[MAX_LOGIN_USERS];
	memset(userCfg,0,sizeof(userCfg));
    if(NULL == username)
    {
        return (-1);
    }
	
    inlen = strlen(username);
    for(i = 0; i < MAX_USERNUM; i++)
    {
        memset(tmp, 0, NAME_LEN);
		hik_UserInfoInit(userCfg);
		strncpy(usr, (char *)userCfg[i].username, NAME_LEN);
        if (strlen((char *)userCfg[i].username) > 0)
            HPRINT_INFO("i:%d username:%s psd:%s\n", i, (char *)userCfg[i].username, (char *)userCfg[i].password);
        len = strlen(usr);
        if(len == 0)
        {
            continue;
        }
		
        clen = strlen(challenge);
        /* encrypt user name */
        encryptHMAC((unsigned char *)usr, len, (unsigned char *)challenge, clen, (unsigned char *)tmp);
        if(strncmp(username, tmp, inlen))
        {
            continue;
        }
        else
        {
            HPRINT_INFO("Match OK %s:%d\n",usr,i);
	     	memcpy(username, usr, len+1);
            return (i);
        }
    }

    HPRINT_ERR("No user matched, challenge failed\n");
    return (-1);
}


/*******************************************************************************
 *         Name: verifyChallengePassword
 *  Description: 判断客户端使用挑战串对密码加密之后的结果是否与本地保存的密码
 *              进行相同的加密匹配
 *
 *       Mode   Type            Name         Description
 * -----------------------------------------------------------------------------
 *        in:   int             index       要验证的用户在配置文件中的索引，用来
 *                                          获取对应的密码
 *              char *          pwd         从客户端接收到的加密之后的密码值
 *              char *          challenge   加密用的挑战串
 *              struct in_addr* clientIp    连接的客户端IP地址
 *              char *          macAddr     客户端连接的对端MAC地址
 *    in-out:   无
 *       out:    无
 *    return:   int                         成功返回0，失败返回-1
 ******************************************************************************/
static int verifyChallengePassword(int *userIdx, char *username , char *pwd, char *challenge, struct in_addr *clientIp, char *macAddr)
{
    int     len = 0;
    unsigned char digest[PASSWD_LEN] = {0};
    unsigned char tmp[PASSWD_LEN] = {0};
	USER	userCfg1[MAX_LOGIN_USERS];
	memset(userCfg1,0,sizeof(userCfg1));

    if((NULL == pwd) || (NULL == challenge) || (NULL == userIdx))
    {
        return (NETRET_ERRORPASSWD);
    }

    int clen = strlen(challenge);
    int index = verifyUser((char *)username, challenge);
    if(index < 0)
    {
        return (NETRET_ERRORPASSWD);
    }
    *userIdx = index;
	HPRINT_INFO("===1 \n");
    /*检查SDK登录的挑战串密码*/
	hik_UserInfoInit(userCfg1);
    HPRINT_INFO("=== 2\n");
	memcpy(tmp, userCfg1[index].password, PASSWD_LEN);
    len = strlen((char *)tmp);
    HPRINT_INFO("===3 \n");

    encryptHMAC(tmp, len, (unsigned char *)challenge, clen, digest);
    HPRINT_INFO("===4\n");
    return 0;
}


/*******************************************************************************
 *         Name: challenge_login
 *  Description: 客户端采用挑战串模式登录设备的入口函数
 *
 *       Mode   Type                    Name            Description
 * -----------------------------------------------------------------------------
 *        in:   int                     connfd          客户端与设备连接的socket
 *              NET_LOGIN_REQ *         reqdata         客户端发送的登录请求数据
 *              struct sockaddr_in *    pClientSockAddr 客户端连接地址信息
 *    in-out:   无
 *       out:   int *                   userid          登录成功后设备返回给客户
 *                                                      端的用户ID
 *    return:   int                     成功返回0，失败返回设备指定的错误号
 ******************************************************************************/
int challenge_login(int connfd, NET_LOGIN_REQ *reqdata, int *userid, struct sockaddr_in *pClientSockAddr)
{
    int     userIdx = -1;
    char    buf[64] = {0};
    char    *p = &buf[0];
    char    cbuf[1024] = {0};
    char    username[NAME_LEN] = {0};
    fd_set  rset; 
    struct timeval      tv;
    struct in_addr      clientIpAddr;
    NET_LOGIN_REQ       login_req;
    NET_LOGIN_CHALLENGE login_cha;
    time_t currTime;

    if((connfd < 0) || (NULL == reqdata))
    {
        return NETRET_OPER_NOPERMIT;
    }

    memset(&login_cha, 0, sizeof(NET_LOGIN_CHALLENGE));

    int ret = genChallenge(reqdata->clientIp, &p);
    if(ret < 0)
    {
        HPRINT_ERR("Gengrate challenge string failed\n");
        return (NETRET_OPER_NOPERMIT);
    }

    /* BASE64 encoding challenge string */
    ret = hik_base64_encode(p, ret, cbuf);

#if 0
    login_cha.length = htonl(sizeof(NET_LOGIN_CHALLENGE));
	login_cha.retVal = htonl(100); /* !!! 9000 specified,需要SDK再次发送加密的用户名和密码 */
    login_cha.devSdkVer = htonl(CURRENT_NETSDK_VERSION);
    memcpy(login_cha.challenge, cbuf, ret);

	login_cha.checkSum = checkByteSum((char *)&(login_cha.retVal),sizeof(NET_LOGIN_CHALLENGE) - 8);

	/* send challenged data */
	HPRINT_INFO("login_cha.retVal=0x%X\n", login_cha.retVal);
	ret = writen(connfd, &login_cha, sizeof(login_cha));
	if(ret != sizeof(login_cha))
	{
		HPRINT_ERR("send challenge to client failed\n");
		return (NETRET_OPER_NOPERMIT);
	}

#else
    login_cha.length = htonl(sizeof(NET_LOGIN_CHALLENGE));	//抓包分析 hik ipc 此段长度为96/76

	if(reqdata->version > NETSDK_VERSION4_0)
	{
		HPRINT_INFO("CMS Login\n");
		login_cha.retVal = htonl(100);
		login_cha.devSdkVer = htonl(NETSDK_VERSION4_0);
		memcpy(login_cha.challenge, cbuf, ret);
	}
	else
	{
		HPRINT_INFO("NVR Login\n");
		login_cha.retVal = htonl(NETRET_NEED_RELOGIN);
	}
	login_cha.checkSum = checkByteSum((char *)&(login_cha.retVal), sizeof(NET_LOGIN_CHALLENGE) - 8);

	ret = writen(connfd, &login_cha, sizeof(login_cha));
	if(ret != sizeof(login_cha))
	{
		HPRINT_ERR("send challenge to client failed\n");
		return (NETRET_OPER_NOPERMIT);
	}

#endif

    tv.tv_sec = 10;
    tv.tv_usec = 0;

    FD_ZERO(&rset);
    FD_SET(connfd, &rset);

    ret = select(connfd+1, &rset, NULL, NULL, &tv);
    if(ret <= 0)
    {
        HPRINT_ERR("Waitting for client challenge password failed, ret=%d, errno=%d, connfd=%d\n", ret, errno, connfd);
        return (NETRET_OPER_NOPERMIT);
    }
    else
    {
        if(FD_ISSET(connfd, &rset))
        {
            ret = read(connfd, cbuf, 1024);
            if(ret > 0)
            {
				/*接收到新的登录信息*/
				memcpy(&login_req, cbuf, sizeof(NET_LOGIN_REQ));

				currTime = time(NULL);
				clientIpAddr.s_addr = reqdata->clientIp;
				memcpy(username,(char *)login_req.username, NAME_LEN);

				ret = verifyChallengePassword(&userIdx, username, (char *)login_req.password, buf, &clientIpAddr, (char *)reqdata->clientMac);
				if(ret != 0)
				{
					/* illegal access */
                    HPRINT_INFO("=== 4444\n");
					handleIllegalAccess(username, &clientIpAddr, currTime, reqdata->version);
					return (ret);
				}
				else
				{
					/* legal access */
                    HPRINT_INFO("=== 5\n");
					handleLegalAccess(userIdx, username, &clientIpAddr, &(pClientSockAddr->sin_addr),
											(char *)reqdata->clientMac, userid, currTime, reqdata->version);
                    HPRINT_INFO("=== 666\n");
					if(*userid == -1)
					{
						/* can't find slot, 128 users logined */
						HPRINT_ERR("Too many users logined.\n");
						return NETRET_EXCEED_MAX_USER;
					}
				}
			}
			else
			{
				//接收到0数据???????
				//为何此处接收不到数据????????
				HPRINT_ERR("ret=%d, errno=%d, %s, connfd=%d\n", ret, errno, strerror(errno), connfd);
			}
        }
		else
		{
			HPRINT_ERR("timeout\n");
		}
    }
    HPRINT_INFO("=== 777\n");

    //sleep(3);

    return 0;
}

