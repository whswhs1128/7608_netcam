#include "hik_netfun.h"
#include "hik_netlogin.h"


extern pthread_mutex_t usrMutex;
extern HikUser *pGlobalUserInfo;
extern int genRandom(void);
extern void handleLegalAccess(int userIdx,	char *username,	struct in_addr *clientIp, struct in_addr *socketIp,	
								char *macAddr, int *userID,	time_t currTime, UINT32 sdkVersion);

/*
 * Function: checkUserIdValid
 * Description:check user validity
 * Input:  userID
 * Output: NONE
 * Return: OK when user valid, or ERROR when user invalid
 */
int checkUserIdValid(int userID)
{
	int i;
	UINT32 retVal = -1;
	HikUSER_INFO *pUserInfo = NULL;

	//HPRINT_INFO("=====userID is %d=======\n", userID);

	pthreadMutexLock(&usrMutex, WAIT_FOREVER);
	for(i = 0; i < MAX_LOGIN_USERS; i++)
	{
		pUserInfo = &pGlobalUserInfo->puserinfo[i];
		//HPRINT_INFO("userID:%d---%d\n", pUserInfo->userID, userID);
		if(pUserInfo->userID == userID)
		{
			retVal = 0;
			break;
		}
	}
	pthreadMutexUnlock(&usrMutex);

	return retVal;
}

/*
 * Function: getUserIdx
 * Description:get user idx
 * Input:  userID
 * Output: NONE
 * Return:the  pGlobalUserInfo Idx
 */
int getUserIdx(int userID)
{
	int i,userIdx = -1;
	HikUSER_INFO *pUserInfo = NULL;
	BOOL bFindUser = FALSE;

	pthreadMutexLock(&usrMutex, WAIT_FOREVER);
	for(i = 0; i < MAX_LOGIN_USERS; i++)
	{
		pUserInfo = &pGlobalUserInfo->puserinfo[i];
		if(pUserInfo->userID == userID)
		{
			userIdx = i;
			bFindUser = TRUE;
			break;
		}
	}
	pthreadMutexUnlock(&usrMutex);

	if(!bFindUser)
	{
		HPRINT_ERR(" not find user\n");
		return -1;
	}

	//HPRINT_INFO(" getUserIdx is %d\n", i);
	return userIdx;
}

/*
 * Function: checkNetUserValid
 * Description:check net user valid
 * Input:
 *		clientIp-client ip
 *		peerSockIp-socket ip
 *		macAddr-client mac addr
 *		pUserInfo-the pointer point to HikUSER_INFO
 * Output: NONE
 * Return: OK when net user valid,
 *		   ERROR when net user invalid
 */
int checkNetUserValid(struct in_addr *clientIp, struct in_addr *peerSockIp, char *macAddr,HikUSER_INFO *pUserInfo)
{
	if(NULL == pUserInfo)
		return -1;
 
#ifdef CHECK_CLIENT_IP
	if(clientIp != NULL && clientIp->s_addr!=pUserInfo->peerClientIp.s_addr)
	{
		HPRINT_ERR("Mismatch client IP: 0x%x, 0x%x\n", clientIp->s_addr, pUserInfo->peerClientIp.s_addr);
		return -1;
	}
#endif

#ifdef CHECK_SOCKET_IP
	if(peerSockIp != NULL	&& peerSockIp->s_addr!=pUserInfo->peerSocketIp.s_addr)
	{
		HPRINT_ERR("Mismatch socket IP: 0x%x, 0x%x\n", peerSockIp->s_addr, pUserInfo->peerSocketIp.s_addr);
		return -1;
	}
#endif

#ifdef CHECK_MAC_ADDR
	if(macAddr!=NULL && memcmp(macAddr, pUserInfo->peerMacAddr, MACADDR_LEN) != 0)
	{
		HPRINT_ERR("Mismatch MAC addresss.\n");
		return -1;
	}
#endif

	return 0;
}

/*
 * Function: getUserSDKVersion
 * Description: get the user's sdk version
 * Input:   userID - the user id
 * Output:  N/A
 * Return:  sdk version if successful, otherwise return 0
 *
 */
