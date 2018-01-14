CFLAGS=-g -std=c99 -Wall
OPTFLAGS=

all : slow_client heap_test buffer_test parse_test

slow_client : slow_client.c ../misc.c
	$(CC) $(CFLAGS) $^ -o $@

heap_test : heap_test.c
	$(CC) $(CFLAGS) $^ -o $@

buffer_test : buffer_test.c ../buffer.c
	$(CC) $(CFLAGS) $^ -o $@

parse_test : parse_test.c ../http_parser.c ../buffer.c
	$(CC) $(CFLAGS) $^ -o $@

test :
	./heap_test
	./buffer_test
	./parse_test

clean :
	rm -f slow_client heap_test buffer_test parse_test
