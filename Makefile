CC = gcc
CFLAGS = -g -Wall -Wextra 
CLIB = -laio

async_server: async_server.o epoll_utils.o socket_utils.o http_parser.o util.o
		$(CC) $(CFLAGS) $^ -o $@  $(CLIB)

async_server.o: async_server.c
		$(CC) $(CFLAGS) -c $<

epoll_utils.o: epoll_utils.c
		$(CC) $(CFLAGS) -c $<

socket_utils.o: socket_utils.c
		$(CC) $(CFLAGS) -c $< $(CLIB)

http_parser.o: http_parser.c
		$(CC) $(CFLAGS) -c $<

util.o: util.c
		$(CC) $(CFLAGS) -c $<


.PHONY: clean
clean:
		rm -f *.o async_server