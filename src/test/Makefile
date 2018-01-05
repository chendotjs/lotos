CFLAGS=-g -std=c99 -Wall
OPTFLAGS=

all : slow_client

slow_client : slow_client.c ../misc.c
	$(CC) $(CFLAGS) $^ -o $@
	od -c GET_REQ.bin
	
clean :
	rm -f slow_client
