/***************************************************************************
*
*
* xgethostbyname.c
*
* Created by Belkin International, Software Engineering on XX/XX/XX.
* Copyright (c) 2012-2013 Belkin International, Inc. and/or its affiliates. All rights reserved.
*
* Belkin International, Inc. retains all right, title and interest (including all
* intellectual property rights) in and to this computer program, which is
* protected by applicable intellectual property laws.  Unless you have obtained
* a separate written license from Belkin International, Inc., you are not authorized
* to utilize all or a part of this computer program for any purpose (including
* reproduction, distribution, modification, and compilation into object code)
* and you must immediately destroy or return to Belkin International, Inc
* all copies of this computer program.  If you are licensed by Belkin International, Inc., your
* rights to utilize this computer program are limited by the terms of that license.
*
* To obtain a license, please contact Belkin International, Inc.
*
* This computer program contains trade secrets owned by Belkin International, Inc.
* and, unless unauthorized by Belkin International, Inc. in writing, you agree to
* maintain the confidentiality of this computer program and related information
* and to not disclose this computer program and related information to any
* other person or entity.
*
* THIS COMPUTER PROGRAM IS PROVIDED AS IS WITHOUT ANY WARRANTIES, AND BELKIN INTERNATIONAL, INC.
* EXPRESSLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING THE WARRANTIES OF
* MERCHANTIBILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE, AND NON-INFRINGEMENT.
*
*
***************************************************************************/
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

#ifndef _OPENWRT_
#include "osUtils.h"
#include "dns.h"
#include "GetHostByName.h"
#include "xgethostbyname.h"

#ifdef WRT11
#include <linux/if.h>
#include <sys/socket.h>
#endif

int		nameservers;
const char 	*nameserver[3];
pthread_mutex_t lookupLock;

#define	MAX_RECURSE	5

/*****************************************************************************/
static char cfgbuf[SIZE_128B];
static char *args[SIZE_16B];
struct resolv_header {
	int id;
	int qr,opcode,aa,tc,rd,ra,rcode;
	int qdcount;
	int ancount;
	int nscount;
	int arcount;
};

struct resolv_question {
	char * dotted;
	int qtype;
	int qclass;
};

struct resolv_answer {
	char * dotted;
	int atype;
	int aclass;
	int ttl;
	int rdlength;
	unsigned char * rdata;
	int rdoffset;
};
#define MAX_RETRIES 3
#define DNS_SERVICE 53
#define REPLY_TIMEOUT 5
int dns_caught_signal = 0;
void dns_catch_signal(int signo) {
	dns_caught_signal = 1;
}

static char *
ws(char **buf)
{
  char *b = *buf;
  char *p;

  /* eat ws */
  while (*b &&
	 (*b == ' '  ||
	  *b == '\n' ||
	  *b == '\t')) b++;
  p = b;

  /* find the end */
  while (*p &&
	 !(*p == ' '  ||
	   *p == '\n' ||
	   *p == '\t')) p++;
  *p = 0;
  *buf = p+1;
  return b;
}

char **
cfgread(FILE *fp)
{
  char *ebuf;
  char *p;
  int i, errno;
/*  int j;	*/

  if (!fp) {
    errno = EIO;
    return (void *)0;
  }

  while (fgets(cfgbuf, sizeof(cfgbuf), fp)) {

    /* ship comment lines */
    if (cfgbuf[0] == '#') continue;

    ebuf = cfgbuf + strlen(cfgbuf);

    p = cfgbuf;
    for (i = 0; i < 16 && p < ebuf; i++) {
      args[i] = ws(&p);
    }
    args[i] = (void *)0;

    /* return if we found something */
    if (strlen(args[0])) return args;
  }
  return (void *)0;
}

