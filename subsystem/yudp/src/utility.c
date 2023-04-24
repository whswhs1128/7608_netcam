
/*
 * yudp_utility api define.
 *
 * Author : Heyong
 * Date   : 2015.11.27
 * Copyright (c) 2015 Harric He(Heyong)
 *
 */

#include "stdafx.h"

#ifndef _WIN32
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#include "yudp_utility.h"

void y_socketclose(int sock)
{
#ifdef _WIN32
    closesocket(sock);
#else
    close(sock);
#endif
}
