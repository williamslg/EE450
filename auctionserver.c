#include "auctionserver.h"

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


/* Load Registration.txt for the Auction server. 
 * List *list: list to contain users; 
 */
int LoadRegistration(List *list)
{
	FILE *fd;
	User *user, *p;
	p = list->next;

	// open the Registration.txt
	if ((fd = fopen("test/Registration.txt", "r")) == NULL) {
		fprintf(stderr, "Phase1: Server can't open Registration.txt\n");
		exit(1);
	}

	// read and check information from Registration.txt
	while (!feof(fd)) {
		// allocate memory for a user
		user = (User *)malloc(sizeof(User));
		if (user == NULL) {
			fprintf(stderr, "Phase1: Out of memory!\n");
			exit(1);
		}
		// save infomation into user struct.
		fscanf(fd, "%s %s %d", user->name, user->password, &user->account);
		if (feof(fd)) {
			free(user);
			break;
		}
		user->type = 0;
		user->next = NULL;
		// check the first 4 digits of account number
		if (user->account < 451900000 || user->account >451999999) {
			fprintf(stderr, "Phase1: Invalid account information!\n");
			free(user);
			continue;
		} 
		//printf("%s %s %d\n", user->name, user->password, user->account);
		// add the user into the list
		if (list->num == 0) {
			list->next = user;
		}
		else {
			p->next = user;
		}
		p = user;
		list->num++;
	}
	fclose(fd);
	if (list->num) {
		return 0;
	} else {
		return -1;
	}
}


/* IPv4, get sockaddr_in/sockaddr_in6 */
void *GetAddr(struct sockaddr *addr)
{
	return &(((struct sockaddr_in *)addr)->sin_addr);
}

int Authorization(List *list)
{
	int error;
	int fdmax;			// max fd number
	int tcpfd, clientfd;// tcp listen fd and client fd
	struct sockaddr_storage client_addr;
	struct sockaddr_in sa;
	socklen_t sin_size;
	char ip[INET6_ADDRSTRLEN]; //ip contains the readable ip address
	fd_set master;		// master file descriptor list 
	fd_set read_fds;	// temp fd list for select()

	User user;
	// load Registration.txt to list.
	error = LoadRegistration(list);
	if (error != 0) {
		fprintf(stderr, "Please check your Registration.txt!\n");
		exit(1);
	}

	// get a tcp socket and start listening
	tcpfd = SetTCP(HOST, PORT1, 1);

	sin_size = sizeof(sa);
	// get the socket IP and TCP port
	error = getsockname(tcpfd, (struct sockaddr*)&sa, &sin_size) ; //Error checking
	if (error == -1) {
		perror("getsockname");
		exit(1);
	}
	strcpy(ip, inet_ntoa(sa.sin_addr));
	// bidder Phase1 on-screen MSG
	printf("\nPhase 1: Auction server has TCP port number %s and IP address %s\n", PORT1, ip);

	// use select() to implement timeout and exit of Phase1.
	FD_ZERO(&master);	//clear the master and temp sets
	FD_ZERO(&read_fds);

	FD_SET(tcpfd, &master);
	fdmax = tcpfd;

	struct timeval tv;	// set timeot
	tv.tv_sec = 20;
	tv.tv_usec = 0;

	int i, n;
	char buf[256];
	int num = 0;

	// for loop to monitor socket
	for (;;) {
		read_fds = master; // copy
		if ((error = select(fdmax+1, &read_fds, NULL, NULL, &tv)) == -1) {
			perror("select");
			exit(1);
		}
		// select() is timeout, it's time to finish Phase1...
		//if (error == 0||num == 4) {
		if (error == 0) {
			printf("\nEnd of Phase 1 for Auction Server\n");
			close(tcpfd);
			break;
		}
		// select() detects a socket changing its state 
		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) {
				if (i == tcpfd) { // a new connection
					sin_size = sizeof(client_addr);
					clientfd = accept(tcpfd, (struct sockaddr *)&client_addr, &sin_size);
					if (clientfd == -1) {
						perror("accept");
					} else {
						FD_SET(clientfd, &master);
						if (clientfd > fdmax) {
							fdmax = clientfd;
						}
					}
				} else { // data from client
					if ((n = recv(i, buf, sizeof(buf), 0)) <= 0) {
						if (n == 0) {
							//printf("a client hung up\n");
						} else {
							perror("recv");
						}
						close(i);
						FD_CLR(i, &master);
					} else {
						buf[n] = '\0';
						//printf("got data %s\n", buf);
						// parse receive message 
						if (strstr(buf, "Seller#") == NULL) {
							error = Checklogin(buf, &user, list);
							if (error == 0) {
								if (write(i, "Accepted#", 9) < 0) {
									close(i);
									return -1;
								}
								num++;
							} else {
								if (write(i, "Rejected#", 9) < 0) {
									close(i);
									return -1;
								}
							} // end of Checklogin
							// get ip information and print it
							sin_size = sizeof(sa);
							getpeername(i, (struct sockaddr*)&sa, &sin_size);
							printf("\nPhase 1: Authentication request. User%d: Username: %s Password: %s Bank Account: %d User IP Addr: %s. Authorized: %s\n", num, user.name, user.password, user.account, inet_ntoa(sa.sin_addr), (error == 0)?"Yes":"No");
						} else {
							if (user.type == 2) {
								printf("\nPhase 1: Auction Server IP Address:%s PreAuction Port Number: %s sent to the <Seller%c>\n", ip, PORT2, buf[7]);
							}
						}
					} // end of receive data from client
				}
			}
		} // end of for loop
	}
	return 0;
}

