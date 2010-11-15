CC = gcc
CFLAGS = -Wall -l pthread
OBJS = common_header.o estrazione.o queue.o control.o #list_management.o

server-roulette : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS)
common_header.o : common_header.c common_header.h
	$(CC) $(CFLAGS) -c common_header.c
estrazione.o : estrazione.c
	$(CC)  $(CFLAGS) -c estrazione.c
queue.o : queue.c queue.h
	$(CC) $(CFLAGS) -c queue.c
control.o : control.c control.h
	$(CC) $(CFLAGS) -c control.c
#list_management.o : list_management.c list_management.h
#	$(CC) $(CFLAGS) -c list_management.c
clean :
	rm $(OBJS) a.out