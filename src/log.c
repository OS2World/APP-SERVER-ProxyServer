/* log.c - for the manipulation of log files. */
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

/* this routine logs a message to the logfile.  perhaps syslogd might be 
 * a nice option to have? */
void log(FILE * logf, char *fmt,...) {
   va_list args;
   time_t nowtime = time(NULL);
   char *p = ctime(&nowtime);
   FILE *cf;
   
   p[strlen(p) - 1] = '\0';
   
   if (!(cf = logf))
     cf = stderr;
   
   va_start(args, fmt);
   fprintf(cf, "%s: ", p);
   vfprintf(cf, fmt, args);
   fprintf(cf, "\n");
   fflush(cf);
   va_end(args);
}
