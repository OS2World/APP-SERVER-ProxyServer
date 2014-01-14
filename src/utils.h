#ifndef _UTILS_H
#define _UTILS_H

char *xstrdup(char *st);
void *xmalloc(int sz);
char *xstrndup(char *st, int sz);
char *rstrtolower(char *st);
float getload();

#define stripnl(x) x[strlen(x)-2]='\0';

#endif /* !_UTILS_H */
