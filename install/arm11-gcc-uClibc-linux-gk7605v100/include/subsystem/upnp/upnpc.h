
/*
 * IPC UPNP api
 *
 * Author : Heyong
 * Date   : 2015.9.28
 * Copyright (c) 2015 Harric He(Heyong)
 *
 */
#ifndef UPNPC_H_INCLUDED
#define UPNPC_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

int init_port_redirection();//call at first.

void add_port_redirection(int port, int external_port, char *protocol);
void add_port_redirection_by_ip(char *ip, int port, int external_port, char *protocol);

void del_port_redirection(int external_port, char *protocol);
void del_all_port_redirection();

void set_port_redirection_state(int port, int external_port, char *protocol, int enable);

void deinit_port_redirection();//must call when operations finish.

/*
test:

if(init_port_redirection() == 0)
{
    add_port_redirection(66666, 55555, "TCP");
    add_port_redirection(44444, 33333, "TCP");

    add_port_redirection_by_ip("192.168.7.25", 2222, 5555, "UDP");
    add_port_redirection_by_ip("192.168.7.28", 2222, 8888, "UDP");

    del_port_redirection(55555, "TCP");
    del_port_redirection(8888, "UDP");
    del_port_redirection(5555, "UDP");
    del_port_redirection(6600, "UDP");
    del_port_redirection(33333, "TCP");
    del_port_redirection(726, "UDP");

    add_port_redirection(1110, 726, "TCP");
    add_port_redirection(726, 1110, "UDP");
    set_port_redirection_state(726, 1110, "UDP", 0);

    deinit_port_redirection();
}

*/

#ifdef __cplusplus
}
#endif

#endif /* UPNPC_H_INCLUDED */