UINT32 getUserSDKVersion(int userID)
{
	HikUSER_INFO *pUserInfo = NULL;
	int userIdx;
	UINT32 sdkVersion;

	if(checkUserIdValid(userID) != 0)
	{
		HPRINT_ERR("Invalid userID %d\n", userID);
		return 0;
	}

	userIdx = getUserIdx(userID);
	if(userIdx == -1)
	{
		return 0;
	}

	pthreadMutexLock(&usrMutex, WAIT_FOREVER);
	pUserInfo = &pGlobalUserInfo->puserinfo[userIdx];
	sdkVersion = pUserInfo->sdkVersion;
	pthreadMutexUnlock(&usrMutex);

	return sdkVersion ;
}


/*
 * Function: verifyUserID
 * Description: check out the user's operation permission
 * Input:   userID - the user id
 *          clientIp - points to the client ip address
 *          peerSockIp - points to the peer socket ip address
 *          macAddr - points to the MAC address
 *          op - operation permission
 * Output:  N/A
 * Return:  NETRET_NO_USERID - the user according to the userid isn't exist
 *          NETRET_QUALIFIED - permit
 *          NETRET_OPER_NOPERMIT - not permit
 *
 */
int verifyUserID(int userID, struct in_addr *clientIp, struct in_addr *peerSockIp, char *macAddr, UINT32 op)
{
	HikUSER_INFO *pUserInfo = NULL;
	int permission;
	int userIdx;

	if(checkUserIdValid(userID) != 0)
	{
		HPRINT_ERR("Invalid userID %d\n", userID);
		return NETRET_NO_USERID;
	}

	userIdx = getUserIdx(userID);
	if(userIdx == -1)
	{
		return NETRET_NO_USERID;
	}

	pthreadMutexLock(&usrMutex, WAIT_FOREVER);
	pUserInfo = &pGlobalUserInfo->puserinfo[userIdx];
	/* remote user, check IP/MAC address */
	if(checkNetUserValid(clientIp, peerSockIp, macAddr,pUserInfo) == -1)
	{
		pthreadMutexUnlock(&usrMutex);
		return NETRET_NO_USERID;
	}
	pthreadMutexUnlock(&usrMutex);
	
	/* !!! show parameter permission */
	if(op==REMOTESHOWPARAMETER || op==LOCALSHOWPARAMETER)
	{
		return NETRET_QUALIFIED;
	}

	pthreadMutexLock(&usrMutex, WAIT_FOREVER);
	permission = pUserInfo->permission;
	if((permission&op) == 0)
	{
		pthreadMutexUnlock(&usrMutex);
		return NETRET_OPER_NOPERMIT;
	}
	pthreadMutexUnlock(&usrMutex);

	return NETRET_QUALIFIED;
}

/*
 * Function: userLogin
 * Description: verify and log in the user
 * Input:   username - points to the user name
 *          passwd - points to the password
 *          clientIp - points to the client ip address
 *          socketIp - points to the socket ip address
 *          macAddr - points to the MAC address
 *          userID - points to the user id
 *          sdkVersion - the sdk version
 * Output:  N/A
 * Return:  OK if successful, otherwise return error status number
 *
 */
int userLogin(char *username, char *passwd,
					struct in_addr *clientIp, struct in_addr *socketIp,
					char *macAddr, int *userID, UINT32 sdkVersion)
{
	time_t currTime;
	int userIdx;
	int retVal;

	HPRINT_INFO("userLogin.\n");

	if(username[0]=='\0' && passwd[0]=='\0')/* refuse emtpy usernam/password */
	{
		HPRINT_INFO("Empty user!!!\n");
		return NETRET_ERRORPASSWD;
	}

#ifdef PROCESS_LOCAL_REPEAT_LOGIN
	if(clientIp==NULL && bLocalLogined)
	{		/* 本地用户重复登录的情况 */
		if(checkLocalRelogin(username, userID) == LOCAL_USER_RELOGIN)
		{
			return 0;
		}
	}
#endif

	currTime = time(NULL);
	retVal = verifyUserPasswd(username, passwd, clientIp, macAddr, sdkVersion, &userIdx);
	if(retVal != 0)
	{
		/* illegal access */
		//handleIllegalAccess(username, clientIp, currTime, sdkVersion);
		return retVal;
	}
	else
	{
		/* legal access */
		handleLegalAccess(userIdx, username, clientIp, socketIp, macAddr, userID, currTime, sdkVersion);
	}

	if(*userID == NO_USER)
	{
		/* can't find slot, 128 users logined */
		HPRINT_INFO("Too many users logined.\n");
		return NETRET_EXCEED_MAX_USER;
	}

	HPRINT_INFO("User login OK: userID = %d, username = %s\n", *userID, username);
	return 0;
}