// split the login info and check if its a valid user
int Checklogin(char *buf, User *user, List *list)
{
	char *sp;	// starting pointer of the buf
	User *p;
	// find the starting point of arguments
	sp = strchr(buf, '#');
	user->type = atoi(strtok(sp+1, " "));
	strcpy(user->name, strtok(NULL, " "));
	strcpy(user->password, strtok(NULL, " "));
	user->account = atoi(strtok(NULL, " "));
	// check the user list to find the corresponding user
	for (p = list->next; p != NULL; p = p->next) {
		if (strcmp(p->name, user->name) == 0) {	// a name matching
			if (strcmp(p->password, user->password) == 0) {	// pass password check
				if (p->account == user->account) {	// pass account check
					p->type = user->type;	// set the type in the list
					return 0;
				} else {
					return -1;
				}
			} else {
				return -1;
			}
		}
	}
	return -1;
}

int Preauction(ItemList *ilist)
{
	int error;
	int fdmax;			// max fd number
	int tcpfd, clientfd;// tcp listen fd and client fd
	int num;
	struct sockaddr_storage client_addr;
	struct sockaddr_in sa;
	socklen_t sin_size;
	char ip[INET6_ADDRSTRLEN]; //ip contains the readable ip address
	fd_set master;		// master file descriptor list 
	fd_set read_fds;	// temp fd list for select()

	tcpfd = SetTCP(HOST, PORT2, 1);
	sin_size = sizeof(sa);
	// get the socket IP and TCP port
	error = getsockname(tcpfd, (struct sockaddr*)&sa, &sin_size) ; //Error checking
	if (error == -1) {
		perror("getsockname");
		exit(1);
	}
	strcpy(ip, inet_ntoa(sa.sin_addr));
	// bidder Phase1 on-screen MSG
	printf("\nPhase 2: Auction Server IP Address: %s PreAuction TCP Port Number: %s\n", ip, PORT2);

	FD_ZERO(&master);	//clear the master and temp sets
	FD_ZERO(&read_fds);

	FD_SET(tcpfd, &master);
	fdmax = tcpfd;

	struct timeval tv;	// set timeot
	tv.tv_sec = 20;
	tv.tv_usec = 0;

	int i, n;
	char buf[256];

	num = 0;
	for (;;) {
		read_fds = master; // copy
		if ((error = select(fdmax+1, &read_fds, NULL, NULL, &tv)) == -1) {
			perror("select");
			exit(1);
		}
		// select() is timeout, it's time to finish Phase1...
		if (error == 0|| num == 2) {
			printf("\nEnd of Phase 2 for Auction Server\n");
			close(tcpfd);
			break;
		}
		// select() detects a socket changing its state 
		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) {
				if (i == tcpfd) { // a new connection
					sin_size = sizeof(client_addr);
					clientfd = accept(tcpfd, (struct sockaddr *)&client_addr, &sin_size);
					if (clientfd == -1) {
						perror("accept");
					} else {
						FD_SET(clientfd, &master);
						if (clientfd > fdmax) {
							fdmax = clientfd;
						}
					}
				} else { // data from client
					if ((n = recv(i, buf, sizeof(buf), 0)) <= 0) {
						if (n == 0) {
							//printf("a client hung up\n");
						} else {
							perror("recv");
						}
						close(i);
						FD_CLR(i, &master);
					} else {
						num++;
						buf[n] = '\0';
						printf("Phase 2: <Seller%c> send item lists.\n", buf[5]);
						printf("%s\n", buf+7);
						//Saveitems(buf, ilist);
						// parse receive message 
					} // end of receive data from client
				}
			}
		} // end of for loop
	}
	return 0;

}

