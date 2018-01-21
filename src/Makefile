CFLAGS=-g -std=c99 -Wall
OPTFLAGS=

OBJS=misc.o ssstr.o lotos_epoll.o buffer.o request.o connection.o \
 http_parser.o server.o main.o

lotos : $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

format :
	find . -iname '*.[ch]' -exec clang-format -i -style="{ColumnLimit: 80}" {} +

clean :
	rm -f *.o lotos
