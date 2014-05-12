#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>


/* setup a TCP connection and return the socket
 * node: IP or nunki.usc.edu or NULL
 * port: TCP port
 * type: 1 server, 0 client
 */
int SetTCP(const char *node, const char *port, int type)
{
	   int tcpfd;
	   struct sockaddr_in	server_addr;
	   int yes = 1;
	   if((tcpfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		   perror("socket");
	   exit(1);
	   }
	   


	   struct hostent *host;
	   host = gethostbyname(node);
	   if (!host){
		   fprintf(stderr, "Could not resolve host name\n");
		   exit(1);
	   }

	   server_addr.sin_family = AF_INET;
	   server_addr.sin_port = htons(atoi(port));
	   server_addr.sin_addr.s_addr = inet_addr(node);
	   //server_addr.sin_addr.s_addr = inet_addr(host->h_addr);

	//bind with the socket
	if (type) {
	if (setsockopt(tcpfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
	perror("setsockopt");
	exit(1);
	}
	if(bind(tcpfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1){
	perror("bind");
	exit(1);
	}
	//listen
	if(listen(tcpfd, BACKLOG) == -1){
	perror("listen");

	exit(1);

	}
	} else {
	if (connect(tcpfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
	perror("connect");
	exit(1);
	}

	}

	return tcpfd;
	/*
	struct addrinfo hints, *servinfo, *p;
	int yes = 1;
	int rv, tcpfd;

	// initialize the hints for the getaddrinfo() to get the "struct addrinfo *list"
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if (type) {
		hints.ai_flags = AI_PASSIVE;
	}
	if ((rv = getaddrinfo(node, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
	}
	// loop through the results to find the first one we can connect to
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((tcpfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}
		// type = 1 server, reuse the static port bind the socket
		if (type) {
			if (setsockopt(tcpfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
				perror("setsockopt");
				exit(1);
			}
			if (bind(tcpfd, p->ai_addr, p->ai_addrlen) == -1) {
				close(tcpfd);
				perror("server: bind");
				continue;
			}
			break;
		} else {	//type = 0 client, connect to the socket
			if (connect(tcpfd, p->ai_addr, p->ai_addrlen) == -1) {
				close(tcpfd);
				perror("client: connect");
				continue;
			}
			break;
		}
	}
	if (type) {
		if (p == NULL) {
			fprintf(stderr, "server: failed to bind\n");
			return -1;
		} else {
			if (listen(tcpfd, BACKLOG) == -1) {
				perror("listen");
				exit(1);
			}
		}
	} else {
		if (p == NULL) {
			fprintf(stderr, "client: failed to connect\n");
			return -1;
		}
	}
	freeaddrinfo(servinfo);
	return tcpfd;
	*/
}

int LoadUserPass(char *filename, User *user)
{
	FILE *fd;
	// open the Registration.txt
	if ((fd = fopen(filename, "r")) == NULL) {
		fprintf(stderr, "Phase1: user can't open %s\n", filename);
		exit(1);
	}
	fscanf(fd, "%d %s %s %d", &user->type, user->name, user->password, &user->account);
	//	printf("%d %s %s %d\n", bidder->type, bidder->name, bidder->password, bidder->account);
	fclose(fd);
	if(user->account < 451900000 || user->account > 451999999) {
		fprintf(stderr, "Phaser1: Invalide account information\n");
		return -1;
	}
	return 0;
}

int SetUDP(const char *node, const char *port, int type)
{
	/*
	   int udpfd;
	   int yes = 1;
	   struct sockaddr_in addr;
	   bzero(&addr, sizeof(struct sockaddr_in));
	   addr.sin_family = AF_INET;
	   addr.sin_port = htons(atoi(port));
	   addr.sin_addr.s_addr = inet_addr(node);

	//set up socket

	if((udpfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
	perror("sock");
	exit(1);
	}

	//bind with the local address
	if (type) {
	if (setsockopt(udpfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
	perror("setsockopt");
	exit(1);
	}
	if(bind(udpfd, (struct sockaddr*)&addr, sizeof(addr)) < 0){
	perror("bind");
	exit(1);
	}
	}

	return udpfd;
	 */

	struct addrinfo hints, *servinfo, *p;
	int rv, udpfd;
	int yes = 1;
	// initialize the hints for the getaddrinfo() to get the "struct addrinfo *list"
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	if (type) {
		hints.ai_flags = AI_PASSIVE;
	}
	if ((rv = getaddrinfo(node, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
	}
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((udpfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}
		// type = 1 server, reuse the static port bind the socket
		if (type) {
			if (setsockopt(udpfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
				perror("setsockopt");
				exit(1);
			}
			if (bind(udpfd, p->ai_addr, p->ai_addrlen) == -1) {
				close(udpfd);
				perror("server: bind");
				continue;
			}
		} else {
			if (connect(udpfd, p->ai_addr, p->ai_addrlen) == -1) {
				close(udpfd);
				perror("client: connect");
				continue;
			}
		}
		break;
	}
	if (p == NULL) {
		fprintf(stderr, "server: failed to bind\n");
		return -1;
	}
	freeaddrinfo(servinfo);
	return udpfd;
}

int Login(char *filename, User *user, int num)
{
	int error, n, tcpfd;
	struct sockaddr_in sa;
	socklen_t sin_size;
	char buf[128];
	error = LoadUserPass(filename, user);
	if (error == -1) {
		return -1;
	}
	// connect to nunki.usc.edu
	tcpfd = SetTCP(HOST, PORT1, 0);
	if (tcpfd == -1) {
		fprintf(stderr, "Can't set up a socket!\n");
		exit(1);
	}
	sin_size = sizeof(sa);
	// get the socket IP and TCP port
	error = getsockname(tcpfd, (struct sockaddr*)&sa, &sin_size) ; //Error checking
	if (error == -1) {
		perror("getsockname");
		exit(1);
	}
	// bidder/seller Phase1 on-screen MSG
	printf("\nPhase 1: <%s%d> has TCP port %d and IP address: %s\n",(user->type== 1)?"Bidder":"seller", num, (int) ntohs(sa.sin_port) ,inet_ntoa(sa.sin_addr));
	printf("\nPhase 1: Login request. User:%s password:%s Bank account:%d\n", user->name, user->password, user->account);
	// send login info to server
	n = sprintf(buf, "Login#%d %s %s %d", user->type, user->name, user->password, user->account);
	if (write(tcpfd, buf, n) < 0) {
		close(tcpfd);
		return -1;
	}

	if ((n = read(tcpfd, buf, n)) < 0) {
		close(tcpfd);
		return -1;
	}
	buf[n] = '\0';
	printf("\nPhase 1: Login request reply: %s\n", buf);
	// if seller is accepted, send the seller num
	if (strcmp(buf, "Rejected#") == 0) {
		return -1;
	}
	if (strcmp(buf, "Accepted#") == 0 && user->type == 2) {
		n = sprintf(buf, "Seller#%d", num);
		if (write(tcpfd, buf, n) < 0) {
			close(tcpfd);
			return -1;
		}
		// instead of getting ip from server,just use the client IP XD
		printf("\nPhase 1: Auction Server has IP Address: %s and PreAuction TCP Port Number: %s\n", inet_ntoa(sa.sin_addr), PORT2);
	}
	return 0;
}

int GetResult(char *port)
{
	char buf[256];
	int n;
	int tcpfd, clientfd;
	struct sockaddr_storage client_addr;
	socklen_t sin_size;

	tcpfd = SetTCP(HOST, port, 1);
	if (tcpfd == -1) {
		fprintf(stderr, "Can't set up a socket!\n");
		exit(1);
	}
	sin_size = sizeof(client_addr);
	clientfd = accept(tcpfd, (struct sockaddr *)&client_addr, &sin_size);
	if ((n = recv(clientfd, buf, sizeof(buf), 0)) < 0) {
		perror("recv");
		exit(1);
	}
	buf[n] = '\0';
	printf("\n%s\n", buf);
	close(clientfd);
	close(tcpfd);
	return 0;
}
/* IPv4, get sockaddr_in/sockaddr_in6 */
/*
   void *GetAddr(struct sockaddr *addr)
   {
   return &(((struct sockaddr_in *)addr)->sin_addr);
   }
 */