// parse the recieve buf and insert the item into the list
int Saveitems(char *buf, ItemList *ilist) 
{
	char *sp;
	Item *item, *p;
	item = (Item *)malloc(sizeof(Item));
	if (item == NULL) {
		fprintf(stderr, "Out of memory!\n");
		exit(1);
	}

	item->num = 0;
	sp = strtok(buf, " \n");
	if (sp != NULL) {
		strcpy(item->owner, sp);
	}
	else {
		free(item);
		return 0;
	}
	sp = strtok(NULL, " \n");
	if (sp != NULL) {
		strcpy(item->name, sp);
	} else {
		free(item);
		return 0;
	}
	item->price = atoi(strtok(NULL, " \n"));
	item->next = NULL;

	if (ilist->next == NULL) {
		ilist->next = item;
	} else {
		for (p = ilist->next; p != NULL; p = p->next) {
			if (p->next == NULL) {
				break;
			}
		}
		p->next = item;
	}
	return 0;
}
int SaveMulti(char *buf, ItemList *ilist)
{
	char *sp, *ep;
	sp = buf;
	while (sp != NULL) {
		ep = strchr(sp, '\n');
		if (ep != NULL) {
			*ep = '\0';
		} 
		Saveitems(sp, ilist);
		if (ep != NULL && ep+1 != NULL) {
			sp = ep + 1;
		}
		else {
			sp = NULL;
		}
	}
	return 0;
}

int Auction(ItemList *ilist)
{
	FILE *fd;
	char buf[256];
	int udpfd1, udpfd2;
	struct hostent *he;
	int n;
	struct sockaddr_in sa;
	struct sockaddr_storage client_addr;
	socklen_t sin_size;

	//char *sp;
	ItemList bidding1;
	ItemList bidding2;
	bidding1.num = 0;
	bidding2.num = 0;
	bidding1.next = NULL;
	bidding2.next = NULL;
	// read whole file into msg
	if ((fd = fopen("test/broadcastList.txt", "r")) == NULL) {
		fprintf(stderr, "Phase3: can't open broadcastList.txt\n");
		exit(1);
	}
	fseek(fd, 0, SEEK_END);
	long fsize = ftell(fd);
	fseek(fd, 0, SEEK_SET);

	char *msg = malloc(fsize + 1);
	fread(msg, fsize, 1, fd);
	fclose(fd);
	msg[fsize] = '\0';

	udpfd1 = SetUDP(HOST, B_UDP_PORT1, 0);
	
	if ((he = gethostbyname(HOST)) == NULL) {  // get the host info
		perror("gethostbyname");
		exit(1);
	}

	sin_size = sizeof(sa);
	int error;
	// get the socket IP and UDP port
	error = getsockname(udpfd1, (struct sockaddr*)&sa, &sin_size) ; //Error checking
	if (error == -1) {
		perror("getsockname");
		exit(1);
	}
	// bidder/seller Phase1 on-screen MSG
	printf("\n!!!!!!!Phase 3:Auction Server IP address %s Auction UDP Port Number: %d\n",inet_ntoa(sa.sin_addr),(int)ntohs(sa.sin_port));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(atoi(B_UDP_PORT1));
	sa.sin_addr = *((struct in_addr *)he->h_addr);
	memset(sa.sin_zero, '\0', sizeof(sa.sin_zero));
	sin_size = sizeof(sa);


	if ((n = sendto(udpfd1, msg, fsize-1, 0, (struct sockaddr*)&sa, sin_size)) == -1) {
		perror("sendto");
		exit(1);
	}

	udpfd2 = SetUDP(HOST, B_UDP_PORT2, 0);
	sa.sin_port = htons(atoi(B_UDP_PORT2));
	if ((n = sendto(udpfd2, msg, fsize-1, 0, (struct sockaddr*)&sa, sin_size)) == -1) {
		perror("sendto");
		exit(1);
	}
	// reload the file to the list for auction use.
	if ((fd = fopen("test/broadcastList.txt", "r")) == NULL) {
		fprintf(stderr, "Phase3: can't open broadcastList.txt\n");
		exit(1);
	}
	while (fgets(buf, 255, fd) != NULL) {
		Saveitems(buf, ilist);
	}
	// receive bidding informatin
	sin_size = sizeof(client_addr);
	if((n = recvfrom(udpfd1, buf, 255, 0, (struct sockaddr *)&client_addr, &sin_size)) == -1) {
		perror("recvfrom");
		exit(1);
	}
	buf[n] = '\0';
	printf("Phase 3: Auction Server received a bidding from <Bidder%c>\n\nPhase 3:\n%s",buf[0], buf+1);
	SaveMulti(buf+2, &bidding1);

	if((n = recvfrom(udpfd2, buf, 255, 0, (struct sockaddr *)&client_addr, &sin_size)) == -1) {
		perror("recvfrom");
		exit(1);
	}
	buf[n] = '\0';
	printf("Phase 3: Auction Server received a bidding from <Bidder%c>\n\nPhase 3:\n%s",buf[0], buf+1);
	SaveMulti(buf+2, &bidding2);
	msg = Compute(ilist, &bidding1, &bidding2);
	sleep(5);
	close(udpfd1);
	close(udpfd2);
	Sendall(msg);
	printf("End of Phase 3 for Auction Server.\n");
	return 0;

}

