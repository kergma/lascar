#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <unistd.h>

#define BUFLEN 512
#define PORT8010 "8010"
#define PORT10080 "10080"

#define MAX(a,b) ((a)>(b)?(a):(b))

void die(char *s)
{
	perror(s);
	exit(1);
}
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in*)sa)->sin_addr);
	};

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
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
	char remoteIP[INET6_ADDRSTRLEN];

	int s8010, s10080, received;
	char buf[BUFLEN];

	s8010=create_listener(SOCK_DGRAM,PORT8010);

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
		if (select(fdmax+1,&read_fds,NULL,NULL,NULL)==-1)
		{
			die("select");
			exit(1);
		};
		printf("\n");
		for (int i=0;i<=fdmax;i++)
		{
			if (!FD_ISSET(i,&read_fds)) continue;
			if (i==s8010)
			{
				if ((received = recvfrom(i, buf, BUFLEN, 0, (struct sockaddr *) &remoteaddr, &addrlen)) == -1)
				{
					die("recvfrom");
					exit(1);
				};

				printf("received UDP packet from %s:%d\n",inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr*)&remoteaddr), remoteIP, INET6_ADDRSTRLEN),ntohs(((struct sockaddr_in*)&remoteaddr)->sin_port));
				printf("data: %d bytes\n" , received);
				fflush(stdout);
			}
			else if (i==s10080)
			{
				int newfd = accept(i, (struct sockaddr *)&remoteaddr, &addrlen);
				if (newfd==-1) die("accept");

				FD_SET(newfd,&master_fds);
				fdmax=MAX(fdmax,newfd);
				printf("TCP connection from %s:%d on socket %d\n",inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr*)&remoteaddr), remoteIP, INET6_ADDRSTRLEN),ntohs(((struct sockaddr_in*)&remoteaddr)->sin_port),newfd);
			}
			else if ((received = recv(i, buf, BUFLEN, 0)) <= 0)
			{
				if (received==0)
					printf("TCP connection closed on socket %d\n",i);
				else
					perror("recv");
				close(i);
				FD_CLR(i,&master_fds);
			}
			else
			{
				printf("received %d bytes from socket %d\n",received,i);
			};
		};


	};

	close(s8010);
	return 0;
}
