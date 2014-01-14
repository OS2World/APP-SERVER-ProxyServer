/* reqs.c - handle relaying between connecitons and establishing of  *
 * connections to servers. */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "sock.h"
#include "tinyproxy.h"
#include "utils.h"
#include "conns.h"
#include "log.h"

extern struct conn_s connections;
extern struct config_s config;
extern struct stat_s stats;
extern struct allowedhdr_s *allowedhdrs;
extern float load;

/* parse a client HTTP request and then establish connection. */
void clientreq(struct conn_s *connptr) {
   char inbuf[HTTPBUF];
   struct conn_s  *connptr2;
   char *p1, *p2, *t1, *t2;
   int fd, len;
   
   if (readline(inbuf, HTTPBUF - 1, connptr->fd) <= 0) {
      predel_conn(connptr);
      return;
   }
   len = strlen(inbuf);
   
   connptr->request = xmalloc(len);
   p1 = inbuf;
   if (!(p2 = strchr(p1, ' '))) {
      httperr(connptr->fd, 400, "Unintelligible request (type a)!");
      predel_conn(connptr);
      return;
   }
   *p2 = '\0';
   strcpy(connptr->request, p1);
   strcat(connptr->request, " /");
   if (!strstr(rstrtolower(p2 + 1), "http://")) {
      httperr(connptr->fd, 400, "Unintelligible request! (type b)");
      predel_conn(connptr);
      return;
   }
   p1 = p2 + 8;
   if (!(p2 = strchr(p1, '/'))) {
      httperr(connptr->fd, 400, "Unintelligible request! (type c)");
      predel_conn(connptr);
      return;
   }
   if ((!strchr(p1, ' ')) || (p2 > strchr(p1, ' '))) {
      httperr(connptr->fd, 400, "Unintelligible request! (type d)");
      predel_conn(connptr);
      return;
   }
   *p2 = '\0';
   t1 = xstrdup(p1);
   if ((t2 = strchr(t1, ':'))) {
      if (strlen(t2) < 2) {
	 httperr(connptr->fd, 400, "Unintelligible request (bad port type a)");
	 predel_conn(connptr);
	 return;
      }
      *t2 = '\0';
      t2++;
      connptr->port = atoi(t2);
   }
   connptr->host = xstrdup(t1);
   free(t1);
   
   if (((p2 + 1) - inbuf) > len) {
      httperr(connptr->fd, 400, "Unintelligible request! (type e)");
      predel_conn(connptr);
      return;
   }
   strcat(connptr->request, p2 + 1);
   
   if((strlen(config.stathost) > 0) && 
	   (!strncmp(connptr->host, config.stathost, strlen(config.stathost)))) {
      showstats(connptr->fd);
      predel_conn(connptr);
      return;
   }
   
   if ((fd = opensock(connptr->host, connptr->port)) < 0) {
      httperr(connptr->fd, 404, "Unable to connect to host!");
      predel_conn(connptr);
      stats.num_badcons++;
      return;
   }
   connptr2 = connptr->other = new_conn(ServerConn, fd);
   write(connptr2->fd, connptr->request, strlen(connptr->request));
   connptr->conntype = IgnoreConn;
   connptr2->other = connptr;
}

/* relay bytes between server and client or client and server.  censor *
 * headers if ANON mode is on. */
void relayreq(struct conn_s *connptr) {
   char inbuf[INBUF];
   int bytesin;
#ifdef ANON
   struct allowedhdr_s *allowedptr = allowedhdrs;
#endif
   
   if ((bytesin = readline(inbuf, INBUF, connptr->fd)) <= 0) {
      predel_conn(connptr);
      predel_conn(connptr->other);
      return;
   }
   connptr->actiontime = connptr->other->actiontime = time(NULL);
#ifdef ANON
   if (connptr->postheader == False) {
      if (!strncmp(inbuf, "\r\n", bytesin)) {
	 connptr->postheader = True;
	 write(connptr->other->fd, "\r\n", 2);
	 stats.num_txrx += (bytesin * 2);
      }
      for (allowedptr = allowedhdrs; allowedptr; allowedptr = allowedptr->next) {
	 if (!strncmp(inbuf, allowedptr->hdrname, strlen(allowedptr->hdrname))) {
	    write(connptr->other->fd, inbuf, bytesin);
	    stats.num_txrx += (bytesin * 2);
	 }
      }
   } else {
#endif
      write(connptr->other->fd, inbuf, bytesin);
      stats.num_txrx += (bytesin * 2);
#ifdef ANON
   }
#endif
}

/* loop that checks for new connections, dispatches to the correct *
 * routines if bytes are pending, checks to see if it's time for  * a
 * garbage collect. */
void getreqs(void) {
   extern int setup_fd;
   fd_set readfds;
   struct conn_s *connptr = &connections;
   struct timeval timeout;
   int fd;
   static int garbc = 0;
   
   if (++garbc >= GARBCOLL_INTERVAL) {
      garbcoll();
      conncoll();
      garbc = 0;
   }
   FD_ZERO(&readfds);
   
   FD_SET(setup_fd, &readfds);
   
   while (connptr) {
      if (connptr->conntype != PlaceHolder) {
	 FD_SET(connptr->fd, &readfds);
      }
      connptr = connptr->next;
   }
   
   timeout.tv_sec = 1;
   timeout.tv_usec = 0;
   
   if (!select(FD_SETSIZE, &readfds, NULL, NULL, &timeout))
     return;
   
   /* don't ask me why this is necessary.. */
   if (config.quit != False)
     return;
   
   if (FD_ISSET(setup_fd, &readfds)) {
      if ((fd = listen_sock()) >= 0) {
	 if((!config.cutoffload) || (load < config.cutoffload)) {
	    new_conn(ClientConn, fd);   
	 } else {
	    stats.num_refused++;
	    httperr(fd, 503, "tinyproxy is not accepting connections due to high system load");
	    close(fd);
	 }
      }
   }
   for (connptr = &connections; connptr; connptr = connptr->next) {
      if (connptr->conntype != PlaceHolder) {
	 if (FD_ISSET(connptr->fd, &readfds)) {
	    switch (connptr->conntype) {
	     case ClientConn:
	       clientreq(connptr);
	       break;
	     case ServerConn:
	     case IgnoreConn:
	       relayreq(connptr);
	       break;
	    }
	 }
      }
   }
}
