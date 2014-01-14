#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <sysexits.h>
#include "tinyproxy.h"
#include "log.h"

extern struct config_s config;
extern struct stat_s stats;
extern float load;

/* for-sure strdup */
char *xstrdup(char *st) {
   char *p;
   
   if (!(p = strdup(st))) {
      log(config.logf, "xstrdup: out of memory!");
      exit(EX_OSERR);
   }
   return (p);
}

char *xstrndup(char *st, int sz) {
   char *p;
   
   if (!(p = malloc(sz + 1))) {
      log(config.logf, "xstrdup: out of memory!");
      exit(EX_OSERR);
   }
   strncpy(p, st, sz);
   return (p);
}

/* for-sure malloc */
void *xmalloc(int sz) {
   char *p;
   
   if (!(p = (char *) malloc(sz))) {
      log(config.logf, "xmalloc: out of memory!");
      exit(EX_OSERR);
   }
   return (p);
}

char *rstrtolower(char *st) {
   char *p = st;
   static char buf[HTTPBUF];
   
   while (*p) {
      buf[p - st] = tolower(*p);
      p++;
   }
   return (buf);
}

#ifdef USE_PROC
void calcload(void) {
   char buf[HTTPBUF], *p;
   FILE *f;
   
   if(!(f=fopen("/proc/loadavg", "rt"))) {
      log(config.logf, "unable to read /proc/loadavg!");
      load=0;
      return;
   }
   fgets(buf, HTTPBUF, f);
   p=strchr(buf, ' ');
   *p='\0';
   sscanf(buf, "%f", &load);
   fclose(f);
}
#else
void calcload(void) {
   FILE *f;
   char buf[HTTPBUF];
   char *p, *y;
   
   if(!(f=popen(UPTIME_PATH, "r"))) {
      log(config.logf, "calcload: unable to exec uptime!");
      load=0;
      return;
   }
   fgets(buf, HTTPBUF, f);
   p=strrchr(buf, ':');
   p+=2;
   y=strchr(p, ',');
   *y='\0';   
   sscanf(p, "%f", &load);
   pclose(f);
}
#endif

void showstats(int fd) {
   char *msg = 
"HTTP/1.0 200 OK\r\n\
Content-type: text/html\r\n\r\n\
<html><head><title>%s stats</title></head>\r\n\
<body>\r\n\
<center><h2>%s run-time statistics</h2></center><hr>\r\n\
<blockquote>\r\n\
Number of requests: %lu<br>\r\n\
Number of connections: %lu<br>\r\n\
Number of bad connections: %lu<br>\r\n\
Number of opens: %lu<br>\r\n\
Number of listens: %lu<br>\r\n\
Number of bytes (tx+rx): %lu<br>\r\n\
Number of garbage collects: %lu<br>\r\n\
Number of idle connection kills: %lu<br>\r\n\
Number of refused connections due to high load: %lu<br>\r\n\
Current system load average: %.2f (recalculated every %lu seconds)<br>\r\n\
</blockquote>\r\n\
</body></html>\r\n";
   char buf[HTTPBUF];
   
   sprintf(buf, msg, VERSION, VERSION, stats.num_reqs, stats.num_cons,
           stats.num_badcons, stats.num_opens, stats.num_listens,
           stats.num_txrx, stats.num_garbage, stats.num_idles,
           stats.num_refused, load, LOAD_RECALCTIMER);
   write(fd, buf, strlen(buf));
}
