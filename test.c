#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <unistd.h>
#include <time.h>

#include "session.h"

#define BUFLEN 2048
#define PORT8010 "8010"
#define PORT10080 "10080"

#define MAX(a,b) ((a)>(b)?(a):(b))

struct device_announce
{
	unsigned char signature[2];
	unsigned char unk2[2];
	unsigned char ff6[6];
	unsigned char ping7[7];
	unsigned char unk3[3];
	unsigned char mac[6];
	unsigned char unkown[6];
} __attribute__((__packed__));

struct announce_reply
{
	unsigned char signature[2];
	unsigned char unk2[2];
	unsigned char device_mac[6];
	unsigned char pong7[7];
	unsigned char unk7[7];
	struct in_addr ip;
	unsigned char zero;
} __attribute__((__packed__));

void die(char *s)
{
	perror(s);
	exit(1);
}

char *format_address(struct sockaddr *a)
{
	static char addrs[INET6_ADDRSTRLEN];
	inet_ntop(a->sa_family,a->sa_family==AF_INET?(void*)&(((struct sockaddr_in*)a)->sin_addr):(void*)&(((struct sockaddr_in6*)a)->sin6_addr),addrs,sizeof addrs);
	return addrs;
}

int create_listener(int socktype, const char *service)
{
	struct addrinfo hints, *listener;
	int s;

	memset(&hints,0,sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = socktype;
	hints.ai_flags = AI_PASSIVE;
	if (getaddrinfo(NULL, service, &hints, &listener) != 0)
	{
		die("getaddrinfo");
	};
	if ((s=socket(listener->ai_family, listener->ai_socktype, listener->ai_protocol)) == -1) die("socket");
	int yes=1;
	if (setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes) == -1) die("setsockopt");
	if (bind(s, listener->ai_addr, listener->ai_addrlen ) == -1) die("bind");
	if (listener->ai_socktype==SOCK_STREAM)
		if (listen(s, 10) == -1) die("listen");
	freeaddrinfo(listener);
	return s;

}
int main (int argc, char *argv[])
{
	struct sockaddr_storage remoteaddr;
	socklen_t addrlen= sizeof remoteaddr;

	int s8010, s10080, received, sent;
	char buf[BUFLEN];

	s8010=create_listener(SOCK_DGRAM,PORT8010);
	int yes=1;
	if (setsockopt(s8010,IPPROTO_IP,IP_PKTINFO,&yes,sizeof yes)==-1) die("setsockopt");

	s10080=create_listener(SOCK_STREAM,PORT10080);

	fd_set read_fds, master_fds;
	int fdmax=MAX(s8010,s10080);
	FD_ZERO(&master_fds);
	FD_SET(s8010,&master_fds);
	FD_SET(s10080,&master_fds);

	printf("s8010 %d s10080 %d\n",s8010,s10080);

	while(1)
	{
		printf("listening on ports %s, %s ...",PORT8010,PORT10080);
		fflush(stdout);
		read_fds=master_fds;
		if (select(fdmax+1,&read_fds,NULL,NULL,NULL)==-1) die("select");
		printf("\n");

		#define TIMESLEN 20
		char times[TIMESLEN];
		time_t t=time(NULL);
		struct tm tb;
		localtime_r(&t,&tb);
		strftime(times,TIMESLEN,"%Y-%m-%d %H:%M:%S",&tb);

		for (int i=0;i<=fdmax;i++)
		{
			if (!FD_ISSET(i,&read_fds)) continue;
			if (i==s8010)
			{
				struct iovec v[]={{buf,BUFLEN}};
				#define CONTROLLEN 40
				char controlbuf[CONTROLLEN];
				struct msghdr mh={&remoteaddr,sizeof remoteaddr, v,1,controlbuf,CONTROLLEN,0};

				if ((received = recvmsg(i, &mh,0))==-1) die("recvmsg");

				printf("%s received UDP packet from %s:%d\n",times,format_address((struct sockaddr*)&remoteaddr),ntohs(((struct sockaddr_in*)&remoteaddr)->sin_port));
				printf("data: %d bytes\n" , received);

				struct cmsghdr *cmsg;
				struct in_pktinfo *pi=NULL;
				for (cmsg=CMSG_FIRSTHDR(&mh); cmsg!=NULL; cmsg=CMSG_NXTHDR(&mh,cmsg))
				{
					if (cmsg->cmsg_level==IPPROTO_IP && cmsg->cmsg_type==IP_PKTINFO)
					{
						pi=(struct in_pktinfo*)CMSG_DATA(cmsg);
						break;
					};

				};
				if (!pi)
				{
					printf("could not get ancillary packet data\n");
					exit(1);
				};

				fflush(stdout);
				session *s=create_session(i);
				session_eat(s,buf,received);
				free_session(s);

				if (received!=sizeof(struct device_announce)) continue;

				struct device_announce a;
				memmove(&a,buf,sizeof(a));

				struct announce_reply r={{0,0},{0x20,0x01},{0,0,0,0,0,0},{0,0,0,0,0,0,0},{0x0f,0x80,0x01,0x65,0x05,0x80,0x0d},pi->ipi_spec_dst,0};
				memmove(&r.signature,&a.signature,sizeof(r.signature));
				memmove(&r.device_mac,&a.mac,sizeof(r.device_mac));
				memmove(&r.pong7,&a.ping7,sizeof(r.pong7));
				r.unk7[3]=a.ping7[1];

				if ((sent=sendto(s8010, &r, sizeof(r), 0, (struct sockaddr*) &remoteaddr, addrlen)) == -1) die("sendto");
				printf("reply sent: %d bytes\n" , sent);
			}
			else if (i==s10080)
			{
				int newfd = accept(i, (struct sockaddr *)&remoteaddr, &addrlen);
				if (newfd==-1) die("accept");

				FD_SET(newfd,&master_fds);
				fdmax=MAX(fdmax,newfd);
				printf("%s TCP connection from %s:%d on socket %d\n",times,format_address((struct sockaddr*)&remoteaddr),ntohs(((struct sockaddr_in*)&remoteaddr)->sin_port),newfd);

				#define FILENAMELEN 1024
				char filename[FILENAMELEN];
				strftime(times,TIMESLEN,"%Y%m%d-%H%M%S",&tb);
				snprintf(filename,FILENAMELEN,"%s.lascar",times);
				printf("filename %s\n",filename);
				create_session_writer(newfd,filename);
			}
			else if ((received = recv(i, buf, BUFLEN, 0)) <= 0)
			{
				if (received==0)
					printf("%s TCP connection closed on socket %d\n",times,i);
				else
					perror("recv");
				close(i);
				FD_CLR(i,&master_fds);
				close_session_writer(i);
			}
			else
			{
				printf("%s received %d bytes from socket %d\n",times,received,i);
				write_session_data(i,buf,received);
			};
		};


	};

	close(s8010);
	return 0;
}
