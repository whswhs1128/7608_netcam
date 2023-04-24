
#ifndef __NET_ARP_H__
#define __NET_ARP_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*fAddOnePeer)(int type, char *peer_ip, char *peer_mac);

#define ARP_RECV_TIMEOUT	(1) //UNIT : SECOND

/*
 	ret : return number of mac query, failed return -1
 	max-size: for example , query_mac[64][18], then max_size is 64
 	timeout: unit is second
*/
extern int ARP_send_request(char *query_ip, char (*query_mac)[18], int max_size, int timeout);
/*
	desc: return when receive one ack
	ret: return 1 if success, else return -1;
*/
extern int ARP_query(char *query_ip, char *query_mac); 
extern int ARP_get_arptable(char *query_ip, char *query_mac);
extern int RARP_send_request(char *query_ip, char *query_mac);// not work well
/*
 * get mac addr by ip
 * 
 */

extern int ARP_listener_start(fAddOnePeer hook);
extern int ARP_listener_stop();


#ifdef __cplusplus
}
#endif


#endif