char **
cfgfind(FILE *fp, char *var)
{
  char **ret;
  char search[80];
  int errno;

  if (!fp || !var) {
    errno = EIO;
    return (void *)0;
  }

  memset(search, 0x00, sizeof(search));
  strncpy(search, var, sizeof(search)-1);

  fseek(fp, 0, SEEK_SET);
  while ((ret = cfgread(fp))) {
    if (!strcmp(ret[0], search)) return ret;
  }
  return (void *)0;
}


void open_nameservers ()
{
	FILE	*fp = NULL;
	char	**arg = NULL;
	int	i = 0;

	fp = fopen ( "/tmp/resolv.conf" , "r" );

	if ( fp != NULL ) {
		if ( (arg = cfgfind ( fp , "nameserver" )) ) {
			for ( i = 1 ; arg[i] ; i++ ) {
	                        if(nameservers>3)
					nameservers=0;	
				nameserver[nameservers] = strdup ( arg[i] );
				nameservers++;
			}
		}
		fclose ( fp );
	} else {
		printf("*********busybox Open file ... Fail************\n");
		printf("\nno /tmp/resolv.conf file");
		fflush(stdout);
	}

	return;
}

int encode_header(struct resolv_header * h, unsigned char * dest, int maxlen)
{
	if (maxlen < 12)
		return -1;

	dest[0] = (h->id & 0xff00) >> 8;
	dest[1] = (h->id & 0x00ff) >> 0;
	dest[2] = (h->qr ? 0x80 : 0) |
		  ((h->opcode & 0x0f) << 3) |
		  (h->aa ? 0x04 : 0) |
		  (h->tc ? 0x02 : 0) |
		  (h->rd ? 0x01 : 0);
	dest[3] = (h->ra ? 0x80 : 0) |
		  (h->rcode & 0x0f);
	dest[4] = (h->qdcount & 0xff00) >> 8;
	dest[5] = (h->qdcount & 0x00ff) >> 0;
	dest[6] = (h->ancount & 0xff00) >> 8;
	dest[7] = (h->ancount & 0x00ff) >> 0;
	dest[8] = (h->nscount & 0xff00) >> 8;
	dest[9] = (h->nscount & 0x00ff) >> 0;
	dest[10] = (h->arcount & 0xff00) >> 8;
	dest[11] = (h->arcount & 0x00ff) >> 0;

	return 12;
}

int decode_header(unsigned char * data, struct resolv_header * h)
{
	h->id = (data[0] << 8) | data[1];
	h->qr = (data[2] & 0x80) ? 1 : 0;
	h->opcode = (data[2] >> 3) & 0x0f;
	h->aa = (data[2] & 0x04) ? 1 : 0;
	h->tc = (data[2] & 0x02) ? 1 : 0;
	h->rd = (data[2] & 0x01) ? 1 : 0;
	h->ra = (data[3] & 0x80) ? 1 : 0;
	h->rcode = data[3] & 0x0f;
	h->qdcount = (data[4] << 8) | data[5];
	h->ancount = (data[6] << 8) | data[7];
	h->nscount = (data[8] << 8) | data[9];
	h->arcount = (data[10] << 8) | data[11];

	return 12;
}

int encode_dotted(const char * dotted, unsigned char * dest, int maxlen)
{
	int used=0;

	while(dotted && *dotted) {
		char * c = strchr(dotted, '.');
		int l = c ? c - dotted : strlen(dotted);

		if (l >= (maxlen-used-1))
			return -1;

		dest[used++] = l;
		memcpy(dest+used, dotted, l);
		used += l;

		if (c)
			dotted = c+1;
		else
			break;
	}

	if (maxlen < 1)
		return -1;

	dest[used++] = 0;

	return used;
}

