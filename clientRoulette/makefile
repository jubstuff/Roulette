CC = gcc
CFLAGS = -Wall -l pthread -g 
OBJS = common_header.o client_header.o client.o

client-roulette : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o client-roulette
client.o : client.c
	$(CC) $(CFLAGS) -c client.c
common_header.o : ../common/common_header.c ../common/common_header.h
	$(CC) $(CFLAGS) -c ../common/common_header.c
client_header.o : client_header.c client_header.h
	$(CC) $(CFLAGS) -c client_header.c
clean :
	rm $(OBJS) client-roulette
