#ifndef __LIB_H_
#define __LIB_H_
void error(int status, int err, char *fmt, ... );
void verify_argc(int argc);
void * set_address ( char *host, char *port, struct sockaddr_in* sap, char *protocol);
SOCKET tcp_client( char *hname, char *sname);
SOCKET tcp_server( char *host, char *port);
int readn(SOCKET fd, char *bp, size_t len);
int readvrec(SOCKET fd, char *bp, size_t len);

#endif