int decode_dotted(const unsigned char * data, int offset,
	char * dest, int maxlen)
{
	int l;
	int measure=1;
	int total = 0;
	int used=0;

	if (!data)
		return -1;

	while ((measure && total++), (l=data[offset++])) {

		if ((l & 0xc0) == (0xc0)) {
			if (measure)
				total++;
			/* compressed item, redirect */
			offset = ((l & 0x3f) << 8) | data[offset];
			measure = 0;
			continue;
		}

		if ((used+l+1) >= maxlen)
			return -1;

		memcpy(dest+used, data+offset, l);
		offset += l;
		used += l;
		if (measure)
			total += l;

		if (data[offset] != 0)
			dest[used++] = '.';
		else
			dest[used++] = '\0';
	}

	return total;
}

int encode_question(struct resolv_question * q,
	unsigned char * dest, int maxlen)
{
	int i;

	i = encode_dotted(q->dotted, dest, maxlen);
	if (i < 0)
		return i;

	dest += i;
	maxlen -= i;

	if (maxlen < 4)
		return -1;

	dest[0] = (q->qtype & 0xff00) >> 8;
	dest[1] = (q->qtype & 0x00ff) >> 0;
	dest[2] = (q->qclass & 0xff00) >> 8;
	dest[3] = (q->qclass & 0x00ff) >> 0;

	return i+4;
}

int length_question(unsigned char * message, int offset)
{
	int i;

	i = length_dotted(message, offset);
	if (i < 0)
		return i;

	return i+4;
}

int decode_answer(unsigned char * message, int offset,
	struct resolv_answer * a)
{
	char temp[SIZE_256B];
	int i;

	i = decode_dotted(message, offset, temp, SIZE_256B);
	if (i < 0)
		return i;

	message += offset+i;

	a->dotted = strdup(temp);
	a->atype  = (message[0] << 8) | message[1]; message += 2;
	a->aclass = (message[0] << 8) | message[1]; message += 2;
	a->ttl = (message[0] << 24) |
		 (message[1] << 16) |
	 	 (message[2] << 8) |
	 	 (message[3] << 0);
	message += 4;
	a->rdlength = (message[0] << 8) | message[1]; message += 2;
	a->rdata = message;
	a->rdoffset = offset+i+10;

	return i+10+a->rdlength;
}

int length_dotted(const unsigned char * data, int offset)
{
	int orig_offset = offset;
	int l;

	if (!data)
		return -1;

	while ((l=data[offset++])) {

		if ((l & 0xc0) == (0xc0)) {
			offset++;
			break;
		}

		offset += l;
	}

	return offset-orig_offset;
}

int decode_question(unsigned char * message, int offset,
	struct resolv_question * q)
{
	char temp[SIZE_256B];
	int i;

	i = decode_dotted(message, offset, temp, SIZE_256B);
	if (i < 0)
		return i;

	offset += i;

	q->dotted = strdup(temp);
	q->qtype  = (message[offset+0] << 8) | message[offset+1];
	q->qclass = (message[offset+2] << 8) | message[offset+3];

	return i+4;
}



