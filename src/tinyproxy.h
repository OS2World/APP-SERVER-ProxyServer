#ifndef _TINYPROXY_H
#define _TINYPROXY_H

/* change these if you want */
#define DEFAULT_LOG  "tinyproxy.log" /* default log file */
#define DEFAULT_PORT 8080       /* default port tinyproxy listens on */
#define DEFAULT_USER "nobody"   /* default user to change to after
                                 * startup. "" = disabled by default. */
#undef ANON                     /* define this if you want no header
				 * forwarding except for headers 
				 * expressly permitted with -a. */
//#define USE_PROC 		/* Define this if you want tinyproxy
				 //* to use /proc/loadavg to determine
//				 * system load (Linux only, I think) */
#ifndef USE_PROC
#define UPTIME_PATH "uptime"  /* path to uptime to determine
					* system load.  This path
                                        * doesn't have to be valid if
                                        * DEFAULT_CUTOFFLOAD is 0 */
#endif /* !USE_PROC */
#define DEFAULT_CUTOFFLOAD 0    /* the default load at which tinyproxy will 
				 * start refusing connections.  0 = disabled
				 * by default. */
/* note for DEFAULT_STATHOST:  this controls remote proxy stats display.
 * for example, the default DEFAULT_STATHOST of "tinyproxy.stats" will
 * mean that when you use the proxy to access http://tinyproxy.stats/",
 * you will be shown the proxy stats.  Set this to something obscure
 * if you don't want random people to be able to see them, or set it to
 * "" to disable.  In the future, I figure maybe some sort of auth
 * might be desirable, but that would involve a major simplicity
 * sacrifice. */
#define DEFAULT_STATHOST "tinyproxy.stats"
                                /* the "hostname" for getting tinyproxy
                                 * stats. "" = disabled by default. */

/* change these if you know what you're doing */
#define SOCK_TIMEOUT 5
#define LOAD_RECALCTIMER 30     /* recalculate load every 30s */
#define HTTP_PORT 80		/* default HTTP port */
#define GARBCOLL_INTERVAL 100	/* once every 100 hits */
#define STALECONN_TIME 60	/* every time conncoll is called, nuke * 
				 * connections idle for > 60s */

/* don't touch anything past this line */
#define VERSION "tinyproxy 1.1"
/* make this long (for absurd GET requests) */
#define HTTPBUF 8192
/* max inbuf read */
#define INBUF 1460
#define flag char

/* different connection types */
enum {
    ClientConn, ServerConn, PlaceHolder, IgnoreConn
};

/* other stuff */
enum {
    False, True
};

struct config_s {
    FILE *logf;
    char *logf_name;
    float cutoffload;
    int port;
    char *stathost;
    flag quit;
    char *changeuser;
};

struct stat_s {
    unsigned long num_reqs;
    unsigned long num_cons;
    unsigned long num_badcons;
    unsigned long num_opens;
    unsigned long num_listens;
    unsigned long num_txrx;
    unsigned long num_garbage;
    unsigned long num_idles;
    unsigned long num_refused;
};

struct conn_s {
    int fd;
    char *host;
    int port;
    char *request;
    flag conntype;
    flag killme;
#ifdef ANON
    flag postheader;
#endif
    struct conn_s *next, *prev, *other;
    time_t inittime, actiontime;
};

#ifdef ANON
struct allowedhdr_s {
    char *hdrname;
    struct allowedhdr_s *next;
};
#endif

/* last, function decls. */
void            httperr(int fd, int err, char *msg);

#endif /* !_TINYPROXY_H */
