CFLAGS=-std=c99 -Wall -O3 -DNDEBUG -DUSE_MEM_POOL=1
OPTFLAGS=

OBJS=misc.o ssstr.o dict.o lotos_epoll.o buffer.o request.o response.o \
 connection.o http_parser.o server.o mem_pool.o main.o

lotos : $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(OPTFLAGS)

test :
	make -C ./test
	make test -C ./test

format :
	find . -iname '*.[ch]' -exec clang-format -i -style="{ColumnLimit: 80}" {} +

clean :
	rm -f *.o lotos

.PHONY : test clean format
