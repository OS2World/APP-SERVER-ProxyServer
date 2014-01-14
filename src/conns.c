/* conns.c - these routines handle the linked list of active connections. */
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include "tinyproxy.h"
#include "log.h"
#include "utils.h"

extern struct conn_s connections;
extern struct config_s config;
extern struct stat_s stats;

/* add a new connection to the linked list */
struct conn_s *new_conn(flag conntype, int fd) {
   struct conn_s  *connptr = &connections;
   struct conn_s  *connptr2;
   
   for (; connptr->next; connptr = connptr->next);;
   
   connptr2 = connptr;
   connptr2->next = connptr = xmalloc(sizeof(struct conn_s));
   
   connptr->fd = fd;
   connptr->host = connptr->request = NULL;
   connptr->conntype = conntype;
   connptr->next = NULL;
   connptr->prev = connptr2;
   connptr->other = NULL;
   connptr->inittime = time(NULL);
   connptr->actiontime = time(NULL);
   connptr->killme = False;
#ifdef ANON
   if (conntype == ServerConn) {
      connptr->postheader = True;
   } else {
      connptr->postheader = False;
   }
#endif
   connptr->port = HTTP_PORT;
   stats.num_cons++;
   
   return (connptr);
}

/* delete a connection from the linked list */
void del_conn(struct conn_s *delconn) {
   if (!delconn) {
      log(config.logf, "internal error: del_conn a null ptr!");
      return;
   }
   delconn->prev->next = delconn->next;
   if (delconn->next)
     delconn->next->prev = delconn->prev;
   free(delconn);
}

/* flag a connection for deletion on the next garbage collect */
void predel_conn(struct conn_s *delconn) {
   close(delconn->fd);
   if (delconn->host && delconn->request)
     stats.num_reqs++;
   if (delconn->host)
     free(delconn->host);
   if (delconn->request)
     free(delconn->request);
   
   delconn->killme = True;
   delconn->conntype = PlaceHolder;
}

/* check for connections that have been idle too long */
void conncoll(void) {
   struct conn_s  *connptr = &connections;
   
   while (connptr) {
      if (((time(NULL) - connptr->actiontime) > STALECONN_TIME) && (connptr->conntype != PlaceHolder)) {
	 predel_conn(connptr);
	 stats.num_idles++;
	 connptr = &connections;
      } else {
	 connptr = connptr->next;
      }
   }
}

/* actually remove all entries in the linked list that have been marked *
 * for deletion. */
void garbcoll(void) {
   struct conn_s  *connptr = &connections;
   
   stats.num_garbage++;
   
   while (connptr) {
      if (connptr->killme == True) {
	 del_conn(connptr);
	 connptr = &connections;
      } else {
	 connptr = connptr->next;
      }
   }
}