int dns_lookup(const char * name, int type, int nscount, const char ** nsip,
	unsigned char ** outpacket, struct resolv_answer * a)
{
    static int id=1;
    int i,j,len;
    int fd;
    int pos;
    static int ns = 0;
    struct sockaddr_in sa;
    int oldalarm;
    __sighandler_t oldhandler;
    struct resolv_header h;
    struct resolv_question q;
    int retries = 0;
    unsigned char * packet = (unsigned char*) malloc(512);
    fd_set					readfds;
    int							ioctl_opt = 1;
    struct timeval  timeoutdns,tv;
    int errno;

    if (!packet)
	goto fail1;

    if (!nscount)
	goto fail1;

    ns %= nscount;

    fd = -1;

    while (retries++ < MAX_RETRIES) {

	if (fd != -1)
	    close(fd);

	fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (fd==-1)
	    goto fail2;


	memset(packet, 0, 512);

	memset(&h, 0, sizeof(h));
	h.id = ++id;
	h.qdcount = 1;
	h.rd = 1;

	i = encode_header(&h, packet, 512);
	if (i < 0)
	    goto fail3;

	q.dotted = (char*)name;
	q.qtype = type;
	q.qclass = 1 /*CLASS_IN*/;

	j = encode_question(&q, packet+i, 512-i);
	if (j < 0)
	    goto fail3;

	len = i+j;

	sa.sin_family = AF_INET;
	sa.sin_port = htons(DNS_SERVICE);
	sa.sin_addr.s_addr = inet_addr(nsip[ns]);
	tv.tv_sec = 3;
	tv.tv_usec=0;

	if(setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
	    printf ("Set socket option for DNS lookup time out failed..\n");
	}

	if (connect(fd, (struct sockaddr*)&sa, sizeof(sa))==-1) {
	    if (errno == ENETUNREACH) {
		/* routing error, presume not transient */
		goto tryall;
	    } else
		/* retry */
		break;
	}
	send(fd, packet, len, 0);

	dns_caught_signal = 0;
	oldalarm = alarm(REPLY_TIMEOUT);
	oldhandler = signal(SIGALRM, dns_catch_signal);
	FD_ZERO(&readfds);
	FD_SET(fd,&readfds);
	ioctl_opt = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, ioctl_opt | O_NONBLOCK);
	timeoutdns.tv_sec	= REPLY_TIMEOUT;
	timeoutdns.tv_usec = 0;
	if ( select(fd+1, &readfds, NULL, NULL, &timeoutdns) <= 0 )
	    goto again;
	if(FD_ISSET(fd, &readfds))
	{
	    i = recv(fd, packet, 512, 0);

	    alarm(0);
	    signal(SIGALRM, oldhandler);
	    alarm(oldalarm);
	}
	else
	{
	    continue;	
	}	

	if (dns_caught_signal)
	{
	    /* timed out, so retry send and receive,
	       to next nameserver on queue */
	    printf("dns_caught_signal\n");
	    goto again;
	}	

	if (i < 12)
	    /* too short ! */
	    goto again;

	decode_header(packet, &h);

	if ((h.id != id) || (!h.qr))
	    /* unsolicited */
	    goto again;

	if ((h.rcode) || (h.ancount < 1)) {
	    /* negative result, not present */
	    goto tryall;
	}

	pos = 12;

	for(j=0;j<h.qdcount;j++) {
	    i = length_question(packet, pos);
	    if (i < 0)
		goto again;
	    pos += i;
	}

	i = decode_answer(packet, pos, a);

	if (i<0) {
	    goto again;
	}

	close(fd);

	if (outpacket)
	    *outpacket = packet;
	else
	    free(packet);
	return (0); /* success! */

tryall:
	/* if there are other nameservers, give them a go,
	   otherwise return with error */
	if (retries >= nscount)
	    break;
again:
	ns = (ns + 1) % nscount;
	continue;
    }


fail3:
    close(fd);
fail2:
    free(packet);
fail1:
    return -1;
}

struct hostent * MyGetHostByAddr(const char * addr, int len, int type)
{
	static struct hostent h;
	static char namebuf[SIZE_256B];
	static struct in_addr in;
	static struct in_addr *addr_list[2];
	unsigned char * packet;
	struct resolv_answer a;
	int i;
	int nest=0;
	
	if (!addr || (len != sizeof(in)) || (type != AF_INET))
		return 0;
		
	memcpy(&in.s_addr, addr, len);

	open_nameservers();
		
	memset(&h, 0, sizeof(h));
	
	addr_list[0] = &in;
	addr_list[1] = 0;
	
	snprintf(namebuf, sizeof(namebuf), "%d.%d.%d.%d.in-addr.arpa",
		(in.s_addr >> 24) & 0xff,
		(in.s_addr >> 16) & 0xff,
		(in.s_addr >> 8) & 0xff,
		(in.s_addr >> 0) & 0xff
		);
	
