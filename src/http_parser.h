#ifndef _PARSER_H__
#define _PARSER_H__

#include "buffer.h"
#include "misc.h"
#include "ssstr.h"
#include <string.h>

/* RFC2616 */
/* Ref: https://www.w3.org/Protocols/rfc2616/rfc2616.html */
/* This is a simple implementation */

/**
 *
 * Request       = Request-Line                   ; Section 5.1
                          (( general-header       ; Section 4.5
                         | request-header         ; Section 5.3
                         | entity-header ) CRLF)  ; Section 7.1
                        CRLF
                        [ message-body ]          ; Section 4.3
 */

/**
 *  Request-Line   = Method SP Request-URI SP HTTP-Version CRLF
 */

/**
 *  Ref: https://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html#sec3.2
 *  http_URL = "http:" "//" host [ ":" port ] [ abs_path [ "?" query ]]
 */

/**
 *        request-header = Accept                 ; Section 14.1
                      | Accept-Charset           ; Section 14.2
                      | Accept-Encoding          ; Section 14.3
                      | Accept-Language          ; Section 14.4
                      | Authorization            ; Section 14.8
                      | Expect                   ; Section 14.20
                      | From                     ; Section 14.22
                      | Host                     ; Section 14.23
                      | If-Match                 ; Section 14.24
                      | If-Modified-Since        ; Section 14.25
                      | If-None-Match            ; Section 14.26
                      | If-Range                 ; Section 14.27
                      | If-Unmodified-Since      ; Section 14.28
                      | Max-Forwards             ; Section 14.31
                      | Proxy-Authorization      ; Section 14.34
                      | Range                    ; Section 14.35
                      | Referer                  ; Section 14.36
                      | TE                       ; Section 14.39
                      | User-Agent               ; Section 14.43
 */

#define INVALID_REQUEST (-1)
#define CRLF_LINE (2)

#define MAX_ELEMENT_SIZE (2048)

/* basic http method */
typedef enum {
  HTTP_DELETE,
  HTTP_GET,
  HTTP_HEAD,
  HTTP_POST,
  HTTP_PUT,
  HTTP_INVALID,
} http_method;

/* HTTP protocol version */
typedef struct {
  unsigned short http_major;
  unsigned short http_minor;
} http_version;

/* Request URL */
typedef struct {
  ssstr_t abs_path;
  ssstr_t query_string;
  ssstr_t mime_extension;
} req_url;

/* parser state used in fsm */
typedef enum {
  /* request line states */
  S_RL_BEGIN = 0,
  S_RL_METHOD,
  S_RL_SP_BEFORE_URL,
  S_RL_URL,
  S_RL_SP_BEFORE_VERSION,
  S_RL_VERSION_H,
  S_RL_VERSION_HT,
  S_RL_VERSION_HTT,
  S_RL_VERSION_HTTP,
  S_RL_VERSION_HTTP_SLASH,
  S_RL_VERSION_MAJOR,
  S_RL_VERSION_DOT,
  S_RL_VERSION_MINOR,
  S_RL_CR_AFTER_VERSION,
  S_RL_LF_AFTER_VERSION,

  /* header states */
  S_HD_BEGIN,
  S_HD_NAME,
  S_HD_COLON,
  S_HD_SP_BEFORE_VAL,
  S_HD_VAL,
  S_HD_CR_AFTER_VAL,
  S_HD_LF_AFTER_VAL,

  /* url states */
  S_URL_BEGIN,
  S_URL_ABS_PATH,
  S_URL_QUERY,
  S_URL_END,
} parser_state;

// https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Transfer-Encoding
typedef enum {
  TE_IDENTITY = 0,
  TE_CHUNKED,
  TE_COMPRESS,
  TE_DEFLATE,
  TE_GZIP,
} transfer_encoding_t;

/* some of the request headers we may parse */
typedef struct {
  ssstr_t cache_control;
  ssstr_t connection;
  ssstr_t date;
  ssstr_t transfer_encoding;
  ssstr_t accept;
  ssstr_t accept_charset;
  ssstr_t accept_encoding;
  ssstr_t accept_language;
  ssstr_t cookie;
  ssstr_t host;
  ssstr_t if_modified_since;
  ssstr_t if_unmodified_since;
  ssstr_t max_forwards;
  ssstr_t range;
  ssstr_t referer;
  ssstr_t user_agent;
  ssstr_t content_length;
} request_headers_t;

typedef struct {
  /* parsed request line result */
  http_method method;
  http_version version;
  ssstr_t request_url_string;
  req_url url;
#if 0
  ssstr_t request_path;
  ssstr_t query_string;
  ssstr_t mime_extension;
#endif

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
  bool isCRLF_LINE;
  size_t body_received;
  bool response_done;
  int buffer_sent;
} parse_archive;

static inline void parse_archive_init(parse_archive *ar, buffer_t *b) {
  memset(ar, 0, sizeof(parse_archive));
  ar->next_parse_pos = b->buf;
  ar->isCRLF_LINE = TRUE;
  ar->content_length = -1; // no Content-Length header
}

extern int parse_request_line(buffer_t *b, parse_archive *ar);
extern int parse_header_line(buffer_t *b, parse_archive *ar);
extern int parse_header_body_identity(buffer_t *b, parse_archive *ar);

#endif
