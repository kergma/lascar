#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFLEN 512
#define PORT 8010

void die(char *s)
{
	perror(s);
	exit(1);
}

int main (int argc, char *argv[])
{
	struct sockaddr_in si_me, si_other;

	int s, i, slen = sizeof(si_other) , received;
	char buf[BUFLEN];

	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		die("cannot create socket");
	};

	memset((char *) &si_me, 0, sizeof(si_me));

	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	if( bind(s, (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
	{
		die("cannot bind socket");
	};

	while(1)
	{
		printf("listening on port %d ...",PORT);
		fflush(stdout);

		if ((received = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1)
		{
			die("recvfrom()");
		};

		printf("received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
		printf("data: %d bytes\n" , received);

	};

	close(s);
	return 0;
}
