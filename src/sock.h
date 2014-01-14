#ifndef _SOCK_H
#define _SOCK_H
int opensock(char *ip_addr, int port);
int init_listen_sock(int port);
int listen_sock(void);
void de_init_listen_sock(void);
int readline(char *ptr, int maxlen, int fd);
#endif
