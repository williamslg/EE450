#ifndef _AUCTIONSERVER_H_
#define _AUCTIONSERVER_H_

#include "common.h"
int LoadRegistration(List *list);
int Checklogin(char *buf, User *user, List *list);
int Saveitems(char *buf, ItemList *ilist);
int Auction(ItemList *ilist);
int SaveMulti(char *buf, ItemList *ilist);
char *Compute(ItemList *ilist, ItemList *bidding1, ItemList *bidding2);
int Sendall(char *msg);


#endif /* _AUCTIONSERVER_H_ */
