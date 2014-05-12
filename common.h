#ifndef _COMMON_H_
#define _COMMON_H_

#define HOST "10.0.2.15"
/* Phase1 server's TCP prot# */
#define PORT1 "1357"

/* Phase2 server's TCP prot# */
#define PORT2 "1457"

/* Phase3 bidders' UDP prot# */
#define B_UDP_PORT1 "3358"
#define B_UDP_PORT2 "3458"

/* Phase3 sellers' TCP prot# */
#define S_TCP_PORT1 "2357"
#define S_TCP_PORT2 "2457"

/* Phase3 bidders' TCP prot# */
#define B_TCP_PORT1 "4357"
#define B_TCP_PORT2 "4457"

/* number of pending connections in listen queue */
#define BACKLOG 10 

/* user(seller/bidder) structure */
typedef struct tagUser {
	int type;	// 0 default, 1 bidder, 2 seller; 
	char name[32];	//username
	char password[32];	//password
	int account;	//account number
	struct tagUser *next;	//ponter points to next user
} User;

/* list structure to contain user */
typedef struct list {
	int num;	//number of users
	struct tagUser *next;
} List;

typedef struct tagItem {
	char owner[32];		// owner's name
	int num;			// owner's # 
	char name[32];	// name of item
	int price;			// price of item
	struct tagItem *next;
} Item;

typedef struct itemlist {
	int num;
	struct tagItem *next;
} ItemList;

int SetTCP(const char *node, const char *port, int type);
int SetUDP(const char *node, const char *port, int type);
int LoadUserPass(char *filename, User *user);
int Login(char *filename, User *user, int num);
int GetResult(char *port);


#endif /* _COMMON_H_ */
