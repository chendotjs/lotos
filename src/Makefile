CFLAGS=-g -std=c99 -Wall
OPTFLAGS=

lotos : server.o main.o misc.o lotos_epoll.o
	$(CC) $(CFLAGS) $^ -o $@

format :
	find . -iname '*.[ch]' -exec clang-format -i -style="{ColumnLimit: 100}" {} +

clean :
	rm -f *.o lotos