char *Compute(ItemList *ilist, ItemList *bidding1, ItemList *bidding2)
{
	char *buf = (char *)malloc(sizeof(char)*128);

	Item *p1, *p2, *p3, *max;
	/*
	for (p1 = ilist->next; p1 != NULL; p1 = p1->next) {
		printf("Total: %s %s %d\n", p1->owner, p1->name, p1->price);
	}
	for (p2 = bidding1->next; p2 != NULL; p2 = p2->next) {
		printf("bidding1: %s %s %d\n", p2->owner, p2->name, p2->price);
	}
	for (p3 = bidding2->next; p3 != NULL; p3 = p3->next) {
		printf("bidding2: %s %s %d\n", p3->owner, p3->name, p3->price);
	}
	*/
	for (p1 = ilist->next; p1 != NULL; p1 = p1->next) {
		max = NULL;
		for (p2 = bidding1->next; p2 != NULL; p2 = p2->next) {
			if (strcmp(p1->owner, p2->owner) == 0 && strcmp(p1->name, p2->name) == 0) {
				if (p1->price > p2->price) {
					max = p1;
				} else {
					max = p2;
				}
				break;
			}
		}
		for (p3 = bidding2->next; p3 != NULL; p3 = p3->next) {
			if (strcmp(p1->owner, p3->owner) == 0 && strcmp(p1->name, p3->name) == 0) {
				if (max == NULL) {
					if (p1->price > p3->price) {
						break;
					}
					else {
						//printf("Find a max: %s %s %d\n", p3->owner, p3->name, p3->price);
						sprintf(buf, "%sPhase 3:Item %s %s was sold at price %d\n", buf, p3->owner, p3->name, p3->price);
					}
				}
				else {
					if (max == p1) {
						if (p1->price > p3->price) {
							break;
						}
						else {
							//printf("Find a max: %s %s %d\n", p3->owner, p3->name, p3->price);
							sprintf(buf, "%sPhase 3:Item %s %s was sold at price %d\n", buf,p3->owner, p3->name, p3->price);
						}
					} else {
						if (p2->price > p3->price) {
							//printf("Find a max: %s %s %d\n", p2->owner, p2->name, p2->price);
							sprintf(buf, "%sPhase 3:Item %s %s was sold at price %d\n", buf, p2->owner, p2->name, p2->price);
						}
						else {
							//printf("Find a max: %s %s %d\n", p3->owner, p3->name, p3->price);
							sprintf(buf, "%sPhase 3:Item %s %s was sold at price %d\n", buf, p3->owner, p3->name, p3->price);
						}
					}
				}
				break;
			}
		}
		if (max != NULL && p3 == NULL && max == p2) {
			//printf("Find a max: %s %s %d\n", p2->owner, p2->name, p2->price);
			sprintf(buf, "%sPhase 3:Item %s %s was sold at price %d\n",buf, p2->owner, p2->name, p2->price);
		}
	}
	
	printf("%s\n", buf);

	return buf;
}
int Sendall(char *msg)
{
	int tcpfd1, tcpfd2, tcpfd3, tcpfd4;
	tcpfd1 = SetTCP(HOST, S_TCP_PORT1, 0);
	write(tcpfd1, msg, 255);
	tcpfd2 = SetTCP(HOST, S_TCP_PORT2, 0);
	write(tcpfd2, msg, 255);
	tcpfd3 = SetTCP(HOST, B_TCP_PORT1, 0);
	write(tcpfd3, msg, 255);
	tcpfd4 = SetTCP(HOST, B_TCP_PORT2, 0);
	write(tcpfd4, msg, 255);
	return 0;
}
int main(int argc, char *agrv[])
{
	List list;			// list contains the users' information
	list.num = 0;
	list.next = NULL;
	ItemList ilist;		// list contains the items
	ilist.num = 0;
	ilist.next = NULL;
	Authorization(&list);
	Preauction(&ilist);
	Auction(&ilist);
	return 0;
}
