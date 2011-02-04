CC = gcc
CFLAGS = -Wall -l pthread -g 
OBJS = common_header.o queue.o client_header.o client.o

client-roulette : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o client-roulette
client.o : client.c
	$(CC) $(CFLAGS) -c client.c
common_header.o : common_header.c common_header.h
	$(CC) $(CFLAGS) -c common_header.c
client_header.o : client_header.c client_header.h
	$(CC) $(CFLAGS) -c client_header.c
queue.o : queue.c queue.h
	$(CC) $(CFLAGS) -c queue.c
clean :
	rm $(OBJS) client-roulette
