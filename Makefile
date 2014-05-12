all: bidder seller auctionserver
seller: seller.o common.o
	gcc -o seller -g seller.o common.o -lnsl -lresolv
bidder: bidder.o common.o
	gcc -o bidder -g bidder.o common.o -lnsl -lresolv
auctionserver: auctionserver.o common.o
	gcc -o auctionserver -g auctionserver.o common.o -lnsl -lresolv
seller.o: seller.c seller.h common.h
	gcc -g -c -Wall seller.c
bidder.o: bidder.c bidder.h common.h
	gcc -g -c -Wall bidder.c
auctionserver.o: auctionserver.c auctionserver.h common.h
	gcc -g -c -Wall auctionserver.c
common.o: common.c common.h
	gcc -g -c -Wall common.c
clean:
	rm -f *.o bidder seller auctionserver