	for(;;) {
	
		i = dns_lookup(namebuf, 12, nameservers, nameserver, &packet, &a);
	
		if (i<0)
			return 0;
			
		memset(namebuf, 0x00, sizeof(namebuf));
		strncpy(namebuf, a.dotted, sizeof(namebuf)-1);
		free(a.dotted);
		
		if (a.atype == 5) { /* CNAME*/
			i = decode_dotted(packet, a.rdoffset, namebuf, SIZE_256B);
			free(packet);
			
			if (i <0)
				return 0;
			if (++nest > MAX_RECURSE)
				return 0;
			continue;
		} else if (a.atype == 12) { /* ADDRESS */
			i = decode_dotted(packet, a.rdoffset, namebuf, SIZE_256B);
			free(packet);
			
			h.h_name = namebuf;
			h.h_addrtype = AF_INET;
			h.h_length = sizeof(in);
			h.h_addr_list = (char**)addr_list;
			break;
		} else {
			free(packet);
			return 0;
		}
	}
		
	return &h;
}

struct hostent *MyGetHostByName ( const char *name )
{
	static struct hostent	h;
	static char		namebuf[SIZE_256B];
	static struct in_addr	in;
	static struct in_addr	*addr_list[2];
	unsigned char		*packet;
	struct resolv_answer	a;
	int			i;
	int			nest = 0;
	register const char *cp;

	if ( !name ) {
		return  NULL;
	} else {
	}


        h.h_name = 0x00;
        h.h_aliases= 0x00;
	h.h_addrtype = 0x00;
        h.h_length = 0x00; 
        h.h_addr_list = 0x00; 


	addr_list[0] = &in;
	addr_list[1] = 0;

	memset(namebuf, 0x00, sizeof(namebuf));
	strncpy(namebuf, name, sizeof(namebuf)-1);


	if (isdigit(name[0]))
		for (cp = name;; ++cp) {
			if (!*cp) {
				if (*--cp == '.')
					break;
				/*
				 * All-numeric, no dot at the end.
				 * Fake up a hostent as if we'd actually
				 * done a lookup.
				 */

			in.s_addr = inet_addr(namebuf);

			h.h_name = namebuf;
			h.h_addrtype = AF_INET;
			h.h_length = sizeof(in);
			h.h_addr_list = (char**)addr_list;

				return (&h);
			}
			if (!isdigit(*cp) && *cp != '.')
				break;
               }
	open_nameservers ();
	for(;;) {
		i = dns_lookup(namebuf, 1, nameservers, nameserver, &packet, &a);
		if (i<0)
			return	NULL;
		memset(namebuf, 0x00, sizeof(namebuf));
		strncpy(namebuf, a.dotted, sizeof(namebuf)-1);
		if(a.dotted != NULL){ free(a.dotted); a.dotted=NULL;}
		if (a.atype == 5) { /* CNAME*/
			i = decode_dotted(packet, a.rdoffset, namebuf, SIZE_256B);
			if(packet != NULL){free(packet); packet=NULL;}

			if (i <0)
				return	NULL;
			if (++nest > MAX_RECURSE)
				return NULL;
			continue;
		} else if (a.atype == 1) { /* ADDRESS */
			memcpy(&in, a.rdata, sizeof(in));
			h.h_name = namebuf;
			h.h_addrtype = AF_INET;
			h.h_length = sizeof(in);
			h.h_addr_list = (char**)addr_list;
			if(packet != NULL){free(packet); packet=NULL;}
			break;
		} else {
			if(packet != NULL){free(packet); packet=NULL;}
			return NULL;
		}
	}


	return &h;
}

#endif //#ifndef _OPENWRT_

struct hostent *xgethostbyname(const char *name)
{

	struct hostent *retval = NULL;

#ifndef _OPENWRT_
	osUtilsGetLock(&lookupLock);
	retval = MyGetHostByName(name);
	osUtilsReleaseLock(&lookupLock);
#else
	retval = gethostbyname(name);
#endif
	return retval;
}
