CFLAGS=-g -std=c99 -Wall
OPTFLAGS=

OBJS=slow_client heap_test buffer_test ssstr_test parse_test

all : $(OBJS)

slow_client : slow_client.c ../misc.c
	$(CC) $(CFLAGS) $^ -o $@

heap_test : heap_test.c
	$(CC) $(CFLAGS) $^ -o $@

buffer_test : buffer_test.c ../buffer.c
	$(CC) $(CFLAGS) $^ -o $@

ssstr_test : ssstr_test.c ../ssstr.c
	$(CC) $(CFLAGS) $^ -o $@

parse_test : parse_test.c ../http_parser.c ../buffer.c
	$(CC) $(CFLAGS) $^ -o $@

test :
	./heap_test
	./buffer_test
	./ssstr_test
	./parse_test

clean :
	rm -f $(OBJS)
