CC = gcc
CFLAGS = -Wall -l pthread -g
OBJS = common_header.o estrazione.o queue.o player.o croupier.o

server-roulette : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o server-roulette
common_header.o : ../common/common_header.c ../common/common_header.h
	$(CC) $(CFLAGS) -c ../common/common_header.c
estrazione.o : estrazione.c
	$(CC)  $(CFLAGS) -c estrazione.c
player.o : player.c player.h
	$(CC) $(CFLAGS) -c player.c
croupier.o : croupier.c croupier.h
	$(CC) $(CFLAGS) -c croupier.c
queue.o : queue.c queue.h
	$(CC) $(CFLAGS) -c queue.c
clean :
	rm $(OBJS) server-roulette 
