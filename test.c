#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <unistd.h>

#define BUFLEN 512
#define PORT "8010"

void die(char *s)
{
	perror(s);
	exit(1);
}

int main (int argc, char *argv[])
{
	struct sockaddr_in si_me, si_other;
	struct addrinfo hints, *listener8010;

	int s8010, slen = sizeof(si_other) , received;
	char buf[BUFLEN];

	memset(&hints,0,sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;
	if (getaddrinfo(NULL, PORT, &hints, &listener8010) != 0)
	{
		die("getaddrinfo");
	};

	if ((s8010=socket(listener8010->ai_family, listener8010->ai_socktype, listener8010->ai_protocol)) == -1)
	{
		die("socket");
	};

	if( bind(s8010, listener8010->ai_addr, listener8010->ai_addrlen ) == -1)
	{
		die("bind");
	};
	freeaddrinfo(listener8010);

	fd_set read_fds;

	while(1)
	{
		printf("listening on port %s ...",PORT);
		fflush(stdout);
		FD_ZERO(&read_fds);
		FD_SET(s8010,&read_fds);
		if (select(s8010+1,&read_fds,NULL,NULL,NULL)==-1)
		{
			die("select");
			exit(1);
		};
		printf("\n");

		if (FD_ISSET(s8010,&read_fds))
		{
			if ((received = recvfrom(s8010, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1)
			{
				die("recvfrom");
				exit(1);
			};


			printf("received packet from %s8010:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
			printf("data: %d bytes\n" , received);
			fflush(stdout);

			if (received!=32) continue;
		};
	};

	close(s8010);
	return 0;
}
