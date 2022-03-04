#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "skel.h"

extern char* program_name;

void error(int status, int err, char *fmt, ... )
{
    va_list ap;

    va_start(ap, fmt);
    fprintf(stderr,"%s: ", program_name);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    if(err)
	fprintf(stderr, ": %s (%d)\n", strerror(err), err);
    if(status)
	exit(status);
}

void verify_argc(int argc)
{
    if (argc==1)
    {
	error(1, errno, "wrong number of arguments (forgot to set port?)\nUsage: %s [host=localhost] port\n", program_name);
	exit(1);
    }
}


void * set_address ( char *hname, char *sname, struct sockaddr_in* sap, char *protocol)
{
    struct servent *sp;
    struct hostent *hp;
    char *endptr;
    short port;

    bzero( sap, sizeof(*sap));
    sap->sin_family = AF_INET;
    if(hname!= NULL){
	if( !inet_aton(hname,&sap->sin_addr))
	{
	    hp=gethostbyname(hname);
	    if(hp==NULL)
		error(1,0,"unknown host: %s\n", hname);
	    sap->sin_addr = *(struct in_addr*)hp->h_addr;
	}
    }
    else
	sap->sin_addr.s_addr = htonl(INADDR_ANY);
    port = strtol(sname, &endptr, 0);
    if( *endptr == '\0')
	sap->sin_port = htons(port);
    else
    {
	sp = getservbyname(sname, protocol);
	if(sp==NULL)
	    error(1,0,"unknown service: %s\n",sname);
	sap->sin_port=sp->s_port;
    }
}
SOCKET tcp_client( char *hname, char *sname)
{
    struct sockaddr_in peer;
    SOCKET s;
    set_address(hname, sname, &peer, "tcp");
    s=socket( AF_INET, SOCK_STREAM, 0);
    if(!isvalidsock(s))
	error(1, errno, "socket call failed");
    if(connect(s,(struct sockaddr*)&peer, sizeof peer))
	error(1, errno, "connect failed");
    return s;
}
SOCKET tcp_server( char *hname, char *sname)
{
    struct sockaddr_in local;
    SOCKET s;
    const int on = 1;
    set_address(hname, sname, &local, "tcp");
    s = socket( AF_INET, SOCK_STREAM, 0);
    if(!isvalidsock(s))
	error(1, errno, "socket call failed");

    if(setsockopt( s, SOL_SOCKET,SO_REUSEADDR, &on, sizeof on))
	error(1, errno, "setsockopt failed");

    if(bind(s,(struct sockaddr*)&local, sizeof local))
	error(1, errno, "bind failed");

    if(listen(s, 0))
	error(1, errno, "listen failed");

    return s;
}

int readn(SOCKET fd, char *bp, size_t len)
{
    int cnt;
    int rc;

    cnt = len;
    while(cnt>0)
    {
	rc=recv(fd,bp,cnt,0);
	if(rc<0)
	{
	    if(errno==EINTR)
		continue;
	    return -1;
	}
	if(rc==0)
	    return len - cnt;
	bp += rc;
	cnt -= rc;
    }
    return len;
}

int readvrec(SOCKET fd, char *bp, size_t len)
{
    uint32_t reclen;
    int rc;
    rc=readn(fd,(char*)&reclen,sizeof reclen);
    if(rc!=sizeof reclen)
	return rc<0?-1:0;
    reclen=ntohl(reclen);
    if(reclen>len)
    {
	while(reclen>0)
	{
	    rc=readn(fd,bp,len);
	    if(rc!=len)
		return rc<0?-1:0;
	    reclen -= len;
	    if(reclen<len)
		len=reclen;
	}
	set_errno(EMSGSIZE);
	return -1;
    }
    /* retrieve the record */
    rc=readn(fd,bp,reclen);
    if(rc!=reclen)
	return rc<0?-1:0;
    return rc;
}
