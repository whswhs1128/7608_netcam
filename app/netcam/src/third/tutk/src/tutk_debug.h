#ifndef __TUTK_DEBUG__
#define __TUTK_DEBUG__

#include <stdio.h>

#define P2P_ERR(fmt, args...)  \
    do{\
        printf("[P2P_ERR] [ %s, Line: %d ]  " fmt "\n", __FILE__, __LINE__,  ##args); \
    }while(0)

#define P2P_WARNING(fmt, args...)  \
    do{\
        printf("[P2P_WARNING] [ %s, Line: %d ]  " fmt "\n", __FILE__, __LINE__,  ##args); \
    }while(0)

#define P2P_INFO(fmt, args...)  \
    do{\
        printf("[P2P_INFO] [ %s, Line: %d ]  " fmt "\n", __FILE__, __LINE__,  ##args); \
    }while(0)

#define P2P_DEBUG(fmt, args...)  \
    do{\
        printf("[P2P_DEBUG] [ %s, Line: %d ]  " fmt "\n", __FILE__, __LINE__,  ##args); \
    }while(0)		
		
#endif
