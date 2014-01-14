#ifndef _CONN_H
#define _CONN_H
struct conn_s *new_conn(flag conntype, int fd);
void del_conn(struct conn_s *connptr);
void conncoll(void);
void garbcoll(void);
void predel_conn(struct conn_s *connptr);
#endif /* !_CONN_H */
