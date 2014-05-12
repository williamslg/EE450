#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<netinet/in.h>

int main(int argc, char *argv[])
{
	int status;
	struct addrinfo hints, *res, *p;
	char ip[INET6_ADDRSTRLEN];
	char *ipver;
	void *addr;

	if (argc != 2) {
		fprintf(stderr, "usage: showip hostname\n");
		return 1;
	}

	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	
	if ((status = getaddrinfo(argv[1], NULL, &hints, &res)) != 0) {
		fprintf(stderr, "getaddrinfo():%s\n", gai_strerror(status));
		return 2;
	}

	for (p = res; p != NULL; p = p->ai_next) {
		if (p->ai_family == AF_INET) {
			ipver = "IPv4";
			addr = &((struct sockaddr_in *)p->ai_addr)->sin_addr;
		}
		else {
			ipver = "IPv6";
			addr = &((struct sockaddr_in6 *)p->ai_addr)->sin6_addr;
		}
		inet_ntop(p->ai_family, addr, ip, sizeof(ip));//INET6_ADDRSTRLEN);
		printf("%s: %s\n", ipver, ip);
	}
	freeaddrinfo(res);
	return 0;
}
