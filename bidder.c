#include "bidder.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>

int Bidding(char *filename, char *port, int num)
{
	FILE *fd;
	int udpfd;
	int n, error;
	struct sockaddr_in client_addr;
	struct sockaddr_in sa;
	//struct hostent *he;
	socklen_t sin_size;
	char ip[INET6_ADDRSTRLEN]; //ip contains the readable ip address
	char buf[256];

	udpfd = SetUDP(HOST, port, 1);

	sin_size = sizeof(sa);
	// get the socket IP and UDP port
	error = getsockname(udpfd, (struct sockaddr*)&sa, &sin_size) ; //Error checking
	if (error == -1) {
		perror("getsockname");
		exit(1);
	}
	strcpy(ip, inet_ntoa(sa.sin_addr));
	// bidder Phase1 on-screen MSG

	// recieve broadlist
	sin_size = sizeof(client_addr);
	if((n = recvfrom(udpfd, buf, 255, 0, (struct sockaddr *)&client_addr, &sin_size)) == -1) {
		perror("recvfrom");
		exit(1);
	}
	printf("\nPhase 3: <Bidder%d> has UDP port number %s and IP address %s\n", num, port, ip);
	buf[n] = '\0';
	printf("Phase3:\n%s\n", buf);
	// send bidding information
	if (num == 1) {
		if ((fd = fopen("test/bidding1.txt", "r")) == NULL) {
			fprintf(stderr, "Phase3: can't open bidding1.txt\n");
			exit(1);
		}
	} else {
		if ((fd = fopen("test/bidding2.txt", "r")) == NULL) {
			fprintf(stderr, "Phase3: can't open bidding2.txt\n");
			exit(1);
		}

	}

	fseek(fd, 0, SEEK_END);
	long fsize = ftell(fd);
	fseek(fd, 0, SEEK_SET);

	char *msg = malloc(fsize + 1);
	fread(msg, fsize, 1, fd);
	fclose(fd);
	msg[fsize] = '\0';
	printf("Phase 3: <Bidder%d>\n%s\n", num, msg);

	sprintf(buf, "%d\n%s", num, msg);


	//he = gethostbyaddr((const char *)&client_addr.sin_addr.s_addr, sizeof(client_addr.sin_addr.s_addr), AF_INET);
	sin_size = sizeof(client_addr);

	sleep(1);
	if ((n = sendto(udpfd, buf, 255, 0, (struct sockaddr*)&client_addr, sin_size)) == -1) {
		perror("sendto");
		exit(1);
	}
	close(udpfd);

	return 0;

}
int main(int argc, char *argv[])
{
	pid_t pid;
	User bidder1, bidder2;
	if ((pid = fork()) == 0) { // bidder2 child process
		Login("test/bidderPass1.txt", &bidder1, 1);
		sleep(20);
		Bidding("test/bidding1.txt", B_UDP_PORT1, 1);
		GetResult(B_TCP_PORT1);
		printf("End of Phase 3 for Bidder.\n");
	} else { // bidder1 parent process
		Login("test/bidderPass2.txt", &bidder2, 2);
		sleep(20);
		Bidding("test/bidding2.txt", B_UDP_PORT2, 2);
		GetResult(B_TCP_PORT2);
		printf("End of Phase 3 for Bidder.\n");
	}
	return 0;
}
