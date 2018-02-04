# Lotos WebServer

**Lotos is a tiny but high-performance HTTP WebServer following the Reactor model, using non-blocking IO and IO multiplexing(epoll ET) to handle concurrency. Lotos is written in pure c and well tested. Several HTTP headers (Connection, Content-Length, etc.) is supported and more will be added in the future.**

## Documents

0x01 | 0x02 | 0x03
---- | ---- | ----
TODO | TODO | TODO

## Environment

- gcc >= 5.4 or clang >= 3.5 (gcc4.9 is not unsupported)
- Linux only, kernel version >= 3.9

## Usage

### Build

```
$ git clone https://github.com/chendotjs/lotos.git
$ cd lotos/src/
$ make && make test
```

### Run

Usage: lotos -r html_root_dir [-p port] [-t timeout] [-w worker_num] [-d (debug mode)]

```
$ ./lotos -r ../www -t 60 -w 4 -p 8888
```

then you can visit <http://localhost:8888/>.

## Feature

- EPOLL Edge Trigger mode, more efficient.
- Nonblocking IO
- Multiprocessing, port reuse.
- TCP connections managed by min-heap data structure.
- HTTP persistent connection. Auto close TCP connection when connection expires.
- Parse HTTP requests using FSM.
- Handle errors and exceptions.

## Test

Unit tests are based on [minctest](https://github.com/codeplea/minctest). It is simple, lightweight, and flexible.

Moreover, I contributed some codes to it.

## Benchmark

Please refer to [BENCHMARK.md](./doc/BENCHMARK.md).