/*
 * Function: userLogout
 * Description: log out the user and reclaim the relative resource
 * Input:   bLocalUser - whether is local user
 *          userID - the user id
 *          clientIp - points to the client ip address
 *          peerSockIp - points to the peer socket ip address
 *          macAddr - points to the MAC address
 * Output:  N/A
 * Return:  OK if successful, otherwise return ERROR
 *
 */
int userLogout(BOOL bLocalUser, int userID, struct in_addr *clientIp, struct in_addr *peerSockIp, char *macAddr)
{
	HikUSER_INFO *pUserInfo = NULL;
	int retVal = -1;
	int userIdx;

#if 1
	if(checkUserIdValid(userID) != 0)
	{
		HPRINT_ERR("Invalid userID %d\n", userID);
		return -1;
	}

	userIdx = getUserIdx(userID);
	if(userIdx == -1)
	{
		goto errExit;
	}
#else
	userIdx= 0;
#endif
	pthreadMutexLock(&usrMutex, WAIT_FOREVER);
	pUserInfo = &pGlobalUserInfo->puserinfo[userIdx];

	if(!bLocalUser)
	{
		/* network user, check client IP and MAC address */
		if(checkNetUserValid(clientIp, peerSockIp, macAddr,pUserInfo) == -1)
		{
			pthreadMutexUnlock(&usrMutex);
			goto errExit;
		}
	}
#if 0
#ifdef PROCESS_LOCAL_REPEAT_LOGIN
	else
	{
		bLocalLogined = FALSE;
		localLoginedUserID = NO_USER;
	}
#endif
#endif

	HPRINT_INFO("####\n");
	pthreadMutexUnlock(&usrMutex);
	retVal = 0;
	pthreadMutexLock(&usrMutex, WAIT_FOREVER);
	pUserInfo->userID = NO_USER;
	pthreadMutexUnlock(&usrMutex);

errExit:

	HPRINT_INFO("User logout  retVal =%d : userID = %d.\n",retVal, userID);
	return retVal;
}

/*
 * Function: userKeepAlive
 * Description: update the user's expire time in order to keep alive
 * Input:   userID - the user id
 *          clientIp - points to the client ip address
 *          peerSockIp - points to the peer socket ip address
 *          macAddr - points to the MAC address
 * Output:  N/A
 * Return:  OK if successful, otherwise return ERROR
 *
 */
int userKeepAlive(int userID, struct in_addr *clientIp, struct in_addr *peerSockIp, char *macAddr)
{
	HikUSER_INFO *pUserInfo = NULL;
	int retVal = -1;
	int userIdx;

	if(checkUserIdValid(userID) != 0)
	{
		HPRINT_ERR("Invalid userID %d\n", userID);
		return -1;
	}

	userIdx = getUserIdx(userID);
	if(userIdx == -1)
	{
		return -1;
	}

   	pthreadMutexLock(&usrMutex, WAIT_FOREVER);
	pUserInfo = &pGlobalUserInfo->puserinfo[userIdx];

	if(checkNetUserValid(clientIp, peerSockIp, macAddr,pUserInfo) == -1)
	{
		goto errExit;
	}

	pUserInfo->expireTime = time(NULL) + KEEP_ALIVE_INTERVAL;
	retVal = 0;

errExit:
   	pthreadMutexUnlock(&usrMutex);

	return retVal;
}





