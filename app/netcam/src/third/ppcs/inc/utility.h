/**
 * Copyright (C) 2011, Skype Limited
 *
 * All intellectual property rights, including but not limited to copyrights,
 * trademarks and patents, as well as know how and trade secrets contained in,
 * relating to, or arising from the internet telephony software of
 * Skype Limited (including its affiliates, "Skype"), including without
 * limitation this source code, Skype API and related material of such
 * software proprietary to Skype and/or its licensors ("IP Rights") are and
 * shall remain the exclusive property of Skype and/or its licensors.
 * The recipient hereby acknowledges and agrees that any unauthorized use of
 * the IP Rights is a violation of intellectual property laws.
 *
 * Skype reserves all rights and may take legal action against infringers of
 * IP Rights.
 *
 * The recipient agrees not to remove, obscure, make illegible or alter any
 * notices or indications of the IP Rights and/or Skype's rights and
 * ownership thereof.
 */

#ifndef UTILITY_H_
#define UTILITY_H_

/*
 *  Logging
 *
 */
#ifndef LOG_TAG
#define LOG_TAG "utilityCode"
#endif

#define Log(...)
#define Log2(Fmt, args...)
#define Log3(Fmt, args...) \
	{	\
		 printf("[I-%s]"Fmt, __FILE__, ##args); \
	}




#endif /* UTILITY_H_ */
