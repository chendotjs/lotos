CFLAGS=-g -std=c99 -Wall
OPTFLAGS=

all : slow_client

slow_client : slow_client.c ../misc.c
	$(CC) $(CFLAGS) $^ -o $@
	
clean :
	rm -f slow_client
