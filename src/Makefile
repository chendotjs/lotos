CFLAGS=-g -std=c99 -Wall
OPTFLAGS=

lotos : server.o main.o misc.o
	$(CC) $(CFLAGS) $^ -o $@

clean :
	rm -f *.o lotos
