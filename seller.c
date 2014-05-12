#include "seller.h"

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

int SendItems(char *filename, int num)
{
	FILE *fd;
	char buf[256];	// buffer of itemlist.txt
	//char msg[256];	// msg to server
	//char owner[32];	// owner of the item
	int tcpfd;
	if ((fd = fopen(filename, "r")) == NULL) {
		fprintf(stderr, "Phaser2: can't open %s\n", filename);
		exit(1);
	}
	tcpfd = SetTCP(HOST, PORT2, 0);
	printf("\nPhase 2: <Seller%d> send item lists.\n", num);
	fseek(fd, 0, SEEK_END);
	long fsize = ftell(fd);
	fseek(fd, 0, SEEK_SET);

	char *msg = malloc(fsize + 1);
	fread(msg, fsize, 1, fd);
	fclose(fd);
	msg[fsize] = '\0';
	printf("Phase 2:\n%s", msg);

	sprintf(buf, "Item#%d %s", num, msg);
	if (write(tcpfd, buf, 256) < 0) {
		close(tcpfd);
		return -1;
	}
	/*
	fgets(owner, 32, fd);
	printf("Phase 2: %s", owner);
	while (fgets(buf, 256, fd) != NULL) {
		printf("Phase 2: %s", buf);
		sprintf(msg, "Item#%d %s %s", num, owner, buf);
		if (write(tcpfd, msg, 256) < 0) {
			close(tcpfd);
			return -1;
		}
	}
	*/
	close(tcpfd);
	return 0;
}


int main(int argc, char *argv[])
{
	pid_t pid;
	int error;
	User seller1, seller2;
	if ((pid = fork()) != 0) { // seller2 child process
		error = Login("test/sellerPass1.txt", &seller1, 1);
		printf("\nEnd of Phase 1 for <Seller%d>.\n", 1);
		if (error < 0) {
			exit(0);
		}
		sleep(20);
		SendItems("test/itemList1.txt", 1);
		printf("\nEnd of Phase 2 for <Seller%d>.\n", 1);
		GetResult(S_TCP_PORT1);
		printf("\nEnd of Phase 3 for <Seller%d>.\n", 1);

	} else { // seller1 parent process
		error = Login("test/sellerPass2.txt", &seller2, 2);
		printf("\nEnd of Phase 1 for <Seller%d>.\n", 2);
		if (error < 0) {
			exit(0);
		}
		sleep(20);
		printf("\n Phaser2:!!!\n");
		SendItems("test/itemList2.txt", 2);
		printf("\nEnd of Phase 2 for <Seller%d>.\n", 2);
		GetResult(S_TCP_PORT2);
		printf("\nEnd of Phase 3 for <Seller%d>.\n", 2);
	}
	return 0;
}
