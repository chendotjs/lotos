CFLAGS=-g -std=c99 -Wall
OPTFLAGS=

OBJS= server.o misc.o lotos_epoll.o connection.o main.o

lotos : $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

format :
	find . -iname '*.[ch]' -exec clang-format -i -style="{ColumnLimit: 80}" {} +

clean :
	rm -f *.o lotos
