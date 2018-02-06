# 设计与实现

## 0x01 重要的结构体

1.配置信息结构(server.h)

```c
typedef struct {
  uint16_t port;   /* listen port */
  bool debug;      /* debug mode */
  int timeout;     /* connection expired time */
  uint32_t worker; /* worker num */
  char *rootdir;   /* html root directory */
  int rootdir_fd;  /* fildes of rootdir */
} config_t;
```

2.请求/响应缓冲结构(buffer.h)

```c
typedef struct {
  int len;    /* used space length in buf */
  int free;   /* free space length in buf */
  char buf[]; /* store data */
} buffer_t;
```

采用柔性数组，可以动态增长，围绕该结构设计了一系列操作函数。

3.简单静态字符串结构(ssstr.h)

```c
typedef struct {
  char *str;
  int len;
} ssstr_t;
```

对于字符串字面常量以及`buffer_t`结构的子串，都可以用该结构描述而无需开启新的缓冲区存储。有效节省了空间。

4.HTTP连接结构(connection.h)

```c
struct connection {
  int fd;                   /* connection fildes */
  struct epoll_event event; /* epoll event */
  struct sockaddr_in saddr; /* IP socket address */
  time_t active_time;       /* connection accpet time */
  int heap_idx;             /* idx at lotos_connections */
  request_t req;            /* request */
};
typedef struct connection connection_t;
```

5.请求信息结构(request.h)

```c
struct request {
  struct connection *c;                 /* belonged connection */
  buffer_t *ib;                         /* request buffer */
  buffer_t *ob;                         /* response buffer */
  parse_archive par;                    /* parse_archive */
  int resource_fd;                      /* resource fildes */
  int resource_size;                    /* resource size */
  int status_code;                      /* response status code */
  int (*req_handler)(struct request *); /* request handler for rl, hd, bd */
  int (*res_handler)(struct request *); /* response handler for hd bd */
};
typedef struct request request_t;
```

6.HTTP请求解析结构(http_parser.h)

```c
typedef struct {
  /* parsed request line result */
  http_method method;
  http_version version;
  ssstr_t request_url_string;
  req_url url;

  /* parsed header lines result */
  bool keep_alive;       /* connection keep alive */
  int content_length;    /* request body content_length */
  int transfer_encoding; /* affect body recv strategy */
  request_headers_t req_headers;

  int num_headers;
  ssstr_t header[2]; /* store header every time `parse_header_line` */

  /* preserve buffer_t state, so when recv new data, we can keep parsing */
  char *next_parse_pos; /* parser position in buffer_t */
  int state;            /* parser state */

  /* private members, do not modify !!! */
  char *method_begin;
  char *url_begin;
  char *header_line_begin;
  char *header_colon_pos;
  char *header_val_begin;
  char *header_val_end;
  size_t body_received;
  int buffer_sent;
  bool isCRLF_LINE;
  bool response_done;
  bool err_req;
} parse_archive;
```

7.错误页面结构(response.h)

```c
typedef struct {
  int err_page_fd;             /* fildes of err page */
  const char *raw_err_page;    /* raw data of err page file */
  size_t raw_page_size;        /* size of err page file */
  buffer_t *rendered_err_page; /* buffer contains err msg */
  size_t rendered_page_size;   /* size of err page file */
} err_page_t;
```

## 0x02 数据结构

### 1.最小堆

### 2.HashMap

buffer_t 和 parser 紧密耦合，与其他部件解耦，可以进行模拟测试。request_t包含这两者，让两者耦合

为了性能，抽象出ssstr_t类型作为parser_archive成员，避免了内存的拷贝

# 测试

写了简单的单元测试，在开发过程中多次修改结构，有单元测试对于重构帮助很大

并发的瓶颈不在于内存的malloc和free（有内存池更好），如果可以利用HTTP/1.1中keep-alive功能，加入Content-Length或者Chunked可以极大增加并发数.
