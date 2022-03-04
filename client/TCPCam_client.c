/* TCPCam_client.c --- TCPCam client  -*- mode: C; -*- 
 * 
 * Copyright (C) 2022, Rytis Paškauskas <rytis.paskauskas@gmail.com>
 *
 * Author: Rytis Paškauskas
 * URL: https://github.com/rytis-paskauskas/TCPCam
 * Created: 2022-03-02
 * Last modified: 
 *
 * Invocation options:
 * 1) host port pass repeat 
 * 2) host port pass repeat filename
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
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

char *program_name;

#include "lib.h" 		/* Don't forget to build with lib.o */

#define BUF_LEN 1024

int main(int argc, char *argv[])
{
    SOCKET s;
    
    FILE *fb = NULL;
    int n, rc, cnt;
    char buf[BUF_LEN], *pwd, *rep;
 
    INIT();
    if(argc == 6)		/* file name is given; write to file only */
    {
	fb = fopen(argv[5], "wb");
	assert(fb!=NULL);
    } else {
	fb = stdout;
	assert(fb!=NULL);
    }
    
    if(argc == 5 || argc == 6) {
	pwd = argv[3];
	rep = argv[4];
    } else {
	error(0,errno, "wrong invocation.\n Alternatives:\n> %s host port password count\n> %s host port password count filename\n",program_name, program_name);
	return 1;
    }
    s = tcp_client(argv[1], argv[2]);
    /* password */
    rc = send(s,pwd,strlen(pwd),0);
    if(rc<=0) {
	error(1, errno, "send failure");
    }
    /* count */
    rc = send(s,rep,strlen(rep),0);
    if(rc<=0) {
	error(1, errno, "send failure");
    }
    /* read the packet */
    uint32_t rlen=0;
    for(uint32_t i=0;;i++)
    {
	rc=readn(s,(char*)&rlen,sizeof rlen);
	if(rc!=sizeof rlen)
	    return rc<0?-1:0;
	rlen=ntohl(rlen);
	/* Read payload */
	fprintf(stderr, "Iteration %u: declared payload=%u bytes. ",i, rlen);

	if(rlen <= BUF_LEN) {	/* SMALL PAYLOAD : read in one go. */
	    rc=readn(s,buf,rlen);
	    if(rc<=0) {
		error(1, errno, "read failure");
	    }
	    fwrite(buf,rlen,1,fb);
	} else {		/* LARGE PAYLOAD : read in chunks */
	    cnt = rlen;
	    /* cur = buf; */
	    while(cnt>0)
	    {
		rc=recv(s,buf,(BUF_LEN<cnt?BUF_LEN:cnt),0);
		if(rc>0) {
		    if(rc<BUF_LEN && buf[rc-1]=='\0') {
			rlen -= 1;
			rc   -= 1;
			cnt  -= 1;
		    }
		    fwrite(buf,rc,1,fb);
		    cnt -= rc;
		} else {
		    if(errno==EINTR)
		    {
			error(0, errno, "read failure. retrying...");
			    continue;
		    }
		}
	    }
	}
	fprintf(stderr,"wrote %u bytes. done\n", rlen);
    }
    fclose(fb);
    return 0;
}
