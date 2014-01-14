/* sock.c - routines for manipulating sockets. */
#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "tinyproxy.h"

extern struct stat_s stats;

/* this routine is so old I can't even remember writing it.  But I do
 * remember that it was an .h file because I didn't know putting code in a
 * header was bad magic yet.  anyway, this routine opens a connection to a
 * system and returns the fd. */
int opensock(char *ip_addr, int port) {
   int sock_fd;
   struct sockaddr_in port_info;
   struct hostent *resolv_addy;
   
   bzero((struct sockaddr *) &port_info, sizeof(port_info));
   
   port_info.sin_family = AF_INET;
   
   if ((resolv_addy = gethostbyname(ip_addr)) == NULL)
     return -1;
   
   bcopy(resolv_addy->h_addr, (char *) &port_info.sin_addr, resolv_addy->h_length);
   port_info.sin_port = htons(port);
   
   if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
     return -1;
   
   if (connect(sock_fd, (struct sockaddr *) &port_info, sizeof(port_info)) == -1)
     return -1;
   
   /*fcntl(sock_fd, F_SETFL, O_NONBLOCK);*/
   
   stats.num_opens++;
   return (sock_fd);
}

int fd;
int setup_fd;
struct sockaddr fooaddr;

/* start listening to a socket. */
int init_listen_sock(int port) {
   struct sockaddr_in sin;
   int i=1;

   if ((setup_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      return -1;
   }

   if(setsockopt(setup_fd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(int)) < 0) {
      return -1;
   }

   bzero(&fooaddr, sizeof(fooaddr));
   bzero(&sin, sizeof(sin));
   sin.sin_family = AF_INET;
   sin.sin_port = htons(port);
   sin.sin_addr.s_addr = inet_addr("0.0.0.0");
   if (bind(setup_fd, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
      return -1;
   }
   if ((listen(setup_fd, 100)) != 0) {
      return -1;
   }
   return 0;
}

/* grab a pending connection */
int listen_sock(void) {
   int sz = sizeof(fooaddr);
   
   fd = accept(setup_fd, &fooaddr, &sz);
   stats.num_listens++;
   return fd;
}

/* stop listening on a socket. */
void de_init_listen_sock(void) {
   close(setup_fd);
}

/* socket-ready readline().  From Stevens p. 280, bugfixed slightly */
int readline(char *ptr, int maxlen, int fd) {
   int n, rc;
   char c;

   for (n = 1; n < maxlen; n++) {
      if ((rc = read(fd, &c, 1)) == 1) {
	 *ptr++ = c;
	 if (c == '\n') {
	    n++;
	    break;
	 }
      } else if (rc == 0)
	   break;
      else
	return (-1);
   }
   
   *ptr = 0;
   return (n - 1);
}
